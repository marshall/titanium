testSuite("Titanium.UI Window API tests", "dummy.html",
{
	run: function()
	{
		test("Titanium.UI window accessors", function()
		{
			var ui = Titanium.UI;

			assertNotNull(ui);
			assertNotNull(ui.mainWindow);
			assertNotNull(ui.currentWindow);
			assertNotNull(ui.createWindow);
		});

		test("Titanium.UI window object API", function()
		{
			var ui = Titanium.UI;
			assertNotNull(ui);

			var m = ui.mainWindow;
			assertNotNull(m);

			var props = ['X', 'Y', 'Width', 'Height', 'MaxHeight', 'MinHeight',
				'MaxWidth', 'MinWidth', 'Bounds',
				'Title', 'URL', 'Transparency', 'Menu', 'ContextMenu', 'Icon'];

			var immutableProps = ['ID'];

			var boolProps = ['Resizable', 'Maximizable', 'Minimizable',
				'Closeable', 'FullScreen', 'Visible', 'UsingChrome', 'TopMost'];
			
			$.each(props, function() {
				assertNotNull(m["get"+this]);
				assertNotNull(m["set"+this]);
			});

			$.each(immutableProps, function() {
				assertNotNull(m["get"+this]);
				assertNull(m["set"+this]);
			});

			$.each(boolProps, function() {
				assertNotNull(m["is"+this]);
				assertNotNull(m["set"+this]);
			});
			
			assertNotNull(m.open);
			assertNotNull(m.close);
			assertNotNull(m.hide);
			assertNotNull(m.show);
			assertNotNull(m.focus);
			assertNotNull(m.unfocus);
			assertNotNull(m.createWindow);
			assertNotNull(m.openFiles);
			assertNotNull(m.getParent);
			assertNotNull(m.addEventListener);
			assertNotNull(m.removeEventListener);
		});

		test("Titanium.UI assert window data", function()
		{
			var ui = Titanium.UI;
			assertNotNull(ui);

			var m = ui.mainWindow;

			assertEquals(m.getID(), "initial");
			assertEquals(m.getWidth(), 800);
			assertEquals(m.getHeight(), 600);
			assertEquals(m.getURL(), "app://com.titaniumapp.apivalidator/index.html");
			assert(m.isMaximizable());
			assert(m.isMinimizable());
			assert(m.isCloseable());
			assert(m.isResizable());
			assert(m.isUsingChrome());
			assertEquals(m.getTransparency(), 1.0);
			
			var b = m.getBounds();
			assertNotNull(b);
			assertEquals(b.x, m.getX());
			assertEquals(b.y, m.getY());
			assertEquals(b.width, m.getWidth());
			assertEquals(b.height, m.getHeight());
			assertNull(m.getParent());
		});
		
		test("Titanium.UI window tests", function() {
			var ui = Titanium.UI;
			var m = ui.mainWindow;
			
			m.hide();
			assert(!m.isVisible());
			
			m.show();
			assert(m.isVisible());
			
			var originalBounds = m.getBounds();
			var b = {
				width: 350, height: 500,
				x: 25, y: 30
			};
			m.setBounds(b);
			assertEquals(m.getX(), 25);
			assertEquals(m.getY(), 30);
			assertEquals(m.getWidth(), 350);
			assertEquals(m.getHeight(), 500);
			
			m.setBounds(originalBounds);
			assertEquals(originalBounds.x, m.getBounds().x);
			assertEquals(originalBounds.y, m.getBounds().y);
			assertEquals(originalBounds.width, m.getBounds().width);
			assertEquals(originalBounds.height, m.getBounds().height);
			
			m.setMaxHeight(500);
			assertEquals(m.getMaxHeight(), 500);
			m.setHeight(599);
			assertEquals(m.getHeight(), 500);
		
		});
		
		test("Titanium.UI window events", function () {
			var firedEvent = null;
			var listener = function(event) {
				firedEvent = event;
			};
			var waitForEvent = function(fn) {
				firedEvent = null;
				fn();
				while (firedEvent == null); 
			};
			
			var m = Titanium.UI.mainWindow;
			
			m.addEventListener(listener);
			m.focus();
			assertEquals(firedEvent, 'focused');
			m.unfocus();
			assertEquals(firedEvent, 'unfocused');
			m.hide();
			assertEquals(firedEvent, 'hidden');
			m.show();
			assertEquals(firedEvent, 'shown');
			m.setFullScreen(true);
			assertEquals(firedEvent, 'fullscreened');
			m.setFullScreen(false);
			assertEquals(firedEvent, 'unfullscreened');
			var originalWidth = m.getWidth();
			m.setWidth(201);
			assertEquals(firedEvent, 'resized');
			m.setWidth(originalWidth);
			var originalX = m.getX();
			m.setX(37);
			assertEquals(firedEvent, 'moved');
			m.setX(originalX);
			
			m.removeEventListener(listener);
			firedEvent = null;
			m.focus();
			assert(firedEvent == null);
		});
	}
});
