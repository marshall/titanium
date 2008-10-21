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
 * This class runs a test file and displays the results.  Execution is
 * handled by calling the inter-frame communication object, which makes sure
 * that tests are executed in the right place.
 * @param the InterFrameCommunication object to use when executing tests
 * @constructor
 */
function FileTestResults(ifc) {
  bindMethods(this);
  // Holds the persistent state of all run tests.
  this.testResults = {};
  if (!ifc || !ifc.isInstalled) {
    window.alert("Cannot locate inter-frame communication object.");
  }
  this.ifc_ = ifc;
  this.instanceId_ = ++FileTestResults.lastInstanceId_;
}

/**
 * Track the last instance ID that was created.
 */
FileTestResults.lastInstanceId_ = 0;

/**
 * The number of tests that have been started.
 */
FileTestResults.prototype.testsStarted = 0;

/**
 * The number of tests that are complete.
 */
FileTestResults.prototype.testsComplete = 0;

/**
 * Time elapsed during tests in seconds.
 */
FileTestResults.prototype.timeElapsed = 0;

/**
 * The name of the test suite this resides in.
 */
FileTestResults.prototype.suiteName = '';

/**
 * Callback for when all tests have completed.
 */
FileTestResults.prototype.onAllTestsComplete = function() {};

/**
 * InterFrameCommunication object
 */
FileTestResults.prototype.ifc_ = null;

/**
 * Result ID, uniquely identifying this set of results.
 */
FileTestResults.prototype.instanceId_ = -1;

/**
 * Starts the tests.
 * @param div The element to populate with the results.
 * @param testData The test file to run.
 */
FileTestResults.prototype.start = function(divName, testData, suiteName) {
  this.divName = divName;
  this.resultSetName = 'rs' + this.instanceId_;
  this.errorName = 'e' + this.instanceId_;
  this.testData_ = testData;
  this.nextRowIndex = 0;
  this.rowLookup_ = {};
  this.pendingLookup_ = {};

  this.suiteName = suiteName;

  // Store starting timestamp in timeElapsed to use later.
  this.timeElapsed = getTimeSeconds();

  // Append a div to stick errors from this test set into.
  appendSimpleElement(this.divName, 'div', this.errorName, '');

  // Add the results block.
  appendSimpleElement(this.divName, 'div', this.resultSetName, '');

  // Execute tests using the inter-frame communication object.
  if (this.testData_.config.useIFrame) {
    this.ifc_.startFrameTests(this);
  } else if (this.testData_.config.useWorker) {
    this.ifc_.startWorkerTests(this);
  }
};

/**
 * Records synchronous test case with results object.
 * @param name
 */
FileTestResults.prototype.handleBeforeSyncTestStart = function(name) {
  var timestamp = getTimeSeconds();
  this.testResults[name] = {status: "started", startTime: timestamp};
};

/**
 * Records async test case with results object.
 * @param name
 */
FileTestResults.prototype.handleBeforeAsyncTestStart = function(name) {
  var timestamp = getTimeSeconds();
  this.testResults[name + '_worker'] =
      {status: "started", startTime: timestamp};
};

/**
 * Called when all tests have been loaded into one of the hosts.
 * @param isWorker Whether the tests were loaded into a worker.
 * @param success Whether the load succeeded.
 * @param errorMessage If success is false, the error message from the failure.
 */
FileTestResults.prototype.handleTestsLoaded = function(isWorker, success,
                                                       errorMessage) {
  if (!success) {
    appendSimpleElement(this.errorName, 'load-error', null, errorMessage);
  }
};

/**
 * Called when an individual test has completed.
 * @param isWorker Whether the test ran in a worker.
 * @param name The name of the test that is complete.
 * @param success Whether the test passed.
 * @param errorMessage If success is false, the error message from the failure.
 */
