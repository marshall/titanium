
import os, os.path as path

class TitaniumSDK:
	
	def __init__(self):
		if Titanium.platform is 'win32':
			self.sdk_root = path.join('C:', 'ProgramData', 'Titanium')
		elif Titanium.platform is 'linux':
			self.sdk_root = "" # ?
		elif Titanium.platform is 'osx':
			self.sdk_root = path.join(path.expanduser('~'), 'Library', 'Titanium')
			
	
	def getInstalledModules(self):
		modules = []
		for path in os.listdir(path.join(self.sdk_root, "modules")):
			modules += path
		return modules

	def makeVersion(self, ver):
		if ver is None:
			return 0
		
		v = ""
		pos = 0
		while True:
			newpos = ver.find(".",pos);
			if newpos is -1:
				v += ver[pos:len(ver)]
				break
			v += ver[pos:newpos]
			pos = newpos + 1
		
		return int(v)
	
	def compareVersions(self, a, b):
		return self.makeVersion(a) - self.makeVersion(b)
	
	def getLatestVersionDir(self,dir):
		if path.exists(dir):
			latestVersion = None
			for path in os.listdir(dir):
				if self.compareVersions(latestVersion, path) < 0:
					latestVersion = path 
			
			return latestVersion
		return None
	
	def getLatestModuleVersion(self,module):
		return self.getLatestVersionDir(path.join(self.sdk_root, "modules", module))
	
	def getLatestRuntimeVersion(self):
		return self.getLatestVersionDir(path.join(self.sdk_root, "runtime"))
	
	def getSDKDir(self):
		return self.sdk_root
	
	def findRuntimeDir(self, projectDir):
		runtimeVersion = self.getLatestRuntimeVersion()
		if runtimeVersion is None:
			projectRuntimeDir = path.join(projectDir, 'runtime')
			if path.exists(projectRuntimeDir):
				return projectRuntimeDir
			else:
				sys.stderr.write("Error locating Titanium runtime!\n")
				return None
			
		return self.getRuntimeDir()
			
		
	def findModuleDir(self, projectDir, module):
		moduleVersion = self.getLatestModuleVersion(module)
		if moduleVersion is None:
			projectModuleDir = path.join(projectDir, 'modules', module)
			if path.exists(projectModuleDir):
				return projectModuleDir
			else:
				sys.stderr.write("Error locating Titanium module: %s\n" % module)
				return None
			
		return self.getModuleDir(module)
	
	def getModuleDir(self, module):
		return path.join(self.sdk_root, 'modules', module, self.getLatestModuleVersion(module))
		
	def getRuntimeDir(self):
		return path.join(self.sdk_root, 'runtime', self.getLatestRuntimeVersion())