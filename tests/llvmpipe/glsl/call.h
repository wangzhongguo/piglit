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

namespace glsl
{
   typedef Value(* CallFunction)(ShaderState& state, const std::vector<Value>&);

   class Call : public Operator
   {
   public:
      Call(const std::string& name, const CallFunction& function)
         : Operator(Operator::CALL), mName(name), mFunction(function)
      {
      }

      Call(const std::string& name, const CallFunction& function, Operator* arg1)
         : Operator(Operator::CALL), mName(name), mFunction(function)
      {
         mArguments.push_back(arg1);
      }

      Call(const std::string& name, const CallFunction& function, Operator* arg1, Operator* arg2)
         : Operator(Operator::CALL), mName(name), mFunction(function)
      {
         mArguments.push_back(arg1);
         mArguments.push_back(arg2);
      }

      Call(const std::string& name, const CallFunction& function, Operator* arg1, Operator* arg2, Operator* arg3)
         : Operator(Operator::CALL), mName(name), mFunction(function)
      {
         mArguments.push_back(arg1);
         mArguments.push_back(arg2);
         mArguments.push_back(arg3);
      }

      Call(const std::string& name, const CallFunction& function, const std::vector<Operator*>& args)
         : Operator(Operator::CALL), mName(name), mFunction(function), mArguments(args)
      {
      }

      ~Call()
      {
         for (std::vector<Operator*>::iterator itr = mArguments.begin(); itr != mArguments.end(); ++itr) {
            delete *itr;
         }
      }

      virtual Value execute(ShaderState& state) const
      {
         std::vector<Value> args;
         for (std::vector<Operator*>::const_iterator itr = mArguments.begin(); itr != mArguments.end(); ++itr) {
            args.push_back((*itr)->execute(state));
         }

         return (*mFunction)(state, args);
      }

      virtual std::string toGLSL() const
      {
         std::string glsl = mName + "(";

         for (std::vector<Operator*>::const_iterator itr = mArguments.begin(); itr != mArguments.end(); ++itr) {
            if (itr != mArguments.begin())
               glsl += ", ";

            glsl += (*itr)->toGLSL();
         }

         return glsl + ")";
      }

   protected:
      std::string mName;
      CallFunction mFunction;
      std::vector<Operator*> mArguments;
   };
}
