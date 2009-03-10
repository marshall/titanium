testSuite("Titanium.UI Menu API tests", "dummy.html", {
	run: function () {
		test("Titanium.UI Menu API", function() {
			assertNotNull(Titanium.UI);
			assertNotNull(Titanium.UI.createMenu);
			assertNotNull(Titanium.UI.createTrayMenu);
			assertNotNull(Titanium.UI.getMenu);
			assertNotNull(Titanium.UI.setMenu);
			assertNotNull(Titanium.UI.getContextMenu);
			assertNotNull(Titanium.UI.setContextMenu);
			assertNotNull(Titanium.UI.setDockMenu);
		});

		test("Titanium.UI Menu object API", function() {
			var menu = Titanium.UI.createMenu("apivalidator");
			assertNotNull(menu);
			assertNotNull(menu.addItem);
			assertNotNull(menu.addSeparator);
			assertNotNull(menu.addSubMenu);
			assertNotNull(menu.isSeparator);
			assertNotNull(menu.isItem);
			assertNotNull(menu.isSubMenu);
			assertNotNull(menu.enable);
			assertNotNull(menu.disable);
			assertNotNull(menu.setLabel);
			assertNotNull(menu.setIcon);

			var submenu = menu.addSubMenu("submenu");
			assertNotNull(submenu);
			assertNotNull(submenu.addItem);
			assertNotNull(submenu.addSeparator);
			assertNotNull(submenu.addSubMenu);
			assertNotNull(submenu.isSeparator);
			assertNotNull(submenu.isItem);
			assertNotNull(submenu.isSubMenu);
			assertNotNull(submenu.enable);
			assertNotNull(submenu.disable);
			assertNotNull(submenu.setLabel);
			assertNotNull(submenu.setIcon);

			var trayMenu = Titanium.UI.createTrayMenu();
			assertNotNull(trayMenu);
			assertNotNull(trayMenu.addItem);
			assertNotNull(trayMenu.addSeparator);
			assertNotNull(trayMenu.addSubMenu);
			assertNotNull(trayMenu.isSeparator);
			assertNotNull(trayMenu.isItem);
			assertNotNull(trayMenu.isSubMenu);
			assertNotNull(trayMenu.enable);
			assertNotNull(trayMenu.disable);
			assertNotNull(trayMenu.setLabel);
			assertNotNull(trayMenu.setIcon);
		});
		
		test("Titanium.UI Menu object data", function() {
			var menu = Titanium.UI.createMenu("apivalidator");
			var submenu = menu.addSubMenu("submenu");
			var trayMenu = Titanium.UI.createTrayMenu();
			var sep = menu.addSeparator();
			var item = menu.addItem("label", function(){});
			
			assert(!menu.isSeparator());
			assert(!menu.isItem());
			assert(!menu.isSubMenu());
			assert(!submenu.isItem());
			assert(submenu.isSubMenu());
			assert(!trayMenu.isSeparator());
			assert(!trayMenu.isItem());
			assert(!trayMenu.isSubMenu());
			
			assert(sep.isSeparator());
			assert(!sep.isItem());
			assert(!sep.isSubMenu());
			
			// need to assert the callback/label ?
			assert(item.isItem());
			assert(!item.isSeparator());
			assert(!item.isSubMenu());
		});
	}
});

