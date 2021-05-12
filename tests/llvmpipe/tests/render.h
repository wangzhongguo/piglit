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

#include "../glsl/program.h"
#include "state.h"

struct Vertex2f {
   float x;
   float y;
};

struct Vertex3f {
   float x;
   float y;
   float z;
};

struct Polygon
{
   Polygon()
   {
      memset(this, 0, sizeof(Polygon));
   }

   const Colour* color;
   const Vertex3f* vertex;
   const Vertex2f* texcoord;

   const Colour* texture;
   int texture_width;
   int texture_height;

   int mode;
   int count;

   struct {
      int width;
      int height;
   } quad;
};

class Render
{
public:
   Render()
      : mBuffer(0), mWidth(0), mHeight(0), mAlphaTestMask(0), mChannelMask(0xFF),
        mClampOutput(false), mProgram(0), mAlphaState(0), mBlendState(0), mMaskState(0)
   {
   }

   ~Render()
   {
      delete [] mBuffer;
      delete [] mAlphaTestMask;
   }

   void render()
   {
      processVertex();
      processFragment();
      mPolygons.clear();
   }

   void resize(int width, int height)
   {
      if (mWidth != width || mHeight != height) {
         delete [] mBuffer;
         delete [] mAlphaTestMask;

         mBuffer = new Colour[width * height];
         mAlphaTestMask = new unsigned char[width * height];
      }

      mWidth = width;
      mHeight = height;
   }

   void clear()
   {
      Colour* pixel = mBuffer;
      Colour clear = mClearColour;

      for (int y = 0; y < mHeight; ++y)
         for (int x = 0; x < mWidth; ++x, ++pixel)
            *pixel = clear;

      unsigned char* mask = mAlphaTestMask;
      memset(mask, 0, mWidth * mHeight * sizeof(unsigned char));
   }

   void setChannelBits(int* bits)
   {
      memcpy(mChannelBits, bits, sizeof(int) * 4);
   }

   void bindTexture2D(int width, int height, const Colour* texture)
   {
      mPolygonConstruct.texture = texture;
      mPolygonConstruct.texture_width = width;
      mPolygonConstruct.texture_height = height;
   }

   void texCoordPointer(const Vertex2f* pointer)
   {
      mPolygonConstruct.texcoord = pointer;
   }

   void vertexPointer(const Vertex3f* pointer)
   {
      mPolygonConstruct.vertex = pointer;
   }

   void colorPointer(const Colour* pointer)
   {
      mPolygonConstruct.color = pointer;
   }

   void drawQuads(int count)
   {
      mPolygonConstruct.mode = GL_QUADS;
      mPolygonConstruct.count = count;
      mPolygons.push_back(mPolygonConstruct);

      mPolygonConstruct = Polygon();
   }

   bool hasChannel(int c) {
      return (mChannelMask & (1 << c)) != 0;
   }

   void setClearColour(const Colour& colour)
   {
      mClearColour = colour;

      for (int c = 0; c < 4; ++c) {
         if (!hasChannel(c)) {
            mClearColour[c] = c < 3 ? 0.0f : 1.0f;
         }
      }
   }

   void setShaderProgram(glsl::Program* program)
   {
      mProgram = program;
   }

   void setAlphaState(AlphaState* state)
   {
      mAlphaState = state;
   }

   void setBlendState(BlendState* state)
   {
      mBlendState = state;
   }

   void setMaskState(MaskState* state)
   {
      mMaskState = state;
   }

   void setClampOutput(bool clamp)
   {
      mClampOutput = clamp;
   }

   void setChannelMask(int channels)
   {
      mChannelMask = channels;
   }

   float* frameBuffer()
   {
      return (float*)mBuffer;
   }

   int channelMask()
   {
      return mChannelMask;
   }

   unsigned char* alphaTestMask()
   {
      return mAlphaTestMask;
   }

   int width()
   {
      return mWidth;
   }

