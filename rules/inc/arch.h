#pragma once

#include <bit>
#include <cstdint>

#if defined(__x86_64__) || defined(_M_X64)
#include <immintrin.h>
#endif

#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define FORCE_INLINE inline __attribute__((always_inline))
#else
#define FORCE_INLINE inline
#endif

FORCE_INLINE int countr_zero(uint64_t b)
{
    return static_cast<int>(std::countr_zero(b));
}

FORCE_INLINE int pop_count(uint64_t b)
{
    return static_cast<int>(std::popcount(b));
}

FORCE_INLINE uint64_t pext_software(uint64_t value, uint64_t mask)
{
    uint64_t result = 0;
    uint64_t result_bit = 1;

    while (mask)
    {
        const uint64_t lowest_bit = mask & (~mask + 1);

        if (value & lowest_bit)
            result |= result_bit;

        mask &= mask - 1;
        result_bit <<= 1;
    }

    return result;
}

FORCE_INLINE uint64_t pext(uint64_t value, uint64_t mask)
{
#if defined(__BMI2__) && (defined(__x86_64__) || defined(_M_X64))
    return _pext_u64(value, mask);
#else
    return pext_software(value, mask);
#endif
}