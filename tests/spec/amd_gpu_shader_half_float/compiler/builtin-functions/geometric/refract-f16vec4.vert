// [config]
// expect_result: pass
// glsl_version: 4.00
// require_extensions: GL_AMD_gpu_shader_half_float
// [end config]
//
// Tests use of half float with refract() builtin

#version 400
#extension GL_AMD_gpu_shader_half_float : enable

f16vec4 test() {

	float16_t N = 0.123HF;
	float16_t I = 0.456HF;
	float16_t eta = 0.789HF;
	
	return refract(f16vec4(I), f16vec4(N), eta);
}
