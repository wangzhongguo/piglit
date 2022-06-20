// [config]
// expect_result: pass
// glsl_version: 4.00
// require_extensions: GL_AMD_gpu_shader_half_float
// [end config]
//
// Tests use of half float with matrixCompMult() builtin

#version 400
#extension GL_AMD_gpu_shader_half_float : enable

f16mat2x2 test_mat2x2() {

	float16_t x = 0.123HF;
	float16_t y = 0.456HF;
	
	return matrixCompMult(f16mat2x2(x), f16mat2x2(y));
}

f16mat2x3 test_mat2x3() {

	float16_t x = 0.123HF;
	float16_t y = 0.456HF;
	
	return matrixCompMult(f16mat2x3(x), f16mat2x3(y));
}

f16mat2x4 test_mat2x4() {

	float16_t x = 0.123HF;
	float16_t y = 0.456HF;
	
	return matrixCompMult(f16mat2x4(x), f16mat2x4(y));
}

f16mat3x2 test_mat3x2() {

	float16_t x = 0.123HF;
	float16_t y = 0.456HF;
	
	return matrixCompMult(f16mat3x2(x), f16mat3x2(y));
}

f16mat3x3 test_mat3x3() {

	float16_t x = 0.123HF;
	float16_t y = 0.456HF;
	
	return matrixCompMult(f16mat3x3(x), f16mat3x3(y));
}

f16mat3x4 test_mat3x4() {

	float16_t x = 0.123HF;
	float16_t y = 0.456HF;
	
	return matrixCompMult(f16mat3x4(x), f16mat3x4(y));
}

f16mat4x2 test_mat4x2() {

	float16_t x = 0.123HF;
	float16_t y = 0.456HF;
	
	return matrixCompMult(f16mat4x2(x), f16mat4x2(y));
}

f16mat4x3 test_mat4x3() {

	float16_t x = 0.123HF;
	float16_t y = 0.456HF;
	
	return matrixCompMult(f16mat4x3(x), f16mat4x3(y));
}

f16mat4x4 test_mat4x4() {

	float16_t x = 0.123HF;
	float16_t y = 0.456HF;
	
	return matrixCompMult(f16mat4x4(x), f16mat4x4(y));
}