var URL = 'http://publisher.titaniumapp.com/api/release-publish';
var TFS = Titanium.Filesystem;

var build_types = 
{
	'osx':['10.5_i386','10.5_i386','10.4_ppc'],
	'win32':['win32'],
	'linux':['32bit_i386','64bit_i386','32bit_ppc']
};

var guids = {
	'distribution':'7F7FA377-E695-4280-9F1F-96126F3D2C2A',
	'runtime':'A2AC5CB5-8C52-456C-9525-601A5B0725DA',
	'module':'1ACE5D3A-2B52-43FB-A136-007BD166CFD0'
};

$(function()
{
	var files = [];

	function updateFilesDisplay()
	{
		var display = "<ul>";
		for (var i = 0; i < files.length; i++)
		{
			var f = files[i];
			display += "<li>";
			display += f.path+" ("+f.type+") ("+f.os+") ("+f.build_type+")";
			display += "</li>";
		}
		display += "</ul>";
		$("#files_display").html(display);
	}

	function addFile()
	{
		Titanium.UI.openFiles(
		function(result)
		{
			for (var i = 0; i < result.length; i++)
			{
				var new_file = Object();
				new_file.path = result[i];
				new_file.type = $('#type').val();
				new_file.os = $('#os').val();
				new_file.build_type = $('#build_type').val();
				files.push(new_file);
			}
			updateFilesDisplay();
		}, 
		{
			directories:false,
			files:true,
			multiple:true
		});
	}

	function clearFiles()
	{
		files = []
		updateFilesDisplay();
	}

	$('#filepicker').click(addFile);
	$('#clearfiles').click(clearFiles);

	function setStatus(msg)
	{
		$('#status').html(msg);
	}

	function sendFile(file)
	{
		$('#status').removeClass('error').fadeIn();

		var type = file.type;
		var os = file.os;
		var build_type = file.build_type;
		var path = file.path;

		try {
		setStatus('Preparing distribution:' + path);
		var guid = guids[type];
		Titanium.API.debug("1");
		var tmp = TFS.createTempDirectory();
		var manifest = TFS.getFile(tmp,'timanifest');
		var from = TFS.getFile(path);
		var name = null;
		var version = null;
		var toks = from.name().split('-');
		if (guid == guids.module)
		{
			name = toks[1];
			version = toks[2];
		}
		else if (guid==guids.runtime)
		{
			name = 'runtime';
			version = toks[1];
		}
		else
		{
			name = 'installer';
			version = toks[2];
		}
		var idx = version.lastIndexOf('.');
		if (idx > 0) version = version.substring(0,idx);
		from.copy(tmp);
		var contents = {'name':name,'guid':guid,'version':version,'build_type':build_type,'os':os,'filename':from.name()};
		alert(swiss.toJSON(contents))
		manifest.write(swiss.toJSON(contents));

		setStatus('Sending:' + path);
		var xhr = Titanium.Network.createHTTPClient();
		xhr.onreadystatechange = function()
		{
			if (this.readyState == 4)
			{
				setStatus('Finished sending:' + path);
				if (this.status == 200)
				{
					sendNextFile();
				}
				else
				{
					$('#status').addClass('error').html('Error publishing');
				}
			}
		};
		// xhr.onsendstream = function(sent,total,remaining)
		// {
		// 	setStatus('Sending...'+sent+'K of '+total+'K, remaining '+remaining+'K');
		// };
		var ts = new Date().getTime();
		var secret = hex_md5($('#secret').val()+"$"+ts);
		var url = URL+"?activate=1&secret="+encodeURIComponent(secret)+"&ts="+ts;
		xhr.open("POST",url);
		xhr.sendDir(tmp);
		} catch (e) {
			alert(e);
		}
	}
	function sendNextFile()
	{
		if (files.length > 0)
		{
			sendFile(files.pop());
			updateFilesDisplay();
		}
	}
	$('button').click(function()
	{
		sendNextFile();
	});


	$('#os').change(function(e)
	{
		var n = build_types[e.srcElement.options[e.srcElement.selectedIndex].value];
		var bt = $('#build_type').get(0);
		bt.options.length = 0;
		for (var c=0;c<n.length;c++)
		{
			bt.options[c] = new Option(n[c],n[c]);
		}
	});
})
