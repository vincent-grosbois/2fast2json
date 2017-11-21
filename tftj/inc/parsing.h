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
	void parse(const std::string& s, const Query& query, OutputReader& outputReader)
	{
		const int max_depth = query._depth;
		const int max_array_depth = query._array_depth;
		const bool needArray = max_array_depth > 0;
		const int n = (static_cast<int>(s.size()) + word_bits - 1) / word_bits;

		int memory_bytes = 
			sizeof(word_t) * n * (8 + max_depth) + (needArray ? sizeof(word_t) * n * (1 + max_array_depth) : 0) +
			sizeof(std::pair<int, word_t >) * sizeof(word_t) * n;

		LinearAllocator alloc(memory_bytes);

		//LinearAllocator alloc2(memory_bytes);

		//step 1:
		//Character_Bitmap char_bitmap0(alloc, n, max_depth, max_array_depth);
		//create_bitmap(char_bitmap, s);

		Character_Bitmap char_bitmap(alloc, n, max_depth, max_array_depth, s);
		create_bitmap_avx2(char_bitmap, s);

		//int diff = memcmp(alloc.start_memory, alloc2.start_memory, memory_bytes);

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

		if (needArray)
		{
			//step 4 for arrays
			remove_string_items(n, char_bitmap.bm_comma, char_bitmap.bm_string);
		}

		build_colon_and_comma_level_bm(n, max_depth, max_array_depth, alloc, char_bitmap);

		if (!query._tree.tree.empty())
		{
			basic_parse_json(0, static_cast<int>(s.size()) - 1, 0, 0, query._tree, char_bitmap, outputReader);
		}
		else if(!query._tree.arrays.empty())
		{
			basic_parse_array(0, static_cast<int>(s.size()) - 1, 0, 0, query._tree, char_bitmap, outputReader);
		}
	}


}




