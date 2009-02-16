//
// load projects when manage projects is clicked
//
var TFS = Titanium.Filesystem;
var TiDeveloper  = {};
TiDeveloper.currentPage = 1;
TiDeveloper.init = false;
TiDeveloper.ytAuthToken = null;

// YOU TUBE STUFF - COME BACK TO LATER
//TiDeveloper.yk ='AI39si6YKvME9BMIerJYsJEAmM46e829Ovs_LMaR_jW1u0-4xHN7ehjW49EUqNgnZaNoK3aY_fUhs62mTw_0_x4EQ3f7uaaW6w';
//TiDeveloper.yc ='ytapi-Appcelerator-TitaniumDevelope-utomjlgd-0';
//
// YouTube Setup
//
// window.onYouTubePlayerReady = function(playerId)
// {
// 	ytplayer = document.getElementById("myytplayer");
// 	$('#ytapiplayer').css('display','none')
// 
// };
// var params = { allowScriptAccess: "always" };
// var atts = { id: "myytplayer" };
// swfobject.embedSWF("http://www.youtube.com/apiplayer?enablejsapi=1&amp;playerapiid=ytplayer", 
//                  "ytapiplayer", "400px", "300px", "8", null, null, params, atts);
// 
// //
// // Authenticate youtube user
// // 
// $.ajax({
// 	type: "POST",
//    	url: "https://www.google.com/youtube/accounts/ClientLogin",
//    	data: "Email=titaniumdev&Passwd=timetrac&service=youtube&source=titanium_developer",
//    	success: function(resp)
// 	{
//      	TiDeveloper.ytAuthToken = resp;
// 		TiDeveloper.loadVids()
//    	},
//    	error: function(resp)
// 	{
// 		if (resp.responseText == 'BadAuthentication')
// 		{
// 			// login failed
// 		}
// 	}
// });

// TiDeveloper.loadVids = function()
// {
// 	$.ajax({
// 		type:"GET",
// 		url:"http://gdata.youtube.com/feeds/api/videos/-/music", 
// 		data:'alt=json',
// 		success: function(data)
// 		{
// 			
// 			var videoData = eval("(" + data + ")");
// 			var count = 0;
// 			$.each(videoData.feed.entry,function() // Protoplasim
// 			{
// 				var html = "<div class='social_row' id='video_"+count+"' videoid='"+this.link[0].href.substr(this.link[0].href.lastIndexOf("=") + 1, this.link[0].href.length)+"'>"
// 				html += "<div style='text-align:center'><img src='"+this.media$group.media$thumbnail[0].url+"' /></div>";
// 				html += "<div style='font-size:11px;color;#fff;text-align:center;margin-top:10px'>";
// 				html +=  this.media$group.media$title.$t
// 				html += "</div>";
// 
// 				html += "</div>";
// 				
// 				$('#youtube_feed').append(html);
// 				$('#video_'+ count).click(function()
// 				{
// 					var id = $(this).attr('videoid')
// 					if(ytplayer)
// 					{
// 						ytplayer.loadVideoById(id, 0);
// 					}
// 				})
// 
// 				count++
// 				// html += "<div style='float:left;width:50%;margin-left:10px'>"+ this.media$group.media$title.$t + "</span></div>";
// 				// 
// 				// var href = this.link[0].href;
// 				// var title = '';
// 				// var desc = this.media$group.media$description.$t;
// 				// var thumbnail = this.media$group.media$thumbnail[2].url;
// 			});
// 		},
// 		error: function(resp)
// 		{
// 			alert(resp);
// 		}
// 	});
// 	
// }
		
// holder var for all projects
TiDeveloper.ProjectArray = [];
var db = openDatabase("TiDeveloper","1.0");
var highestId = 0;

// generic count format function
function formatCountMessage(count,things)
{
	return (count == 0) ? 'You have no '+things+'s' : count == 1 ? 'You have 1 '+things : 'You have ' + count + ' '+things+'s';
}

