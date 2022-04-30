/* Copyright © 2011 Intel Corporation
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
#include "piglit-glx-util.h"
#include "common.h"

static bool try_render_type(int type)
{
	const int attribs[] = {
		GLX_RENDER_TYPE, type,
		None
	};
	GLXContext ctx;
	bool pass = true;

	ctx = glXCreateContextAttribsARB(dpy, fbconfig, NULL, True, attribs);
	XSync(dpy, 0);

	if (ctx != NULL) {
		fprintf(stderr,
			"Created OpenGL context with invalid render-type "
			"0x%08x, but this should have failed.\n",
			type);
		glXDestroyContext(dpy, ctx);
		pass = false;
	}

	/* The GLX_ARB_create_context spec says:
	 *
	 *     "* If attribute GLX_RENDER_TYPE does not describe a valid
	 *        rendering type, BadValue is generated."
	 */
	if (!validate_glx_error_code(BadValue, -1)) {
		if (ctx == NULL)
			fprintf(stderr, "Render type = 0x%08x\n", type);

		pass = false;
	}

	return pass;
}

int main(int argc, char **argv)
{
	static const int invalid_render_types[] = {
		GLX_COLOR_INDEX_BIT,
		GLX_RGBA_BIT,
		GLX_RGBA_FLOAT_BIT_ARB,
		GLX_RGBA_UNSIGNED_FLOAT_BIT_EXT,
		0,
		-1,
		~GLX_RGBA_TYPE,
		~GLX_COLOR_INDEX_TYPE
	};
	bool pass = true;
	unsigned i;

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-auto"))
			piglit_automatic = 1;
		else
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
	}

	GLX_ARB_create_context_setup();

	for (i = 0; i < ARRAY_SIZE(invalid_render_types); i++) {
		pass = try_render_type(invalid_render_types[i])
			&& pass;
	}

	if (!piglit_is_glx_extension_supported(dpy, "GLX_ARB_fbconfig_float")) {
		pass = try_render_type(GLX_RGBA_FLOAT_TYPE_ARB)
			&& pass;
	}

	if (!piglit_is_glx_extension_supported(dpy, "GLX_EXT_fbconfig_packed_float")) {
		pass = try_render_type(GLX_RGBA_UNSIGNED_FLOAT_TYPE_EXT)
			&& pass;
	}

	GLX_ARB_create_context_teardown();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	return 0;
}
