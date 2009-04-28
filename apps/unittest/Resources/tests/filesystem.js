function createDirTree(name) {
	var dir = Titanium.Filesystem.getFile(base, name);
	if(! dir.exists()) {
		dir.createDirectory();
	}
	
	var file1 = Titanium.Filesystem.getFileStream(dir, "file1.txt");
	var file2 = Titanium.Filesystem.getFileStream(dir, "file2.txt");
	var subDir1 = Titanium.Filesystem.getFile(dir, "subDir1");
	subDir1.createDirectory();
	var file3 = Titanium.Filesystem.getFileStream(subDir1, "file3.txt");
	
	file1.open(Titanium.Filesystem.FILESTREAM_MODE_WRITE);
	file1.write("Text for file1");
	file1.close();
	
	file2.open(Titanium.Filesystem.FILESTREAM_MODE_WRITE);
	file2.write("Text for file2");
	file2.close();
	
	file3.open(Titanium.Filesystem.FILESTREAM_MODE_WRITE);
	file3.write("Text for file3");
	file3.close();
}

Titanium.AppTest.addResult('filesystem.namespace',typeof(Titanium.Filesystem)=='object');

Titanium.AppTest.addResult('filesystem.method_getApplicationDataDirectory',typeof(Titanium.Filesystem.getApplicationDataDirectory)=='function');
Titanium.AppTest.addResult('filesystem.method_getFile',typeof(Titanium.Filesystem.getFile)=='function');
Titanium.AppTest.addResult('filesystem.method_getFileStream',typeof(Titanium.Filesystem.getFileStream)=='function');

// clean up testing folder if needed
var base = Titanium.Filesystem.getFile(Titanium.Filesystem.getApplicationDataDirectory(), "unittest_filesystem");
if(base.exists() && base.isDirectory()) {
	base.deleteDirectory(true);
}
else if(base.exists() && base.isFile()) {
	base.deleteFile();
}

base.createDirectory();

/** getFile **/
try {
	var f = Titanium.Filesystem.getFile(base, "getFileTest.txt");
	Titanium.AppTest.addResult('filesystem.getFile',f!=null,"file object should not be null");
}
catch (e) {
	Titanium.AppTest.addResult('filesystem.getFile',false,"failed with exception: "+e);
}

/** getFileStream **/
try {
	var fs = Titanium.Filesystem.getFile(base, "getFileStreamTest.txt");
	Titanium.AppTest.addResult('filesystem.getFile',fs!=null,"filestream object should not be null");
}
catch (e) {
	Titanium.AppTest.addResult('filesystem.getFileStream',false,"failed with exception: "+e);
}

/** Temp file **/
try {
	Titanium.AppTest.addResult('filesystem.method_createTempFile',typeof(Titanium.Filesystem.createTempFile)=='function');
	
	var f = Titanium.Filesystem.createTempFile();
	Titanium.AppTest.addResult('filesystem.createTempFile',f != null,"temp file object should not be null");
	Titanium.AppTest.addResult('filesystem.exists',f.exists(),"created temp file should exist");
}
catch (e) {
	Titanium.AppTest.addResult('filesystem.createTempFile',false,"failed with exception: "+e);
}

/** Temp directory **/
try {
	Titanium.AppTest.addResult('filesystem.method_createTempDirectory',typeof(Titanium.Filesystem.createTempDirectory)=='function');
	
	var f = Titanium.Filesystem.createTempDirectory();
	Titanium.AppTest.addResult('filesystem.createTempDirectory',f != null,"temp directory object should not be null");
	Titanium.AppTest.addResult('filesystem.exists',f.exists(),"created temp directory should exist");
}
catch (e) {
	Titanium.AppTest.addResult('filesystem.createTempDirectory',false,"failed with exception: "+e);
}

