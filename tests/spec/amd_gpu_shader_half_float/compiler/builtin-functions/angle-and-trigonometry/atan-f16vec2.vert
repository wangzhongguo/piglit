// [config]
// expect_result: pass
// glsl_version: 4.00
// require_extensions: GL_AMD_gpu_shader_half_float
// [end config]
//
// Tests use of half float with atan() builtin

#version 400
#extension GL_AMD_gpu_shader_half_float : enable

f16vec2 test() {

	float16_t x = 0.999HF;
	float16_t y = 0.555HF;

	return atan(f16vec2(x)) + atan(f16vec2(y), f16vec2(x));
}
