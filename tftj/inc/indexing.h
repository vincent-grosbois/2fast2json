#ifndef __TFTJ_INDEXING__
#define __TFTJ_INDEXING__

#include <vector>

#include "linear_allocator.h"
#include "bits.h"

namespace tftj
{
	struct CharacterBitmap
	{
		const int n;
		const int max_children_depth;
		const int max_siblings_depth;

		LinearAllocator& allocator;

		word_t* const bm_backslash;
		word_t* const bm_quote;
		word_t* const bm_colon;
		word_t* const bm_lbrace;
		word_t* const bm_rbrace;
		word_t* const bm_lbracket;
		word_t* const bm_rbracket;

		word_t* const bm_string;

		word_t* const bm_colons;

		//only needed for arrays:
		word_t* const bm_comma;
		word_t* const bm_commas;

		//the initial json to parse:
		const std::string& str;

		//permanent memory requirement : sizeof(word_t) * n * (8 +  max_children_depth) + (max_siblings_depth > 0 ? sizeof(word_t) * n * (1 +  max_siblings_depth) : 0)
		//temp memory requirement : 0
		CharacterBitmap(LinearAllocator& linearAllocator, int n, int max_children_depth, int max_siblings_depth, const std::string& str) :
			n(n),
			max_children_depth(max_children_depth),
			max_siblings_depth(max_siblings_depth),
			allocator(linearAllocator),
			bm_backslash(linearAllocator.allocate<word_t>(n)),
			bm_quote(linearAllocator.allocate<word_t>(n)),
			bm_colon(linearAllocator.allocate<word_t>(n)),
			bm_lbrace(linearAllocator.allocate<word_t>(n)),
			bm_rbrace(linearAllocator.allocate<word_t>(n)),
			bm_lbracket(linearAllocator.allocate<word_t>(n)),
			bm_rbracket(linearAllocator.allocate<word_t>(n)),
			bm_string(linearAllocator.allocate<word_t>(n)),
			bm_colons(linearAllocator.allocate<word_t>(max_children_depth*n)),
			bm_comma(max_siblings_depth > 0 ? linearAllocator.allocate<word_t>(n) : nullptr),
			bm_commas(max_siblings_depth > 0 ? linearAllocator.allocate<word_t>(max_siblings_depth*n) : nullptr),
			str(str)
		{ }

		CharacterBitmap(const CharacterBitmap&) = delete;
		CharacterBitmap(const CharacterBitmap&&) = delete;
		CharacterBitmap& operator=(const CharacterBitmap&) = delete;
		CharacterBitmap& operator=(const CharacterBitmap&&) = delete;

		//TODO deallocate on delete!!
	};

	//remove from bm_quote the quotes that are escaped, ie the quotes that are preceded by an *odd* number of backslashes
	//it might be necessary to go back several words before to find where the preceding backslashes end
	void check_for_escaped_quotes(int n, const word_t* bm_backslash, word_t* bm_quote)
	{
		word_t bm_unstructured_quote_from_last_word = 0;

		//handle multiple "\" characters consecutively: only an odd number of backslashes before a quote is an escaped quote
		for (int i = 0; i < n; ++i)
		{
			word_t unstructured_quote = 0;

			//check for \" pattern of this word
			word_t next_word_first_quote = (i < n - 1) ? (bm_quote[i + 1] << (word_bits - 1)) : 0;
			word_t backslash_quote = ((bm_quote[i] >> 1) + next_word_first_quote) & bm_backslash[i];

			while (backslash_quote != 0)
			{
				//we have a \" pattern: now try to find how far back the \ sequence goes

				int conseq_backslash_num = 0;

				word_t backslash_mask = bm_backslash[i];

				word_t blackslash_quote_s = smear(backslash_quote);
				int cnt = popcount(blackslash_quote_s);
				word_t backslash_b_mask = (backslash_mask & blackslash_quote_s) << (word_bits - cnt);
				int leading_ones_num = leading_zeros(~backslash_b_mask);
				conseq_backslash_num += leading_ones_num;

				//the \ sequences goes all the way to the begining of the word: check in previous words as well
				if (leading_ones_num == cnt)
				{
					for (int j = i - 1; j >= 0; --j)
					{
						backslash_mask = bm_backslash[j];
						int leading_ones = leading_zeros(~backslash_mask);
						conseq_backslash_num += leading_ones;
						if (leading_ones != word_bits)
						{
							break;
						}
					}
				}

				//only flag the quote if there is an odd number of backslashes
				if ((conseq_backslash_num & 1) == 1)
				{
					unstructured_quote |= extract(backslash_quote);
				}

				backslash_quote = remove(backslash_quote);
			}

			bm_quote[i] &= ~((unstructured_quote << 1) | bm_unstructured_quote_from_last_word);

			bm_unstructured_quote_from_last_word = unstructured_quote >> (word_bits - 1);
		}

	}

	//set 1s in bm_string when the character is within a string (ie between real quotes), or maybe on the fence
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

	//remove characters from any bitmap when they are within a string
	void remove_string_items(int n, word_t* bitmap, const word_t* bm_string)
	{
		for (int i = 0; i < n; ++i)
		{
			bitmap[i] &= ~bm_string[i];
		}
	}


	struct token
	{
		int index;
		word_t word;
		bool is_braces; // is { or }
	};

