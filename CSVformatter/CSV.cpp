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

// �R���e�X�g����
void contestReplace(CSV& csv)
{
	auto contestMapText = FileIO::loadCSVtoString("contestscore.csv");
	std::unordered_map<std::string, std::string> contestMap;//(�o�ȕ�ԍ�,����);
	for (const auto & pair : contestMapText)
	{
		contestMap[pair.front()] = pair.back();
	}
	//�o�ȕ�ԍ������āA�R���e�X�g�̏��ʂ̗�����B
	//�o�ȕ�ԍ��̗�͍폜���āAqid:1�̗�����B
	csv[0][0] = "�R���e�X�g����";
	csv[0][1] = "qie";//�o�ȕ�ԍ��͍폜
	for (int y = 1; y < csv.size(); ++y)
	{
		const auto& index = csv[y][0];
		const auto& rank = contestMap[index];
		csv[y][0] = rank;
		csv[y][1] = "qid:1";
	}
}

//C�� ����܂ł̎��Ɠ��e�ɂ͖������Ă��܂����B
void manzokuReplace(CSV& csv)
{
	std::unordered_map<std::string, double> map =
	{
		{ "�������Ă���",1.0 },
		{ "��▞�����Ă���",0.5 },
		{ "�ǂ���Ƃ������Ȃ�",0 },
		{ "���s��������",-0.5 },
		{ "�s��������",-1.0 },
	};

	Replace(csv, 2, map);

}

//D�� 1�T�Ԃ̂����A���Ǝ��ԈȊO�ł��̎��Ƃ̊w�K(���K�A���K�A�h��Ȃ�)�ɔ�₵�Ă��鎞�Ԃ͂ǂ̒��x�ł����B
void jikanReplace(CSV& csv)
{
	std::unordered_map<std::string, double> map =
	{
		{ "4���Ԉȏ�",1.0 },
		{ "4���Ԗ���",0.8 },
		{ "2���Ԗ���",0.4 },
		{ "1���Ԗ���",0.2 },
		{ "30������",0.1 },
		{ "�قƂ�ǂ��Ă��Ȃ�", 0.0 }
	};
	Replace(csv, 3, map);

}

//E~L�� �ԓx�A���P�[�g
void taidoReplace(CSV& csv)
{
	std::unordered_map<std::string, double> map =
	{
		{ "�ƂĂ������v��",1.0 },
		{ "�����v��", 2.0 / 3.0 },
		{ "��₻���v��", 1.0 / 3.0 },
		{ "�ǂ���Ƃ������Ȃ�",0.0 },
		{ "���܂肻���v��Ȃ�",-1.0 / 3.0 },
		{ "�����v��Ȃ�",-2.0 / 3.0 },
		{ "�܂����������v��Ȃ�",-1.0 },
	};
	for (int i = 4; i <= 11; ++i)
	{
		Replace(csv, i, map);
	}

}

//��]�N���X�A���P�[�g
void kibouReplace(CSV& csv)
{
	std::unordered_map<std::string, double> map =
	{
		{ "����",1.0 },
		{ "�ǂ���ł��ǂ�", 0.0 },
		{ "����", -1.0 },
	};
	Replace(csv, static_cast<int>(csv[0].size() - 1), map);
}

//�S���A���P�[�g
void sinriReplace(CSV& csv)
{
	for (int x = 12; x < csv[0].size() - 1; ++x)
	{
		double max = 0.0;
		//���̗�̍ő�l��������
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

