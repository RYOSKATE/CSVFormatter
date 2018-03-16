#pragma once
class Param
{
public:
	Param() = default;
	Param(size_t i)
	{
		setFileName(i);
	}

	void setFileName(size_t i);

	std::string workDirPath;
	std::string svnRankLearnFileName = "svm_rank_learn";
	std::string trainFileName = "train.dat";
	std::string modelFileName = "model.dat";

	std::string testFileName = "test.dat";
	std::string predFileName = "predictions.txt";
	std::string tmpFileName = "tmp";
	double c = 0.01f;
	int p = 1;//norm 1: L1-norm, 2: squared slaks
	int o = 2;//scaling 1:SLAC, 2:MARGIN
	int l = 1;//Loss function 0:zero/one, 1?

	enum Kernel
	{
		LINEAR = 0,
		RBF = 2,
	};

	int t = LINEAR;
	double g;// for rbf

	std::vector<std::string> buf;

	std::vector<const char*> toString();
	std::vector<std::string> toStringWithoutFilename();
};

