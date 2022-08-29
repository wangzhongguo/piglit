/*
 * Copyright © 2015 Intel Corporation
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

/**
 * \file object-namespace-pollution.c
 * Verify that the GL implementation does not pollute the object namespace.
 *
 * At least through Mesa 11.1.0, Mesa drivers that use "meta" have some
 * problems with respect to the OpenGL object namespace.  Many places inside
 * meta allocate objects using a mechanism similar to \c glGen*.  This poses
 * serious problems for applications that create objects without using the
 * associated \c glGen* function (so called "user generated names").
 *
 * Section 3.8.12 (Texture Objects) of the OpenGL 2.1 (May 16, 2008) spec
 * says:
 *
 *     "The command
 *
 *         void GenTextures( sizei n, uint *textures );
 *
 *     returns n previously unused texture object names in textures. These
 *     names are marked as used, for the purposes of GenTextures only, but
 *     they acquire texture state and a dimensionality only when they are
 *     first bound, just as if they were unused."
 *
 * Calling glBindTexture on an unused name makes that name be used.  An
 * application can mix user generated names and GL generated names only if it
 * is careful not to reuse names that were previously returned by
 * glGenTextures.  In practice this means that all user generated names must
 * be used (i.e., bound) before calling glGenTextures.
 *
 * This effectively means that the GL implementation (or, realistically, GL
 * middleware) is \b never allowed to use glGenTextures because the
 * application cannot know what names were returned.
 *
 * This applies to most kinds of GL objects.
 *
 * - buffers
 * - textures
 * - framebuffers
 * - renderbuffers
 * - queries
 * - vertex programs (from GL_ARB_vertex_program)
 * - fragment programs (from GL_ARB_fragment_program)
 * - vertex arrays (from GL_APPLE_vertex_array_object)
 * - fragment shaders (from GL_ATI_fragment_shader)
 *
 * Many object types (ARB vertex array objects, transform feedback objects,
 * program pipeline objects, GLSL shader / program objects, Intel performance
 * query objects, sampler objects, etc.) forbid user generated names.
 *
 * Some object types (NVIDIA or APPLE fences, EXT vertex shaders, NVIDIA
 * transform feedback objects) could probably also suffer from this problem.
 * However, Mesa does not support these objects, so we don't need to test
 * them.
 *
 * GL_AMD_performance_monitor does not specify whether or not user generated
 * names are allowed.
 *
 * This test attempts to observe this kind of invalid behavior.  First a GL
 * operation is performed that may need to create an object.  Then the test
 * creates several objects with user generated names.  Finally, the GL
 * operations are performed again.  If the GL implemented generated new names
 * for the purpose of the operations, those names will likely conflict with
 * one of the user generated names.  This should be observable in one of three
 * ways.
 *
 * - When the test tries to create the object, the object will already exist.
 *   This is detected by the \c glIs* functions.
 *
 * - After the second call to the GL operation, the application's object will
 *   have modified state.
 *
 * - The second call to the GL operation will fail to perform correctly
 *   because the application modified its data.
 *
 * Only the first two methods are employed by this test.  This should catch
 * the vast majority of possible failures.  Verifying the correctness of the
 * GL operation would add a lot of complexity to the test.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 12;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH | PIGLIT_GL_VISUAL_STENCIL;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

struct enum_value_pair {
	GLenum value;
	GLint expected;
};

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

/**
 * Spare objects used by test cases.
 *
 * Some tests need to use objects for the GL operation begin tested.  For
 * example, the \c glGenerateMipmap test needs a texture.  These objects
 * cannot be created using \c glGen* because that would conflict with the rest
 * of the test.  Instead statically allocate object names starting with some
 * high number that we hope the GL won't use or generate during the test.
 */
#define FIRST_SPARE_OBJECT 600

/**
 * Linear feedback shift register random number generator
 *
 * Simple Galois LFSRs that is loosely based on
 * https://en.wikipedia.org/wiki/Linear_feedback_shift_register
 *
 * The value of \c state is updated to reflect the new state of the LFSR.
 * This new value should be passed in for the next iteration.
 *
 * \return
 * Either 0 or 1 based on the incoming value of \c state.
 */
static unsigned
lfsr(uint16_t *state)
{
	unsigned output = *state & 1;

	/* For an LFSR, zero is a fixed point, and that's no good for
	 * generating additional values.
	 */
	assert(*state != 0);

	/* If the output bit is zero, just shift it out.  If the output bit is
	 * one, shift it out and toggle some bits.
	 */
	*state = (*state >> 1) ^ (-output & 0x0000B400);

	return output;
}

/**
 * Fill some memory with pseudorandom values
 *
 * Using two seed values, a pair of LFSRs are used to generate pseudorandom
 * values to fill the specified memory buffer.  Separate invocations with
 * identical \c bytes, \c seed1, and \c seed2 parameters will generate
 * identical data.  This can be used generate data to initialize a buffer and
 * regenerate the same data to validate the buffer.
 */
static void
generate_random_data(void *_output, size_t bytes, uint16_t seed1,
		     uint16_t seed2)
{
	uint8_t *output = (uint8_t *) _output;

	/* If the two seeds are the same, the whole "random" buffer will be
	 * filled with zeroes.
	 */
	assert(seed1 != seed2);

	for (unsigned i = 0; i < bytes; i++) {
		uint8_t byte = 0;

		for (unsigned bit = 0; bit < 8; bit++) {
			byte <<= 1;
			byte |= lfsr(&seed1) ^ lfsr(&seed2);
		}

		output[i] = byte;
	}
}

/** \name Methods for operating on buffer objects */
/*@{*/
#define BUFFER_DATA_SIZE 1024

static bool
create_buffer(unsigned name)
{
	uint8_t data[BUFFER_DATA_SIZE];

	if (!piglit_is_extension_supported("GL_ARB_vertex_buffer_object") &&
	    piglit_get_gl_version() < 14) {
		printf("%s requires vertex buffer objects.\n", __func__);
		piglit_report_result(PIGLIT_SKIP);
	}

	generate_random_data(data, sizeof(data), GL_ARRAY_BUFFER, name);

	if (glIsBuffer(name)) {
		printf("\t%s,%d: %u is already a buffer\n",
		       __func__, __LINE__, name);
		return false;
	}

	glBindBuffer(GL_ARRAY_BUFFER, name);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return piglit_check_gl_error(GL_NO_ERROR);
}

