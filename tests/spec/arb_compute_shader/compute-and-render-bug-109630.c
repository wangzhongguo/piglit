/*
 * Copyright © 2020 Intel Corporation
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

/*
 * \file compute-and-render-bug-109630
 *
 * Test verifies that after compute workload geometry renders fine.
 * This exercises hardware issue in Intel's GEN9 GPU which resulted
 * in geometry flickering see https://bugs.freedesktop.org/show_bug.cgi?id=109630
 * There is no guarantied way to reproduce the issue but to run the
 * core part of the test many times.
 *
 * \author Andrii Kryvytskyi <andrii.o.kryvytskyi@globallogic.com>
 */

#include "piglit-util-gl.h"
PIGLIT_GL_TEST_CONFIG_BEGIN

config.supports_gl_core_version = 42;
config.window_height = 1024;
config.window_width = 1024;
config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static const char *cs_text =
    "#version 420\n"
    "#extension GL_ARB_compute_shader: require\n"
    "\n"
    "layout(local_size_x = 1) in; \n"
    "\n"
    "void main()\n"
    "{\n"
    "}\n";

static const GLchar* vs_text =
    "#version 140\n"
    "\n"
    "in vec3 pos;\n"
    "uniform mat3 transform;\n"
    "\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(pos * transform, 1.0f);\n"
    "}\n";

static const GLchar* fs_text =
    "#version 140\n"
    "\n"
    "uniform vec4 color;\n"
    "out vec4 fragColor;\n"
    "\n"
    "void main()\n"
    "{\n"
    "   fragColor = color;\n"
    "}\n";

static GLuint cs_prog;
static GLuint prog;
static GLuint vao;
static GLuint vbo;

void
piglit_init(int argc, char **argv)
{
    piglit_require_extension("GL_ARB_compute_shader");

    cs_prog = glCreateProgram();
    GLint compute_shader =
        piglit_compile_shader_text(GL_COMPUTE_SHADER, cs_text);

    glAttachShader(cs_prog, compute_shader);
    glLinkProgram(cs_prog);

    if (!piglit_link_check_status(cs_prog))
        piglit_report_result(PIGLIT_FAIL);

    const float vertices[] = {
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f
    };

    prog = piglit_build_simple_program(vs_text, fs_text);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    if (!piglit_check_gl_error(GL_NO_ERROR))
        piglit_report_result(PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void) {
    //Transform matrix is required for reproducing flickers.
    const float transform_matr[] = {
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0
    };

    bool pass = true;

    const float expected[] = { 0.0f, 1.0f, 0.0f, 1.0f };

    glBindBuffer(GL_ARRAY_BUFFER, vao);
    glBindVertexArray(vbo);

    GLint transform_loc = glGetUniformLocation(prog, "transform");
    GLint color_loc = glGetUniformLocation(prog, "color");

    for (int i = 0; i < 50; i++) {
        glClearColor(1.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(cs_prog);
        glDispatchCompute(1, 1, 1);

        glUseProgram(prog);

        glUniformMatrix3fv(transform_loc, 1, GL_FALSE, transform_matr);
        glUniform4f(color_loc, 1.0f, 1.0f, 0.0f, 1.0f);

        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 200);

        glUniform4f(color_loc, 0.0f, 1.0f, 0.0f, 1.0f);

        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 1);

        glFinish();

        if (!piglit_probe_pixel_rgba(0, 0, expected)) {
            printf("Failed on interation #%d \n", i);
            pass = false;
            break;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    piglit_present_results();

    return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
