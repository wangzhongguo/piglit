// [config]
// expect_result: pass
// glsl_version: 4.00
// require_extensions: GL_AMD_gpu_shader_half_float
// [end config]
//
// Tests use of half float with outerProduct() builtin

#version 400
#extension GL_AMD_gpu_shader_half_float : enable

f16mat2 test_mat2() {

	float16_t c = 0.123HF;
	float16_t r = 0.456HF;
	
	return outerProduct(f16vec2(c), f16vec2(r));
}

f16mat3 test_mat3() {

	float16_t c = 0.123HF;
	float16_t r = 0.456HF;
	
	return outerProduct(f16vec3(c), f16vec3(r));
}

f16mat4 test_mat4() {

	float16_t c = 0.123HF;
	float16_t r = 0.456HF;
	
	return outerProduct(f16vec4(c), f16vec4(r));
}

f16mat2x3 test_mat2x3() {

	float16_t c = 0.123HF;
	float16_t r = 0.456HF;
	
	return outerProduct(f16vec3(c), f16vec2(r));
}

f16mat3x2 test_mat3x2() {

	float16_t c = 0.123HF;
	float16_t r = 0.456HF;
	
	return outerProduct(f16vec2(c), f16vec3(r));
}

f16mat2x4 test_mat2x4() {

	float16_t c = 0.123HF;
	float16_t r = 0.456HF;
	
	return outerProduct(f16vec4(c), f16vec2(r));
}

f16mat4x2 test_mat4x2() {

	float16_t c = 0.123HF;
	float16_t r = 0.456HF;
	
	return outerProduct(f16vec2(c), f16vec4(r));
}

f16mat3x4 test_mat3x4() {

	float16_t c = 0.123HF;
	float16_t r = 0.456HF;
	
	return outerProduct(f16vec4(c), f16vec3(r));
}

f16mat4x3 test_mat4x3() {

	float16_t c = 0.123HF;
	float16_t r = 0.456HF;
	
	return outerProduct(f16vec3(c), f16vec4(r));
}