// State Machine for UI tab state
TiDeveloper.stateMachine = new App.StateMachine('ui_state');
TiDeveloper.stateMachine.addState('manage','l:menu[val=manage]',false);
TiDeveloper.stateMachine.addState('create','l:menu[val=create]',false);
TiDeveloper.stateMachine.addState('api','l:menu[val=api]',false);
TiDeveloper.stateMachine.addState('interact','l:menu[val=interact]',true);
TiDeveloper.currentState = null;
TiDeveloper.stateMachine.addListener(function()
{
	TiDeveloper.currentState = this.getActiveState();
	if (TiDeveloper.currentState != 'interact')
	{
		TiDeveloper.startIRCTrack();
	}
	else
	{
		TiDeveloper.stopIRCTrack();
		
	}
});
TiDeveloper.ircMessageCount = 0;
TiDeveloper.startIRCTrack = function()
{
	TiDeveloper.ircMessageCount = 0;
	$('#irc_message_count').html('');
	$('#irc_message_count').css('display','inline');
	
};
TiDeveloper.stopIRCTrack = function()
{
	TiDeveloper.ircMessageCount = 0;
	$('#irc_message_count').css('display','none');
	$('#irc_message_count').html('');
	
};
TiDeveloper.updateAppData = function()
{
	// write manifest
	var values = {};
	values.name = $('#project_name_value').html();
	values.publisher = $('#project_pub_value').html();
	values.dir = $('#project_dir_value').html();
	values.image = $('#project_pub_image_value').html();
	values.url = $('#project_pub_url_value').html();
	Titanium.Project.updateManifest(values)

	var id = $('#project_id_value').get(0).value;

	// update database
    db.transaction(function (tx) 
    {
        tx.executeSql("UPDATE Projects set name = ?, directory = ?, publisher = ?, url = ?, image = ? WHERE id = ?", 
		[values.name,values.dir,values.publisher,values.url,values.image, id]);
    });
	
	var project = findProjectById(id)
	project.name = values.name
	project.dir = values.dir
	project.publisher = values.publisher
	project.url = values.url
	project.image = values.image
	
};

$MQL('l:row.selected',function(msg)
{
	var msgObj = {}
	var project = findProjectById(msg.payload.project_id);
	msgObj.date = project.date;
	msgObj.name = project.name;
	msgObj.location = TiDeveloper.formatDirectory(project.dir);
	msgObj.fullLocation = project.dir;
	msgObj.pub = project.publisher
	msgObj.url = project.url;
	msgObj.image = project.image;
	$MQ('l:project.detail.data',msgObj)
	
	// setup editable fields
	$('.edit').click(function()
	{
		if ($(this).attr('edit_mode') != 'true')
		{
			// only one active edit field at a time
			var activeFiles = $('div[edit_mode=true]');
			if (activeFiles)
			{
				for (var i=0;i<activeFiles.length;i++)
				{
					var id = $(activeFiles[i]).attr('id');
					$(activeFiles[i]).html($('#'+id+'_input').val())
					TiDeveloper.updateAppData();
					$(activeFiles[i]).get(0).removeAttribute('edit_mode');
				}
			}
			
			// process click
			var el = $(this).get(0);
			var value = el.innerHTML;
			el.setAttribute('edit_mode','true');
			
			// create input and focus
			$(this).html('<input id="'+el.id+'_input" value="'+value+'" type="text" style="width:350px"/>');
			$('#'+el.id+'_input').focus();
			
			// listen for enter
			$('#'+el.id+'_input').keyup(function(e)
			{
				if (e.keyCode==13)
				{
					el.innerHTML = $('#'+el.id+'_input').val() 
					el.removeAttribute('edit_mode');
					TiDeveloper.updateAppData();
				}
				else if (e.keyCode==27)
				{
					el.innerHTML = value; 
					el.removeAttribute('edit_mode');
				}
			});

		}
	})
	
})

