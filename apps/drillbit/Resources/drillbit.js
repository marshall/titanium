
var TFS = Titanium.Filesystem;
var TA  = Titanium.App;

var current_test_load = null;
var current_test = null;

function update_status(msg,hide)
{
	$('#status').html(msg).css('visibility','visible');
	if (hide)
	{
		$('#status').fadeOut(5000);
	}
}
function test_status(name,classname)
{
	var el = $('#test_'+name+' td.status');
	el.html(classname);
	el.removeClass('untested').removeClass('failed')
	  .removeClass('passed').addClass(classname.toLowerCase());
}

function describe(description,test)
{
	current_test_load.description = description;
	current_test_load.test = test;
	current_test_load.timeout = test.timeout || 5000;
	current_test_load = null;
}

function make_function(f,scope)
{
	if (typeof(f)=='function')
	{
		if (typeof(scope)=='undefined')
		{
			return '(' + String(f) + ')();\n';
		}
		else
		{
			var expr = '(function(){var _scope = ' + scope + ';\n';
			expr+='(' + String(f) + ').call(_scope,_scope);\n';
			expr+='})();\n';
			return expr;
		}
	}
	return '';
}

function show_test_details(name)
{
	alert(name);
	//tests[name];
}

var tests = {};
window.onload = function()
{
	var test_names = [];
	
	var test_dir = TFS.getFile(TA.appURLToPath('app://tests'));
	var test_harness_dir = TFS.getFile(TA.appURLToPath('app://test_harness'));
	var results_dir = TFS.getFile(TA.appURLToPath('app://test_results'));
	var drillbit_funcs = TFS.getFile(TA.appURLToPath('app://drillbit_func.js')).read();
	var dir_list = test_dir.getDirectoryListing();
	
	results_dir.createDirectory();
	
	for (var c=0;c<dir_list.length;c++)
	{
		var f = dir_list[c];
		if (!f.isDirectory())
		{
			continue;
		}
		var name = f.name();
		var ext = f.extension();
		name = name.replace('.'+ext,'');
		var dir = TFS.getFile(test_dir,name);
		var jsfile = TFS.getFile(dir,name+'.js');
		if (!jsfile.exists())
		{
			continue;
		}
		var entry = tests[name];
		if (!entry)
		{
			entry = {name:name,dir:dir};
			tests[name] = entry;
			test_names.push(name);
		}
		entry[ext] = f;
		current_test_load = entry;
		eval(jsfile.read());
	}

	test_names.sort();
	
	
	var table = '<table>' +
		'<tr>'+
			'<th>Include</th>'+
			'<th>Test</th>'+
			'<th>Description</th>'+
			'<th>Result</th>'+
		'</tr>'+
	'';
		
	for (var c=0;c<test_names.length;c++)
	{
		var name = test_names[c];
		var entry = tests[name];
		table+=
		'<tr id="test_'+name+'" class="test">'+
			'<td class="check"><div class="checkbox checked"></div></td>'+
			'<td class="name">'+name+'</td>'+
			'<td class="description">'+entry.description+'</td>'+
			'<td class="status untested" onclick="show_test_details(\'' + name + '\')">Untested</td>'+
		'</tr>';
	}
	
	$('#table').html(table);
	$('#table .checkbox').click(function()
	{
		if ($(this).is('.checked'))
		{
			$(this).removeClass('checked');
		}
		else
		{
			$(this).addClass('checked');
		}
	});
	
	// get the runtime dir
	var runtime_dir = TFS.getFile(Titanium.Process.getEnv('KR_RUNTIME'));
	var modules_dir = TFS.getFile(TFS.getApplicationDirectory(),'modules');
	
	var run_button = $('#run').get(0);

	// create the test harness directory
	if (!test_harness_dir.exists())
	{
		test_harness_dir.createDirectory();
	}
	
	// create app structure
	var app = Titanium.createApp(runtime_dir,test_harness_dir,'test_harness','CF0D2CB7-B4BD-488F-9F8E-669E6B53E0C4',false);
	
	var executing_tests = null;
	var tiapp_backup = TFS.getFile(app.base,'_tiapp.xml');
	var manifest_backup = TFS.getFile(app.base,'_manifest');
	
	var mymanifest = TFS.getFile(TFS.getApplicationDirectory(),'manifest');
	var manifest = TFS.getFile(app.base,'manifest');
	var manifest_contents = mymanifest.read();

	manifest_contents.replace('#appname:UnitTest','#appname:UnitTest Harness');
	manifest_contents.replace('#appid:com.titaniumapp.unittest.driver','#appid:com.titaniumapp.unittest');
	manifest_contents.replace('#guid:D83B08F4-B43B-4909-9FEE-336CDB44750B','#guid:CF0D2CB7-B4BD-488F-9F8E-669E6B53E0C4');
	manifest_contents.replace('#desc:Unit Test Driver','#desc:Unit Test Harness');

	manifest.write(manifest_contents);
	
	var tiapp = Titanium.Project.writeTiXML('com.titaniumapp.unittest','test_harness','Appcelerator','http://titaniumapp.com',app.base);
	
	tiapp.copy(tiapp_backup);
	manifest.copy(manifest_backup);

	var ti_contents = tiapp.read();
	var non_visual_ti = ti_contents.replace('<visible>true</visible>','<visible>false</visible>');
	
	// copy in our user script which is the driver
	var user_scripts_dir = TFS.getFile(app.resources,'userscripts');
	user_scripts_dir.createDirectory();
	
	var tests_started = 0;
	
	function run_test(entry)
	{
		var dir = app.resources;
		
		// make sure we cleanup
		var list = user_scripts_dir.getDirectoryListing();
		for (var c=0;c<list.length;c++)
		{
			var lf = list[c];
			if (lf.isFile())
			{
				lf.deleteFile();
			}
		}
		

		// we always initially override
		tiapp_backup.copy(tiapp);
		manifest_backup.copy(manifest);

		// make sure we have an index file always
		var tofile = TFS.getFile(dir,'index.html');
		var html = '<html><head><script type="text/javascript"></script></head><body>Running...'+entry.name+'</body></html>';
		tofile.write(html);
		
		var html_found = false;
		function strip_extension(f)
		{
			var name = f.name();
			return name.replace('.'+f.extension(),'');
		}
		
		var files = entry.dir.getDirectoryListing();
		for (var c=0;c<files.length;c++)
		{
			var src = files[c];
			var same_as_testname = strip_extension(src) == entry.name;
			if (src.name() == entry.name+'.js')
			{
				continue;
			}
			if (same_as_testname)
			{
				var ext = src.extension();
				switch(ext)
				{
					case 'xml':
					{
						var tofile = TFS.getFile(app.base,'tiapp.xml');
						src.copy(tofile);
						break;
					}
					case 'html':
					{
						var tofile = TFS.getFile(dir,'index.html');
						src.copy(tofile);
						html_found = true;
						break;
					}
					case 'usjs':
					{
						var tofile = TFS.getFile(user_scripts_dir,entry.name+'.js');
						src.copy(tofile);
						break;
					}
					case 'manifest':
					{
						var tofile = TFS.getFile(app.base,'manifest');
						src.copy(tofile);
						break;
					}
				}
			}
			else
			{
				// just copy the file otherwise
				src.copy(dir);
			}
		}

		// make it non-visual if no HTML found
		if (!html_found)
		{
			tiapp.write(non_visual_ti);
		}
		
		var us = '// ==UserScript==\n';
		us+='// @name	Titanium App Tester\n';
		us+='// @author	Appcelerator\n';
		us+='// @description	Titanium Tests\n';
		us+='// @include	app://com.titaniumapp.unittest/index.html\n';
		us+='// @version 	0.1\n';
		us+='// ==/UserScript==\n\n';
		
		us+=drillbit_funcs + '\n';
		us+="TitaniumTest.NAME = '"+entry.name+"';\n";
		
		us+="try{";
		us+=make_function(entry.test.before_all);
		us+="}catch(e){Titanium.API.error('before_all caught error:'+e+' at line: '+e.line);}\n";

		// we skip these from being re-included
		var excludes = ['before','before_all','after','after_all','timeout'];
		for (var f in entry.test)
		{
			var i = excludes.indexOf(f);
			if (i==-1)
			{
				us+="TitaniumTest.tests.push(function(){\n";
				us+="// "+f+"\n";
				us+="var xscope = new TitaniumTest.Scope('"+f+"');"
				us+=make_function(entry.test.before,'xscope');
				
				us+="try {\n";
				us+="TitaniumTest.currentTest='" + f + "';\n";
				us+=make_function(entry.test[f],'xscope');
				
				i = f.indexOf('_as_async');
				if (i==-1)
				{
					us+="TitaniumTest.testPassed('"+f+"');\n";
				}

				us+="}\n";
				us+="catch(___e){\n";
				us+="TitaniumTest.testFailed('"+f+"',___e);\n";
				us+="}";

				us+=make_function(entry.test.after,'xscope');
				us+="//--- "+f+" ---\n";
				us+="});\n\n"
			}
		}
		
		us+="TitaniumTest.on_complete = function(){\n";
		us+="try{";
		us+=make_function(entry.test.after_all);
		us+="}catch(e){Titanium.API.error('after_all caught error:'+e+' at line: '+e.line);}\n";
		us+="TitaniumTest.complete();\n";
		us+="};\n";
		us+="TitaniumTest.run_next_test();\n";

		// poor man's hack to insert line numbers
		var newus = '';
		var lines = us.split("\n");
		var ready = false;
		for (var linenum=0;linenum<lines.length;linenum++)
		{
			var line = lines[linenum];
			if (!ready)
			{
				if (line.indexOf('TitaniumTest.NAME')==0)
				{
					ready = true;
				}
				newus+=line+"\n";
				continue;
			}
			var idx = line.indexOf('should_be');
			if (idx != -1)
			{
				var endIdx = line.lastIndexOf(')');
				if (line.charAt(endIdx-1)=='(')
				{
					line = line.substring(0,endIdx) + 'null,' + (linenum+1) + ');';
				}
				else
				{
					line = line.substring(0,endIdx) + ',' + (linenum+1) + ');';
				}
			}
			newus+=line+"\n";
		}
		
		var runner_js = TFS.getFile(user_scripts_dir,entry.name+'_driver.js');
		runner_js.write(newus);
		
		
		var profile_path = TFS.getFile(results_dir,entry.name+'.prof');
		var log_path = TFS.getFile(results_dir,entry.name+'.log');

		profile_path.deleteFile();
		log_path.deleteFile();

		var args = ['--profile="'+profile_path+'"','--logpath="'+log_path+'"','--runtime_override="'+runtime_dir+'"','--module_override="'+modules_dir+'"','--no-console-logging'];
		Titanium.Process.setEnv('KR_DEBUG','true');
		var process = Titanium.Process.launch(app.executable.nativePath(),args);
		process.onread = function(data)
		{
			Titanium.API.debug("PROCESS:"+data);
		};
		var size = 0;
		var timer = null;
		var start_time = new Date().getTime();
		
		// start a stuck process monitor in which we check the 
		// size of the profile file -- if we're not doing anything
		// we should have a file that hasn't changed in sometime
		timer = setInterval(function()
		{
			var t = new Date().getTime();
			var newsize = profile_path.size();
			if (newsize == size)
			{
				if (t-start_time>=entry.timeout)
				{
					clearInterval(timer);
					current_test.failed = true;
					update_status(current_test.name + " timed out");
					test_status(current_test.name,'failed');
					process.terminate();
					// run_next_test();
					return;
				}
			}
			else
			{
				size = newsize;
			}
			start_time = t;
		},1000);
		
		process.onexit = function(exitcode)
		{
			clearInterval(timer);
			if (!current_test.failed)
			{
				var r = TFS.getFile(results_dir,current_test.name+'.json').read();
				var results = eval('('+r+')');
				current_test.results = results;
				test_status(current_test.name,results.failed>0?'failed':'passed');
				update_status(current_test.name + ' complete ... '+results.passed+' passed, '+results.failed+' failed');
			}
			run_next_test();
		};
	}
	

	function run_next_test()
	{
		if (executing_tests==null || executing_tests.length == 0)
		{
			var test_duration = (new Date().getTime() - tests_started)/1000;
			executing_tests = null;
			current_test = null;
			run_button.disabled = false;
			update_status('Testing complete ... took ' + test_duration + ' seconds',true);
			return;
		}
		var entry = executing_tests.shift();
		current_test = entry;
		current_test.failed = false;
		update_status('Executing: '+entry.name);
		test_status(entry.name,'Running');
		setTimeout(function(){run_test(entry)},1);
	}
	
	run_button.onclick = function ()
	{
		run_button.disabled = true;
		update_status('Building test harness ... one moment');
		executing_tests = [];
		
		$.each($('#table tr.test'),function()
		{
			if ($(this).find('.checkbox').is('.checked'))
			{
				var name = $(this).find('.name').html();
				var entry = tests[name];
				executing_tests.push(entry);
			}
		});
		
		tests_started = new Date().getTime();
		setTimeout(run_next_test,1);
	};
};

