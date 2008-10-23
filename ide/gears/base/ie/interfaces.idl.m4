// Copyright 2006, Google Inc.
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
//
// Instructions for defining a new interface:
// - Generate a GUID; this will be your interface GUID.
// - Copy an existing module's ".idl" file. Replace 'uuid' with your interface
//     GUID. Update the interface name. Define your properties/methods.
//
// If your interface needs to be returned as IDispatch instead of FooInterface:
// - Generate a second GUID; this will be your class GUID.
// - Copy an appropriate "uuid ... coclass" block below. Replace 'uuid' with
//     your class GUID. Update the interface name.
//
// Additionally, if you want to let callers use "new ActiveXObject" to
// instantiate this interface:
// - Copy an existing module's ".rgs" file. Replace the CLSID values with your
//     class GUID. Update the inteface name. But leave the TypeLib value as it
//     is. (There is only one type library for all our interfaces; see below.)
// - Update your class definition, adding the DECLARE_REGISTRY_RESOURCEID and
//     OBJECT_ENTRY_AUTO macros, and adding CLSID_Foo as the second parameter
//     to CComCoClass.  (See GearsFactory for an example.)

import "oaidl.idl";
import "ocidl.idl";

import "ui/ie/html_dialog_host.idl";

#ifdef WINCE
import "ui/ie/html_dialog_bridge_iemobile.idl";
import "ui/ie/html_dialog_host_iemobile.idl";
#endif

// The ModuleWrapper C++ class implements GearsModuleProviderInterface in
// order to let us distinguish VARIANTs that are ModuleWrappers from other
// VARIANTs, and convert from a VARIANT* to a ModuleWrapper*.
[
  object,
  uuid(edf6d581-3eab-3305-7598-18e0cd19d7b6),
  nonextensible,
  pointer_default(unique)
]
interface GearsModuleProviderInterface : IUnknown {
  // retval->byref is assigned a pointer to the C++ ModuleWrapper object.
  [propget] HRESULT moduleWrapper([out, retval] VARIANT *retval);
};

//------------------------------------------------------------------------------
// GearsTypelib
//------------------------------------------------------------------------------
[
  uuid(7708913A-B86C-4D91-B325-657DD5363433),
  version(1.0)
]
library GearsTypelib
{
  importlib("stdole2.tlb");

  [
    uuid(E0FEFE40-FBF9-42AE-BA58-794CA7E3FB53)
  ]
  coclass BrowserHelperObject
  {
    [default] interface IUnknown;
  };

  [
    uuid(619C4FDA-4D52-4C7C-BAF2-5654DA16E675)
  ]
  coclass HtmlDialogHost
  {
    [default] interface HtmlDialogHostInterface;
  };

#ifdef WINCE
  [
    uuid(134AB400-1A81-4fc8-85DD-29CD51E9D6DE)
  ]
  coclass PIEDialogBridge
  {
    [default] interface PIEDialogBridgeInterface;
  };
  
  [
    uuid(F5CE1289-2B44-4628-89DC-F440542AFE99)
  ]
  coclass PIEDialogHost
  {
    [default] interface PIEDialogHostInterface;
  };
  
#endif

  [
    uuid(0B4350D1-055F-47A3-B112-5F2F2B0D6F08)
  ]
  coclass ToolsMenuItem
  {
    [default] interface IUnknown;
  };

  [
    uuid(09371E80-6AB5-4341-81E8-BFF3FB8CC749)
  ]
  coclass ModuleWrapper
  {
    [default] interface IDispatch;
  };

  // This COM class is what gets created when one calls
  // new ActiveXObject("Gears.Factory") in JavaScript.
  [
    uuid(C93A7319-17B3-4504-87CD-03EFC6103E6E)
  ]
  coclass GearsFactory
  {
    [default] interface IDispatch;
  };
};
