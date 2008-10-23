/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 * 
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * The Original Code is Komodo code.
 * 
 * The Initial Developer of the Original Code is ActiveState Software Inc.
 * Portions created by ActiveState Software Inc are Copyright (C) 2000-2007
 * ActiveState Software Inc. All Rights Reserved.
 * 
 * Contributor(s):
 *   ActiveState Software Inc
 * 
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

#ifndef __SCIMOZ_EVENTS__
#define __SCIMOZ_EVENTS__

#include "ISciMozEvents.h"

// A linked-list of all event listeners.
class EventListener {
public:
	EventListener(ISciMozEvents *listener, bool tryWeakRef, uint32 _mask) {
		mask = _mask;
		bIsWeak = PR_FALSE;
		if (tryWeakRef) {
			nsCOMPtr<nsISupportsWeakReference> wr = do_QueryInterface(listener);
			if (wr) {
				wr->GetWeakReference((nsIWeakReference **)&pListener);
				bIsWeak = PR_TRUE;
			}
		}
		if (!bIsWeak)
			listener->QueryInterface(NS_GET_IID(ISciMozEvents), (void **)&pListener);
		pNext = NULL;
	}
	~EventListener() {
		if (pListener) pListener->Release();
	}
  
	// Does one EventListener equal another???
	bool Equals(ISciMozEvents *anotherListener) {
		ISciMozEvents *pret;
		get(&pret);
		return anotherListener == pret;
	}

	void * get(ISciMozEvents **pret) {
		if (bIsWeak) {
			nsIWeakReference *pw = (nsIWeakReference *)pListener;
			if (NS_SUCCEEDED(pw->QueryReferent(NS_GET_IID(ISciMozEvents), (void **)pret)))
				return (void *)this;
		} else {
			pListener->AddRef();
			*pret = (ISciMozEvents *)pListener;
			return (void *)this;
		}
		return nsnull;
	}


	ISciMozEvents *pListener;
	int32 mask;
	EventListener *pNext;
	bool bIsWeak;
};

class EventListeners {
public:
	EventListeners() {pFirst = nsnull;}
	~EventListeners() {
		EventListener *pLook = pFirst;
		while (pLook) {
			EventListener *pTemp = pLook->pNext;
			delete pLook;
			pLook = pTemp;
		}
	}
	NPError Add( ISciMozEvents *listener, bool tryWeakRef, uint32 mask) {
		EventListener *l = new EventListener(listener, tryWeakRef, mask);
		if (!l || l->pListener == NULL) {
			delete l;
			return NPERR_GENERIC_ERROR;
		}
		l->pNext = pFirst;
		pFirst = l;
		return NPERR_NO_ERROR;
	}
	NPError Remove( ISciMozEvents *listener ) {
		// No real need to check for weak-reference support.
		// If someone added a weak-reference, then almost
		// by definition they dont want to manage the lifetime
		// themselves.
		EventListener *l = pFirst, *last = nsnull;

		for (;l;l=l->pNext) {
			// KenS: We check for identity using the Equals() member of EventListener
			if (l->Equals(listener)) {
				if (last==nsnull)
					pFirst = l->pNext;
				else
					last->pNext = l->pNext;
				delete l;
				return NPERR_NO_ERROR;
			}
			last = l;
		}
		NS_WARNING("Attempt to remove a listener that does not exist!\n");
		return NPERR_GENERIC_ERROR;
	}
	void *GetNext(int32 lookMask, void *from, ISciMozEvents **pret) {
		EventListener *l = from ? ((EventListener *)from)->pNext : pFirst;
		for (;l;l=l->pNext) {
			if (l->mask & lookMask) {
				return l->get(pret);
			}
		}
		return nsnull;
	}
protected:
	EventListener *pFirst;
};

#endif
