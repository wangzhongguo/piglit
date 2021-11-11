/*
 * Copyright 2011 Red Hat, Inc.
 * Copyright 2011 Intel Corporation.
 * Copyright 2021 Advanced Micro Devices, Inc.
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
 * Test single pbuffer can be used across display.
 *
 * From GLX spec:
 *
 *   This creates a single GLXPbuffer and returns its XID. Like other drawable types,
 *   GLXPbuffers are shared; any client which knows the associated XID can use a
 *   GLXPbuffer.
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

int piglit_width = 50, piglit_height = 50;

static GLXFBConfig
get_fbconfig(Display *dpy)
{
	int n;
	int attrs[] = {
		GLX_DOUBLEBUFFER, 0,
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT | GLX_WINDOW_BIT,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		None
	};
	GLXFBConfig *configs = glXChooseFBConfig(dpy, DefaultScreen(dpy), attrs, &n);
	if (!configs || n < 1) {
		fprintf(stderr, "Can't find proper fbconfig.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	return configs[0];
}

static GLXPbuffer
get_pbuffer(Display *dpy, GLXFBConfig config)
{
	int attrs[] = {
		GLX_PBUFFER_WIDTH, piglit_width,
		GLX_PBUFFER_HEIGHT, piglit_height,
		GLX_PRESERVED_CONTENTS, True,
		GLX_LARGEST_PBUFFER, False,
		None
	};
	GLXPbuffer pbuffer = glXCreatePbuffer(dpy, config, attrs);
	if (!pbuffer) {
		fprintf(stderr, "Fail to create pbuffer.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	return pbuffer;
}

static GLXContext
get_context(Display *dpy, GLXFBConfig config)
{
	GLXContext ctx = glXCreateNewContext(dpy, config, GLX_RGBA_TYPE, NULL, True);
	if (!ctx) {
		fprintf(stderr, "Fail to create context.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	return ctx;
}

static void
create_displays(Display *dpy[], GLXFBConfig config[], GLXContext ctx[])
{
	int i;
	for (i = 0; i < 2; i++) {
		dpy[i] = XOpenDisplay(NULL);
		if (dpy[i] == NULL) {
			fprintf(stderr, "couldn't open display %d\n", i);
			piglit_report_result(PIGLIT_FAIL);
		}

		piglit_glx_get_error(dpy[i], NULL);
		piglit_require_glx_version(dpy[i], 1, 3);

		config[i] = get_fbconfig(dpy[i]);
		ctx[i] = get_context(dpy[i], config[i]);
	}
}

static void
test_cross_display(Display *dpy1, GLXContext ctx1,
		   Display *dpy2, GLXContext ctx2,
		   GLXPbuffer pbuffer, const float color[])
{
	bool pass;

	/* Display 1 */
	glXMakeCurrent(dpy1, pbuffer, ctx1);
	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	glClearColor(color[0], color[1], color[2], color[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Make sure pbuffer has been updated. */
	glFlush();

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, color);
	if (!pass) {
		fprintf(stderr, "Clear check fail for first display.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Display 2 */
	glXMakeCurrent(dpy2, pbuffer, ctx2);

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, color);
	if (!pass) {
		fprintf(stderr, "Init color check fail for second display.\n");
		piglit_report_result(PIGLIT_FAIL);
	}
}

int
main(int argc, char **argv)
{
	Display *dpy[2];
	GLXFBConfig config[2];
	GLXContext ctx[2];
	GLXPbuffer pbuffer;
	const float green[] = {0.0, 1.0, 0.0, 0.0};
	const float red[] = {1.0, 0.0, 0.0, 0.0};

	/* create multi display */
	create_displays(dpy, config, ctx);

	/* create pbuffer on first display */
	pbuffer = get_pbuffer(dpy[0], config[0]);

	/* test pbuffer from dpy[0] to dpy[1] */
	test_cross_display(dpy[0], ctx[0], dpy[1], ctx[1], pbuffer, green);

	/* test pbuffer from dpy[1] to dpy[0] */
	test_cross_display(dpy[1], ctx[1], dpy[0], ctx[0], pbuffer, red);

	piglit_report_result(PIGLIT_PASS);

	return 0;
}
