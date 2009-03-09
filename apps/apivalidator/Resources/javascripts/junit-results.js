/*!(c) 2006-2009 Appcelerator, Inc. http://appcelerator.org
 * Licensed under the Apache License, Version 2.0. Please visit
 * http://license.appcelerator.com for full copy of the License.
 **/

// Generate a JUnit XML results file from the tests that have run
$(document).ready(function() {
	Titanium.API.debug("install junit results plugin..");
	var resultsPath = null;
	if (Titanium.App.arguments.length > 1) {
		resultsPath = Titanium.App.arguments[1];
	}
	var testSuites = new Array();
	var currentSuite = null, currentTest = null;
	var id = 0;
	TestMonkey.installTestRunnerPlugin({
		onAddTestSuite: function (suiteName)
		{
			currentSuite = {
				name: suiteName,
				package: Titanium.App.getID(),
				errorCount: 0,
				failureCount: 0,
				testCount: 0,
				tests: [],
				time: 0,
				timestamp: new Date(),
				id: id
			};
			
			testSuites.push(currentSuite);
			id++;
		},
		
		onBeforeTestCase: function(test)
		{
			currentTest = {
				name: test.name,
				time: new Date().getTime(),
				failures: [],
				errors: []
			};
			
			currentSuite.tests.push(currentTest);
		},
		
		onAfterTestCase: function(test)
		{
			var errorMessage = test.testcase;
			var errored = false, failed = false;
			currentSuite.testCount++;
			$.each(test.results, function() {
				if (this.error) {
					currentSuite.errorCount++;
					currentTest.errors.push({
						assert: this.assert,
						error: this.error
					});
				}
				else if (!this.result && this.message) {
					currentSuite.failureCount++;
					currentTest.failures.push({
						assert: this.assert,
						error: this.message
					});
				}
			});
			
			currentTest.time = new Date().getTime() - currentTest.time;
		},
		
		onAfterTestSuite: function()
		{
			currentSuite.time =
				new Date().getTime() - currentSuite.timestamp.getTime();
		},
		
		onAfterTestRunner: function()
		{
			this.serializeResults();
		},
		
		serializeResults: function()
		{
			var self = this;
			var doc = document.implementation.createDocument("", "testsuites", null);
			$.each(testSuites, function() {
				var suite = this;
				var suiteEl = doc.createElement("testsuite");
				suiteEl.setAttribute("errors", suite.errorCount);
				suiteEl.setAttribute("failures", suite.failureCount);
				suiteEl.setAttribute("id", suite.id);
				suiteEl.setAttribute("name", suite.name);
				suiteEl.setAttribute("package", suite.package);
				suiteEl.setAttribute("tests", suite.testCount);
				suiteEl.setAttribute("time", suite.time / 1000.0);
				suiteEl.setAttribute("timestamp", suite.timestamp.toUTCString());
				doc.documentElement.appendChild(suiteEl);
				$.each(this.tests, function() {
					self.serializeTest(suite, this, suiteEl);
				});
			});
			
			var xml = (new XMLSerializer()).serializeToString(doc);
			if (resultsPath == null) {
				resultsPath = Titanium.App.appURLToPath("app://results.xml");
			}
			Titanium.API.debug("resultsPath="+resultsPath);
			var fileStream = Titanium.Filesystem.getFileStream(resultsPath);
			fileStream.open("write");
			fileStream.write(xml);
			fileStream.close();
		},
		
		serializeTest: function(suite, test, suiteEl)
		{
			var self = this;
			var testCaseEl = suiteEl.ownerDocument.createElement("testcase");
			testCaseEl.setAttribute("classname", suite.package);
			testCaseEl.setAttribute("name", test.name);
			testCaseEl.setAttribute("time", test.time/ 1000.0);
			suiteEl.appendChild(testCaseEl);
			
			$.each(test.failures, function() {
				self.serializeFailure(test, this, testCaseEl);
			});
			
			$.each(test.errors, function() {
				self.serializeError(test, this, testCaseEl);
			});
		},
		
		serializeFailure: function(test, failure, testCaseEl)
		{
			var failureEl = testCaseEl.ownerDocument.createElement("failure");
			failureEl.setAttribute("message", failure.error);
			var errorText = testCaseEl.ownerDocument.createTextNode(failure.assert + " " + failure.error);
			failureEl.appendChild(errorText);
			testCaseEl.appendChild(failureEl);
		},
		
		serializeError: function(test, error, testCaseEl)
		{
			var errorEl = testCaseEl.ownerDocument.createElement("error");
			errorEl.setAttribute("type", error.error);
			var errorText = testCaseEl.ownerDocument.createTextNode(error.assert + " " + error.error);
			errorEl.appendChild(errorText);
			testCaseEl.appendChild(errorEl);
		}
	});
});
