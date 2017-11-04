#ifndef __TFTJ_QUERY__
#define __TFTJ_QUERY__

#include <unordered_map>
#include <string>
#include <algorithm>


struct QueryNode;
bool operator==(const std::pair<std::vector<int>, QueryNode*>& v1, const std::vector<int>& indices)
{
	return v1.first == indices;
}

struct QueryNode
{
	std::string node;
	bool isQueried;
	int queryIndex;

	std::unordered_map<std::string, QueryNode*> tree;
	std::unordered_map<int, QueryNode*> arrays;

	QueryNode(const std::string& name)
		: node(name)
	{
		isQueried = false;
	}

	void addQueried(int index)
	{
		isQueried = true;
		queryIndex = index;
	}

	QueryNode* getArrayQueried(const std::vector<int>& indices)
	{
		if (!tree.empty())
		{
			return nullptr; //todo: error code
		}

		QueryNode* root = this;
		for (int i : indices)
		{
			if (root->arrays.find(i) == root->arrays.end())
			{
				std::string name;
				for (auto& i : indices)
				{
					name += "[" + std::to_string(i) + "]";
				}
				root->arrays[i] = new QueryNode(node.empty() ? name : node + "." + name);
			}
			root = root->arrays[i];
		}
		return root;
	}

	QueryNode* getJsonQueried(const std::string& str)
	{
		if (str == "")
		{
			return this;
		}

		if (!arrays.empty())
		{
			return nullptr;
		}

		if (tree[str] == nullptr)
		{
			tree[str] = new QueryNode(node.empty() ? str : node + "." + str);
		}
		return tree[str];
	}

	~QueryNode()
	{
		for (auto& l : tree)
		{
			delete l.second;
		}
	}

	QueryNode()
	{
		isQueried = false;
	}

	QueryNode(QueryNode&& ref):
		isQueried(ref.isQueried),
		node(ref.node),
		tree(ref.tree),
		arrays(ref.arrays)
	{
		ref.tree.clear();
		ref.arrays.clear();
	}

	QueryNode(const QueryNode&) = delete;
	QueryNode& operator=(const QueryNode&) = delete;
	QueryNode& operator=(const QueryNode&&) = delete;
};


QueryNode parse_query(const std::vector<std::string>& query, int* depth, int* array_depth);

struct Query
{
	typedef QueryNode map_t;

	map_t _tree;
	int _depth;
	int _array_depth;

	Query(const std::vector<std::string>& query):
		_tree(parse_query(query, &_depth, &_array_depth))
	{	}


private:
	Query(const Query&) = delete;
	Query(const Query&&) = delete;
	Query& operator=(const Query&) = delete;
	Query& operator=(const Query&&) = delete;
};

bool do_query(const Query::map_t& tree, const std::string& field, const Query::map_t** inner_tree)
{
	auto it = tree.tree.find(field);
	if(it == tree.tree.end())
	{
		return false;
	}

	*inner_tree = it->second;
	return true;
};

void parseQuery2(const std::string& str, QueryNode& root, int& depth, int& array_depth, int index);

QueryNode parse_query(const std::vector<std::string>& query, int* depth, int* array_depth)
{
	QueryNode root;

	int max_depth = 0;
	int max_array_depth = 0;
	int index = 0;
	for(auto q : query)
	{
		int depth = 0;
		int array_depth = 0;
		parseQuery2(q, root, depth, array_depth, index);
		if (depth > max_depth)
			max_depth = depth;
		if (array_depth > max_array_depth)
			max_array_depth = array_depth;
		++index;
	}
	*depth = max_depth; 
	*array_depth = max_array_depth;
	return root;
}

void parseQuery2(const std::string& str, QueryNode& root, int& depth, int& array_depth, int index)
{
	if (str == "")
		return;

	++depth;

	size_t dot_pos = str.find('.');
	std::string start = str.substr(0, dot_pos);

	size_t l_bracket_pos = start.find('[');
	std::string start_no_index = start.substr(0, l_bracket_pos);

	QueryNode* node = root.getJsonQueried(start_no_index);

	std::vector<int> indices;

	while (l_bracket_pos != std::string::npos)
	{
		std::string next = start.substr(l_bracket_pos + 1);
		size_t r_bracket_pos = next.find(']');
		std::string index = next.substr(0, r_bracket_pos);
		indices.push_back(std::stoi(index));
		start = next.substr(r_bracket_pos + 1);
		l_bracket_pos = start.find('[');
		++array_depth;
	}

	QueryNode* array_node = indices.empty() ? node : node->getArrayQueried(indices);

	if (dot_pos == std::string::npos)
	{
		array_node->addQueried(index);
		return;
	}

	std::string next = str.substr(dot_pos + 1);

	parseQuery2(next, *array_node, depth, array_depth, index);
}

#endif