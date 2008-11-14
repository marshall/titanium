
ti.XML = {};
ti.XML.ElementNode = 1;
ti.XML.AttributeNode = 2;
ti.XML.TextNode = 3;

function TitaniumElement (element)
{
	this.name = element.nodeName;
	this.type = ti.XML.ElementNode;
	this.attributeList = {};
	this.elementList = [];
	this.value = "";
	
	for (var i = 0; i < element.attributes.length; i++)
	{
		var attr = element.attributes.item(i);
		this.attributeList[attr.nodeName] = attr.nodeValue;
	}
	
	for (var i = 0; i < element.childNodes.length; i++)
	{
		if (element.childNodes[i].nodeType == ti.XML.ElementNode) {
			this.elementList.push(new TitaniumElement(element.childNodes[i]));
		}
		else if (element.childNodes[i].nodeType == ti.XML.TextNode) {
			this.value += element.childNodes[i].nodeValue;
		}
	}
}

TitaniumElement.prototype.attr = function(name, value)
{
	if (value != null) {
		this.attributeList[name] = value;
	}
	return this.attributeList[name];
};

TitaniumElement.prototype.elements = function(name)
{
	if (name == null) return this.elementList;
	
	var els = [];
	for (var i = 0; i < this.elementList.length; i++) {
		if (this.elementList[i].name == name) els.push(this.elementList[i]);
	}
	
	return els;
};

TitaniumElement.prototype.element = function(name, value)
{
	var elements = this.elements(name);
	if (elements.length < 1) return null;
	var element = elements[0];
	
	if (value != null) {
		element.value = value;
	}
	return element;
};

TitaniumElement.prototype.elementValue = function(name)
{
	var el = this.element(name);
	if (el != null) {
		return el.value;
	}
	return null;
};

TitaniumElement.prototype.eachElement = function (name, f)
{
	var callback = null;
	var filter = null;
	
	if (typeof(name) == 'function')
	{
		callback = name;
	}
	else if (typeof(name) == 'string')
	{
		filter = name;
		callback = f;
	}
	
	var els = this.elements(name);
	for (var i = 0; i < els.length; i++) {
		f(els[i]);
	}
};

function TitaniumDocument (contents)
{
	if (typeof(contents) == 'string')
	{
			this.initFromString(contents);
	}
	else if ('childNodes' in contents)
	{
		this.initFromDOM(contents)
	}
};

TitaniumDocument.prototype.initFromDOM = function(doc)
{
	this.rootElement = new TitaniumElement(doc.documentElement);
};

TitaniumDocument.prototype.initFromString = function(string)
{
	var parser = new DOMParser();
	var doc = parser.parseFromString(string, "text/xml");
	
	this.initFromDOM(doc);
};

TitaniumDocument.prototype.root = function() {
	return this.rootElement;
};

ti.XML.Document = TitaniumDocument;
ti.XML.Element = TitaniumElement;