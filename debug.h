#include <bitset>
#include <vector>
#include <iostream>

#include "bits.h"
namespace tftj
{
	void dump_vector(const std::vector<word_t>&  v)
	{
		for (auto i : v)
		{
			std::cout << std::bitset<word_bits>(i) << '\n';
		}
		std::cout << std::endl;
	}

	void dump_str(const std::string& s, bool reversed = true)
	{
		int size = (s.length() + word_bits - 1) / word_bits;
		for (int i = 0; i < size; ++i)
		{
			auto substr = s.substr(i*word_bits, word_bits);
			if (!reversed)
			{
				std::cout << substr << '\n';
			}
			else
			{
				for (auto it = substr.rbegin(); it != substr.rend(); ++it)
				{
					std::cout << *it;
				}
				std::cout << '\n';
			}
		}
		std::cout << std::endl;
	}
}