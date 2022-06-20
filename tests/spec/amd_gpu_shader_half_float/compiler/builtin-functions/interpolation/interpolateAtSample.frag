// [config]
// expect_result: pass
// glsl_version: 4.00
// require_extensions: GL_AMD_gpu_shader_half_float
// [end config]
//
// Tests use of half float with interpolateAtSample() builtin

#version 400
#extension GL_AMD_gpu_shader_half_float : enable

in float16_t p;

float16_t test() {

    int s = 1;

	return interpolateAtSample(p, s);
}