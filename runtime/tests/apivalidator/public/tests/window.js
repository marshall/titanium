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
			assert(w.mainWindow != null);
			assert(w.currentWindow != null);
			assert(w.createWindow != null);
		});

		test("window object API", function()
		{
			var m = w.mainWindow;

			var assertGetterSetter = function (prop,b) {
				var get = "get";
				if (b) {
					get = "is";
				}
				eval("assert(m."+get+prop+" != null);");
				eval("assert(m.set"+prop+" != null);");
			};

			var props = ['ID', 'X', 'Y', 'Width', 'Height', 'Bounds',
				'Title', 'URL', 'Transparency'];

			var boolProps = ['Resizable', 'Maximizable', 'Minimizable',
				'Closeable', 'Fullscreen', 'Visible', 'UsingChrome', 'UsingScrollbars'];

			$.each(props, function() {
				assertGetterSetter(this, false);	
			});

			$.each(boolProps, function() {
				assertGetterSetter(this, true);
			});


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
