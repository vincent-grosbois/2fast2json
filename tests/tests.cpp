#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include "../tftj/inc/config.h"
#include "../tftj/inc/bitmap.h"
#include "../tftj/inc/avx/bitmap_avx2.h"
#include "../tftj/inc/indexing.h"
#include "../tftj/inc/linear_allocator.h"

#include <algorithm>

using namespace tftj;

word_t reverseBits(word_t num)
{
	const int num_of_bits = sizeof(num) * 8;
	word_t reverse_num = 0;

	for (int i = 0; i < num_of_bits; ++i)
	{
		word_t temp = (num & (1uLL << i));
		if (temp)
			reverse_num |= (1uLL << ((num_of_bits - 1) - i));
	}

	return reverse_num;
}

TEST_CASE("Character bitmap should work") {

	std::string input = R"(TOTO{OTO:OTOdgn\\jfnjksrdr","dlkk00})";
	int n = (input.size() + word_bits - 1) / word_bits;
	LinearAllocator alloc(1000);

	Character_Bitmap bm(alloc, n, 1, 1, input);

	create_bitmap(bm, input);

#ifdef TFTJ_ENVIRONMENT32
	REQUIRE(bm.n == 2);

	REQUIRE(bm.bm_backslash[0] == 0x00018000);
	REQUIRE(bm.bm_backslash[1] == 0x00000000);

	REQUIRE(bm.bm_colon[0] == 0x00000100);
	REQUIRE(bm.bm_colon[1] == 0x00000000);

	REQUIRE(bm.bm_lbrace[0] == 0x00000010);
	REQUIRE(bm.bm_lbrace[1] == 0x00000000);

	REQUIRE(bm.bm_quote[0] == 0x14000000);
	REQUIRE(bm.bm_quote[1] == 0x00000000);

	REQUIRE(bm.bm_rbrace[0] == 0x00000000);
	REQUIRE(bm.bm_rbrace[1] == 0x00000008);
#else
	REQUIRE(bm.n == 1);

	REQUIRE(bm.bm_backslash[0] == 0x0000000000018000);

	REQUIRE(bm.bm_colon[0] == 0x0000000000000100);

	REQUIRE(bm.bm_lbrace[0] == 0x0000000000000010);

	REQUIRE(bm.bm_quote[0] == 0x0000000014000000);

	REQUIRE(bm.bm_rbrace[0] == 0x0000000800000000);
#endif
}

TEST_CASE("AVX2 character bitmap should work") {

	std::string input = R"(TOTO{OTO:OTOdgn\\jfnjksrdr","dlkk00})";
	int n = (input.size() + word_bits - 1) / word_bits;
	LinearAllocator alloc(1000);
	Character_Bitmap bm(alloc, n, 1, 1, input);
	create_bitmap_avx2(bm, input);

#ifdef TFTJ_ENVIRONMENT32
	REQUIRE(bm.n == 2);

	REQUIRE(bm.bm_backslash[0] == 0x00018000);
	REQUIRE(bm.bm_backslash[1] == 0x00000000);

	REQUIRE(bm.bm_colon[0] == 0x00000100);
	REQUIRE(bm.bm_colon[1] == 0x00000000);

	REQUIRE(bm.bm_lbrace[0] == 0x00000010);
	REQUIRE(bm.bm_lbrace[1] == 0x00000000);

	REQUIRE(bm.bm_quote[0] == 0x14000000);
	REQUIRE(bm.bm_quote[1] == 0x00000000);

	REQUIRE(bm.bm_rbrace[0] == 0x00000000);
	REQUIRE(bm.bm_rbrace[1] == 0x00000008);
#else
	REQUIRE(bm.n == 1);

	REQUIRE(bm.bm_backslash[0] == 0x0000000000018000);

	REQUIRE(bm.bm_colon[0] == 0x0000000000000100);

	REQUIRE(bm.bm_lbrace[0] == 0x0000000000000010);

	REQUIRE(bm.bm_quote[0] == 0x0000000014000000);

	REQUIRE(bm.bm_rbrace[0] == 0x0000000800000000);
#endif
}


