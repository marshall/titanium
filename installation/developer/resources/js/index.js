//
// load projects when manage projects is clicked
//
var TiDeveloper  = {};
TiDeveloper.currentPage = 1;
TiDeveloper.init = false;

// holder var for all projects
TiDeveloper.ProjectArray = [
	{id:0,name:'Titanium',date:'11/01/2008'},
	{id:1,name:'Playtanium',date:'11/01/2008'},
	{id:2,name:'Tweetanium',date:'11/01/2008'},
	{id:3,name:'MS Excel',date:'11/01/2008'},
	{id:4,name:'MS Word',date:'11/01/2008'},
	{id:5,name:'MS Powerpoint',date:'11/01/2008'},
	{id:6,name:'Facebook',date:'11/01/2008'},
	{id:7,name:'Meebo',date:'11/01/2008'},
	{id:8,name:'Fluto',date:'11/01/2008'},
	{id:9,name:'Frankenfurter',date:'11/01/2008'},
	{id:10,name:'Doofus',date:'11/01/2008'},
	{id:11,name:'Janus',date:'11/01/2008'},
	{id:12,name:'Mint',date:'11/01/2008'},
	{id:13,name:'Installer',date:'11/01/2008'},
	{id:14,name:'Sample 2',date:'11/01/2008'},
	{id:15,name:'Sandbox',date:'11/01/2008'},
	{id:16,name:'Test App',date:'11/01/2008'},
	{id:17,name:'Test App 2',date:'11/01/2008'},
	{id:18,name:'My App',date:'11/01/2008'},
	{id:19,name:'Desktop 2',date:'11/01/2008'},
	{id:20,name:'Fred',date:'11/01/2008'}
	
];

//
//  create.project mock service
//
$MQL('l:create.project.request',function(msg)
{
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
})

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
	var val = (files['0'])?files['0']:'';
	$MQ('l:file.selected',{value:val});
})



