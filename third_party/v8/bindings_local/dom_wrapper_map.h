// Copyright 2007 Google Inc. All Rights Reserved.

#ifndef BINDINGS_V8_DOM_WRAPPER_MAP
#define BINDINGS_V8_DOM_WRAPPER_MAP

#include <hash_map>
#include "v8_helpers.h"
#include "gears/base/common/message_queue.h"

// A table of wrappers with weak pointers.
// This table allows us to avoid track wrapped objects for debugging
// and for ensuring that we don't double wrap the same object.
template<class KeyType, class ValueType>
class WeakReferenceMap {
 public:
  typedef std::pair<ValueType*, ThreadId> MapValueType;
  typedef stdext::hash_map<KeyType*, MapValueType> Map;

  WeakReferenceMap(v8::WeakReferenceCallback callback) :
       weak_reference_callback_(callback) { }

  // Get the JS wrapper object of an object.
  virtual v8::Persistent<ValueType> get(KeyType* obj) {
    Map::iterator it = map_.find(obj);
    return (it != map_.end()) ? v8::Persistent<ValueType>(it->second.first)
      : v8::Persistent<ValueType>();
  }

  virtual void set(KeyType* obj, v8::Persistent<ValueType> wrapper) {
    ASSERT(!contains(obj));
    wrapper.MakeWeak(obj, weak_reference_callback_);
    map_[obj] = MapValueType(
        *wrapper, ThreadMessageQueue::GetInstance()->GetCurrentThreadId());
  }

  virtual ThreadId forget(KeyType* obj) {
    ASSERT(obj);
    Map::iterator it = map_.find(obj);
    if (it != map_.end()) {
      ValueType* wrapper = it->second.first;
      ThreadId thread_id = it->second.second;
      v8::Persistent<ValueType> handle(wrapper);
      handle.Dispose();
      handle.Clear();
      map_.erase(it);
      return thread_id;
    }
    return ThreadId();
  }

  bool contains(KeyType* obj) {
    return map_.find(obj) != map_.end();
  }

  Map& impl() {
    return map_;
  }

 protected:
  Map map_;
  v8::WeakReferenceCallback weak_reference_callback_;
};


template <class KeyType>
class DOMWrapperMap : public WeakReferenceMap<KeyType, v8::Object> {
 public:
  DOMWrapperMap(v8::WeakReferenceCallback callback) :
       WeakReferenceMap(callback) { }
};

#endif  // BINDINGS_V8_DOM_WRAPPER_MAP
