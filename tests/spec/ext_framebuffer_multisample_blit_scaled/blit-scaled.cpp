/*
 * Copyright © 2013 Intel Corporation
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

/** \file blit-scaled.cpp
 *
 * This test verifies the accuracy of scaled blitting from a multisampled
 * buffer to a single-sampled buffer by comparing the output from following
 * rendering scenarios:
 * 1. Scaled blit using EXT_multisample_framebuffer_blit_scaled.
 * 2. Scaled blit using glsl shader program.
 *
 * Note: This test is specific to Intel's implementation of extension
 * EXT_multisample_framebuffer_blit_scaled and may not produce expected
 * results on other hardware. Currently test passes with all of the scaling
 * factors between 0.1 to 2.5 on Intel's i965 drivers and NVIDIA's proprietary
 * linux drivers.
 */

#include "piglit-test-pattern.h"
#include "piglit-fbo.h"
using namespace piglit_util_fbo;
using namespace piglit_util_test_pattern;

const int pattern_width = 258; const int pattern_height = 258;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 30;

	config.window_width = pattern_width * 2;
	config.window_height = pattern_height;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static int samples;
static TestPattern *test_pattern;
static unsigned prog, vao, vertex_buf;
const float srcX0 = 6, srcY0 = 7, dstX0 = 0, dstY0 = 0;
const float srcX1 = pattern_width / 2, srcY1 = pattern_height / 2;
static Fbo multisampled_tex, multisampled_fbo, singlesampled_fbo;

