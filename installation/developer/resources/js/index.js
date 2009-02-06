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

// show initial project list
$MQ('l:project.list.response',{count:2,'rows':[{name:'Titanium',date:'11/1/2009'},{name:'Tweetanium',date:'10/12/2009'}]})

//
// project search request
//
$MQL('l:project.list.request',function(msg)
{
	$MQ('l:project.list.response',{count:3,'rows':[{name:'Titanium',date:'11/1/2009'},{name:'Tweetanium',date:'10/12/2009'}]})
	
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



