/*
 * Copyright (c) 2017 Timothy Arceri
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL VMWARE AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * Tests that api errors are thrown where expected for the
 * GL_EXT_memory_object extension.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20; /* Need 2.0 for DSA tests */
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END


static bool
test_gen_semaphores_value_errors()
{
	GLuint sem;

	glGenSemaphoresEXT(-1, &sem);

	return piglit_check_gl_error(GL_INVALID_VALUE);
}

static bool
test_delete_semaphores_value_errors()
{
	GLuint sem = 0;

	glDeleteSemaphoresEXT(-1, &sem);

	return piglit_check_gl_error(GL_INVALID_VALUE);
}

static bool
test_semaphore_parameter_enum_errors()
{
	GLuint sem;
	GLuint64 param;

	glGenSemaphoresEXT(1, &sem);

	/**
	 * The spec does not define any valid parameters
	 * in EXT_external_objects or in EXT_external_objects_fd
	 */
	glSemaphoreParameterui64vEXT(0, sem, &param);

	return piglit_check_gl_error(GL_INVALID_ENUM);
}

static bool
test_get_semaphore_parameter_enum_errors()
{
	GLuint sem;
	GLuint64 param;

	glGenSemaphoresEXT(1, &sem);
	glGetSemaphoreParameterui64vEXT(0, sem, &param);

	return piglit_check_gl_error(GL_INVALID_ENUM);
}

#define X(f, desc)					     	\
	do {							\
		const bool subtest_pass = (f);			\
		piglit_report_subtest_result(subtest_pass	\
					     ? PIGLIT_PASS : PIGLIT_FAIL, \
					     (desc));		\
		pass = pass && subtest_pass;			\
	} while (0)


enum piglit_result
piglit_display(void)
{
	bool pass = true;

	X(test_gen_semaphores_value_errors(), "gen-semaphores-bad-value");
	X(test_delete_semaphores_value_errors(), "delete-semaphores-bad-value");
	X(test_semaphore_parameter_enum_errors(), "semaphore-parameter-bad-enum");
	X(test_get_semaphore_parameter_enum_errors(), "get-semaphore-parameter-bad-enum");

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_semaphore");
}
