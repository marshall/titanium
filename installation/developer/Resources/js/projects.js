TiDeveloper.Projects = {};

TiDeveloper.Projects.stats_url = 'http://publisher.titaniumapp.com/api/app-stats';
TiDeveloper.Projects.publish_url = 'http://publisher.titaniumapp.com/api/publish';
TiDeveloper.Projects.publish_status_url = 'http://publisher.titaniumapp.com/api/publish-status';

TiDeveloper.Projects.projectArray = [];
TiDeveloper.Projects.modules = [];
TiDeveloper.Projects.module_map = {};
TiDeveloper.Projects.runtimeDir = null;
TiDeveloper.Projects.runtimeVersion = null;
TiDeveloper.Projects.requiredModuleMap = {};
TiDeveloper.Projects.requiredModules = ['api','tiapp','tifilesystem','tiplatform','tiui','javascript'];
TiDeveloper.Projects.selectedProject = null;
TiDeveloper.Projects.packagingInProgress = {};


//
//  Initialization message - setup all initial states
//
$MQL('l:app.compiled',function()
{
	// load or initialize project table
	db.transaction(function(tx) 
	{   
		
		// see if project table exists
	   	tx.executeSql("SELECT COUNT(*) FROM Projects", [], function(result) 
	   	{
			// see if we need to run db migration
	        tx.executeSql("SELECT name,completed FROM Migrations", [], function(tx, result) 
			{
				// nope, we're good
	          	TiDeveloper.Projects.loadProjects(true); 
			},
			// create record an run it
			function(tx, error)
			{
				// run migration
				tx.executeSql('CREATE TABLE Migrations (name TEXT, completed REAL)');
				tx.executeSql('INSERT INTO Migrations values("PR3",1)');
				TiDeveloper.Projects.runGUIDMigration();
			});

	   	}, function(tx, error) 
	   	{
	       tx.executeSql("CREATE TABLE Projects (id REAL UNIQUE, guid TEXT, description TEXT, timestamp REAL, name TEXT, directory TEXT, appid TEXT, publisher TEXT, url TEXT, image TEXT)", [], function(result) 
		   { 
	          TiDeveloper.Projects.loadProjects(true); 
	       });
	   });
	});
	
});

//
// Add GUID to all projects 
//
TiDeveloper.Projects.runGUIDMigration = function()
{
	// dump all data and generate GUIDs
	db.transaction(function(tx) 
	{
        tx.executeSql("SELECT id, timestamp, name, directory, appid, publisher, url, image FROM Projects order by timestamp", [], function(tx, result) 
		{
			var a = [];
            for (var i = 0; i < result.rows.length; ++i) {
                var row = result.rows.item(i);
				var guid = Titanium.Platform.createUUID();
				var projRow = {};
				projRow['timestamp'] = row['timestamp'];
				projRow['id'] = row['id']
				projRow['guid'] = guid;
				projRow['name'] = row['name'];
				projRow['directory'] = row['directory'];
				projRow['appid'] = row['appid'];
				projRow['publisher'] = row['publisher'];
				projRow['url'] = row['url'];
				projRow['image'] = row['image'];
				projRow['description'] = row['name'] + ' is a cool new app created by ' + row['publisher'];
				a.push(projRow);
			}
			if (a.length == 0)
			{
				TiDeveloper.Projects.loadProjects(true);
			}
			// delete and re-add rows
			tx.executeSql('DROP TABLE Projects',[],function(tx,result)
			{
				// re-create table
				tx.executeSql("CREATE TABLE Projects (id REAL UNIQUE, guid TEXT, description TEXT, timestamp REAL, name TEXT, directory TEXT, appid TEXT, publisher TEXT, url TEXT, image TEXT)",[],
					function()
					{
						// re-add rows
						var rowCount =0
						for (var i=0;i<a.length;i++)
						{
							tx.executeSql('INSERT into Projects (id,guid,description,timestamp,name,directory,appid,publisher,url,image) values (?,?,?,?,?,?,?,?,?,?)',
							[a[i]['id'], a[i]['guid'],a[i]['description'],a[i]['timestamp'],a[i]['name'],a[i]['directory'],a[i]['appid'],a[i]['publisher'],a[i]['url'],a[i]['image']],
							function()
							{
								rowCount++;
								// if we're done, reload projects
								if (rowCount == a.length)
								{
									TiDeveloper.Projects.loadProjects(true);
								}
							});
						}
					});

			})

        });
	});	
}

