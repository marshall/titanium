TiFile = function (file, child)
{
	if (typeof(file) == "object")
	{
		if (child == null) {
			this.name = file.name;
			this.path = file.path;
			this.size = file.size;
			this.isDir = file.isDir;
			this.files = [];
	
			if (file.isDir) {
				for (var i = 0; i < file.files.length; i++)
				{
					this.files.push(new TiFile(file.files[i]));
				}
			}
		}
		// else {
		// 			if (typeof(child) == "string") {
		// 				this.name = child.split(TiFile.Separator);
		// 				this.path = TiFile.join(file.getPath(), child.getPath());
		// 			}
		// 		}
	} else if (typeof(file) == "string") {
		this.path = file;
		var tokens = file.split(TiFile.Separator);
		this.name = tokens[tokens.length-1];
		this.isDir = false;
		this.size = -1;
		this.files = [];
	}
};

TiFile.Separator = "/";
TiFile.UserHome = null;
TiFile.getUserHome = function ()
{
	if (TiFile.UserHome == null) {
		TiFile.UserHome = appcelerator.getUserHome();
	}
	return TiFile.UserHome;
}

TiFile.prototype.isDirectory = function ()
{
	return this.isDir;
};

TiFile.prototype.getName = function ()
{
	return this.name;
};

TiFile.prototype.getPath = function ()
{
	return this.path;
};

TiFile.prototype.getSize = function ()
{
	return this.size;
};

TiFile.prototype.getFileAtPath = function (path)
{
	var pathTokens = path.split("/");
	
	var file = this;
	for (var i = 0; i < pathTokens.length; i++)
	{
		var child = file.getFile(pathTokens[i]);
		if (child == null)
			return null;
		
		file = child;
	}
	
	return file;
};

TiFile.prototype.getFile = function (name)
{
	for (var i = 0; i < this.files.length; i++)
	{
		if (this.files[i].name == name) {
			return this.files[i];
		}
	}
	return null;
};

TiFile.prototype.getFiles = function ()
{
	return this.files;
};

TiFile.prototype.read = function ()
{
	return appcelerator.readFile(this.path);
};

TiFile.prototype.write = function (content)
{
	return appcelerator.writeFile(this.path, content);
};

TiFile.join = function ()
{
	var joined = "";
	for (var i = 0; i < arguments.length; i++)
	{
		joined += arguments[i];
		if (i < arguments.length - 1) {
			joined += TiFile.Separator;
		}
	}
	return joined;
};

TiProject = function (name, path, service)
{
	this.name = name;
	this.path = path;
	this.service = service;
	this.openFiles = [];
	
	var files = appcelerator.getFileTree(path);
	this.files = [];
	for (var i = 0; i < files.length; i++) {
		this.files.push(new TiFile(files[i]));
	}
};

TiProject.loadProject = function (node)
{
	var projectName = node.getAttribute("name");
	var projectPath= node.getAttribute("path");
	var projectService = node.getAttribute("service");
	
	var project = new TiProject(projectName, projectPath, projectService);
	
	for (var i = 0; i < node.childNodes.length; i++) {
		var child = node.childNodes[i];
		if (child.localName == "openFiles") {
			for (var j = 0; j < child.childNodes.length; j++) {
				var fileNode = child.childNodes[j];
				if (fileNode.localName == "file") {
					project.getOpenFiles().push(project.getFileAtPath(fileNode.getAttribute("path")));
				}
			}
		}
	}
	
	project.addToTree();
	return project;
};

TiProject.loadProjects = function ()
{	
	var projectsXml = new TiFile(TiFile.join(TiFile.getUserHome(), '.titanium', 'projects.xml'));
	console.debug("loading projects from " + projectsXml.getPath());
	
	var xml = projectsXml.read();
	
	var doc = Appcelerator.Titanium.Core.XML.parseFromString(xml);
	var stylesNode = doc.childNodes[0];
	var projects = [];
	
	for (var i = 0; i < stylesNode.childNodes.length; i++) {
		var child = stylesNode.childNodes[i];
		if (child.localName == "project") {
			projects.push(TiProject.loadProject(child));
		}
	}
	return projects;
};

TiProject.saveProjects = function (projects)
{
	var doc = Appcelerator.Titanium.Core.XML.newDocument();
	var root = doc.createElement("projects");
	
	for (var i = 0; i < projects.length; i++) {
		var dom = projects[i].toDOM(doc);
		root.appendChild(dom);
	}
	
	var xml = new XMLSerializer().serializeToString(doc);
	var projectsXml = new TiFile(TiFile.join(TiFile.getUserHome(), '.titanium', 'projects.xml'));
	projectsXml.write(xml);
};

TiProject.prototype.getFiles = function () {
	return this.files;
};

TiProject.prototype.getOpenFiles = function () {
	return this.openFiles;
};

TiProject.prototype.getPath = function () {
	return this.path;
};

TiProject.prototype.getName = function () {
	return this.name;
};

TiProject.prototype.getService = function () {
	return this.service;
};

TiProject.prototype.getFile = function (name)
{
	if (name.indexOf(TiFile.Separator) < 0) {
		for (var i = 0; i < this.files.length; i++)
		{
			if (this.files[i].name == name) {
				return this.files[i];
			}
		}
		return null;
	}
	else {
		return this.getFileAtPath(name);
	}
};

TiProject.prototype.getFileAtPath = function (path) {
	var pathTokens = path.split("/");
	
	var file = this;
	for (var i = 0; i < pathTokens.length; i++)
	{
		var child = file.getFile(pathTokens[i]);
		if (child == null)
			return null;
		
		file = child;
	}
	
	return file;
};

function _addToTree (dir, node)
{
	var files = dir.getFiles();
	for (var i = 0; i < files.length; i++) {
		var newNode = node.append({
			title: files[i].getName(),
			path: files[i].getPath(),
			size: files[i].getSize(),
			isFolder: files[i].isDirectory(),
			key: files[i].getPath()
		});
		
		if (files[i].isDirectory()) {
			_addToTree(files[i], newNode);
		}
	}
}

TiProject.prototype.addToTree = function () {

	var root = $("#sidetree").dynatree("getRoot");

	var projectNode = root.append({
		title: this.name,
		tooltip: "Titanium Project: " + this.name,
		isFolder: true,
		icon: "appcelerator_project.png",
		key: this
	});
	
	_addToTree(this, projectNode);
};

TiProject.prototype.toDOM = function (document) {
	var projectEl = document.createElement("project");
	projectEl.setAttribute("name", this.name);
	projectEl.setAttribute("service", this.service);
	projectEl.setAttribute("path", this.path);
	
	if (this.openFiles.length > 0) {
		var openFilesEl = document.createElement("openFiles");
		for (var i = 0; i < this.openFiles.length; i++) {
			var fileEl = document.createElement("file");
			fileEl.setAttribute("path", this.openFiles[i].getPath());
			openFilesEl.appendChild(fileEl);
		}
		projectEl.appendChild(openFilesEl);
	}
	
	return projectEl;
};