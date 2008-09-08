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
 * Class supporting inter-frame communication requirements.
 * Contains common methods accessed by both the executing tests
 * and the reporter/controller.
 * @constructor
 */
function IFC() {
  bindMethods(this);
  this.installed_ = (window.google && google.gears);
  this.publisher_ = new ResultsPublisher(this);
}

/**
 * Contains the currently executing frame host, used for verification.
 */
IFC.prototype.currentFrameHost_ = null;

/**
 * True if gears is installed.
 */
IFC.prototype.installed_ = false;

/**
 * The ResultsPublisher to use when publishing results.
 */
IFC.prototype.publisher_ = null;

/**
 * The suite selected on the frameset's URL.  The suite name is appended
 * immediately after the ? on the URL.
 */
IFC.prototype.suite_ = location.search.replace(/^\?/, '');

/**
 * Retrieves the currently executing frame host.
 */
IFC.prototype.getCurrentFrameHost = function() {
  return this.currentFrameHost_;
}

/**
 * Retrieves the current version of gears installed.
 */
IFC.prototype.getBuildInfo = function() {
  return (this.installed_) ?
      google.gears.factory.getBuildInfo() : '(not installed)';
}

/**
 * Retrieves the results publisher to use when publishing results.
 */
IFC.prototype.getResultsPublisher = function() {
  return this.publisher_;
}

/**
 * Retrieves the name of the suite chosen for execution, or '' if none is set.
 */
IFC.prototype.getSuite = function(suite) {
  return this.suite_;
}

/**
 * Retrieves the user agent.
 */
IFC.prototype.getUserAgent = function() {
  return navigator.userAgent;
}

/**
 * Returns true if gears is installed.
 */
IFC.prototype.isInstalled = function() {
  return this.installed_;
}

/**
 * Returns true if the window's location is a web-based protocol (and not local)
 */
IFC.prototype.isWebProtocol = function() {
  return location.protocol.substring(0, 4) == 'http';
}


/**
 * Logs a message to the test execution log (frame).
 */
IFC.prototype.log = function(message) {
  var frame = getDOMFrameById('log');
  frame.window.log(message);
}


/**
 * Starts a set of frame tests.
 */
IFC.prototype.startFrameTests = function(results) {
  var frameHost = new IFrameHost();
  this.currentFrameHost_ = frameHost;
  frameHost.onTestsLoaded = partial(results.handleTestsLoaded, false);
  frameHost.onTestComplete = partial(results.handleTestComplete, false);
  frameHost.onAsyncTestStart = partial(results.handleAsyncTestStart, false);
  frameHost.onAllTestsComplete = partial(results.handleAllTestsComplete, false);
  frameHost.onBeforeTestStart = results.handleBeforeSyncTestStart;
  frameHost.load(results.testData_.relativePath);
};

/**
 * Starts a set of worker tests
 */
IFC.prototype.startWorkerTests = function(results) {
  var workerHost = new WorkerHost(this);
  workerHost.onTestsLoaded = partial(results.handleTestsLoaded, true);
  workerHost.onTestComplete = partial(results.handleTestComplete, true);
  workerHost.onAsyncTestStart = partial(results.handleAsyncTestStart, true);
  workerHost.onAllTestsComplete = partial(results.handleAllTestsComplete, true);
  workerHost.onBeforeTestStart = results.handleBeforeAsyncTestStart;
  workerHost.load(results.testData_.relativePath);
};
