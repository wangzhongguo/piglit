/*
 * Copyright © 2012 Intel Corporation
 * Copyright © 2020 Advanced Micro Devices, Inc.
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

#include "piglit-fbo.h"
using namespace piglit_util_fbo;

/**
 * The test draws an alpha gradient with alpha-to-coverage into an MSAA FBO,
 * and resolves it into the left or right half of the window.
 *
 * The left half has alpha-to-coverage dithering enabled, while the right half
 * has it disabled. Both halves should differ if MSAA is enabled to indicate
 * that dithering was applied.
 */

#define FB_SIZE 256

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 44;

	config.window_width = FB_SIZE * 2;
	config.window_height = FB_SIZE;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END


static Fbo ms_fbo;
static GLint num_samples;

static const char *vs =
	"#version 440\n"
        "layout(location = 0) in vec2 vertexPosition;\n"
	"void main()\n"
	"{\n"
	"  gl_Position = vec4(vertexPosition, 0.0, 1.0);\n"
	"}\n";

static const char *fs =
	"#version 440\n"
        "layout(location = 0) out vec4 color;\n"
        "void main()\n"
        "{\n"
	/* Draw an alpha gradient. */
	"   color = vec4(1, 1, 1, length(gl_FragCoord.xy / vec2(256 * 1.3)));\n"
        "}\n";

void
piglit_init(int argc, char **argv)
{
	char *endptr = NULL;

        if (argc == 2) {
		num_samples = strtol(argv[1], &endptr, 0);
	} else {
		printf("Usage: %s <num_samples>\n", argv[0]);
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_require_gl_version(44);
	piglit_require_extension("GL_NV_alpha_to_coverage_dither_control");

	/* Skip the test if num_samples > GL_MAX_SAMPLES */
	GLint max_samples;
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
	if (num_samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);

	FboConfig temp_config = FboConfig(num_samples, FB_SIZE, FB_SIZE);
        temp_config.num_tex_attachments = 1;
	ms_fbo.setup(temp_config);

	glUseProgram(piglit_build_simple_program(vs, fs));

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
}

void
draw_pattern(bool enable, int xoffset)
{
	glBindFramebuffer(GL_FRAMEBUFFER, ms_fbo.handle);
	ms_fbo.set_viewport();

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glAlphaToCoverageDitherControlNV(enable ? GL_ALPHA_TO_COVERAGE_DITHER_ENABLE_NV :
						  GL_ALPHA_TO_COVERAGE_DITHER_DISABLE_NV);
	piglit_draw_rect(-1, -1, 2, 2);

	/* Resolve MSAA to the left or right half of the window system framebuffer. */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, ms_fbo.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, FB_SIZE, FB_SIZE,
			  xoffset, 0, xoffset + FB_SIZE, FB_SIZE,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

enum piglit_result
piglit_display()
{
	/* Draw without dithering to the right half, and with dithering to the left half. */
	draw_pattern(false, FB_SIZE);
	draw_pattern(true, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);

	/* Only the left half has dithering enabled, so it's not equal if dithering is enabled. */
	bool equal = piglit_probe_rects_equal(0, 0, FB_SIZE, 0, FB_SIZE, FB_SIZE, GL_RGB);

	/* If MSAA is enabled, both halves should be different. Otherwise they should be equal
	 * (no dithering).
	 */
	bool pass = num_samples > 0 ? !equal : equal;

	if (pass && !equal)
		puts("Ignore the probe messages. The pixels should differ to pass this test.");

	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
