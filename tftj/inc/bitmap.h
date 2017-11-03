#ifndef __TFTJ_BITMAP__
#define __TFTJ_BITMAP__

#include "config.h"
#include "indexing.h"

namespace tftj
{
	//non-SIMD version
	template<bool useArray>
	void create_bitmap_base(Character_Bitmap& bitmap, const std::string& s)
	{
		memset(bitmap.bm_backslash, 0, sizeof(word_t)*bitmap.n);
		memset(bitmap.bm_quote, 0, sizeof(word_t)*bitmap.n);
		memset(bitmap.bm_colon, 0, sizeof(word_t)*bitmap.n);
		memset(bitmap.bm_lbrace, 0, sizeof(word_t)*bitmap.n);
		memset(bitmap.bm_rbrace, 0, sizeof(word_t)*bitmap.n);

		if (useArray)
		{
			memset(bitmap.bm_comma, 0, sizeof(word_t)*bitmap.n);
			memset(bitmap.bm_lbracket, 0, sizeof(word_t)*bitmap.n);
			memset(bitmap.bm_rbracket, 0, sizeof(word_t)*bitmap.n);
		}

		int remainder = 0;
		int size = s.size();

		for (int i = 0; i < size; ++i)
		{
			int index = i / word_bits;
			word_t mask = word_t(1) << remainder;

			switch (s[i]) {
			case '\\':
				bitmap.bm_backslash[index] |= mask;
				break;
			case '"':
				bitmap.bm_quote[index] |= mask;
				break;
			case ':':
				bitmap.bm_colon[index] |= mask;
				break;
			case '{':
				bitmap.bm_lbrace[index] |= mask;
				break;
			case '}':
				bitmap.bm_rbrace[index] |= mask;
				break;
			}

			if (useArray)
			{
				switch (s[i]) {
				case ',':
					bitmap.bm_comma[index] |= mask;
					break;
				case '[':
					bitmap.bm_lbracket[index] |= mask;
					break;
				case ']':
					bitmap.bm_rbracket[index] |= mask;
					break;
				}
			}

			if (++remainder >= word_bits)
			{
				remainder = 0;
			}
		}
	}

	void create_bitmap(Character_Bitmap& bitmap, const std::string& s)
	{
		if (bitmap.max_array_depth > 0)
		{
			create_bitmap_base<true>(bitmap, s);
		}
		else
		{
			create_bitmap_base<false>(bitmap, s);
		}
	}

}


#endif