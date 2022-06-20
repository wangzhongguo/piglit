// [config]
// expect_result: pass
// glsl_version: 4.00
// require_extensions: GL_AMD_gpu_shader_half_float
// [end config]
//
// Tests use of half float with dFdy() builtin

#version 400
#extension GL_AMD_gpu_shader_half_float : enable

float16_t test() {

	float16_t p = 0.567HF;

	return dFdy(p);
}