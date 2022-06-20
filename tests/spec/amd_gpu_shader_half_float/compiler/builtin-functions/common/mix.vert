// [config]
// expect_result: pass
// glsl_version: 4.00
// require_extensions: GL_AMD_gpu_shader_half_float
// [end config]
//
// Tests use of half float with mix() builtin

#version 400
#extension GL_AMD_gpu_shader_half_float : enable

float16_t test() {

	float16_t x = 0.999HF;
	float16_t y = 0.567HF;
	float16_t a = 0.890HF;
	bool a_bool = false;

	return mix(x, y, a) + mix(x, y, a_bool);
}
