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

#include <vector>
#include "value.h"

namespace glsl
{
   class Selector
   {
   public:
      enum Channels {
         X,
         Y,
         Z,
         W,
         NONE
      };

      static const Selector x;
      static const Selector y;
      static const Selector z;
      static const Selector w;
      static const Selector xy;
      static const Selector xyzw;

   public:
      Selector()
      {
      }

      Selector(const std::vector<Channels>& src)
         : mMask(src)
      {
      }

      Selector(const Value& v)
      {
         for (size_t i = 0; i < v.size(); ++i)
            add((Channels)i);
      }

      Selector(Channels c1)
      {
         add(c1);
      }

      Selector(Channels c1, Channels c2)
      {
         add(c1);
         add(c2);
      }

      Selector(Channels c1, Channels c2, Channels c3)
      {
         add(c1);
         add(c2);
         add(c3);
      }

      Selector(Channels c1, Channels c2, Channels c3, Channels c4)
      {
         add(c1);
         add(c2);
         add(c3);
         add(c4);
      }

      void add(Channels c)
      {
         if (c == NONE)
            return;

         mMask.push_back(c);
      }

      Value select(const Value& in) const
      {
         Value out;
         for (std::vector<Channels>::const_iterator itr = mMask.begin(); itr != mMask.end(); ++itr)
            out.add(in[*itr]);

         return out;
      }

      Value select(const Value& left, const Value& right) const
      {
         assert(right.size() == mMask.size());

         Value out = left;
         for (size_t i = 0; i < mMask.size(); ++i)
            out[mMask.at(i)] = right[i];

         return out;
      }

      std::string toString() const
      {
         static const char* name[] = { "x", "y", "z", "w" };
         std::string str;

         for (size_t i = 0; i < mMask.size(); ++i)
            str += name[mMask.at(i)];

         if (str.compare("xyzw") == 0)
            return std::string();
         else
            return str;
      }

      const std::vector<Channels>& mask() const
      {
         return mMask;
      }

   private:
      std::vector<Channels> mMask;
   };
}