function createRecord(options,callback)
{
	var date = new Date();
	var dateStr = (date.getMonth()+1)+"/"+date.getDate()+"/"+date.getFullYear();
	var record = {
		name: options.name,
		dir: String(options.dir),
		id: ++highestId,
		appid: options.appid,
		date: dateStr,
		publisher:options.publisher,
		url:options.url,
		image:options.image
	};
    db.transaction(function (tx) 
    {
        tx.executeSql("INSERT INTO Projects (id, timestamp, name, directory, appid, publisher, url, image) VALUES (?, ?, ?, ?, ?, ?, ?, ?)", [record.id,date.getTime(),record.name,record.dir,record.appid,record.publisher,record.url,record.image]);
    },
	function(error)
	{
		callback({code:1,id:error.id,msg:error.message})
	},
	function()
	{
		TiDeveloper.ProjectArray.push(record);
		callback({code:0})
	});
}

function loadProjects()
{

	db.transaction(function(tx) 
	{
        tx.executeSql("SELECT id, timestamp, name, directory, appid, publisher, url, image FROM Projects order by timestamp", [], function(tx, result) 
		{
			TiDeveloper.ProjectArray = [];
            for (var i = 0; i < result.rows.length; ++i) {
                var row = result.rows.item(i);
				// check to see if the user has deleted it and if
				// so remove it
				var cd = TFS.getFile(row['directory']);
				if (!cd.exists())
				{
					tx.executeSql('DELETE FROM Projects where id = ?',[row['id']]);
				}
				else
				{
					var date = new Date();
					date.setTime(row['timestamp'])
					
					TiDeveloper.ProjectArray.push({
						id: row['id'],
						date: (date.getMonth()+1)+"/"+date.getDate()+"/"+date.getFullYear(),
						name: row['name'],
						dir: row['directory'],
						appid: row['appid'],
						publisher: row['publisher'],
						url: row['url'],
						image: row['image']
					});
					if (highestId < row['id'])
					{
						highestId = row['id'];
					}
				}
            }
			TiDeveloper.currentPage = 1;
			var data = TiDeveloper.getProjectPage(10,TiDeveloper.currentPage);
			var count = formatCountMessage(TiDeveloper.ProjectArray.length,'project');
			$('#project_count_hidden').val(TiDeveloper.ProjectArray.length)
			$MQ('l:project.list.response',{count:count,page:TiDeveloper.currentPage,totalRecords:TiDeveloper.ProjectArray.length,'rows':data})
        }, function(tx, error) {
            alert('Failed to retrieve projects from database - ' + error.message);
            return;
        });
	});	
}


//
//  Initial project load
//
$MQL('l:app.compiled',function()
{
	db.transaction(function(tx) 
	{
	   tx.executeSql("SELECT COUNT(*) FROM Projects", [], function(result) 
	   {
	       loadProjects();
	   }, function(tx, error) 
	   {
	       tx.executeSql("CREATE TABLE Projects (id REAL UNIQUE, timestamp REAL, name TEXT, directory TEXT, appid TEXT, publisher TEXT, url TEXT, image TEXT)", [], function(result) 
		   { 
	          loadProjects(); 
	       });
	   });
	});
	
});


//
//  create.project service
//
$MQL('l:create.project.request',function(msg)
{
	try
	{
		var result = Titanium.Project.create(msg.payload.project_name,msg.payload.project_location,msg.payload.publisher,msg.payload.url,msg.payload.image);
		if (result.success)
		{
			var options = {name:result.name, dir:result.basedir,appid:result.id,publisher:msg.payload.publisher,url:msg.payload.url,image:msg.payload.image}
			var r = createRecord(options,function(obj)
			{
				if (obj.code == 0)
				{
					$MQ('l:create.project.response',{result:'success'});
					var count = formatCountMessage(TiDeveloper.ProjectArray.length,'project');
					$MQ('l:project.list.response',{count:count,page:1,totalRecords:TiDeveloper.ProjectArray.length,'rows':TiDeveloper.ProjectArray})
				}
				else
				{
					$MQ('l:create.project.response',{result:'error',msg:obj.msg});
				}
			});		
		}
		else
		{
			$MQ('l:create.project.response',{result:'error',msg:result.message});
		}
	}
	catch(E)
	{
		$MQ('l:create.project.response',{result:'error',msg:E});
	}
});

