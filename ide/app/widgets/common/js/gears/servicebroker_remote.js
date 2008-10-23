/**
 * this is a pure gears-powered servicebroker
 *
 * this code will be executed inside the gears workerpool (outside of browser)
 */

var wp = google.gears.workerPool;
var timer = null;
var mainSender = null;

function send(method,url,postBody,contentType,sender)
{
	var req = google.gears.factory.create('beta.httprequest');
	req.open(method,url);
	req.setRequestHeader('Content-Type',contentType);
	req.setRequestHeader('X-Requested-With','XMLHttpRequest');
	req.onreadystatechange = function() 
	{
	  if (req.readyState == 4) 
	  {
         wp.sendMessage(req.getResponseHeader('Content-type')+'|||'+req.status+'|||'+req.responseText,sender||mainSender);
	  }
	};
	req.send(postBody);
}

function config(parameters)
{
	if (parameters.poll && parameters.url)
	{
		// create a polling timer that will constantly invoke a poll
		// based on pre-configured iterval
		google.gears.factory.create('beta.timer').setInterval(function()
		{
			send('GET',parameters.url,'text/plain');
		},parameters.interval);
	}
}

wp.onmessage = function(a, b, message) 
{
	if (!mainSender)
	{
		mainSender = message.sender;
	}
	
	var instructions = eval('('+a+')');
	
	switch(instructions.type)
	{
		case 'gears.init':
		{
			config(instructions);
			return;
		}
	}
	
	var url = instructions.url;
    var method = instructions.method;
    var postBody = instructions.postBody;
    var contentType = instructions.contentType;
    
	send(method,url,postBody,contentType,message.sender);
}