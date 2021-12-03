/** @file test.c
 *
 * Tests gl_LastFragDepthARM/gl_LastFragStencilARM with different ZS attachment formats.
 */

#include "piglit-util-gl.h"

static const int BUF_SIZE = 400;

PIGLIT_GL_TEST_CONFIG_BEGIN

#if defined(PIGLIT_USE_OPENGL)
	config.supports_gl_core_version = 33;
#elif defined(PIGLIT_USE_OPENGL_ES3)
	config.supports_gl_es_version = 30;
#endif

	config.window_width = BUF_SIZE;
	config.window_height = BUF_SIZE;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#if defined(PIGLIT_USE_OPENGL)
#define SHADER_VERSION_HEADER "#version 330 core\n"
#define FRAGMENT_SHADER_PRECISION
#elif defined(PIGLIT_USE_OPENGL_ES3)
#define SHADER_VERSION_HEADER "#version 300 es\n"
#define FRAGMENT_SHADER_PRECISION "precision highp float;\n"
#endif

/* Viewport-covering triangle with z = 0 (0.5 in zbuffer) */
static const char vertex_shader[] =
SHADER_VERSION_HEADER
"void main()\n"
"{\n"
"  const vec2 NDC[3] = vec2[3](\n"
"    vec2(-1.0, -1.0),\n"
"    vec2(+3.0, -1.0),\n"
"    vec2(-1.0, +3.0)\n"
"  );\n"
"  gl_Position = vec4(NDC[gl_VertexID], 0.0, 1.0);\n"
"}\n";

#define FRAGMENT_SHADER_HEADER \
	"#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require\n" \
	FRAGMENT_SHADER_PRECISION \
	"layout(location = 0) out vec4 color;\n"

/* First draw expects z = 0, writes g = 0, z = 0.5
 * Second draw expects z >= 0.5, writes g = 1, z = 0.5
 */
static const char fragment_shader_basic_depth[] =
SHADER_VERSION_HEADER
FRAGMENT_SHADER_HEADER
"void main()\n"
"{\n"
"  float g = gl_LastFragDepthARM * 2.1;\n"
"  color = vec4(1.0 - g, g, 0.0, 1.0);\n"
"}\n";

/* First draw expects s = 0, writes g = 0, s = 1
 * Second draw expects s = 1, writes g = 1, s = 1
 */
static const char fragment_shader_basic_stencil[] =
SHADER_VERSION_HEADER
FRAGMENT_SHADER_HEADER
"void main()\n"
"{\n"
"  float g = float(gl_LastFragStencilARM == 1);\n"
"  color = vec4(1.0 - g, g, 0.0, 1.0);\n"
"}\n";

/* First draw expects z = 0, writes g = 0.51, z = 0.51
 * Second draw expects z >= 0.5, writes g = 1, z = 1
 */
static const char fragment_shader_export_depth[] =
SHADER_VERSION_HEADER
FRAGMENT_SHADER_HEADER
"void main()\n"
"{\n"
"  float g = gl_LastFragDepthARM + 0.51;\n"
"  gl_FragDepth = g;\n"
"  color = vec4(1.0 - g, g, 0.0, 1.0);\n"
"}\n";

/* First draw expects s = 0, writes g = 0, s = 5
 * Second draw expects s = 5, writes g = 1, s = 0
 */
static const char fragment_shader_export_stencil[] =
SHADER_VERSION_HEADER
"#extension GL_ARB_shader_stencil_export : require\n"
FRAGMENT_SHADER_HEADER
"void main()\n"
"{\n"
"  float g = float(gl_LastFragStencilARM == 5);\n"
"  gl_FragStencilRefARB = g < 0.5 ? 5 : 0;\n"
"  color = vec4(1.0 - g, g, 0.0, 1.0);\n"
"}\n";

static bool g_has_depth;
static bool g_has_stencil;
static GLuint g_fbo;

