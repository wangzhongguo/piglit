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

#include "piglit-util-gl.h"
#include "shader.h"
#include "state.h"

namespace glsl
{
   class Program
   {
   public:
      Program(Shader* vertex = NULL, Shader* fragment = NULL)
         : mVertex(vertex), mFragment(fragment)
      {
         memset(&mStateGL, 0, sizeof(mStateGL));
      }

      ~Program()
      {
         delete mVertex;
         delete mFragment;

         if (mStateGL.vertex)
            glDeleteShader(mStateGL.vertex);

         if (mStateGL.fragment)
            glDeleteShader(mStateGL.fragment);

         if (mStateGL.program)
            glDeleteProgram(mStateGL.program);
      }

      void setFragmentShader(Shader* shader)
      {
         mFragment = shader;
      }

      void setVertexShader(Shader* shader)
      {
         mVertex = shader;
      }

      void setUniform(const std::string& name, const Value& value)
      {
         loadProgram();

         if (mVertex)
            mVertex->variable(name) = value;

         if (mFragment)
            mFragment->variable(name) = value;

         mShaderState[name] = value;

         glUseProgram(mStateGL.program);

         if (value.size() == 4) {
            glUniform4fv(mShaderState[name].location(), 1, mShaderState[name].value());
         }
      }

      void setUniformSampler2D(const std::string& name, GLuint sampler)
      {
         loadProgram();

         Value value = Value((float)sampler);
         if (mVertex)
            mVertex->variable(name) = value;

         if (mFragment)
            mFragment->variable(name) = value;

         mShaderState[name] = value;

         glUseProgram(mStateGL.program);
         glUniform1i(mShaderState[name].location(), sampler);
      }

      void applyState()
      {
         loadProgram();
         glUseProgram(mStateGL.program);
      }

      Shader* vertex()
      {
         return mVertex;
      }

      Shader* fragment()
      {
         return mFragment;
      }

      const std::map<std::string, Variable>& variables() const {
         return mShaderState.mVariables;
      }

   private:
      void loadProgram()
      {
         if (mStateGL.program)
            return;

         /* Create shader in GL */
         if (mVertex)
            mStateGL.vertex = piglit_compile_shader_text(GL_VERTEX_SHADER, mVertex->toGLSL().c_str());

         if (mFragment)
            mStateGL.fragment = piglit_compile_shader_text(GL_FRAGMENT_SHADER, mFragment->toGLSL().c_str());

         mStateGL.program = piglit_link_simple_program(mStateGL.vertex, mStateGL.fragment);

         glUseProgram(mStateGL.program);

         /* Capture current variables */
         if (mVertex)
            mShaderState.mVariables.insert(mVertex->variables().begin(), mVertex->variables().end());

         if (mFragment)
            mShaderState.mVariables.insert(mFragment->variables().begin(), mFragment->variables().end());

         /* Get uniform locations */
         for (std::map<std::string, Variable>::iterator itr = mShaderState.mVariables.begin(); itr != mShaderState.mVariables.end(); ++itr) {
            if (itr->second.isUniform())
               itr->second.setLocation(glGetUniformLocation(mStateGL.program, itr->first.c_str()));
         }
      }

   private:
      struct {
         GLuint vertex;
         GLuint fragment;
         GLuint program;
      } mStateGL;

      Shader* mVertex;
      Shader* mFragment;
      ShaderState mShaderState;
   };
}
