testSuite("Titanium.UI API tests", "dummy.html", {
	run: function () {
		
		test("Titanium.UI runtime accessor", function()
		{
			assertNotNull(Titanium);
			assertNotNull(Titanium.UI);
		});
		
		test("Titanium.UI API", function() {
			assertNotNull(Titanium.UI);
			assertNotNull(Titanium.UI.setIcon);
			assertNotNull(Titanium.UI.addTray);
			assertNotNull(Titanium.UI.clearTray);
			assertNotNull(Titanium.UI.setDockIcon);
			assertNotNull(Titanium.UI.setBadgeImage);
			assertNotNull(Titanium.UI.getIdleTime);
		});
	},
});
