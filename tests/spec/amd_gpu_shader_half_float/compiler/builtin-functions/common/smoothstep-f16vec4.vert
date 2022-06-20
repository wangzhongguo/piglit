// [config]
// expect_result: pass
// glsl_version: 4.00
// require_extensions: GL_AMD_gpu_shader_half_float
// [end config]
//
// Tests use of half float with smoothstep() builtin

#version 400
#extension GL_AMD_gpu_shader_half_float : enable

f16vec4 test() {

	float16_t x = 0.999HF;
	float16_t edge0 = 0.567HF;
	float16_t edge1 = 0.890HF;

	return smoothstep(f16vec4(edge0), f16vec4(edge1), f16vec4(x)) +
		smoothstep(edge0, edge1, f16vec4(x));
}
