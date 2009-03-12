testSuite("Titanium.Desktop API tests", "dummy.html",
{
	run: function()
	{
		test("Titanium.Desktop API", function()
		{
			assertNotNull(Titanium.Desktop);
			assertNotNull(Titanium.Desktop.openApplication);
			assertNotNull(Titanium.Desktop.openURL);
		});
	}
});
