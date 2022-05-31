/*
 * Copyright © 2016 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** @file layout-std430-fp64-mixed-shader.c
 *
 * Tests that shader storage block reads/writes in GLSL works correctly (offsets
 * and values) when interface packing qualifier is std430, using mixed content.
 *
 * From GL_ARB_shader_storage_buffer_object:
 *
 *  "When using the "std430" storage layout, shader storage
 *   blocks will be laid out in buffer storage identically to uniform and
 *   shader storage blocks using the "std140" layout, except that the base
 *   alignment of arrays of scalars and vectors in rule (4) and of structures
 *   in rule (9) are not rounded up a multiple of the base alignment of a vec4."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.window_width = 100;
	config.window_height = 100;
	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END

#define SSBO_SIZE1 4
#define SSBO_SIZE2 6
#define SSBO_SIZE3 4
#define SSBO_SIZE4 58

#define TOLERANCE 1e-5
#define DIFFER(a,b) ((a > b ? a - b : b - a) > TOLERANCE)

static const char vs_code[] =
	"#version 150\n"
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
	"#extension GL_ARB_gpu_shader_fp64 : require\n"
	"\n"
	"struct B { double b1[3]; };\n"
	"struct A {\n"
	"	double a1;\n"
	"	dvec3 a2;\n"
	"	dmat2 a4[2];\n"
	"	double a5;\n"
	"	B sb[2];\n"
	"};\n"
	"\n"
	"layout(std430, binding=2) buffer ssbo {\n"
	"	vec2 u;\n"
	"	dvec4 v;\n"
	"	float f;\n"
	"	A s;\n"
	"	dmat3x4 m;\n"
	"	dvec2 v2a[3];\n"
	"	dvec3 v3a[2];\n"
	"	double unsized_array[];\n"
	"};\n"
	"\n"
	"in vec4 piglit_vertex;\n"
	"out vec4 vertex_to_gs;\n"
	"\n"
	"void main() {\n"
	"	vertex_to_gs = piglit_vertex;\n"
	"	v.yz = dvec2(1.0, 2.0);\n"
	"	f = 4.0;\n"
	"	s.a2.x = 6.0lf;\n"
	"	s.a2.y = 7.0lf;\n"
	"	s.sb[0].b1[0] = 18.333333333333333259lf;\n"
	"	s.sb[0].b1[1] = 19.0lf;\n"
	"	m[1] = dvec4(25.0, 26.0, 27.0, 28.0);\n"
	"	v2a[0].yx = dvec2(34.0, 33.0) * s.a5;\n"
	"	v2a[1].y = 36.0lf;\n"
	"	v3a[0].xz = dvec2(39.0, 41.0);\n"
	"	v3a[1].y = 43.0lf;\n"
	"	int index = int(v.x); // index should be zero\n"
	"	unsized_array[index + gl_VertexID] = unsized_array.length();\n"
	"}\n";

static const char gs_source[] =
	"#version 150\n"
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
	"#extension GL_ARB_gpu_shader_fp64 : require\n"
	"\n"
	"struct B { double b1[3]; };\n"
	"struct A {\n"
	"	double a1;\n"
	"	dvec3 a2;\n"
	"	dmat2 a4[2];\n"
	"	double a5;\n"
	"	B sb[2];\n"
	"};\n"
	"\n"
	"layout(std430, binding=2) buffer ssbo {\n"
	"	vec2 u;\n"
	"	dvec4 v;\n"
	"	float f;\n"
	"	A s;\n"
	"	dmat3x4 m;\n"
	"	dvec2 v2a[3];\n"
	"	dvec3 v3a[2];\n"
	"	double unsized_array[];\n"
	"};\n"
	"layout(triangles) in;\n"
	"layout(triangle_strip, max_vertices = 3) out;\n"
	"\n"
	"in vec4 vertex_to_gs[3];\n"
	"\n"
	"void main() {\n"
	"	for (int i = 0; i < 3; i++) {\n"
	"		gl_Position = vertex_to_gs[i] + vec4(s.a1);\n"
	"		EmitVertex();\n"
	"	}\n"
	"	s.a4[0] = dmat2(-1.333333333333333259lf, 11.0, 12.0, 13.0);\n"
	"}\n";

static const char fs_source[] =
	"#version 150\n"
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
	"#extension GL_ARB_gpu_shader_fp64 : require\n"
	"\n"
	"struct B { double b1[3]; };\n"
	"struct A {\n"
	"	double a1;\n"
	"	dvec3 a2;\n"
	"	dmat2 a4[2];\n"
	"	double a5;\n"
	"	B sb[2];\n"
	"};\n"
	"\n"
	"layout(std430, binding=2) buffer ssbo {\n"
	"	vec2 u;\n"
	"	dvec4 v;\n"
	"	float f;\n"
	"	A s;\n"
	"	dmat3x4 m;\n"
	"	dvec2 v2a[3];\n"
	"	dvec3 v3a[2];\n"
	"	double unsized_array[];\n"
	"};\n"
	"\n"
	"out vec4 color;\n"
	"\n"
	"void main() {\n"
	"	color = vec4(0,1,0,1);\n"
	"	v.xw = dvec2(0.0, 3.0) + u;\n"
	"	s.a2.z = 8.0lf;\n"
	"	s.a4[1] = dmat2(14.0, 15.0, 16.0, 17.0);\n"
	"	s.sb[1].b1[2] = 20.0lf;\n"
	"	m[0] = dvec4(21.333333333333333259lf, 22.0, 23.0, 24.0);\n"
	"	m[2] = dvec4(29.0, 30.0, 31.0, 32.0);\n"
	"	v2a[1].x = 35.0lf;\n"
	"	v2a[2].xy = dvec2(37.0, 38.0);\n"
	"	v3a[0].y = 40.0lf;\n"
	"	v3a[1].xz = dvec2(42.0, 44.0);\n"
	"	int index = int(v.z + gl_FragCoord.x);\n"
	"	unsized_array[index] = unsized_array.length() * 2.0LF;\n"
	"}\n";

GLuint prog;

float ssbo_values1[SSBO_SIZE1]  = { 6.0, 7.0, 0.0, 0.0,  // vec2 u
};

double ssbo_values2[SSBO_SIZE2] = { 0.0, 0.0,		 // padding
				    0.0, 0.0, 0.0, 0.0,  // dvec4 v
};

float ssbo_values3[SSBO_SIZE3]  = { 0.0, 0.0, 0.0, 0.0,  // float f
};

double ssbo_values4[SSBO_SIZE4] = { 0.0, 0.0,		 // padding
				    1.0, 0.0, 0.0, 0.0,  // double s.a1
				    0.0, 0.0, 0.0, 0.0,  // dvec3 s.a2
				    0.0, 0.0, 0.0, 0.0,  // dmat2 s.a4[0]
				    0.0, 0.0, 0.0, 0.0,  // dmat2 s.a4[1]
				    2.0, 0.0, 0.0, 0.0,  // double s.a5, s.sb[0].b1
				    0.0, 0.0, 0.0, 0.0,  // double s.sb[1].b1
				    0.0, 0.0, 0.0, 0.0,  // dmat3x4 m[0]
				    0.0, 0.0, 0.0, 0.0,  // dmat3x4 m[1]
				    0.0, 0.0, 0.0, 0.0,  // dmat3x4 m[2]
				    0.0, 0.0, 0.0, 0.0,  // dvec2 v2a[3]
				    0.0, 0.0, 0.0, 0.0,  //
				    0.0, 0.0, 0.0, 0.0,  // dvec3 v3a[2]
				    0.0, 0.0, 0.0, 0.0,  //
				    0.0, 0.0, 0.0, 0.0,  // double unsized_array[0-3]
};

float expected1[SSBO_SIZE1]  = { 6.0, 7.0, 0.0,  0.0,	 // vec2 u
};

double expected2[SSBO_SIZE2] = { 0.0, 0.0,		 // padding
				 6.0, 1.0, 2.0, 10.0,	 // dvec4 v
};

float expected3[SSBO_SIZE3]  = { 4.0, 0.0, 0.0,  0.0,	 // float f
};

double expected4[SSBO_SIZE4] = {  0.0,  0.0,				  // padding
				  1.0,  0.0,  0.0,  0.0,		  // double s.a1
				  6.0,  7.0,  8.0,  0.0,		  // dvec3 s.a2
				 -1.333333333333333259, 11.0, 12.0, 13.0, // dmat2 s.a4[0]
				 14.0, 15.0, 16.0, 17.0,		  // dmat2 s.a4[1]
				  2.0, 18.333333333333333259, 19.0,  0.0, // double s.a5, s.sb[0].b1
				  0.0,  0.0, 20.0,  0.0,		  // double s.sb[1].b1
				 21.333333333333333259, 22.0, 23.0, 24.0, // dmat3x4 m[0]
				 25.0, 26.0, 27.0, 28.0,		  // dmat3x4 m[1]
				 29.0, 30.0, 31.0, 32.0,		  // dmat3x4 m[2]
				 66.0, 68.0, 35.0, 36.0,		  // dvec2 v2a[3]
				 37.0, 38.0,  0.0,  0.0,		  //
				 39.0, 40.0, 41.0,  0.0,		  // dvec3 v3a[2]
				 42.0, 43.0, 44.0,  0.0,		  //
				  4.0,  4.0,  4.0,  4.0,		  // double unsized_array[0-3]
};

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint buffer;
	unsigned int i;
	float *map_f;
	double *map_d;
	void *data;
	void *data_base;
	GLint num_vertex_ssbo;

	piglit_require_extension("GL_ARB_shader_storage_buffer_object");
	piglit_require_extension("GL_ARB_gpu_shader_fp64");

	glGetIntegerv(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, &num_vertex_ssbo);
	if (num_vertex_ssbo < 1)
		piglit_report_result(PIGLIT_SKIP);

	prog = piglit_build_simple_program_multiple_shaders(
		GL_VERTEX_SHADER, vs_code,
		GL_GEOMETRY_SHADER, gs_source,
		GL_FRAGMENT_SHADER, fs_source,
		NULL);

	glUseProgram(prog);

	glClearColor(0, 0, 0, 0);

	data = malloc((SSBO_SIZE1 + SSBO_SIZE3) * sizeof(float) +
		      (SSBO_SIZE2 + SSBO_SIZE4) * sizeof(double));

	data_base = data;
	memcpy(data_base, ssbo_values1, SSBO_SIZE1 * sizeof(float));

	data_base = ((float *) data_base) + SSBO_SIZE1;
	memcpy(data_base, ssbo_values2, SSBO_SIZE2 * sizeof(double));

	data_base = ((double *) data_base) + SSBO_SIZE2;
	memcpy(data_base, ssbo_values3, SSBO_SIZE3 * sizeof(float));

	data_base = ((float *) data_base) + SSBO_SIZE3;
	memcpy(data_base, ssbo_values4, SSBO_SIZE4 * sizeof(double));

	glGenBuffers(1, &buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, buffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
		     (SSBO_SIZE1 + SSBO_SIZE3) * sizeof(GLfloat) +
		     (SSBO_SIZE2 + SSBO_SIZE4) * sizeof(GLdouble),
		     data,
		     GL_DYNAMIC_DRAW);

	glViewport(0, 0, piglit_width, piglit_height);

	piglit_draw_rect(-1, -1, 2, 2);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
	map_f = glMapBuffer(GL_SHADER_STORAGE_BUFFER,  GL_READ_ONLY);

#define CHECK_RESULTS(map, expected, size)				    \
	for (i = 0; i < size; i++) {					    \
		if (DIFFER(*map, expected[i])) {			    \
			printf("#expected[%d] = %.14g. Read value %.14g\n", \
			       i, expected[i], *map);			    \
			pass = false;					    \
		}							    \
		map++;							    \
	}								    \

	CHECK_RESULTS(map_f, expected1, SSBO_SIZE1);

	map_d = (double *) map_f;
	CHECK_RESULTS(map_d, expected2, SSBO_SIZE2);

	map_f = (float *) map_d;
	CHECK_RESULTS(map_f, expected3, SSBO_SIZE3);

	map_d = (double *) map_f;
	CHECK_RESULTS(map_d, expected4, SSBO_SIZE4);

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	free(data);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	glDeleteProgram(prog);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
