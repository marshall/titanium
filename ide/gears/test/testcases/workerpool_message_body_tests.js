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

// This is the JavaScript for a worker who simply echoes whatever message it
// receives back to the sender.
var kEchoWorkerCode =
    'var wp = google.gears.workerPool;' +
    'wp.onmessage = function(a, b, m) {' +
    '  wp.sendMessage(m.body, m.sender);' +
    '};';

// This is the JavaScript for a worker who replies twice of whatever message
// it receives back to the sender.  This is polymorphic, so numbers will be
// doubled in value, but strings will be doubled in the concatenation sense.
var kDoubleWorkerCode =
    'var wp = google.gears.workerPool;' +
    'wp.onmessage = function(a, b, m) {' +
    '  wp.sendMessage(m.body + m.body, m.sender);' +
    '};';

function testFundamentalTypeMessages() {
  startAsync();
  // "Fundamental type" means boolean, string, integer, double.
  var testCases = [
    true,
    false,
    'ping',
    'pong',
    0,
    1,
    2,
    -1,
    65536,
    -65536,
    2147483648,
    -2147483648,
    4294967296,
    3.14159,
    0.000001,
    -9.999,
    1000000000000.5
  ];

  var numMessagesReceived = 0;
  var wp = google.gears.factory.create('beta.workerpool');
  wp.onmessage = function(text, sender, message) {
    assertEqual(testCases[numMessagesReceived], message.body,
                'Incorrect message body');
    // Regardless of the original type of the message sent, text (as in the
    // first arg to onmessage) and message.text (the text property of the
    // third arg to onmessage) should be a string, for backwards compatability.
    assertEqual('string', typeof(text),
                'Incorrect type for text');
    assertEqual('string', typeof(message.text),
                'Incorrect type for message.text');
    numMessagesReceived++;
    if (numMessagesReceived == testCases.length) {
      completeAsync();
    }
  };

  var childId = wp.createWorker(kEchoWorkerCode);
  for (var i = 0; i < testCases.length; i++) {
    wp.sendMessage(testCases[i], childId);
  }
}

function testSimpleArrayMessage() {
  startAsync();
  var wp = google.gears.factory.create('beta.workerpool');
  wp.onmessage = function(text, sender, message) {
    var mb = message.body;
    assertEqual(3, mb.length, 'Incorrect message body');
    assertEqual(true, mb[0], 'Incorrect message body');
    assertEqual(2.71828, mb[1], 'Incorrect message body');
    assertEqual('foo', mb[2], 'Incorrect message body');
    completeAsync();
  };

  var childId = wp.createWorker(kEchoWorkerCode);
  wp.sendMessage([true, 2.71828, 'foo'], childId);
}

function testSimpleObjectMessage() {
  startAsync();
  var wp = google.gears.factory.create('beta.workerpool');
  wp.onmessage = function(text, sender, message) {
    var mb = message.body;
    assertEqual(true, mb.b, 'Incorrect message body');
    assertEqual(2.71828, mb.d, 'Incorrect message body');
    assertEqual('foo', mb.s, 'Incorrect message body');
    completeAsync();
  };

  var childId = wp.createWorker(kEchoWorkerCode);
  wp.sendMessage({
    'b': true,
    'd': 2.71828,
    's': 'foo'
  }, childId);
}

function testCoercedObjectPropertiesMessage() {
  startAsync();
  var wp = google.gears.factory.create('beta.workerpool');
  wp.onmessage = function(text, sender, message) {
    var mb = message.body;
    assertEqual('one', mb['1'], 'Incorrect message body');
    assertEqual('two', mb['2'], 'Incorrect message body');
    assertEqual('three', mb['true'], 'Incorrect message body');
    assertEqual('four', mb['false'], 'Incorrect message body');
    completeAsync();
  };

  var childId = wp.createWorker(kEchoWorkerCode);
  wp.sendMessage({
    '1': 'one',
    '2': 'two',
    'true': 'three',
    'false': 'four'
  }, childId);
}

