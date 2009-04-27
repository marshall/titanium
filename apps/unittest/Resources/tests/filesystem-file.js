function createFile(name,text) {
	var fs = Titanium.Filesystem.getFileStream(base, name);
	fs.open(Titanium.Filesystem.FILESTREAM_MODE_WRITE);
	fs.write(text);
	fs.close();
}

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

// clean up testing folder if needed
var base = Titanium.Filesystem.getFile(Titanium.Filesystem.getApplicationDataDirectory(), "unittest_filesystem_file");

if(base.exists() && base.isDirectory()) {
	base.deleteDirectory(true);
}
//alert("a2");
else if(base.exists() && base.isFile()) {
	base.deleteFile();
}
//alert("a3");
base.createDirectory();

/** File props **/
try {
	var f = Titanium.Filesystem.getFile(base, "newFile.txt");
	Titanium.AppTest.addResult('filesystem.file.fileProps',f!=null);
	Titanium.AppTest.addResult('filesystem.file.fileProps',!f.exists(),"newFile.txt should not exist");
	
	// create new file to test props with
	createFile("filePropsTest.txt", "This is the text for the text file.");
	
	f = Titanium.Filesystem.getFile(base, "filePropsTest.txt");
	Titanium.AppTest.addResult('filesystem.file.fileProps',f!=null,"f should not be null");
	Titanium.AppTest.addResult('filesystem.file.fileProps',f.exists(),"file should exist");
	Titanium.AppTest.addResult('filesystem.file.fileProps',f.toString()!=null,"file toString should be valid");
	Titanium.AppTest.addResult('filesystem.file.fileProps',f.name()!=null,"file name should not be null");
	Titanium.AppTest.addResult('filesystem.file.fileProps',f.extension()=='txt',"file extension should be txt");
	Titanium.AppTest.addResult('filesystem.file.fileProps',f.isFile(),"file should be a file");
	Titanium.AppTest.addResult('filesystem.file.fileProps',!f.isDirectory(),"file should not be a directory");
	Titanium.AppTest.addResult('filesystem.file.fileProps',!f.isHidden(),"file should not be hidden");
	Titanium.AppTest.addResult('filesystem.file.fileProps',!f.isSymbolicLink(),"file should not be a symbolic link");
	Titanium.AppTest.addResult('filesystem.file.fileProps',!f.isExecutable(),"file should not be executable");
	Titanium.AppTest.addResult('filesystem.file.fileProps',!f.isReadonly(),"file should not be readonly");
	Titanium.AppTest.addResult('filesystem.file.fileProps',f.isWriteable(),"file should be writable");
	Titanium.AppTest.addResult('filesystem.file.fileProps',f.createTimestamp()!=null,"file should return valid create timestamp");
	Titanium.AppTest.addResult('filesystem.file.fileProps',f.modificationTimestamp()!=null,"file should return valid modificatino timestamp");
	Titanium.AppTest.addResult('filesystem.file.fileProps',f.nativePath()!=null,"file should return valid native path");
	Titanium.AppTest.addResult('filesystem.file.fileProps',f.size()!=null,"file size should not be null");
	
	// directory props
	f = Titanium.Filesystem.getFile(base, "dirPropsTest");
	Titanium.AppTest.addResult('filesystem.file.fileProps',f!=null,"directory object should not be null");
	Titanium.AppTest.addResult('filesystem.file.fileProps',!f.exists(),"directory should not exist yet");
	f.createDirectory();
	Titanium.AppTest.addResult('filesystem.file.fileProps',f.exists(),"directory should exist by now");
	Titanium.AppTest.addResult('filesystem.file.fileProps',!f.isHidden(),"directory should not be hidden");
	Titanium.AppTest.addResult('filesystem.file.fileProps',!f.isSymbolicLink(),"directory should not be symbolic link");
	Titanium.AppTest.addResult('filesystem.file.fileProps',!f.isExecutable(),"directory should not be executable");
	Titanium.AppTest.addResult('filesystem.file.fileProps',!f.isReadonly(),"directory should not be read only");
	Titanium.AppTest.addResult('filesystem.file.fileProps',f.isWriteable(),"directory should be writable");
	Titanium.AppTest.addResult('filesystem.file.fileProps',f.createTimestamp()!=null,"directory should return valid create timestamp");
	Titanium.AppTest.addResult('filesystem.file.fileProps',f.modificationTimestamp()!=null,"directory should return valid modification timestamp");
	Titanium.AppTest.addResult('filesystem.file.fileProps',f.nativePath()!=null,"directory should return valid native path");
	Titanium.AppTest.addResult('filesystem.file.fileProps',f.size()!=null,"directory size should not be null");
	Titanium.AppTest.addResult('filesystem.file.fileProps',f.spaceAvailable()!=null,"directory space available should not be null");
}
catch (e) {
	Titanium.AppTest.addResult('filesystem.file.fileProps',false,"failed with exception: "+e);
}

