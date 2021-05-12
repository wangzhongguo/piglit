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

#include "test.h"
#include "render.h"
#include "../utils/piglit-ext.h"

Colour Colour::Black  = Colour(0.0f, 0.0f, 0.0f, 1.0f);
Colour Colour::White  = Colour(1.0f, 1.0f, 1.0f, 1.0f);
Colour Colour::Red    = Colour(1.0f, 0.0f, 0.0f, 1.0f);
Colour Colour::Green  = Colour(0.0f, 1.0f, 0.0f, 1.0f);
Colour Colour::Blue   = Colour(0.0f, 0.0f, 1.0f, 1.0f);
Colour Colour::Zero   = Colour(0.0f, 0.0f, 0.0f, 0.0f);

Render gRender;

GLboolean Test::run()
{
   beginTest();
   applyState();
   draw();

   GLboolean pass = piglit_probe_image_rgba_error_mask(0, 0,
                                                       gRender.width(), gRender.height(),
                                                       gRender.frameBuffer(),
                                                       gRender.channelMask(),
                                                       gRender.alphaTestMask(),
                                                       dumpPrefix(),
                                                       &mErrorX, &mErrorY);
   if (!pass)
      dumpState();

   return pass;
}

void Test::dumpState()
{
   mDraw.dump();
   mMask.dump();
   mBlend.dump();
   mAlpha.dump();
   mTexture.dump();

   if (mTexture.enabled) {
      int texX = mErrorX - mDraw.x;
      int texY = mErrorY - mDraw.y;

      if (texX < mDraw.width && texY < mDraw.height)
         std::cout << "\t" << "[" << texX << ", " << texY << "] = " << mTexture.image[texX + texY * mTexture.width] << std::endl;
   }

   mShader.dump();
}

void Test::beginTest()
{
}

void Test::applyState()
{
   glUseProgram(0);

   glShadeModel(GL_SMOOTH);
   glDisable(GL_BLEND);
   glDisable(GL_ALPHA_TEST);
   glDisable(GL_TEXTURE_2D);
   glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

   glClearColor(mDraw.clear[0], mDraw.clear[1], mDraw.clear[2], mDraw.clear[3]);
   glClear(GL_COLOR_BUFFER_BIT);

   if (mDraw.style == DrawState::Solid) {
      glColor4fv(mDraw.colour);
   } else {
      glColor4fv(Colour::White);
   }

   if (mBlend.enabled) {
      glEnable(GL_BLEND);
      glBlendEquation(mBlend.equation);
      glBlendFuncSeparate(mBlend.rgbSrcFactor, mBlend.rgbDstFactor, mBlend.aSrcFactor, mBlend.aDstFactor);
      glBlendColor(mBlend.colour[0], mBlend.colour[1], mBlend.colour[2], mBlend.colour[3]);
   }

   if (mAlpha.enabled) {
      glEnable(GL_ALPHA_TEST);
      glAlphaFunc(mAlpha.function, mAlpha.reference);
   }

   if (mShader.enabled) {
      mShader.program->applyState();
   }

   if (mTexture.enabled) {
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, mTexture.texture);
   }

   if (mMask.enabled) {
      glColorMask(mMask.mask[0], mMask.mask[1], mMask.mask[2], mMask.mask[3]);
   }

   /* XXX: Comment this out if clamp color is not supported in opengl implementation */
   if (mBlend.clamp) {
      glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_TRUE);
      glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_TRUE);
   } else {
      glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE);
      glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE);
   }

   gRender.resize(gFrameBuffer.width(), gFrameBuffer.height());

   gRender.setChannelBits(gFrameBuffer.channelBits());
   gRender.setChannelMask(gFrameBuffer.channelMask());
   gRender.setAlphaState(&mAlpha);
   gRender.setBlendState(&mBlend);
   gRender.setMaskState(&mMask);
   gRender.setClearColour(mDraw.clear);
   gRender.setShaderProgram(mShader.program);
}

void Test::draw()
{
   if (!mDraw.enabled) {
      return;
   }

   float verts[4][3];
   float tex[4][2];
   Colour colours[4];

   verts[0][0] = mDraw.x;
   verts[0][1] = mDraw.y;
   verts[0][2] = 0.0f;
   tex[0][0]   = 0.0f;
   tex[0][1]   = 0.0f;
   colours[0]  = mDraw.style == DrawState::Solid ? mDraw.colour : mDraw.gradient1;

   verts[1][0] = mDraw.x + mDraw.width;
   verts[1][1] = mDraw.y;
   verts[1][2] = 0.0f;
   tex[1][0]   = 1.0f;
   tex[1][1]   = 0.0f;
   colours[1]  = mDraw.style == DrawState::Solid ? mDraw.colour : (mDraw.direction == DrawState::Horizontal ? mDraw.gradient2 : mDraw.gradient1);

   verts[2][0] = mDraw.x + mDraw.width;
   verts[2][1] = mDraw.y + mDraw.height;
   verts[2][2] = 0.0f;
   tex[2][0]   = 1.0f;
   tex[2][1]   = 1.0f;
   colours[2]  = mDraw.style == DrawState::Solid ? mDraw.colour : mDraw.gradient2;

   verts[3][0] = mDraw.x;
   verts[3][1] = mDraw.y + mDraw.height;
   verts[3][2] = 0.0f;
   tex[3][0]   = 0.0f;
   tex[3][1]   = 1.0f;
   colours[3]  = mDraw.style == DrawState::Solid ? mDraw.colour : (mDraw.direction == DrawState::Horizontal ? mDraw.gradient1 : mDraw.gradient2);

   glEnableClientState(GL_VERTEX_ARRAY);

   glEnableClientState(GL_COLOR_ARRAY);
   glColorPointer(4, GL_FLOAT, 0, colours);

   if (mTexture.enabled) {
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glTexCoordPointer(2, GL_FLOAT, 0, tex);
   }

   glVertexPointer(3, GL_FLOAT, 0, verts);

   glDrawArrays(GL_QUADS, 0, 4);

   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_COLOR_ARRAY);
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);

   gRender.setClampOutput(gFrameBuffer.type() != GL_FLOAT && gFrameBuffer.type() != GL_HALF_FLOAT);
   gRender.clear();

   if (mTexture.enabled) {
      gRender.bindTexture2D(mTexture.width, mTexture.height, mTexture.image);
      gRender.texCoordPointer((Vertex2f*)tex);
   }

   gRender.colorPointer(colours);
   gRender.vertexPointer((Vertex3f*)verts);
   gRender.drawQuads(4);

   gRender.render();
}
