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

#include "state.h"
#include "../glsl/program.h"
#include "../utils/framebuffer.h"
#include "../utils/glenum.h"

#include <iostream>

std::ostream& operator<<(std::ostream& out, const Colour& colour)
{
   return out << colour[0] << ", " << colour[1] << ", " << colour[2] << ", " << colour[3];
}

std::ostream& operator<<(std::ostream& out, const SamplerColour& colour)
{
   return out << colour[0] << ", " << colour[1] << ", " << colour[2] << ", " << colour[3];
}

std::ostream& operator<<(std::ostream& out, const GLboolean& val)
{
   return out << glEnumToString(val);
}

std::ostream& operator<<(std::ostream& out, const SamplerMask& mask)
{
   return out << mask[0] << ", " << mask[1] << ", " << mask[2] << ", " << mask[3];
}

std::ostream& operator<<(std::ostream& out, const glsl::Value& value)
{
   return out << value.toString();
}

std::ostream& operator<<(std::ostream& out, const SamplerClampf& value)
{
   return out << (float)value;
}

std::ostream& operator<<(std::ostream& out, const SamplerDrawSize& value)
{
   return out << (int)value;
}

std::ostream& operator<<(std::ostream& out, const glsl::Variable& variable)
{
   return out << variable.name() << " = " << variable.value();
}

std::ostream& operator<<(std::ostream& out, const SamplerAlphaFunction& value)
{
   return out << glEnumToString(value, glAlphaFunc_func);
}

std::ostream& operator<<(std::ostream& out, const SamplerBlendEquation& value)
{
   return out << glEnumToString(value, glBlendEquation_mode);
}

std::ostream& operator<<(std::ostream& out, const SamplerBlendSrcFactor& value)
{
   return out << glEnumToString(value, glBlendFunc_factor);
}

std::ostream& operator<<(std::ostream& out, const SamplerBlendDstFactor& value)
{
   return out << glEnumToString(value, glBlendFunc_factor);
}

std::ostream& operator<<(std::ostream& out, const DrawState::SamplerDrawStyle& style){
   if (style == 0)
      out << "gradient";
   else if(style == 1)
      out << "solid";

   return out;
}

std::ostream& operator<<(std::ostream& out, const DrawState::SamplerDrawDirection& direction){
   if (direction == 0)
      out << "horizontal";
   else if(direction == 1)
      out << "vertical";

   return out;
}

BlendState::BlendState()
{
   clamp = gFrameBuffer.clamped();
}

void BlendState::dump()
{
   std::cout << "BlendState.enabled = " << enabled << std::endl;

   if (!enabled)
      return;

   std::cout << "\t" << "rgsSrcFactor = " << rgbSrcFactor << std::endl;
   std::cout << "\t" << "rgbDstFactor = " << rgbDstFactor << std::endl;
   std::cout << "\t" << "aSrcFactor   = " << aSrcFactor << std::endl;
   std::cout << "\t" << "aDstFactor   = " << aDstFactor << std::endl;
   std::cout << "\t" << "equation     = " << equation << std::endl;
   std::cout << "\t" << "colour       = " << colour << std::endl;
}

void BlendState::randomise()
{
   TestState::randomise();

   rgbSrcFactor.randomise();
   rgbDstFactor.randomise();

   aSrcFactor.randomise();
   aDstFactor.randomise();

   equation.randomise();

   colour.randomise();
}


AlphaState::AlphaState()
{
}

void AlphaState::dump()
{
   std::cout << "AlphaState.enabled = " << enabled << std::endl;

   if (!enabled)
      return;

   std::cout << "\t" << "function = " << function << std::endl;
   std::cout << "\t" << "reference = " << reference << std::endl;
}

void AlphaState::randomise()
{
   TestState::randomise();

   function.randomise();
   reference.randomise();
}


TextureState::TextureState()
   : width(0), height(0), texture(0), image(0), bgra_u8(0)
{
   chance = 50;
}

TextureState::~TextureState()
{
   if (image)
      delete [] image;

   if (bgra_u8)
      delete [] bgra_u8;

   if (texture)
      glDeleteTextures(1, &texture);
}

void TextureState::dump()
{
   std::cout << "TextureState.enabled = " << enabled << std::endl;

   if (!enabled)
      return;

   std::cout << "\t" << "width = " << width << std::endl;
   std::cout << "\t" << "height = " << height << std::endl;
}

void TextureState::randomise()
{
   TestState::randomise();
}


MaskState::MaskState()
{
   chance = 20;
}

void MaskState::dump()
{
   std::cout << "MaskState.enabled = " << enabled << std::endl;

   if (!enabled)
      return;

   std::cout << "\t" << "mask = " << mask << std::endl;
}

void MaskState::randomise()
{
   TestState::randomise();

   mask.randomise();
}


ShaderState::ShaderState()
   : program(0)
{
   chance = 100;
}

ShaderState::~ShaderState()
{
   if (program)
      delete program;
}

void ShaderState::dump()
{
   std::cout << "ShaderState.enabled = " << enabled << std::endl;

   if (!enabled)
      return;

   for (std::map<std::string, glsl::Variable>::const_iterator itr = program->variables().begin(); itr != program->variables().end(); ++itr) {
      if (itr->second.isUniform())
         std::cout << "\t" << itr->second << std::endl;
   }

   std::cout << "Vertex Shader:" << std::endl;
   std::cout << program->vertex()->toGLSL() << std::endl;

   std::cout << "Fragment Shader:" << std::endl;
   std::cout << program->fragment()->toGLSL() << std::endl;
}

void ShaderState::randomise()
{
   TestState::randomise();
}

DrawState::Randomiser DrawState::mRandomiser;

DrawState::DrawState()
{
   chance = 100;
   x = 0;
   y = 0;
   width = 256;
   height = 256;
   colour = Colour::White;
   clear = Colour::Black;
   style = DrawState::Solid;
}

void DrawState::dump()
{
   std::cout << "DrawState.enabled = " << enabled << std::endl;

   if (!enabled)
      return;

   std::cout << "\t" << "position  = " << x << ", " << y << std::endl;
   std::cout << "\t" << "size      = " << width << ", " << height << std::endl;
   std::cout << "\t" << "clear     = " << clear << std::endl;
   std::cout << "\t" << "style     = " << style << std::endl;

   if (style == DrawState::Solid) {
      std::cout << "\t" << "color     = " << colour << std::endl;
   }else if (style == DrawState::Gradient) {
      std::cout << "\t" << "direction = " << direction << std::endl;
      std::cout << "\t" << "gradient1 = " << gradient1 << std::endl;
      std::cout << "\t" << "gradient2 = " << gradient2 << std::endl;
   }
}

void DrawState::randomise()
{
   TestState::randomise();

   width.randomise();
   height.randomise();

   x = gRandom.random<int>(0, 256 - width);
   y = gRandom.random<int>(0, 256 - height);

   clear.randomise();
   colour.randomise();
   gradient1.randomise();
   gradient2.randomise();
   style.randomise();
   direction.randomise();
}
