//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
// npmac.cpp
//
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

// even though we dont link to the sdk, we still use its headers
#include <npapi.h>
#include <npruntime.h>
#include <npfunctions.h>
#include "plugin.h"

#ifdef __ppc__

// glue for mapping outgoing Macho function pointers to TVectors
struct TFPtoTVGlue{
    void* glue[2];
};

struct {
    TFPtoTVGlue     newp;
    TFPtoTVGlue     destroy;
    TFPtoTVGlue     setwindow;
    TFPtoTVGlue     newstream;
    TFPtoTVGlue     destroystream;
    TFPtoTVGlue     asfile;
    TFPtoTVGlue     writeready;
    TFPtoTVGlue     write;
    TFPtoTVGlue     print;
    TFPtoTVGlue     event;
    TFPtoTVGlue     urlnotify;
    TFPtoTVGlue     getvalue;
    TFPtoTVGlue     setvalue;

    TFPtoTVGlue     shutdown;
} gPluginFuncsGlueTable;

static inline void* SetupFPtoTVGlue(TFPtoTVGlue* functionGlue, void* fp)
{
    functionGlue->glue[0] = fp;
    functionGlue->glue[1] = 0;
    return functionGlue;
}

#define PLUGIN_TO_HOST_GLUE(name, fp) (SetupFPtoTVGlue(&gPluginFuncsGlueTable.name, (void*)fp))

// glue for mapping netscape TVectors to Macho function pointers
struct TTVtoFPGlue {
    uint32 glue[6];
};

struct {
    TTVtoFPGlue             geturl;
    TTVtoFPGlue             posturl;
    TTVtoFPGlue             requestread;
    TTVtoFPGlue             newstream;
    TTVtoFPGlue             write;
    TTVtoFPGlue             destroystream;
    TTVtoFPGlue             status;
    TTVtoFPGlue             uagent;
    TTVtoFPGlue             memalloc;
    TTVtoFPGlue             memfree;
    TTVtoFPGlue             memflush;
    TTVtoFPGlue             reloadplugins;
    TTVtoFPGlue             getJavaEnv;
    TTVtoFPGlue             getJavaPeer;
    TTVtoFPGlue             geturlnotify;
    TTVtoFPGlue             posturlnotify;
    TTVtoFPGlue             getvalue;
    TTVtoFPGlue             setvalue;
    TTVtoFPGlue             invalidaterect;
    TTVtoFPGlue             invalidateregion;
    TTVtoFPGlue             forceredraw;
} gNetscapeFuncsGlueTable;

static void* SetupTVtoFPGlue(TTVtoFPGlue* functionGlue, void* tvp)
{
    static const TTVtoFPGlue glueTemplate = { 0x3D800000, 0x618C0000, 0x800C0000, 0x804C0004, 0x7C0903A6, 0x4E800420 };

    memcpy(functionGlue, &glueTemplate, sizeof(TTVtoFPGlue));
    functionGlue->glue[0] |= ((UInt32)tvp >> 16);
    functionGlue->glue[1] |= ((UInt32)tvp & 0xFFFF);
    ::MakeDataExecutable(functionGlue, sizeof(TTVtoFPGlue));
    return functionGlue;
}

#define HOST_TO_PLUGIN_GLUE(name, fp) (SetupTVtoFPGlue(&gNetscapeFuncsGlueTable.name, (void*)fp))

#else

#define PLUGIN_TO_HOST_GLUE(name, fp) (fp)
#define HOST_TO_PLUGIN_GLUE(name, fp) (fp)

#endif /* __ppc__ */


#pragma mark -


//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
// Globals
//
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

short			gResFile;			// Refnum of the plugin’s resource file
NPNetscapeFuncs	gNetscapeFuncs;		// Function table for procs in Netscape called by plugin

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
// Wrapper functions for all calls from the plugin to Netscape.
// These functions let the plugin developer just call the APIs
// as documented and defined in npapi.h, without needing to know
// about the function table and call macros in npupp.h.
//
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


void NPN_Version(int* plugin_major, int* plugin_minor, int* netscape_major, int* netscape_minor)
{
	*plugin_major = NP_VERSION_MAJOR;
	*plugin_minor = NP_VERSION_MINOR;
	*netscape_major = gNetscapeFuncs.version >> 8;		// Major version is in high byte
	*netscape_minor = gNetscapeFuncs.version & 0xFF;	// Minor version is in low byte
}

