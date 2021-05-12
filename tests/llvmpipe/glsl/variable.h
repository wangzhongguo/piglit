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

#include "value.h"

namespace glsl
{
   class Variable
   {
   public:
      enum Type {
         BuiltIn,
         Attribute,
         Local,
         Varying,
         Uniform,
         UniformSampler
      };

   public:
      Variable()
         : mLocation(0)
      {
      }

      Variable(const std::string& name, Type type, const Value& value)
         : mType(type), mName(name), mValue(value), mLocation(0)
      {
      }

      const std::string& name() const
      {
         return mName;
      }

      std::string toGLSL() const
      {
         if (mType == UniformSampler) {
            return std::string("uniform sampler2D ") + mName;
         } else if (mType == Uniform) {
            return std::string("uniform ") + mValue.toTypeString() + " " + mName;
         } else if (mType == Varying) {
            return std::string("varying ") + mValue.toTypeString() + " " + mName;
         } else if (mType == Attribute) {
            return std::string("attribute ") + mValue.toTypeString() + " " + mName;
         } else {
            return mValue.toTypeString() + " " + mName;
         }
      }

      Type type() const
      {
         return mType;
      }

      unsigned int location()
      {
         return mLocation;
      }

      void setType(Type type)
      {
         mType = type;
      }

      void setLocation(unsigned int location)
      {
         mLocation = location;
      }

      bool declare() const
      {
         return isAttribute() || isVarying() || isUniform();
      }

      bool isAttribute() const
      {
         return mType == Attribute;
      }

      bool isUniform() const
      {
         return mType == Uniform || mType == UniformSampler;
      }

      bool isVarying() const
      {
         return mType == Varying;
      }

      Value value() const
      {
         return mValue;
      }

      template<typename T>
      T* valuePtr() const
      {
         return mValue.valuePtr<T>();
      }

      template<typename T>
      T& value() const
      {
         return mValue.value<T>();
      }

      Variable& operator=(const Value& rhs)
      {
         mValue = rhs;
         return *this;
      }

   private:
      Type mType;
      std::string mName;
      Value mValue;
      unsigned int mLocation;
   };
}