static bool
validate_buffer(unsigned name)
{
	static const struct enum_value_pair test_vectors[] = {
		{ GL_BUFFER_SIZE, BUFFER_DATA_SIZE },
		{ GL_BUFFER_USAGE, GL_STATIC_DRAW },
		{ GL_BUFFER_ACCESS, GL_READ_WRITE },
		{ GL_BUFFER_MAPPED, GL_FALSE },
	};
	bool pass = true;
	uint8_t data[BUFFER_DATA_SIZE];
	void *map;

	if (!glIsBuffer(name)) {
		printf("\t%s,%d: %u is not a buffer\n",
		       __func__, __LINE__, name);
		return false;
	}

	glBindBuffer(GL_ARRAY_BUFFER, name);

	for (unsigned i = 0; i < ARRAY_SIZE(test_vectors); i++) {
		GLint got;

		glGetBufferParameteriv(GL_ARRAY_BUFFER,
				       test_vectors[i].value,
				       &got);

		if (got != test_vectors[i].expected) {
			printf("\t%s,%d: %s of %u: got 0x%x, expected 0x%x\n",
			       __func__, __LINE__,
			       piglit_get_gl_enum_name(test_vectors[i].value),
			       name, got, test_vectors[i].expected);
			pass = false;
		}
	}

	generate_random_data(data, sizeof(data), GL_ARRAY_BUFFER, name);
	map = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	if (map != NULL) {
		if (memcmp(map, data, sizeof(data)) != 0) {
			printf("\t%s,%d: Data mismatch in %u\n",
			       __func__, __LINE__, name);
			pass = false;
		}
	} else {
		printf("\t%s,%d: Unable to map %u\n",
		       __func__, __LINE__, name);
		pass = false;
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	return pass;
}
/*@}*/

/** \name Methods for operating on framebuffer objects */
/*@{*/
static bool
create_framebuffer(unsigned name)
{
	if (!piglit_is_extension_supported("GL_EXT_framebuffer_object")) {
		printf("%s requires GL_EXT_framebuffer_object.\n", __func__);
		piglit_report_result(PIGLIT_SKIP);
	}

	if (glIsFramebufferEXT(name)) {
		printf("\t%s,%d: %u is already a framebuffer\n",
		       __func__, __LINE__, name);
		return false;
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER, name);
	glBindFramebufferEXT(GL_FRAMEBUFFER, piglit_winsys_fbo);

	return piglit_check_gl_error(GL_NO_ERROR);
}

static bool
validate_framebuffer(unsigned name)
{
	static const struct enum_value_pair test_vectors[] = {
		{ GL_COLOR_ATTACHMENT0, 0 },
		{ GL_DEPTH_ATTACHMENT, 0 },
		{ GL_STENCIL_ATTACHMENT, 0 },
	};
	bool pass = true;

	if (!glIsFramebufferEXT(name)) {
		printf("\t%s,%d: %u is not a framebuffer\n",
		       __func__, __LINE__, name);
		return false;
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER, name);

	for (unsigned i = 0; i < ARRAY_SIZE(test_vectors); i++) {
		GLint got;

		glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER,
							 test_vectors[i].value,
							 GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
							 &got);

		if (got != test_vectors[i].expected) {
			printf("\t%s,%d: %s of %u: got 0x%x, expected 0x%x\n",
			       __func__, __LINE__,
			       piglit_get_gl_enum_name(test_vectors[i].value),
			       name, got, test_vectors[i].expected);
			pass = false;
		}
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER, piglit_winsys_fbo);

	return piglit_check_gl_error(GL_NO_ERROR) && pass;
}
/*@}*/

/** \name Methods for operating on vertex or fragment program objects */
/*@{*/
static char *
generate_program_source(GLenum target, unsigned key)
{
	char *source = NULL;

	if (target == GL_VERTEX_PROGRAM_ARB) {
		const char *position;
		const char *normal;
		const char *other;
		const char *mvp;
		const char *mv_invtrans;

		if ((key & 2) != 0) {
			mvp = "state.matrix.mvp";
			position =
				"DP4   p.x, mvp[0], vp;\n"
				"DP4   p.y, mvp[1], vp;\n"
				"DP4   p.z, mvp[2], vp;\n"
				"DP4   p.w, mvp[3], vp;\n"
				;
		} else {
			mvp = "state.matrix.mvp.transpose";
			position =
				"MUL   r0, mvp[0], vp.xxxx;\n"
				"MAD   r0, mvp[1], vp.yyyy, r0;\n"
				"MAD   r0, mvp[2], vp.zzzz, r0;\n"
				"MAD   r0, mvp[3], vp.wwww, r0;\n"
				"MOV   p, r0;\n"
				;
		}

		if ((key & 4) != 0) {
			mv_invtrans = "state.matrix.modelview.invtrans";
			normal =
				"DP3   n.x, mv_invtrans[0], vn;\n"
				"DP3   n.y, mv_invtrans[1], vn;\n"
				"DP3   n.z, mv_invtrans[2], vn;\n"
				;
		} else {
			mv_invtrans = "state.matrix.modelview.inverse";
			normal =
				"MUL   r0.xyz, mv_invtrans[0], vn;\n"
				"MAD   r0.xyz, mv_invtrans[1], vn, r0;\n"
				"MAD   r0.xyz, mv_invtrans[2], vn, r0;\n"
				"SWZ   n, r0, x, y, z, 0;\n"
				;
		}

		if ((key & 8) != 0) {
			other =
				"# Output the color\n"
				"MOV   t, vertex.color;\n"
				;
		} else {
			other =
				"# Output the texture coordinate\n"
				"MOV   t, vertex.texcoord[0];\n"
				;
		}

		(void)!asprintf(&source,
			 "!!ARBvp1.0\n"
			 "# Program key 0x%04x\n"
			 "ATTRIB vp = vertex.position;\n"
			 "ATTRIB vn = vertex.normal;\n"
			 "PARAM mvp[4] = { %s.row[0..3] };\n"
			 "PARAM mv_invtrans[3] = { %s.row[0..2] };\n"
			 "OUTPUT p = result.position;\n"
			 "OUTPUT n = result.texcoord[0];\n"
			 "OUTPUT t = result.texcoord[1];\n"
			 "TEMP r0;\n"
			 "\n"
			 "# Transform the vertex to clip coordinates.\n"
			 "%s"
			 "\n"
			 "# Transform the normal to eye coordinates.\n"
			 "%s"
			 "\n"
			 "%s"
			 "END",
			 key,
			 mvp,
			 mv_invtrans,
			 position,
			 normal,
			 other);
	} else if (target == GL_FRAGMENT_PROGRAM_ARB) {
		const char *tex;
		const char *half_dir;
		const char *light_dir;

		if ((key & 2) != 0) {
			tex = "TEX   r0, t, texture[0], 2D;";
		} else {
			tex = "TEX   r0, t, texture[0], 1D;";
		}

		if ((key & 4) != 0) {
			light_dir = "program.env[0]";
		} else {
			light_dir = "state.light[0].position";
		}

		if ((key & 4) != 0) {
			half_dir = "program.env[1]";
		} else {
			half_dir = "state.light[0].half";
		}

		(void)!asprintf(&source,
			 "!!ARBfp1.0\n"
			 "# Program key 0x%04x\n"
			 "ATTRIB n = fragment.texcoord[0];\n"
			 "ATTRIB t = fragment.texcoord[1];\n"
			 "PARAM light_dir = %s;\n"
			 "PARAM half_dir = %s;\n"
			 "OUTPUT c = result.color;\n"
			 "TEMP r0, r1;\n"
			 "\n"
			 "# Sample the texture\n"
			 "%s"
			 "\n"
			 "# Compute lighting\n"
			 "DP3   r1.x, n, light_dir;\n"
			 "DP3   r1.y, n, half_dir;\n"
			 "MOV   r1.w, 126.2;\n"
			 "LIT   r1, r1;\n"
			 "\n"
			 "# Combine lighting and texture\n"
			 "MUL   r0, r1.y, r0;\n"
			 "ADD   c.xyz, r1.z, r0;\n"
			 "MOV   c.w, 1.0;\n"
			 "END",
			 key,
			 light_dir,
			 half_dir,
			 tex);
	} else {
		printf("Unknown program target %s (0x%04x) in %s.\n",
		       piglit_get_gl_enum_name(target), target,
		       __func__);
		piglit_report_result(PIGLIT_FAIL);
	}

	return source;
}

