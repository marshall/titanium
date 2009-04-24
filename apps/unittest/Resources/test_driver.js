//
//	Titanium Test Driver
//
//  This is the main driver JS file for running automated unit and application level tests.
//

var TFS = Titanium.Filesystem;
		
// global TestDriver object		
Titanium.TestDriver = {};

// array of test files
Titanium.TestDriver.testFiles = [];

// array of tiapp XML files
Titanium.TestDriver.xmlFiles = [];

// array of test names by test file
Titanium.TestDriver.testsByFile = {};

// target project root directory
Titanium.TestDriver.rootDir = null;

// project object (if in app mode)
Titanium.TestDriver.project = null;

// current test file counter
Titanium.TestDriver.currentTestFile = -1;

// test modes
Titanium.TestDriver.UNITMODE = 'unit';
Titanium.TestDriver.APPMODE = 'app';
Titanium.TestDriver.mode = null;

// current temp dir for launched project
Titanium.TestDriver.currentTempDir = null;

// where results go directory
Titanium.TestDriver.resultsDir = null;

// unique test names (only can be used once)
Titanium.TestDriver.uniqueTestNames = {};

//
//  This function identifies all test files and parses all javascript to 
//  create a list of all available tests to be executed
//
Titanium.TestDriver.init = function(project)
{
	if (!project)
	{
		alert('loadTests: requires either a directory or a project object');
		return;
	}
	if (typeof(project)=='object')
	{
		Titanium.TestDriver.mode = Titanium.TestDriver.APPMODE; 
		Titanium.TestDriver.rootDir = project.dir;
		Titanium.TestDriver.project = project;
	}
	else
	{
		Titanium.TestDriver.mode = Titanium.TestDriver.UNITMODE; 
		Titanium.TestDriver.rootDir = project;
	}
	
	Titanium.TestDriver.setupFramework();
	
	// create screenshot directories (this is where all screenshots are stored before publishing results)
	Titanium.TestDriver.resultsDir = TFS.getFile(Titanium.App.appURLToPath('app://results'));
	if (Titanium.TestDriver.resultsDir.exists() == false)
	{
		Titanium.TestDriver.resultsDir.createDirectory(true);
	}
	
	// check to see if we want to only run a specific
	// set of tests which are by name only - no extension
	var specificTests = null;
	for (var c=0;c<Titanium.App.arguments.length;c++)
	{
		var arg = Titanium.App.arguments[c];
		if (arg.indexOf('--tests=')==0)
		{
			specificTests = arg.substring(8).split(',');
		}
	}
	
	// load test files
	var testDir = TFS.getFile(Titanium.TestDriver.rootDir,'tests');
	var tests = testDir.getDirectoryListing();
	
	for (var i=0;i<tests.length;i++)
	{
		var testFile = TFS.getFile(tests[i]);
		if (specificTests)
		{
			var testNamePart = testFile.name().substring(0,testFile.name().length-testFile.extension().length-1);
			var idx = specificTests.indexOf(testNamePart);
			if (idx == -1)
			{
				continue;
			}
		}

	    // look for JS and HTML test files
		if (testFile.extension() == 'html' || testFile.extension() == 'js')
		{
			Titanium.TestDriver.testFiles.push({name:testFile.name(),file:testFile});
			
			// parse JS to get superset list of all tests
			parseJS(testFile);
		}
		// look for tiapp.xml files (naming convention is 'test_file_name'.xml)
		else if (testFile.extension() == 'xml')
		{
			Titanium.TestDriver.xmlFiles.push({name:testFile.name(),file:testFile});
		}	
	}

	// start test exection
	Titanium.TestDriver.loadNextTest();
	
	//
	// private inner function for parsing tests from each file
	//
	function parseJS(file)
	{
		try
		{
			// open file and read
			var f = TFS.getFile(file);
			var contents = f.read();

			// find patterns
			var resultRegEx =  /Titanium\.AppTest\.addResult\('([a-zA-Z0-9\.\-_!]+)'/g;
			var screenshotRegEx =  /Titanium\.AppTest\.takeScreenshot\('([a-zA-Z0-9\.\-_!]+)'/g;
			var results = contents.match(resultRegEx);
			var shots = contents.match(screenshotRegEx);
	
			// initialize 
			Titanium.TestDriver.testsByFile[file.name()] = [];
			
			if (results != null)
			{
				for (var i=0;i<results.length;i++)
				{
					var name = results[i].substring(28,(results[i].length-1));
					// only add test names once
					if (!Titanium.TestDriver.uniqueTestNames[name])
					{
						Titanium.TestDriver.testsByFile[file.name()].push({test:results[i] + ')', name:name,result:'not_executed',file:file.name()});
						Titanium.TestDriver.uniqueTestNames[name]=true;
					}
				}
			}

			if (shots != null)
			{
				for (var i=0;i<shots.length;i++)
				{
					var name = shots[i].substring(33,(shots[i].length-1));
					// only add test names once
					if (!Titanium.TestDriver.uniqueTestNames[name])
					{
						Titanium.TestDriver.testsByFile[file.name()].push({test:shots[i] + ')',name:name,result:'not_executed',file:file.name()});
						Titanium.TestDriver.uniqueTestNames[name] = true;
					}
				}
			}
			
			return;
		}
		catch (e)
		{
			Titanium.API.error('Exception caught parsing test files, message: ' + e.message);
			return;
		}
	}
};

Titanium.TestDriver.setupFramework = function()
{
};

//
// helper function to find xml file for current test.
//
function getXML(name)
{
	for (var i=0;i<Titanium.TestDriver.xmlFiles.length;i++)
	{
		var xml = Titanium.TestDriver.xmlFiles[i].name
		if (xml.substring(0,(xml.length -4)) == name)
		{
			return Titanium.TestDriver.xmlFiles[i];
		}
	}
	return null;
}

//
// Helper function to Launch temporary app for running tests
//
Titanium.TestDriver.launchApp = function(testfile)
{
	/*** THIS WAS IN setupFramework **/
	
	// delete tempdir if exists
	if (Titanium.TestDriver.currentTempDir != null && Titanium.TestDriver.currentTempDir.exists()==true)
	{
		Titanium.TestDriver.currentTempDir.deleteDirectory(true);
		Titanium.TestDriver.currentTempDir = null;
	}
	
	var project = {};
	
	// if MODE is Unit
	if (Titanium.TestDriver.mode == Titanium.TestDriver.UNITMODE)
	{
		//optimize this
		var appdir = TFS.getFile(Titanium.App.appURLToPath('app://_app'));
		if (!appdir.exists())
		{
			appdir.createDirectory(true);
		}
		
		project.name = "unittest";
		project.url = "http://titaniumapp.com";
		project.rootdir = String(appdir);
		project.dir = project.rootdir +'/'+ project.name;
		project.appid = 'com.titaniumapp.unittest';
		project.publisher = "Appcelerator";
		project.desc = 'automated unit test app';
		project.guid = Titanium.Platform.createUUID();
		
		var outdir = TFS.getFile(project.rootdir,project.name);
		// record this temp dir for deletion next time
		Titanium.TestDriver.currentTempDir = outdir;
		
		if (!outdir.exists())
		{
			var htmlContents = 'test executing...';
			if (testfile.file.extension() == 'html')
			{
				htmlContents = testfile.file.read();			
			}

			// create project
			Titanium.Project.create(project.name,project.guid,project.desc,project.rootdir,project.publisher,project.url,null,null,htmlContents);
		}
	}
	// otherwise we are in APP mode
	else
	{
		//FIXME: not sure how to deal with this on the refactor
		//project = Titanium.TestDriver.project;
	}

	// create userscripts dir
	Titanium.TestDriver.resources = TFS.getFile(project.dir,'Resources');
	Titanium.TestDriver.userscripts = TFS.getFile(Titanium.TestDriver.resources,'userscripts');
	Titanium.TestDriver.userscripts.createDirectory(true);

	// copy in test processor
	var processor = TFS.getFile(Titanium.App.appURLToPath('app://test_processor.js'));
	processor.copy(Titanium.TestDriver.userscripts);
	
	Titanium.TestDriver.project = project;
	
	// write out manifest
	var mf = TFS.getFile(TFS.getApplicationDirectory(),'manifest');
	if (!mf.exists())
	{
		mf.copy(Titanium.TestDriver.project.dir);
	}

	var rd = TFS.getFile(TFS.getApplicationDirectory(),'runtime');
	if (rd.exists())
	{
		rd.copy(Titanium.TestDriver.project.dir);
	}
	
	var md = TFS.getFile(TFS.getApplicationDirectory(),'modules');
	if (md.exists())
	{
		md.copy(Titanium.TestDriver.project.dir);
	}

	/*** end of setupFramework **/

	var xmlFile = null;
	var testName = null;
	
	// copy in test file
	if (testfile.file.extension() == 'html')
	{
		// look for a tiapp.xml file with same name as test file
		testName = testfile.name.substring(0,testfile.name.length -5);
		xmlFile = getXML(testName);

		var images = TFS.getFile(Titanium.App.appURLToPath('app://tests/resources'));
		if (images.exists()==true)
		{
			images.copy(TFS.getFile(Titanium.TestDriver.project.dir,'Resources','resources'));
		}
	}
	else
	{
		// look for tiapp.xml file with same name as test file
		testName = testfile.name.substring(0,testfile.name.length -3);
		xmlFile = getXML(testName);
		
		// copy test file into tests directory
		var testdir = TFS.getFile(Titanium.TestDriver.resources,'tests');
		testdir.createDirectory(true)
		testfile.file.copy(testdir);
	}

	// copy test tiapp.xml if exists
	if (xmlFile != null)
	{
		var dir = TFS.getFile(Titanium.TestDriver.project.dir);
		xmlFile.file.copy(dir);
		var newXML = TFS.getFile(dir,xmlFile.name)
		newXML.rename('tiapp.xml');
	}

	var profile_path = TFS.getFile(Titanium.TestDriver.resultsDir,testName+'.prof');
	var log_path = TFS.getFile(Titanium.TestDriver.resultsDir,testName+'.log');

	profile_path.deleteFile();
	log_path.deleteFile();

	var args = ['--profile="'+profile_path+'"','--logpath="'+log_path+'"'];
	
	// launch app
	var pid = Titanium.Project.launch(Titanium.TestDriver.project,false,function(p)
	{
		// on exit, record results and move to the next
		p.onexit = function(exitcode)
		{
			// read results 
			var resultsFile = TFS.getFile(TFS.getUserDirectory(),'titanium.testresults')
			if (resultsFile.exists() ==true)
			{
				var results =  eval('('+resultsFile.read() +')');

				// remove file
				resultsFile.deleteFile();
				
				// read in the log
				var log = TFS.getFile(log_path).read();

				// update results
				var superset = Titanium.TestDriver.testsByFile[testfile.name];
				for (var i=0;i<results.length;i++)
				{
					var r = results[i];
					for (var j=0;j<superset.length;j++)
					{
						if (superset[j].name == r.name)
						{
							superset[j].result = r.result;
							superset[j].detail = r.detail;
							superset[j].log = log;

							// screenshot, copy and delete
							if (r.path)
							{
								var f = TFS.getFile(r.path);		
								var dest = TFS.getFile(Titanium.TestDriver.resultsDir);
								f.copy(dest);
								f.deleteFile(true)
							}
							break;
						}
					}
				}
				
			}
			
			// start next test
			Titanium.TestDriver.loadNextTest();
		};
	},args);
};

//
// Run next test
//
Titanium.TestDriver.loadNextTest = function()
{
	Titanium.TestDriver.currentTestFile++;
	if (Titanium.TestDriver.currentTestFile == Titanium.TestDriver.testFiles.length)
	{
		// we are done
		Titanium.TestDriver.summarizeResults();
		return;
	}

	// send status_update
	var pctComplete = (Titanium.TestDriver.currentTestFile+1)/Titanium.TestDriver.testFiles.length;
	$MQ('l:titanium_test_pctcomplete',{current:(Titanium.TestDriver.currentTestFile+1),total:Titanium.TestDriver.testFiles.length,pct_complete:parseInt(pctComplete *100)});;
	
	//  launch next test
	Titanium.TestDriver.launchApp(Titanium.TestDriver.testFiles[Titanium.TestDriver.currentTestFile]);
	
};

//
// Create test results object
//
Titanium.TestDriver.summarizeResults = function()
{
	var totalTests = 0;
	var totalPassed = 0;
	var totalFailed = 0;
	var totalPending = 0;
	var totalNotExecuted = 0;
	var resultArray = [];
	for (var i=0;i<Titanium.TestDriver.testFiles.length;i++)
	{
		var results = Titanium.TestDriver.testsByFile[Titanium.TestDriver.testFiles[i].name];
		for (var j=0;j<results.length;j++)
		{
			totalTests++;
			if (results[j].result == 'true')
			{
				results[j].result = 'passed'
				totalPassed++;
			}
			else if (results[j].result == 'false')
			{
				results[j].result = 'failed'
				totalFailed++;
				
			}
			else if (results[j].result == 'pending')
			{
				totalPending++;
			}
			else
			{
				totalNotExecuted++;
			}
			resultArray.push(results[j]);
		}
	}
	$MQ('l:titanium_test_results',{rows:resultArray,'test_count':totalTests,passed:totalPassed,failed:totalFailed,pending:totalPending,'not_executed':totalNotExecuted})
};
