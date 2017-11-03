#ifndef __TFTJ_INDEXING__
#define __TFTJ_INDEXING__

#include <vector>

#include "linear_allocator.h"
#include "bits.h"

namespace tftj
{
	struct Character_Bitmap
	{
		const int n;
		const int max_depth;
		const int max_array_depth;

		LinearAllocator& allocator;

		word_t* const bm_backslash;
		word_t* const bm_quote;
		word_t* const bm_colon;
		word_t* const bm_lbrace;
		word_t* const bm_rbrace;

		word_t* const bm_string;

		word_t* const bm_colons;

		//only needed for arrays:
		word_t* const bm_comma;
		word_t* const bm_lbracket;
		word_t* const bm_rbracket;
		word_t* const bm_commas;

		const std::string& str;

		//permanent memory requirement : sizeof(word_t) * n * (6 +  max_depth) + (max_array_depth > 0 ? sizeof(word_t) * n * (3 +  max_array_depth) : 0)
		//temp memory requirement : 0
		Character_Bitmap(LinearAllocator& linearAllocator, int n, int max_depth, int max_array_depth, const std::string& str) :
			n(n),
			max_depth(max_depth),
			max_array_depth(max_array_depth),
			allocator(linearAllocator),
			bm_backslash(linearAllocator.allocate<word_t>(n)),
			bm_quote(linearAllocator.allocate<word_t>(n)),
			bm_colon(linearAllocator.allocate<word_t>(n)),
			bm_lbrace(linearAllocator.allocate<word_t>(n)),
			bm_rbrace(linearAllocator.allocate<word_t>(n)),
			bm_string(linearAllocator.allocate<word_t>(n)),
			bm_colons(linearAllocator.allocate<word_t>(max_depth*n)),
			bm_comma(max_array_depth > 0 ? linearAllocator.allocate<word_t>(n) : nullptr),
			bm_lbracket(max_array_depth > 0 ? linearAllocator.allocate<word_t>(n) : nullptr),
			bm_rbracket(max_array_depth > 0 ? linearAllocator.allocate<word_t>(n) : nullptr),
			bm_commas(max_array_depth > 0 ? linearAllocator.allocate<word_t>(max_array_depth*n) : nullptr),
			str(str)
		{ }

		Character_Bitmap(const Character_Bitmap&) = delete;
		Character_Bitmap(const Character_Bitmap&&) = delete;
		Character_Bitmap& operator=(const Character_Bitmap&) = delete;
		Character_Bitmap& operator=(const Character_Bitmap&&) = delete;
	};


	void check_for_escaped_quotes(int n, const word_t* bm_backslash, word_t* bm_quote)
	{
		//check for '\"' pattern
		std::vector<word_t> bm_backslash_quote = std::vector<word_t>(n);
		for (int i = 0; i < n - 1; ++i)
		{
			bm_backslash_quote[i] = ((bm_quote[i] >> 1) + (bm_quote[i + 1] << (word_bits - 1))) & bm_backslash[i];
		}
		bm_backslash_quote[n - 1] = (bm_quote[n - 1] >> 1) & bm_backslash[n - 1];

		std::vector<word_t> bm_unstructured_quote = std::vector<word_t>(n);

		//handle multiple "\" characters consecutively: only an odd number of backslashes before a quote is an escaped quote
		for (int i = 0; i < n; ++i)
		{
			word_t unstructured_quote = 0;
			word_t backslash_quote = bm_backslash_quote[i];

			while (backslash_quote != 0)
			{
				int conseq_backslash_num = 0;
				for (int j = i; j >= 0; --j)
				{
					word_t backslash_mask = bm_backslash[j];
					if (j == i)
					{
						word_t blackslash_quote_s = smear(backslash_quote);
						int cnt = popcount(blackslash_quote_s);
						word_t backslash_b_mask = (backslash_mask & blackslash_quote_s) << (word_bits - cnt);
						int leading_ones_num = leading_zeros(~backslash_b_mask);
						conseq_backslash_num += leading_ones_num;
						if (leading_ones_num != cnt)
						{
							break;
						}
					}
					else
					{
						int leading_ones = leading_zeros(~backslash_mask);
						conseq_backslash_num += leading_ones;
						if (leading_ones != word_bits)
						{
							break;
						}
					}

				}
				if ((conseq_backslash_num & 1) == 1)
				{
					unstructured_quote |= extract(backslash_quote);
				}

				backslash_quote = remove(backslash_quote);

			}
			bm_unstructured_quote[i] = unstructured_quote;
		}

		//TODO this loop can be integrated in the previous one
		bm_quote[0] &= ~(bm_unstructured_quote[0] << 1);
		for (int i = 1; i < n; ++i)
		{
			bm_quote[i] &= ~((bm_unstructured_quote[i] << 1) | (bm_unstructured_quote[i - 1] >> (word_bits - 1)));
		}

	}

