TiDeveloper.Projects = {};
TiDeveloper.Projects.projectArray = [];
TiDeveloper.Projects.modules = [];
TiDeveloper.Projects.module_map = {};
TiDeveloper.Projects.runtimeDir = null;
TiDeveloper.Projects.runtimeVersion = null;
TiDeveloper.Projects.requiredModuleMap = {};
TiDeveloper.Projects.requiredModules = ['api','tiapp','tifilesystem','tiplatform','tiui']


$MQL('l:app.compiled',function()
{
	$MQ('l:package_links',{date:'04/10/09',rows:[{'family':'win','platform':'Windows XP SP2',date:'04/10/09'},{'family':'win','platform':'Windows Vista',date:'04/10/09'},{'family':'mac','platform':'Mac OSX 10.5',date:'04/10/09'},{'family':'mac','platform':'Mac OSX 10.4',date:'04/10/09'},{'family':'linux','platform':'Linux Ubuntu',date:'04/10/09'}]})
	$MQ('l:downloads',{'type':'OS','rows':[{name:'Mac',value:2500},{name:'Windows',value:5000},{name:'Linux',value:320}]});
});

//
//  Initialization message - setup all initial states
//
$MQL('l:app.compiled',function()
{
	// load or initialize project table
	db.transaction(function(tx) 
	{
	   tx.executeSql("SELECT COUNT(*) FROM Projects", [], function(result) 
	   {
	       TiDeveloper.Projects.loadProjects(true);
	   }, function(tx, error) 
	   {
	       tx.executeSql("CREATE TABLE Projects (id REAL UNIQUE, timestamp REAL, name TEXT, directory TEXT, appid TEXT, publisher TEXT, url TEXT, image TEXT)", [], function(result) 
		   { 
	          TiDeveloper.Projects.loadProjects(true); 
	       });
	   });
	});
	
});

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
	Titanium.Project.updateManifest(values)

	var id = $('#project_id_value').get(0).value;

	// update database
    db.transaction(function (tx) 
    {
        tx.executeSql("UPDATE Projects set name = ?, directory = ?, publisher = ?, url = ?, image = ? WHERE id = ?", 
		[values.name,values.dir,values.publisher,values.url,values.image, id]);
    });
	
	// update our array cache
	var project = TiDeveloper.Projects.findProjectById(id)
	project.name = values.name
	project.dir = values.dir
	project.publisher = values.publisher
	project.url = values.url
	project.image = values.image
	
};

//
// Set row selection listener for project list
//
$MQL('l:row.selected',function(msg)
{
	var msgObj = {}
	var project = TiDeveloper.Projects.findProjectById(msg.payload.project_id);
	msgObj.date = project.date;
	msgObj.name = project.name;
	msgObj.location = TiDeveloper.Projects.formatDirectory(project.dir);
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
			$(this).html('<input id="'+el.id+'_input" value="'+value+'" type="text" style="width:350px"/>');
			$('#'+el.id+'_input').focus();
			
			// listen for enter
			$('#'+el.id+'_input').keyup(function(e)
			{
				if (e.keyCode==13)
				{
					el.innerHTML = $('#'+el.id+'_input').val() 
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
	})
	
})

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
		TiDeveloper.Projects.projectArray.push(record);
		callback({code:0})
	});
}

