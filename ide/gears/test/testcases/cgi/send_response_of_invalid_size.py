size = int(self.query['size'][0])

self.send_response(200)
self.send_header('Content-Type', 'text/plain')
self.send_header('Content-Length', size)
self.send_header('X-Gears-Decoded-Content-Length', size)
self.end_headers()

# Send data in blocks.  Less memory intensive than allocating
# and sending one big string, but faster than sending one 
# byte at a time.  This is a compromise, because there are some
# tests that want to send large quantities of data quickly, 
# and others that want to interact with the httprequest object
# during transfer.
block_size = 1024 * 16
num_blocks = (size - 1) / block_size
block_data = 'a' * block_size
remainder = 'a' * ((size - 1) % block_size)

for i in xrange(num_blocks):
  self.outgoing.append(block_data)
self.outgoing.append(remainder)
