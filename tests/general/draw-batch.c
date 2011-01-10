/*
 * Copyright © 2011 Marek Olšák <maraeo@gmail.com>
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
 *
 */

#include "piglit-util.h"

int piglit_width = 100, piglit_height = 70;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;

void
piglit_init(int argc, char **argv)
{
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
	if (!GLEW_VERSION_1_4) {
		printf("Requires OpenGL 1.4.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	float array[] = {
		10, 10,	   1, 0, 0,
		27, 10,	   1, 0, 0,
		10, 30,	   1, 0, 0,

		30, 10,	   0, 1, 0,
		47, 10,	   0, 1, 0,
		30, 30,	   0, 1, 0,

		50, 10,	   0, 0, 1,
		67, 10,	   0, 0, 1,
		50, 30,	   0, 0, 1,

		70, 10,	   1, 0, 1,
		87, 10,	   1, 0, 1,
		70, 30,	   1, 0, 1
	};
	float seccol[] = {
		0.2, 0.2, 0.2,
		0.2, 0.2, 0.2,
		0.2, 0.2, 0.2,

		0.2, 0.2, 0.2,
		0.2, 0.2, 0.2,
		0.2, 0.2, 0.2,

		0.2, 0.2, 0.2,
		0.2, 0.2, 0.2,
		0.2, 0.2, 0.2,

		0.2, 0.2, 0.2,
		0.2, 0.2, 0.2,
		0.2, 0.2, 0.2,
	     };
	float c0[] = {1, 0.2, 0.2};
	float c1[] = {0.2, 1, 0.2};
	float c2[] = {0.2, 0.2, 1};
	float c3[] = {1, 0.2, 1};
	short indices[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
	};
	int i;

	printf("DrawElements: Bottom, DrawArrays: Top\n");

	glEnable(GL_COLOR_SUM);
	glLoadIdentity();

	/* State change: Vertex arrays. */
	glVertexPointer(2, GL_FLOAT, 20, array);
	glColorPointer(3, GL_FLOAT, 20, array + 2);
	glSecondaryColorPointer(3, GL_FLOAT, 0, seccol);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_SECONDARY_COLOR_ARRAY);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Draw. */
	for (i = 0; i < 4; i++)
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, indices + i*3);

	/* State change: Constant buffer. */
	glTranslatef(0, 30, 0);

	/* Draw. */
	for (i = 0; i < 4; i++)
		glDrawArrays(GL_TRIANGLES, i*3, 3);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_SECONDARY_COLOR_ARRAY);

	pass = pass && piglit_probe_pixel_rgb(15, 15, c0);
	pass = pass && piglit_probe_pixel_rgb(35, 15, c1);
	pass = pass && piglit_probe_pixel_rgb(55, 15, c2);
	pass = pass && piglit_probe_pixel_rgb(75, 15, c3);

	pass = pass && piglit_probe_pixel_rgb(15, 45, c0);
	pass = pass && piglit_probe_pixel_rgb(35, 45, c1);
	pass = pass && piglit_probe_pixel_rgb(55, 45, c2);
	pass = pass && piglit_probe_pixel_rgb(75, 45, c3);

	glutSwapBuffers();
	return pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE;
}