function testNestedArrayMessage() {
  startAsync();
  var wp = google.gears.factory.create('beta.workerpool');
  wp.onmessage = function(text, sender, message) {
    var mb = message.body;
    assertEqual(4, mb.length, 'Incorrect message body');
    assertEqual(11, mb[0], 'Incorrect message body');
    assertEqual(22, mb[1], 'Incorrect message body');
    assertEqual(2, mb[2].length, 'Incorrect message body');
    assertEqual(33, mb[2][0], 'Incorrect message body');
    assertEqual(44, mb[3][0], 'Incorrect message body');
    assertEqual(99, mb[2][1], 'Incorrect message body');
    completeAsync();
  };

  var childId = wp.createWorker(kEchoWorkerCode);
  wp.sendMessage([11, 22, [33, 99], [44, 99]], childId);
}

function testComplexMessage() {
  startAsync();
  var wp = google.gears.factory.create('beta.workerpool');
  wp.onmessage = function(text, sender, message) {
    var mb = message.body;
    assertEqual(true, mb['foo'], 'Incorrect message body');
    assertEqual(10, mb['b'][0], 'Incorrect message body');
    assertEqual(40, mb['b'][3]['forty'], 'Incorrect message body');
    assertEqual('gears', mb['b'][4]['google'], 'Incorrect message body');
    completeAsync();
  };

  var cc = { 'google':'gears' };
  var bb = [10, 20, 30, { 'forty':40, 'fifty':50 }, cc];
  var aa = {};
  aa['foo'] = true;
  aa['bar'] = null;
  aa['b'] = bb;

  var childId = wp.createWorker(kEchoWorkerCode);
  wp.sendMessage(aa, childId);
}

function testFunctionMessageFails() {
  var wp = google.gears.factory.create('beta.workerpool');
  wp.onmessage = function(text, sender, message) {
    assert(false, 'Function message should not be sendable');
  };

  var f = function() { return 'foo'; };

  var childId = wp.createWorker(kEchoWorkerCode);
  assertError(function() {
    wp.sendMessage(f, childId);
  }, 'Cannot marshal a JavaScript function.',
     'Function message should not be sendable');
}

function testFunctionElementMessageFails() {
  var wp = google.gears.factory.create('beta.workerpool');
  wp.onmessage = function(text, sender, message) {
    assert(false, 'Function element should not be sendable');
  };

  var f = function() { return 'foo'; };
  var a = [0, 1, 2, f, 4];

  var childId = wp.createWorker(kEchoWorkerCode);
  assertError(function() {
    wp.sendMessage(a, childId);
  }, 'Cannot marshal a JavaScript function.',
     'Function element should not be sendable');
}

function testFunctionPropertyMessageFails() {
  var wp = google.gears.factory.create('beta.workerpool');
  wp.onmessage = function(text, sender, message) {
    assert(false, 'Function property should not be sendable');
  };

  var f = function() { return 'foo'; };
  var o = {
    'foo': f,
    'baz': 7
  };

  var childId = wp.createWorker(kEchoWorkerCode);
  assertError(function() {
    wp.sendMessage(o, childId);
  }, 'Cannot marshal a JavaScript function.',
     'Function property should not be sendable');
}

function testNullAndUndefinedMessageFails() {
  var wp = google.gears.factory.create('beta.workerpool');
  wp.onmessage = function(text, sender, message) {
    assert(false, 'Null / undefined message should not be sendable');
  };

  var childId = wp.createWorker(kEchoWorkerCode);

  assertError(function() {
    wp.sendMessage(null, childId);
  }, 'The message parameter has an invalid type.',
     'Null message should not be sendable');

  assertError(function() {
    var u;
    wp.sendMessage(u, childId);
  }, 'The message parameter has an invalid type.',
     'Undefined message should not be sendable');
}

function testNullElementMessageSucceeds() {
  startAsync();
  var wp = google.gears.factory.create('beta.workerpool');
  wp.onmessage = function(text, sender, message) {
    var mb = message.body;
    assertEqual(3, mb.length, 'Incorrect message body');
    assertEqual(null, mb[0], 'Incorrect message body');
    assertEqual(1337, mb[1], 'Incorrect message body');
    assertEqual(null, mb[2], 'Incorrect message body');
    completeAsync();
  };

  var childId = wp.createWorker(kEchoWorkerCode);
  wp.sendMessage([null, 1337, null], childId);
}

