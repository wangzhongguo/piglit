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

/*
 * Tests for exercising llvmpipe linear optimizations.
 *
 * Authors:
 *    James Benton
 */

#include "piglit-util-gl.h"

#include "utils/framebuffer.h"
#include "utils/glenum.h"

#include "glsl/shader.h"
#include "tests/random.h"

#include "version.h"

#include <time.h>
#include <iostream>

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 256;
	config.window_height = 256;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

Random gRandom;
FrameBuffer gFrameBuffer;

bool RandomTest::enabled = true;

int random_test_count = 10000;

bool test_perform_next = false;
std::vector<Test*> tests;
std::vector<Test*>::iterator test_current;

enum piglit_result
piglit_display(void)
{
   bool pass = true;

   gFrameBuffer.bind();
   gFrameBuffer.tolerance(piglit_tolerance);
   piglit_ortho_projection(gFrameBuffer.width(), gFrameBuffer.height(), GL_FALSE);

   if (piglit_automatic) {
      for (std::vector<Test*>::iterator itr = tests.begin(); itr != tests.end(); ++itr) {
         Test* test = *itr;
         pass &= test->run();
      }

      RandomTest rnd;
      for (int i = 0; i < random_test_count; ++i)
         pass &= rnd.run();
   } else {
      if (test_perform_next) {
         test_current++;

         if (test_current == tests.end())
            test_current = tests.begin();
      }

      Test* test = *test_current;
      pass = test->run();

      gFrameBuffer.unbind();

      glUseProgram(0);
      glDisable(GL_BLEND);
      glDisable(GL_ALPHA_TEST);
      glViewport(0, 0, piglit_width, piglit_height);

      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
      glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,   GL_REPLACE);
      glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);

      glColor4f(1, 1, 1, 1);
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, gFrameBuffer.texture());

      piglit_draw_rect_tex(0, 0, piglit_width, piglit_height, 0, 0, 1, 1);

      glDisable(GL_TEXTURE_2D);
      piglit_swap_buffers();

      test_perform_next = false;
   }

   fflush(stdout);
   fflush(stderr);

   return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void fbo_formats_key_func(unsigned char key, int x, int y)
{
   if (key == 'n') {
      test_perform_next = true;
   }

   piglit_escape_exit_key(key, x, y);
}

void piglit_init(int argc, char** argv)
{
   std::cout.precision(9);
   std::cout << "git head: " << GIT_HEAD << std::endl;

   gFrameBuffer.setSize(piglit_width, piglit_height);
   gRandom.setImplementation(new Mersenne(0xbeefface));

   for (int i = 1; i < argc; ++i) {
      if (strcmp(argv[i], "-rgba32f") == 0) {
         gFrameBuffer.setFormat(GL_RGBA);
         gFrameBuffer.setInternalFormat(GL_RGBA32F);
         gFrameBuffer.setType(GL_FLOAT);
      } else if (strcmp(argv[i], "-clamped") == 0) {
         gFrameBuffer.setClamped(true);
      } else if (strcmp(argv[i], "-unclamped") == 0) {
         gFrameBuffer.setClamped(false);
      } else if (i + 1 < argc) {
         if (strcmp(argv[i], "-test") == 0) {
            tests.push_back(new RandomTest(argv[++i]));
         } else if (strcmp(argv[i], "-count") == 0) {
            random_test_count = atoi(argv[++i]);
         } else if (strcmp(argv[i], "-type") == 0) {
            gFrameBuffer.setType(glStringToEnum(argv[++i]));
         } else if (strcmp(argv[i], "-internalformat") == 0) {
            gFrameBuffer.setInternalFormat(glStringToEnum(argv[++i]));
         } else if (strcmp(argv[i], "-format") == 0) {
            gFrameBuffer.setFormat(glStringToEnum(argv[++i]));
         }
      }
   }

   if (gFrameBuffer.type() != GL_FLOAT && gFrameBuffer.type() != GL_HALF_FLOAT && !gFrameBuffer.clamped()) {
      gFrameBuffer.setClamped(true);
   }

   piglit_require_extension("GL_EXT_framebuffer_object");
   piglit_require_extension("GL_ARB_texture_env_combine");

   piglit_require_vertex_shader();
   piglit_require_fragment_shader();

   piglit_set_keyboard_func(fbo_formats_key_func);

   glDisable(GL_DITHER);

   if (RandomTest::enabled) {
      if (!piglit_automatic)
         tests.push_back(new RandomTest());
   }

   test_current = tests.begin();
}
