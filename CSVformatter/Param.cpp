#include "Param.h"

namespace fs = std::experimental::filesystem;
void Param::setFileName(size_t i)
{
	const std::string currentPath = fs::current_path().string();
	std::ostringstream os;
	os << std::this_thread::get_id();
	std::string currentThreadId = os.str();
	workDirPath = currentPath + "\\" + currentThreadId;
	const auto path = workDirPath;
	if (!fs::exists(path))
	{
		fs::create_directory(path);
	}
	trainFileName = currentThreadId + "\\" + (std::to_string(i) + "train.dat");
	modelFileName = currentThreadId + "\\" + (std::to_string(i) + "model.dat");
	testFileName = currentThreadId + "\\" + (std::to_string(i) + "test.dat");
	predFileName = currentThreadId + "\\" + (std::to_string(i) + "predictions.dat");
	tmpFileName = currentThreadId + "\\" + (std::to_string(i) + "tmp");
}

std::vector<const char*> Param::toString()
{
	buf = toStringWithoutFilename();
	
	buf.emplace_back(trainFileName);
	buf.emplace_back(modelFileName);
	//for (const auto b : buf)
	//{
	//	std::cout << b << " ";
	//}
	std::cout << "\n";
	const auto ret = map(buf, [](const std::string& x) {return x.c_str(); });
	return ret;
}

std::vector<std::string> Param::toStringWithoutFilename()
{
	buf.clear();
	buf.reserve(14);

	buf.emplace_back("-c"); buf.emplace_back(std::to_string(c));
	buf.emplace_back("-p"); buf.emplace_back(std::to_string(p));
	buf.emplace_back("-o"); buf.emplace_back(std::to_string(o));
	buf.emplace_back("-l"); buf.emplace_back(std::to_string(l));

	buf.emplace_back("-t"); buf.emplace_back(std::to_string(t));
	if (t == RBF)
	{
		buf.emplace_back("-g"); buf.emplace_back(std::to_string(g));
	}
	return buf;
}
