#pragma once
#include "CSV.h"

class Param;
class RankSVM
{
public:
	RankSVM() = default;
	RankSVM(const std::string& path) :currentDir(path) {}
	~RankSVM() = default;
	int mainParam(int argc, char* argv[]);
	int mainFeature(int argc, char* argv[]);
	int mainPython(int argc, char* argv[]);
	int mainBruteforce(size_t n, size_t r);

	static constexpr int numOfFoldCrossValidation = 5;
	static constexpr int maxContestRank = 74;
	static constexpr size_t numOfElements = 48;
private:
	void replaceCSV(CSV& csv);
	void reformCSV(CSV& csv);
	void reformCSVForFeature(CSV& csv, const std::vector<char>& useElements, bool isAdd);
	void sortCSVbyFirstCol(CSV& csv);

	double learn(Param param, size_t i, const std::vector<CSV>& data);
	std::vector<CSV> getDividedCSV();

	std::string currentDir;
};
