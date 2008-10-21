
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



Appcelerator.Widget.Script =
{
	getName: function()
	{
		return 'appcelerator script';
	},
	getDescription: function()
	{
		return 'script widget';
	},
	getVersion: function()
	{
		return '1.0.1';
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
		return 'app:script';
	},
	getActions: function()
	{
		return ['execute'];
	},	
	getAttributes: function()
	{
        return [{
            name: 'on',
            optional: true,
			type: Appcelerator.Types.onExpr,
            description: "May be used to execute the script's content."
        }];
	},
	execute: function(id,parameterMap,data,scope,version,customdata,direction,type)
	{
		var code = parameterMap['code'];
		var script = code.toFunction(true);
		if (script == true) return;
		script.call({data:data||{},scope:scope,version:version,type:type,direction:direction});
	},
	compileWidget: function(params)
	{
		eval(params['code']);
	},
	buildWidget: function(element,parameters)
	{
		var code = Appcelerator.Compiler.getHtml(element);
		code = code.replace(/\/\*.*\*\//g,'');
		
		if (code && code.trim().length > 0)
		{
			parameters['code'] = String.unescapeXML(code);

			if (parameters['on'])
			{
				return {
					'position' : Appcelerator.Compiler.POSITION_REMOVE
				};
			}
			else
			{
				return {
					'position' : Appcelerator.Compiler.POSITION_REMOVE,
					'compile' : true
				};
			}
		}

		return {
			'position' : Appcelerator.Compiler.POSITION_REMOVE
		};
	}
};

Appcelerator.Widget.register('app:script',Appcelerator.Widget.Script);
