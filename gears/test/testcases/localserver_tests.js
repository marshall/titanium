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

var localServer = google.gears.factory.create('beta.localserver');
var STORE_NAME = 'unit_test';
var UPDATE_STATUS = {
  ok: 0,
  checking: 1,
  updating: 2,
  failure: 3
};

function getFreshStore() {
  if (localServer.openStore(STORE_NAME)) {
    localServer.removeStore(STORE_NAME);
  }
  return localServer.createStore(STORE_NAME);
}

function getFreshManagedStore() {
  if (localServer.openManagedStore(STORE_NAME)) {
    localServer.removeManagedStore(STORE_NAME);
  }
  return localServer.createManagedStore(STORE_NAME);
}

function updateManagedStore(url, callback) {
  var managedStore = getFreshManagedStore();
  managedStore.manifestUrl = url;

  managedStore.checkForUpdate();

  // Wait for the update to complete
  var timerId = timer.setInterval(function() {
    var status = managedStore.updateStatus;
    if (status == UPDATE_STATUS.ok ||
        status == UPDATE_STATUS.failure) {
      timer.clearInterval(timerId);
      callback(managedStore);
    }
  }, 50);
}

function testEmptyParams() {
  var shouldFail = [
    'localServer.createStore()',
    'localServer.createStore("")',
    'localServer.createStore(null)',
    'localServer.createStore(undefined)',
    'localServer.createManagedStore()',
    'localServer.createManagedStore("")',
    'localServer.createManagedStore(null)',
    'localServer.createManagedStore(undefined)'
  ];
  
  for (var i = 0, str; str = shouldFail[i]; i++) {
    assertError(function() {
      eval(str)
    }, null, 'Incorrectly allowed call "%s"'.subs(str));
  }
}

function testCaptureUrl() {
  var captureUri = '/testcases/test_file_1024.txt';
  var renameUri = '/testcases/renamed.txt';
  var expectedCaptureContent = '1111111111111111111111111111111111111111111111';
  var resourceStore = getFreshStore();

  assert(!resourceStore.isCaptured(captureUri),
         'test file should not be captured');

  startAsync();
  resourceStore.capture(captureUri, function(url, success, id) {
    assert(success, 'Original should have succeeded');
    assert(resourceStore.isCaptured(captureUri),
           'Original should have been captured');

    // Make a copy of it
    var copyUri = "/testcases/copied.txt";
    resourceStore.copy(captureUri, copyUri);
    assert(resourceStore.isCaptured(copyUri), 'Copy should have been captured');
    assert(resourceStore.isCaptured(captureUri),
           'Original should have been captured after copy');

    // Rename it
    resourceStore.rename(captureUri, renameUri);
    assert(resourceStore.isCaptured(renameUri),
           'Rename should have been captured');
    assert(!resourceStore.isCaptured(captureUri),
           'Original should not have been captured after rename');

    // Verify the local server claims to be able to serve renameUri now
    assert(localServer.canServeLocally(renameUri),
           'Rename should have been servable');

    // Fetch the contents of renameUri and see if its what we expect
    httpGet(renameUri, function(content) {
      var renameContent = content;
      assert(renameContent.startsWith(expectedCaptureContent),
             'Unexpected content in renamed resource');

      // Disable our store and verify we can no longer fetch renameUri
      // note: depends on renameUri not being available on the server)
      resourceStore.enabled = false;
      assert(!localServer.canServeLocally(renameUri),
             'Should not have been able to serve after disable');

      // Fetch and make sure disabled
      httpGet(renameUri, function(content) {
        var disabledContent = content;
        assertNull(disabledContent, 'Should not have served disabled content');

        // Now re-enable and try to redirect back into cache
        resourceStore.enabled = true;
        httpGet("testcases/cgi/server_redirect.py?location=" + renameUri,
          function(content) {
            var redirectedContent = content;
            assertEqual(renameContent, redirectedContent, 
                        'Redirected content should match');

            // Now remove the uris, and verify isCaptured() returns false.
            resourceStore.remove(renameUri);
            resourceStore.remove(copyUri);
            assert(!resourceStore.isCaptured(renameUri),
                   'Rename should not have been captured after remove');
            assert(!resourceStore.isCaptured(copyUri),
                   'Copy should not have been captured after remove');

            completeAsync();
          }
        );
      });
    });
  });
}