	//permanent memory requirement : 0
	//temporary memory requirement : sizeof(token) * sizeof(word_t) * n
	void build_colon_and_comma_level_bm(int n, int max_depth, int max_array_depth, LinearAllocator& alloc, CharacterBitmap& bitmap)
	{
		ScopedAllocated<token> scopeAllocated(alloc, sizeof(word_t)*n);

		token* tokens = scopeAllocated.allocated;
		int tokens_index = 0;

		int braces_depth = 0;
		int brackets_depth = 0;

		for (int i = 0; i < max_depth; ++i)
		{
			memcpy(&bitmap.bm_colons[i * n], bitmap.bm_colon, n * sizeof(word_t));
		}

		for (int i = 0; i < max_array_depth; ++i)
		{
			memcpy(&bitmap.bm_commas[i * n], bitmap.bm_comma, n * sizeof(word_t));
		}


		for (int i = 0; i < n; ++i)
		{
			word_t w_left = bitmap.bm_lbrace[i] | bitmap.bm_lbracket[i];
			word_t w_right = bitmap.bm_rbrace[i] | bitmap.bm_rbracket[i];
			while (true)
			{
				const word_t b_right = extract(w_right);
				word_t b_left = extract(w_left);
				while (b_left != 0 && (b_right == 0 || b_left < b_right)) {

					//push a new opening token on the stack
					tokens[tokens_index].index = i;
					tokens[tokens_index].word = b_left;
					tokens[tokens_index].is_braces = b_left & bitmap.bm_lbrace[i];
					if (tokens[tokens_index].is_braces)
					{
						braces_depth++;
					}
					else
					{
						brackets_depth++;
					}

					tokens_index++;
					w_left = remove(w_left);
					b_left = extract(w_left);
				}

				if (b_right != 0)
				{
					if (tokens_index == 0)
					{
						//closing braces or brackets with no opening braces or bracket: malformed json
						throw new int(0); //TODO better handling
					}

					auto t = tokens[tokens_index - 1];
					--tokens_index;
					if (t.is_braces) //TODO should check if the closing symbol matches the opening token
					{
						braces_depth--;
					}
					else
					{
						brackets_depth--;
					}
					if (tokens_index == 0)
						return; //TODO: here we know if the type of the full json is array or json object

					//outside of this block, are we inside braces or brackets?
					bool in_braces = tokens[tokens_index - 1].is_braces;

					if (in_braces)
					{
						int level = braces_depth - 1;
						if (level >= 0 && level < max_depth) {

							int index_left = t.index; //word index of matching left brace
							b_left = t.word; //bit index of matching left brace

							//at this level, erase all colons that are between left and right matching braces:
							if (i == index_left) {
								bitmap.bm_colons[level*n + i] &= ~(b_right - b_left);
							}
							else {
								bitmap.bm_colons[level*n + index_left] &= (b_left - 1);
								memset(&bitmap.bm_colons[level*n + index_left + 1], 0, (i - (index_left + 1)) * sizeof(word_t));
								bitmap.bm_colons[level*n + i] &= ~(b_right - 1);
							}
						}
					}
					else
					{
						int level = brackets_depth - 1;
						if (level >= 0 && level < max_array_depth) {

							int index_left = t.index; //word index of matching left brace
							b_left = t.word; //bit index of matching left brace

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

				}
				w_right = remove(w_right);
				if (b_right == 0)
				{
					break;
				}
			}
		}
	}

	//TODO check if end is included
	std::vector<int> generate_items_position(int start, int end, const word_t* bm_item)
	{
		std::vector<int> res;
		for (int i = start / word_bits; i < (end + word_bits - 1) / word_bits; ++i)
		{
			word_t b_items = bm_item[i];
			while (b_items != 0)
			{
				word_t b_item = extract(b_items);
				int offset = word_bits*i + popcount(b_item - 1);
				if (offset >= start && offset <= end)
				{
					res.push_back(offset);
				}
				b_items = remove(b_items);
			}

		}
		return res;
	}


	//between start and end, we look for the last pair of quotes
	//we start with the last words and go backwards, however this gets complex since each word is iterated forward
	bool search_pre_field_indices(const word_t* b_quote, int start, int end, std::pair<int, int>& pair) {
		int start_quote_index = 0;
		int end_quote_index = 0;
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
						start_quote_index = offset;
					}
					else {
						start_quote_index = end_quote_index;
						end_quote_index = offset;
					}
					++n_quote;
				}
				m_quote = remove(m_quote);
			}
			if (n_quote >= 2) {
				pair = std::make_pair(start_quote_index, end_quote_index);
				return true;
			}
			if (n_quote == 1) {
				ei_set = true;
			}
		}
		return false; //todo should throw here
	}

	//look for the value between si and ei
	//we remove all whitespaces character and also the last closing character (either ] } or , )
	std::pair<int, int> search_post_value_indices(const std::string& rec, int si, int ei, bool ignore_once_char_braces_or_bracket) {
		bool ignore_once_char_ignored = false;
		int n = static_cast<int>(rec.length());
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
			//TODO better error handling
			throw new int(0);
		}
		while (si <= ei) {
			if (rec[ei] == ' ' ||
				rec[ei] == '\t' ||
				rec[ei] == '\r' ||
				rec[ei] == '\n') {
				--ei;
			}
			else if (
				( ignore_once_char_braces_or_bracket && (rec[ei] == '}' || rec[ei] == ']')) ||
				(!ignore_once_char_braces_or_bracket && (rec[ei] == ','))
				)
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
			//TODO better error handling
			throw new int(0);
		}
		return std::make_pair(si, ei);
	}

}
#endif