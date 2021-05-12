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

#include "call.h"
#include "constant.h"
#include "immediate.h"
#include "binaryoperator.h"

#include <map>

namespace glsl
{
   class Shader
   {
   public:
      Shader()
      {
         addVariable(Variable("vTexture0", Variable::BuiltIn, Value::vec4));
         addVariable(Variable("vFragColor", Variable::BuiltIn, Value::vec4));

         addVariable(Variable("gl_Color", Variable::BuiltIn, Value::vec4));
         addVariable(Variable("gl_Vertex", Variable::BuiltIn, Value::vec4));
         addVariable(Variable("gl_Position", Variable::BuiltIn, Value::vec4));
         addVariable(Variable("gl_BackColor", Variable::BuiltIn, Value::vec4));
         addVariable(Variable("gl_FragColor", Variable::BuiltIn, Value::vec4));
         addVariable(Variable("gl_FrontColor", Variable::BuiltIn, Value::vec4));
         addVariable(Variable("gl_TexCoord[0]", Variable::BuiltIn, Value::vec4));
         addVariable(Variable("gl_MultiTexCoord0", Variable::BuiltIn, Value::vec4));
         addVariable(Variable("gl_ModelViewProjectionMatrix", Variable::BuiltIn, Value::vec4));
      }

      ~Shader()
      {
         for (std::vector<Operator*>::iterator itr = mProgram.begin(); itr != mProgram.end(); ++itr)
            delete *itr;
      }

      Constant addLocalVec4(const std::string& name)
      {
         return addVariable(Variable(name, Variable::Local, Value::vec4));
      }

      Constant addUniformVec4(const std::string& name)
      {
         return addVariable(Variable(name, Variable::Uniform, Value::vec4));
      }

      Constant addUniformSampler2D(const std::string& name)
      {
         return addVariable(Variable(name, Variable::UniformSampler, Value::none));
      }

      Constant addVariable(const Variable& variable)
      {
         mVariables[variable.name()] = variable;
         return Constant(variable);
      }

      Constant operator[](const std::string& name)
      {
         std::map<std::string, Variable>::iterator itr = mVariables.find(name);
         assert(itr != mVariables.end());
         return Constant(itr->second);
      }

      Variable& variable(const std::string& name)
      {
         return mVariables[name];
      }

      Shader& operator<<(Operator* op)
      {
         mProgram.push_back(op);
         return *this;
      }

      void start(ShaderState& state)
      {
         state = ShaderState(mVariables);
      }

      void run(ShaderState& state)
      {
         for (std::vector<Operator*>::const_iterator itr = mProgram.begin(); itr != mProgram.end(); ++itr) {
            (*itr)->execute(state);
         }
      }

      void end(ShaderState& state)
      {
      }

      std::string toGLSL() const
      {
         std::stringstream ss;
         ss.precision(9);

         for (std::map<std::string, Variable>::const_iterator itr = mVariables.begin(); itr != mVariables.end(); ++itr) {
            const Variable& var = itr->second;

            if (var.declare())
               ss << var.toGLSL() << ";" << std::endl;
         }

         if (ss.tellp())
            ss << std::endl;

         ss << "void main(void)" << std::endl;
         ss << "{" << std::endl;

         for (std::map<std::string, Variable>::const_iterator itr = mVariables.begin(); itr != mVariables.end(); ++itr) {
            const Variable& var = itr->second;

            if (var.type() == Variable::Local)
               ss << "\t" << var.toGLSL() << ";" << std::endl;
         }

         for (std::vector<Operator*>::const_iterator itr = mProgram.begin(); itr != mProgram.end(); ++itr) {
            ss << "\t" << (*itr)->toGLSL() << ";" << std::endl;
         }

         ss << "}" << std::endl;
         return ss.str();
      }

      const std::map<std::string, Variable>& variables() const
      {
         return mVariables;
      }

   private:
      std::map<std::string, Variable> mVariables;
      std::vector<Operator*> mProgram;
   };
}
