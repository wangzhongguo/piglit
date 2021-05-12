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
#include "constant.h"

namespace glsl
{
   class BinaryOperator : public Operator
   {
   public:
      enum bOperator {
         ADD,
         SUB,
         MUL,
         DIV,
         MOV
      };

   public:
      BinaryOperator(bOperator op, Operator* left, Operator* right)
         : Operator(Operator::BINARY), mOperator(op), mLeft(left), mRight(right)
      {
      }

      ~BinaryOperator()
      {
         if (mLeft)
            delete mLeft;

         if (mRight)
            delete mRight;
      }

      virtual Value execute(ShaderState& state) const
      {
         switch (mOperator) {
            case ADD:
               return mLeft->execute(state) + mRight->execute(state);
            case SUB:
               return mLeft->execute(state) - mRight->execute(state);
            case MUL:
               return mLeft->execute(state) * mRight->execute(state);
            case DIV:
               return mLeft->execute(state) / mRight->execute(state);
            case MOV:
               assert(mLeft->operatorType() == Operator::CONSTANT);
               ((Constant*)mLeft)->setValue(state, mRight->execute(state));
               return mLeft->execute(state);
            default:
               return Value::vec4;
         }
      }

      virtual std::string toGLSL() const
      {
         std::string glsl = mLeft->toGLSL() + " " + toOperatorString() + " " + mRight->toGLSL();

         if (mOperator != MOV && (mLeft->operatorType() == Operator::BINARY || mRight->operatorType() == Operator::BINARY))
            return std::string("(") + glsl + ")";
         else
            return glsl;
      }

      bOperator op() const
      {
         return mOperator;
      }

      Operator* left() const
      {
         return mLeft;
      }

      Operator* right() const
      {
         return mRight;
      }

   protected:
      std::string toOperatorString() const
      {
         static const char* names[] = {
            "+",
            "-",
            "*",
            "/",
            "="
         };

         return names[mOperator];
      }

   protected:
      bOperator mOperator;
      Operator* mLeft;
      Operator* mRight;
   };
}
