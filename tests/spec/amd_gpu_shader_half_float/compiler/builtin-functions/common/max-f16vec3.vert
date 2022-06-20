// [config]
// expect_result: pass
// glsl_version: 4.00
// require_extensions: GL_AMD_gpu_shader_half_float
// [end config]
//
// Tests use of half float with max() builtin

#version 400
#extension GL_AMD_gpu_shader_half_float : enable

f16vec3 test() {

	float16_t x = 0.999HF;
	float16_t y = 0.567HF;

	return max(f16vec3(x), f16vec3(y)) + max(f16vec3(x), y);
}
