// [config]
// expect_result: pass
// glsl_version: 4.00
// require_extensions: GL_AMD_gpu_shader_half_float GL_AMD_shader_trinary_minmax
// [end config]
//
// Tests use of half float with min3() builtin

#version 400
#extension GL_AMD_gpu_shader_half_float : enable
#extension GL_AMD_shader_trinary_minmax : enable

float16_t test() {

	float16_t x = 0.999HF;
	float16_t y = 0.567HF;
	float16_t z = 0.123HF;

	return min3(x, y, z);
}
