// Automatically generated from /cygdrive/c/Users/Marshall/Source/WEBKIT~1/WebCore/bindings/js/JSWorkerContext.cpp using /cygdrive/c/Users/Marshall/Build/WEBKIT~1/include/JavaScriptCore/create_hash_table. DO NOT EDIT!

namespace WebCore {

using namespace JSC;

static const struct HashTableValue JSWorkerContextPrototypeTableValues[5] = {
   { "addEventListener", DontDelete|Function, (intptr_t)jsWorkerContextPrototypeFunctionAddEventListener, (intptr_t)3 },
   { "removeEventListener", DontDelete|Function, (intptr_t)jsWorkerContextPrototypeFunctionRemoveEventListener, (intptr_t)3 },
   { "dispatchEvent", DontDelete|Function, (intptr_t)jsWorkerContextPrototypeFunctionDispatchEvent, (intptr_t)2 },
   { "postMessage", DontDelete|Function, (intptr_t)jsWorkerContextPrototypeFunctionPostMessage, (intptr_t)2 },
   { 0, 0, 0, 0 }
};

extern const struct HashTable JSWorkerContextPrototypeTable =
#if ENABLE(PERFECT_HASH_SIZE)
    { 7, JSWorkerContextPrototypeTableValues, 0 };
#else
    { 8, 7, JSWorkerContextPrototypeTableValues, 0 };
#endif

} // namespace

namespace WebCore {

using namespace JSC;

static const struct HashTableValue JSWorkerContextTableValues[6] = {
   { "location", DontDelete|ReadOnly, (intptr_t)jsWorkerContextLocation, (intptr_t)0 },
   { "navigator", DontDelete|ReadOnly, (intptr_t)jsWorkerContextNavigator, (intptr_t)0 },
   { "onmessage", DontDelete, (intptr_t)jsWorkerContextOnmessage, (intptr_t)setJSWorkerContextOnmessage },
   { "MessageEvent", DontDelete, (intptr_t)jsWorkerContextMessageEvent, (intptr_t)setJSWorkerContextMessageEvent },
   { "WorkerLocation", DontDelete, (intptr_t)jsWorkerContextWorkerLocation, (intptr_t)setJSWorkerContextWorkerLocation },
   { 0, 0, 0, 0 }
};

extern const struct HashTable JSWorkerContextTable =
#if ENABLE(PERFECT_HASH_SIZE)
    { 127, JSWorkerContextTableValues, 0 };
#else
    { 17, 15, JSWorkerContextTableValues, 0 };
#endif

} // namespace