//
// Handling paging requests
//
$MQL('l:page.data.request',function(msg)
{
	// paging gets called in both search and list
	// cases - if search yields 0 results, do nothing
	if (TiDeveloper.ProjectArray.length == 0)return;
	
	var state =msg.payload
	var rowsPerPage = state.rowsPerPage
	var page = state.page
	TiDeveloper.currentPage = page;
	var data = TiDeveloper.getProjectPage(rowsPerPage,page);
	var count = formatCountMessage(TiDeveloper.ProjectArray.length,'project');
	
	$MQ('l:project.list.response',{count:count,page:page,totalRecords:TiDeveloper.ProjectArray.length,'rows':data})
});


TiDeveloper.formatDirectory =function(dir,truncate)
{
	// return whole dir
	if (truncate == false)return dir;
	
	if (dir != null)
	{
		var dirStr = dir
		if (dir.length > 40)
		{
			dirStr = dir.substring(0,40) + '...';
			$('#project_detail_dir_a').css('display','block');
			$('#project_detail_dir_span').css('display','none');
		}
		else
		{
			$('#project_detail_dir_span').css('display','block');
			$('#project_detail_dir_a').css('display','none');
		}
	}
	return dirStr;
}
//
// Get a page of data
//
TiDeveloper.getProjectPage = function(pageSize,page)
{
	var pageData = [];
	var start = (page==0)?0:(pageSize * page) - pageSize;
	var end = ((pageSize * page) > TiDeveloper.ProjectArray.length)?TiDeveloper.ProjectArray.length:(pageSize * page);
	for (var i=start;i<end;i++)
	{
		pageData.push(TiDeveloper.ProjectArray[i])
	}
	return pageData
};

var modules = [];
var module_map = {};

setTimeout(function()
{
	var result = Titanium.Project.getModulesAndRuntime();
	modules.push({name:'Titanium Runtime',versions:result.runtime.versions,dir:result.runtime.dir});
	for (var c=0;c<result.modules.length;c++)
	{
		var name = result.modules[c].name;
		module_map[name]=result.modules[c];
		modules.push({name:name,versions:result.modules[c].versions,dir:result.modules[c].dir});
	
	}
},500);

//
//  Project Package Request - get details about modules, etc
//
$MQL('l:package.project.request',function(msg)
{
	try
	{
		$MQ('l:package.project.data',{rows:modules});
	 	$MQ('l:package.all',{val:'network'});
	}
	catch (E)
	{
		alert("Exception = "+E);
	}
});

function findProject(name)
{
	for (var i=0;i<TiDeveloper.ProjectArray.length;i++)
	{
		if (TiDeveloper.ProjectArray[i].name == name)
		{
			return TiDeveloper.ProjectArray[i];
		}
	}
	return null;
}