FileTestResults.prototype.handleTestComplete = function(isWorker, name, success,
                                                        errorMessage) {
  var timestamp = getTimeSeconds();
  var statusKey = (isWorker) ? name + '_worker' : name;
  var status = (success) ? "passed" : "failed";

  // Calculate the elapsed time for the test case
  if (this.testResults && this.testResults[statusKey]) {
    var elapsed = timestamp - this.testResults[statusKey]['startTime'];
  } else {
    var elapsed = -1;
  }

  this.testResults[statusKey] = {status: status,
                                 message: errorMessage,
                                 elapsed: elapsed};
  var pendingLookupKey = name + '_' + isWorker;
  var pendingCellId = this.pendingLookup_[pendingLookupKey];

  if (!pendingCellId) {
    var cellId = this.createResultRow_(isWorker, name, success, errorMessage);
    this.pendingLookup_[pendingLookupKey] = cellId;
    this.testsStarted++;
  } else {
    this.updateResultCell_(pendingCellId, success, errorMessage);
  }

  this.testsComplete++;
};

/**
 * Called when an async test has started.
 * @param isWorker Whether the test is running in a worker.
 * @param name The name of the test that was started.
 */
FileTestResults.prototype.handleAsyncTestStart = function(isWorker, name) {
  this.testsStarted++;

  var timestamp = getTimeSeconds();
  var statusKey = (isWorker) ? name + '_worker' : name;
  this.testResults[statusKey] = {status: "started", startTime: timestamp};

  var pendingLookupKey = name + '_' + isWorker;
  var pendingCellId = this.pendingLookup_[pendingLookupKey];
  if (!pendingCellId) {
    var cellId = this.createResultRow_(isWorker, name, null, null);
    this.pendingLookup_[pendingLookupKey] = cellId;
  }
};

/**
 * Called when all synchronous tests have been completed.
 * @param isWorker Whether the tests were run in a worker.
 */
FileTestResults.prototype.handleAllTestsComplete = function(isWorker) {
  if (!isWorker && this.testData_.config.useWorker) {
    this.ifc_.startWorkerTests(this);
  } else {
    // Store time since test set began.  Start time is stored in timeElapsed.
    this.timeElapsed = getTimeSeconds() - this.timeElapsed;
    this.onAllTestsComplete();
  }
};

/**
 * Creates a result row for the UI. If the last two parameters are not passed,
 * The row is displayed in 'pending' status.
 * @param isWorker Whether the row is for a worker test.
 * @param name The name of test to create a row for.
 * @param opt_success If the test is complete, whether it was successful.
 * @param opt_errorMessage If the test is complete and was a failure, the error
 * message to display.
 */
FileTestResults.prototype.createResultRow_ = function(isWorker, name,
                                                      opt_success,
                                                      opt_errorMessage) {
  // Inc the row counter. (we start at 0 + 1).
  var rowId = this.resultSetName + '-r' + (++this.nextRowIndex);
  var resultCellId = rowId + 't';

  // Append the row.  Assign it the rowId as the ID for later updates.
  appendSimpleElement(this.resultSetName, 'div', rowId, '');

  if (isWorker) {
    name += ' (worker)';
  }

  // Append the name cell to the contents of the new row.
  appendSimpleElement(rowId, 'span', null, name);

  // Append the results cell.  If at all possible, do so in its final state.
  if (typeof(opt_success) == 'boolean') {
    // We have results - so write the result block directly.
    appendSimpleElement(rowId, 'span', resultCellId,
                        (opt_success) ? ' OK ' : opt_errorMessage);
    setDOMNodeClass(resultCellId, (opt_success) ? 'success' : 'failure');
  } else {
    // Append a 'pending' result cell.
    appendSimpleElement(rowId, 'span', resultCellId, 'pending...');
    setDOMNodeClass(resultCellId, 'pending');
  }
  return resultCellId;
};

/**
 * Updates the result cell, which displays the 'OK' or error message.
 * @param cell The TD element containing the result.
 * @param success Whether the test passed.
 * @param errorMessage If the test failed, th error message to display.
 */
FileTestResults.prototype.updateResultCell_ = function(cellId, success,
                                                       errorMessage) {
  if (success) {
    setDOMNodeClass(cellId, 'success');
    setDOMNodeText(cellId, ' OK ');
  } else {
    setDOMNodeClass(cellId, ' failure ');
    setDOMNodeText(cellId, ' ' + errorMessage + ' ');
  }
};

/**
 * Creates a JSON friendly representation of test results.
 */
FileTestResults.prototype.toJson = function() {
  return {suitename: this.suiteName,
          filename: this.testData_.relativePath,
          results: this.testResults,
          elapsed: this.timeElapsed};
};
