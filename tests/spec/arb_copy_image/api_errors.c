/*
 * Copyright 2014 Intel Corporation
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

/** @file simple.c
 *
 * A simple test of glCopyImageSubData that copies a square from one
 * 2D texture to another and back.  This test exercises texture to texture,
 * texture to renderbuffer, renderbuffer to texture, and renderbuffer to
 * renderbuffer copies.
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 13;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

#define ARRAY_LENGTH(X) (sizeof(X)/sizeof(*(X)))

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_copy_image");
	piglit_require_extension("GL_EXT_framebuffer_object");
}

static GLuint
image_create(GLenum target)
{
	GLuint name;
	if (target == GL_RENDERBUFFER_EXT)
		glGenRenderbuffers(1, &name);
	else
		glGenTextures(1, &name);
	return name;
}

static void
image_delete(GLenum target, GLuint name)
{
	if (target == GL_RENDERBUFFER_EXT)
		glDeleteRenderbuffers(1, &name);
	else
		glDeleteTextures(1, &name);
}

static void
image_storage(GLenum target, GLuint name, GLenum internal_format,
	      GLsizei width, GLsizei height)
{
	if (target == GL_RENDERBUFFER_EXT) {
		glBindRenderbuffer(target, name);
		glRenderbufferStorage(target, internal_format, width, height);
	} else {
		glBindTexture(target, name);
		glTexStorage2D(target, 4, internal_format, width, height);
	}
}

/* only testing legal targets below */
static const GLenum targets[] = {
	GL_TEXTURE_1D,
	GL_TEXTURE_1D_ARRAY,
	GL_TEXTURE_2D,
	GL_TEXTURE_RECTANGLE,
	GL_TEXTURE_2D_ARRAY,
	GL_TEXTURE_2D_MULTISAMPLE,
	GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
	GL_TEXTURE_CUBE_MAP,
	GL_TEXTURE_CUBE_MAP_ARRAY,
	GL_TEXTURE_3D
};

static bool
test_simple_errors(GLenum src_target, GLenum dst_target)
{
	bool pass = true;
	GLuint i, src, src2, dst;

	src = image_create(src_target);
	dst = image_create(dst_target);

	/* Test all three combinations of incomplete src or dst  */
	glCopyImageSubData(src, src_target, 0, 0, 0, 0,
			   dst, dst_target, 0, 0, 0, 0, 0, 0, 0);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	image_storage(src_target, src, GL_RGBA8, 32, 32);
	assert(piglit_check_gl_error(GL_NO_ERROR));

	glCopyImageSubData(src, src_target, 0, 0, 0, 0,
			   dst, dst_target, 0, 0, 0, 0, 0, 0, 0);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	image_storage(dst_target, dst, GL_RGBA8, 32, 32);
	assert(piglit_check_gl_error(GL_NO_ERROR));

	/* We want to test with empty src but valid dst */
	src2 = image_create(src_target);

	glCopyImageSubData(src2, src_target, 0, 0, 0, 0,
			   dst, dst_target, 0, 0, 0, 0, 0, 0, 0);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	/* This is no longer needed */
	image_delete(src_target, src2);

	/* Section 18.3.2 (Copying Between Images) of the OpenGL 4.5 Core
	 * Profile spec says:
	 *
	 *     "An INVALID_ENUM error is generated if either target is
	 *      not RENDERBUFFER or a valid non-proxy texture target;
	 *      is TEXTURE_BUFFER or one of the cubemap face selectors
	 *      described in table 8.18; or if the target does not
	 *      match the type of the object."
	 */
	if (src_target != GL_RENDERBUFFER_EXT) {
		for (i = 0; i < ARRAY_LENGTH(targets); ++i) {
			if (targets[i] == src_target)
				continue;

			/* here, targets[i] doesn't match src object's target */
			glCopyImageSubData(src, targets[i], 0, 0, 0, 0,
					   dst, dst_target, 0, 0, 0, 0,
					   0, 0, 0);
			pass &= piglit_check_gl_error(GL_INVALID_ENUM);
			if (!pass)
				return false;
		}
	}

	/* Section 18.3.2 (Copying Between Images) of the OpenGL 4.5 Core
	 * Profile spec says:
	 *
	 *     "An INVALID_ENUM error is generated if either target is
	 *      not RENDERBUFFER or a valid non-proxy texture target;
	 *      is TEXTURE_BUFFER or one of the cubemap face selectors
	 *      described in table 8.18; or if the target does not
	 *      match the type of the object."
	 */
	if (dst_target != GL_RENDERBUFFER_EXT) {
		for (i = 0; i < ARRAY_LENGTH(targets); ++i) {
			if (targets[i] == dst_target)
				continue;

			/* here, targets[i] doesn't match dst object's target */
			glCopyImageSubData(src, src_target, 0, 0, 0, 0,
					   dst, targets[i], 0, 0, 0, 0,
					   0, 0, 0);
			pass &= piglit_check_gl_error(GL_INVALID_ENUM);
			if (!pass)
				return false;
		}
	}

	/* Section 18.3.2 (Copying Between Images) of the OpenGL 4.5 Core
	 * Profile spec says:
	 *
	 *     "An INVALID_VALUE error is generated if either name does not
	 *     correspond to a valid renderbuffer or texture object according
	 *     to the corresponding target parameter."
	 */
	/* 4523 should be a bogus renderbuffer/texture */
	glCopyImageSubData(4523, src_target, 0, 0, 0, 0,
			   dst, dst_target, 0, 0, 0, 0, 0, 0, 0);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);
	glCopyImageSubData(src, src_target, 0, 0, 0, 0,
			   4523, dst_target, 0, 0, 0, 0, 0, 0, 0);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);

	/* Invalid level */
	glCopyImageSubData(src, src_target, 5, 0, 0, 0,
			   dst, dst_target, 0, 0, 0, 0, 0, 0, 0);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);
	glCopyImageSubData(src, src_target, 0, 0, 0, 0,
			   dst, dst_target, 5, 0, 0, 0, 0, 0, 0);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);

	/* Region out of bounds */
	glCopyImageSubData(src, src_target, 0, 7, 5, 2,
			   dst, dst_target, 0, 0, 0, 0, 26, 25, 20);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);
	glCopyImageSubData(src, src_target, 0, 7, 5, 2,
			   dst, dst_target, 0, 0, 0, 0, 25, 30, 20);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);
	glCopyImageSubData(src, src_target, 0, 7, 5, 2,
			   dst, dst_target, 0, 0, 0, 0, 25, 24, 31);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);
	glCopyImageSubData(src, src_target, 0, 0, 0, 0,
			   dst, dst_target, 0, 7, 5, 2, 26, 25, 20);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);
	glCopyImageSubData(src, src_target, 0, 0, 0, 0,
			   dst, dst_target, 0, 7, 5, 2, 25, 30, 20);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);
	glCopyImageSubData(src, src_target, 0, 0, 0, 0,
			   dst, dst_target, 0, 7, 5, 2, 25, 24, 31);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);

	image_delete(src_target, src);
	image_delete(dst_target, dst);

	return pass;
}