try {
	var f = f = Titanium.Filesystem.getFile(base, "fileTest-Resolve");
	Titanium.AppTest.addResult('filesystem.file.resolve',f!=null,"file object should not be null");
	Titanium.AppTest.addResult('filesystem.file.resolve',f.resolve("filename.txt")!=null,"file resolve should not return null");
}
catch (e) {
	Titanium.AppTest.addResult('filesystem.file.resolve',false,"failed with exception: "+e);
}


/** copy/move/rename/delete file **/
try {
	var f = Titanium.Filesystem.getFile(base, "fileToCopy.txt");
	Titanium.AppTest.addResult('filesystem.file.copy',f!=null,"file object should not be null");
	Titanium.AppTest.addResult('filesystem.file.copy',!f.exists(),"fileToCopy.txt should not exist");
	
	// create new file to test props with
	createFile("fileToCopy.txt","This ist he text for the test file.");
	
	var copiedF = Titanium.Filesystem.getFile(base, "copiedFile.txt");
	var r = f.copy(copiedF);
	Titanium.AppTest.addResult('filesystem.file.copy',r,"copy should have succeeded");
	Titanium.AppTest.addResult('filesystem.file.copy',copiedF.exists(),"copied file should exist");
	
	var movedF = Titanium.Filesystem.getFile(base, "movedFile.txt");
	r = copiedF.move(movedF);
	Titanium.AppTest.addResult('filesystem.file.move',r,"move should have succeeded");
	Titanium.AppTest.addResult('filesystem.file.move',movedF.exists(),"moved file should exist");
	
	var renamedF = Titanium.Filesystem.getFile(base, "renamedFile.txt");
	r = movedF.rename("renamedFile.txt");
	Titanium.AppTest.addResult('filesystem.file.rename',r,"rename should have succeeded");
	Titanium.AppTest.addResult('filesystem.file.rename',renamedF.exists(),"renamed file should exist");
	
	r = renamedF.deleteFile();
	Titanium.AppTest.addResult('filesystem.file.delete',r,"delete should have succeeded");
}
catch (e) {
	Titanium.AppTest.addResult('filesystem.file.copy',false,"failed with exception: "+e);
}

/** create/delete directory **/
try {
	var d = Titanium.Filesystem.getFile(base, "playDirectory");
	Titanium.AppTest.addResult('filesystem.file.directory',d!=null,"directory object should not be null");
	Titanium.AppTest.addResult('filesystem.file.directory',!d.exists(),"playDirectory should not exist");
	
	// create new directory
	var r = d.createDirectory();
	Titanium.AppTest.addResult('filesystem.file.directoryCreate',r,"create directory should have succeeded");
	Titanium.AppTest.addResult('filesystem.file.directory',d.exists(),"playDirectory should exist by now");
	
	r = d.deleteDirectory();
	Titanium.AppTest.addResult('filesystem.file.directoryDelete',r,"delete directory should have succeeded");
	Titanium.AppTest.addResult('filesystem.file.directory',!d.exists(),"playDirectory should not exist anymore");
}
catch (e) {
	Titanium.AppTest.addResult('filesystem.file.directory',false,"failed with exception: "+e);
}

