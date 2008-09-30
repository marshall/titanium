
Appcelerator.Titanium.Styles = new Object();

Appcelerator.Titanium.Styles.xml =  {
	_lineNumber: {
		foreground: '#9c9c9c', background: '#5e5e5e'
	},
	_default: {
		foreground: '#f8f8f8', background: '#2b2b2b', bold: true,
		font: 'Monaco'
	},
	_caret: {
		foreground: '#f8f8f8'
	},
	0: { /* Whitespace */
		foreground:	'#f8f8f8', background: '#2b2b2b'
	},
	1: { /* Tags */
		foreground: '#7f90aa', background: '#2b2b2b'
	},
	2: { /* Unknown tags */
		foreground: '#7f90aa', background: '#2b2b2b'
	},
	3: { /* Known attributes */
		foreground: '#7f90aa', background: '#2b2b2b'
	},
	4: { /* Unknown attributes */
		foreground: '#7f90aa', background: '#2b2b2b'
	},
	5: { /* Numbers */
		foreground: '#61ce3c', background: '#2b2b2b'
	},
	6: { /* Double quoted string */
		foreground: '#61ce3c', background: '#2b2b2b'
	},
	7: { /* Single quoted string */
		foreground: '#81ee5c', background: '#2b2b2b'
	},
	8: { /* "Other" inside tring */
		foreground: '#61ce3c', background: '#2b2b2b'
	},
	9: { /* Comment */
		foreground: '#7c7c7c', background: '#2b2b2b', italic: true
	},
	10: { /* Entities */
		foreground: '#0000ff', background: '#2b2b2b', bold: true
	},
	11: { /* /> tag endings */
		foreground: '#a5a5e5', background: '#2b2b2b'
	},
	12: { /* <? identifier begin */
		foreground: '#61ce3c', background: '#2b2b2b'
	},
	13: { /* ?> identifier end */
		foreground: '#61ce3c', background: '#2b2b2b'
	},
	17: { /* CDATA */
		foreground: '#61ce3c', background: '#2b2b2b'
	},
	21: { /* SGML default */
		foreground: '#000000', background: '#a6caf0'
	},
	24: { /* SGML double string? */
		foreground: '#61ce3c', background: '#2b2b2b'
	},
	25: { /* SGML single string? */
		foreground: '#61ce3c', background: '#2b2b2b'
	},
};

Appcelerator.Titanium.Styles.html = Appcelerator.Titanium.Styles.xml;
