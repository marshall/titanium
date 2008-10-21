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

#ifdef GLINT_ENABLE_XML

#include <string>

#include "glint/include/type_system.h"
#include "glint/crossplatform/core_util.h"

#include "glint/include/column.h"
#include "glint/include/button.h"
#include "glint/include/image_node.h"
#include "glint/include/formatted_text.h"
#include "glint/include/nine_grid.h"
#include "glint/include/root_ui.h"
#include "glint/include/row.h"
#include "glint/include/simple_text.h"

namespace glint {

#define TYPE_SYSTEM_STRICT_CHECK

static const char* glint_namespace = "http://www.google.com/glint";

static TypeInfo type_info[] = {
  { glint_namespace,         // namespace_uri
    "RootUI",                // Type name
    glint_namespace,         // Base Type namespace
    "BaseObject",            // Base Type name
    "child",                 // Default property name
    RootUI::CreateInstance,  // Creates default-initialized instance
    NULL,                    // Creates instance from string value, if exists
  },

  { glint_namespace,
    "Node",
    glint_namespace,
    "BaseObject",
    "child",
    Node::CreateInstance,
    NULL,
  },

  { glint_namespace,
    "ImageNode",
    glint_namespace,
    "Node",
    NULL,
    ImageNode::CreateInstance,
    NULL,
  },

  { glint_namespace,
    "SimpleText",
    glint_namespace,
    "Node",
    NULL,
    SimpleText::CreateInstance,
    NULL,
  },

  { glint_namespace,
    "FormattedText",
    glint_namespace,
    "Node",
    NULL,
    FormattedText::CreateInstance,
    NULL,
  },

  { glint_namespace,
    "Row",
    glint_namespace,
    "Node",
    NULL,
    Row::CreateInstance,
    NULL,
  },

  { glint_namespace,
    "Column",
    glint_namespace,
    "Node",
    NULL,
    Column::CreateInstance,
    NULL,
  },

  { glint_namespace,
    "NineGrid",
    glint_namespace,
    "Node",
    NULL,
    NineGrid::CreateInstance,
    NULL,
  },

  { glint_namespace,
    "Button",
    glint_namespace,
    "NineGrid",
    NULL,
    Button::CreateInstance,
    NULL,
  },
};

// Shortcut to declare a property of SimpleText with a
// "set from string" function.
#define PROP_STR(element_type, property_name, set_from_string_function) \
{ property_name,      \
    glint_namespace,  \
      element_type,   \
        NULL,         \
          false,      \
            NULL,     \
              set_from_string_function },

// If the type of property is 'NULL', that means the parser should not
// parse it but rather pass as a string to the property setter function.
// This is a usual case for int- bool- Rect- and other "non-structured"
// types.
static PropertyInfo property_info[] = {
//  name
//      owner's namespace
//         owner's name
//             type
//                 value_is_uri
//                      setter function ptr
//                          setter from string function ptr
  { "child",
        glint_namespace,
            "Node",
                "Node",
                    false,
                        Node::SetChild,
                            NULL },
  { "child",
        glint_namespace,
            "RootUI",
                "Node",
                    false,
                        RootUI::SetChild,
                            NULL },

  { "source",
        glint_namespace,
            "ImageNode",
                NULL,
                    true,
                        NULL,
                            ImageNode::SetSource },
  { "source",
        glint_namespace,
            "NineGrid",
                NULL,
                    true,
                        NULL,
                            NineGrid::SetSource },

  PROP_STR("Node", "alpha",                Node::SetAlpha)
  PROP_STR("Node", "transform",            Node::SetTransform)
  PROP_STR("Node", "horizontal_alignment", Node::SetHorizontalAlignment)
  PROP_STR("Node", "vertical_alignment",   Node::SetVerticalAlignment)
  PROP_STR("Node", "margin",               Node::SetMargin)
  PROP_STR("Node", "background",           Node::SetBackground)
  PROP_STR("Node", "min_width",            Node::SetMinWidth)
  PROP_STR("Node", "max_width",            Node::SetMaxWidth)
  PROP_STR("Node", "min_height",           Node::SetMinHeight)
  PROP_STR("Node", "max_height",           Node::SetMaxHeight)
  PROP_STR("Node", "ignore_clip",          Node::SetIgnoreClip)
  PROP_STR("Node", "id",                   Node::SetId)
  PROP_STR("SimpleText", "text",        SimpleText::SetText)
  PROP_STR("SimpleText", "font_family", SimpleText::SetFontFamily)
  PROP_STR("SimpleText", "font_size",   SimpleText::SetFontSize)
  PROP_STR("SimpleText", "foreground",  SimpleText::SetForeground)
  PROP_STR("SimpleText", "bold",        SimpleText::SetBold)
  PROP_STR("SimpleText", "italic",      SimpleText::SetItalic)
  PROP_STR("FormattedText", "markup",     FormattedText::SetMarkup)
  PROP_STR("Row", "direction",    Row::SetDirection)
  PROP_STR("Row", "distribution", Row::SetDistribution)
  PROP_STR("NineGrid", "shadow", NineGrid::SetShadow)
  PROP_STR("NineGrid", "center_width", NineGrid::SetCenterWidth)
  PROP_STR("NineGrid", "center_height", NineGrid::SetCenterHeight)
  PROP_STR("Button", "enabled", Button::SetEnabled)
};

TypeSystem* TypeSystem::instance_ = NULL;

TypeSystem* TypeSystem::GetInstance() {
  if (!instance_) {
    instance_ = new TypeSystem();
    for (size_t i = 0; i < sizeof(type_info) / sizeof(type_info[0]); ++i) {
      instance_->AddTypeInfo(type_info[i]);
    }
    for (size_t i = 0;
         i < sizeof(property_info) / sizeof(property_info[0]);
         ++i) {
      instance_->AddPropertyInfo(property_info[i]);
    }
  }
  return instance_;
}

bool TypeSystem::AddTypeInfo(const TypeInfo& info) {
  TypeDescriptor *descriptor = new TypeDescriptor(info);
  if (types_.find(descriptor->name()) != types_.end())
    return false;
  types_.insert(make_pair(descriptor->name(), descriptor));
  return true;
}

bool TypeSystem::AddPropertyInfo(const PropertyInfo& info) {
  PropertyDescriptor *descriptor = new PropertyDescriptor(info);
  TypeDescriptor* owner = descriptor->owner();
  if (!owner) {
#ifdef TYPE_SYSTEM_STRICT_CHECK
    platform()->CrashWithMessage(
        "TypeSystem initialization: Adding property %s to object %s\n",
        info.name,
        info.owner_name);
#endif
    return false;
  }
  return owner->AddProperty(descriptor);
}

TypeDescriptor* TypeSystem::FindType(const std::string& name) {
  TypeMap::const_iterator loc = types_.find(name);
  if (loc == types_.end())
    return NULL;
  return loc->second;
}

static std::string BuildUniversalName(const char* namespace_uri,
                                      const char* name) {
  return std::string(namespace_uri) + "#" + name;
}

PropertyDescriptor::PropertyDescriptor(const PropertyInfo& info) {
  ASSERT(info.name && info.owner_name && info.owner_namespace_uri);
  name_ = info.name;
  owner_ = TypeDescriptor::FromName(
      BuildUniversalName(info.owner_namespace_uri, info.owner_name));
  if (info.type_name) {
    type_name_ = BuildUniversalName(info.owner_namespace_uri, info.type_name);
  }
  type_ = NULL;
  value_is_uri_ = info.value_is_uri;
  set_property_function_ = info.set_property_function_;
  set_from_string_function_ = info.set_from_string_function_;
}

TypeDescriptor* PropertyDescriptor::type() {
  if (!type_) {
    type_ = TypeDescriptor::FromName(type_name_);
  }
  return type_;
}

SetPropertyResult PropertyDescriptor::SetProperty(BaseObject *owner,
                                                  BaseObject *value) const {
  return set_property_function_ ? set_property_function_(owner, value) :
                                  PROPERTY_NOT_SET;
}

SetPropertyResult PropertyDescriptor::SetPropertyFromString(
    BaseObject *owner,
    const std::string& value) const {
  return set_from_string_function_ ? set_from_string_function_(owner, value) :
                                     PROPERTY_NOT_SET;
}

TypeDescriptor::TypeDescriptor(const TypeInfo& info) {
  ASSERT(info.name && info.namespace_uri);
  name_ = BuildUniversalName(info.namespace_uri, info.name);
  if (info.base_type_name) {
    base_type_name_ = BuildUniversalName(info.base_type_namespace_uri,
                                         info.base_type_name);
  }
  if (info.default_property_type_name) {
    default_property_name_ = info.default_property_type_name;
  }
  base_type_ = NULL;
  default_property_ = NULL;
  create_function_ = info.create_instance;
  create_from_string_function_ = info.create_from_string;
}

TypeDescriptor* TypeDescriptor::base_type() {
  if (!base_type_) {
    base_type_ = TypeDescriptor::FromName(base_type_name_);
  }
  return base_type_;
}

PropertyDescriptor* TypeDescriptor::default_property() {
  if (!default_property_) {
    TypeDescriptor* type = this;
    while (type) {
      default_property_ = type->FindProperty(type->default_property_name_);
      if (default_property_)
        break;
      type = type->base_type();
    }
  }
  return default_property_;
}

BaseObject* TypeDescriptor::CreateInstance() const {
  return create_function_ ? create_function_() : NULL;
}

BaseObject* TypeDescriptor::CreateFromString(const std::string& value) const {
  return create_from_string_function_ ? create_from_string_function_(value) :
                                        NULL;
}

bool TypeDescriptor::AddProperty(PropertyDescriptor* property) {
  if (properties_.find(property->name()) != properties_.end())
    return false;
  properties_.insert(make_pair(property->name(), property));
  return true;
}

PropertyDescriptor* TypeDescriptor::FindProperty(
    const std::string& name) const {
      PropertyMap::const_iterator i = properties_.find(name);
      return i == properties_.end() ? NULL : i->second;
}

TypeDescriptor* TypeDescriptor::FromName(const std::string& name) {
  TypeSystem* types = TypeSystem::GetInstance();
  return types ? types->FindType(name) : NULL;
}

bool TypeDescriptor::IsOfType(TypeDescriptor* another) {
  TypeDescriptor* type = this;
  while (type) {
    // This is intended to be a pointer equality check - types are
    // unique instances, retrieving a type twice for the same type yields
    // the same pointer.
    if (type == another)
      return true;
    type = type->base_type();
  }
  return false;
}


}  // namespace glint

#endif  // GLINT_ENABLE_XML
