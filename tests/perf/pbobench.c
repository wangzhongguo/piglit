/*
 * Copyright (C) 2009 VMware, Inc.
 * Copyright (C) 2017 Advanced Micro Devices, Inc.
 * Copyright (C) 2021 Valve Corporation
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VMWARE BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * Authors:
 *    Mike Blumenkrantz <michael.blumenkrantz@gmail.com>
 */

#include "common.h"
#include <stdbool.h>
#include "piglit-util-gl.h"

static bool color = true;
static int selected_test_index = -1;

PIGLIT_GL_TEST_CONFIG_BEGIN

   config.supports_gl_compat_version = 45;
   config.supports_gl_core_version = 45;
   for (int i = 1; i < argc; i++) {
      if (!strcmp(argv[i], "-nocolor")) {
         color = false;
      }
      if (!strcmp(argv[i], "-test")) {
         if (i == argc - 1) {
            fprintf(stderr, "-test requires an argument\n");
            exit(1);
         }

         const char *testnum = argv[i + 1];
         char *endptr;
         selected_test_index = strtol(testnum, &endptr, 10);

         if (endptr != argv[i + 1] + strlen(testnum)) {
            fprintf(stderr,
               "Failed to parse test number '%s'\n", testnum);
            exit(1);
         }

         printf("Running only test %d\n", selected_test_index);
         i++;
      }

      if (!strcmp(argv[i], "-help")) {
         fprintf(stderr, "pbobench [-test TESTNUM] [-nocolor]\n");
         exit(1);
      }
   }
   puts(config.supports_gl_core_version ? "Using Core profile." :
                      "Using Compatibility profile.");

   config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE |
                               PIGLIT_GL_VISUAL_DEPTH | PIGLIT_GL_VISUAL_STENCIL;

PIGLIT_GL_TEST_CONFIG_END

typedef struct Format {
   GLenum internal_format;
   GLenum format;
   GLenum type;
} Format;

static const Format format_es[] = {
	{GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE},
	{GL_RGB5_A1,			GL_RGBA,			GL_UNSIGNED_BYTE},
	{GL_RGBA4,				GL_RGBA,			GL_UNSIGNED_BYTE},
	{GL_SRGB8_ALPHA8,		GL_RGBA,			GL_UNSIGNED_BYTE},
	{GL_RGBA8_SNORM,		GL_RGBA,			GL_BYTE},
	{GL_RGBA4,				GL_RGBA,			GL_UNSIGNED_SHORT_4_4_4_4},
	{GL_RGB5_A1,			GL_RGBA,			GL_UNSIGNED_SHORT_5_5_5_1},
	{GL_RGB10_A2,			GL_RGBA,			GL_UNSIGNED_INT_2_10_10_10_REV},
	{GL_RGB5_A1,			GL_RGBA,			GL_UNSIGNED_INT_2_10_10_10_REV},
	{GL_RGBA16F,			GL_RGBA,			GL_HALF_FLOAT},
	{GL_RGBA32F,			GL_RGBA,			GL_FLOAT},
	{GL_RGBA16F,			GL_RGBA,			GL_FLOAT},
	{GL_RGB8,				GL_RGB,			GL_UNSIGNED_BYTE},
	{GL_RGB565,			GL_RGB,			GL_UNSIGNED_BYTE},
	{GL_SRGB8,				GL_RGB,			GL_UNSIGNED_BYTE},
	{GL_RGB8_SNORM,		GL_RGB,			GL_BYTE},
	{GL_RGB565,			GL_RGB,			GL_UNSIGNED_SHORT_5_6_5},
	{GL_R11F_G11F_B10F,	GL_RGB,			GL_UNSIGNED_INT_10F_11F_11F_REV},
	{GL_R11F_G11F_B10F,	GL_RGB,			GL_HALF_FLOAT},
	{GL_R11F_G11F_B10F,	GL_RGB,			GL_FLOAT},
	{GL_RGB9_E5,			GL_RGB,			GL_UNSIGNED_INT_5_9_9_9_REV},
	{GL_RGB9_E5,			GL_RGB,			GL_HALF_FLOAT},
	{GL_RGB9_E5,			GL_RGB,			GL_FLOAT},
	{GL_RGB16F,			GL_RGB,			GL_HALF_FLOAT},
	{GL_RGB32F,			GL_RGB,			GL_FLOAT},
	{GL_RGB16F,			GL_RGB,			GL_FLOAT},
	{GL_RG8,				GL_RG,				GL_UNSIGNED_BYTE},
	{GL_RG8_SNORM,			GL_RG,				GL_BYTE},
	{GL_RG16F,				GL_RG,				GL_HALF_FLOAT},
	{GL_RG32F,				GL_RG,				GL_FLOAT},
	{GL_RG16F,				GL_RG,				GL_FLOAT},
	{GL_R8,				GL_RED,			GL_UNSIGNED_BYTE},
	{GL_R8_SNORM,			GL_RED,			GL_BYTE},
	{GL_R16F,				GL_RED,			GL_HALF_FLOAT},
	{GL_R32F,				GL_RED,			GL_FLOAT},
	{GL_R16F,				GL_RED,			GL_FLOAT},
	{GL_RGBA,				GL_RGBA,			GL_UNSIGNED_BYTE},
	{GL_RGBA,				GL_RGBA,			GL_UNSIGNED_SHORT_4_4_4_4},
	{GL_RGBA,				GL_RGBA,			GL_UNSIGNED_SHORT_5_5_5_1},
	{GL_RGB,				GL_RGB,			GL_UNSIGNED_BYTE},
	{GL_RGB,				GL_RGB,			GL_UNSIGNED_SHORT_5_6_5},
	{GL_LUMINANCE_ALPHA,	GL_LUMINANCE_ALPHA,GL_UNSIGNED_BYTE},
	{GL_LUMINANCE,			GL_LUMINANCE,		GL_UNSIGNED_BYTE},
	{GL_ALPHA,				GL_ALPHA,			GL_UNSIGNED_BYTE}
};

