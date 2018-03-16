#include "CSV.h"
#include "FileIO.h"
template<class Type>
void Replace(CSV& csv, int colIndex, const std::unordered_map<std::string, Type>& map)
{
	//std::string cell;
	try
	{
		for (int y = 1; y < csv.size(); ++y)
		{
			auto& cell = csv[y][colIndex];
			cell = std::to_string(map.at(cell));
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what()/* << ":" << cell*/;
	}
}

// コンテスト順位
void contestReplace(CSV& csv)
{
	auto contestMapText = FileIO::loadCSVtoString("contestscore.csv");
	std::unordered_map<std::string, std::string> contestMap;//(出席簿番号,順位);
	for (const auto & pair : contestMapText)
	{
		contestMap[pair.front()] = pair.back();
	}
	//出席簿番号を見て、コンテストの順位の列を作る。
	//出席簿番号の列は削除して、qid:1の列を作る。
	csv[0][0] = "コンテスト順位";
	csv[0][1] = "qie";//出席簿番号は削除
	for (int y = 1; y < csv.size(); ++y)
	{
		const auto& index = csv[y][0];
		const auto& rank = contestMap[index];
		csv[y][0] = rank;
		csv[y][1] = "qid:1";
	}
}

//C列 これまでの授業内容には満足していますか。
void manzokuReplace(CSV& csv)
{
	std::unordered_map<std::string, double> map =
	{
		{ "満足している",1.0 },
		{ "やや満足している",0.5 },
		{ "どちらともいえない",0 },
		{ "やや不満がある",-0.5 },
		{ "不満がある",-1.0 },
	};

	Replace(csv, 2, map);

}

//D列 1週間のうち、授業時間以外でこの授業の学習(自習、演習、宿題など)に費やしている時間はどの程度ですか。
void jikanReplace(CSV& csv)
{
	std::unordered_map<std::string, double> map =
	{
		{ "4時間以上",1.0 },
		{ "4時間未満",0.8 },
		{ "2時間未満",0.4 },
		{ "1時間未満",0.2 },
		{ "30分未満",0.1 },
		{ "ほとんどしていない", 0.0 }
	};
	Replace(csv, 3, map);

}

//E~L列 態度アンケート
void taidoReplace(CSV& csv)
{
	std::unordered_map<std::string, double> map =
	{
		{ "とてもそう思う",1.0 },
		{ "そう思う", 2.0 / 3.0 },
		{ "ややそう思う", 1.0 / 3.0 },
		{ "どちらともいえない",0.0 },
		{ "あまりそう思わない",-1.0 / 3.0 },
		{ "そう思わない",-2.0 / 3.0 },
		{ "まったくそう思わない",-1.0 },
	};
	for (int i = 4; i <= 11; ++i)
	{
		Replace(csv, i, map);
	}

}

//希望クラスアンケート
void kibouReplace(CSV& csv)
{
	std::unordered_map<std::string, double> map =
	{
		{ "中級",1.0 },
		{ "どちらでも良い", 0.0 },
		{ "初級", -1.0 },
	};
	Replace(csv, static_cast<int>(csv[0].size() - 1), map);
}

//心理アンケート
void sinriReplace(CSV& csv)
{
	for (int x = 12; x < csv[0].size() - 1; ++x)
	{
		double max = 0.0;
		//その列の最大値を見つける
		for (int y = 1; y < csv.size(); ++y)
		{
			double value = std::stod(csv[y][x]);
			if (max < value)
			{
				max = value;
			}
		}
		for (int y = 1; y < csv.size(); ++y)
		{
			double value = std::stod(csv[y][x]);
			csv[y][x] = std::to_string(value / max);
		}
	}
}

