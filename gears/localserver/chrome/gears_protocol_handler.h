// Copyright 2008, Google Inc.
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

#ifndef GEARS_LOCALSERVER_CHROME_GEARS_PROTOCOL_HANDLER_H__
#define GEARS_LOCALSERVER_CHROME_GEARS_PROTOCOL_HANDLER_H__

#include "gears/base/common/string16.h"
#include "gears/localserver/common/localserver_db.h"

// This class handles service of a special "gears" protocol.  It can serve
// responses for URLs that look like:
//   gears://source/path
// Currently, the only source that is handled is "resources", which will be
// loaded from the library resource with the same name as 'path'.  An example is
// "gears://resources/permissions_dialog.html".
class GearsProtocolHandler {
 public:
  // Fills in the data for the given URL into 'payload', and returns true on
  // success.
  static bool Service(const std::string16 &url,
                      WebCacheDB::PayloadInfo *payload);
};

// The gears protocol name.
extern const char kGearsScheme[];

#endif  // GEARS_LOCALSERVER_CHROME_GEARS_PROTOCOL_HANDLER_H__
