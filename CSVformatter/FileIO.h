#pragma once

namespace FileIO
{
	//pathのcsvファイルをstringの二次元vectorに読み込み返す。
	std::vector<std::vector<std::string>> loadCSVtoString(const std::string& path, const char esc_sep = ',');

	//dataを指定した区切り文字でpathに書き出す。
	bool saveCSV(const std::string& path, const std::vector<std::vector<std::string>>& data, const char esc_sep = ',');

	//ファイルサイズを返す。
	size_t getFileSize(const char* filepath);

	//Type型のbinaryファイルを読み込む
	template <typename Type>
	std::vector<Type> loadRaw(const std::string& rawFilePath, size_t size = 0)
	{
		std::ifstream fin(rawFilePath, std::ios::in | std::ios::binary);
		//  ファイルを開く
		//  ios::in は読み込み専用  ios::binary はバイナリ形式

		if (!fin)
		{
			std::cout << rawFilePath << "が開けません\n";
			return std::vector<Type>();
		}

		//サイズを確認
		if (size == 0)
		{
			size = getFileSize(rawFilePath.c_str());
		}

		std::vector<Type> raw(size);
		fin.read((char*)raw.data(), sizeof(Type)*size);
		std::cout << rawFilePath << "の内容を読み込みました。\n";
		return raw;
	}

	//Type型のbinaryファイルを書き出す
	template <typename Type>
	bool writeRaw(const std::string& outFilePath, const std::vector<Type>& volume)
	{
		std::ofstream fout;
		fout.open(outFilePath, std::ios::out | std::ios::binary | std::ios::trunc);
		//  ファイルを開く
		//  ios::out は書き込み専用(省略可)
		//  ios::binary はバイナリ形式で出力(省略するとアスキー形式で出力)
		//  ios::truncはファイルを新規作成(省略可)
		//  ios::addにすると追記になる

		if (!fout) {
			std::cout << outFilePath << "を作成できません\n";
			return false;
		}

		fout.write((char*)volume.data(), sizeof(Type)*volume.size());

		fout.close();  //ファイルを閉じる
		std::cout << outFilePath << "を出力しました。\n";
		return true;
	}
};