/** Common directories **/
try {
	Titanium.AppTest.addResult('filesystem.method_getProgramsDirectory',typeof(Titanium.Filesystem.getProgramsDirectory)=='function');
	Titanium.AppTest.addResult('filesystem.method_getApplicationDirectory',typeof(Titanium.Filesystem.getApplicationDirectory)=='function');
	Titanium.AppTest.addResult('filesystem.method_getRuntimeHomeDirectory',typeof(Titanium.Filesystem.getRuntimeHomeDirectory)=='function');
	Titanium.AppTest.addResult('filesystem.method_getResourcesDirectory',typeof(Titanium.Filesystem.getResourcesDirectory)=='function');
	Titanium.AppTest.addResult('filesystem.method_getDesktopDirectory',typeof(Titanium.Filesystem.getDesktopDirectory)=='function');
	Titanium.AppTest.addResult('filesystem.method_getDocumentsDirectory',typeof(Titanium.Filesystem.getDocumentsDirectory)=='function');
	Titanium.AppTest.addResult('filesystem.method_getUserDirectory',typeof(Titanium.Filesystem.getUserDirectory)=='function');
	
	Titanium.AppTest.addResult('filesystem.getProgramsDirectory',Titanium.Filesystem.getProgramsDirectory()!=null);
	Titanium.AppTest.addResult('filesystem.getApplicationDirectory',Titanium.Filesystem.getApplicationDirectory()!=null);
	Titanium.AppTest.addResult('filesystem.getRuntimeHomeDirectory',Titanium.Filesystem.getRuntimeHomeDirectory()!=null);
	Titanium.AppTest.addResult('filesystem.getResourcesDirectory',Titanium.Filesystem.getResourcesDirectory()!=null);
	Titanium.AppTest.addResult('filesystem.getDesktopDirectory',Titanium.Filesystem.getDesktopDirectory()!=null);
	Titanium.AppTest.addResult('filesystem.getDocumentsDirectory',Titanium.Filesystem.getDocumentsDirectory()!=null);
	Titanium.AppTest.addResult('filesystem.getUserDirectory',Titanium.Filesystem.getUserDirectory()!=null);
}
catch (e) {
	Titanium.AppTest.addResult('filesystem.commonDirectories',false,"failed with exception: "+e);
}

/** constants, properties **/
try {
	Titanium.AppTest.addResult('filesystem.method_getLineEnding',typeof(Titanium.Filesystem.getLineEnding)=='function');
	Titanium.AppTest.addResult('filesystem.method_getSeparator',typeof(Titanium.Filesystem.getSeparator)=='function');
	Titanium.AppTest.addResult('filesystem.property_FILESTREAM_MODE_READ',typeof(Titanium.Filesystem.FILESTREAM_MODE_READ)=='string');
	Titanium.AppTest.addResult('filesystem.property_FILESTREAM_MODE_WRITE',typeof(Titanium.Filesystem.FILESTREAM_MODE_WRITE)=='string');
	Titanium.AppTest.addResult('filesystem.property_FILESTREAM_MODE_APPEND',typeof(Titanium.Filesystem.FILESTREAM_MODE_APPEND)=='string');
	
	Titanium.AppTest.addResult('filesystem.getLineEnding',Titanium.Filesystem.getLineEnding()!=null);
	Titanium.AppTest.addResult('filesystem.getSeparator',Titanium.Filesystem.getSeparator()!=null);
}
catch (e) {
	Titanium.AppTest.addResult('filesystem.commonDirectories',false,"failed with exception: "+e);
}

/** root directories **/
try {
	Titanium.AppTest.addResult('filesystem.method_getRootDirectories',typeof(Titanium.Filesystem.getRootDirectories)=='function');
	
	var rootDirs = Titanium.Filesystem.getRootDirectories();
	Titanium.AppTest.addResult('filesystem.getRootDirectories',rootDirs!=null,"getRootDirectories() should not return null");
	Titanium.AppTest.addResult('filesystem.getRootDirectories',rootDirs.length>0,"There should be at least one root directory");
	var rootDirFirst = rootDirs[0];
	Titanium.AppTest.addResult('filesystem.getRootDirectories',typeof(rootDirFirst)=='object',"The items returned must be File objects");
	Titanium.AppTest.addResult('filesystem.getRootDirectories',rootDirFirst.isDirectory(),"The items returned must be directories");
}
catch (e) {
	Titanium.AppTest.addResult('filesystem.getRootDirectories',false,"failed with exception: "+e);
}

/** Async copy **/
try {
	var fromDir = Titanium.Filesystem.getFile(base, "ayncCopyFrom");
	var toDir = Titanium.Filesystem.getFile(base, "asynCopyTo");
	
	createDirTree("ayncCopyFrom");
	Titanium.Filesystem.asyncCopy(fromDir,toDir,function() {
		var listings = toDir.getDirectoryListing();
		Titanium.AppTest.addResult('filesystem.asyncCopy',listings!=null,'unable to read file names in the folder copied');
		Titanium.AppTest.addResult('filesystem.asyncCopy',listings.length==3,'folder copied should contain 3 files/folders');
		
		var toSubDir1 = Titanium.Filesystem.getFile(fromDir, "subDir1");
		Titanium.AppTest.addResult('filesystem.asyncCopy',toSubDir1.isDirectory(),'subDir1 should be a directory');
		
		var subDirListings = toSubDir1.getDirectoryListing();
		Titanium.AppTest.addResult('filesystem.asyncCopy',subDirListings!=null,'unable to read file names in the folder copied');
		Titanium.AppTest.addResult('filesystem.asyncCopy',subDirListings.length==1,'subDir1 in folder copied should contain 1 files/folders');
	});
	
}
catch (e) {
	Titanium.AppTest.addResult('filesystem.asyncCopy',false,"failed with exception: "+e);
}
