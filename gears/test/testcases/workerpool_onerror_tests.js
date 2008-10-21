// Helper used to test that the onerror handler bubbles correctly.
function workerPoolBubbleTest(name, onerrorContent, shouldBubble, firstError) {
  var fullName = 'WorkerPoolBubbleTest: ' + name;

  var wp = google.gears.factory.create('beta.workerpool');
  // Have to create a reference to the wp so that it doesn't get gc'd early.
  workerPoolBubbleTest.wp = wp;

  if (shouldBubble) {
    var errors = [fullName];
    if (firstError) {
      errors.unshift(firstError);
    }
    waitForGlobalErrors(errors);
  }

  var str = [];

  if (onerrorContent) {
    str.push('google.gears.workerPool.onerror = function() {');
    str.push(onerrorContent);
    str.push('}');
  }

  str.push('throw new Error("' + fullName + '")');
  wp.createWorker(str.join('\n'));
};

// Tests begin here

function testNoBubble() {
  workerPoolBubbleTest('no bubble', 'return true;', false, null);
}

function testBubble1() {
  workerPoolBubbleTest('bubble 1', 'return false;', true, null);
}

// These next two should *not* bubble because they get coerced to <true>.
function testBubble2() {
  workerPoolBubbleTest('bubble 2', 'return 42;', false, null);
}

function testBubble3() {
  workerPoolBubbleTest('bubble 3', 'return {};', false, null);
}

function testBubble4() {
  workerPoolBubbleTest('bubble 4', 'return;', true, null);
}

function testBubble5() {
  workerPoolBubbleTest('bubble 5', null, true, null);
}

function testNestedOuterError() {
  workerPoolBubbleTest('nested error outer',
                       'throw new Error("nested error inner")', true,
                       'nested error inner');
}

function testWorkerOnError() {
  // Test that onerror inside a worker gets called.  
  // If onerror is not called the test will time out.
  startAsync();

  var wp = google.gears.factory.create('beta.workerpool');

  wp.onmessage = function() {
    completeAsync();
  };

  var childId = wp.createWorker(
      ['var parentId;',
       'google.gears.workerPool.onmessage = function(m, senderId) {',
       'parentId = senderId;',
       'eval(\'throw new Error("hello");\')',
       '}',
       'google.gears.workerPool.onerror = function() {',
       'google.gears.workerPool.sendMessage("", parentId);',
       'return true;',
       '}'].join('\n'));
  wp.sendMessage('hello', childId);
}

// Helper used to test that an error thrown in a child worker is correctly
// bubbled to its parent worker and to the main page.
function childWorkerBubbleTest(name, childOnerror, parentOnerror,
                               shouldBubble) {
  var fullName = 'childWorkerBubbleTest: ' + name;

  var wp = google.gears.factory.create('beta.workerpool');
  // Have to create a reference to the wp so that it doesn't get gc'd early.
  workerPoolBubbleTest.wp = wp;

  if (shouldBubble) {
    waitForGlobalErrors([fullName]);
  }

  // This must not contain line breaks or single quotes.
  var childCode = '';
  if (childOnerror) {
    childCode += 'google.gears.workerPool.onerror = function() {';
    childCode += childOnerror;
    childCode += '};';
  }
  childCode += 'throw new Error("' + fullName + '");';

  var str = [];
  if (parentOnerror) {
    str.push('google.gears.workerPool.onerror = function() {');
    str.push(parentOnerror);
    str.push('}');
  }
  str.push('var childWorkerPool = ' +
      'google.gears.factory.create("beta.workerpool");');
  str.push('childWorkerPool.createWorker(\'' + childCode + '\');');
  wp.createWorker(str.join('\n'));
};

// Tests begin here

function testChildWorkerError1() {
  // No onerror handler in either child or parent, error should bubble to main
  // page.
  childWorkerBubbleTest('child bubble 1', null, null, true);
}

function testChildWorkerError2() {
  // onerror handler in parent only, error should not bubble to main page.
  childWorkerBubbleTest('child bubble 2', null, 'return true;', false);
}

function testChildWorkerError3() {
  // onerror handler in child only, error should not bubble to main page.
  childWorkerBubbleTest('child bubble 3', 'return true;', null, false);
}
function testChildWorkerError4() {
  // onerror handler in both parent and child, error should not bubble to main
  // page.
  childWorkerBubbleTest('child bubble 4', 'return true;', 'return true;',
                        false);
}
