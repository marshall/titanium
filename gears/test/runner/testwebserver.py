#!/usr/bin/python2.4
#
# Asynchronous webserver used for testing. 
#
# The intent is to have:
# a) A non-blocking server, and
# b) Server with controllable lifecycle,
# so that it can be used as an embedded webserver within test runner process.
#
# Original version:
#   http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/440665

import asynchat 
import asyncore
import socket
import SimpleHTTPServer
import sys
import cgi
import cStringIO
import os
import traceback
import time
import zlib
import urlparse
import urllib
import re
from config import Config
sys.path.extend(Config.ADDITIONAL_PYTHON_LIBRARY_PATHS)
import simplejson
import time
import threading
from collections import deque


__version__ = '.1'


def popall(self):
  r = len(self) * [None]
  for i in xrange(len(r)):
    r[i] = self.popleft()
  return r


class WriteWrapper:
  """ Adapter for BaseHttpServer.BaseHTTPRequestHandler.wfile object.
  
  Utilized by RequestHandler to provide consistent method to write into
  output stream.
  
  Args:
    buffer: instance of buffer to append data to
  """
  def __init__(self, buffer):
    self.buffer = buffer


  def write(self, data):
    self.buffer.append(data)


class ParseHeaders(dict):
  """ Deprecated mimetools.Message class replacement.
  
  Works like a dictionary with case-insensitive keys. """

  def __init__(self, infile, *args):
    self._ci_dict = {}
    lines = infile.readlines()
    for line in lines:
      k,v = line.split(':', 1)
      self._ci_dict[k.lower()] = self[k] = v.strip()
    self.headers = self.keys()
  

  def getheader(self, key, default=''):
    return self._ci_dict.get(key.lower(), default)
  

  def get(self, key, default=''):
    return self._ci_dict.get(key.lower(), default)


