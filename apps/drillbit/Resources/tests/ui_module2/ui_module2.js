describe("UI Module Tests",{
	test_user_window_title: function()
	{
		var w = Titanium.UI.getCurrentWindow().createWindow('http://www.google.com');

		var title = "blahahahahahahahahahahahahaha";
		w.setTitle(title);
		value_of(w.getTitle()).should_be(title);
		title = "    ___       ";
		w.setTitle(title);
		value_of(w.getTitle()).should_be(title);
		title = "    _¬˚∆¬˚¬∂ƒ¬∆ø´ˆ∆¬ßƒ∂ˆ__       ";
		w.setTitle(title);
		value_of(w.getTitle()).should_be(title);
		title = "";
		w.setTitle(title);
		value_of(w.getTitle()).should_be(title);

		w.open();

		title = "blahahahahahahahahahahahahaha";
		w.setTitle(title);
		value_of(w.getTitle()).should_be(title);
		title = "    ___       ";
		w.setTitle(title);
		value_of(w.getTitle()).should_be(title);
		title = "    _¬˚∆¬˚¬∂ƒ¬∆ø´ˆ∆¬ßƒ∂ˆ__       ";
		w.setTitle(title);
		value_of(w.getTitle()).should_be(title);
		title = "";
		w.setTitle(title);
		value_of(w.getTitle()).should_be(title);

		w.close();
	},

	test_user_window_url: function()
	{
		var w = Titanium.UI.getCurrentWindow().createWindow('http://www.google.com');

		var url = "http://www.google.com";
		w.setURL(url);
		value_of(w.getURL()).should_be(url);
		url = "http://gmail.com";
		w.setURL(url);
		value_of(w.getURL()).should_be(url);
		url = "http://www.bbc.co.uk";
		w.setURL(url);
		value_of(w.getURL()).should_be(url);
		url = "http://mail.google.com";
		w.setURL(url);
		value_of(w.getURL()).should_be(url);

		w.open();

		url = "http://www.google.com";
		w.setURL(url);
		value_of(w.getURL()).should_be(url);
		url = "http://gmail.com";
		w.setURL(url);
		value_of(w.getURL()).should_be(url);
		url = "http://www.bbc.co.uk";
		w.setURL(url);
		value_of(w.getURL()).should_be(url);
		url = "http://mail.google.com";
		w.setURL(url);
		value_of(w.getURL()).should_be(url);
		w.close();
	},
	test_window_listener_as_async: function(callback)
	{
		var w = Titanium.UI.getCurrentWindow().createWindow('app://blahblah.html');

		var closeMsg = "close";
		var closedMsg = "closed";
		var openMsg = "open";
		var openedMsg = "opened";
		var hiddenMsg = "hidden";

		var messageLog = { };
		messageLog[closeMsg]  = false;
		messageLog[closedMsg]  = false
		messageLog[openMsg]  = false;
		messageLog[openedMsg]  = false;
		messageLog[hiddenMsg]  = false;

		w.addEventListener(function(eventName, eventData)
		{
			messageLog[eventName] = true;
		});

		w.open();
		setTimeout(function()
		{
			w.close();
			if (messageLog[openMsg] !== true)
			{
				callback.failed("Did not detect open message");
			}
			else if (messageLog[openedMsg] !== true)
			{
				callback.failed("Did not detect opened message");
			}
			else
			{
				callback.passed();
			}
		}, 1000);
	}
});
