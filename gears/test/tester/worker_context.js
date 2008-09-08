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
 * This file runs inside a worker and communicates with WorkerHost to run a
 * test file inside a worker.
 */

// Use an anonymous function to avoid polluting the global scope.
(function() {
  var parentWorkerId;
  var global = this;
  var testUrl;
  var includes = [
    '../../../third_party/jsonjs/json_noeval.js',
    'lang.js',
    'assert.js',
    'runner_base.js',
    'harness.js'
  ];

  /**
   * Receives messages from the parent worker. In our case, we only receive
   * one message, and it contains the URL of the test file to run.
   * @param message The message that was received.
   * @param senderId The ID of the worker that sent the message.
   */
  google.gears.workerPool.onmessage = function(message, senderId) {
    testUrl = message;
    parentWorkerId = senderId;

    // Before we can run the test, we have to load all our includes.
    loadNextInclude(testUrl);
  };

  /**
   * Loads the first include in includes list and then removes it. If there are
   * no more includes to load, starts the actual test. Have to export to global
   * scope so that the setTimeout() code below can access it.
   */
  global.loadNextInclude = function() {
    var url = includes.shift();
    if (!url) {
      startTest();
      return;
    }

    var req = google.gears.factory.create('beta.httprequest');
    req.open('GET', url, false);
    req.onreadystatechange = function() {
      if (req.readyState == 4) {
        if (req.status == 0 || req.status == 200) {
          // HACK: There is no way to eval code in the global script in
          // IE/workers without the magic execScript() method. Work around by
          // using the script form of setTimeout(), which does run in the global
          // scope.
          var timer = google.gears.factory.create('beta.timer');
          timer.setTimeout(req.responseText + '\nloadNextInclude();', 0);
        }
      }
    };
    req.send(null);
  }

  /**
   * Load the test file and run it.
   */
  function startTest() {
    var harness = new Harness();

    harness.onTestsLoaded = partial(handleHarnessCallback, 'onTestsLoaded');
    harness.onTestComplete = partial(handleHarnessCallback, 'onTestComplete');
    harness.onAsyncTestStart = partial(handleHarnessCallback,
                                       'onAsyncTestStart');
    harness.onAllTestsComplete = partial(handleHarnessCallback,
                                         'onAllTestsComplete');
    harness.onBeforeTestStart = partial(handleHarnessCallback,
                                        'onBeforeTestStart');
    harness.load(testUrl + '?r=' + new Date().getTime());
  }

  /**
   * Handles all the callbacks from the test harness and forwards them up to the
   * parent worker.
   * @param name The name of the callback that was invoked.
   * @param var_args Variable arguments passed to the callback.
   */
  function handleHarnessCallback(name, var_args) {
    var messageName = name;

    var args = Array.prototype.slice.call(arguments, 1);
    google.gears.workerPool.sendMessage(JSON.stringify({
      name: messageName,
      args: args
    }), parentWorkerId);
  }
})();

/**
 * Simple debug utility used to debug at all costs. Note that this is sometimes
 * needed before we get the first message, hence the need to assume the parent
 * worker ID.
 */
function printLine(msg) {
  // HACK: assumes the parent worker id is zero.
  // TODO(aa): Replace this when we have proper console support from workers.
  google.gears.workerPool.sendMessage('DEBUG ' + msg, 0);
}