function testUndefinedElementMessageSucceeds() {
  startAsync();
  var wp = google.gears.factory.create('beta.workerpool');
  var undefined_value;
  wp.onmessage = function(text, sender, message) {
    var mb = message.body;
    assertEqual(3, mb.length, 'Incorrect message body');
    assertEqual(undefined_value, mb[0], 'Incorrect message body');
    assertEqual(1337, mb[1], 'Incorrect message body');
    assertEqual(undefined_value, mb[2], 'Incorrect message body');
    completeAsync();
  };

  var childId = wp.createWorker(kEchoWorkerCode);
  wp.sendMessage([undefined_value, 1337, undefined_value], childId);
}

function testBlobMessageSucceeds() {
  // hasSameContentsAs() not present in opt builds.
  if (!isDebug) {
    return;
  }

  // createBlobFromString not present in official builds.
  if (isOfficial) {
    return;
  }
  
  startAsync();
  var wp = google.gears.factory.create('beta.workerpool');
  var ccTests = google.gears.factory.create('beta.test');
  var fooBlob = ccTests.createBlobFromString('fooBlob');
  var barBlob = ccTests.createBlobFromString('barBlob');
  var fooBlobToo = ccTests.createBlobFromString('fooBlob');

  assert(!fooBlob.hasSameContentsAs(barBlob), 'Blobs are incorrectly equal');
  assert(fooBlob.hasSameContentsAs(fooBlobToo),
      'Blobs are incorrectly unequal');

  wp.onmessage = function(text, sender, message) {
    assert(fooBlob.hasSameContentsAs(message.body), 'Incorrect message body');
    completeAsync();
  };

  var childId = wp.createWorker(kEchoWorkerCode);
  wp.sendMessage(fooBlob, childId);
}

function testArbitraryGearsModuleMessageFails() {
  var wp = google.gears.factory.create('beta.workerpool');
  wp.onmessage = function(text, sender, message) {
    assert(false, 'An arbitrary Gears module should not be sendable');
  };

  var childId = wp.createWorker(kEchoWorkerCode);
  assertError(function() {
    wp.sendMessage(google.gears.factory.create('beta.timer'), childId);
  }, null, 'An arbitrary Gears module should not be sendable');
}

function testWorkerAdditionIsPolymorphic() {
  startAsync();
  var testCases = [
    [3, 6],
    ['three', 'threethree']
  ];

  var numMessagesReceived = 0;
  var wp = google.gears.factory.create('beta.workerpool');
  wp.onmessage = function(text, sender, message) {
    assertEqual(testCases[numMessagesReceived][1], message.body,
                'Incorrect message body');
    numMessagesReceived++;
    if (numMessagesReceived == testCases.length) {
      completeAsync();
    }
  };

  var childId = wp.createWorker(kDoubleWorkerCode);
  for (var i = 0; i < testCases.length; i++) {
    wp.sendMessage(testCases[i][0], childId);
  }
}

function testSendingCyclicObjectFails() {
  var wp = google.gears.factory.create('beta.workerpool');
  wp.onmessage = function(text, sender, message) {
    assert(false, 'Cyclic objects should not be sendable');
  };

  var childId = wp.createWorker(kEchoWorkerCode);
  assertError(function() {
    var aa = [1, 2];
    var bb = [7, aa];
    aa.push(bb);
    wp.sendMessage(aa, childId);
  }, 'Cannot marshal an object that contains a cycle.',
     'Cyclic objects should not be sendable');
}

// We don't really care if sending window.document succeeds or fails - the
// behavior on sending non-JSON objects is unspecified, and the actual results
// are browser-implementation dependent.  For example, some browsers might
// represent document.location as a string (which is marshalable), others
// might use some other class (which might not be marshalable).
// However, regardless of whether or not it succeeds or fails, trying to do so
// should not crash the browser, nor should it get stuck in an infinite loop,
// and this is what we are testing here.
function testSendingWindowDotDocumentDoesNotCrash() {
  // This one cannot run in a worker.
  if (typeof document == 'undefined') {
    return;
  }

  startAsync();
  var wp = google.gears.factory.create('beta.workerpool');
  wp.onmessage = function(text, sender, message) {
    assertEqual('object', typeof(message.body), 'Incorrect message body');
    completeAsync();
  };

  var childId = wp.createWorker(kEchoWorkerCode);
  try {
    wp.sendMessage(window.document, childId);
  } catch (error) {
    completeAsync();
  }
}