function testCaptureFragment() {
  var baseUri = '/testcases/test_file_fragment';
  var resourceStore = getFreshStore();

  startAsync();
  resourceStore.capture(baseUri + '#foo', function(url, success, id) {
    assert(success, 'Capture should have succeeded');
    assert(resourceStore.isCaptured(baseUri),
           'baseUri without fragment should be captured');
    assert(resourceStore.isCaptured(baseUri + '#foo'),
           '#foo should be captured');
    assert(resourceStore.isCaptured(baseUri + '#bar'),
           '#bar should be captured');
    completeAsync();
  });
}

function testCaptureMany() {
  var urls = {
    "/testcases/test_file_0.txt": 0,
    "/testcases/nonexistent_file": -1,  // should fail
    "/testcases/test_file_1.txt": 1,
    "/testcases/cgi/server_redirect.py?location=/testcases/nonexistent_file": -1,
    "/testcases/test_file_1024.txt": 1024,
    "/testcases/cgi/send_response_of_size.py?size=10": 10,
    "/testcases/cgi/send_response_of_invalid_size.py?size=10": -2
  };

  var resourceStore = getFreshStore();
  var captureCompleteCount = 0;
  var urlList = getObjectProps(urls);
  var results = {};

  startAsync();

  // Capture all the URLs
  resourceStore.capture(urlList, function(url, success, id) {
    assert(!(url in results),
           'Callback called more than once for url "%s"'.subs(url));
    results[url] = 1;

    if (urls[url] < 0) {
      assert(!success, 'Capture of "%s" should have failed'.subs(url));
    } else {
      assert(success, 'Capture of "%s" should have succeeded'.subs(url));
    }

    // Once they are all complete, fetch them all to make sure they were
    // captured correctly.
    ++captureCompleteCount;
    if (captureCompleteCount == urlList.length) {
      fetchNextUrl();
    }
  });

  function fetchNextUrl() {
    var url = urlList.shift();

    if (!url) {
      completeAsync();
      return;
    }

    httpGet(url, function(content) {
      if (urls[url] == -1) {
        assertNull(content,
                   'Should not have been able to fetch "%s"'.subs(url));
      } else if (urls[url] >= 0) {
        assertNotNull(content, 'Should have been able to fetch "%s"'.subs(url));
        assertEqual('text/plain', resourceStore.getHeader(url, "Content-Type"),
                    'Wrong contentType for url "%s"'.subs(url));
        assertEqual(urls[url], content.length,
                   'Wrong content length for url "%s"'.subs(url));
      }

      fetchNextUrl();
    });
  }
}

function testCaptureCrossDomain() {
  var resourceStore = getFreshStore();

  assertError(function() {
    resourceStore.capture('http://cross.domain.not/',
        function(url, success, id) {
      assert(false, 'Should not have fired callback');
    });
  }, null, 'Should have thrown error trying to capture cross-domain resource');
}

function testCaptureWithNullCallback() {	
  var resourceStore = getFreshStore();	
  resourceStore.capture('/testcases/nonexistent_file', null);	
}

function testPostRedirectBackToCache() {
  var captureUri = '/testcases/test_file_1024.txt';
  var renameUri = '/testcases/renamed_not_on_server.txt';
  var resourceStore = getFreshStore();

  startAsync();

  // capture something
  resourceStore.capture(captureUri, function(url, success, id) {
    assert(success, 'capture failed');

    // rename it to something that does not exist on the server
    resourceStore.rename(captureUri, renameUri);

    // now send a POST that redirects to it
    httpPost("/testcases/cgi/server_redirect.py?location=" + renameUri,
      "ignored data string",
      function(content) {
        assert(content != null, 'Should have redirected to our cache');
        completeAsync();
      });
  });
}

function testCaptureInstantAbort() {
  var urls = [
    "/testcases/test_file_0.txt",
    "/testcases/nonexistent_file",  // should fail
    "/testcases/test_file_1.txt",
    "/testcases/cgi/server_redirect.py?location=/testcases/nonexistent_file",
    "/testcases/test_file_1024.txt"
  ];
  var resourceStore = getFreshStore();
  var captureCompleteCount = 0;

  startAsync();
  var captureId = resourceStore.capture(urls, function(url, success, id) {
    assert(!success, 'Capture should have been aborted');
    assert(!resourceStore.isCaptured(url), url+' should not be captured');
    captureCompleteCount++;
    if (captureCompleteCount == urls.length) {
      completeAsync();
    }
  });
  resourceStore.abortCapture(captureId);
}

