#pragma once
#include <cmath>
#include <xmmintrin.h>

namespace ReyEngine::Math{
   template <typename T>
   T clamp(T min, T max, T value){
      if (value < min) return min;
      if (value > max) return max;
      return value;
   }

   //fast sqrt using intrinsics
   inline double fsqrt(float v){
   #ifdef __x86_64__
      // Initialize a float value
      float value = 4.0f;
      // Load the float into an SSE register
      __m128 input = _mm_set_ss(value);
      __m128 result = _mm_rsqrt_ss(input);
      float rsqrt;
      _mm_store_ss(&rsqrt, result);
      return 1.0f/rsqrt;
   #elif __arm__
      //not implemented, fall back to regular squart
      return std::sqrt(v);
   #endif
   }


}