#ifdef _MSC_VER
__pragma(warning(suppress:4100 4101))
#else
__attribute__((unused))
#endif
static const Format format_es_int[] = {
	{GL_RGBA8UI,			GL_RGBA_INTEGER,	GL_UNSIGNED_BYTE},
	{GL_RGBA8I,			GL_RGBA_INTEGER,	GL_BYTE},
	{GL_RGBA16UI,			GL_RGBA_INTEGER,	GL_UNSIGNED_SHORT},
	{GL_RGBA16I,			GL_RGBA_INTEGER,	GL_SHORT},
	{GL_RGBA32UI,			GL_RGBA_INTEGER,	GL_UNSIGNED_INT},
	{GL_RGBA32I,			GL_RGBA_INTEGER,	GL_INT},
	{GL_RGB10_A2UI,		GL_RGBA_INTEGER,	GL_UNSIGNED_INT_2_10_10_10_REV},
	{GL_RGB8UI,			GL_RGB_INTEGER,	GL_UNSIGNED_BYTE},
	{GL_RGB8I,				GL_RGB_INTEGER,	GL_BYTE},
	{GL_RGB16UI,			GL_RGB_INTEGER,	GL_UNSIGNED_SHORT},
	{GL_RGB16I,			GL_RGB_INTEGER,	GL_SHORT},
	{GL_RGB32UI,			GL_RGB_INTEGER,	GL_UNSIGNED_INT},
	{GL_RGB32I,			GL_RGB_INTEGER,	GL_INT},
	{GL_RG8UI,				GL_RG_INTEGER,		GL_UNSIGNED_BYTE},
	{GL_RG8I,				GL_RG_INTEGER,		GL_BYTE},
	{GL_RG16UI,			GL_RG_INTEGER,		GL_UNSIGNED_SHORT},
	{GL_RG16I,				GL_RG_INTEGER,		GL_SHORT},
	{GL_RG32UI,			GL_RG_INTEGER,		GL_UNSIGNED_INT},
	{GL_RG32I,				GL_RG_INTEGER,		GL_INT},
	{GL_R8UI,				GL_RED_INTEGER,	GL_UNSIGNED_BYTE},
	{GL_R8I,				GL_RED_INTEGER,	GL_BYTE},
	{GL_R16UI,				GL_RED_INTEGER,	GL_UNSIGNED_SHORT},
	{GL_R16I,				GL_RED_INTEGER,	GL_SHORT},
	{GL_R32UI,				GL_RED_INTEGER,	GL_UNSIGNED_INT},
	{GL_R32I,				GL_RED_INTEGER,	GL_INT},
	{GL_DEPTH_COMPONENT24,GL_DEPTH_COMPONENT,GL_UNSIGNED_INT},
	{GL_DEPTH_COMPONENT16,GL_DEPTH_COMPONENT,GL_UNSIGNED_INT},
	{GL_DEPTH_COMPONENT16,GL_DEPTH_COMPONENT,GL_UNSIGNED_SHORT},
	{GL_DEPTH_COMPONENT32F,GL_DEPTH_COMPONENT,GL_FLOAT},
	{GL_DEPTH24_STENCIL8,GL_DEPTH_STENCIL,	GL_UNSIGNED_INT_24_8},
	{GL_DEPTH32F_STENCIL8,GL_DEPTH_STENCIL,	GL_FLOAT_32_UNSIGNED_INT_24_8_REV},
};

