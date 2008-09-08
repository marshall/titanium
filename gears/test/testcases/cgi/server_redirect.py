def location(path, create_full_url = False):
  scheme = urlparse.urlparse(path)[0]
  if scheme:
    return path
  else:
    if create_full_url:
      # URL is always going to be http://localhost:8001 (unless we will 
      # implement https scheme), the other alternative to get hostname  would 
      # be using gethostbyaddr() but it will resolve to actual host name.
      
      # join method will ignore first first path if 
      # second argument has a forward slash
      file_path = os.path.normpath(os.path.join("/testcases/cgi", path))
      full_url = ("http://localhost:8001%s" %file_path)
      return full_url
    else:
      return path
        
if self.query and self.query.has_key('location'):
  self.send_response(302)
  url = location(self.query['location'][0], create_full_url=self.query.has_key('full'))
  self.send_header('location', '%s' %url)
  self.end_headers()
else:
  self.send_reponse(200)
  self.end_headers()
  self.outgoing.append('Please provide "location" parameter')
