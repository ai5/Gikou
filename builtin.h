
#pragma once
#ifndef BUILTIN_H_
#define BUILTIN_H_

// vcでのbuiltin関数

#if defined(_MSC_VER)
#define LODWORD(_qw)    ((unsigned int)(_qw))
#define HIDWORD(_qw)    ((unsigned int)(((_qw) >> 32) & 0xffffffff))

inline unsigned int __builtin_ctz(unsigned int x) { unsigned long r; _BitScanForward(&r, x); return r; }
inline unsigned int __builtin_clz(unsigned int x) { unsigned long r; _BitScanReverse(&r, x); return 31 - r; }
inline unsigned int __builtin_popcount(unsigned int x){ return __popcnt(x); }

#if defined(_WIN64)
inline unsigned long long __builtin_ctzll(unsigned long long x) { unsigned long r; _BitScanForward64(&r, x); return r; }
inline unsigned long long __builtin_clzll(unsigned long long x) { unsigned long r; _BitScanReverse64(&r, x); return 63 - r; }
inline unsigned long long __builtin_popcountll(unsigned long long x) { return __popcnt64(x); }
#else
inline unsigned long long __builtin_ctzll(unsigned long long x) { return LODWORD(x) ? __builtin_ctz(LODWORD(x)) : __builtin_ctz(HIDWORD(x)) + 32; }
inline unsigned long long __builtin_clzll(unsigned long long x) { return HIDWORD(x) ? __builtin_clz(HIDWORD(x)) : __builtin_clz(LODWORD(x)) + 32; }
inline unsigned long long __builtin_popcountll(unsigned long long x) { return __builtin_popcount(LODWORD(x)) + __builtin_popcount(HIDWORD(x)); }
#endif

inline void __builtin_prefetch(const void *addr, ...)
{
	_mm_prefetch((const char*)addr, _MM_HINT_T0);
}
#endif


#endif //BUILTIN_H_
