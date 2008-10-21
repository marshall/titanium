// Copyright 2008, Google Inc.
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

var console = google.gears.factory.create('beta.console');
console.onlog = handleEvent;

// Determine if we are in a WorkerPool process or not (see handleEvent)
var in_worker = false;
var from_worker = 'Worker:false';
if (google.gears.workerPool) {
  in_worker = true;
  from_worker = 'Worker:true';
}

// Argument validation tests
function testNoParameters() {
  assertError(function() { console.log() }, null,
      'Calling console.log with no parameters should fail');
}
function testTooFewParameters() {
  assertError(function() { console.log('test'); }, null,
      'Calling console.log with only one parameter should fail');
}
function testTooManyParameters() {
  assertError(function() { console.log('test', 't', ['t'], 't'); }, null,
      'Calling console.log with more than three parameters should fail');
}
function testTypeParameterNull() {
  assertError(function() { console.log(null, 'test'); }, null,
      'Calling console.log with type null should fail');
}
function testTypeParameterUndefined() {
  assertError(function() { console.log(undefined, 'test'); }, null,
      'Calling console.log with type undefined should fail');
}
function testTypeParameterEmpty() {
  assertError(function() { console.log('', 'test'); }, null,
      'Calling console.log with type as empty string should fail');
}
function testMessageParameterNull() {
  assertError(function() { console.log('test', null); }, null,
      'Calling console.log with message null should fail');
}
function testMessageParameterUndefined() {
  assertError(function() { console.log('test', undefined); }, null,
      'Calling console.log with message undefined should fail');
}
function testMessageParameterEmpty() {
  assertError(function() { console.log('test', ''); }, null,
      'Calling console.log with message as empty string should fail');
}
function testArgsParameterNull() {
  startAsync();
  console.log(from_worker + '#testArgsParameterNull',
      'This%sShould%sNot%sBe%sChanged', null);
}
function testArgsParameterUndefined() {
  startAsync();
  console.log(from_worker + '#testArgsParameterUndefined',
      'This%sShould%sNot%sBe%sChanged', undefined);
}
function testArgsParameterEmpty() {
  startAsync();
  console.log(from_worker + '#testArgsParameterEmpty',
      'This%sShould%sNot%sBe%sChanged', []);
}

// Test argument interpolation
function testArgsInterpolationOne() {
  startAsync();
  console.log(from_worker + '#testArgsInterpolationOne',
      '%s', ['one']);
}
function testArgsInterpolationTooMany() {
  startAsync();
  console.log(from_worker + '#testArgsInterpolationTooMany',
      '%s%s%s', ['one', 'two', 'three', 'four', 'five']);
}
function testArgsInterpolationTooFew() {
  startAsync();
  console.log(from_worker + '#testArgsInterpolationTooFew',
      '%s%s%s%s%s', ['one', 'two', 'three']);
}

// Test type coercion
function testArgsInterpolationNull() {
  startAsync();
  console.log(from_worker + '#testArgsInterpolationNull',
      '%s%s%s', ['one', null, 'two']);
}
function testArgsInterpolationUndefined() {
  startAsync();
  console.log(from_worker + '#testArgsInterpolationUndefined',
      '%s%s%s', ['one', undefined, 'two']);
}
function testArgsInterpolationInt() {
  startAsync();
  console.log(from_worker + '#testArgsInterpolationInt',
      '%s%s%s', [1, 0, -3]);
}
function testArgsInterpolationDouble() {
  startAsync();
  console.log(from_worker + '#testArgsInterpolationDouble',
      '%s%s%s', [-1.1, 0.0, 3.14159]);
}
function testArgsInterpolationBool() {
  startAsync();
  console.log(from_worker + '#testArgsInterpolationBool',
      '%s%s', [true, false]);
}
function testArgsInterpolationArray() {
  startAsync();
  console.log(from_worker + '#testArgsInterpolationArray',
      '%s', [['one', 'two', 'three']]);
}
function testArgsInterpolationObject() {
  startAsync();
  console.log(from_worker + '#testArgsInterpolationObject',
      '%s', [{ data: 123, toString:
      function() { return 'Object data: ' + this.data; }}]);
}
function testArgsInterpolationString() {
  startAsync();
  console.log(from_worker + '#testArgsInterpolationString',
      '%s', [new String('one')]);
}

// Test recursive interpolation
function testArgsInterpolationRecursive() {
  startAsync();
  console.log(from_worker + '#testArgsInterpolationRecursive',
      '%s%s', ['%sone%s', '%stwo%s', 'three', 'four', 'five', 'six']);
}

