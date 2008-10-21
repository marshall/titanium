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

// SnapshotStack is a stack of Transforms and elements.  When
// snapshotting, Glint walks the tree of Nodes and pushes for
// each Node a transform that maps from local coordinates to
// screen coordinates, as well as an element of the tree being
// snapshot.  For a node that is included in the snapshot, a
// corresponding new element is pushed.  For a node that is not
// included in the snapshot, the element for its nearest
// ancestor that is included is pushed instead.

#ifndef GLINT_INCLUDE_SNAPSHOT_STACK_H__
#define GLINT_INCLUDE_SNAPSHOT_STACK_H__

#include <string>
#include "glint/include/array.h"
#include "glint/include/base_object.h"
#include "glint/include/transform.h"

namespace glint {

class SnapshotStack;

class SnapshotContext : public BaseObject {
 public:
  SnapshotContext(SnapshotStack* stack,
                  const std::string& tag_name,
                  const Transform& transform);

  std::string tag_name() {
    return tag_name_;
  }

  const Transform& transform() {
    return transform_;
  }

  void AddAttribute(const std::string& name, const std::string& value);
  void AddAttribute(const std::string& name, int value);

 private:
  Transform transform_;
  SnapshotStack* stack_;
  std::string tag_name_;
  DISALLOW_EVIL_CONSTRUCTORS(SnapshotContext);
};

class SnapshotStack : public BaseObject {
 public:
  SnapshotStack() {
  }

  // Push transform without creating a new child element.
  void Push(const Transform& local_transform);

  // Push transform with a new element with the required name.
  void Push(const Transform& local_transform, const std::string& tagName);

  void Pop();

  SnapshotContext* Top();

  void AppendXmlString(const std::string& tail);

  std::string xml() { return xml_; };

 private:
  Array<SnapshotContext> array_;
  std::string xml_;
  DISALLOW_EVIL_CONSTRUCTORS(SnapshotStack);
};

}  // namespace glint

#endif  // GLINT_INCLUDE_SNAPSHOT_STACK_H__