# RequestHandler class extends the SimpleHTTPServer,
# adopts the naming convention from that library
class RequestHandler(asynchat.async_chat, 
                     SimpleHTTPServer.SimpleHTTPRequestHandler):
  """ TestWebserver request handler.
  
  Uses async IO to handle incoming request.
  """
  server_version = 'AsyncTestHTTPServer/' + __version__
  protocol_version = 'HTTP/1.1'
  MessageClass = ParseHeaders
  blocksize = 4096
  use_buffer = False
  write_slowly = False


  def __init__(self, conn, addr, server):
    asynchat.async_chat.__init__(self, conn)
    self.client_address = addr
    self.connection = conn
    self.server = server
    self.set_terminator ('\r\n\r\n')
    self.incoming = deque()
    self.outgoing = deque()
    self.rfile = None
    self.wfile = WriteWrapper(self.outgoing)
    self.found_terminator = self.handle_request_line
    self.request_version = 'HTTP/1.1'
    self.code = None
  

  def collect_incoming_data(self, data):
    if not data:
      self.ac_in_buffer = ''
    else:
      self.incoming.append(data)


  def prepare_POST(self):
    bytesToRead = int(self.headers.getheader('content-length'))
    self.set_terminator(bytesToRead)
    self.incoming.clear()
    self.found_terminator = self.handle_post_data
  

  def handle_post_data(self):
    self.rfile = cStringIO.StringIO(''.join(popall(self.incoming)))
    self.rfile.seek(0)
    self.do_POST()
     
      
  def do_POST(self):
    qspos = self.path.find('?')
    if qspos >= 0:
      # Added query to allow test server to run redirects at POST, and the 
      # workaround to allow redirect arg are to be a query argument. 
      self.query = cgi.parse_qs(self.path[qspos + 1:], keep_blank_values=1)
      self.path = self.path[:qspos]
    ctype, pdict = cgi.parse_header(self.headers.getheader('content-type'))
    length = int(self.headers.getheader('content-length'))
    if ctype == 'multipart/form-data':
      self.body = cgi.parse_multipart(self.rfile, pdict)
    elif ctype == 'application/x-www-form-urlencoded':
      qs = self.rfile.read(length)
      self.server.notifyOnDoPost(qs, self.path)
      self.body = cgi.parse_qs(qs, keep_blank_values=1)
    else:
      self.body = self.rfile.read(length)
    self.handle_data()


  def do_GET(self):
    qspos = self.path.find('?')
    if qspos >= 0:
      self.body = cgi.parse_qs(self.path[qspos + 1:], keep_blank_values=1)
      self.query = self.body
      self.path = self.path[:qspos]
    self.handle_data()
    

  def isCgiCall(self):
    """ Identify whether the request requires script execution.
    
    Extra server functionality is processed using python scriplets.
    """
    return re.match(r'(.*)(testcases/cgi/([A-Za-z0-9_]+).py)', self.path)
    
  
  def cgiFilePath(self, path):
    """ Translates the request path to the local path of the scriptlet.

    Args:
      path: path from the request
    
    Returns:
      Local path to the executable script
    """    
    relative_path = re.match(r'.*(testcases/cgi/([A-Za-z0-9_]+).py)$', path) \
      .group(1)
    return os.path.join(self.server.server_root_dir, relative_path)


  def serveCgi(self):
    """ Find and run the scriptlet to generate the dynamic response. """
    execfile(self.cgiFilePath(self.path))


  def handle_data(self):
    """ Prepare response per incoming GET request. """
    path = self.translate_path(self.path)

    # Dynamic response
    if self.isCgiCall():
      self.serveCgi()

    # Static response
    else:
      f = self.send_head()
      self.log_request(self.code)
      if f:
        self.outgoing.append(f)
    
    # Signal end of response
    self.outgoing.append(None)
    

  def handle_request_line(self):
    """ Handles new request. """
    self.rfile = cStringIO.StringIO(''.join(popall(self.incoming)))
    self.rfile.seek(0)
    self.raw_requestline = self.rfile.readline()
    self.parse_request()

    # Call the method requested, if supported.
    if self.command in ['GET','HEAD']:
      method = 'do_' + self.command
      if hasattr(self, method):
        getattr(self, method)()
    elif self.command == 'POST':
      self.prepare_POST()
    else:
      self.send_error(501, 'Unsupported method (%s)' % self.command)


  def end_headers(self):
    self.outgoing.append('\r\n')


  def handle_error(self):
    traceback.print_exc(sys.stderr)
    self.close()


  def writable(self):
    return len(self.outgoing) and self.connected 


  def handle_write(self):
    """ Sends next chunk of outgoing data from the queue. """
    if self.__has_empty_outgoing_buffer():
      return

    # Look for the next actual (nonzero) piece of data in outgoing
    # and meanwhile remove any non-data found.
    outBuff = self.outgoing
    while len(outBuff):
      data = outBuff.popleft()

      # End of response
      if data is None:
        self.close()
        return

      # If we're writing a file, get next chunk for sending.
      elif hasattr(data, 'read'):
        outgoing_file = data
        first_block = outgoing_file.read(self.blocksize)
        # File contains no more data, don't send.
        if not first_block:
          del outgoing_file
          continue
        else:
          outBuff.appendleft(outgoing_file)
          data = first_block
          break

      # If string data, send it.
      elif len(data):
        break

    # Send this data string, if it fails or fails partially, re-append
    # unsent data to outgoing.
    try:
      num_sent = self.send(data)
      if (self.write_slowly):
        time.sleep(0.01)
      if num_sent < len(data):
        if not num_sent:
          outBuff.appendleft(data)
        elif self.use_buffer:
          outBuff.appendleft(buffer(data, num_sent))
        else:
          outBuff.appendleft(data[num_sent:])

    except socket.error, m:
      if isinstance(m, (str, unicode)):
        self.log_error(m)
      elif isinstance(m, tuple) and isinstance(m[-1], (str, unicode)):
        self.log_error(m[-1])
      else:
        self.log_error(str(m))
      self.handle_error()


  def translate_path(self, path):
    """Translate URL path to OS path that will be served by the server. 

    There are special cases that server has to handle: some of the files 
    are above the server root directory and instead of copying them we 
    actually serve them from directories above server root. Firefox
    will trim '..' directory references if normalized path points to directory
    above root directory, so matching files by pattern and pointing to 
    the right location looks as the best option so far.
    """
    # abandon query parameters
    raw_path = urlparse.urlparse(path)[2]
    path = urllib.unquote(raw_path)

    for filename, new_path in Config.SPECIAL_URL_TO_FILE_MAPPINGS.iteritems():
      if path.find(filename) > -1:
        path = new_path
    
    if path.startswith('/'):
      path = path[1:]
    full_path = os.path.join(self.server.server_root_dir, path)
    return full_path


  def send_response(self, code, message=None):
    """ Send response code and headers. """
    if self.code:
      return
    self.code = code
    if message is None:
      if code in self.responses:
        message = self.responses[code][0]
      else:
        message = ''
    self.wfile.write('%s %d %s\r\n' %
                     (self.protocol_version, code, message))
    self.send_header('Server', self.version_string())
    self.send_header('Date', self.date_time_string())
    # This webserver shuts down the connection after every request.
    # This behavior can tickle bug 568 under Safari in some circumstances.
    # Be explicit that this is our behavior with the following header.
    # This also works around bug 568.
    self.send_header('Connection', 'close')
    if self.path.endswith('set_cookie.txt'):
      self.send_header('Set-Cookie', 'Gears_testwebserver=true')

  def log_message(self, format, *args):
    sys.stderr.write('%s - - [%s] %s "%s" "%s"\n' %
                     (self.address_string(),
                      self.log_date_time_string(),
                      format % args,
                      self.headers.get('referer', ''),
                      self.headers.get('user-agent', '')))


  def __has_empty_outgoing_buffer(self):  
    if len(self.outgoing):
      return False
    else:
      return True     