static bool
test_compressed_alignment_errors()
{
	bool pass = true;
	GLuint tex[4];

	glGenTextures(4, tex);

	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexStorage2D(GL_TEXTURE_2D, 1,
		       GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 128, 128);
	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16UI, 32, 32);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST_MIPMAP_NEAREST);

	/* Check for alignment constraints */
	/* bad width = 21 */
	glCopyImageSubData(tex[0], GL_TEXTURE_2D, 0, 0, 0, 0,
			   tex[1], GL_TEXTURE_2D, 0, 0, 0, 0, 21, 24, 1);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);
	/* bad height = 22 */
	glCopyImageSubData(tex[0], GL_TEXTURE_2D, 0, 0, 0, 0,
			   tex[1], GL_TEXTURE_2D, 0, 0, 0, 0, 20, 22, 1);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);
	/* bad srcX = 2 */
	glCopyImageSubData(tex[0], GL_TEXTURE_2D, 0, 2, 0, 0,
			   tex[1], GL_TEXTURE_2D, 0, 0, 0, 0, 20, 24, 1);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);
	/* bad srcY = 1 */
	glCopyImageSubData(tex[0], GL_TEXTURE_2D, 0, 0, 1, 0,
			   tex[1], GL_TEXTURE_2D, 0, 0, 0, 0, 20, 24, 1);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);

	/* Section 18.3.2 (Copying Between Images) of the OpenGL 4.5 Core
	 * Profile spec says:
	 *
	 *     "An INVALID_OPERATION error is generated if the texel size of
	 *     the uncompressed image is not equal to the block size of the
	 *     compressed image."
	 */
	glBindTexture(GL_TEXTURE_2D, tex[2]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB16UI, 32, 32);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST_MIPMAP_NEAREST);
	glCopyImageSubData(tex[0], GL_TEXTURE_2D, 0, 0, 0, 0,
			   tex[2], GL_TEXTURE_2D, 0, 0, 0, 0, 20, 20, 1);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	/* Section 18.3.2 (Copying Between Images) of the OpenGL 4.5 Core
	 * Profile spec says:
	 *
	 *     "An INVALID_OPERATION error is generated if the formats are
	 *     not compatible."
	 *
	 * The definition of compatible refers back to table 8.22 "Compatible
	 * internal formats for TextureView."  This table does not contain
	 * S3TC formats because they are from an older extension.  Given the
	 * different encodings of DXT1 and DXT3 textures, it is reasonable to
	 * assume they would not be compatible for texture views, and this
	 * matches at least NVIDIA's implementation.
	 */
	glBindTexture(GL_TEXTURE_2D, tex[3]);
	glTexStorage2D(GL_TEXTURE_2D, 1,
		       GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 32, 32);
	glCopyImageSubData(tex[0], GL_TEXTURE_2D, 0, 0, 0, 0,
			   tex[3], GL_TEXTURE_2D, 0, 0, 0, 0, 20, 20, 1);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	glDeleteTextures(4, tex);

	return pass;
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	pass &= test_simple_errors(GL_TEXTURE_2D, GL_TEXTURE_2D);
	pass &= test_simple_errors(GL_RENDERBUFFER, GL_TEXTURE_2D);
	pass &= test_simple_errors(GL_TEXTURE_2D, GL_RENDERBUFFER);
	pass &= test_simple_errors(GL_RENDERBUFFER, GL_RENDERBUFFER);
	if (piglit_is_extension_supported("GL_EXT_texture_compression_s3tc")) {
		pass &= test_compressed_alignment_errors();
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
