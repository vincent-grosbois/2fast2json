#ifndef __TFTJ_BASIC_PARSING__
#define __TFTJ_BASIC_PARSING__

#include "config.h"
#include "query.h"
#include "indexing.h"
#include "output_reader.h"
#include <unordered_set>

namespace tftj
{
	void basic_parse_json(int start, int end, int depth, int array_depth, QueryNode& query, const CharacterBitmap& data, OutputReader& out, bool useSpeculativeParsing);
	void speculative_parse_json(int start, int end, int depth, int array_depth, QueryNode& query, const CharacterBitmap& data, OutputReader& out);

	void basic_parse_array(int start_, int end_, int depth, int array_depth, QueryNode& query, const CharacterBitmap& data, OutputReader& out, bool useSpeculativeParsing)
	{
		//check if the next opening symbol is a [

		int word_start = start_ / word_bits;
		int word_end = (end_ + word_bits - 1) / word_bits;

		bool l_brackets_found = false;

		for (int w = word_start; w <= word_end; ++w)
		{
			word_t cumulative = data.bm_lbrace[w] | data.bm_lbracket[w];

			word_t b_cumulative = extract(cumulative);

			while (b_cumulative != 0 && 
				((w == word_start && (word_start*word_bits + popcount(smear(b_cumulative))-1 < start_ ) )||
				 (w == word_end && (word_end*word_bits + popcount(smear(b_cumulative)) - 1 >= end_)))
				)
			{
				cumulative = remove(cumulative);
				b_cumulative = extract(cumulative);
			}

			if (b_cumulative != 0)
			{
				l_brackets_found = b_cumulative & data.bm_lbracket[w];
				break;
			}
		}

		if (!l_brackets_found)
		{
			return;
		}


		std::vector<int> pos_commas = generate_items_position(start_, end_, &data.bm_commas[array_depth*data.n]);
		for (auto& p : query.sibling_nodes)
		{
			int array_index = p.first;
			if (array_index - 1 >= pos_commas.size())
			{
				continue;
			}
			auto start = ((array_index == 0                ) ? start_  : pos_commas[array_index - 1]) + 1;
			auto end =   ((array_index == pos_commas.size()) ? end_    : pos_commas[array_index    ]) - 1;
			if (p.second->is_queried)
			{
				//std::string field = data.str.substr(start, end - start + 1);
				//std::cout << "key: " << p.second->node << "  : \n" << field << "\n\n";
				out.received(p.second->query_index, start, end);
			}
			if (!p.second->children_nodes.empty())
			{
				if (!useSpeculativeParsing)
				{
					basic_parse_json(start, end, depth + 1, array_depth, *p.second, data, out, false);
				}
				else
				{
					speculative_parse_json(start, end, depth + 1, array_depth, *p.second, data, out);
				}
			}
			else if (!p.second->sibling_nodes.empty())
			{
				basic_parse_array(start, end, depth, array_depth + 1, *p.second, data, out, useSpeculativeParsing);
			}
		}
	}

	void basic_parse_json(int start, int end, int depth, int array_depth, QueryNode& query, const CharacterBitmap& data, OutputReader& out, bool useSpeculativeParsing)
	{
		//check if the next opening symbol is a {

		int word_start = start / word_bits;
		int word_end = (end + word_bits - 1) / word_bits;

		bool l_braces_found = false;

		for (int w = word_start; w <= word_end; ++w)
		{
			word_t cumulative = data.bm_lbrace[w] | data.bm_lbracket[w];
			word_t b_cumulative = extract(cumulative);

			while (b_cumulative != 0 &&
				((w == word_start && (word_start*word_bits + popcount(smear(b_cumulative)) - 1 < start)) ||
				(w == word_end && (word_end*word_bits + popcount(smear(b_cumulative)) - 1 >= end)))
				)
			{
				cumulative = remove(cumulative);
				b_cumulative = extract(cumulative);
			}

			if (b_cumulative != 0) {
				l_braces_found = b_cumulative & data.bm_lbrace[w];
				break;
			}
		}

		if (!l_braces_found)
		{
			return;
		}

		std::vector<int> pos = generate_items_position(start, end, &data.bm_colons[depth*data.n]);

		int value_end_i = end;

		std::vector<std::pair<std::string, int>> list; //todo pre-alloc on nb of queried fields
		std::unordered_set<std::string> nodes_found;
		std::vector<std::pair<std::string, int>> list_nodes;

		for (int i = static_cast<int>(pos.size()) - 1; i >= 0; --i)
		{
			//look for the last quoted word between pos[i-1] and pos[i]: this is the key at this position
			std::pair<int, int> key_location;
			bool success = search_pre_field_indices(data.bm_quote, i > 0 ? pos[i - 1] : 0, pos[i], key_location);

			std::string key = data.str.substr(key_location.first + 1, key_location.second - key_location.first - 1);

			//look for the field in the query data
			auto it = query.children_nodes.find(key);
			if (it != query.children_nodes.end())
			{
				QueryNode* inner_tree = it->second;

				//check for the value after the current : and before the start of the next key
				std::pair<int, int> value_indices = search_post_value_indices(data.str, pos[i] + 1, value_end_i, i == (pos.size() - 1));

				if (inner_tree->is_queried) //todo handle the case where the key already appeared
				{
					out.received(inner_tree->query_index, value_indices.first, value_indices.second);
				}

				if (!inner_tree->children_nodes.empty())
				{
					if (!useSpeculativeParsing)
					{
						basic_parse_json(value_indices.first, value_indices.second, depth + 1, array_depth, *inner_tree, data, out, false);
					}
					else
					{
						speculative_parse_json(value_indices.first, value_indices.second, depth + 1, array_depth, *inner_tree, data, out);
					}
				}
				else if (!inner_tree->sibling_nodes.empty())
				{
					basic_parse_array(value_indices.first, value_indices.second, depth, array_depth, *inner_tree, data, out, useSpeculativeParsing);
				}

				if (!useSpeculativeParsing)
				{
					list.push_back(std::make_pair(key, i));
					nodes_found.emplace(key);
				}
			}

			value_end_i = key_location.first - 1;

		}

		if (!useSpeculativeParsing)
		{
			for (auto& v : query.children_nodes)
			{
				if (nodes_found.find(v.first) == nodes_found.end())
				{
					list_nodes.push_back(std::make_pair(v.first, -1));
				}
			}
			for (auto& v : list)
			{
				list_nodes.push_back(v);
			}

			//should be an assert:
			if (list_nodes.size() != query.children_nodes.size())
			{
				throw 1;
			}

			query.speculative_tree.insert(list_nodes.data(), list_nodes.size());
		}

	}

