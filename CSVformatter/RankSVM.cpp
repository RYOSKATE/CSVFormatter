#include "RankSVM.h"
#include "FileIO.h"
#include "Param.h"
#include "Node.h"
#include "CSV.h"
#include "combination.h"
#define MULTI_THREAD
//#define MULTI_THREAD_TCP
void RankSVM::replaceCSV(CSV& csv)
{
	contestReplace(csv);
	manzokuReplace(csv);
	jikanReplace(csv);
	taidoReplace(csv);
	sinriReplace(csv);
	kibouReplace(csv);
}

void RankSVM::reformCSV(CSV& csv)
{
	csv.erase(csv.begin());//ヘッダ削除
	Erase_if(csv, [](const std::vector<std::string>& row) {return std::stoi(row[0]) <= 0; });//順位情報なしを削除

	for (int y = 0; y < csv.size(); ++y)
	{
		for (int x = 2; x < csv[y].size(); ++x)
		{
			csv[y][x] = std::to_string(x - 1) + ":" + csv[y][x];
		}
	}
}

void RankSVM::reformCSVForFeature(CSV& csv, const std::vector<char>& useElements, bool isAdd)
{
	csv.erase(csv.begin());//ヘッダ削除
	Erase_if(csv, [](const std::vector<std::string>& row) {return std::stoi(row[0]) <= 0; });//順位情報なしを削除

	{
		CSV tmp(csv.size());
		for (int y = 0; y < csv.size(); ++y)
		{
			tmp.reserve(csv[y].size());
			tmp[y].push_back(csv[y][0]);
			tmp[y].push_back(csv[y][1]);
			for (int x = 2; x < csv[y].size(); ++x)
			{
				if (isAdd ^ (std::find(useElements.begin(), useElements.end(), x - 2) == useElements.end()))
				{
					tmp[y].push_back(csv[y][x]);
				}
			}
			tmp[y].shrink_to_fit();
		}
		csv = std::move(tmp);
	}

	for (int y = 0; y < csv.size(); ++y)
	{
		for (int x = 2; x < csv[y].size(); ++x)
		{
			csv[y][x] = std::to_string(x - 1) + ":" + csv[y][x];
		}
	}
}


void RankSVM::sortCSVbyFirstCol(CSV& csv)
{
	std::sort(csv.begin(), csv.end(), [](const std::vector<std::string>& a, const std::vector<std::string>& b) {return std::stoi(a[0]) < std::stoi(b[0]); });
}

