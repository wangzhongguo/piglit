/*  BEGIN_COPYRIGHT -*- glean -*-
 *
 *  Copyright (C) 1999  Allen Akin   All Rights Reserved.
 *  Copyright (C) 2015  Intel Corporation.
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
 * END_COPYRIGHT
 */

/** @file occlusion_query_conform.c
 *
 *	Conformance test on ARB_occlusion_query extension.
 *
 *	Authors:
 *	Wei Wang <wei.z.wang@intel.com>
 *	Adapted to Piglit by Juliet Fru <julietfru@gmail.com>, September 2015
 */

#include "piglit-util-gl.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

static const struct piglit_gl_test_config * piglit_config;

PIGLIT_GL_TEST_CONFIG_BEGIN config.supports_gl_compat_version = 10;

config.window_visual =
	PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE |
	PIGLIT_GL_VISUAL_DEPTH;

	piglit_config = &config;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END static GLuint
find_unused_id(void)
{
	GLuint id;
	glGenQueries(1, &id);
	return id;

}

/* If multiple queries are issued on the same target and id prior to calling
 * GetQueryObject[u]iVARB, the result returned will always be from the last
 * query issued.  The results from any queries before the last one will be lost
 * if the results are not retrieved before starting a new query on the same
 * target and id.
 */
static enum piglit_result
conformOQ_GetObjivAval_multi1(void *data)
{
	GLuint id = *(GLuint *)data;
	GLint ready;
	GLuint passed = 0;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, 0.0, 25.0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -10.0);

	/* draw the occluder (red) */
	glColorMask(1, 1, 1, 1);
	glDepthMask(GL_TRUE);
	glColor3f(1, 0, 0);
	piglit_draw_rect(-0.5, 0.5, 0.5, -0.5);

	glPushMatrix();
	glTranslatef(0.0, 0.0, -5.0);
	glColorMask(0, 0, 0, 0);
	glDepthMask(GL_FALSE);

	/* draw the 1st box (green) which is occluded by the occluder partly */
	glBeginQueryARB(GL_SAMPLES_PASSED_ARB, id);
	glColor3f(0, 1, 0);
	piglit_draw_rect(-0.51, 0.51, 0.51, -0.51);
	glEndQueryARB(GL_SAMPLES_PASSED_ARB);

	/* draw the 2nd box (blue) which is occluded by the occluder throughly */
	glBeginQueryARB(GL_SAMPLES_PASSED_ARB, id);
	glColor3f(0, 0, 1);
	piglit_draw_rect(-0.4, 0.4, 0.4, -0.4);
	glEndQueryARB(GL_SAMPLES_PASSED_ARB);

	glPopMatrix();

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	do {
		glGetQueryObjectivARB(id, GL_QUERY_RESULT_AVAILABLE_ARB,
				      &ready);
	} while (!ready);
	glGetQueryObjectuivARB(id, GL_QUERY_RESULT_ARB, &passed);

	/* 'passed' should be zero */
	return passed > 0 ? PIGLIT_FAIL : PIGLIT_PASS;
}

/* If mutiple queries are issued on the same target and diff ids prior
 * to calling GetQueryObject[u]iVARB, the results should be
 * corresponding to those queries (ids) respectively.
 */
static enum piglit_result
conformOQ_GetObjivAval_multi2(void *data)
{
	GLuint passed1 = 0, passed2 = 0, passed3 = 0;
	GLuint id1, id2, id3;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, 0.0, 25.0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -10.0);


	/* draw the occluder (red) */
	glColorMask(1, 1, 1, 1);
	glDepthMask(GL_TRUE);
	glColor3f(1, 0, 0);
	piglit_draw_rect(-0.5, 0.5, 0.5, -0.5);

	glPushMatrix();
	glTranslatef(0.0, 0.0, -5.0);
	glColorMask(0, 0, 0, 0);
	glDepthMask(GL_FALSE);

	id1 = find_unused_id();
	glBeginQueryARB(GL_SAMPLES_PASSED_ARB, id1);
	/* draw green quad, much larger than occluder */
	glColor3f(0, 1, 0);
	piglit_draw_rect(-0.7, 0.7, 0.7, -0.7);
	glEndQueryARB(GL_SAMPLES_PASSED_ARB);

	id2 = find_unused_id();
	glBeginQueryARB(GL_SAMPLES_PASSED_ARB, id2);
	/* draw blue quad, slightly larger than occluder */
	glColor3f(0, 0, 1);
	piglit_draw_rect(-0.53, 0.53, 0.53, -0.53);
	glEndQueryARB(GL_SAMPLES_PASSED_ARB);


	id3 = find_unused_id();
	glBeginQueryARB(GL_SAMPLES_PASSED_ARB, id3);
	/* draw white quad, smaller than occluder (should not be visible) */
	glColor3f(1, 1, 1);
	piglit_draw_rect(-0.4, 0.4, 0.4, -0.4);
	glEndQueryARB(GL_SAMPLES_PASSED_ARB);


	glPopMatrix();

	glGetQueryObjectuivARB(id1, GL_QUERY_RESULT_ARB, &passed1);
	glGetQueryObjectuivARB(id2, GL_QUERY_RESULT_ARB, &passed2);
	glGetQueryObjectuivARB(id3, GL_QUERY_RESULT_ARB, &passed3);

	glDepthMask(GL_TRUE);


	glDeleteQueriesARB(1, &id1);
	glDeleteQueriesARB(1, &id2);
	glDeleteQueriesARB(1, &id3);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	if (passed1 > passed2 && passed2 > passed3 && passed3 == 0)
		return PIGLIT_PASS;
	else
		return PIGLIT_FAIL;
}

