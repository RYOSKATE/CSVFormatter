#include "RankSVM.h"

template<class Func>
void execWithTimer(Func f)
{
	
	const auto start = std::chrono::system_clock::now();
	f();
	const auto end = std::chrono::system_clock::now();
	const auto elapse = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
	int h = elapse / 3600;
	int m = (elapse / 60) % 60;
	int s = elapse % 60;
	std::cout << h << "ŽžŠÔ" << m << "•ª" << s << "•b("<< elapse << "•b)\n";
}

int main(int argc, char* argv[])
{
	execWithTimer([=] 
	{
		auto filepath = (1 < argc) ? argv[1] : R"(C:\Users\ry14z\Documents\VISUAL~1\Projects\CSVformatter\CSVformatter\combine.csv)";
		RankSVM rankSVM(filepath);
		//rankSVM.mainBruteforce(RankSVM::numOfElements, 4);
		rankSVM.mainPython(argc, argv);
	});
	return 0;
}
