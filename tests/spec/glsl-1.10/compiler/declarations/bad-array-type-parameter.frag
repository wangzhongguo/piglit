// [config]
// expect_result: fail
// glsl_version: 1.10
// [end config]
//
// Compilation should fail because the type 'B' is unknown.
//
// Reproduces Mesa bugzilla #33313.

void b(B x[1]);

void main() { gl_FragColor = vec4(1.0); }
