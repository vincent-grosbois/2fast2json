#ifndef __TFTJ_BITS__
#define __TFTJ_BITS__

#include "config.h" 

namespace tftj
{
	int popcount(word_t w)
	{
		return static_cast<int>(TFTJ_POPCNT(w));
	}

	int leading_zeros(word_t w)
	{
		return static_cast<int>(TFTJ_LZCNT(w));
	}

	word_t remove(word_t a)
	{
		return a & (a - 1);
	}

	word_t extract(word_t a)
	{
#ifdef TFTJ_MSVC
#pragma warning(push)
#pragma warning(disable:4146) //remove warning C4146: unary minus operator applied to unsigned type
#endif
		return a & -a;
#ifdef TFTJ_MSVC
#pragma warning(pop)
#endif
	}

	word_t smear(word_t a)
	{
		return a ^ (a - 1);
	}

}

/*
enum encoding_type
{
	ASCII,
	UTF8
};

template<encoding_type e>
struct encoding_traits
{ };

template<>
struct encoding_traits<ASCII>
{
	typedef ::std::string string_type;

};*/


#endif