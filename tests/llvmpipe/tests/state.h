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
#include "../utils/colour.h"
#include "../utils/sampler.h"
#include "../glsl/variable.h"

#include <string>
#include <vector>
#include <iostream>

//Windows...
#ifdef DrawState
#undef DrawState
#endif

namespace glsl
{
   class Program;
};

typedef Sampler<
   glEnum< GL_ZERO,
   glEnum< GL_ONE,
   glEnum< GL_SRC_COLOR,
   glEnum< GL_ONE_MINUS_SRC_COLOR,
   glEnum< GL_DST_COLOR,
   glEnum< GL_ONE_MINUS_DST_COLOR,
   glEnum< GL_SRC_ALPHA,
   glEnum< GL_ONE_MINUS_SRC_ALPHA,
   glEnum< GL_DST_ALPHA,
   glEnum< GL_ONE_MINUS_DST_ALPHA,
   glEnum< GL_CONSTANT_COLOR,
   glEnum< GL_ONE_MINUS_CONSTANT_COLOR,
   glEnum< GL_CONSTANT_ALPHA,
   glEnum< GL_ONE_MINUS_CONSTANT_ALPHA,
   glEnum< GL_SRC_ALPHA_SATURATE
> > > > > > > > > > > > > > > >
SamplerBlendSrcFactor;

typedef Sampler<
   glEnum< GL_ZERO,
   glEnum< GL_ONE,
   glEnum< GL_SRC_COLOR,
   glEnum< GL_ONE_MINUS_SRC_COLOR,
   glEnum< GL_DST_COLOR,
   glEnum< GL_ONE_MINUS_DST_COLOR,
   glEnum< GL_SRC_ALPHA,
   glEnum< GL_ONE_MINUS_SRC_ALPHA,
   glEnum< GL_DST_ALPHA,
   glEnum< GL_ONE_MINUS_DST_ALPHA,
   glEnum< GL_CONSTANT_COLOR,
   glEnum< GL_ONE_MINUS_CONSTANT_COLOR,
   glEnum< GL_CONSTANT_ALPHA,
   glEnum< GL_ONE_MINUS_CONSTANT_ALPHA
> > > > > > > > > > > > > > >
SamplerBlendDstFactor;

typedef Sampler<
   glEnum< GL_FUNC_ADD,
   glEnum< GL_FUNC_SUBTRACT,
   glEnum< GL_FUNC_REVERSE_SUBTRACT,
   glEnum< GL_MIN,
   glEnum< GL_MAX
> > > > >
> SamplerBlendEquation;

typedef Sampler<
   glEnum< GL_NEVER,
   glEnum< GL_LESS,
   glEnum< GL_EQUAL,
   glEnum< GL_LEQUAL,
   glEnum< GL_GREATER,
   glEnum< GL_NOTEQUAL,
   glEnum< GL_GEQUAL,
   glEnum< GL_ALWAYS
> > > > > > > >
> SamplerAlphaFunction;

typedef Sampler< Colour > SamplerColour;
typedef Sampler< GLboolean[4] > SamplerMask;
typedef Sampler< SamplerRange< Sampleri<1>, Sampleri<256> > > SamplerDrawSize;
typedef Sampler< SamplerRange< Samplerfr<0, 1>, Samplerfr<1, 1> > > SamplerClampf;

std::ostream& operator<<(std::ostream& out, const Colour& value);
std::ostream& operator<<(std::ostream& out, const GLboolean& value);
std::ostream& operator<<(std::ostream& out, const SamplerMask& value);
std::ostream& operator<<(std::ostream& out, const glsl::Value& value);
std::ostream& operator<<(std::ostream& out, const SamplerColour& value);
std::ostream& operator<<(std::ostream& out, const SamplerClampf& value);
std::ostream& operator<<(std::ostream& out, const glsl::Variable& value);
std::ostream& operator<<(std::ostream& out, const SamplerDrawSize& value);
std::ostream& operator<<(std::ostream& out, const SamplerAlphaFunction& value);
std::ostream& operator<<(std::ostream& out, const SamplerBlendEquation& value);
std::ostream& operator<<(std::ostream& out, const SamplerBlendSrcFactor& value);
std::ostream& operator<<(std::ostream& out, const SamplerBlendDstFactor& value);

struct TestState
{
   TestState()
      : chance(80), enabled(false)
   {
   }

   virtual void dump() = 0;

   virtual void randomise()
   {
      enabled = gRandom.percent() < chance;
   }

   unsigned int chance;
   bool enabled;
};

struct BlendState : public TestState
{
   BlendState();

   virtual void dump();
   virtual void randomise();

   SamplerBlendSrcFactor rgbSrcFactor;
   SamplerBlendDstFactor rgbDstFactor;

   SamplerBlendSrcFactor aSrcFactor;
   SamplerBlendDstFactor aDstFactor;

   SamplerBlendEquation equation;

   SamplerColour colour;

   bool clamp;
};

struct AlphaState : public TestState
{
   AlphaState();

   virtual void dump();
   virtual void randomise();

   SamplerAlphaFunction function;
   SamplerClampf reference;
};

struct TextureState : public TestState
{
   TextureState();
   ~TextureState();

   virtual void dump();
   virtual void randomise();

   int width;
   int height;

   GLuint texture;

   Colour* image;
   unsigned char* bgra_u8;
};

struct MaskState : public TestState
{
   MaskState();

   virtual void dump();
   virtual void randomise();

   SamplerMask mask;
};

struct ShaderState : public TestState
{
   ShaderState();
   ~ShaderState();

   virtual void dump();
   virtual void randomise();

   glsl::Program* program;
};

struct DrawState : public TestState
{
   enum Style {
      Gradient,
      Solid
   };

   enum Direction {
      Horizontal,
      Vertical
   };

   class Randomiser {
   public:
      Style random(Style min, Style max)
      {
         return (Style)gRandom.random(0, 2);
      }

      Direction random(Direction min, Direction max)
      {
         return (Direction)gRandom.random(0, 2);
      }
   };

   static Randomiser mRandomiser;

   typedef Sampler< SamplerRange< Samplert<Style, Gradient>, Samplert<Style, Solid>, Randomiser, &mRandomiser > > SamplerDrawStyle;
   typedef Sampler< SamplerRange< Samplert<Direction, Horizontal>, Samplert<Direction, Vertical>, Randomiser, &mRandomiser > > SamplerDrawDirection;

   DrawState();

   virtual void dump();
   virtual void randomise();

   int x;
   int y;

   SamplerDrawSize width;
   SamplerDrawSize height;

   SamplerColour clear;
   SamplerColour colour;

   SamplerColour gradient1;
   SamplerColour gradient2;

   SamplerDrawStyle style;
   SamplerDrawDirection direction;
};
