#include <string.h>
#include <string>
#include <fstream>
#include "inc/indexing.h"
#include "query.h"
#include "inc/debug.h"
#include "inc/bitmap.h"
#include "inc/avx/bitmap_avx2.h"
#include "inc/basic_parsing.h"
#include "inc/output_reader.h"

namespace tftj
{
	void parse(const std::string& json, Query& query, OutputReader& outputReader, bool useAvx2, bool useSpeculativeParsing)
	{
		const int max_depth = query.children_depth;
		const int max_array_depth = query.siblings_depth;
		const bool need_array = max_array_depth > 0;

		const int string_size = static_cast<int>(json.size());
		const int n = (string_size + word_bits - 1) / word_bits;

		int memory_bytes = 
			sizeof(word_t) * n * (8 + max_depth) + (need_array ? sizeof(word_t) * n * (1 + max_array_depth) : 0) +
			sizeof(token) * sizeof(word_t) * n;

		LinearAllocator alloc(memory_bytes);

		//step 1: create bitmap array
		CharacterBitmap char_bitmap(alloc, n, max_depth, max_array_depth, json);
		if (useAvx2)
		{
			create_bitmap_avx2(char_bitmap, json);
		}
		else
		{
			create_bitmap(char_bitmap, json);
		}

		//step 2: build structural quotes bitmap
		check_for_escaped_quotes(n, char_bitmap.bm_backslash, char_bitmap.bm_quote);

		//step 3: build string bitmap
		build_string_bitmap(n, char_bitmap.bm_quote, char_bitmap.bm_string);

		//step 4: remove token characters that are within strings
		remove_string_items(n, char_bitmap.bm_colon, char_bitmap.bm_string);
		remove_string_items(n, char_bitmap.bm_lbrace, char_bitmap.bm_string);
		remove_string_items(n, char_bitmap.bm_rbrace, char_bitmap.bm_string);
		remove_string_items(n, char_bitmap.bm_lbracket, char_bitmap.bm_string);
		remove_string_items(n, char_bitmap.bm_rbracket, char_bitmap.bm_string);

		if (need_array)
		{
			//step 4 for arrays
			remove_string_items(n, char_bitmap.bm_comma, char_bitmap.bm_string);
		}

		build_colon_and_comma_level_bm(n, max_depth, max_array_depth, alloc, char_bitmap);

		if (!query.tree.children_nodes.empty())
		{
			if (useSpeculativeParsing)
			{
				speculative_parse_json(0, string_size - 1, 0, 0, query.tree, char_bitmap, outputReader);
			}
			else
			{
				basic_parse_json(0, string_size - 1, 0, 0, query.tree, char_bitmap, outputReader, false);
			}
		}
		else if(!query.tree.sibling_nodes.empty())
		{
			basic_parse_array(0, string_size - 1, 0, 0, query.tree, char_bitmap, outputReader, useSpeculativeParsing);
		}
	}

	class Parser
	{
		Query& query;
		OutputReader& outputReader;
		const bool useAvx2;
		const int num_training;
		int num_parsed;
		bool use_speculative_parsing;

		Parser(Parser&) = delete;
		Parser(Parser&&) = delete;
		Parser& operator=(Parser&) = delete;
		Parser& operator=(Parser&&) = delete;

	public :
		Parser(Query& query, OutputReader& outputReader, bool useAvx2, int num_training):
			query(query),
			outputReader(outputReader),
			useAvx2(useAvx2),
			num_training(num_training),
			num_parsed(0),
			use_speculative_parsing(false)
		{ }

		void parse(const std::string& json)
		{
			tftj::parse(json, query, outputReader, useAvx2, use_speculative_parsing);
			++num_parsed;
			if (!use_speculative_parsing && num_training > 0 && num_parsed >= num_training)
			{
				query.tree.CleanSpeculativeTree();
				use_speculative_parsing = true;
			}
		}

	};

}




