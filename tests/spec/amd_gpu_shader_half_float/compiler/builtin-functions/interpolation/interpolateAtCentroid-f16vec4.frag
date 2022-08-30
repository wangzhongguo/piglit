// [config]
// expect_result: pass
// glsl_version: 4.00
// require_extensions: GL_AMD_gpu_shader_half_float
// [end config]
//
// Tests use of half float with interpolateAtCentroid() builtin

#version 400
#extension GL_AMD_gpu_shader_half_float : enable

in f16vec4 p;

f16vec4 test() {

	return interpolateAtCentroid(p);
}