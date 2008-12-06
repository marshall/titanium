foo = {};

ti.ready(function()
{
	var notification_windows = 0;
	window.foo.Notification = function()
	{
		var width = 300, height = 60, notificationDelay = 3000;
		var myid = 'notification_'+(notification_windows++);
		var mywindow = ti.Window.createWindow({
				width:width,
				height:height,
				transparency:.99,
				usingChrome:'true',
				id:myid,
				visible:'false',
				url:'app://blank'
		});
		var self = this;
		var title = '', message = '', icon = '';
		mywindow.open();
		this.setTitle = function(value)
		{
			title = value;
		}
		this.setMessage = function(value)
		{
			message = value;
		}
		this.setIcon = function(value)
		{
			icon = value;
		}
		this.show = function()
		{
			mywindow.setX(screen.availWidth-width-20);
			mywindow.setY(10);
			mywindow.setTransparency(.99);
			mywindow.setURL('notification.html?title='+encodeURIComponent(title)+'&message='+encodeURIComponent(message)+'&icon='+encodeURIComponent(icon));
			mywindow.show(true);
		}
		this.hide = function()
		{
			mywindow.hide(true);
		}
		return this;
	}
});

