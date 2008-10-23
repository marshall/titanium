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
 *   Bradley Baetz <bbaetz@student.usyd.edu.au>
 *   Malcolm Smith <malsmith@cs.rmit.edu.au>
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

#include <gecko_sdk/include/nsILocalFile.h>
#include <gecko_sdk/include/nsIIOService.h>
#include <gecko_sdk/include/nsIInputStream.h>
#include <gecko_internal/nsIFileProtocolHandler.h>
#include <gecko_internal/nsIFileStreams.h>
#include "gears/base/firefox/ns_file_utils.h"

#define NS_LOCALFILEINPUTSTREAM_CONTRACTID \
    "@mozilla.org/network/file-input-stream;1"

#define NS_LOCALFILEOUTPUTSTREAM_CONTRACTID \
    "@mozilla.org/network/file-output-stream;1"


// From nsNetUtil.h, NS_NewLocalFileInputStream
// Ask FF to create a file stream to read a local file.
nsresult NSFileUtils::NewLocalFileInputStream(nsIInputStream **aResult,
                                              nsIFile *aFile,
                                              PRInt32 aIOFlags,
                                              PRInt32 aPerm,
                                              PRInt32 aBehaviorFlags) {
  nsresult rv;
  nsCOMPtr<nsIFileInputStream> in =
      do_CreateInstance(NS_LOCALFILEINPUTSTREAM_CONTRACTID, &rv);
  if (NS_SUCCEEDED(rv)) {
    rv = in->Init(aFile, aIOFlags, aPerm, aBehaviorFlags);
    if (NS_SUCCEEDED(rv))
      NS_ADDREF(*aResult = in);
  }
  return rv;
}

// From nsNetUtil.h, NS_NewLocalFileOutputStream
// Ask FF to create a file stream to write a local file.
nsresult NSFileUtils::NewLocalFileOutputStream(nsIOutputStream **aResult,
                                               nsIFile *aFile,
                                               PRInt32 aIOFlags,
                                               PRInt32 aPerm,
                                               PRInt32 aBehaviorFlags) {
  nsresult rv;
  nsCOMPtr<nsIFileOutputStream> out =
      do_CreateInstance(NS_LOCALFILEOUTPUTSTREAM_CONTRACTID, &rv);
  if (NS_SUCCEEDED(rv)) {
    rv = out->Init(aFile, aIOFlags, aPerm, aBehaviorFlags);
    if (NS_SUCCEEDED(rv))
      NS_ADDREF(*aResult = out);
  }
  return rv;
}


// From nsNetUtil.h, NS_GetFileProtocolHandler
nsresult NSFileUtils::GetFileProtocolHandler(nsIFileProtocolHandler **result) {
  nsresult rv = NS_OK;
  nsCOMPtr<nsIIOService> ioservice =
      do_GetService("@mozilla.org/network/io-service;1");
  NS_ENSURE_STATE(ioservice);
  if (ioservice) {
    nsCOMPtr<nsIProtocolHandler> handler;
    rv = ioservice->GetProtocolHandler("file", getter_AddRefs(handler));
    if (NS_SUCCEEDED(rv))
      rv = CallQueryInterface(handler, result);
  }
  return rv;
}

// Contributed by Michael Nordman
nsresult NSFileUtils::GetFileFromURLSpec(const nsAString &inURL,
                                         nsIFile **result) {
  nsCString url_utf8;
  nsresult rv = NS_UTF16ToCString(inURL, NS_CSTRING_ENCODING_UTF8, url_utf8);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = GetFileFromURLSpec(url_utf8, result);
  return rv;
}


// From nsNetUtil.h, GetFileFromURLSpec
nsresult NSFileUtils::GetFileFromURLSpec(const nsACString &inURL,
                                         nsIFile **result) {
  nsresult rv;
  nsCOMPtr<nsIFileProtocolHandler> handler;
  rv = GetFileProtocolHandler(getter_AddRefs(handler));
  if (NS_SUCCEEDED(rv))
    rv = handler->GetFileFromURLSpec(inURL, result);
  return rv;
}
