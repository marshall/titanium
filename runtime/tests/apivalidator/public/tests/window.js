testSuite("ti.Window API tests",
{

	run: function()
	{
		var w = null;
	
		test("runtime accessor", function()
		{
			assert(ti != null);
			assert(ti.Window != null);

			w = ti.Window;
		});

		test("toplevel window API", function()
		{
			assert(w != null);
			assert(w.mainWindow != null);
			assert(w.currentWindow != null);
			assert(w.createWindow != null);
		});

		test("window object API", function()
		{
			assert(w != null);

			var m = w.mainWindow;
			assert(m != null);

			var props = ['ID', 'X', 'Y', 'Width', 'Height', 'Bounds',
				'Title', 'URL', 'Transparency'];

			var boolProps = ['Resizable', 'Maximizable', 'Minimizable',
				'Closeable', 'Fullscreen', 'Visible', 'UsingChrome', 'UsingScrollbars'];

			$.each(props, function() {
				assert(m["get"+this] != null);
				assert(m["set"+this] != null);
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
			var m = w.mainWindow;
			var c = w.currentWindow;

			assert(m == c);
			assert(m.getID() == "main");
			assert(m.getX() == 100);
			assert(m.getY() == 100);
			assert(m.getWidth() == 400);
			assert(m.getHeight() == 400);
			
			var b = m.getBounds();
			assert(b != null);
		});
	}
});