function testCaptureDeferredAbort() {
  var urls = [
    "/testcases/test_file_0.txt",
    "/testcases/test_file_1.txt",
    "/testcases/test_file_fragment",
    "/testcases/test_file_1024.txt"
  ];
  var resourceStore = getFreshStore();
  var captureCompleteCount = 0;

  startAsync();
  resourceStore.capture(urls, function(url, success, id) {
    captureCompleteCount++;
    if (captureCompleteCount == 2) {
      resourceStore.abortCapture(id);
    }
    if (captureCompleteCount == urls.length) {
      completeAsync();
    }
  });
}

function testCaptureBlob() {
 startAsync();
 var url = '/testcases/test_file_1024.txt';
 httpGetAsRequest(url, function(request) {
   var store = getFreshStore();
   var captureUrl = '/captured.txt';
   var responseBlob = request.responseBlob;
   store.captureBlob(responseBlob, captureUrl);
   httpGetAsRequest(captureUrl, function(request2) {
     assertBlobProbablyEqual(responseBlob, request2.responseBlob);
     completeAsync();
     });
   });
}

function testGoodManifest() {
  startAsync();

  // First, fetch url1's contents for later comparison
  httpGet("/testcases/manifest-url1.txt", function(content) {
    var expectedUrl1Content = content;
    
    // Then, capture a manifest containing many references to that URL
    updateManagedStore("/testcases/manifest-good.txt", function(managedStore) {
      assertEqual(UPDATE_STATUS.ok, managedStore.updateStatus,
                  'updateStatus should be OK after good manifest');

      assertEqual('1', managedStore.currentVersion,
          'currentVersion should reflect the value in the manifest file');

      // TODO(aa): Would be cool if we could actually return null in this case
      assertEqual('', managedStore.lastErrorMessage,
          'lastErrorMessage should be empty string after good manifest');

      var testUrls = [
        '/testcases/manifest-url1.txt',
        '/testcases/manifest-url1.txt?query',
        '/testcases/alias-to-manifest-url1.txt',
        '/testcases/redirect-to-manifest-url1.txt',
        '/testcases/unicode?foo=bar'
      ];

      for (var i = 0; i < testUrls.length; i++) {
        assert(localServer.canServeLocally(testUrls[i]),
               'Should be able to serve "%s" locally'.subs(testUrls[i]));
      }   
      
      var httpGetCount = 0;
      
      for (var i = 0; i < testUrls.length; i++) {
        var nextUrl = testUrls[i];
        httpGet(nextUrl, function(content) {
          httpGetCount++;
          assertEqual(expectedUrl1Content, content, 
                      'Incorrect content for url "%s"'.subs(nextUrl));
          if (httpGetCount == testUrls.length) {
            completeAsync();
          }
        });
      }
      
    });
  });
}

function testBadManifest() {
  startAsync();
  
  updateManagedStore("/testcases/manifest-bad.txt", function(managedStore) {
    assertEqual(UPDATE_STATUS.failure, managedStore.updateStatus,
                'updateStatus should be FAILED after bad manifest');

    var message = managedStore.lastErrorMessage;
    assert(message.indexOf("Download of") != -1 &&
               message.indexOf("failed") != -1 &&
               message.indexOf("404") != -1, 
           'Incorrect lastErrorMessage after bad manifest');

    completeAsync();
  });
}

function testInvalidManifest() {
  startAsync();

  updateManagedStore('/testcases/manifest-ugly.txt', function(managedStore) {
    assertEqual(UPDATE_STATUS.failure, managedStore.updateStatus,
                'updateStatus should be FAILED after ivalid manifest');

    assert(managedStore.lastErrorMessage.startsWith("Invalid manifest"),
           'Incorrect lastErrorMessage after invalid manifest');

    completeAsync();
  });
}

