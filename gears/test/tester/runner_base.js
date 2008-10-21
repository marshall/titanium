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
 * Baseclass providing common callbacks for all things that run tests.
 * @constructor
 */
function RunnerBase() {
}

/**
 * Callback for when all tests are loaded.
 * @param success Whether the tests were successfully loaded.
 * @param errorMessage The message to display if success is false.
 */
RunnerBase.prototype.onTestsLoaded = function(success, errorMessage) {};

/**
 * Callback for when an individual test has completed.
 * @param name The name of the test that completed.
 * @param success Whether the test passed.
 * @param errorMessage The message to display if the test failed.
 */
RunnerBase.prototype.onTestComplete = function(name, success, errorMessage) {};

/**
 * Callback for when all tests have completed.
 */
RunnerBase.prototype.onAllTestsComplete = function() {};

/**
 * Callback for when an asynchronous test has started.
 * @param name The name of the test that was started.
 * TODO(aa): We don't really need this. We can simply create each row in the
 * 'pending' state to begin with and change it to complete when the test
 * completes.
 */
RunnerBase.prototype.onAsyncTestStart = function(name) {};

/**	
 * Callback for when any test is about to start.	
 * @param name The name of the test that will be started.	
 */	
RunnerBase.prototype.onBeforeTestStart = function(name) {};
