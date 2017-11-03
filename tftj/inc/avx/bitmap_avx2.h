#ifndef __TFTJ_BITMAP_AVX2__
#define __TFTJ_BITMAP_AVX2__

#include "../config.h" 
#include "../indexing.h"

#include <immintrin.h>

namespace tftj
{
	__m256i load_partial_avx2(char* data_, int remainder);
	__m256i make_256bits_mask(char ref);

	void create_bitmap_avx2_32bits(Character_Bitmap& bitmap, const std::string& s);
	void create_bitmap_avx2_64bits(Character_Bitmap& bitmap, const std::string& s);

	void create_bitmap_avx2(Character_Bitmap& bitmap, const std::string& s)
	{
#ifdef TFTJ_ENVIRONMENT64
		create_bitmap_avx2_64bits(bitmap, s);
#else
		create_bitmap_avx2_32bits(bitmap, s);
#endif // TFTJ_ENVIRONMENT64
	}

#ifdef TFTJ_ENVIRONMENT64
	template<bool useArray>
	void create_bitmap_avx2_64bits_base(Character_Bitmap& bitmap, const std::string& s)
	{
		const int n = s.size();
		const int num_full_words = n / 64;
		const int remainder = n % 64;

		static const __m256i mask_backslash = make_256bits_mask('\\');
		static const __m256i mask_quote = make_256bits_mask('"');
		static const __m256i mask_colon = make_256bits_mask(':');
		static const __m256i mask_leftbraces = make_256bits_mask('{');
		static const __m256i mask_rightbraces = make_256bits_mask('}');

		//used for array only
		static const __m256i mask_comma = make_256bits_mask(',');
		static const __m256i mask_leftbrackets = make_256bits_mask('[');
		static const __m256i mask_rightbrackets = make_256bits_mask(']');

		char* data = const_cast<char*>(s.data());

		for (int i = 0; i < num_full_words; ++i)
		{
			//unaligned load 
			__m256i val1 = _mm256_loadu_si256(reinterpret_cast<__m256i*>(data));
			__m256i val2 = _mm256_loadu_si256(reinterpret_cast<__m256i*>(data + 32));

			{
				unsigned int bitmask_backslash1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_backslash, val1));
				unsigned int bitmask_backslash2 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_backslash, val2));
				bitmap.bm_backslash[i] = (word_t(bitmask_backslash2) << 32) | word_t(bitmask_backslash1);
			}
			{
				unsigned int bitmask_quote1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_quote, val1));
				unsigned int bitmask_quote2 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_quote, val2));
				bitmap.bm_quote[i] = (word_t(bitmask_quote2) << 32) | word_t(bitmask_quote1);
			}
			{
				unsigned int bitmask_colon1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_colon, val1));
				unsigned int bitmask_colon2 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_colon, val2));
				bitmap.bm_colon[i] = (word_t(bitmask_colon2) << 32) | word_t(bitmask_colon1);
			}
			{
				unsigned int bitmask_leftbraces1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_leftbraces, val1));
				unsigned int bitmask_leftbraces2 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_leftbraces, val2));
				bitmap.bm_lbrace[i] = (word_t(bitmask_leftbraces2) << 32) | word_t(bitmask_leftbraces1);
			}
			{
				unsigned int bitmask_rightbraces1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_rightbraces, val1));
				unsigned int bitmask_rightbraces2 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_rightbraces, val2));
				bitmap.bm_rbrace[i] = (word_t(bitmask_rightbraces2) << 32) | word_t(bitmask_rightbraces1);
			}

			if (useArray)
			{
				{
					unsigned int bitmask_comma1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_comma, val1));
					unsigned int bitmask_comma2 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_comma, val2));
					bitmap.bm_comma[i] = (word_t(bitmask_comma2) << 32) | word_t(bitmask_comma1);
				}
				{
					unsigned int bitmask_leftbrackets1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_leftbrackets, val1));
					unsigned int bitmask_leftbrackets2 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_leftbrackets, val2));
					bitmap.bm_lbracket[i] = (word_t(bitmask_leftbrackets2) << 32) | word_t(bitmask_leftbrackets1);
				}
				{
					unsigned int bitmask_rightbrackets1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_rightbrackets, val1));
					unsigned int bitmask_rightbrackets2 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_rightbrackets, val2));
					bitmap.bm_rbracket[i] = (word_t(bitmask_rightbrackets2) << 32) | word_t(bitmask_rightbrackets1);
				}
			}

			data += 64;
		}

		if (remainder > 32)
		{
			//load unaligned
			__m256i val1 = _mm256_loadu_si256(reinterpret_cast<__m256i*>(data));
			__m256i val2 = load_partial_avx2(data + 32, remainder-32);

			{
				unsigned int bitmask_backslash1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_backslash, val1));
				unsigned int bitmask_backslash2 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_backslash, val2));
				bitmap.bm_backslash[num_full_words] = (word_t(bitmask_backslash2) << 32) | word_t(bitmask_backslash1);
			}
			{
				unsigned int bitmask_quote1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_quote, val1));
				unsigned int bitmask_quote2 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_quote, val2));
				bitmap.bm_quote[num_full_words] = (word_t(bitmask_quote2) << 32) | word_t(bitmask_quote1);
			}
			{
				unsigned int bitmask_colon1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_colon, val1));
				unsigned int bitmask_colon2 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_colon, val2));
				bitmap.bm_colon[num_full_words] = (word_t(bitmask_colon2) << 32) | word_t(bitmask_colon1);
			}
			{
				unsigned int bitmask_leftbraces1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_leftbraces, val1));
				unsigned int bitmask_leftbraces2 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_leftbraces, val2));
				bitmap.bm_lbrace[num_full_words] = (word_t(bitmask_leftbraces2) << 32) | word_t(bitmask_leftbraces1);
			}
			{
				unsigned int bitmask_rightbraces1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_rightbraces, val1));
				unsigned int bitmask_rightbraces2 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_rightbraces, val2));
				bitmap.bm_rbrace[num_full_words] = (word_t(bitmask_rightbraces2) << 32) | word_t(bitmask_rightbraces1);
			}

			if (useArray)
			{
				{
					unsigned int bitmask_comma1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_comma, val1));
					unsigned int bitmask_comma2 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_comma, val2));
					bitmap.bm_comma[num_full_words] = (word_t(bitmask_comma2) << 32) | word_t(bitmask_comma1);
				}
				{
					unsigned int bitmask_leftbrackets1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_leftbrackets, val1));
					unsigned int bitmask_leftbrackets2 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_leftbrackets, val2));
					bitmap.bm_lbracket[num_full_words] = (word_t(bitmask_leftbrackets2) << 32) | word_t(bitmask_leftbrackets1);
				}
				{
					unsigned int bitmask_rightbrackets1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_rightbrackets, val1));
					unsigned int bitmask_rightbrackets2 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_rightbrackets, val2));
					bitmap.bm_rbracket[num_full_words] = (word_t(bitmask_rightbrackets2) << 32) | word_t(bitmask_rightbrackets1);
				}
			}
		}
		else if (remainder == 32)
		{
			//load unaligned
			__m256i val = _mm256_loadu_si256(reinterpret_cast<__m256i*>(data));
			{
				unsigned int bitmask_backslash = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_backslash, val));
				bitmap.bm_backslash[num_full_words] = word_t(bitmask_backslash);
			}
			{
				unsigned int bitmask_quote = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_quote, val));
				bitmap.bm_quote[num_full_words] = word_t(bitmask_quote);
			}
			{
				unsigned int bitmask_colon = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_colon, val));
				bitmap.bm_colon[num_full_words] = word_t(bitmask_colon);
			}
			{
				unsigned int bitmask_leftbraces = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_leftbraces, val));
				bitmap.bm_lbrace[num_full_words] = word_t(bitmask_leftbraces);
			}
			{
				unsigned int bitmask_rightbraces = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_rightbraces, val));
				bitmap.bm_rbrace[num_full_words] = word_t(bitmask_rightbraces);
			}
			if (useArray)
			{
				{
					unsigned int bitmask_comma = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_comma, val));
					bitmap.bm_comma[num_full_words] = word_t(bitmask_comma);
				}
				{
					unsigned int bitmask_leftbrackets = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_leftbrackets, val));;
					bitmap.bm_lbracket[num_full_words] = word_t(bitmask_leftbrackets);
				}
				{
					unsigned int bitmask_rightbrackets = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_rightbrackets, val));
					bitmap.bm_rbracket[num_full_words] = word_t(bitmask_rightbrackets);
				}
			}
		}
		else if (remainder > 0) //so remainder is < 32 and > 0
		{
			__m256i val = load_partial_avx2(data, remainder);
			{
				unsigned int bitmask_backslash = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_backslash, val));
				bitmap.bm_backslash[num_full_words] = word_t(bitmask_backslash);
			}
			{
				unsigned int bitmask_quote = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_quote, val));
				bitmap.bm_quote[num_full_words] = word_t(bitmask_quote);
			}
			{
				unsigned int bitmask_colon = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_colon, val));
				bitmap.bm_colon[num_full_words] = word_t(bitmask_colon);
			}
			{
				unsigned int bitmask_leftbraces = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_leftbraces, val));
				bitmap.bm_lbrace[num_full_words] = word_t(bitmask_leftbraces);
			}
			{
				unsigned int bitmask_rightbraces = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_rightbraces, val));
				bitmap.bm_rbrace[num_full_words] = word_t(bitmask_rightbraces);
			}
			if (useArray)
			{
				{
					unsigned int bitmask_comma = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_comma, val));
					bitmap.bm_comma[num_full_words] = word_t(bitmask_comma);
				}
				{
					unsigned int bitmask_leftbrackets = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_leftbrackets, val));;
					bitmap.bm_lbracket[num_full_words] = word_t(bitmask_leftbrackets);
				}
				{
					unsigned int bitmask_rightbrackets = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_rightbrackets, val));
					bitmap.bm_rbracket[num_full_words] = word_t(bitmask_rightbrackets);
				}
			}
		}
	}

	void create_bitmap_avx2_64bits(Character_Bitmap& bitmap, const std::string& s)
	{
		if (bitmap.max_array_depth > 0)
		{
			create_bitmap_avx2_64bits_base<true>(bitmap, s);
		}
		else
		{
			create_bitmap_avx2_64bits_base<false>(bitmap, s);
		}
	}