#ifdef _MSC_VER
__pragma(warning(suppress:4100 4101))
#else
__attribute__((unused))
#endif
static const Format format_core[] = {
	{GL_RGB,			GL_RGB,			GL_UNSIGNED_BYTE_3_3_2},
	{GL_RGB,			GL_RGB,			GL_UNSIGNED_BYTE_2_3_3_REV},
	{GL_RGB,			GL_RGB,			GL_UNSIGNED_SHORT_5_6_5},
	{GL_RGB,			GL_RGB,			GL_UNSIGNED_SHORT_5_6_5_REV},
	{GL_RGB,			GL_RGB,			GL_UNSIGNED_INT_10F_11F_11F_REV},
	{GL_RGB,			GL_RGB,			GL_UNSIGNED_INT_5_9_9_9_REV},
	{GL_RGBA,			GL_RGBA,		GL_UNSIGNED_SHORT_4_4_4_4},
	{GL_RGBA,			GL_RGBA,		GL_UNSIGNED_SHORT_4_4_4_4_REV},
	{GL_BGRA,			GL_BGRA,		GL_UNSIGNED_SHORT_4_4_4_4_REV},
	{GL_BGRA,			GL_BGRA,		GL_UNSIGNED_SHORT_4_4_4_4},
	{GL_RGBA,			GL_RGBA,		GL_UNSIGNED_SHORT_5_5_5_1},
	{GL_BGRA,			GL_BGRA,		GL_UNSIGNED_SHORT_5_5_5_1},
	{GL_RGBA,			GL_RGBA,		GL_UNSIGNED_SHORT_1_5_5_5_REV},
	{GL_BGRA,			GL_BGRA,		GL_UNSIGNED_SHORT_1_5_5_5_REV},
	{GL_RGBA,			GL_RGBA,		GL_UNSIGNED_INT_8_8_8_8},
	{GL_BGRA,			GL_BGRA,		GL_UNSIGNED_INT_8_8_8_8},
	{GL_RGBA,			GL_RGBA,		GL_UNSIGNED_INT_8_8_8_8_REV},
	{GL_BGRA,			GL_BGRA,		GL_UNSIGNED_INT_8_8_8_8_REV},
	{GL_RGBA,			GL_RGBA,		GL_UNSIGNED_INT_10_10_10_2},
	{GL_BGRA,			GL_BGRA,		GL_UNSIGNED_INT_10_10_10_2},
	{GL_RGBA,			GL_RGBA,		GL_UNSIGNED_INT_2_10_10_10_REV},
	{GL_BGRA,			GL_BGRA,		GL_UNSIGNED_INT_2_10_10_10_REV},
};

