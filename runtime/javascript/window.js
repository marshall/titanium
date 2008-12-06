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
			win.hide(); // call this to cause the os to physically hide it
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
ti.Extras.fadeInWindow = function(win,stopAt,speed)
{
	speed = speed || 100;
	stopAt = stopAt || 1.0;
	var timer = null;
	win.setTransparency(0.1);
	timer = setInterval(function()
	{
		var t = win.getTransparency() + .1;
		if (t >= stopAt)
		{
			clearInterval(timer);
			// set it back to the original and hide the window
			win.setTransparency(stopAt);
			return;
		}
		win.setTransparency(t);
	}, speed);
};

if (ti.Window.currentWindow.isUsingChrome())
{
	var offsetx, offsety, handler;

	var defaultHandler = function(target,x,y)
	{
		if (y>30) return false;
		if ($(target).is(':input,img')) return false;
		return true;
	};
	
	function mover(e)
	{
		window.moveBy(e.clientX-offsetx,e.clientY-offsety);
		return false;
	}

	function cancel()
	{
		$(document).unbind('mousemove',mover);
	}

	$(document).bind('mousedown',function(e)
	{
		if (!handler) return; // allow this to be turned off by setting null
		var moveable = handler(e.target,e.clientX,e.clientY);
		if (moveable)
		{
			offsetx = e.clientX;
			offsety = e.clientY;
			$(document).bind('mousemove',mover);
			$(document).bind('mouseup',cancel);
		}
	});

	//
	// allow the application to specify the draggable logic on whether
	// a custom chrome window should be dragged on a mousedown event.
	// pass a function to this method and then the framework will call
	// your function on determining that the mouse was pressed in the 
	// window and you can determine whether or not you want us to 
	// proceed with tracking the movement of the window.  you can pass
	// null to this method to completely disable window dragging
	//
	// 
	ti.Extras.setDraggableRegionHandler = function(fn)
	{
		handler = fn;
	};
	
	// by default force the default region handler to be 
	// installed and used. the default handler will only drag if the 
	// mouse is in the top 30 pixels of the top of the window and
	// the mouse event isn't targeted in a form input element (such as
	// input, textarea, button) or an image element
	handler = defaultHandler;
}
