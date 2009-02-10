//
// load projects when manage projects is clicked
//
var TFS = Titanium.Filesystem;
var TiDeveloper  = {};
TiDeveloper.currentPage = 1;
TiDeveloper.init = false;

// holder var for all projects
TiDeveloper.ProjectArray = [];
var db = openDatabase("TiDeveloper","1.0");
var highestId = 0;

function createRecord(name,dir)
{
	var record = {
		name: name,
		dir: dir,
		id: highestId++,
		date: new Date().getTime()
	};
	TiDeveloper.ProjectArray.push(record);
    db.transaction(function (tx) 
    {
        tx.executeSql("INSERT INTO Projects (id, timestamp, name, directory) VALUES (?, ?, ?, ?)", [record.id, record.date, record.name, record.dir]);
    });
}

function loadProjects()
{
	db.transaction(function(tx) 
	{
        tx.executeSql("SELECT id, timestamp, name, directory FROM Projects", [], function(tx, result) 
		{
            for (var i = 0; i < result.rows.length; ++i) {
                var row = result.rows.item(i);
				TiDeveloper.ProjectArray.push({
					id: row['id'],
					date: row['timestamp'],
					name: row['name'],
					dir: row['directory']
				});
				if (highestId < row['id'])
				{
					highestId = row['id'];
				}
            }
        }, function(tx, error) {
            alert('Failed to retrieve projects from database - ' + error.message);
            return;
        });
	});	
}

