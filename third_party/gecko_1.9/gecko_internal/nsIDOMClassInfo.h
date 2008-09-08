/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Johnny Stenback <jst@netscape.com> (original author)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef nsIDOMClassInfo_h___
#define nsIDOMClassInfo_h___

#include "nsIClassInfoImpl.h"
#include "nsVoidArray.h"
#include "nsDOMClassInfoID.h"
#include "nsIXPCScriptable.h"

#define DOM_BASE_SCRIPTABLE_FLAGS                                          \
  (nsIXPCScriptable::USE_JSSTUB_FOR_ADDPROPERTY |                          \
   nsIXPCScriptable::USE_JSSTUB_FOR_DELPROPERTY |                          \
   nsIXPCScriptable::USE_JSSTUB_FOR_SETPROPERTY |                          \
   nsIXPCScriptable::ALLOW_PROP_MODS_DURING_RESOLVE |                      \
   nsIXPCScriptable::ALLOW_PROP_MODS_TO_PROTOTYPE |                        \
   nsIXPCScriptable::DONT_ASK_INSTANCE_FOR_SCRIPTABLE |                    \
   nsIXPCScriptable::DONT_REFLECT_INTERFACE_NAMES)

#define DEFAULT_SCRIPTABLE_FLAGS                                           \
  (DOM_BASE_SCRIPTABLE_FLAGS |                                             \
   nsIXPCScriptable::WANT_NEWRESOLVE |                                     \
   nsIXPCScriptable::WANT_CHECKACCESS |                                    \
   nsIXPCScriptable::WANT_PRECREATE |                                      \
   nsIXPCScriptable::WANT_POSTCREATE)

#define DOM_DEFAULT_SCRIPTABLE_FLAGS                                       \
  (DEFAULT_SCRIPTABLE_FLAGS |                                              \
   nsIXPCScriptable::DONT_ENUM_QUERY_INTERFACE |                           \
   nsIXPCScriptable::CLASSINFO_INTERFACES_ONLY)


typedef nsIClassInfo* (*nsDOMClassInfoExternalConstructorFnc)
  (const char* aName);


/**
 * nsIClassInfo helper macros
 */

#define NS_CLASSINFO_MAP_BEGIN(_class)

#define NS_CLASSINFO_MAP_BEGIN_EXPORTED(_class)

#define NS_CLASSINFO_MAP_ENTRY(_interface)

#define NS_CLASSINFO_MAP_ENTRY_FUNCTION(_function)

#define NS_CLASSINFO_MAP_END


#include "nsIServiceManager.h"
#include "nsIDOMScriptObjectFactory.h"
#include "nsDOMCID.h"

