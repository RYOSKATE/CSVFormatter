#include "FileIO.h"

//��؂蕶��esc_sep�ŋ�؂�ꂽ�e�L�X�g�𕪊�����B
static std::vector<std::string> TextToToken(const std::string& text, const char esc_sep = ',')
{
	// CSV�p��؂蕶�����`����
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

//ifstream��ǂݍ��ށB
static std::vector<std::vector<std::string>> ReadFromStream(std::ifstream& in, const char esc_sep = ',')
{
	//VRD�t�@�C����ǂݍ���string�̓񎟌�vector�Ɋi�[���Ԃ��B
	std::vector<std::vector<std::string>> allTexts;
	allTexts.reserve(USHRT_MAX);//�Ƃ肠�����̃��U�[�u

	std::string rowText;//1�s�̃e�L�X�g
	while (in && std::getline(in, rowText))
	{
		// 1�s�����������ꂽtoken��m_allTexts��push
		const std::vector<std::string> tokens = TextToToken(rowText, esc_sep);

		allTexts.push_back(tokens);
	}
	allTexts.shrink_to_fit();
	return allTexts;
}

//path��csv�t�@�C����string�̓񎟌�vector�ɓǂݍ��ݕԂ��B
std::vector<std::vector<std::string>> FileIO::loadCSVtoString(const std::string& path, const char esc_sep/* =','*/)
{
	std::ifstream fin(path.c_str());
	if (!fin) {
		std::cout << path << "���I�[�v���ł��܂���ł����B\n";
		return std::vector<std::vector<std::string>>();
	}
	auto allTexts = ReadFromStream(fin, esc_sep);

	//std::cout << path << "�̓��e��ǂݍ��݂܂����B\n";

	return allTexts;
}

//data���w�肵����؂蕶����path�ɏ����o���B
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