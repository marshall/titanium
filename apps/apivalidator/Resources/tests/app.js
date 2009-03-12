testSuite("Titanium.App API tests", "dummy.html",
{
	run: function()
	{
		test("Titanium.App exists", function()
		{
			assertNotNull(Titanium);
			assertNotNull(Titanium.App);	
		});

		test("Titanium.App top-level API", function()
		{
			assertNotNull(Titanium.App);
			assertNotNull(Titanium.App.getID);
			assertNotNull(Titanium.App.getName);
			assertNotNull(Titanium.App.getVersion);
			assertNotNull(Titanium.App.getUpdateURL);
			assertNotNull(Titanium.App.getGUID);
			assertNotNull(Titanium.App.appURLToPath);
			assertNotNull(Titanium.App.arguments);
			assertNotNull(Titanium.App.exit);
			assertNotNull(Titanium.App.loadProperties);
			assertNotNull(Titanium.version);
			assertNotNull(Titanium.platform);
		});
		
		test("Titanium.App top-level data", function()
		{
			assertEquals(Titanium.App.getID(), "com.titaniumapp.apivalidator");
			assertEquals(Titanium.App.getName(), "apivalidator");
			assertEquals(Titanium.App.getVersion(), "0.1");
			assertEquals(Titanium.App.getUpdateURL(), "http://updatesite.titaniumapp.com");
			assert(Titanium.App.arguments.length >= 1);
			assert(Titanium.platform in {'win32':1, 'osx':1, 'linux':1});
		});
	}
});
