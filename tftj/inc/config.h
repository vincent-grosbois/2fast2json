#ifndef __TFTJ_CONFIG__
#define __TFTJ_CONFIG__

#include <cstdint>

//#define TFTJ_NO_INTRINSICS
//#define TFTJ_NO_64BITS


#if _WIN32 || _WIN64
#include <intrin.h>
#define TFTJ_MSVC
#if defined(_WIN64) && !defined(TFTJ_NO_64BITS)
#define TFTJ_ENVIRONMENT64
#else
#define TFTJ_ENVIRONMENT32
#endif
#endif


namespace tftj
{
#ifdef TFTJ_ENVIRONMENT64
	typedef ::std::uint64_t word_t;
	const int word_bits = 64;
#define TFTJ_POPCNT(x) __popcnt64(x)
#define TFTJ_LZCNT(x) __lzcnt64(x)
#endif
#ifdef TFTJ_ENVIRONMENT32
	typedef ::std::uint32_t word_t;
	const int word_bits = 32;
#define TFTJ_POPCNT(x) __popcnt(x)
#define TFTJ_LZCNT(x) __lzcnt(x)
#endif

	struct TFTJException
	{
		const std::string& message;
	public:
		TFTJException(const std::string& message):
			message(message)
		{ }
	};

}

#endif