double RankSVM::learn(Param param, size_t i, const std::vector<CSV>& data)
{
	const size_t numOfRows = std::accumulate(data.begin(), data.end(), (size_t)0, [](size_t init, const CSV& data) {return init + data.size(); });
	param.setFileName(i + 1);

	CSV train;
	train.reserve(numOfRows - data[i].size());

	for (size_t k = 0; k < numOfFoldCrossValidation; ++k)
	{
		if (i == k)
		{
			CSV test = data[k];
			sortCSVbyFirstCol(test);
			FileIO::saveCSV(param.testFileName, test, ' ');
		}
		else
		{
			std::copy(data[k].begin(), data[k].end(), std::back_inserter(train));
		}
	}
	sortCSVbyFirstCol(train);
	FileIO::saveCSV(param.trainFileName, train, ' ');

	auto command = param.toString();
	std::string argv1 = currentDir + "svm_rank_learn.exe";
	for (const auto& c : command)
	{
		argv1 += " ";
		argv1 += c;
	}

	argv1 += " > " + param.tmpFileName;
	const int ret1 = system(argv1.c_str());

	//学習に失敗
	if (ret1 != 0 || !std::experimental::filesystem::exists(param.modelFileName))
	{
		return -1.0;
	}
	std::string argv2 = currentDir + std::string("svm_rank_classify.exe ");
	argv2 += param.testFileName.c_str() + std::string(" ") + param.modelFileName.c_str() + " " + param.predFileName.c_str();
	argv2 += " > " + param.tmpFileName;
	const int ret2 = system(argv2.c_str());

	//予測に失敗
	if (ret2 != 0 || !std::experimental::filesystem::exists(param.testFileName))
	{
		return -1.0;
	}

	//test読み込み
	const auto test = FileIO::loadCSVtoString(param.testFileName, ' ');
	std::vector<double> y_true;//74-真の順位(昇順)
	y_true.reserve(test.size());
	for (const auto& t : test)
	{
		y_true.push_back(maxContestRank - std::stod(t.front()) + 1);
	}

	//pred読み込み
	const auto pred_str = FileIO::loadCSVtoString(param.predFileName, ' ');

	std::vector<std::pair<double, double>> _y_pred;//(真の順位,予想値)
	_y_pred.reserve(pred_str.size());
	for (int m = 0; m<y_true.size(); ++m)
	{
		_y_pred.emplace_back(std::make_pair(y_true[m], std::stod(pred_str[m][0])));
	}

	auto allOfUnique = [](decltype(_y_pred) _y_pred)->bool
	{
		const size_t size = _y_pred.size();
		_y_pred.erase(std::unique(_y_pred.begin(), _y_pred.end(), [](std::pair<double, double> a, std::pair<double, double> b) {return a.second == b.second; }), _y_pred.end());
		return size == _y_pred.size();
	};

	auto dcg = [](const std::vector<double>& y)->double
	{
		double ret = y.front();
		for (int i = 1; i < y.size(); ++i)
		{
			ret += (y[i] / log2(i + 1));
		}
		return ret;
	};

	const double dcg_true = dcg(y_true);
	//if (i == 2)
	//{
	//	for (int k = 0; k < _y_pred.size(); ++k)
	//	{
	//		ofs2 << _y_pred[k].first << ",";
	//	}
	//	ofs2 << "\n";
	//}

	//予想値の順に並び替え
	std::stable_sort(_y_pred.begin(), _y_pred.end(), [](std::pair<double, double> a, std::pair<double, double> b) {return a.second < b.second; });
	std::vector<double> y_pred(_y_pred.size());
	for (int m = 0; m < y_pred.size(); ++m)
	{
		y_pred[m] = _y_pred[m].first;
	}
	//if (i == 2)
	//{
	//	for (int k = 0; k < _y_pred.size(); ++k)
	//	{
	//		ofs2 << _y_pred[k].first << ",";
	//	}
	//}
	if (allOfUnique(_y_pred))//予想に重複なし
	{
		const double dcg_pred = dcg(y_pred);
		const double dcg = dcg_pred / dcg_true;
		return dcg;
	}

	//予想に重複あり
	constexpr int numOfRandomTry = 100;//この回数だけ同率順位のものの中で値をランダムに入れ替え平均をとる。
	std::unordered_map<double, std::vector<int>> doubled;
	for (size_t k = 0; k < _y_pred.size(); ++k)
	{
		for (size_t m = k; m < _y_pred.size(); ++m)
		{
			//重複
			if (_y_pred[k].second == _y_pred[m].second)
			{
				doubled[_y_pred[k].second].push_back(m);
			}
			else
			{
				break;
			}
		}
		k += doubled[_y_pred[k].second].size() - 1;
	}

	std::array<double, numOfRandomTry> dcg_preds;
	for (int k = 0; k < numOfRandomTry; ++k)
	{
		for (const auto& d : doubled)
		{
			std::vector<double> buf;
			buf.reserve(d.second.size());
			for (auto ra : d.second)
			{
				buf.push_back(y_pred[ra]);
			}
			std::random_shuffle(buf.begin(), buf.end());
			for (int m = 0; m < buf.size(); ++m)
			{
				y_pred[d.second[m]] = buf[m];
			}
		}
		const double dcg_pred = dcg(y_pred);
		dcg_preds[k] = dcg_pred / dcg_true;
	}
	return std::accumulate(dcg_preds.begin(), dcg_preds.end(), 0.0) / numOfRandomTry;
	//auto y_bad = y_true;
	//std::reverse(y_bad.begin(), y_bad.end());
	//const double dcg_bad = dcg(y_bad)/ dcg_true;
	//dcgs[i] = (dcgs[i] - dcg_bad) / (1.0 - dcg_bad);
}