db.transaction(function(tx) 
{
   tx.executeSql("SELECT COUNT(*) FROM Projects", [], function(result) 
   {
       loadProjects();
   }, function(tx, error) 
   {
       tx.executeSql("CREATE TABLE Projects (id REAL UNIQUE, timestamp REAL, name TEXT, directory TEXT)", [], function(result) 
	   { 
          loadProjects(); 
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
		var result = Titanium.Project.create(msg.payload.project_name,msg.payload.project_location);
		if (result.success)
		{
			createRecord(result.name,result.basedir);
			$MQ('l:create.project.response',{result:'success'});
			$MQ('l:project.list.response',{page:1,totalRecords:TiDeveloper.ProjectArray.length,'rows':TiDeveloper.ProjectArray})
		}
		else
		{
			$MQ('l:create.project.response',{result:'error',message:result.message});
		}
	}
	catch(E)
	{
		alert('Exception = '+E);
	}
});

//
// Handling paging requests
//
$MQL('l:page.data.request',function(msg)
{
	var state =msg.payload
	var rowsPerPage = state.rowsPerPage
	var page = state.page
	TiDeveloper.currentPage = page;
	var data = TiDeveloper.getProjectPage(rowsPerPage,page);
	$MQ('l:project.list.response',{page:page,totalRecords:TiDeveloper.ProjectArray.length,'rows':data})
});
TiDeveloper.formatDirectory =function(dir)
{
	var dirStr = dir
	if (dir.length > 40)
	{
		dirStr = dir.substring(0,40) + '...';
		$('#project_detail_dir_a').css('display','block');
		$('#project_detail_dir_span').css('display','none');
		$('#project_detail_dir_a').html(dirStr);
	}
	else
	{
		$('#project_detail_dir_span').css('display','block');
		$('#project_detail_dir_a').css('display','none');
		$('#project_detail_dir_span').html(dirStr);
	}
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

//
// Load initial projects
//
$MQL('l:menu',function(msg)
{
	if (msg.payload.val == 'manage' && TiDeveloper.init == false)
	{
		var data = TiDeveloper.getProjectPage(10,0)
		$MQ('l:project.list.response',{totalRecords:TiDeveloper.ProjectArray.length,'rows':data});
		TiDeveloper.init=true;
	}
});
//
//  Project Package Request - get details about modules, etc
//
$MQL('l:package.project.request',function(msg)
{
	$MQ('l:package.project.data',{rows:[
	{name:'Titanium Runtime'},
	{name:'Ruby Language Module'},
	{name:'Python'},
	{name:'network'},
	{name:'file'},
	{name:'chat'},
	{name:'java'},
	{name:'php'},
	{name:'desktop_core'},
	{name:'custom_module_1'},
	{name:'custom_module_2'},
	{name:'custom_module_3'},
	{name:'custom_module_4'}
		
		
	]})
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

//
// Create Package Request
//
$MQL('l:create.package.request',function(msg)
{
	// elements that are included for network bundle
	var network = $("div[state='network']").length;
	
	// elements that are included (bundled)
	var bundled = $("div[state='bundled']").length;

	// project name
	var project = $('#package_project_name').html();
	
	var launch = msg.payload.launch;
	
	if (msg.payload.launch ==true)
	{
		// do  launch
		var dest = Titanium.Process.getEnv('KR_RUNTIME');
		var runtimeDir = TFS.getFile(dest);
		var ext = (Titanium.platform == 'win32') ? '.exe' : '';
		var exec = TFS.getFile(runtimeDir,'template','kboot'+ext);
		var proj = findProject(project);
		// try
		// 		{
		// 			Titanium.Process.launch(String(exec),'--start="'+proj.dir+'"');
		// 		}
		// 		catch (E)
		// 		{
		// 			alert("Error: "+E);
		// 		}
	}
	else
	{
		
	}
})

//
//  Delete a project
//	
$MQL('l:delete.project.request',function(msg)
{
	var name = msg.payload.name;
	var id = msg.payload.id
	if (confirm('Are you sure you want to delete project: ' + name + '?')==true)
	{
		for (var i=0;i<TiDeveloper.ProjectArray.length;i++)
		{
			if (TiDeveloper.ProjectArray[i].id == id)
			{
				TiDeveloper.ProjectArray.splice(i,1);
				break;
			}
		}
	}
	var data = TiDeveloper.getProjectPage(10,TiDeveloper.currentPage);
	$MQ('l:project.list.response',{page:TiDeveloper.currentPage,totalRecords:TiDeveloper.ProjectArray.length,'rows':data})
})
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
	        tx.executeSql("SELECT id, timestamp, name, directory FROM Projects where name LIKE 'foobar'", [], function(tx, result) 
			{
				try
				{
					var results = [];
		            for (var i = 0; i < result.rows.length; ++i) {
		                var row = result.rows.item(i);
						results.push({
							id: row['id'],
							date: row['timestamp'],
							name: row['name'],
							dir: row['directory']
						});
					}
					$MQ('l:project.search.response',{totalRecords:results.length,'rows':results});
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
})

//
// Show file dialog and send value
//
$MQL('l:show.filedialog',function()
{
	var files = Titanium.UI.openFiles({directories:true});
	var val = files.length ? files[0] : '';
	$MQ('l:file.selected',{value:val});
})

setTimeout(function()
{
	try
	{
		var myNick = Titanium.Platform.username+'1';
		var myName = Titanium.Platform.username+'1';
		var myNameStr = Titanium.Platform.username+'1';

		$('#irc').append('<div style="color:#111">you are joining the room. one moment...</div>');
		
		var irc = Titanium.Network.createIRCClient();
		irc.connect("irc.freenode.net",6667,myNick,myName,myNameStr,String(new Date().getTime()),function(cmd,channel,data,nick)
		{
			// switch on command
			switch(cmd)
			{
				case 'PRIVMSG':
				{
					$('#irc').append('<div style="color:yellow">' + nick + ': <span style="color:#fff">' + channel.substring(1,channel.length) + '</span></div>');
					break;
				}
				case '366':
				{
					var users = irc.getUsers('#titanium_dev')
				}
				case 'JOIN':
				{
					if (nick == myNick)
					{
						$('#irc').append('<div style="color:#111"> you are now in the room. </div>');
						break
					}
					$('#irc').append('<div style="color:#aaa">' + nick + ' has joined the room </div>');
					break;
					
				}
				case 'PART':
				{
					$('#irc').append('<div style="color:#aaa">' + nick + ' has left the room </div>');
					break;
					
				}
			}
			$('#irc').append('<div>'+cmd+"=>"+channel+':'+data+ ' ' + nick + '</div>');
			$('#irc').get(0).scrollTop = $('#irc').get(0).scrollHeight;
		});

		irc.join("#titanium_dev");
		$('#irc_send').click(function()
		{
			irc.send('#titanium_dev',$('#irc_msg').val());
			$('#irc').append('<div><span style="color:#fff">'+myNick + ':</span> ' + $('#irc_msg').val() + '</div>');
			$('#irc_msg').val('');
			$('#irc').get(0).scrollTop = $('#irc').get(0).scrollHeight;

		})
	}
	catch(E)
	{
		alert("Exception: "+E);
	}
},1000);