static void
print_usage_and_exit(const char *prog_name)
{
	printf("Usage: %s <num_samples> [array]\n"
	       "    array: Use GL_TEXTURE_2D_MULTISAMPLE_ARRAY instead of GL_TEXTURE_2D_MULTISAMPLE\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

void
compile_shader(GLenum target)
{
	static const char *vert =
		"#version 130\n"
		"in vec2 piglit_vertex;\n"
		"in vec2 piglit_texcoord;\n"
		"out vec2 texCoords;\n"
		"void main()\n"
		"{\n"
		"  gl_Position = vec4(piglit_vertex, 0.0, 1.0);\n"
		"  texCoords = piglit_texcoord;\n"
		"}\n";
	/* Bilinear filtering of samples using shader program */
	static const char *frag_template =
		"#version 130\n"
		"#extension GL_ARB_texture_multisample : require\n"
		"uniform %s texSampler;\n"
		"uniform float src_width, src_height;\n"
		"in vec2 texCoords;\n"
		"out vec4 out_color;\n"
		"void main()\n"
		"{\n"
		"%s"
		"  vec2 interp;\n"
		"  const vec2 scale = vec2(%ff, %ff);\n"
		"  const vec2 scale_inv = vec2(%ff, %ff);\n"
		"  const vec2 s_0_offset = vec2(%ff, %ff);\n"
		"  vec2 s_0_coord, s_1_coord, s_2_coord, s_3_coord;\n"
		"  vec4 s_0_color, s_1_color, s_2_color, s_3_color;\n"
		"  vec4 x_0_color, x_1_color;\n"
		"  vec2 tex_coord = texCoords - s_0_offset;\n"
		"\n"
		"  tex_coord *= scale;\n"
		"  tex_coord.x = clamp(tex_coord.x, 0.0f, scale.x * src_width - 1.0f);\n"
		"  tex_coord.y = clamp(tex_coord.y, 0.0f, scale.y * src_height - 1.0f);\n"
		"  interp = fract(tex_coord);\n"
		"  tex_coord = ivec2(tex_coord) * scale_inv;\n"
		"\n"
		"  /* Compute the sample coordinates used for filtering. */\n"
		"  s_0_coord = tex_coord;\n"
		"  s_1_coord = tex_coord + vec2(scale_inv.x, 0.0f);\n"
		"  s_2_coord = tex_coord + vec2(0.0f, scale_inv.y);\n"
		"  s_3_coord = tex_coord + vec2(scale_inv.x, scale_inv.y);\n"
		"\n"
		"  /* Fetch sample color values. */\n"
		"%s"
		"  s_0_color = TEXEL_FETCH(s_0_coord)\n"
		"  s_1_color = TEXEL_FETCH(s_1_coord)\n"
		"  s_2_color = TEXEL_FETCH(s_2_coord)\n"
		"  s_3_color = TEXEL_FETCH(s_3_coord)\n"
		"#undef TEXEL_FETCH\n"
		"\n"
		"  /* Do bilinear filtering on sample colors. */\n"
		"  x_0_color = mix(s_0_color, s_1_color, interp.x);\n"
		"  x_1_color = mix(s_2_color, s_3_color, interp.x);\n"
		"  out_color = mix(x_0_color, x_1_color, interp.y);\n"
		"}\n";

	char* frag, *texel_fetch_macro;
	const char*sample_number, *sample_map = "";
	float x_scale, y_scale;

	if (samples == 16)
		x_scale = 4;
	else
		x_scale = 2;

	y_scale = samples / x_scale;

	/* Below switch is used to setup the shader expression, which computes
	 * sample index and map it to to a sample number on Intel hardware.
	 * Sample index layout shows the numbering of slots in a rectangular
	 * grid of samples with in a pixel. Sample number layout shows the
	 * rectangular grid of samples roughly corresponding to the real sample
	 * locations with in a pixel. Sample number layout matches the sample
	 * index layout in case of 2X and 4x MSAA, but they are different in
	 * case of 8X MSAA.
	 *
	 * 2X MSAA sample index / number layout
	 *           ---------
	 *           | 0 | 1 |
	 *           ---------
	 *
	 * 4X MSAA sample index / number layout
	 *           ---------
	 *           | 0 | 1 |
	 *           ---------
	 *           | 2 | 3 |
	 *           ---------
	 *
	 * 8X MSAA sample index layout    8x MSAA sample number layout
	 *           ---------                      ---------
	 *           | 0 | 1 |                      | 3 | 7 |
	 *           ---------                      ---------
	 *           | 2 | 3 |                      | 5 | 0 |
	 *           ---------                      ---------
	 *           | 4 | 5 |                      | 1 | 2 |
	 *           ---------                      ---------
	 *           | 6 | 7 |                      | 4 | 6 |
	 *           ---------                      ---------
	 *
	 * 16X MSAA sample index layout  16x MSAA sample number layout
	 *         -----------------            -----------------
	 *         | 0 | 1 | 2 | 3 |            |15 |10 | 9 | 7 |
	 *         -----------------            -----------------
	 *         | 4 | 5 | 6 | 7 |            | 4 | 1 | 3 |13 |
	 *         -----------------            -----------------
	 *         | 8 | 9 |10 |11 |            |12 | 2 | 0 | 6 |
	 *         -----------------            -----------------
	 *         |12 |13 |14 |15 |            |11 | 8 | 5 |14 |
	 *         -----------------            -----------------
	 */
	switch(samples) {
	case 2:
		sample_number =  "int(2 * fract(coord.x))";
		break;
	case 4:
		sample_number =  "int(2 * fract(coord.x) + 4 * fract(coord.y))";
	break;
	case 8:
		sample_map = "  const int sample_map[8] = int[8](3, 7, 5, 0, 1, 2, 4, 6);\n";
		sample_number = "sample_map[int(2 * fract(coord.x) + 8 * fract(coord.y))]";
		break;
	case 16:
		sample_map = "  const int sample_map[16] = int[16](15, 10, 9, 7, 4, 1, 3, 13,\n"
			     "                                     12, 2, 0, 6, 11, 8, 5, 14);\n";
		sample_number = "sample_map[int(4 * fract(coord.x) + 16 * fract(coord.y))]";
		break;
	default:
		printf("Unsupported sample count %d\n", samples);
		piglit_report_result(PIGLIT_SKIP);
	}

	char const*target_string;
	if (target == GL_TEXTURE_2D_MULTISAMPLE) {
		(void)!asprintf(&texel_fetch_macro,
			 "#define TEXEL_FETCH(coord) texelFetch(texSampler, "
			 "ivec2(coord), %s);\n",
			 sample_number);
		target_string = "sampler2DMS";
	} else {
		/* The layer for the array texture is hardcoded to 1. */
		(void)!asprintf(&texel_fetch_macro,
			 "#define TEXEL_FETCH(coord) texelFetch(texSampler, "
			 "ivec3(coord, 1), %s);\n",
			 sample_number);
		target_string = "sampler2DMSArray";
	}

	(void)!asprintf(&frag, frag_template, target_string, sample_map,
		 x_scale, y_scale,
		 1.0f / x_scale, 1.0f / y_scale,
		 0.5f / x_scale, 0.5f / y_scale,
		 texel_fetch_macro);

	/* Compile program */
	prog = piglit_build_simple_program(vert, frag);

	/* Set up vertex array object */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Set up vertex input buffer */
	glGenBuffers(1, &vertex_buf);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float),
			      (void *) 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float),
			      (void *) (2*sizeof(float)));

	/* Set up element input buffer to tessellate a quad into
	 * triangles
	 */
	unsigned int indices[6] = { 0, 1, 2, 0, 2, 3 };
	GLuint element_buf;
	glGenBuffers(1, &element_buf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
		     GL_STATIC_DRAW);
	free(frag);
	free(texel_fetch_macro);
}

