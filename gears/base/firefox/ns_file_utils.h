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

#ifndef GEARS_BASE_FIREFOX_NS_FILE_UTILS_H__
#define GEARS_BASE_FIREFOX_NS_FILE_UTILS_H__

#include "gears/base/common/common_ff.h"

class nsIFile;
class nsIInputStream;
class nsIOutputStream;
class nsIFileProtocolHandler;

// Various File utilities lifted from gecko sources
class NSFileUtils {
 public:
  /**
   * Creates an input stream for the given file
   * @param file          - file to write to (must QI to nsILocalFile)
   * @param flags         - file open flags listed in prio.h
   * @param perm          - file mode bits listed in prio.h
   * @param behaviorFlags flags specifying various behaviors of the class
   *        (see enumerations in the nsIFileInputStream)
   */
   static nsresult NewLocalFileInputStream(nsIInputStream **result,
                                           nsIFile *file,
                                           PRInt32 flags = -1,
                                           PRInt32 perm = -1,
                                           PRInt32 behaviorFlags = 0);

  /**
   * Creates an output stream for the given file
   * @param file          - file to write to (must QI to nsILocalFile)
   * @param flags         - file open flags listed in prio.h
   * @param perm          - file mode bits listed in prio.h
   * @param behaviorFlags flags specifying various behaviors of the class
   *        (currently none supported)
   */
  static nsresult NewLocalFileOutputStream(nsIOutputStream **result,
                                           nsIFile *file,
                                           PRInt32 flags = -1,
                                           PRInt32 perm = -1,
                                           PRInt32 behaviorFlags = 0);

  // Creates a file object for the given file url
  static nsresult GetFileFromURLSpec(const nsAString  &url,
                                     nsIFile **result);

  // Creates a file object for the given file url
  static nsresult GetFileFromURLSpec(const nsACString &url_utf8,
                                     nsIFile **result);

  // Returns a reference to the file protocol handler
  static nsresult GetFileProtocolHandler(nsIFileProtocolHandler **result);
};

#endif // GEARS_BASE_FIREFOX_NS_FILE_UTILS_H__
