<?
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

// This PHP file initializes a new client by assinging a client ID and returning
// the current version of the note.

require("_functions.php");
require("_database.php");

$id = validateUserCookie();
$id = db_escape($id);

// Create a new client ID.
// We use a MySQL variable to capture the value that next_client_id had before
// the update so that we are atomic. Variables are connection-specific, and we
// open a new connection for each PHP page view.
db_query_set("update user set next_client_id = next_client_id + 1 
              where id = '$id' and @prev_client_id := next_client_id");

$client = firstRow(db_query_get("select @prev_client_id as prev_client_id"));
$client = $client['prev_client_id'];

// Now get the latest version of the note from the database.
$rslt = firstRow(db_query_get("select version, content from user
                               where id = '$id'"));
$version = $rslt['version'];
$content = $rslt['content'];

print "$client\n$version\n$content";
