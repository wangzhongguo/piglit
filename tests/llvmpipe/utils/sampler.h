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

#include "piglit-util-gl.h"
#include "colour.h"
#include "mersenne.h"

/* Template value containers */
template<typename T, T v>
class Samplert
{
public:
   typedef T type;

   static T value()
   {
      return v;
   }
};

template<float& v>
class Samplerf
{
public:
   typedef float type;

   static float value()
   {
      return v;
   }
};

template<int v>
class Sampleri
{
public:
   typedef int type;

   static int value()
   {
      return v;
   }
};

template<int top, int bot>
class Samplerfr
{
public:
   typedef float type;

   static float value()
   {
      return top / (float)bot;
   }
};

/* Value range container */
template<class min, class max, class RandClass = Random, RandClass* ptr = &gRandom>
class SamplerRange
{
public:
   static typename min::type randomise()
   {
      return ptr->random(min::value(), max::value());
   }
};

/* Empty sampler class for specialisation */
template<typename T>
class Sampler
{
};

/* Range sampler */
template<class min, class max, class RandClass, RandClass* ptr>
class Sampler< SamplerRange<min, max, RandClass, ptr> >
{
public:
   Sampler()
   {
      randomise();
   }

   operator typename min::type()
   {
      return mValue;
   }

   operator typename min::type() const
   {
      return mValue;
   }

   typename min::type value() const
   {
      return mValue;
   }

   Sampler< SamplerRange<min, max, RandClass, ptr> >& operator=(typename min::type value)
   {
      mValue = value;
      return *this;
   }

   typename min::type randomise()
   {
      return mValue = SamplerRange<min, max, RandClass, ptr>::randomise();
   }

private:
   typename min::type mValue;
};

/* Colour mask sampler */
template<>
class Sampler< GLboolean[4] >
{
public:
   Sampler()
   {
      randomise();
   }

   const GLboolean* value() const
   {
      return mValue;
   }

   operator GLboolean*()
   {
      return mValue;
   }

   operator const GLboolean*() const
   {
      return mValue;
   }

   GLboolean* randomise()
   {
      for (int i = 0; i < 4; ++i)
         mValue[i] = gRandom.random<GLboolean>();

      return mValue;
   }

private:
   GLboolean mValue[4];
};

/* GLcolour sampler */
template<>
class Sampler< Colour >
{
public:
   Sampler()
   {
      randomise();
   }

   const Colour& value() const
   {
      return mValue;
   }

   operator Colour()
   {
      return mValue;
   }

   operator Colour&()
   {
      return mValue;
   }

   float& operator[](int i)
   {
      return mValue[i];
   }

   operator const Colour&() const
   {
      return mValue;
   }

   operator const GLfloat*() const
   {
      return mValue;
   }

   float operator[](int i) const
   {
      return mValue[i];
   }

   Sampler< Colour >& operator=(const Colour& value)
   {
      mValue = value;
      return *this;
   }

   Colour randomise()
   {
      mValue = gRandom.random<Colour>();
      return mValue;
   }

private:
   Colour mValue;
};

/* Used to signifiy end of glEnum chain */
class glEnumEnd
{
public:
   const static size_t size = 0;

   static GLenum at(int i)
   {
      assert(0);
      return -1;
   }

private:
};

/* Used to create template array of GLenum values */
template<GLenum val, class next = glEnumEnd>
class glEnum
{
public:
   const static size_t size = 1 + next::size;

   static GLenum at(int i)
   {
      if (i == 0)
         return val;
      else
         return next::at(i - 1);
   }

private:
};

/* Sampler to randomly pick from array of GLenum */
template<GLenum val, class next>
class Sampler< glEnum<val, next> >
{
public:
   const static size_t size = glEnum<val, next>::size;

public:
   Sampler()
   {
      randomise();
   }

   operator GLenum()
   {
      return mValue;
   }

   operator GLenum() const
   {
      return mValue;
   }

   Sampler< glEnum<val, next> >& operator=(GLenum value)
   {
      mValue = value;
      return *this;
   }

   GLenum randomise()
   {
      return mValue = at(gRandom.random<int>(size));
   }

   static GLenum at(size_t i)
   {
      return glEnum<val, next>::at(i);
   }

private:
   GLenum mValue;
};
