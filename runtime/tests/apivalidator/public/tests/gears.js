testSuite("Gears binding tests", "dummy.html", {

	run: function ()
	{
		test("top level Gears modules", function() {
			assert(ti != null);
			assert(ti.Desktop != null);
			assert(ti.Database != null);
			assert(ti.Filesystem != null);
		});
	}
}); 
