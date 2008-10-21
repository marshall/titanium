import os
import sys
import BaseHTTPServer
import SimpleHTTPServer

def main():
  # If present, argv[1] is the directory to use as the localhost root.
  if len(sys.argv) > 1:
    os.chdir(sys.argv[1])

  print '''
Started HTTP server.  http://localhost/... maps to:
  %s%s...
Press Ctrl-Break or Ctrl-C to exit.
''' % (os.getcwd(), os.sep)

  server_class = BaseHTTPServer.HTTPServer
  handler_class = SimpleHTTPServer.SimpleHTTPRequestHandler

  server_addr = ('', 80)
  httpd = server_class(server_addr, handler_class)
  httpd.serve_forever()

if __name__ == '__main__':
  main()
