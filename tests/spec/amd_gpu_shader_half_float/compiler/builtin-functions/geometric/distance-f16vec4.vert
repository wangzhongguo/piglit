// [config]
// expect_result: pass
// glsl_version: 4.00
// require_extensions: GL_AMD_gpu_shader_half_float
// [end config]
//
// Tests use of half float with distance() builtin

#version 400
#extension GL_AMD_gpu_shader_half_float : enable

float16_t test() {

	float16_t p0 = 0.123HF;
	float16_t p1 = 0.456HF;

	return distance(f16vec4(p0), f16vec4(p1));
}
