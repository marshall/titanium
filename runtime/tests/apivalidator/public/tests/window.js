testSuite("ti.Window API tests",
{

	run: function()
	{
		test("runtime accessor", function()
		{
			assert(ti != null);
			assert(ti.Window != null);
		});

		test("toplevel window API", function()
		{
			var w = ti.Window;

			assert(w != null);
			assert(w.mainWindow != null);
			assert(w.currentWindow != null);
			assert(w.createWindow != null);
		});

		test("window object API", function()
		{
			var w = ti.Window;
			assert(w != null);

			var m = w.mainWindow;
			assert(m != null);

			var props = ['X', 'Y', 'Width', 'Height', 'Bounds',
				'Title', 'URL', 'Transparency'];

			var immutableProps = ['ID'];

			var boolProps = ['Resizable', 'Maximizable', 'Minimizable',
				'Closeable', 'Fullscreen', 'Visible', 'UsingChrome', 'UsingScrollbars'];

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


			assert(m.open != null);
			assert(m.hide != null);
			assert(m.show != null);
			assert(m.close != null);
		});

		test("assert correct data", function()
		{
			var w = ti.Window;
			assert(w != null);

			var m = w.mainWindow;

			assert(m.getID() == "initial");
			assert(m.getWidth() == 800);
			assert(m.getHeight() == 600);
			assert(m.getURL() == "app://index.html");
			assert(m.isMaximizable());
			assert(m.isMinimizable());
			assert(m.isCloseable());
			assert(m.isResizable());
			assert(m.isUsingChrome());
			assert(m.isUsingScrollbars());
			assert(m.getTransparency() == 1.0);
			
			var b = m.getBounds();
			assert(b != null);
			assert(b.getX() == m.getX());
			assert(b.getY() == m.getY());
			assert(b.getWidth() == m.getWidth());
			assert(b.getHeight() == m.getHeight());

		});
	}
});
