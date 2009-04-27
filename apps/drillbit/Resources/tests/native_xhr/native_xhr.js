describe("native XHR tests",
{
	before: function()
	{
		this.xhr = Titanium.Network.createHTTPClient();
	},
	
	after: function()
	{
		this.xhr = null;
	},
	
	xhr_properties:function()
	{
		value_of(this.xhr).should_be_object();
		
		var methods = ['open','abort','setRequestHeader','send','sendFile',
					   'sendDir','getResponseHeader','setTimeout'];
					
		var props = ['readyState','UNSENT','OPENED','HEADERS_RECEIVED','LOADING',
					 'DONE','responseText','responseXML','status','statusText',
					 'connected','onreadystatechange','ondatastream','onsendstream'];

		for (var c=0;c<methods.length;c++)
		{
			var method = methods[c];
			value_of(this.xhr[method]).should_be_function();
		}
		
		for (var c=0;c<props.length;c++)
		{
			var prop = props[c];
			value_of(this.xhr[prop]).should_not_be_undefined();
		}

		value_of(this.xhr.readyState).should_be(0);
		value_of(this.xhr.connected).should_be_false();
		
		value_of(this.xhr.UNSENT).should_be(0);
		value_of(this.xhr.OPENED).should_be(1);
		value_of(this.xhr.HEADERS_RECEIVED).should_be(2);
		value_of(this.xhr.LOADING).should_be(3);
		value_of(this.xhr.DONE).should_be(4);

	},
	
	twitter_as_async:function(callback)
	{
		value_of(this.xhr).should_be_object();
	
		var timer = null;
		var url = 'http://twitter.com/statuses/public_timeline.json';
		var xhr = this.xhr;
		
		this.xhr.onreadystatechange = function()
		{
			try
			{
				if (this.readyState == xhr.HEADERS_RECEIVED)
				{
					value_of(xhr.getResponseHeader('Content-Type')).should_be('application/json;charset=utf-8');
				}
				else if (this.readyState == xhr.DONE)
				{
					clearTimeout(timer);
					callback.passed();
					break;
				}
			}
			catch(e)
			{
				clearTimeout(timer);
				callback.failed(e);
			}
		};
		this.xhr.open("GET",url);
		value_of(this.xhr.readyState).should_be(this.xhr.OPENED);
		this.xhr.send(null);
		
		timer = setTimeout(function()
		{
			callback.failed('native XHR twitter timed out');
		},20000);
	}
});