   int height()
   {
      return mHeight;
   }

private:
   void processVertex()
   {
      for (std::vector<Polygon>::iterator itr = mPolygons.begin(); itr != mPolygons.end(); ++itr) {
         Polygon& polygon = *itr;

         const Vertex2f* tex = polygon.texcoord;
         const Vertex3f* vert = polygon.vertex;

         assert(vert);
         assert(polygon.count == 4);
         assert(polygon.mode == GL_QUADS);

         polygon.quad.width  = vert[2].x - vert[0].x;
         polygon.quad.height = vert[2].y - vert[0].y;

         /* Ensure we are dealing with a screen aligned rectangle! */
         assert(vert[1].x - vert[0].x == polygon.quad.width);
         assert(vert[2].x - vert[3].x == polygon.quad.width);
         assert(vert[2].y - vert[1].y == polygon.quad.height);
         assert(vert[3].y - vert[0].y == polygon.quad.height);
         assert(vert[1].y - vert[0].y == 0);
         assert(vert[3].y - vert[2].y == 0);

         /* Ensure simple 1 to 1 texture blit */
         if (tex) {
            assert(tex[0].x == 0.0f && tex[0].y == 0.0f);
            assert(tex[1].x == 1.0f && tex[1].y == 0.0f);
            assert(tex[2].x == 1.0f && tex[2].y == 1.0f);
            assert(tex[3].x == 0.0f && tex[3].y == 1.0f);
            assert(!polygon.texture || (polygon.texture_width == polygon.quad.width && polygon.texture_height == polygon.quad.height));
         }
      }
   }

   void processFragment()
   {
      glsl::ShaderState state;
      glsl::Shader* fragment;
      int stride = mWidth;

      fragment = mProgram ? mProgram->fragment() : NULL;

      if(fragment)
         fragment->start(state);

      Colour& gl_Color = state["gl_Color"].value<Colour>();
      int* gl_TexCoord0 = state["gl_TexCoord[0]"].valuePtr<int>();
      Colour& gl_FragColor = state["gl_FragColor"].value<Colour>();

      for (std::vector<Polygon>::iterator itr = mPolygons.begin(); itr != mPolygons.end(); ++itr) {
         Polygon& polygon = *itr;

         const Colour* color = polygon.color;
         const Vertex3f* vert  = polygon.vertex;

         int drawX = vert[0].x;
         int drawY = vert[0].y;

         Colour* pixelBuffer  = mBuffer + drawX + stride * drawY;
         unsigned char* maskBuffer = mAlphaTestMask + drawX + stride * drawY;

         Colour ldy, rdy;
         ldy = (color[3] - color[0]) / (float)polygon.quad.height;
         rdy = (color[2] - color[1]) / (float)polygon.quad.height;

         if (polygon.texture) {
            state["$TEX0_PTR"] = glsl::Value::create<const Colour*>((const Colour*)polygon.texture);
            state["$TEX0_WIDTH"] = glsl::Value::create<int>(polygon.texture_width);
            state["$TEX0_HEIGHT"] = glsl::Value::create<int>(polygon.texture_height);
         }

         for (int y = 0; y < polygon.quad.height; ++y) {
            Colour lc, rc;
            lc = color[0] + ldy * (0.5f + (float)y);
            rc = color[1] + rdy * (0.5f + (float)y);

            Colour dxc;
            dxc = (rc - lc) / (float)polygon.quad.width;

            for (int x = 0; x < polygon.quad.width; ++x) {
               Colour& dst = pixelBuffer[x + y * stride];
               unsigned char& mask = maskBuffer[x + y * stride];

               gl_TexCoord0[0] = x;
               gl_TexCoord0[1] = y;
               gl_Color = lc + dxc * (0.5f + (float)x);

               /* Perform shader or fixed function */
               if (fragment) {
                  fragment->run(state);
               } else {
                  if (polygon.texture) {
                     gl_FragColor = polygon.texture[x + y * polygon.texture_width];
                  } else {
                     gl_FragColor = gl_Color;
                  }
               }

               /* Clamp if necessary */
               if (mBlendState && mBlendState->clamp) {
                  gl_FragColor.clamp(0.0f, 1.0f);
               }

               /* If close to alpha transition then mask the test */
               if (fabs(gl_FragColor[3] - mAlphaState->reference) <= (3.0f / 255.0f)) {
                  mask = true;
               }

               if (!doAlphaTest(gl_FragColor)) {
                  gl_FragColor = mClearColour;
               } else {
                  clampToChannelBits(dst);

                  /* Blend and mask */
                  gl_FragColor = doBlend(gl_FragColor, dst);
                  gl_FragColor = doMask(gl_FragColor);
               }

               if (mClampOutput) {
                  gl_FragColor.clamp(0.0f, 1.0f);
               }

               for (int c = 0; c < 4; ++c) {
                  if (!hasChannel(c)) {
                     gl_FragColor[c] = c < 3 ? 0.0f : 1.0f;
                  }
               }

               dst = gl_FragColor;
            }
         }
      }

      if (fragment)
         fragment->end(state);
   }

