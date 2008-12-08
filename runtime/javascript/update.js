//
// this is the main update application checker. it's job is to check the
// update service to determine if the application has been updated and
// if so, notify the user that an update is available
//
try
{
	// refuse to do update checks while we're in a notification window
	if (window.document.location.href.indexOf("ti://notification")==-1)
	{
		var threshold = (60000 * 60 * 6); // 6 hours minimum time between checks
		var db = new ti.Database;
		db.open("titanium");
		db.execute("create table if not exists UpdateCheck (timestamp int)");
		var rs = db.execute("select timestamp from UpdateCheck");
		var check = true;
		var found = false;
		if (rs.isValidRow())
		{
			var ts = rs.field(0);
			found = true;
			var now = new Date().getTime();
			var diff = now - ts;
			ti.App.debug("last time we checked for update in seconds: "+(diff/1000));
			if (diff < threshold)
			{
				check = false;
			}
		}
		rs.close();
		if (check)
		{
			if (found)
			{
				db.execute("update UpdateCheck set timestamp = ?",[new Date().getTime()]);
			}
			else
			{
				db.execute("insert into UpdateCheck values (?)",[new Date().getTime()]);
			}
			var random = 1000 + Math.round(9999*Math.random());
			// don't do it immediately when the app is coming up ... do it some 
			// random time between 1-10 seconds after we're running
			ti.App.debug("Update check will occur in "+(random/1000)+" seconds");
			setTimeout(function()
			{
				try
				{
					// we're going to do the update check in a gears HTTP thread instead of the main one
					var url = 'http://updatesite.titaniumapp.com/distribution/titanium';
					var payload = {
						aguid:ti.App.getGUID(),
						aid:ti.App.getID(),
						ver:ti.App.getVersion(),
						url:ti.App.getUpdateURL(),
						name:ti.App.getName(),
						tv:ti.version,
						os:ti.platform
					};
					$.ajax({
						type:'GET',
						url:url,
						async: true,
						cache: false,
						dataType: 'json',
						data:payload,
						success: function(data,statusText)
						{
							ti.App.debug("update server returned status: "+statusText+", success: "+data.success);
							if (data.success)
							{
								ti.App.debug("we have version: "+payload.ver+", current version: "+data.version);
								if (payload.ver != data.version)
								{
									// we have an update
									var notify = new ti.Notification;
									notify.setTitle(payload.name+" Update");
									var msg = "An update ("+data.version+") for "+payload.name+" has been found. ";
									msg+="<a href='"+data.url+"' target='ti:systembrowser'>Download update</a>";
									notify.setMessage(msg);
									notify.setDelay(8000);
									notify.show();
									//TODO: set application icon here
									//TODO: play a quick sound here
								}
							}
						},
						error: function(xhr,status,error)
						{
							ti.App.debug("exception returned from update server: "+status+", error: "+error);
						}
					});
				}
				catch (JQE)
				{
					ti.App.debug("update check error: "+JQE+", line: "+JQE.line);
				}
			},random);
		}
		db.close();
	}
}
catch(DE)
{
	ti.App.debug("Error loading: "+DE+" at line: "+DE.line);
}

