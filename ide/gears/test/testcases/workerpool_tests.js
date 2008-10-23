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

// TODO(cprince): use the following to test performance of large messages
//var LONG_STRING_PADDING = 'a';
//for (var i = 0; i < 10; ++i) {  // generates a 2^N character string
//  LONG_STRING_PADDING = LONG_STRING_PADDING + LONG_STRING_PADDING;
//}

function testSynchronizationStressTest() {
  startAsync();

  var NUM_WORKERS = 10;  // set >= 10 to break Firefox (2007/02/22)
  var workerIndexToWorkerId = [];
  var workerIdToWorkerIndex = [];
  var totalMessagesReceived = 0;
  var nextValueToRecvFromWorkerId = []; // [worker] -> value
  var NUM_REQUEST_LOOPS = 5;            // set workers * requests * responses
  var NUM_RESPONSES_PER_REQUEST = 100;  // >= 10000 to break IE (2007/02/22)
  var EXPECTED_RESPONSES_PER_WORKER =
    NUM_REQUEST_LOOPS * NUM_RESPONSES_PER_REQUEST;

  var createBeginTime;
  var createEndTime;
  var sendRecvBeginTime;
  var sendRecvEndTime;

  // Parent/child worker code.  Each child is programmed to respond with the
  // the number of messages requested by the parent.  The parent is programmed
  // to tally each response, and to look for dropped messages.

  function parentOnMessage(text, sender, m) {
    // Track cumulative per-worker message count.
    // Detect out-of-order responses.
    var retval = parseInt(m.text);
    var expected = nextValueToRecvFromWorkerId[sender];
    assertEqual(expected, retval, 'Out of order value');
    nextValueToRecvFromWorkerId[sender] = retval + 1;

    var workerIndex = workerIdToWorkerIndex[sender];
    var firstValueToRecv = workerIndex * EXPECTED_RESPONSES_PER_WORKER;

    if (0 == (nextValueToRecvFromWorkerId[sender] % 
              EXPECTED_RESPONSES_PER_WORKER)) {
      // will update this once per worker, which is fine
      // TODO(aa): Add support to the test runner for visibly logging
      // information like this.
      sendRecvEndTime = new Date().getTime();

      var numReceived = nextValueToRecvFromWorkerId[sender] - firstValueToRecv;
      assertEqual(EXPECTED_RESPONSES_PER_WORKER, numReceived,
                  'Did not receive all messages from worker %s'.subs(workerId));
    }

    ++totalMessagesReceived;
    if (totalMessagesReceived == EXPECTED_RESPONSES_PER_WORKER * NUM_WORKERS) {
      completeAsync();
    }

    assert(retval < (firstValueToRecv + EXPECTED_RESPONSES_PER_WORKER),
           'Received an extra message %s for worker %s'.subs(retval, workerId));
  }

  function childInit() {
    messageTotal = 0;
    google.gears.workerPool.onmessage = childOnMessage;
  }

  function childOnMessage(text, sender, m) {
    var numResponses = parseInt(m.text);
    for (var i = 0; i < numResponses; ++i) {
      google.gears.workerPool.sendMessage(String(gFirstResponse + messageTotal),
                                          sender);
      messageTotal += 1;
    }
  }

  var childCode = childOnMessage + childInit + 'childInit();';
  var workerPool = google.gears.factory.create('beta.workerpool');
  workerPool.onmessage = parentOnMessage;


  // Create all workers.
  createBeginTime = new Date().getTime();
  for (var t = 0; t < NUM_WORKERS; ++t) {
    var firstResponse = t * EXPECTED_RESPONSES_PER_WORKER;
    // test with different code in every worker
    var thisChildCode = 'var gFirstResponse = ' + firstResponse + ';' +
                        childCode;
    var childId = workerPool.createWorker(thisChildCode);
    workerIndexToWorkerId[t] = childId;
    workerIdToWorkerIndex[childId] = t;
    nextValueToRecvFromWorkerId[childId] = firstResponse;
  }
  createEndTime = new Date().getTime();


  // Loop over the workers several times, asking them to return some number
  // of messages each time. This floods the message path into the parent worker.
  sendRecvBeginTime = new Date().getTime();
  for (var i = 0; i < NUM_REQUEST_LOOPS; ++i) {
    for (var t = 0; t < NUM_WORKERS; ++t) {
      var workerId = workerIndexToWorkerId[t];
      workerPool.sendMessage(String(NUM_RESPONSES_PER_REQUEST) + ',' +
                             String(t * EXPECTED_RESPONSES_PER_WORKER),
                             workerId);
    }
  }
}

function testGCWithFunctionClosures() {
  if (!isDebug) {
    // This test relies on forceGC(), which only exists in debug builds.
    return;
  }

  startAsync();

  function childThread() {
    google.gears.workerPool.onmessage = function(text, sender, m) {
      if (m.text != 'ping') {
        throw new Error('unexpected message "' + m.text + '" to child worker');
      }
      google.gears.workerPool.sendMessage('pong', m.sender);
    }

    google.gears.workerPool.forceGC();
  }

  var wp = google.gears.factory.create('beta.workerpool');
  wp.onmessage = function(text, sender, m) {
    assertEqual('pong', m.text, 'Unexpected message to parent worker');
    completeAsync();
  }

  var childCode = childThread + ';childThread();';
  var childId = wp.createWorker(childCode);

  wp.forceGC();
  wp.sendMessage('ping', childId);
  wp.forceGC();
}

function testOnMessageTests() {
  startAsync();
  var wp1 = google.gears.factory.create('beta.workerpool');
  wp1.onmessage = function(text, sender, message) {
    assertEqual('PING1', text, 'Incorrect text');
    assertEqual(text, message.text, 'Incorrect message.text');
    assertEqual(sender, message.sender, 'Incorrect sender');
    assertNotEqual('', message.origin, 'Incorrect origin');
    completeAsync();
  };

  // m.text is deprecated, but is still provided for backwards compatability.
  // Instead of m.text, new code should use m.body.  For m.body tests, see
  // workerpool_message_body_tests.js.
  var childId = wp1.createWorker('var wp = google.gears.workerPool;' +
                                 'wp.onmessage = function(a, b, m) {' +
                                 '  wp.sendMessage(m.text, m.sender);' +
                                 '};');
  wp1.sendMessage('PING1', childId);
}
