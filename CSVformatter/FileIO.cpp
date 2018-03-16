#include "FileIO.h"

//区切り文字esc_sepで区切られたテキストを分割する。
static std::vector<std::string> TextToToken(const std::string& text, const char esc_sep = ',')
{
	// CSV用区切り文字を定義する
	std::vector<std::string> tokens;
	tokens.reserve(6);

	std::istringstream stream(text);
	std::string tmp;
	while (getline(stream, tmp, esc_sep))
	{
		tokens.push_back(move(tmp));
	}
	tokens.shrink_to_fit();
	return tokens;
}

//ifstreamを読み込む。
static std::vector<std::vector<std::string>> ReadFromStream(std::ifstream& in, const char esc_sep = ',')
{
	//VRDファイルを読み込みstringの二次元vectorに格納し返す。
	std::vector<std::vector<std::string>> allTexts;
	allTexts.reserve(USHRT_MAX);//とりあえずのリザーブ

	std::string rowText;//1行のテキスト
	while (in && std::getline(in, rowText))
	{
		// 1行分が分割されたtokenをm_allTextsにpush
		const std::vector<std::string> tokens = TextToToken(rowText, esc_sep);

		allTexts.push_back(tokens);
	}
	allTexts.shrink_to_fit();
	return allTexts;
}

//pathのcsvファイルをstringの二次元vectorに読み込み返す。
std::vector<std::vector<std::string>> FileIO::loadCSVtoString(const std::string& path, const char esc_sep/* =','*/)
{
	std::ifstream fin(path.c_str());
	if (!fin) {
		std::cout << path << "をオープンできませんでした。\n";
		return std::vector<std::vector<std::string>>();
	}
	auto allTexts = ReadFromStream(fin, esc_sep);

	//std::cout << path << "の内容を読み込みました。\n";

	return allTexts;
}

//dataを指定した区切り文字でpathに書き出す。
bool FileIO::saveCSV(const std::string& path, const std::vector<std::vector<std::string>>& data, const char esc_sep/* = ','*/)
{
	std::ofstream fout(path.c_str(), std::ios::out | std::ios::trunc);
	
	for (const auto& line : data)
	{
		bool first = true;
		for (const auto& cell : line)
		{
			if (!std::exchange(first, false))
			{
				fout << esc_sep;
			}
			fout << cell;
		}
		fout << "\n";
	}
	return true;
}

size_t FileIO::getFileSize(const char* filepath)
{
	HANDLE h = CreateFile(filepath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (h == INVALID_HANDLE_VALUE)
		return 0;
	size_t filesize = GetFileSize(h, 0);
	CloseHandle(h);
	return filesize;
}