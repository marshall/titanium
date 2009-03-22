testSuite("Kroll Binding Tests", "binding.html",
{
	run: function()
	{
		test("test python to js conversions", function()
		{
			assertFunction(window.js_py_get_array);
			var r = window.js_py_get_array();
			assertNotNull(r);
			assertArray(r);
			assertEquals(r.length,3);
		});
		
		/* TI-133 crashes
		test("test ruby to js conversions", function()
		{
			assertFunction(window.js_rb_get_array);
			var r = window.js_rb_get_array();
			assertNotNull(r);
			assertArray(r);
			assertEquals(r.length,3);
		});*/
	}
});
