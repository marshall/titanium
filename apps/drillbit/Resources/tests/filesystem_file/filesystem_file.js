describe("Ti.Filesystem File tests",{
	before_all:function()
	{
		// clean up testing folder if needed
		var base = Titanium.Filesystem.getFile(Titanium.Filesystem.getApplicationDataDirectory(), "unittest_filesystem_file");
		if(base.exists() && base.isDirectory()) {
			base.deleteDirectory(true);
		}
		else if(base.exists() && base.isFile()) {
			base.deleteFile();
		}
	
		base.createDirectory();
		
		this.base = base;
	},
	
	file_props:function()
	{
		var f = Titanium.Filesystem.getFile(this.base, "newFile.txt");
		value_of(f).should_not_be_null();
		value_of(f.exists()).should_be_false();
		
		// create new file to test props with
		createFile(this.base,"filePropsTest.txt", "This is the text for the text file.");
		
		f = Titanium.Filesystem.getFile(this.base, "filePropsTest.txt");
		value_of(f).should_not_be_null();
		value_of(f.exists()).should_be_true();
		value_of(f.toString()).should_not_be_null();
		value_of(f.name()).should_not_be_null();
		value_of(f.extension()).should_be('txt');
		value_of(f.isFile()).should_be_true();
		value_of(f.isDirectory()).should_be_false();
		value_of(f.isHidden()).should_be_false();
		value_of(f.isSymbolicLink()).should_be_false();
		value_of(f.isExecutable()).should_be_false();
		value_of(f.isReadonly()).should_be_false();
		value_of(f.isWriteable()).should_be_true();
		value_of(f.createTimestamp()).should_not_be_null();
		value_of(f.modificationTimestamp()).should_not_be_null();
		value_of(f.nativePath()).should_not_be_null();
		value_of(f.size()).should_not_be_null();	
	},
	
	directory_props:function()
	{
		// directory props
		var f = Titanium.Filesystem.getFile(this.base, "dirPropsTest");
		value_of(f).should_not_be_null();
		value_of(f.exists()).should_be_false();
		f.createDirectory();
		value_of(f.exists()).should_be_true();
		value_of(f.isHidden()).should_be_false();
		value_of(f.isSymbolicLink()).should_be_false();
		value_of(f.isExecutable()).should_be_false();
		value_of(f.isReadonly()).should_be_false();
		value_of(f.isWriteable()).should_be_true();
		value_of(f.createTimestamp()).should_not_be_null();
		value_of(f.modificationTimestamp()).should_not_be_null();
		value_of(f.nativePath()).should_not_be_null();
		value_of(f.size()).should_not_be_null();
		value_of(f.spaceAvailable()).should_not_be_null();
	},
	
	resolve:function()
	{
		var f = f = Titanium.Filesystem.getFile(this.base, "fileTest-Resolve");
		value_of(f).should_not_be_null();
		value_of(f.resolve("filename.txt")).should_not_be_null();
	},
	
	file_operations:function()
	{
		var f = Titanium.Filesystem.getFile(this.base, "fileToCopy.txt");
		value_of(f).should_not_be_null();
		value_of(f.exists()).should_be_false();
		
		// create new file to test props with
		createFile(this.base,"fileToCopy.txt","This ist he text for the test file.");
		
		var copiedF = Titanium.Filesystem.getFile(this.base, "copiedFile.txt");
		var r = f.copy(copiedF);
		value_of(r).should_be_true();
		value_of(copiedF.exists()).should_be_true();
		
		var movedF = Titanium.Filesystem.getFile(this.base, "movedFile.txt");
		r = copiedF.move(movedF);
		value_of(r).should_be_true();
		value_of(movedF.exists()).should_be_true();
		
		var renamedF = Titanium.Filesystem.getFile(this.base, "renamedFile.txt");
		r = movedF.rename("renamedFile.txt");
		value_of(r).should_be_true();
		value_of(renamedF.exists()).should_be_true();
		
		r = renamedF.deleteFile();
		value_of(r).should_be_true();
	},
	
	directory_operations:function()
	{
		var d = Titanium.Filesystem.getFile(this.base, "playDirectory");
		value_of(d).should_not_be_null();
		value_of(d.exists()).should_be_false();
		
		// create new directory
		var r = d.createDirectory();
		value_of(r).should_be_true();
		value_of(d.exists()).should_be_true();
		
		r = d.deleteDirectory();
		value_of(r).should_be_true();
		value_of(d.exists()).should_be_false();
	},
	
	directory_listing:function()
	{
		var d = Titanium.Filesystem.getFile(this.base, "directoryListingTest");
		value_of(d).should_not_be_null();
		value_of(d.exists()).should_be_false();
		
		createDirTree(this.base,"directoryListingTest");
		value_of(d.exists()).should_be_true();

		var listings = d.getDirectoryListing();
		value_of(listings).should_not_be_null();
		value_of(listings.length).should_be(3);
		
		var subDir1 = Titanium.Filesystem.getFile(d, "subDir1");
		value_of(subDir1.isDirectory()).should_be_true();
		
		var subDirListings = subDir1.getDirectoryListing();
		value_of(subDirListings).should_not_be_null();
		value_of(subDirListings.length).shoud_be(1);
	},
	
	parent:function()
	{
		var f = Titanium.Filesystem.getFile(this.base, "parentTestFile.txt");
		value_of(f).should_not_be_null();
		value_of(f.exists()).should_be_false();
		
		var parent = f.parent();
		value_of(parent.toString()).shoud_be(this.base.toString());
	},
	
	shortcut:function()
	{
		createFile(this.base,"shotcutTestFile.txt","text for test file");
		var f = Titanium.Filesystem.getFile(this.base, "shortcutTestFile.txt");
		value_of(f).should_not_be_null();
		value_of(f.exists()).should_be_true();
		
		var shortcutFile = Titanium.Filesystem.getFile(this.base, "my-shortcut");
		var r = f.createShortcut(shortcutFile);
		value_of(r).should_be_true();
		value_of(shortcutFile.exists()).should_be_true();
	},
	
	file_permissions:function()
	{
		createFile(this.base,"permissionsTestFile.txt","text for test file");
		var f = Titanium.Filesystem.getFile(this.base, "permissionsTestFile.txt");
		value_of(f).should_not_be_null();
		value_of(f.exists()).should_be_true();
		value_of(f.isExecutable()).should_be_false();
		value_of(f.isReadonly()).should_be_false();
		value_of(f.isWriteable()).should_be_true();
		
		f.setExecutable(true);
		value_of(f.isExecutable()).should_be_true();
		
		f.setReadonly(false);
		value_of(f.isReadonly()).should_be_false();
		
		f.setWriteable(false);
		value_of(f.isWriteable()).should_be_false();
	}
});

function createFile(base,name,text) {
	var fs = Titanium.Filesystem.getFileStream(base, name);
	fs.open(Titanium.Filesystem.FILESTREAM_MODE_WRITE);
	fs.write(text);
	fs.close();
}

function createDirTree(base,name) {
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

