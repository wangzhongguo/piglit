/*
 * Copyright (c) 2014 Intel Corporation
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

#include "piglit-util-gl.h"

/** @file stencil.c
 *
 * A test of using glClearTexSubImage to clear a depth texture.
 * Because on GLES the depth texture can't be read back directly,
 * The result is checked by drawing to the framebuffer with the depth test
 * enabled so that the depth texture content defines the result.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 31;

        config.window_width = 64;
        config.window_height = 64;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END


GLint in_color_loc = -1;
GLint in_depth_loc = -1;

static void
init_program()
{
	GLuint prog;

	static const char vs_source[] =
		"#version 300 es\n"
		"in mediump vec2 piglit_vertex;\n"
		"uniform vec2 fb_size;\n"
		"uniform float in_depth;\n"
		"\n"
		"void main()\n"
		"{\n"
		"        gl_Position = vec4(piglit_vertex * 2.0 /\n"
		"                           fb_size - 1.0,\n"
		"                           in_depth, 1.0);\n"
		"}\n";

	const char fs_source[] = {
		"#version 300 es\n"
		"out mediump vec4 color;\n"
		"uniform mediump vec4 in_color;\n"
		"\n"
		"void main()\n"
		"{\n"
		"        color = in_color;\n"
		"}\n"};

	prog = piglit_build_simple_program(vs_source, fs_source);

	glUseProgram(prog);

	GLuint uniform = glGetUniformLocation(prog, "fb_size");
	glUniform2f(uniform, piglit_width, piglit_height);

	in_depth_loc = glGetUniformLocation(prog, "in_depth");
	in_color_loc = glGetUniformLocation(prog, "in_color");

	assert(in_color_loc >= 0);
	assert(in_depth_loc >= 0);

}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_clear_texture");
	init_program();

}

static void
draw_rect(const GLfloat color[], GLfloat depth)
{
	glDepthFunc(GL_GREATER);
	glUniform4fv(in_color_loc, 1, color);
	glUniform1f(in_depth_loc, depth);

	float verts[4][2];

	verts[0][0] = 0;
	verts[0][1] = 0;
	verts[1][0] = piglit_width;
	verts[1][1] = 0;
	verts[2][0] = 0;
	verts[2][1] = piglit_height;
	verts[3][0] = piglit_width;
	verts[3][1] = piglit_height;

	GLuint buf = 0;
	GLuint old_buf = 0;
	GLuint vao = 0;
	GLuint old_vao = 0;

	glGetIntegerv(GL_ARRAY_BUFFER_BINDING,
		      (GLint *) &old_buf);
	glGenBuffers(1, &buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	glBufferData(GL_ARRAY_BUFFER,
		     (sizeof(GLfloat) * 4 * 2),
		     verts,
		     GL_STATIC_DRAW);

	glVertexAttribPointer(PIGLIT_ATTRIB_POS, 2, GL_FLOAT,
			      GL_FALSE, 0,
			      0);
	glEnableVertexAttribArray(PIGLIT_ATTRIB_POS);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableVertexAttribArray(PIGLIT_ATTRIB_POS);

	glBindBuffer(GL_ARRAY_BUFFER, old_buf);
	glDeleteBuffers(1, &buf);

	glBindVertexArray(old_vao);
	glDeleteVertexArrays(1, &vao);
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	GLuint fbo;
	GLuint tex[2];
	const GLfloat red[] = {1.0, 0.0, 0.0, 1.0};
	const GLfloat green[] = {0.0, 1.0, 0.0, 1.0};
	const GLfloat blue[] = {0.0, 0.0, 1.0, 1.0};

	glViewport(0, 0, piglit_width, piglit_height);
	glGenTextures(2, tex);
	glBindTexture(GL_TEXTURE_2D, tex[0]);

	/* Set up texture for test result */
	glTexImage2D(GL_TEXTURE_2D,
		     0, /* level */
		     GL_RGBA,
		     piglit_width, piglit_height,
		     0, /* border */
		     GL_RGBA, GL_UNSIGNED_BYTE,
		     0);

	/* Set up stencil texture to test */
	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexImage2D(GL_TEXTURE_2D,
		     0, /* level */
		     GL_DEPTH_COMPONENT32F,
		     piglit_width, piglit_height,
		     0, /* border */
		     GL_DEPTH_COMPONENT, GL_FLOAT,
		     0);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex[0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex[1], 0);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glViewport(0, 0, piglit_width, piglit_height);

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClearDepthf(0.9f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	draw_rect(red, 1.0f);


	GLfloat depth_clear_value0 = 0.5;

	glClearTexImageEXT(tex[1],
			   0, /* level */
			   GL_DEPTH_COMPONENT,
			   GL_FLOAT,
			   &depth_clear_value0);

	/* Since the whole depth texture was cleared with 0.5 this should
	   overwrite the whole framebuffer */
	draw_rect(green, 0.6);

	GLfloat depth_clear_value1 = 0.2;

	glClearTexSubImageEXT(tex[1],
			      0, /* level */
			      piglit_width >> 2, piglit_height >> 2, 0,
			      piglit_width >> 1, piglit_height >> 1, 1,
			      GL_DEPTH_COMPONENT,
			      GL_FLOAT,
			      &depth_clear_value1);

	/* We only cleared a sub-area in the middle so only this
	   are should now be drawn */
	draw_rect(blue, 0.3);

	/* Copy to output for display and check pixels in the two areas */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, piglit_width, piglit_height,
			  0, 0, piglit_width, piglit_height,
			  GL_COLOR_BUFFER_BIT,
			  GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
	glDeleteTextures(2, tex);
	glDeleteFramebuffers(1, &fbo);

	if (!piglit_probe_pixel_rgba(1, 1, green))
		pass = false;

	if (!piglit_probe_pixel_rgba(piglit_width >> 1, piglit_height >> 1, blue))
		pass = false;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
