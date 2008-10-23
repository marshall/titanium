
/*
 * Copyright 2006-2008 Appcelerator, Inc.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *    http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. 
 */



Appcelerator.Widget.Content =
{
	getName: function()
	{
		return 'appcelerator content';
	},
	getDescription: function()
	{
		return 'content widget support modularizing of code by breaking them into separate files which can be loaded either at load time or dynamically based on a message';
	},
	getVersion: function()
	{
		return '1.0.2';
	},
	getSpecVersion: function()
	{
		return 1.0;
	},
	getAuthor: function()
	{
		return 'Jeff Haynie';
	},
	getModuleURL: function ()
	{
		return 'http://www.appcelerator.org';
	},
	isWidget: function ()
	{
		return true;
	},
	getWidgetName: function()
	{
		return 'app:content';
	},
	getActions: function()
	{
		return ['execute'];
	},	
	getAttributes: function()
	{
		var T = Appcelerator.Types;
		return [{name: 'on', optional: true, type: T.onExpr,
		         description: "May be used to execute/load the content."},
				{name: 'src', optional: true, type: T.pathOrUrl,
				 description: "The source for the content file to load."},
				{name: 'args', optional: true, type: T.json,
				 description: "Used to replace text in the content file."},
				{name: 'lazy', optional: true, defaultValue: 'false', type: T.bool,
				 description: "Indicates whether the content file should be lazy loaded."},
				{name: 'reload', optional: true, defaultValue: 'true', type: T.bool,
				 description: "Indicates whether the content file should be refetched and reloaded on every execute. If false, execute will do nothing if already executed."},
				{name: 'onload', optional: true, type: T.messageSend,
				 description: "Fire this message when content file is loaded."},
				{name: 'onfetch', optional: true, type: T.messageSend,
				 description: "Fire this message when content file is fetched but before being loaded."},
 				{name: 'property', optional: true, type: T.messageSend,
 				 description: "The name of the property in the message payload to be used for the src"},
				{name:'useframe', optional: true, defaultValue: 'true', type: T.bool, 
				 description: "Use a hidden iframe when fetching the content, instead of an Ajax request. This is normally not required."}
		];
	},
	execute: function(id,parameterMap,data,scope)
	{
	    if (parameterMap['property'] && data[parameterMap['property']])
	    {
	        parameterMap['src'] = data[parameterMap['property']];
	    }
		if (!parameterMap['reload'])
		{
			if (!$(id).fetched && !parameterMap['fetched'])
			{
				Appcelerator.Widget.Content.fetch(id,parameterMap['src'],parameterMap['args'],parameterMap['onload'],parameterMap['onfetch'],parameterMap['useframe']);
				$(id).fetched = true;
			}
		}
		else
		{
			Appcelerator.Widget.Content.fetch(id,parameterMap['src'],parameterMap['args'],parameterMap['onload'],parameterMap['onfetch'],parameterMap['useframe']);
		}
	},
	compileWidget: function(parameters)
	{
		if (!(parameters['lazy'] == 'true') && parameters['src'])
		{
			Appcelerator.Widget.Content.fetch(parameters['id'],parameters['src'],parameters['args'],parameters['onload'],parameters['onfetch'],parameters['useframe']);
			parameters['fetched'] = true;
		}
	},
	buildWidget: function(element,parameters,state)
	{
		parameters['reload'] = (parameters['reload'] == 'true');
		
		return {
			'position' : Appcelerator.Compiler.POSITION_REPLACE,
			'presentation' : '',
			'compile' : true
		};
	},
	fetch: function (target,src,args,onload,onfetch,useframe)
	{
        target = $(target);
        target.style.visibility='hidden';

		if (useframe != 'true')
		{
			new Ajax.Request(src,
			{
				asynchronous:true,
				method:'get',
				onSuccess:function(resp)
				{
					if (onfetch)
					{
						$MQ(onfetch,{'src':src,'args':args});
					}
					var scope = target.getAttribute('scope') || target.scope;
					var state = Appcelerator.Compiler.createCompilerState();
					target.state = state;
					var html = resp.responseText.stripScripts();
					var match = /<body[^>]*>([\s\S]*)?<\/body>/mg.exec(html);
					if (match)
					{
						html = match[1];
					}
					html = Appcelerator.Compiler.addIENameSpace('<div>'+html+'</div>');
					if (args)
					{
						// replace tokens in our HTML with our args
						var t = Appcelerator.Compiler.compileTemplate(html);
						html = t(args.evalJSON());
					}
					// turn off until we're done compiling
					target.innerHTML = html;
					state.onafterfinish=function()
					{
						 // turn it back on once we're done compiling
					     target.style.visibility='visible';
			             if (onload)
			             {
			                $MQ(onload,{'src':src,'args':args});
			             }
					};
					// evaluate scripts
					resp.responseText.evalScripts();
					Appcelerator.Compiler.compileElement(target.firstChild,state);
					state.scanned=true;
					Appcelerator.Compiler.checkLoadState(target);
				}
			});
		}
		else
		{
			Appcelerator.Util.IFrame.fetch(src,function(doc)
			{
				if (onfetch)
				{
					$MQ(onfetch,{'src':src,'args':args});
				}

				var scope = target.getAttribute('scope') || target.scope;
				doc.setAttribute('scope',scope);
				doc.scope = scope;
				Appcelerator.Compiler.getAndEnsureId(doc);
				var contentHTML = doc.innerHTML;
				var state = Appcelerator.Compiler.createCompilerState();
				target.state = state;
				var html = '<div>'+contentHTML.stripScripts()+'</div>';
				if (args)
				{
					// replace tokens in our HTML with our args
					var t = Appcelerator.Compiler.compileTemplate(html);
					html = t(args.evalJSON());
				}
				// turn off until we're done compiling
				target.innerHTML = html;
				state.onafterfinish=function()
				{
					 // turn it back on once we're done compiling
				     target.style.visibility='visible';
		             if (onload)
		             {
		                $MQ(onload,{'src':src,'args':args});
		             }
				};
				// evaluate scripts
				contentHTML.evalScripts();
				Appcelerator.Compiler.compileElement(target.firstChild,state);
				state.scanned=true;
				Appcelerator.Compiler.checkLoadState(target);
			},true,true);
		}
	}
};

Appcelerator.Widget.register('app:content',Appcelerator.Widget.Content);
