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

// This JavaScript file is loaded into the worker thread.

// injected into the namespace by Gears
var factory = google.gears.factory;
// injected into the namespace by Gears
var workerPool = google.gears.workerPool;
var localServer;
var store;
var mainThreadId;  // set on receipt of our first message
                   // would be nice to provide this via thread mgr API
var STORE_NAME = "thread-test-store";
var capture1Ok = false;
var capture2Ok = false;
var capture2Count = 0;


// Setup to receive thread messages
workerPool.onmessage = OnWorkerPoolMessage;

function OnWorkerPoolMessage(msg, sender) {
  switch(msg) {
    case "Init":
      try {
        mainThreadId = sender;
        localServer = factory.create('beta.localserver', '1.0');
        Send("Init OK");
      } catch(e) {
        Send("Init exception - " + e);
      }
      break;
    case "TestLocalServer":
      TestLocalServer();
      break;
    case "TestResourceStore":
      TestResourceStore();
      break;
    case "TestManagedResourceStore":
      Send("TestManagedResourceStore - not implemented");
      break;
    default:
      Send("Unknown message " + msg);
      break;
  }
}

function Send(msg) {
  workerPool.sendMessage(msg, mainThreadId);
}

function TestLocalServer() {
  try {
    var testUrl = "notCaptured.url";

    if (localServer.canServeLocally(testUrl))
      throw "localServer.canServeLocally return true, expecting false";

    store = localServer.createStore(STORE_NAME);
    if (!store)
      throw "localServer.createStore failed";

    Send("TestLocalServer: domain = " + store.domain);

    localServer.removeStore(STORE_NAME);

    store = localServer.openStore(STORE_NAME);
    if (store)
      throw "localServer.openStore returned something, expecting null";

    store = localServer.createManagedStore(STORE_NAME);
    if (!store)
      throw "localServer.createManagedStore failed";

    localServer.removeManagedStore(STORE_NAME);

    store = localServer.openManagedStore(STORE_NAME);
    if (store)
      throw "localServer.openManagedStore returned something, " +
            "expecting null";

    Send("TestLocalServer OK");
  } catch (ex) {
    Send("TestLocalServer exception - " + ex);
  }
}


function VerifyFunctionThrows(func) {
  try {
    func();
    return false;
  } catch (e) {
    return true;
  }
}

function TestResourceStore() {
  try {
    var testUrl = "threaded_webcapture_test.js";

    store = localServer.createStore(STORE_NAME);
    if (store.isCaptured(testUrl)) {
      throw "store.isCaptured returned true, expecting false";
    }

    // Verify we cannot createFileSubmitter or call captureFile
    // on a worker thread

    var func = function() { store.createFileSubmitter(); }
    if (!VerifyFunctionThrows(func)) {
      throw "createFileSubmitter failed to throw an exception";
    }

    func = function() {
      var fileInputSpoofer = {
        type: "file",
        value: "c:\\autoexec.bat",
        QueryInterface: function(iid) {
          return this;
        }
      };
      store.captureFile(fileInputSpoofer, "shouldFail");
    };
    if (!VerifyFunctionThrows (func)) {
      throw "captureFile(fileInputSpoofer) failed to throw an exception";
    }

    func = function() { store.captureFile("c:\\autoexec.bat", "shouldFail"); }
    if (!VerifyFunctionThrows(func)) {
      throw "captureFile(string) failed to throw an exception";
    }

    capture1Ok = false;
    capture2Ok = false;
    capture2Count = 0;

    // test where 1st param in capture() is a single string
    store.capture(testUrl, CaptureCallback1);

    Send("TestResourceStore - initiated");
  } catch (ex) {
    Send("TestResourceStore exception - " + ex);
  }
}

function CaptureCallback1(url, success, id) {
  try {
    capture1Ok = success;

    Send("CaptureCallback1: " + url + ", " + success + ", " + id);

    // test where 1st param in capture() is an array of strings
    var testUrls = ["threaded_webcapture_test.html", "shouldFailToCapture"];
    store.capture(testUrls, CaptureCallback2);

  } catch (ex) {
    Send("CaptureCallback1 exception - " + ex);
  }
}

function CaptureCallback2(url, success, id) {
  try {
    Send("CaptureCallback2: " + url + ", " + success + ", " + id);

    ++capture2Count;
    if (capture2Count == 1) {
      capture2Ok = success;
      return;
    } else {

      // the second url should fail
      capture2Ok = capture2Ok && !success;
      Send("TestResourceStore - " +
           ((capture1Ok && capture2Ok) ? "OK" : "FAILED"));

      localServer.removeStore(STORE_NAME);
      Send("All done");
    }
  } catch (ex) {
    Send("CaptureCallback2 exception - " + ex);
  }
}

