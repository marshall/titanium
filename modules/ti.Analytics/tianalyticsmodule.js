//
// application analytics module
//
(function()
{
	var url = "http://publisher.titaniumapp.com/api/app-track";
	var guid = Titanium.App.getGUID();
	var sid = Titanium.Platform.createUUID();
	var debug = false;
	
	function send(qsv,async)
	{
		try
		{
			async = (typeof async=='undefined') ? true : async;
			qsv.mid = Titanium.Platform.id;
			qsv.guid = guid;
			qsv.sid = sid;
			
			var qs = '';
			for (var p in qsv)
			{
				var v = typeof(qsv[p])=='undefined' ? '' : String(qsv[p]);
				qs+=p+'='+Titanium.Network.encodeURIComponent(v)+'&';
			}
			// this is asynchronous
			var xhr = Titanium.Network.createHTTPClient();
			xhr.setRequestHeader('Content-Type','application/x-www-form-urlencoded');
			if (debug)
			{
				xhr.onreadystatechange = function()
				{
					if (this.readyState==4)
					{
						Titanium.API.debug("++ received:"+this.responseText);
					}
				}
			}
			xhr.open('POST',url,async);
			xhr.send(qs);
		}
		catch(E)
		{
			Titanium.API.debug("Error sending data: "+E);
		}
	}
	
	Titanium.API.set("Titanium.Analytics.addEvent",function(event,data)
	{
		send({'event':event,'data':data});
	});
	
	Titanium.API.register("ti.UI.stop",function(name)
	{
		send({'event':'ti.end'},false);
	});
	
	Titanium.API.register("ti.UI.window.page.init",function(name,event)
	{
		try
		{
			var user_window = event.window;
			if (user_window.track_registered===true)
			{
				return;
			}
			user_window.track_registered = true;
			if (!Titanium.Platform.id)
			{
				Titanium.API.debug("No machine id found");
				return;
			}
			send({
				'platform': Titanium.platform,
				'version':Titanium.version,
				'mac_addr': Titanium.Platform.macaddress,
				'os':Titanium.Platform.name,
				'ostype':Titanium.Platform.ostype,
				'osver':Titanium.Platform.version,
				'osarch':Titanium.Platform.architecture,
				'oscpu':Titanium.Platform.processorCount,
				'un':Titanium.Platform.username,
				'ip':Titanium.Platform.address,
				'event':'ti.start'
			});
		}
		catch(e)
		{
			// never never never die in this function
			Titanium.API.error("Error: "+e);
		}
	});	
})();