//
// Import a project
//
TiDeveloper.Projects.importProject = function(f)
{
	var dir = f;
	var file = TFS.getFile(dir,'manifest');
	if (file.exists() == false)
	{
		alert('This directory does not contain valid Titanium project.  Please try again.');
		return;
	}
	
	// create object for DB record
	var options = {};
	options.dir = dir;
	
	// read manifest values to create new db record
	var line = file.readLine(true);
	var entry = Titanium.Project.parseEntry(line);
	for (var i=0;i<1000;i++)
	{
		if (entry == null)
		{
			line = file.readLine();
			if (!line || line == null)break;
			entry = Titanium.Project.parseEntry(line);
		}
		if (entry.key.indexOf('appname') != -1)
		{
			options.name = entry.value;
		}
		else if (entry.key.indexOf('publisher') != -1)
		{
			options.publisher = entry.value;
		}
		else if (entry.key.indexOf('url') != -1)
		{
			options.url = entry.value;
		}
		else if (entry.key.indexOf('image') != -1)
		{
			options.image = entry.value;
		}
		else if (entry.key.indexOf('appid') != -1)
		{
			options.appid = entry.value;
		}
		else if (entry.key.indexOf('guid') != -1)
		{
			options.guid = entry.value;
		}
		else if (entry.key.indexOf('desc') != -1)
		{
			options.description = entry.desc;
		}

		entry = null;
	}
	
	if (!options.description)
	{
		options.description = options.name + ' is a cool new app created by ' + options.publisher;
	}
	// if no guid, create
	if (!options.guid)
	{
		options.guid = Titanium.Platform.createUUID();
	}
	
	TiDeveloper.Projects.createRecord(options,function(obj)
	{
		TiDeveloper.Projects.loadProjects();
	});
	
};
//
// Listener for Import project
//
$MQL('l:import.project',function()
{
	$MQ('l:show.filedialog',{callback:TiDeveloper.Projects.importProject});
})

//
// Update project manifest data - can be edited via developer UI
//
TiDeveloper.Projects.updateAppData = function()
{
	// write manifest
	var values = {};
	values.name = $('#project_name_value').html();
	values.publisher = $('#project_pub_value').html();
	values.dir = $('#project_dir_value').html();
	values.image = $('#project_pub_image_value').html();
	values.url = $('#project_pub_url_value').html();
	values.description = $('#project_desc_value').html();

	var id = $('#project_id_value').get(0).value;

	// update database
    db.transaction(function (tx) 
    {
        tx.executeSql("UPDATE Projects set name = ?, description = ?, directory = ?, publisher = ?, url = ?, image = ? WHERE id = ?", 
		[values.name,values.description,values.dir,values.publisher,values.url,values.image, id]);
    });
	
	// update our array cache
	var project = TiDeveloper.Projects.findProjectById(id);
	project.name = values.name;
	project.dir = values.dir;
	project.publisher = values.publisher;
	project.url = values.url;
	project.image = values.image;
	project.description = values.description;
	
};

//
// Refresh project download stats
//
TiDeveloper.Projects.refreshStats = function(guid)
{
	var statsArray = [];
	var statsLastUpdate = null;
	var statsAvailable = false;
	
    db.transaction(function (tx) 
    {
    	tx.executeSql("SELECT platform, count, date from ProjectDownloads WHERE guid = ?",[guid],
 			function(tx,result)
			{
				var date = null;
				// cycle through downloads
	           	for (var i = 0; i < result.rows.length; ++i) 
			   	{
	                var row = result.rows.item(i);
					var platform = row['platform'];
					var count = row['count'];
				 	statsLastUpdate= row['date'];
					statsArray.push({'name':platform,'value':count,});
				}
				if (result.rows.length > 0) statsAvailable = true;
			},
			function(error)
			{
				// create table
				tx.executeSql('CREATE TABLE ProjectDownloads (guid TEXT, platform TEXT, count TEXT, date TEXT)');
			}
		);
    });

	// load stats
	$('#download_stats_none').css('display','none');
	$('#download_stats').css('display','none');
	$('#download_stats_loading').css('display','block');
	
	var url = TiDeveloper.make_url(TiDeveloper.Projects.stats_url,{
		'guid':guid
	});
	
	$.ajax({
		type:'GET',
		dataType:'json',
		url:url,
		
		// update data
		success: function(data)
		{
			if (data.length > 0)
			{
				$('#download_stats_none').css('display','none')
				$('#download_stats').css('display','block');
				$('#download_stats_loading').css('display','none');
			}
		},
		error: function()
		{
			// if we have them, send message
			if (statsAvailable==true)
			{
				$('#download_stats_none').css('display','none');
				$('#download_stats').css('display','block');
				$('#download_stats_loading').css('display','none');
				$MQ('l:package_download_stats',{date:statsLastUpdate, rows:statsArray})
			}
			else
			{
				//TODO:remove
				setTimeout(function()
				{
					$('#download_stats_none').css('display','block');
					$('#download_stats').css('display','none');
					$('#download_stats_loading').css('display','none');
					
				},1000);
			}
		}
	});
	
};

//
// Refresh Download stats
//
$MQL('l:refresh_downlaod_stats',function(msg)
{
	TiDeveloper.Projects.refreshStats(TiDeveloper.Projects.selectedProject.guid)
})