	void build_string_bitmap(int n, const word_t* bm_struct_quotes, word_t* bm_string)
	{
		int count = 0;
		for (int i = 0; i < n; ++i)
		{
			word_t m_struct_quote = bm_struct_quotes[i];
			word_t m_string = 0;
			while (m_struct_quote != 0)
			{
				word_t m = smear(m_struct_quote);
				m_string = m_string ^ m;
				m_struct_quote = remove(m_struct_quote);
				++count;
			}
			if ((count & 1) == 1)
			{
				m_string = ~m_string;
			}
			bm_string[i] = m_string;
		}
	}

	void remove_string_items(int n, word_t* bm, const word_t* bm_string)
	{
		for (int i = 0; i < n; ++i)
		{
			bm[i] &= ~bm_string[i];
		}
	}

	//permanent memory requirement : 0
	//temporary memory requirement : sizeof(std::pair<int, word_t>) * sizeof(word_t) * n
	void build_colon_level_bm(int n, int max_depth, LinearAllocator& alloc, Character_Bitmap& bitmap)
	{
		std::pair<int, word_t>* left_braces_stack = alloc.allocate<std::pair<int, word_t>>(sizeof(word_t)*n);
		int stack_index = -1;

		for (int i = 0; i < max_depth; ++i)
		{
			memcpy(&bitmap.bm_colons[i * n], bitmap.bm_colon, n * sizeof(word_t));
		}

		for (int i = 0; i < n; ++i)
		{
			word_t w_left = bitmap.bm_lbrace[i];
			word_t w_right = bitmap.bm_rbrace[i];
			while (true)
			{
				const word_t b_right = extract(w_right);
				word_t b_left = extract(w_left);
				while (b_left != 0 && (b_right == 0 || b_left < b_right)) {
					left_braces_stack[++stack_index] = std::make_pair(i, b_left);
					w_left = remove(w_left);
					b_left = extract(w_left);
				}

				if (b_right != 0)
				{
					if (stack_index < 0)
					{
						//closing braces with no opening braces: malformed json
						throw new int(0); //TODO better handling
					}

					std::pair<int, word_t> p = left_braces_stack[stack_index--]; 
					int level = stack_index;
					if (level > -1 && level < max_depth) {

						int index_left = p.first; //word index of matching left brace
						b_left = p.second; //bit index of matching left brace

						//at this level, erase all colons that are between left and right matching braces:
						if (i == index_left) {
							bitmap.bm_colons[level*n + i] &= ~(b_right - b_left);
						}
						else {
							bitmap.bm_colons[level*n + index_left] &= (b_left - 1);
							memset(&bitmap.bm_colons[level*n + index_left+1], 0, (i - (index_left+1)) * sizeof(word_t));
							bitmap.bm_colons[level*n + i] &= ~(b_right - 1);
						}
					}

				}
				w_right = remove(w_right);
				if (b_right == 0)
				{
					break;
				}
			}
		}

		alloc.deallocate<std::pair<int, word_t>>(sizeof(word_t)*n);
	}

