/*
 * WARNING this file was generated by Appcelerator's idl2npapi
 */


#include "npSupport.h"
#include "ISciMozEvents_np.h"
#include "ISciMozEvents.h"

static void ISciMozEvents_initializeIdentifiers(void)
{
    NPN_GetStringIdentifiers(_ISciMozEvents_pluginPropertyIdentifierNames, _ISciMozEvents_NumberOfProperties, _ISciMozEvents_pluginPropertyIdentifiers);
    NPN_GetStringIdentifiers(_ISciMozEvents_pluginMethodIdentifierNames, _ISciMozEvents_NumberOfMethods, _ISciMozEvents_pluginMethodIdentifiers);
}

bool ISciMozEvents_identifiersInitialized = false;

bool ISciMozEvents_pluginInvokeDefault(NPObject *obj, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    return false;
}

void ISciMozEvents_pluginInvalidate(NPObject *obj)
{
    // Release any remaining references to JavaScript objects.
}

extern ISciMozEvents* Create_ISciMozEvents();

NPObject *ISciMozEvents_pluginAllocate(NPP npp, NPClass *theClass)
{
    ISciMozEvents *newInstance = Create_ISciMozEvents();

    if (!ISciMozEvents_identifiersInitialized) {
        ISciMozEvents_identifiersInitialized = true;
        ISciMozEvents_initializeIdentifiers();
    }

    return newInstance;
}

void ISciMozEvents_pluginDeallocate(NPObject *obj)
{
    free(obj);
}

bool ISciMozEvents_pluginRemoveProperty (NPObject *npobj, NPIdentifier name) {
  return false;
}

bool ISciMozEvents_pluginEnumerate (NPObject *npobj, NPIdentifier **value, uint32_t *count) {
  *count = _ISciMozEvents_NumberOfProperties + _ISciMozEvents_NumberOfMethods;

  value = (NPIdentifier**) malloc(sizeof(NPIdentifier*) * (*count));
  int i = 0, index = 0;

  for (i = 0; i < _ISciMozEvents_NumberOfProperties; i++, index++) value[index] = &_ISciMozEvents_pluginPropertyIdentifiers[i];
  for (i = 0; i < _ISciMozEvents_NumberOfMethods; i++, index++) value[index] = &_ISciMozEvents_pluginMethodIdentifiers[i];

  return true;
}

bool ISciMozEvents_pluginHasProperty(NPObject *obj, NPIdentifier name)
{
    int i;
    for (i = 0; i < _ISciMozEvents_NumberOfProperties; i++)
        if (name ==  _ISciMozEvents_pluginPropertyIdentifiers[i])
            return true;
    return false;
}

bool ISciMozEvents_pluginHasMethod(NPObject *obj, NPIdentifier name)
{
    int i;
    for (i = 0; i <  _ISciMozEvents_NumberOfMethods; i++)
        if (name ==  _ISciMozEvents_pluginMethodIdentifiers[i])
            return true;
    return false;
}


bool ISciMozEvents_pluginGetProperty(NPObject *obj, NPIdentifier name, NPVariant *variant) {
	ISciMozEvents *instance = (ISciMozEvents *) obj;
	
	
	return false;
}

bool ISciMozEvents_pluginSetProperty(NPObject *obj, NPIdentifier name, NPVariant *variant) {
	ISciMozEvents *instance = (ISciMozEvents *) obj;
	
	
	return false;
}


bool ISciMozEvents_pluginInvoke(NPObject *obj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result) {
	ISciMozEvents *instance = (ISciMozEvents *) obj;
	
	if (name == _ISciMozEvents_pluginMethodIdentifiers[_ISciMozEvents_onCharAdded]) {
		instance->onCharAdded(NPVARIANT_TO_INT32(args[0]));
		VOID_TO_NPVARIANT(*result);
		
		return true;
	}
	if (name == _ISciMozEvents_pluginMethodIdentifiers[_ISciMozEvents_onSavePointReached]) {
		instance->onSavePointReached();
		VOID_TO_NPVARIANT(*result);
		
		return true;
	}
	if (name == _ISciMozEvents_pluginMethodIdentifiers[_ISciMozEvents_onSavePointLeft]) {
		instance->onSavePointLeft();
		VOID_TO_NPVARIANT(*result);
		
		return true;
	}
	if (name == _ISciMozEvents_pluginMethodIdentifiers[_ISciMozEvents_onDoubleClick]) {
		instance->onDoubleClick();
		VOID_TO_NPVARIANT(*result);
		
		return true;
	}
	if (name == _ISciMozEvents_pluginMethodIdentifiers[_ISciMozEvents_onUpdateUI]) {
		instance->onUpdateUI();
		VOID_TO_NPVARIANT(*result);
		
		return true;
	}
	if (name == _ISciMozEvents_pluginMethodIdentifiers[_ISciMozEvents_onModified]) {
		instance->onModified(NPVARIANT_TO_INT32(args[0]),NPVARIANT_TO_INT32(args[1]),NPStringToString(NPVARIANT_TO_STRING(args[2])),NPVARIANT_TO_INT32(args[3]),NPVARIANT_TO_INT32(args[4]),NPVARIANT_TO_INT32(args[5]),NPVARIANT_TO_INT32(args[6]),NPVARIANT_TO_INT32(args[7]));
		VOID_TO_NPVARIANT(*result);
		
		return true;
	}
	if (name == _ISciMozEvents_pluginMethodIdentifiers[_ISciMozEvents_onMarginClick]) {
		instance->onMarginClick(NPVARIANT_TO_INT32(args[0]),NPVARIANT_TO_INT32(args[1]),NPVARIANT_TO_INT32(args[2]));
		VOID_TO_NPVARIANT(*result);
		
		return true;
	}
	if (name == _ISciMozEvents_pluginMethodIdentifiers[_ISciMozEvents_onPosChanged]) {
		instance->onPosChanged(NPVARIANT_TO_INT32(args[0]));
		VOID_TO_NPVARIANT(*result);
		
		return true;
	}
	if (name == _ISciMozEvents_pluginMethodIdentifiers[_ISciMozEvents_onZoom]) {
		instance->onZoom();
		VOID_TO_NPVARIANT(*result);
		
		return true;
	}
	if (name == _ISciMozEvents_pluginMethodIdentifiers[_ISciMozEvents_onHotSpotDoubleClick]) {
		instance->onHotSpotDoubleClick(NPVARIANT_TO_INT32(args[0]),NPVARIANT_TO_INT32(args[1]));
		VOID_TO_NPVARIANT(*result);
		
		return true;
	}
	if (name == _ISciMozEvents_pluginMethodIdentifiers[_ISciMozEvents_onDwellStart]) {
		instance->onDwellStart(NPVARIANT_TO_INT32(args[0]),NPVARIANT_TO_INT32(args[1]),NPVARIANT_TO_INT32(args[2]));
		VOID_TO_NPVARIANT(*result);
		
		return true;
	}
	if (name == _ISciMozEvents_pluginMethodIdentifiers[_ISciMozEvents_onDwellEnd]) {
		instance->onDwellEnd(NPVARIANT_TO_INT32(args[0]),NPVARIANT_TO_INT32(args[1]),NPVARIANT_TO_INT32(args[2]));
		VOID_TO_NPVARIANT(*result);
		
		return true;
	}
	
	return false;
}