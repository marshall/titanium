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

var currentUrl = location.href;

var sameOriginPath =
  currentUrl.substring(0, 1 + currentUrl.lastIndexOf('/'));
var sameOriginWorkerFile = '../testcases/workerpool_same_origin.js';

// The cross origin path has the same host as currentUrl 
// with the port number incremented.
var currentPort = currentUrl.substring(currentUrl.lastIndexOf(':') + 1, 
                                       currentUrl.length);
currentPort = currentPort.substring(0, currentPort.indexOf('/'));
crossOriginPort = parseInt(currentPort) + 1;

var crossOriginPath = 
  currentUrl.substring(0, 1 + currentUrl.lastIndexOf(':')) +
  crossOriginPort + '/testcases/';
var crossOriginWorkerFile = 'workerpool_cross_origin.js';
var crossOriginWorkerFileNoPerms = 'workerpool_same_origin.js';

function testCreateWorkerFromUrl1() {
  startAsync();

  var wp = google.gears.factory.create('beta.workerpool');
  wp.onmessage = function(text, sender, m) {
    completeAsync();
  }
  var childId = wp.createWorkerFromUrl(sameOriginWorkerFile);
  wp.sendMessage('PING1', childId);
}

function testCreateWorkerFromUrl2() {
  startAsync();

  // Cleanup any local DB before starting test.  
  var db = google.gears.factory.create('beta.database');
  db.open('worker_js');
  db.execute('drop table if exists PING2').close();
  db.close();

  var wp = google.gears.factory.create('beta.workerpool');
  wp.onmessage = function(text, sender, m) {
    // Worker database SHOULD exist in parent origin.
    var db = google.gears.factory.create('beta.database');
    db.open('worker_js');
    var rs = db.execute('select * from sqlite_master where name = ? limit 1',
                        ['PING2']);
    handleResult(rs, function(rs) {
      assert(rs.isValidRow(), 'PING2 table should have been created');
    });
    db.close();
    completeAsync();
  };

  var childId = wp.createWorkerFromUrl(sameOriginPath + sameOriginWorkerFile);
  wp.sendMessage('PING2', childId);
}

function testCreateWorkerFromUrl3() {
  startAsync();

  // Cleanup any local DB before starting test.  
  var db = google.gears.factory.create('beta.database');
  db.open('worker_js');
  db.execute('drop table if exists PING3').close();
  db.close();

  var wp = google.gears.factory.create('beta.workerpool');
  wp.onmessage = function(text, sender, m) {
    // Worker database should NOT exist in parent origin.
    var db = google.gears.factory.create('beta.database');
    db.open('worker_js');
    var rs = db.execute('select * from sqlite_master where name = ? limit 1',
                        ['PING3']);
    handleResult(rs, function(rs) {
      assert(!rs.isValidRow(), 'PING3 table should not have been created');
    });
    db.close();
    completeAsync();
  };

  // TODO(cprince): In dbg builds, add a 2nd param to createWorkerFromUrl()
  // so callers can simulate a different origin without being online.
  //if (!gIsDebugBuild) {
  var childId = wp.createWorkerFromUrl(crossOriginPath + crossOriginWorkerFile);
  //} else {
  //  var childId = wp.createWorkerFromUrl(sameOriginPath +
  //                                            crossOriginWorkerFile,
  //                                        crossOriginPath);
  //}
  wp.sendMessage('PING3', childId);
}

// Test fails intermittently
// TODO(ace): Uncomment or remove this test case upon resolution of Issue 388
//function testCreateWorkerFromUrl4() {
//  var workerUrl = '/non-existent-file.js';
//
//  waitForGlobalErrors([workerUrl]);
//
//  var wp = google.gears.factory.create('beta.workerpool');
//  wp.createWorkerFromUrl(workerUrl);
//}

function testCreateWorkerFromUrl5() {
  var expectedError = 'Page does not have permission to use Appcelerator Titanium';
  
  var wp = google.gears.factory.create('beta.workerpool');
  // Have to keep a reference to the workerpool otherwise, sometimes the message
  // never gets processed!
  // TODO(aa): Investigate why this happens -- ThreadInfo objects are
  // AddRef()'ing the workerpool, so I would assume this shouldn't be possible.
  testCreateWorkerFromUrl5.wp = wp;

  waitForGlobalErrors([expectedError]);

  var childId = wp.createWorkerFromUrl(
    crossOriginPath + crossOriginWorkerFileNoPerms);
  // TODO(cprince): Could add debug-only origin override here too.
  wp.sendMessage('PING5', childId);
}

function testOneShotWorkerFromUrl() {
  // Not having a global reference to wp or any callbacks causes this
  // GearsWorkerPool instance to get GC'd before page unload. This found a bug
  // where the HttpRequest used to load from url was getting destroyed from a
  // different thread than it was created on.
  var wp = google.gears.factory.create('beta.workerpool');
  wp.createWorkerFromUrl(sameOriginWorkerFile);
}
