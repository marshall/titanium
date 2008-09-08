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


// TODO(aa): There needs to be a way to get the cross-origin URL from inside a
// test. Or, more generally, there needs to be some test configuration that gets
// setup in the main frame, and passed into both iframe and worker contexts.
var crossOriginUrl = 'http://localhost:8002';
crossOriginUrl += '/testcases/test_file_1024.txt';

// TODO(bgarcia): Add a test that examines the responseText after the request
// has entered the interactive state, but before any part of the response has
// been received.

function testGet200() {
  startAsync();
  doRequest(
      '/testcases/test_file_1.txt', 'GET', null, null, // url, method, data, reqHeaders[]
      200, '1', null, 1);  // expected status, responseText, responseHeaders[], responseLength
}

function testPost200() {
  startAsync();

  var data = 'hello';
  var headers = [["Name1", "Value1"],
                 ["Name2", "Value2"]];
  var expectedHeaders = getExpectedEchoHeaders(headers);
  expectedHeaders.push(["echo-Content-Type", "text/plain"]);

  doRequest('testcases/cgi/echo_request.py', 'POST', data, headers, 200, data,
            expectedHeaders, data.length);
}

function testPostBlob200() {
  startAsync();

  var data = 'This is not a valid manifest!\n';
  var headers = [["Name1", "Value1"],
                 ["Name2", "Value2"]];
  var expectedHeaders = getExpectedEchoHeaders(headers);
  expectedHeaders.push(["echo-Content-Type", "application/octet-stream"]);

  // Obtain a blob so that we can upload it.
  httpGetAsRequest('/testcases/manifest-ugly.txt', function(request) {
    assertEqual(200, request.status, 'Failed to locate test blob');
    assert(isObject(request.responseBlob), 'Failed to download test blob');
    assertEqual(data.length, request.responseBlob.length,
                'Unexpected length of test blob');

    doRequest('testcases/cgi/echo_request.py', 'POST', request.responseBlob,
              headers, 200, data, expectedHeaders, data.length);
  });
}

function testPostBlobFail() {
  // FailBlobs are only available in debug builds.
  if (isDebug) {
    startAsync();
    var blob = google.gears.factory.create('beta.failblob', '1.0', 500);
    doRequest('testcases/cgi/echo_request.py', 'POST', blob,
              null, 0, null, null, null);
  }
}

function testPost302_200() {
  // A POST that gets redirected should GET the new location
  startAsync();

  var data = 'hello';
  var expectedHeaders = [["echo-Method", "GET"]];

  doRequest('/testcases/cgi/server_redirect.py?location=/testcases/cgi/echo_request.py', 'POST', data,
            null, 200, null, expectedHeaders, null);
}

function testGet404() {
  startAsync();
  doRequest('nosuchfile___', 'GET', null, null, 404, null, null);
}

function testGet302_200() {
  startAsync();
  doRequest('testcases/cgi/server_redirect.py?location=/testcases/test_file_1.txt', 'GET', null,
            null, 200, '1', null, 1);
}

function testGet302_404() {
  startAsync();
  doRequest('testcases/cgi/server_redirect.py?location=nosuchfile___', 'GET', null, null,
            404, null, null, null);
}  

function testGetNoCrossOrigin() {
  assertError(function() {
    doRequest(crossOriginUrl, 'GET', null, null, 0, null, null, null);
  });
}

function testGet302NoCrossOrigin() {
  startAsync();
  var headers = [["location", crossOriginUrl]];
  doRequest('testcases/cgi/server_redirect.py?location=' + crossOriginUrl,
            'GET', null, null, 302, "", headers, 0);
}

function testRequestDisallowedHeaders() {
  var headers = [["Referer", "http://somewhere.else.com/"]];
  assertError(function() {
    doRequest('should_fail', 'GET', null, headers, null, null, null, null);
  });
}

function testRequestReuse() {
  startAsync();

  var reusedRequest = google.gears.factory.create('beta.httprequest');
  var numGot = 0;
  var numToGet = 2;

  getOne();
  
  function getOne() {          
    var url = 'testcases/cgi/echo_request.py?' + numGot;
    reusedRequest.onreadystatechange = function() {
      if (reusedRequest.readyState == 4) {
        assertEqual(200, reusedRequest.status);
        ++numGot;
        
        if (numGot == numToGet) {
          completeAsync();
        } else  if (numGot > numToGet) {
          assert(false, 'got too many');
        } else {
          getOne();
        }
      }
    };
    reusedRequest.open('GET', url, true);
    reusedRequest.send(null);
  }
}

