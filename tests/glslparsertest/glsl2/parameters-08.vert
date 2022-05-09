// [config]
// expect_result: fail
// glsl_version: 1.10
//
// [end config]

/* FAIL - size must be specified for formal parameter arrays
 */
float a(float x[], int y)
{
  return x[y];
}
