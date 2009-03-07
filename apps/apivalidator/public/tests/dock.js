testSuite("ti.Dock API tests", "dummy.html",
{
	run: function()
	{
		test("toplevel Dock API", function()
		{
			assert(ti != null);
			assert(ti.Dock != null);
			assert(ti.Dock.setBadge != null);
		});
	}
});
