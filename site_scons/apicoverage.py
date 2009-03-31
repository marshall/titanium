#
# Titanium API Coverage Generator
#
# Initial Author: Jeff Haynie, 3/30/09
#
import glob, re, os.path as path
import fnmatch, os
import simplejson as json

class GlobDirectoryWalker:
    # a forward iterator that traverses a directory tree

    def __init__(self, directory, pattern="*"):
        self.stack = [directory]
        self.pattern = pattern
        self.files = []
        self.index = 0

    def __getitem__(self, index):
        while 1:
            try:
                file = self.files[self.index]
                self.index = self.index + 1
            except IndexError:
                # pop next directory from stack
                self.directory = self.stack.pop()
                self.files = os.listdir(self.directory)
                self.index = 0
            else:
                # got a filename
                fullname = os.path.join(self.directory, file)
                if os.path.isdir(fullname) and not os.path.islink(fullname):
                    self.stack.append(fullname)
                if fnmatch.fnmatch(file, self.pattern):
                    return fullname

	
def generate_api_coverage(dir,fs):
	pattern = '@tiapi\(([^\)]*)\)\s+(.*)'
	
	extensions = ['cc','c','cpp','m','mm','js']

	files = []
	modules = {}

	for ext in extensions:
		for i in GlobDirectoryWalker(dir,'*.'+ext):
			files.append(i)

	for f in files:
		fh = open(str(f),'r')
		content = fh.read()
		for m in re.finditer(pattern,content):
			description = m.group(2).strip()
			metadata = {}
			for value in m.group(1).strip().split(','):
				key,value = value.split('=')
				# trivial type conversions
				if value == 'True' or value == 'true':
					value = True
				elif value == 'False' or value == 'false':
					value = False
				elif re.match('^[0-9]+$',value):
					value = int(value)
				elif re.match('^[0-9\.]+$',value):
					value = float(value)
				metadata[key]=value
				
			api = metadata['name']
			del metadata['name']
			tok = api.split('.')
			module = tok[0]
			entry = {}
			if modules.has_key(module):
				entry = modules[module]
			else:
				modules[module] = entry
			tok.pop(0)
			name = '.'.join(tok)
			if not entry.has_key(name):
				entry[name]={'metadata':metadata,'description':description}

	fs.write(json.dumps(modules, sort_keys=True, indent=4))

#
#EXAMPLE:
#
#import sys
#generate_api_coverage('.',sys.stdout)