import os

class Config:
  """ Contains constants with values that can change frequently. 
  
  Lists some of the files that can't be served from the root directory and
  provide path to the files relative to server root directory. Usually 
  thirdparty libraries fall into this category, and we decided to take this 
  approach instead of copying files and keeping them in sync manually.
  """
  SPECIAL_URL_TO_FILE_MAPPINGS = {
    "gears_init.js": "../sdk/gears_init.js",
    "json_noeval.js": "../../third_party/jsonjs/json_noeval.js"
  }
  
  # List of paths to libraries that are bundled with runner and
  # need to be available in the path before the bootstrap is invoked.
  ADDITIONAL_PYTHON_LIBRARY_PATHS = [
    os.path.join(os.path.dirname(__file__), '../../../third_party'),
    os.path.join(os.path.dirname(__file__), '../../../third_party/pexpect')
  ]
