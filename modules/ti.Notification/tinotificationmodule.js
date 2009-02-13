var notification_windows = 0;

Titanium.api.debug("setting Titanium.notification..");

Titanium.Notification = function()
{
  var width = 300, height = 60, notificationDelay = 3000;
  if (Titanium.platform == "win32") {
    height = 80;  
  }
  
  var showing = false;
  var myid = 'notification_'+(notification_windows++);
  var transparency = .99;
  //if (ti.platform == "win32") {
  //  transparency = 1.0;  
  //}
  
  var mywindow = Titanium.Window.createWindow({
      width:width,
      height:height,
      transparency:transparency,
      usingChrome:true,
      id:myid,
      visible:false,
      url:'app://blank'
  });
  var self = this;
  var title = '', message = '', icon = '';
  var hideTimer = null;
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
  this.setDelay = function(value)
  {
    notificationDelay = value;
  }
  this.show = function(animate,autohide)
  {
    showing = true;
    if (hideTimer)
    {
      clearTimeout(hideTimer);
    }
    animate = (animate==null) ? true : animate;
    autohide = (autohide==null) ? true : autohide;
    mywindow.setX(screen.availWidth-width-20);
    if (Titanium.platform == "osx" || Titanium.platform == 'linux') {
      mywindow.setY(10);
    } else if (Titanium.platform == "win32") {
      mywindow.setY(screen.availHeight-height-10);  
    }
    
    mywindow.setTransparency(.99);
    mywindow.setURL('app://tinotification.html?title='+encodeURIComponent(title)+'&message='+encodeURIComponent(message)+'&icon='+encodeURIComponent(icon));
    mywindow.show(animate);
    if (autohide)
    {
      hideTimer = setTimeout(function()
      {
        self.hide();
      },notificationDelay + (animate ? 1000 : 0));
    }
  }
  this.hide = function(animate)
  {
    animate = (animate==null) ? true : animate;
    showing = false;
    if (hideTimer)
    {
      clearTimeout(hideTimer);
      hideTimer=null;
    }
    mywindow.hide(animate);
  }
  return this;
}