NPError NPN_GetURLNotify(NPP instance, const char* url, const char* window, void* notifyData)
{
	int navMinorVers = gNetscapeFuncs.version & 0xFF;
	NPError err;
	
	if( navMinorVers >= NPVERS_HAS_NOTIFICATION )
	{
		gNetscapeFuncs.geturlnotify(instance, url, window, notifyData);
	}
	else
	{
		err = NPERR_INCOMPATIBLE_VERSION_ERROR;
	}
	return err;
}

NPError NPN_GetURL(NPP instance, const char* url, const char* window)
{
	return gNetscapeFuncs.geturl(instance, url, window);
}

NPError NPN_PostURLNotify(NPP instance, const char* url, const char* window, uint32 len, const char* buf, NPBool file, void* notifyData)
{
	int navMinorVers = gNetscapeFuncs.version & 0xFF;
	NPError err;
	
	if( navMinorVers >= NPVERS_HAS_NOTIFICATION )
	{
		err = gNetscapeFuncs.posturlnotify(instance, url, window, len, buf, file, notifyData);
	}
	else
	{
		err = NPERR_INCOMPATIBLE_VERSION_ERROR;
	}
	return err;
}

NPError NPN_PostURL(NPP instance, const char* url, const char* window, uint32 len, const char* buf, NPBool file)
{
	return gNetscapeFuncs.posturl(instance, url, window, len, buf, file);
}

NPError NPN_RequestRead(NPStream* stream, NPByteRange* rangeList)
{
	return gNetscapeFuncs.requestread(stream, rangeList);
}

NPError NPN_NewStream(NPP instance, NPMIMEType type, const char* window, NPStream** stream)
{
	int navMinorVers = gNetscapeFuncs.version & 0xFF;
	NPError err;
	
	if( navMinorVers >= NPVERS_HAS_STREAMOUTPUT )
	{
		err = gNetscapeFuncs.newstream(instance, type, window, stream);
	}
	else
	{
		err = NPERR_INCOMPATIBLE_VERSION_ERROR;
	}
	return err;
}

int32 NPN_Write(NPP instance, NPStream* stream, int32 len, void* buffer)
{
	int navMinorVers = gNetscapeFuncs.version & 0xFF;
	NPError err;
	
	if( navMinorVers >= NPVERS_HAS_STREAMOUTPUT )
	{
		err = gNetscapeFuncs.write(instance, stream, len, buffer);
	}
	else
	{
		err = NPERR_INCOMPATIBLE_VERSION_ERROR;
	}
	return err;
}

NPError	NPN_DestroyStream(NPP instance, NPStream* stream, NPError reason)
{
	int navMinorVers = gNetscapeFuncs.version & 0xFF;
	NPError err;
	
	if( navMinorVers >= NPVERS_HAS_STREAMOUTPUT )
	{
		err = gNetscapeFuncs.destroystream(instance, stream, reason);
	}
	else
	{
		err = NPERR_INCOMPATIBLE_VERSION_ERROR;
	}
	return err;
}

void NPN_Status(NPP instance, const char* message)
{
	gNetscapeFuncs.status(instance, message);
}

const char* NPN_UserAgent(NPP instance)
{
	return gNetscapeFuncs.uagent(instance);
}

void* NPN_MemAlloc(uint32 size)
{
	return gNetscapeFuncs.memalloc(size);
}

void NPN_MemFree(void* ptr)
{
	gNetscapeFuncs.memfree(ptr);
}

uint32 NPN_MemFlush(uint32 size)
{
	return gNetscapeFuncs.memflush(size);
}

void NPN_ReloadPlugins(NPBool reloadPages)
{
	gNetscapeFuncs.reloadplugins(reloadPages);
}

#ifdef OJI
JRIEnv* NPN_GetJavaEnv(void)
{
	return gNetscapeFuncs.getJavaEnv();
}

jobject  NPN_GetJavaPeer(NPP instance)
{
	return gNetscapeFuncs.getJavaPeer(instance);
}
#endif

NPError NPN_GetValue(NPP instance, NPNVariable variable, void *value)
{
	return gNetscapeFuncs.getvalue(instance, variable, value);
}

NPError NPN_SetValue(NPP instance, NPPVariable variable, void *value)
{
	return gNetscapeFuncs.setvalue(instance, variable, value);
}

