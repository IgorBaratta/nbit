#ifdef _MSC_VER

#include <intrin.h>

#define __builtin_clzll __lzcnt64

static inline int __builtin_popcountll(unsigned long long x)
{
    return static_cast<int>(__popcnt64(x));
}

static inline int __builtin_clzll(unsigned long long x)
{
    return static_cast<int>(__lzcnt64(x));
}

static inline int __builtin_ctzll(unsigned long long x)
{
    unsigned long index;
    _BitScanForward64(&ret, x);
    return static_cast<int> index;
}

#endif