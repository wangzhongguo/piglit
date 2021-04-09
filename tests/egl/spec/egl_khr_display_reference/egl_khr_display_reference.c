/*
 * Copyright 2021 Eric Engestrom
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

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>

#include "piglit-util.h"
#include "piglit-util-egl.h"


PFNEGLQUERYDISPLAYATTRIBEXTPROC queryDisplayAttrib;
PFNEGLGETPLATFORMDISPLAYEXTPROC getPlatformDisplayEXT;


static bool
check_tracking(EGLint platform_code,
               const EGLint *attr,
               EGLBoolean expected_tracking)
{
    EGLDisplay dpy = getPlatformDisplayEXT(platform_code,
                                           EGL_DEFAULT_DISPLAY,
                                           attr);
    if (!dpy) {
        printf("%s(): Failed to get EGLDisplay\n", __func__);
        piglit_report_result(PIGLIT_FAIL);
    }

    if (!eglInitialize(dpy, NULL, NULL)) {
        printf("%s(): eglInitialize(dpy) failed\n", __func__);
        piglit_report_result(PIGLIT_FAIL);
    }

    EGLAttrib query_tracking;
    if (!queryDisplayAttrib(dpy,
                            EGL_TRACK_REFERENCES_KHR,
                            &query_tracking)) {
        printf("%s(): Failed to query display\n", __func__);
        piglit_report_result(PIGLIT_FAIL);
    }

    if (!eglTerminate(dpy)) {
        printf("%s(): eglTerminate(dpy) failed\n", __func__);
        piglit_report_result(PIGLIT_FAIL);
    }

    return query_tracking == expected_tracking;
}

int
main(void)
{
    /*
     * From the spec:
     *
     *   Interactions with EGL_KHR_platform_android:
     *
     *       If eglGetPlatformDisplay() is called with <platform> set to
     *       EGL_PLATFORM_ANDROID_KHR, the default value of
     *       EGL_TRACK_REFERENCES_KHR is EGL_TRUE.
     *
     *   Interactions with EGL_EXT_platform_device, EGL_KHR_platform_gbm,
     *   EGL_KHR_platform_x11, and EGL_KHR_platform_wayland:
     *
     *       If eglGetPlatformDisplay() is called with <platform> set to
     *       EGL_PLATFORM_DEVICE_EXT, EGL_PLATFORM_GBM_KHR, EGL_PLATFORM_X11_KHR,
     *       or EGL_PLATFORM_WAYLAND_KHR, the default value of
     *       EGL_TRACK_REFERENCES_KHR is EGL_FALSE.
     */
    static const struct platform {
        EGLint code;
        EGLint default_value;
        const char *name;
    } platforms[] = {
        { .code = EGL_PLATFORM_ANDROID_KHR,      .name = "android",     .default_value = EGL_TRUE },
        { .code = EGL_PLATFORM_DEVICE_EXT,       .name = "device",      .default_value = EGL_FALSE },
        { .code = EGL_PLATFORM_GBM_KHR,          .name = "gbm",         .default_value = EGL_FALSE },
        { .code = EGL_PLATFORM_SURFACELESS_MESA, .name = "surfaceless", .default_value = EGL_DONT_CARE },
        { .code = EGL_PLATFORM_X11_KHR,          .name = "x11",         .default_value = EGL_FALSE },
        { .code = EGL_PLATFORM_WAYLAND_KHR,      .name = "wayland",     .default_value = EGL_FALSE },
    };
    enum piglit_result result = PIGLIT_SKIP;

    piglit_require_egl_extension(EGL_NO_DISPLAY, "EGL_KHR_display_reference");

    if (!piglit_is_egl_extension_supported(EGL_NO_DISPLAY,
                                           "EGL_EXT_platform_base")) {
        printf("EGL_EXT_platform_base is required by EGL_KHR_display_reference\n");
        piglit_report_result(PIGLIT_FAIL);
    }

    getPlatformDisplayEXT = (void *) eglGetProcAddress("eglGetPlatformDisplayEXT");
    if (!getPlatformDisplayEXT) {
        printf("eglGetPlatformDisplay() is NULL\n");
        piglit_report_result(PIGLIT_FAIL);
    }

    queryDisplayAttrib = (void *) eglGetProcAddress("eglQueryDisplayAttribKHR");
    if (!queryDisplayAttrib) {
        printf("eglQueryDisplayAttribKHR() is NULL\n");
        piglit_report_result(PIGLIT_FAIL);
    }

    for (int i = 0; i < ARRAY_SIZE(platforms); i++) {
        const struct platform *platform = &platforms[i];
        static const EGLint attr_track_ref[] = {
            EGL_TRACK_REFERENCES_KHR, EGL_TRUE,
            EGL_NONE
        };
        static const EGLint attr_no_tracking[] = {
            EGL_TRACK_REFERENCES_KHR, EGL_FALSE,
            EGL_NONE
        };
        EGLDisplay dpy1, dpy2;
        EGLAttrib query_tracking;

        if (!piglit_egl_get_default_display(platform->code)) {
            printf("Skipping unsupported platform 0x%X (%s)\n\n",
                   platform->code, platform->name);
            continue;
        }

        printf("Testing platform 0x%X (%s)\n",
               platform->code, platform->name);

        if (platform->default_value != EGL_DONT_CARE) {
            printf("Checking default value\n");
            if (!check_tracking(platform->code, NULL, platform->default_value)) {
                printf("Tracking should be %s by default, but is not\n",
                       platform->default_value ? "ON" : "OFF");
                piglit_report_result(PIGLIT_FAIL);
            }
        }


        /*
         * Check that tracking is enabled and disabled as requested.
         */

        printf("Trying to turn tracking ON\n");
        if (!check_tracking(platform->code, attr_track_ref, EGL_TRUE)) {
            printf("Failed to turn tracking ON\n");
            piglit_report_result(PIGLIT_FAIL);
        }

        printf("Trying to turn tracking OFF\n");
        if (!check_tracking(platform->code, attr_no_tracking, EGL_FALSE)) {
            printf("Failed to turn tracking OFF\n");
            piglit_report_result(PIGLIT_FAIL);
        }


        /*
         * Check that two eglInitialize() and one eglTerminate() leave the display
         * intact (internal ref > 0).
         */

        dpy1 = getPlatformDisplayEXT(platform->code,
                                     EGL_DEFAULT_DISPLAY,
                                     attr_track_ref);
        if (!dpy1) {
            printf("Failed to get EGLDisplay for platform %#x\n", platform->code);
            piglit_report_result(PIGLIT_FAIL);
        }

        if (!eglInitialize(dpy1, NULL, NULL)) {
            printf("First eglInitialize(dpy1) failed\n");
            piglit_report_result(PIGLIT_FAIL);
        }

        if (!eglInitialize(dpy1, NULL, NULL)) {
            printf("Second eglInitialize(dpy1) failed\n");
            piglit_report_result(PIGLIT_FAIL);
        }

        if (!eglTerminate(dpy1)) {
            printf("First eglTerminate(dpy1) failed\n");
            piglit_report_result(PIGLIT_FAIL);
        }

        /* check that dpy1 is still valid */
        if (!queryDisplayAttrib(dpy1,
                                EGL_TRACK_REFERENCES_KHR,
                                &query_tracking)) {
            printf("Looks like dpy1 was terminated prematurely\n");
            piglit_report_result(PIGLIT_FAIL);
        }

        if (!eglTerminate(dpy1)) {
            printf("Second eglTerminate(dpy1) failed\n");
            piglit_report_result(PIGLIT_FAIL);
        }

        /* check that dpy1 is no longer valid */
        if (queryDisplayAttrib(dpy1,
                               EGL_TRACK_REFERENCES_KHR,
                               &query_tracking)) {
            printf("Looks like dpy1 wasn't terminated when refcount reached zero\n");
            piglit_report_result(PIGLIT_FAIL);
        }


        /*
         * Check that two eglGetPlatformDisplay() with identical params result in
         * the same display (which guaranteed by the EGL spec), and that one
         * eglInitialize() on each and an eglTerminate() on the first one still
         * leaves the second display intact.
         */

        dpy2 = getPlatformDisplayEXT(platform->code,
                                     EGL_DEFAULT_DISPLAY,
                                     attr_track_ref);
        if (!dpy2) {
            printf("Failed to get EGLDisplay\n");
            piglit_report_result(PIGLIT_FAIL);
        }

        /* EGL spec guarantees this, but let's check anyway */
        if (dpy1 != dpy2) {
            printf("Second eglGetPlatformDisplay() didn't return the same display\n");
            piglit_report_result(PIGLIT_FAIL);
        }

        if (!eglInitialize(dpy1, NULL, NULL)) {
            printf("eglInitialize(dpy1) failed\n");
            piglit_report_result(PIGLIT_FAIL);
        }

        if (!eglInitialize(dpy2, NULL, NULL)) {
            printf("eglInitialize(dpy2) failed\n");
            piglit_report_result(PIGLIT_FAIL);
        }

        if (!eglTerminate(dpy1)) {
            printf("eglTerminate(dpy1) failed\n");
            piglit_report_result(PIGLIT_FAIL);
        }

        /* check that dpy2 is still valid */
        if (!queryDisplayAttrib(dpy2,
                                EGL_TRACK_REFERENCES_KHR,
                                &query_tracking)) {
            printf("Looks like dpy2 was terminated prematurely\n");
            piglit_report_result(PIGLIT_FAIL);
        }

        if (!eglTerminate(dpy2)) {
            printf("eglTerminate(dpy2) failed\n");
            piglit_report_result(PIGLIT_FAIL);
        }

        /* check that dpy2 is no longer valid */
        if (queryDisplayAttrib(dpy2,
                               EGL_TRACK_REFERENCES_KHR,
                               &query_tracking)) {
            printf("Looks like dpy2 wasn't terminated when refcount reached zero\n");
            piglit_report_result(PIGLIT_FAIL);
        }

        result = PIGLIT_PASS;
        printf("\n");
    }

    piglit_report_result(result);
}
