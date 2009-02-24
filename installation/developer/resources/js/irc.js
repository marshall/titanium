TiDeveloper.IRC = {};
TiDeveloper.IRC.nick = null;
TiDeveloper.IRC.channel = "#titanium_app";
TiDeveloper.IRC.count = 0;
TiDeveloper.IRC.ircClient = null;
TiDeveloper.IRC.messageCount = 0;

//
//  Initialization message - setup all initial states
//
$MQL('l:app.compiled',function()
{
	// load or initialize IRC data
	db.transaction(function(tx) 
	{
	   tx.executeSql("SELECT nick FROM IRC", [], function(tx,result) 
	   {
		   for (var i = 0; i < result.rows.length; ++i) 
		   {
                var row = result.rows.item(i);
				TiDeveloper.IRC.nick = row['nick'];
				break;
			}
	   }, function(tx, error) 
	   {
	       tx.executeSql("CREATE TABLE IRC (id REAL UNIQUE, nick TEXT)");
		   tx.executeSql("INSERT INTO IRC (id, nick) values (?,?)",[1,Titanium.Platform.username])
		   TiDeveloper.IRC.nick = Titanium.Platform.username;
	   });
	});

	var currentSelectionIdx = -1;
	var savedPossibilities = null;
	var savedName = null;
	
	// this code supports using the tab to cycle
	// through handles to find an existing user when
	// entering in a handle in the text box
	$('#irc_msg').keydown(function(e)
	{
		if (e.keyCode!=9)
		{
			currentSelectionIdx=-1;
			savedPossibilities=null;
			savedName=null;
			return;
		}
		var prefix = $('#irc_msg').val();
		if (prefix.length > 0 && savedName && savedName==prefix)
		{
			if (savedPossibilities && currentSelectionIdx!=-1)
			{
				if (currentSelectionIdx + 1 >= savedPossibilities.length)
				{
					currentSelectionIdx = -1;
				}
				var match = savedPossibilities[++currentSelectionIdx];
				savedName = match + ': ';
				$('#irc_msg').val(savedName);
				return false;
			}
		}
		var users = $('#irc_users div');
		savedPossibilities = [];
		savedName = null;
		currentSelectionIdx = -1;
		for (var c=0;c<users.length;c++)
		{
			var name = users.get(c).innerHTML;
			var idx = name.indexOf(prefix);
			if (idx==0)
			{
				savedPossibilities.push(name);
			}
		}
		if (savedPossibilities.length > 0)
		{
			currentSelectionIdx = 0;
			savedName = savedPossibilities[0] + ': ';
			$('#irc_msg').val(savedName);
			return false;
		}
	});
});


//
// Format IRC nickname
//
TiDeveloper.IRC.formatNick =  function(name)
{
	return name.replace(' ','_');
}

//
// Setup focus listener
//
$MQL('l:tideveloper.windowFocus',function(msg)
{
	var focus = msg.payload.focus
	if (focus == false)
	{
		TiDeveloper.IRC.messageCount = 0;
	}
	else
	{
		TiDeveloper.IRC.messageCount = 0;
		Titanium.UI.setBadge('');
	}
})

//
// Setup connectivity listener
//
$MQL('l:tideveloper.network',function(msg)
{
	if (msg.payload.online == true)
	{
		TiDeveloper.IRC.initialize()	
	}
	else
	{
		TiDeveloper.IRC.ircClient.disconnect();
		TiDeveloper.IRC.ircClient = null;
		$('#irc').html('<div style="color:#aaa">you are offline</div>');
		$('#irc_users').empty();
	}
});

TiDeveloper.IRC.updateNickInDB = function(username)
{
	// update database
    db.transaction(function (tx) 
    {
        tx.executeSql("UPDATE IRC set nick = ? WHERE id = 1", 
		[username]);
    });

	TiDeveloper.IRC.nick = username;
};

//
// Initialize Chat
//
var userSetNick = null;

