
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



Appcelerator.Widget.Folder =
{
	modulePath:null,
	
	setPath: function(path)
	{
		this.modulePath = path;
	},
	getName: function()
	{
		return 'appcelerator folder';
	},
	getDescription: function()
	{
		return 'folder widget';
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
		return 'app:folder';
	},
	getAttributes: function()
    {
        return [];
    },
	getActions: function()
	{
		return ['openAll','closeAll', 'selectItem','open','close'];
	},
	openAll: function(id,parameterMap,data,scope,version)
  {
		var nodes = $(id).childNodes;
		
		for (var c=0;c<nodes.length;c++)
		{
			nodes[c]._onopened();
		}
  },
	closeAll: function(id,parameterMap,data,scope,version)
  {
		var nodes = $(id).childNodes;
		for (var c=0;c<nodes.length;c++)
		{
			nodes[c]._onclosed();
		}
  },
	close: function(id,parameterMap,data,scope,version,customActionArguments)
  {
		var folderIdx = Object.getNestedProperty(customActionArguments[0],'value');
		folderIdx = parseInt(folderIdx)*2;

		var folderId = id + '_folder_' + folderIdx;
		$(folderId)._onclosed();
  },
	open: function(id,parameterMap,data,scope,version,customActionArguments)
  {
		var folderIdx = Object.getNestedProperty(customActionArguments[0],'value');
		folderIdx = parseInt(folderIdx)*2;

		var folderId = id + '_folder_' + folderIdx;
		$(folderId)._onopened();
  },
	selectItem: function(id,parameterMap,data,scope,version,customActionArguments)
  {
		// the args come in as an array of the form: [{key:..,value:...},{key:..,value:...}] so we iterate over the array to account for order differences
		var folderIdx;
		var nodeIdx;		
		
		for (var i = 0; i < customActionArguments.length; i++)
		{
			if (Object.getNestedProperty(customActionArguments[i],'key') == 'folder')
			{
				folderIdx = Object.getNestedProperty(customActionArguments[i],'value');
			} else if (Object.getNestedProperty(customActionArguments[i],'key') == 'item')
			{
				nodeIdx = Object.getNestedProperty(customActionArguments[i],'value');				
			}
		}
		
		//Folder ids are even numbers for whatever reason so map the user's folder index to its folder number
		folderIdx = parseInt(folderIdx)*2;

		var folderId = id + '_folder_' + folderIdx;
		var nodeId = id + '_folder_' + folderIdx + '_child_' + nodeIdx;
		$(folderId)._onopened();
		$(nodeId)._onopened();
 	},
	getChildNodes: function() {
		var T = Appcelerator.Types;
		
		var folderItemAttributes = [{
            name: 'open',
            type: T.messageReceive,
            description: 'A condition which will trigger the item to be opened'
        }, {
            name: 'onopen',
            type: T.messageSend,
            description: ''
        }, {
            name: 'onclose',
            type: T.messageSend,
            description: ''
        }, {
            name: 'opened_icon',
            optional: true,
            type: T.pathOrUrl
        }, {
            name: 'closed_icon',
            optional: true,
            type: T.pathOrUrl
        }];
		
		return [{
			name: 'folder',
			attributes: folderItemAttributes,
			childNodes: [{
				name: 'item',
				attributes: folderItemAttributes
			}]
		}];
	},
	compileWidget: function (params)
	{
		var itemCloser = function(exclude)
		{
			params['itemnodes'].each(function(child){if (exclude!=child && $(child).opened) $(child)._onclosed();});
		}

		var closeChildren = function(parentid, childid)
		{
			$A($(parentid+'_children').childNodes).findAll(function(n){return n.nodeType==1;}).each(function(n){if (n.id!=childid) $(n.id)._onclosed();});
		}
		
		var nodes = params['nodes'];
		
		for (var c=0;c<nodes.length;c++)
		{
			(function(){
			var node = nodes[c];
			if (node.nodeType == 1 && node.nodeName.toLowerCase() == 'folder')
			{
				var parentid = node._parentid;
				var parentnode = $(parentid);
				
				node._children.each(function(child,b)
				{
					var childid = child._childid;
					var open = child.getAttribute('open');
					var openaction = child.getAttribute('onopen');
					var closeaction = child.getAttribute('onclose');					
					
					if (openaction)
					{
						openaction = Appcelerator.Compiler.makeAction(childid,openaction);
					}

					if (closeaction)
					{
						closeaction = Appcelerator.Compiler.makeAction(childid,closeaction);
					}
					
					if (open)
					{
						var scriptcode = "$('"+parentid+"')._onopened(); $('"+childid+"')._onopened();";
						Appcelerator.Compiler.handleCondition(child, open, 'function['+scriptcode+']', null, 0, null);
					}
					$(childid)._onopened = function() 
					{ 
						if (!$(childid).opened) 
						{ 
							itemCloser(childid); 
							Element.removeClassName(childid+'_item','closed');
							Element.addClassName(childid+'_item','opened');
							$(childid).opened=true;
							Element.hide(childid+'_closed');
							Element.show(childid+'_opened');
							closeChildren(parentid, childid);
							if (openaction)
							{
								openaction();
							}
						}
					};
					
					$(childid)._onclosed = function()
					{
						if ($(childid).opened)
						{
							$(childid).opened=false; 
							Element.removeClassName(childid+'_item','open');
							Element.addClassName(childid+'_item','closed');
							Element.hide(childid+'_opened'); 
							Element.show(childid+'_closed');
							if (closeaction)
							{
								closeaction();
							}
						}
					};
					
					$(childid+'_closed').onclick = function()
					{
						$(childid)._onopened();
					};
					
					$(childid+'_opened').onclick = function()
					{
						$(childid)._onclosed();
					};
					
					$(childid).onclick = function(e)
					{
						e = Event.getEvent(e);
						e.stop();
						
						if ($(childid).opened)
						{
							$(childid)._onclosed(); 
						}
						else
						{
							$(childid)._onopened();
						}
					};
					
					$(childid)._onclosed();
				});
				
				$(parentid+'_closed').onclick = function()
				{
					parentnode.opened=true;
					Element.hide(parentid+'_closed');
					Element.show(parentid+'_opened');
					Element.toggle(parentid+'_children');
				};
				
				$(parentid+'_opened').onclick = function()
				{
					parentnode.opened=false;
					Element.hide(parentid+'_opened');
					Element.show(parentid+'_closed');
					Element.toggle(parentid+'_children');
				}
				
				parentnode._onopened = function()
				{
					parentnode.opened=true;
					Element.hide(parentid+'_closed');
					Element.show(parentid+'_opened');
					Element.show(parentid+'_children');	
					closeChildren(parentid, null);
				};
				
				parentnode._onclosed = function()
				{
					parentnode.opened=false;
					Element.show(parentid+'_closed');
					Element.hide(parentid+'_opened');
					Element.hide(parentid+'_children');					
					closeChildren(parentid, null);					
				}
				
				parentnode.onclick = function(e)
				{
					e=Event.getEvent(e);
					e.stop();
					if (!parentnode.opened)
					{ 
						parentnode._onopened();
					} 
					else
					{ 
						parentnode._onclosed();
					}
				};
			}
			})();
		}
	},
	buildWidget: function(element, parameters)
	{
		var html = '';
		var x = 0;
		var id = element.id;
		var itemnodes = [];
		
		if (Appcelerator.Browser.isIE)
		{
			// NOTE: in IE, you have to append with namespace
			var newhtml = element.innerHTML;
			newhtml = newhtml.replace(/<FOLDER/g,'<APP:FOLDER').replace(/\/FOLDER>/g,'/APP:FOLDER>');
			newhtml = newhtml.replace(/<ITEM/g,'<APP:ITEM').replace(/\/ITEM>/g,'/APP:ITEM>');
			element.innerHTML = newhtml;
		}
		
		for (var c=0;c<element.childNodes.length;c++)
		{
			var node = element.childNodes[c];
			if (node.nodeType == 1 && node.nodeName.toLowerCase() == 'folder')
			{
				var folder_opened_icon = node.getAttribute('opened_icon') || Appcelerator.Widget.Folder.modulePath + 'images/folder_opened.png';
				var folder_closed_icon = node.getAttribute('closed_icon') || Appcelerator.Widget.Folder.modulePath + 'images/folder_closed.png';
				var parentid = id+"_folder_"+ (x++);
				node._parentid = parentid;
				html+='<div id="'+parentid+'">';
				html+='<div class="folder" id="'+parentid+'_folder"><img class="folder_image" src="'+folder_closed_icon+'" id="'+parentid+'_closed"/>';
				html+='<img class="folder_image" src="'+folder_opened_icon+'" id="'+parentid+'_opened" style="display:none"/>';
				html+='<span class="folder_name" id="'+parentid+'_name">'+node.getAttribute('name')+'</span></div>';
				html+='<div class="folder_children" id="'+parentid+'_children" style="display:none">';
				var children = $A(node.childNodes).findAll(function(n){ return n.nodeType == 1 && n.nodeName.toLowerCase()=='item'; });
				node._children = children;
				var childcloser='';
				children.each(function(child,b)
				{
					var item_open_icon = child.getAttribute('opened_icon') || Appcelerator.Widget.Folder.modulePath + 'images/item_opened.png';
					var item_closed_icon = child.getAttribute('closed_icon') || Appcelerator.Widget.Folder.modulePath + 'images/item_closed.png';
					var childid = parentid+'_child_'+b;
					child._childid = childid;
					html+='<div id="'+childid+'">';
					html+='<div id="'+childid+'_item" class="item"><img class="item_image" src="'+item_closed_icon+'" id="'+childid+'_closed"/>';
					html+='<img class="item_image" src="'+item_open_icon+'" id="'+childid+'_opened" style="display:none"/>';
					html+='<span class="item_name" id="'+childid+'_name">'+Appcelerator.Compiler.getHtml(child,true)+'</span>';
					html+='</div></div>';
					var openaction = child.getAttribute('onopen');
					var closeaction = child.getAttribute('onclose');
					if (openaction)
					{
						openaction = Appcelerator.Compiler.makeAction(childid,openaction) +";";
					}
					if (closeaction)
					{
						closeaction = Appcelerator.Compiler.makeAction(childid,closeaction) + ";";
					}
					itemnodes.push(childid);
				});
				html+='</div></div>';
				x++;
			}
		}
		
		parameters['nodes'] = element.childNodes;
		parameters['itemnodes'] = itemnodes;
		
		return {
			'position' : Appcelerator.Compiler.POSITION_REPLACE,
			'presentation' : html,
			'compile' : true
		};
	}
};


Appcelerator.Core.loadModuleCSS('app:folder','folder.css');
Appcelerator.Widget.register('app:folder',Appcelerator.Widget.Folder);

