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

/* test_invalid_format was taken from the arb_clear_texture tests */

#include "common.h"

static void
readback_color_texture(GLuint texture,
		       int layer,
		       GLuint width,
		       GLuint height,
		       GLenum format,
		       GLenum type,
		       GLsizei bufsize,
		       GLubyte *data)
{
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER,
			       GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D,
			       texture,
			       layer);
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	if (piglit_is_extension_supported("GL_KHR_robustness"))
		glReadnPixelsKHR(0, 0, width, height, format, type, bufsize, data);
	else
		glReadPixels(0, 0, width, height, format, type, data);
	glDeleteFramebuffers(1, &fbo);
}

void
get_tex_image(GLuint texture,
	      int layer,
	      GLuint width,
	      GLuint height,
	      GLenum format,
	      GLenum type,
	      GLsizei bufsize,
	      GLubyte *data)
{
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	if (format == GL_DEPTH_COMPONENT ||
	    format == GL_STENCIL_INDEX ||
	    format == GL_DEPTH_STENCIL) {
		assert("reading back from DS textures is not implemented.");
	} else {
		readback_color_texture(texture, layer, width, height, format, type, bufsize, data);
	}
}

bool
test_invalid_format(GLenum internalFormat,
		    GLenum texImageFormat,
		    GLenum texImageType,
		    GLenum clearValueFormat,
		    GLenum clearValueType)
{
	static const GLubyte dummy_data[sizeof (float) * 4];
	bool pass = true;
	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D,
		     0, /* level */
		     internalFormat,
		     1, 1, /* width/height */
		     0, /* border */
		     texImageFormat,
		     texImageType,
		     dummy_data);

	pass &= piglit_check_gl_error(GL_NO_ERROR);

	glClearTexImage(tex, 0, clearValueFormat, clearValueType, dummy_data);

	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &tex);

	return pass;
}

void
init_program(const char *texdim)
{
	GLuint prog;
	GLuint uniform;

	static const char vs_source[] =
		"#version 300 es\n"
		"in mediump vec2 piglit_vertex;\n"
		"in mediump vec3 piglit_texcoord;\n"
		"uniform vec2 fb_size;\n"
		"out mediump vec3 tex_coord;\n"
		"\n"
		"void main()\n"
		"{\n"
		"        gl_Position = vec4(piglit_vertex * 2.0 /\n"
		"                           fb_size - 1.0,\n"
		"                           0.0, 1.0);\n"
		"        tex_coord = piglit_texcoord;\n"
		"}\n";

	const char fs_source[] = {
		"#version 300 es\n"
		"uniform mediump sampler%s tex;\n"
		"in mediump vec3 tex_coord;\n"
		"out mediump vec4 color;\n"
		"\n"
		"void main()\n"
		"{\n"
		"        color = texture%s(tex, tex_coord);\n"
		"}\n"};

	char buffer[1024] = {0};
	snprintf(buffer, 1024, fs_source, texdim, texdim);

	prog = piglit_build_simple_program(vs_source, buffer);

	glUseProgram(prog);

	uniform = glGetUniformLocation(prog, "tex");
	glUniform1i(uniform, 0);

	uniform = glGetUniformLocation(prog, "fb_size");
	glUniform2f(uniform, piglit_width, piglit_height);
}