function findProjectById(id)
{
	for (var i=0;i<TiDeveloper.ProjectArray.length;i++)
	{
		if (TiDeveloper.ProjectArray[i].id == id)
		{
			return TiDeveloper.ProjectArray[i];
		}
	}
	return null;
}
//
// Create Package Request
//
$MQL('l:create.package.request',function(msg)
{
	try
	{
		// elements that are included for network bundle
		var networkEl = $("div[state='network']");

		// elements that are included (bundled)
		var bundledEl = $("div[state='bundled']");

		// elements that are excluded
		var excludedEl = $("div[state='exclude']");
		
		var excluded = {};
		
		$.each(excludedEl,function()
		{
			var key = $.trim($(this).html());
			excluded[key]=true;
		});
		
		// project name
		var project_name = $('#package_project_name').html();

		var launch = (msg.payload.launch == 'no')?false:true;
		var install = typeof(msg.payload.install)=='undefined' ? false : msg.payload.install;

		var project = findProject(project_name);
		var resources = TFS.getFile(project.dir,'resources');

		// build the manifest
		var manifest = '#appname:'+project_name+'\n';
		manifest+='#appid:'+project.appid+'\n';
		manifest+='#publisher:'+project.publisher+'\n';

		if (project.image)
		{
			var image = TFS.getFile(project.image);
			var image_dest = TFS.getFile(resources,image.name());
			image.copy(image_dest);
			manifest+='#image:'+image.name()+'\n';
		}
		
		manifest+='#url:'+project.url+'\n';
		manifest+='runtime:'+modules[0].versions[0]+'\n';
		
		// 0 is always runtime, skip it
		for (var c=1;c<modules.length;c++)
		{
			if (!excluded[modules[c].name])
			{
				manifest+=modules[c].name+':'+modules[c].versions[0]+'\n';
			}
		}
		
		var mf = TFS.getFile(project.dir,'manifest');
		mf.write(manifest);
		
		var dist = TFS.getFile(project.dir,'dist',Titanium.platform);
		dist.createDirectory(true);
		
		var runtime = TFS.getFile(modules[0].dir,modules[0].versions[0]);
		
		var app = Titanium.createApp(runtime,dist,project_name,project.appid,install);
		var app_manifest = TFS.getFile(app.base,'manifest');
		app_manifest.write(manifest);
		var resources = TFS.getFile(project.dir,'resources');
		var tiapp = TFS.getFile(project.dir,'tiapp.xml');
		tiapp.copy(app.base);
		var launch_fn = function()
		{
			if (launch)
			{
				Titanium.Desktop.openApplication(app.executable.nativePath());
			}
		};
		TFS.asyncCopy(resources,app.resources,function()
		{
			var module_dir = TFS.getFile(app.base,'modules');
			var runtime_dir = TFS.getFile(app.base,'runtime');
			var modules_to_bundle = [];
			$.each(bundledEl,function()
			{
				var key = $.trim($(this).html());
				var target, dest;
				if (key == 'Titanium Runtime') //TODO: we need to make this defined
				{
					runtime_dir.createDirectory();
					target = TFS.getFile(modules[0].dir,modules[0].versions[0]);
					dest = runtime_dir;
				}
				else
				{
					module_dir.createDirectory();
					var module = module_map[key];
					target = TFS.getFile(module.dir,module.versions[0]);
					dest = TFS.getFile(module_dir,module.dir.name());
				}
				modules_to_bundle.push({target:target,dest:dest});
			});
			
			if (modules_to_bundle.length > 0)
			{
				var count = 0;
				for (var c=0;c<modules_to_bundle.length;c++)
				{
					var e = modules_to_bundle[c];
					TFS.asyncCopy(e.target,e.dest,function(filename,c,total)
					{
						if (++count==modules_to_bundle.length)
						{
							// link libraries if runtime included
							if (e.dest == runtime_dir)
							{
								Titanium.linkLibraries(e.dest);
							}
							launch_fn();
						}
					});
				}
			}
			else
			{
				// no modules to bundle, installer the net installer
				var net_installer_src = TFS.getFile(runtime,'installer');
				var net_installer_dest = TFS.getFile(app.base,'installer');
				TFS.asyncCopy(net_installer_src,net_installer_dest,function(filename,c,total)
				{
					launch_fn();
				});
			}
			
		});
	}
	catch(E)
	{
		alert("Exception = "+E);
	}
})

//
//  Delete a project
//	
$MQL('l:delete.project.request',function(msg)
{
	var name = msg.payload.name;
	var id = msg.payload.project_id
	var project = findProjectById(id);
	var file = Titanium.Filesystem.getFile(project.dir);
	alert(id + ' ' + project + ' ' + file)

	file.deleteDirectory(true);
	
	db.transaction(function (tx) 
    {
        tx.executeSql("DELETE FROM Projects where id = ?", [id]);
		loadProjects();
    });
});