TEST_CASE("backslash quotes should be escaped") {

#ifdef TFTJ_ENVIRONMENT64
std::string input	= R"(toto"quoted"__\"notquoted\"uuu\\"quoted\\"uuu\\\"unquoted\\\")";
word_t result		=  0b0000100000010000000000000000000010000000010000000000000000000000;

LinearAllocator alloc(1000);
int n = (input.size() + word_bits - 1) / word_bits;
Character_Bitmap bm(alloc, n, 1, 1, input);
create_bitmap(bm, input);
check_for_escaped_quotes(bm.n, bm.bm_backslash, bm.bm_quote); 
auto a = reverseBits(result);
REQUIRE(bm.bm_quote[0] == a);

#else
std::string input = R"(toto"quoted"__\"notquoted\"uo\\\\"quoted\\"uuu\\\"unquoted\\\")";
word_t result1 =     0b00001000000100000000000000000000;
word_t result2 =                                     0b01000000001000000000000000000000;

LinearAllocator alloc(1000);
int n = (input.size() + word_bits - 1) / word_bits;
Character_Bitmap bm(alloc, n, 1, 1, input);
create_bitmap(bm, input);


check_for_escaped_quotes(bm.n, bm.bm_backslash, bm.bm_quote);

REQUIRE(bm.bm_quote[0] == reverseBits(result1));
REQUIRE(bm.bm_quote[1] == reverseBits(result2));
#endif

}


TEST_CASE("build string bitmap") {

#ifdef TFTJ_ENVIRONMENT64
std::string input = R"(toto"quoted"djgkjdbgjk"123456"jsdnfkjd""jkjk)";

word_t result1     = 0b0000011111110000000000011111110000000001000000000000000000000000;
LinearAllocator alloc(1000);
int n = (input.size() + word_bits - 1) / word_bits;
Character_Bitmap bm(alloc, n, 1, 1, input);
create_bitmap(bm, input);

build_string_bitmap(bm.n, bm.bm_quote, bm.bm_string);

REQUIRE(bm.bm_string[0] == reverseBits(result1));

#else
std::string input = R"(toto"quoted"djgkjdbgjk"123456"jsdnfkjd""jkjk)";
word_t result1     = 0b00000111111100000000000111111100;
word_t result2                                     = 0b00000001000000000000000000000000;

LinearAllocator alloc(1000);
int n = (input.size() + word_bits - 1) / word_bits;
Character_Bitmap bm(alloc, n, 1, 1, input);
create_bitmap(bm, input);

build_string_bitmap(bm.n, bm.bm_quote, bm.bm_string);

REQUIRE(bm.bm_string[0] == reverseBits(result1));
REQUIRE(bm.bm_string[1] == reverseBits(result2));
#endif

}

TEST_CASE("remove tokens inside string") {

#ifdef TFTJ_ENVIRONMENT64
	std::string input = R"(toto"quoted\"djg{jdbgjkb:23456aj,dnfkj}aajkj")";

	LinearAllocator alloc(1000);
	int n = (input.size() + word_bits - 1) / word_bits;
	Character_Bitmap bm(alloc, n, 1, 1, input);
	create_bitmap(bm, input);
	check_for_escaped_quotes(bm.n, bm.bm_backslash, bm.bm_quote);
	build_string_bitmap(bm.n, bm.bm_quote, bm.bm_string);
	remove_string_items(n, bm.bm_colon, bm.bm_string);
	remove_string_items(n, bm.bm_lbrace, bm.bm_string);
	remove_string_items(n, bm.bm_rbrace, bm.bm_string);

	REQUIRE(bm.bm_colon[0] == 0);
	REQUIRE(bm.bm_lbrace[0] == 0);
	REQUIRE(bm.bm_rbrace[0] == 0);
#else
	std::string input = R"(toto"quoted\"djg{jdbgjkb:23456aj,dnfkj}aajkj")";

	LinearAllocator alloc(1000);
	int n = (input.size() + word_bits - 1) / word_bits;
	Character_Bitmap bm(alloc, n, 1, 1, input);
	create_bitmap(bm, input);
	check_for_escaped_quotes(bm.n, bm.bm_backslash, bm.bm_quote);
	build_string_bitmap(bm.n, bm.bm_quote, bm.bm_string);
	remove_string_items(n, bm.bm_colon, bm.bm_string);
	remove_string_items(n, bm.bm_lbrace, bm.bm_string);
	remove_string_items(n, bm.bm_rbrace, bm.bm_string);

	REQUIRE(bm.bm_colon[0] == 0);
	REQUIRE(bm.bm_colon[1] == 0);
	REQUIRE(bm.bm_lbrace[0] == 0);
	REQUIRE(bm.bm_lbrace[1] == 0);
	REQUIRE(bm.bm_rbrace[0] == 0);
	REQUIRE(bm.bm_rbrace[1] == 0);
#endif

}