	void speculative_parse_json(int start, int end, int depth, int array_depth, QueryNode& query, const CharacterBitmap& data, OutputReader& out)
	{
		//check if the next opening symbol is a {

		int word_start = start / word_bits;
		int word_end = (end + word_bits - 1) / word_bits;

		bool l_braces_found = false;

		for (int w = word_start; w <= word_end; ++w)
		{
			word_t cumulative = data.bm_lbrace[w] | data.bm_lbracket[w];
			word_t b_cumulative = extract(cumulative);

			while (b_cumulative != 0 &&
				((w == word_start && (word_start*word_bits + popcount(smear(b_cumulative)) - 1 < start)) ||
				(w == word_end && (word_end*word_bits + popcount(smear(b_cumulative)) - 1 >= end)))
				)
			{
				cumulative = remove(cumulative);
				b_cumulative = extract(cumulative);
			}

			if (b_cumulative != 0) {
				l_braces_found = b_cumulative & data.bm_lbrace[w];
				break;
			}
		}

		if (!l_braces_found)
		{
			return;
		}

		std::vector<int> pos = generate_items_position(start, end, &data.bm_colons[depth*data.n]);


		SpeculativeParsingNode* speculativeParsingNode = &query.speculative_tree;

		bool success = false;

		while (speculativeParsingNode != nullptr)
		{
			success = speculativeParsingNode->children.empty();

			//try all the potential nodes
			for (auto node : speculativeParsingNode->children)
			{
				if (node->index == -1)
				{
					break; //key is not found --> TODO
				}

				int i = node->index;

				if (i >= pos.size())
				{
					continue; //this object doesn't even have enough fields --> try next
				}

				//look for the last quoted word between pos[i-1] and pos[i]: this is the key at this position
				std::pair<int, int> key_location;
				search_pre_field_indices(data.bm_quote, i > 0 ? pos[i - 1] : 0, pos[i], key_location);

				std::string key = data.str.substr(key_location.first + 1, key_location.second - key_location.first - 1);

				if (key != node->name) //verify if the key is indeed here
				{
					continue; // no --> try next children
				}

				success = true;

				//look for the field in the query data
				auto it = query.children_nodes.find(key);

				QueryNode* inner_tree = it->second;

				//check for the value after the current : and before the start of the next key

				int value_end_i = end;

				if (i+1 < pos.size())
				{
					std::pair<int, int> key_location;
					bool success = search_pre_field_indices(data.bm_quote, pos[i], pos[i+1], key_location);
					value_end_i = key_location.first - 1;
				}

				std::pair<int, int> value_indices = search_post_value_indices(data.str, pos[i] + 1, value_end_i, i == (pos.size() - 1));

				if (inner_tree->is_queried) //todo handle the case where the key already appeared
				{
					out.received(inner_tree->query_index, value_indices.first, value_indices.second);
				}

				if (!inner_tree->children_nodes.empty())
				{
					speculative_parse_json(value_indices.first, value_indices.second, depth + 1, array_depth, *inner_tree, data, out);
				}
				else if (!inner_tree->sibling_nodes.empty())
				{
					basic_parse_array(value_indices.first, value_indices.second, depth, array_depth, *inner_tree, data, out, true);
				}

				speculativeParsingNode = node;
				break; //we're finished exploring this node
			}

			if (!success)
			{
				break; //couldn't complete speculative parsing
			}

			if (speculativeParsingNode->children.empty())
			{
				break; //we exhausted all the nodes
			}
		}

		if (!success)
		{
			//fallback on basic parsing, from scratch
			basic_parse_json(start, end, depth, array_depth, query, data, out, false);
		}

	}

}
#endif