// Copyright 2007, Google Inc.
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

#ifndef GEARS_UI_IE_TOOLS_MENU_ITEM_H__
#define GEARS_UI_IE_TOOLS_MENU_ITEM_H__

#include "genfiles/interfaces.h"
#include "gears/base/ie/atl_headers.h"
#include "gears/base/ie/resource.h" // for .rgs resource ids (IDR_*)

// This class is used to add a menu items to the tools menu in Internet
// Explorer. We don't do very much of interest here, but there is a lot of
// required boilerplate code.
// For more detail, see:
// http://msdn.microsoft.com/library/default.asp?url=/workshop/browser/ext/tutorials/menu.asp
class ATL_NO_VTABLE ToolsMenuItem
    : public CComObjectRootEx<CComMultiThreadModel>,
      public CComCoClass<ToolsMenuItem, &CLSID_ToolsMenuItem>,
#ifdef WINCE
      public IObjectWithSiteImpl<ToolsMenuItem>,
      public IContextMenu {
#else
      public IOleCommandTarget {
#endif
 public:
  DECLARE_REGISTRY_RESOURCEID(IDR_TOOLSMENUITEM)
  DECLARE_NOT_AGGREGATABLE(ToolsMenuItem)
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  BEGIN_COM_MAP(ToolsMenuItem)
#ifdef WINCE
    COM_INTERFACE_ENTRY(IObjectWithSite)
    COM_INTERFACE_ENTRY_IID(IID_IContextMenu, IContextMenu)
#else
    COM_INTERFACE_ENTRY(IOleCommandTarget)
#endif
  END_COM_MAP()

#ifdef WINCE
  STDMETHOD(QueryContextMenu)(HMENU hmenu, UINT index_menu, UINT id_cmd_first, 
              UINT id_cmd_last, UINT flags);
  STDMETHOD(GetCommandString)(UINT id_cmd, UINT flags, UINT *reserved, 
              LPSTR command_name, UINT command_name_len);
  STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO command_info);
 private:
  int command_first_;
#else
 public:
  STDMETHOD(QueryStatus)(const GUID *command_group_id, ULONG num_commands,
                         OLECMD *commands, OLECMDTEXT *command_text);
  STDMETHOD(Exec)(const GUID *command_group_id, DWORD command_id, 
                  DWORD exec_options, VARIANTARG *args, VARIANTARG *output);
#endif
};
OBJECT_ENTRY_AUTO(__uuidof(ToolsMenuItem), ToolsMenuItem)

#endif  // GEARS_UI_IE_TOOLS_MENU_ITEM_H__