	//permanent memory requirement : 0
	//temporary memory requirement : sizeof(std::pair<int, word_t>) * sizeof(word_t) * n
	void build_comma_level_bm(int n, int max_depth, LinearAllocator& alloc, Character_Bitmap& bitmap)
	{  //TODO also remove commas that are within {} blocks
		std::pair<int, word_t>* left_brackets_stack = alloc.allocate<std::pair<int, word_t>>(sizeof(word_t)*n);
		int stack_index = -1;

		for (int i = 0; i < max_depth; ++i)
		{
			memcpy(&bitmap.bm_commas[i * n], bitmap.bm_comma, n * sizeof(word_t));
		}

		for (int i = 0; i < n; ++i)
		{
			word_t w_left = bitmap.bm_lbracket[i];
			word_t w_right = bitmap.bm_rbracket[i];
			while (true)
			{
				const word_t b_right = extract(w_right);
				word_t b_left = extract(w_left);
				while (b_left != 0 && (b_right == 0 || b_left < b_right)) {
					left_brackets_stack[++stack_index] = std::make_pair(i, b_left);
					w_left = remove(w_left);
					b_left = extract(w_left);
				}

				if (b_right != 0)
				{
					if (stack_index < 0)
					{
						//closing brackets with no opening brackets: malformed json
						throw new int(0); //TODO better error handling
					}

					std::pair<int, word_t> p = left_brackets_stack[stack_index--];
					int level = stack_index;
					if (level > -1 && level < max_depth) {

						int index_left = p.first; //word index of matching left brace
						b_left = p.second; //bit index of matching left brace

						//at this level, erase all colons that are between left and right matching braces:
						if (i == index_left) {
							bitmap.bm_commas[level*n + i] &= ~(b_right - b_left);
						}
						else {
							bitmap.bm_commas[level*n + index_left] &= (b_left - 1);
							memset(&bitmap.bm_commas[level*n + index_left + 1], 0, (i - (index_left + 1)) * sizeof(word_t));
							bitmap.bm_commas[level*n + i] &= ~(b_right - 1);
						}
					}

				}
				w_right = remove(w_right);
				if (b_right == 0)
				{
					break;
				}
			}
		}

		alloc.deallocate<std::pair<int, word_t>>(sizeof(word_t)*n);
	}

	std::vector<int> generate_comma_position(int start, int end, const word_t* bm_comma)
	{
		std::vector<int> res;
		for (int i = start / word_bits; i < (end + word_bits - 1) / word_bits; ++i)
		{
			word_t b_cols = bm_comma[i];
			while (b_cols != 0)
			{
				word_t b_col = extract(b_cols);
				int offset = word_bits*i + popcount(b_col - 1);
				if (offset >= start && offset <= end)
				{
					res.push_back(offset);
				}
				b_cols = remove(b_cols);
			}

		}
		return res;
	}

	std::vector<int> generate_colon_position(int start, int end, const word_t* bm_col)
	{
		std::vector<int> res;
		for (int i = start / word_bits; i < (end + word_bits - 1) / word_bits; ++i)
		{
			word_t b_cols = bm_col[i];
			while (b_cols != 0)
			{
				word_t b_col = extract(b_cols);
				int offset = word_bits*i + popcount(b_col - 1);
				if (offset >= start && offset <= end)
				{
					res.push_back(offset);
				}
				b_cols = remove(b_cols);
			}

		}
		return res;
	}

	bool search_pre_field_indices(const word_t* b_quote, int start, int end, std::pair<int, int>* pair) {
		int si = 0;
		int ei = 0;
		bool ei_set = false;
		int n_quote = 0;
		for (int i = (end + word_bits - 1) / word_bits - 1; i >= start / word_bits; --i) {
			word_t m_quote = b_quote[i];
			while (m_quote != 0) {
				word_t m_bit = extract(m_quote);
				int offset = i * word_bits + popcount(m_bit - 1);
				if (offset >= end) {
					break;
				}
				if (start < offset) {
					if (ei_set) {
						si = offset;
					}
					else {
						si = ei;
						ei = offset;
					}
					++n_quote;
				}
				m_quote = remove(m_quote);
			}
			if (n_quote >= 2) {
				*pair = std::make_pair(si, ei);
				return true;
			}
			if (n_quote == 1) {
				ei_set = true;
			}
		}
		return false;
	}

	std::pair<int, int> search_post_value_indices(const std::string& rec, int si, int ei, char ignore_once_char) {
		bool ignore_once_char_ignored = false;
		int n = rec.length();
		while (si < n) {
			if (rec[si] == ' ' ||
				rec[si] == '\t' ||
				rec[si] == '\r' ||
				rec[si] == '\n') {
				++si;
			}
			else {
				break;
			}
		}

		if (si == n) {
			//return Err(Error::from(ErrorKind::InvalidRecord));
		}
		while (si <= ei) {
			if (rec[ei] == ' ' ||
				rec[ei] == '\t' ||
				rec[ei] == '\r' ||
				rec[ei] == '\n') {
				--ei;
			}
			else if (rec[ei] == ignore_once_char)
			{
				if (ignore_once_char_ignored) {
					break;
				}
				ignore_once_char_ignored = true;
				--ei;
			}
			else {
				break;
			}
		}

		if (ei < si) {
			//return Err(Error::from(ErrorKind::InvalidRecord));
		}
		return std::make_pair(si, ei);
	}

}
#endif