void
ms_blit_scaled_glsl(const Fbo *src_fbo)
{
	float vertex_data[4][4] = {
		{ -1, -1, srcX0, srcY0 },
		{ -1,  1, srcX0, srcY1 },
		{  1,  1, srcX1, srcY1 },
		{  1, -1, srcX1, srcY0 }};

	const GLenum target = src_fbo->config.layers == 0
		? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D_MULTISAMPLE_ARRAY;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(target, src_fbo->color_tex[0]);
	glUseProgram(prog);
	glBindVertexArray(vao);

	/* Set up uniforms */
	glUseProgram(prog);
	glUniform1i(glGetUniformLocation(prog, "texSampler"), 0);
	glUniform1f(glGetUniformLocation(prog, "src_width"),
		    multisampled_fbo.config.width);
	glUniform1f(glGetUniformLocation(prog, "src_height"),
		    multisampled_fbo.config.height);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data,
		     GL_STREAM_DRAW);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *) 0);
}

void
piglit_init(int argc, char **argv)
{
	if (argc < 2 || argc > 3)
		print_usage_and_exit(argv[0]);

	/* 1st arg: num_samples */
	char *endptr = NULL;
	int num_samples = strtol(argv[1], &endptr, 0);
	if (endptr != argv[1] + strlen(argv[1]))
		print_usage_and_exit(argv[0]);

        GLenum texture_target = GL_TEXTURE_2D_MULTISAMPLE;
	if (argc > 2) {
		if (strcmp(argv[2], "array") == 0)
			texture_target = GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
		else
			print_usage_and_exit(argv[0]);
	}

	piglit_require_extension("GL_ARB_vertex_array_object");
	piglit_require_extension("GL_EXT_framebuffer_multisample_blit_scaled");

	/* Skip the test if num_samples > GL_MAX_SAMPLES */
	GLint max_samples;
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
	if (num_samples == 0 || num_samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);

	singlesampled_fbo.setup(FboConfig(0,
					  2 * pattern_width,
					  pattern_height));
	/* Create two multisample FBOs with same dimensions and sample count
	 * but different color attachment types.
	 */
	FboConfig msConfig(num_samples, pattern_width, pattern_height);
	multisampled_fbo.setup(msConfig);
	msConfig.num_tex_attachments = 1;
	msConfig.num_rb_attachments = 0; /* default value is 1 */
	if (texture_target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY) {
		msConfig.layers = 2;
		msConfig.attachment_layer = 1;
	}
	multisampled_tex.setup(msConfig);

	/* Implementation might not create a buffer with requested sample
	 * count. So, query the actual sample count of buffer.
	 */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisampled_tex.handle);
	glGetIntegerv(GL_SAMPLES, &samples);

	test_pattern = new Triangles();
	test_pattern->compile();

	compile_shader(texture_target);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	puts("Testing Intel-specific behavior.");
}

