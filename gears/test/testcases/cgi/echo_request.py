self.send_response(200)
self.send_header('Content-type', 'text/html')
self.send_header('echo-Method', self.command)
for header, value in self.headers.iteritems():
  self.send_header( ('echo-%s' %header), value)

self.end_headers()

if self.command == 'POST':
  if self.body:
    self.outgoing.append('%s' %self.body)
self.outgoing.append (None)
