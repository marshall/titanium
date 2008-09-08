// Copyright (c) 2006, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// postfix_evaluator-inl.h: Postfix (reverse Polish) notation expression
// evaluator.
//
// Documentation in postfix_evaluator.h.
//
// Author: Mark Mentovai

#ifndef PROCESSOR_POSTFIX_EVALUATOR_INL_H__
#define PROCESSOR_POSTFIX_EVALUATOR_INL_H__


#include <sstream>

#include "processor/postfix_evaluator.h"
#include "google_breakpad/processor/memory_region.h"
#include "processor/logging.h"

namespace google_breakpad {

using std::istringstream;
using std::ostringstream;


// A small class used in Evaluate to make sure to clean up the stack
// before returning failure.
class AutoStackClearer {
 public:
  explicit AutoStackClearer(vector<string> *stack) : stack_(stack) {}
  ~AutoStackClearer() { stack_->clear(); }

 private:
  vector<string> *stack_;
};


template<typename ValueType>
bool PostfixEvaluator<ValueType>::Evaluate(const string &expression,
                                           DictionaryValidityType *assigned) {
  // Ensure that the stack is cleared before returning.
  AutoStackClearer clearer(&stack_);

  // Tokenize, splitting on whitespace.
  istringstream stream(expression);
  string token;
  while (stream >> token) {
    // There are enough binary operations that do exactly the same thing
    // (other than the specific operation, of course) that it makes sense
    // to share as much code as possible.
    enum BinaryOperation {
      BINARY_OP_NONE = 0,
      BINARY_OP_ADD,
      BINARY_OP_SUBTRACT,
      BINARY_OP_MULTIPLY,
      BINARY_OP_DIVIDE_QUOTIENT,
      BINARY_OP_DIVIDE_MODULUS
    };

    BinaryOperation operation = BINARY_OP_NONE;
    if (token == "+")
      operation = BINARY_OP_ADD;
    else if (token == "-")
      operation = BINARY_OP_SUBTRACT;
    else if (token == "*")
      operation = BINARY_OP_MULTIPLY;
    else if (token == "/")
      operation = BINARY_OP_DIVIDE_QUOTIENT;
    else if (token == "%")
      operation = BINARY_OP_DIVIDE_MODULUS;

    if (operation != BINARY_OP_NONE) {
      // Get the operands.
      ValueType operand1, operand2;
      if (!PopValues(&operand1, &operand2)) {
        BPLOG(ERROR) << "Could not PopValues to get two values for binary "
                        "operation " << token << ": " << expression;
        return false;
      }

      // Perform the operation.
      ValueType result;
      switch (operation) {
        case BINARY_OP_ADD:
          result = operand1 + operand2;
          break;
        case BINARY_OP_SUBTRACT:
          result = operand1 - operand2;
          break;
        case BINARY_OP_MULTIPLY:
          result = operand1 * operand2;
          break;
        case BINARY_OP_DIVIDE_QUOTIENT:
          result = operand1 / operand2;
          break;
        case BINARY_OP_DIVIDE_MODULUS:
          result = operand1 % operand2;
          break;
        case BINARY_OP_NONE:
          // This will not happen, but compilers will want a default or
          // BINARY_OP_NONE case.
          BPLOG(ERROR) << "Not reached!";
          return false;
          break;
      }

      // Save the result.
      PushValue(result);
    } else if (token == "^") {
      // ^ for unary dereference.  Can't dereference without memory.
      if (!memory_) {
        BPLOG(ERROR) << "Attempt to dereference without memory: " <<
                        expression;
        return false;
      }

      ValueType address;
      if (!PopValue(&address)) {
        BPLOG(ERROR) << "Could not PopValue to get value to derefence: " <<
                        expression;
        return false;
      }

      ValueType value;
      if (!memory_->GetMemoryAtAddress(address, &value)) {
        BPLOG(ERROR) << "Could not dereference memory at address " <<
                        HexString(address) << ": " << expression;
        return false;
      }

      PushValue(value);
    } else if (token == "=") {
      // = for assignment.
      ValueType value;
      if (!PopValue(&value)) {
        BPLOG(ERROR) << "Could not PopValue to get value to assign: " <<
                        expression;
        return false;
      }

      // Assignment is only meaningful when assigning into an identifier.
      // The identifier must name a variable, not a constant.  Variables
      // begin with '$'.
      string identifier;
      if (PopValueOrIdentifier(NULL, &identifier) != POP_RESULT_IDENTIFIER) {
        BPLOG(ERROR) << "PopValueOrIdentifier returned a value, but an "
                        "identifier is needed to assign " <<
                        HexString(value) << ": " << expression;
        return false;
      }
      if (identifier.empty() || identifier[0] != '$') {
        BPLOG(ERROR) << "Can't assign " << HexString(value) << " to " <<
                        identifier << ": " << expression;
        return false;
      }

      (*dictionary_)[identifier] = value;
      if (assigned)
        (*assigned)[identifier] = true;
    } else {
      // The token is not an operator, it's a literal value or an identifier.
      // Push it onto the stack as-is.  Use push_back instead of PushValue
      // because PushValue pushes ValueType as a string, but token is already
      // a string.
      stack_.push_back(token);
    }
  }

  // If there's anything left on the stack, it indicates incomplete execution.
  // This is a failure case.  If the stack is empty, evalution was complete
  // and successful.
  BPLOG_IF(ERROR, !stack_.empty()) << "Incomplete execution: " << expression;
  return stack_.empty();
}


template<typename ValueType>
typename PostfixEvaluator<ValueType>::PopResult
PostfixEvaluator<ValueType>::PopValueOrIdentifier(
    ValueType *value, string *identifier) {
  // There needs to be at least one element on the stack to pop.
  if (!stack_.size())
    return POP_RESULT_FAIL;

  string token = stack_.back();
  stack_.pop_back();

  // First, try to treat the value as a literal.  In order for this to
  // succed, the entire string must be parseable as ValueType.  If this
  // isn't possible, it can't be a literal, so treat it as an identifier
  // instead.
  istringstream token_stream(token);
  ValueType literal;
  if (token_stream >> literal && token_stream.peek() == EOF) {
    if (value) {
      *value = literal;
    }
    return POP_RESULT_VALUE;
  } else {
    if (identifier) {
      *identifier = token;
    }
    return POP_RESULT_IDENTIFIER;
  }
}


template<typename ValueType>
bool PostfixEvaluator<ValueType>::PopValue(ValueType *value) {
  ValueType literal;
  string token;
  PopResult result;
  if ((result = PopValueOrIdentifier(&literal, &token)) == POP_RESULT_FAIL) {
    return false;
  } else if (result == POP_RESULT_VALUE) {
    // This is the easy case.
    *value = literal;
  } else {  // result == POP_RESULT_IDENTIFIER
    // There was an identifier at the top of the stack.  Resolve it to a
    // value by looking it up in the dictionary.
    typename DictionaryType::const_iterator iterator =
        dictionary_->find(token);
    if (iterator == dictionary_->end()) {
      // The identifier wasn't found in the dictionary.  Don't imply any
      // default value, just fail.
      BPLOG(ERROR) << "Identifier " << token << " not in dictionary";
      return false;
    }

    *value = iterator->second;
  }

  return true;
}


template<typename ValueType>
bool PostfixEvaluator<ValueType>::PopValues(ValueType *value1,
                                            ValueType *value2) {
  return PopValue(value2) && PopValue(value1);
}


template<typename ValueType>
void PostfixEvaluator<ValueType>::PushValue(const ValueType &value) {
  ostringstream token_stream;
  token_stream << value;
  stack_.push_back(token_stream.str());
}


}  // namespace google_breakpad


#endif  // PROCESSOR_POSTFIX_EVALUATOR_INL_H__
