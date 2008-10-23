import unittest
import pmock
from runner import TestWebserver
from runner import RequestHandler
import datetime
import os

class TestWebserverTest(unittest.TestCase):
  """ Testcases for TestWebserver. 
  
  Target code uses real sockets therefore it is difficult to have 
  proper interaction type testing.
  """  
  def testStartTimesout(self):
    webserverRunner = TestWebserver(".", 8899)
    webserverRunner.startServing()
    before = datetime.datetime.now()
    webserverRunner.startTest(3)
    webserverRunner.testResults()
    after = datetime.datetime.now()
    self.assertTrue(((after - before).seconds) >= 3)
    webserverRunner.shutdown()
  
    
  def testStartAndShutdown(self):
    webserverRunner = TestWebserver(".", 8899)
    webserverRunner.startServing()
    webserverRunner.startTest(1)
    webserverRunner.testResults()
    webserverRunner.startTest(1)
    webserverRunner.testResults()
    webserverRunner.shutdown()
  
  
  def testInitialization(self):
    webserverRunner = TestWebserver(".", 8899)
    self.assertFalse(webserverRunner.test_instance_is_running)
    self.assertFalse(webserverRunner.test_completion_lock.isSet())
    self.assertFalse(webserverRunner.running)
    webserverRunner.shutdown()
    

  def testStartinWebserverUpdatesState(self):
    webserverRunner = TestWebserver(".", 8899)
    webserverRunner.startServing()
    webserverRunner.startTest(1)
    self.assertTrue(webserverRunner.test_instance_is_running)
    self.assertFalse(webserverRunner.test_completion_lock.isSet())
    # TODO: assert on value using some thread syncronization mechanism
    # self.assertTrue(webserverRunner.running) 
    webserverRunner.testResults()
    webserverRunner.shutdown()    
    
    
  def testNotifyOnDoPostProcessesRightUrl(self):
    webserverRunner = TestWebserver(".", 8899)
    webserverRunner.test_instance_is_running = True
    webserverRunner.test_completion_lock.clear()
    
    webserverRunner.notifyOnDoPost('{"foo": "bar"}', "some url")
    self.assertEqual('', webserverRunner.json_test_result)
    self.assertFalse(webserverRunner.test_completion_lock.isSet())
    
    webserverRunner.notifyOnDoPost('{"foo": "bar"}', 
                                   TestWebserver.RESULTS_POSTBACK_FILE_PATTERN)
    self.assertEqual({'foo': 'bar'}, webserverRunner.json_test_result)
    self.assertTrue(webserverRunner.test_completion_lock.isSet())
    webserverRunner.shutdown()
  
  
class RequestHandlerTest(unittest.TestCase):

  
  def setUp(self):
    self.server_mock = pmock.Mock()
    self.server_mock.server_root_dir = "rootdirectory_for_server"
    self.request_handler = RequestHandler(self.__connectionStub(), 
                                          pmock.Mock(), self.server_mock)
    
    
  def tearDown(self):
    self.request_handler.close()
    self.server_mock.verify()    

  
  def testDoPostCallsServerNotifyOnDoPost(self):
    post_data = "DATA CONTENT"
    full_request_path = "/tester/gui.html?foobar=goofy"
    request_path = "/tester/gui.html"
    self.request_handler.path = full_request_path
    self.server_mock.expects(pmock.once()).notifyOnDoPost(pmock.eq(post_data), 
                                                      pmock.eq(request_path))

    self.__mockHeaders(pmock)
    self.request_handler.rfile = self.__mockRfile(post_data, pmock)
    try:
      self.request_handler.do_POST()
    # We are letting some errors happen, although mocking as 
    # much as possible there is not way we can use composition to 
    # get async and http server init code out. 
    except TypeError, e:
      pass

    
  def testIsCgiCall(self):
    cgi_paths = ['testcases/cgi/test.py', 
                 '/testcases/cgi/test.py', 
                 '/testcases/cgi/with_underscore.py',
                 '/testcases/cgi/with_num8er.py', 
                 '/testcases/cgi/cApital.py',
                  'foobar/testcases/cgi/test.py'
                 ]

    for path in cgi_paths:
      self.request_handler.path = path 
      self.assertTrue(self.request_handler.isCgiCall())

    non_cgi_paths = ['testcases/cgi/notPyExtesnsion.html', 
                     'testcases/subcgi/test.py', 'cgi/test.html',
                     'cgi/test.py', '/cgi/test.py', 'noncgi/test.py'
                     ]
    
    for path in non_cgi_paths: 
      self.request_handler.path = path
      self.assertFalse(self.request_handler.isCgiCall())

  
  def testCgiFilePath(self):
    self.assertEqual( os.path.join(
              self.server_mock.server_root_dir, 'testcases/cgi/test.py'),
              self.request_handler.cgiFilePath('testcases/cgi/test.py'))
    
  
  def __connectionStub(self):
    connection = pmock.Mock()
    connection.stubs().fileno()
    connection.stubs().setblocking(pmock.eq(0))
    connection.stubs().getpeername()
    connection.stubs().close()
    return connection

  def __mockHeaders(self, pmock):
    headers_mock = pmock.Mock()
    headers_mock.stubs().getheader(pmock.eq('content-type')) \
        .will(pmock.return_value('application/x-www-form-urlencoded'))
    headers_mock.stubs().getheader(pmock.eq('content-length')) \
        .will(pmock.return_value(312))
    self.request_handler.headers = headers_mock
    

  def __mockRfile(self, post_data, pmock):
    rfile_mock = pmock.Mock()
    rfile_mock.stubs().read(pmock.eq(312)).will(pmock.return_value(post_data))
    return rfile_mock
    
    