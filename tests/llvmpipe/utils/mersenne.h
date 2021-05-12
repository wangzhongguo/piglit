/*
 * Copyright © 2012-2021 VMware, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef MERSENNE_H
#define MERSENNE_H

#include <string>
#include <stdio.h>
#include <stdint.h>

#include "random.h"

class Mersenne : public IRandom
{
public:
   static const uint32_t n = 624;
   static const uint32_t m = 397;
   static const uint32_t rand_max = 0xFFFFFFFF;

public:
   Mersenne()
   {
   }

   Mersenne(unsigned int seed)
   {
      init(seed);
   }

   virtual void setState(const std::string& state){
      uint32_t seed, index;
      sscanf(state.c_str(), "%08x%03d", &seed, &index);

      init(seed);
      mIndex = index;
   }

   virtual std::string state()
   {
      char buffer[32];
      sprintf(buffer, "%08x%03d", mSeed, mIndex);
      return buffer;
   }

   virtual unsigned int max()
   {
      return 0xFFFFFFFF;
   }

   virtual unsigned int value()
   {
      uint32_t x = mState[mIndex++];
      x ^= (x >> 11);
      x ^= (x << 7) & 0x9D2C5680UL;
      x ^= (x << 15) & 0xEFC60000UL;
      x ^= (x >> 18);

      if (mIndex == n){
         init(x);
         return value();
      }

      return x;
   }

   void init(uint32_t seed)
   {
      mIndex = 0;
      mSeed = seed;
      mState[0] = seed;

      for (uint32_t i = 1; i < n; ++i)
         mState[i] = 1812433253 * (mState[i - 1] ^ (mState[i - 1] >> 30)) + i;

      for (uint32_t i = 0; i < (n - m); ++i)
         mState[i] = mState[i + m] ^ twist(mState[i], mState[i + 1]);

      for (uint32_t i = n - m; i < (n - 1); ++i)
         mState[i] = mState[i + m - n] ^ twist(mState[i], mState[i + 1]);

      mState[n - 1] = mState[m - 1] ^ twist(mState[n - 1], mState[0]);
   }

   inline uint32_t twist(uint32_t u, uint32_t v)
   {
      return (((u & 0x80000000) | (v & 0x7FFFFFFF)) >> 1) ^ ((v & 1) ? 0x9908B0DF : 0);
   }

private:
   uint32_t mSeed;
   uint32_t mIndex;
   uint32_t mState[n];
};

#endif // MERSENNE_H
