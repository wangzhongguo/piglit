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

#include "glenum.h"

#define e(x) {x, #x}

static struct {
   GLenum value;
   const char* name;
} enums[] = {
   e(GL_FALSE),
   e(GL_TRUE),

   /* glAlphaFunc func */
   e(glAlphaFunc_func),
   e(GL_NEVER), e(GL_ALWAYS),
   e(GL_LESS), e(GL_LEQUAL),
   e(GL_EQUAL), e(GL_NOTEQUAL),
   e(GL_GREATER), e(GL_GEQUAL),

   /* glBlendEquation mode */
   e(glBlendEquation_mode),
   e(GL_FUNC_ADD),
   e(GL_FUNC_SUBTRACT),
   e(GL_FUNC_REVERSE_SUBTRACT),
   e(GL_MIN),
   e(GL_MAX),

   /* glBlendFunc factor */
   e(glBlendFunc_factor),
   e(GL_ZERO), e(GL_ONE),
   e(GL_SRC_COLOR), e(GL_ONE_MINUS_SRC_COLOR),
   e(GL_DST_COLOR), e(GL_ONE_MINUS_DST_COLOR),
   e(GL_SRC_ALPHA), e(GL_ONE_MINUS_SRC_ALPHA),
   e(GL_DST_ALPHA), e(GL_ONE_MINUS_DST_ALPHA),
   e(GL_CONSTANT_COLOR), e(GL_ONE_MINUS_CONSTANT_COLOR),
   e(GL_CONSTANT_ALPHA), e(GL_ONE_MINUS_CONSTANT_ALPHA),
   e(GL_SRC_ALPHA_SATURATE),

   /* glTexImage2D internalFormat */
   e(glTexImage2D_internalFormat),
   e(GL_ALPHA), e(GL_ALPHA4), e(GL_ALPHA8), e(GL_ALPHA12), e(GL_ALPHA16),
   e(GL_COMPRESSED_ALPHA),
   e(GL_COMPRESSED_RGB), e(GL_COMPRESSED_RGBA),
   e(GL_COMPRESSED_LUMINANCE), e(GL_COMPRESSED_LUMINANCE_ALPHA), e(GL_COMPRESSED_INTENSITY),
   e(GL_DEPTH_COMPONENT), e(GL_DEPTH_COMPONENT16), e(GL_DEPTH_COMPONENT24), e(GL_DEPTH_COMPONENT32),
   e(GL_LUMINANCE), e(GL_LUMINANCE4), e(GL_LUMINANCE8), e(GL_LUMINANCE12), e(GL_LUMINANCE16),
   e(GL_LUMINANCE_ALPHA), e(GL_LUMINANCE4_ALPHA4), e(GL_LUMINANCE6_ALPHA2), e(GL_LUMINANCE8_ALPHA8),
   e(GL_LUMINANCE12_ALPHA4), e(GL_LUMINANCE12_ALPHA12), e(GL_LUMINANCE16_ALPHA16),
   e(GL_INTENSITY), e(GL_INTENSITY4), e(GL_INTENSITY8), e(GL_INTENSITY12), e(GL_INTENSITY16),
   e(GL_R3_G3_B2), e(GL_RGB), e(GL_RGB4), e(GL_RGB5), e(GL_RGB8), e(GL_RGB10), e(GL_RGB12), e(GL_RGB16), e(GL_RGB16F), e(GL_RGB32F),
   e(GL_RGBA), e(GL_RGBA2), e(GL_RGBA4), e(GL_RGB5_A1), e(GL_RGBA8), e(GL_RGB10_A2), e(GL_RGBA12), e(GL_RGBA16), e(GL_RGBA16F), e(GL_RGBA32I), e(GL_RGBA32F),
   e(GL_SLUMINANCE), e(GL_SLUMINANCE8), e(GL_SLUMINANCE_ALPHA), e(GL_SLUMINANCE8_ALPHA8),
   e(GL_SRGB), e(GL_SRGB8),
   e(GL_SRGB_ALPHA), e(GL_SRGB8_ALPHA8),
   e(GL_RED), e(GL_R8), e(GL_R16), e(GL_R16F), e(GL_R32F),
   e(GL_RG), e(GL_RG8), e(GL_RG16), e(GL_RG16F), e(GL_RG32F),
   e(1), e(2), e(3), e(4),

   /* glTexImage2D format */
   e(glTexImage2D_format),
   e(GL_COLOR_INDEX),
   e(GL_RED),
   e(GL_GREEN),
   e(GL_BLUE),
   e(GL_ALPHA),
   e(GL_RG),
   e(GL_RGB),
   e(GL_BGR),
   e(GL_RGBA),
   e(GL_BGRA),
   e(GL_LUMINANCE),
   e(GL_LUMINANCE_ALPHA),

   /* glTexImage2D type */
   e(glTexImage2D_type),
   e(GL_BITMAP),
   e(GL_FLOAT),
   e(GL_HALF_FLOAT),
   e(GL_UNSIGNED_BYTE), e(GL_BYTE),
   e(GL_UNSIGNED_SHORT), e(GL_SHORT),
   e(GL_UNSIGNED_INT), e(GL_INT),
   e(GL_UNSIGNED_BYTE_3_3_2), e(GL_UNSIGNED_BYTE_2_3_3_REV),
   e(GL_UNSIGNED_SHORT_5_6_5), e(GL_UNSIGNED_SHORT_5_6_5_REV),
   e(GL_UNSIGNED_SHORT_4_4_4_4), e(GL_UNSIGNED_SHORT_4_4_4_4_REV),
   e(GL_UNSIGNED_SHORT_5_5_5_1), e(GL_UNSIGNED_SHORT_1_5_5_5_REV),
   e(GL_UNSIGNED_INT_8_8_8_8), e(GL_UNSIGNED_INT_8_8_8_8_REV),
   e(GL_UNSIGNED_INT_10_10_10_2), e(GL_UNSIGNED_INT_2_10_10_10_REV),

   { 0, 0 }
};

std::string glEnumToString(GLenum val, glEnumTypes type)
{
   bool atType = type == GL_ENUM_TYPE;
   for (int i = 0; enums[i].name; ++i) {
      if (enums[i].value == type)
         atType = true;

      if (atType && enums[i].value == val)
         return enums[i].name;
   }

   assert("You should add this enum!" && 0);
   return std::string();
}

GLenum glStringToEnum(const std::string& val)
{
   for (int i = 0; enums[i].name; ++i)
      if (val.compare(enums[i].name) == 0)
         return enums[i].value;

   return 0;
}
