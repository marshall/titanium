testSuite("Titanium.Dock API tests", "dummy.html",
{
	run: function()
	{
		test("toplevel Dock API", function()
		{
			assert(Titanium != null);
			assert(Titanium.Dock != null);
			assert(Titanium.Dock.setBadge != null);
		});
	}
});
