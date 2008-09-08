How to write tests:

Gears unit tests are contained in the files listed in the test/testcases/ 
directory. If you add a new file, you need to update test/testcases/config.js 
to point to it.

The unit test runner can automatically run your tests in workers as well as
in the main HTML document. You should take advantage of this where possible.
To do so, just set the useWorker flag to true for your test file in config.js.

Unit tests have access to a variety of utility functions in test/tester/lang.js 
and test/tester/assert.js.

The test runner also has support for asynchronous tests. To use it, set up your
test as usual in a test* function. After you start the asynchronous bit, call
startAsync(). When your asynchronous test completes successfully, call
completeAsync() to notify the test runner that the next test should begin.
Asynchronous tests that don't call completeAsync() are eventually timed out and
marked failed. For an example of asynchronous tests, see
testcases/timer_tests.js.

The test webserver binds itself to all bound IP addresses on the machine;
it is visible on 127.0.0.1 (localhost) as well as any other connected 
interfaces.

How to run tests:

Run python test/runner/testwebserver.py
Then go to http://localhost:8001/tester/gui.html  (or your favorite host name)
