describe("UI Module Tests",{
	test_ui_module_methods: function()
	{
		value_of(Titanium.UI.addTray).should_be_function();
		value_of(Titanium.UI.clearTray).should_be_function();
		value_of(Titanium.UI.createMenu).should_be_function();
		value_of(Titanium.UI.createTrayMenu).should_be_function();
		value_of(Titanium.UI.getContextMenu).should_be_function();
		value_of(Titanium.UI.getIdleTime).should_be_function();
		value_of(Titanium.UI.getMenu).should_be_function();
		value_of(Titanium.UI.mainWindow).should_be_object();
		value_of(Titanium.UI.setContextMenu).should_be_function();
		value_of(Titanium.UI.setDockIcon).should_be_function();
		value_of(Titanium.UI.setDockMenu).should_be_function();
		value_of(Titanium.UI.setIcon).should_be_function();
		value_of(Titanium.UI.setMenu).should_be_function();
		value_of(Titanium.UI.setBadge).should_be_function();
		value_of(Titanium.UI.setBadgeImage).should_be_function();
	},
	test_windows_array: function()
	{
		value_of(Titanium.UI.windows).should_be_object();
		value_of(Titanium.UI.windows.length).should_be(1);
	},
	test_window_max_size: function()
	{
		var w = Titanium.UI.currentWindow.createWindow();
		w.setHeight(700);
		w.setWidth(700);

		value_of(w.getHeight()).should_be(700);
		value_of(w.getWidth()).should_be(700);

		w.setMaxHeight(500);
		value_of(w.getHeight()).should_be(500);

		w.setMaxWidth(400);
		value_of(w.getWidth()).should_be(400);

		w.setHeight(700);
		w.setWidth(700);
		value_of(w.getHeight()).should_be(500);
		value_of(w.getWidth()).should_be(400);

		w.setMaxWidth(-1);
		w.setMaxHeight(-1);
		w.setHeight(700);
		w.setWidth(700);

		w.open();

		value_of(w.getHeight()).should_be(700);
		value_of(w.getWidth()).should_be(700);

		w.setMaxHeight(500);
		value_of(w.getHeight()).should_be(500);

		w.setMaxWidth(400);
		value_of(w.getWidth()).should_be(400);

		w.setHeight(700);
		w.setWidth(700);
		value_of(w.getHeight()).should_be(500);
		value_of(w.getWidth()).should_be(400);

		w.close();
	},
	test_window_min_size: function()
	{
		var w = Titanium.UI.currentWindow.createWindow();
		w.setHeight(100);
		w.setWidth(100);

		value_of(w.getHeight()).should_be(100);
		value_of(w.getWidth()).should_be(100);

		w.setMinHeight(500);
		value_of(w.getHeight()).should_be(500);

		w.setMinWidth(400);
		value_of(w.getWidth()).should_be(400);

		w.setHeight(100);
		w.setWidth(100);
		value_of(w.getHeight()).should_be(500);
		value_of(w.getWidth()).should_be(400);

		w.setMinWidth(-1);
		w.setMinHeight(-1);
		w.setHeight(100);
		w.setWidth(100);

		w.open();
		value_of(w.getHeight()).should_be(100);
		value_of(w.getWidth()).should_be(100);

		w.setMinHeight(500);
		value_of(w.getHeight()).should_be(500);

		w.setMinWidth(400);
		value_of(w.getWidth()).should_be(400);

		w.setHeight(100);
		w.setWidth(100);
		value_of(w.getHeight()).should_be(500);
		value_of(w.getWidth()).should_be(400);

		w.close();
	},
	test_window_set_height: function()
	{
		var w = Titanium.UI.currentWindow.createWindow();
		w.setHeight(100);
		value_of(w.getHeight()).should_be(100);
		w.setHeight(200);
		value_of(w.getHeight()).should_be(200);
		w.setHeight(100);
		value_of(w.getHeight()).should_be(100);
		w.setHeight(10000);
		value_of(w.getHeight()).should_be(10000);
		w.setHeight(100);
		value_of(w.getHeight()).should_be(100);
		w.setHeight(-1);
		value_of(w.getHeight()).should_be(100);
		w.setHeight(0);
		value_of(w.getHeight()).should_be(100);
		w.open();
		w.setHeight(100);
		value_of(w.getHeight()).should_be(100);
		w.setHeight(200);
		value_of(w.getHeight()).should_be(200);
		w.setHeight(100);
		value_of(w.getHeight()).should_be(100);
		w.setHeight(10000);
		value_of(w.getHeight()).should_be(10000);
		w.setHeight(100);
		value_of(w.getHeight()).should_be(100);
		w.setHeight(-1);
		value_of(w.getHeight()).should_be(100);
		w.setHeight(0);
		value_of(w.getHeight()).should_be(100);
	},
	test_window_set_width: function()
	{
		var w = Titanium.UI.currentWindow.createWindow();
		w.setWidth(100);
		value_of(w.getWidth()).should_be(100);
		w.setWidth(200);
		value_of(w.getWidth()).should_be(200);
		w.setWidth(100);
		value_of(w.getWidth()).should_be(100);
		w.setWidth(10000);
		value_of(w.getWidth()).should_be(10000);
		w.setWidth(100);
		value_of(w.getWidth()).should_be(100);
		w.setWidth(-1);
		value_of(w.getWidth()).should_be(100);
		w.setWidth(0);
		value_of(w.getWidth()).should_be(100);
		w.open()
		w.setWidth(100);
		value_of(w.getWidth()).should_be(100);
		w.setWidth(200);
		value_of(w.getWidth()).should_be(200);
		w.setWidth(100);
		value_of(w.getWidth()).should_be(100);
		w.setWidth(10000);
		value_of(w.getWidth()).should_be(10000);
		w.setWidth(100);
		value_of(w.getWidth()).should_be(100);
		w.setWidth(-1);
		value_of(w.getWidth()).should_be(100);
		w.setWidth(0);
		value_of(w.getWidth()).should_be(100);
	},
	test_window_set_closeable: function()
	{
		var w = Titanium.UI.currentWindow.createWindow({closeable: false});
		value_of(w.isCloseable()).should_be_false();
		w.setCloseable(true);
		value_of(w.isCloseable()).should_be_true();
		w.open();
		value_of(w.isCloseable()).should_be_true();
		w.setCloseable(false);
		value_of(w.isCloseable()).should_be_false();
		w.setCloseable(true);
		value_of(w.isCloseable()).should_be_true();
	},
	test_window_set_minimizable: function()
	{
		var w = Titanium.UI.currentWindow.createWindow({minimizable: false});
		value_of(w.isMinimizable()).should_be_false();
		w.setMinimizable(true);
		value_of(w.isMinimizable()).should_be_true();
		w.open();
		value_of(w.isMinimizable()).should_be_true();
		w.setMinimizable(false);
		value_of(w.isMinimizable()).should_be_false();
		w.setMinimizable(true);
		value_of(w.isMinimizable()).should_be_true();
	},
	test_window_set_maximizable: function()
	{
		var w = Titanium.UI.currentWindow.createWindow({maximizable: false});
		value_of(w.isMaximizable()).should_be_false();
		w.setMaximizable(true);
		value_of(w.isMaximizable()).should_be_true();
		w.open();
		value_of(w.isMaximizable()).should_be_true();
		w.setMaximizable(false);
		value_of(w.isMaximizable()).should_be_false();
		w.setMaximizable(true);
		value_of(w.isMaximizable()).should_be_true();
	},
	test_window_set_using_chrome: function()
	{
		var w = Titanium.UI.currentWindow.createWindow({usingChrome: false});
		value_of(w.isUsingChrome()).should_be_false();
		w.setUsingChrome(true);
		value_of(w.isUsingChrome()).should_be_true();
		w.open();
		value_of(w.isUsingChrome()).should_be_true();
		w.setUsingChrome(false);
		value_of(w.isUsingChrome()).should_be_false();
		w.setUsingChrome(true);
		value_of(w.isUsingChrome()).should_be_true();
	},
	test_window_visibility: function()
	{
		var w = Titanium.UI.currentWindow.createWindow({visible: false});
		value_of(w.isVisible()).should_be_false();
		w.open();
		value_of(w.isVisible()).should_be_false();
		w.close();

		var w = Titanium.UI.currentWindow.createWindow({visible: true});
		value_of(w.isVisible()).should_be_false();
		w.setVisible(true);
		value_of(w.isVisible()).should_be_false();
		w.open();
		value_of(w.isVisible()).should_be_true();

		w.setVisible(false);
		value_of(w.isVisible()).should_be_false();

		w.show();
		value_of(w.isVisible()).should_be_true();
		w.hide();
		value_of(w.isVisible()).should_be_false();
	},
    
});
