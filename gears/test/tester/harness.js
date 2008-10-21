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
 * This class loads and runs the tests in a given URL in the current context.
 *
 * Typical usage:
 *
 * var harness = new Harness();
 * harness.onTestsLoad = function(success, errorMessage) {
 *   ...
 * };
 * harness.onTestComplete = function(name, success, errorMessage) {
 *   ...
 * };
 * harness.onAsyncTestStart = function(name) {
 *   ...
 * };
 * harness.load('mytests.js');
 *
 * @constructor
 */
function Harness() {
  bindMethods(this);
  Harness.current_ = this;
  this.testNames_ = [];
}
Harness.inherits(RunnerBase);

/**
 * Length of time to wait before giving up on async tests.
 */
Harness.ASYNC_TIMEOUT_MSEC = 15000; // 15 seconds

/**
 * The current harness active in this context.
 */
Harness.current_ = null;

/**
 * The latest GearsHttpRequest instance used.
 */
Harness.prototype.request_ = null;

/**
 * The name of current running test.
 */
Harness.prototype.currentTestName_ = null;

/**
 * The url of the test to load.
 */
Harness.prototype.testUrl_ = null;

/**
 * The global scope to use for finding test names. In IE this is a special
 * 'RuntimeObject' that allows lookup in the global scope by name.
 */
Harness.prototype.globalScope_ = null;

/**
 * An array of all tests found in this context.
 */
Harness.prototype.testNames_ = null;

/**
 * The index of the current running test in testNames_.
 */
Harness.prototype.currentTestIndex_ = -1;

/**
 * The id of the timer that is used to timeout startAsync().
 */
Harness.prototype.asyncTimerId_ = null;

/**
 * The id of the timer that is used to timeout waitForGlobalErrors().
 */
Harness.prototype.globalErrorTimerId_ = null;

/**
 * A list of global errors being waited for with waitForGlobalErrors().
 */
Harness.prototype.expectedGlobalErrors_ = null;

/**
 * Load and run a test file.
 * @param url The url of a file containing the tests to run.
 */
Harness.prototype.load = function(testUrl) {
  this.request_ = google.gears.factory.create('beta.httprequest');
  this.request_.onreadystatechange = this.handleRequestReadyStateChange_;
  this.testUrl_ = testUrl;
  this.requestCompleted_ = false;

  try {
    this.request_.open('GET', testUrl, false);
    this.request_.send(null);
  } catch (e) {
    this.onTestsLoaded(false, 'Error loading tests: ' + e.message);
  }
};

/**
 * Handles the onreadystatechange event when loading a test file.
 */
Harness.prototype.handleRequestReadyStateChange_ = function() {
  if (this.request_.readyState == 4) {
    if (this.requestCompleted_)
      return;  
    this.requestCompleted_ = true;

    if (this.request_.status == 0 || this.request_.status == 200) {
      // Have to use this hack to eval in the global scope in IE workers.
      var timer = google.gears.factory.create('beta.timer');
      timer.setTimeout(
        '\n' + // This whitespace is required. For an explanation of why, see:
               // http://code.google.com/p/google-gears/issues/detail?id=265
        this.request_.responseText +
        '\nHarness.current_.handleTestsLoaded_(true)',
        0);
    } else {
      this.onTestsLoaded(
        false,
        'Could not load tests: ' + this.testUrl_ + '. Status: ' +
            this.request_.status + ' ' + this.request_.statusText);
    }
  }
};

/**
 * A wrapper for handleGlobalError_ for use in a worker. The argument to
 * workerPool.onerror is an error object, not a message string.
 */
Harness.prototype.workerHandleGlobalError_ = function(error) {
  return this.handleGlobalError_(error.message);
}

/**
 * Called when a test file has been loaded. Evaluate it and run the tests.
 */
