testSuite("ti.App API tests", "dummy.html",
{
	run: function()
	{
		test("runtime accessor", function()
		{
			assert(ti != null);
			assert(ti.App != null);	
		});

		test("toplevel App API", function()
		{
			assert(ti.App != null);
			assert(ti.App.include != null);
			assert(ti.App.debug != null);
			assert(ti.App.hide != null);
			assert(ti.App.show != null);
			assert(ti.App.activate != null);
			assert(ti.App.quit != null);	
		});

		test("include test", function()
		{
			assert(ti.App.include != null);

			ti.App.include("app://tests/testinclude.js");
			assert(window.TESTVAL == 100);
		});
	}
});
