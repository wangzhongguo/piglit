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

#pragma once

#include "operator.h"
#include "selector.h"
#include "variable.h"
#include "immediate.h"

namespace glsl
{
   class Constant : public Operator
   {
      friend class Shader;
      friend class ShaderConstant;

   public:
      Constant(const Variable& variable)
         : Operator(Operator::CONSTANT), mVariable(variable.name()), mSelector(variable.value())
      {
      }

      Constant(const std::string& variable, Selector selector)
         : Operator(Operator::CONSTANT), mVariable(variable), mSelector(selector)
      {
      }

      Constant(const Constant& src, const Selector& selector)
         : Operator(Operator::CONSTANT), mVariable(src.mVariable), mSelector(selector)
      {
      }

      ~Constant()
      {
      }

      virtual Value execute(ShaderState& state) const
      {
         return mSelector.select(state[mVariable].value());
      }

      virtual std::string toGLSL() const
      {
         std::string selector = mSelector.toString();
         if (selector.length())
            return mVariable + "." + selector;
         else
            return mVariable;
      }

      void setValue(ShaderState& result, const Value& value) const
      {
         result[mVariable] = mSelector.select(result[mVariable].value(), value);
      }

   protected:
      std::string mVariable;
      Selector mSelector;
   };
}
