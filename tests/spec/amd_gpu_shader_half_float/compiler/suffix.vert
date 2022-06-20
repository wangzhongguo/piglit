// [config]
// expect_result: pass
// glsl_version: 4.00
// require_extensions: GL_AMD_gpu_shader_half_float
// [end config]
//
// Tests use of lower and uppercase half float suffix

#version 400
#extension GL_AMD_gpu_shader_half_float : enable

float16_t test() {

	float16_t x = 0.123HF;
	float16_t y = 0.123hf;
	
	return x + y;
}
