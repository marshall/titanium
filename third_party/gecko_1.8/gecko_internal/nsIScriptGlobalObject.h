/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 sw=2 et tw=80: */
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
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

#ifndef nsIScriptGlobalObject_h__
#define nsIScriptGlobalObject_h__

#include "nsISupports.h"
#include "nsEvent.h"

class nsIScriptContext;
class nsIDOMDocument;
class nsIDOMEvent;
class nsPresContext;
class nsIDocShell;
class nsIDOMWindowInternal;
class nsIScriptGlobalObjectOwner;
struct JSObject;

#define NS_ISCRIPTGLOBALOBJECT_IID \
{ 0xd326a211, 0xdc31, 0x45c6, \
 { 0x98, 0x97, 0x22, 0x11, 0xea, 0xbc, 0xd0, 0x1c } }

/**
 * The JavaScript specific global object. This often used to store
 * per-window global state.
 */

class nsIScriptGlobalObject : public nsISupports
{
public:
  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISCRIPTGLOBALOBJECT_IID)

  virtual void SetContext(nsIScriptContext *aContext) = 0;
  virtual nsIScriptContext *GetContext() = 0;
  virtual nsresult SetNewDocument(nsIDOMDocument *aDocument,
                                  nsISupports *aState,
                                  PRBool aRemoveEventListeners,
                                  PRBool aClearScope) = 0;
  virtual void SetDocShell(nsIDocShell *aDocShell) = 0;
  virtual nsIDocShell *GetDocShell() = 0;
  virtual void SetOpenerWindow(nsIDOMWindowInternal *aOpener)=0;

  /**
   * Let the script global object know who its owner is.
   * The script global object should not addref the owner. It
   * will be told when the owner goes away.
   * @return NS_OK if the method is successful
   */
  virtual void SetGlobalObjectOwner(nsIScriptGlobalObjectOwner* aOwner) = 0;

  /**
   * Get the owner of the script global object. The method
   * addrefs the returned reference according to regular
   * XPCOM rules, even though the internal reference itself
   * is a "weak" reference.
   */
  virtual nsIScriptGlobalObjectOwner *GetGlobalObjectOwner() = 0;

  virtual nsresult HandleDOMEvent(nsPresContext* aPresContext, 
                                  nsEvent* aEvent, 
                                  nsIDOMEvent** aDOMEvent,
                                  PRUint32 aFlags,
                                  nsEventStatus* aEventStatus)=0;

  virtual JSObject *GetGlobalJSObject() = 0;

  /**
   * Called when the global JSObject is finalized
   */

  virtual void OnFinalize(JSObject *aJSObject) = 0;

  /**
   * Called to enable/disable scripts.
   */
  virtual void SetScriptsEnabled(PRBool aEnabled, PRBool aFireTimeouts) = 0;

  /**
   * Set a new arguments array for this window. This will be set on
   * the window right away (if there's an existing document) and it
   * will also be installed on the window when the next document is
   * loaded.  If argc is nonzero, argv must be non-null.
   *
   * @param aArgc the number of args
   * @param aArgv the pointer to the args.  This may be cast to jsval* and the
   *        args are found at
   *        ((jsval*)aArgv)[0], ..., ((jsval*)aArgv)[aArgc - 1]
   */
  virtual nsresult SetNewArguments(PRUint32 aArgc, void* aArgv) = 0;
};

#endif