function testAbortSimple() {
  var request = google.gears.factory.create('beta.httprequest');
  request.abort();
}

function testAbortAfterOpen() {
  var urlbase = '/testcases/cgi/send_response_of_size.py?size=';
  var request = google.gears.factory.create('beta.httprequest');
  request.open('GET', urlbase + 1, true);
  request.abort();
}

function testAbortAfterSend() {
  var urlbase = '/testcases/cgi/send_response_of_size.py?size=';
  var request = google.gears.factory.create('beta.httprequest');
  request.open('GET', urlbase + 2, true);
  request.send();
  request.abort();
}


function testAbortAfterInteractive() {
  startAsync();
  var urlbase = '/testcases/cgi/send_response_of_size.py?write_slowly=1&size=';
  var request = google.gears.factory.create('beta.httprequest');
  request.open('GET', urlbase + 1000000, true);
  request.onreadystatechange = function() {
    if (request.readyState >= 3) {
      assertEqual(3, request.readyState);  // we dont want it to be complete yet
      request.abort();
      completeAsync();
    }
  }
  request.send();
}



function testAbortWithReusedObject() {
  startAsync();

  var urlbase = '/testcases/cgi/send_response_of_size.py?write_slowly=1&size=';
  var request = google.gears.factory.create('beta.httprequest');

  // just call abort
  request.abort();

  // reusing the same GHR: open then abort
  request.open('GET', urlbase + 1, true);
  request.abort();

  // reusing the same GHR: open, send, and then abort
  request.open('GET', urlbase + 2, true);
  request.send();
  request.abort();

  // reusing the same GHR: open, send, wait for interactive, then abort
  request.open('GET', urlbase + 1000000, true);
  request.onreadystatechange = function() {
    if (request.readyState >= 3) {
      assertEqual(3, request.readyState);  // we dont want it to be complete yet
      request.abort();

      // reusing the same GHR: make a successful request
      request.open('GET', urlbase + 10, true);
      request.onreadystatechange = function() {
        if (request.readyState == 4) {
          assertEqual(200, request.status);
          assertEqual(10, request.responseText.length);
          request.abort();  // shouldn't hurt
          completeAsync();
        }
      };
      request.send();
    }
  };
  request.send();
}


function testGetCapturedResource() {
  startAsync();

  var myLocalServer = google.gears.factory.create('beta.localserver');
  // We don't delete and recreate the store or captured url to avoid
  // interfering with this same test running in the other thread. 
  var storeName = 'testGet_CapturedResource';
  myLocalServer.removeStore(storeName);
  var myStore = myLocalServer.createStore(storeName);
  var url = 'testcases/cgi/echo_request.py?httprequest_a_captured_url';
  var captureSuccess;


  var expectedHeaders = [["echo-Method", "GET"]];
  
  myStore.capture(url, function(url, success, id) {
    assert(success, 'Expected captured to succeed');
    doRequest(url, 'GET', null, null, 200, null, expectedHeaders, null);
  });
}

function testGet_BinaryResponse() {
  // TODO(michaeln): do something reasonable with binary responses
}

function testNullOnReadyStateChange() {
  var nullHandlerRequest = google.gears.factory.create('beta.httprequest');
  nullHandlerRequest.onreadystatechange = function() {};
  nullHandlerRequest.onreadystatechange = null;
  nullHandlerRequest.open('GET', 'nosuchfile___');
  nullHandlerRequest.send();

  var unsetHandlerRequest = google.gears.factory.create('beta.httprequest');
  unsetHandlerRequest.open('GET', 'nosuchfile___');
  unsetHandlerRequest.send();
}

// Generates header name value pairs testcases/cgi/echo_requests.py will respond
// with for the given request headers.
function getExpectedEchoHeaders(requestHeaders) {
  var echoHeaders = [];
  for (var i = 0; i < requestHeaders.length; ++i) {
    var name = 'echo-' + requestHeaders[i][0];
    var value = requestHeaders[i][1];
    echoHeaders.push([name, value]);
  }
  return echoHeaders;
}

