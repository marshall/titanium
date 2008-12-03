/*!(c) 2006-2008 Appcelerator, Inc. http://appcelerator.org
 * Licensed under the Apache License, Version 2.0. Please visit
 * http://license.appcelerator.com for full copy of the License.
 **/

// A simple plugin that automatically runs all test suites on page load
$(document).ready(function() {

	TestMonkey.installTestRunnerPlugin({
		onManifestLoaded: function ()
		{
			testRunner(testSuiteNames);
		}
	});
});
