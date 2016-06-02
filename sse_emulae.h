
#pragma once
#ifndef SSE_EMULATE_H_
#define SSE_EMULATE_H_

// SSE3以降の命令をエミュレーション
// SSEPlusから必要なものをコピペしたもの
// http://developer.amd.com/tools-and-sdks/archive/legacy-cpu-gpu-tools/sseplus-project/

typedef union
{
	__m128i m128i;
	__m64	m64[2];
	uint64_t u64[2];
	int64_t  s64[2];
	uint32_t u32[4];
	int32_t  s32[4];
	uint16_t u16[8];
	int16_t  s16[8];
	uint8_t  u8[16];
	int8_t   s8[16];
} sse_m128;

#ifdef _MSC_VER
#include <emmintrin.h> // SSE2.0
#ifdef _WIN64
__inline __m128i _mm_movpi64_epi64(__m64 __A)
{
	return _mm_set_epi64x(0LL, __A.m64_i64);
}
#endif
#endif // MSC_VER

#ifdef USE_SSE42

#ifndef _WIN64
 #ifdef _MSC_VER
  #define _mm_extract_epi64 _MM_EXTRACT_EPI64
 #else
  #define _mm_extract_epi64 _MM_EXTRACT_EPI64
  #define _mm_insert_epi64 _MM_INSERT_EPI64
 #endif
#endif

#else
#define _mm_testc_si128 _MM_TESTC_SI128
#define _mm_testz_si128 _MM_TESTZ_SI128
#define _mm_extract_epi64 _MM_EXTRACT_EPI64
#define _mm_insert_epi64 _MM_INSERT_EPI64
#define _mm_cvtepu8_epi16 _MM_CVTEPU8_EPI16
#define _mm_min_epu16 _MM_MIN_EPU16
#endif

#ifdef _mm_testc_si128
__inline int _MM_TESTZ_SI128(__m128i a, __m128i b)
{
	sse_m128 t;
	t.m128i = _mm_and_si128(a, b);
	t.m128i = _mm_packs_epi32(t.m128i, _mm_setzero_si128());
	return t.u64[0] == 0;
}
#endif

#ifdef _mm_testz_si128
__inline int _MM_TESTC_SI128(__m128i a, __m128i b)
{
	a = _mm_xor_si128(a, b);
	return _MM_TESTZ_SI128(a, a);
}
#endif

#ifdef _mm_extract_epi64
__inline __int64 _MM_EXTRACT_EPI64(__m128i a, const int ndx)
{
	sse_m128 mask;
	switch (ndx & 0x1)
	{
	case 1:  a = _mm_srli_si128(a, 8); break;
	}

	mask.m128i = _mm_set_epi32(0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF);
	mask.m128i = _mm_and_si128(mask.m128i, a);

	return mask.u64[0];
}
#endif

#ifdef _mm_insert_epi64
__inline __m128i _MM_INSERT_EPI64(__m128i a, __int64 b, const int ndx)
{
	sse_m128 A;
	A.m128i = a;

	A.u64[ndx & 0x1] = b;
	return A.m128i;
}
#endif

#ifdef _mm_cvtepu8_epi16
__inline __m128i _MM_CVTEPU8_EPI16(__m128i a)
{
	__m128i b = _mm_setzero_si128();

	return _mm_unpacklo_epi8(a, b);
}
#endif

#ifdef _mm_min_epu16
#define MIN( sd, s) (sd<s)?sd:s;
__inline __m128i _MM_MIN_EPU16(__m128i a, __m128i b)
{
	sse_m128 temp;
	sse_m128 A;
	sse_m128 B;
	
	A.m128i = a;
	B.m128i = b;

	temp.u16[0] = MIN(A.u16[0], B.u16[0]);
	temp.u16[1] = MIN(A.u16[1], B.u16[1]);
	temp.u16[2] = MIN(A.u16[2], B.u16[2]);
	temp.u16[3] = MIN(A.u16[3], B.u16[3]);
	temp.u16[4] = MIN(A.u16[4], B.u16[4]);
	temp.u16[5] = MIN(A.u16[5], B.u16[5]);
	temp.u16[6] = MIN(A.u16[6], B.u16[6]);
	temp.u16[7] = MIN(A.u16[7], B.u16[7]);

	return temp.m128i;
}
#endif


#endif //BUILTIN_H_