/**
 * Selects a program target based on the key and the supported extensions.
 */
static GLenum
select_program_target(unsigned key)
{
	GLenum target = 0;

	if (piglit_is_extension_supported("GL_ARB_vertex_program")) {
		target = GL_VERTEX_PROGRAM_ARB;
	}

	if (piglit_is_extension_supported("GL_ARB_fragment_program")) {
		if (target == 0 || (key & 1) == 0)
			target = GL_FRAGMENT_PROGRAM_ARB;
	}

	return target;
}

static bool
create_program(unsigned name)
{
	char *source;
	const GLenum target = select_program_target(name);

	if (target == 0) {
		printf("%s requires GL_ARB_vertex_program or "
		       "GL_ARB_fragment_program.\n",
		       __func__);
		piglit_report_result(PIGLIT_SKIP);
	}

	if (glIsProgramARB(name)) {
		printf("\t%s,%d: %u is already a program\n",
		       __func__, __LINE__, name);
		return false;
	}

	glBindProgramARB(target, name);

	source = generate_program_source(target, name);
	glProgramStringARB(target, GL_PROGRAM_FORMAT_ASCII_ARB,
			   strlen(source), source);

	free(source);

	glBindProgramARB(target, 0);

	return piglit_check_gl_error(GL_NO_ERROR);
}

static bool
validate_program(unsigned name)
{
	bool pass = true;
	const GLenum target = select_program_target(name);
	char *expected = generate_program_source(target, name);
	const int expected_length = strlen(expected);
	char *got;
	GLint length;

	if (!glIsProgramARB(name)) {
		printf("\t%s,%d: %u is not a program\n",
		       __func__, __LINE__, name);
		free(expected);
		return false;
	}

	glBindProgramARB(target, name);

	/* The GL_ARB_vertex_program spec does not say whether or not length
	 * includes a terminating NUL.  It says:
	 *
	 *     "If <pname> is PROGRAM_LENGTH_ARB ... GetProgramivARB returns
	 *     one integer holding the program string length (in bytes)
	 *     ... for the program object currently bound to <target>."
	 *
	 * and, with respect to glGetProgramStringARB it says:
	 *
	 *     "<n> ubytes are returned into the array program where <n> is
	 *     the length of the program in ubytes, as returned by
	 *     GetProgramivARB when <pname> is PROGRAM_LENGTH_ARB."
	 */
	glGetProgramivARB(target,
			  GL_PROGRAM_LENGTH_ARB,
			  &length);
	if (length != expected_length && length != (expected_length + 1)) {
		printf("\t%s,%d: Program %u length invalid: got %d, expected "
		       "%d or %d.\n",
		       __func__, __LINE__,
		       name,
		       length, expected_length, expected_length + 1);
		pass = false;
	}

	got = calloc(length, sizeof(char));
	glGetProgramStringARB(target,
			      GL_PROGRAM_STRING_ARB,
			      got);

	if (memcmp(expected, got, expected_length) != 0) {
		printf("\t%s,%d: %u source mismatch\n",
		       __func__, __LINE__,
		       name);
		pass = false;
	}

	free(got);
	free(expected);

	glBindProgramARB(target, 0);

	return piglit_check_gl_error(GL_NO_ERROR) && pass;
}
/*@}*/

/** \name Methods for operating on renderbuffer objects */
/*@{*/
static bool
create_renderbuffer(unsigned name)
{
	if (!piglit_is_extension_supported("GL_EXT_framebuffer_object")) {
		printf("%s requires GL_EXT_framebuffer_object.\n", __func__);
		piglit_report_result(PIGLIT_SKIP);
	}

	if (glIsRenderbufferEXT(name)) {
		printf("\t%s,%d: %u is already a renderbuffer\n",
		       __func__, __LINE__, name);
		return false;
	}

	glBindRenderbufferEXT(GL_RENDERBUFFER, name);
	glRenderbufferStorageEXT(GL_RENDERBUFFER, GL_RGBA8,
				 17 /* width */, 15 /* height */);

	glBindRenderbufferEXT(GL_RENDERBUFFER, 0);

	return piglit_check_gl_error(GL_NO_ERROR);
}

