
function colorToInt (color) {
		var c = color;
		if (color.charAt(0) == '#') {
			c = c.substr(1);
		}
		
		var value = new Number(parseInt(c,16));
		var red = ((value>>16) & 0xFF);
		var blue = (value & 0xFF);
		var green = (value & 0xFF00);
		
		value = (blue << 16) | green | red;
		
		alert("color="+color+",red="+red+",green="+(green>>8)+",blue="+blue);
		
		return value;
}

Appcelerator.Titanium.Editor = function () { };

Appcelerator.Titanium.Editor.prototype.init = function (editor) {
	this.editor = editor;
	
	//this.editor.sendMessage(Appcelerator.Titanium.Constants.SCI_SETCARETWIDTH, 5, 0);
	//this.editor.sendMessage(Appcelerator.Titanium.Constants.SCI_SETCARETSTYLE, Appcelerator.Titanium.Constants.CARETSTYLE_BLOCK, 0);
};

Appcelerator.Titanium.Editor.prototype.getEditor = function () {
	return this.editor;
};

Appcelerator.Titanium.Editor.prototype.setStyleForeground = function (styleNumber, fgColor) {
	//return this.editor.sendMessage(
	//	Appcelerator.Titanium.Constants.SCI_STYLESETFORE,
	//	styleNumber,
	//	colorToInt(fgColor));
	this.editor.setStyleForeground(styleNumber, fgColor);
};

Appcelerator.Titanium.Editor.prototype.setStyleBackground = function (styleNumber, bgColor) {
	//return this.editor.sendMessage(
	//	Appcelerator.Titanium.Constants.SCI_STYLESETBACK,
	//	styleNumber,
	//	colorToInt(bgColor));
		
	this.editor.setStyleBackground(styleNumber, bgColor);
};

Appcelerator.Titanium.Editor.prototype.setStyleItalic = function (styleNumber, italic) {
	return this.editor.sendMessage(
		Appcelerator.Titanium.Constants.SCI_SETSTYLEITALIC,
		styleNumber,
		italic ? 1 : 0);
};

Appcelerator.Titanium.Editor.prototype.setStyleBold = function (styleNumber, bold) {
	return this.editor.sendMessage( 
		Appcelerator.Titanium.Constants.SCI_SETSTYLEBOLD,
		styleNumber,
		bold ? 1 : 0);
};

Appcelerator.Titanium.Editor.prototype.setStyleFont = function (styleNumber, font) {
	this.editor.setStyleFont(styleNumber, font);
	
	//return this.editor.sendMessage(
	//	Appcelerator.Titanium.Constants.SCI_STYLESETFONT,
	//	styleNumber,
	//	font);
};

Appcelerator.Titanium.Editor.prototype.setLexer = function (lexer) {
	return this.editor.sendMessage(
		Appcelerator.Titanium.Constants.SCI_SETLEXER, lexer);
};

Appcelerator.Titanium.Editor.prototype.setLanguage = function (language) {
	this.language = language;
	return this.editor.sendMessage(
		Appcelerator.Titanium.Constants.SCI_SETLEXERLANGUAGE, language);
};

Appcelerator.Titanium.Editor.prototype.setCaretForeground = function (fgColor) {
	//return this.editor.sendMessage(
	//	Appcelerator.Titanium.Constants.SCI_SETCARETFORE, fgColor);
	this.editor.setCaretForeground(fgColor);
};

Appcelerator.Titanium.Editor.prototype.applyStyleToken = function (token)
{
	var styleNumber = 0;
	if (token.id == "_default") { styleNumber = 32; }
	else if (token.id == "_lineNumber") { styleNumber = 33; }
	else if (token.id == '_caret') { 
		this.setCaretForeground(token.fg); return;
	}
	else { styleNumber = parseInt(token.id); }
		
	if (token.fg != null) {
		this.setStyleForeground(styleNumber, token.fg);
	}
	if (token.bg != null) {
		this.setStyleBackground(styleNumber, token.bg);
	}
	if ('bold' in token) {
		this.setStyleBold(styleNumber, token.bold);
	}
	if ('italic' in token) {
		this.setStyleItalic(styleNumber, token.italic);
	}
	if ('font' in token && token.font != null) {
		this.setStyleFont(styleNumber, token.font);
	}
};

Appcelerator.Titanium.Editor.prototype.applyStyle = function (styleName) {
	
	var styles = Appcelerator.Titanium.Styles;
	var tokens = styles.getStyleTokens(styleName);
	var languageTokens = styles.getLanguageTokens(styleName, this.language);
	
	for (var i = 0; i < tokens.length; i++) {
		var token = tokens[i];
		
		this.applyStyleToken(token);
	}
	
	for (var i = 0; i < languageTokens.length; i++) {
		var token = languageTokens[i];
		
		this.applyStyleToken(token);
	}
	
	this.editor.redraw();
};

Appcelerator.Titanium.Editor.prototype.openFile = function (fullPath) {
	var extension = fullPath.substr(fullPath.lastIndexOf('.')+1);
	this.editor.openFile(fullPath);

	if (extension == "py") {
		this.setLexer(Appcelerator.Titanium.Constants.SCLEX_PYTHON);
		this.setLanguage("python");
	} else if (extension == "rb") {
		this.setLexer(Appcelerator.Titanium.Constants.SCLEX_RUBY);
		this.setLanguage("ruby");
	} else if (extension == "html")  {
		this.setLexer(Appcelerator.Titanium.Constants.SCLEX_HTML);
		this.setLanguage("hypertext");
	} else if (extension == "xml") {
		this.setLexer(Appcelerator.Titanium.Constants.SCLEX_XML);
		this.setLanguage("xml");
	} else if (extension == "pl") {
		this.setLexer(Appcelerator.Titanium.Constants.SCLEX_PERL);
		this.setLanguage("perl");
	} else if (extension == "js") {
		//this.setLexer(Appcelerator.Titanium.Constants.SCLEX_NULL);
		this.setLanguage("javascript");
	}

	this.applyStyle("default");
};

Appcelerator.Titanium.Editor.prototype.saveFile = function () {
	this.editor.saveFile();
};