static void
run_draws(const char *fragment_shader)
{
	GLuint prog = piglit_build_simple_program(vertex_shader, fragment_shader);
	if (!prog) {
		piglit_report_result(PIGLIT_FAIL);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);
	const float clear_color[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	glClearBufferfv(GL_COLOR, 0, clear_color);
	glClearBufferfi(GL_DEPTH_STENCIL, 0, 0.0f, 0);

	glUseProgram(prog);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, BUF_SIZE, BUF_SIZE, 0, 0, BUF_SIZE, BUF_SIZE,
		GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);

	const float expected[] = { 0.0f, 1.0f, 0.0f, 1.0f };
	if (!piglit_probe_rect_rgba(0, 0, BUF_SIZE, BUF_SIZE, expected)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_present_results();

	glDeleteProgram(prog);
}

enum piglit_result
piglit_display(void)
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_TRUE);

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilMask(0xFF);

	glViewport(0, 0, BUF_SIZE, BUF_SIZE);

	if (g_has_depth) {
		printf("Basic depth fetch test\n");
		run_draws(fragment_shader_basic_depth);

		printf("Depth export+fetch test\n");
		run_draws(fragment_shader_export_depth);
	}

	if (g_has_stencil) {
		printf("Basic stencil fetch test\n");
		run_draws(fragment_shader_basic_stencil);

		if (piglit_is_extension_supported("GL_ARB_shader_stencil_export")) {
			printf("Stencil export+fetch test\n");
			run_draws(fragment_shader_export_stencil);
		}
	}

	return PIGLIT_PASS;
}

static void
usage(const char *test_name)
{
	printf("Usage: %s <format> <samples>\n"
		"  where <format> :\n"
		"    GL_STENCIL_INDEX8\n"
		"    GL_DEPTH_COMPONENT16\n"
		"    GL_DEPTH_COMPONENT24\n"
		"    GL_DEPTH_COMPONENT32F\n"
		"    GL_DEPTH24_STENCIL8\n"
		"    GL_DEPTH32F_STENCIL8\n"
		"  <samples> : 1, 2, 4, 8, 16\n", test_name);
}

static void
create_fbo(GLenum zs_format, GLsizei samples)
{
	GLuint color;
	glGenRenderbuffers(1, &color);
	glBindRenderbuffer(GL_RENDERBUFFER, color);
	if (samples > 1) {
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGBA8, BUF_SIZE, BUF_SIZE);
	} else {
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, BUF_SIZE, BUF_SIZE);
	}

	GLuint zs;
	glGenRenderbuffers(1, &zs);
	glBindRenderbuffer(GL_RENDERBUFFER, zs);
	if (samples > 1) {
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, zs_format, BUF_SIZE, BUF_SIZE);
	} else {
		glRenderbufferStorage(GL_RENDERBUFFER, zs_format, BUF_SIZE, BUF_SIZE);
	}

	glGenFramebuffers(1, &g_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color);
	if (g_has_depth) {
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, zs);
	}
	if (g_has_stencil) {
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, zs);
	}

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		printf("FBO incomplete (%s)\n", piglit_get_gl_enum_name(status));
		piglit_report_result(PIGLIT_FAIL);
	}
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARM_shader_framebuffer_fetch_depth_stencil");

	if (argc < 3) {
		usage(argv[0]);
		piglit_report_result(PIGLIT_SKIP);
	}

	const struct {
		GLenum format;
		bool has_z;
		bool has_s;
	} known_zs_formats[] = {
		{ GL_STENCIL_INDEX8, false, true },
		{ GL_DEPTH_COMPONENT16, true, false },
		{ GL_DEPTH_COMPONENT24, true, false },
		{ GL_DEPTH_COMPONENT32F, true, false },
		{ GL_DEPTH24_STENCIL8, true, true },
		{ GL_DEPTH32F_STENCIL8, true, true }
	};

	const GLenum format = piglit_get_gl_enum_from_name(argv[1]);
	bool found_format = false;
	for (int i = 0; i < ARRAY_SIZE(known_zs_formats); i++) {
		if (known_zs_formats[i].format == format) {
			g_has_depth = known_zs_formats[i].has_z;
			g_has_stencil = known_zs_formats[i].has_s;
			found_format = true;
			break;
		}
	}

	if (!found_format) {
		printf("Unsupported format\n");
		usage(argv[0]);
		piglit_report_result(PIGLIT_SKIP);
	}

	const GLsizei samples = atoi(argv[2]);
	if (samples < 1) {
		printf("Sample count must be positive\n");
		usage(argv[0]);
		piglit_report_result(PIGLIT_SKIP);
	}

	GLint max_samples;
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
	if (samples > max_samples) {
		printf("Sample count %d is not supported (max %d)\n", samples, max_samples);
		piglit_report_result(PIGLIT_SKIP);
	}

	create_fbo(format, samples);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
}