static bool
validate_renderbuffer(unsigned name)
{
	static const struct enum_value_pair test_vectors[] = {
		{ GL_RENDERBUFFER_WIDTH, 17 },
		{ GL_RENDERBUFFER_HEIGHT, 15 },
		{ GL_RENDERBUFFER_INTERNAL_FORMAT, GL_RGBA8 },
		{ GL_RENDERBUFFER_DEPTH_SIZE, 0 },
		{ GL_RENDERBUFFER_STENCIL_SIZE, 0 },
	};
	bool pass = true;

	if (!glIsRenderbufferEXT(name)) {
		printf("\t%s,%d: %u is not a renderbuffer\n",
		       __func__, __LINE__, name);
		return false;
	}

	glBindRenderbufferEXT(GL_RENDERBUFFER, name);

	for (unsigned i = 0; i < ARRAY_SIZE(test_vectors); i++) {
		GLint got;

		glGetRenderbufferParameterivEXT(GL_RENDERBUFFER,
						test_vectors[i].value,
						&got);

		if (got != test_vectors[i].expected) {
			printf("\t%s,%d: %s of %u: got 0x%x, expected 0x%x\n",
			       __func__, __LINE__,
			       piglit_get_gl_enum_name(test_vectors[i].value),
			       name, got, test_vectors[i].expected);
			pass = false;
		}
	}

	glBindRenderbufferEXT(GL_RENDERBUFFER, 0);

	return piglit_check_gl_error(GL_NO_ERROR) && pass;
}
/*@}*/

/** \name Methods for operating on texture objects */
/*@{*/
#define TEXTURE_DATA_SIZE (16 * 16 * sizeof(GLuint))

static bool
create_texture(unsigned name)
{
	uint8_t data[TEXTURE_DATA_SIZE];

	generate_random_data(data, sizeof(data), GL_TEXTURE_2D, name);

	if (glIsTexture(name)) {
		printf("\t%s,%d: %u is already a texture\n",
		       __func__, __LINE__, name);
		return false;
	}

	glBindTexture(GL_TEXTURE_2D, name);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 16, 16, 0, GL_RGBA,
		     GL_UNSIGNED_INT_8_8_8_8, data);
	glBindTexture(GL_TEXTURE_2D, 0);

	return piglit_check_gl_error(GL_NO_ERROR);
}

static bool
validate_texture(unsigned name)
{
	static const struct enum_value_pair tex_test_vectors[] = {
		{ GL_TEXTURE_WRAP_S, GL_REPEAT },
		{ GL_TEXTURE_WRAP_T, GL_REPEAT },
		{ GL_TEXTURE_WRAP_R, GL_REPEAT },
		{ GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR },
		{ GL_TEXTURE_MAG_FILTER, GL_LINEAR },
		{ GL_TEXTURE_BASE_LEVEL, 0 },
		{ GL_TEXTURE_MAX_LEVEL, 1000 },
	};
	static const struct enum_value_pair tex_level_test_vectors[] = {
		{ GL_TEXTURE_WIDTH, 16 },
		{ GL_TEXTURE_HEIGHT, 16 },
		{ GL_TEXTURE_INTERNAL_FORMAT, GL_RGBA8 },
	};
	bool pass = true;
	uint8_t data[TEXTURE_DATA_SIZE];
	uint8_t texels[TEXTURE_DATA_SIZE];

	if (!glIsTexture(name)) {
		printf("\t%s,%d: %u is not a texture\n",
		       __func__, __LINE__, name);
		return false;
	}

	glBindTexture(GL_TEXTURE_2D, name);

	for (unsigned i = 0; i < ARRAY_SIZE(tex_test_vectors); i++) {
		GLint got;

		glGetTexParameteriv(GL_TEXTURE_2D,
				    tex_test_vectors[i].value,
				    &got);

		if (got != tex_test_vectors[i].expected) {
			printf("\t%s,%d: %s of %u: got 0x%x, expected 0x%x\n",
			       __func__, __LINE__,
			       piglit_get_gl_enum_name(tex_test_vectors[i].value),
			       name, got, tex_test_vectors[i].expected);
			pass = false;
		}
	}

	for (unsigned i = 0; i < ARRAY_SIZE(tex_level_test_vectors); i++) {
		GLint got;

		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
					 tex_level_test_vectors[i].value,
					 &got);

		if (got != tex_level_test_vectors[i].expected) {
			printf("\t%s,%d: %s of %u: got 0x%x, expected 0x%x\n",
			       __func__, __LINE__,
			       piglit_get_gl_enum_name(tex_level_test_vectors[i].value),
			       name, got, tex_level_test_vectors[i].expected);
			pass = false;
		}
	}

	/* Try to use glGetnTexImageARB.  If the test's 16x16 texture was
	 * replaced with something larger, the call to glGetTexImage will
	 * probably segfault.  This could be worked around, but it doesn't
	 * seem worth it.
	 *
	 * If the texture size did change, the glGetTexLevelParameteriv loop
	 * above will have already detected it.
	 */
	generate_random_data(data, sizeof(data), GL_TEXTURE_2D, name);
	if (piglit_is_extension_supported("GL_ARB_robustness")) {
		/* Note: if sizeof(texels) is less than the size of the image,
		 * glGetnTexImageARB will generate GL_INVALID_OPERATION.
		 */
		glGetnTexImageARB(GL_TEXTURE_2D, 0, GL_RGBA,
				  GL_UNSIGNED_INT_8_8_8_8,
				  sizeof(texels), texels);
	} else {
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA,
			      GL_UNSIGNED_INT_8_8_8_8,
			      texels);
	}

	if (memcmp(texels, data, sizeof(data)) != 0) {
		printf("\t%s,%d: Data mismatch in %u\n",
		       __func__, __LINE__, name);
		pass = false;
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	return piglit_check_gl_error(GL_NO_ERROR) && pass;
}
/*@}*/

/** \name Methods for operating on Apple vertex array objects */
/*@{*/
static float dummy_vao_data[257];

