// [config]
// expect_result: pass
// glsl_version: 4.00
// require_extensions: GL_AMD_gpu_shader_half_float GL_ARB_gpu_shader_int64
// [end config]
//
// Tests use of half float conversions with the scalar constructors

#version 400
#extension GL_AMD_gpu_shader_half_float : enable
#extension GL_ARB_gpu_shader_int64 : enable

float16_t test_int64_to_f16() {

	int64_t a = 5l;

	return float16_t(a);
}

float16_t test_uint64_to_f16() {

	uint64_t a = 5ul;

	return float16_t(a);
}

int64_t test_f16_to_int64() {

	float16_t a = 1.0hf;

	return int64_t(a);
}

uint64_t test_f16_to_uint64() {

	float16_t a = 2.0hf;

	return uint64_t(a);
}
