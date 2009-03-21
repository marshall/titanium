TiDeveloper.Sandbox = {};
TiDeveloper.Sandbox.lastTempDir = null;
TiDeveloper.Sandbox.apiSamples = [
{key:'select something',code:''},
{	key:'Titanium.App variables',
	code:"&lt;div&gt;Titanium.App.getID() = &lt;script&gt;document.write(Titanium.App.getID());&lt;/script&gt;\n&lt;/div&gt;\n\n"+
	"&lt;div&gt;Titanium.App.getName() = &lt;script&gt;document.write(Titanium.App.getName());&lt;/script&gt;\n&lt;/div&gt;\n\n"+
	"&lt;div&gt;Titanium.App.getVersion() = &lt;script&gt;document.write(Titanium.App.getVersion());&lt;/script&gt;\n&lt;/div&gt;\n\n"+
	"&lt;div&gt;Titanium.App.appURLToPath() = &lt;script&gt;document.write(Titanium.App.appURLToPath('app://index.html'));&lt;/script&gt;\n&lt;/div&gt;\n\n"	
}
];
$(document).ready(function()	
{
	$('#text_editor').markItUp(mySettings);
	$MQ('l:populate.api.selector',{rows:TiDeveloper.Sandbox.apiSamples})
});

TiDeveloper.Sandbox.setAPI = function()
{
	$('#text_editor').append($('select#api_selector option:selected').val()) 
}

$MQL('l:launch.sandbox',function(msg)
{
	var project = {};
	project.name = "sandbox";
	project.url = "http://www.titaniumapp.com";
	project.rootdir = Titanium.Filesystem.createTempDirectory().toString();
	project.dir = project.rootdir +'/'+ project.name;
	project.appid = 'com.titanium.sandbox';
	project.publisher = "Titanium";

	var jsLibs = {jquery:false,jquery_ui:false,prototype_js:false,scriptaculous:false,dojo:false,mootools:false,swf:false,yui:false};
	if ($('#jquery_js_sandbox').hasClass('selected_js'))
	{
		jsLibs.jquery = true;
	}
	if ($('#entourage_js_sandbox').hasClass('selected_js'))
	{
		jsLibs.entourage = true;
	}
	if ($('#prototype_js_sandbox').hasClass('selected_js'))
	{
		jsLibs.prototype_js = true;
	}
	if ($('#scriptaculous_js_sandbox').hasClass('selected_js'))
	{
		jsLibs.scriptaculous = true;
	}
	if ($('#dojo_js_sandbox').hasClass('selected_js'))
	{
		jsLibs.dojo = true;
	}
	if ($('#mootools_js_sandbox').hasClass('selected_js'))
	{
		jsLibs.mootools = true;
	}
	if ($('#swfobject_js_sandbox').hasClass('selected_js'))
	{
		jsLibs.swf = true;
	}
	if ($('#yahoo_js_sandbox').hasClass('selected_js'))
	{
		jsLibs.yahoo = true;
	}

	var outdir = TFS.getFile(project.rootdir,project.name);

	// remove last sandbox temp dir
	if (TiDeveloper.Sandbox.lastTempDir != null)
	{
		TiDeveloper.Sandbox.lastTempDir.deleteDirectory(true);
	}
	// record this temp dir for deletion next time
	TiDeveloper.Sandbox.lastTempDir = outdir;

	var guid = Titanium.Platform.createUUID();
	Titanium.Project.create(project.name,guid,'sandbox app',project.rootdir,project.publisher,project.url,null,jsLibs, $('#text_editor').val());
	TiDeveloper.Projects.launchProject(project,false)
})
