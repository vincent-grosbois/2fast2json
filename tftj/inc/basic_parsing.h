#ifndef __TFTJ_BASIC_PARSING__
#define __TFTJ_BASIC_PARSING__

#include "config.h"
#include "query.h"
#include "indexing.h"
namespace tftj
{
	int c = 0;
	std::string aa;


	void basic_parse(int start, int end, int depth, int array_depth, const Query::map_t& query, const Character_Bitmap& data);

	void recurse_for_array(int start_, int end_, int depth, int array_depth, const Query::map_t& query, const Character_Bitmap& data)
	{
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
				std::cout << "key: " << p.second->node << "  : \n" << field << "\n\n";
			}
			if (!p.second->tree.empty())
			{
				//todo : not do it if this is not a json
				basic_parse(start, end, depth+1, array_depth, *p.second, data);
			}
			else if (!p.second->arrays.empty())
			{//todo : not do it if this is not an array
				recurse_for_array(start, end, depth, array_depth+1, *p.second, data);
			}
		}
	}

	void basic_parse(int start, int end, int depth, int array_depth, const Query::map_t& query, const Character_Bitmap& data)
	{
		std::vector<int> pos = generate_colon_position(start, end, &data.bm_colons[depth*data.n]);

		int value_end_i = end;

		for (int i = pos.size() - 1; i >= 0; --i)
		{
			//look for the indices that a quoted word between post[i-1] and pos[i]: this is the key at this position
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
					std::string field = data.str.substr(value_indices.first, value_indices.second - value_indices.first + 1);
					std::cout << "key: " << inner_tree->node << "  : \n" << field << "\n\n";
					++c;
					aa = field;
				}
				if (!inner_tree->tree.empty())
				{
					//todo : not do it if this is not a json
					basic_parse(value_indices.first, value_indices.second, depth + 1, array_depth, *inner_tree, data);
				}
				else if (!inner_tree->arrays.empty())
				{
					//todo : not do it if this is not an array
					recurse_for_array(value_indices.first, value_indices.second, depth, array_depth, *inner_tree, data);
				}
			}

			value_end_i = field_i.first - 1;

		}

	}

}
#endif