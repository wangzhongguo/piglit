// [config]
// expect_result: pass
// glsl_version: 4.00
// require_extensions: GL_AMD_gpu_shader_half_float
// [end config]
//
// Tests use of half float with interpolateAtOffset() builtin

#version 400
#extension GL_AMD_gpu_shader_half_float : enable

in f16vec3 p;

f16vec3 test() {

    f16vec2 offset = f16vec2(0.123, 0.456);

	return interpolateAtOffset(p, offset);
}