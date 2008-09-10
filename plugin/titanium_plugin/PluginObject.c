/*
 *  PluginObject.h
 *  titanium_plugin
 *
 *  Created by Marshall on 9/9/08.
 *  Copyright 2008 Appcelerator, Inc. All rights reserved.
 *
 */

#import "PluginObject.h"

static void pluginInvalidate(NPObject *obj);
static bool pluginHasProperty(NPObject *obj, NPIdentifier name);
static bool pluginHasMethod(NPObject *obj, NPIdentifier name);
static bool pluginGetProperty(NPObject *obj, NPIdentifier name, NPVariant *variant);
static bool pluginSetProperty(NPObject *obj, NPIdentifier name, const NPVariant *variant);
static bool pluginInvoke(NPObject *obj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result);
static bool pluginInvokeDefault(NPObject *obj, const NPVariant *args, uint32_t argCount, NPVariant *result);
static NPObject *pluginAllocate(NPP npp, NPClass *theClass);
static void pluginDeallocate(NPObject *obj);

static NPClass pluginClass = { 
NP_CLASS_STRUCT_VERSION,
pluginAllocate, 
pluginDeallocate, 
pluginInvalidate,
pluginHasMethod,
pluginInvoke,
pluginInvokeDefault,
pluginHasProperty,
pluginGetProperty,
pluginSetProperty,
};

NPClass *getPluginClass(void)
{
    return &pluginClass;
}

static bool identifiersInitialized = false;

#define ID_FILE_PROPERTY               0
#define NUM_PROPERTY_IDENTIFIERS        1

static NPIdentifier pluginPropertyIdentifiers[NUM_PROPERTY_IDENTIFIERS];
static const NPUTF8 *pluginPropertyIdentifierNames[NUM_PROPERTY_IDENTIFIERS] = {
"file"
};

#define ID_GETFILE_METHOD                      0
#define NUM_METHOD_IDENTIFIERS                  1

static NPIdentifier pluginMethodIdentifiers[NUM_METHOD_IDENTIFIERS];
static const NPUTF8 *pluginMethodIdentifierNames[NUM_METHOD_IDENTIFIERS] = {
"getFile"
};

static void initializeIdentifiers(void)
{
    browser->getstringidentifiers(pluginPropertyIdentifierNames, NUM_PROPERTY_IDENTIFIERS, pluginPropertyIdentifiers);
    browser->getstringidentifiers(pluginMethodIdentifierNames, NUM_METHOD_IDENTIFIERS, pluginMethodIdentifiers);
}

bool pluginHasProperty(NPObject *obj, NPIdentifier name)
{
    int i;
    for (i = 0; i < NUM_PROPERTY_IDENTIFIERS; i++)
        if (name == pluginPropertyIdentifiers[i])
            return true;
    return false;
}

bool pluginHasMethod(NPObject *obj, NPIdentifier name)
{
    int i;
    for (i = 0; i < NUM_METHOD_IDENTIFIERS; i++)
        if (name == pluginMethodIdentifiers[i])
            return true;
    return false;
}

bool pluginGetProperty(NPObject *obj, NPIdentifier name, NPVariant *variant)
{
    PluginObject *plugin = (PluginObject *)obj;
    if (name == pluginPropertyIdentifiers[ID_FILE_PROPERTY]) {
		STRINGZ_TO_NPVARIANT(plugin->filename, *variant);
        return true;
    }
    return false;
}

bool pluginSetProperty(NPObject *obj, NPIdentifier name, const NPVariant *variant)
{
    return false;
}

bool pluginInvoke(NPObject *obj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    PluginObject *plugin = (PluginObject *)obj;
    if (name == pluginMethodIdentifiers[ID_GETFILE_METHOD]) {
		STRINGZ_TO_NPVARIANT(plugin->filename, *result);
        return true;
    }
    return false;
}

bool pluginInvokeDefault(NPObject *obj, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    return false;
}

void pluginInvalidate(NPObject *obj)
{
    // Release any remaining references to JavaScript objects.
}

NPObject *pluginAllocate(NPP npp, NPClass *theClass)
{
    PluginObject *newInstance = malloc(sizeof(PluginObject));
    
    if (!identifiersInitialized) {
        identifiersInitialized = true;
        initializeIdentifiers();
    }
	
    newInstance->npp = npp;
	
    return &newInstance->header;
}

void pluginDeallocate(NPObject *obj) 
{
    free(obj);
}