/*
 * void GetQueryivARB(enum target, enum pname, int *params);
 *
 * If <pname> is QUERY_COUNTER_BITS_ARB, the number of bits in the counter
 * for <target> will be placed in <params>.  The minimum number of query
 * counter bits allowed is a function of the implementation's maximum
 * viewport dimensions (MAX_VIEWPORT_DIMS).  If the counter is non-zero,
 * then the counter must be able to represent at least two overdraws for
 * every pixel in the viewport using only one sample buffer.  The formula to
 * compute the allowable minimum value is below (where n is the minimum
 * number of bits):
 *   n = (min (32, ceil (log2 (maxViewportWidth x maxViewportHeight x 2) ) ) ) or 0
 */
static enum piglit_result
conformOQ_GetQueryCounterBits(void *data)
{
	int bit_num, dims[2];
	float min_impl, min_bit_num;

	/* get the minimum bit number supported by the implementation, 
	 * and check the legality of result of GL_QUERY_COUNTER_BITS_ARB 
	 */
	glGetQueryivARB(GL_SAMPLES_PASSED_ARB, GL_QUERY_COUNTER_BITS_ARB,
			&bit_num);
	glGetIntegerv(GL_MAX_VIEWPORT_DIMS, dims);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	min_impl =
		ceil(logf((float) dims[0] * (float) dims[1] * 1.0 * 2.0) /
		     logf(2.0));
	min_bit_num = 32.0 > min_impl ? min_impl : 32.0;

	if ((float) bit_num < min_bit_num)
		return PIGLIT_FAIL;

	return PIGLIT_PASS;
}

/* If BeginQueryARB is called with an unused <id>, that name is marked as used
 * and associated with a new query object.
 */
static enum piglit_result
conformOQ_Begin_unused_id(void *data)
{
	unsigned int id;
	enum piglit_result pass = PIGLIT_PASS;

	id = find_unused_id();

	if (id == 0)
		return PIGLIT_FAIL;

	glBeginQuery(GL_SAMPLES_PASSED_ARB, id);

	if (glIsQueryARB(id) == GL_FALSE) {
		printf("Error : Begin with a unused id failed.");
		pass = PIGLIT_FAIL;
	}

	glEndQuery(GL_SAMPLES_PASSED_ARB);

	return pass;
}

/* if EndQueryARB is called while no query with the same target is in progress,
 * an INVALID_OPERATION error is generated.
 */
static enum piglit_result
conformOQ_EndAfter(void *data)
{
	GLuint id = *(GLuint *)data;

	glBeginQueryARB(GL_SAMPLES_PASSED_ARB, id);
	glEndQueryARB(GL_SAMPLES_PASSED_ARB);

	glEndQueryARB(GL_SAMPLES_PASSED_ARB);

	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		return PIGLIT_FAIL;

	return PIGLIT_PASS;
}

/* If BeginQueryARB is called while another query is already in progress with
 * the same target, an INVALID_OPERATION error should be generated.
 */
static enum piglit_result
conformOQ_BeginIn(void *data)
{
	GLuint id = *(GLuint *)data;
	enum piglit_result pass = PIGLIT_PASS;

	glBeginQueryARB(GL_SAMPLES_PASSED_ARB, id);

	/* Issue another BeginQueryARB while another query is already in progress */
	glBeginQueryARB(GL_SAMPLES_PASSED_ARB, id);

	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		pass = PIGLIT_FAIL;

	glEndQueryARB(GL_SAMPLES_PASSED_ARB);
	return pass;
}

