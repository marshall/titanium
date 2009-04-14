// ==UserScript==
// @name	Titanium App Tester
// @author	Appcelerator
// @description	Test Processor for Titanium UI Tests
// @include	app://*.html
// @version 	0.1
// ==/UserScript==


Titanium.AppTest = {};
Titanium.AppTest.results = [];
Titanium.AppTest.screenshots = [];
Titanium.AppTest.resultFile = Titanium.Filesystem.getFile(Titanium.Filesystem.getUserDirectory(),'titanium.testresults');

//
// Exit app after timeout
//
Titanium.AppTest.timeoutFunction = setTimeout(function()
{
	Titanium.App.exit();

},2000)

//
// Add a test result
// @name - name of your test
// @result - (true|false)
// @detail - can contain error info or success info
Titanium.AppTest.addResult = function(name, result, detail)
{
	if (result && typeof(detail)=='undefined') detail = 'passed';
	Titanium.AppTest.results.push({name:name,result:result,detail:detail});
	Titanium.AppTest.writeResults();
};

//
// take a screenshot
// @name - name of your test
// @callback - screenshot is asnychronous so this is the callback for when its done
//
Titanium.AppTest.takeScreenshot = function(name, callback)
{
	var shotPath = Titanium.Filesystem.getUserDirectory();
	shotPath += Titanium.Filesystem.getSeparator();
	shotPath += name + ".png";
	try
	{
		// delay screenshot to ensure test changes have taken place
		setTimeout(function()
		{
			Titanium.Desktop.takeScreenshot(shotPath);
			Titanium.AppTest.screenshots.push({name:name,result:'pending',path:shotPath});
			Titanium.AppTest.writeResults();

			// need to delay execution so screenshot can capture UI state (screenshot is async)
			setTimeout(function()
			{
				if (callback) callback();

			},100);
			
		},100);
	}
	catch (e)
	{
		Titanium.AppTest.addResult(name,'screenshot','failed','screenshot failed with exception ' + e.message);
	}
	
};

//
// set the timeout for your test
// @timeout - timeout value in milliseconds
Titanium.AppTest.setTimeout = function(timeout)
{
	clearTimeout(Titanium.AppTest.timeoutFunction);
	Titanium.AppTest.timeoutFunction = setTimeout(function()
	{
		Titanium.App.exit();
	},timeout)
};

//
// API to programatically stop test
//
Titanium.AppTest.stop = function()
{
	clearTimeout(Titanium.AppTest.timeoutFunction);
	Titanium.App.exit();
};

//
// Helper function that writes out results to a file
//
Titanium.AppTest.writeResults = function()
{
	var str = '['
	for (var i=0;i<Titanium.AppTest.results.length;i++)
	{
		str += '{';
		str += '"name":"' + Titanium.AppTest.results[i].name + '",';
		str += '"result":"' + Titanium.AppTest.results[i].result + '",';
		str += '"detail":"' + Titanium.AppTest.results[i].detail;
		str += '"},';

	}
	for (var i=0;i<Titanium.AppTest.screenshots.length;i++)
	{

		str += '{';
		str += '"name":"' + Titanium.AppTest.screenshots[i].name + '",';
		str += '"result":"' + Titanium.AppTest.screenshots[i].result + '",';
		str += '"path":"' + Titanium.AppTest.screenshots[i].path;
		str += '"},';
		
	}
	str = str.substring(0,(str.length -1)) + ']';
	Titanium.AppTest.resultFile.write(str)
	
};

// LOAD TESTS
var testDir = Titanium.Filesystem.getFile(Titanium.App.appURLToPath('app://tests'));

// RUN JS-based TEST
if (testDir.exists())
{
	var testFiles = testDir.getDirectoryListing();
	var testDir = Titanium.Filesystem.getFile(Titanium.App.appURLToPath('app://tests'));
	var testFiles = testDir.getDirectoryListing();
	var f = Titanium.Filesystem.getFile(testFiles[0]);
	var contents = f.read();
	eval('((function(){'+contents+'})(window))');
}
// RUN HTML-based TEST
else
{
	startTest();
}
