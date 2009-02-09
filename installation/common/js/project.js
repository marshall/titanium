
if (typeof(Titanium)=='undefined') Titanium = {};

var TFS = Titanium.Filesystem;

Titanium.Project = 
{
	bundle:function()
	{
		
	},
	create:function(name,dir)
	{
		var outdir = TFS.getFile(dir,name);
		if (!outdir.isDirectory())
		{
			outdir.createDirectory(true);
		}
		var normalized_name = name.replace(' ','_');
		// write out the TIAPP.xml
		var tiappxml = this.XML_PROLOG;
		tiappxml+=this.makeEntry('id','com.yourdomain.'+normalized_name);
		tiappxml+=this.makeEntry('name',name);
		tiappxml+=this.makeEntry('version','1.0');
		tiappxml+=this.makeEntry('copyright','2009 by YourCompany');
		tiappxml+="<window>\n";
		tiappxml+=this.makeEntry('id','initial');
		tiappxml+=this.makeEntry('title',name);
		tiappxml+=this.makeEntry('url','app://index.html');
		tiappxml+=this.makeEntry('width','700');
		tiappxml+=this.makeEntry('maxwidth','3000');
		tiappxml+=this.makeEntry('minwidth','0');
		tiappxml+=this.makeEntry('height','800');
		tiappxml+=this.makeEntry('maxheight','3000');
		tiappxml+=this.makeEntry('minheight','0');
		tiappxml+=this.makeEntry('fullscreen','false');
		tiappxml+=this.makeEntry('resizable','true');
		tiappxml+=this.makeEntry('chrome','false');
		tiappxml+=this.makeEntry('maximizable','true');
		tiappxml+=this.makeEntry('minimizable','true');
		tiappxml+=this.makeEntry('closeable','true');
		tiappxml+="</window>\n";
		tiappxml+=this.XML_EPILOG;
		var ti = TFS.getFile(outdir,'tiapp.xml');
		ti.write(tiappxml);
		var resources = TFS.getFile(outdir,'resources');
		resources.createDirectory();
		var index = TFS.getFile(resources,'index.html');
		index.write('<html>\n<head>\n</head>\n<body>\nHello,world\n</body>\n</html>');
		return true;
	},
	makeEntry:function(key,value,attrs)
	{
		var str = '<' + key;
		if (attrs)
		{
			str+=' ';
			var values = [];
			for (name in attrs)
			{
				var v = attrs[name];
				if (v)
				{
					values.push_back(name + '=' + '"' + v + '"');
				}
			}
			str+=values.join(' ');
		}
		str+='>' + value + '</'+key+'>\n';
		return str;
	}
};

Titanium.Project.XML_PROLOG = "<?xml version='1.0' encoding='UTF-8'?>\n" +
	"<ti:app xmlns:ti='http://ti.appcelerator.org'>\n";
	
Titanium.Project.XML_EPILOG = "</ti:app>";
	
