// [config]
// expect_result: pass
// glsl_version: 4.00
// require_extensions: GL_AMD_gpu_shader_half_float
// [end config]
//
// Tests use of half float with min() builtin

#version 400
#extension GL_AMD_gpu_shader_half_float : enable

f16vec4 test() {

	float16_t x = 0.999HF;
	float16_t y = 0.567HF;

	return min(f16vec4(x), f16vec4(y)) + min(f16vec4(x), y);
}
