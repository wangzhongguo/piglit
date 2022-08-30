// [config]
// expect_result: pass
// glsl_version: 4.00
// require_extensions: GL_AMD_gpu_shader_half_float
// [end config]
//
// Tests use of half float with step() builtin

#version 400
#extension GL_AMD_gpu_shader_half_float : enable

float16_t test() {

	float16_t x = 0.999HF;
	float16_t edge = 0.567HF;

	return step(edge, x);
}