#define NS_INTERFACE_MAP_ENTRY_DOM_CLASSINFO(_class)                       \
  if (aIID.Equals(NS_GET_IID(nsIClassInfo))) {                             \
    static NS_DEFINE_CID(kDOMSOF_CID, NS_DOM_SCRIPT_OBJECT_FACTORY_CID);   \
                                                                           \
    nsresult rv;                                                           \
    nsCOMPtr<nsIDOMScriptObjectFactory> sof(do_GetService(kDOMSOF_CID,     \
                                                          &rv));           \
    if (NS_FAILED(rv)) {                                                   \
      *aInstancePtr = nsnull;                                              \
      return rv;                                                           \
    }                                                                      \
                                                                           \
    foundInterface =                                                       \
      sof->GetClassInfoInstance(eDOMClassInfo_##_class##_id);              \
  } else

// Looks up the nsIClassInfo for a class name registered with the 
// nsScriptNamespaceManager. Remember to release NS_CLASSINFO_NAME(_class)
// (eg. when your module unloads).
#define NS_INTERFACE_MAP_ENTRY_EXTERNAL_DOM_CLASSINFO(_class)              \
  if (aIID.Equals(NS_GET_IID(nsIClassInfo))) {                             \
    extern nsISupports *NS_CLASSINFO_NAME(_class);                         \
    if (NS_CLASSINFO_NAME(_class)) {                                       \
      foundInterface = NS_CLASSINFO_NAME(_class);                          \
    } else {                                                               \
      static NS_DEFINE_CID(kDOMSOF_CID, NS_DOM_SCRIPT_OBJECT_FACTORY_CID); \
                                                                           \
      nsresult rv;                                                         \
      nsCOMPtr<nsIDOMScriptObjectFactory> sof(do_GetService(kDOMSOF_CID,   \
                                                            &rv));         \
      if (NS_FAILED(rv)) {                                                 \
        *aInstancePtr = nsnull;                                            \
        return rv;                                                         \
      }                                                                    \
                                                                           \
      foundInterface =                                                     \
        sof->GetExternalClassInfoInstance(NS_LITERAL_STRING(#_class));     \
                                                                           \
      if (foundInterface) {                                                \
        NS_CLASSINFO_NAME(_class) = foundInterface;                        \
        NS_CLASSINFO_NAME(_class)->AddRef();                               \
      }                                                                    \
    }                                                                      \
  } else


#define NS_DECL_DOM_CLASSINFO(_class) \
  nsISupports *NS_CLASSINFO_NAME(_class) = nsnull;

// {891a7b01-1b61-11d6-a7f2-f690b638899c}
#define NS_IDOMCI_EXTENSION_IID  \
{ 0x891a7b01, 0x1b61, 0x11d6, \
{ 0xa7, 0xf2, 0xf6, 0x90, 0xb6, 0x38, 0x89, 0x9c } }

class nsIDOMScriptObjectFactory;

class nsIDOMCIExtension : public nsISupports {
public:  
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IDOMCI_EXTENSION_IID)

  NS_IMETHOD RegisterDOMCI(const char* aName,
                           nsIDOMScriptObjectFactory* aDOMSOFactory) = 0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsIDOMCIExtension, NS_IDOMCI_EXTENSION_IID)

#define NS_DOMCI_EXTENSION_NAME(_module) ns##_module##DOMCIExtension
#define NS_DOMCI_EXTENSION_CONSTRUCTOR(_module) \
  ns##_module##DOMCIExtensionConstructor
#define NS_DOMCI_EXTENSION_CONSTRUCTOR_IMP(_extension) \
  NS_GENERIC_FACTORY_CONSTRUCTOR(_extension)

#define NS_DOMCI_EXTENSION(_module)                                       \
class NS_DOMCI_EXTENSION_NAME(_module) : public nsIDOMCIExtension         \
{                                                                         \
public:                                                                   \
  NS_DOMCI_EXTENSION_NAME(_module)();                                     \
  virtual ~NS_DOMCI_EXTENSION_NAME(_module)();                            \
                                                                          \
  NS_DECL_ISUPPORTS                                                       \
                                                                          \
  NS_IMETHOD RegisterDOMCI(const char* aName,                             \
                           nsIDOMScriptObjectFactory* aDOMSOFactory);     \
};                                                                        \
                                                                          \
NS_DOMCI_EXTENSION_CONSTRUCTOR_IMP(NS_DOMCI_EXTENSION_NAME(_module))      \
                                                                          \
NS_DOMCI_EXTENSION_NAME(_module)::NS_DOMCI_EXTENSION_NAME(_module)()      \
{                                                                         \
}                                                                         \
                                                                          \
NS_DOMCI_EXTENSION_NAME(_module)::~NS_DOMCI_EXTENSION_NAME(_module)()     \
{                                                                         \
}                                                                         \
                                                                          \
NS_IMPL_ISUPPORTS1(NS_DOMCI_EXTENSION_NAME(_module), nsIDOMCIExtension)   \
                                                                          \
NS_IMETHODIMP                                                             \
NS_DOMCI_EXTENSION_NAME(_module)::RegisterDOMCI(const char* aName,        \
                                                nsIDOMScriptObjectFactory* aDOMSOFactory) \
{

#define NS_DOMCI_EXTENSION_ENTRY_BEGIN(_class)                            \
  if (strcmp(aName, #_class) == 0) {                               \
    static const nsIID* interfaces[] = {

#define NS_DOMCI_EXTENSION_ENTRY_INTERFACE(_interface)                    \
      &NS_GET_IID(_interface),

// Don't forget to register the primary interface (_proto) in the 
// JAVASCRIPT_DOM_INTERFACE category, or prototypes for this class
// won't work (except if the interface name starts with nsIDOM).
#define NS_DOMCI_EXTENSION_ENTRY_END_HELPER(_class, _proto, _hasclassif,  \
                                            _constructorcid)              \
      nsnull                                                              \
    };                                                                    \
    aDOMSOFactory->RegisterDOMClassInfo(#_class, nsnull, _proto,          \
                                        interfaces,                       \
                                        DOM_DEFAULT_SCRIPTABLE_FLAGS,     \
                                        _hasclassif, _constructorcid);    \
    return NS_OK;                                                         \
  }

#define NS_DOMCI_EXTENSION_ENTRY_END(_class, _proto, _hasclassif,         \
                                     _constructorcid)                     \
  NS_DOMCI_EXTENSION_ENTRY_END_HELPER(_class, &NS_GET_IID(_proto),        \
                                      _hasclassif, _constructorcid)

#define NS_DOMCI_EXTENSION_ENTRY_END_NO_PRIMARY_IF(_class, _hasclassif,   \
                                                   _constructorcid)       \
  NS_DOMCI_EXTENSION_ENTRY_END_HELPER(_class, nsnull, _hasclassif,        \
                                      _constructorcid)

#define NS_DOMCI_EXTENSION_END                                            \
  return NS_ERROR_FAILURE;                                                \
}


#endif /* nsIDOMClassInfo_h___ */
