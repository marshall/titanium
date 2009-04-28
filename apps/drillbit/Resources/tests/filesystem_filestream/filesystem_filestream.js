describe("Ti.Filesystem FileStream tests",{
	before_all:function()
	{
		// clean up testing folder if needed
		var base = Titanium.Filesystem.getFile(Titanium.Filesystem.getApplicationDataDirectory(), "unittest_filesystem_filestream");
		if(base.exists() && base.isDirectory()) {
			base.deleteDirectory(true);
		}
		else if(base.exists() && base.isFile()) {
			base.deleteFile();
		}
	
		base.createDirectory();
		
		this.base = base;
	},
	
	other_props:function()
	{
		value_of(Titanium.Filesystem.getLineEnding).should_be_function();
		value_of(Titanium.Filesystem.getSeparator).should_be_function();
		value_of(Titanium.Filesystem.FILESTREAM_MODE_READ).should_not_be_null();
		value_of(Titanium.Filesystem.FILESTREAM_MODE_WRITE).should_not_be_null();
		value_of(Titanium.Filesystem.FILESTREAM_MODE_APPEND).should_not_be_null();
				
		value_of(Titanium.Filesystem.getLineEnding()).should_not_be_null();
		value_of(Titanium.Filesystem.getSeparator()).should_not_be_null();
	},
	
	write_read:function()
	{
		var textToWrite = "This is the text to write in the file";
		var fs = Titanium.Filesystem.getFileStream(this.base, "writeTestFile.txt");
		fs.open(Titanium.Filesystem.FILESTREAM_MODE_WRITE);
		fs.write(textToWrite);
		fs.close();
		
		var f = Titanium.Filesystem.getFile(this.base, "writeTestFile.txt");
		value_of(f).should_not_be_null();
		value_of(f.exists()).should_be_true();
		
		// read back the file contents
		fs.open(Titanium.Filesystem.FILESTREAM_MODE_READ);
		var textRead = fs.read();
		fs.close();
		value_of(textToWrite).should_be(textRead);
	},
	
	write_append_read:function()
	{
		var filename = "writeAppendTestFile.txt";
		var textToWrite = "This is the text to write in the file.";
		var textToAppend = "This is the text to append.";
		
		var fs = Titanium.Filesystem.getFileStream(this.base, filename);
		fs.open(Titanium.Filesystem.FILESTREAM_MODE_WRITE);
		fs.write(textToWrite);
		fs.close();
		
		fs = Titanium.Filesystem.getFileStream(this.base, filename);
		fs.open(Titanium.Filesystem.FILESTREAM_MODE_APPEND);
		fs.write(textToAppend);
		fs.close();
		
		var f = Titanium.Filesystem.getFile(this.base, filename);
		value_of(f).should_not_be_null();
		value_of(f.exists()).should_be_true();
		
		// read back the file contents
		fs.open(Titanium.Filesystem.FILESTREAM_MODE_READ);
		var textRead = fs.read();
		fs.close();
		value_of(textToWrite+textToAppend).should_be(textRead);	
	}

});