//
// Set row selection listener for project list
//
$MQL('l:row.selected',function(msg)
{
	var msgObj = {};
	var project = TiDeveloper.Projects.findProjectById(msg.payload.project_id);
	TiDeveloper.Projects.selectedProject = project;	
	msgObj.date = project.date;
	msgObj.name = project.name;
	msgObj.location = TiDeveloper.Projects.formatDirectory(project.dir);
	msgObj.fullLocation = project.dir;
	msgObj.pub = project.publisher;
	msgObj.url = project.url;
	msgObj.image = project.image;
	msgObj.description = project.description;
	$MQ('l:project.detail.data',msgObj);
	
	if (TiDeveloper.Projects.packagingInProgress[project.guid] == true)
	{
		$('#packaging_none').css('display','none');
		$('#packaging_error').css('display','none');		
		$('#packaging_listing').css('display','none');
		$('#packaging_in_progress').css('display','block');
		
	}
	else
	{
		// get download info for DOWNLOAD tab
	    db.transaction(function (tx) 
	    {
	        tx.executeSql("SELECT url, platform, version, date from ProjectPackages WHERE guid = ?",[project.guid],
	 			function(tx,result)
				{
					if (result.rows.length == 0)
					{
						$('#packaging_none').css('display','block');
						$('#packaging_error').css('display','none');		
						$('#packaging_listing').css('display','none');
						$('#packaging_in_progress').css('display','none');
					}
					else
					{
						var a =[];
						var date = null;
						// cycle through downloads
			           	for (var i = 0; i < result.rows.length; ++i) 
					   	{
			                var row = result.rows.item(i);
							var url = row['url'];
							var platform = row['platform'];
							var version = row['version'];
							var date = row['date'];
							var platformShort = null;
							if (platform.indexOf('Win')!=-1) platformShort = "win";
							if (platform.indexOf('Linux')!=-1) platformShort = "linux";
							if (platform.indexOf('Mac')!=-1) platformShort = "mac";
							a.push({'url':url,'platform':platform,'version':version,'platform_short':platformShort});
						}
						$MQ('l:package_links',{date:date, rows:a});
						$('#packaging_none').css('display','none');
						$('#packaging_listing').css('display','block');
						$('#packaging_in_progress').css('display','none');
						$('#packaging_error').css('display','none');		
					}
				},
				function(error)
				{
					// create table
					tx.executeSql('CREATE TABLE ProjectPackages (guid TEXT, url TEXT, platform TEXT, version TEXT, date TEXT)');
					// show no downloads message
					$('#packaging_none').css('display','block');
					$('#packaging_listing').css('display','none');
					$('#packaging_in_progress').css('display','none');
					$('#packaging_error').css('display','none');		
				}
			);
	    });
	}
	
	TiDeveloper.Projects.refreshStats(project.guid);
	
	// setup editable fields for INFO tab
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
					$(activeFiles[i]).html($('#'+id+'_input').val());
					TiDeveloper.Projects.updateAppData();
					$(activeFiles[i]).get(0).removeAttribute('edit_mode');
				}
			}
			
			// process click
			var el = $(this).get(0);

			// if field requires file dialog - show
			if (el.id == 'project_pub_image_value')
			{
				// show dialog
				$MQ('l:show.filedialog',{'for':'project_image','target':'project_pub_image_value'});
				
				// listen for value selection
				$MQL('l:file.selected',function(msg)
				{
					var target = msg.payload.target;
					if (target=='project_pub_image_value')
					{
						el.removeAttribute('edit_mode');
						TiDeveloper.Projects.updateAppData();
					}
				});
			}
			var value = el.innerHTML;
			el.setAttribute('edit_mode','true');
			
			// create input and focus
			$(this).html('<input id="'+el.id+'_input" value="'+value+'" type="text" style="width:350px" maxlength="150"/>');
			$('#'+el.id+'_input').focus();
			
			// listen for enter
			$('#'+el.id+'_input').keyup(function(e)
			{
				if (e.keyCode==13)
				{
					el.innerHTML = $('#'+el.id+'_input').val();
					el.removeAttribute('edit_mode');
					TiDeveloper.Projects.updateAppData();
				}
				else if (e.keyCode==27)
				{
					el.innerHTML = value; 
					el.removeAttribute('edit_mode');
				}
			});

		}
	});
	
});

