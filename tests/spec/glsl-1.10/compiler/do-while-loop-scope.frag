/* [config]
 * expect_result: fail
 * glsl_version: 1.10
 * [end config]
 */

void main()
{
   do
      int var_1 = 0;
   while (false);

   var_1++;
}
