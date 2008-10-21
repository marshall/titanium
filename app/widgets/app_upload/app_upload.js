
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



Appcelerator.Widget.Upload =
{
	getName: function()
	{
		return 'appcelerator upload';
	},
	getDescription: function()
	{
		return 'upload widget';
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
		return 'app:upload';
	},
	getAttributes: function()
	{
        var T = Appcelerator.Types;        
        return [{
            name: 'on',
            optional: true,
			type: T.onExpr
        }, {
            name: 'maxsize',
            optional: true,
			type: T.naturalNumber,
			description: 'Maximum file size (in bytes?) that can be uploaded'
        }, {
            name: 'service',
            optional: true,
			type: T.messageSend,
			description: 'Name of the service that should be notified when the upload is complete'
        }];
    },
	dontParseOnAttributes: function()
	{
		return true;
	},
	execute: function(id,parameterMap,data,scope)
	{
		Appcelerator.Widget.Template.fetch(id,parameterMap['src'],parameterMap['args']);
	},
	compileWidget: function(params)
	{
		var id = params['id'];
		
		// make sure form elements have a name attribute as they won't be included
		// in the submit if they don't (web1.0 stuff)
		var e = Form.Methods.getElements(id+'_form');
		if (e && e.length > 0)
		{
			for (var c=0, len=e.length; c<len; c++)
			{
				var t = e[c];
				var name = t.getAttribute('name');
				if (!name)
				{
					t.setAttribute('name', t.id);
				}
			}
		}
		
		var upload = Appcelerator.ServerConfig['upload'];
		$(id+'_form').action = upload.value;
	},
	buildWidget: function(element,parameters,state)
	{
		var data = Appcelerator.Compiler.getHtml(element,true);
		var targetid = element.id+'_target';
		var type = parameters['service'];
		var on = parameters['on'];
		var onstr = on ? ('on="'+on+'"') : '';
		var maxsize = parameters['maxsize'];

		if (type && type.indexOf(":")>0)
		{			
			type = type.split(":")[1];
		}
				
		var html = '<form method="POST" id="'+element.id+'_form" enctype="multipart/form-data" target="'+targetid+'" '+onstr+'>';
		if (maxsize)
		{
			html+='<input type="hidden" name="MAX_FILE_SIZE" value="'+maxsize+'"/>';
		}
		html+="<input type='hidden' name='instanceid' value='"+Appcelerator.instanceid+"'/>";
		html+="<input type='hidden' name='type' value='"+type+"'/>";
		html+="<input type='hidden' name='callback' value='r:appcelerator.ping'/>";
		html+=data;
		html+='</form>';
		
		// put iframe as child of body so position absolute won't be relative in case parent is relative
		new Insertion.Bottom(document.body,'<iframe name="'+targetid+'" id="'+targetid+'" width="1" height="1" src="about:blank" style="position:absolute;top:-400px;left:-400px;width:1px;height:1px;"></iframe>');

		return {
			'position' : Appcelerator.Compiler.POSITION_REPLACE,
			'presentation' : html,
			'compile' : true,
			'wire':true
		};
	}
};

Appcelerator.Widget.register('app:upload',Appcelerator.Widget.Upload);