static const int vao_vertex_size[] = { 2, 3, 4 };
static const int vao_color_size[] = { 3, 4 };
static const int vao_texture_size[] = { 1, 2, 3, 4 };
static const GLenum vao_vertex_type[] = { GL_SHORT, GL_INT, GL_FLOAT, GL_DOUBLE };
static const GLenum vao_normal_type[] = { GL_BYTE, GL_SHORT, GL_INT, GL_FLOAT, GL_DOUBLE };
static const GLenum vao_color_type[] = {
	GL_BYTE, GL_UNSIGNED_BYTE,
	GL_SHORT, GL_UNSIGNED_SHORT,
	GL_INT,	GL_UNSIGNED_INT,
	GL_FLOAT,
	GL_DOUBLE
};

#define PICK(a, i) a[i % ARRAY_SIZE(a)]
#define STRIDE(x) (((x) % 16) * sizeof(int))

/* 4 arrays, and each has at most 4 values. */
#define VAO_DATA_SIZE (4 * 4)

static bool
create_vertex_array(unsigned name)
{
	uint8_t data[VAO_DATA_SIZE];

	if (!piglit_is_extension_supported("GL_APPLE_vertex_array_object")) {
		printf("%s requires GL_APPLE_vertex_array_object.\n", __func__);
		piglit_report_result(PIGLIT_SKIP);
	}

	if (glIsVertexArrayAPPLE(name)) {
		printf("\t%s,%d: %u is already a vertex array\n",
		       __func__, __LINE__, name);
		return false;
	}

	glBindVertexArrayAPPLE(name);

	generate_random_data(data,
			     sizeof(data),
			     GL_VERTEX_ARRAY_BINDING_APPLE,
			     name);

	glVertexPointer(PICK(vao_vertex_size, data[0]),
			PICK(vao_vertex_type, data[1]),
			STRIDE(data[2]),
			&dummy_vao_data[data[3]]);
	glNormalPointer(PICK(vao_normal_type, data[4]),
			STRIDE(data[5]),
			&dummy_vao_data[data[6]]);
	glColorPointer(PICK(vao_color_size, data[7]),
		       PICK(vao_color_type, data[8]),
		       STRIDE(data[9]),
		       &dummy_vao_data[data[10]]);
	glTexCoordPointer(PICK(vao_texture_size, data[11]),
			  PICK(vao_vertex_type, data[12]),
			  STRIDE(data[13]),
			  &dummy_vao_data[data[14]]);

	glBindVertexArrayAPPLE(0);

	return piglit_check_gl_error(GL_NO_ERROR);
}

static bool
check_integer_query(GLenum pname, int expected, unsigned name,
		    const char *caller)
{
	int got;

	glGetIntegerv(pname, &got);
	if (got != expected) {
		printf("\t%s,%d: %s of %u: got 0x%x, expected 0x%x\n",
		       caller, __LINE__,
		       piglit_get_gl_enum_name(pname),
		       name,
		       got, expected);
		return false;
	}

	return true;
}

static bool
check_pointer_query(GLenum pname, const void *expected, unsigned name,
		    const char *caller)
{
	void *got;

	glGetPointerv(pname, &got);
	if (got != expected) {
		printf("\t%s,%d: %s of %u: got %p, expected %p\n",
		       caller, __LINE__,
		       piglit_get_gl_enum_name(pname),
		       name,
		       got, expected);
		return false;
	}

	return true;
}

static bool
validate_vertex_array(unsigned name)
{
	uint8_t data[VAO_DATA_SIZE];
	bool pass = true;

	if (!piglit_is_extension_supported("GL_APPLE_vertex_array_object"))
		return true;

	if (!glIsVertexArrayAPPLE(name)) {
		printf("\t%s,%d: %u is not a vertex array\n",
		       __func__, __LINE__, name);
		return false;
	}

	glBindVertexArrayAPPLE(name);

	generate_random_data(data,
			     sizeof(data),
			     GL_VERTEX_ARRAY_BINDING_APPLE,
			     name);

	pass = check_integer_query(GL_VERTEX_ARRAY_SIZE,
				   PICK(vao_vertex_size, data[0]),
				   name,
				   __func__) && pass;
	pass = check_integer_query(GL_VERTEX_ARRAY_TYPE,
				   PICK(vao_vertex_type, data[1]),
				   name,
				   __func__) && pass;
	pass = check_integer_query(GL_VERTEX_ARRAY_STRIDE,
				   STRIDE(data[2]),
				   name,
				   __func__) && pass;
	pass = check_pointer_query(GL_VERTEX_ARRAY_POINTER,
				   &dummy_vao_data[data[3]],
				   name,
				   __func__) && pass;

	pass = check_integer_query(GL_NORMAL_ARRAY_TYPE,
				   PICK(vao_normal_type, data[4]),
				   name,
				   __func__) && pass;
	pass = check_integer_query(GL_NORMAL_ARRAY_STRIDE,
				   STRIDE(data[5]),
				   name,
				   __func__) && pass;
	pass = check_pointer_query(GL_NORMAL_ARRAY_POINTER,
				   &dummy_vao_data[data[6]],
				   name,
				   __func__) && pass;

	pass = check_integer_query(GL_COLOR_ARRAY_SIZE,
				   PICK(vao_color_size, data[7]),
				   name,
				   __func__) && pass;
	pass = check_integer_query(GL_COLOR_ARRAY_TYPE,
				   PICK(vao_color_type, data[8]),
				   name,
				   __func__) && pass;
	pass = check_integer_query(GL_COLOR_ARRAY_STRIDE,
				   STRIDE(data[9]),
				   name,
				   __func__) && pass;
	pass = check_pointer_query(GL_COLOR_ARRAY_POINTER,
				   &dummy_vao_data[data[10]],
				   name,
				   __func__) && pass;

	pass = check_integer_query(GL_TEXTURE_COORD_ARRAY_SIZE,
				   PICK(vao_texture_size, data[11]),
				   name,
				   __func__) && pass;
	pass = check_integer_query(GL_TEXTURE_COORD_ARRAY_TYPE,
				   PICK(vao_vertex_type, data[12]),
				   name,
				   __func__) && pass;
	pass = check_integer_query(GL_TEXTURE_COORD_ARRAY_STRIDE,
				   STRIDE(data[13]),
				   name,
				   __func__) && pass;
	pass = check_pointer_query(GL_TEXTURE_COORD_ARRAY_POINTER,
				   &dummy_vao_data[data[14]],
				   name,
				   __func__) && pass;

	glBindVertexArrayAPPLE(0);
	return piglit_check_gl_error(GL_NO_ERROR) && pass;
}
/*@}*/

