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

#include "random.h"
#include "mersenne.h"
#include "piglit-ext.h"

Random::Random()
   : mRandom(NULL)
{
}

Random::Random(IRandom* impl)
   : mRandom(impl)
{
}

Random::~Random()
{
}

std::string Random::state()
{
   return mRandom->state();
}

void Random::setImplementation(IRandom* impl)
{
   mRandom = impl;
}

void Random::setState(const std::string& state)
{
   mRandom->setState(state);
}

unsigned int Random::percent()
{
   return random<unsigned int>(100);
}

unsigned int Random::randomUInt()
{
   return random<unsigned int>();
}

template<> int Random::random<int>()
{
   return abs(int(mRandom->value()));
}

template<> int Random::random<int>(int max)
{
   return random<int>() % max;
}

template<> int Random::random<int>(int min, int max)
{
   return min + random<int>(max - min);
}

template<> unsigned int Random::random<unsigned int>()
{
   return mRandom->value();
}

template<> unsigned int Random::random<unsigned int>(unsigned int max)
{
   return random<unsigned int>() % max;
}

template<> unsigned int Random::random<unsigned int>(unsigned int min, unsigned int max)
{
   return min + random<unsigned int>(max - min);
}

template<> float Random::random<float>()
{
   return mRandom->value() / (float)mRandom->max();
}

template<> float Random::random<float>(float max)
{
   return random<float>() * max;
}

template<> float Random::random<float>(float min, float max)
{
   return min + random<float>(max - min);
}

template<> Colour Random::random<Colour>()
{
   return Colour(ubyte_to_float(random<unsigned int>(0, 256)),
                 ubyte_to_float(random<unsigned int>(0, 256)),
                 ubyte_to_float(random<unsigned int>(0, 256)),
                 ubyte_to_float(random<unsigned int>(0, 256)));
}

template<> GLboolean Random::random<GLboolean>()
{
   return random<int>(2);
}
