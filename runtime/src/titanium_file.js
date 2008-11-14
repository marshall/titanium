
ti.File = ti.gearsPlugin.create('ti.fileclass');
ti.Path = {};
ti.Path.Resources = TiNative.getResourcePath();
ti.Path.Separator = ti.File.pathSeparator;

ti.Path.resource = function(path)
{
	var resourcePath = ti.Path.Resources;
	if (typeof(path) == 'array') {
		for (var i = 0; i < path.length; i++) {
			resourcePath = ti.Path.join(resourcePath, path[i]);
		}
		return resourcePath;
	}
	
	for (var i = 0; i < arguments.length; i++) {
		var token = arguments[i].replace(/[\/]+/g, ti.Path.Separator);
		resourcePath = ti.Path.join(resourcePath, arguments[i]);
	}
	
	return resourcePath;
}

ti.Path.join = function ()
{	
	var joined = "";
	
	for (var i = 0; i < arguments.length; i++)
	{
		var tok = arguments[i];
		joined += tok;
		
		if (i < arguments.length - 1 &&
			tok.lastIndexOf(ti.Path.Separator) != tok.length - 1)
		{
			joined += ti.Path.Separator;
		}
	}
	
	return joined;
};