std::vector<CSV> RankSVM::getDividedCSV()
{
	auto csv = FileIO::loadCSVtoString("combine.csv");
	replaceCSV(csv);
	reformCSV(csv);
	sortCSVbyFirstCol(csv);
	FileIO::saveCSV("convert_sorted.csv", csv, ' ');
	std::random_shuffle(csv.begin(), csv.end());
	return divide(std::move(csv), numOfFoldCrossValidation);
}


int RankSVM::mainParam(int argc, char* argv[])
{
	const auto data = getDividedCSV();
	std::vector<double> ndcgAves;
	std::vector<std::string> params;

	std::array<std::array<int, 3>, 8> tops =
	{
		{
			{ 0,0,0 },
			{ 0,0,1 },
			{ 0,1,0 },
			{ 0,1,1 },
			{ 1,0,0 },
			{ 1,0,1 },
			{ 1,1,0 },
			{ 1,1,1 }
		}
	};
#ifdef MULTI_THREAD_TCP
	Concurrency::parallel_for_each(tops.begin(), tops.end(), [&](const std::array<int, 3>& top)
	{
		int index_t = top[0];
		int index_o = top[1];
		int index_p = top[2];
#else
	for (int index_t = 0; index_t < 2; ++index_t)
	{
		for (int index_o = 0; index_o < 2; ++index_o)
		{
			for (int index_p = 0; index_p < 2; ++index_p)
			{
#endif
				for (int index_c = 1; index_c <= 7; ++index_c)

				{
					for (int index_g = 1; index_g <= 7; ++index_g)
					{
						std::vector<double> dcgs(numOfFoldCrossValidation);
						Param default_param;
						default_param.c = std::pow(10.0, -1 * index_c);
						default_param.p = index_p + 1;
						default_param.o = index_o + 1;
						if (index_t == 0)
						{
							default_param.t = Param::Kernel::LINEAR;
							index_g = 7;
						}
						else
						{
							default_param.t = Param::Kernel::RBF;
							default_param.g = std::pow(10.0, -1 * index_g);
						}

#ifdef MULTI_THREAD
						concurrency::parallel_for(0, numOfFoldCrossValidation, [&](size_t i)
#else
						for (size_t i = 0; i < numOfFoldCrossValidation; ++i)
#endif
						{

							dcgs[i] = learn(default_param, i, data);
						}
#ifdef MULTI_THREAD
						);
#endif

						//パラメータ記録
						const auto paramStrVec = default_param.toStringWithoutFilename();
						std::string paramStr;
						for (const auto& paramstr : paramStrVec)
						{
							paramStr += paramstr + " ";
						}
						params.push_back(paramStr);


						//ndcg記録
						if (std::any_of(dcgs.begin(), dcgs.end(), [](double d) {return d == -1; }))
						{
							ndcgAves.push_back(0.0);
							continue;
						}
						else
						{
							const double dcgAve = std::accumulate(dcgs.begin(), dcgs.end(), 0.0) / dcgs.size();
							std::cout << dcgAve << "\n";
							ndcgAves.push_back(dcgAve);
						}

					}
				}
			}

#ifdef MULTI_THREAD_TCP
			);
#else
		}
	}
#endif
	std::ofstream ofs("test.csv", std::ios::trunc);
	for (int i = 0; i < ndcgAves.size(); ++i)
	{
		ofs << i << "," << ndcgAves[i] << "," << params[i] << "\n";
	}
	return 0;
	}

int RankSVM::mainBruteforce(size_t n, size_t r)
{
	constexpr bool isAdd = true;
	auto originalCSV = FileIO::loadCSVtoString("combine.csv");
	replaceCSV(originalCSV);

	std::vector<double> ndcgAves;
	std::vector<std::string> params;

	size_t count = 0;
	combination<char>(n, r, [&](const std::vector<char>& arr)
	{
		auto csv = originalCSV;
		reformCSVForFeature(csv, arr, isAdd);
		sortCSVbyFirstCol(csv);
		//FileIO::saveCSV("convert_sorted.csv", csv, ' ');
		std::random_shuffle(csv.begin(), csv.end());
		const auto data = divide(std::move(csv), numOfFoldCrossValidation);

		//0,1,0は落ちる
		std::array<std::array<int, 3>, 8> tops =
		{
			{
				{ 0,0,0 },
				{ 0,0,1 },
				//{ 0,1,0 },
				{ 0,1,1 },
				{ 1,0,0 },
				{ 1,0,1 },
				//{ 1,1,0 },
				{ 1,1,1 }
			}
		};
		std::mutex key;
#ifdef MULTI_THREAD_TCP
		Concurrency::parallel_for_each(tops.begin(), tops.end(), [&](const std::array<int, 3>& top)
		{
			int index_t = top[0];
			int index_o = top[1];
			int index_p = top[2];
#else
		for (int index_t = 0; index_t < 2; ++index_t)
		{
			for (int index_o = 0; index_o < 2; ++index_o)
			{
				for (int index_p = 0; index_p < 2; ++index_p)
				{
#endif
					for (int index_c = 1; index_c <= 7; ++index_c)

					{
						for (int index_g = 1; index_g <= 7; ++index_g)
						{
							std::vector<double> dcgs(numOfFoldCrossValidation);
							Param default_param;
							default_param.c = std::pow(10.0, -1 * index_c);
							default_param.p = index_p + 1;
							default_param.o = index_o + 1;
							if (index_t == 0)
							{
								default_param.t = Param::Kernel::LINEAR;
								index_g = 7;
							}
							else
							{
								default_param.t = Param::Kernel::RBF;
								default_param.g = std::pow(10.0, -1 * index_g);
							}
#ifdef MULTI_THREAD
							concurrency::parallel_for(0, numOfFoldCrossValidation, [&](size_t i)
#else
							for (size_t i = 0; i < numOfFoldCrossValidation; ++i)
#endif
							{
								dcgs[i] = learn(default_param, i, data);
							}
#ifdef MULTI_THREAD
							);
#endif
	//パラメータ記録
	const auto paramStrVec = default_param.toStringWithoutFilename();
	std::string paramStr;
	for (auto ar : arr)
	{
		paramStr += std::to_string(ar) + ",";
	}
	paramStr += ",";
	for (const auto& paramstr : paramStrVec)
	{
		paramStr += paramstr + " ";
	}

	std::lock_guard<std::mutex> locker(key);
	params.push_back(paramStr);

	//ndcgの平均記録
	if (std::any_of(dcgs.begin(), dcgs.end(), [](double d) {return d == -1; }))
	{
		ndcgAves.push_back(-1.0f);
	}
	else
	{
		const double dcgAve = std::accumulate(dcgs.begin(), dcgs.end(), 0.0) / dcgs.size();
		//std::cout << dcgAve << "\n";
		//ofs2 << dcgAve << "\n";
		ndcgAves.push_back(dcgAve);
	}
	std::cout << ++count << "," << ndcgAves.back() << "," << params.back() << "\n";
						}
					}
				}
#ifdef MULTI_THREAD_TCP
				);
#else
			}
		}
#endif
		});

		const std::string outFilename = (isAdd ? "Add" : "Sub") + std::string("Bruteforce.csv");
		std::ofstream ofs(outFilename, std::ios::trunc);
		for (int i = 0; i < ndcgAves.size(); ++i)
		{
			ofs << i << "," << ndcgAves[i] << "," << params[i] << "\n";
		}
		return 0;
	}

int RankSVM::mainFeature(int argc, char* argv[])
{
	constexpr bool isAdd = true;
	std::vector<std::vector<char>> records;
	std::stack<std::vector<Node>> nodesPool;
	{
		std::vector<Node> nodes;
		for (size_t i = 0; i < numOfElements; ++i)
		{
			nodes.emplace_back(1, (char)i);
		}
		nodesPool.emplace(std::move(nodes));
	}
	std::ofstream ofs("test_step_add.csv", std::ios::trunc);
	std::ofstream ofs2("pred.csv", std::ios::trunc);
	double parentValue = 0.0;
	while (!nodesPool.empty())
	{
		auto nodes = nodesPool.top();
		nodesPool.pop();

		//for (auto& ar : arr)
		for (auto& node : nodes)
		{
			//Node node;
			//for (auto a : ar)
			//{
			//	node.indexes.push_back(a);
			//}
			auto csv = FileIO::loadCSVtoString("combine.csv");
			replaceCSV(csv);
			const auto& useElements = node.indexes;
			reformCSVForFeature(csv, useElements, isAdd);
			sortCSVbyFirstCol(csv);
			FileIO::saveCSV("convert_sorted.csv", csv, ' ');
			std::random_shuffle(csv.begin(), csv.end());
			const auto data = divide(std::move(csv), numOfFoldCrossValidation);

			std::vector<double> ndcgAves;
			std::vector<std::string> params;

			std::vector<double> dcgs(numOfFoldCrossValidation, -1);
			Param default_param;
			default_param.c = 0.000010f;
			default_param.p = 1;
			default_param.o = 2;
			default_param.l = 1;
			default_param.t = Param::Kernel::RBF;
			default_param.g = 0.000100f;
#ifdef MULTI_THREAD
			concurrency::parallel_for(0, numOfFoldCrossValidation, [&](size_t i)
#else
			for (size_t i = 0; i < numOfFoldCrossValidation; ++i)
#endif
			{
				dcgs[i] = learn(default_param, i, data);
			}
#ifdef MULTI_THREAD
			);
#endif
			const auto paramStrVec = default_param.toStringWithoutFilename();
			std::string paramStr;
			for (const auto& paramstr : paramStrVec)
			{
				paramStr += paramstr + " ";
			}
			params.push_back(paramStr);

			if (std::any_of(dcgs.begin(), dcgs.end(), [](double d) {return d == -1; }))
			{
				ndcgAves.push_back(-1.0f);
			}
			else
			{
				const double dcgAve = std::accumulate(dcgs.begin(), dcgs.end(), 0.0) / dcgs.size();
				ofs2 << dcgAve << "\n";
				std::cout << dcgAve << "\n";
				ndcgAves.push_back(dcgAve);
			}

			for (int i = 0; i < ndcgAves.size(); ++i)
			{
				ofs << i << "," << ndcgAves[i];
				for (auto v : useElements)
				{
					ofs << "," << (int)v;
				}
				ofs << "\n";
			}

			node.value = ndcgAves.back();
			auto indexes = useElements;
			std::sort(indexes.begin(), indexes.end());
			records.emplace_back(indexes);
			//if (useElements.front() == 27)
			//	exit(0);
		}

		const auto maxValueNode = *std::max_element(nodes.begin(), nodes.end(), [](const Node& a, const Node& b) {return a.value < b.value; });
		const double parentValue = maxValueNode.parentValue;
		const double maxValue = maxValueNode.value;

		//if (maxValue < parentValue)
		//{
		//	continue;//次のnodesPoolのセットへ
		//}

		//最大値が複数あるかもしれないので全て取得
		std::vector<size_t> maxes;
		for (size_t i = 0; i < nodes.size(); ++i)
		{
			if (maxValue <= nodes[i].value)
			{
				maxes.push_back(i);
			}
		}

		for (size_t m : maxes)
		{
			decltype(nodes) bufNodes;
			const auto& useElements = nodes[m].indexes;
			for (char i = 0; i < numOfElements; ++i)
			{
				if (std::find(useElements.begin(), useElements.end(), i) != useElements.end())
				{
					continue;
				}
				std::vector<char> buf = useElements;
				buf.reserve(buf.size() + 1);
				buf.push_back(i);
				auto sortedBuf = buf;
				std::sort(sortedBuf.begin(), sortedBuf.end());
				if (std::find(records.begin(), records.end(), sortedBuf) == records.end())
				{
					std::sort(buf.begin(), buf.end());
					bufNodes.emplace_back(std::move(buf), maxValue);
				}
			}
			bufNodes.erase(std::unique(bufNodes.begin(), bufNodes.end(), [](const Node& na, const Node& nb) {return na.indexes == nb.indexes; }), bufNodes.end());
			if (!bufNodes.empty())
			{
				nodesPool.emplace(std::move(bufNodes));
			}
		}

		//FileIO::saveCSV("test.csv", CSV(1, map(ndcgAves, [](double d)->std::string {return std::to_string(d); })));
	}
	return 0;
}

#if 0
template<class T> using Vector = std::vector<T>;
template<class T> using Stack = std::stack<T>;
#else

template<class T> using Vector = Concurrency::concurrent_vector<T>;
template<class T> using Stack = Concurrency::concurrent_queue<T>;
#endif
namespace fs = std::experimental::filesystem;
int RankSVM::mainPython(int argc, char* argv[])
{
	std::cout << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10);
	constexpr bool isAdd = true;
	std::vector<std::vector<char>> records;
	std::queue<std::vector<Node>> nodesPool;
	{
		std::vector<Node> nodes;
		//どうせ最初は25と43
		//nodes.emplace_back(1, (char)43);
		//nodes.emplace_back(1, (char)35);

		/*const std::vector<char> useElements = { 7,13,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47 };
		{
			std::vector<Node> bufNodes;
			for (char i = 0; i < numOfElements; ++i)
			{
				if (std::find(useElements.begin(), useElements.end(), i) != useElements.end())
				{
					continue;
				}
				std::vector<char> buf = useElements;
				buf.reserve(buf.size() + 1);
				buf.push_back(i);
				std::vector<char> sortedBuf(buf.size());
				std::copy(buf.begin(), buf.end(), sortedBuf.begin());
				std::sort(sortedBuf.begin(), sortedBuf.end());
				if (std::find(records.begin(), records.end(), sortedBuf) == records.end())
				{
					std::sort(buf.begin(), buf.end());
					records.push_back(sortedBuf);
					bufNodes.push_back(Node(std::move(buf), 0.0));
				}
			}
			bufNodes.erase(std::unique(bufNodes.begin(), bufNodes.end(), [](const Node& na, const Node& nb) {return na.indexes == nb.indexes; }), bufNodes.end());
			if (!bufNodes.empty())
			{
				std::vector<Node> _nodes;
				_nodes.reserve(bufNodes.size());
				for (auto& n : bufNodes)
				{
					_nodes.push_back(std::move(n));
				}
				nodesPool.push(_nodes);
			}
		}
*/
		//最初からやる場合
		for (size_t i = 0; i < numOfElements; ++i)
		{
			nodes.emplace_back(1, (char)i);
			records.push_back(nodes.back().indexes);
		}
		nodesPool.push(nodes);
	}
	const std::string PythonSource((std::istreambuf_iterator<char>(std::ifstream("ProgrammingSkillPredictModel.py"))),
		std::istreambuf_iterator<char>());
	std::ofstream ofs("test_step_add.csv", std::ios::trunc);
	std::ofstream ofs2("pred.csv", std::ios::trunc);
	std::ofstream ofs3("depth.csv", std::ios::trunc);
	double parentValue = 0.0;
	std::atomic_int atomic_counter = 0;

	std::random_device seed_gen;
	std::mt19937 engine(seed_gen());

	//int roundCount = 0;//同率一位のときはシードを変えて繰り返して平均をとって一位が一つになるまでやる。
	while (!nodesPool.empty())
	{
		std::vector<Node> nodes = nodesPool.front();
		nodesPool.pop();
		Vector<std::string> resultStr;
		if (!nodes.empty())
		{
			std::cout << nodes.front().indexes.size() << "個の組み合わせ," << nodes.size() << "通り," << /*roundCount << "回目" <<*/ "開始\n";
		}
		//for (auto& ar : arr)
		//for (auto& node : nodes)
		Concurrency::parallel_for_each(nodes.begin(), nodes.end(),[&](Node& node)
		{
			const int counter = ++atomic_counter;

			constexpr int numOfTry = 9;
			std::array<double, numOfTry> scores = { 0 };
			const auto useElements = node.indexes;
			for (int roundCount = 0; roundCount < numOfTry; ++roundCount)
			{
				//Node node;
				//for (auto a : ar)
				//{
				//	node.indexes.push_back(a);
				//}
				//auto csv = FileIO::loadCSVtoString("combine.csv");
				//replaceCSV(csv);
				//const auto& useElements = node.indexes;
				//reformCSVForFeature(csv, useElements, isAdd);
				//sortCSVbyFirstCol(csv);
				//FileIO::saveCSV("convert_sorted.csv", csv, ' ');
				//std::random_shuffle(csv.begin(), csv.end());
				//const auto data = divide(std::move(csv), numOfFoldCrossValidation);
				//
				//Vector<double> ndcgAves;
				//Vector<std::string> params;
				//
				//Vector<double> dcgs(numOfFoldCrossValidation, -1);
				//Param default_param;
				//default_param.c = 0.000010f;
				//default_param.p = 1;
				//default_param.o = 2;
				//default_param.l = 1;
				//default_param.t = Param::Kernel::RBF;
				//default_param.g = 0.000100f;
				//#ifdef MULTI_THREAD
				//			concurrency::parallel_for(0, numOfFoldCrossValidation, [&](size_t i)
				//#else
				//			for (size_t i = 0; i < numOfFoldCrossValidation; ++i)
				//#endif
				//			{
				//				dcgs[i] = learn(default_param, i, data);
				//			}
				//#ifdef MULTI_THREAD
				//			);
				//#endif
				std::string listStr = "[";
				std::string currentThreadId;
				std::string sourceFilename;
				std::string resultFilename;
				{
					{
						std::ostringstream os;
						os << std::this_thread::get_id();
						currentThreadId = std::to_string(counter);
					}

					for (auto c : useElements)
					{
						listStr += std::to_string((int)c) + ",";
					}
					listStr.replace(listStr.size() - 1, 1, "]");

					auto pythonSource = PythonSource;
					{
						const auto pos = pythonSource.find("[]");
						pythonSource.replace(pos, 2, listStr);
					}
					if (0 < roundCount)
					{
						const std::string random_state = "random_state=0";
						const auto new_random_state = "random_state=" + std::to_string(roundCount);
						const auto pos = pythonSource.find(random_state);
						pythonSource.replace(pos, random_state.size(), new_random_state);
					}

					{
						const auto pos = pythonSource.find("result.txt");
						resultFilename = currentThreadId + "_result.txt";
						pythonSource.replace(pos, 10, resultFilename);
					}

					sourceFilename = currentThreadId + "_ProgrammingSkillPredictModel.py";
					std::ofstream(sourceFilename, std::ios::trunc) << pythonSource;
				}

				//std::string argv1 = R"(C:\Users\Ryosuke\AppData\Local\Programs\Python\Python36\python.exe )" + sourceFilename;
				//std::string argv1 = R"(C:\PROGRA~1\Python36\python.exe )" + sourceFilename;
				std::string argv1 = R"(C:\PROGRA~1\Anaconda3\python.exe )" + sourceFilename;

				const int ret1 = system(argv1.c_str());

				std::string scoreStr;
				{
					std::ifstream ifs(resultFilename);
					std::getline(ifs, scoreStr);
				}
				try
				{
					fs::remove(fs::path(sourceFilename));
					fs::remove(fs::path(resultFilename));
				}
				catch (fs::filesystem_error& e)
				{
					resultStr.push_back(e.what());
				}
				const double score = std::stod(scoreStr);
				scores[roundCount] = score;
			}
			std::sort(scores.begin(), scores.end());
			double score = scores[scores.size() / 2];
			node.value = score;
			//ここまで繰り返して中央値を取る。
			{
				std::ostringstream ss;
				ss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10);
				ss << counter << "," << score;
				for (auto v : useElements)
				{
					ss << "," << (int)v;
				}
				ss << "\n";
				//std::cout << "\n";
				//resultStr.push_back(ss.str());
				std::cout << ss.str();
				ofs << ss.str();
			}

			//if (useElements.front() == 27)
			//	exit(0);

		}
		);

		for (const auto& s : resultStr)
		{
			std::cout << s;
		}

		//if (0 < roundCount)
		//{
		//	for (auto& n : nodes)
		//	{
		//		n.value = (n.parentValue*roundCount + n.value) / (roundCount + 1);
		//	}
		//}
		const auto maxValueNode = *std::max_element(nodes.begin(), nodes.end(), [](const Node& a, const Node& b) {return a.value < b.value; });
		const double parentValue = maxValueNode.parentValue;
		const double maxValue = maxValueNode.value;

		//これを無効化すると改善しなくても最良のものを一つ追加して続ける。
		//if (maxValue < parentValue)
		//{
		//	continue;//次のnodesPoolのセットへ
		//}

		//最大値が複数あるかもしれないので全て取得
		std::vector<size_t> maxes;
		for (size_t i = 0; i < nodes.size(); ++i)
		{
			if (maxValue <= nodes[i].value)
			{
				maxes.push_back(i);
			}
		}

		//if (2 <= maxes.size())
		//{
		//	//シードを変更して今回のを記録して次の回へ。
		//	++roundCount;
		//	std::vector<Node> newNodes;
		//	for (auto i : maxes)
		//	{
		//		newNodes.push_back(nodes[i]);
		//		newNodes.back().parentValue = nodes[i].value;
		//	}
		//	newNodes.shrink_to_fit();
		//	nodesPool.push(newNodes);
		//}
		//else
		{
			//roundCount = 0;
			if(1<maxes.size())
			{
				std::shuffle(maxes.begin(), maxes.end(), engine);
				auto m = maxes.front();
				maxes.clear();
				maxes.push_back(m);
			}
			for (size_t m : maxes)
			{
				std::vector<Node> bufNodes;
				const auto& useElements = nodes[m].indexes;
				for (char i = 0; i < numOfElements; ++i)
				{
					if (std::find(useElements.begin(), useElements.end(), i) != useElements.end())
					{
						continue;
					}
					std::vector<char> buf = useElements;
					buf.reserve(buf.size() + 1);
					buf.push_back(i);
					std::vector<char> sortedBuf(buf.size());
					std::copy(buf.begin(), buf.end(), sortedBuf.begin());
					std::sort(sortedBuf.begin(), sortedBuf.end());
					if (std::find(records.begin(), records.end(), sortedBuf) == records.end())
					{
						std::sort(buf.begin(), buf.end());
						records.push_back(sortedBuf);
						bufNodes.push_back(Node(std::move(buf), maxValue));
					}
				}
				bufNodes.erase(std::unique(bufNodes.begin(), bufNodes.end(), [](const Node& na, const Node& nb) {return na.indexes == nb.indexes; }), bufNodes.end());
				if (!bufNodes.empty())
				{
					std::vector<Node> _nodes;
					_nodes.reserve(bufNodes.size());
					for (auto& n : bufNodes)
					{
						_nodes.push_back(std::move(n));
					}
					nodesPool.push(_nodes);
				}
			}
		}
		//FileIO::saveCSV("test.csv", CSV(1, map(ndcgAves, [](double d)->std::string {return std::to_string(d); })));
	}
	return 0;
}
