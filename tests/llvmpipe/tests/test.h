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

#include "state.h"
#include "../glsl/program.h"
#include "../utils/piglit-ext.h"
#include "../utils/framebuffer.h"

class Test {
public:
   Test()
      : mErrorX(0), mErrorY(0)
   {
      mDraw.width  = piglit_width;
      mDraw.height = piglit_height;
   }

   ~Test()
   {
   }

   /* Run test */
   GLboolean run();

   /* Blending */
   void setBlendFunction(GLenum rgbaSrcFactor, GLenum rgbaDstFactor)
   {
      setBlendFunction(rgbaSrcFactor, rgbaDstFactor, rgbaSrcFactor, rgbaDstFactor);
   }

   void setBlendFunction(GLenum rgbSrcFactor, GLenum rgbDstFactor, GLenum aSrcFactor, GLenum aDstFactor)
   {
      mBlend.rgbSrcFactor = rgbSrcFactor;
      mBlend.rgbDstFactor = rgbDstFactor;
      mBlend.aSrcFactor = aSrcFactor;
      mBlend.aDstFactor = aDstFactor;

      setBlendEnabled(true);
   }

   void setBlendEquation(GLenum equation)
   {
      mBlend.equation = equation;

      setBlendEnabled(true);
   }

   void setBlendColour(const Colour& colour)
   {
      mBlend.colour = colour;

      setBlendEnabled(true);
   }

   void setBlendEnabled(bool enabled)
   {
      mBlend.enabled = enabled;
   }

   /* Alpha Test */
   void setAlphaFunction(GLenum function)
   {
      mAlpha.function = function;

      setAlphaEnabled(true);
   }

   void setAlphaFunction(GLenum function, GLclampf ref)
   {
      mAlpha.function = function;
      mAlpha.reference = ref;

      setAlphaEnabled(true);

   }

   void setAlphaReference(GLclampf ref)
   {
      mAlpha.reference = ref;

      setAlphaEnabled(true);
   }

   void setAlphaEnabled(bool enabled)
   {
      mAlpha.enabled = enabled;
   }

   /* Shader */
   void setShaderEnabled(bool enabled)
   {
      mShader.enabled = true;
   }

   void setShader(glsl::Shader* vertex, glsl::Shader* fragment)
   {
      if (!mShader.program)
         mShader.program = new glsl::Program();

      mShader.program->setVertexShader(vertex);
      mShader.program->setFragmentShader(fragment);
      mShader.enabled = true;
   }

   void setVertexShader(glsl::Shader* vertex)
   {
      if (!mShader.program)
         mShader.program = new glsl::Program();

      mShader.program->setVertexShader(vertex);
      mShader.enabled = true;
   }

   void setFragmentShader(glsl::Shader* fragment)
   {
      if (!mShader.program)
         mShader.program = new glsl::Program();

      mShader.program->setFragmentShader(fragment);
      mShader.enabled = true;
   }

   void setShaderProgram(glsl::Program* program)
   {
      mShader.program = program;
      mShader.enabled = true;
   }

   /* Colour mask */
   void setColourMask(const GLboolean* mask)
   {
      memcpy((GLboolean*)mMask.mask, mask, sizeof(GLboolean) * 4);
   }

   /* Colours */
   void setClearColour(const Colour& colour)
   {
      mDraw.clear = colour;
   }

   void setDrawColour(const Colour& colour)
   {
      mDraw.colour = colour;
   }

   void setDrawStyle(DrawState::Style style)
   {
      mDraw.style = style;
   }

   void setDrawGradient(const Colour& colour1, const Colour& colour2, DrawState::Direction direction)
   {
      mDraw.colour = Colour::White;
      mDraw.gradient1 = colour1;
      mDraw.gradient2 = colour2;
      mDraw.direction = direction;
      mDraw.style = DrawState::Gradient;
   }

   /* Texture */
   void setTexture(int width, int height, Colour* image)
   {
      if (mTexture.width != width || mTexture.height != height) {
         if (mTexture.bgra_u8)
            delete [] mTexture.bgra_u8;

         if (mTexture.image)
            delete [] mTexture.image;

         mTexture.bgra_u8 = new unsigned char[width * height * 4];
      }

      mTexture.width = width;
      mTexture.height = height;

      mTexture.image = image;

      unsigned char* bgra_u8 = mTexture.bgra_u8;
      for (int y = 0; y < height; ++y) {
         for (int x = 0 ; x < width; ++x, ++image, bgra_u8 += 4) {
            bgra_u8[0] = float_to_ubyte(image->blue());
            bgra_u8[1] = float_to_ubyte(image->green());
            bgra_u8[2] = float_to_ubyte(image->red());
            bgra_u8[3] = float_to_ubyte(image->alpha());
         }
      }

      if (mTexture.texture == 0)
         glGenTextures(1, &mTexture.texture);

      glBindTexture(GL_TEXTURE_2D, mTexture.texture);

      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
      glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,   GL_REPLACE);
      glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      if (gFrameBuffer.format() == GL_BGRA) {
         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mTexture.width, mTexture.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, mTexture.bgra_u8);
      } else {
         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mTexture.width, mTexture.height, 0, GL_RGBA, GL_FLOAT, mTexture.image);
      }
   }

   /* Accessors */
   const DrawState* drawState() const
   {
      return &mDraw;
   }

   const MaskState* maskState() const
   {
      return &mMask;
   }
   const BlendState* blendState() const
   {
      return &mBlend;
   }

   const AlphaState* alphaState() const
   {
      return &mAlpha;
   }

   const ShaderState* shaderState() const
   {
      return &mShader;
   }

   const TextureState* textureState() const
   {
      return &mTexture;
   }

   virtual const char* dumpPrefix() const
   {
      return NULL;
   }

protected:
   virtual void beginTest();
   virtual void applyState();
   virtual void draw();
   virtual void dumpState();

protected:
   int mErrorX, mErrorY;
   DrawState mDraw;
   MaskState mMask;
   BlendState mBlend;
   AlphaState mAlpha;
   ShaderState mShader;
   TextureState mTexture;
};
