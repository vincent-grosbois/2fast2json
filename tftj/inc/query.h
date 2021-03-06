#ifndef __TFTJ_QUERY__
#define __TFTJ_QUERY__

#include <unordered_map>
#include <string>
#include <algorithm>

namespace tftj {


	struct SpeculativeParsingNode
	{
		const std::string name;
		int index;
		int count;
		std::vector<SpeculativeParsingNode*> children;

		SpeculativeParsingNode():
			name("root"), //root node
			index(-1),
			count(0)
		{ }

		SpeculativeParsingNode(const std::string& name, int index) :
			name(name),
			index(index),
			count(1)
		{ }

		~SpeculativeParsingNode()
		{
			for (auto& p : children)
			{
				delete p;
			}
		}

		//sort by descending counts and keep at most max_children
		void Clean(int max_children)
		{
			struct by_reverse_count {
				bool operator()(SpeculativeParsingNode* const &a, SpeculativeParsingNode* const &b) {
					return a->count > b->count;
				}
			};

			std::sort(children.begin(), children.end(), by_reverse_count());

			if (max_children < children.size())
			{
				for (int i = max_children; i < children.size(); ++i)
				{
					delete children[i];
				}
				children.resize(max_children);
			}
		}

		//TODO refactor without recursion
		void insert(const std::pair<std::string, int>* list, int size)
		{
			if (size == 0)
			{
				return;
			}

			auto& e = list[0];

			SpeculativeParsingNode* child = nullptr;
			for (auto& i : children)
			{
				if (i->name == e.first && i->index == e.second)
				{
					++(i->count);
					child = i;
					break;
				}
			}

			if (child == nullptr)
			{
				child = new SpeculativeParsingNode(e.first, e.second);
				children.push_back(child);
			}


			child->insert(list + 1, size - 1);
		}
	};

	struct QueryException
	{
		QueryException(const std::string& str) : str(str) { }
		std::string str;
	};

	struct QueryNode
	{
		//full name of this node (starting from root)
		//used for debug only
		const std::string name;

		//did we ask for this specific node? (otherwise it means we asked for children/siblings)
		bool is_queried;

		//index of this specific field in the initial query
		int query_index;

		//children nodes, ie if this is 'node1' we go to 'node1.a', 'node1.b' etc
		std::unordered_map<std::string, QueryNode*> children_nodes;

		//sibling nodes, ie if this is 'node1' we go to 'node1[1]', 'node1[4]' etc
		std::unordered_map<int, QueryNode*> sibling_nodes;

		SpeculativeParsingNode speculative_tree;


		QueryNode(const std::string& node_name) :
			name(node_name),
			is_queried(false),
			query_index(-1)
		{ }

		//move ctor: move all containers to new instance
		QueryNode(QueryNode&& ref) :
			name(ref.name),
			is_queried(ref.is_queried),
			query_index(ref.query_index),
			children_nodes(std::move(ref.children_nodes)),
			sibling_nodes(std::move(ref.sibling_nodes))
		{
			ref.children_nodes.clear();
			ref.sibling_nodes.clear();
		}

		void addQueried(int index)
		{
			if (is_queried)
			{
				throw QueryException("Node " + name + " cannot be queried several times");
			}

			is_queried = true;
			query_index = index;
		}

		//get, or create and get, the sibling of this node at a given multi-index
		QueryNode* getSibling(const std::vector<int>& multi_index)
		{
			if (!children_nodes.empty())
			{
				throw QueryException("Trying to add a sub-array for node " + name + " which already contains children");
			}

			QueryNode* root = this;
			std::string index_suffix = "";
			for (int i : multi_index)
			{
				index_suffix += "[" + std::to_string(i) + "]";
				if (root->sibling_nodes.find(i) == root->sibling_nodes.end())
				{
					root->sibling_nodes[i] = new QueryNode(name + index_suffix);
				}
				root = root->sibling_nodes[i];
			}
			return root;
		}

