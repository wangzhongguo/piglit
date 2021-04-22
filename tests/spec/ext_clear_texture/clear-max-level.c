/*
 * Copyright (c) 2015 VMware, Inc.
 * Copyright (c) 2021 Collabora Ltd
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

/** @file clear-max-level.c
 *
 * Exercise an nvidia driver bug where clearing a texture mipmap level
 * fails if the level is >= GL_TEXTURE_MAX_LEVEL.
 *
 * BTW, glCopyImageSubData() seems to also fail if the src/dest mipmap level
 * is >= GL_TEXTURE_MAX_LEVEL.
 */

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_es_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END

#define width 32
#define height 32
#define bufsize (width * height * 4)
static bool
test_clear(GLint maxLevel)
{
	const GLenum target = GL_TEXTURE_2D;
	const int numLevels = 3;
	const GLenum texInternalFormat = GL_RGBA8;

	int l;
	bool pass = true;
	GLuint tex;

	/* Create texture */
	glGenTextures(1, &tex);
	glBindTexture(target, tex);
	glTexStorage2D(target,
		       numLevels,
		       texInternalFormat,
		       width,
		       height);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, maxLevel);

	/* clear levels to unique values */
	for (l = 0; l < numLevels; ++l) {
		GLubyte value[4];
		value[0] = value[1] = value[2] = value[3] = (unsigned char)(256 * l * 0.125);
		glClearTexImageEXT(tex, l, GL_RGBA, GL_UNSIGNED_BYTE, value);
		if (!piglit_check_gl_error(GL_NO_ERROR))
			return false;
	}

	/* check results */
	for (l = 0; l < numLevels; l++) {
		float e = l * 0.125;
		float expected[4] = {e, e, e, e};
		int w = MAX2(1, width >> l);
		int h = MAX2(1, height >> l);

		pass &= piglit_probe_texel_rect_rgba(GL_TEXTURE_2D, l, 0, 0,
						     w, h, expected);
	}

	glDeleteTextures(1, &tex);

	return pass;
}


void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	piglit_require_extension("GL_EXT_clear_texture");

	pass = test_clear(0) && pass;
	pass = test_clear(1) && pass;
	pass = test_clear(2) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	/* unused */
	return PIGLIT_FAIL;
}
