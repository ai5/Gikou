
#pragma once
#ifndef BUILTIN_H_
#define BUILTIN_H_

// vcでのbuiltin関数

#if defined(_MSC_VER)
#define LODWORD(_qw)    ((unsigned int)(_qw))
#define HIDWORD(_qw)    ((unsigned int)(((_qw) >> 32) & 0xffffffff))

inline int __builtin_ctz(unsigned int x) { unsigned long r; _BitScanForward(&r, x); return r; }
inline int __builtin_clz(unsigned int x) { unsigned long r; _BitScanReverse(&r, x); return 31 - r; }

#if USE_SSE42
inline int __builtin_popcount(unsigned int x){ return __popcnt(x); }
#else
inline int __builtin_popcount(unsigned int a) {
	// やねうら王で見たコード
	a = (a & UINT32_C(0x55555555)) + (a >> 1 & UINT32_C(0x55555555));
	a = (a & UINT32_C(0x33333333)) + (a >> 2 & UINT32_C(0x33333333));
	a = (a & UINT32_C(0x0f0f0f0f)) + (a >> 4 & UINT32_C(0x0f0f0f0f));
	a = (a & UINT32_C(0x00ff00ff)) + (a >> 8 & UINT32_C(0x00ff00ff));
	a = (a & UINT32_C(0x0000ffff)) + (a >> 16 & UINT32_C(0x0000ffff));
	return (int32_t)a;
}

#endif

#if defined(_WIN64)
inline int __builtin_ctzll(unsigned long long x) { unsigned long r; _BitScanForward64(&r, x); return r; }
inline int __builtin_clzll(unsigned long long x) { unsigned long r; _BitScanReverse64(&r, x); return 63 - r; }
#if USE_SSE42
inline int __builtin_popcountll(unsigned long long x) { return __popcnt64(x); }
#else
inline int __builtin_popcountll(unsigned long long a) { 
	a = (a & UINT64_C(0x5555555555555555)) + (a >> 1 & UINT64_C(0x5555555555555555));
	a = (a & UINT64_C(0x3333333333333333)) + (a >> 2 & UINT64_C(0x3333333333333333));
	a = (a & UINT64_C(0x0f0f0f0f0f0f0f0f)) + (a >> 4 & UINT64_C(0x0f0f0f0f0f0f0f0f));
	a = (a & UINT64_C(0x00ff00ff00ff00ff)) + (a >> 8 & UINT64_C(0x00ff00ff00ff00ff));
	a = (a & UINT64_C(0x0000ffff0000ffff)) + (a >> 16 & UINT64_C(0x0000ffff0000ffff));
	return (int32_t)a + (int32_t)(a >> 32);
}

#endif
#else
inline int __builtin_ctzll(unsigned long long x) { return LODWORD(x) ? __builtin_ctz(LODWORD(x)) : __builtin_ctz(HIDWORD(x)) + 32; }
inline int __builtin_clzll(unsigned long long x) { return HIDWORD(x) ? __builtin_clz(HIDWORD(x)) : __builtin_clz(LODWORD(x)) + 32; }
inline int __builtin_popcountll(unsigned long long x) { return __builtin_popcount(LODWORD(x)) + __builtin_popcount(HIDWORD(x)); }
#endif

inline void __builtin_prefetch(const void *addr, ...)
{
	_mm_prefetch((const char*)addr, _MM_HINT_T0);
}
#endif


#endif //BUILTIN_H_