//
// load projects from db and populate array cache
//
TiDeveloper.Projects.loadProjects = function(init)
{
	db.transaction(function(tx) 
	{
        tx.executeSql("SELECT id, timestamp, name, directory, appid, publisher, url, image FROM Projects order by timestamp", [], function(tx, result) 
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
					date.setTime(row['timestamp'])
					
					TiDeveloper.Projects.projectArray.push({
						id: row['id'],
						date: (date.getMonth()+1)+"/"+date.getDate()+"/"+date.getFullYear(),
						name: row['name'],
						dir: row['directory'],
						appid: row['appid'],
						publisher: row['publisher'],
						url: row['url'],
						image: row['image']
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
				$MQ('l:menu',{val:'manage'})
			}
			
			$('#project_count_hidden').val(TiDeveloper.Projects.projectArray.length)
			var firstCall = (init)?1:0
			$MQ('l:project.list.response',{firstCall:firstCall,count:count,page:TiDeveloper.currentPage,totalRecords:TiDeveloper.Projects.projectArray.length,'rows':data})
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
		
		var result = Titanium.Project.create(msg.payload.project_name,msg.payload.project_location,msg.payload.publisher,msg.payload.url,msg.payload.image,jsLibs);
		if (result.success)
		{
			var options = {name:result.name, dir:result.basedir,appid:result.id,publisher:msg.payload.publisher,url:msg.payload.url,image:msg.payload.image}
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
	
	var state =msg.payload
	var rowsPerPage = state.rowsPerPage
	var page = state.page
	TiDeveloper.currentPage = page;
	var data = TiDeveloper.Projects.getProjectPage(rowsPerPage,page);
	var count = TiDeveloper.formatCountMessage(TiDeveloper.Projects.projectArray.length,'project');
	
	$MQ('l:project.list.response',{count:count,page:page,totalRecords:TiDeveloper.Projects.projectArray.length,'rows':data})
});


//
// Format directory string for display purposes
//
TiDeveloper.Projects.formatDirectory =function(dir,truncate)
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
TiDeveloper.Projects.getProjectPage = function(pageSize,page)
{
	var pageData = [];
	var start = (page==0)?0:(pageSize * page) - pageSize;
	var end = ((pageSize * page) > TiDeveloper.Projects.projectArray.length)?TiDeveloper.Projects.projectArray.length:(pageSize * page);
	for (var i=start;i<end;i++)
	{
		pageData.push(TiDeveloper.Projects.projectArray[i])
	}
	return pageData
};


TiDeveloper.Projects.getModules = function()
{
	var result = Titanium.Project.getModulesAndRuntime();
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
//  Project Package Request - get details about modules, etc
//
$MQL('l:package.project.request',function(msg)
{
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
		// load modules
		TiDeveloper.Projects.getModules();
		
		var resources = TFS.getFile(project.dir,'Resources');

		// build the manifest
		var manifest = '#appname:'+project.name+'\n';
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
		manifest+='runtime:'+TiDeveloper.Projects.runtimeVersion+'\n';

		// write out required modules
		for (var i=0;i<TiDeveloper.Projects.requiredModules.length;i++)
		{
			manifest+= TiDeveloper.Projects.requiredModules[i] +':'+ TiDeveloper.Projects.requiredModuleMap[TiDeveloper.Projects.requiredModules[i]].versions[0]+'\n';
		}
		// write out optional modules
		for (var c=0;c<TiDeveloper.Projects.modules.length;c++)
		{
			manifest+=TiDeveloper.Projects.modules[c].name+':'+TiDeveloper.Projects.modules[c].versions[0]+'\n';
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
				Titanium.Process.setEnv('KR_DEBUG','true');
				Titanium.Desktop.openApplication(app.executable.nativePath());
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

//
// Create Package Request
//
$MQL('l:create.package.request',function(msg)
{
	try
	{
		// load modules
		TiDeveloper.Projects.getModules();

		// project name
		var project_name = $('#package_project_name').html();
		var project = TiDeveloper.Projects.findProject(project_name);
		
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
		
		// timanifest file
		timanifest += '"appname":"'+project_name+'",\n';
		timanifest += '"appid":"'+project.appid+'",\n';
		timanifest += '"appversion":"1.0",\n';
		timanifest += '"mid":"'+Titanium.Platform.id+'",\n';
		timanifest += '"publisher":"'+project.publisher+'",\n';
		timanifest += '"url":"'+project.url+'",\n';
	    timanifest += '"visibility":"public",\n';
		
		var platforms = '"platforms":[';
		if (buildMac) platforms += '"osx"'
		if (buildMac && (buildWin || buildLinux)) platforms += ',';
		if (buildWin) platforms += '"win32"';
		if (buildLinux) platforms += ','
		if (buildLinux) platforms +='"linux"';
		platforms+= '],\n';
		
		timanifest += platforms;
		
		timanifest += '"runtime":{"version":"0.2","package":"'+networkRuntime+'"},\n';
		
		timanifest += '"guid":"'+ Titanium.Platform.createUUID()+'",\n';
//		timanifest += '"guid":"'+ new Date().getTime()+'",\n';
		
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
			$.each(excludedEl,function()
			{
				var key = $.trim($(this).html());
				if (key == module)
				{
					modules+='{"name":"'+module+'","version":'+'"'+TiDeveloper.Projects.modules[c].versions[0]+'","package":"exclude"}';
				}
			});
			$.each(bundledEl,function()
			{
				var key = $.trim($(this).html());
				if (key == module)
				{
					modules+='{"name":"'+module+'","version":'+'"'+TiDeveloper.Projects.modules[c].versions[0]+'","package":"include"}';
				}
			});
			$.each(networkEl,function()
			{
				var key = $.trim($(this).html());
				if (key == module)
				{
					modules+='{"name":"'+module+'","version":'+'"'+TiDeveloper.Projects.modules[c].versions[0]+'","package":"network"}';
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
		
		var xhr = Titanium.Network.createHTTPClient();
		var ticket = null
		xhr.onreadystatechange = function()
		{
			// 4 means that the POST has completed
			if (this.readyState == 4)
			{
				if (this.status == 200)
				{
				    var json = swiss.evalJSON(this.responseText);
					alert('got 200 ticket= ' + json.ticket);
					TiDeveloper.Projects.pollPackagingRequest(json.ticket);
				}
				else
				{
					alert('upload error')
					TiDeveloper.Projects.handlePackagingError('upload');
				}
			}
		} ;
		xhr.open("POST",'http://publisher.titaniumapp.com/api/publish');
		xhr.sendDir(project.dir);    

		// write out required modules
		// for (var i=0;i<TiDeveloper.Projects.requiredModules.length;i++)
		// {
		// 	manifest+= TiDeveloper.Projects.requiredModules[i] +':'+ TiDeveloper.Projects.requiredModuleMap[TiDeveloper.Projects.requiredModules[i]].versions[0]+'\n';
		// }
		// write out optional modules
		// for (var c=0;c<TiDeveloper.Projects.modules.length;c++)
		// {
		// 	if (!excluded[TiDeveloper.Projects.modules[c].name])
		// 	{
		// 		manifest+=TiDeveloper.Projects.modules[c].name+':'+TiDeveloper.Projects.modules[c].versions[0]+'\n';
		// 	}
		// }

		
		
		// $.each(excludedEl,function()
		// {
		// 	var key = $.trim($(this).html());
		// 	excluded[key]=true;
		// });
		// 
		// var resources = TFS.getFile(project.dir,'Resources');
		// 
		// // build the manifest
		// manifest = '#appname:'+project_name+'\n';
		// manifest+='#appid:'+project.appid+'\n';
		// manifest+='#publisher:'+project.publisher+'\n';
		// 
		// if (project.image)
		// {
		// 	var image = TFS.getFile(project.image);
		// 	var image_dest = TFS.getFile(resources,image.name());
		// 	image.copy(image_dest);
		// 	manifest+='#image:'+image.name()+'\n';
		// }
		// 
		// manifest+='#url:'+project.url+'\n';
		// manifest+='runtime:'+TiDeveloper.Projects.runtimeVersion+'\n';
		// 
		// // write out required modules
		// for (var i=0;i<TiDeveloper.Projects.requiredModules.length;i++)
		// {
		// 	manifest+= TiDeveloper.Projects.requiredModules[i] +':'+ TiDeveloper.Projects.requiredModuleMap[TiDeveloper.Projects.requiredModules[i]].versions[0]+'\n';
		// }
		// // write out optional modules
		// for (var c=0;c<TiDeveloper.Projects.modules.length;c++)
		// {
		// 	if (!excluded[TiDeveloper.Projects.modules[c].name])
		// 	{
		// 		manifest+=TiDeveloper.Projects.modules[c].name+':'+TiDeveloper.Projects.modules[c].versions[0]+'\n';
		// 	}
		// }
		// 
		// var mf = TFS.getFile(project.dir,'manifest');
		// mf.write(manifest);
		// 
		// var dist = TFS.getFile(project.dir,'dist',Titanium.platform);
		// dist.createDirectory(true);
		// 
		// var runtime = TFS.getFile(TiDeveloper.Projects.runtimeDir,TiDeveloper.Projects.runtimeVersion);
		// var app = Titanium.createApp(runtime,dist,project_name,project.appid,install);
		// var app_manifest = TFS.getFile(app.base,'manifest');
		// app_manifest.write(manifest);
		// var resources = TFS.getFile(project.dir,'Resources');
		// var tiapp = TFS.getFile(project.dir,'tiapp.xml');
		// tiapp.copy(app.base);

		// 1. get data from UI
		// 2. create temp dir with timanifest + tiapp.xml + Resources;
		// 3. call service
		// 4. get ticket and start poll with ticket (store ticket for later use + guid + status (app table))
		// Titanium.Filesystem.createTempDirectory() = returns file object
		// Titanium.Filesystem.createTempFile(); = returns file object
		// TODO: DO SEPARATE LAUNCH FUNCTION
		// var launch_fn = function()
		// {
		// 	if (launch)
		// 	{
		// 		Titanium.Process.setEnv('KR_DEBUG','true');
		// 		Titanium.Desktop.openApplication(app.executable.nativePath());
		// 	}
		// };
	}
	catch(E)
	{
		alert("Exception = "+E);
	}
})

//
// error function for failed packaging request
//
TiDeveloper.Projects.handlePackagingError = function(type)
{
	alert('Error during ' + type)
}

TiDeveloper.Projects.pollPackagingRequest = function(ticket)
{          
	$.get('http://d.titaniumapp.com/publish-status?ticket='+ticket,function(result)
	{
		var r = swiss.evalJSON(result)
	   	if (r.status == 'complete')
	   	{
    		alert('done ' + swiss.toJSON(r));
	   	}
	   	else if (r.status != 'working')
	   	{
	      	TiDeveloper.Projects.handlePackagingError('packaging')
	   	}
		else
		{
			setTimeout(TiDeveloper.Projects.pollPackagingRequest(ticket),2000);
		}
	});
};
//
//  Delete a project
//	
$MQL('l:delete.project.request',function(msg)
{
	var name = msg.payload.name;
	var id = msg.payload.project_id
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
	        tx.executeSql("SELECT id, timestamp, appid, publisher, url, image, name, directory FROM Projects where name LIKE '%' || ? || '%'", [q], function(tx, result) 
			{
				try
				{
					TiDeveloper.Projects.projectArray = [];
		            for (var i = 0; i < result.rows.length; ++i) 
					{
		                var row = result.rows.item(i);
						var date = new Date();
						date.setTime(row['timestamp'])
						TiDeveloper.Projects.projectArray.push({
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
		}
	},
	props);
});