// A helper that initiates a request and examines the response.
function doRequest(url, method, data, requestHeaders, expectedStatus,
                   expectedResponse, expectedHeaders, expectedResponseLength) {
  var request = google.gears.factory.create('beta.httprequest');

  // TODO(aa): We are seeing sporadic failures in these tests. A theory is that
  // HttpRequest is getting gc'd. Remove this test of that theory when it is
  // proven or disproven.
  global.kungFuGrip = request;

  var lastDownloadPosition = 0;
  function handleDownloadProgress(event) {
    assert(event.loaded > lastDownloadPosition);
    lastDownloadPosition = event.loaded;
  }

  request.onprogress = handleDownloadProgress;
  request.upload.onprogress = handleUploadProgress;
  request.onreadystatechange = handleReadyStateChange;
  request.open(method, url, true);

  if (requestHeaders) {
    for (var i = 0; i < requestHeaders.length; ++i) {
      request.setRequestHeader(requestHeaders[i][0],
                               requestHeaders[i][1]);
    }
  }

  request.send(data);

  var uploadProgressCalled = false;
  var lastUploadPosition = 0;
  function handleUploadProgress(event) {
    assert(method == 'POST' || method == 'PUT');
    assert(data);
    assert(data.length > 0);
    assert(event.total >= data.length);  // May be larger due to encoding.
    assert(event.loaded > lastUploadPosition);
    lastUploadPosition = event.loaded;
    uploadProgressCalled = true;
  }

  var success = false;
  function handleReadyStateChange() {
    var state = request.readyState;
    assert(state >= 0 && state <= 4, 'Invalid readyState value, ' + state);

    if (state != 4) {
      return;
    }

    assert(!success,
           'onreadystatechange called multiple times with readyState == 4');
    success = true;

    // Make sure we can fetch all properties
    try {
      assert(isNumber(request.status),
             'Should be able to get status after request');
    } catch (e) {
      assertEqual(expectedStatus, 0,
                  'Unexpected exception when accessing status');
    }
    if (expectedStatus != 0) {
      assert(isString(request.statusText),
             'Should be able to get statusText after request');
      assert(isString(request.getAllResponseHeaders()),
             'Should be able to call getAllResponseHeaders() after request');
      assert(isString(request.responseText),
             'Should be able to get responseText after request');
      assert(isObject(request.responseBlob),
             'Should be able to get responseBlob after request');

      // see if we got what we expected to get
      if (expectedStatus != null) {
        assertEqual(expectedStatus, request.status,
                    'Wrong value for status property');
      }

      if (expectedHeaders != null) {
        for (var i = 0; i < expectedHeaders.length; ++i) {
          var name = expectedHeaders[i][0];
          var expectedValue = expectedHeaders[i][1];
          var actualValue = request.getResponseHeader(name);
          assertEqual(expectedValue, actualValue,
                      'Wrong value for header "%s"'.subs(name));
        }
      }

      if (expectedResponse != null) {
        assertEqual(expectedResponse, request.responseText,
                    'Wrong responseText');
      }

      if (expectedResponseLength != null) {
        assertEqual(expectedResponseLength, request.responseBlob.length,
                    'Wrong expectedResponseLength');
      }

      if (expectedResponse != null) {
        assert(isString(request.responseText),
               'Should be able to get responseText repeatedly');
      }

      if (expectedResponseLength != null) {
        assert(isObject(request.responseBlob),
               'Should be able to get responseBlob repeatedly');
      }
    }
    if (method == 'POST' || method == 'PUT') {
      // It appears that the timing can work out so that the progress
      // callback does not get called.
      // TODO(bgarcia): determine if this is something that can be fixed.
      //assert(uploadProgressCalled);
    }
    completeAsync();
  }
}

// Helper function used by callback exception tests.
function callbackExceptionTest(async, message) {
  var request = google.gears.factory.create('beta.httprequest');
  request.onreadystatechange = function() {
    if (request.readyState == 4) {
      throw new Error(message);
    }
  };
  // This test does not require a particular URL, we just need the
  // onreadystatechange handler to be called.
  request.open('GET', 'nosuchfile___', async);
  waitForGlobalErrors([message]);
  request.send();
}

function testCallbackExceptionAsync() {
  callbackExceptionTest(true, 'exception from HTTPRequest callback - async');
}

function testCallbackExceptionSync() {
  callbackExceptionTest(false, 'exception from HTTPRequest callback');
}

function testSetGetOnreadystatechange() {
  var request = google.gears.factory.create('beta.httprequest');
  var aFunction = function() {
    return 'ignored return value';
  };
  request.onreadystatechange = aFunction;
  assertEqual(aFunction, request.onreadystatechange);
}

function testSetGetOnprogress() {
  var request = google.gears.factory.create('beta.httprequest');
  var handleProgress = function() {
    return 'this is a test';
  };
  request.onprogress = handleProgress;
  assertEqual(handleProgress, request.onprogress);
  request.upload.onprogress = handleProgress;
  assertEqual(handleProgress, request.upload.onprogress);
}
