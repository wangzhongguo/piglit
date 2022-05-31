/*
 * Copyright (c) 2014 VMware, Inc.
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** @file explicit-offset-bufferstorage.c
 *
 * This is a copy of the arb_uniform_buffer_object bufferstorage test updated
 * to make use of explicit offsets.
 *
 * Test rendering with UBOs.  We draw four squares with different
 * positions, sizes, rotations and colors where those parameters come
 * from UBOs. Same as rendering.c, except that the UBOs are
 * persistently mapped.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static const char vert_shader_text[] =
	"#version 150\n"
	"#extension GL_ARB_enhanced_layouts : require\n"
	"#extension GL_ARB_uniform_buffer_object : require\n"
	"\n"
	"in vec4 piglit_vertex;\n"
	"\n"
	"layout(std140) uniform;\n"
	"uniform ub_pos_size {\n"
	"  layout(offset = 8) vec2 pos;\n"
	"  layout(offset = 24) float size;\n"
	"};\n"
	"uniform ub_rot {float rotation; };\n"
	"\n"
	"void main()\n"
	"{\n"
	"   mat2 m;\n"
	"   m[0][0] = m[1][1] = cos(rotation); \n"
	"   m[0][1] = sin(rotation); \n"
	"   m[1][0] = -m[0][1]; \n"
	"   gl_Position.xy = m * piglit_vertex.xy * vec2(size) + pos;\n"
	"   gl_Position.zw = vec2(0, 1);\n"
	"}\n";

static const char frag_shader_text[] =
	"#version 150\n"
	"#extension GL_ARB_enhanced_layouts : require\n"
	"#extension GL_ARB_uniform_buffer_object : require\n"
	"\n"
	"layout(std140) uniform;\n"
	"uniform ub_color {\n"
	"  layout(offset = 0) vec4 color;\n"
	"  layout(offset = 20) float color_scale;\n"
	"} named_ub;\n"
	"\n"
	"void main()\n"
	"{\n"
	"   gl_FragColor = named_ub.color * named_ub.color_scale;\n"
	"}\n";

#define NUM_SQUARES 4
#define NUM_UBOS 3

/* Square positions and sizes */
static const float pos_size[NUM_SQUARES][7] = {
	{ 0.0, 0.0, -0.5, -0.5, 0.0, 0.0, 0.1 },
	{ 0.0, 0.0,  0.5, -0.5, 0.0, 0.0, 0.2 },
	{ 0.0, 0.0, -0.5,  0.5, 0.0, 0.0, 0.3 },
	{ 0.0, 0.0,  0.5,  0.5, 0.0, 0.0, 0.4 }
};

/* Square color and color_scales */
static const float color[NUM_SQUARES][8] = {
	{ 2.0, 0.0, 0.0, 1.0, 0.0, 0.50, 0.0, 0.0 },
	{ 0.0, 4.0, 0.0, 1.0, 0.0, 0.25, 0.0, 0.0 },
	{ 0.0, 0.0, 5.0, 1.0, 0.0, 0.20, 0.0, 0.0 },
	{ 0.2, 0.2, 0.2, 0.2, 0.0, 5.00, 0.0, 0.0 }
};

/* Square rotations */
static const float rotation[NUM_SQUARES] = {
	0.0,
	0.1,
	0.2,
	0.3
};

static GLuint prog;
static GLuint buffers[NUM_UBOS];
static void *ubos[NUM_UBOS];


static void
setup_ubos(void)
{
	static const char *names[NUM_UBOS] = {
		"ub_pos_size",
		"ub_color",
		"ub_rot"
	};
	int i;

	glGenBuffers(NUM_UBOS, buffers);

	for (i = 0; i < NUM_UBOS; i++) {
		GLint index, size;

		/* query UBO index */
		index = glGetUniformBlockIndex(prog, names[i]);

		/* query UBO size */
		glGetActiveUniformBlockiv(prog, index,
					  GL_UNIFORM_BLOCK_DATA_SIZE, &size);

		printf("UBO %s: index = %d, size = %d\n",
		       names[i], index, size);

		/* Allocate UBO */
		glBindBuffer(GL_UNIFORM_BUFFER, buffers[i]);
		glBufferStorage(GL_UNIFORM_BUFFER, size, NULL,
				GL_MAP_WRITE_BIT |
				GL_MAP_PERSISTENT_BIT |
				GL_MAP_COHERENT_BIT |
				GL_DYNAMIC_STORAGE_BIT);

		piglit_check_gl_error(GL_NO_ERROR);

		ubos[i] = glMapBufferRange(GL_UNIFORM_BUFFER, 0, size,
					   GL_MAP_WRITE_BIT |
					   GL_MAP_PERSISTENT_BIT |
					   GL_MAP_COHERENT_BIT);

		piglit_check_gl_error(GL_NO_ERROR);

		if (!ubos[i])
			piglit_report_result(PIGLIT_FAIL);

		/* Attach UBO */
		glBindBufferBase(GL_UNIFORM_BUFFER, i, buffers[i]);
		glUniformBlockBinding(prog, index, i);

		if (!piglit_check_gl_error(GL_NO_ERROR))
			piglit_report_result(PIGLIT_FAIL);
	}
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_enhanced_layouts");
	piglit_require_extension("GL_ARB_buffer_storage");

	prog = piglit_build_simple_program(vert_shader_text, frag_shader_text);
	assert(prog);
	glUseProgram(prog);

	setup_ubos();

	glClearColor(0.2, 0.2, 0.2, 0.2);
}


static bool
probe(int x, int y, int color_index)
{
	float expected[4];

	/* mul color by color_scale */
	expected[0] = color[color_index][0] * color[color_index][5];
	expected[1] = color[color_index][1] * color[color_index][5];
	expected[2] = color[color_index][2] * color[color_index][5];
	expected[3] = color[color_index][3] * color[color_index][5];

	return piglit_probe_pixel_rgba(x, y, expected);
}


enum piglit_result
piglit_display(void)
{
	GLsync fence;
	bool pass = true;
	int x0 = piglit_width / 4;
	int x1 = piglit_width * 3 / 4;
	int y0 = piglit_height / 4;
	int y1 = piglit_height * 3 / 4;
	int i;

	glViewport(0, 0, piglit_width, piglit_height);

	glClear(GL_COLOR_BUFFER_BIT);

	for (i = 0; i < NUM_SQUARES; i++) {
		/* Wait for any previous rendering to finish before
		 * updating the UBOs
		 */
		fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT,
				 GL_TIMEOUT_IGNORED);

		/* Load UBO data */
		memcpy(ubos[0], pos_size[i], sizeof(pos_size[0]));
		memcpy(ubos[1], color[i], sizeof(color[0]));
		memcpy(ubos[2], &rotation[i], sizeof(rotation[0]));

		piglit_draw_rect(-1, -1, 2, 2);
	}

	pass = probe(x0, y0, 0) && pass;
	pass = probe(x1, y0, 1) && pass;
	pass = probe(x0, y1, 2) && pass;
	pass = probe(x1, y1, 3) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
