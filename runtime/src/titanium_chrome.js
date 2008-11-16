ti.Chrome = {};

ti.Chrome.run = function ()
{
	if (ti.App.windows.length > 1)
	{
		// We only launch non-main windows, it's the job of the Shell app to do the first-pass parse and
		// create the primary window
		
		for (var i = 1; i < ti.App.windows.length; i++)
		{
			if ('openWindow' in TiNative) {
				var win = ti.App.windows[i];
				
				TiNative.openWindow(win.width, win.height, win.title, win.start);
			}
		}
	}
}
