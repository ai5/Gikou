
#pragma once
#ifndef SSE_EMULATE_H_
#define SSE_EMULATE_H_

// SSE3以降の命令をエミュレーション
// SSEPlusから必要なものをコピペしたもの
// http://developer.amd.com/tools-and-sdks/archive/legacy-cpu-gpu-tools/sseplus-project/

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
#define _mm_extract_epi64 _MM_EXTRACT_EPI64
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
	__m128i t;
	t = _mm_and_si128(a, b);
	t = _mm_packs_epi32(t, _mm_setzero_si128());
	return t.m128i_u64[0] == 0;
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
	__m128i mask;
	switch (ndx & 0x1)
	{
	case 1:  a = _mm_srli_si128(a, 8); break;
	}

	mask = _mm_set_epi32(0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF);
	mask = _mm_and_si128(mask, a);

	return mask.m128i_u64[0];
}
#endif

#ifdef _mm_insert_epi64
__inline __m128i _MM_INSERT_EPI64(__m128i a, __int64 b, const int ndx)
{
	__m128i A;
	A = a;

	A.m128i_u64[ndx & 0x1] = b;
	return A;
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
	__m128i temp;

	temp.m128i_u16[0] = MIN(a.m128i_u16[0], b.m128i_u16[0]);
	temp.m128i_u16[1] = MIN(a.m128i_u16[1], b.m128i_u16[1]);
	temp.m128i_u16[2] = MIN(a.m128i_u16[2], b.m128i_u16[2]);
	temp.m128i_u16[3] = MIN(a.m128i_u16[3], b.m128i_u16[3]);
	temp.m128i_u16[4] = MIN(a.m128i_u16[4], b.m128i_u16[4]);
	temp.m128i_u16[5] = MIN(a.m128i_u16[5], b.m128i_u16[5]);
	temp.m128i_u16[6] = MIN(a.m128i_u16[6], b.m128i_u16[6]);
	temp.m128i_u16[7] = MIN(a.m128i_u16[7], b.m128i_u16[7]);

	return temp;
}
#endif


#endif //BUILTIN_H_

