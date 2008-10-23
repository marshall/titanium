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
 * This class runs a unit test file in a worker and reports the results back to
 * the caller.
 * @constructor
 */
function WorkerHost(ifc) {
  bindMethods(this);
  this.ifc_ = ifc;
}

WorkerHost.inherits(RunnerBase);

/**
 * How long to wait for the worker to wait for the worker to load and report
 * back before giving up with an error.
 */
WorkerHost.LOAD_TIMEOUT_MS = 60000;

/**
 * Id of the timer waiting for load timeout.
 */
WorkerHost.prototype.timerId_ = -1;

/**
 * Id of the worker the tests are running in.
 */
WorkerHost.prototype.workerId_ = -1;

/**
 * Workerpool the tests are runing in.
 */
WorkerHost.prototype.workerPool_ = null;

/**
 * Load and run a test file.
 * @param url The url of a file containing the tests to run.
 */
WorkerHost.prototype.load = function(url) {
  this.ifc_.log("Creating worker pool.");
  this.workerPool_ = google.gears.factory.create('beta.workerpool');
  this.workerPool_.onmessage = this.handleMessage_;

  this.ifc_.log("Creating worker.");
  this.workerId_ = this.workerPool_.createWorkerFromUrl('worker_context.js');
  this.ifc_.log("Loading " + url + " into worker.");
  this.workerPool_.sendMessage(url, this.workerId_);

  this.timerId_ = window.setTimeout(
    partial(this.onTestsLoaded, false, 'Could not load worker: ' + url),
    WorkerHost.LOAD_TIMEOUT_MS);
};

/**
 * Called when a message is received from the worker.
 * @param message The message that was received from the worker.
 * @param senderId The ID of the worker sending the message.
 */
WorkerHost.prototype.handleMessage_ = function(message, senderId) {
  if (/^DEBUG/.test(message)) {
    this.ifc_.log('Worker: ' + message);
    return;
  }

  message = JSON.parse(message);

  if (message.name == 'onTestsLoaded') {
    window.clearTimeout(this.timerId_);
  }

  this[message.name].apply(null, message.args);
};
