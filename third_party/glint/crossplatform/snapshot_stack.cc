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

#include <string>
#include <sstream>

#include "glint/include/snapshot_stack.h"
#include "glint/crossplatform/core_util.h"

namespace glint {

SnapshotContext::SnapshotContext(SnapshotStack* stack,
                                 const std::string& tag_name,
                                 const Transform& transform)
  : stack_(stack),
    tag_name_(tag_name) {
  ASSERT(stack);
  ASSERT(!tag_name.empty());
  transform_.Set(transform);
}

void SnapshotContext::AddAttribute(const std::string& name,
                                   const std::string& value) {
  std::string attribute = " " + name + "=\"" + value + "\"";
  stack_->AppendXmlString(attribute);
}

void SnapshotContext::AddAttribute(const std::string& name, int value) {
  // Convert int value into string.
  std::ostringstream string_stream;
  string_stream << value;
  std::string string_value = string_stream.str();
  AddAttribute(name, string_value);
}

void SnapshotStack::Push(const Transform &local_transform) {
  Push(local_transform, std::string());
}

void SnapshotStack::Push(const Transform &local_transform,
                         const std::string& tag_name) {
  Transform transform;
  // Compute transform from local to screen coordinates based on parent's
  // such transform and transform from local to parent coordinates.
  if (array_.length() > 0) {
    transform.Set(Top()->transform());
    transform.AddPreTransform(local_transform);
  } else {
    transform.Set(local_transform);
  }

  if (array_.length() > 0) {
    xml_ += ">\n";
  }

  array_.Add(new SnapshotContext(this, tag_name, transform));

  if (!tag_name.empty()) {
    xml_ += "<" + tag_name;
  }
}

void SnapshotStack::Pop() {
  ASSERT(array_.length() > 1);
  std::string tag_name = Top()->tag_name();
  if (!tag_name.empty()) {
    xml_ += "</" + tag_name + ">\n";
  }
  array_.RemoveAt(array_.length() - 1);
}

SnapshotContext *SnapshotStack::Top() {
  ASSERT(array_.length() > 0);
  return array_[array_.length() - 1];
}

void SnapshotStack::AppendXmlString(const std::string& tail) {
  xml_ += tail;
}

}  // namespace glint
