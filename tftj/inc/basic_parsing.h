#ifndef __TFTJ_BASIC_PARSING__
#define __TFTJ_BASIC_PARSING__

#include "config.h"
#include "query.h"
#include "indexing.h"
#include "output_reader.h"
namespace tftj
{
	int c = 0;
	std::string aa;


	void basic_parse_json(int start, int end, int depth, int array_depth, const Query::map_t& query, const Character_Bitmap& data, OutputReader& out);

	void basic_parse_array(int start_, int end_, int depth, int array_depth, const Query::map_t& query, const Character_Bitmap& data, OutputReader& out)
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

			if (b_cumulative != 0) {
				l_brackets_found = b_cumulative &  data.bm_lbracket[w];
				break;
			}
		}

		if (!l_brackets_found)
		{
			return;
		}


		std::vector<int> pos_commas = generate_comma_position(start_, end_, &data.bm_commas[array_depth*data.n]);
		for (auto& p : query.arrays)
		{
			if (p.first - 1 >= pos_commas.size())
			{
				continue;
			}
			auto start = pos_commas[p.first - 1] + 1;
			auto end = ((p.first == pos_commas.size()) ? end_ : pos_commas[p.first]) - 1;
			if (p.second->isQueried)
			{
				std::string field = data.str.substr(start, end - start + 1);
				//std::cout << "key: " << p.second->node << "  : \n" << field << "\n\n";
				out.received(p.second->queryIndex, start, end);
			}
			if (!p.second->tree.empty())
			{
				basic_parse_json(start, end, depth+1, array_depth, *p.second, data, out);
			}
			else if (!p.second->arrays.empty())
			{
				basic_parse_array(start, end, depth, array_depth+1, *p.second, data, out);
			}
		}
	}

	void basic_parse_json(int start, int end, int depth, int array_depth, const Query::map_t& query, const Character_Bitmap& data, OutputReader& out)
	{
		//check if the next opening symbol is a {

		int word_start = start / word_bits;
		int word_end = (end + word_bits-1) / word_bits;

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
				l_braces_found = b_cumulative &  data.bm_lbrace[w];
				break;
			}
		}

		if (!l_braces_found)
		{
			return;
		}

		std::vector<int> pos = generate_colon_position(start, end, &data.bm_colons[depth*data.n]);

		int value_end_i = end;

		for (int i = pos.size() - 1; i >= 0; --i)
		{
			//look for the indices that a quoted word between pos[i-1] and pos[i]: this is the key at this position
			std::pair<int, int> field_i;
			bool success = search_pre_field_indices(data.bm_quote, i > 0 ? pos[i - 1] : 0, pos[i], &field_i);

			std::string key = data.str.substr(field_i.first + 1, field_i.second - field_i.first - 1);

			//look for the field in the query data
			const Query::map_t* inner_tree = nullptr;
			bool has_field = do_query(query, key, &inner_tree);

			//std::cout << "field:\t" <<  field << " : " << has_field << '\n';

			if (has_field)
			{
				std::pair<int, int> value_indices = search_post_value_indices(data.str, pos[i] + 1, value_end_i, i == (pos.size() - 1) ? true : false);

				if (inner_tree->isQueried) {
					//std::string field = data.str.substr(value_indices.first, value_indices.second - value_indices.first + 1);
					//std::cout << "key: " << inner_tree->node << "  : \n" << field << "\n\n";
					out.received(inner_tree->queryIndex, value_indices.first, value_indices.second);
					++c;
					//aa = field;
				}
				if (!inner_tree->tree.empty())
				{
					basic_parse_json(value_indices.first, value_indices.second, depth + 1, array_depth, *inner_tree, data, out);
				}
				else if (!inner_tree->arrays.empty())
				{
					basic_parse_array(value_indices.first, value_indices.second, depth, array_depth, *inner_tree, data, out);
				}
			}

			value_end_i = field_i.first - 1;

		}

	}

}
#endif