bool test_ms_blit_scaled(const Fbo& ms_fbo)
{
	GLfloat scale;
	bool pass = true, result = true;

	/* Draw the test pattern into the framebuffer with texture
	 * attachment.
	 */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisampled_tex.handle);
	glViewport(0, 0, srcX1, srcY1);
	glClear(GL_COLOR_BUFFER_BIT);
	test_pattern->draw(TestPattern::no_projection);

	if (ms_fbo.config.num_tex_attachments == 0) {
		/* Blit the framebuffer with multisample texture attachment
		 * into the framebuffer with multisample renderbuffer
		 * attachment.
		 */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampled_tex.handle);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisampled_fbo.handle);
		glClear(GL_COLOR_BUFFER_BIT);
		glBlitFramebuffer(0, 0,
				  multisampled_tex.config.width,
				  multisampled_tex.config.height,
				  0, 0,
				  multisampled_tex.config.width,
				  multisampled_tex.config.height,
				  GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	for (scale = 0.1; scale < 2.5f; scale += 0.1) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
		glClear(GL_COLOR_BUFFER_BIT);

		/* Do scaled resolve of multisampled_fbo to left half of
		 * singlesampled_fbo.
		 */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, ms_fbo.handle);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, singlesampled_fbo.handle);
                glClearColor(0.0, 1.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
                glClearColor(0.0, 0.0, 0.0, 0.0);
		glEnable(GL_SCISSOR_TEST);
		glScissor(0, 0, pattern_width, pattern_height);
		glBlitFramebuffer(srcX0, srcY0,
				  srcX1, srcY1,
				  dstX0, dstY0,
				  dstX0 + srcX1 * scale, dstY0 + srcY1 * scale,
				  GL_COLOR_BUFFER_BIT,
				  GL_SCALED_RESOLVE_FASTEST_EXT);
		glDisable(GL_SCISSOR_TEST);

		/* Use multisampled texture to draw in to right half of scaled
		 * single-sampled buffer using shader program.
		 */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampled_tex.handle);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, singlesampled_fbo.handle);
		glViewport(pattern_width + dstX0, dstY0, srcX1 * scale, srcY1 * scale);
		ms_blit_scaled_glsl(&multisampled_tex);

		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
                glBindFramebuffer(GL_READ_FRAMEBUFFER, singlesampled_fbo.handle);
                result = piglit_probe_rect_halves_equal_rgba(0, 0,
                                                           piglit_width,
                                                           piglit_height);
		pass = result && pass;

		glBindFramebuffer(GL_READ_FRAMEBUFFER, singlesampled_fbo.handle);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
		glBlitFramebuffer(0, 0, 2 * pattern_width, piglit_height,
				  0, 0, 2 * pattern_width, piglit_height,
				  GL_COLOR_BUFFER_BIT, GL_NEAREST);
		piglit_present_results();
		printf("MS attachment = %12s, scale = %f, result = %s\n",
		       ms_fbo.config.num_tex_attachments > 0 ?
		       "TEXTURE" :
		       "RENDERBUFFER",
		       scale, result ? "pass" : "fail");
	}
	return pass;
}

enum piglit_result
piglit_display()
{
	bool pass = true;
	printf("Left Image: multisample scaled blit using extension.\n"
	       "Right Image: multisample scaled blit using shader program.\n");
	pass = test_ms_blit_scaled(multisampled_tex) && pass;

	/* In a full piglit run, the FBO test does not need to be done for
	 * both the array and non-array version of the test.  Just do it for
	 * the non-array run to match the old behavior.
	 */
	if (multisampled_tex.config.layers == 0)
		pass = test_ms_blit_scaled(multisampled_fbo) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_WARN;
}
