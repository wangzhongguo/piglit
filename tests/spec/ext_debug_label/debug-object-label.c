/*
 * Copyright (c) 2013 Timothy Arceri <t_arceri@yahoo.com.au>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL AUTHORS AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "piglit-util-gl.h"

static const char *TestLabel = "Test Label";
#define TestLabelLen 10

PIGLIT_GL_TEST_CONFIG_BEGIN

#ifdef PIGLIT_USE_OPENGL
	config.supports_gl_compat_version = 11;
#else /* using GLES */
	config.supports_gl_es_version = 11;
#endif

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}

/* <label> will be null-terminated. The actual number of
 * characters written into <label>,
 * excluding the null terminator, is returned in <length>.
 */
static bool
check_label_and_length(char *label, int length, GLenum object)
{
	bool pass = true;
	const char *name = piglit_get_gl_enum_name(object);

	printf("Checking label and length of %s object\n", name);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	if (length != TestLabelLen || (strcmp(TestLabel, label) != 0)) {
		fprintf(stderr, "Label or length does not match in %s object\n", name);
		printf("  actual label: %s actual length: %i\n", label, length);
		printf("  expected label: %s expected length: %i\n", TestLabel, TestLabelLen);
		pass = false;
	}

	return pass;
}

/* trivial tests to get/set labels on all the different types of objects */
static bool
test_object_label_types()
{
	#define numObjects 12
	GLsizei length[numObjects]; /* create a fresh variable for each object to test */
	GLchar label[numObjects][TestLabelLen + 1];
	bool pass = true;

	enum test_object_indices {
		BUFFER_IDX, SHADER_IDX, PROGRAM_IDX, VERTEX_ARRAY_IDX, RENDERBUFFER_IDX,
		FRAMEBUFFER_IDX, QUERY_IDX, PROGRAM_PIPELINE_IDX, TRANSFORM_FEEDBACK_IDX,
		SAMPLER_IDX, TEXTURE_IDX, DISPLAY_LIST_IDX
	};

	GLuint buffer;
	GLuint shader;
	GLuint program;
	GLuint vertexArray;
	GLuint query;
#ifdef PIGLIT_USE_OPENGL
	GLuint programPipeline;
#endif
	GLuint transformFeedback;
	GLuint sampler;
	GLuint texture;
	GLuint renderbuffer;
	GLuint framebuffer;
#ifdef PIGLIT_USE_OPENGL
	GLuint displayList;
#endif

	/* Test BUFFER */
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glLabelObjectEXT(GL_BUFFER_OBJECT_EXT, buffer, 0, TestLabel);
	glGetObjectLabelEXT(GL_BUFFER_OBJECT_EXT, buffer, TestLabelLen + 1, &length[BUFFER_IDX], label[BUFFER_IDX]);

	pass = check_label_and_length(label[BUFFER_IDX], length[BUFFER_IDX],
				      GL_BUFFER) && pass;

	glDeleteBuffers(1, &buffer);

	if (piglit_get_gl_version() >= 20) {
		/* Test SHADER */
		shader = glCreateShader(GL_FRAGMENT_SHADER);
		glLabelObjectEXT(GL_SHADER_OBJECT_EXT, shader, 0, TestLabel);
		glGetObjectLabelEXT(GL_SHADER_OBJECT_EXT, shader, TestLabelLen + 1,
				 &length[SHADER_IDX], label[SHADER_IDX]);

		pass = check_label_and_length(label[SHADER_IDX], length[SHADER_IDX],
					      GL_SHADER) && pass;

		glDeleteShader(shader);

		/* Test PROGRAM */
		program = glCreateProgram();
		glLabelObjectEXT(GL_PROGRAM_OBJECT_EXT, program, 0, TestLabel);
		glGetObjectLabelEXT(GL_PROGRAM_OBJECT_EXT, program, TestLabelLen + 1,
				 &length[PROGRAM_IDX], label[PROGRAM_IDX]);

		pass = check_label_and_length(label[PROGRAM_IDX], length[PROGRAM_IDX],
					      GL_PROGRAM) && pass;

		glDeleteProgram(program);
	}

	/* GL or GLES >= 3.0 supports vertex arrays*/
	if (piglit_get_gl_version() >= 30) {
		/* Test VERTEX_ARRAY */
		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		glLabelObjectEXT(GL_VERTEX_ARRAY_OBJECT_EXT, vertexArray, 0, TestLabel);
		glGetObjectLabelEXT(GL_VERTEX_ARRAY_OBJECT_EXT, vertexArray, TestLabelLen + 1,
				 &length[VERTEX_ARRAY_IDX], label[VERTEX_ARRAY_IDX]);

		pass = check_label_and_length(label[VERTEX_ARRAY_IDX], length[VERTEX_ARRAY_IDX],
					      GL_VERTEX_ARRAY) && pass;

		glDeleteVertexArrays(1, &vertexArray);
	}

	/* GLES supports render buffer and frame buffer since 2.0 */
	if (piglit_is_gles() || piglit_get_gl_version() >= 30) {
		/* Test RENDERBUFFER */
		glGenRenderbuffers(1, &renderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
		glLabelObjectEXT(GL_RENDERBUFFER, renderbuffer, 0, TestLabel);
		glGetObjectLabelEXT(GL_RENDERBUFFER, renderbuffer, TestLabelLen + 1,
				 &length[RENDERBUFFER_IDX], label[RENDERBUFFER_IDX]);

		pass = check_label_and_length(label[RENDERBUFFER_IDX], length[RENDERBUFFER_IDX],
					      GL_RENDERBUFFER) && pass;

		glDeleteRenderbuffers(1, &renderbuffer);

		/* Test FRAMEBUFFER */
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glLabelObjectEXT(GL_FRAMEBUFFER, framebuffer, 0, TestLabel);
		glGetObjectLabelEXT(GL_FRAMEBUFFER, framebuffer, TestLabelLen + 1,
				 &length[FRAMEBUFFER_IDX], label[FRAMEBUFFER_IDX]);

		pass = check_label_and_length(label[FRAMEBUFFER_IDX], length[FRAMEBUFFER_IDX],
					      GL_FRAMEBUFFER) && pass;

		glDeleteFramebuffers(1, &framebuffer);
	}

	/* GLES >= 3.0 or GL compat */
	if (piglit_is_extension_supported("GL_ARB_occlusion_query") ||
	    piglit_get_gl_version() >= 30) {
		/* Test QUERY */
		glGenQueries(1, &query);
		if (piglit_is_gles()) {
			glBeginQuery(GL_ANY_SAMPLES_PASSED, query);
			glEndQuery(GL_ANY_SAMPLES_PASSED);
		} else {
			glBeginQuery(GL_SAMPLES_PASSED, query);
			glEndQuery(GL_SAMPLES_PASSED);
		}
		glLabelObjectEXT(GL_QUERY_OBJECT_EXT, query, 0, TestLabel);
		glGetObjectLabelEXT(GL_QUERY_OBJECT_EXT, query, TestLabelLen + 1, &length[QUERY_IDX], label[QUERY_IDX]);

		pass = check_label_and_length(label[QUERY_IDX], length[QUERY_IDX],
					      GL_QUERY) && pass;

		glDeleteQueries(1, &query);
	}

#ifdef PIGLIT_USE_OPENGL
	/* Test PROGRAM_PIPELINE */
	if (piglit_is_extension_supported("GL_ARB_separate_shader_objects")) {
		glGenProgramPipelines(1, &programPipeline);
		glBindProgramPipeline(programPipeline);
		glLabelObjectEXT(GL_PROGRAM_PIPELINE_OBJECT_EXT, programPipeline, 0, TestLabel);
		glGetObjectLabelEXT(GL_PROGRAM_PIPELINE_OBJECT_EXT, programPipeline, TestLabelLen + 1,
				 &length[PROGRAM_PIPELINE_IDX], label[PROGRAM_PIPELINE_IDX]);

		pass = check_label_and_length(label[PROGRAM_PIPELINE_IDX], length[PROGRAM_PIPELINE_IDX],
					      GL_PROGRAM_PIPELINE)  && pass;

		glDeleteProgramPipelines(1, &programPipeline);
	}
#endif /* PIGLIT_USE_OPENGL */

	/* Test TRANSFORM_FEEDBACK */
	if ((piglit_is_gles() && piglit_get_gl_version() >= 30) ||
		piglit_is_extension_supported("GL_ARB_transform_feedback2")) {
		glGenTransformFeedbacks(1, &transformFeedback);
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedback);
		glLabelObjectEXT(GL_TRANSFORM_FEEDBACK, transformFeedback, 0, TestLabel);
		glGetObjectLabelEXT(GL_TRANSFORM_FEEDBACK, transformFeedback, TestLabelLen + 1,
				 &length[TRANSFORM_FEEDBACK_IDX], label[TRANSFORM_FEEDBACK_IDX]);

		pass = check_label_and_length(label[TRANSFORM_FEEDBACK_IDX], length[TRANSFORM_FEEDBACK_IDX],
					      GL_TRANSFORM_FEEDBACK) && pass;

		glDeleteTransformFeedbacks(1, &transformFeedback);
	}

	/* Test SAMPLER */
	if ((piglit_is_gles() && piglit_get_gl_version() >= 30) ||
		piglit_is_extension_supported("GL_ARB_sampler_objects")) {
		glGenSamplers(1, &sampler);
		glBindSampler(0, sampler);
		glLabelObjectEXT(GL_SAMPLER, sampler, 0, TestLabel);
		glGetObjectLabelEXT(GL_SAMPLER, sampler, TestLabelLen + 1, &length[SAMPLER_IDX], label[SAMPLER_IDX]);

		pass = check_label_and_length(label[SAMPLER_IDX], length[SAMPLER_IDX],
					      GL_SAMPLER) && pass;

		glDeleteSamplers(1, &sampler);
	}

	/* Test TEXTURE */
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glLabelObjectEXT(GL_TEXTURE, texture, 0, TestLabel);
	glGetObjectLabelEXT(GL_TEXTURE, texture, TestLabelLen + 1, &length[TEXTURE_IDX], label[TEXTURE_IDX]);

	pass = check_label_and_length(label[TEXTURE_IDX], length[TEXTURE_IDX],
				      GL_TEXTURE) && pass;

	glDeleteTextures(1, &texture);

	return pass;
}

