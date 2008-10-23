// Copyright 2008, Google Inc.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. Neither the name of Google Inc. nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <algorithm>
#include "gears/base/common/js_marshal.h"
#include "gears/base/common/base_class.h"
#include "gears/base/common/js_types.h"
#include "third_party/scoped_ptr/scoped_ptr.h"


static void DeleteMarshaledJsTokens(
    std::map<std::string16, MarshaledJsToken*> *mjt_map) {
  for (std::map<std::string16, MarshaledJsToken*>::iterator i =
      mjt_map->begin(); i != mjt_map->end(); ++i) {
    delete i->second;
  }
}


static void DeleteMarshaledJsTokens(
    std::vector<MarshaledJsToken*> *mjt_vector) {
  for (std::vector<MarshaledJsToken*>::iterator i =
      mjt_vector->begin(); i != mjt_vector->end(); ++i) {
    delete *i;
  }
}


MarshaledJsToken::MarshaledJsToken()
    : type_(JSPARAM_UNKNOWN) {
}


MarshaledJsToken::~MarshaledJsToken() {
  switch (type_) {
    case JSPARAM_STRING16:
      delete value_.string_value;
      break;
    case JSPARAM_OBJECT:
      DeleteMarshaledJsTokens(value_.object_value);
      delete value_.object_value;
      break;
    case JSPARAM_ARRAY:
      DeleteMarshaledJsTokens(value_.array_value);
      delete value_.array_value;
      break;
    case JSPARAM_MODULE:
      delete value_.marshaled_module_value;
      break;
    default:
      // do nothing
      break;
  }
}


// static
MarshaledJsToken *MarshaledJsToken::Marshal(
    const JsToken &token,
    JsRunnerInterface *js_runner,
    JsContextPtr js_context,
    std::string16 *error_message_out) {
  JsTokenVector object_stack;  // storage used to detect cycles
  return MarshaledJsToken::Marshal(
      token, js_runner, js_context, error_message_out, &object_stack);
}


// Helper to detect when a object or array being marshaled contains a cycle.
// While objects/arrays are being marshaled, their token is pushed on a stack,
// and popped off upon completion of the object/array. Prior to marshalling
// nested objects/array, we look for them on that stack.
// static
bool MarshaledJsToken::CausesCycle(JsRunnerInterface *js_runner,
                                   const JsToken &token,
                                   JsTokenVector *object_stack,
                                   std::string16 *error_message_out) {
  JsTokenVector::iterator found = std::find_if(
                              object_stack->begin(),
                              object_stack->end(),
                              std::bind2nd(JsTokenEqualTo(js_runner), token));
  if (found != object_stack->end()) {
    *error_message_out = STRING16(
        L"Cannot marshal an object that contains a cycle.");
    return true;
  }
  return false;
}


// static
MarshaledJsToken *MarshaledJsToken::Marshal(
    const JsToken &token,
    JsRunnerInterface *js_runner,
    JsContextPtr js_context,
    std::string16 *error_message_out,
    JsTokenVector *object_stack) {
  scoped_ptr<MarshaledJsToken> mjt;

  switch (JsTokenGetType(token, js_context)) {
    case JSPARAM_BOOL: {
      bool value;
      if (JsTokenToBool_NoCoerce(token, js_context, &value)) {
        mjt.reset(new MarshaledJsToken());
        mjt->type_ = JSPARAM_BOOL;
        mjt->value_.bool_value = value;
      }
      break;
    }
    case JSPARAM_INT: {
      int value;
      if (JsTokenToInt_NoCoerce(token, js_context, &value)) {
        mjt.reset(new MarshaledJsToken());
        mjt->type_ = JSPARAM_INT;
        mjt->value_.int_value = value;
      }
      break;
    }
    case JSPARAM_DOUBLE: {
      double value;
      if (JsTokenToDouble_NoCoerce(token, js_context, &value)) {
        mjt.reset(new MarshaledJsToken());
        mjt->type_ = JSPARAM_DOUBLE;
        mjt->value_.double_value = value;
      }
      break;
    }
    case JSPARAM_STRING16: {
      std::string16 value;
      if (JsTokenToString_NoCoerce(token, js_context, &value)) {
        mjt.reset(new MarshaledJsToken());
        mjt->type_ = JSPARAM_STRING16;
        mjt->value_.string_value = new std::string16(value);
      }
      break;
    }
    case JSPARAM_OBJECT: {
      // Check to see if our JavaScript object is acutally a Gears module.
      ModuleImplBaseClass *object_as_module = NULL;
      if (JsTokenToModule(js_runner, js_context, token, &object_as_module)) {
        MarshaledModule *marshaled_module =
            static_cast<ModuleImplBaseClass*>(object_as_module)->
            AsMarshaledModule();
        if (marshaled_module) {
          mjt.reset(new MarshaledJsToken());
          mjt->type_ = JSPARAM_MODULE;
          mjt->value_.marshaled_module_value = marshaled_module;
        } else {
          *error_message_out = STRING16(
              L"Cannot marshal arbitrary Gears modules.");
        }
      } else {
        // else it's a regular JavaScript object (that isn't a Gears module).
        if (!CausesCycle(js_runner, token, object_stack, error_message_out)) {
          object_stack->push_back(token);
          JsObject value;
          if (value.SetObject(token, js_context)) {
            mjt.reset(new MarshaledJsToken());
            if (!mjt->InitializeFromObject(value, js_runner, js_context,
                                           error_message_out, object_stack)) {
              mjt.reset(NULL);
            }
          }
          object_stack->pop_back();
        }
      }
      break;
    }
    case JSPARAM_ARRAY: {
      if (!CausesCycle(js_runner, token, object_stack, error_message_out)) {
        object_stack->push_back(token);
        JsArray value;
        if (value.SetArray(token, js_context)) {
          mjt.reset(new MarshaledJsToken());
          if (!mjt->InitializeFromArray(value, js_runner, js_context,
                                        error_message_out, object_stack)) {
            mjt.reset(NULL);
          }
        }
        object_stack->pop_back();
      }
      break;
    }
    case JSPARAM_FUNCTION: {
      *error_message_out = STRING16(L"Cannot marshal a JavaScript function.");
      break;
    }
    case JSPARAM_NULL: {
      mjt.reset(new MarshaledJsToken());
      mjt->type_ = JSPARAM_NULL;
      break;
    }
    case JSPARAM_UNDEFINED: {
      mjt.reset(new MarshaledJsToken());
      mjt->type_ = JSPARAM_UNDEFINED;
      break;
    }
    default: {
      *error_message_out = STRING16(L"Cannot marshal an arbitrary token.");
      break;
    }
  }
  return mjt.release();
}


