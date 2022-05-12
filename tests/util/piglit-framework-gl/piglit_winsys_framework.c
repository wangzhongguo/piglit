/*
 * Copyright © 2012 Intel Corporation
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

#include <assert.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif

#include "piglit-util-gl.h"
#include "piglit-util-waffle.h"

#include "piglit_gbm_framework.h"
#include "piglit_gl_framework.h"
#include "piglit_wgl_framework.h"
#include "piglit_winsys_framework.h"
#include "piglit_wl_framework.h"
#include "piglit_x11_framework.h"
#include "piglit_sl_framework.h"

struct piglit_winsys_framework*
piglit_winsys_framework(struct piglit_gl_framework *gl_fw)
{
	return (struct piglit_winsys_framework*) gl_fw;
}

static void
swap_buffers(struct piglit_gl_framework *gl_fw)
{
	waffle_window_swap_buffers(piglit_wfl_framework(gl_fw)->window);
}

static void
run_test(struct piglit_gl_framework *gl_fw,
         int argc, char *argv[])
{
	struct piglit_winsys_framework *winsys_fw = piglit_winsys_framework(gl_fw);
	bool no_window = false;
	const char *env_no_window = getenv("PIGLIT_NO_WINDOW");


	if (env_no_window != NULL) {
		if (strcmp(env_no_window, "0") == 0) {
			no_window = false;
		} else if (strcmp(env_no_window, "1") == 0) {
			no_window = true;
		} else {
			fprintf(stderr, "PIGLIT_NO_WINDOW has invalid"
				" value: %s\n", env_no_window);
			abort();
		}
	}

	if (gl_fw->test_config->init)
		gl_fw->test_config->init(argc, argv);

	if (!gl_fw->test_config->requires_displayed_window &&
	    piglit_automatic && no_window) {
		enum piglit_result result = PIGLIT_PASS;
		if (gl_fw->test_config->display)
			result = gl_fw->test_config->display();

		piglit_report_result(result);
	}

	/* In non-auto mode, the user wishes to see the window regardless
	 * of the value of piglit_gl_test_config::require_displayed_window.
	 */
	winsys_fw->show_window(winsys_fw);
	winsys_fw->enter_event_loop(winsys_fw);
	abort();
}

static void
set_keyboard_func(struct piglit_gl_framework *gl_fw,
                  void (*func)(unsigned char key, int x, int y))
{
	piglit_winsys_framework(gl_fw)->user_keyboard_func = func;
}

static void
post_redisplay(struct piglit_gl_framework *gl_fw)
{
	piglit_winsys_framework(gl_fw)->need_redisplay = true;
}

/**
 * Create a integer array of WAFFLE_x attribute/value pairs from
 * the given test_config (which specifies color, depth, samples, etc.
 * \param min_color  minimum bits per channel for color components.
 * \param min_z  minimum bits for depth/Z buffer.
 */
static const int32_t*
choose_config_attribs(const struct piglit_gl_test_config *test_config,
		      int min_color, int min_z)
{
	static int32_t attrib_list[64];
	int i = 0;


	if (test_config->window_visual &
	    (PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_RGBA)) {
		attrib_list[i++] = WAFFLE_RED_SIZE;
		attrib_list[i++] = min_color;
		attrib_list[i++] = WAFFLE_GREEN_SIZE;
		attrib_list[i++] = min_color;
		attrib_list[i++] = WAFFLE_BLUE_SIZE;
		attrib_list[i++] = min_color;
	}

	if (test_config->window_visual & PIGLIT_GL_VISUAL_RGBA) {
		attrib_list[i++] = WAFFLE_ALPHA_SIZE;
		attrib_list[i++] = min_color;
	}

	if (test_config->window_visual & PIGLIT_GL_VISUAL_DEPTH) {
		attrib_list[i++] = WAFFLE_DEPTH_SIZE;
		attrib_list[i++] = min_z;
	}

	if (test_config->window_visual & PIGLIT_GL_VISUAL_STENCIL) {
		attrib_list[i++] = WAFFLE_STENCIL_SIZE;
		attrib_list[i++] = 1;
	}

	if (!(test_config->window_visual & PIGLIT_GL_VISUAL_DOUBLE)) {
		attrib_list[i++] = WAFFLE_DOUBLE_BUFFERED;
		attrib_list[i++] = false;
	}

	if (test_config->window_visual & PIGLIT_GL_VISUAL_ACCUM) {
		attrib_list[i++] = WAFFLE_ACCUM_BUFFER;
		attrib_list[i++] = true;
	}

	if (test_config->window_samples > 1) {
		attrib_list[i++] = WAFFLE_SAMPLE_BUFFERS;
		attrib_list[i++] = 1;
		attrib_list[i++] = WAFFLE_SAMPLES;
		attrib_list[i++] = test_config->window_samples;
	}

	attrib_list[i++] = WAFFLE_NONE;

	return attrib_list;
}

struct piglit_gl_framework*
piglit_winsys_framework_factory(const struct piglit_gl_test_config *test_config)
{
	int32_t platform = piglit_wfl_framework_choose_platform(test_config);

	switch (platform) {
#ifdef PIGLIT_HAS_X11
	case WAFFLE_PLATFORM_GLX:
	case WAFFLE_PLATFORM_X11_EGL:
		return piglit_x11_framework_create(test_config, platform);
#endif

#ifdef PIGLIT_HAS_GBM
	case WAFFLE_PLATFORM_GBM:
		return piglit_gbm_framework_create(test_config);
#endif

#ifdef PIGLIT_HAS_WAYLAND
	case WAFFLE_PLATFORM_WAYLAND:
		return piglit_wl_framework_create(test_config);
#endif

#ifdef PIGLIT_HAS_WGL
	case WAFFLE_PLATFORM_WGL:
		return piglit_wgl_framework_create(test_config);
#endif

#ifdef PIGLIT_HAS_EGL
	case WAFFLE_PLATFORM_SURFACELESS_EGL:
		return piglit_sl_framework_create(test_config);
#endif

	default:
		assert(0);
		return NULL;
	}
}

bool
piglit_winsys_framework_init(struct piglit_winsys_framework *winsys_fw,
                             const struct piglit_gl_test_config *test_config,
                             int32_t platform)
{
	struct piglit_wfl_framework *wfl_fw = &winsys_fw->wfl_fw;
	struct piglit_gl_framework *gl_fw = &wfl_fw->gl_fw;
	bool ok = true;

	// First try to get 8-bit color channels, 24-bit Z
	ok = piglit_wfl_framework_init(wfl_fw, test_config, platform,
				choose_config_attribs(test_config, 8, 24));
	if (!ok) {
		// Try shallower color/Z.  We may get 565 color and 16-bit Z.
		ok = piglit_wfl_framework_init(wfl_fw, test_config, platform,
				choose_config_attribs(test_config, 1, 1));
	}

	if (!ok)
		goto fail;

	winsys_fw->user_keyboard_func = piglit_escape_exit_key;

	wfl_fw->gl_fw.post_redisplay = post_redisplay;
	wfl_fw->gl_fw.set_keyboard_func = set_keyboard_func;

	gl_fw->run_test = run_test;
	gl_fw->swap_buffers = swap_buffers;

	return true;

fail:
	piglit_winsys_framework_teardown(winsys_fw);
	return false;
}

void
piglit_winsys_framework_teardown(struct piglit_winsys_framework *winsys_fw)
{
	piglit_wfl_framework_teardown(&winsys_fw->wfl_fw);
}
