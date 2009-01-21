testSuite("Titanium.Filesystem API tests", "dummy.html", {
	var stageDir = "c:\temp\";
	var fileNormal = stageDir + "file-normal.txt";
	var fileDir = stageDir + "normalDir";
	var fileHidden = stageDir + "file-hidden.txt";
	var fileLink = stageDir + "file-link.txt";
	var fileMissing = stageDir + "file-doesnt-exist.txt";
	var fileCopyName = stageDir + "file-normal-copy.txt";
	var fileMoveName = stageDir + "file-normal-moved.txt";
	var dirCreateName = stageDir + "dir-created";
	var dirCreatedWithContentsName = stageDir + "dir-created-with-contents";
	var subDirCreatedName = dirCreateWithConentsName + "\"" + "sub-dir-created";
	var fileWriteName = stageDir + "file-written.txt";
	
	run: function () {
		test("top level API", function() {
			assert(Titanium != null);
			assert(Titanium.Filesystem != null);
			assert(Titanium.Filesystem.getFile != null);
			assert(Titanium.Filesystem.getApplicationDirectory != null);
			assert(Titanium.Filesystem.getResourcesDirectory != null);
			assert(Titanium.Filesystem.getDesktopDirectory != null);
			assert(Titanium.Filesystem.getDocumentsDirectory != null);
			assert(Titanium.Filesystem.getUserDirectory != null);
			assert(Titanium.Filesystem.getRootDirectories != null);
			assert(Titanium.Filesystem.createTempDirectory != null);
			assert(Titanium.Filesystem.createTempFile != null);

			assert(Titanium.Filesystem.getApplicationDirectory() != null);
			assert(Titanium.Filesystem.getResourcesDirectory() != null);
			assert(Titanium.Filesystem.getDesktopDirectory() != null);
			assert(Titanium.Filesystem.getDocumentsDirectory() != null);
			assert(Titanium.Filesystem.getUserDirectory() != null);
			assert(Titanium.Filesystem.getRootDirectories() != null);
			assert(Titanium.Filesystem.createTempDirectory() != null);
			assert(Titanium.Filesystem.createTempFile() != null);	
		});

		test("file object API - file types", function() {
			assert(Titanium != null);
			assert(Titanium.Filesystem != null);

			var file = null;
			
			file = Titanium.Filesystem.getFile(fileNormal);
			assert(file != null);
			assert(file.isFile != null);
			assert(file.isDirectory != null);
			assert(file.isHidden != null);
			assert(file.isSymbolicLink != null);
			
			assert(file.exists() == true);
			assert(file.isFile() == true);
			assert(file.isDirectory() == false);
			assert(file.isHidden() == false);
			assert(file.isSymbolicLink == false);
			
			file = Titanium.Filesystem.getFile(fileDir);
			assert(file.exists() == true);
			assert(file.isFile() == false);
			assert(file.isDirectory() == true);
			assert(file.isHidden() == false);
			assert(file.isSymbolicLink == false);
			
			file = Titanium.Filesystem.getFile(fileHidden);
			assert(file.exists() == true);
			assert(file.isFile() == true);
			assert(file.isDirectory() == false);
			assert(file.isHidden() == true);
			assert(file.isSymbolicLink == false);
			
			file = Titanium.Filesystem.getFile(fileLink);
			assert(file.exists() == true);
			assert(file.isFile() == true);
			assert(file.isDirectory() == false);
			assert(file.isHidden() == true);
			assert(file.isSymbolicLink == false);
			
			file = Titanium.Filesystem.getFile(fileMissing);
			assert(file.exists() == false);
			assert(file.isFile() == false);
			assert(file.isDirectory() == false);
			assert(file.isHidden() == false);
			assert(file.isSymbolicLink == false);
		});
		
		test("file object API - file props", function() {
			assert(Titanium != null);
			assert(Titanium.Filesystem != null);

			var file = null;
			
			file = Titanium.Filesystem.getFile(fileNormal);
			assert(file != null);
			assert(file.createTimestamp != null);
			assert(file.modificationTimestamp != null);
			assert(file.name != null);
			assert(file.extension != null);
			assert(file.nativePath != null);
			assert(file.size != null);
			
			assert(file.createTimestamp() != null);
			assert(file.modificationTimestamp() != null);
			assert(file.name() != null);
			assert(file.extension() != null);
			assert(file.nativePath() != null);
			assert(file.size() != null);
			assert(file.toString() != null);
		});
		
		test("file object API - read file", function() {
			assert(Titanium != null);
			assert(Titanium.Filesystem != null);

			var file = null;
			
			file = Titanium.Filesystem.getFile(fileNormal);
			assert(file != null);
			
			assert(file.read() != null);
		});
		
		test("file object API - copy / delete file", function() {
			assert(Titanium != null);
			assert(Titanium.Filesystem != null);

			var file = null;
			
			file = Titanium.Filesystem.getFile(fileNormal);
			assert(file != null);
			assert(file.copy != null);
			
			var copied = file.copy(fileCopyName);
			assert(copied == true);
			
			file = Titanium.Filesystem.getFile(fileCopyName);
			assert(file.exists() == true);
			
			var deleted = file.deleteFile();
			assert(deleted == true);
		});
		
		test("file object API - move file", function() {
			assert(Titanium != null);
			assert(Titanium.Filesystem != null);

			var file = null;
			
			file = Titanium.Filesystem.getFile(fileNormal);
			assert(file != null);
			assert(file.move != null);
			
			var moved = file.move(fileMoveName);
			assert(moved == true);
			
			file = Titanium.Filesystem.getFile(fileMoveName);
			assert(file.exists() == true);
			
			moved = file.move(fileNormal);
			assert(moved == true);
		});
		
		test("file object API - create / delete dir", function() {
			assert(Titanium != null);
			assert(Titanium.Filesystem != null);

			var file = null;
			
			file = Titanium.Filesystem.getFile(dirCreateName);
			assert(file != null);
			assert(file.createDirectory != null);
			
			var created = file.createDirectory();
			assert(created == true);
			
			var deleted = file.deleteDirectory();
			assert(deleted == true);
		});
		
		test("file object API - create dir / should not delete dir with contents", function() {
			assert(Titanium != null);
			assert(Titanium.Filesystem != null);

			var file = null;
			
			file = Titanium.Filesystem.getFile(dirCreatedWithContentsName);
			assert(file != null);
			assert(file.createDirectory != null);
			
			var created = file.createDirectory();
			assert(created == true);
			
			created = file.createDirectory(subDirCreatedName);
			assert(created == true);
			
			// should fail because we don't specify 'deleteContents' param
			file = Titanium.Filesystem.getFile(dirCreatedWithContentsName);
			var deleted = file.deleteDirectory();
			assert(deleted == false);
			
			// should fail because we're specifying to not delete contents
			file = Titanium.Filesystem.getFile(dirCreatedWithContentsName);
			deleted = file.deleteDirectory(false);
			assert(deleted == false);
			
			// should succeed because we specify to delete contents
			file = Titanium.Filesystem.getFile(dirCreatedWithContentsName);
			deleted = file.deleteDirectory(true);
			assert(deleted == true);
		});
		
		test("file object API - dir listing", function() {
			assert(Titanium != null);
			assert(Titanium.Filesystem != null);

			var file = null;
			
			file = Titanium.Filesystem.getFile(stageDir);
			assert(file != null);
			assert(file.getDirectoryListing != null);
			
			var files = file.getDirectoryListing();
			assert(files != null);
		});
		
		test("file object API - file write", function() {
			assert(Titanium != null);
			assert(Titanium.Filesystem != null);

			var file = null;
			
			file = Titanium.Filesystem.getFile(fileWriteName);
			assert(file != null);
			assert(file.write != null);
			
			var textToWrite = "this is the text to write."
			var success = file.write(textToWrite);
			assert(success == true);
			
			var textRead = file.read();
			assert(textRead == textToWrite);
		});
	}
});