		//get, or create and get, the children of this node at a given sub-level
		QueryNode* getChildQueried(const std::string& childName)
		{
			if (childName == "")
			{
				return this;
			}

			if (!sibling_nodes.empty())
			{
				throw QueryException("Trying to add children for node " + name + " which already contains a sub-array");
			}

			if (children_nodes[childName] == nullptr)
			{
				children_nodes[childName] = new QueryNode(name.empty() ? childName : name + "." + childName);
			}
			return children_nodes[childName];
		}

		void CleanSpeculativeTree()
		{
			speculative_tree.Clean(5);

			for (auto& n : children_nodes)
			{
				n.second->CleanSpeculativeTree();
			}

			for (auto& n : sibling_nodes)
			{
				n.second->CleanSpeculativeTree();
			}
		}

		~QueryNode()
		{
			for (auto& n : children_nodes)
			{
				delete n.second;
			}

			for (auto& n : sibling_nodes)
			{
				delete n.second;
			}
		}

		QueryNode(const QueryNode&) = delete;
		QueryNode& operator=(const QueryNode&) = delete;
		QueryNode& operator=(const QueryNode&&) = delete;
	};


	QueryNode parse_query(const std::vector<std::string>& query, int& children_depth, int& siblings_depth);

	struct Query
	{
		QueryNode tree;
		int children_depth;
		int siblings_depth;

		Query(const std::vector<std::string>& query) :
			tree(parse_query(query, children_depth, siblings_depth))
		{	}


		Query(const Query&) = delete;
		Query(const Query&&) = delete;
		Query& operator=(const Query&) = delete;
		Query& operator=(const Query&&) = delete;
	};

	void parse_query_internal(const std::string& str, QueryNode& root, int& children_depth, int& siblings_depth, int index);

	QueryNode parse_query(const std::vector<std::string>& query, int& max_children_depth, int& max_siblings_depth)
	{
		QueryNode root("");

		max_children_depth = 0;
		max_siblings_depth = 0;

		int index = 0;
		for (auto& q : query)
		{
			int children_depth = 0;
			int siblings_depth = 0;
			parse_query_internal(q, root, children_depth, siblings_depth, index);

			if (children_depth > max_children_depth)
				max_children_depth = children_depth;

			if (siblings_depth > max_siblings_depth)
				max_siblings_depth = siblings_depth;

			++index;
		}

		return root;
	}

	void parse_query_internal(const std::string& query, QueryNode& root, int& depth, int& array_depth, int index)
	{
		if (query == "")
			return;

		++depth;

		//here, query is for instance "a[2][3].b.c"

		size_t dot_pos = query.find('.');
		std::string query_before_dot = query.substr(0, dot_pos);

		//query_before_dot is "a[2][3]"

		size_t l_bracket_pos = query_before_dot.find('[');
		std::string query_before_brackets = query_before_dot.substr(0, l_bracket_pos);

		//query_before_brackets is "a"

		//query this node from the root if it exists:
		QueryNode* node = root.getChildQueried(query_before_brackets);

		//iterate over the remaining "[2][3]" string to get the multi-index queried:

		std::vector<int> array_multi_index;
		while (l_bracket_pos != std::string::npos)
		{
			std::string next = query_before_dot.substr(l_bracket_pos + 1);
			size_t r_bracket_pos = next.find(']');
			std::string index = next.substr(0, r_bracket_pos);
			array_multi_index.push_back(std::stoi(index));
			query_before_dot = next.substr(r_bracket_pos + 1);
			l_bracket_pos = query_before_dot.find('[');
			++array_depth;
		}

		if (!array_multi_index.empty())
		{
			node = node->getSibling(array_multi_index);
		}

		if (dot_pos == std::string::npos)
		{
			//if no deeper node is queried, it means we are interested in this node: flag it
			node->addQueried(index);
			return;
		}

		std::string query_after_dot = query.substr(dot_pos + 1);

		//query_after_dot is "b.c"

		//recurse with "b.c", but the corresponding root node is now located at "a[2][3]"
		parse_query_internal(query_after_dot, *node, depth, array_depth, index);
	}
}

#endif