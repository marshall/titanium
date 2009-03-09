testSuite("Titanium.UI API tests", "dummy.html",
{

	run: function()
	{
		test("runtime accessor", function()
		{
			assert(Titanium != null);
			assert(Titanium.UI != null);
		});

		test("toplevel UI API", function()
		{
			var ui = Titanium.UI;

			assert(ui != null);
			assert(ui.mainWindow != null);
			assert(ui.currentWindow != null);
			assert(ui.createWindow != null);
		});

		test("window object API", function()
		{
			var ui = Titanium.UI;
			assert(ui != null);

			var m = ui.mainWindow;
			assert(m != null);

			var props = ['X', 'Y', 'Width', 'Height', 'Bounds',
				'Title', 'URL', 'Transparency'];

			var immutableProps = ['ID'];

			var boolProps = ['Resizable', 'Maximizable', 'Minimizable',
				'Closeable', 'Fullscreen', 'Visible', 'UsingScrollbars'];

			var immutableBoolProps = ['UsingChrome'];
			
			$.each(props, function() {
				assert(m["get"+this] != null);
				assert(m["set"+this] != null);
			});

			$.each(immutableProps, function() {
				assert(m["get"+this] != null);
				assert(m["set"+this] == null);
			});

			$.each(boolProps, function() {
				assert(m["is"+this] != null);
				assert(m["set"+this] != null);
			});
			
			$.each(immutableProps, function() {
				assert(m["get"+this] != null);
				assert(m["set"+this] == null);
			});


			assert(m.open != null);
			assert(m.hide != null);
			assert(m.show != null);
			assert(m.close != null);
		});

		test("assert correct data", function()
		{
			var ui = Titanium.UI;
			assert(ui != null);

			var m = ui.mainWindow;

			assert(m.getID() == "initial");
			assert(m.getWidth() == 800);
			assert(m.getHeight() == 600);
			assert(m.getURL() == "app://com.titaniumapp.apivalidator/index.html");
			assert(m.isMaximizable());
			assert(m.isMinimizable());
			assert(m.isCloseable());
			assert(m.isResizable());
			assert(m.isUsingChrome());
			assert(m.isUsingScrollbars());
			assert(m.getTransparency() == 1.0);
			
			var b = m.getBounds();
			assert(b != null);
			assert(b.x == m.getX());
			assert(b.y == m.getY());
			assert(b.width == m.getWidth());
			assert(b.height == m.getHeight());

		});
	}
});