#endif

#ifdef TFTJ_ENVIRONMENT32

	template<bool useArray>
	void create_bitmap_avx2_32bits_base(Character_Bitmap& bitmap, const std::string& s)
	{
		const int n = s.size();
		const int num_full_words = n / 32;
		const int remainder = n % 32;

		static const __m256i mask_backslash = make_256bits_mask('\\');
		static const __m256i mask_quote = make_256bits_mask('"');
		static const __m256i mask_colon = make_256bits_mask(':');
		static const __m256i mask_leftbraces = make_256bits_mask('{');
		static const __m256i mask_rightbraces = make_256bits_mask('}');

		//used for array only
		static const __m256i mask_comma = make_256bits_mask(',');
		static const __m256i mask_leftbrackets = make_256bits_mask('[');
		static const __m256i mask_rightbrackets = make_256bits_mask(']');

		char* data = const_cast<char*>(s.data());

		for (int i = 0; i < num_full_words; ++i)
		{
			//unaligned load 
			__m256i val1 = _mm256_loadu_si256(reinterpret_cast<__m256i*>(data));
			{
				unsigned int bitmask_backslash1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_backslash, val1));
				bitmap.bm_backslash[i] = word_t(bitmask_backslash1);
			}
			{
				unsigned int bitmask_quote1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_quote, val1));
				bitmap.bm_quote[i] = word_t(bitmask_quote1);
			}
			{
				unsigned int bitmask_colon1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_colon, val1));
				bitmap.bm_colon[i] = word_t(bitmask_colon1);
			}
			{
				unsigned int bitmask_leftbraces1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_leftbraces, val1));
				bitmap.bm_lbrace[i] = word_t(bitmask_leftbraces1);
			}
			{
				unsigned int bitmask_rightbraces1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_rightbraces, val1));
				bitmap.bm_rbrace[i] = word_t(bitmask_rightbraces1);
			}
			if (useArray)
			{
				{
					unsigned int bitmask_comma1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_comma, val1));
					bitmap.bm_comma[i] = word_t(bitmask_comma1);
				}
				{
					unsigned int bitmask_leftbrackets1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_leftbrackets, val1));
					bitmap.bm_lbracket[i] = word_t(bitmask_leftbrackets1);
				}
				{
					unsigned int bitmask_rightbrackets1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_rightbrackets, val1));
					bitmap.bm_rbracket[i] = word_t(bitmask_rightbrackets1);
				}
			}

			data += 32;
		}

		if (remainder > 0)
		{
			__m256i val = load_partial_avx2(data, remainder);
			{
				unsigned int bitmask_backslash = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_backslash, val));
				bitmap.bm_backslash[num_full_words] = word_t(bitmask_backslash);
			}
			{
				unsigned int bitmask_quote = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_quote, val));
				bitmap.bm_quote[num_full_words] = word_t(bitmask_quote);
			}
			{
				unsigned int bitmask_colon = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_colon, val));
				bitmap.bm_colon[num_full_words] = word_t(bitmask_colon);
			}
			{
				unsigned int bitmask_leftbraces = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_leftbraces, val));
				bitmap.bm_lbrace[num_full_words] = word_t(bitmask_leftbraces);
			}
			{
				unsigned int bitmask_rightbraces = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_rightbraces, val));
				bitmap.bm_rbrace[num_full_words] = word_t(bitmask_rightbraces);
			}
			if (useArray)
			{
				{
					unsigned int bitmask_comma1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_comma, val));
					bitmap.bm_comma[num_full_words] = word_t(bitmask_comma1);
				}
				{
					unsigned int bitmask_leftbrackets1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_leftbrackets, val));
					bitmap.bm_lbracket[num_full_words] = word_t(bitmask_leftbrackets1);
				}
				{
					unsigned int bitmask_rightbrackets1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(mask_rightbrackets, val));
					bitmap.bm_rbracket[num_full_words] = word_t(bitmask_rightbrackets1);
				}
			}
		}
	}

	void create_bitmap_avx2_32bits(Character_Bitmap& bitmap, const std::string& s)
	{
		if (bitmap.max_array_depth > 0)
		{
			create_bitmap_avx2_32bits_base<true>(bitmap, s);
		}
		else
		{
			create_bitmap_avx2_32bits_base<false>(bitmap, s);
		}
	}
