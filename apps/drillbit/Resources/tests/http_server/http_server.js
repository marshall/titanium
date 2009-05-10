describe("http server tests",
{
	get_request_as_async: function(callback)
	{
		value_of(Titanium.Network.createHTTPServer).should_be_function();
		
		var server = Titanium.Network.createHTTPServer();
		value_of(server).should_be_object();
		value_of(server.bind).should_be_function();
		
		server.bind(8082,function(request,response)
		{
			try
			{
				value_of(request.getMethod()).should_be('GET');
				value_of(request.getURI()).should_be('/foo');
				value_of(request.hasHeader('Foo')).should_be_false();
				value_of(request.getHeader('Foo')).should_be_null();
				value_of(request.getContentType()).should_be('');
				value_of(request.getContentLength()).should_be(-1);
				value_of(request.read()).should_be_null();
				value_of(server.isClosed()).should_be_false();
				server.close();
				value_of(server.isClosed()).should_be_true();
				callback.passed();
			}
			catch(e)
			{
				callback.failed(e);
			}
		});
		
		var xhr = Titanium.Network.createHTTPClient();
		xhr.open("GET","http://127.0.0.1:8082/foo");
		xhr.send(null);
		
	},
	post_request_with_body_as_async: function(callback)
	{
		var server = Titanium.Network.createHTTPServer();
		
		server.bind(8082,function(request,response)
		{
			try
			{
				value_of(request.getMethod()).should_be('POST');
				value_of(request.getURI()).should_be('/foo');
				value_of(request.getContentType()).should_be('application/x-www-form-urlencoded');
				value_of(request.hasHeader('Foo')).should_be_true();
				value_of(request.getHeader('Foo')).should_be('Bar');
				value_of(request.getContentLength()).should_be(3);
				var blob = request.read();
				value_of(blob).should_be_object();
				value_of(blob.length).should_be(3);
				value_of(blob).should_be("a=b");
				callback.passed();
			}
			catch(e)
			{
				callback.failed(e);
			}
		});
		
		var xhr = Titanium.Network.createHTTPClient();
		xhr.setRequestHeader('Content-Type','application/x-www-form-urlencoded');
		xhr.setRequestHeader('Foo','Bar');
		xhr.open("POST","http://127.0.0.1:8082/foo");
		xhr.send("a=b");
	}
});