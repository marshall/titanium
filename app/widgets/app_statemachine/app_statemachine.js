
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



Appcelerator.Widget.Statemachine =
{
	getName: function()
	{
		return 'appcelerator statemachine';
	},
	getDescription: function()
	{
		return 'statemachine widget';
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
		return 'app:statemachine';
	},
    getAttributes: function(){
        return [{
            name: 'initial',
            optional: true,
            description: "Initial state of the state machine."
        }, {
            name: 'id',
            optional: false,
            description: "Id of the statemachine, which can be used as a web expression condition"
        }];
    },
    getChildNodes: function(){
        return [{
            name: 'state',
            attributes: [{
                name: 'name',
                description: 'state name'
            }, {
                name: 'if',
                description: 'state condition'
            }]
        }];
    },
	buildWidget: function(element, parameters)
	{
        var initial = parameters['initial'];
        var initialFound = false;
        var id = parameters['id'];
        
        var states = [];
        if (Appcelerator.Browser.isIE)
        {
            // NOTE: in IE, you have to append with namespace
            var newhtml = element.innerHTML;
            newhtml = newhtml.replace(/<STATE/g,'<APP:STATE');
            newhtml = newhtml.replace(/\/STATE>/g,'/APP:STATE>');
            element.innerHTML = newhtml;
        }
        
        for (var c=0,len=element.childNodes.length;c<len;c++)   
        {
            var child = element.childNodes[c];
            if (child.nodeType == 1 && child.nodeName.toLowerCase() == 'state')
            {
                var name = child.getAttribute('name');
                var cond = child.getAttribute('if');
                
                if (initial && initial == name)
                {
                    initialFound = true;
                }
                states.push({'name':name,'cond':cond});
            }
        }

        if (initial)
        {
            if (!initialFound)
            {
                throw "invalid initial state - couldn't find state: "+initial+" for "+id;
            }
        }

        parameters['states']=states;
        
        return {
            'position' : Appcelerator.Compiler.POSITION_REPLACE,
            'presentation':'',
            'compile' : true
        };
	},
	compileWidget:function(parameters)
	{
	    var id = parameters['id'];
	    var element = $(id);
	    var initial = parameters['initial'];
		element.value = initial || '';
		var states = parameters['states'];
		var conditions = [];
		
		for (var c=0,len=states.length;c<len;c++)	
		{
			var state = states[c];
		    var name = state.name;
			var cond = state.cond;
			Appcelerator.Compiler.StateMachine.addState(id,name,null);
			conditions.push(Appcelerator.Compiler.StateMachine.compileStateCondition(name,cond));
		}
		
		var compiled = Appcelerator.Compiler.StateMachine.buildConditions(conditions);
		
		if (initial)
		{
			//
			// go ahead and set the initial state
			// and invoke appropriate listeners 
			//
			element.value = initial;
			Appcelerator.Compiler.StateMachine.resetOnStateListeners();
			if (Appcelerator.Compiler.StateMachine.initialStateLoaders)
			{
				Appcelerator.Compiler.StateMachine.initialStateLoaders.push([id,initial]);
			}
		}

		// compile when first accessed
		var codeFunction = null;
		
        $MQL(compiled.types, function(type, data, datatype, direction){
            try {
                if (!codeFunction) {
                    codeFunction = compiled.code.toFunction();
                    compiled = null;
                }
                
                var key = direction + ':' + type;
                var obj = {
                    messagetype: key,
                    type: type,
                    datatype: datatype,
                    direction: direction,
                    data: data
                };
                var state = codeFunction.call(obj);
                
                if (state) {
                    Appcelerator.Compiler.StateMachine.fireStateMachineChange(id, state, true);
                }
                else {
                    state = Appcelerator.Compiler.StateMachine.getActiveState(id);
                    
                    if (state) {
                        Appcelerator.Compiler.StateMachine.fireStateMachineChange(id, state, null);
                    }
                }
            } 
            catch (e) {
                $E('Error processing message: ' + direction + ':' + type + ' - ' + Object.getExceptionDetail(e));
            }
        }, element.scope, element);

		var stateListener = function(statemachine,state,on_off)
		{
			if (on_off)
			{
				element.value = state;
			}
		};
		Appcelerator.Compiler.StateMachine.registerStateListener(id,stateListener);
		Appcelerator.Compiler.StateMachine.fireOnStateListeners();
		
		Appcelerator.Compiler.addTrash(element, function()
		{
			if (stateListener)
			{
				Appcelerator.Compiler.StateMachine.unregisterStateListener(stateListener);
				stateListener = null;
			}
			Appcelerator.Util.ServiceBroker.removeListener(listener);
		});
	}
};

Appcelerator.Widget.register('app:statemachine',Appcelerator.Widget.Statemachine);
