
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



Appcelerator.Widget.Http =
{
    getName: function()
    {
        return 'appcelerator http';
    },
    getDescription: function()
    {
        return 'http widget';
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
        return 'app:http';
    },
    getAttributes: function()
    {
		var T = Appcelerator.Types;
        return [{
            name: 'on',
            optional: false,
            type: T.onExpr,
            description: "Used to execute requests"
        }, {
            name: 'response',
            optional: true,
            type: T.messageSend,
            description: 'Message to fire when successful. Will contain JSONified payload'
        }, {
            name: 'error',
            optional: true,
            type: T.messageSend,
            description: 'Message to fire when errors are received from server'
        }];
    },
	getChildNodes: function()
    {
		// response and error attributes are optional if the parent widget has them
		var T = Appcelerator.Types;         
        return [{
            name: 'uri',
            attributes: [{
				name: 'method',
				optional: false,
				type: T.enumeration('get', 'post', 'put', 'delete')
			}, {
				name: 'uri',
				optional: false,
				type: T.pathOrUrl,
				description: 'Where to send the HTTP request, '+
				    'may contain template variables pulled from the triggering message'
			}, {
                name: 'response',
                optional: true,
                type: T.messageSend,
				description: 'Message to fire when successful. Will contain JSONified payload'
            }, {
                name: 'error',
                optional: true,
                type: T.messageSend,
				description: 'Message to fire when errors are received from server'
            }, {
                name: 'property',
                optional: false,
                type: T.identifier,
				description: ''
            }, {
                name: 'responseRegex',
                optional: true,
                type: T.regex,
				description: 'Regular expression used to capture a section of text from the HTTP Response. '+
				    'The first group of the match will be used.'
            }, {
                name: 'contentType',
                optional: true,
                defaultValue: 'application/x-www-form-urlencoded',
                type: T.openEnumeration('application/x-www-form-urlencoded',
				        'text/plain', 'text/xml', 'text/json', 'text/javascript')
            }]
        }];
    },
    getActions: function()
    {
        return ['post', 'get', 'options', 'put', 'delete'];
    },
    options: function(id,params,data,scope,version)
    {
        Appcelerator.Widget.Http.performFetch(params, data, 'options');
    },
    put: function(id,params,data,scope,version)
    {
        Appcelerator.Widget.Http.performFetch(params, data, 'put');
    },
    'delete': function(id,params,data,scope,version)
    {
        Appcelerator.Widget.Http.performFetch(params, data, 'delete');
    },
    post: function(id,params,data,scope,version)
    {
        Appcelerator.Widget.Http.performFetch(params, data, 'post');
    },
    get: function(id,params,data,scope,version)
    {
		Appcelerator.Widget.Http.performFetch(params, data, 'get');
    },
    performFetch: function(params, data, method)
    {
        var uri = params['uri'][method];
        if (!uri)
        {
			$E('no method mapped for '+method);
			return;
        }

        var response = uri['response'] || params['response'];
        var error = uri['error'] || params['error'];
        var property = uri['property'];
        var responseRegex = uri['responseRegex'];
		var contentType = uri['contentType'] || 'application/x-www-form-urlencoded';

		var array = null;
		if (property)
		{
			array = Object.getNestedProperty(data,property) || [];
		}
		else
		{
			// removing the toString property on data
			array = Object.toJSON(data).evalJSON();
		}
		var methodParams = array;

		var body = uri['body'];
		if (body != '')
		{
	        var bodycompiled = eval(Appcelerator.Compiler.compileTemplate(body, true, 'body_init_'+params['id']) + '; body_init_'+params['id']);
	        body = bodycompiled(array).trim();
			if (!uri['contentType'])
			{
				contentType = 'text/plain;charset=UTF-8';
			}
			methodParams = null;
		}
		
        var compiled = eval(uri['uri'] + '; init_'+params['id']);
        var uriLink = compiled(array).trim();
        
        //TODO: support https
        if (uriLink.startsWith('http://'))
        {
            var uriStrip = uriLink.match('http://[^/]*');
            var currentStrip = window.location.href.match('http://[^/]*');
            
            $D('uriStrip='+uriStrip+', currentStrip='+currentStrip+', uriLink='+uriLink+', current location='+window.location.href);
            
            if (currentStrip && uriStrip && (uriStrip[0].toLowerCase() != currentStrip[0].toLowerCase()))
            {
                var proxy = Appcelerator.ServerConfig['proxy'];
                if (proxy)
                {
					if (methodParams && method != 'post' && method != 'put') 
					{
						if (uriLink.indexOf('?') < 0)
						{
							uriLink += '?';
						}
						else
						{
							uriLink += '&';
						}
						
						for (var k in methodParams)
						{
							uriLink += k + '=' + methodParams[k] + '&';
						}
					}
                    uriLink = proxy.value + '?url='+ encodeURIComponent(encodeURI(uriLink));
					methodParams = null;
                }
            }
        }

        $D('app:http sending request to ' + uriLink + ' with params '  + methodParams);
        new Ajax.Request(uriLink,
        {
            method: method,
            parameters: methodParams,
            evalJSON: false,
            evalJS: false,
			postBody: body,
			contentType: contentType,
            onSuccess: function (result)
            {
                contentType = result.getResponseHeader('Content-Type');
                $D('app:http onSuccess doing ' + method + ' to ' + uriLink + ', status = ' + result.status + ', contentType = ' + contentType + ', text = '+ result.responseText);
                if (response)
                {
                    var json_result = {};

                    if (contentType.indexOf('/xml') > 0)
                    {
                        json_encode_xml(result.responseXML.documentElement, json_result);
                    }
                    else if (contentType.indexOf('/json') > 0 || contentType.indexOf('/plain') > 0 || contentType.indexOf('/javascript') > 0)
                    {
                        var text = result.responseText.trim();
                        if (responseRegex)
                        {
                           var re = new RegExp(responseRegex);
                           var match = re.exec(text);
                           if (match && match.length > 1)
                           {
                              text = match[1];
                           }
                        }
                        json_result = text.evalJSON();
                    }
                    else
                    {
                        // using some custom protocol
                        json_result.contentType = contentType;
                        json_result.responseText = result.responseText;
                    }
                    (function()
                    {
                        $MQ(response, json_result);
                    }).defer();  
                }
            },
            onException: function (resp, ex)
            {
                var msg = new String(ex);
                $E('app:http onException doing ' + method + ' to ' + uriLink + ', exception was: '+msg);
                if (error)
                {
                    $MQ(error, {msg: msg, uri:uriLink});
                }
            },
            onFailure: function (result, json)
            {
                $E('app:http onFailure doing ' + method + ' to ' + uriLink + ', status = ' + result.status + ', text = ' + result.statusText);
                if (error)
                {
                    $MQ(error, {msg: result.statusText, uri: uriLink});
                }
            }
        });
    },
    buildWidget: function(element,parameters)
    {
        var uris = {};
        
        if (Appcelerator.Browser.isIE)
        {
            // NOTE: in IE, you have to append with namespace
            var newhtml = element.innerHTML;
            newhtml = newhtml.replace(/<URI/g,'<APP:URI').replace(/\/URI>/g,'/APP:URI>');
            element.innerHTML = newhtml;
        }

        for (var c=0; c<element.childNodes.length; c++)
        {
            (function()
            {
                var node = element.childNodes[c];
                if (node.nodeType == 1 && node.nodeName == 'URI')
                {
                    uriLink = Appcelerator.Compiler.compileTemplate(node.getAttribute('uri'),true,'init_'+element.id);
                    var uri = {uri: uriLink, response: node.getAttribute('response'), args: node.getAttribute('args'), property: node.getAttribute('property'), error: node.getAttribute('error'), responseRegex: node.getAttribute('responseRegex'), body: node.innerHTML};
                    uris[node.getAttribute('method')] = uri;
                }
            })();
        }

        parameters['uri'] = uris;

        return {
            'presentation' : '',
            'position' : Appcelerator.Compiler.POSITION_REPLACE
        };
    }
};

Appcelerator.Widget.register('app:http',Appcelerator.Widget.Http);
