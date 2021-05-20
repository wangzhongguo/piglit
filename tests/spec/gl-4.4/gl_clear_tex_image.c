/*
 * Copyright © 2021 Advanced Micro Devices, Inc.
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
 * This test verifies glClearTexImage behavior, and reproduce the bug reported
 * at https://gitlab.freedesktop.org/mesa/mesa/-/issues/3646.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 44;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	int tex[] = {
		piglit_rgbw_texture_1d(),
		piglit_rgbw_texture(GL_RGBA, 2, 2, false, false, GL_FLOAT),
		piglit_rgbw_texture_3d(),
		piglit_array_texture(GL_TEXTURE_1D_ARRAY, GL_RGBA, 2, 1, 2, false),
		piglit_array_texture(GL_TEXTURE_2D_ARRAY, GL_RGBA, 2, 2, 2, false)
	};

	const float colors[] = {
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f, 1.0f,
	};

	for (int i = 0; i < ARRAY_SIZE(tex); i++) {
		glClearTexImage(tex[i], 0, GL_RGBA, GL_FLOAT, &colors[4 * i]);
	}

	glBindTexture(GL_TEXTURE_1D, tex[0]);
	pass &= piglit_probe_texel_volume_rgba(
		GL_TEXTURE_1D, 0, 0, 0, 0, 2, 1, 1, &colors[0]);

	glBindTexture(GL_TEXTURE_2D, tex[1]);
	pass &= piglit_probe_texel_volume_rgba(
		GL_TEXTURE_2D, 0, 0, 0, 0, 2, 2, 1, &colors[4]);

	glBindTexture(GL_TEXTURE_3D, tex[2]);
	pass &= piglit_probe_texel_volume_rgba(
		GL_TEXTURE_3D, 0, 0, 0, 0, 2, 2, 2, &colors[8]);

	glBindTexture(GL_TEXTURE_1D_ARRAY, tex[3]);
	pass &= piglit_probe_texel_volume_rgba(
		GL_TEXTURE_1D_ARRAY, 0, 0, 0, 0, 2, 2, 1, &colors[12]);

	glBindTexture(GL_TEXTURE_2D_ARRAY, tex[4]);
	pass &= piglit_probe_texel_volume_rgba(
		GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, 2, 2, 2, &colors[16]);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
