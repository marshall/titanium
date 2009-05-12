Titanium.AppTest.addResult('filesystem.namespace',typeof(Titanium.Filesystem)=='object');

Titanium.AppTest.addResult('filesystem.method_getApplicationDataDirectory',typeof(Titanium.Filesystem.getApplicationDataDirectory)=='function');
Titanium.AppTest.addResult('filesystem.method_getFile',typeof(Titanium.Filesystem.getFile)=='function');
Titanium.AppTest.addResult('filesystem.method_getFileStream',typeof(Titanium.Filesystem.getFileStream)=='function');

// clean up testing folder if needed
var base = Titanium.Filesystem.getFile(Titanium.Filesystem.getApplicationDataDirectory(), "unittest_filesystem_filestream");
if(base.exists() && base.isDirectory()) {
	base.deleteDirectory(true);
}
else if(base.exists() && base.isFile()) {
	base.deleteFile();
}

base.createDirectory();

/** write/read **/
try {
	var textToWrite = "This is the text to write in the file";
	var fs = Titanium.Filesystem.getFileStream(base, "writeTestFile.txt");
	fs.open(Titanium.Filesystem.MODE_WRITE);
	fs.write(textToWrite);
	fs.close();
	
	var f = Titanium.Filesystem.getFile(base, "writeTestFile.txt");
	Titanium.AppTest.addResult('filesystem.filestream.write',f!=null,"f should not be null");
	Titanium.AppTest.addResult('filesystem.filestream.write',f.exists(),"writeTestFile.txt should exist");
	
	// read back the file contents
	fs.open(Titanium.Filesystem.MODE_READ);
	var textRead = fs.read();
	fs.close();
	Titanium.AppTest.addResult('filesystem.filestream.write',textToWrite==textRead,"text written is not the same as text read");
}
catch (e) {
	Titanium.AppTest.addResult('filesystem.filestream.write',false,"failed with exception: "+e);
}

/** write/append/read **/
try {
	var filename = "writeAppendTestFile.txt";
	var textToWrite = "This is the text to write in the file.";
	var textToAppend = "This is the text to append.";
	
	var fs = Titanium.Filesystem.getFileStream(base, filename);
	fs.open(Titanium.Filesystem.MODE_WRITE);
	fs.write(textToWrite);
	fs.close();
	
	fs = Titanium.Filesystem.getFileStream(base, filename);
	fs.open(Titanium.Filesystem.MODE_APPEND);
	fs.write(textToAppend);
	fs.close();
	
	var f = Titanium.Filesystem.getFile(base, filename);
	Titanium.AppTest.addResult('filesystem.filestream.writeappend',f!=null,"f should not be null");
	Titanium.AppTest.addResult('filesystem.filestream.writeappend',f.exists(),"writeAppendTestFile.txt should exist");
	
	// read back the file contents
	fs.open(Titanium.Filesystem.MODE_READ);
	var textRead = fs.read();
	fs.close();
	Titanium.AppTest.addResult('filesystem.filestream.writeappend',(textToWrite+textToAppend)==textRead,"existing + appended text is not the same as the text read");
}
catch (e) {
	Titanium.AppTest.addResult('filesystem.filestream.writeappend',false,"failed with exception: "+e);
}