   void clampToChannelBits(Colour& colour, bool alphaOnly = false)
   {
      for (int i = 0; i < 4; ++i) {
         int bits = mChannelBits[i];

         if (alphaOnly && i != 3)
            continue;

         if (bits == 0 || bits == 32)
             continue;

         int val = 0.5f + (colour[i] * ((1 << bits) - 1));
         colour[i] = (float)val / (float)((1 << bits) - 1);
      }
   }

   Colour doMask(const Colour& in)
   {
      if (!mMaskState || !mMaskState->enabled)
         return in;

      Colour out;
      for (int i = 0; i < 4; ++i)
         out[i] = mMaskState->mask[i] ? in[i] : mClearColour[i];

      return out;
   }

   bool doAlphaTest(const Colour& in)
   {
      if (!mAlphaState || !mAlphaState->enabled)
         return true;

      int src = in[3] * 255.0f;
      int ref = mAlphaState->reference * 255.0f;

      switch(mAlphaState->function){
         case GL_NEVER:
            return false;
         case GL_LESS:
            return src <  ref;
         case GL_EQUAL:
            return src == ref;
         case GL_LEQUAL:
            return src <= ref;
         case GL_GREATER:
            return src >  ref;
         case GL_NOTEQUAL:
            return src != ref;
         case GL_GEQUAL:
            return src >= ref;
         case GL_ALWAYS:
            return true;
      }

      return false;
   }

   Colour doBlend(const Colour& src, const Colour& dst)
   {
      if (!mBlendState || !mBlendState->enabled) {
         return src;
      }

      Colour fSrc, fDst;

      if (mBlendState->equation == GL_MIN || mBlendState->equation == GL_MAX) {
         fSrc = src;
         fDst = dst;
      } else {
         fSrc = doBlendFunction(mBlendState->rgbSrcFactor, src, dst, false);
         fDst = doBlendFunction(mBlendState->rgbDstFactor, src, dst, false);

         fSrc[3] = doBlendFunction(mBlendState->aSrcFactor, src, dst, true)[3];
         fDst[3] = doBlendFunction(mBlendState->aDstFactor, src, dst, true)[3];

         fSrc *= src;
         fDst *= dst;
      }

      return doBlendEquation(mBlendState->equation, fSrc, fDst);
   }