/** \name GL operation wrapper functions. */
/*@{*/
static bool
do_Bitmap(void)
{
	GLint old_unpack_alignment;
	uint8_t bitmap[16 * 16 / 8];

	glGetIntegerv(GL_UNPACK_ALIGNMENT, &old_unpack_alignment);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	/* Enable depth test to avoid i965 blit path. */
	glEnable(GL_DEPTH_TEST);

	memset(bitmap, 0xff, sizeof(bitmap));
	glBitmap(16, 16, 0, 0, 0, 0, bitmap);

	glDisable(GL_DEPTH_TEST);
	glPixelStorei(GL_UNPACK_ALIGNMENT, old_unpack_alignment);

	return piglit_check_gl_error(GL_NO_ERROR);
}

static bool
do_BlitFramebuffer(void)
{
	const GLuint fbos[2] = { FIRST_SPARE_OBJECT, FIRST_SPARE_OBJECT + 1 };
	const GLuint tex[2] = { FIRST_SPARE_OBJECT, FIRST_SPARE_OBJECT + 1 };
	bool pass = true;

	/* GL_ARB_framebuffer_object and OpenGL 3.0 require that
	 * glGenFramebuffers be used.  This test really does require
	 * GL_EXT_framebuffer_object and GL_EXT_framebuffer_blit.
	 */
	if (!(piglit_is_extension_supported("GL_EXT_framebuffer_object") &&
	      piglit_is_extension_supported("GL_EXT_framebuffer_blit"))) {
		printf("%s requires EXT framebuffer objects.\n", __func__);
		piglit_report_result(PIGLIT_SKIP);
	}

	/* Generate the texture objects that will be attached to the
	 * framebuffer objects for the test.
	 */
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 16, 16, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 16, 16, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);

	/* Generate the framebuffer objects. */
	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, fbos[0]);
	glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  GL_TEXTURE_2D, tex[0], 0 /* level */);
	if (glCheckFramebufferStatusEXT(GL_DRAW_FRAMEBUFFER) !=
	    GL_FRAMEBUFFER_COMPLETE) {
		printf("\t%s,%d: Draw framebuffer is not complete.\n",
		       __func__, __LINE__);
		pass = false;
	}

	glBindFramebufferEXT(GL_READ_FRAMEBUFFER, fbos[1]);
	glFramebufferTexture2DEXT(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  GL_TEXTURE_2D, tex[1], 0 /* level */);
	if (glCheckFramebufferStatusEXT(GL_READ_FRAMEBUFFER) !=
	    GL_FRAMEBUFFER_COMPLETE) {
		printf("\t%s,%d: Read framebuffer is not complete.\n",
		       __func__, __LINE__);
		pass = false;
	}

	/* Do the "real" test. */
	glBlitFramebufferEXT(0 /* srcX0 */, 0 /* srcY0 */,
			     8 /* srcX1 */, 8 /* srcY1 */,
			     0 /* dstX0 */, 0 /* dstY0 */,
			     8 /* dstX1 */, 8 /* dstY1 */,
			     GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Final clean up. */
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);

	glDeleteTextures(ARRAY_SIZE(tex), tex);
	glDeleteFramebuffersEXT(ARRAY_SIZE(fbos), fbos);

	return piglit_check_gl_error(GL_NO_ERROR) && pass;
}

static bool
do_Clear(void)
{
	/* Pick a clear value that should avoid common hardware "fast clear"
	 * optimizations.
	 */
	glClearColor(0.5, 0.7, 0.8, 0.2);

	glClear(GL_COLOR_BUFFER_BIT);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	return piglit_check_gl_error(GL_NO_ERROR);
}

static bool
do_ClearTexSubImage(void)
{
	GLuint tex = FIRST_SPARE_OBJECT;

	/* Pick a clear value that should avoid common hardware "fast clear"
	 * optimizations.
	 */
	const GLuint clear_data = 0xDEADBEEF;

	if (!piglit_is_extension_supported("GL_ARB_clear_texture")) {
		printf("%s requires GL_ARB_clear_texture.\n", __func__);
		piglit_report_result(PIGLIT_SKIP);
	}

	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 16, 16, 0, GL_RGBA,
		     GL_UNSIGNED_INT_8_8_8_8, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glClearTexSubImage(tex, 0 /* level */,
			   0 /* xoffset */, 0 /* yoffset */, 0 /* zoffset */,
			   16 /* width */, 16 /* height */, 1 /* depth */,
			   GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
			   &clear_data);

	glDeleteTextures(1, &tex);

	return piglit_check_gl_error(GL_NO_ERROR);
}

static bool
do_CopyImageSubData(void)
{
	const GLuint tex[2] = { FIRST_SPARE_OBJECT, FIRST_SPARE_OBJECT + 1 };

	if (!piglit_is_extension_supported("GL_ARB_copy_image")) {
		printf("%s requires GL_ARB_copy_image.\n", __func__);
		piglit_report_result(PIGLIT_SKIP);
	}

	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 16, 16, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 16, 16, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);

	glCopyImageSubData(tex[0], GL_TEXTURE_2D, 0 /* srcLevel */,
			   0 /* srcX */, 0 /* srcY */, 0 /* srcZ */,
			   tex[1], GL_TEXTURE_2D, 0 /* dstLevel */,
			   0 /* dstX */, 0 /* dstY */, 0 /* dstZ */,
			   16 /* srcWidth */, 16 /* srcHeight */,
			   1 /* srcDepth */);

	glDeleteTextures(ARRAY_SIZE(tex), tex);

	return piglit_check_gl_error(GL_NO_ERROR);
}

static bool
do_CopyPixels(void)
{
	/* Set non-1.0 pixel zoom to avoid i965 blit path. */
	glPixelZoom(1.5f, 1.5f);

	glRasterPos2f(0.5, 0.5);
	glCopyPixels(0, 0, 4, 4, GL_COLOR);

	glPixelZoom(1.0f, 1.0f);

	return piglit_check_gl_error(GL_NO_ERROR);
}

