
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
#endif

#endif //BUILTIN_H_

