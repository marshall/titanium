ti.ready(function()
{
	// run db script
	$('#db_button').click(runDB);
	
	// reset db scripts
	$('#db_reset').click(function()
	{
		$('.code_results_db').empty();
	});
	function runDB()
	{
		var db = new ti.Database;
		$('.code_results_db').html('new db object...');	
		db.open('testdb');
		$('.code_results_db').append('<br/>opened db...');	
		db.execute("create table if not exists Test (value text)");
		$('.code_results_db').append('<br/>created table...');
		
		// delete records otherwise demos can get crazy
		db.execute('delete from Test');
		
		db.execute("insert into Test values(?)",['test 1']);
		$('.code_results_db').append('<br/>inserted record...');
		var rs = db.execute("select value from Test");
		$('.code_results_db').append('<br/>executed select...');

		while(rs.isValidRow())
		{
			alert('retreieved value = ' + rs.field(0))
			rs.next();
		}
		rs.close();
		db.close();
		$('.code_results_db').append('<br/>resources closed...');
		
	}
	// run win script
	$('#win_button').click(runWin);	
	function runWin()
	{
		var win = ti.Window.createWindow();
		win.setHeight(600);
		win.setWidth(800);
		win.setResizable(true);
		win.setURL('http://www.titaniumapp.com');
		win.open();
		win.show();
		
	}
	
	// run notification script
	$('#not_button').click(runNotification);
	function runNotification()
	{
		var notification = new ti.Notification;		
		notification.setTitle('My Title');
		notification.setMessage('It worked - pretty cool!');
		notification.setIcon('app://images/logo.png');
		notification.show();
		
	}

	// run tray script
	$('#tray_button').click(runTray);	
	function runTray()
	{
		// in PR1, win32 doesn't support PNG directly
		var trayIcon = (ti.platform == "win32")? "app://images/tray.ico":"app://images/tray.png";
	    var menu = ti.Menu.createTrayMenu(trayIcon,null,function(sysmenu)
	    {
	       if (ti.Window.currentWindow.isVisible())
	       {
	          ti.Window.currentWindow.hide();
	       }
	       else
	       {
	          ti.Window.currentWindow.show();
	       }
	    });
		
	}
	
	// run sound script
	$('#sound_button').click(runSound);
	function runSound()
	{
		var sound = ti.Media.createSound('app://audio/On.mp3');
		sound.play();
	}

	// run menu script
	$('#menu_button').click(runAppMenu);
	function runAppMenu()
	{
		var menu = ti.Menu.createAppMenu('Window');
		menu.addItem('My Menu', function()
		{
			// do nothing
		});
	}
	
	// run file script
	$('#file_button').click(runFiles);
	function runFiles()
	{
		ti.Desktop.openFiles(function(files)
		{
			alert('files length ' + files.length)
		})
		
	}
	
	// close window
	$('.close').click(function()
	{
		ti.Window.currentWindow.close();
	})
	
});

