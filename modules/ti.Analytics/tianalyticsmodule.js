//
// application analytics module
//
(function()
{
	var update_check_delay = 5000; // how many ms before we initiate check

//	var update_check_interval_secs = (60000 * 24) / 1000; // once per 24 hours
	var update_check_interval_secs = 1; // once per startup during testing
	
	var url = Titanium.App.getStreamURL("app-track");
	var guid = null;
	var sid = null;
	var debug = false;
	var initialized = false;
	var window = null;
	
	function send(qsv,async,timeout)
	{
		try
		{
			// if we're offline we don't even attempt these
			if (qsv.event!='ti.start' && qsv.event!='ti.end' && Titanium.Network.online===false)
			{
				//TODO: we need to place these in DB and re-send later
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
			qsv.version = Titanium.version;
			qsv.app_version = Titanium.App.getVersion();
			qsv.os = Titanium.Platform.name;
			qsv.ostype = Titanium.Platform.ostype;
			qsv.osarch = Titanium.Platform.architecture;
			qsv.oscpu = Titanium.Platform.processorCount;
			qsv.un = Titanium.Platform.username;
			qsv.ip = Titanium.Platform.address;
			
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
			Titanium.API.debug("Error sending analytics data: "+E);
		}
	}
	
	/**
	 * @tiapi(method=True,name=Analytics.addEvent,since=0.3) Sends an analytics event associated with the application
	 * @tiarg(for=Analytics.addEvent,type=string,name=event) event name
	 * @tiarg(for=Analytics.addEvent,type=object,name=data,optional=True) event data
	 */
	Titanium.API.set("Analytics.addEvent", function(event,data)
	{
		send({'event':event,'data':data});
	});
	
	var update_handler;

	/**
	 * @tiapi(property=True,name=UpdateManager.onupdate,since=0.4) Set the update handler implementation function that will be invoked when an update is detected
	 */
	Titanium.API.set("UpdateManager.onupdate", function(f)
	{
		update_handler = f;
	});
	
	/**
	 * @tiapi(method=True,name=UpdateManager.startMonitor,since=0.4) Check the update service for a new version
	 * @tiarg(for=UpdateManager.startMonitor,name=component,type=string) Name of the component
	 * @tiarg(for=UpdateManager.startMonitor,name=callback,type=function) Function callback to call when completed
	 * @tiarg(for=UpdateManager.startMonitor,name=interval,type=int) Interval in milliseconds for how often to check for an update
	 * @tiresult(for=UpdateManager.startMonitor,type=int) Returns a handle which should use used to cancel the monitor
	 */
	Titanium.API.set("UpdateManager.startMonitor", function(component,callback,interval)
	{
		if (interval == undefined || interval == null || interval < (60000) * 5)
		{
			interval = 60000*5;	//default is 5 minutes
		}
		
		// schedule the timer to fire
		var timer = window.setInterval(function()
		{
			// perform the check
			updateCheck(component,null,callback);	
		});
		return timer;
	});

	/**
	 * @tiapi(method=True,name=UpdateManager.cancelMonitor,since=0.4) Check the update service for a new version
	 * @tiarg(for=UpdateManager.cancelMonitor,name=id,type=int) The monitor id returned from startMonitor
	 */
	Titanium.API.set("UpdateManager.cancelMonitor", function(id)
	{
		window.clearInterval(id);
	});
	
	// NOTE: this is a private api and not documented
	Titanium.API.set("UpdateManager.install", function(name,version)
	{
		var url = Titanium.App.getComponentUpdateURL(name,version);
		var xhr = Titanium.Network.createHTTPClient();
		var tmpfile = Titanium.Filesystem.createTempFile();
		var filestream = Titanium.Filesystem.createFileStream(tmpfile);
		filestream.open(filestream.MODE_WRITE,true,false);
		xhr.onchange = function()
		{
			filestream.close();
			if (name == 'runtime')
			{
			}
			else if (name == 'sdk')
			{
			}
			else if (name == 'mobilesdk')
			{
			}
			else
			{
				// module
			}
			// unzip it to the correct location

			// if (type == KrollUtils::MODULE) 
			// {
			// 	destDir = [NSString stringWithFormat:@"%@/modules/osx/%@/%@", installDirectory, name, version];
			// }
			// else if (type == KrollUtils::RUNTIME) 
			// {
			// 	destDir = [NSString stringWithFormat:@"%@/runtime/osx/%@", installDirectory, version];
			// }
			// else if (type == KrollUtils::SDK || type == KrollUtils::MOBILESDK)
			// {
			// 	destDir = installDirectory;
			// }
			// else if (type == KrollUtils::APP_UPDATE)
			// {
			// 	destDir = [NSString stringWithUTF8String:app->path.c_str()];
			// }

		};
		xhr.ondatastream = function(count,size,buffer,buffer_length)
		{
			filestream.write(buffer);
		};
		xhr.open('POST',url,true);
		xhr.send(null);
	});
	

	function updateCheck(component,version,callback)
	{
		try
		{
			var url = Titanium.App.getStreamURL("release-list");
			var xhr = Titanium.Network.createHTTPClient();
			xhr.setRequestHeader('Content-Type','application/x-www-form-urlencoded');
			var qs = 'version='+Titanium.Network.encodeURIComponent(version)+'&name='+Titanium.Network.encodeURIComponent(component)+'&mid='+Titanium.Network.encodeURIComponent(Titanium.Platform.id)+'&limit=1&guid='+Titanium.Network.encodeURIComponent(Titanium.App.getGUID());
			xhr.onreadystatechange = function()
			{
				if (this.readyState==4)
				{
					try
					{
						var json = window.Titanium.JSON.parse(this.responseText);
						if (!json.success)
						{
							Titanium.API.error("Error response from update service: "+json.message);
							callback(false);
							return;
						}
						if (json.releases.length > 0)
						{
							// we might have an update
							// compare our version with the 
							// remote version
							var update = json.releases[0];
							callback(true,update);
						}
						else
						{
							callback(false);
						}
					}
					catch(e)
					{
						Titanium.API.error("Exception communicating to update service: "+e);
						callback(false);
					}
				}
			}
			xhr.open('POST',url,true);
			xhr.send(qs);
		}
		catch(e)
		{
			Titanium.API.error("Error performing update check = "+e);
			callback(false);
		}
	}
	
	function insertUpdateTimestamp(db,initial)
	{
		try
		{
			if (initial)
			{
				db.execute("insert into last_check values(strftime('%s','now'))")
			}
			else
			{
				db.execute("update last_check set time = strftime('%s','now')")
			}
		}
		catch(e)
		{
			Titanium.API.error("Error updating update db = "+e);
		}
	}
	function updateDetected(updateSpec)
	{
		// if we have a handler, delegate to that dude
		// and he's now responsible for doing the update stuff
		if (typeof update_handler == 'function')
		{
			update_handler(updateSpec);
			return;
		}
		
		//TODO: fix release notes
		
		// ok, we'll handle it then...
		window.Titanium.UI.showDialog({
			'url': 'ti://tianalytics/update.html',
			'width': 450,  //600,
			'height': 170,  //350,
			'resizable':false,
			'parameters':{
				'name':Titanium.App.getName(),
				'icon':Titanium.App.getIcon(),
				'ver_from':Titanium.App.getVersion(),
				'ver_to':updateSpec.version,
				'notes_url':null
			},
			'onclose':function(result)
			{
				if (result == 'install')
				{
					// write our the new manifest for the update
					var datadir = Titanium.Filesystem.getApplicationDataDirectory();
					var update = Titanium.Filesystem.getFile(datadir,'.update');
					update.write(updateSpec.manifest);
					
					// restart ourselves to cause the install
					Titanium.Process.restart();
				}
			}
		});
	}
	function isUpdateRequired(newVersion, oldVersion)
	{
		return true;//FIXME
		var a = newVersion.split('.');
		var b = oldVersion.split('.');
		var c = 0;
		while ( c < a.length )
		{
			var foo = parseInt(a[c]);
			if (b.length < c) return true;
			var bar = parseInt(b[c]);
			if (foo > bar) return true;
			c++;
		}
		return false;
	}
	function sendUpdateCheck()
	{
		updateCheck('app-update',Titanium.App.getVersion(),function(success,update)
		{
			if (success && isUpdateRequired(update.version,Titanium.App.getVersion()))
			{
				updateDetected(update);
			}
		});
	}
	function checkForUpdate()
	{
		var db = Titanium.Database.open("app_updates");
		var initial = false;
		db.execute("create table if not exists last_check(time long)");
		try
		{
			var rs = db.execute("select strftime('%s','now')-time from last_check");
			var duration = rs.field(0);
			rs.close();
			if (duration == null)
			{
				initial = true;
			}
			if (!duration || duration >= update_check_interval_secs)
			{
				// time to perform check
				sendUpdateCheck();
			}
		}
		catch(e)
		{
			Titanium.API.error("Error in ti.Analytics checkForUpdate. Error="+e);	
		}
		insertUpdateTimestamp(db,initial);
		db.close();
	}
	
	Titanium.API.register("ti.UI.stop",function(name)
	{
		window = null;
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
			
			// map in our window scope
			window = event.scope;
			
			guid = Titanium.App.getGUID();
			sid = Titanium.Platform.createUUID();
			
			send({'event':'ti.start'});
			
			// schedule the update check
			window.setTimeout(function(){
				checkForUpdate();
			},update_check_delay);
		}
		catch(e)
		{
			// never never never die in this function
			Titanium.API.error("Error: "+e);
		}
	});
})();
