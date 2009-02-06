(function()
{
	App.TreeModel = function(data)
	{
		this.data = {};
		this.rootNodes = [];

		this.isLeaf = function(id)
		{
			return (this.getChildCount(id) == 0);
		};
		this.getRootNodes = function()
		{
			return this.rootNodes;
		};
		this.getChildCount = function(id)
		{
			return (this.data[id] && this.data[id].children)?this.data[id].children.length:0;
		};
		this.getChildren = function(id)
		{
			return (this.data[id] && this.data[id].children)?this.data[id].children:[];
		};
		this.getParentId = function(id)
		{
			var d= this.data[id];
			var p = (d)?d['parent']:null;
			return p;
		};
		this.addNode = function(parentId,node)
		{
			// if we already have it, return
			if (this.data[node.id])return false;
			
			// add to stack
			this.data[node.id] = node;
			
			if (parentId == null)
			{
				this.rootNodes.push(node);
			}
			else
			{
				this.data[node.id]['parent']=parentId;

				var parent = this.data[parentId];

				if (!parent)return false;

				// add to parent

				// first child
				if (!parent.children.length)
				{
					parent.children = [];
					parent.children[0] = node;
				}
				else
				{
					// do we already have this child
					var childExists = false;
					for (var i=0; i<parent.children.length;i++)
					{
						if (parent.children[i].id == node.id)
						{
							childExists = true;
							break;
						}
					}
					// if not, then add
					if (childExists == false)
					{
						// last child
						if (parent.children.length <= node.id)
						{
							parent.children[parent.children.length] = node;
						}
						// in the middle
						else
						{
							var currentLength = parent.children.length
							for (var i=currentLength;i>node.id;i--)
							{
								parent.children[i] = parent.children[i-1];
							}
							parent.children[node.id] = node;
						}	
					}
				}
				
			}
			return true;
		};
		this.getNode = function(id)
		{
			return (this.data[id] != null)?this.data[id]:null
		};
		this.removeNode = function(id)
		{
			if (!this.data[id])return;
			
			// remove children
			var children = (this.data[id] && this.data[id].children)?this.data[id].children:[];
			children.splice(0,children.length)

			// remove from parent if exists
			var parent = this.data[this.data[id].parent];
			if (parent)
			{
				for (var i=0;i<parent.children.length;i++)
				{
					if (parent.children[i].id == id)
					{
						parent.children.splice(i,1);
						break;
					}
				}
			}

			// remove object
			this.data[id] = null;
			
			for (var i=0;i<this.rootNodes.length;i++)
			{
				if (id == this.rootNodes[i].id)
				{
					this.rootNodes.splice(i,1);
					break;
				}
			}

		};
		this.updateNode = function(id,node)
		{
			if (!this.data[id])return;

			// delete node's individual children
			if (this.data[id].children)
			{
				for (var i=0;i<this.data[id].children.length;i++)
				{
					var child = this.data[id].children[i];
					if (this.data[child.id])
						this.data[child.id]=null;
				}
			}
			// update the node
			// NOTE: the node's index isn't touched in this case
			this.data[id] = node;

			// add node's new children to this.data
			if (node.children)
			{
				for (var i=0;i<node.children.length;i++)
				{
					var child = node.children[i];
					this.data[child.id] = child;
					this.data['parent'] = id;
				}
			}
		};
		this.flattenData = function(data,parentId)
		{
			if (!data)return;
			for (var i=0;i<data.length;i++)
			{
				var node = data[i];
				this.data[node.id] = node;
				this.data[node.id]['parent'] = parentId;
				if (parentId == null)
				{
					this.rootNodes.push(this.data[node.id]); 
				}

				if (node.children)
				{
					this.flattenData(node.children,node.id)
				}
			}
			
		};
		this.flattenData(data,null);
		
	}
	
	App.HTMLTreeModel = function(data)
	{
		this.data = {};
		this.rootNodes = [];

		this.flattenData = function(childNodes,parentId)
		{

			for (var i=0;i<childNodes.length;i++)
			{
				var node = childNodes[i];
				// only process DIVs
				if (node.nodeType == 1 && node.tagName && node.tagName.toUpperCase() == 'DIV')
				{
					var nodeData = {};
					// replicate attributes and store by name
					var attrs = (node.attributes)?node.attributes:[]
					for (var j=0;j<attrs.length;j++)
					{
						var value = node.getAttribute(attrs[j].name);
						if (value=="true")value=true;
						else if(value=="false")value=false;

						nodeData[attrs[j].name.toLowerCase()] = value;
					}
					// ensure we have an id
					if (!nodeData['id'])
					{
						node.id = nodeData['id'] = App.Compiler.getAndEnsureId(node);
					}
					// store innerHTML as attribute
					nodeData['html'] = node.innerHTML.trim();

					// record data
					this.data[node.id] = nodeData;
					this.data[node.id]['parent'] = parentId;

					// if we are a child - store children
					if (parentId != null)
					{
						if (!this.data[parentId].children)
						{
							this.data[parentId].children = [];
						}
						this.data[parentId].children[this.data[parentId].children.length] = nodeData;

					}
					else
					{
						this.rootNodes.push(this.data[node.id]); 
					}

					// if we have children - process
					if (node.childNodes.length > 0)
					{
						this.flattenData(node.childNodes,node.id);
					}
				}
			}

		}
		this.flattenData(data,null)
		
	};
		
	App.HTMLTreeModel.prototype = new App.TreeModel();
})();

