testSuite("ti.Menu API tests", "dummy.html", {
	run: function () {
		test("top level API", function() {
			assert(ti != null);
			assert(ti.Menu != null);
			assert(ti.Menu.createAppMenu != null);
			assert(ti.Menu.createTrayMenu != null);
		});

		test("menu object API", function() {
			assert(ti != null);
			assert(ti.Menu != null);

			var menu = ti.Menu.createAppMenu("apivalidator");
			assert(menu != null);
			assert(menu.addItem != null);
			assert(menu.addSeparator != null);
			assert(menu.addSubMenu != null);

			var submenu = menu.addSubMenu("submenu");
			assert(submenu != null);
			assert(submenu.addItem != null);
			assert(submenu.addSeparator != null);
			assert(submenu.addSubMenu != null);

			menu = ti.Menu.createTrayMenu("app://tests/test.ico", "Caption", function() {});
			assert(menu != null);
			assert(menu.addItem != null);
			assert(menu.addSeparator != null);
			assert(menu.addSubMenu != null);
			
		});
	}
});

