testSuite("Titanium.Menu API tests", "dummy.html", {
	run: function () {
		test("top level API", function() {
			assert(ti != null);
			assert(Titanium.Menu != null);
			assert(Titanium.Menu.createAppMenu != null);
			assert(Titanium.Menu.createTrayMenu != null);
		});

		test("menu object API", function() {
			assert(ti != null);
			assert(Titanium.Menu != null);

			var menu = Titanium.Menu.createAppMenu("apivalidator");
			assert(menu != null);
			assert(menu.addItem != null);
			assert(menu.addSeparator != null);
			assert(menu.addSubMenu != null);

			var submenu = menu.addSubMenu("submenu");
			assert(submenu != null);
			assert(submenu.addItem != null);
			assert(submenu.addSeparator != null);
			assert(submenu.addSubMenu != null);

			menu = Titanium.Menu.createTrayMenu("app://tests/test.ico", "Caption", function() {});
			assert(menu != null);
			assert(menu.addItem != null);
			assert(menu.addSeparator != null);
			assert(menu.addSubMenu != null);
			
		});
	}
});

