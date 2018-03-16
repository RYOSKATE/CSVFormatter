#pragma once

namespace FileIO
{
	//path��csv�t�@�C����string�̓񎟌�vector�ɓǂݍ��ݕԂ��B
	std::vector<std::vector<std::string>> loadCSVtoString(const std::string& path, const char esc_sep = ',');

	//data���w�肵����؂蕶����path�ɏ����o���B
	bool saveCSV(const std::string& path, const std::vector<std::vector<std::string>>& data, const char esc_sep = ',');

	//�t�@�C���T�C�Y��Ԃ��B
	size_t getFileSize(const char* filepath);

	//Type�^��binary�t�@�C����ǂݍ���
	template <typename Type>
	std::vector<Type> loadRaw(const std::string& rawFilePath, size_t size = 0)
	{
		std::ifstream fin(rawFilePath, std::ios::in | std::ios::binary);
		//  �t�@�C�����J��
		//  ios::in �͓ǂݍ��ݐ�p  ios::binary �̓o�C�i���`��

		if (!fin)
		{
			std::cout << rawFilePath << "���J���܂���\n";
			return std::vector<Type>();
		}

		//�T�C�Y���m�F
		if (size == 0)
		{
			size = getFileSize(rawFilePath.c_str());
		}

		std::vector<Type> raw(size);
		fin.read((char*)raw.data(), sizeof(Type)*size);
		std::cout << rawFilePath << "�̓��e��ǂݍ��݂܂����B\n";
		return raw;
	}

	//Type�^��binary�t�@�C���������o��
	template <typename Type>
	bool writeRaw(const std::string& outFilePath, const std::vector<Type>& volume)
	{
		std::ofstream fout;
		fout.open(outFilePath, std::ios::out | std::ios::binary | std::ios::trunc);
		//  �t�@�C�����J��
		//  ios::out �͏������ݐ�p(�ȗ���)
		//  ios::binary �̓o�C�i���`���ŏo��(�ȗ�����ƃA�X�L�[�`���ŏo��)
		//  ios::trunc�̓t�@�C����V�K�쐬(�ȗ���)
		//  ios::add�ɂ���ƒǋL�ɂȂ�

		if (!fout) {
			std::cout << outFilePath << "���쐬�ł��܂���\n";
			return false;
		}

		fout.write((char*)volume.data(), sizeof(Type)*volume.size());

		fout.close();  //�t�@�C�������
		std::cout << outFilePath << "���o�͂��܂����B\n";
		return true;
	}
};