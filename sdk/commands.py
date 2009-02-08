
import os, os.path as path
import shutil
import glob, distutils.dir_util as dir_util
from sdk import TitaniumSDK
from xml.dom.minidom import getDOMImplementation, parse
from properties import Properties

sdk = TitaniumSDK()

class CreateProject:
	def execute(self, args):
		if len(args) is 0:
			printUsage()
		
		self.projectName = args[0]
		self.selfDir = path.dirname(path.realpath(__file__))
		self.projectDir = path.abspath(self.projectName)
		self.resourcesDir = path.join(self.selfDir, 'Resources')
		
		if not path.exists(self.projectDir):
			os.mkdir(self.projectDir)
		
		self.generateManifest()
		self.generateTiApp()
		self.copyResources()
		
	def generateManifest(self):
		properties = Properties()
		properties.setProperty('appname', self.projectName)
		properties.setProperty('appid', 'com.titaniumapp.%s' % self.projectName)
		runtimeVersion = sdk.getLatestRuntimeVersion()
		properties.setProperty('runtime', runtimeVersion)
		for module in sdk.getInstalledModules():
			version = sdk.getLatestModuleVersion(module)
			if version is not None:
				properties.setProperty(module, version)
		
		manifest = open(path.join(self.projectDir, 'manifest'), 'w+')
		properties.store(manifest)
	
	def appendElement(self, parent, name, value):
		el = self.doc.createElement(name)
		el.appendChild(self.doc.createTextNode(value))
		parent.appendChild(el)
		
	def generateTiApp(self):
		tiApp = open(path.join(self.projectDir, 'tiapp.xml'), 'w+')
		impl = getDOMImplementation()
		self.doc = impl.createDocument(None, "ti:app", None)
		rootEl = doc.documentElement
		self.appendElement(rootEl, "id", "com.titaniumapp."+self.projectName)
		self.appendElement(rootEl, "name", self.projectName)
		self.appendElement(rootEl, "version", "0.1")
		
		windowEl = doc.createElement("window")
		rootEl.appendChild(windowEl)
		self.appendElement(windowEl, "id", "initial")
		self.appendElement(windowEl, "title", "Titanium App: " + self.projectName)
		self.appendElement(windowEl, "url", "app://index.html")
		self.appendElement(windowEl, "width", "800")
		self.appendElement(windowEl, "height", "600")
		self.appendElement(windowEl, "fullscreen", "false")
		self.appendElement(windowEl, "maximizable", "true")
		self.appendElement(windowEl, "minimizable", "true")
		self.appendElement(windowEl, "closeable", "true")
		self.appendElement(windowEl, "resizable", "true")
		
		tiApp.write(self.doc.toprettyxml())
		tiApp.close()
	
	def copyResources(self):
		shutil.copytree(self.resourcesDir, path.join(self.projectDir, 'Resources'))
		
		
class PackageProject:
	
	def execute(self, args):
		self.projectDir = path.cwd()
		self.selfDir = path.dirname(path.realpath(__file__))
		self.manifest = Properties()
		manifestFile = open(path.join(self.projectDir, 'manifest'), 'r')
		self.manifest.load(manifestFile)
		
		self.projectName = self.manifest.getProperty('appname')
		self.projectResourcesDir = path.join(self.projectDir, 'Resources')
		self.tiApp = parse(self.join(self.projectDir, 'tiapp.xml'))
	
		self.package()	
	
	def package(self):
		kboot = path.join(sdk.getRuntimeDir(), 'kboot')
		if Titanium.platform is 'win32':
			kboot += ".exe"
		
		stageDir = path.join(self.projectDir, 'stage')
		appDir = path.join(stageDir, self.projectName)
		if Titanium.platform is 'osx':
			appDir += ".app"
			
		appRuntimeDir = path.join(appDir, 'runtime');
		appModulesDir = path.join(appDir, 'modules');
		
		if path.isdir(appDir):
			dir_util.remove_tree(appDir)
		
		for d in [appDir, appRuntimeDir, appModulesDir]:
			os.makedirs(d)
		
		# Gather all runtime third-party libraries
		for lib in runtime_libs:
			path = p.join(build.dir, 'lib' + lib+ '.so')
			shutil.copy(path, runtime)
		
		pattern = path.join(sdk.findRuntimeDir(self.projectDir), 'lib', '*')
		for d in glob.glob(pattern):
			shutil.copy(d, runtime)
		
		# Gather all module libs
		modules = []
		for p in self.manifest.propertyKeys():
			if p is not 'appname' and p is not 'appid' and p is not 'runtime':
				modules += p
				
		for m in modules:
			mlib = None
			if Titanium.platform is 'win32':
				mlib = path.join(sdk.findModuleDir(self.projectDir, m), m + '.dll')
			elif Titanium.platform is 'linux':
				mlib = path.join(sdk.findModuleDir(self.projectDir, m), 'lib' + m + '.so')
			elif Titanium.platform is 'osx':
				mlib = path.join(sdk.findModuleDir(self.projectDir, m), 'lib' + m + '.dylib')
			
			outDir = path.join(appModulesDir, m)
			os.makedirs(outDir)
			shutil.copy(mlib, outDir)
		
		executableName = path.join(appDir, self.projectName)
		if Titanium.platform is 'win32':
			executableName += ".exe"
		
		shutil.copy(kboot, executableName)
		dir_util.copy_tree(self.projectResourcesDir, path.join(appDir, "Resources"), preserve_symlinks=True)
		shutil.copy(path.join(self.projectDir, 'tiapp.xml'), path.join(appDir, 'tiapp.xml'))
		shutil.copy(path.join(self.projectDir, 'manifest'), path.join(appDir, 'manifest'))
	
sdk_commands["create:project"] = CreateProject()