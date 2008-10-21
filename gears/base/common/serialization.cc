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

#include <assert.h>
#include "gears/base/common/serialization.h"
#include "gears/base/common/string_utils.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

std::map<SerializableClassId, SerializableFactoryMethod>
  Serializable::constructors_;

Mutex Serializable::constructors_lock_;  // Protects constructors_

void Serializable::RegisterClass(SerializableClassId class_id,
                                 SerializableFactoryMethod factory) {
  MutexLock lock(&constructors_lock_);
  constructors_[class_id] = factory;
}

Serializable *Serializable::CreateClass(SerializableClassId class_id) {
  std::map<SerializableClassId, SerializableFactoryMethod>::iterator
    constructor;
  {
    // We only need to hold the lock during the find.
    MutexLock lock(&constructors_lock_);
    constructor = constructors_.find(class_id);
  }
  if (constructor != constructors_.end()) {
    return constructor->second();
  } else {
    return NULL;
  }
}

void Serializer::WriteBool(const bool data) {
  uint8 data_byte = data ? 1 : 0;
  WriteBytes(&data_byte, sizeof(uint8));
}

void Serializer::WriteInt(const int data) {
  WriteBytes(&data, sizeof(int));
}

void Serializer::WriteInt64(const int64 data) {
  WriteBytes(&data, sizeof(int64));
}

void Serializer::WriteString(const char16 *data) {
  std::string utf8_string;
  if (!String16ToUTF8(data, &utf8_string)) {
    // If string conversion fails, insert an empty string.
    utf8_string = "";
  }

  WriteInt(utf8_string.size());
  WriteBytes(utf8_string.c_str(), utf8_string.size());
}

void Serializer::WriteBytes(const void *data, size_t length) {
  buffer_->insert(buffer_->end(), (uint8 *)data, ((uint8 *)data)+length);
}

bool Serializer::WriteObject(const Serializable *obj) {
  if (!obj) {
    WriteInt(SERIALIZABLE_NULL);
    WriteInt(0);
    return true;
  }

  SerializableClassId class_id = obj->GetSerializableClassId();
  if (class_id == SERIALIZABLE_NULL) {
    return false;
  }

  // Store the state in case we need to rewind.
  size_t save_beginning = buffer_->size();

  WriteInt(class_id);

  // Store the position of the size and insert a placeholder.
  size_t size_position = buffer_->size();
  WriteInt(0);

  size_t obj_data_position = buffer_->size();
  if (!obj->Serialize(this)) {
    // Rewind the buffer and fail.
    buffer_->resize(save_beginning);
    return false;
  }

  // Store the size of the data in the serialized buffer.
  int size = static_cast<int>(buffer_->size() - obj_data_position);
  *(reinterpret_cast<int*>(&buffer_->at(size_position))) = size;

  return true;
}

bool Deserializer::CreateAndReadObject(Serializable **out) {
  assert(out);

  int class_id;
  int size;

  if (!ReadInt(&class_id) || !ReadInt(&size)) {
    *out = NULL;
    return false;
  }

  if (class_id == SERIALIZABLE_NULL) {
    assert(size == 0);
    *out = NULL;
    return true;
  }

  // Store the object's data position, and advance the read position to the
  // next object.
  size_t object_data_position = read_pos_;
  read_pos_ += size;

  *out = Serializable::CreateClass(static_cast<SerializableClassId>(class_id));
  if (!*out) {
    return false;
  }

  // Create an embedded deserializer to prevent us from reading past the end of
  // the object.
  Deserializer deserializer(&buffer_[object_data_position], size);
  if ((*out)->Deserialize(&deserializer)) {
    return true;
  } else {
    delete *out;
    *out = NULL;
    return false;
  }
}

bool Deserializer::ReadBool(bool *output) {
  uint8 data;
  if (!ReadBytes(&data, sizeof(uint8))) return false;
  *output = data != 0;
  return true;
}

bool Deserializer::ReadInt(int *output) {
  return ReadBytes(output, sizeof(int));
}

bool Deserializer::ReadInt64(int64 *output) {
  return ReadBytes(output, sizeof(int64));
}

bool Deserializer::ReadString(std::string16 *output) {
  int size;
  if (!ReadInt(&size)) return false;

  scoped_ptr<char> data(new char[size + 1]);
  if (!ReadBytes(data.get(), size)) return false;

  // The serialized data is not NULL terminated.
  data.get()[size] = '\0';

  return UTF8ToString16(data.get(), output);
}

bool Deserializer::ReadBytes(void *output, size_t length) {
  if (read_pos_ + length > length_) return false;
  memcpy(output, &buffer_[read_pos_], length);
  read_pos_ += length;
  return true;
}