#ifdef _MSC_VER
__pragma(warning(suppress:4100 4101))
#else
__attribute__((unused))
#endif
static const Format format_core_int[] = {
	{GL_RGB_INTEGER,	GL_RGB_INTEGER,GL_UNSIGNED_BYTE_3_3_2},
	{GL_RGB_INTEGER,	GL_RGB_INTEGER,GL_UNSIGNED_BYTE_2_3_3_REV},
	{GL_RGB_INTEGER,	GL_RGB_INTEGER,GL_UNSIGNED_SHORT_5_6_5},
	{GL_RGB_INTEGER,	GL_RGB_INTEGER,GL_UNSIGNED_SHORT_5_6_5_REV},
	{GL_RGBA_INTEGER,	GL_RGBA_INTEGER,GL_UNSIGNED_SHORT_4_4_4_4},
	{GL_RGBA_INTEGER,	GL_RGBA_INTEGER,GL_UNSIGNED_SHORT_4_4_4_4_REV},
	{GL_BGRA_INTEGER,	GL_BGRA_INTEGER,GL_UNSIGNED_SHORT_4_4_4_4_REV},
	{GL_BGRA_INTEGER,	GL_BGRA_INTEGER,GL_UNSIGNED_SHORT_4_4_4_4},
	{GL_RGBA_INTEGER,GL_RGBA_INTEGER,GL_UNSIGNED_SHORT_5_5_5_1},
	{GL_BGRA_INTEGER,GL_BGRA_INTEGER,GL_UNSIGNED_SHORT_5_5_5_1},
	{GL_RGBA_INTEGER,GL_RGBA_INTEGER,GL_UNSIGNED_SHORT_1_5_5_5_REV},
	{GL_BGRA_INTEGER,GL_BGRA_INTEGER,GL_UNSIGNED_SHORT_1_5_5_5_REV},
	{GL_RGBA_INTEGER,GL_RGBA_INTEGER,GL_UNSIGNED_INT_8_8_8_8},
	{GL_BGRA_INTEGER,GL_BGRA_INTEGER,GL_UNSIGNED_INT_8_8_8_8},
	{GL_RGBA_INTEGER,GL_RGBA_INTEGER,GL_UNSIGNED_INT_8_8_8_8_REV},
	{GL_BGRA_INTEGER,GL_BGRA_INTEGER,GL_UNSIGNED_INT_8_8_8_8_REV},
	{GL_RGBA_INTEGER,GL_RGBA_INTEGER,GL_UNSIGNED_INT_10_10_10_2},
	{GL_BGRA_INTEGER,GL_BGRA_INTEGER,GL_UNSIGNED_INT_10_10_10_2},
	{GL_RGBA_INTEGER,GL_RGBA_INTEGER,GL_UNSIGNED_INT_2_10_10_10_REV},
	{GL_BGRA_INTEGER,GL_BGRA_INTEGER,GL_UNSIGNED_INT_2_10_10_10_REV},
	{GL_DEPTH_STENCIL,GL_DEPTH_STENCIL,GL_UNSIGNED_INT_24_8},
	{GL_DEPTH_STENCIL,GL_DEPTH_STENCIL,GL_FLOAT_32_UNSIGNED_INT_24_8_REV}
};

#ifdef _MSC_VER
__pragma(warning(suppress:4100 4101))
#else
__attribute__((unused))
#endif
static const Format formats_EXT_texture_type_2_10_10_10_REV[] = {
	{GL_RGBA,GL_RGBA,GL_UNSIGNED_INT_2_10_10_10_REV_EXT},
	{GL_RGB,GL_RGB,GL_UNSIGNED_INT_2_10_10_10_REV_EXT}
};

#ifdef _MSC_VER
__pragma(warning(suppress:4100 4101))
#else
__attribute__((unused))
#endif
static const Format formats_OES_required_internalformat[] = {
	{GL_RGB8_OES,GL_RGB,GL_UNSIGNED_INT_2_10_10_10_REV_EXT},
	{GL_RGB565,GL_RGB,GL_UNSIGNED_INT_2_10_10_10_REV_EXT}
};

const static Format *cur_format;
static uint32_t *readpixels;
static unsigned width, height, xoffset;
static GLuint tex;
#define BUF_LEN (4 * 1024 * 1024)

