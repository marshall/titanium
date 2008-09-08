// Copyright (C) 2006 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// postfix_evaluator.h: Postfix (reverse Polish) notation expression evaluator.
//
// PostfixEvaluator evaluates an expression, using the expression itself
// in postfix (reverse Polish) notation and a dictionary mapping constants
// and variables to their values.  The evaluator supports standard
// arithmetic operations, assignment into variables, and when an optional
// MemoryRange is provided, dereferencing.  (Any unary key-to-value operation
// may be used with a MemoryRange implementation that returns the appropriate
// values, but PostfixEvaluator was written with dereferencing in mind.)
//
// The expression language is simple.  Expressions are supplied as strings,
// with operands and operators delimited by whitespace.  Operands may be
// either literal values suitable for ValueType, or constants or variables,
// which reference the dictionary.  The supported binary operators are +
// (addition), - (subtraction), * (multiplication), / (quotient of division),
// and % (modulus of division).  The unary ^ (dereference) operator is also
// provided.  These operators allow any operand to be either a literal
// value, constant, or variable.  Assignment (=) of any type of operand into
// a variable is also supported.
//
// The dictionary is provided as a map with string keys.  Keys beginning
// with the '$' character are treated as variables.  All other keys are
// treated as constants.  Any results must be assigned into variables in the
// dictionary.  These variables do not need to exist prior to calling
// Evaluate, unless used in an expression prior to being assigned to.  The
// internal stack state is not made available after evaluation, and any
// values remaining on the stack are treated as evidence of incomplete
// execution and cause the evaluator to indicate failure.
//
// PostfixEvaluator is intended to support evaluation of "program strings"
// obtained from MSVC frame data debugging information in pdb files as
// returned by the DIA APIs.
//
// Author: Mark Mentovai

#ifndef PROCESSOR_POSTFIX_EVALUATOR_H__
#define PROCESSOR_POSTFIX_EVALUATOR_H__


#include <map>
#include <string>
#include <vector>

namespace google_breakpad {

using std::map;
using std::string;
using std::vector;

class MemoryRegion;

template<typename ValueType>
class PostfixEvaluator {
 public:
  typedef map<string, ValueType> DictionaryType;
  typedef map<string, bool> DictionaryValidityType;

  // Create a PostfixEvaluator object that may be used (with Evaluate) on
  // one or more expressions.  PostfixEvaluator does not take ownership of
  // either argument.  |memory| may be NULL, in which case dereferencing
  // (^) will not be supported.  |dictionary| may be NULL, but evaluation
  // will fail in that case unless set_dictionary is used before calling
  // Evaluate.
  PostfixEvaluator(DictionaryType *dictionary, MemoryRegion *memory)
      : dictionary_(dictionary), memory_(memory), stack_() {}

  // Evaluate the expression.  The results of execution will be stored
  // in one (or more) variables in the dictionary.  Returns false if any
  // failures occure during execution, leaving variables in the dictionary
  // in an indeterminate state.  If assigned is non-NULL, any keys set in
  // the dictionary as a result of evaluation will also be set to true in
  // assigned, providing a way to determine if an expression modifies any
  // of its input variables.
  bool Evaluate(const string &expression, DictionaryValidityType *assigned);

  DictionaryType* dictionary() const { return dictionary_; }

  // Reset the dictionary.  PostfixEvaluator does not take ownership.
  void set_dictionary(DictionaryType *dictionary) {dictionary_ = dictionary; }

 private:
  // Return values for PopValueOrIdentifier
  enum PopResult {
    POP_RESULT_FAIL = 0,
    POP_RESULT_VALUE,
    POP_RESULT_IDENTIFIER
  };

  // Retrieves the topmost literal value, constant, or variable from the
  // stack.  Returns POP_RESULT_VALUE if the topmost entry is a literal
  // value, and sets |value| accordingly.  Returns POP_RESULT_IDENTIFIER
  // if the topmost entry is a constant or variable identifier, and sets
  // |identifier| accordingly.  Returns POP_RESULT_FAIL on failure, such
  // as when the stack is empty.
  PopResult PopValueOrIdentifier(ValueType *value, string *identifier);

  // Retrieves the topmost value on the stack.  If the topmost entry is
  // an identifier, the dictionary is queried for the identifier's value.
  // Returns false on failure, such as when the stack is empty or when
  // a nonexistent identifier is named.
  bool PopValue(ValueType *value);

  // Retrieves the top two values on the stack, in the style of PopValue.
  // value2 is popped before value1, so that value1 corresponds to the
  // entry that was pushed prior to value2.  Returns false on failure.
  bool PopValues(ValueType *value1, ValueType *value2);

  // Pushes a new value onto the stack.
  void PushValue(const ValueType &value);

  // The dictionary mapping constant and variable identifiers (strings) to
  // values.  Keys beginning with '$' are treated as variable names, and
  // PostfixEvaluator is free to create and modify these keys.  Weak pointer.
  DictionaryType *dictionary_;

  // If non-NULL, the MemoryRegion used for dereference (^) operations.
  // If NULL, dereferencing is unsupported and will fail.  Weak pointer.
  MemoryRegion *memory_;

  // The stack contains state information as execution progresses.  Values
  // are pushed on to it as the expression string is read and as operations
  // yield values; values are popped when used as operands to operators.
  vector<string> stack_;
};

}  // namespace google_breakpad


#endif  // PROCESSOR_POSTFIX_EVALUATOR_H__