//
// project search request
//
$MQL('l:project.search.request',function(msg)
{
	var q = msg.payload.search_value;
	db.transaction(function(tx) 
	{
		try
		{
	        tx.executeSql("SELECT id, timestamp, appid, publisher, url, image, name, directory FROM Projects where name LIKE '%' || ? || '%'", [q], function(tx, result) 
			{
				try
				{
					TiDeveloper.ProjectArray = [];
		            for (var i = 0; i < result.rows.length; ++i) 
					{
		                var row = result.rows.item(i);
						var date = new Date();
						date.setTime(row['timestamp'])
						TiDeveloper.ProjectArray.push({
							id: row['id'],
							date: (date.getMonth()+1)+"/"+date.getDate()+"/"+date.getFullYear(),
							name: row['name'],
							dir: row['directory'],
							appid: row['appid'],
							publisher: row['publisher'],
							url: row['url'],
							image: row['image']
						});
					}
					$MQ('l:project.search.response',{count:TiDeveloper.ProjectArray.length,page:1,totalRecords:TiDeveloper.ProjectArray.length,'rows':TiDeveloper.ProjectArray});
				}
				catch (EX)
				{
					alert("EXCEPTION = "+EX);
				}
			});
		}
		catch (E)
		{
			alert("E="+e);
		}
	});
});

//
// Show file dialog and send value
//
$MQL('l:show.filedialog',function(msg)
{
	var el = msg.payload['for'];
	var props = {multiple:false};
	if (el == 'project_image')
	{
		props.directories = false;
		props.files = true;
		props.types = ['gif','png','jpg'];
	}
	else
	{
		props.directories = true;
		props.files = false;
	}
	
	Titanium.UI.openFiles(function(f)
	{
		if (f.length)
		{
			$MQ('l:file.selected',{'for':el,'value':f[0]});
		}
	},
	props);
});

TiDeveloper.getCurrentTime = function()
{
	var curDateTime = new Date()
  	var curHour = curDateTime.getHours()
  	var curMin = curDateTime.getMinutes()
  	var curAMPM = "am"
  	var curTime = ""
  	if (curHour >= 12){
    	curHour -= 12
    	curAMPM = "pm"
    }
  	if (curHour == 0) curHour = 12
  	curTime = curHour + ":" + ((curMin < 10) ? "0" : "") + curMin + curAMPM;
  	return curTime;
	
};
var irc_count = 0;

