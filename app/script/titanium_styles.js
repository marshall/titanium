
Appcelerator.Titanium.Styles = {
	styles: {}
};

Appcelerator.Titanium.Styles.createToken = function (tokenNode)
{
	var tokenId = tokenNode.getAttribute("id");
	var tokenName = tokenNode.getAttribute("name");
	var fgColor = tokenNode.getAttribute("fg");
	var bgColor = tokenNode.getAttribute("bg");
	var bold = tokenNode.getAttribute("bold");
	var italic = tokenNode.getAttribute("italic");
	var font = tokenNode.getAttribute("font");
	
	var token = { id: tokenId, name: tokenName };
	token.fg = fgColor;
	token.bg = bgColor;
	token.bold = bold == null ? false : bold == "true";
	token.italic = italic == null ? false : italic == "true";
	token.font = font;
	
	return token;
};

Appcelerator.Titanium.Styles.createStyle = function (styleNode)
{
	var styleName = styleNode.getAttribute("name");
	Appcelerator.Titanium.Styles.styles[styleName] = { tokens: [], languageTokens: {} };
	
	for (var i = 0; i < styleNode.childNodes.length; i++) {
		var child = styleNode.childNodes[i];
		if (child.localName == "token") {
			Appcelerator.Titanium.Styles.styles[styleName].tokens.push(Appcelerator.Titanium.Styles.createToken(child));
		}
		else if (child.localName == "language") {
			Appcelerator.Titanium.Styles.createLanguage(styleName, child);
		}
	}
};

Appcelerator.Titanium.Styles.createLanguage = function (styleName, languageNode)
{
	var languageId = languageNode.getAttribute("id");
	Appcelerator.Titanium.Styles.styles[styleName].languageTokens[languageId] = [];
	
	for (var i = 0; i < languageNode.childNodes.length; i++) {
		var child = languageNode.childNodes[i];
		if (child.localName == "token") {
			
			Appcelerator.Titanium.Styles.styles[styleName].languageTokens[languageId].push(
				Appcelerator.Titanium.Styles.createToken(child));
		}
	}
};

Appcelerator.Titanium.Styles.init = function (url)
{
	var doc = Appcelerator.Titanium.Core.XML.parseFromURL(url);
	var stylesNode = doc.childNodes[0];
	
	for (var i = 0; i < stylesNode.childNodes.length; i++) {
		var child = stylesNode.childNodes[i];
		if (child.localName == "style") {
			Appcelerator.Titanium.Styles.createStyle(child);
		}
	}
	
	Appcelerator.Titanium.Styles.styles['default'].languageTokens['javascript'] = 
		Appcelerator.Titanium.Styles.getLanguageTokens('default', 'hypertext');
};

Appcelerator.Titanium.Styles.getStyleTokens = function (style)
{
	return Appcelerator.Titanium.Styles.styles[style].tokens;
};

Appcelerator.Titanium.Styles.getLanguageTokens = function (style, language)
{
	return Appcelerator.Titanium.Styles.styles[style].languageTokens[language];
};
