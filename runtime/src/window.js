ti.Extras = {};

//
// utility to fade out a window. takes a TiUserWindow
// and an optional speed.  the bigger the number, the 
// slower the fade. the smaller the number, the faster
// the fade
//
ti.Extras.fadeOutWindow = function(win,speed)
{
	speed = speed || 100;
	var original = win.getTransparency();
	var timer = null;
	timer = setInterval(function()
	{
		var t = win.getTransparency() - .1;
		if (t < 0.0)
		{
			clearInterval(timer);
			// set it back to the original and hide the window
			win.hide();
			win.setTransparency(original);
			return;
		}
		win.setTransparency(t);
	}, speed);
};

//
// utility to fade in a window. takes a TiUserWindow
// and an optional speed.  the bigger the number, the 
// slower the fade. the smaller the number, the faster
// the fade
//
ti.Extras.fadeInWindow = function(win,speed)
{
	speed = speed || 100;
	var timer = null;
	timer = setInterval(function()
	{
		var t = win.getTransparency() + .1;
		if (t >= 1.0)
		{
			clearInterval(timer);
			// set it back to the original and hide the window
			win.show();
			win.setTransparency(1.0);
			return;
		}
		win.setTransparency(t);
	}, speed);
};