//
// Create a project record in the DB and update array cache
//
TiDeveloper.Projects.createRecord = function(options,callback)
{
	var date = new Date();
	var dateStr = (date.getMonth()+1)+"/"+date.getDate()+"/"+date.getFullYear();
	var record = {
		name: options.name,
		dir: String(options.dir),
		id: ++TiDeveloper.highestId,
		appid: options.appid,
		date: dateStr,
		publisher:options.publisher,
		url:options.url,
		image:options.image,
		guid:options.guid,
		description:options.description
	};
    db.transaction(function (tx) 
    {
        tx.executeSql("INSERT INTO Projects (id, guid, description,timestamp, name, directory, appid, publisher, url, image) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", [record.id,record.guid,record.description,date.getTime(),record.name,record.dir,record.appid,record.publisher,record.url,record.image]);
    },
	function(error)
	{
		alert('error insert ' + error);
		callback({code:1,id:error.id,msg:error.message});
	},
	function()
	{
		TiDeveloper.Projects.projectArray.push(record);
		callback({code:0});
	});
}

//
// load projects from db and populate array cache
//
TiDeveloper.Projects.loadProjects = function(init)
{
	db.transaction(function(tx) 
	{
        tx.executeSql("SELECT id, guid, description, timestamp, name, directory, appid, publisher, url, image FROM Projects order by timestamp", [], function(tx, result) 
		{
			TiDeveloper.Projects.projectArray = [];
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
					date.setTime(row['timestamp']);
					
					TiDeveloper.Projects.projectArray.push({
						id: row['id'],
						date: (date.getMonth()+1)+"/"+date.getDate()+"/"+date.getFullYear(),
						name: row['name'],
						dir: row['directory'],
						appid: row['appid'],
						publisher: row['publisher'],
						url: row['url'],
						image: row['image'],
						guid: row['guid'],
						description:row['description']
					});
					if (TiDeveloper.highestId < row['id'])
					{
						TiDeveloper.highestId = row['id'];
					}
				}
            }
			TiDeveloper.currentPage = 1;
			var data = TiDeveloper.Projects.getProjectPage(10,TiDeveloper.currentPage);
			var count = TiDeveloper.formatCountMessage(TiDeveloper.Projects.projectArray.length,'project');
			
			// show create project if none
			if (TiDeveloper.Projects.projectArray.length == 0)
			{
				$MQ('l:menu',{val:'manage'});
			}
			
			$('#project_count_hidden').val(TiDeveloper.Projects.projectArray.length);
			var firstCall = (init)?1:0;
			$MQ('l:project.list.response',{firstCall:firstCall,count:count,page:TiDeveloper.currentPage,totalRecords:TiDeveloper.Projects.projectArray.length,'rows':data});
        });
	});	
}

