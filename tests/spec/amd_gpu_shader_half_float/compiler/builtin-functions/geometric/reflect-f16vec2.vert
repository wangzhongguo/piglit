// [config]
// expect_result: pass
// glsl_version: 4.00
// require_extensions: GL_AMD_gpu_shader_half_float
// [end config]
//
// Tests use of half float with reflect() builtin

#version 400
#extension GL_AMD_gpu_shader_half_float : enable

f16vec2 test() {

	float16_t N = 0.123HF;
	float16_t I = 0.456HF;
	
	return reflect(f16vec2(I), f16vec2(N));
}
