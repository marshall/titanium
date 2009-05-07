//
// application analytics module
//
(function()
{
	
	var url = Titanium.App.getStreamURL("app-track");
	var guid = null;
	var sid = null;
	var debug = false;
	var initialized = false;
	
	function send(qsv,async,timeout)
	{
		try
		{
			// if we're offline we don't even attempt these
			if (qsv.event!='ti.start' && Titanium.Network.online===false)
			{
				Titanium.API.debug("we're not online - skipping analytics");
				return;
			}
			async = (typeof async=='undefined') ? true : async;
			qsv.mid = Titanium.Platform.id;
			qsv.guid = guid;
			qsv.sid = sid;
			qsv.mac_addr = Titanium.Platform.macaddress;
			qsv.osver = Titanium.Platform.version;
			qsv.platform = Titanium.platform;
			qsv.version =Titanium.version;
			qsv.app_version =Titanium.App.getVersion();
			qsv.os =Titanium.Platform.name;
			qsv.ostype =Titanium.Platform.ostype;
			qsv.osarch =Titanium.Platform.architecture;
			qsv.oscpu =Titanium.Platform.processorCount;
			qsv.un =Titanium.Platform.username;
			qsv.ip =Titanium.Platform.address;
			
			
			var qs = '';
			for (var p in qsv)
			{
				var v = typeof(qsv[p])=='undefined' ? '' : String(qsv[p]);
				qs+=p+'='+Titanium.Network.encodeURIComponent(v)+'&';
			}
			// this is asynchronous
			var xhr = Titanium.Network.createHTTPClient();
			if (timeout > 0)
			{
				xhr.setTimeout(timeout);
			}
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
	
	/**
	 * @tiapi(method=True,name=Analytics.addEvent,since=0.3) send an analytics event associated with the application
	 * @tiarg(for=Analytics.addEvent,type=string,name=event) event name
	 * @tiarg(for=Analytics.addEvent,type=object,name=data,optional=True) event data
	 */
	Titanium.API.set("Analytics.addEvent", function(event,data)
	{
		send({'event':event,'data':data});
	});
	
	Titanium.API.register("ti.UI.stop",function(name)
	{
		send({'event':'ti.end'},false,5000);
	});
	
	Titanium.API.register("ti.UI.window.page.init",function(name,event)
	{
		try
		{
			if (initialized===true)
			{
				return;
			}

			initialized = true;

			if (!Titanium.Platform.id)
			{
				Titanium.API.debug("No machine id found");
				return;
			}
			
			guid = Titanium.App.getGUID();
			sid = Titanium.Platform.createUUID();
			
			send({
				'platform': Titanium.platform,
				'version':Titanium.version,
				'app_version':Titanium.App.getVersion(),
				'os':Titanium.Platform.name,
				'ostype':Titanium.Platform.ostype,
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
