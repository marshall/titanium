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
			assert(Titanium.App.include != null);
			assert(Titanium.App.debug != null);
			assert(Titanium.App.hide != null);
			assert(Titanium.App.show != null);
			assert(Titanium.App.activate != null);
			assert(Titanium.App.quit != null);	
		});
	}
});