static void
pbo_download(unsigned count)
{
   unsigned i;
   for (i = 0; i < count; i++)
      glGetTextureSubImage(tex, 0, xoffset, 0, 0, width, height, 1, cur_format->format, cur_format->type, 4 * BUF_LEN, 0);
}

#define COLOR_RESET   "\033[0m"
#define COLOR_RED   "\033[31m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_YELLOW   "\033[1;33m"
#define COLOR_CYAN   "\033[1;36m"

static double
perf_run(perf_rate_func f, double base_rate)
{
   static unsigned test_index;
   test_index++;

   if (base_rate && selected_test_index != -1 && test_index != selected_test_index)
      return 0;

   double rate = perf_measure_cpu_rate(f, 0.5);
   if (!base_rate && selected_test_index != -1 && test_index != selected_test_index)
      return rate;
   double ratio = base_rate ? rate / base_rate : 1;

   const char *ratio_color = base_rate == 0 ? COLOR_RESET :
      ratio > 0.7 ? COLOR_GREEN :
      ratio > 0.4 ? COLOR_YELLOW : COLOR_RED;

   char space[128];
   memset(space, ' ', sizeof(space));
   space[56 - strlen(piglit_get_gl_enum_name(cur_format->internal_format)) - strlen(piglit_get_gl_enum_name(cur_format->type))] = 0;
   printf(" %3u,%s %s, %s%ux%u,       %s%5u%s,              %s%.1f%%%s\n",
          test_index, piglit_get_gl_enum_name(cur_format->internal_format), piglit_get_gl_enum_name(cur_format->type), space, width, height,
          color ? COLOR_CYAN : "",
          (unsigned)(rate),
          color ? COLOR_RESET : "",
          color ? ratio_color : "",
          100 * ratio,
          color ? COLOR_RESET : "");
   return rate;
}

static void
perf_pbo_variant(void)
{
   unsigned pot;
   double base_rate[12];
   const Format *r32f = NULL;

   for (unsigned i = 0; i < ARRAY_SIZE(format_es); i++) {
      if (format_es[i].internal_format == GL_R32F) {
         r32f = &format_es[i];
         break;
      }
   }
   assert(r32f);

   for (pot = 5; pot <= 10; pot++) {
      width = height = 1<<pot;
      xoffset = 0;
      cur_format = r32f;
      base_rate[pot] = perf_run(pbo_download, 0);
      for (unsigned i = 0; i < ARRAY_SIZE(format_es); i++) {
         cur_format = &format_es[i];
         if (cur_format == r32f)
            continue;
         perf_run(pbo_download, base_rate[pot]);
      }
   }
}

void
piglit_init(int argc, char **argv)
{
   piglit_require_gl_version(45);
   piglit_require_extension("GL_ARB_pixel_buffer_object");
   piglit_require_extension("GL_ARB_get_texture_sub_image");

   GLuint pbo;
   /* 1024x1024 2D texture */
   uint32_t *pixels = malloc(BUF_LEN);
   readpixels = malloc(4 * BUF_LEN);

   /* Init pixels with content */
   for (uint32_t y = 0; y < 1024; ++y)
      for (uint32_t x = 0; x < 1024; ++x) {
         uint32_t lo = y;
         uint32_t hi = x;
         uint32_t p = lo | (hi << 16);
         pixels[x + (1024 * y)] = p;
      }

   glGenTextures(1, &tex);
   glBindTexture(GL_TEXTURE_2D, tex);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 1024, 1024, 0, GL_RED, GL_FLOAT, pixels);

   glGenBuffers(1, &pbo);
   glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
   glBufferData(GL_PIXEL_PACK_BUFFER, 4 * BUF_LEN, readpixels, GL_STATIC_READ);
   glPixelStorei(GL_PACK_ALIGNMENT, 4);

   puts("   #, Test name,                                              size ,   GetTexSubImage2D/s, Difference vs the 1st");
   perf_pbo_variant();
   exit(0);
}

/** Called from test harness/main */
enum piglit_result
piglit_display(void)
{
   return PIGLIT_FAIL;
}
