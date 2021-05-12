/*
 * Copyright © 2012-2021 VMware, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "piglit-util-gl.h"
#include "shader.h"
#include "functions.h"

namespace glsl
{
   const Value Value::none;
   const Value Value::vec2(0.0f, 0.0f);
   const Value Value::vec3(0.0f, 0.0f, 0.0f);
   const Value Value::vec4(0.0f, 0.0f, 0.0f, 0.0f);

   const Selector Selector::x = Selector(Selector::X);
   const Selector Selector::y = Selector(Selector::Y);
   const Selector Selector::z = Selector(Selector::Z);
   const Selector Selector::w = Selector(Selector::W);
   const Selector Selector::xy = Selector(Selector::X, Selector::Y);
   const Selector Selector::xyzw = Selector(Selector::X, Selector::Y, Selector::Z, Selector::W);

   Value vec2(float x, float y)
   {
      return Value(x, y);
   }

   Value vec3(float x, float y, float z)
   {
      return Value(x, y, z);
   }

   Value vec4(float x, float y, float z, float w)
   {
      return Value(x, y, z, w);
   }

   Value shader_texture2D(ShaderState& state, const std::vector<Value>& arguments)
   {
      Colour* tex_ptr = state["$TEX0_PTR"].value().value<Colour*>();
      int width = state["$TEX0_WIDTH"].value().value<int>();
      const int* xy = arguments.at(1).valuePtr<int>();

      return tex_ptr[xy[0] + xy[1] * width];
   }

   Call* texture2D(Operator* sampler, Operator* coords)
   {
      return new Call("texture2D", shader_texture2D, sampler, coords);
   }
}
