// [config]
// expect_result: pass
// glsl_version: 4.00
// require_extensions: GL_AMD_gpu_shader_half_float
// [end config]
//
// Tests use of half float with inverse() builtin

#version 400
#extension GL_AMD_gpu_shader_half_float : enable

f16mat2 test_mat2() {

	float16_t m = 0.123HF;
	
	return inverse(f16mat2(m));
}

f16mat3 test_mat3() {

	float16_t m = 0.123HF;
	
	return inverse(f16mat3(m));
}

f16mat4 test_mat4() {

	float16_t m = 0.123HF;
	
	return inverse(f16mat4(m));
}