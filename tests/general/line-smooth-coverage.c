/*
 * Copyright © 2022 Collabora Ltd
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
 *
 * Authors:
 *    Erik Faye-Lund <erik.faye-lund@collabora.com>
 *
 */

/*
 * Test that the coverage of multiple line-segments split at various positions
 * adds up to one.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 300;
	config.window_height = 100;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glColor4f(0.5, 0.5, 0.5, 1.0);
	glEnable(GL_LINE_SMOOTH);
	/* GL AA lines produce an alpha value */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glLineWidth(3.0f);

	int y;
	for (y = 0; y * 4 < piglit_height; y++) {
		float seg_width = 0.25 + y * 0.25;
		float x1;
		for (x1 = 0; x1 < piglit_width; x1 += seg_width) {
			float x2 = x1 + seg_width;

			glBegin(GL_LINES);
			glVertex2f(x1, 1.5 + y * 4);
			glVertex2f(x2, 1.5 + y * 4);
			glEnd();
		}
	}

	GLboolean pass = GL_TRUE;
	for (y = 1; y < piglit_height; y += 4) {
		float expected[4] = { 0.5, 0.5, 0.5, 1.0 };
		pass = pass && piglit_probe_rect_rgb(0, y, piglit_width, 1, expected);
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
