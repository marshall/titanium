
ti.debug("here 1");
ti.XML = {};
ti.XML.ElementNode = 1;
ti.XML.AttributeNode = 2;
ti.XML.TextNode = 3;
ti.debug("here 2");
function TitaniumElement (element)
{
	this.name = element.nodeName;
	this.type = ti.XML.ElementNode;
	this.attributes = {};
	this.elements = [];
	this.value = "";
	
	for (var i = 0; i < element.attributes.length; i++)
	{
		var attr = element.attributes.item(i);
		this.attributes[attr.nodeName] = attr.nodeValue;
	}
	
	for (var i = 0; i < element.childNodes.length; i++)
	{
		if (element.childNodes[i].nodeType == ti.XML.ElementNode) {
			this.children.push(new TitaniumElement(element.childNodes[i]));
		}
		else if (element.childNodes[i].nodeType == ti.XML.TextNode) {
			this.value += element.childNodes[i].nodeValue;
		}
	}
}
ti.debug("here 3");
TitaniumElement.prototype.attr = function(name, value)
{
	if (value != null) {
		attribute[name] = value;
	}
	return attributes[name];
};
ti.debug("here 4");
TitaniumElement.prototype.elements = function(name)
{
	if (name == null) return this.elements;
	
	var els = [];
	for (var i = 0; i < this.elements.length; i++) {
		if (this.elements[i].name == name) els.push(this.elements[i]);
	}
	
	return els;
};

TitaniumElement.prototype.element = function(name, value)
{
	var elements = this.elements(name);
	if (elements.length != 1) return;
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
	ti.debug('doc constructor');
	
	if ('childNodes' in contents)
	{
		ti.debug("init from dom");
		this.initFromDOM(contents)
	}
	else
	{
		ti.debug("init from string");
		this.initFromString(contents);
	}
};

TitaniumDocument.prototype.initFromDOM = function(doc)
{
	this.rootElement = new TitaniumElement(doc.documentElement);
};

TitaniumDocument.prototype.initFromString = function(string)
{
	ti.debug("parsing string: " + string);
	
	var parser = new DOMParser();
	var doc = parser.parseFromString(string, "text/xml");
	
	ti.debug("initializing ti.XML.Doc from " + doc);
	this.initFromDoc(doc);
};

TitaniumDocument.prototype.root = function() {
	return this.rootElement;
};

ti.XML.Document = TitaniumDocument;
ti.XML.Element = TitaniumElement;