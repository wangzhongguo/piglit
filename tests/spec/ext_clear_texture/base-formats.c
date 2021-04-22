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

/** @file integer.c
 *
 * A test of using glClearTexSubImage to clear sub-regions textures
 * that can be read back from a fbo as is. Each format is created as a 4x4
 * texture where the first four texels are cleared to known values
 * using separate calls to glClearTexSubImage. The values are chosen
 * to potentially trigger problems with signed conversions for integer
 * textures. The restof the texture is initalised to zeroes. The textures
 * are then read back by attaching them to a fbo and calling glReadPixels
 * or glReadnPixelsKHR if robustness is supported.
 */

#define TEX_WIDTH 4
#define TEX_HEIGHT 4

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

/* Values to try clearing the texture to. The number of bytes used
 * will depend on the component size for the format. The actual value
 * used will depend on the endianness of the architecture but this
 * shouldn't really matter for the test */

static const uint32_t
clear_values[][4] = {
	{ 0xffffffffU, 0x00000000U, 0x12345678U, 0x78274827U },
	{ 0x00000000U, 0xffffffffU, 0x12345678U, 0x78274827U },
	{ 0x12345678U, 0x00000000U, 0xffffffffU, 0x78274827U },
	{ 0xa82748b7U, 0x12345678U, 0x00000000U, 0xffffffffU },
};

struct format {
	GLenum internal_format;
	GLenum format;
	GLenum type;
	int component_size;
	int n_components;
};

static const struct format
formats[] = {
	{ GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT, 4, 4 },
	{ GL_RGBA16UI, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT, 2, 4 },
	{ GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, 1, 4 },
	{ GL_RGBA32I, GL_RGBA_INTEGER, GL_INT, 4, 4 },
	{ GL_RGBA16I, GL_RGBA_INTEGER, GL_SHORT, 2, 4 },
	{ GL_RGBA8I, GL_RGBA_INTEGER, GL_BYTE, 1, 4 },
	{ GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, 1, 4 },
	{ GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 1, 4 },
	{ GL_RGBA32F, GL_RGBA, GL_FLOAT, 4, 4 },
};

static const struct format
srgb_format = { GL_SRGB8_ALPHA8_EXT, GL_RGBA, GL_UNSIGNED_BYTE, 1, 4 };

static GLuint
create_texture(const struct format *format)
{
	GLubyte *tex_data;
	int texel_size;
	GLuint tex;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	texel_size = format->component_size * format->n_components;
	tex_data = malloc(texel_size * TEX_WIDTH * TEX_HEIGHT);

	memset(tex_data, 0, texel_size * TEX_WIDTH * TEX_HEIGHT);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D,
		     0, /* level */
		     format->internal_format,
		     TEX_WIDTH, TEX_HEIGHT,
		     0, /* border */
		     format->format,
		     format->type,
		     tex_data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	free(tex_data);

	return tex;
}
 
static bool
clear_texture(GLuint tex,
	      const struct format *format)
{
	unsigned i;

	for (i = 0; i < ARRAY_SIZE(clear_values); i++) {
		glClearTexSubImageEXT(tex,
				      0, /* level */
				      i % TEX_WIDTH, /* x */
				      i / TEX_WIDTH, /* y */
				      0, /* z */
				      1, 1, 1, /* width/height/depth */
				      format->format,
				      format->type,
				      clear_values[i]);

		if (!piglit_check_gl_error(GL_NO_ERROR))
			return false;
	}

	return true;
}

static bool
check_texture(GLuint tex,
	      const struct format *format)
{
	GLubyte *tex_data, *p;
	unsigned i, j, texel_size;
	bool pass = true;

	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	texel_size = format->component_size * format->n_components;
	p = tex_data = malloc(texel_size * TEX_WIDTH * TEX_HEIGHT);

	get_tex_image(tex, 0, TEX_WIDTH, TEX_HEIGHT,
		      format->format, format->type,
		      texel_size * TEX_WIDTH * TEX_HEIGHT,
		      tex_data);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	for (i = 0; i < ARRAY_SIZE(clear_values); i++) {
		if (memcmp(p, clear_values[i], texel_size)) {
			pass = false;
			break;
		}
		p += texel_size;
	}

	/* The rest of the values should be zeroes */
	for (i = ARRAY_SIZE(clear_values); i < TEX_WIDTH * TEX_HEIGHT; i++)
		for (j = 0; j < texel_size; j++)
			if (*(p++) != 0) {
				pass = false;
				break;
			}

	free(tex_data);

	return pass;
}

static bool
check_format(const struct format *fmt)
{
	GLuint tex;

	bool pass = true;

	tex = create_texture(fmt);
	pass &= clear_texture(tex, fmt);
	pass &= check_texture(tex, fmt);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &tex);

	printf("internalFormat = %s, format = %s, type = %s : %s\n",
	       piglit_get_gl_enum_name(fmt->internal_format),
	       piglit_get_gl_enum_name(fmt->format),
	       piglit_get_gl_enum_name(fmt->type),
	       pass ? "pass" : "fail");

	return pass;
}

void
piglit_init(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	unsigned  i;
	bool pass = true;

	piglit_require_extension("GL_EXT_clear_texture");

	for (i = 0; i < ARRAY_SIZE(formats); i++) {
		pass &= check_format(&formats[i]);
	}

	pass &= check_format(&srgb_format);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* unused */
	return PIGLIT_FAIL;
}
