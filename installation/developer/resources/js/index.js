//
// load projects when manage projects is clicked
//
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
	if (Titanium.App.Properties.hasProperty("projects"))
	{
		var projects = Titanium.App.Properties.getList("projects");
		for (var c=0;c<projects.length;c++)
		{
			var project = projects[c];
			var id = project[0];
			var name = project[1];
			var date = project[2];
			var dir = project[3];
			
			TiDeveloper.ProjectArray.push({
				id: id,
				name:name,
				date:data,
				dir:dir
			});
		}
	}
})();

function save()
{
	Titanium.App.Properties.setList("projects",TiDeveloper.ProjectArray);
}

//
//  create.project mock service
//
$MQL('l:create.project.request',function(msg)
{
	/*
	setTimeout(function()
	{
		if (msg.payload.project_name == 'foo')
		{
			$MQ('l:create.project.response',{result:'error',message:'Project already exists - try again...'});
		}
		else
		{
			$MQ('l:create.project.response',{result:'success'});
		}
		
	},1000);
	*/
	
	// CHECK
	
	//TODO: do we check for existence of directory and fail?
	var result = Titanium.Project.create(msg.payload.project_name,msg.payload.project_location);
	if (result.success)
	{
		TiDeveloper.ProjectArray.push({
			id: String(TiDeveloper.ProjectArray.length),
			name: result.name,
			date: makeDate(),
			dir: result.basedir
		});
		$MQ('l:create.project.response',{result:'success'});
		save();
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
		var myNick = 'jeffhaynie1';
		var myName = "jeff haynie";
		var myNameStr = "jeffhaynie"

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