/* if the query object named by <id> is currently active, then an
 * INVALID_OPERATION error is generated.
 */
static enum piglit_result
GetObjectAvailableIn(void *data)
{
	GLuint id = *(GLuint *)data;
	enum piglit_result pass = PIGLIT_PASS;
	GLint param;

	glBeginQueryARB(GL_SAMPLES_PASSED_ARB, id);

	glGetQueryObjectivARB(id, GL_QUERY_RESULT_AVAILABLE_ARB, &param);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		pass = PIGLIT_FAIL;

	glGetQueryObjectuivARB(id, GL_QUERY_RESULT_AVAILABLE_ARB,
			       (GLuint *) & param);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		pass = PIGLIT_FAIL;

	if (pass == PIGLIT_FAIL) {
		printf(" Error: No GL_INVALID_OPERATION generated if "
		       "GetQueryObjectuiv with GL_QUERY_RESULT_AVAILABLE_ARB "
		       "in the active progress.\n");
	}
	glEndQueryARB(GL_SAMPLES_PASSED_ARB);

	return pass;
}

/* if the query object named by <id> is currently active, then an
 * INVALID_OPERATION error is generated. */
static enum piglit_result
conformOQ_GetObjResultIn(void *data)
{
	GLuint id = *(GLuint *)data;
	enum piglit_result pass = PIGLIT_PASS;
	GLint param;

	glBeginQueryARB(GL_SAMPLES_PASSED_ARB, id);

	glGetQueryObjectivARB(id, GL_QUERY_RESULT_ARB, &param);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		pass = PIGLIT_FAIL;

	glGetQueryObjectuivARB(id, GL_QUERY_RESULT_ARB, (GLuint *) & param);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		pass = PIGLIT_FAIL;

	if (pass == PIGLIT_FAIL) {
		printf(" Error: No GL_INVALID_OPERATION generated if "
		       "GetQueryObject[u]iv with GL_QUERY_RESULT_ARB "
		       "in the active progress.\n");
	}

	glEndQueryARB(GL_SAMPLES_PASSED_ARB);
	return pass;
}

/* If <id> is not the name of a query object, then an INVALID_OPERATION error
 * is generated.
 */
static enum piglit_result
conformOQ_GetObjivAval(void *data)
{
	GLuint id = *(GLuint *)data;
	GLuint id_tmp;
	GLint param;

	glBeginQueryARB(GL_SAMPLES_PASSED_ARB, id);
	glEndQueryARB(GL_SAMPLES_PASSED_ARB);

	id_tmp = find_unused_id();

	if (id_tmp == 0)
		return PIGLIT_FAIL;

	glGetQueryObjectivARB(id_tmp, GL_QUERY_RESULT_AVAILABLE_ARB, &param);

	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		return PIGLIT_FAIL;

	return PIGLIT_PASS;
}

/* Basic tests on query id generation and deletion */
static enum piglit_result
conformOQ_Gen_Delete(void *data)
{
	const unsigned int id_n = 64;
	GLuint *ids1 = NULL, *ids2 = NULL;
	unsigned int i, j;
	enum piglit_result pass = PIGLIT_PASS;

	ids1 = (GLuint *) malloc(id_n * sizeof(GLuint));
	ids2 = (GLuint *) malloc(id_n * sizeof(GLuint));

	if (!ids1 || !ids2) {
		printf(" Error: Cannot alloc memory to pointer ids[12].\n");
		if (ids1)
			free(ids1);
		if (ids2)
			free(ids2);
		return PIGLIT_FAIL;
	}

	glGenQueriesARB(id_n, ids1);
	glGenQueriesARB(id_n, ids2);

	/* compare whether <id> generated during the previous 2 rounds are
	 * duplicated */
	for (i = 0; i < id_n; i++) {
		for (j = 0; j < id_n; j++) {
			if (ids1[i] == ids2[j]) {
				char str[1000];
				sprintf(str, "ids1[%d] == ids2[%d] == %u.", i,
					j, ids1[i]);
				printf(" Error:  %s\n", str);
				pass = PIGLIT_FAIL;
			}
		}
	}

	/* Note: the spec seems to indicate that glGenQueries reserves query
	 * IDs but doesn't create query objects for those IDs.  A query object
	 * isn't created until they are used by glBeginQuery.  So this part
	 * of the test is invalid.
	 */
#if 0
	/* Checkout whether the Query ID just generated is valid */
	for (i = 0; i < id_n; i++) {
		if (glIsQueryARB(ids1[i]) == GL_FALSE) {
			char str[1000];
			sprintf(str, "id [%d] just generated is not valid.",
				ids1[i]);
			printf(" Error: %s\n", str);
			pass = PIGLIT_FAIL;
		}
	}
#endif

	/* if <id> is a non-zero value that is not the name of a query object,
	 * IsQueryARB returns FALSE.
	 */
	glDeleteQueriesARB(id_n, ids1);
	for (i = 0; i < id_n; i++) {
		if (glIsQueryARB(ids1[i]) == GL_TRUE) {
			char str[1000];
			sprintf(str, "id [%d] just deleted is still valid.",
				ids1[i]);
			printf(" Error: %s\n", str);
			pass = PIGLIT_FAIL;
		}
	}

	/* Delete only for sanity purpose */
	glDeleteQueriesARB(id_n, ids2);

	if (ids1)
		free(ids1);
	if (ids2)
		free(ids2);


	ids1 = (GLuint *) malloc(id_n * sizeof(GLuint));
	if (ids1 == NULL)
		return PIGLIT_FAIL;

	for (i = 0; i < id_n; i++) {
		glGenQueriesARB(1, ids1 + i);
		for (j = 0; j < i; j++) {
			if (ids1[i] == ids1[j]) {
				char str[1000];
				sprintf(str, "duplicated id generated [%u]",
					ids1[i]);
				printf(" Error: %s\n", str);
				pass = PIGLIT_FAIL;
			}
		}
	}

	glDeleteQueriesARB(id_n, ids1);
	if (ids1)
		free(ids1);

	return pass;
}

