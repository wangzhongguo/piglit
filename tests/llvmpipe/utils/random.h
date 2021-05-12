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

#include "colour.h"
#include "piglit-util-gl.h"

class IRandom
{
public:
   virtual ~IRandom(){}

   virtual unsigned int max() = 0;
   virtual unsigned int value() = 0;

   virtual std::string state() = 0;
   virtual void setState(const std::string& state) = 0;
};

class Random
{
public:
   Random();
   Random(IRandom* impl);
   ~Random();

   std::string state();

   void setImplementation(IRandom* impl);
   void setState(const std::string& state);

   template<typename T> T random();
   template<typename T> T random(T max);
   template<typename T> T random(T min, T max);

   unsigned int percent();

   template<typename T>
   std::vector<T> shuffle(const std::vector<T>& in)
   {
      std::vector<T> clone = in;
      std::vector<T> result;

      while(clone.size()){
         unsigned int idx = randomUInt() % clone.size();
         result.push_back(clone[idx]);
         clone.erase(clone.begin() + idx);
      }

      return result;
   }

private:
   unsigned int randomUInt();

private:
   IRandom* mRandom;
};

template<> int Random::random<int>();
template<> int Random::random<int>(int max);
template<> int Random::random<int>(int min, int max);

template<> unsigned int Random::random<unsigned int>();
template<> unsigned int Random::random<unsigned int>(unsigned int max);
template<> unsigned int Random::random<unsigned int>(unsigned int min, unsigned int max);

template<> float Random::random<float>();
template<> float Random::random<float>(float max);
template<> float Random::random<float>(float min, float max);

template<> Colour Random::random<Colour>();
template<> GLboolean Random::random<GLboolean>();

extern Random gRandom;