TiDeveloper.IRC.initialize = function()
{
	try
	{
		var username = TiDeveloper.IRC.formatNick(TiDeveloper.IRC.nick);
		var nick_counter = 1;
		var setNicknameAttempted = false;
		
		// initial message
		$('#irc').append('<div style="color:#aaa">you are joining the <span style="color:#42C0FB">Titanium Developer</span> IRC channel <span style="color:#42C0FB">'+TiDeveloper.IRC.channel+'</span>. one moment...</div>');
		TiDeveloper.IRC.ircClient = Titanium.Network.createIRCClient();
		TiDeveloper.IRC.ircClient.connect("irc.freenode.net",6667,username,username,username,String(new Date().getTime()),function(cmd,channel,data,nick)
		{
			var time = TiDeveloper.getCurrentTime();

			// switch on command
			switch(cmd)
			{	
				case '433':
				{
					alert(userSetNick)
					// user is trying to set their nickname
					if (userSetNick != null)
					{
						if ($('.' + userSetNick).length == 0)
						{
							$('#irc').append('<div style="color:#aaa;margin-bottom:8px">' + username + ' is now known as <span style"color:#42C0FB">'+userSetNick+'</span></div>');
							$('.'+username).html('');
							$('#irc_users').append('<div class="'+userSetNick+'" >'+userSetNick+'</div>');
							username = userSetNick;
						
							// update database
						    TiDeveloper.IRC.updateNickInDB(username);
						}
						else
						{
							$('#irc').append('<div style="color:#aaa;margin-bottom:8px">' + userSetNick + ' is already taken. try again.</div>');
							return;
						}
					}
					
					// we tried to set the nick, but it's taken
					// so increment
					else if (setNicknameAttepted == true)
					{
						username = TiDeveloper.IRC.formatNick(TiDeveloper.IRC.nick + (++nick_counter));
					}
					// initial attempt to set nickname
					else
					{
						username = TiDeveloper.IRC.formatNick(TiDeveloper.IRC.nick);
						setNicknameAttempted = true;
					}
					// try again with a new nick
					TiDeveloper.IRC.ircClient.setNick(username);
					break;
				}
				case 'NICK':
				{
					// this is good and indicates the NICK was accepted
				    TiDeveloper.IRC.updateNickInDB(username);
					$('.'+username).html('');
					$('#irc_users').append('<div class="'+username+'" >'+username+'</div>');
					break;
				}
				case 'NOTICE':
				case 'PRIVMSG':
				{
					if (nick && nick!='NickServ')
					{
						if (TiDeveloper.windowFocused == false)
						{
							TiDeveloper.IRC.messageCount++;
							Titanium.UI.setBadge(String(TiDeveloper.IRC.messageCount));
						}
						var rawMsg = String(channel.substring(1,channel.length));
						var urlMsg = TiDeveloper.formatURIs(rawMsg);
						var str = username + ":";
						var msg = urlMsg.replace(username +":","<span style='color:#42C0FB'>" + username + ": </span>");
						if (TiDeveloper.windowFocused == false && msg.indexOf(str) != -1)
						{
							var notification = Titanium.Notification.createNotification(window);
							notification.setTitle("New Message");
							notification.setMessage(msg);
							notification.setIcon("app://images/information.png");
							notification.show();
						}	
						$('#irc').append('<div style="color:#42C0FB;font-size:14px;float:left;margin-bottom:8px;width:90%">' + nick + ': <span style="color:white;font-family:Arial;font-size:12px">' + msg + '</span></div><div style="float:right;color:#ccc;font-size:11px;width:10%;text-align:right">'+time+'</div><div style="clear:both"></div>');
					}
					else if (nick=='NickServ' && (channel.indexOf('This nickname is registered')>=0 || channel.indexOf('Invalid password for')>=0))
					{
						$('#irc').append('<div style="color:#aaa;margin-bottom:8px">' + data + ' is already taken. trying another variation...</div>');
						username = TiDeveloper.IRC.formatNick(data + (++nick_counter));
						setNicknameAttempted = true;
						// try again with a new nick
						TiDeveloper.IRC.ircClient.setNick(username);
					}
					break;
				}
				case '366':
				{					
					var users = TiDeveloper.IRC.ircClient.getUsers(TiDeveloper.IRC.channel);
					$MQ('l:online.count',{count:users.length});
					TiDeveloper.IRC.count = users.length;
					for (var i=0;i<users.length;i++)
					{
						$('#irc_users').append('<div class="'+users[i].name+'">'+users[i].name+'</div>');
					}
				}
				case 'JOIN':
				{
					if (nick.indexOf('freenode.net') != -1)
					{
						return;
					}
					
					if (nick == username)
					{
						$('#irc').append('<div style="color:#aaa;margin-bottom:20px"> you are now in the room. your handle is: <span style="color:#42C0FB">'+username+'</span>.  You can change your handle using: <span style="color:#42C0FB">/nick new_handle</span></div>');
						break;
					}
					else
					{
						TiDeveloper.IRC.count++;
					}
					$('#irc').append('<div style="color:#aaa;margin-bottom:8px">' + nick + ' has joined the room </div>');
					$('#irc_users').append('<div class="'+nick+'" >'+nick+'</div>');
					$MQ('l:online.count',{count:TiDeveloper.IRC.count});
					break;
				}
				case 'QUIT':
				case 'PART':
				{
					$('#irc').append('<div style="color:#aaa;margin-bottom:8px">' + nick + ' has left the room </div>');
					$('.'+nick).html('');
					TiDeveloper.IRC.count--;
					$MQ('l:online.count',{count:TiDeveloper.IRC.count});
					break;
				}
			}
			$('#irc').get(0).scrollTop = $('#irc').get(0).scrollHeight;
		});

		TiDeveloper.IRC.ircClient.join(TiDeveloper.IRC.channel);
	}
	catch(E)
	{
//		alert("Exception: "+E);
	}
	
}

$MQL('l:send.irc.msg',function()
{
	if (TiDeveloper.online == true)
	{
		TiDeveloper.IRC.currentSelection = null;
		var time = TiDeveloper.getCurrentTime();
		var urlMsg = TiDeveloper.formatURIs($('#irc_msg').val());
		var rawMsg = $('#irc_msg').val()
		if (rawMsg.indexOf('/nick') == 0)
		{
			userSetNick = rawMsg.split(' ')[1];
		}
		TiDeveloper.IRC.ircClient.send(TiDeveloper.IRC.channel,rawMsg);
		$('#irc').append('<div style="color:#ff9900;font-size:14px;float:left;margin-bottom:8px;width:90%">' + TiDeveloper.IRC.nick + ': <span style="color:white;font-size:12px;font-family:Arial">' + urlMsg + '</span></div><div style="float:right;color:#ccc;font-size:11px;width:10%;text-align:right">'+time+'</div><div style="clear:both"></div>');
		$('#irc_msg').val('');
		$('#irc').get(0).scrollTop = $('#irc').get(0).scrollHeight;
		
	}
});


