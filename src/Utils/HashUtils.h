#pragma once

class HashUtils
{
public:
   // Combine two hash values
   template<typename T>
   static void HashCombine(size_t &seed, const T &value)
   {
      seed ^= std::hash<T>{}(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
   }

   // Combine multiple fields
   template<typename... Args>
   static size_t HashFields(const Args &... args)
   {
      size_t seed = 0;
      (HashCombine(seed, args), ...);
      return seed;
   }

   // Hash a raw buffer (optionally sampled to reduce cost)
   static size_t HashBuffer(const uint8_t *data, size_t size, size_t maxSamples = 64)
   {
      if (!data || size == 0) return 0;

      size_t seed = 0;
      size_t step = size / maxSamples + 1;
      for (size_t i = 0; i < size; i += step)
      {
         HashCombine(seed, data[i]);
      }
      return seed;
   }
};
