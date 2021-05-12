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

#include <string>
#include <vector>
#include <sstream>

#include "../utils/colour.h"

namespace glsl
{
   class Value
   {
   public:
      static const Value none;
      static const Value vec2;
      static const Value vec3;
      static const Value vec4;

   public:
      Value()
         : mSize(0)
      {
      }

      Value(int components, float value)
         : mSize(0)
      {
         for (int i = 0; i < components; ++i)
            add(value);
      }

      Value(float x)
         : mSize(0)
      {
         add(x);
      }

      Value(float x, float y)
         : mSize(0)
      {
         add(x);
         add(y);
      }

      Value(float x, float y, float z)
         : mSize(0)
      {
         add(x);
         add(y);
         add(z);
      }

      Value(float x, float y, float z, float w)
         : mSize(0)
      {
         add(x);
         add(y);
         add(z);
         add(w);
      }

      Value(const Colour& colour)
         : mSize(4)
      {
         memcpy(mValue, (const float*)colour, sizeof(float) * 4);
      }

      Value(const Value& other)
         : mSize(other.mSize)
      {
         memcpy(mValue, other.mValue, sizeof(float) * 4);
      }

      template<typename T>
      static Value create(const T& val)
      {
         Value value;
         *(value.valuePtr<T>()) = val;
         return value;
      }

      float& x()
      {
         assert(mSize >= 1);
         return mValue[0];
      }

      float& y()
      {
         assert(mSize >= 2);
         return mValue[1];
      }

      float& z()
      {
         assert(mSize >= 3);
         return mValue[2];
      }

      float& w()
      {
         assert(mSize == 4);
         return mValue[3];
      }

      float& operator[](int i)
      {
         return mValue[i];
      }

      const float& operator[](int i) const
      {
         return mValue[i];
      }

      operator const GLfloat*() const
      {
         return mValue;
      }

      size_t size() const
      {
         return mSize;
      }

      void add(float value)
      {
         assert(mSize < 4);
         mValue[mSize++] = value;
      }

      template<typename T>
      T* valuePtr() const
      {
         return (T*)&mValue;
      }

      template<typename T>
      T& value() const
      {
         return *(T*)&mValue;
      }

      std::string toTypeString() const
      {
         if (mSize == 1)
            return "float";

         std::string type = "vec";
         type.push_back((char)('0' + mSize));
         return type;
      }

      std::string toString() const
      {
         std::stringstream ss;
         ss.precision(9);

         ss << std::fixed;

         if (mSize > 1)
            ss << "vec" << mSize << "(";

         for (size_t i = 0; i < mSize; ++i) {
            if (i > 0)
               ss << ", ";

            ss << mValue[i];
         }

         if (mSize > 1)
            ss << ")";

         return ss.str();
      }

      bool operator >(const Value& rhs) const
      {
         assert(mSize == rhs.mSize);

         for (size_t i = 0; i < mSize; ++i) {
            if (! (mValue[i] > rhs[i]) )
               return false;
         }

         return true;
      }

      bool operator <(const Value& rhs) const
      {
         assert(mSize == rhs.mSize);

         for (size_t i = 0; i < mSize; ++i) {
            if (! (mValue[i] < rhs[i]) )
               return false;
         }

         return true;
      }

      bool operator >=(const Value& rhs) const
      {
         assert(mSize == rhs.mSize);

         for (size_t i = 0; i < mSize; ++i) {
            if (! (mValue[i] >= rhs[i]) )
               return false;
         }

         return true;
      }

      bool operator <=(const Value& rhs) const
      {
         assert(mSize == rhs.mSize);

         for (size_t i = 0; i < mSize; ++i) {
            if (! (mValue[i] <= rhs[i]) )
               return false;
         }

         return true;
      }

      Value& operator =(const Value& rhs)
      {
         mSize = rhs.mSize;
         memcpy(mValue, rhs.mValue, sizeof(float) * 4);
         return *this;
      }

      Value& operator =(const Colour& rhs)
      {
         mSize = 4;
         memcpy(mValue, (const float*)rhs, sizeof(float) * 4);
         return *this;
      }

      Value& operator +=(const Value& rhs)
      {
         *this = *this + rhs;
         return *this;
      }

      Value& operator -=(const Value& rhs)
      {
         *this = *this- rhs;
         return *this;
      }

      Value& operator *=(const Value& rhs)
      {
         *this = *this * rhs;
         return *this;
      }

      Value& operator /=(const Value& rhs)
      {
         *this = *this / rhs;
         return *this;
      }

      Value operator +(const Value& rhs) const
      {
         assert(mSize == rhs.mSize);

         Value result;
         result.mSize = mSize;

         for (size_t i = 0; i < mSize; ++i)
            result.mValue[i] = mValue[i] + rhs.mValue[i];

         return result;
      }

      Value operator -(const Value& rhs) const
      {
         assert(mSize == rhs.mSize);

         Value result;
         result.mSize = mSize;

         for (size_t i = 0; i < mSize; ++i)
            result.mValue[i] = mValue[i] - rhs.mValue[i];

         return result;
      }

      Value operator *(const Value& rhs) const
      {
         assert(mSize == rhs.mSize || rhs.mSize == 1);

         Value result;
         result.mSize = mSize;

         if (rhs.mSize == 1) {
            for (size_t i = 0; i < rhs.mSize; ++i) {
               result.mValue[i] = mValue[i] * rhs.mValue[0];
            }
         } else {
            for (size_t i = 0; i < mSize; ++i) {
               result.mValue[i] = mValue[i] * rhs.mValue[i];
            }
         }

         return result;
      }

      Value operator /(const Value& rhs) const
      {
         assert(mSize == rhs.mSize || rhs.mSize == 1);

         Value result;
         result.mSize = mSize;

         if (rhs.mSize == 1) {
            for (size_t i = 0; i < rhs.mSize; ++i) {
               result.mValue[i] = mValue[i] / rhs.mValue[0];
            }
         } else {
            for (size_t i = 0; i < mSize; ++i) {
               result.mValue[i] = mValue[i] / rhs.mValue[i];
            }
         }

         return result;
      }

   protected:
      size_t mSize;
      float mValue[4];
   };

   Value vec2(float x, float y);
   Value vec3(float x, float y, float z);
   Value vec4(float x, float y, float z, float w);
}