setTimeout(function()
{
	try
	{
		var myNick = Titanium.Platform.username+'1';
		var myName = Titanium.Platform.username+'1';
		var myNameStr = Titanium.Platform.username+'1';

		$('#irc').append('<div style="color:#aaa">you are joining the <span style="color:#42C0FB">Titanium Developer</span> chat room. one moment...</div>');
		
		var irc = Titanium.Network.createIRCClient();
		irc.connect("irc.freenode.net",6667,myNick,myName,myNameStr,String(new Date().getTime()),function(cmd,channel,data,nick)
		{
			
			var time = TiDeveloper.getCurrentTime();

			// switch on command
			switch(cmd)
			{
				case 'NOTICE':
				case 'PRIVMSG':
				{
					if (nick && nick!='NickServ')
					{
						if (TiDeveloper.currentState != 'interact') TiDeveloper.ircMessageCount ++;
						$('#irc_message_count').html(TiDeveloper.ircMessageCount);						
						var rawMsg = String(channel.substring(1,channel.length));
						var urlMsg = TiDeveloper.formatURIs(rawMsg);
						var str = myNick + ":";
						var msg = urlMsg.replace(myNick +":","<span style='color:#42C0FB'>" + myNick + ": </span>");
						$('#irc').append('<div style="color:yellow;float:left">' + nick + ': <span style="color:white">' + msg + '</span></div><div style="float:right;color:#ccc;font-size:11px">'+time+'</div><div style="clear:both"></div>');
					}
					break;
				}
				case '366':
				{					
					var users = irc.getUsers('#titanium_dev');
					$MQ('l:online.count',{count:users.length});
					irc_count = users.length;
					for (var i=0;i<users.length;i++)
					{
						if (users[i].operator == true)
						{
							$('#irc_users').append('<div class="'+users[i].name+'" style="color:#42C0FB">'+users[i].name+'(op)</div>');
						}
						else if (users[i].voice==true)
						{
							$('#irc_users').append('<div class="'+users[i].name+'" style="color:#42C0FB">'+users[i].name+'(v)</div>');
						}
						else
						{
							$('#irc_users').append('<div class="'+users[i].name+'">'+users[i].name+'</div>');
						}
					}
				}
				case 'JOIN':
				{
					if (nick.indexOf('freenode.net') != -1)
					{
						continue;
					}
					
					if (nick == myNick)
					{
						$('#irc').append('<div style="color:#aaa"> you are now in the room. your handle is: <span style="color:#42C0FB">'+myNick+'</span> </div>');
						break
					}
					else
					{
						irc_count++;
					}
					$('#irc').append('<div style="color:#aaa">' + nick + ' has joined the room </div>');
					$('#irc_users').append('<div class="'+nick+'" style="color:#457db3">'+nick+'</div>');
					$MQ('l:online.count',{count:irc_count});
					break;
					
				}
				case 'QUIT':
				case 'PART':
				{
					$('#irc').append('<div style="color:#aaa">' + nick + ' has left the room </div>');
					$('.'+nick).html('');
					irc_count--;
					$MQ('l:online.count',{count:irc_count});
					break;
				}
			}
			$('#irc').get(0).scrollTop = $('#irc').get(0).scrollHeight;
		});

		irc.join("#titanium_dev");
		$MQL('l:send.irc.msg',function()
		{
			var time = TiDeveloper.getCurrentTime();
			irc.send('#titanium_dev',$('#irc_msg').val());
			$('#irc').append('<div style="color:yellow;float:left">' + myNick + ': <span style="color:white">' + $('#irc_msg').val() + '</span></div><div style="float:right;color:#ccc;font-size:11px">'+time+'</div><div style="clear:both"></div>');
			$('#irc_msg').val('');
			$('#irc').get(0).scrollTop = $('#irc').get(0).scrollHeight;

		})
	}
	catch(E)
	{
		alert("Exception: "+E);
	}
},1000);

TiDeveloper.formatURIs = function(str)
{
	var URI_REGEX = /((([hH][tT][tT][pP][sS]?|[fF][tT][pP])\:\/\/)?([\w\.\-]+(\:[\w\.\&%\$\-]+)*@)?((([^\s\(\)\<\>\\\"\.\[\]\,@;:]+)(\.[^\s\(\)\<\>\\\"\.\[\]\,@;:]+)*(\.[a-zA-Z]{2,4}))|((([01]?\d{1,2}|2[0-4]\d|25[0-5])\.){3}([01]?\d{1,2}|2[0-4]\d|25[0-5])))(\b\:(6553[0-5]|655[0-2]\d|65[0-4]\d{2}|6[0-4]\d{3}|[1-5]\d{4}|[1-9]\d{0,3}|0)\b)?((\/[^\/][\w\.\,\?\'\\\/\+&%\$#\=~_\-@]*)*[^\.\,\?\"\'\(\)\[\]!;<>{}\s\x7F-\xFF])?)/;
	
	return $.gsub(str,URI_REGEX,function(m)
	{
		return '<a target="ti:systembrowser" href="' + m[0] + '">' + m[0] + '</a>';
	})
	
};

$.extend(
{
	gsub:function(source,pattern,replacement)
	{
		if (typeof(replacement)=='string')
		{
			var r = String(replacement);
			replacement = function()
			{
				return r;
			}
		}
	 	var result = '', match;
	    while (source.length > 0) {
	      if (match = source.match(pattern)) {
			result += source.slice(0, match.index);
	        result += String(replacement(match));
	        source  = source.slice(match.index + match[0].length);
	      } else {
	        result += source, source = '';
	      }
	    }
		return result;
	}
});