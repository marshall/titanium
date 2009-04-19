Titanium.UI.currentWindow.setX(0);
Titanium.UI.currentWindow.setY(0);
var resultArray = [];
var totalTests = 0;
var totalPassed = 0;
var totalFailed = 0;
var totalPending = 0;
var totalNotExecuted = 0;

//
// Get results when tests are complete
//
$MQL('l:titanium_test_results',function(msg)
{
	resultArray = msg.payload.rows;
	totalTests = msg.payload.test_count;
	totalPassed = msg.payload.passed;
	totalFailed = msg.payload.failed;
	totalPending = msg.payload.pending;
	totalNotExecuted = msg.payload.not_executed
	
});


//
// Handle row selection
//
$MQL('l:rowselect',function(msg)
{
	var name = msg.payload.name;
	for (var i=0;i<resultArray.length;i++)
	{
		if (resultArray[i].name == name)
		{
			var detail = resultArray[i].detail
			if (!resultArray[i].detail) detail = 'no detail';
			$MQ('l:detail',{detail:detail,log:resultArray[i].log});
			break;
		}
	}
});


var shotArray = [];
var shotIndex = 0;
var currentView = 'results';

//
// set key handles for screenshot view
//
$(document).keyup(function(e)
{
	if (currentView == 'shots')
	{
		var code = e.keyCode;
		if (code == 80)
		{
			$MQ('l:shot',{val:'pass'})
		}
		else if (code == 70)
		{
			$MQ('l:shot',{val:'fail'})
		}
		else if (code == 78)
		{
			$MQ('l:next');
		}
		else if (code == 86)
		{
			$MQ('l:prev');
		}
	}
})
//
// Show Screenshot View
//
$MQL('l:showshots',function(msg)
{
	// resize and reset
	Titanium.UI.currentWindow.setWidth(1000);
	Titanium.UI.currentWindow.setHeight(680);

	// only load array once
	if (shotArray.length == 0)
	{
		// create shot array
		var path = Titanium.Filesystem.getFile(Titanium.App.appURLToPath('app://results')) + Titanium.Filesystem.getSeparator();
		for (var i=0;i<resultArray.length;i++)
		{
			if (resultArray[i].result == 'pending')
			{
				shotArray.push({idx:i,html:'<div class="shot_image"><img  style="height:500px" width="900px" src="file://' + path + resultArray[i].name + '.png"/></div>',img:(path + resultArray[i].name)});
			}
		}
		setShotMetadata();
		
	}
	currentView = 'shots';
});

//
// Click handler for Screenshot - goto fullscreen
//

$(document).ready(function()
{
	$('#screenshot_list').click(function()
	{
		var img = '<img src="file://'+shotArray[shotIndex].img + '.png"/>';
		var w = Titanium.UI.createWindow("app://screenshot_fullscreen.html");

		// open window
		w.open();
		w.setFullScreen(true);
		w.setTopMost(true);
		
		// let window fully load then start test
		setTimeout(function()
		{
		    w.window.document.body.innerHTML = img;

		},300);
	})
});

//
// Show Results Table (resize)
//
$MQL('l:showtable',function(msg)
{
	// delay a bit for UI transition
	setTimeout(function()
	{
		Titanium.UI.currentWindow.setWidth(700);
		Titanium.UI.currentWindow.setHeight(500);
		currentView = 'results'
		
	},100);
});

//
// Show Next Screenshot
//
$MQL('l:next',function()
{
	shotIndex++;
	if (shotIndex == shotArray.length)
	{
		shotIndex=0;
	}
	setShotMetadata();
});


//
// Show Prev Screenshot
//
$MQL('l:prev',function()
{
	shotIndex--
	if (shotIndex == -1)
	{
		shotIndex= (shotArray.length -1)
	}
	setShotMetadata();
});

//
//  Capture screenshot result
//
$MQL('l:shot',function(msg)
{
	// screenshot passed
	if (msg.payload.val == 'pass')
	{
		if (resultArray[shotArray[shotIndex].idx].result == 'pending')
		{
			totalPassed++;
			totalPending--;
		}
		else if (resultArray[shotArray[shotIndex].idx].result == 'failed')
		{
			totalPassed++;
			totalFailed--;
		}
		resultArray[shotArray[shotIndex].idx].result = 'passed'
		
	}
	// screenshot failed
	else
	{
		if (resultArray[shotArray[shotIndex].idx].result == 'pending')
		{
			totalFailed++;
			totalPending--;
		}
		else if (resultArray[shotArray[shotIndex].idx].result == 'passed')
		{
			totalFailed++;
			totalPassed--;
		}
		resultArray[shotArray[shotIndex].idx].result = 'failed'

	}
	// record notes about screenshot
	resultArray[shotArray[shotIndex].idx].detail = $('#screenshot_note').val();
	
	// go to next shot
	$MQ('l:next');
});

//
// Re-fire results after screenshots have been reviewed
//
$MQL('l:showtable',function(msg)
{
	$MQ('l:titanium_test_results',{rows:resultArray,'test_count':totalTests,passed:totalPassed,failed:totalFailed,pending:totalPending,'not_executed':totalNotExecuted});
	
})

//
// helper function to set screenshot meta data based on the current screenshot
//
function setShotMetadata()
{
	$('#test_name').html(resultArray[shotArray[shotIndex].idx].test + '<div style="float:right;font-size:11px;position:relative;top:-5px" class="'+resultArray[shotArray[shotIndex].idx].result+'">' +resultArray[shotArray[shotIndex].idx].result+ '</div><span style="float:right;margin-right:5px" class="small_blue"> showing: ' +(shotIndex+1) + ' of ' + shotArray.length + '</span><div style="clear:both"></div>');
	$('#screenshot_list').html(shotArray[shotIndex].html);

	if (resultArray[shotArray[shotIndex].idx].detail)
	{
		$('#screenshot_note').val(resultArray[shotArray[shotIndex].idx].detail)
	}
	else
	{
		$('#screenshot_note').val('');
	}

};

