#pragma once

template <typename Container, typename Pred> inline
void Erase_if(Container& c, Pred pred)
{
	c.erase(std::remove_if(c.begin(), c.end(), pred), c.end());
}

template <class T, class F>
decltype(auto) map(const std::vector<T>& a, const F fn) {
	std::vector<decltype(fn(a[0]))> result = {};
	std::transform(a.cbegin(), a.cend(), std::back_inserter(result), fn);
	return result;
}

template<class T>
std::vector<std::vector<T>> divide(std::vector<T>&& v, int n)
{
	std::vector<std::vector<T>> result;
	const size_t size = v.size();
	const size_t partial_length = (size + n - 1) / n;
	for (auto i = v.begin(); i < v.end(); i += partial_length)
	{
		if (i < v.end() - partial_length)
		{
			result.emplace_back(std::make_move_iterator(i), std::make_move_iterator(i + partial_length));
		}
		else
		{
			result.emplace_back(std::make_move_iterator(i), std::make_move_iterator(v.end()));
			break;
		}
	}
	v.clear();
	return result;
}

template<class T>
std::vector<std::vector<T>> divided(const std::vector<T>& v, int n)
{
	return divide(std::vector<std::vector<T>>(v), n);
}
