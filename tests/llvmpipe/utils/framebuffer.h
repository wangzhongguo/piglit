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
#include "glenum.h"
#include <iostream>

class FrameBuffer
{
public:
   FrameBuffer()
      : mWidth(0), mHeight(0), mTexture(0), mFrameBuffer(0)
   {
      mInternalFormat = GL_RGBA;
      mFormat = GL_BGRA;
      mType = GL_UNSIGNED_BYTE;
   }

   ~FrameBuffer()
   {
      if (mTexture)
         glDeleteTextures(1, &mTexture);

      /*if (mFrameBuffer)
         glDeleteFramebuffersEXT(1, &mFrameBuffer);*/
   }

   GLuint object() const
   {
      return mFrameBuffer;
   }

   GLuint texture() const
   {
      return mTexture;
   }

   int width() const
   {
      return mWidth;
   }

   int height() const
   {
      return mHeight;
   }

   bool clamped()
   {
      return mClamped;
   }

   GLenum type()
   {
      return mChannelType;
   }

   GLenum format()
   {
      return mFormat;
   }

   GLint internalFormat()
   {
      return mInternalFormat;
   }

   void tolerance(float* tolerance) {
      bool floating = mChannelType == GL_FLOAT || mChannelType == GL_HALF_FLOAT;

      for (int i = 0; i < 4; ++i) {
         int bits = mChannelBits[i];

         if (bits <= 1) {
            tolerance[i] = 0.6f;
            continue;
         }

         if (bits < 4) {
            tolerance[i] = (1.15f / ((1 << bits) - 1));
         } else if (bits < 8) {
            tolerance[i] = (2.0f / ((1 << bits) - 1));
            continue;
         }

         if (floating) {
            if (bits == 32)
               bits = 23;
            else if (bits== 16)
               bits = 10;
         }

         tolerance[i] = 5.0f / ((1 << bits) - 1);
      }
   }

   int* channelBits()
   {
      return mChannelBits;
   }

   int channelMask()
   {
      int mask = 0;
      for (int i = 0; i < 4; ++i) {
         if (mChannelBits[i])
            mask |= 1 << i;
      }

      return mask;
   }

   void setSize(int width, int height)
   {
      mWidth = width;
      mHeight = height;
   }

   void setClamped(bool value)
   {
      mClamped = value;
   }

   void setType(GLenum value)
   {
      if (!value)
         return;

      mType = value;
      mChannelType = value;
   }

   void setFormat(GLenum value)
   {
      if (!value)
         return;

      mFormat = value;
   }

   void setInternalFormat(GLint value)
   {
      if (!value)
         return;

      mInternalFormat = value;
   }

   void bind()
   {
      if (!mFrameBuffer && !create()) {
         std::cout << "Unable to create frame buffer" << std::endl;
         std::cout << "\t" << "internalFormat = " << glEnumToString(internalFormat(), glTexImage2D_internalFormat) << std::endl;
         std::cout << "\t" << "format = "         << glEnumToString(format(),         glTexImage2D_format)         << std::endl;
         std::cout << "\t" << "type = "           << glEnumToString(type(),           glTexImage2D_type)           << std::endl;
         exit(-1);
      }

      glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);
      glViewport(0, 0, mWidth, mHeight);
   }

   void unbind()
   {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
   }

   bool create()
   {
      GLenum status;

      glGenTextures(1, &mTexture);
      glBindTexture(GL_TEXTURE_2D, mTexture);
      glTexImage2D(GL_TEXTURE_2D, 0, mInternalFormat, mWidth, mHeight, 0, mFormat, mType, NULL);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

      glGenFramebuffers(1, &mFrameBuffer);
      glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture, 0);

      assert(glGetError() == 0);
      //glEnable(GL_FRAMEBUFFER_SRGB); => check for srgb

      status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER);
      if (status != GL_FRAMEBUFFER_COMPLETE) {
         printf("fbo incomplete (status = 0x%04x)\n", status);
         return false;
      }

      glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE,   &mChannelBits[0]);
      glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE, &mChannelBits[1]);
      glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE,  &mChannelBits[2]);
      glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE, &mChannelBits[3]);

      glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE, &mChannelType);

      int size = 0;
      glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_LUMINANCE_SIZE, &size);

      if (size)
         mChannelBits[0] = size;

      size = 0;
      glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTENSITY_SIZE, &size);

      if (size)
         mChannelBits[0] = size;

      return true;
   }

private:
   int mWidth;
   int mHeight;
   GLuint mTexture;
   GLuint mFrameBuffer;

   GLint mChannelType;
   GLint mChannelBits[4];

   bool mClamped;
   GLenum mType;
   GLenum mFormat;
   GLint mInternalFormat;
};

extern FrameBuffer gFrameBuffer;