static bool
do_CopyTexSubImage2D(void)
{
	GLuint tex = FIRST_SPARE_OBJECT;

	/* Set non-1.0 pixel zoom to avoid i965 blorp path. */
	glPixelZoom(1.5f, 1.5f);

	glBindTexture(GL_TEXTURE_2D, tex);

	/* Pick GL_LUMINANCE8_ALPHA8 because most hardware can support it
	 * natively, and most hardware cannot use a blit fast path from RGB or
	 * RGBA to LA.  i965 currently cannot.
	 */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8_ALPHA8, 16, 16, 0,
		     GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, NULL);

	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 16, 16);

	glPixelZoom(1.0f, 1.0f);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &tex);

	return piglit_check_gl_error(GL_NO_ERROR);
}

static bool
do_DrawPixels(void)
{
	GLuint pixels[16 * 16];

	memset(pixels, 0x81, sizeof(pixels));
	glDrawPixels(16, 16, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, pixels);
	glDrawPixels(16, 16, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, pixels);
	glDrawPixels(16, 16, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, pixels);

	return piglit_check_gl_error(GL_NO_ERROR);
}

static bool
do_GenerateMipmap(void)
{
	const GLuint tex = FIRST_SPARE_OBJECT;
	bool pass = true;

	if (!piglit_is_extension_supported("GL_EXT_framebuffer_object") &&
	    !piglit_is_extension_supported("GL_ARB_framebuffer_object") &&
	    piglit_get_gl_version() < 30) {
		printf("%s requires framebuffer objects.\n", __func__);
		piglit_report_result(PIGLIT_SKIP);
	}

	if (glIsTexture(tex)) {
		printf("\t%s,%d: %u is already a texture\n",
		       __func__, __LINE__, tex);
		pass = false;
	}

	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 16, 16, 0, GL_RGBA,
		     GL_UNSIGNED_INT_8_8_8_8, NULL);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &tex);

	return piglit_check_gl_error(GL_NO_ERROR) && pass;
}

static bool
do_GetTexImage(void)
{
	const GLuint tex = FIRST_SPARE_OBJECT;
	const GLuint pbo = FIRST_SPARE_OBJECT;
	uint8_t data[TEXTURE_DATA_SIZE];
	bool pass = true;

	if (!piglit_is_extension_supported("GL_EXT_pixel_buffer_object") &&
	    piglit_get_gl_version() < 30) {
		printf("%s requires pixel buffer objects.\n", __func__);
		piglit_report_result(PIGLIT_SKIP);
	}

	if (glIsTexture(tex)) {
		printf("\t%s,%d: %u is already a texture\n",
		       __func__, __LINE__, tex);
		pass = false;
	}

	if (glIsBuffer(pbo)) {
		printf("\t%s,%d: %u is already a buffer object\n",
		       __func__, __LINE__, pbo);
		pass = false;
	}

	/* Generate the initial texture object.  The random number seed values
	 * used are irrelevant.
	 */
	generate_random_data(data, sizeof(data), GL_PIXEL_UNPACK_BUFFER, pbo);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 16, 16, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, data);


	/* Generate the buffer object that will be used for the PBO download
	 * from the texture.
	 */
	glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
	glBufferData(GL_PIXEL_PACK_BUFFER, sizeof(data), NULL, GL_STATIC_READ);

	/* Do the "real" test. */
	glGetTexImage(GL_TEXTURE_2D, 0 /* level */,
		      GL_RGBA, GL_UNSIGNED_BYTE, BUFFER_OFFSET(0));

	/* Final clean up. */
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &tex);

	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	glDeleteBuffers(1, &pbo);

	return piglit_check_gl_error(GL_NO_ERROR) && pass;
}

static bool
do_GetTexImage_compressed(void)
{
	const GLuint tex = FIRST_SPARE_OBJECT;
	const GLenum internal_format =
		piglit_is_extension_supported("GL_EXT_texture_compression_s3tc")
		? GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
		: GL_COMPRESSED_RGBA_FXT1_3DFX;

	/* DXT1 has 4x4 block size.  The image will be 16x16 texels,
	 * so that means 16 blocks.  Each block is 64-bits (or 8
	 * bytes).  That's a total of 16 * 8 = 128 bytes.
	 *
	 * FXT1 has 8x4 block size.  The image will be 16x16 texels,
	 * so that means 8 blocks.  Each block is 128-bits (or 16
	 * bytes).  That's a total of 8 * 16 = 128 bytes.
	 */
	uint8_t data[128];

	/* Buffer to hold the decompressed texture. */
	uint8_t texels[16 * 16 * 4];

	bool pass = true;

	/* This test requires:
	 *
	 *   GL_EXT_texture_compression_s3tc or GL_3DFX_texture_compression_FXT1
	 *
	 * and
	 *
	 *   GL_ARB_texture_compression or OpenGL 1.3
	 */
	if (!(piglit_is_extension_supported("GL_ARB_texture_compression") ||
	      piglit_get_gl_version() >= 13) ||
	    !(piglit_is_extension_supported("GL_EXT_texture_compression_s3tc") ||
	      piglit_is_extension_supported("GL_3DFX_texture_compression_FXT1"))) {
		printf("%s requires either S3TC or FXT1 texture compression.\n",
		       __func__);
		piglit_report_result(PIGLIT_SKIP);
	}

	if (glIsTexture(tex)) {
		printf("\t%s,%d: %u is already a texture\n",
		       __func__, __LINE__, tex);
		pass = false;
	}

	/* Generate the initial texture object.  The random number seed values
	 * used are irrelevant.
	 */
	generate_random_data(data, sizeof(data), GL_PIXEL_UNPACK_BUFFER, tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glCompressedTexImage2D(GL_TEXTURE_2D, 0 /* level */, internal_format,
			       16 /* width */, 16 /* height */, 0 /* border */,
			       sizeof(data), data);

	/* Do the "real" test. */
	glGetTexImage(GL_TEXTURE_2D, 0 /* level */,
		      GL_RGBA, GL_UNSIGNED_BYTE, texels);

	/* Final clean up. */
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &tex);

	return piglit_check_gl_error(GL_NO_ERROR) && pass;
}