class TestWebserver(asyncore.dispatcher):
  """ Webserver implementation that allows controlling its lifecycle.
  
  Used together with Gears JavaScript tests.
   
  Args:
    server_root_dir: directory that will be server root
    port: listening port
    ip: ip of the server
    handler: BaseHTTPServer.BaseHTTPRequestHandler and
      asynchat.async_chat implementation
  """
  RESULTS_POSTBACK_FILE_PATTERN = 'gui.html'

  def __init__ (self, server_root_dir, port=8001, ip='0.0.0.0',
                handler=RequestHandler):
    self.ip = ip
    self.port = port
    self.HandlerClass = handler
    asyncore.dispatcher.__init__(self)
    self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
    self.set_reuse_addr()
    self.bind((ip, port))
    self.listen(5)
    self.server_root_dir = self.__makePathAbsolute(server_root_dir)
    self.json_test_result = '' 
    self.running = False
    self.test_completion_lock = threading.Event()
    self.test_instance_is_running = False


  def startServing(self):
    """ Starts the webserver in another thread. """
    if self.running:
      raise StandardError('Server was already started')
    

    class ServerThread(threading.Thread):
      def __init__(self, server):
        self.server = server
        threading.Thread.__init__(self)

      def run(self):
        self.server.running = True
        self.server.startLooping()
        self.server.running = False
        
    thread = ServerThread(self)
    thread.start()
  

  def startLooping(self):
    """ Asyncore call that makes server start processing requests. """
    asyncore.loop(use_poll=True)


  def shutdown(self): 
    self.close()


  def notifyOnDoPost(self, post_data, url):
    """ Called by request handler on POST.
    
    Handler should call the method on POST and the data is processes 
    if the URL matches TestWebserver.RESULTS_POSTBACK_FILE_PATTERN that
    is used to receive in-browser test results.
    """
    if (self.test_instance_is_running and 
        url.find(TestWebserver.RESULTS_POSTBACK_FILE_PATTERN) > -1):
      
      if self.test_completion_lock.isSet():
        raise StandardError('Test completion lock was set, that '
                            'is an illigal state.')
      self.json_test_result = simplejson.loads(post_data)
      self.test_completion_lock.set()


  def startTest(self, test_timeout):
    """ Starts test specific to one browser. """
    if self.test_instance_is_running:
      raise StandardError('One of the tests is already running.')

    self.test_timeout = test_timeout
    self.test_instance_is_running = True
    self.test_completion_lock.clear()
    self.json_test_result = ''


  def testResults(self):
    """ Provide in-browser test results in hash format.
    
    This is an overloaded method and represents:
    1. Querying test results; and
    2. Asking testcases to terminate after 'test_timout' value specified
      in startTest() method parameter if the tests haven't been completed 
      between calling startTest() and invoking this method.
    """
    if not self.test_instance_is_running:
      raise StandardError('None of the tests were started')
    
    self.test_completion_lock.wait(self.test_timeout)
    test_result = ''
    if self.test_completion_lock.isSet():
      test_result = self.json_test_result
    else:
      print ('Browser tests failed to complete and timed out.  '
             'It is likely that either the test webserver failed '
             'to start, or one of the test cases hung.')
      test_result = 'TIMED-OUT'
    self.test_completion_lock.clear()
    self.test_instance_is_running = False
    self.json_test_result = ''
    return test_result


  def handle_accept(self):
    """ Overridden asyncore.dispatcher.handle_accept(). """
    try:
      conn, addr = self.accept()
    except socket.error:
      self.log_info ('warning: server accept() threw an exception', 'warning')
      return
    except TypeError:
      self.log_info ('warning: server accept() threw EWOULDBLOCK', 'warning')
      return
    self.HandlerClass(conn, addr, self)


  def __makePathAbsolute(self, root_directory):
    """ Resolve root directory to an absolute path.
    
    Uses current directory as a starting point. 
    """
    if os.path.isabs(root_directory):
      return root_directory
    else:
      return os.path.join(os.getcwd(), root_directory)


def server_root_dir():
  return os.path.join(os.path.dirname(__file__), '../')


if __name__ == '__main__':
  if len(sys.argv) > 1:
    try:
      port_number = int(sys.argv[1])
    except:
      print ('usage:\nArgument must be an integer'
             'port number, or blank for default.')
      sys.exit()
  else:
    port_number = 8001

  # Instantiating second server for cross domain tests.
  TestWebserver(server_root_dir(), port=port_number)
  TestWebserver(server_root_dir(), port=(port_number + 1)).startServing()
