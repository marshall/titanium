/*
 * WARNING this file was generated by Appcelerator's idl2npapi
 */
 
#ifndef _ISCIMOZCONTROLLER_NP_H
#define _ISCIMOZCONTROLLER_NP_H

#include <npapi.h>
#include <npruntime.h>
#include <npfunctions.h>
#include <string>

static void ISciMozController_pluginInvalidate(NPObject *obj);
static bool ISciMozController_pluginHasProperty(NPObject *obj, NPIdentifier name);
static bool ISciMozController_pluginHasMethod(NPObject *obj, NPIdentifier name);
static bool ISciMozController_pluginGetProperty(NPObject *obj, NPIdentifier name, NPVariant *variant);
static bool ISciMozController_pluginSetProperty(NPObject *obj, NPIdentifier name, const NPVariant *variant);
static bool ISciMozController_pluginInvoke(NPObject *obj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result);
static bool ISciMozController_pluginInvokeDefault(NPObject *obj, const NPVariant *args, uint32_t argCount, NPVariant *result);
static NPObject* ISciMozController_pluginAllocate(NPP npp, NPClass *theClass);
static void ISciMozController_pluginDeallocate(NPObject *obj);
static bool ISciMozController_pluginRemoveProperty (NPObject *npobj, NPIdentifier name);
static bool ISciMozController_pluginEnumerate (NPObject *npobj, NPIdentifier **value, uint32_t *count);

static NPClass ISciMozController_pluginClass = {
NP_CLASS_STRUCT_VERSION,
ISciMozController_pluginAllocate,
ISciMozController_pluginDeallocate,
ISciMozController_pluginInvalidate,
ISciMozController_pluginHasMethod,
ISciMozController_pluginInvoke,
ISciMozController_pluginInvokeDefault,
ISciMozController_pluginHasProperty,
ISciMozController_pluginGetProperty,
ISciMozController_pluginSetProperty,
ISciMozController_pluginRemoveProperty
};

enum {
	_ISciMozController_NumberOfProperties
};

static NPIdentifier _ISciMozController_pluginPropertyIdentifiers[_ISciMozController_NumberOfProperties];
static const NPUTF8 *_ISciMozController_pluginPropertyIdentifierNames[_ISciMozController_NumberOfProperties] = {
};

enum {
	_ISciMozController_init,
	_ISciMozController_test_scimoz,
	_ISciMozController_NumberOfMethods
};

static NPIdentifier _ISciMozController_pluginMethodIdentifiers[_ISciMozController_NumberOfMethods];
static const NPUTF8 *_ISciMozController_pluginMethodIdentifierNames[_ISciMozController_NumberOfMethods] = {
	"init",
	"test_scimoz"
};

#endif