Harness.prototype.handleTestsLoaded_ = function(content) {
  this.onTestsLoaded(true);

  // IE has a bug where you can't iterate the objects in the global scope, but
  // it luckily has this hack you can use to get around it.
  this.globalScope_ = global.RuntimeObject ? global.RuntimeObject('test*')
                                           : global;

  // Find all the test names
  for (var name in this.globalScope_) {
    if (name.substring(0, 4) == 'test') {
      // SAFARI-TEMP
      var test_is_enabled = !(isSafari && 
                              this.globalScope_[name]._disable_in_safari);

      // NPAPI-TEMP
      var test_is_enabled = test_is_enabled && !(isNPAPI &&
          this.globalScope_[name]._disable_in_npapi);
      if (test_is_enabled) {
        this.testNames_.push(name);
      }
    }
  }

  // The global error handler is in a different place inside a worker than in
  // a document.
  if (google.gears.workerPool) {
    google.gears.workerPool.onerror = this.workerHandleGlobalError_;
  } else {
    window.onerror = this.handleGlobalError_;
    // On IE6, at least, assigning window.onerror leads to a circular COM
    // reference, and hence a memory leak. Presumably the circular reference
    // is between between this (an instance of Harness) and the global object
    // called window. To break this circular reference, we reset window.onerror
    // on page unload.
    window.onunload = function() {
      window.onerror = null;
    }
  }

  // Run the first test directly; no need to schedule.
  this.runTests_();
};

/**
 * Perform the tests contained within the current context.
 * As well as being called by handleTestsLoaded to initiate test execution,
 * it is also called by callbacks to continue processing of tests after
 * an asynchronous test prematurely terminates the test execution loop
 * pending test completion.
 *
 * All roads lead to Rome.
 */
Harness.prototype.runTests_ = function() {
  while (++this.currentTestIndex_ <= this.testNames_.length) {
    try {
      this.runNextTest_();
    } catch(e) {
      // We explicitly call the error handler, rather than relying on the
      // browser to call window.onerror, because window.onerror is not supported
      // on Safari and window.onerror seems not to be called in this particular
      // case on WinCE.
      // TODO(steveblock): Understand the cause of this behavior on WinCE.
      this.handleGlobalError_(e.message);
      return;
    }
    if (this.asyncTimerId_ || this.globalErrorTimerId_) {
      // break out of the loop if we started an async test.
      return;
    }
  }
}

/**
 * Starts the next test, or ends the test run if there are no more tests.
 */
Harness.prototype.runNextTest_ = function() {
  // Clear any async timer still dangling from the previous test
  if (this.asyncTimerId_) {
    timer.clearTimeout(this.asyncTimerId_);
    this.asyncTimerId_ = null;
  }

  if (this.globalErrorTimerId_) {
    timer.clearTimeout(this.globalErrorTimerId_);
    this.globalErrorTimerId_ = null;
  }

  if (this.currentTestIndex_ == this.testNames_.length) {
    // We're done!
    this.onAllTestsComplete();
    return;
  }

  // Start the next test
  this.currentTestName_ = this.testNames_[this.currentTestIndex_];
  this.expectedErrors_ = null;

  // Tests that caller thinks are going to be async can sometimes actually work
  // out to be synchronous (eg, when xhr.onreadystatechange fires inside
  // send()). Detect this by comparing currentTestIndex_ before and after the
  // test is executed.
  var testIndexBefore = this.currentTestIndex_;

  // Let everyone know (call back) that a test is starting
  this.onBeforeTestStart(this.currentTestName_);

  // This might throw, but that is OK -- handleGlobalError_ will get called and
  // we continue there. Doing that is better than try/catch here so that when
  // unit tests fail the default browser error UI gets invoked, which contains
  // more information than we can easily get out of a caught error.
  this.globalScope_[this.currentTestName_]();

  // If the test index has changed, the test already called completeAsync.
  if (testIndexBefore != this.currentTestIndex_) {
    return;
  }

  // The test was started successfully if we got here, but it might not be done.
  // If the test wasn't asynchronous, it is done, so finish up and move on.
  if (!this.asyncTimerId_ && !this.globalErrorTimerId_) {
    this.onTestComplete(this.currentTestName_, true);
  }
};

