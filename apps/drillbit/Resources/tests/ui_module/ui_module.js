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

		w.setMaxWidth(10000);
		w.setMaxHeight(10000);
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

		w.setMinWidth(1);
		w.setMinHeight(1);
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
});