static bool
test_object_label()
{
	GLsizei length;
	GLuint buffer;
	GLuint invalidBufferName;
	GLchar label[TestLabelLen + 1];
	bool pass = true;

	puts("Test LabelObject");

	glGenBuffers(1, &buffer);

	/* If <label> is NULL, any debug label is effectively removed from the object.
	 */
	glLabelObjectEXT(GL_BUFFER, buffer, 0, TestLabel);
	glLabelObjectEXT(GL_BUFFER, buffer, 0, NULL);
	glGetObjectLabelEXT(GL_BUFFER, buffer, TestLabelLen + 1, &length, label);

	if (length != 0 || (strcmp("", label) != 0)) {
		fprintf(stderr, "Setting label to NULL should remove the label\n");
		printf("  actual label: %s actual length: %i\n", label, length);
		pass = false;
	}

	/* An INVALID_ENUM error is generated by ObjectLabel if <identifier> is not
	 * one of the object types.
	 */
	glLabelObjectEXT(GL_ARRAY_BUFFER, buffer, 0, TestLabel);

	if (!piglit_check_gl_error(GL_INVALID_ENUM)) {
		fprintf(stderr, "GL_INVALID_ENUM should be generated when the LabelObject identifier is invalid\n");
		pass = false;
	}

	/* INVALID_VALUE is generated by LabelObjectEXT if <length> is less than zero. */
	glLabelObjectEXT(GL_BUFFER, buffer, -1, TestLabel);

	if (!piglit_check_gl_error(GL_INVALID_VALUE)) {
		fprintf(stderr, "GL_INVALID_VALUE should be generated when the LabelObject length is less than 0\n");
		pass = false;
	}

	/* INVALID_OPERATION is generated by LabelObjectEXT
	 * if the type of <object> does not match <type>.
	 */
	invalidBufferName = buffer;
	glDeleteBuffers(1, &buffer);
	glLabelObjectEXT(GL_BUFFER, invalidBufferName, 0, TestLabel);

	if (!piglit_check_gl_error(GL_INVALID_OPERATION)) {
		fprintf(stderr, "GL_INVALID_OPERATION should be generated when the LabelObject name is invalid\n");
		pass = false;
	}

	return pass;
}

