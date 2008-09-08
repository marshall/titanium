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

/**
 * This class runs a unit test file in an iframe and reports the results back to
 * the caller.
 * @constructor
 */
function IFrameHost() {
  bindMethods(this);
  this.instanceId_ = ++IFrameHost.lastInstanceId_;
}

IFrameHost.inherits(RunnerBase);

/**
 * Last instance ID created.
 */
IFrameHost.lastInstanceId_ = -1;

/**
 * Amount of time to wait to load iframe before giving up and reporting an
 * error.
 */
IFrameHost.LOAD_TIMEOUT_MS = 60000;

/**
 * Id of the timer waiting for load timeout.
 */
IFrameHost.prototype.timerId_ = -1;

/**
 * Unique identifier of this instance.
 */
IFrameHost.prototype.instanceId_ = -1;

IFrameHost.prototype.getId = function() {
  return this.instanceId_;
}

/**
 * Load and run a test file.
 * @param url The url of a file containing the tests to run.
 */
IFrameHost.prototype.load = function(url) {
  var frame = getDOMFrameById('exec');
  var src = 'iframe_context.html?id=' + this.instanceId_ +
            '&test=' + encodeURIComponent(url);
  frame.document.location.replace(src);
  IFrameHost.current_ = this;

  this.timerId_ = window.setTimeout(
    partial(this.onTestsLoaded, false, 'Frame not loaded after timeout: ' + url),
    IFrameHost.LOAD_TIMEOUT_MS);
};

/**
 * Called by iframe_context.js when tests have been loaded into the iframe.
 * @param success Whether the test file was successfully loaded.
 * @param errorMessage If success is false, the error message to display.
 */
IFrameHost.prototype.handleTestsLoaded = function(success, errorMessage) {
  window.clearTimeout(this.timerId_);
  this.onTestsLoaded(success, errorMessage);
};
