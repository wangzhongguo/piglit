/* [config]
 * expect_result: pass
 * glsl_version: 1.20
 * require_extensions: GL_ARB_shader_storage_buffer_object
 * [end config]
 */

/*
 * From ARB_shader_storage_buffer_object spec:
 *
 *  If an array has not been explicitly sized and is not the last declared
 *  member of a shader storage block, the value returned by the length method is
 *  not a constant expression and will be determined when a program is linked.
 *
 * Using .length() method on implicitly-sized arrays was originally introduced
 * with ARB_shader_storage_buffer_object extension. It was promoted to core in
 * GLSL 4.3.
 *
 * GLSL 4.60.5 introduced a contradiction, that was later resolved:
 * See private GLSL issue 32.
 */

#version 120
#extension GL_ARB_shader_storage_buffer_object: require

void main()
{
   float unsized_array[];
   unsized_array[0] = unsized_array.length(); // implicitly-sized array
   gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}
