#pragma once
using CSV = std::vector<std::vector<std::string>>;

// �R���e�X�g����
void contestReplace(CSV& csv);

//C�� ����܂ł̎��Ɠ��e�ɂ͖������Ă��܂����B
void manzokuReplace(CSV& csv);

//D�� 1�T�Ԃ̂����A���Ǝ��ԈȊO�ł��̎��Ƃ̊w�K(���K�A���K�A�h��Ȃ�)�ɔ�₵�Ă��鎞�Ԃ͂ǂ̒��x�ł����B
void jikanReplace(CSV& csv);

//E~L�� �ԓx�A���P�[�g
void taidoReplace(CSV& csv);

// ��]�N���X�A���P�[�g
void kibouReplace(CSV& csv);

//�S���A���P�[�g
void sinriReplace(CSV& csv);