// [config]
// expect_result: pass
// glsl_version: 4.50
// require_extensions: GL_AMD_gpu_shader_half_float
// [end config]
//
// Tests use of half float with fwidthFine() builtin

#version 450
#extension GL_AMD_gpu_shader_half_float : enable

f16vec3 test() {

	float16_t p = 0.567HF;

	return fwidthFine(f16vec3(p));
}