static bool
test_get_object_label()
{
	#define numBuffers 4
	GLsizei length;
	GLuint buffers[numBuffers];
	GLuint invalidBufferName;
	GLchar label[TestLabelLen + 1];
	bool pass = true;

	enum test_object_indices {
		TEST_BUFSIZE_IDX, TEST_NO_LABEL_IDX, TEST_NULL_LABEL_IDX, TEST_NULL_LENGTH_IDX
	};

	puts("Test GetObjectLabel");

	glGenBuffers(numBuffers, buffers);

	/* The maximum number of characters that may
	 * be written into <label>, including the null terminator, is specified by
	 * <bufSize>.
	 */
	glBindBuffer(GL_ARRAY_BUFFER, buffers[TEST_BUFSIZE_IDX]);
	glLabelObjectEXT(GL_BUFFER, buffers[TEST_BUFSIZE_IDX], 0, TestLabel);
	glGetObjectLabelEXT(GL_BUFFER, buffers[TEST_BUFSIZE_IDX], TestLabelLen, &length, label);

	if (length != 9 || (strcmp("Test Labe", label) != 0)) {
		fprintf(stderr, "BufSize should limit the maximum label length to 9\n");
		printf("  actual label: %s actual length: %i\n", label, length);
		pass = false;
	}

	/* If no debug label was specified for the object then <label>
	 * will contain a null-terminated empty string, and zero will be returned
	 * in <length>.
	 */
	glBindBuffer(GL_ARRAY_BUFFER, buffers[TEST_NO_LABEL_IDX]);
	glGetObjectLabelEXT(GL_BUFFER, buffers[TEST_NO_LABEL_IDX], TestLabelLen + 1, &length, label);

	if (length != 0 || (strcmp("", label) != 0)) {
		fprintf(stderr, "Label should be empty and length 0\n");
		printf("  actual label: %s actual length: %i\n", label, length);
		pass = false;
	}

	/* If <label> is NULL and <length> is non-NULL then no string
	 * will be returned and the length of the label will be returned in
	 * <length>.
	 */
	glBindBuffer(GL_ARRAY_BUFFER, buffers[TEST_NULL_LABEL_IDX]);
	glLabelObjectEXT(GL_BUFFER, buffers[TEST_NULL_LABEL_IDX], 0, TestLabel);
	glGetObjectLabelEXT(GL_BUFFER, buffers[TEST_NULL_LABEL_IDX], TestLabelLen + 1, &length, NULL);

	if (length != TestLabelLen) {
		fprintf(stderr, "Label length should be %i\n", TestLabelLen);
		printf("  actual length: %i\n", length);
		pass = false;
	}

	/* If <length> is NULL, no length is returned.
	 */
	glBindBuffer(GL_ARRAY_BUFFER, buffers[TEST_NULL_LENGTH_IDX]);
	glLabelObjectEXT(GL_BUFFER, buffers[TEST_NULL_LENGTH_IDX], 0, TestLabel);
	glGetObjectLabelEXT(GL_BUFFER, buffers[TEST_NULL_LENGTH_IDX], TestLabelLen + 1, NULL, label);

	if (strcmp(TestLabel, label) != 0) {
		fprintf(stderr, "Label doent match expected string when length NULL\n");
		printf("  label: %s expected: %s\n", label, TestLabel);
		pass = false;
	}

	/* An INVALID_ENUM error is generated by GetObjectLabel if identifier is not
	 * one of the valid object types
	 */
	glGetObjectLabelEXT(GL_ARRAY_BUFFER, buffers[TEST_NULL_LENGTH_IDX], TestLabelLen + 1, &length, label);

	if (!piglit_check_gl_error(GL_INVALID_ENUM)) {
		fprintf(stderr, "GL_INVALID_ENUM should be generated when GetObjectLabel identifier is invalid\n");
		pass = false;
	}

	/* INVALID_VALUE is generated by GetObjectLabelEXT if <bufSize> is less than zero. */
	glGetObjectLabelEXT(GL_BUFFER, buffers[TEST_NULL_LENGTH_IDX], -1, &length, label);

	if (!piglit_check_gl_error(GL_INVALID_VALUE)) {
		fprintf(stderr, "GL_INVALID_VALUE should be generated when GetObjectLabel bufSize is less than 0\n");
		pass = false;
	}

	/* INVALID_OPERATION is generated by GetObjectLabelEXT
	 * if the type of <object> does not match <type>.
	 */
	invalidBufferName = buffers[TEST_NULL_LENGTH_IDX];
	glDeleteBuffers(numBuffers, buffers);
	glGetObjectLabelEXT(GL_BUFFER, invalidBufferName, TestLabelLen + 1, &length, label);

	if (!piglit_check_gl_error(GL_INVALID_OPERATION)) {
		fprintf(stderr, "GL_INVALID_OPERATION should be generated when GetObjectLabel name is invalid\n");
		pass = false;
	}

	return pass;
}

void piglit_init(int argc, char **argv)
{
	bool pass = true;

#ifdef PIGLIT_USE_OPENGL
	piglit_require_gl_version(11);
#endif
	piglit_require_extension("GL_EXT_debug_label");

	pass = test_object_label_types() && pass;
	pass = test_object_label() && pass;
	pass = test_get_object_label() && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
