
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



Appcelerator.Widget.Message =
{
	getName: function()
	{
		return 'appcelerator message';
	},
	getDescription: function()
	{
		return 'message widget for generating messages (either remote or local)';
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
		return 'app:message';
	},
	getActions: function()
	{
		return ['execute','stop'];
	},	
	getAttributes: function()
	{
        var T = Appcelerator.Types;
        return [{
            name: 'on',
            optional: true,
            type: T.onExpr,
            description: "May be used to express when the message should be fired (executed)."
        }, {
            name: 'name',
            optional: false,
            type: T.messageSend,
            description: "The name of the message to be fired."
        }, {
            name: 'args',
            optional: true,
            type: T.json,
            description: "The argument payload of the message."
        }, {
            name: 'version',
            optional: true,
            description: "The version attached to the message."
        }, {
            name: 'interval',
            optional: true,
            type: T.time,
            description: "Indicates that an time interval that the message will continously be fired."
        }]
	},
	
	execute: function(id,parameterMap,data,scope,version)
    {
        Appcelerator.Widget.Message.sendMessage(parameterMap);
    },
    stop: function(id,parameterMap,data,scope,version)
    {
        var timer = parameterMap['timer'];
        if(timer)
        {
            clearInterval(timer);
            parameterMap['timer'] = null;
        }
        else
        {
            $D('Message '+parameterMap['name']+' is not currently sending, cannot stop');
        }
    },
	compileWidget: function(parameters)
	{
		Appcelerator.Widget.Message.sendMessage(parameters);
	},
	buildWidget: function(element, attributes)
	{
		var name = attributes['name'];
		var args = attributes['args'];
		var version = attributes['version'];
		var on = attributes['on'];
		
		if (args)
		{
			args = String.unescapeXML(args).replace(/\n/g,'').replace(/\t/g,'');
		}
		
		var interval = attributes['interval'];
		
		var parameters = {args:args, name:name, scope:element.scope, interval:interval,version:version};
		
		if (on)
		{
			return {
				'position' : Appcelerator.Compiler.POSITION_REMOVE,
				'parameters': parameters
			};
		}
		else
		{
			return {
				'position' : Appcelerator.Compiler.POSITION_REMOVE,
				'compile': true,
				'parameters': parameters
			};
		}
	},
	/*
	 * If the widget has an interval set, begin sending polling messages,
	 * otherwise send a one-shot message.
	 */
	sendMessage: function(params)
	{
		var name = params.name;
		var args = params.args;
		var version = params.version;
		var scope = params.scope;
		var interval = params.interval;
		var data = null;
		
		if (args && args != 'null')
        {
            data = Object.evalWithinScope(args, window);
        }

        if (interval == null || !params['timer']) $MQ(name, data, scope, version);      
        if (interval != null)
        {
            var time = Appcelerator.Util.DateTime.timeFormat(interval);

            if (time > 0 && !params['timer'])
            {
                params['timer'] = setInterval(function()
                {
                    if (args && args != 'null')
                    {
                    	// re-evaluate each time so you can dynamically change data each interval
                    	data = Object.evalWithinScope(args, window);
                    }
                    $MQ(name, data, scope, version);
                }, time);
            }
        }
	}
};

Appcelerator.Widget.register('app:message',Appcelerator.Widget.Message);