void NPN_InvalidateRect(NPP instance, NPRect *rect)
{
	gNetscapeFuncs.invalidaterect(instance, rect);
}

void NPN_InvalidateRegion(NPP instance, NPRegion region)
{
	gNetscapeFuncs.invalidateregion(instance, region);
}

void NPN_ForceRedraw(NPP instance)
{
	gNetscapeFuncs.forceredraw(instance);
}

#pragma mark -

// here the plugin creates a plugin instance object which 
// will be associated with this newly created NPP instance and 
// will do all the neccessary job
NPError NPP_New(NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc,
                char* argn[], char* argv[], NPSavedData* saved)
{
  if(instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  NPError rv = NPERR_NO_ERROR;

  // create a new plugin instance object
  // initialization will be done when the associated window is ready


  PluginInstance * plugin = NS_NewPluginInstance(instance);
  if(plugin == NULL)
    return NPERR_OUT_OF_MEMORY_ERROR;

  // associate the plugin instance object with NPP instance
  instance->pdata = (void *)plugin;
  return rv;
}

// here is the place to clean up and destroy the PluginInstance object
NPError NPP_Destroy (NPP instance, NPSavedData** save)
{
  if(instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  NPError rv = NPERR_NO_ERROR;

  PluginInstance * plugin = (PluginInstance *)instance->pdata;
  if(plugin != NULL) {
    plugin->shut();
    NS_DestroyPluginInstance(plugin);
  }
  return rv;
}

// during this call we know when the plugin window is ready or
// is about to be destroyed so we can do some gui specific
// initialization and shutdown
NPError NPP_SetWindow (NPP instance, NPWindow* pNPWindow)
{    
  if(instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  NPError rv = NPERR_NO_ERROR;

  if(pNPWindow == NULL)
    return NPERR_GENERIC_ERROR;

  PluginInstance * plugin = (PluginInstance *)instance->pdata;

  if(plugin == NULL) 
    return NPERR_GENERIC_ERROR;

  // window just created
  if(!plugin->isInitialized() && (pNPWindow->window != NULL)) { 
    if(!plugin->init(pNPWindow)) {
      NS_DestroyPluginInstance(plugin);
      return NPERR_MODULE_LOAD_FAILED_ERROR;
    }
  }

  // window goes away
  if((pNPWindow->window == NULL) && plugin->isInitialized())
    return plugin->SetWindow(pNPWindow);

  // window resized?
  if(plugin->isInitialized() && (pNPWindow->window != NULL))
    return plugin->SetWindow(pNPWindow);

  // this should not happen, nothing to do
  if((pNPWindow->window == NULL) && !plugin->isInitialized())
    return plugin->SetWindow(pNPWindow);

  return rv;
}

NPError NPP_NewStream(NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16* stype)
{
  if(instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  PluginInstance * plugin = (PluginInstance *)instance->pdata;
  if(plugin == NULL) 
    return NPERR_GENERIC_ERROR;

  NPError rv = NPN_NewStream(instance, type, stream->url, &stream);
  return rv;
}

int32 NPP_WriteReady (NPP instance, NPStream *stream)
{
  if(instance == NULL)
    return 0x0fffffff;

  PluginInstance * plugin = (PluginInstance *)instance->pdata;
  if(plugin == NULL) 
    return 0x0fffffff;

  //int32 rv = plugin->WriteReady(stream);
  return NPERR_NO_ERROR;
}

int32 NPP_Write (NPP instance, NPStream *stream, int32 offset, int32 len, void *buffer)
{   
  if(instance == NULL)
    return len;

  PluginInstance * plugin = (PluginInstance *)instance->pdata;
  if(plugin == NULL) 
    return len;

  int32 rv = NPN_Write(instance, stream, len, buffer);
  return rv;
}

NPError NPP_DestroyStream (NPP instance, NPStream *stream, NPError reason)
{
  if(instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  PluginInstance * plugin = (PluginInstance *)instance->pdata;
  if(plugin == NULL) 
    return NPERR_GENERIC_ERROR;

  NPError rv = NPN_DestroyStream(instance, stream, reason);
  return rv;
}

void NPP_StreamAsFile (NPP instance, NPStream* stream, const char* fname)
{
  if(instance == NULL)
    return;

  PluginInstance * plugin = (PluginInstance *)instance->pdata;
  if(plugin == NULL) 
    return;

  //plugin->StreamAsFile(stream, fname);
}

void NPP_Print (NPP instance, NPPrint* printInfo)
{
  if(instance == NULL)
    return;

  PluginInstance * plugin = (PluginInstance *)instance->pdata;
  if(plugin == NULL) 
    return;

  //plugin->Print(printInfo);
}

void NPP_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData)
{
  if(instance == NULL)
    return;

  PluginInstance * plugin = (PluginInstance *)instance->pdata;
  if(plugin == NULL) 
    return;

  //plugin->URLNotify(url, reason, notifyData);
}

NPError	NPP_GetValue(NPP instance, NPPVariable variable, void *value)
{
  if(instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  PluginInstance * plugin = (PluginInstance *)instance->pdata;
  if(plugin == NULL) 
    return NPERR_GENERIC_ERROR;

  NPError rv = plugin->GetValue(variable, value);
  return rv;
}

NPError NPP_SetValue(NPP instance, NPNVariable variable, void *value)
{
  if(instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  PluginInstance * plugin = (PluginInstance *)instance->pdata;
  if(plugin == NULL) 
    return NPERR_GENERIC_ERROR;

  NPError rv = plugin->SetValue(variable, value);
  return rv;
}

int16	NPP_HandleEvent(NPP instance, void* event)
{
  if(instance == NULL)
    return 0;

  PluginInstance * plugin = (PluginInstance *)instance->pdata;
  if(plugin == NULL) 
    return 0;

  uint16 rv = plugin->HandleEvent(event);
  return rv;
}

#ifdef OJI
jref NPP_GetJavaClass (void)
{
  return NULL;
}
#endif

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
// Wrapper functions for all calls from Netscape to the plugin.
// These functions let the plugin developer just create the APIs
// as documented and defined in npapi.h, without needing to 
// install those functions in the function table or worry about
// setting up globals for 68K plugins.
//
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

NPError 	Private_Initialize(void);
void 		Private_Shutdown(void);
NPError		Private_New(NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc, char* argn[], char* argv[], NPSavedData* saved);
NPError 	Private_Destroy(NPP instance, NPSavedData** save);
NPError		Private_SetWindow(NPP instance, NPWindow* window);
NPError		Private_NewStream(NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16* stype);
NPError		Private_DestroyStream(NPP instance, NPStream* stream, NPError reason);
int32		Private_WriteReady(NPP instance, NPStream* stream);
int32		Private_Write(NPP instance, NPStream* stream, int32 offset, int32 len, void* buffer);
void		Private_StreamAsFile(NPP instance, NPStream* stream, const char* fname);
void		Private_Print(NPP instance, NPPrint* platformPrint);
int16 		Private_HandleEvent(NPP instance, void* event);
void        Private_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData);
#ifdef OJI
jobject		Private_GetJavaClass(void);
#endif
NPError     Private_GetValue(NPP instance, NPPVariable variable, void *result);
NPError     Private_SetValue(NPP instance, NPNVariable variable, void *value);


NPError Private_Initialize(void)
{
	return NS_PluginInitialize();
}

void Private_Shutdown(void)
{
	NS_PluginShutdown();
}


NPError	Private_New(NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc, char* argn[], char* argv[], NPSavedData* saved)
{
	return NPP_New(pluginType, instance, mode, argc, argn, argv, saved);	
}

NPError Private_Destroy(NPP instance, NPSavedData** save)
{
	return NPP_Destroy(instance, save);
}

NPError Private_SetWindow(NPP instance, NPWindow* window)
{
	return NPP_SetWindow(instance, window);
}

NPError Private_NewStream(NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16* stype)
{
	return NPP_NewStream(instance, type, stream, seekable, stype);
}

int32 Private_WriteReady(NPP instance, NPStream* stream)
{
	return NPP_WriteReady(instance, stream);
}

int32 Private_Write(NPP instance, NPStream* stream, int32_t offset, int32_t len, void* buffer)
{
	return NPP_Write(instance, stream, offset, len, buffer);
}

void Private_StreamAsFile(NPP instance, NPStream* stream, const char* fname)
{
	NPP_StreamAsFile(instance, stream, fname);
}


NPError Private_DestroyStream(NPP instance, NPStream* stream, NPError reason)
{
	return NPP_DestroyStream(instance, stream, reason);
}

int16 Private_HandleEvent(NPP instance, void* event)
{
	return NPP_HandleEvent(instance, event);
}

void Private_Print(NPP instance, NPPrint* platformPrint)
{
	NPP_Print(instance, platformPrint);
}

void Private_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData)
{
	NPP_URLNotify(instance, url, reason, notifyData);
}

#ifdef OJI
jobject Private_GetJavaClass(void)
{
    jobject clazz = NPP_GetJavaClass();
    if (clazz)
    {
		JRIEnv* env = NPN_GetJavaEnv();
		return (jobject)JRI_NewGlobalRef(env, clazz);
    }
    return NULL;
}
#endif

NPError	Private_GetValue(NPP instance, NPPVariable variable, void *value) {
	return NPP_GetValue(instance, variable, value);
}

NPError	Private_SetValue(NPP instance, NPNVariable variable, void *value) {
	return NPP_SetValue(instance, variable, value);
}

#ifdef __GNUC__
// gcc requires that main have an 'int' return type
int main(NPNetscapeFuncs* nsTable, NPPluginFuncs* pluginFuncs, NPP_ShutdownProcPtr unloadUpp);
#else
NPError main(NPNetscapeFuncs* nsTable, NPPluginFuncs* pluginFuncs, NPP_ShutdownProcPtr unloadUpp);
#endif

#ifdef __GNUC__
DEFINE_API_C(int) main(NPNetscapeFuncs* nsTable, NPPluginFuncs* pluginFuncs, NPP_ShutdownProcPtr unloadUpp)
#else
DEFINE_API_C(NPError) main(NPNetscapeFuncs* nsTable, NPPluginFuncs* pluginFuncs, NPP_ShutdownProcPtr unloadUpp)
#endif
{
	NPError err = NPERR_NO_ERROR;
	
	//
	// Ensure that everything Netscape passed us is valid!
	//
	if ((nsTable == NULL) || (pluginFuncs == NULL) || (unloadUpp == NULL))
		err = NPERR_INVALID_FUNCTABLE_ERROR;
	
	//
	// Check the “major” version passed in Netscape’s function table.
	// We won’t load if the major version is newer than what we expect.
	// Also check that the function tables passed in are big enough for
	// all the functions we need (they could be bigger, if Netscape added
	// new APIs, but that’s OK with us -- we’ll just ignore them).
	//
	if (err == NPERR_NO_ERROR)
	{
		if ((nsTable->version >> 8) > NP_VERSION_MAJOR)		// Major version is in high byte
			err = NPERR_INCOMPATIBLE_VERSION_ERROR;
	}
		
	
	if (err == NPERR_NO_ERROR)
	{
		//
		// Copy all the fields of Netscape’s function table into our
		// copy so we can call back into Netscape later.  Note that
		// we need to copy the fields one by one, rather than assigning
		// the whole structure, because the Netscape function table
		// could actually be bigger than what we expect.
		//
		
		int navMinorVers = nsTable->version & 0xFF;

		gNetscapeFuncs.version          = nsTable->version;
		gNetscapeFuncs.size             = nsTable->size;
		gNetscapeFuncs.posturl          = HOST_TO_PLUGIN_GLUE(posturl, nsTable->posturl);
		gNetscapeFuncs.geturl           = HOST_TO_PLUGIN_GLUE(geturl, nsTable->geturl);
		gNetscapeFuncs.requestread      = HOST_TO_PLUGIN_GLUE(requestread, nsTable->requestread);
		gNetscapeFuncs.newstream        = HOST_TO_PLUGIN_GLUE(newstream, nsTable->newstream);
		gNetscapeFuncs.write            = HOST_TO_PLUGIN_GLUE(write, nsTable->write);
		gNetscapeFuncs.destroystream    = HOST_TO_PLUGIN_GLUE(destroystream, nsTable->destroystream);
		gNetscapeFuncs.status           = HOST_TO_PLUGIN_GLUE(status, nsTable->status);
		gNetscapeFuncs.uagent           = HOST_TO_PLUGIN_GLUE(uagent, nsTable->uagent);
		gNetscapeFuncs.memalloc         = HOST_TO_PLUGIN_GLUE(memalloc, nsTable->memalloc);
		gNetscapeFuncs.memfree          = HOST_TO_PLUGIN_GLUE(memfree, nsTable->memfree);
		gNetscapeFuncs.memflush         = HOST_TO_PLUGIN_GLUE(memflush, nsTable->memflush);
		gNetscapeFuncs.reloadplugins    = HOST_TO_PLUGIN_GLUE(reloadplugins, nsTable->reloadplugins);
		if( navMinorVers >= NPVERS_HAS_LIVECONNECT )
		{
			gNetscapeFuncs.getJavaEnv   = HOST_TO_PLUGIN_GLUE(getJavaEnv, nsTable->getJavaEnv);
			gNetscapeFuncs.getJavaPeer  = HOST_TO_PLUGIN_GLUE(getJavaPeer, nsTable->getJavaPeer);
		}
		if( navMinorVers >= NPVERS_HAS_NOTIFICATION )
		{	
			gNetscapeFuncs.geturlnotify 	= HOST_TO_PLUGIN_GLUE(geturlnotify, nsTable->geturlnotify);
			gNetscapeFuncs.posturlnotify 	= HOST_TO_PLUGIN_GLUE(posturlnotify, nsTable->posturlnotify);
		}
		gNetscapeFuncs.getvalue         = HOST_TO_PLUGIN_GLUE(getvalue, nsTable->getvalue);
		gNetscapeFuncs.setvalue         = HOST_TO_PLUGIN_GLUE(setvalue, nsTable->setvalue);
		gNetscapeFuncs.invalidaterect   = HOST_TO_PLUGIN_GLUE(invalidaterect, nsTable->invalidaterect);
		gNetscapeFuncs.invalidateregion = HOST_TO_PLUGIN_GLUE(invalidateregion, nsTable->invalidateregion);
		gNetscapeFuncs.forceredraw      = HOST_TO_PLUGIN_GLUE(forceredraw, nsTable->forceredraw);
		
		//
		// Set up the plugin function table that Netscape will use to
		// call us.  Netscape needs to know about our version and size
		// and have a UniversalProcPointer for every function we implement.
		//
		pluginFuncs->version        = (NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR;
		pluginFuncs->size           = sizeof(NPPluginFuncs);
		pluginFuncs->newp           = PLUGIN_TO_HOST_GLUE(newp, Private_New);
		pluginFuncs->destroy        = PLUGIN_TO_HOST_GLUE(destroy, Private_Destroy);
		pluginFuncs->setwindow      = PLUGIN_TO_HOST_GLUE(setwindow, Private_SetWindow);
		pluginFuncs->newstream      = PLUGIN_TO_HOST_GLUE(newstream, Private_NewStream);
		pluginFuncs->destroystream  = PLUGIN_TO_HOST_GLUE(destroystream, Private_DestroyStream);
		pluginFuncs->asfile         = PLUGIN_TO_HOST_GLUE(asfile, Private_StreamAsFile);
		pluginFuncs->writeready     = PLUGIN_TO_HOST_GLUE(writeready, Private_WriteReady);
		pluginFuncs->write          = PLUGIN_TO_HOST_GLUE(write, Private_Write);
		pluginFuncs->print          = PLUGIN_TO_HOST_GLUE(print, Private_Print);
		pluginFuncs->event          = PLUGIN_TO_HOST_GLUE(event, Private_HandleEvent);
		if( navMinorVers >= NPVERS_HAS_NOTIFICATION )
		{	
			pluginFuncs->urlnotify = PLUGIN_TO_HOST_GLUE(urlnotify, Private_URLNotify);
		}
#ifdef OJI
		if( navMinorVers >= NPVERS_HAS_LIVECONNECT )
		{
			pluginFuncs->javaClass	= (JRIGlobalRef) Private_GetJavaClass();
		}
#else
                pluginFuncs->javaClass = NULL;
#endif
		if( navMinorVers >= NPVERS_HAS_LIVECONNECT )
		{
			pluginFuncs->getvalue = PLUGIN_TO_HOST_GLUE(getvalue, Private_GetValue);
			pluginFuncs->setvalue = PLUGIN_TO_HOST_GLUE(setvalue, Private_SetValue);
		}

		//*unloadUpp = PLUGIN_TO_HOST_GLUE(shutdown, Private_Shutdown);

		gResFile = CurResFile();
		err = Private_Initialize();
	}

	return err;
}
