
ti.File = ti.gearsPlugin.create('ti.fileclass');
ti.Path = {};
ti.Path.Resources = TiNative.getResourcePath();
ti.Path.Separator = ti.File.pathSeparator;

ti.Path.resource = function(path)
{
	ti.debug("resource: " + path);
	
	if (typeof(path) == 'array') {
		var resourcePath = ti.Path.Resources;
		for (var i = 0; i < path.length; i++) {
			resourcePath = ti.Path.join(resourcePath, path[i]);
		}
		return resourcePath;
	}
	
	return ti.Path.join(ti.Path.Resources, path);
}

ti.Path.join = function ()
{
	ti.debug("join: " + arguments);
	
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