function testIllegalRedirectManifest() {
  startAsync();

  updateManagedStore('/testcases/manifest-illegal-redirect.txt',
    function(managedStore) {
      assertEqual(
        UPDATE_STATUS.failure, managedStore.updateStatus,
        'updateStatus should be FAILED after illegal-redirect manifest');

      assert(managedStore.lastErrorMessage.indexOf('302') > -1,
             'Incorrect lastErrorMessage after illegal-redirect manifest');

      completeAsync();
    }
  );
}

function testManagedResourceStoreCallbacks() {
  startAsync();
  var progress = 0;
  var FILES_TOTAL = 3;
  var managedStore = getFreshManagedStore();
  managedStore.manifestUrl = '/testcases/manifest-good.txt';

  managedStore.onprogress = function(e) {
    assert(e.filesTotal == FILES_TOTAL, 'Wrong filesTotal in onprogress.');
    assert(e.filesComplete <= FILES_TOTAL,
           'filesComplete out of range in onprogress');
    progress += 1;
  };
  
  managedStore.oncomplete = function(e) {
    assert(e.newVersion == '1', 'Incorrect version in oncomplete.');
    assert(progress == FILES_TOTAL + 1,
           'onprogress called incorrect number of times.');
    completeAsync();
  };

  managedStore.checkForUpdate();
}

function testManagedResourceStoreThreads() {
  startAsync();
  var progress = 0;
  var FILES_TOTAL = 3;
  var managedStore = getFreshManagedStore();
  managedStore.manifestUrl = '/testcases/manifest-good.txt';

  managedStore.onprogress = function(e) {
    assert(e.filesTotal == FILES_TOTAL, 'Wrong filesTotal in onprogress.');
    assert(e.filesComplete <= FILES_TOTAL,
           'filesComplete out of range in onprogress');
    progress += 1;
  };

  managedStore.oncomplete = function(e) {
    assert(e.newVersion == '1', 'Incorrect version in oncomplete.');
    assert(progress == FILES_TOTAL + 1,
           'onprogress called incorrect number of times.');
    completeAsync();
  };

  var workerpool = google.gears.factory.create('beta.workerpool');
  var workerId = workerpool.createWorker(String(workerInit) +
                                         String(workerOnMessage) +
                                         'workerInit();');

  workerpool.sendMessage(managedStore.name, workerId);

  function workerInit() {
    google.gears.workerPool.onmessage = workerOnMessage;
  }

  function workerOnMessage(text, sender, m) {
    var localserver = google.gears.factory.create('beta.localserver');
    var managedStore = localserver.openManagedStore(text);
    managedStore.checkForUpdate();
  }
}

function testManagedResourceStoreErrorCallback() {
  startAsync();
  var managedStore = getFreshManagedStore();
  managedStore.manifestUrl = '/testcases/manifest-bad.txt';

  managedStore.onerror = function(e) {
    assert(e.message.startsWith('Download of'), 'Error string was incorrect.');
    completeAsync();
  }

  managedStore.checkForUpdate();
}

function testManagedResourceStoreErrorThreads() {
  startAsync();
  var managedStore = getFreshManagedStore();
  managedStore.manifestUrl = '/testcases/manifest-bad.txt';

  managedStore.onerror = function(e) {
    assert(e.message.startsWith('Download of'), 'Error string was incorrect.');
    completeAsync();
  }

  var workerpool = google.gears.factory.create('beta.workerpool');
  var workerId = workerpool.createWorker(String(workerInit) +
                                         String(workerOnMessage) +
                                         'workerInit();');

  workerpool.sendMessage(managedStore.name, workerId);

  function workerInit() {
    google.gears.workerPool.onmessage = workerOnMessage;
  }

  function workerOnMessage(text, sender, m) {
    var localserver = google.gears.factory.create('beta.localserver');
    var managedStore = localserver.openManagedStore(text);
    managedStore.checkForUpdate();
  }
}

function testManagedResourceStoreInvalidContentLength() {
  startAsync();
  var managedStore = getFreshManagedStore();
  managedStore.manifestUrl = '/testcases/manifest-invalid-content-length.txt';

  managedStore.onerror = function(e) {
    assert(e.message.startsWith('Download of'), 'Error string was incorrect.');
    completeAsync();
  }

  managedStore.checkForUpdate();
}
