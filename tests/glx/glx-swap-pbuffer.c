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
 * Test swap buffer works for pbuffer.
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

int piglit_width = 50, piglit_height = 50;

static GLXFBConfig
get_fbconfig(Display *dpy)
{
	int n;
	int attrs[] = {
		GLX_DOUBLEBUFFER, 1,
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
clear_and_check_result(Display *dpy, GLXPbuffer pbuffer, const float *color)
{
	bool pass;

	glClearColor(color[0], color[1], color[2], color[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	glXSwapBuffers(dpy, pbuffer);

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, color);
	if (!pass) {
		fprintf(stderr, "Clear check fail for color (%f %f %f %f).\n",
			color[0], color[1], color[2], color[3]);
		piglit_report_result(PIGLIT_FAIL);
	}
}

int
main(int argc, char **argv)
{
	Display *dpy;
	GLXFBConfig config;
	GLXPbuffer pbuffer;
	GLXContext ctx;
	const float green[] = {0.0, 1.0, 0.0, 0.0};
	const float red[] = {1.0, 0.0, 0.0, 0.0};

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_glx_get_error(dpy, NULL);
	piglit_require_glx_version(dpy, 1, 3);

	config = get_fbconfig(dpy);
	pbuffer = get_pbuffer(dpy, config);
	ctx = get_context(dpy, config);

	glXMakeCurrent(dpy, pbuffer, ctx);
	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	/* Check front buffer has been updated after swap buffer. */
	glReadBuffer(GL_FRONT);

	clear_and_check_result(dpy, pbuffer, green);
	clear_and_check_result(dpy, pbuffer, red);

	glXDestroyPbuffer(dpy, pbuffer);

	piglit_report_result(PIGLIT_PASS);

	return 0;
}
