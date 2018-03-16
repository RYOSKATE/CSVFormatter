#pragma once

template <typename Tlist, typename Tfunc>
void __combination(
	Tlist const& list, typename Tlist::size_type const n, Tfunc const& func,
	Tlist& tmp, typename Tlist::size_type const i, typename Tlist::size_type const j)
{
	if (i == n)
	{
		func(tmp);
		return;
	}

	for (typename Tlist::size_type k = j; k != list.size() + (1 + i - n); ++k)
	{
		tmp[i] = list[k];
		__combination(list, n, func, tmp, i + 1, k + 1);
	}
}

template <typename Tlist, typename Tfunc>
void combination(
	Tlist const& list, typename Tlist::size_type const n, Tfunc const& func)
{
	Tlist tmp(n);
	__combination(list, n, func, tmp, 0, 0);
}

template <typename Type, typename Tfunc>
void combination(size_t n, size_t r, Tfunc const& func)
{
	std::vector<Type> buf(n);
	std::iota(buf.begin(), buf.end(), 0);
	combination(buf, r, func);
}

template <typename Type>
void generateConbiFile(Type n, Type r)
{
	//constexpr size_t n = 48;
	//constexpr size_t r = 4;
	std::vector<Type> list(n);
	std::iota(list.begin(), list.end(), 0);

	std::ofstream ofs("combi.csv", std::ios::trunc);
	ofs << "{";
	size_t count = 0;
	combination(list, r, [&ofs, &count](const std::vector<size_t>& a)
	{
		ofs << "{" << (*a.begin());
		for (auto i(a.begin() + 1); i != a.end(); ++i)
		{
			ofs << "," << (*i);
		}
		ofs << "},";
		std::cout << ++count << ",";
	});
	ofs << "};";
}