// Test logging across Worker boundary by logging from a worker and
// handling the log event in the main script.
// NOTE: this test assumes that the test on the main page runs *before*
// the same test in the worker. Also this test is only valid if the
// worker test runs since it will always return success straight away
// for the main page.
function testCrossBoundaryLog() {
  if (in_worker) {
    startAsync();
    // Will only be caught by the handler in the main page
    console.log('Worker:false#testCrossBoundaryLog', 'Logged from a worker!');
  }
}

// Helper callback for testing console.onlog and logging functionality
function handleEvent(log_event) {
  // Because these tests are also run in a WorkerPool process, this handler
  // is duplicated and registered twice (once in the WorkerPool process and
  // once in the main page). Console log events, however, cross worker
  // boundaries, so *both* handlers will be called whenever a message is
  // logged in a script. For most cases we want to ignore these duplicate
  // messages. We acheive this by using a custom event type:
  // 'Worker:true#testName'   - log event generated by testName, originating
  //                            from a worker process
  // 'Worker:false#testName'  - log event generated by testName, originating
  //                            from the main script
  // 'Worker:ignore#testName' - log event generated by testName, to be
  //                            handled by *both* event handlers

  // Determine the name of the test and where the test came from
  var type_parts = log_event.type.split('#');
  var log_origin = type_parts[0];
  var test_name = type_parts[1];
  
  if (log_origin == 'Worker:ignore') {
    // Called twice!

  } else if (from_worker == log_origin) {
    // Only called for local log events
    
    // Test argument validation
    if (test_name == 'testArgsParameterNull') {
      assertEqual('This%sShould%sNot%sBe%sChanged', log_event.message);
      completeAsync();
    }
    else if (test_name == 'testArgsParameterUndefined') {
      assertEqual('This%sShould%sNot%sBe%sChanged', log_event.message);
      completeAsync();
    }
    else if (test_name == 'testArgsParameterEmpty') {
      assertEqual('This%sShould%sNot%sBe%sChanged', log_event.message);
      completeAsync();
    }

    // Test argument interpolation
    else if (test_name == 'testArgsInterpolationOne') {
      assertEqual('one', log_event.message);
      completeAsync();
    }
    else if (test_name == 'testArgsInterpolationTooMany') {
      assertEqual('onetwothree', log_event.message);
      completeAsync();
    }
    else if (test_name == 'testArgsInterpolationTooFew') {
      assertEqual('onetwothree%s%s', log_event.message);
      completeAsync();
    }

    // Test type coercion
    else if (test_name == 'testArgsInterpolationNull') {
      assertEqual('onenulltwo', log_event.message);
      completeAsync();
    }
    else if (test_name == 'testArgsInterpolationUndefined') {
      assertEqual('oneundefinedtwo', log_event.message);
      completeAsync();
    }
    else if (test_name == 'testArgsInterpolationInt') {
      assertEqual('10-3', log_event.message);
      completeAsync();
    }
    else if (test_name == 'testArgsInterpolationDouble') {
      assertEqual('-1.103.14159', log_event.message);
      completeAsync();
    }
    else if (test_name == 'testArgsInterpolationBool') {
      assertEqual('truefalse', log_event.message);
      completeAsync();
    }
    else if (test_name == 'testArgsInterpolationArray') {
      assertEqual('one,two,three', log_event.message);
      completeAsync();
    }
    else if (test_name == 'testArgsInterpolationObject') {
      assertEqual('Object data: 123', log_event.message);
      completeAsync();
    }
    else if (test_name == 'testArgsInterpolationString') {
      assertEqual('one', log_event.message);
      completeAsync();
    }

    // Test recursive interpolation
    else if (test_name == 'testArgsInterpolationRecursive') {
      assertEqual('%sone%s%stwo%s', log_event.message);
      completeAsync();
    }
    
    // Test cross boundary logging
    else if (test_name == 'testCrossBoundaryLog') {
      if (!in_worker) {
        // Got a log message from workerPool process, send one back to
        // complete test
        assertEqual('Logged from a worker!', log_event.message);
        console.log('Worker:true#testCrossBoundaryLog',
            'Logged from the main script!');
      } else {
        // Received a log back from main script, done!
        assertEqual('Logged from the main script!', log_event.message);
        completeAsync();
      }
    }

    // Bad test name
    else {
      assert(false, 'No handler for test');
      completeAsync();
    }
  }
}
