/*
 * Copyright (c) 2021 VMware, Inc.
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
 *
 * This test was designed to exercise as many of the Mesa Gallium draw
 * module's vertex pipeline stages as possible, particularly those which
 * allocate extra vertex attributes.
 *
 * It does not yet check for correctness, and is instead meant to be used with
 * memory debuggers to trap buffer overflows.
 *
 * Authors:
 *    Jose Fonseca <jfonseca@vmware.com>
 */

#include "piglit-util-gl.h"

static const GLint W = 800;
static const GLint H = 600;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 30;

	config.window_width = W;
	config.window_height = H;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const char *
vertexShader =
	"#version 130\n"
	"in vec4 A0;\n"
	"in vec4 A1;\n"
	"flat out vec4 fs_color0;\n"
	"flat out vec4 fs_color1;\n"
	"flat out vec4 fs_generic0;\n"
	"uniform vec4 CS[6];\n"
	"void main() {\n"
	"  vec4 position;\n"
	"  position  = A0.xxxx * CS[0];\n"
	"  position += A0.yyyy * CS[1];\n"
	"  position += A0.zzzz * CS[2];\n"
	"  position += A0.wwww * CS[3];\n"
	"  gl_ClipDistance[0] = dot(position, CS[4]);\n"
	"  gl_ClipDistance[1] = dot(position, CS[5]);\n"
	"  gl_Position = position;\n"
	"  fs_color0 = A1;\n"
	"  fs_color1 = vec4(0, 0, 0, 0);\n"
	"  fs_generic0 = vec4(0, 0, 0, 1);\n"
	"}\n"
;

static const char *
fragmentShader =
	"#version 130\n"
	"#extension GL_EXT_gpu_shader4 : require\n"
	"flat in vec4 fs_color0;\n"
	"out vec4 fragColor0;\n"
	"void main(){\n"
	"  fragColor0.r = gl_PrimitiveID / 12.0;\n"
	"  fragColor0.g = gl_FrontFacing ? 1.0 : 0.0;\n"
	"  fragColor0.b = fs_color0.b;\n"
	"  fragColor0.a = 1.0;\n"
	"}\n"
;

static const float
uniforms[][4] = {
	{ 0.90533f, 3.223402e-08f, -2.673027e-08f, -2.670354e-08f },
	{ -2.417551e-08f, 1.207107f, 5.346054e-08f, 5.340707e-08f },
	{ 4.835103e-08f, -1.289361e-07f, 0.5005005f, 0.5f },
	{ 0.0f, 0.0f, 2.902903f, 3.0f },
	{ 0.0f, 0.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f, 0.0f },
};

typedef struct Vertex {
	 float position[3];
	 unsigned color;
} Vertex;

static const Vertex
vertices[] = {
	 { { -1.0f, -1.0f, -1.0f }, 0xffffffffU },
	 { { -1.0f, -1.0f,  1.0f }, 0xffffff00U },
	 { { -1.0f,  1.0f, -1.0f }, 0xffff00ffU },
	 { { -1.0f,  1.0f,  1.0f }, 0xffff0000U },
	 { {  1.0f, -1.0f, -1.0f }, 0xff00ffffU },
	 { {  1.0f, -1.0f,  1.0f }, 0xff00ff00U },
	 { {  1.0f,  1.0f, -1.0f }, 0xff0000ffU },
	 { {  1.0f,  1.0f,  1.0f }, 0xff000000U },
};

static const GLushort
indices[] = {
	 0, 1, 3,
	 3, 2, 0,
	 4, 5, 7,
	 7, 6, 4,
	 0, 1, 5,
	 5, 4, 0,
	 2, 3, 7,
	 7, 6, 2,
	 0, 2, 6,
	 6, 4, 0,
	 1, 3, 7,
	 7, 5, 1,
};

static const GLubyte
stipple[32][4] = {
	{ 0x80, 0x80, 0x80, 0x80 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x80, 0x80, 0x80, 0x80 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x80, 0x80, 0x80, 0x80 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x80, 0x80, 0x80, 0x80 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 },
};

enum piglit_result piglit_display(void)
{
	GLuint program = glCreateProgram();
	GLuint vs = piglit_compile_shader_text(GL_VERTEX_SHADER,
			vertexShader);
	GLuint fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER,
			fragmentShader);
	glAttachShader(program, vs);
	glDeleteShader(vs);
	glAttachShader(program, fs);
	glDeleteShader(fs);
	glBindAttribLocation(program, 0, "A0");
	glBindAttribLocation(program, 1, "A1");
	glBindFragDataLocation(program, 0, "fragColor0");
	glLinkProgram(program);
	glDetachShader(program, fs);
	glDetachShader(program, vs);
	if (!piglit_link_check_status(program)) {
		glDeleteProgram(program);
		piglit_report_result(PIGLIT_FAIL);
	}
	glUseProgram(program);
	GLint l0 = glGetUniformLocation(program, "CS[0]");
	assert(l0 >= 0);
	glUniform4fv(l0, ARRAY_SIZE(uniforms), uniforms[0]);

	glViewport(0, 0, W, H);
	glDepthRange(0.0, 1.0);
	glEnable(GL_SCISSOR_TEST);
	glScissor(0, 0, W, H);
	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CLIP_DISTANCE0);
	glEnable(GL_CLIP_DISTANCE1);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	glFrontFace(GL_CCW);

	/* Smooth points for front faces.
	 */
	glPolygonMode(GL_FRONT, GL_POINT);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(9.0);
	if (GL_FALSE) {
		glEnable(GL_POLYGON_STIPPLE);
		glPolygonStipple(stipple[0]);
	}

	/* Smooth lines for back faces.
	 */
	glPolygonMode(GL_BACK, GL_LINE);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(3.0);
	glEnable(GL_LINE_STIPPLE);
	glLineStipple(3, 0xfafa);

	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	GLuint vb = 0;
	glCreateBuffers(1, &vb);
	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);
	GLuint ib = 0;
	glCreateBuffers(1, &ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
		(const void *)offsetof(Vertex, position));
	glVertexAttribPointer(1, GL_BGRA, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex),
		(const void *)offsetof(Vertex, color));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glValidateProgram(program);
	GLint status = GL_FALSE;
	glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
	if (status != GL_TRUE) {
		piglit_report_result(PIGLIT_FAIL);
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glDrawElementsBaseVertex(GL_TRIANGLES, ARRAY_SIZE(indices),
		GL_UNSIGNED_SHORT, NULL, 0);

	piglit_present_results();

	glBindVertexArray(0);
	glUseProgram(0);
	glDeleteBuffers(1, &vb);
	glDeleteBuffers(1, &ib);
	glDeleteVertexArrays(1, &vao);
	glDeleteProgram(program);

	return PIGLIT_PASS;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_gpu_shader4");
}
