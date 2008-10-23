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

var REQUEST_TIMEOUT_MS = 5000; // 5 seconds

/**
 * Utility to do an xmlhttprequest.
 *
 * @param {String} method  "GET" or "POST"
 * @param {String} url  Url to request. Any params will be appended after "?".
 * @param {Object} opt_params  Params to add to url. If specified, appened
 *     after "?".
 * @param {Function(String, String, String)} handler  A handler to call when the
 *     request is complete. Handler is guaranteed to call unless request is 
 *     aborted (see @return). Params are status, statusText, and responseText. 
 *     All will be null if the connection failed.
 * @param {Object} opt_body  Body to send in case method = POST.
 *
 * @return {Function}  Function to call to abort request. If this function is
 *     called handler function will not be.
 */
function doRequest(method, url, opt_params, handler, opt_body) {
  if (!opt_params) {
    opt_params = {};
  }

  // To prevent browsers from caching xhr responses
  var terms = ['r=' + new Date().getTime()];
  for (var n in opt_params) {
    terms.push(n + '=' + opt_params[n]);
  }

  url += '?' + terms.join('&');

  var req = createRequest();

  try {
    // Firefox throws on open() when it is offline; IE throws on send().
    req.open(method, url, true /* async */);
    req.send(opt_body || null);
  } catch(e) {
    req = null;

    // Set a short timeout just to get off the stack so that the call flow is
    // the same as with successful requests (subtle bugs otherwise).
    window.setTimeout(handler, 0);

    // Return a nop function so that callers don't need to care whether we
    // succeeded if they want to abort the previous request.
    return function(){};
  }

  var timerId = window.setTimeout(function() {
    req.abort();
    req = null;
    handler();
  }, REQUEST_TIMEOUT_MS);

  req.onreadystatechange = function() {
    // IE fires onreadystatechange when you abort. Check to make sure we're
    // not in this situation before proceeding.
    if (!abort) {
      return;
    }

    try {
      if (req.readyState == 4) {
        var status, statusText, responseText;

        try {
          var status = req.status;
          var statusText = req.statusText;
          var responseText = req.responseText;
        } catch (e) {
          // We cannot get properties while the window is closing.
        }

        req = null;
        window.clearTimeout(timerId);
        handler(status, statusText, responseText);
      }
    } catch (e) {
      if (console.error) {
        console.error(e);
      } else {
        throw e;
      }
    }
  };

  var abort = function() {
    if (req) {
      abort = null;
      req.abort();
      req = null;
      window.clearTimeout(timerId);
    }
  };

  return abort;
}


/**
 * Utility to get an XMLHttpRequest object in the correct browser-dependent way.
 * @return {Object}  An XMLHttpRequest instance.
 */
function createRequest() {
  try {
    return new XMLHttpRequest();
  } catch (e) {
    try {
      return new ActiveXObject("Msxml2.XMLHTTP");
    } catch (e2) {
      try {
  return new ActiveXObject("Microsoft.XMLHTTP");
      } catch (e3) {
  // poo
      }
    }
  }

  return null;
}

