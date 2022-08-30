// [config]
// expect_result: pass
// glsl_version: 4.00
// require_extensions: GL_AMD_gpu_shader_half_float GL_ARB_shading_language_420pack
// [end config]
//
// Tests use of half float implicit conversions

#version 400
#extension GL_AMD_gpu_shader_half_float : enable
#extension GL_ARB_shading_language_420pack : enable

float test_float16_t_to_float() {

	float16_t a = 0.123hf;

	return a;
}

vec2 test_f16vec2_to_vec2() {

	f16vec2 a = f16vec2(0.123hf);

	return a;
}

vec3 test_f16vec3_to_vec3() {

	f16vec3 a = f16vec3(0.123hf);

	return a;
}

vec4 test_f16vec4_to_vec4() {

	f16vec4 a = f16vec4(0.123hf);

	return a;
}

mat2 test_f16mat2_to_mat2() {

	f16mat2 a = f16mat2(0.123hf);

	return a;
}

mat3 test_f16mat3_to_mat3() {

	f16mat3 a = f16mat3(0.123hf);

	return a;
}

mat4 test_f16mat4_to_mat4() {

	f16mat4 a = f16mat4(0.123hf);

	return a;
}

mat2x3 test_f16mat2x3_to_mat2x3() {

	f16mat2x3 a = f16mat2x3(0.123hf);

	return a;
}

mat2x4 test_f16mat2x4_to_mat2x4() {

	f16mat2x4 a = f16mat2x4(0.123hf);

	return a;
}

mat3x2 test_f16mat3x2_to_mat3x2() {

	f16mat3x2 a = f16mat3x2(0.123hf);

	return a;
}

mat3x4 test_f16mat3x4_to_mat3x4() {

	f16mat3x4 a = f16mat3x4(0.123hf);

	return a;
}

mat4x2 test_f16mat4x2_to_mat4x2() {

	f16mat4x2 a = f16mat4x2(0.123hf);

	return a;
}

mat4x3 test_f16mat4x3_to_mat4x3() {

	f16mat4x3 a = f16mat4x3(0.123hf);

	return a;
}