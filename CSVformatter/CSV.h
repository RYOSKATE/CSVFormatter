#pragma once
using CSV = std::vector<std::vector<std::string>>;

// コンテスト順位
void contestReplace(CSV& csv);

//C列 これまでの授業内容には満足していますか。
void manzokuReplace(CSV& csv);

//D列 1週間のうち、授業時間以外でこの授業の学習(自習、演習、宿題など)に費やしている時間はどの程度ですか。
void jikanReplace(CSV& csv);

//E~L列 態度アンケート
void taidoReplace(CSV& csv);

// 希望クラスアンケート
void kibouReplace(CSV& csv);

//心理アンケート
void sinriReplace(CSV& csv);