///**
// * Titanium Application Update module
// */
//
//Titanium.App.UpdateResult = function(update_available,version,date,release_notes_url)
//{
//	this.update_available = update_available;
//	this.version = version;
//	this.date = date;
//	this.release_notes_url = release_notes_url;
//};
//
//Titanium.App.UpdateResult.prototype.updateAvailable = function()
//{
//	return this.update_available;
//}
//
//Titanium.App.UpdateResult.prototype.install = function()
//{
//};
//
//
////
//// 1. check to see if this is the first install, if so, initialize DB
////
//// 2. upon not first time, read from DB the latest update and schedule
////    timer to check for update
////
//// 3. if update found, prompt user with update box
////
//// 4. if user chooses to update, launch the network install with
////    the manifest (or restart, etc to cause install)
////
//// NOTES: for each app, need to store update preferences like don't update
////
//
//
//(function()
//{
//	var debug = false;
//	var delay_before_check = 20000;
//	var update_check_interval = 1440000;
//	var timeout = 10000;
//	var initialized = false;
//	var in_update_check = false;
//	var update_callbacks;
//	
//	Titanium.App.Update = 
//	{
//		/**
//		 * @tiapi(method=True,name=App.Update.check,since=0.4) perform an update check
//		 * @tiarg(for=App.Update.check,type=object,name=window) window object
//		 * @tiarg(for=App.Update.check,type=function,name=callback) function that is called upon completion of check with the result
//		 */
//		check: function(window,callback)
//		{
//			if (Titanium.Network.online===false)
//			{
//				return false;
//			}
//		
//			if (in_update_check)
//			{
//				update_callbacks.push(callback);
//				return;
//			}
//			update_callbacks = [callback];
//			in_update_check = true;
//			
//			var qsv = {};
//			qsv.mid = Titanium.Platform.id;			
//			qsv.guid = Titanium.App.getGUID();
//			qsv.version = Titanium.App.getVersion();
//			
//			var qs = '';
//			for (var p in qsv)
//			{
//				var v = typeof(qsv[p])=='undefined' ? '' : String(qsv[p]);
//				qs+=p+'='+Titanium.Network.encodeURIComponent(v)+'&';
//			}
//
//			var url = Titanium.App.getStreamURL("app-update-check");
//			
//			// connect and run the check
//			var xhr = Titanium.Network.createHTTPClient();
//			if (timeout > 0)
//			{
//				xhr.setTimeout(timeout);
//			}
//			xhr.setRequestHeader('Content-Type','application/x-www-form-urlencoded');
//			xhr.onreadystatechange = function()
//			{
//				if (this.readyState==4)
//				{
//					Titanium.API.debug("++ received:"+this.responseText);
//					var json = window.eval('('+this.responseText+')');
//					
//					// update the database after the check to record when
//					// we last checked for the update
//					var db = Titanium.Database.openDatabase("app_updates");
//					db.execute("delete from check");
//					db.execute("insert into check values(?)",new Date().getTime());
//					db.close();
//					
//					// now check to see if we have an update ...
//					// initiate the callback
//					var response = new Titanium.App.UpdateResult(json.update_available,json.version,json.release_date,json.release_notes_url);
//					
//					// invoke callbacks
//					for (var c=0;c<update_callbacks.length;c++)
//					{
//						try
//						{
//							var callback = update_callbacks[c];
//							callback(response);
//						}
//						catch(ex)
//						{
//							Titanium.API.error("Exception making update callback:"+ex);
//						}
//					}
//					update_callbacks = null;
//				}
//			}
//			xhr.open('POST',url,true);
//			xhr.send(qs);
//		},
//		initializeDB: function(db)
//		{
//			// create the table the first time through
//			db.execute("create table check (timestamp INTEGER)");
//		},
//		setup: function()
//		{
//			var db = Titanium.Database.openDatabase("app_updates");
//			try
//			{
//				var rs = db.execute("select timestamp from check");
//				if (rs.isValidRow())
//				{
//					var timestamp = rs.field(0);
//					var age_since_last_check = (new Date().getTime()) - timestamp;
//					if (age_since_last_check < update_check_interval)
//					{
//						// skip the check, we've checked recently
//						return false;
//					}
//				}
//				// if now rows, fall through
//			}
//			catch(e)
//			{
//				this.initializeDB(db);
//				// this means we're in setup and we want to skip
//				// this update check on the first run
//				return false;
//			}
//			db.close();
//			return true;
//		}
//	};
//	
//	Titanium.API.register("ti.UI.window.page.init",function(name,event)
//	{
//		try
//		{
//			// we only want to run this once, per app
//			if (initialized===true)
//			{
//				return;
//			}
//
//			initialized = true;
//			
//			// schedule timer
//			event.scope.setTimeout(function()
//			{
//				if (setup())
//				{
//					Titanium.App.Update.check(event.scope,function(response)
//					{
//						if (response.updateAvailable())
//						{
//							//TODO: show the update UI dialog
//							
//							// if the user wants to update, call install
//							response.install();
//						}
//					});
//				}
//			},delay_before_check);
//		}
//		catch(e)
//		{
//			Titanium.API.error("Exception attempting application update check. Exception:"+e);
//		}	
//	});
//	
//})();
