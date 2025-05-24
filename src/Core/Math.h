#pragma once

namespace Math
{
   // Basic utility functions
   template<typename T>
   constexpr T Min(T a, T b) { return (a < b) ? a : b; }

   template<typename T>
   constexpr T Max(T a, T b) { return (a > b) ? a : b; }

   template<typename T>
   constexpr T Clamp(T v, T lo, T hi)
   {
      return (v < lo) ? lo : (v > hi) ? hi : v;
   }

   template<typename T, typename U>
   constexpr auto Lerp(T a, T b, U t) -> decltype(a + (b - a) * t)
   {
      return a + (b - a) * t;
   }

   template<typename T>
   constexpr T Saturate(T v)
   {
      return Clamp(v, static_cast<T>(0), static_cast<T>(1));
   }

   // Angle conversions
   constexpr float ToRadians(const float degrees)
   {
      return degrees * (PI / 180.0f);
   }

   constexpr float ToDegrees(const float radians)
   {
      return radians * (180.0f / PI);
   }

   // Smoothstep interpolation
   template<typename T>
   constexpr T Smoothstep(T edge0, T edge1, T x)
   {
      T t = Clamp((x - edge0) / (edge1 - edge0), static_cast<T>(0), static_cast<T>(1));
      return t * t * (static_cast<T>(3) - static_cast<T>(2) * t);
   }

   // Sign function
   template<typename T>
   constexpr int Sign(T v)
   {
      return (v > 0) - (v < 0);
   }

   // Exponential smoothing (damped interpolation)
   template<typename T, typename U>
   constexpr auto Damp(T x, T y, U lambda, U dt) -> decltype(x + (y - x) * (static_cast<U>(1) - std::exp(-lambda * dt)))
   {
      return x + (y - x) * (static_cast<U>(1) - std::exp(-lambda * dt));
   }

   // Remap function
   template<typename T>
   constexpr T Remap(T value, T min1, T max1, T min2, T max2)
   {
      return min2 + (max2 - min2) * ((value - min1) / (max1 - min1));
   }
}
