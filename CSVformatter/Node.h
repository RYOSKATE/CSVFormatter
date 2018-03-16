#pragma once
struct Node
{
	std::vector<char> indexes;
	double parentValue = 0.0f;
	double value = 0.0f;
	Node() = default;
	~Node() = default;

	Node &operator=(const Node &o) = default;
	Node(const Node &o) = default;
	Node(int i, char v)
		: indexes(i, v)
	{}

	Node(Node&& n)
		:indexes(std::move(n.indexes))
		, parentValue(n.parentValue)
	{}

	Node(decltype(indexes) && vec, double v)
		:indexes(std::move(vec)), parentValue(v)
	{}
};
