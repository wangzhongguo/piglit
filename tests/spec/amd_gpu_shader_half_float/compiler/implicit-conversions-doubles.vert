// [config]
// expect_result: pass
// glsl_version: 4.00
// require_extensions: GL_AMD_gpu_shader_half_float
// [end config]
//
// Tests use of half float implicit conversions to doubles

#version 400
#extension GL_AMD_gpu_shader_half_float : enable

double test_float16_t_to_double() {

	float16_t a = 0.123hf;

	return a;
}

dvec2 test_f16vec2_to_dvec2() {

	f16vec2 a = f16vec2(0.123hf);

	return a;
}

dvec3 test_f16vec3_to_dvec3() {

	f16vec3 a = f16vec3(0.123hf);

	return a;
}

dvec4 test_f16vec4_to_dvec4() {

	f16vec4 a = f16vec4(0.123hf);

	return a;
}

dmat2 test_f16mat2_to_dmat2() {

	f16mat2 a = f16mat2(0.123hf);

	return a;
}

dmat3 test_f16mat3_to_dmat3() {

	f16mat3 a = f16mat3(0.123hf);

	return a;
}

dmat4 test_f16mat4_to_dmat4() {

	f16mat4 a = f16mat4(0.123hf);

	return a;
}

dmat2x3 test_f16mat2x3_to_dmat2x3() {

	f16mat2x3 a = f16mat2x3(0.123hf);

	return a;
}

dmat2x4 test_f16mat2x4_to_dmat2x4() {

	f16mat2x4 a = f16mat2x4(0.123hf);

	return a;
}

dmat3x2 test_f16mat3x2_to_dmat3x2() {

	f16mat3x2 a = f16mat3x2(0.123hf);

	return a;
}

dmat3x4 test_f16mat3x4_to_dmat3x4() {

	f16mat3x4 a = f16mat3x4(0.123hf);

	return a;
}

dmat4x2 test_f16mat4x2_to_dmat4x2() {

	f16mat4x2 a = f16mat4x2(0.123hf);

	return a;
}

dmat4x3 test_f16mat4x3_to_dmat4x3() {

	f16mat4x3 a = f16mat4x3(0.123hf);

	return a;
}