static bool
do_TexSubImage2D(void)
{
	const GLuint tex = FIRST_SPARE_OBJECT;
	const GLuint pbo = FIRST_SPARE_OBJECT;
	uint8_t data[TEXTURE_DATA_SIZE];
	bool pass = true;

	if (!piglit_is_extension_supported("GL_EXT_pixel_buffer_object") &&
	    piglit_get_gl_version() < 30) {
		printf("%s requires pixel buffer objects.\n", __func__);
		piglit_report_result(PIGLIT_SKIP);
	}

	if (glIsTexture(tex)) {
		printf("\t%s,%d: %u is already a texture\n",
		       __func__, __LINE__, tex);
		pass = false;
	}

	if (glIsBuffer(pbo)) {
		printf("\t%s,%d: %u is already a buffer object\n",
		       __func__, __LINE__, pbo);
		pass = false;
	}

	/* Generate the initial texture object.
	 *
	 * NOTE: This must occur before binding the PBO.  Otherwise
	 * the NULL texel pointer will be interpreted as a zero offset
	 * in the buffer, and glTexImage2D will upload data from the
	 * PBO.  This is not the intent of this test.
	 */
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 16, 16, 0, GL_RGBA,
		     GL_UNSIGNED_INT_8_8_8_8, NULL);


	/* Generate the buffer object that will be used for the PBO upload
	 * to the texture.
	 */
	generate_random_data(data, sizeof(data), GL_PIXEL_UNPACK_BUFFER, pbo);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, sizeof(data), data,
		     GL_STATIC_DRAW);

	/* Do the "real" test. */
	glTexSubImage2D(GL_TEXTURE_2D, 0 /* level */,
			0 /* xoffset */, 0 /* yoffset */,
			16 /* width */, 16 /* height */,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, BUFFER_OFFSET(0));

	/* Final clean up. */
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &tex);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	glDeleteBuffers(1, &pbo);

	return piglit_check_gl_error(GL_NO_ERROR) && pass;
}
/*@}*/

static const struct operation {
	const char *name;
	bool (*func)(void);
} operation_table[] = {
	{ "glBitmap", do_Bitmap },
	{ "glBlitFramebuffer", do_BlitFramebuffer },
	{ "glClear", do_Clear },
	{ "glClearTexSubImage", do_ClearTexSubImage },
	{ "glCopyImageSubData", do_CopyImageSubData },
	{ "glCopyPixels", do_CopyPixels },
	{ "glCopyTexSubImage2D", do_CopyTexSubImage2D },
	{ "glDrawPixels", do_DrawPixels },
	{ "glGenerateMipmap", do_GenerateMipmap },
	{ "glGetTexImage", do_GetTexImage },
	{ "glGetTexImage-compressed", do_GetTexImage_compressed },
	{ "glTexSubImage2D", do_TexSubImage2D },
};

static const struct object_type {
	const char *name;
	bool (*create)(unsigned name);
	bool (*validate)(unsigned name);
} object_type_table[] = {
	{ "buffer", create_buffer, validate_buffer },
	{ "framebuffer", create_framebuffer, validate_framebuffer },
	{ "program", create_program, validate_program },
	{ "renderbuffer", create_renderbuffer, validate_renderbuffer },
	{ "texture", create_texture, validate_texture },
	{ "vertex-array", create_vertex_array, validate_vertex_array },
};

static NORETURN void
usage(const char *prog)
{
	unsigned i;

	printf("Usage:\n");
	printf("\t%s operation object-type\n\n", prog);
	printf("Where operation is one of:\n");

	for (i = 0; i < ARRAY_SIZE(operation_table); i++)
		printf("\t%s\n", operation_table[i].name);

	printf("\nAnd object-type is one of:\n");

	for (i = 0; i < ARRAY_SIZE(object_type_table); i++)
		printf("\t%s\n", object_type_table[i].name);

	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	unsigned i;
	const struct object_type *object_type = NULL;
	const struct operation *operation = NULL;
	unsigned first_unused_texture;
	unsigned first_unused_framebuffer;

	if (argc != 3)
		usage(argv[0]);

	for (i = 0; i < ARRAY_SIZE(operation_table); i++) {
		if (strcmp(argv[1], operation_table[i].name) == 0) {
			operation = &operation_table[i];
			break;
		}
	}

	if (operation == NULL)
		usage(argv[0]);

	for (i = 0; i < ARRAY_SIZE(object_type_table); i++) {
		if (strcmp(argv[2], object_type_table[i].name) == 0) {
			object_type = &object_type_table[i];
			break;
		}
	}

	if (object_type == NULL)
		usage(argv[0]);

	printf("Test case %s with %s\n", object_type->name, operation->name);

	/* This is a bit ugly, but it is necessary.  When a test is run with
	 * -fbo, the piglit framework will create some textures before calling
	 * piglit_init.  These textures will likely have names that conflict
	 * with the names that are used by this test, so the test should avoid
	 * them.
	 *
	 * HOWEVER, the test should only avoid lower numbered textures.  If
	 * the piglit framework created a texture named 1, the test should
	 * still try to use a buffer object named 1.
	 */
	for (first_unused_texture = 1;
	     first_unused_texture < 16;
	     first_unused_texture++) {
		if (!glIsTexture(first_unused_texture))
			break;
	}

	if (first_unused_texture >= 16)
		piglit_report_result(PIGLIT_FAIL);

	for (first_unused_framebuffer = 1;
	     first_unused_framebuffer < 16;
	     first_unused_framebuffer++) {
		if (!glIsFramebufferEXT(first_unused_framebuffer))
			break;
	}

	if (first_unused_framebuffer >= 16)
		piglit_report_result(PIGLIT_FAIL);

	pass = operation->func() && pass;

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	for (unsigned name = 1; name < 16; name++) {
		if (strcmp("texture", object_type->name) == 0 &&
		    name < first_unused_texture)
			continue;

		if (strcmp("framebuffer", object_type->name) == 0 &&
		    name < first_unused_framebuffer)
			continue;

		pass = object_type->create(name) && pass;
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	pass = operation->func() && pass;

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	for (unsigned name = 1; name < 16; name++) {
		if (strcmp("texture", object_type->name) == 0 &&
		    name < first_unused_texture)
			continue;

		if (strcmp("framebuffer", object_type->name) == 0 &&
		    name < first_unused_framebuffer)
			continue;

		pass = object_type->validate(name) && pass;
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHABLE */
	return PIGLIT_FAIL;
}
