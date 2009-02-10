//
// load projects when manage projects is clicked
//
var TFS = Titanium.Filesystem;
var TiDeveloper  = {};
TiDeveloper.currentPage = 1;
TiDeveloper.init = false;

// holder var for all projects
TiDeveloper.ProjectArray = [];


function makeDate()
{
	var d = new Date;
	return (d.getMonth() + 1) + '/' + d.getDate() + '/' + d.getFullYear();
}

(function()
{
	//TODO: switch this to SQL ASAP
	if (Titanium.App.Properties.hasProperty("project_count"))
	{
		var count = Titanium.App.Properties.getInt("project_count");
		for (var c=0;c<count;c++)
		{
			var project = Titanium.App.Properties.getList("project."+c);
			TiDeveloper.ProjectArray.push({
				id:project[0],
				name:project[1],
				date:project[2],
				dir:project[3]
			});
		}
	}
})();

function save()
{
	Titanium.App.Properties.setInt("project_count",TiDeveloper.ProjectArray.length);
	for (var c=0;c<TiDeveloper.ProjectArray.length;c++)
	{
		var project = TiDeveloper.ProjectArray[c];
		var entry = [String(project.id),project.name,project.date,String(project.dir)];
		Titanium.App.Properties.setList("project."+c,entry);
	}
}

//
//  create.project mock service
//
$MQL('l:create.project.request',function(msg)
{
	//TODO: do we check for existence of directory and fail?
	var result = Titanium.Project.create(msg.payload.project_name,msg.payload.project_location);
	if (result.success)
	{
		TiDeveloper.ProjectArray.push({
			id: String(TiDeveloper.ProjectArray.length),
			name: result.name,
			date: makeDate(),
			dir: String(result.basedir)
		});
		$MQ('l:create.project.response',{result:'success'});
		save();
		$MQ('l:project.list.response',{page:1,totalRecords:TiDeveloper.ProjectArray.length,'rows':TiDeveloper.ProjectArray})
	}
	else
	{
		$MQ('l:create.project.response',{result:'error',message:result.message});
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
	TiDeveloper.ProjectArray = [
		{id:0,name:'Project 1',date:'11/10/2009'},
		{id:1,name:'Project 2',date:'10/12/2009'},
		{id:2,name:'Project 3',date:'10/12/2009'}
	];
	$MQ('l:project.search.response',{totalRecords:3,'rows':TiDeveloper.ProjectArray})
	
})

//
// Show file dialog and send value
//
$MQL('l:show.filedialog',function()
{
	var files = Titanium.Desktop.openFiles({directories:true});
	var val = files.length ? files[0] : '';
	$MQ('l:file.selected',{value:val});
})

setTimeout(function()
{
	try
	{
		var irc = Titanium.Network.createIRCClient();
		irc.connect("irc.freenode.net",6667,"jeffhaynie1","jeff haynie","jeffhaynie",String(new Date().getTime()),function(cmd,channel,data)
		{
			$('#irc').append(cmd+"=>"+channel+':'+data+'\n');
		});
		irc.join("#titanium_dev");
		$('#irc_send').click(function()
		{
			irc.send('#titanium_dev',$('#irc_msg').val());
			$('#irc_msg').val('');
		})
	}
	catch(E)
	{
		alert("Exception: "+E);
	}
},1000);

