// [config]
// expect_result: pass
// glsl_version: 4.00
// require_extensions: GL_AMD_gpu_shader_half_float
// [end config]
//
// Tests use of half float conversions with the scalar constructors

#version 400
#extension GL_AMD_gpu_shader_half_float : enable

float16_t test_bool_to_f16() {

	bool a = true;

	return float16_t(a);
}

float16_t test_int_to_f16() {

	int a = 5;

	return float16_t(a);
}

float16_t test_uint_to_f16() {

	uint a = 5u;

	return float16_t(a);
}

float16_t test_float_to_f16() {

	float a = 0.345;

	return float16_t(a);
}

float16_t test_double_to_f16() {

	double a = 0.345;

	return float16_t(a);
}

bool test_f16_to_bool() {

	float16_t a = 0.0hf;

	return bool(a);
}

int test_f16_to_int() {

	float16_t a = 2.0hf;

	return int(a);
}

uint test_f16_to_uint() {

	float16_t a = 3.0hf;

	return uint(a);
}

float test_f16_to_float() {

	float16_t a = 0.345hf;

	return float(a);
}

double test_f16_to_double() {

	float16_t a = 0.345hf;

	return double(a);
}
