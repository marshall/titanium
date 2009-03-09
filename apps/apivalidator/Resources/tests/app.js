testSuite("Titanium.App API tests", "dummy.html",
{
	run: function()
	{
		test("runtime accessor", function()
		{
			assert(Titanium != null);
			assert(Titanium.App != null);	
		});

		test("toplevel App API", function()
		{
			assert(Titanium.App != null);
			assert(Titanium.App.getID != null);
			assert(Titanium.App.getName != null);
			assert(Titanium.App.getVersion != null);
			assert(Titanium.App.getUpdateURL != null);
			assert(Titanium.App.getGUID != null);
			assert(Titanium.App.appURLToPath != null);
			assert(Titanium.App.arguments != null);
			assert(Titanium.App.exit != null);
			assert(Titanium.App.loadProperties != null);
			assert(Titanium.version != null);
			assert(Titanium.platform != null);	
		});
	}
});