//
//  create.project service
//
$MQL('l:create.project.request',function(msg)
{
	try
	{
		var jsLibs = {jquery:false,jquery_ui:false,prototype_js:false,scriptaculous:false,dojo:false,mootools:false,swf:false,yui:false};
		if ($('#jquery_js').hasClass('selected_js'))
		{
			jsLibs.jquery = true;
		}
		if ($('#jqueryui_js').hasClass('selected_js'))
		{
			jsLibs.jquery_ui = true;
		}
		if ($('#prototype_js').hasClass('selected_js'))
		{
			jsLibs.prototype_js = true;
		}
		if ($('#scriptaculous_js').hasClass('selected_js'))
		{
			jsLibs.scriptaculous = true;
		}
		if ($('#dojo_js').hasClass('selected_js'))
		{
			jsLibs.dojo = true;
		}
		if ($('#mootools_js').hasClass('selected_js'))
		{
			jsLibs.mootools = true;
		}
		if ($('#swfobject_js').hasClass('selected_js'))
		{
			jsLibs.swf = true;
		}
		if ($('#yahoo_js').hasClass('selected_js'))
		{
			jsLibs.yahoo = true;
		}

		var guid = Titanium.Platform.createUUID();
		var result = Titanium.Project.create(msg.payload.project_name,guid,msg.payload.description,msg.payload.project_location,msg.payload.publisher,msg.payload.url,msg.payload.image,jsLibs);
		if (result.success)
		{
			var options = {name:result.name, guid:guid,description:msg.payload.description,dir:result.basedir,appid:result.id,publisher:msg.payload.publisher,url:msg.payload.url,image:msg.payload.image}
			var r = TiDeveloper.Projects.createRecord(options,function(obj)
			{
				if (obj.code == 0)
				{
					$MQ('l:create.project.response',{result:'success'});
					var count = TiDeveloper.formatCountMessage(TiDeveloper.Projects.projectArray.length,'project');
					$MQ('l:project.list.response',{count:count,page:1,totalRecords:TiDeveloper.Projects.projectArray.length,'rows':TiDeveloper.Projects.projectArray})
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
	if (TiDeveloper.Projects.projectArray.length == 0)return;
	
	var state =msg.payload;
	var rowsPerPage = state.rowsPerPage;
	var page = state.page;
	TiDeveloper.currentPage = page;
	var data = TiDeveloper.Projects.getProjectPage(rowsPerPage,page);
	var count = TiDeveloper.formatCountMessage(TiDeveloper.Projects.projectArray.length,'project');
	
	$MQ('l:project.list.response',{count:count,page:page,totalRecords:TiDeveloper.Projects.projectArray.length,'rows':data})
});


//
// Format directory string for display purposes
//
TiDeveloper.Projects.formatDirectory =function(dir)
{
	
	if (dir != null)
	{
		var dirStr = dir;
		if (dir.length > 70)
		{
			dirStr = dir.substring(0,70) + '...';
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
TiDeveloper.Projects.getProjectPage = function(pageSize,page)
{
	var pageData = [];
	var start = (page==0)?0:(pageSize * page) - pageSize;
	var end = ((pageSize * page) > TiDeveloper.Projects.projectArray.length)?TiDeveloper.Projects.projectArray.length:(pageSize * page);
	for (var i=start;i<end;i++)
	{
		pageData.push(TiDeveloper.Projects.projectArray[i]);
	}
	return pageData;
};


TiDeveloper.Projects.getModules = function(appDir)
{
	// reset vars
	TiDeveloper.Projects.module_map = {};
	TiDeveloper.Projects.requiredModuleMap = {};
	TiDeveloper.Projects.modules = [];

	var result = Titanium.Project.getModulesAndRuntime(appDir);
	TiDeveloper.Projects.runtimeDir = result.runtime.dir;
	TiDeveloper.Projects.runtimeVersion = result.runtime.versions[0];
	for (var c=0;c<result.modules.length;c++)
	{
		var name = result.modules[c].name;
		if (TiDeveloper.Projects.requiredModules.indexOf(name) == -1)
		{
			TiDeveloper.Projects.module_map[name]=result.modules[c];
			TiDeveloper.Projects.modules.push({name:name,versions:result.modules[c].versions,dir:result.modules[c].dir});
		}
		else
		{
			TiDeveloper.Projects.requiredModuleMap[name]=result.modules[c];
			
		}
	}
};

//
//  Handle Package Project Button - enable/disable
//
$MQL('l:os_platform_click',function()
{
	if ($('.selected_os').length > 0)
	{
		$('#package_project_button').attr('disabled','false');
	}
	else
	{
		$('#package_project_button').attr('disabled','true');
	}
})

//
//  Project Package Request - get details about modules, etc
//
$MQL('l:package.project.request',function(msg)
{
	var project = TiDeveloper.Projects.findProjectById(msg.payload.id);
	TiDeveloper.Projects.getModules(project.dir);
	$MQ('l:package.project.data',{rows:TiDeveloper.Projects.modules});
 	$MQ('l:package.all',{val:'network'});

});

//
// Find project by name
//
TiDeveloper.Projects.findProject = function(name)
{
	for (var i=0;i<TiDeveloper.Projects.projectArray.length;i++)
	{
		if (TiDeveloper.Projects.projectArray[i].name == name)
		{
			return TiDeveloper.Projects.projectArray[i];
		}
	}
	return null;
}

//
// Find project by ID
//
TiDeveloper.Projects.findProjectById = function(id)
{
	for (var i=0;i<TiDeveloper.Projects.projectArray.length;i++)
	{
		if (TiDeveloper.Projects.projectArray[i].id == id)
		{
			return TiDeveloper.Projects.projectArray[i];
		}
	}
	return null;
}

//
// Get a project name by id
//
TiDeveloper.Projects.getProjectName = function(id)
{
	var p =  TiDeveloper.Projects.findProjectById(id);
	return (p)?p.name:'Project Not Found';
}

//
// Launch or launch and install project locally
//
TiDeveloper.Projects.launchProject = function(project, install)
{
	try
	{
		TiDeveloper.Projects.getModules(project.dir);

		var resources = TFS.getFile(project.dir,'Resources');

		// build the manifest
		var manifest = '#appname:'+project.name+'\n';
		manifest+='#appid:'+project.appid+'\n';
		manifest+='#publisher:'+project.publisher+'\n';

		if (project.image)
		{
			var image = TFS.getFile(project.image);
			var image_dest = TFS.getFile(resources,image.name());
			if (image.exists())
			{
				image.copy(image_dest);
			}
			manifest+='#image:'+image.name()+'\n';
		}

		manifest+='#url:'+project.url+'\n';
		manifest+='#guid:'+project.guid+'\n';
		manifest+='#desc:'+project.description+'\n';

		manifest+='runtime:'+TiDeveloper.Projects.runtimeVersion+'\n';
		// write out required modules
		for (var i=0;i<TiDeveloper.Projects.requiredModules.length;i++)
		{
			manifest+= TiDeveloper.Projects.requiredModules[i] +':'+ TiDeveloper.Projects.requiredModuleMap[TiDeveloper.Projects.requiredModules[i]].versions[0]+'\n';
		}
		// write out optional modules
		for (var c=0;c<TiDeveloper.Projects.modules.length;c++)
		{
			var version = (TiDeveloper.Projects.modules[c].versions)?TiDeveloper.Projects.modules[c].versions[0]:'0.1';
			manifest+=TiDeveloper.Projects.modules[c].name+':'+version+'\n';
		}

		var mf = TFS.getFile(project.dir,'manifest');
		mf.write(manifest);
		var dist = TFS.getFile(project.dir,'dist',Titanium.platform);
		dist.createDirectory(true);
		var runtime = TFS.getFile(TiDeveloper.Projects.runtimeDir,TiDeveloper.Projects.runtimeVersion);
		var app = Titanium.createApp(runtime,dist,project.name,project.appid,install);
		var app_manifest = TFS.getFile(app.base,'manifest');
		app_manifest.write(manifest);
		var resources = TFS.getFile(project.dir,'Resources');
		var tiapp = TFS.getFile(project.dir,'tiapp.xml');
		tiapp.copy(app.base);
		TFS.asyncCopy(resources,app.resources,function()
		{
			// no modules to bundle, install the net installer
			var net_installer_src = TFS.getFile(runtime,'installer');
			var net_installer_dest = TFS.getFile(app.base,'installer');
			TFS.asyncCopy(net_installer_src,net_installer_dest,function(filename,c,total)
			{
				var appModules = TFS.getFile(project.dir,"modules");
				if (appModules.exists())
				{
					var moduleDest = TFS.getFile(app.base,"modules");
					TFS.asyncCopy(appModules,moduleDest, function()
					{
						Titanium.Process.setEnv('KR_DEBUG','true');
						Titanium.Desktop.openApplication(app.executable.nativePath());
					})
				}
				else
				{
					Titanium.Process.setEnv('KR_DEBUG','true');
					Titanium.Desktop.openApplication(app.executable.nativePath());
				}
			});
		});
	}
	catch(e)
	{
		alert('Error launching app ' + e);
	}
	
}

$MQL('l:launch.project.request',function(msg)
{
	var project_name = $('#package_project_name').html();
	var project = TiDeveloper.Projects.findProject(project_name);
	TiDeveloper.Projects.launchProject(project,false);
});

$MQL('l:launch.project.installer.request',function(msg)
{
	var project_name = $('#package_project_name').html();
	var project = TiDeveloper.Projects.findProject(project_name);
	TiDeveloper.Projects.launchProject(project,true);
});

//
// Create Package Request
//
$MQL('l:create.package.request',function(msg)
{
	try
	{
		// project name and project
		var project_name = $('#package_project_name').html();
		var project = TiDeveloper.Projects.findProject(project_name);

		// make sure required files/dirs are present
		var resources = TFS.getFile(project.dir,'Resources');
		if (!resources.exists())
		{
			alert('Your project is missing the Resources directory.  This directory is required for packaging.');
			return;
		}
		var tiapp = TFS.getFile(project.dir,'tiapp.xml');
		if (!tiapp.exists())
		{
			alert('Your tiapp.xml file is missing.  This file is required for packaging.');
			return;
		}

		// load modules
		TiDeveloper.Projects.getModules(project.dir);
		
		// manifest files to write out
		var manifest = '';
		var timanifest = "{\n";

		// OS options
		var buildMac = ($('#platform_mac').hasClass('selected_os'))?true:false;
		var buildWin = ($('#platform_windows').hasClass('selected_os'))?true:false;
		var buildLinux = ($('#platform_linux').hasClass('selected_os'))?true:false;

		// base runtime option
		var networkRuntime = ($('#required_modules_network').attr('checked') ==true)?'network':'include';

		// elements that are included for network bundle
		var networkEl = $("div[state='network']");

		// elements that are included (bundled)
		var bundledEl = $("div[state='bundled']");

		// elements that are excluded
		var excludedEl = $("div[state='exclude']");
		
		var excluded = {};
		

		//
		// Write out Manifest
		//
		
		// capture excluded modules
		$.each(excludedEl,function()
		{
			var key = $.trim($(this).html());
			excluded[key]=true;
		});
		
		
		// build the manifest
		manifest = '#appname:'+project_name+'\n';
		manifest+='#appid:'+project.appid+'\n';
		manifest+='#publisher:'+project.publisher+'\n';

		if (project.image)
		{
			var image = TFS.getFile(project.image);
			var image_dest = TFS.getFile(resources,image.name());
			if (image.exists())
			{
				image.copy(image_dest);
			}
			manifest+='#image:'+image.name()+'\n';
		}
		
		manifest+='#url:'+project.url+'\n';
		manifest+='#guid:'+project.guid +'\n';
		manifest+='#desc:'+project.description +'\n';
		manifest+='runtime:'+TiDeveloper.Projects.runtimeVersion+'\n';
		
		// write out required modules
		for (var i=0;i<TiDeveloper.Projects.requiredModules.length;i++)
		{
			manifest+= TiDeveloper.Projects.requiredModules[i] +':'+ TiDeveloper.Projects.requiredModuleMap[TiDeveloper.Projects.requiredModules[i]].versions[0]+'\n';
		}
		// write out optional modules
		for (var c=0;c<TiDeveloper.Projects.modules.length;c++)
		{
			if (!excluded[TiDeveloper.Projects.modules[c].name])
			{
				var version = (TiDeveloper.Projects.modules[c].versions)?TiDeveloper.Projects.modules[c].versions[0]:'1.0';
				manifest+=TiDeveloper.Projects.modules[c].name+':'+version+'\n';
			}
		}
		
		var mf = TFS.getFile(project.dir,'manifest');
		mf.write(manifest);

		//
		// Write out TIMANIFEST
		//
		timanifest += '"appname":"'+project_name+'",\n';
		timanifest += '"appid":"'+project.appid+'",\n';
		timanifest += '"appversion":"1.0",\n';
		timanifest += '"mid":"'+Titanium.Platform.id+'",\n';
		timanifest += '"publisher":"'+project.publisher+'",\n';
		timanifest += '"url":"'+project.url+'",\n';
		timanifest += '"desc":"'+project.description+'",\n';
		var visibility = ($('#package_public').attr('checked')==true)?'public':'private';
	    timanifest += '"visibility":"'+visibility+'",\n';
		
		var platforms = '"platforms":[';
		if (buildMac) platforms += '"osx"'
		if (buildMac && (buildWin || buildLinux)) platforms += ',';
		if (buildWin) platforms += '"win32"';
		if (buildLinux) platforms += ','
		if (buildLinux) platforms +='"linux"';
		platforms+= '],\n';
		
		timanifest += platforms;
		
		timanifest += '"runtime":{"version":"'+Titanium.version+'","package":"'+networkRuntime+'"},\n';
		
		timanifest += '"guid":"'+ project.guid+'",\n';
		
		var modules = '"modules":[';
		
		// required modules
		if (networkRuntime ==true)
		{
			for (var i=0;i<TiDeveloper.Projects.requiredModules.length;i++)
			{
				modules+='{"name":'+'"'+TiDeveloper.Projects.requiredModules[i]+'","version":'+'"'+TiDeveloper.Projects.requiredModuleMap[TiDeveloper.Projects.requiredModules[i]].versions[0]+'"'+',"package":"network"}';
				if (i<(TiDeveloper.Projects.requiredModules.length))
				{
					modules+= ',\n';
				}
			}
		}
		else
		{
			for (var i=0;i<TiDeveloper.Projects.requiredModules.length;i++)
			{
				modules+='{"name":'+'"'+TiDeveloper.Projects.requiredModules[i]+'","version":'+'"'+TiDeveloper.Projects.requiredModuleMap[TiDeveloper.Projects.requiredModules[i]].versions[0]+'"'+',"package":"include"}';
				if (i<(TiDeveloper.Projects.requiredModules.length))
				{
					modules+= ',\n';
				}
			}
		}

		// write out optional modules
		for (var c=0;c<TiDeveloper.Projects.modules.length;c++)
		{
			var module = TiDeveloper.Projects.modules[c].name;
			var version = (TiDeveloper.Projects.modules[c].versions)?TiDeveloper.Projects.modules[c].versions[0]:Titanium.version;
			
			$.each(excludedEl,function()
			{
				var key = $.trim($(this).html());
				if (key == module)
				{
					modules+='{"name":"'+module+'","version":'+'"'+version+'","package":"exclude"}';
				}
			});
			$.each(bundledEl,function()
			{
				var key = $.trim($(this).html());
				if (key == module)
				{
					modules+='{"name":"'+module+'","version":'+'"'+version+'","package":"include"}';
				}
			});
			$.each(networkEl,function()
			{
				var key = $.trim($(this).html());
				if (key == module)
				{
					modules+='{"name":"'+module+'","version":'+'"'+version+'","package":"network"}';
				}
			});
			if (c<(TiDeveloper.Projects.modules.length-1))
			{
				modules+= ',\n';
			}
			
		}
		timanifest+= modules + ']}\n';
		var timanifestFile = TFS.getFile(project.dir,'timanifest');
		timanifestFile.write(timanifest);
				
		//
		// NOW CREATE TEMP DIR AND MOVE CONTENTS FOR PACKAGING
		//
		
		var destDir = Titanium.Filesystem.createTempDirectory();
		var modules = TFS.getFile(project.dir,'modules');
		var timanifest = TFS.getFile(project.dir,'timanifest');
		var manifest = TFS.getFile(project.dir,'manifest');
		
		// copy files to temp dir
		var resDir = TFS.getFile(destDir,'Resources');
		resDir.createDirectory();
		
		//FIXME: we can't do this ... async may not finish before you
		//go to package
		TFS.asyncCopy(resources, resDir,function(){});		
		TFS.asyncCopy(tiapp, destDir,function(){});		
		TFS.asyncCopy(timanifest, destDir,function(){});		
		TFS.asyncCopy(manifest, destDir,function(){});		
		
		// if project has modules, copy
		if (modules.exists())
		{
			// create resources dir
			var resDir = TFS.getFile(destDir,'modules');
			resDir.createDirectory();
			TFS.asyncCopy(modules, resDir,function(){});
		}
		alert(destDir);

		// packaging request
		var xhr = Titanium.Network.createHTTPClient();
		var ticket = null;
		xhr.onreadystatechange = function()
		{
			// 4 means that the POST has completed
			if (this.readyState == 4)
			{
				if (this.status == 200)
				{
				    var json = swiss.evalJSON(this.responseText);
					destDir.deleteDirectory(true);
					TiDeveloper.Projects.pollPackagingRequest(json.ticket,project.guid);
				}
				else
				{
					$('#packaging_none').css('display','none');
					$('#packaging_listing').css('display','none');
					$('#packaging_error').css('display','block');		
					$('#packaging_in_progress').css('display','none');
					TiDeveloper.Projects.packagingInProgress[project.guid] = false;
					destDir.deleteDirectory(true);
				}
			}
		};
		
		xhr.open("POST",TiDeveloper.Projects.publish_url);
		xhr.sendDir(destDir);    

		TiDeveloper.Projects.packagingInProgress[project.guid] = true;
		$('#packaging_none').css('display','none');
		$('#packaging_listing').css('display','none');
		$('#packaging_error').css('display','none');		
		$('#packaging_in_progress').css('display','block');
		$MQ('l:create.package.response',{result:0});
		
	}
	catch(E)
	{
		alert("Exception = "+E);
	}
});


TiDeveloper.Projects.pollPackagingRequest = function(ticket,guid)
{          
	var url = TiDeveloper.make_url(TiDeveloper.Projects.publish_status_url,{
		'ticket':ticket
	});
	$.getJSON(url,function(r)
	{
	   	if (r.status == 'complete')
	   	{
    		alert('done ' + swiss.toJSON(r));
			TiDeveloper.Projects.packagingInProgress[project.guid] = false;

			// INSERT DATA INTO DB AND SHOW DATA
			// SEND MESSAGE TO POPULATE TABLE
			$('#packaging_none').css('display','none');
			$('#packaging_error').css('display','none');
			$('#packaging_listing').css('display','block');
			$('#packaging_in_progress').css('display','none');
		}
		else if (r.status != 'working')
		{
			$('#packaging_none').css('display','none');
			$('#packaging_listing').css('display','none');
			$('#packaging_error').css('display','block');		
			$('#packaging_in_progress').css('display','none');
	   	}
		else
		{
			// poll every 10 seconds
			setTimeout(function()
			{
				TiDeveloper.Projects.pollPackagingRequest(ticket,guid);
			},10000);
		}
	});
};
//
//  Delete a project
//	
$MQL('l:delete.project.request',function(msg)
{
	var name = msg.payload.name;
	var id = msg.payload.project_id;
	var project = TiDeveloper.Projects.findProjectById(id);
	var file = Titanium.Filesystem.getFile(project.dir);

	file.deleteDirectory(true);
	
	db.transaction(function (tx) 
    {
        tx.executeSql("DELETE FROM Projects where id = ?", [id]);
		TiDeveloper.Projects.loadProjects();
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
	        tx.executeSql("SELECT id, guid, description, timestamp, appid, publisher, url, image, name, directory FROM Projects where name LIKE '%' || ? || '%'", [q], function(tx, result) 
			{
				try
				{
					TiDeveloper.Projects.projectArray = [];
		            for (var i = 0; i < result.rows.length; ++i) 
					{
		                var row = result.rows.item(i);
						var date = new Date();
						date.setTime(row['timestamp']);
						TiDeveloper.Projects.projectArray.push({
							id: row['id'],
							date: (date.getMonth()+1)+"/"+date.getDate()+"/"+date.getFullYear(),
							name: row['name'],
							dir: row['directory'],
							appid: row['appid'],
							publisher: row['publisher'],
							url: row['url'],
							image: row['image'],
							guid:row['guid'],
							description:row['description']
						});
					}
					$MQ('l:project.search.response',{count:TiDeveloper.Projects.projectArray.length,page:1,totalRecords:TiDeveloper.Projects.projectArray.length,'rows':TiDeveloper.Projects.projectArray});
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
	var target = msg.payload.target;
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
			$MQ('l:file.selected',{'target':target,'for':el,'value':f[0]});
			if (msg.payload.callback)
			{
				msg.payload.callback(f[0]);
			}
		}
	},
	props);
});

