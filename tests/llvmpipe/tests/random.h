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

#include "test.h"
#include "randomshader.h"

class RandomTest : public Test
{
public:
   static bool enabled;

public:
   RandomTest()
      : mLoadState(false)
   {
   }

   RandomTest(const std::string& test)
      : mLoadState(true), mRandomState(test)
   {
   }

   ~RandomTest()
   {
   }

protected:
   virtual void dumpState()
   {
      std::cout << "RandomTest id: " << mRandomState << std::endl;

      Test::dumpState();
   }

   virtual const char* dumpPrefix() const
   {
      return mRandomState.c_str();
   }

   virtual void beginTest()
   {
      if (mLoadState) {
         gRandom.setState(mRandomState);
      } else {
         mRandomState = gRandom.state();
      }

      mDraw.randomise();
      mMask.randomise();
      mBlend.randomise();
      mAlpha.randomise();
      mShader.randomise();
      mTexture.randomise();

      mBlend.clamp = gFrameBuffer.clamped();

      /*
      mDraw.x = mDraw.y = 0;
      mDraw.width = piglit_width;
      mDraw.height = piglit_height;
      mDraw.style = DrawState::Solid;
      mTexture.enabled = false;
      mShader.enabled = false;
      mBlend.enabled = false;
      mAlpha.enabled = false;
      mMask.enabled = false;
      */

      if (mShader.enabled) {
         if (mShader.program) {
            delete mShader.program;
            mShader.program = 0;
         }

         RandomShader shader;
         mShader.program = shader.program();

         if (shader.usesUniform()) {
            mShader.program->setUniform("colour", gRandom.random<Colour>());
         }

         mTexture.enabled = shader.usesTexture();
      }

      if (mTexture.enabled) {
         Colour* image = mTexture.image;
         if (!image || mDraw.width != mTexture.width || mDraw.height != mTexture.height) {
            image = new Colour[mDraw.width * mDraw.height];
         }

         generateTexture(mDraw.width, mDraw.height, image);
         setTexture(mDraw.width, mDraw.height, image);

         if (mShader.enabled) {
            mShader.program->setUniformSampler2D("texture_id", 0);
         }
      }
   }

   void generateTexture(int width, int height, Colour* result)
   {
      int r = 0, g = 0, b = 0;
      r = gRandom.random<int>(3);
      do { g = gRandom.random<int>(3); } while(r == g);
      do { b = gRandom.random<int>(3); } while(r == b && g == b);

      for (int y = 0; y < height; ++y) {
         for (int x = 0; x < width; ++x) {
            Colour& pixel = result[x + y * width];
            /* A easily identifiable pattern with x / y co-ordinates in color values
            pixel[0] = (x % 10) / 10.0f;
            pixel[1] = (y % 10) / 10.0f;
            pixel[2] = (x % 10) / 10.0f + (y % 10) / 100.0f;
            pixel[3] = 1.0f;
            */
            pixel[r] = ((float)(width - x)) / ((float)width);
            pixel[g] = ((float)(x + y)) / ((float)(height + width));
            pixel[b] = ((float)y) / ((float)height);
            pixel[3] = 1.0f;
         }
      }
   }

private:
   bool mLoadState;
   std::string mRandomState;
};
