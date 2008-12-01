/*!(c) 2006-2008 Appcelerator, Inc. http://appcelerator.org
 * Licensed under the Apache License, Version 2.0. Please visit
 * http://license.appcelerator.com for full copy of the License.
 *
 * TestMonkey lives at http://github.com/jhaynie/testmonkey/tree/master
 **/
window.TestMonkey = {};

(function(scope,$){
	
	if (typeof(AppC) != 'undefined')
	{
		// turn off report stats so it doesn't throw off tests
		AppC.config.report_stats=false;
	}
		
	var testRunnerPlugin = null;
	
	/**
	 * the plugin is responsible for driving the UI and doing
	 * all the stuff responsible for setting up and executing
	 * the test monkey system
	 *
	 */
	TestMonkey.installTestRunnerPlugin = function(callback)
	{
		testRunnerPlugin = callback;
	};
	
	var assertions = {};
	/**
	 * plugin to add your own assertion types.  name should be the 
	 * name of the assertion and handler is the callback function to
	 * execution the assertion.  all internal built-in assertions use
	 * this same interface to register themselves. it is possible to
	 * override the builtins by calling this method with same name as
	 * a built-in assertion.
	 */
	TestMonkey.installAssertionType = function(name,handler)
	{
		assertions[name]=handler;
	};
	
	TestMonkey.installAssertionType('',function(win,frame,testcase,assertion,args)
	{
		return runAssertion(args[0]);
	});
	
	TestMonkey.installAssertionType('Visible',function(win,frame,testcase,assertion,args)
	{
		return [this.css('visibility')=='visible',this.css('visibility')];
	});

	TestMonkey.installAssertionType('Hidden',function(win,frame,testcase,assertion,args)
	{
		return [this.css('visibility')=='hidden',this.css('visibility')];
	});

	TestMonkey.installAssertionType('Disabled',function(win,frame,testcase,assertion,args)
	{
		return [this.attr('disabled'),this.attr('disabled')];
	});

	TestMonkey.installAssertionType('Enabled',function(win,frame,testcase,assertion,args)
	{
		return [!this.attr('disabled'),this.attr('disabled')];
	});

	TestMonkey.installAssertionType('CSS',function(win,frame,testcase,assertion,args)
	{
		return [this.css(args[0]) == args[1],this.css(args[0])];
	});

	TestMonkey.installAssertionType('Attr',function(win,frame,testcase,assertion,args)
	{
		return [String(this.attr(args[0])) == String(args[1]),String(this.attr(args[0]))];
	});

	TestMonkey.installAssertionType('Class',function(win,frame,testcase,assertion,args)
	{
		return [this.hasClass(args[0]),this.attr('class')];
	});
	
	TestMonkey.installAssertionType('HTML',function(win,frame,testcase,assertion,args)
	{
		return [this.html()==args[0],this.html()];
	});

	TestMonkey.installAssertionType('Value',function(win,frame,testcase,assertion,args)
	{
		return [this.val()==args[0],this.val()];
	});

	TestMonkey.installAssertionType('Text',function(win,frame,testcase,assertion,args)
	{
		return [this.text()==args[0],this.text()];
	});

	TestMonkey.installAssertionType('Empty',function(win,frame,testcase,assertion,args)
	{
		return [this.text()=='',this.text()];
	});

	TestMonkey.installAssertionType('Checked',function(win,frame,testcase,assertion,args)
	{
		return [this.get(0).checked,this.get(0).checked];
	});

	TestMonkey.installAssertionType('Unchecked',function(win,frame,testcase,assertion,args)
	{
		return [!this.get(0).checked,this.get(0).checked];
	});

	TestMonkey.installAssertionType('Valid',function(win,frame,testcase,assertion,args)
	{
		var result = this.data('validatorResult');
		return [result,result];
	});

	TestMonkey.installAssertionType('Invalid',function(win,frame,testcase,assertion,args)
	{
		var result = this.data('validatorResult');
		return [!result,result];
	});

	TestMonkey.installAssertionType('Pub',function(win,frame,testcase,assertion,args)
	{
		// in this context, we're inside the top level window not the
		// execution context, we can use win variable to get the window 
		// context of the test
		var q = win.AppC.getLastQueue();
		if (!q)
		{
			return [false,"no message has been sent"];
		}
		var name = App.normalizePub(args[0]), data = args[1];
		var lastPubType = q.name;
		var lastPubData = q.data;
		if (name!=lastPubType)
		{
			return [false,name+" was not correct. expected: "+name+", was: "+lastPubType];
		}
		if (!lastPubData)
		{
			return [false,name+" missing data payload: "+$.toJSON(data)];
		}
		if (data)
		{
			for (var key in data)
			{
				var v1 = lastPubData[key];
				var v2 = data[key];
				if (v1!=v2)
				{
					if (key!='event')
					{
						return [false,name+" has incorrect data payload entry for key: "+key+", expected: "+v2+", was: "+v1];
					}
				}
			}
		}
		return [true,name+'=>'+$.toJSON(data)];
	});

	function fireEvent()
	{
		if (testRunnerPlugin)
		{
			var name = arguments[0], args = arguments.length > 1 ? $.makeArray(arguments).slice(1) : [];
			var fn = testRunnerPlugin[name];
			if (fn)
			{
				fn.apply(testRunnerPlugin,args);
			}
			// support a catch-all event handler
			fn = testRunnerPlugin['onEvent'];
			if (fn)
			{
				args.unshift(name);
				fn.apply(testRunnerPlugin,args);
			}
		}
	}

	var currentDescriptor = null, currentSuite = null;
	
	
	/**
	 * called from the test execution environment to run the tests
	 */
	scope.testRunner = function()
	{
		// cleanup in case we call this multiple times
		currentDescriptor = currentSuite = currentTestCase = null;
		testCases = [];
		
		var suites = [];
		var it = typeof(arguments[0].push)=='function' ? arguments[0] : arguments;
		$.each(it,function()
		{
			var descriptor = testSuites[this];
			if (descriptor)
			{
				suites.push([this,descriptor]);
			}
		});

		fireEvent('beforeTestRunner',suites);
		
		
		// we first have to run through them so all the tests can be recorded
		// they won't run at this point
		$.each(suites,function()
		{
			var descriptor = currentDescriptor = this[1];
			var name = currentSuite = this[0];
			var error = false;
			try
			{
				descriptor.run();
			}
			catch(E)
			{
				error = E;
			}
			currentDescriptor = null;
		});

		// set it to the first one in the list
		currentSuite = null;
		if (currentSuite) fireEvent('beforeTestSuite',suites[0][0]);

		fireEvent('beforeTestCases',testCases);

		fireEvent('beforeAssertionCount');
		var assertCount = 0;
		
		$.each(testCases,function()
		{
			var testcase = this;
			assertCount += testcase.asserts.length;  
			testcase.running = false;
			testcase.ready = true;
		});
		
		fireEvent('afterAssertionCount',assertCount);
		
		var total = 0, loaded = 0;

		// we need to load any pending HTML for the test
		// and then wait until they're all complete before we 
		// start running the test cases
		$.each(suites,function()
		{
			var descriptor = this[1];
			var html = descriptor.html;
			descriptor.htmlID = String(Math.round( Math.random() * 10000 ));
			if (html)
			{
				total+=1;
				loadTestFrame(html,function(content)
				{
					// mark the id for the frame onto the descriptor
					descriptor.content = content;
					loaded+=1;
					if (loaded == total)
					{
						executeNextTestCase();
					}
				});
			}
			else
			{
				// we later use marker to indicate where we need to replace content
				descriptor.content = "<html><head>####MARKER####</head><body></body></html>";
			}
		});

		if (total==0)
		{
			executeNextTestCase();
		}
	}
	
	function executeNextTestCase()
	{
		var nextTestCase = null;
		$.each(testCases,function()
		{
			if (this.ready)
			{
				nextTestCase = this;
				return false;
			}
		});
		
		if (nextTestCase)
		{
			if (currentSuite!=nextTestCase.suite)
			{
				if (currentSuite)
				{
					fireEvent('afterTestSuite',currentSuite);
					var currentD = testSuites[currentSuite];
				}
				currentSuite = nextTestCase.suite;
				fireEvent('beforeTestSuite',currentSuite);
			}
			nextTestCase.ready = false;
			var testcase = nextTestCase;
			fireEvent('beforeTestCase',testcase);
			var descriptor = testcase.descriptor;
			var error = false;
			try
			{
				executeTest(testcase,descriptor);
			}
			catch(E)
			{
				error = E;
				testcase.running = false;
				testcase.error = E;
				testcase.message = "Exception running testcase: "+E;
			}
		}
		else
		{
			if (currentSuite)
			{
				fireEvent('afterTestSuite',currentSuite);
				var currentD = testSuites[currentSuite];
			}
			fireEvent('afterTestCases',testCases);
			fireEvent('afterTestRunner');
		}
	}

	function runAssertion(value)
	{
		switch(typeof(value))
		{
			case 'boolean':
				return [value===true,value];
			case 'function':
				value = $.toFunction('(' + String(value) + ')()')();
				return [(value ? true : false), value];
			default:
				return [(value ? true : false), value];
		}
	}
	
	function internalAssert()
	{
		var win = arguments[0], frame = arguments[1];
		var idx = arguments[2], type = arguments[3]||'';
		var args = $.makeArray(arguments).splice(4);
		var result = false;
		var testcase = currentTestCase;
		var assert = testcase.asserts[idx];
//		alert('internalAssert='+win+',frame='+frame+',idx='+idx+',type='+type);
		try
		{
			var handler = assertions[type];
			if (handler)
			{
				result = handler.apply(this,[win,frame,testcase,assert,args]);
			}
			var message = result ? result[0] ? null : 'value was "' + result[1] + '" (' + typeof(result[1]) + ')' : 'handler not found: '+type;
			testcase.results.push({assert:assert,result:result[0],message:message,idx:idx});
		}
		catch (E)
		{
			testcase.results.push({assert:assert,result:false,error:E,message:String(E),idx:idx});
		}
	}
	
	function getHtml(f)
	{
		var n = $(f.get(0).cloneNode(true));
		// pull out the firebug injected HTML since we don't really want to see that
		n.find('#_firebugConsoleInjector,#_firebugConsole').remove();
		return '<html>\n'+jQuery(n).html()+'\n</html>';
	}
	
	var currentTestCase = null;
	var currentTestCaseId = 1;

	function executeTest(testcase,descriptor)
	{
		currentTestCase = testcase;
		testcase.results = [];
		testcase.running = true;
		testcase.id = currentTestCaseId++;
		var timer = null;
		var count = 0;
		var total = testcase.asserts.length;
		var id = descriptor.htmlID+'_'+testcase.id;
		
		function getFrame()
		{
			return jQuery("#"+id).contents().find("html");
		}
		window.testScope = function()
		{
			var self = this;
			var win = null;
			this.descriptor = descriptor;
			// these functions are specially mapped into the execution
			// environment as delegates
			this.setup = function(w)
			{
				win = w;
				if (descriptor.setup) try { descriptor.setup(); } catch (E) {}
			}
			this.teardown = function()
			{
				if (descriptor.teardown) try { descriptor.teardown(); } catch (E) {}
			}
			this.assertTestCase=function()
			{
				if (testcase.running)
				{
					var args = [win,getFrame()];
					for (var c=0;c<arguments.length;c++)
					{
						args[c+2] = arguments[c];
					}
					return internalAssert.apply(this,args);
				}
				return false;
			}
			this.log=function(msg)
			{
				if (!testcase.logs)
				{
					testcase.logs = [];
				}
				testcase.logs.push(msg);
			}
			this.error = function(E)
			{
				testcase.failed = true;
				testcase.error = E;
				testcase.message = "Exception running testcase: "+E;
				testcase.results.push({'result':false,'error':E,'message':testcase.message});
				self.end(true,false);
			}
			this.fail=function(msg)
			{
				testcase.message = msg;
				testcase.explicitFailure = true;
				testcase.results.push({'result':false,'message':testcase.message});
				self.end(true,false);
			}
			this.completed = function()
			{
				if (typeof(testcase.timeout)=='undefined')
				{
					self.end(false,false);
				}
			}
			this.end=function(failed,timeout)
			{
				count++;
				if (timer)
				{
					clearTimeout(timer);
					timer=null;
				}
				if (timeout && count < total)
				{
					// timeout the rest of the assertions
					for (var c=count+1;c<total;c++)
					{
						testcase.results.push({'result':false,'message':'timed out'});
					}
				}
				self.teardown();
				var f = getFrame();
				f.find('#__testMonkeySDK,#__testMonkeyJS').remove();
				testcase.after_dom = getHtml(f);
				try
				{
					if (!testcase.running) return;
					testcase.running = false;
					if (failed)
					{
						testcase.failed = true;
					}
					else
					{
						var passed = true;
						if (testcase.results.length == 0)
						{
							testcase.results.push({'result':true,'message':'passed'});
						}
						jQuery.each(testcase.results,function()
						{
							if (!this.result)
							{
								passed=false;
								return false;
							}
						});
						testcase.failed = !passed;
						if (passed && !testcase.message) testcase.message ="Assertions Passed";
						if (!passed && !testcase.message) testcase.message = "Assertion Failures";
					}
					if (timeout)
					{
						testcase.timeout = true;
						testcase.message = "Timed out";
						testcase.results.push({'result':false,'message':testcase.message});
					}
					fireEvent('afterTestCase',testcase,descriptor);
					removeTestFrame(id);
					executeNextTestCase();
				}
				catch (E)
				{
					alert('Error ending test: ' + E);
				}
			}
		}
		
		// we keep it at global scope so the execution environment can
		// easily use the running scope instance
		window.testMonkeyScope = new window.testScope;
		
		try
		{
			jQuery("<iframe style='position:absolute;left:-10px;top:-10px;height:1px;width:1px;' id='" + (id)+"'></iframe>").appendTo("body");
			var body = jQuery("#"+id).contents().find("body").get(0);
			
			if (!body) body = jQuery("#"+id).contents().find("html").get(0);
			
			var doc = body.ownerDocument;
			
			var setupCode = "" + 
			"(function($,scope){\n"+
			" $.noConflict();\n"+
			" $(function(){\n "+	
			"    $.fn.assertTestCase = function(){ return scope.assertTestCase.apply(this,arguments) }\n" + 
			"    function log() { return scope.log.apply(this,arguments) }\n" + 
			"    function end() { return scope.end.apply(this,arguments) }\n" + 
			"    function fail() { return scope.fail.apply(this,arguments) }\n" + 
			"    function error() { return scope.error.apply(this,arguments) }\n" + 
			"    function assertTestCase() { return scope.assertTestCase.apply(this,arguments) }\n" + 
			"    scope.setup.call(scope.descriptor,window);\n" + 
			"    try{\n" + 
			'      ('+testcase.code+').call(scope.descriptor);\n' + 
			"    }catch(E){\n" + 
			"      scope.error(E);\n" + 
			"    }\n" + 
			"    scope.completed();\n" + 
			" });\n " + 
			"})(window.jQuery,parent.window.testMonkeyScope);\n";

			// inject our library
			var code = "<script id=\"__testMonkeySDK\" type=\"text/javascript\" src=\"" + AppC.sdkJS + "\"></script>\n";

			// now inject our test execution environment
			code += "<script id=\"__testMonkeyJS\" type=\"text/javascript\">" + setupCode + "</script>\n";

			var jscode = descriptor.content.replace('####MARKER####',code);

			$.info(jscode);

			if (typeof(testcase.timeout)!='undefined')
			{
				// run the timer here in case we have problems loading the test HTML code itself
				timer=setTimeout(function(){
					testMonkeyScope.end(true,true)
				},testcase.timeout);
			}

			// write the test + our bootstrap code
			doc.open("text/html","replace");
			doc.writeln(jscode);
			doc.close();
		}
		catch(E)
		{
			testcase.failed = true;
			testcase.error = E;
			testcase.message = "Exception running testcase: "+E;
			testcase.results.push({'result':false,'error':E,'message':testcase.message});
			testMonkeyScope.end(true,false);
		}
	}
	
	scope.extractCodeLine = function (code,expr)
	{
		var result = expr;
		var idx = code.indexOf(expr);
		if (idx!=-1)
		{
			var end = idx + expr.length;
			idx--;
			// back up to the beginning of the line
			while ( idx >= 0)
			{
				var ch = code.charAt(idx);
				if (ch == ';' || ch=='\n' || ch=='\r')
				{
					break;
				}
				result = ch + result;
				idx--;
			}
			// go to the end of the line
			while ( end < code.length )
			{
				var ch = code.charAt(end);
				result = result + ch;
				if (ch == ';' || ch=='\n' || ch=='\r')
				{
					break;
				}
				end++;
			}
		}
		return result;
	}
	
	var testFrameId = 1;
	
	function loadTestFrame(url,fn)
	{
		var id = '__testdriver_content_'+(testFrameId++);
		url = URI.absolutizeURI(url,AppC.docRoot+'tests/');
		
		return jQuery.ajax({
			type: "GET",
			url: url,
			success: function(html)
			{
				// search to see what injection point we need to 
				// put the code.  we want it to be at the earliest point
				// possible. but it will not run until after the doc is loaded
				var begin = html.indexOf('<head');
				if (begin > 0)
				{
					begin = html.indexOf('>',begin);
				}
				else
				{
					begin = html.indexOf('<body');
					if (begin < 0)
					{
						begin = html.indexOf('<html');
						if (begin<0)
						{
							html = '<html>' + html + '</html>';
							begin = 0;
						}
						begin = html.indexOf('>',begin);
						html = html.substring(0,begin) + '<head></head>' + html.substring(begin+1);
						begin = begin + 6;
					}
					else
					{
						html = html.substring(0,begin-1) + '<head></head>' + html.substring(begin);
						begin = (begin-1) + 6;
					}
				}
				var start = '';
				if (begin>0) start = html.substring(0,begin);
				var end = html.substring(begin);
				// we later use marker to indicate where we need to replace content
				fn(start + '####MARKER####' + end);
			}
		});
	}
	
	function removeTestFrame(id)
	{
		// for this frame, we need to drop back to DOM instead of just .remove it seems
		var el = $("#"+id);
		if (el.length > 0)
		{
			var node = el.get(0);
			// FF seems to not be happy if we remove to early (shows a spinning icon)
			setTimeout(function()
			{
				node.parentNode.removeChild(node);
			},10);
		}
	}
	
	function escapeString(str)
	{
		return $.gsub(str,'"',"\\\"");
	}
	
	function preProcessCode(code)
	{
		var re = /assert(.*?)[\s]?\((.*)?\)/;
		var _asserts = [];
		var newcode = $.gsub(code,re,function(m)
		{
			_asserts.push(m[0]); 
			var prefix = m[1] ? '"' + escapeString(m[1]) + '"' : 'null';
			var params = m[2] || 'null';
			return 'assertTestCase(' + (_asserts.length-1) + ','+prefix+','+params+')';
		});
		var asserts = [];
		$.each(_asserts,function()
		{
			asserts.push(extractCodeLine(code,this));
		});
		return { asserts: asserts,code: newcode }
	}
	
	var testCases = [];

	scope.testAsync = function()
	{
		var name = arguments[0], timeout = 1000, fn = null;
		if (arguments.length == 2) 
		{
			fn = arguments[1];
		}
		else
		{
			timeout = arguments[1];
			fn = arguments[2];
		}
		var results = preProcessCode(String(fn));
		testCases.push({
			name: name,
			code: results.code,
			testcase: String(fn),
			timeout: timeout,
			asserts: results.asserts,
			descriptor: currentDescriptor,
			suite:currentSuite
		});
	}
	
	scope.test = function()
	{
		var name = arguments[0], fn = arguments[1];
		var results = preProcessCode(String(fn));
		testCases.push({
			name: name,
			code: results.code,
			testcase: String(fn),
			asserts: results.asserts,
			descriptor: currentDescriptor,
			suite:currentSuite
		});
	};
	
	var testSuites = {};
	
	scope.testSuite = function(name,html,descriptor)
	{
		if (typeof(html)!='string')
		{
			descriptor = html;
			html = null;
		}
		
		descriptor.html=html;
		testSuites[name]=descriptor;
		
		fireEvent("addTestSuite",name,descriptor,html);
		
		this.run = function()
		{
			testRunner(name);
		}
		
		return this;
	}
	
})(window,jQuery);

