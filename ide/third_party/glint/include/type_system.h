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

// Type System is a declarative description of object types and their
// properties. Type System is needed in Glint to be able to implement
// XML parser from extensible description language - so that 3rd party and
// Glint itself coudl easily introduce namespaces, object types and
// their properties into Glint XML language.
//
// See also xml_parser.h for more information about Glint XML language.

#ifndef GLINT_INCLUDE_TYPE_SYSTEM_H__
#define GLINT_INCLUDE_TYPE_SYSTEM_H__

#ifdef GLINT_ENABLE_XML

#include <map>
#include "glint/include/base_object.h"
#include "glint/include/types.h"

namespace glint {

enum SetPropertyResult {
  PROPERTY_OK,
  PROPERTY_NOT_SUPPORTED_ON_OBJECT,
  PROPERTY_HAS_INCORRECT_FORMAT,
  PROPERTY_BAD_VALUE,
  PROPERTY_NOT_SET,
};

typedef BaseObject* (*CreateInstanceFunction)();
typedef BaseObject* (*CreateFromStringFunction)(const std::string& value);
// NOLINT is here to prevent lint from complaining that SetPropertyResult
// function has extra space before '(' - but it is not a function.
typedef SetPropertyResult (*SetPropertyFunction)(BaseObject* owner,  // NOLINT
                                                 BaseObject* value);
typedef SetPropertyResult (*SetPropertyFromStringFunction)(          // NOLINT
    BaseObject* owner,
    const std::string& value);

class PropertyDescriptor;
class TypeDescriptor;

typedef std::map<std::string, PropertyDescriptor*> PropertyMap;
typedef std::map<std::string, TypeDescriptor*> TypeMap;

struct TypeInfo {
  const char* namespace_uri;
  const char* name;
  const char* base_type_namespace_uri;
  const char* base_type_name;
  const char* default_property_type_name;  // NULL if no default property
  CreateInstanceFunction create_instance;
  CreateFromStringFunction create_from_string;
};

// Note: properties do not have the namespace_uri, they beong to the same
// namespace as their owner objects.
struct PropertyInfo {
  const char* name;
  const char* owner_namespace_uri;
  const char* owner_name;
  const char* type_name;
  bool value_is_uri;
  SetPropertyFunction set_property_function_;
  SetPropertyFromStringFunction set_from_string_function_;
};

class TypeSystem : public BaseObject {
 public:
  // singleton
  static TypeSystem* GetInstance();
  bool AddTypeInfo(const TypeInfo &info);
  bool AddPropertyInfo(const PropertyInfo &info);
  TypeDescriptor* FindType(const std::string& name);
 private:
  TypeSystem() {}
  static TypeSystem* instance_;
  TypeMap types_;
  DISALLOW_EVIL_CONSTRUCTORS(TypeSystem);
};

class TypeDescriptor : public BaseObject {
 public:
  explicit TypeDescriptor(const TypeInfo& info);
  std::string name() const { return name_; }
  TypeDescriptor* base_type();
  PropertyDescriptor* default_property();
  BaseObject* CreateInstance() const;
  BaseObject* CreateFromString(const std::string& value) const;
  bool AddProperty(PropertyDescriptor* property);
  PropertyDescriptor* FindProperty(const std::string& name) const;
  // treats the name as a namespace_uri + '#' + type_name
  static TypeDescriptor* FromName(const std::string& name);
  bool IsOfType(TypeDescriptor* another);

 private:
  std::string name_;
  std::string base_type_name_;
  std::string default_property_name_;
  TypeDescriptor* base_type_;
  PropertyDescriptor* default_property_;
  CreateInstanceFunction create_function_;
  CreateFromStringFunction create_from_string_function_;
  PropertyMap properties_;
  DISALLOW_EVIL_CONSTRUCTORS(TypeDescriptor);
};

class PropertyDescriptor : public BaseObject {
 public:
  explicit PropertyDescriptor(const PropertyInfo& info);
  std::string name() const { return name_; }
  // owner is a type of object that has this property.
  TypeDescriptor* owner() const { return owner_; }
  // Note: In case when the property value type is not BaseObject-derived,
  // this returns NULL. That means the property is a simple one
  // (string or int or bool for example) and can only be set by SetProperty
  // variant that takes a string. Otherwise, the parser always builds a value
  // first and then uses SetProperty that has a BaseObject as a parameter.
  TypeDescriptor* type();
  bool value_is_uri() { return value_is_uri_; }
  SetPropertyResult SetProperty(BaseObject *owner, BaseObject *value) const;
  SetPropertyResult SetPropertyFromString(BaseObject *owner,
                                          const std::string& value) const;

 private:
  std::string name_;
  std::string type_name_;
  TypeDescriptor* owner_;
  TypeDescriptor* type_;
  SetPropertyFunction set_property_function_;
  SetPropertyFromStringFunction set_from_string_function_;
  bool value_is_uri_;
  DISALLOW_EVIL_CONSTRUCTORS(PropertyDescriptor);
};

}  // namespace glint

#endif  // GLINT_ENABLE_XML

#endif  // GLINT_INCLUDE_TYPE_SYSTEM_H__


