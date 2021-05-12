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

#pragma once

#include <string.h>
#include <algorithm>

class Colour
{
public:
   static Colour White;
   static Colour Black;
   static Colour Zero;
   static Colour Red;
   static Colour Green;
   static Colour Blue;

public:
   Colour()
   {
      memcpy(mColour, White, sizeof(float) * 4);
   }

   Colour(const float* src)
   {
      setColour(src);
   }

   Colour(float r, float g, float b, float a = 1.0f)
   {
      setColour(r, g, b, a);
   }

   void setColour(const float* src)
   {
      memcpy(mColour, src, sizeof(float) * 4);
   }

   void setColour(float r, float g, float b, float a)
   {
      mColour[0] = r;
      mColour[1] = g;
      mColour[2] = b;
      mColour[3] = a;
   }

   void setAlpha(float a)
   {
      mColour[3] = a;
   }

   void clampByte()
   {
      for (int i = 0; i < 4; ++i) {
         mColour[i] = ((int)(mColour[i] * 255.0f)) / (float)255.0f;
      }
   }

   void clamp(float min, float max)
   {
      for (int i = 0; i < 4; ++i) {
         if (mColour[i] < min)
            mColour[i] = min;

         if (mColour[i] > max)
            mColour[i] = max;
      }
   }

   void min(float min)
   {
      for (int i = 0; i < 4; ++i) {
         if (mColour[i] < min)
            mColour[i] = min;
      }
   }

   void max(float max)
   {
      for (int i = 0; i < 4; ++i) {
         if (mColour[i] > max)
            mColour[i] = max;
      }
   }

   float red() const
   {
      return mColour[0];
   }

   float green() const
   {
      return mColour[1];
   }

   float blue() const
   {
      return mColour[2];
   }

   float alpha() const
   {
      return mColour[3];
   }

   inline float& operator[](int i)
   {
      return mColour[i];
   }

   inline float operator[](int i) const
   {
      return mColour[i];
   }

   inline operator float*()
   {
      return mColour;
   }

   inline operator const float*() const
   {
      return mColour;
   }

   static Colour min(const Colour& c1, const Colour& c2)
   {
      return Colour(std::min(c1.mColour[0], c2.mColour[0]),
                    std::min(c1.mColour[1], c2.mColour[1]),
                    std::min(c1.mColour[2], c2.mColour[2]),
                    std::min(c1.mColour[3], c2.mColour[3]));
   }

   static Colour max(const Colour& c1, const Colour& c2)
   {
      return Colour(std::max(c1.mColour[0], c2.mColour[0]),
                    std::max(c1.mColour[1], c2.mColour[1]),
                    std::max(c1.mColour[2], c2.mColour[2]),
                    std::max(c1.mColour[3], c2.mColour[3]));
   }

   inline Colour& operator =(const Colour& o)
   {
      if(this != &o)
         setColour(o.mColour);

      return *this;
   }

   inline Colour& operator =(const Colour* o)
   {
      if(this != o)
         setColour(o->mColour);

      return *this;
   }

   inline Colour& operator =(const float*& src)
   {
      if(mColour != src)
         setColour(src);

      return *this;
   }

   inline Colour& operator *=(const float& f)
   {
      for (int i = 0; i < 4; ++i)
         mColour[i] *= f;

      return *this;
   }

   inline Colour& operator *=(const Colour& src)
   {
      for (int i = 0; i < 4; ++i)
         mColour[i] *= src[i];

      return *this;
   }

   inline Colour& operator *=(const float*& src)
   {
      for (int i = 0; i < 4; ++i)
         mColour[i] *= src[i];

      return *this;
   }

   inline Colour operator *(const float& f) const
   {
      return Colour(mColour[0] * f, mColour[1] * f, mColour[2] * f, mColour[3] * f);
   }

   inline Colour operator *(const Colour& f) const
   {
      return Colour(mColour[0] * f[0], mColour[1] * f[1], mColour[2] * f[2], mColour[3] * f[3]);
   }

   inline Colour operator /(const float& f) const
   {
      return Colour(mColour[0] / f, mColour[1] / f, mColour[2] / f, mColour[3] / f);
   }

   inline Colour operator /(const Colour& f) const
   {
      return Colour(mColour[0] / f[0], mColour[1] / f[1], mColour[2] / f[2], mColour[3] / f[3]);
   }

   inline Colour operator -(const Colour& b) const
   {
      return Colour(mColour[0] - b[0], mColour[1] - b[1], mColour[2] - b[2], mColour[3] - b[3]);
   }

   inline Colour operator +(const Colour& b) const
   {
      return Colour(mColour[0] + b[0], mColour[1] + b[1], mColour[2] + b[2], mColour[3] + b[3]);
   }

private:
   float mColour[4];
};