bool MarshaledJsToken::Unmarshal(
    ModuleEnvironment *module_environment,
    JsScopedToken *out) {
  JsRunnerInterface *js_runner = module_environment->js_runner_;
  bool success = false;
  switch (type_) {
    case JSPARAM_BOOL: {
      success = BoolToJsToken(
          js_runner->GetContext(), value_.bool_value, out);
      break;
    }
    case JSPARAM_INT: {
      success = IntToJsToken(
          js_runner->GetContext(), value_.int_value, out);
      break;
    }
    case JSPARAM_DOUBLE: {
      success = DoubleToJsToken(
          js_runner->GetContext(), value_.double_value, out);
      break;
    }
    case JSPARAM_STRING16: {
      success = StringToJsToken(
          js_runner->GetContext(), value_.string_value->c_str(), out);
      break;
    }
    case JSPARAM_OBJECT: {
      scoped_ptr<JsObject> object(js_runner->NewObject());
      *out = object->token();

      std::map<std::string16, MarshaledJsToken*> *o = value_.object_value;
      for (std::map<std::string16, MarshaledJsToken*>::iterator i =
          o->begin(); i != o->end(); ++i) {
        JsScopedToken property_value;
        if (!i->second->Unmarshal(module_environment, &property_value)) {
          return false;
        }
        object->SetProperty(i->first, property_value);
      }
      success = true;
      break;
    }
    case JSPARAM_ARRAY: {
      scoped_ptr<JsArray> array(js_runner->NewArray());
      *out = array->token();

      std::vector<MarshaledJsToken*> *a = value_.array_value;
      int n = a->size();
      for (int i = 0; i < n; i++) {
        JsScopedToken token;
        MarshaledJsToken *mjt = (*a)[i];
        if (mjt && mjt->Unmarshal(module_environment, &token)) {
          array->SetElement(i, token);
        }
      }
      success = true;
      break;
    }
    case JSPARAM_MODULE: {
      success =
          value_.marshaled_module_value->Unmarshal(module_environment, out);
      break;
    }
    case JSPARAM_NULL: {
      success = NullToJsToken(js_runner->GetContext(), out);
      break;
    }
    case JSPARAM_UNDEFINED: {
      success = UndefinedToJsToken(js_runner->GetContext(), out);
      break;
    }
    case JSPARAM_UNKNOWN: {
      success = false;
      break;
    }
    default: {
      assert(false);
      break;
    }
  }
  return success;
}


bool MarshaledJsToken::InitializeFromObject(
    JsObject &js_object,
    JsRunnerInterface *js_runner,
    JsContextPtr js_context,
    std::string16 *error_message_out,
    JsTokenVector *object_stack) {
  std::vector<std::string16> property_names;
  if (!js_object.GetPropertyNames(&property_names)) {
    return false;
  }

  scoped_ptr<std::map<std::string16, MarshaledJsToken*> > o(
      new std::map<std::string16, MarshaledJsToken*>);
  for (std::vector<std::string16>::iterator i = property_names.begin();
      i != property_names.end(); ++i) {
    JsScopedToken property_value;
    if (!js_object.GetProperty(*i, &property_value)) {
      DeleteMarshaledJsTokens(o.get());
      return false;
    }
    MarshaledJsToken *marshaled_pv = Marshal(
        property_value, js_runner, js_context, error_message_out, object_stack);
    if (!marshaled_pv) {
      DeleteMarshaledJsTokens(o.get());
      return false;
    }
    (*o)[*i] = marshaled_pv;
  }

  type_ = JSPARAM_OBJECT;
  value_.object_value = o.release();
  return true;
}


bool MarshaledJsToken::InitializeFromArray(
    JsArray &js_array,
    JsRunnerInterface *js_runner,
    JsContextPtr js_context,
    std::string16 *error_message_out,
    JsTokenVector *object_stack) {
  int n;
  if (!js_array.GetLength(&n)) {
    return false;
  }

  scoped_ptr<std::vector<MarshaledJsToken*> > a(
      new std::vector<MarshaledJsToken*>);
  for (int i = 0; i < n; i++) {
    JsScopedToken token;
    if (js_array.GetElement(i, &token)) {
      MarshaledJsToken *element_mjt = Marshal(
          token, js_runner, js_context, error_message_out, object_stack);
      if (element_mjt) {
        a->push_back(element_mjt);
      } else {
        DeleteMarshaledJsTokens(a.get());
        return false;
      }
    } else {
      a->push_back(NULL);
    }
  }

  type_ = JSPARAM_ARRAY;
  value_.array_value = a.release();
  return true;
}