/**
 * Starts the current test running asynchronously. After this is called,
 * completeAsync() must eventually be called or the test will be marked as
 * failed after a timeout period.
 */
Harness.prototype.startAsync = function() {
  if (this.asyncTimerId_) {
    throw new Error('Test is already asynchronous');
  }

  this.onAsyncTestStart(this.currentTestName_);

  this.asyncTimerId_ = timer.setTimeout(this.handleAsyncTimeout_,
                                        Harness.ASYNC_TIMEOUT_MSEC);
};

/**
 * Called when an async test times out. Mark the test failed and start the next
 * test.
 */
Harness.prototype.handleAsyncTimeout_ = function() {
  this.asyncTimerId_ = null;
  this.onTestComplete(this.currentTestName_, false,
                      'Asynchronous test timed out. Call completeAsync() to ' +
                      'mark an asynchronous test successful.');

  this.runTests_();
};

/**
 * Marks the currently running asynchronous test as complete and starts the next
 * test.
 */
Harness.prototype.completeAsync = function() {
  if (!this.asyncTimerId_) {
    throw new Error('Test is not asynchronous');
  }

  this.onTestComplete(this.currentTestName_, true);
  this.runTests_();
};

/**
 * Wait for the specified global errors to occur and then run the next test. If
 * the errors do not occur, in order, or if a different error occurs, the test
 * is marked failed.
 *
 * Each error message is a substring of a global error that is expected to
 * occur.
 */
Harness.prototype.waitForGlobalErrors = function(errorMessages) {
  // TODO(aa): Support a callback after all expected errors have been caught?
  // This would allow tests to verify that invariants are preserved after errors
  // are thrown.
  if (this.expectedErrors_) {
    throw new Error('Already waiting for errors');
  }

  if (!errorMessages.length) {
    throw new Error('Must specify array of at least one expected global error');
  }

  this.onAsyncTestStart(this.currentTestName_);

  this.expectedErrors_ = errorMessages;
  this.globalErrorTimerId_ = timer.setTimeout(this.handleGlobalErrorTimeout_,
                                              Harness.ASYNC_TIMEOUT_MSEC);
};

/**
 * Called just before an error is reported to the browser UI. Decide whether to
 * let the browser display the error (if the error was unexpected) and whether
 * to start the next test (if there are no more errors we are waiting for).
 */
Harness.prototype.handleGlobalError_ = function(message) {
  var expectedError = this.expectedErrors_ && this.expectedErrors_.shift();

  // If the error was expected, swallow it and either wait for the next expected
  // error or run the next test.
  if (expectedError && message.indexOf(expectedError) > -1) {
    if (!this.expectedErrors_.length) {
      // No more expected errors, mark test succeeded and continue.
      this.onTestComplete(this.currentTestName_, true);
      this.runTests_();
    } // else, wait for the next error

    return true; // swallow the error, don't show the browser default error UI.
  }

  // If the error was unexpected, show an error and run the next test.
  if (expectedError) {
    // There was an expected error, but the received error doesn't match.
    this.onTestComplete(
      this.currentTestName_, false,
      'Error "%s" does not match expected error "%s"'.subs(
        message, expectedError));
  } else {
    this.onTestComplete(this.currentTestName_, false, message);
  }

  this.runTests_();
  return false; // show the browser default error UI.
};

/**
 * Called when waitForGlobalErrors() times out. Mark the test as failed and
 * start the next one.
 */
Harness.prototype.handleGlobalErrorTimeout_ = function() {
  this.globalErrorTimerId_ = null;
  this.onTestComplete(
    this.currentTestName_, false,
    'Expected errors did not occur: ' + this.expectedErrors_.join(','));
  this.runTests_();
};