/* If <id> is zero, IsQueryARB should return FALSE.*/
static enum piglit_result
conformOQ_IsIdZero(void *data)
{
	if (glIsQueryARB(0) == GL_TRUE) {
		printf(" Error: zero is treated as a valid id by glIsQueryARB().\n");
		return PIGLIT_FAIL;
	}

	return PIGLIT_PASS;
}

/* If BeginQueryARB is called with an <id> of zero, an INVALID_OPERATION error
 * should be generated.
 */
static enum piglit_result
conformOQ_BeginIdZero(void *data)
{
	glBeginQueryARB(GL_SAMPLES_PASSED_ARB, 0);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		return PIGLIT_FAIL;

	return PIGLIT_PASS;
}

GLuint queryId;

static const struct piglit_subtest tests[] = {
	{
		"GetQueryCounterBits",
		"get_query_counter_bits",
		conformOQ_GetQueryCounterBits,
		NULL
	},
	{
		"GetObjivAval_multi1",
		"get_objiv_aval_multi1",
		conformOQ_GetObjivAval_multi1,
		&queryId,
	},
	{
		"GetObjivAval_multi2",
		"get_objiv_aval_multi2",
		conformOQ_GetObjivAval_multi2,
		NULL
	},
	{
		"Begin_unused_id",
		"begin_unused_id",
		conformOQ_Begin_unused_id,
		NULL
	},
	{
		"BeginAfter",
		"begin_after",
		conformOQ_EndAfter,
		&queryId,
	},
	{
		"BeginIn",
		"begin_in",
		conformOQ_BeginIn,
		&queryId,
	},
	{
		"GetObjectAvailableIn",
		"get_object_available_in",
		GetObjectAvailableIn,
		&queryId,
	},
	{
		"GetObjResultIn",
		"get_obj_result_in",
		conformOQ_GetObjResultIn,
		&queryId,
	},
	{
		"GetObjivAval",
		"get_objiv_aval",
		conformOQ_GetObjivAval,
		&queryId,
	},
	{
		"Gen_Delete",
		"gen_delete",
		conformOQ_Gen_Delete,
		NULL,
	},
	{
		"IsIdZero",
		"is_id_zero",
		conformOQ_IsIdZero,
		NULL
	},
	{
		"BeginIdZero",
		"begin_id_zero",
		conformOQ_BeginIdZero,
		NULL
	},
	{ NULL }
};

void
piglit_init(int argc, char **argv)
{
	glClearColor(0.0, 0.2, 0.3, 0.0);
	glClearDepth(1.0);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	piglit_require_extension("GL_ARB_occlusion_query");
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

enum piglit_result
piglit_display(void)
{
	glEnable(GL_DEPTH_TEST);
	glGenQueriesARB(1, &queryId);

	if (queryId == 0)
		return PIGLIT_FAIL;

	enum piglit_result result = PIGLIT_PASS;
	result = piglit_run_selected_subtests(
		tests,
		piglit_config->selected_subtests,
		piglit_config->num_selected_subtests,
		result);
	piglit_present_results();

	glDeleteQueriesARB(1, &queryId);

	return result;
}
