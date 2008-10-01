
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



Appcelerator.Widget.If =
{
	getName: function()
	{
		return 'appcelerator if';
	},
	getDescription: function()
	{
		return 'if widget';
	},
	getVersion: function()
	{
		return '1.0';
	},
	getSpecVersion: function()
	{
		return 1.0;
	},
	getAuthor: function()
	{
		return 'Hamed Hashemi';
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
		return 'app:if';
	},
	getAttributes: function()
	{        
        return [{
            name: 'expr',
            optional: false,
            type: Appcelerator.Types.javascriptExpr,
            description: "The javascript expression to execute"
        }];
	},
	getChildNodes: function()
	{        
        return [{
            name: 'else',
			optional: true,
			maxNumber: 1
        }, {
			name: 'elseif',
			optional: true,
			attributes: [{
				name: 'expr',
				optional: false,
				type: Appcelerator.Types.javascriptExpr
			}]
		}];
	},
	compileWidget: function(params)
	{
		var id = params['id'];
		
		if (eval(Appcelerator.Widget.If.generateConditional(params['ifcond']['cond'])))
		{
			$(id).innerHTML = params['ifcond']['code'];
			Appcelerator.Compiler.dynamicCompile($(id));
		}
		else
		{
			for (var c=0;c<params['elseifconds'].length;c++)
			{
				var condition = params['elseifconds'][c];
				
				if (eval(Appcelerator.Widget.If.generateConditional(condition['cond'])))
				{
					$(id).innerHTML = condition.code;
					Appcelerator.Compiler.dynamicCompile($(id));
					return;
				}
			}
			
			var elsecond = params['elsecond'];
			if (elsecond)
			{
				$(id).innerHTML = elsecond.code;
				Appcelerator.Compiler.dynamicCompile($(id));
			}
		}
	},
	uniqueFunctionId: 0,
	generateConditional: function(code)
	{
		Appcelerator.Widget.If.uniqueFunctionId++;
		var fname = 'app_if_function'+'_'+Appcelerator.Widget.If.uniqueFunctionId;
		var code = 'var '+fname+' = function () { if ('+code+')';
		code += '{ return true; }';
		code += 'else { return false; }};';
		code += fname+'();';
		return code;		
	},
	buildWidget: function(element,params)
	{
		var ifcond = {code: '', cond: params['expr']};
		var elseifconds = [];
		var elsecond;
		
		if (Appcelerator.Browser.isIE)
		{
			// NOTE: in IE, you have to append with namespace
			var newhtml = element.innerHTML;
			newhtml = newhtml.replace(/<ELSEIF/g,'<APP:ELSEIF').replace(/\/ELSEIF>/g,'/APP:ELSEIF>');
			newhtml = newhtml.replace(/<ELSE/g,'<APP:ELSE').replace(/\/ELSE>/g,'/APP:ELSE>');
			element.innerHTML = newhtml;
		}
		
        if (Appcelerator.Browser.isOpera)
        {
            // NOTE: opera returns case-sensitive tag names, causing the conditions to fail
            var newhtml = element.innerHTML;
            newhtml = newhtml.replace(/<ELSEIF/gi,'<ELSEIF').replace(/\/ELSEIF>/gi,'/ELSEIF>');
            newhtml = newhtml.replace(/<ELSE/gi,'<ELSE').replace(/\/ELSE>/gi,'/ELSE>');
            element.innerHTML = newhtml;
        }
        
		for (var c=0; c<element.childNodes.length; c++)
		{
			(function()
			{
				var code, cond;
				var node = element.childNodes[c];
				
				if (node.nodeType == 1 && node.nodeName == 'ELSEIF')
				{
					if (elsecond)
					{
						throw ('syntax error: elseif after an else detected.');
					}
					elseifconds.push({code: Appcelerator.Compiler.getHtml(node), cond: node.getAttribute('expr')});
				}
				else if (node.nodeType == 1 && node.nodeName == 'ELSE')
				{
					if (elsecond)
					{
						throw ('syntax error: more than one else statement detected.');
					}
					elsecond = {code: Appcelerator.Compiler.getHtml(node)};
				}
				else if (node.nodeType == 1)
				{
					if (elsecond || elseifconds.length > 0)
					{
						throw ('syntax error: html code after an else or elseif detected.');
					}
					ifcond['code'] += Appcelerator.Compiler.convertHtml(Appcelerator.Util.Dom.toXML(node, true), true);
				}
				else if (node.nodeType == 3)
				{
					var val = node.nodeValue.trim();
					if ((elsecond || elseifconds.length > 0) && val.length > 0)
					{
						throw ('Html code after an else or elseif detected.');
					}
					ifcond['code'] += val;					
				}
			})();
		}
		
		params['ifcond'] = ifcond;		
		params['elseifconds'] = elseifconds;
		params['elsecond'] = elsecond;

		return {
			'presentation' : '',
			'position' : Appcelerator.Compiler.POSITION_REPLACE,
			'compile' : true
		};
	}
};

Appcelerator.Widget.register('app:if',Appcelerator.Widget.If);
