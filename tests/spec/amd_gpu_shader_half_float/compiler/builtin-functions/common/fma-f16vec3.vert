// [config]
// expect_result: pass
// glsl_version: 4.00
// require_extensions: GL_AMD_gpu_shader_half_float
// [end config]
//
// Tests use of half float with fma() builtin

#version 400
#extension GL_AMD_gpu_shader_half_float : enable

f16vec3 test() {

	float16_t a = 0.999HF;
	float16_t b = 0.345HF;
	float16_t c = 0.678HF;

	return fma(f16vec3(a), f16vec3(b), f16vec3(c));
}
