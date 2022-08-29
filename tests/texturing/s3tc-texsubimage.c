/*
 * Copyright © 2009 Intel Corporation
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
 *    Chris Lord <chris@openedhand.com>
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file s3tc-teximage.c
 *
 * Tests that a full S3TC-compressed mipmap tree can be created and
 * used.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

#ifdef PIGLIT_USE_OPENGL
	config.supports_gl_compat_version = 11;
#else // PIGLIT_USE_OPENGL_ES2
	config.supports_gl_es_version = 20;
#endif

	config.window_width = 500;
	config.window_height = 600;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

#define SIZE 128

const float red[4] =   {1.0, 0.0, 0.0, 1.0};
const float green[4] = {0.0, 1.0, 0.0, 1.0};
const float blue[4] =  {0.0, 0.0, 1.0, 1.0};
const float white[4] = {1.0, 1.0, 1.0, 1.0};

#ifdef  PIGLIT_USE_OPENGL_ES2

const char *vs_source =
	"#version 100\n"
	"attribute vec4 piglit_vertex;\n"
	"attribute vec2 piglit_texcoord;\n"
	"varying mediump vec2 tex_coord;\n"
	"uniform mat4 proj;\n"
	"\n"
	"void main()\n"
	"{\n"
	"        gl_Position = proj * piglit_vertex;\n"
	"        tex_coord = piglit_texcoord;\n"
	"}\n";

const char *fs_source =
	"#version 100\n"
	"varying mediump vec2 tex_coord;\n"
	"uniform sampler2D tex;\n"
	"\n"
	"void main()\n"
	"{\n"
	"        gl_FragColor = texture2D(tex, tex_coord);\n"
	"}\n";

#include "piglit-matrix.h"

GLint tex_program;

#endif

static void
display_mipmaps(int start_x, int start_y)
{
	int i;

#ifdef PIGLIT_USE_OPENGL
	glEnable(GL_TEXTURE_2D);
#endif

	/* Display all the mipmap levels */
	for (i = SIZE; i > 0; i /= 2) {
		piglit_draw_rect_tex(start_x, start_y, i, i,
		                     0.0f, 0.0f, 1.0f, 1.0f);
		start_x += i + 5;
	}
}

static GLuint
create_texture(GLenum format)
{
	GLfloat *data;
	int size, x, y, level;
	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_NEAREST);
	data = malloc(SIZE * SIZE * 4 * sizeof(GLfloat));

	for (level = 0, size = SIZE; size > 0; level++, size >>= 1) {
		for (y = 0; y < size; y++) {
			for (x = 0; x < size; x++) {
				const float *color;

				if (size == 4)
					color = red;
				else if (size == 2)
					color = green;
				else if (size == 1)
					color = blue;
				else if (x < size / 2 && y < size / 2)
					color = red;
				else if (y < size / 2)
					color = green;
				else if (x < size / 2)
					color = blue;
				else
					color = white;

				memcpy(data + (y * size + x) * 4, color,
				       4 * sizeof(float));
			}
		}

		/* Create empty texture */
		glTexImage2D(GL_TEXTURE_2D, level, format,
			     size, size, 0,
			     GL_RGBA, GL_FLOAT, NULL);

		glPixelStorei(GL_UNPACK_ROW_LENGTH, size);

		/* Fill in sub regions of texture */
		if (size <= 4) {
			glTexSubImage2D(GL_TEXTURE_2D, level,
					0, 0, size, size,
					GL_RGBA, GL_FLOAT, data);
		} else {
			float *reds = data;
			float *greens = data + size / 2 * 4;
			float *blues = reds + (size / 2) * size * 4;
			float *whites = blues + size / 2 * 4;

			glTexSubImage2D(GL_TEXTURE_2D, level,
					0, 0,
					size / 2, size / 2,
					GL_RGBA, GL_FLOAT, reds);
			glTexSubImage2D(GL_TEXTURE_2D, level,
					size / 2, 0,
					size / 2, size / 2,
					GL_RGBA, GL_FLOAT, greens);
			glTexSubImage2D(GL_TEXTURE_2D, level,
					0, size / 2,
					size / 2, size / 2,
					GL_RGBA, GL_FLOAT, blues);
			glTexSubImage2D(GL_TEXTURE_2D, level,
					size / 2, size / 2,
					size / 2, size / 2,
					GL_RGBA, GL_FLOAT, whites);
		}
	}
	free(data);
	return tex;
}

static GLboolean
check_resulting_mipmaps(int x, int y)
{
	GLboolean pass = GL_TRUE;
	int size;

	for (size = SIZE; size > 0; size /= 2) {
		if (size == 4)
			pass = pass && piglit_probe_pixel_rgb(x + 2, y + 2,
							      red);
		else if (size == 2)
			pass = pass && piglit_probe_pixel_rgb(x + 1, y + 1,
							      green);
		else if (size == 1)
			pass = pass && piglit_probe_pixel_rgb(x, y,
							      blue);
		else {
			pass = pass && piglit_probe_pixel_rgb(x + size / 4,
							      y + size / 4,
							      red);
			pass = pass && piglit_probe_pixel_rgb(x + size * 3 / 4,
							      y + size / 4,
							      green);
			pass = pass && piglit_probe_pixel_rgb(x + size / 4,
							      y + size * 3 / 4,
							      blue);
			pass = pass && piglit_probe_pixel_rgb(x + size * 3 / 4,
							      y + size * 3 / 4,
							      white);
		}
		x += size + 5;
	}

	return pass;
}

enum piglit_result
piglit_display(void)
{
	GLuint tex;
	GLboolean pass = GL_TRUE;

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	tex = create_texture(GL_COMPRESSED_RGB_S3TC_DXT1_EXT);
	display_mipmaps(10, 10 + (10 + SIZE) * 0);
	glDeleteTextures(1, &tex);
	tex = create_texture(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT);
	display_mipmaps(10, 10 + (10 + SIZE) * 1);
	glDeleteTextures(1, &tex);
	tex = create_texture(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT);
	display_mipmaps(10, 10 + (10 + SIZE) * 2);
	glDeleteTextures(1, &tex);
	tex = create_texture(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);
	display_mipmaps(10, 10 + (10 + SIZE) * 3);
	glDeleteTextures(1, &tex);

	pass = pass && check_resulting_mipmaps(10, 10 + (10 + SIZE) * 0);
	pass = pass && check_resulting_mipmaps(10, 10 + (10 + SIZE) * 1);
	pass = pass && check_resulting_mipmaps(10, 10 + (10 + SIZE) * 2);
	pass = pass && check_resulting_mipmaps(10, 10 + (10 + SIZE) * 3);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_texture_compression_s3tc");

#ifdef PIGLIT_USE_OPENGL

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

#else // PIGLIT_USE_OPENGL_ES2

	tex_program = piglit_build_simple_program(vs_source, fs_source);
	glUseProgram(tex_program);
	GLint proj_loc = glGetUniformLocation(tex_program, "proj");
	piglit_ortho_uniform(proj_loc, piglit_width, piglit_height);

#endif
}