   Colour doBlendFunction(GLenum function, const Colour& src, const Colour& dst, bool alpha)
   {
      Colour result;

      switch(function){
         case GL_ZERO:
            if (!alpha) {
               result[0] = 0.0f;
               result[1] = 0.0f;
               result[2] = 0.0f;
            } else {
               result[3] = 0.0f;
            }
            break;
         case GL_ONE:
            if (!alpha) {
               result[0] = 1.0f;
               result[1] = 1.0f;
               result[2] = 1.0f;
            } else {
               result[3] = 1.0f;
            }
            break;
         case GL_SRC_COLOR:
            if (!alpha) {
               result[0] = src[0];
               result[1] = src[1];
               result[2] = src[2];
            } else {
               result[3] = src[3];
            }
            break;
         case GL_ONE_MINUS_SRC_COLOR:
            if (!alpha) {
               result[0] = 1.0f - src[0];
               result[1] = 1.0f - src[1];
               result[2] = 1.0f - src[2];
            } else {
               result[3] = 1.0f - src[3];
            }
            break;
         case GL_DST_COLOR:
            if (!alpha) {
               result[0] = dst[0];
               result[1] = dst[1];
               result[2] = dst[2];
            } else {
               result[3] = dst[3];
            }
            break;
         case GL_ONE_MINUS_DST_COLOR:
            if (!alpha) {
               result[0] = 1.0f - dst[0];
               result[1] = 1.0f - dst[1];
               result[2] = 1.0f - dst[2];
            } else {
               result[3] = 1.0f - dst[3];
            }
            break;
         case GL_SRC_ALPHA:
            if (!alpha) {
               result[0] = src[3];
               result[1] = src[3];
               result[2] = src[3];
            } else {
               if (!hasChannel(3)) {
                  result[3] = 1.0f;
               } else {
                  result[3] = src[3];
               }
            }
            break;
         case GL_ONE_MINUS_SRC_ALPHA:
            if (!alpha) {
               result[0] = 1.0f - src[3];
               result[1] = 1.0f - src[3];
               result[2] = 1.0f - src[3];
            } else {
               if (!hasChannel(3)) {
                  result[3] = 0.0f;
               } else {
                  result[3] = 1.0f - src[3];
               }
            }
            break;
         case GL_DST_ALPHA:
            if (!alpha) {
               result[0] = dst[3];
               result[1] = dst[3];
               result[2] = dst[3];
            } else {
               result[3] = dst[3];
            }
            break;
         case GL_ONE_MINUS_DST_ALPHA:
            if (!alpha) {
               result[0] = 1.0f - dst[3];
               result[1] = 1.0f - dst[3];
               result[2] = 1.0f - dst[3];
            } else {
               result[3] = 1.0f - dst[3];
            }
            break;
         case GL_CONSTANT_COLOR:
            if (!alpha) {
               result[0] = mBlendState->colour[0];
               result[1] = mBlendState->colour[1];
               result[2] = mBlendState->colour[2];
            } else {
               result[3] = mBlendState->colour[3];
            }
            break;
         case GL_ONE_MINUS_CONSTANT_COLOR:
            if (!alpha) {
               result[0] = 1.0f - mBlendState->colour[0];
               result[1] = 1.0f - mBlendState->colour[1];
               result[2] = 1.0f - mBlendState->colour[2];
            } else {
               result[3] = 1.0f - mBlendState->colour[3];
            }
            break;
         case GL_CONSTANT_ALPHA:
            if (!alpha) {
               result[0] = mBlendState->colour[3];
               result[1] = mBlendState->colour[3];
               result[2] = mBlendState->colour[3];
            } else {
               result[3] = mBlendState->colour[3];
            }
            break;
         case GL_ONE_MINUS_CONSTANT_ALPHA:
            if (!alpha) {
               result[0] = 1.0f - mBlendState->colour[3];
               result[1] = 1.0f - mBlendState->colour[3];
               result[2] = 1.0f - mBlendState->colour[3];
            } else {
               result[3] = 1.0f - mBlendState->colour[3];
            }
            break;
         case GL_SRC_ALPHA_SATURATE:
            if (!alpha) {
               result[0] = MIN2(src[3], 1.0f - dst[3]);
               result[1] = MIN2(src[3], 1.0f - dst[3]);
               result[2] = MIN2(src[3], 1.0f - dst[3]);
            } else {
               result[3] = 1.0f;
            }
            break;
         default:
            assert(0);
      }

      return result;
   }

   Colour doBlendEquation(GLenum equation, const Colour& src, const Colour& dst)
   {
      switch(equation){
         case GL_FUNC_ADD:
            return src + dst;
         case GL_FUNC_SUBTRACT:
            return src - dst;
         case GL_FUNC_REVERSE_SUBTRACT:
            return dst - src;
         case GL_MIN:
            return Colour::min(src, dst);
         case GL_MAX:
            return Colour::max(src, dst);
         default:
            assert(0);
            return Colour::Green;
      };
   }

private:
   Colour* mBuffer;
   int mWidth, mHeight;
   unsigned char* mAlphaTestMask;

   int mChannelMask;
   int mChannelBits[4];
   bool mClampOutput;
   Colour mClearColour;

   glsl::Program* mProgram;
   AlphaState* mAlphaState;
   BlendState* mBlendState;
   MaskState* mMaskState;

   Polygon mPolygonConstruct;
   std::vector<Polygon> mPolygons;
};