#endif

	__m256i make_256bits_mask(char ref)
	{
		return _mm256_setr_epi8(ref, ref, ref, ref, ref, ref, ref, ref, ref, ref, ref, ref, ref, ref, ref, ref, ref, ref, ref, ref, ref, ref, ref, ref, ref, ref, ref, ref, ref, ref, ref, ref);
	}

	__m256i load_partial_avx2(char* data, int remainder)
	{
		__m256i val;

		switch (remainder)
		{
		case 1:
			val = _mm256_setr_epi8(data[0], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 2:
			val = _mm256_setr_epi8(data[0], data[1], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 3:
			val = _mm256_setr_epi8(data[0], data[1], data[2], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 4:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 5:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 6:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 7:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], data[6], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 8:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 9:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 10:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 11:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 12:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 13:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 14:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 15:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 16:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 17:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15], data[16], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 18:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15], data[16], data[17], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 19:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15], data[16], data[17], data[18], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 20:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15], data[16], data[17], data[18], data[19], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 21:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15], data[16], data[17], data[18], data[19], data[20], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 22:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15], data[16], data[17], data[18], data[19], data[20], data[21], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 23:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15], data[16], data[17], data[18], data[19], data[20], data[21], data[22], 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 24:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15], data[16], data[17], data[18], data[19], data[20], data[21], data[22], data[23], 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 25:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15], data[16], data[17], data[18], data[19], data[20], data[21], data[22], data[23], data[24], 0, 0, 0, 0, 0, 0, 0);
			break;
		case 26:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15], data[16], data[17], data[18], data[19], data[20], data[21], data[22], data[23], data[24], data[25], 0, 0, 0, 0, 0, 0);
			break;
		case 27:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15], data[16], data[17], data[18], data[19], data[20], data[21], data[22], data[23], data[24], data[25], data[26], 0, 0, 0, 0, 0);
			break;
		case 28:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15], data[16], data[17], data[18], data[19], data[20], data[21], data[22], data[23], data[24], data[25], data[26], data[27], 0, 0, 0, 0);
			break;
		case 29:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15], data[16], data[17], data[18], data[19], data[20], data[21], data[22], data[23], data[24], data[25], data[26], data[27], data[28], 0, 0, 0);
			break;
		case 30:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15], data[16], data[17], data[18], data[19], data[20], data[21], data[22], data[23], data[24], data[25], data[26], data[27], data[28], data[29], 0, 0);
			break;
		case 31:
			val = _mm256_setr_epi8(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15], data[16], data[17], data[18], data[19], data[20], data[21], data[22], data[23], data[24], data[25], data[26], data[27], data[28], data[29], data[30], 0);
			break;
		default:
			val = _mm256_setr_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		}

		return val;
	}

}

#endif