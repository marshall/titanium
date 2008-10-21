import unittest
from runner import TestRunner
import pmock

class TestRunnerTest(unittest.TestCase):
    
  def setUp(self):
    self.test_url = TestRunner.TEST_URL
    self.test_webserver = pmock.Mock()
    self.browser_launcher = pmock.Mock()
    self.browser_launcher.stubs().type().will(pmock.return_value("launcher"))
    self.browser_launcher.stubs().killAllInstances()


  def tearDown(self):
    self.test_webserver.verify()
    self.browser_launcher.verify()
    
    
  def testRunTestsLaunchesTestForEachBrowser(self):
    browser_launchers = [pmock.Mock(), pmock.Mock()]
    self.test_webserver.expects(pmock.once()).startServing()
    expected_results = {}
    launcher_id = 0
    for browser_launcher in browser_launchers:
      browser_launcher.stubs().type() \
                              .will(pmock.return_value(str(launcher_id)))
      expected_results[browser_launcher.type()] = "TIMED-OUT"
      launcher_id += 1

    for browser_launcher in browser_launchers:
      self.test_webserver.expects(pmock.once()) \
                                  .startTest(pmock.eq(TestRunner.TIMEOUT)) \
        .will(pmock.return_value(expected_results[browser_launcher.type()]))
      browser_launcher.expects(pmock.once()) \
                                  .launch(pmock.eq(TestRunner.TEST_URL))
      self.test_webserver.expects(pmock.once()).testResults() \
        .will(pmock.return_value(expected_results[browser_launcher.type()]))
      browser_launcher.expects(pmock.once()).killAllInstances()
      
    self.test_webserver.expects(pmock.once()).shutdown()
      
    aggregated_results = TestRunner(browser_launchers, 
                                    self.test_webserver).runTests()
    self.assertEqual(expected_results, aggregated_results)
    
    for browser_launcher in browser_launchers:
      browser_launcher.verify()
  
  
  def testRunTestsRequiresAtLeastOneBrowserLaucher(self):
    self.assertRaises(ValueError, TestRunner, [], self.test_webserver)
    
    
  def testRunTestsWebserverLanchedBeforeBrowserInvoked(self):
    self.test_webserver.expects(pmock.once()).startServing()
    self.test_webserver.expects(pmock.once()) \
                                .startTest(pmock.eq(TestRunner.TIMEOUT))
      
    self.browser_launcher.expects(pmock.once()) \
      .launch(pmock.eq(TestRunner.TEST_URL)) \
      .after("startTest", self.test_webserver)
    
    self.test_webserver.expects(pmock.once()) \
      .testResults() \
      .after("launch", self.browser_launcher)
      
    self.test_webserver.expects(pmock.once()).shutdown()
    
    TestRunner([self.browser_launcher], self.test_webserver).runTests()
    
  
  def testOneBrowserLaunchFailureDoesNotAffectOtherBrowserTesting(self):
    self.test_webserver.expects(pmock.once()).startServing()
    self.test_webserver.expects(pmock.once()) \
                                .startTest(pmock.eq(TestRunner.TIMEOUT))
    self.test_webserver.expects(pmock.once()) \
                                .startTest(pmock.eq(TestRunner.TIMEOUT))
    
    
    failing_browser_launcher = pmock.Mock()
    failing_browser_launcher.stubs().type() \
        .will(pmock.return_value("launcher1"))
    failing_browser_launcher.expects(pmock.once()) \
      .launch(pmock.eq(TestRunner.TEST_URL)) \
      .will(pmock.raise_exception(RuntimeError("browser lauch failed")))
    failing_browser_launcher.expects(pmock.once()).killAllInstances()
    
    self.test_webserver.expects(pmock.once()) \
      .testResults() \
      .after("launch", failing_browser_launcher)
      

    second_browser_launcher = pmock.Mock()
    second_browser_launcher.stubs().type() \
        .will(pmock.return_value("launcher2"))
    second_browser_launcher.expects(pmock.once()) \
      .launch(pmock.eq(TestRunner.TEST_URL)) 
    second_browser_launcher.expects(pmock.once()).killAllInstances()
    
    self.test_webserver.expects(pmock.once()) \
      .testResults() \
      .after("launch", second_browser_launcher)
      
      
    self.test_webserver.expects(pmock.once()).shutdown()
    
    TestRunner([failing_browser_launcher, second_browser_launcher], 
               self.test_webserver).runTests()
    failing_browser_launcher.verify()
    second_browser_launcher.verify()
    

  def testOneBrowserKillFailureDoesNotAffectOtherBrowserTesting(self):
    self.test_webserver.expects(pmock.once()).startServing()
    self.test_webserver.expects(pmock.once()) \
                                .startTest(pmock.eq(TestRunner.TIMEOUT))
    self.test_webserver.expects(pmock.once()) \
                                .startTest(pmock.eq(TestRunner.TIMEOUT))
    
    
    failing_browser_launcher = pmock.Mock()
    failing_browser_launcher.stubs().type() \
        .will(pmock.return_value("launcher1"))
    failing_browser_launcher.expects(pmock.once()) \
      .launch(pmock.eq(TestRunner.TEST_URL))
    failing_browser_launcher.expects(pmock.once()).killAllInstances() \
      .will(pmock.raise_exception(RuntimeError("browser kill failed")))
    
    self.test_webserver.expects(pmock.once()) \
      .testResults() \
      .after("launch", failing_browser_launcher)
      

    second_browser_launcher = pmock.Mock()
    second_browser_launcher.stubs().type() \
        .will(pmock.return_value("launcher2"))
    second_browser_launcher.expects(pmock.once()) \
      .launch(pmock.eq(TestRunner.TEST_URL)) 
    second_browser_launcher.expects(pmock.once()).killAllInstances()
    
    self.test_webserver.expects(pmock.once()) \
      .testResults() \
      .after("launch", second_browser_launcher)
      
      
    self.test_webserver.expects(pmock.once()).shutdown()
    
    TestRunner([failing_browser_launcher, second_browser_launcher], 
               self.test_webserver).runTests()
    failing_browser_launcher.verify()
    second_browser_launcher.verify()
        

  def testBrowserTypeMustBeUnique(self):
    browser_launcher1 = pmock.Mock()
    browser_launcher2 = pmock.Mock()
    
    duplicated_type = "same type value"
    browser_launcher1.stubs().type().will(pmock.return_value(duplicated_type))
    browser_launcher2.stubs().type().will(pmock.return_value(duplicated_type))
    
    self.assertRaises(ValueError, TestRunner, 
                      [browser_launcher1, browser_launcher2], 
                      self.test_webserver)
    
  
if __name__ == "__main__":
  unittest.main()    
  