/** directory listing **/
try {
	var d = Titanium.Filesystem.getFile(base, "directoryListingTest");
	Titanium.AppTest.addResult('filesystem.file.directoryListing',d!=null,"directory object should not be null");
	Titanium.AppTest.addResult('filesystem.file.directoryListing',!d.exists(),"directoryListingTest should not exist");
	
	createDirTree("directoryListingTest");
	Titanium.AppTest.addResult('filesystem.file.directoryListing',d.exists(),"directoryListingTest should exist by now");

	var listings = d.getDirectoryListing();
	Titanium.AppTest.addResult('filesystem.file.directoryListing',listings!=null,'unable to read file names in the folder copied');
	Titanium.AppTest.addResult('filesystem.file.directoryListing',listings.length==3,'folder copied should contain 3 files/folders');
	
	var subDir1 = Titanium.Filesystem.getFile(d, "subDir1");
	Titanium.AppTest.addResult('filesystem.file.directoryListing',subDir1.isDirectory(),'subDir1 should be a directory');
	
	var subDirListings = subDir1.getDirectoryListing();
	Titanium.AppTest.addResult('filesystem.fiel.directoryListing',subDirListings!=null,'unable to read file names in the folder copied');
	Titanium.AppTest.addResult('filesystem.file.directoryListing',subDirListings.length==1,'subDir1 in folder copied should contain 1 files/folders');
}
catch (e) {
	Titanium.AppTest.addResult('filesystem.file.directoryListing',false,"failed with exception: "+e);
}

/** parent **/
try {
	var f = Titanium.Filesystem.getFile(base, "parentTestFile.txt");
	Titanium.AppTest.addResult('filesystem.file.parent',f!=null,"file object should not be null");
	Titanium.AppTest.addResult('filesystem.file.parent',!f.exists(),"parentTestFile.txt should not exist");
	
	var parent = f.parent();
	Titanium.AppTest.addResult('filesystem.file.parent',parent.toString()==base.toString(),"parent toString should equal base toString");
}
catch (e) {
	Titanium.AppTest.addResult('filesystem.file.directoryListing',false,"failed with exception: "+e);
}

/** create shortcut **/
try {
	createFile("shotcutTestFile.txt","text for test file");
	var f = Titanium.Filesystem.getFile(base, "shortcutTestFile.txt");
	Titanium.AppTest.addResult('filesystem.file.shortcut',f!=null,"file object should not be null");
	Titanium.AppTest.addResult('filesystem.file.shortcut',f.exists(),"shortcutTestFile.txt should exist");
	
	var shortcutFile = Titanium.Filesystem.getFile(base, "my-shortcut");
	var r = f.createShortcut(shortcutFile);
	Titanium.AppTest.addResult('filesystem.file.shortcut',r,"create shortcut should have succeeded");
	Titanium.AppTest.addResult('filesystem.file.shortcut',shortcutFile.exists(),"shortcut file should exist");
}
catch (e) {
	Titanium.AppTest.addResult('filesystem.file.directoryListing',false,"failed with exception: "+e);
}

/** executable/readonly/writable **/
try {
	createFile("permissionsTestFile.txt","text for test file");
	var f = Titanium.Filesystem.getFile(base, "permissionsTestFile.txt");
	Titanium.AppTest.addResult('filesystem.file.permissions',f!=null,"file object should not be null");
	Titanium.AppTest.addResult('filesystem.file.permissions',f.exists(),"permissionsTestFile.txt should exist by now");
	Titanium.AppTest.addResult('filesystem.file.permissions',!f.isExecutable(),"file should not be executable");
	Titanium.AppTest.addResult('filesystem.file.permissions',!f.isReadonly(),"file should not be read only");
	Titanium.AppTest.addResult('filesystem.file.permissions',f.isWriteable(),"file should be writeable");
	
	f.setExecutable(true);
	Titanium.AppTest.addResult('filesystem.file.permissions',f.isExecutable(),"file should be executable");
	
	f.setReadonly(false);
	Titanium.AppTest.addResult('filesystem.file.permissions',!f.isReadonly(),"file should not be read only");
	
	f.setWriteable(false);
	Titanium.AppTest.addResult('filesystem.file.permissions',!f.isWriteable(),"file should not be writable");
}
catch (e) {
	Titanium.AppTest.addResult('filesystem.file.directoryListing',false,"failed with exception: "+e);
}