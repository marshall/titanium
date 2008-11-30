/*!(c) 2006-2008 Appcelerator, Inc. http://appcelerator.org
 * Licensed under the Apache License, Version 2.0. Please visit
 * http://license.appcelerator.com for full copy of the License.
 * Version: 3.0.0, Released: 11/04/2008
 **/

/*- The following file(s) are subject to license agreements by their respective license owners. Ends at text: END THIRD PARTY SOURCE */

if (typeof(jQuery)=='undefined'){
(function(){
/*!
 * jQuery 1.2.6 - New Wave Javascript
 *
 * Copyright (c) 2008 John Resig (jquery.com)
 * Dual licensed under the MIT (MIT-LICENSE.txt)
 * and GPL (GPL-LICENSE.txt) licenses.
 *
 * $Date: 2008-05-24 14:22:17 -0400 (Sat, 24 May 2008) $
 * $Rev: 5685 $
 */

// Map over jQuery in case of overwrite
var _jQuery = window.jQuery,
// Map over the $ in case of overwrite
	_$ = window.$;

var jQuery = window.jQuery = window.$ = function( selector, context ) {
	// The jQuery object is actually just the init constructor 'enhanced'
	return new jQuery.fn.init( selector, context );
};

// A simple way to check for HTML strings or ID strings
// (both of which we optimize for)
var quickExpr = /^[^<]*(<(.|\s)+>)[^>]*$|^#(\w+)$/,

// Is it a simple selector
	isSimple = /^.[^:#\[\.]*$/,

// Will speed up references to undefined, and allows munging its name.
	undefined;

jQuery.fn = jQuery.prototype = {
	init: function( selector, context ) {
		// Make sure that a selection was provided
		selector = selector || document;

		// Handle $(DOMElement)
		if ( selector.nodeType ) {
			this[0] = selector;
			this.length = 1;
			return this;
		}
		
// Appcelerator patch for jQuery that supports backwards compatible-mode which attempts to emulate
// Prototype's $() that allows you to just pass ID instead of #id (which is correct css)
if ( window.AppceleratorjQueryPatch && typeof(selector)=='string' && selector.charAt(0)!='#' && /\w+/.test(selector) )  
{
	var elem = document.getElementById(selector);
	if (elem)
	{
		// if found we return the unwrapped DOMElement object instead of the 
		// wrapped jQuery object given that's what Prototype (sortof) returns
		return elem;
	}
}
// Handle HTML strings
		if ( typeof selector == "string" ) {
			// Are we dealing with HTML string or an ID?
			var match = quickExpr.exec( selector );

			// Verify a match, and that no context was specified for #id
			if ( match && (match[1] || !context) ) {

				// HANDLE: $(html) -> $(array)
				if ( match[1] )
					selector = jQuery.clean( [ match[1] ], context );

				// HANDLE: $("#id")
				else {
					var elem = document.getElementById( match[3] );

					// Make sure an element was located
					if ( elem ){
						// Handle the case where IE and Opera return items
						// by name instead of ID
						if ( elem.id != match[3] )
							return jQuery().find( selector );

						// Otherwise, we inject the element directly into the jQuery object
						return jQuery( elem );
					}
					selector = [];
				}

			// HANDLE: $(expr, [context])
			// (which is just equivalent to: $(content).find(expr)
			} else
				return jQuery( context ).find( selector );

		// HANDLE: $(function)
		// Shortcut for document ready
		} else if ( jQuery.isFunction( selector ) )
			return jQuery( document )[ jQuery.fn.ready ? "ready" : "load" ]( selector );

		return this.setArray(jQuery.makeArray(selector));
	},

	// The current version of jQuery being used
	jquery: "1.2.6",

	// The number of elements contained in the matched element set
	size: function() {
		return this.length;
	},

	// The number of elements contained in the matched element set
	length: 0,

	// Get the Nth element in the matched element set OR
	// Get the whole matched element set as a clean array
	get: function( num ) {
		return num == undefined ?

			// Return a 'clean' array
			jQuery.makeArray( this ) :

			// Return just the object
			this[ num ];
	},

	// Take an array of elements and push it onto the stack
	// (returning the new matched element set)
	pushStack: function( elems ) {
		// Build a new jQuery matched element set
		var ret = jQuery( elems );

		// Add the old object onto the stack (as a reference)
		ret.prevObject = this;

		// Return the newly-formed element set
		return ret;
	},

	// Force the current matched set of elements to become
	// the specified array of elements (destroying the stack in the process)
	// You should use pushStack() in order to do this, but maintain the stack
	setArray: function( elems ) {
		// Resetting the length to 0, then using the native Array push
		// is a super-fast way to populate an object with array-like properties
		this.length = 0;
		Array.prototype.push.apply( this, elems );

		return this;
	},

	// Execute a callback for every element in the matched set.
	// (You can seed the arguments with an array of args, but this is
	// only used internally.)
	each: function( callback, args ) {
		return jQuery.each( this, callback, args );
	},

	// Determine the position of an element within
	// the matched set of elements
	index: function( elem ) {
		var ret = -1;

		// Locate the position of the desired element
		return jQuery.inArray(
			// If it receives a jQuery object, the first element is used
			elem && elem.jquery ? elem[0] : elem
		, this );
	},

	attr: function( name, value, type ) {
		var options = name;

		// Look for the case where we're accessing a style value
		if ( name.constructor == String )
			if ( value === undefined )
				return this[0] && jQuery[ type || "attr" ]( this[0], name );

			else {
				options = {};
				options[ name ] = value;
			}

		// Check to see if we're setting style values
		return this.each(function(i){
			// Set all the styles
			for ( name in options )
				jQuery.attr(
					type ?
						this.style :
						this,
					name, jQuery.prop( this, options[ name ], type, i, name )
				);
		});
	},

	css: function( key, value ) {
		// ignore negative width and height values
		if ( (key == 'width' || key == 'height') && parseFloat(value) < 0 )
			value = undefined;
		return this.attr( key, value, "curCSS" );
	},

	text: function( text ) {
		if ( typeof text != "object" && text != null )
			return this.empty().append( (this[0] && this[0].ownerDocument || document).createTextNode( text ) );

		var ret = "";

		jQuery.each( text || this, function(){
			jQuery.each( this.childNodes, function(){
				if ( this.nodeType != 8 )
					ret += this.nodeType != 1 ?
						this.nodeValue :
						jQuery.fn.text( [ this ] );
			});
		});

		return ret;
	},

	wrapAll: function( html ) {
		if ( this[0] )
			// The elements to wrap the target around
			jQuery( html, this[0].ownerDocument )
				.clone()
				.insertBefore( this[0] )
				.map(function(){
					var elem = this;

					while ( elem.firstChild )
						elem = elem.firstChild;

					return elem;
				})
				.append(this);

		return this;
	},

	wrapInner: function( html ) {
		return this.each(function(){
			jQuery( this ).contents().wrapAll( html );
		});
	},

	wrap: function( html ) {
		return this.each(function(){
			jQuery( this ).wrapAll( html );
		});
	},

	append: function() {
		return this.domManip(arguments, true, false, function(elem){
			if (this.nodeType == 1)
				this.appendChild( elem );
		});
	},

	prepend: function() {
		return this.domManip(arguments, true, true, function(elem){
			if (this.nodeType == 1)
				this.insertBefore( elem, this.firstChild );
		});
	},

	before: function() {
		return this.domManip(arguments, false, false, function(elem){
			this.parentNode.insertBefore( elem, this );
		});
	},

	after: function() {
		return this.domManip(arguments, false, true, function(elem){
			this.parentNode.insertBefore( elem, this.nextSibling );
		});
	},

	end: function() {
		return this.prevObject || jQuery( [] );
	},

	find: function( selector ) {
		var elems = jQuery.map(this, function(elem){
			return jQuery.find( selector, elem );
		});

		return this.pushStack( /[^+>] [^+>]/.test( selector ) || selector.indexOf("..") > -1 ?
			jQuery.unique( elems ) :
			elems );
	},

	clone: function( events ) {
		// Do the clone
		var ret = this.map(function(){
			if ( jQuery.browser.msie && !jQuery.isXMLDoc(this) ) {
				// IE copies events bound via attachEvent when
				// using cloneNode. Calling detachEvent on the
				// clone will also remove the events from the orignal
				// In order to get around this, we use innerHTML.
				// Unfortunately, this means some modifications to
				// attributes in IE that are actually only stored
				// as properties will not be copied (such as the
				// the name attribute on an input).
				var clone = this.cloneNode(true),
					container = document.createElement("div");
				container.appendChild(clone);
				return jQuery.clean([container.innerHTML])[0];
			} else
				return this.cloneNode(true);
		});

		// Need to set the expando to null on the cloned set if it exists
		// removeData doesn't work here, IE removes it from the original as well
		// this is primarily for IE but the data expando shouldn't be copied over in any browser
		var clone = ret.find("*").andSelf().each(function(){
			if ( this[ expando ] != undefined )
				this[ expando ] = null;
		});

		// Copy the events from the original to the clone
		if ( events === true )
			this.find("*").andSelf().each(function(i){
				if (this.nodeType == 3)
					return;
				var events = jQuery.data( this, "events" );

				for ( var type in events )
					for ( var handler in events[ type ] )
						jQuery.event.add( clone[ i ], type, events[ type ][ handler ], events[ type ][ handler ].data );
			});

		// Return the cloned set
		return ret;
	},

	filter: function( selector ) {
		return this.pushStack(
			jQuery.isFunction( selector ) &&
			jQuery.grep(this, function(elem, i){
				return selector.call( elem, i );
			}) ||

			jQuery.multiFilter( selector, this ) );
	},

	not: function( selector ) {
		if ( selector.constructor == String )
			// test special case where just one selector is passed in
			if ( isSimple.test( selector ) )
				return this.pushStack( jQuery.multiFilter( selector, this, true ) );
			else
				selector = jQuery.multiFilter( selector, this );

		var isArrayLike = selector.length && selector[selector.length - 1] !== undefined && !selector.nodeType;
		return this.filter(function() {
			return isArrayLike ? jQuery.inArray( this, selector ) < 0 : this != selector;
		});
	},

	add: function( selector ) {
		return this.pushStack( jQuery.unique( jQuery.merge(
			this.get(),
			typeof selector == 'string' ?
				jQuery( selector ) :
				jQuery.makeArray( selector )
		)));
	},

	is: function( selector ) {
		return !!selector && jQuery.multiFilter( selector, this ).length > 0;
	},

	hasClass: function( selector ) {
		return this.is( "." + selector );
	},

	val: function( value ) {
		if ( value == undefined ) {

			if ( this.length ) {
				var elem = this[0];

				// We need to handle select boxes special
				if ( jQuery.nodeName( elem, "select" ) ) {
					var index = elem.selectedIndex,
						values = [],
						options = elem.options,
						one = elem.type == "select-one";

					// Nothing was selected
					if ( index < 0 )
						return null;

					// Loop through all the selected options
					for ( var i = one ? index : 0, max = one ? index + 1 : options.length; i < max; i++ ) {
						var option = options[ i ];

						if ( option.selected ) {
							// Get the specifc value for the option
							value = jQuery.browser.msie && !option.attributes.value.specified ? option.text : option.value;

							// We don't need an array for one selects
							if ( one )
								return value;

							// Multi-Selects return an array
							values.push( value );
						}
					}

					return values;

				// Everything else, we just grab the value
				} else
					return (this[0].value || "").replace(/\r/g, "");

			}

			return undefined;
		}

		if( value.constructor == Number )
			value += '';

		return this.each(function(){
			if ( this.nodeType != 1 )
				return;

			if ( value.constructor == Array && /radio|checkbox/.test( this.type ) )
				this.checked = (jQuery.inArray(this.value, value) >= 0 ||
					jQuery.inArray(this.name, value) >= 0);

			else if ( jQuery.nodeName( this, "select" ) ) {
				var values = jQuery.makeArray(value);

				jQuery( "option", this ).each(function(){
					this.selected = (jQuery.inArray( this.value, values ) >= 0 ||
						jQuery.inArray( this.text, values ) >= 0);
				});

				if ( !values.length )
					this.selectedIndex = -1;

			} else
				this.value = value;
		});
	},

	html: function( value ) {
		return value == undefined ?
			(this[0] ?
				this[0].innerHTML :
				null) :
			this.empty().append( value );
	},

	replaceWith: function( value ) {
		return this.after( value ).remove();
	},

	eq: function( i ) {
		return this.slice( i, i + 1 );
	},

	slice: function() {
		return this.pushStack( Array.prototype.slice.apply( this, arguments ) );
	},

	map: function( callback ) {
		return this.pushStack( jQuery.map(this, function(elem, i){
			return callback.call( elem, i, elem );
		}));
	},

	andSelf: function() {
		return this.add( this.prevObject );
	},

	data: function( key, value ){
		var parts = key.split(".");
		parts[1] = parts[1] ? "." + parts[1] : "";

		if ( value === undefined ) {
			var data = this.triggerHandler("getData" + parts[1] + "!", [parts[0]]);

			if ( data === undefined && this.length )
				data = jQuery.data( this[0], key );

			return data === undefined && parts[1] ?
				this.data( parts[0] ) :
				data;
		} else
			return this.trigger("setData" + parts[1] + "!", [parts[0], value]).each(function(){
				jQuery.data( this, key, value );
			});
	},

	removeData: function( key ){
		return this.each(function(){
			jQuery.removeData( this, key );
		});
	},

	domManip: function( args, table, reverse, callback ) {
		var clone = this.length > 1, elems;

		return this.each(function(){
			if ( !elems ) {
				elems = jQuery.clean( args, this.ownerDocument );

				if ( reverse )
					elems.reverse();
			}

			var obj = this;

			if ( table && jQuery.nodeName( this, "table" ) && jQuery.nodeName( elems[0], "tr" ) )
				obj = this.getElementsByTagName("tbody")[0] || this.appendChild( this.ownerDocument.createElement("tbody") );

			var scripts = jQuery( [] );

			jQuery.each(elems, function(){
				var elem = clone ?
					jQuery( this ).clone( true )[0] :
					this;

				// execute all scripts after the elements have been injected
				if ( jQuery.nodeName( elem, "script" ) )
					scripts = scripts.add( elem );
				else {
					// Remove any inner scripts for later evaluation
					if ( elem.nodeType == 1 )
						scripts = scripts.add( jQuery( "script", elem ).remove() );

					// Inject the elements into the document
					callback.call( obj, elem );
				}
			});

			scripts.each( evalScript );
		});
	}
};

// Give the init function the jQuery prototype for later instantiation
jQuery.fn.init.prototype = jQuery.fn;

function evalScript( i, elem ) {
	if ( elem.src )
		jQuery.ajax({
			url: elem.src,
			async: false,
			dataType: "script"
		});

	else
		jQuery.globalEval( elem.text || elem.textContent || elem.innerHTML || "" );

	if ( elem.parentNode )
		elem.parentNode.removeChild( elem );
}

function now(){
	return +new Date;
}

jQuery.extend = jQuery.fn.extend = function() {
	// copy reference to target object
	var target = arguments[0] || {}, i = 1, length = arguments.length, deep = false, options;

	// Handle a deep copy situation
	if ( target.constructor == Boolean ) {
		deep = target;
		target = arguments[1] || {};
		// skip the boolean and the target
		i = 2;
	}

	// Handle case when target is a string or something (possible in deep copy)
	if ( typeof target != "object" && typeof target != "function" )
		target = {};

	// extend jQuery itself if only one argument is passed
	if ( length == i ) {
		target = this;
		--i;
	}

	for ( ; i < length; i++ )
		// Only deal with non-null/undefined values
		if ( (options = arguments[ i ]) != null )
			// Extend the base object
			for ( var name in options ) {
				var src = target[ name ], copy = options[ name ];

				// Prevent never-ending loop
				if ( target === copy )
					continue;

				// Recurse if we're merging object values
				if ( deep && copy && typeof copy == "object" && !copy.nodeType )
					target[ name ] = jQuery.extend( deep, 
						// Never move original objects, clone them
						src || ( copy.length != null ? [ ] : { } )
					, copy );

				// Don't bring in undefined values
				else if ( copy !== undefined )
					target[ name ] = copy;

			}

	// Return the modified object
	return target;
};

var expando = "jQuery" + now(), uuid = 0, windowData = {},
	// exclude the following css properties to add px
	exclude = /z-?index|font-?weight|opacity|zoom|line-?height/i,
	// cache defaultView
	defaultView = document.defaultView || {};

jQuery.extend({
	noConflict: function( deep ) {
		window.$ = _$;

		if ( deep )
			window.jQuery = _jQuery;

		return jQuery;
	},

	// See test/unit/core.js for details concerning this function.
	isFunction: function( fn ) {
		return !!fn && typeof fn != "string" && !fn.nodeName &&
			fn.constructor != Array && /^[\s[]?function/.test( fn + "" );
	},

	// check if an element is in a (or is an) XML document
	isXMLDoc: function( elem ) {
		return elem.documentElement && !elem.body ||
			elem.tagName && elem.ownerDocument && !elem.ownerDocument.body;
	},

	// Evalulates a script in a global context
	globalEval: function( data ) {
		data = jQuery.trim( data );

		if ( data ) {
			// Inspired by code by Andrea Giammarchi
			// http://webreflection.blogspot.com/2007/08/global-scope-evaluation-and-dom.html
			var head = document.getElementsByTagName("head")[0] || document.documentElement,
				script = document.createElement("script");

			script.type = "text/javascript";
			if ( jQuery.browser.msie )
				script.text = data;
			else
				script.appendChild( document.createTextNode( data ) );

			// Use insertBefore instead of appendChild  to circumvent an IE6 bug.
			// This arises when a base node is used (#2709).
			head.insertBefore( script, head.firstChild );
			head.removeChild( script );
		}
	},

	nodeName: function( elem, name ) {
		return elem.nodeName && elem.nodeName.toUpperCase() == name.toUpperCase();
	},

	cache: {},

	data: function( elem, name, data ) {
		elem = elem == window ?
			windowData :
			elem;

		var id = elem[ expando ];

		// Compute a unique ID for the element
		if ( !id )
			id = elem[ expando ] = ++uuid;

		// Only generate the data cache if we're
		// trying to access or manipulate it
		if ( name && !jQuery.cache[ id ] )
			jQuery.cache[ id ] = {};

		// Prevent overriding the named cache with undefined values
		if ( data !== undefined )
			jQuery.cache[ id ][ name ] = data;

		// Return the named cache data, or the ID for the element
		return name ?
			jQuery.cache[ id ][ name ] :
			id;
	},

	removeData: function( elem, name ) {
		elem = elem == window ?
			windowData :
			elem;

		var id = elem[ expando ];

		// If we want to remove a specific section of the element's data
		if ( name ) {
			if ( jQuery.cache[ id ] ) {
				// Remove the section of cache data
				delete jQuery.cache[ id ][ name ];

				// If we've removed all the data, remove the element's cache
				name = "";

				for ( name in jQuery.cache[ id ] )
					break;

				if ( !name )
					jQuery.removeData( elem );
			}

		// Otherwise, we want to remove all of the element's data
		} else {
			// Clean up the element expando
			try {
				delete elem[ expando ];
			} catch(e){
				// IE has trouble directly removing the expando
				// but it's ok with using removeAttribute
				if ( elem.removeAttribute )
					elem.removeAttribute( expando );
			}

			// Completely remove the data cache
			delete jQuery.cache[ id ];
		}
	},

	// args is for internal usage only
	each: function( object, callback, args ) {
		var name, i = 0, length = object.length;

		if ( args ) {
			if ( length == undefined ) {
				for ( name in object )
					if ( callback.apply( object[ name ], args ) === false )
						break;
			} else
				for ( ; i < length; )
					if ( callback.apply( object[ i++ ], args ) === false )
						break;

		// A special, fast, case for the most common use of each
		} else {
			if ( length == undefined ) {
				for ( name in object )
					if ( callback.call( object[ name ], name, object[ name ] ) === false )
						break;
			} else
				for ( var value = object[0];
					i < length && callback.call( value, i, value ) !== false; value = object[++i] ){}
		}

		return object;
	},

	prop: function( elem, value, type, i, name ) {
		// Handle executable functions
		if ( jQuery.isFunction( value ) )
			value = value.call( elem, i );

		// Handle passing in a number to a CSS property
		return value && value.constructor == Number && type == "curCSS" && !exclude.test( name ) ?
			value + "px" :
			value;
	},

	className: {
		// internal only, use addClass("class")
		add: function( elem, classNames ) {
			jQuery.each((classNames || "").split(/\s+/), function(i, className){
				if ( elem.nodeType == 1 && !jQuery.className.has( elem.className, className ) )
					elem.className += (elem.className ? " " : "") + className;
			});
		},

		// internal only, use removeClass("class")
		remove: function( elem, classNames ) {
			if (elem.nodeType == 1)
				elem.className = classNames != undefined ?
					jQuery.grep(elem.className.split(/\s+/), function(className){
						return !jQuery.className.has( classNames, className );
					}).join(" ") :
					"";
		},

		// internal only, use hasClass("class")
		has: function( elem, className ) {
			return jQuery.inArray( className, (elem.className || elem).toString().split(/\s+/) ) > -1;
		}
	},

	// A method for quickly swapping in/out CSS properties to get correct calculations
	swap: function( elem, options, callback ) {
		var old = {};
		// Remember the old values, and insert the new ones
		for ( var name in options ) {
			old[ name ] = elem.style[ name ];
			elem.style[ name ] = options[ name ];
		}

		callback.call( elem );

		// Revert the old values
		for ( var name in options )
			elem.style[ name ] = old[ name ];
	},

	css: function( elem, name, force ) {
		if ( name == "width" || name == "height" ) {
			var val, props = { position: "absolute", visibility: "hidden", display:"block" }, which = name == "width" ? [ "Left", "Right" ] : [ "Top", "Bottom" ];

			function getWH() {
				val = name == "width" ? elem.offsetWidth : elem.offsetHeight;
				var padding = 0, border = 0;
				jQuery.each( which, function() {
					padding += parseFloat(jQuery.curCSS( elem, "padding" + this, true)) || 0;
					border += parseFloat(jQuery.curCSS( elem, "border" + this + "Width", true)) || 0;
				});
				val -= Math.round(padding + border);
			}

			if ( jQuery(elem).is(":visible") )
				getWH();
			else
				jQuery.swap( elem, props, getWH );

			return Math.max(0, val);
		}

		return jQuery.curCSS( elem, name, force );
	},

	curCSS: function( elem, name, force ) {
		var ret, style = elem.style;

		// A helper method for determining if an element's values are broken
		function color( elem ) {
			if ( !jQuery.browser.safari )
				return false;

			// defaultView is cached
			var ret = defaultView.getComputedStyle( elem, null );
			return !ret || ret.getPropertyValue("color") == "";
		}

		// We need to handle opacity special in IE
		if ( name == "opacity" && jQuery.browser.msie ) {
			ret = jQuery.attr( style, "opacity" );

			return ret == "" ?
				"1" :
				ret;
		}
		// Opera sometimes will give the wrong display answer, this fixes it, see #2037
		if ( jQuery.browser.opera && name == "display" ) {
			var save = style.outline;
			style.outline = "0 solid black";
			style.outline = save;
		}

		// Make sure we're using the right name for getting the float value
		if ( name.match( /float/i ) )
			name = styleFloat;

		if ( !force && style && style[ name ] )
			ret = style[ name ];

		else if ( defaultView.getComputedStyle ) {

			// Only "float" is needed here
			if ( name.match( /float/i ) )
				name = "float";

			name = name.replace( /([A-Z])/g, "-$1" ).toLowerCase();

			var computedStyle = defaultView.getComputedStyle( elem, null );

			if ( computedStyle && !color( elem ) )
				ret = computedStyle.getPropertyValue( name );

			// If the element isn't reporting its values properly in Safari
			// then some display: none elements are involved
			else {
				var swap = [], stack = [], a = elem, i = 0;

				// Locate all of the parent display: none elements
				for ( ; a && color(a); a = a.parentNode )
					stack.unshift(a);

				// Go through and make them visible, but in reverse
				// (It would be better if we knew the exact display type that they had)
				for ( ; i < stack.length; i++ )
					if ( color( stack[ i ] ) ) {
						swap[ i ] = stack[ i ].style.display;
						stack[ i ].style.display = "block";
					}

				// Since we flip the display style, we have to handle that
				// one special, otherwise get the value
				ret = name == "display" && swap[ stack.length - 1 ] != null ?
					"none" :
					( computedStyle && computedStyle.getPropertyValue( name ) ) || "";

				// Finally, revert the display styles back
				for ( i = 0; i < swap.length; i++ )
					if ( swap[ i ] != null )
						stack[ i ].style.display = swap[ i ];
			}

			// We should always get a number back from opacity
			if ( name == "opacity" && ret == "" )
				ret = "1";

		} else if ( elem.currentStyle ) {
			var camelCase = name.replace(/\-(\w)/g, function(all, letter){
				return letter.toUpperCase();
			});

			ret = elem.currentStyle[ name ] || elem.currentStyle[ camelCase ];

			// From the awesome hack by Dean Edwards
			// http://erik.eae.net/archives/2007/07/27/18.54.15/#comment-102291

			// If we're not dealing with a regular pixel number
			// but a number that has a weird ending, we need to convert it to pixels
			if ( !/^\d+(px)?$/i.test( ret ) && /^\d/.test( ret ) ) {
				// Remember the original values
				var left = style.left, rsLeft = elem.runtimeStyle.left;

				// Put in the new values to get a computed value out
				elem.runtimeStyle.left = elem.currentStyle.left;
				style.left = ret || 0;
				ret = style.pixelLeft + "px";

				// Revert the changed values
				style.left = left;
				elem.runtimeStyle.left = rsLeft;
			}
		}

		return ret;
	},

	clean: function( elems, context ) {
		var ret = [];
		context = context || document;
		// !context.createElement fails in IE with an error but returns typeof 'object'
		if (typeof context.createElement == 'undefined')
			context = context.ownerDocument || context[0] && context[0].ownerDocument || document;

		jQuery.each(elems, function(i, elem){
			if ( !elem )
				return;

			if ( elem.constructor == Number )
				elem += '';

			// Convert html string into DOM nodes
			if ( typeof elem == "string" ) {
				// Fix "XHTML"-style tags in all browsers
				elem = elem.replace(/(<(\w+)[^>]*?)\/>/g, function(all, front, tag){
					return tag.match(/^(abbr|br|col|img|input|link|meta|param|hr|area|embed)$/i) ?
						all :
						front + "></" + tag + ">";
				});

				// Trim whitespace, otherwise indexOf won't work as expected
				var tags = jQuery.trim( elem ).toLowerCase(), div = context.createElement("div");

				var wrap =
					// option or optgroup
					!tags.indexOf("<opt") &&
					[ 1, "<select multiple='multiple'>", "</select>" ] ||

					!tags.indexOf("<leg") &&
					[ 1, "<fieldset>", "</fieldset>" ] ||

					tags.match(/^<(thead|tbody|tfoot|colg|cap)/) &&
					[ 1, "<table>", "</table>" ] ||

					!tags.indexOf("<tr") &&
					[ 2, "<table><tbody>", "</tbody></table>" ] ||

				 	// <thead> matched above
					(!tags.indexOf("<td") || !tags.indexOf("<th")) &&
					[ 3, "<table><tbody><tr>", "</tr></tbody></table>" ] ||

					!tags.indexOf("<col") &&
					[ 2, "<table><tbody></tbody><colgroup>", "</colgroup></table>" ] ||

					// IE can't serialize <link> and <script> tags normally
					jQuery.browser.msie &&
					[ 1, "div<div>", "</div>" ] ||

					[ 0, "", "" ];

				// Go to html and back, then peel off extra wrappers
				div.innerHTML = wrap[1] + elem + wrap[2];

				// Move to the right depth
				while ( wrap[0]-- )
					div = div.lastChild;

				// Remove IE's autoinserted <tbody> from table fragments
				if ( jQuery.browser.msie ) {

					// String was a <table>, *may* have spurious <tbody>
					var tbody = !tags.indexOf("<table") && tags.indexOf("<tbody") < 0 ?
						div.firstChild && div.firstChild.childNodes :

						// String was a bare <thead> or <tfoot>
						wrap[1] == "<table>" && tags.indexOf("<tbody") < 0 ?
							div.childNodes :
							[];

					for ( var j = tbody.length - 1; j >= 0 ; --j )
						if ( jQuery.nodeName( tbody[ j ], "tbody" ) && !tbody[ j ].childNodes.length )
							tbody[ j ].parentNode.removeChild( tbody[ j ] );

					// IE completely kills leading whitespace when innerHTML is used
					if ( /^\s/.test( elem ) )
						div.insertBefore( context.createTextNode( elem.match(/^\s*/)[0] ), div.firstChild );

				}

				elem = jQuery.makeArray( div.childNodes );
			}

			if ( elem.length === 0 && (!jQuery.nodeName( elem, "form" ) && !jQuery.nodeName( elem, "select" )) )
				return;

			if ( elem[0] == undefined || jQuery.nodeName( elem, "form" ) || elem.options )
				ret.push( elem );

			else
				ret = jQuery.merge( ret, elem );

		});

		return ret;
	},

	attr: function( elem, name, value ) {
		// don't set attributes on text and comment nodes
		if (!elem || elem.nodeType == 3 || elem.nodeType == 8)
			return undefined;

		var notxml = !jQuery.isXMLDoc( elem ),
			// Whether we are setting (or getting)
			set = value !== undefined,
			msie = jQuery.browser.msie;

		// Try to normalize/fix the name
		name = notxml && jQuery.props[ name ] || name;

		// Only do all the following if this is a node (faster for style)
		// IE elem.getAttribute passes even for style
		if ( elem.tagName ) {

			// These attributes require special treatment
			var special = /href|src|style/.test( name );

			// Safari mis-reports the default selected property of a hidden option
			// Accessing the parent's selectedIndex property fixes it
			if ( name == "selected" && jQuery.browser.safari )
				elem.parentNode.selectedIndex;

			// If applicable, access the attribute via the DOM 0 way
			if ( name in elem && notxml && !special ) {
				if ( set ){
					// We can't allow the type property to be changed (since it causes problems in IE)
					if ( name == "type" && jQuery.nodeName( elem, "input" ) && elem.parentNode )
						throw "type property can't be changed";

					elem[ name ] = value;
				}

				// browsers index elements by id/name on forms, give priority to attributes.
				if( jQuery.nodeName( elem, "form" ) && elem.getAttributeNode(name) )
					return elem.getAttributeNode( name ).nodeValue;

				return elem[ name ];
			}

			if ( msie && notxml &&  name == "style" )
				return jQuery.attr( elem.style, "cssText", value );

			if ( set )
				// convert the value to a string (all browsers do this but IE) see #1070
				elem.setAttribute( name, "" + value );

			var attr = msie && notxml && special
					// Some attributes require a special call on IE
					? elem.getAttribute( name, 2 )
					: elem.getAttribute( name );

			// Non-existent attributes return null, we normalize to undefined
			return attr === null ? undefined : attr;
		}

		// elem is actually elem.style ... set the style

		// IE uses filters for opacity
		if ( msie && name == "opacity" ) {
			if ( set ) {
				// IE has trouble with opacity if it does not have layout
				// Force it by setting the zoom level
				elem.zoom = 1;

				// Set the alpha filter to set the opacity
				elem.filter = (elem.filter || "").replace( /alpha\([^)]*\)/, "" ) +
					(parseInt( value ) + '' == "NaN" ? "" : "alpha(opacity=" + value * 100 + ")");
			}

			return elem.filter && elem.filter.indexOf("opacity=") >= 0 ?
				(parseFloat( elem.filter.match(/opacity=([^)]*)/)[1] ) / 100) + '':
				"";
		}

		name = name.replace(/-([a-z])/ig, function(all, letter){
			return letter.toUpperCase();
		});

		if ( set )
			elem[ name ] = value;

		return elem[ name ];
	},

	trim: function( text ) {
		return (text || "").replace( /^\s+|\s+$/g, "" );
	},

	makeArray: function( array ) {
		var ret = [];

		if( array != null ){
			var i = array.length;
			//the window, strings and functions also have 'length'
			if( i == null || array.split || array.setInterval || array.call )
				ret[0] = array;
			else
				while( i )
					ret[--i] = array[i];
		}

		return ret;
	},

	inArray: function( elem, array ) {
		for ( var i = 0, length = array.length; i < length; i++ )
		// Use === because on IE, window == document
			if ( array[ i ] === elem )
				return i;

		return -1;
	},

	merge: function( first, second ) {
		// We have to loop this way because IE & Opera overwrite the length
		// expando of getElementsByTagName
		var i = 0, elem, pos = first.length;
		// Also, we need to make sure that the correct elements are being returned
		// (IE returns comment nodes in a '*' query)
		if ( jQuery.browser.msie ) {
			while ( elem = second[ i++ ] )
				if ( elem.nodeType != 8 )
					first[ pos++ ] = elem;

		} else
			while ( elem = second[ i++ ] )
				first[ pos++ ] = elem;

		return first;
	},

	unique: function( array ) {
		var ret = [], done = {};

		try {

			for ( var i = 0, length = array.length; i < length; i++ ) {
				var id = jQuery.data( array[ i ] );

				if ( !done[ id ] ) {
					done[ id ] = true;
					ret.push( array[ i ] );
				}
			}

		} catch( e ) {
			ret = array;
		}

		return ret;
	},

	grep: function( elems, callback, inv ) {
		var ret = [];

		// Go through the array, only saving the items
		// that pass the validator function
		for ( var i = 0, length = elems.length; i < length; i++ )
			if ( !inv != !callback( elems[ i ], i ) )
				ret.push( elems[ i ] );

		return ret;
	},

	map: function( elems, callback ) {
		var ret = [];

		// Go through the array, translating each of the items to their
		// new value (or values).
		for ( var i = 0, length = elems.length; i < length; i++ ) {
			var value = callback( elems[ i ], i );

			if ( value != null )
				ret[ ret.length ] = value;
		}

		return ret.concat.apply( [], ret );
	}
});

var userAgent = navigator.userAgent.toLowerCase();

// Figure out what browser is being used
jQuery.browser = {
	version: (userAgent.match( /.+(?:rv|it|ra|ie)[\/: ]([\d.]+)/ ) || [])[1],
	safari: /webkit/.test( userAgent ),
	opera: /opera/.test( userAgent ),
	msie: /msie/.test( userAgent ) && !/opera/.test( userAgent ),
	mozilla: /mozilla/.test( userAgent ) && !/(compatible|webkit)/.test( userAgent )
};

var styleFloat = jQuery.browser.msie ?
	"styleFloat" :
	"cssFloat";

jQuery.extend({
	// Check to see if the W3C box model is being used
	boxModel: !jQuery.browser.msie || document.compatMode == "CSS1Compat",

	props: {
		"for": "htmlFor",
		"class": "className",
		"float": styleFloat,
		cssFloat: styleFloat,
		styleFloat: styleFloat,
		readonly: "readOnly",
		maxlength: "maxLength",
		cellspacing: "cellSpacing"
	}
});

jQuery.each({
	parent: function(elem){return elem.parentNode;},
	parents: function(elem){return jQuery.dir(elem,"parentNode");},
	next: function(elem){return jQuery.nth(elem,2,"nextSibling");},
	prev: function(elem){return jQuery.nth(elem,2,"previousSibling");},
	nextAll: function(elem){return jQuery.dir(elem,"nextSibling");},
	prevAll: function(elem){return jQuery.dir(elem,"previousSibling");},
	siblings: function(elem){return jQuery.sibling(elem.parentNode.firstChild,elem);},
	children: function(elem){return jQuery.sibling(elem.firstChild);},
	contents: function(elem){return jQuery.nodeName(elem,"iframe")?elem.contentDocument||elem.contentWindow.document:jQuery.makeArray(elem.childNodes);}
}, function(name, fn){
	jQuery.fn[ name ] = function( selector ) {
		var ret = jQuery.map( this, fn );

		if ( selector && typeof selector == "string" )
			ret = jQuery.multiFilter( selector, ret );

		return this.pushStack( jQuery.unique( ret ) );
	};
});

jQuery.each({
	appendTo: "append",
	prependTo: "prepend",
	insertBefore: "before",
	insertAfter: "after",
	replaceAll: "replaceWith"
}, function(name, original){
	jQuery.fn[ name ] = function() {
		var args = arguments;

		return this.each(function(){
			for ( var i = 0, length = args.length; i < length; i++ )
				jQuery( args[ i ] )[ original ]( this );
		});
	};
});

jQuery.each({
	removeAttr: function( name ) {
		jQuery.attr( this, name, "" );
		if (this.nodeType == 1)
			this.removeAttribute( name );
	},

	addClass: function( classNames ) {
		jQuery.className.add( this, classNames );
	},

	removeClass: function( classNames ) {
		jQuery.className.remove( this, classNames );
	},

	toggleClass: function( classNames ) {
		jQuery.className[ jQuery.className.has( this, classNames ) ? "remove" : "add" ]( this, classNames );
	},

	remove: function( selector ) {
		if ( !selector || jQuery.filter( selector, [ this ] ).r.length ) {
			// Prevent memory leaks
			jQuery( "*", this ).add(this).each(function(){
				jQuery.event.remove(this);
				jQuery.removeData(this);
			});
			if (this.parentNode)
				this.parentNode.removeChild( this );
		}
	},

	empty: function() {
		// Remove element nodes and prevent memory leaks
		jQuery( ">*", this ).remove();

		// Remove any remaining nodes
		while ( this.firstChild )
			this.removeChild( this.firstChild );
	}
}, function(name, fn){
	jQuery.fn[ name ] = function(){
		return this.each( fn, arguments );
	};
});

jQuery.each([ "Height", "Width" ], function(i, name){
	var type = name.toLowerCase();

	jQuery.fn[ type ] = function( size ) {
		// Get window width or height
		return this[0] == window ?
			// Opera reports document.body.client[Width/Height] properly in both quirks and standards
			jQuery.browser.opera && document.body[ "client" + name ] ||

			// Safari reports inner[Width/Height] just fine (Mozilla and Opera include scroll bar widths)
			jQuery.browser.safari && window[ "inner" + name ] ||

			// Everyone else use document.documentElement or document.body depending on Quirks vs Standards mode
			document.compatMode == "CSS1Compat" && document.documentElement[ "client" + name ] || document.body[ "client" + name ] :

			// Get document width or height
			this[0] == document ?
				// Either scroll[Width/Height] or offset[Width/Height], whichever is greater
				Math.max(
					Math.max(document.body["scroll" + name], document.documentElement["scroll" + name]),
					Math.max(document.body["offset" + name], document.documentElement["offset" + name])
				) :

				// Get or set width or height on the element
				size == undefined ?
					// Get width or height on the element
					(this.length ? jQuery.css( this[0], type ) : null) :

					// Set the width or height on the element (default to pixels if value is unitless)
					this.css( type, size.constructor == String ? size : size + "px" );
	};
});

// Helper function used by the dimensions and offset modules
function num(elem, prop) {
	return elem[0] && parseInt( jQuery.curCSS(elem[0], prop, true), 10 ) || 0;
}var chars = jQuery.browser.safari && parseInt(jQuery.browser.version) < 417 ?
		"(?:[\\w*_-]|\\\\.)" :
		"(?:[\\w\u0128-\uFFFF*_-]|\\\\.)",
	quickChild = new RegExp("^>\\s*(" + chars + "+)"),
	quickID = new RegExp("^(" + chars + "+)(#)(" + chars + "+)"),
	quickClass = new RegExp("^([#.]?)(" + chars + "*)");

jQuery.extend({
	expr: {
		"": function(a,i,m){return m[2]=="*"||jQuery.nodeName(a,m[2]);},
		"#": function(a,i,m){return a.getAttribute("id")==m[2];},
		":": {
			// Position Checks
			lt: function(a,i,m){return i<m[3]-0;},
			gt: function(a,i,m){return i>m[3]-0;},
			nth: function(a,i,m){return m[3]-0==i;},
			eq: function(a,i,m){return m[3]-0==i;},
			first: function(a,i){return i==0;},
			last: function(a,i,m,r){return i==r.length-1;},
			even: function(a,i){return i%2==0;},
			odd: function(a,i){return i%2;},

			// Child Checks
			"first-child": function(a){return a.parentNode.getElementsByTagName("*")[0]==a;},
			"last-child": function(a){return jQuery.nth(a.parentNode.lastChild,1,"previousSibling")==a;},
			"only-child": function(a){return !jQuery.nth(a.parentNode.lastChild,2,"previousSibling");},

			// Parent Checks
			parent: function(a){return a.firstChild;},
			empty: function(a){return !a.firstChild;},

			// Text Check
			contains: function(a,i,m){return (a.textContent||a.innerText||jQuery(a).text()||"").indexOf(m[3])>=0;},

			// Visibility
			visible: function(a){return "hidden"!=a.type&&jQuery.css(a,"display")!="none"&&jQuery.css(a,"visibility")!="hidden";},
			hidden: function(a){return "hidden"==a.type||jQuery.css(a,"display")=="none"||jQuery.css(a,"visibility")=="hidden";},

			// Form attributes
			enabled: function(a){return !a.disabled;},
			disabled: function(a){return a.disabled;},
			checked: function(a){return a.checked;},
			selected: function(a){return a.selected||jQuery.attr(a,"selected");},

			// Form elements
			text: function(a){return "text"==a.type;},
			radio: function(a){return "radio"==a.type;},
			checkbox: function(a){return "checkbox"==a.type;},
			file: function(a){return "file"==a.type;},
			password: function(a){return "password"==a.type;},
			submit: function(a){return "submit"==a.type;},
			image: function(a){return "image"==a.type;},
			reset: function(a){return "reset"==a.type;},
			button: function(a){return "button"==a.type||jQuery.nodeName(a,"button");},
			input: function(a){return /input|select|textarea|button/i.test(a.nodeName);},

			// :has()
			has: function(a,i,m){return jQuery.find(m[3],a).length;},

			// :header
			header: function(a){return /h\d/i.test(a.nodeName);},

			// :animated
			animated: function(a){return jQuery.grep(jQuery.timers,function(fn){return a==fn.elem;}).length;}
		}
	},

	// The regular expressions that power the parsing engine
	parse: [
		// Match: [@value='test'], [@foo]
		/^(\[) *@?([\w-]+) *([!*$^~=]*) *('?"?)(.*?)\4 *\]/,

		// Match: :contains('foo')
		/^(:)([\w-]+)\("?'?(.*?(\(.*?\))?[^(]*?)"?'?\)/,

		// Match: :even, :last-child, #id, .class
		new RegExp("^([:.#]*)(" + chars + "+)")
	],

	multiFilter: function( expr, elems, not ) {
		var old, cur = [];

		while ( expr && expr != old ) {
			old = expr;
			var f = jQuery.filter( expr, elems, not );
			expr = f.t.replace(/^\s*,\s*/, "" );
			cur = not ? elems = f.r : jQuery.merge( cur, f.r );
		}

		return cur;
	},

	find: function( t, context ) {
		// Quickly handle non-string expressions
		if ( typeof t != "string" )
			return [ t ];

		// check to make sure context is a DOM element or a document
		if ( context && context.nodeType != 1 && context.nodeType != 9)
			return [ ];

		// Set the correct context (if none is provided)
		context = context || document;

		// Initialize the search
		var ret = [context], done = [], last, nodeName;

		// Continue while a selector expression exists, and while
		// we're no longer looping upon ourselves
		while ( t && last != t ) {
			var r = [];
			last = t;

			t = jQuery.trim(t);

			var foundToken = false,

			// An attempt at speeding up child selectors that
			// point to a specific element tag
				re = quickChild,

				m = re.exec(t);

			if ( m ) {
				nodeName = m[1].toUpperCase();

				// Perform our own iteration and filter
				for ( var i = 0; ret[i]; i++ )
					for ( var c = ret[i].firstChild; c; c = c.nextSibling )
						if ( c.nodeType == 1 && (nodeName == "*" || c.nodeName.toUpperCase() == nodeName) )
							r.push( c );

				ret = r;
				t = t.replace( re, "" );
				if ( t.indexOf(" ") == 0 ) continue;
				foundToken = true;
			} else {
				re = /^([>+~])\s*(\w*)/i;

				if ( (m = re.exec(t)) != null ) {
					r = [];

					var merge = {};
					nodeName = m[2].toUpperCase();
					m = m[1];

					for ( var j = 0, rl = ret.length; j < rl; j++ ) {
						var n = m == "~" || m == "+" ? ret[j].nextSibling : ret[j].firstChild;
						for ( ; n; n = n.nextSibling )
							if ( n.nodeType == 1 ) {
								var id = jQuery.data(n);

								if ( m == "~" && merge[id] ) break;

								if (!nodeName || n.nodeName.toUpperCase() == nodeName ) {
									if ( m == "~" ) merge[id] = true;
									r.push( n );
								}

								if ( m == "+" ) break;
							}
					}

					ret = r;

					// And remove the token
					t = jQuery.trim( t.replace( re, "" ) );
					foundToken = true;
				}
			}

			// See if there's still an expression, and that we haven't already
			// matched a token
			if ( t && !foundToken ) {
				// Handle multiple expressions
				if ( !t.indexOf(",") ) {
					// Clean the result set
					if ( context == ret[0] ) ret.shift();

					// Merge the result sets
					done = jQuery.merge( done, ret );

					// Reset the context
					r = ret = [context];

					// Touch up the selector string
					t = " " + t.substr(1,t.length);

				} else {
					// Optimize for the case nodeName#idName
					var re2 = quickID;
					var m = re2.exec(t);

					// Re-organize the results, so that they're consistent
					if ( m ) {
						m = [ 0, m[2], m[3], m[1] ];

					} else {
						// Otherwise, do a traditional filter check for
						// ID, class, and element selectors
						re2 = quickClass;
						m = re2.exec(t);
					}

					m[2] = m[2].replace(/\\/g, "");

					var elem = ret[ret.length-1];

					// Try to do a global search by ID, where we can
					if ( m[1] == "#" && elem && elem.getElementById && !jQuery.isXMLDoc(elem) ) {
						// Optimization for HTML document case
						var oid = elem.getElementById(m[2]);

						// Do a quick check for the existence of the actual ID attribute
						// to avoid selecting by the name attribute in IE
						// also check to insure id is a string to avoid selecting an element with the name of 'id' inside a form
						if ( (jQuery.browser.msie||jQuery.browser.opera) && oid && typeof oid.id == "string" && oid.id != m[2] )
							oid = jQuery('[@id="'+m[2]+'"]', elem)[0];

						// Do a quick check for node name (where applicable) so
						// that div#foo searches will be really fast
						ret = r = oid && (!m[3] || jQuery.nodeName(oid, m[3])) ? [oid] : [];
					} else {
						// We need to find all descendant elements
						for ( var i = 0; ret[i]; i++ ) {
							// Grab the tag name being searched for
							var tag = m[1] == "#" && m[3] ? m[3] : m[1] != "" || m[0] == "" ? "*" : m[2];

							// Handle IE7 being really dumb about <object>s
							if ( tag == "*" && ret[i].nodeName.toLowerCase() == "object" )
								tag = "param";

							r = jQuery.merge( r, ret[i].getElementsByTagName( tag ));
						}

						// It's faster to filter by class and be done with it
						if ( m[1] == "." )
							r = jQuery.classFilter( r, m[2] );

						// Same with ID filtering
						if ( m[1] == "#" ) {
							var tmp = [];

							// Try to find the element with the ID
							for ( var i = 0; r[i]; i++ )
								if ( r[i].getAttribute("id") == m[2] ) {
									tmp = [ r[i] ];
									break;
								}

							r = tmp;
						}

						ret = r;
					}

					t = t.replace( re2, "" );
				}

			}

			// If a selector string still exists
			if ( t ) {
				// Attempt to filter it
				var val = jQuery.filter(t,r);
				ret = r = val.r;
				t = jQuery.trim(val.t);
			}
		}

		// An error occurred with the selector;
		// just return an empty set instead
		if ( t )
			ret = [];

		// Remove the root context
		if ( ret && context == ret[0] )
			ret.shift();

		// And combine the results
		done = jQuery.merge( done, ret );

		return done;
	},

	classFilter: function(r,m,not){
		m = " " + m + " ";
		var tmp = [];
		for ( var i = 0; r[i]; i++ ) {
			var pass = (" " + r[i].className + " ").indexOf( m ) >= 0;
			if ( !not && pass || not && !pass )
				tmp.push( r[i] );
		}
		return tmp;
	},

	filter: function(t,r,not) {
		var last;

		// Look for common filter expressions
		while ( t && t != last ) {
			last = t;

			var p = jQuery.parse, m;

			for ( var i = 0; p[i]; i++ ) {
				m = p[i].exec( t );

				if ( m ) {
					// Remove what we just matched
					t = t.substring( m[0].length );

					m[2] = m[2].replace(/\\/g, "");
					break;
				}
			}

			if ( !m )
				break;

			// :not() is a special case that can be optimized by
			// keeping it out of the expression list
			if ( m[1] == ":" && m[2] == "not" )
				// optimize if only one selector found (most common case)
				r = isSimple.test( m[3] ) ?
					jQuery.filter(m[3], r, true).r :
					jQuery( r ).not( m[3] );

			// We can get a big speed boost by filtering by class here
			else if ( m[1] == "." )
				r = jQuery.classFilter(r, m[2], not);

			else if ( m[1] == "[" ) {
				var tmp = [], type = m[3];

				for ( var i = 0, rl = r.length; i < rl; i++ ) {
					var a = r[i], z = a[ jQuery.props[m[2]] || m[2] ];

					if ( z == null || /href|src|selected/.test(m[2]) )
						z = jQuery.attr(a,m[2]) || '';

					if ( (type == "" && !!z ||
						 type == "=" && z == m[5] ||
						 type == "!=" && z != m[5] ||
						 type == "^=" && z && !z.indexOf(m[5]) ||
						 type == "$=" && z.substr(z.length - m[5].length) == m[5] ||
						 (type == "*=" || type == "~=") && z.indexOf(m[5]) >= 0) ^ not )
							tmp.push( a );
				}

				r = tmp;

			// We can get a speed boost by handling nth-child here
			} else if ( m[1] == ":" && m[2] == "nth-child" ) {
				var merge = {}, tmp = [],
					// parse equations like 'even', 'odd', '5', '2n', '3n+2', '4n-1', '-n+6'
					test = /(-?)(\d*)n((?:\+|-)?\d*)/.exec(
						m[3] == "even" && "2n" || m[3] == "odd" && "2n+1" ||
						!/\D/.test(m[3]) && "0n+" + m[3] || m[3]),
					// calculate the numbers (first)n+(last) including if they are negative
					first = (test[1] + (test[2] || 1)) - 0, last = test[3] - 0;

				// loop through all the elements left in the jQuery object
				for ( var i = 0, rl = r.length; i < rl; i++ ) {
					var node = r[i], parentNode = node.parentNode, id = jQuery.data(parentNode);

					if ( !merge[id] ) {
						var c = 1;

						for ( var n = parentNode.firstChild; n; n = n.nextSibling )
							if ( n.nodeType == 1 )
								n.nodeIndex = c++;

						merge[id] = true;
					}

					var add = false;

					if ( first == 0 ) {
						if ( node.nodeIndex == last )
							add = true;
					} else if ( (node.nodeIndex - last) % first == 0 && (node.nodeIndex - last) / first >= 0 )
						add = true;

					if ( add ^ not )
						tmp.push( node );
				}

				r = tmp;

			// Otherwise, find the expression to execute
			} else {
				var fn = jQuery.expr[ m[1] ];
				if ( typeof fn == "object" )
					fn = fn[ m[2] ];

				if ( typeof fn == "string" )
					fn = eval("false||function(a,i){return " + fn + ";}");

				// Execute it against the current filter
				r = jQuery.grep( r, function(elem, i){
					return fn(elem, i, m, r);
				}, not );
			}
		}

		// Return an array of filtered elements (r)
		// and the modified expression string (t)
		return { r: r, t: t };
	},

	dir: function( elem, dir ){
		var matched = [],
			cur = elem[dir];
		while ( cur && cur != document ) {
			if ( cur.nodeType == 1 )
				matched.push( cur );
			cur = cur[dir];
		}
		return matched;
	},

	nth: function(cur,result,dir,elem){
		result = result || 1;
		var num = 0;

		for ( ; cur; cur = cur[dir] )
			if ( cur.nodeType == 1 && ++num == result )
				break;

		return cur;
	},

	sibling: function( n, elem ) {
		var r = [];

		for ( ; n; n = n.nextSibling ) {
			if ( n.nodeType == 1 && n != elem )
				r.push( n );
		}

		return r;
	}
});
/*
 * A number of helper functions used for managing events.
 * Many of the ideas behind this code orignated from
 * Dean Edwards' addEvent library.
 */
jQuery.event = {

	// Bind an event to an element
	// Original by Dean Edwards
	add: function(elem, types, handler, data) {
		if ( elem.nodeType == 3 || elem.nodeType == 8 )
			return;

		// For whatever reason, IE has trouble passing the window object
		// around, causing it to be cloned in the process
		if ( jQuery.browser.msie && elem.setInterval )
			elem = window;

		// Make sure that the function being executed has a unique ID
		if ( !handler.guid )
			handler.guid = this.guid++;

		// if data is passed, bind to handler
		if( data != undefined ) {
			// Create temporary function pointer to original handler
			var fn = handler;

			// Create unique handler function, wrapped around original handler
			handler = this.proxy( fn, function() {
				// Pass arguments and context to original handler
				return fn.apply(this, arguments);
			});

			// Store data in unique handler
			handler.data = data;
		}

		// Init the element's event structure
		var events = jQuery.data(elem, "events") || jQuery.data(elem, "events", {}),
			handle = jQuery.data(elem, "handle") || jQuery.data(elem, "handle", function(){
				// Handle the second event of a trigger and when
				// an event is called after a page has unloaded
				if ( typeof jQuery != "undefined" && !jQuery.event.triggered )
					return jQuery.event.handle.apply(arguments.callee.elem, arguments);
			});
		// Add elem as a property of the handle function
		// This is to prevent a memory leak with non-native
		// event in IE.
		handle.elem = elem;

		// Handle multiple events separated by a space
		// jQuery(...).bind("mouseover mouseout", fn);
		jQuery.each(types.split(/\s+/), function(index, type) {
			// Namespaced event handlers
			var parts = type.split(".");
			type = parts[0];
			handler.type = parts[1];

			// Get the current list of functions bound to this event
			var handlers = events[type];

			// Init the event handler queue
			if (!handlers) {
				handlers = events[type] = {};

				// Check for a special event handler
				// Only use addEventListener/attachEvent if the special
				// events handler returns false
				if ( !jQuery.event.special[type] || jQuery.event.special[type].setup.call(elem) === false ) {
					// Bind the global event handler to the element
					if (elem.addEventListener)
						elem.addEventListener(type, handle, false);
					else if (elem.attachEvent)
						elem.attachEvent("on" + type, handle);
				}
			}

			// Add the function to the element's handler list
			handlers[handler.guid] = handler;

			// Keep track of which events have been used, for global triggering
			jQuery.event.global[type] = true;
		});

		// Nullify elem to prevent memory leaks in IE
		elem = null;
	},

	guid: 1,
	global: {},

	// Detach an event or set of events from an element
	remove: function(elem, types, handler) {
		// don't do events on text and comment nodes
		if ( elem.nodeType == 3 || elem.nodeType == 8 )
			return;

		var events = jQuery.data(elem, "events"), ret, index;

		if ( events ) {
			// Unbind all events for the element
			if ( types == undefined || (typeof types == "string" && types.charAt(0) == ".") )
				for ( var type in events )
					this.remove( elem, type + (types || "") );
			else {
				// types is actually an event object here
				if ( types.type ) {
					handler = types.handler;
					types = types.type;
				}

				// Handle multiple events seperated by a space
				// jQuery(...).unbind("mouseover mouseout", fn);
				jQuery.each(types.split(/\s+/), function(index, type){
					// Namespaced event handlers
					var parts = type.split(".");
					type = parts[0];

					if ( events[type] ) {
						// remove the given handler for the given type
						if ( handler )
							delete events[type][handler.guid];

						// remove all handlers for the given type
						else
							for ( handler in events[type] )
								// Handle the removal of namespaced events
								if ( !parts[1] || events[type][handler].type == parts[1] )
									delete events[type][handler];

						// remove generic event handler if no more handlers exist
						for ( ret in events[type] ) break;
						if ( !ret ) {
							if ( !jQuery.event.special[type] || jQuery.event.special[type].teardown.call(elem) === false ) {
								if (elem.removeEventListener)
									elem.removeEventListener(type, jQuery.data(elem, "handle"), false);
								else if (elem.detachEvent)
									elem.detachEvent("on" + type, jQuery.data(elem, "handle"));
							}
							ret = null;
							delete events[type];
						}
					}
				});
			}

			// Remove the expando if it's no longer used
			for ( ret in events ) break;
			if ( !ret ) {
				var handle = jQuery.data( elem, "handle" );
				if ( handle ) handle.elem = null;
				jQuery.removeData( elem, "events" );
				jQuery.removeData( elem, "handle" );
			}
		}
	},

	trigger: function(type, data, elem, donative, extra) {
		// Clone the incoming data, if any
		data = jQuery.makeArray(data);

		if ( type.indexOf("!") >= 0 ) {
			type = type.slice(0, -1);
			var exclusive = true;
		}

		// Handle a global trigger
		if ( !elem ) {
			// Only trigger if we've ever bound an event for it
			if ( this.global[type] )
				jQuery("*").add([window, document]).trigger(type, data);

		// Handle triggering a single element
		} else {
			// don't do events on text and comment nodes
			if ( elem.nodeType == 3 || elem.nodeType == 8 )
				return undefined;

			var val, ret, fn = jQuery.isFunction( elem[ type ] || null ),
				// Check to see if we need to provide a fake event, or not
				event = !data[0] || !data[0].preventDefault;

			// Pass along a fake event
			if ( event ) {
				data.unshift({
					type: type,
					target: elem,
					preventDefault: function(){},
					stopPropagation: function(){},
					timeStamp: now()
				});
				data[0][expando] = true; // no need to fix fake event
			}

			// Enforce the right trigger type
			data[0].type = type;
			if ( exclusive )
				data[0].exclusive = true;

			// Trigger the event, it is assumed that "handle" is a function
			var handle = jQuery.data(elem, "handle");
			if ( handle )
				val = handle.apply( elem, data );

			// Handle triggering native .onfoo handlers (and on links since we don't call .click() for links)
			if ( (!fn || (jQuery.nodeName(elem, 'a') && type == "click")) && elem["on"+type] && elem["on"+type].apply( elem, data ) === false )
				val = false;

			// Extra functions don't get the custom event object
			if ( event )
				data.shift();

			// Handle triggering of extra function
			if ( extra && jQuery.isFunction( extra ) ) {
				// call the extra function and tack the current return value on the end for possible inspection
				ret = extra.apply( elem, val == null ? data : data.concat( val ) );
				// if anything is returned, give it precedence and have it overwrite the previous value
				if (ret !== undefined)
					val = ret;
			}

			// Trigger the native events (except for clicks on links)
			if ( fn && donative !== false && val !== false && !(jQuery.nodeName(elem, 'a') && type == "click") ) {
				this.triggered = true;
				try {
					elem[ type ]();
				// prevent IE from throwing an error for some hidden elements
				} catch (e) {}
			}

			this.triggered = false;
		}

		return val;
	},

	handle: function(event) {
		// returned undefined or false
		var val, ret, namespace, all, handlers;

		event = arguments[0] = jQuery.event.fix( event || window.event );

		// Namespaced event handlers
		namespace = event.type.split(".");
		event.type = namespace[0];
		namespace = namespace[1];
		// Cache this now, all = true means, any handler
		all = !namespace && !event.exclusive;

		handlers = ( jQuery.data(this, "events") || {} )[event.type];

		for ( var j in handlers ) {
			var handler = handlers[j];

			// Filter the functions by class
			if ( all || handler.type == namespace ) {
				// Pass in a reference to the handler function itself
				// So that we can later remove it
				event.handler = handler;
				event.data = handler.data;

				ret = handler.apply( this, arguments );

				if ( val !== false )
					val = ret;

				if ( ret === false ) {
					event.preventDefault();
					event.stopPropagation();
				}
			}
		}

		return val;
	},

	fix: function(event) {
		if ( event[expando] == true )
			return event;

		// store a copy of the original event object
		// and "clone" to set read-only properties
		var originalEvent = event;
		event = { originalEvent: originalEvent };
		var props = "altKey attrChange attrName bubbles button cancelable charCode clientX clientY ctrlKey currentTarget data detail eventPhase fromElement handler keyCode metaKey newValue originalTarget pageX pageY prevValue relatedNode relatedTarget screenX screenY shiftKey srcElement target timeStamp toElement type view wheelDelta which".split(" ");
		for ( var i=props.length; i; i-- )
			event[ props[i] ] = originalEvent[ props[i] ];

		// Mark it as fixed
		event[expando] = true;

		// add preventDefault and stopPropagation since
		// they will not work on the clone
		event.preventDefault = function() {
			// if preventDefault exists run it on the original event
			if (originalEvent.preventDefault)
				originalEvent.preventDefault();
			// otherwise set the returnValue property of the original event to false (IE)
			originalEvent.returnValue = false;
		};
		event.stopPropagation = function() {
			// if stopPropagation exists run it on the original event
			if (originalEvent.stopPropagation)
				originalEvent.stopPropagation();
			// otherwise set the cancelBubble property of the original event to true (IE)
			originalEvent.cancelBubble = true;
		};

		// Fix timeStamp
		event.timeStamp = event.timeStamp || now();

		// Fix target property, if necessary
		if ( !event.target )
			event.target = event.srcElement || document; // Fixes #1925 where srcElement might not be defined either

		// check if target is a textnode (safari)
		if ( event.target.nodeType == 3 )
			event.target = event.target.parentNode;

		// Add relatedTarget, if necessary
		if ( !event.relatedTarget && event.fromElement )
			event.relatedTarget = event.fromElement == event.target ? event.toElement : event.fromElement;

		// Calculate pageX/Y if missing and clientX/Y available
		if ( event.pageX == null && event.clientX != null ) {
			var doc = document.documentElement, body = document.body;
			event.pageX = event.clientX + (doc && doc.scrollLeft || body && body.scrollLeft || 0) - (doc.clientLeft || 0);
			event.pageY = event.clientY + (doc && doc.scrollTop || body && body.scrollTop || 0) - (doc.clientTop || 0);
		}

		// Add which for key events
		if ( !event.which && ((event.charCode || event.charCode === 0) ? event.charCode : event.keyCode) )
			event.which = event.charCode || event.keyCode;

		// Add metaKey to non-Mac browsers (use ctrl for PC's and Meta for Macs)
		if ( !event.metaKey && event.ctrlKey )
			event.metaKey = event.ctrlKey;

		// Add which for click: 1 == left; 2 == middle; 3 == right
		// Note: button is not normalized, so don't use it
		if ( !event.which && event.button )
			event.which = (event.button & 1 ? 1 : ( event.button & 2 ? 3 : ( event.button & 4 ? 2 : 0 ) ));

		return event;
	},

	proxy: function( fn, proxy ){
		// Set the guid of unique handler to the same of original handler, so it can be removed
		proxy.guid = fn.guid = fn.guid || proxy.guid || this.guid++;
		// So proxy can be declared as an argument
		return proxy;
	},

	special: {
		ready: {
			setup: function() {
				// Make sure the ready event is setup
				bindReady();
				return;
			},

			teardown: function() { return; }
		},

		mouseenter: {
			setup: function() {
				if ( jQuery.browser.msie ) return false;
				jQuery(this).bind("mouseover", jQuery.event.special.mouseenter.handler);
				return true;
			},

			teardown: function() {
				if ( jQuery.browser.msie ) return false;
				jQuery(this).unbind("mouseover", jQuery.event.special.mouseenter.handler);
				return true;
			},

			handler: function(event) {
				// If we actually just moused on to a sub-element, ignore it
				if ( withinElement(event, this) ) return true;
				// Execute the right handlers by setting the event type to mouseenter
				event.type = "mouseenter";
				return jQuery.event.handle.apply(this, arguments);
			}
		},

		mouseleave: {
			setup: function() {
				if ( jQuery.browser.msie ) return false;
				jQuery(this).bind("mouseout", jQuery.event.special.mouseleave.handler);
				return true;
			},

			teardown: function() {
				if ( jQuery.browser.msie ) return false;
				jQuery(this).unbind("mouseout", jQuery.event.special.mouseleave.handler);
				return true;
			},

			handler: function(event) {
				// If we actually just moused on to a sub-element, ignore it
				if ( withinElement(event, this) ) return true;
				// Execute the right handlers by setting the event type to mouseleave
				event.type = "mouseleave";
				return jQuery.event.handle.apply(this, arguments);
			}
		}
	}
};

jQuery.fn.extend({
	bind: function( type, data, fn ) {
		return type == "unload" ? this.one(type, data, fn) : this.each(function(){
			jQuery.event.add( this, type, fn || data, fn && data );
		});
	},

	one: function( type, data, fn ) {
		var one = jQuery.event.proxy( fn || data, function(event) {
			jQuery(this).unbind(event, one);
			return (fn || data).apply( this, arguments );
		});
		return this.each(function(){
			jQuery.event.add( this, type, one, fn && data);
		});
	},

	unbind: function( type, fn ) {
		return this.each(function(){
			jQuery.event.remove( this, type, fn );
		});
	},

	trigger: function( type, data, fn ) {
		return this.each(function(){
			jQuery.event.trigger( type, data, this, true, fn );
		});
	},

	triggerHandler: function( type, data, fn ) {
		return this[0] && jQuery.event.trigger( type, data, this[0], false, fn );
	},

	toggle: function( fn ) {
		// Save reference to arguments for access in closure
		var args = arguments, i = 1;

		// link all the functions, so any of them can unbind this click handler
		while( i < args.length )
			jQuery.event.proxy( fn, args[i++] );

		return this.click( jQuery.event.proxy( fn, function(event) {
			// Figure out which function to execute
			this.lastToggle = ( this.lastToggle || 0 ) % i;

			// Make sure that clicks stop
			event.preventDefault();

			// and execute the function
			return args[ this.lastToggle++ ].apply( this, arguments ) || false;
		}));
	},

	hover: function(fnOver, fnOut) {
		return this.bind('mouseenter', fnOver).bind('mouseleave', fnOut);
	},

	ready: function(fn) {
		// Attach the listeners
		bindReady();

		// If the DOM is already ready
		if ( jQuery.isReady )
			// Execute the function immediately
			fn.call( document, jQuery );

		// Otherwise, remember the function for later
		else
			// Add the function to the wait list
			jQuery.readyList.push( function() { return fn.call(this, jQuery); } );

		return this;
	}
});

jQuery.extend({
	isReady: false,
	readyList: [],
	// Handle when the DOM is ready
	ready: function() {
		// Make sure that the DOM is not already loaded
		if ( !jQuery.isReady ) {
			// Remember that the DOM is ready
			jQuery.isReady = true;

			// If there are functions bound, to execute
			if ( jQuery.readyList ) {
				// Execute all of them
				jQuery.each( jQuery.readyList, function(){
					this.call( document );
				});

				// Reset the list of functions
				jQuery.readyList = null;
			}

			// Trigger any bound ready events
			jQuery(document).triggerHandler("ready");
		}
	}
});

var readyBound = false;

function bindReady(){
	if ( readyBound ) return;
	readyBound = true;

	// Mozilla, Opera (see further below for it) and webkit nightlies currently support this event
	if ( document.addEventListener && !jQuery.browser.opera)
		// Use the handy event callback
		document.addEventListener( "DOMContentLoaded", jQuery.ready, false );

	// If IE is used and is not in a frame
	// Continually check to see if the document is ready
	if ( jQuery.browser.msie && window == top ) (function(){
		if (jQuery.isReady) return;
		try {
			// If IE is used, use the trick by Diego Perini
			// http://javascript.nwbox.com/IEContentLoaded/
			document.documentElement.doScroll("left");
		} catch( error ) {
			setTimeout( arguments.callee, 0 );
			return;
		}
		// and execute any waiting functions
		jQuery.ready();
	})();

	if ( jQuery.browser.opera )
		document.addEventListener( "DOMContentLoaded", function () {
			if (jQuery.isReady) return;
			for (var i = 0; i < document.styleSheets.length; i++)
				if (document.styleSheets[i].disabled) {
					setTimeout( arguments.callee, 0 );
					return;
				}
			// and execute any waiting functions
			jQuery.ready();
		}, false);

	if ( jQuery.browser.safari ) {
		var numStyles;
		(function(){
			if (jQuery.isReady) return;
			if ( document.readyState != "loaded" && document.readyState != "complete" ) {
				setTimeout( arguments.callee, 0 );
				return;
			}
			if ( numStyles === undefined )
				numStyles = jQuery("style, link[rel=stylesheet]").length;
			if ( document.styleSheets.length != numStyles ) {
				setTimeout( arguments.callee, 0 );
				return;
			}
			// and execute any waiting functions
			jQuery.ready();
		})();
	}

	// A fallback to window.onload, that will always work
	jQuery.event.add( window, "load", jQuery.ready );
}

jQuery.each( ("blur,focus,load,resize,scroll,unload,click,dblclick," +
	"mousedown,mouseup,mousemove,mouseover,mouseout,change,select," +
	"submit,keydown,keypress,keyup,error").split(","), function(i, name){

	// Handle event binding
	jQuery.fn[name] = function(fn){
		return fn ? this.bind(name, fn) : this.trigger(name);
	};
});

// Checks if an event happened on an element within another element
// Used in jQuery.event.special.mouseenter and mouseleave handlers
var withinElement = function(event, elem) {
	// Check if mouse(over|out) are still within the same parent element
	var parent = event.relatedTarget;
	// Traverse up the tree
	while ( parent && parent != elem ) try { parent = parent.parentNode; } catch(error) { parent = elem; }
	// Return true if we actually just moused on to a sub-element
	return parent == elem;
};

// Prevent memory leaks in IE
// And prevent errors on refresh with events like mouseover in other browsers
// Window isn't included so as not to unbind existing unload events
jQuery(window).bind("unload", function() {
	jQuery("*").add(document).unbind();
});
jQuery.fn.extend({
	// Keep a copy of the old load
	_load: jQuery.fn.load,

	load: function( url, params, callback ) {
		if ( typeof url != 'string' )
			return this._load( url );

		var off = url.indexOf(" ");
		if ( off >= 0 ) {
			var selector = url.slice(off, url.length);
			url = url.slice(0, off);
		}

		callback = callback || function(){};

		// Default to a GET request
		var type = "GET";

		// If the second parameter was provided
		if ( params )
			// If it's a function
			if ( jQuery.isFunction( params ) ) {
				// We assume that it's the callback
				callback = params;
				params = null;

			// Otherwise, build a param string
			} else {
				params = jQuery.param( params );
				type = "POST";
			}

		var self = this;

		// Request the remote document
		jQuery.ajax({
			url: url,
			type: type,
			dataType: "html",
			data: params,
			complete: function(res, status){
				// If successful, inject the HTML into all the matched elements
				if ( status == "success" || status == "notmodified" )
					// See if a selector was specified
					self.html( selector ?
						// Create a dummy div to hold the results
						jQuery("<div/>")
							// inject the contents of the document in, removing the scripts
							// to avoid any 'Permission Denied' errors in IE
							.append(res.responseText.replace(/<script(.|\s)*?\/script>/g, ""))

							// Locate the specified elements
							.find(selector) :

						// If not, just inject the full result
						res.responseText );

				self.each( callback, [res.responseText, status, res] );
			}
		});
		return this;
	},

	serialize: function() {
		return jQuery.param(this.serializeArray());
	},
	serializeArray: function() {
		return this.map(function(){
			return jQuery.nodeName(this, "form") ?
				jQuery.makeArray(this.elements) : this;
		})
		.filter(function(){
			return this.name && !this.disabled &&
				(this.checked || /select|textarea/i.test(this.nodeName) ||
					/text|hidden|password/i.test(this.type));
		})
		.map(function(i, elem){
			var val = jQuery(this).val();
			return val == null ? null :
				val.constructor == Array ?
					jQuery.map( val, function(val, i){
						return {name: elem.name, value: val};
					}) :
					{name: elem.name, value: val};
		}).get();
	}
});

// Attach a bunch of functions for handling common AJAX events
jQuery.each( "ajaxStart,ajaxStop,ajaxComplete,ajaxError,ajaxSuccess,ajaxSend".split(","), function(i,o){
	jQuery.fn[o] = function(f){
		return this.bind(o, f);
	};
});

var jsc = now();

jQuery.extend({
	get: function( url, data, callback, type ) {
		// shift arguments if data argument was ommited
		if ( jQuery.isFunction( data ) ) {
			callback = data;
			data = null;
		}

		return jQuery.ajax({
			type: "GET",
			url: url,
			data: data,
			success: callback,
			dataType: type
		});
	},

	getScript: function( url, callback ) {
		return jQuery.get(url, null, callback, "script");
	},

	getJSON: function( url, data, callback ) {
		return jQuery.get(url, data, callback, "json");
	},

	post: function( url, data, callback, type ) {
		if ( jQuery.isFunction( data ) ) {
			callback = data;
			data = {};
		}

		return jQuery.ajax({
			type: "POST",
			url: url,
			data: data,
			success: callback,
			dataType: type
		});
	},

	ajaxSetup: function( settings ) {
		jQuery.extend( jQuery.ajaxSettings, settings );
	},

	ajaxSettings: {
		url: location.href,
		global: true,
		type: "GET",
		timeout: 0,
		contentType: "application/x-www-form-urlencoded",
		processData: true,
		async: true,
		data: null,
		username: null,
		password: null,
		accepts: {
			xml: "application/xml, text/xml",
			html: "text/html",
			script: "text/javascript, application/javascript",
			json: "application/json, text/javascript",
			text: "text/plain",
			_default: "*/*"
		}
	},

	// Last-Modified header cache for next request
	lastModified: {},

	ajax: function( s ) {
		// Extend the settings, but re-extend 's' so that it can be
		// checked again later (in the test suite, specifically)
		s = jQuery.extend(true, s, jQuery.extend(true, {}, jQuery.ajaxSettings, s));

		var jsonp, jsre = /=\?(&|$)/g, status, data,
			type = s.type.toUpperCase();

		// convert data if not already a string
		if ( s.data && s.processData && typeof s.data != "string" )
			s.data = jQuery.param(s.data);

		// Handle JSONP Parameter Callbacks
		if ( s.dataType == "jsonp" ) {
			if ( type == "GET" ) {
				if ( !s.url.match(jsre) )
					s.url += (s.url.match(/\?/) ? "&" : "?") + (s.jsonp || "callback") + "=?";
			} else if ( !s.data || !s.data.match(jsre) )
				s.data = (s.data ? s.data + "&" : "") + (s.jsonp || "callback") + "=?";
			s.dataType = "json";
		}

		// Build temporary JSONP function
		if ( s.dataType == "json" && (s.data && s.data.match(jsre) || s.url.match(jsre)) ) {
			jsonp = "jsonp" + jsc++;

			// Replace the =? sequence both in the query string and the data
			if ( s.data )
				s.data = (s.data + "").replace(jsre, "=" + jsonp + "$1");
			s.url = s.url.replace(jsre, "=" + jsonp + "$1");

			// We need to make sure
			// that a JSONP style response is executed properly
			s.dataType = "script";

			// Handle JSONP-style loading
			window[ jsonp ] = function(tmp){
				data = tmp;
				success();
				complete();
				// Garbage collect
				window[ jsonp ] = undefined;
				try{ delete window[ jsonp ]; } catch(e){}
				if ( head )
					head.removeChild( script );
			};
		}

		if ( s.dataType == "script" && s.cache == null )
			s.cache = false;

		if ( s.cache === false && type == "GET" ) {
			var ts = now();
			// try replacing _= if it is there
			var ret = s.url.replace(/(\?|&)_=.*?(&|$)/, "$1_=" + ts + "$2");
			// if nothing was replaced, add timestamp to the end
			s.url = ret + ((ret == s.url) ? (s.url.match(/\?/) ? "&" : "?") + "_=" + ts : "");
		}

		// If data is available, append data to url for get requests
		if ( s.data && type == "GET" ) {
			s.url += (s.url.match(/\?/) ? "&" : "?") + s.data;

			// IE likes to send both get and post data, prevent this
			s.data = null;
		}

		// Watch for a new set of requests
		if ( s.global && ! jQuery.active++ )
			jQuery.event.trigger( "ajaxStart" );

		// Matches an absolute URL, and saves the domain
		var remote = /^(?:\w+:)?\/\/([^\/?#]+)/;

		// If we're requesting a remote document
		// and trying to load JSON or Script with a GET
		if ( s.dataType == "script" && type == "GET"
				&& remote.test(s.url) && remote.exec(s.url)[1] != location.host ){
			var head = document.getElementsByTagName("head")[0];
			var script = document.createElement("script");
			script.src = s.url;
			if (s.scriptCharset)
				script.charset = s.scriptCharset;

			// Handle Script loading
			if ( !jsonp ) {
				var done = false;

				// Attach handlers for all browsers
				script.onload = script.onreadystatechange = function(){
					if ( !done && (!this.readyState ||
							this.readyState == "loaded" || this.readyState == "complete") ) {
						done = true;
						success();
						complete();
						head.removeChild( script );
					}
				};
			}

			head.appendChild(script);

			// We handle everything using the script element injection
			return undefined;
		}

		var requestDone = false;

		// Create the request object; Microsoft failed to properly
		// implement the XMLHttpRequest in IE7, so we use the ActiveXObject when it is available
		var xhr = window.ActiveXObject ? new ActiveXObject("Microsoft.XMLHTTP") : new XMLHttpRequest();

		// Open the socket
		// Passing null username, generates a login popup on Opera (#2865)
		if( s.username )
			xhr.open(type, s.url, s.async, s.username, s.password);
		else
			xhr.open(type, s.url, s.async);

		// Need an extra try/catch for cross domain requests in Firefox 3
		try {
			// Set the correct header, if data is being sent
			if ( s.data )
				xhr.setRequestHeader("Content-Type", s.contentType);

			// Set the If-Modified-Since header, if ifModified mode.
			if ( s.ifModified )
				xhr.setRequestHeader("If-Modified-Since",
					jQuery.lastModified[s.url] || "Thu, 01 Jan 1970 00:00:00 GMT" );

			// Set header so the called script knows that it's an XMLHttpRequest
			xhr.setRequestHeader("X-Requested-With", "XMLHttpRequest");

			// Set the Accepts header for the server, depending on the dataType
			xhr.setRequestHeader("Accept", s.dataType && s.accepts[ s.dataType ] ?
				s.accepts[ s.dataType ] + ", */*" :
				s.accepts._default );
		} catch(e){}

		// Allow custom headers/mimetypes
		if ( s.beforeSend && s.beforeSend(xhr, s) === false ) {
			// cleanup active request counter
			s.global && jQuery.active--;
			// close opended socket
			xhr.abort();
			return false;
		}

		if ( s.global )
			jQuery.event.trigger("ajaxSend", [xhr, s]);

		// Wait for a response to come back
		var onreadystatechange = function(isTimeout){
			// The transfer is complete and the data is available, or the request timed out
			if ( !requestDone && xhr && (xhr.readyState == 4 || isTimeout == "timeout") ) {
				requestDone = true;

				// clear poll interval
				if (ival) {
					clearInterval(ival);
					ival = null;
				}

				status = isTimeout == "timeout" && "timeout" ||
					!jQuery.httpSuccess( xhr ) && "error" ||
					s.ifModified && jQuery.httpNotModified( xhr, s.url ) && "notmodified" ||
					"success";

				if ( status == "success" ) {
					// Watch for, and catch, XML document parse errors
					try {
						// process the data (runs the xml through httpData regardless of callback)
						data = jQuery.httpData( xhr, s.dataType, s.dataFilter );
					} catch(e) {
						status = "parsererror";
					}
				}

				// Make sure that the request was successful or notmodified
				if ( status == "success" ) {
					// Cache Last-Modified header, if ifModified mode.
					var modRes;
					try {
						modRes = xhr.getResponseHeader("Last-Modified");
					} catch(e) {} // swallow exception thrown by FF if header is not available

					if ( s.ifModified && modRes )
						jQuery.lastModified[s.url] = modRes;

					// JSONP handles its own success callback
					if ( !jsonp )
						success();
				} else
					jQuery.handleError(s, xhr, status);

				// Fire the complete handlers
				complete();

				// Stop memory leaks
				if ( s.async )
					xhr = null;
			}
		};

		if ( s.async ) {
			// don't attach the handler to the request, just poll it instead
			var ival = setInterval(onreadystatechange, 13);

			// Timeout checker
			if ( s.timeout > 0 )
				setTimeout(function(){
					// Check to see if the request is still happening
					if ( xhr ) {
						// Cancel the request
						xhr.abort();

						if( !requestDone )
							onreadystatechange( "timeout" );
					}
				}, s.timeout);
		}

		// Send the data
		try {
			xhr.send(s.data);
		} catch(e) {
			jQuery.handleError(s, xhr, null, e);
		}

		// firefox 1.5 doesn't fire statechange for sync requests
		if ( !s.async )
			onreadystatechange();

		function success(){
			// If a local callback was specified, fire it and pass it the data
			if ( s.success )
				s.success( data, status );

			// Fire the global callback
			if ( s.global )
				jQuery.event.trigger( "ajaxSuccess", [xhr, s] );
		}

		function complete(){
			// Process result
			if ( s.complete )
				s.complete(xhr, status);

			// The request was completed
			if ( s.global )
				jQuery.event.trigger( "ajaxComplete", [xhr, s] );

			// Handle the global AJAX counter
			if ( s.global && ! --jQuery.active )
				jQuery.event.trigger( "ajaxStop" );
		}

		// return XMLHttpRequest to allow aborting the request etc.
		return xhr;
	},

	handleError: function( s, xhr, status, e ) {
		// If a local callback was specified, fire it
		if ( s.error ) s.error( xhr, status, e );

		// Fire the global callback
		if ( s.global )
			jQuery.event.trigger( "ajaxError", [xhr, s, e] );
	},

	// Counter for holding the number of active queries
	active: 0,

	// Determines if an XMLHttpRequest was successful or not
	httpSuccess: function( xhr ) {
		try {
			ti.App.debug("xhr.status="+xhr.status);
			// IE error sometimes returns 1223 when it should be 204 so treat it as success, see #1450
			return !xhr.status && location.protocol == "file:" ||
				( xhr.status >= 200 && xhr.status < 300 ) || xhr.status == 304 || xhr.status == 1223 ||
				jQuery.browser.safari && xhr.status == undefined;
		} catch(e){}
		return false;
	},

	// Determines if an XMLHttpRequest returns NotModified
	httpNotModified: function( xhr, url ) {
		try {
			var xhrRes = xhr.getResponseHeader("Last-Modified");

			// Firefox always returns 200. check Last-Modified date
			return xhr.status == 304 || xhrRes == jQuery.lastModified[url] ||
				jQuery.browser.safari && xhr.status == undefined;
		} catch(e){}
		return false;
	},

	httpData: function( xhr, type, filter ) {
		var ct = xhr.getResponseHeader("content-type"),
			xml = type == "xml" || !type && ct && ct.indexOf("xml") >= 0,
			data = xml ? xhr.responseXML : xhr.responseText;

		if ( xml && data.documentElement.tagName == "parsererror" )
			throw "parsererror";
			
		// Allow a pre-filtering function to sanitize the response
		if( filter )
			data = filter( data, type );

		// If the type is "script", eval it in global context
		if ( type == "script" )
			jQuery.globalEval( data );

		// Get the JavaScript object, if JSON is used.
		if ( type == "json" )
			data = eval("(" + data + ")");

		return data;
	},

	// Serialize an array of form elements or a set of
	// key/values into a query string
	param: function( a ) {
		var s = [];

		// If an array was passed in, assume that it is an array
		// of form elements
		if ( a.constructor == Array || a.jquery )
			// Serialize the form elements
			jQuery.each( a, function(){
				s.push( encodeURIComponent(this.name) + "=" + encodeURIComponent( this.value ) );
			});

		// Otherwise, assume that it's an object of key/value pairs
		else
			// Serialize the key/values
			for ( var j in a )
				// If the value is an array then the key names need to be repeated
				if ( a[j] && a[j].constructor == Array )
					jQuery.each( a[j], function(){
						s.push( encodeURIComponent(j) + "=" + encodeURIComponent( this ) );
					});
				else
					s.push( encodeURIComponent(j) + "=" + encodeURIComponent( jQuery.isFunction(a[j]) ? a[j]() : a[j] ) );

		// Return the resulting serialization
		return s.join("&").replace(/%20/g, "+");
	}

});
jQuery.fn.extend({
	show: function(speed,callback){
		return speed ?
			this.animate({
				height: "show", width: "show", opacity: "show"
			}, speed, callback) :

			this.filter(":hidden").each(function(){
				this.style.display = this.oldblock || "";
				if ( jQuery.css(this,"display") == "none" ) {
					var elem = jQuery("<" + this.tagName + " />").appendTo("body");
					this.style.display = elem.css("display");
					// handle an edge condition where css is - div { display:none; } or similar
					if (this.style.display == "none")
						this.style.display = "block";
					elem.remove();
				}
			}).end();
	},

	hide: function(speed,callback){
		return speed ?
			this.animate({
				height: "hide", width: "hide", opacity: "hide"
			}, speed, callback) :

			this.filter(":visible").each(function(){
				this.oldblock = this.oldblock || jQuery.css(this,"display");
				this.style.display = "none";
			}).end();
	},

	// Save the old toggle function
	_toggle: jQuery.fn.toggle,

	toggle: function( fn, fn2 ){
		return jQuery.isFunction(fn) && jQuery.isFunction(fn2) ?
			this._toggle.apply( this, arguments ) :
			fn ?
				this.animate({
					height: "toggle", width: "toggle", opacity: "toggle"
				}, fn, fn2) :
				this.each(function(){
					jQuery(this)[ jQuery(this).is(":hidden") ? "show" : "hide" ]();
				});
	},

	slideDown: function(speed,callback){
		return this.animate({height: "show"}, speed, callback);
	},

	slideUp: function(speed,callback){
		return this.animate({height: "hide"}, speed, callback);
	},

	slideToggle: function(speed, callback){
		return this.animate({height: "toggle"}, speed, callback);
	},

	fadeIn: function(speed, callback){
		return this.animate({opacity: "show"}, speed, callback);
	},

	fadeOut: function(speed, callback){
		return this.animate({opacity: "hide"}, speed, callback);
	},

	fadeTo: function(speed,to,callback){
		return this.animate({opacity: to}, speed, callback);
	},

	animate: function( prop, speed, easing, callback ) {
		var optall = jQuery.speed(speed, easing, callback);

		return this[ optall.queue === false ? "each" : "queue" ](function(){
			if ( this.nodeType != 1)
				return false;

			var opt = jQuery.extend({}, optall), p,
				hidden = jQuery(this).is(":hidden"), self = this;

			for ( p in prop ) {
				if ( prop[p] == "hide" && hidden || prop[p] == "show" && !hidden )
					return opt.complete.call(this);

				if ( p == "height" || p == "width" ) {
					// Store display property
					opt.display = jQuery.css(this, "display");

					// Make sure that nothing sneaks out
					opt.overflow = this.style.overflow;
				}
			}

			if ( opt.overflow != null )
				this.style.overflow = "hidden";

			opt.curAnim = jQuery.extend({}, prop);

			jQuery.each( prop, function(name, val){
				var e = new jQuery.fx( self, opt, name );

				if ( /toggle|show|hide/.test(val) )
					e[ val == "toggle" ? hidden ? "show" : "hide" : val ]( prop );
				else {
					var parts = val.toString().match(/^([+-]=)?([\d+-.]+)(.*)$/),
						start = e.cur(true) || 0;

					if ( parts ) {
						var end = parseFloat(parts[2]),
							unit = parts[3] || "px";

						// We need to compute starting value
						if ( unit != "px" ) {
							self.style[ name ] = (end || 1) + unit;
							start = ((end || 1) / e.cur(true)) * start;
							self.style[ name ] = start + unit;
						}

						// If a +=/-= token was provided, we're doing a relative animation
						if ( parts[1] )
							end = ((parts[1] == "-=" ? -1 : 1) * end) + start;

						e.custom( start, end, unit );
					} else
						e.custom( start, val, "" );
				}
			});

			// For JS strict compliance
			return true;
		});
	},

	queue: function(type, fn){
		if ( jQuery.isFunction(type) || ( type && type.constructor == Array )) {
			fn = type;
			type = "fx";
		}

		if ( !type || (typeof type == "string" && !fn) )
			return queue( this[0], type );

		return this.each(function(){
			if ( fn.constructor == Array )
				queue(this, type, fn);
			else {
				queue(this, type).push( fn );

				if ( queue(this, type).length == 1 )
					fn.call(this);
			}
		});
	},

	stop: function(clearQueue, gotoEnd){
		var timers = jQuery.timers;

		if (clearQueue)
			this.queue([]);

		this.each(function(){
			// go in reverse order so anything added to the queue during the loop is ignored
			for ( var i = timers.length - 1; i >= 0; i-- )
				if ( timers[i].elem == this ) {
					if (gotoEnd)
						// force the next step to be the last
						timers[i](true);
					timers.splice(i, 1);
				}
		});

		// start the next in the queue if the last step wasn't forced
		if (!gotoEnd)
			this.dequeue();

		return this;
	}

});

var queue = function( elem, type, array ) {
	if ( elem ){

		type = type || "fx";

		var q = jQuery.data( elem, type + "queue" );

		if ( !q || array )
			q = jQuery.data( elem, type + "queue", jQuery.makeArray(array) );

	}
	return q;
};

jQuery.fn.dequeue = function(type){
	type = type || "fx";

	return this.each(function(){
		var q = queue(this, type);

		q.shift();

		if ( q.length )
			q[0].call( this );
	});
};

jQuery.extend({

	speed: function(speed, easing, fn) {
		var opt = speed && speed.constructor == Object ? speed : {
			complete: fn || !fn && easing ||
				jQuery.isFunction( speed ) && speed,
			duration: speed,
			easing: fn && easing || easing && easing.constructor != Function && easing
		};

		opt.duration = (opt.duration && opt.duration.constructor == Number ?
			opt.duration :
			jQuery.fx.speeds[opt.duration]) || jQuery.fx.speeds.def;

		// Queueing
		opt.old = opt.complete;
		opt.complete = function(){
			if ( opt.queue !== false )
				jQuery(this).dequeue();
			if ( jQuery.isFunction( opt.old ) )
				opt.old.call( this );
		};

		return opt;
	},

	easing: {
		linear: function( p, n, firstNum, diff ) {
			return firstNum + diff * p;
		},
		swing: function( p, n, firstNum, diff ) {
			return ((-Math.cos(p*Math.PI)/2) + 0.5) * diff + firstNum;
		}
	},

	timers: [],
	timerId: null,

	fx: function( elem, options, prop ){
		this.options = options;
		this.elem = elem;
		this.prop = prop;

		if ( !options.orig )
			options.orig = {};
	}

});

jQuery.fx.prototype = {

	// Simple function for setting a style value
	update: function(){
		if ( this.options.step )
			this.options.step.call( this.elem, this.now, this );

		(jQuery.fx.step[this.prop] || jQuery.fx.step._default)( this );

		// Set display property to block for height/width animations
		if ( this.prop == "height" || this.prop == "width" )
			this.elem.style.display = "block";
	},

	// Get the current size
	cur: function(force){
		if ( this.elem[this.prop] != null && this.elem.style[this.prop] == null )
			return this.elem[ this.prop ];

		var r = parseFloat(jQuery.css(this.elem, this.prop, force));
		return r && r > -10000 ? r : parseFloat(jQuery.curCSS(this.elem, this.prop)) || 0;
	},

	// Start an animation from one number to another
	custom: function(from, to, unit){
		this.startTime = now();
		this.start = from;
		this.end = to;
		this.unit = unit || this.unit || "px";
		this.now = this.start;
		this.pos = this.state = 0;
		this.update();

		var self = this;
		function t(gotoEnd){
			return self.step(gotoEnd);
		}

		t.elem = this.elem;

		jQuery.timers.push(t);

		if ( jQuery.timerId == null ) {
			jQuery.timerId = setInterval(function(){
				var timers = jQuery.timers;

				for ( var i = 0; i < timers.length; i++ )
					if ( !timers[i]() )
						timers.splice(i--, 1);

				if ( !timers.length ) {
					clearInterval( jQuery.timerId );
					jQuery.timerId = null;
				}
			}, 13);
		}
	},

	// Simple 'show' function
	show: function(){
		// Remember where we started, so that we can go back to it later
		this.options.orig[this.prop] = jQuery.attr( this.elem.style, this.prop );
		this.options.show = true;

		// Begin the animation
		this.custom(0, this.cur());

		// Make sure that we start at a small width/height to avoid any
		// flash of content
		if ( this.prop == "width" || this.prop == "height" )
			this.elem.style[this.prop] = "1px";

		// Start by showing the element
		jQuery(this.elem).show();
	},

	// Simple 'hide' function
	hide: function(){
		// Remember where we started, so that we can go back to it later
		this.options.orig[this.prop] = jQuery.attr( this.elem.style, this.prop );
		this.options.hide = true;

		// Begin the animation
		this.custom(this.cur(), 0);
	},

	// Each step of an animation
	step: function(gotoEnd){
		var t = now();

		if ( gotoEnd || t > this.options.duration + this.startTime ) {
			this.now = this.end;
			this.pos = this.state = 1;
			this.update();

			this.options.curAnim[ this.prop ] = true;

			var done = true;
			for ( var i in this.options.curAnim )
				if ( this.options.curAnim[i] !== true )
					done = false;

			if ( done ) {
				if ( this.options.display != null ) {
					// Reset the overflow
					this.elem.style.overflow = this.options.overflow;

					// Reset the display
					this.elem.style.display = this.options.display;
					if ( jQuery.css(this.elem, "display") == "none" )
						this.elem.style.display = "block";
				}

				// Hide the element if the "hide" operation was done
				if ( this.options.hide )
					this.elem.style.display = "none";

				// Reset the properties, if the item has been hidden or shown
				if ( this.options.hide || this.options.show )
					for ( var p in this.options.curAnim )
						jQuery.attr(this.elem.style, p, this.options.orig[p]);
			}

			if ( done )
				// Execute the complete function
				this.options.complete.call( this.elem );

			return false;
		} else {
			var n = t - this.startTime;
			this.state = n / this.options.duration;

			// Perform the easing function, defaults to swing
			this.pos = jQuery.easing[this.options.easing || (jQuery.easing.swing ? "swing" : "linear")](this.state, n, 0, 1, this.options.duration);
			this.now = this.start + ((this.end - this.start) * this.pos);

			// Perform the next step of the animation
			this.update();
		}

		return true;
	}

};

jQuery.extend( jQuery.fx, {
	speeds:{
		slow: 600,
 		fast: 200,
 		// Default speed
 		def: 400
	},
	step: {
		scrollLeft: function(fx){
			fx.elem.scrollLeft = fx.now;
		},

		scrollTop: function(fx){
			fx.elem.scrollTop = fx.now;
		},

		opacity: function(fx){
			jQuery.attr(fx.elem.style, "opacity", fx.now);
		},

		_default: function(fx){
			fx.elem.style[ fx.prop ] = fx.now + fx.unit;
		}
	}
});
// The Offset Method
// Originally By Brandon Aaron, part of the Dimension Plugin
// http://jquery.com/plugins/project/dimensions
jQuery.fn.offset = function() {
	var left = 0, top = 0, elem = this[0], results;

	if ( elem ) with ( jQuery.browser ) {
		var parent       = elem.parentNode,
		    offsetChild  = elem,
		    offsetParent = elem.offsetParent,
		    doc          = elem.ownerDocument,
		    safari2      = safari && parseInt(version) < 522 && !/adobeair/i.test(userAgent),
		    css          = jQuery.curCSS,
		    fixed        = css(elem, "position") == "fixed";

		// Use getBoundingClientRect if available
		if ( elem.getBoundingClientRect ) {
			var box = elem.getBoundingClientRect();

			// Add the document scroll offsets
			add(box.left + Math.max(doc.documentElement.scrollLeft, doc.body.scrollLeft),
				box.top  + Math.max(doc.documentElement.scrollTop,  doc.body.scrollTop));

			// IE adds the HTML element's border, by default it is medium which is 2px
			// IE 6 and 7 quirks mode the border width is overwritable by the following css html { border: 0; }
			// IE 7 standards mode, the border is always 2px
			// This border/offset is typically represented by the clientLeft and clientTop properties
			// However, in IE6 and 7 quirks mode the clientLeft and clientTop properties are not updated when overwriting it via CSS
			// Therefore this method will be off by 2px in IE while in quirksmode
			add( -doc.documentElement.clientLeft, -doc.documentElement.clientTop );

		// Otherwise loop through the offsetParents and parentNodes
		} else {

			// Initial element offsets
			add( elem.offsetLeft, elem.offsetTop );

			// Get parent offsets
			while ( offsetParent ) {
				// Add offsetParent offsets
				add( offsetParent.offsetLeft, offsetParent.offsetTop );

				// Mozilla and Safari > 2 does not include the border on offset parents
				// However Mozilla adds the border for table or table cells
				if ( mozilla && !/^t(able|d|h)$/i.test(offsetParent.tagName) || safari && !safari2 )
					border( offsetParent );

				// Add the document scroll offsets if position is fixed on any offsetParent
				if ( !fixed && css(offsetParent, "position") == "fixed" )
					fixed = true;

				// Set offsetChild to previous offsetParent unless it is the body element
				offsetChild  = /^body$/i.test(offsetParent.tagName) ? offsetChild : offsetParent;
				// Get next offsetParent
				offsetParent = offsetParent.offsetParent;
			}

			// Get parent scroll offsets
			while ( parent && parent.tagName && !/^body|html$/i.test(parent.tagName) ) {
				// Remove parent scroll UNLESS that parent is inline or a table to work around Opera inline/table scrollLeft/Top bug
				if ( !/^inline|table.*$/i.test(css(parent, "display")) )
					// Subtract parent scroll offsets
					add( -parent.scrollLeft, -parent.scrollTop );

				// Mozilla does not add the border for a parent that has overflow != visible
				if ( mozilla && css(parent, "overflow") != "visible" )
					border( parent );

				// Get next parent
				parent = parent.parentNode;
			}

			// Safari <= 2 doubles body offsets with a fixed position element/offsetParent or absolutely positioned offsetChild
			// Mozilla doubles body offsets with a non-absolutely positioned offsetChild
			if ( (safari2 && (fixed || css(offsetChild, "position") == "absolute")) ||
				(mozilla && css(offsetChild, "position") != "absolute") )
					add( -doc.body.offsetLeft, -doc.body.offsetTop );

			// Add the document scroll offsets if position is fixed
			if ( fixed )
				add(Math.max(doc.documentElement.scrollLeft, doc.body.scrollLeft),
					Math.max(doc.documentElement.scrollTop,  doc.body.scrollTop));
		}

		// Return an object with top and left properties
		results = { top: top, left: left };
	}

	function border(elem) {
		add( jQuery.curCSS(elem, "borderLeftWidth", true), jQuery.curCSS(elem, "borderTopWidth", true) );
	}

	function add(l, t) {
		left += parseInt(l, 10) || 0;
		top += parseInt(t, 10) || 0;
	}

	return results;
};


jQuery.fn.extend({
	position: function() {
		var left = 0, top = 0, results;

		if ( this[0] ) {
			// Get *real* offsetParent
			var offsetParent = this.offsetParent(),

			// Get correct offsets
			offset       = this.offset(),
			parentOffset = /^body|html$/i.test(offsetParent[0].tagName) ? { top: 0, left: 0 } : offsetParent.offset();

			// Subtract element margins
			// note: when an element has margin: auto the offsetLeft and marginLeft 
			// are the same in Safari causing offset.left to incorrectly be 0
			offset.top  -= num( this, 'marginTop' );
			offset.left -= num( this, 'marginLeft' );

			// Add offsetParent borders
			parentOffset.top  += num( offsetParent, 'borderTopWidth' );
			parentOffset.left += num( offsetParent, 'borderLeftWidth' );

			// Subtract the two offsets
			results = {
				top:  offset.top  - parentOffset.top,
				left: offset.left - parentOffset.left
			};
		}

		return results;
	},

	offsetParent: function() {
		var offsetParent = this[0].offsetParent;
		while ( offsetParent && (!/^body|html$/i.test(offsetParent.tagName) && jQuery.css(offsetParent, 'position') == 'static') )
			offsetParent = offsetParent.offsetParent;
		return jQuery(offsetParent);
	}
});


// Create scrollLeft and scrollTop methods
jQuery.each( ['Left', 'Top'], function(i, name) {
	var method = 'scroll' + name;
	
	jQuery.fn[ method ] = function(val) {
		if (!this[0]) return;

		return val != undefined ?

			// Set the scroll offset
			this.each(function() {
				this == window || this == document ?
					window.scrollTo(
						!i ? val : jQuery(window).scrollLeft(),
						 i ? val : jQuery(window).scrollTop()
					) :
					this[ method ] = val;
			}) :

			// Return the scroll offset
			this[0] == window || this[0] == document ?
				self[ i ? 'pageYOffset' : 'pageXOffset' ] ||
					jQuery.boxModel && document.documentElement[ method ] ||
					document.body[ method ] :
				this[0][ method ];
	};
});
// Create innerHeight, innerWidth, outerHeight and outerWidth methods
jQuery.each([ "Height", "Width" ], function(i, name){

	var tl = i ? "Left"  : "Top",  // top or left
		br = i ? "Right" : "Bottom"; // bottom or right

	// innerHeight and innerWidth
	jQuery.fn["inner" + name] = function(){
		return this[ name.toLowerCase() ]() +
			num(this, "padding" + tl) +
			num(this, "padding" + br);
	};

	// outerHeight and outerWidth
	jQuery.fn["outer" + name] = function(margin) {
		return this["inner" + name]() +
			num(this, "border" + tl + "Width") +
			num(this, "border" + br + "Width") +
			(margin ?
				num(this, "margin" + tl) + num(this, "margin" + br) : 0);
	};

});})();

}

/* END THIRD PARTY SOURCE */

/* prolog.js */

/**
 * this file is included *before* any other thirdparty libraries or 
 * SDK files.  you must assume that you have no external capabilities in this file
 */
 
(function($)
{
	

//--------------------------------------------------------------------------------

/* uri.js */

/*!
 ******************************************************************************
  uri_funcs.js - URI functions based on STD 66 / RFC 3986

  Author (original): Mike J. Brown <mike at skew.org>
  Version: 2007-01-04

  License: Unrestricted use and distribution with any modifications permitted,
  so long as:
  1. Modifications are attributed to their author(s);
  2. The original author remains credited;
  3. Additions derived from other code libraries are credited to their sources
  and used under the terms of their licenses.

*******************************************************************************/

/** 
 * slight modifications by Jeff Haynie of Appcelerator
 */

var absoluteUriRefRegex = /^[A-Z][0-9A-Z+\-\.]*:/i;
var splitUriRefRegex = /^(([^:\/?#]+):)?(\/\/([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?$/;
var reMissingGroupSupport = (typeof "".match(/(a)?/)[1] != "string");

URI = {};

/**
 * This function determines whether the given URI reference is absolute
 * (has a scheme).
 */
URI.isAbsolute = function(uriRef) 
{
	return absoluteUriRefRegex.test(uriRef);
};

/*
splitUriRef(uriRef)

This function splits a URI reference into an Array of its principal components,
[scheme, authority, path, query, fragment] as per STD 66 / RFC 3986 appendix B.
*/

URI.splitUriRef = function(uriRef) 
{
	var parts = uriRef.match(splitUriRefRegex);
	parts.shift();
	var scheme=parts[1], auth=parts[3], path=parts[4], query=parts[6], frag=parts[8];
	if (!reMissingGroupSupport) {
		var undef;
		if (parts[0] == "") scheme = undef;
		if (parts[2] == "") auth = undef;
		if (parts[5] == "") query = undef;
		if (parts[7] == "") frag = undef;
	}
	parts = [scheme, auth, this.uriPathRemoveDotSegments(path), query, frag];
	return parts;
};

/*
unsplitUriRef(uriRefSeq)

This function, given an Array as would be produced by splitUriRef(),
assembles and returns a URI reference as a string.
*/
URI.unsplitUriRef=function(uriRefSeq) 
{
    var uriRef = "";
    if (typeof uriRefSeq[0] != "undefined") uriRef += uriRefSeq[0] + ":";
    if (typeof uriRefSeq[1] != "undefined") uriRef += "//" + uriRefSeq[1];
    uriRef += uriRefSeq[2];
    if (typeof uriRefSeq[3] != "undefined") uriRef += "?" + uriRefSeq[3];
    if (typeof uriRefSeq[4] != "undefined") uriRef += "#" + uriRefSeq[4];
    return uriRef;
}

/*
uriPathRemoveDotSegments(path)

This function supports absolutizeURI() by implementing the remove_dot_segments
function described in RFC 3986 sec. 5.2.  It collapses most of the '.' and '..'
segments out of a path without eliminating empty segments. It is intended
to be used during the path merging process and may not give expected
results when used independently.

Based on code from 4Suite XML:
http://cvs.4suite.org/viewcvs/4Suite/Ft/Lib/Uri.py?view=markup
*/
URI.uriPathRemoveDotSegments = function (path) 
{
	// return empty string if entire path is just "." or ".."
	if (path == "." || path == "..") {
		return "";
	}
	// remove all "./" or "../" segments at the beginning
	while (path) {
		if (path.substring(0,2) == "./") {
			path = path.substring(2);
		} else if (path.substring(0,3) == "../") {
			path = path.substring(3);
		} else if (path.substring(0,2)=="//") {
		   path = path.substring(1);
		} else {
			break;
		}
	}
	// We need to keep track of whether there was a leading slash,
	// because we're going to drop it in order to prevent our list of
	// segments from having an ambiguous empty first item when we call
	// split().
	var leading_slash = false;
	if (path.charAt(0) == "/") {
		path = path.substring(1);
		if (path.charAt(0)=='/')
		{
			path = path.substring(1);
		}
		leading_slash = true;
	}
	// replace a trailing "/." with just "/"
	if (path.substring(path.length - 2) == "/.") {
		path = path.substring(0, path.length - 1);
	}
	// convert the segments into a list and process each segment in
	// order from left to right.
	var segments = path.split("/");
	var keepers = [];
	segments = segments.reverse();
	while (segments.length) {
		var seg = segments.pop();
		// '..' means drop the previous kept segment, if any.
		// If none, and if the path is relative, then keep the '..'.
		// If the '..' was the last segment, ensure
		// that the result ends with '/'.
		if (seg == "..") {
			if (keepers.length) {
				keepers.pop();
			} else if (! leading_slash) {
				keepers.push(seg);
			}
			if (! segments.length) {
				keepers.push("");
			}
		// ignore '.' segments and keep all others, even empty ones
		} else if (seg != ".") {
			keepers.push(seg);
		}
	}
	// reassemble the kept segments
	return (leading_slash && "/" || "") + keepers.join("/");
}

/*
absolutizeURI(uriRef, baseUri)

This function resolves a URI reference to absolute form as per section 5 of
STD 66 / RFC 3986. The URI reference is considered to be relative to the
given base URI.

It is the caller's responsibility to ensure that the base URI matches
the absolute-URI syntax rule of RFC 3986, and that its path component
does not contain '.' or '..' segments if the scheme is hierarchical.
Unexpected results may occur otherwise.

Based on code from 4Suite XML:
http://cvs.4suite.org/viewcvs/4Suite/Ft/Lib/Uri.py?view=markup
*/
URI.absolutizeURI = function(uriRef, baseUri)
{
	// Ensure base URI is absolute
	if (! baseUri || ! URI.isAbsolute(baseUri)) {
		 throw Error("baseUri '" + baseUri + "' is not absolute");
	}
	// shortcut for the simplest same-document reference cases
	if (uriRef == "" || uriRef.charAt(0) == "#") {
		return baseUri.split('#')[0] + uriRef;
	}
	var tScheme, tAuth, tPath, tQuery;
	// parse the reference into its components
	var parts = URI.splitUriRef(uriRef);
	var rScheme=parts[0], rAuth=parts[1], rPath=parts[2], rQuery=parts[3], rFrag=parts[4];
	// if the reference is absolute, eliminate '.' and '..' path segments
	// and skip to the end
	if (typeof rScheme != "undefined") {
		var tScheme = rScheme;
		var tAuth = rAuth;
		var tPath = URI.uriPathRemoveDotSegments(rPath);
		var tQuery = rQuery;
	} else {
		// the base URI's scheme, and possibly more, will be inherited
		parts = URI.splitUriRef(baseUri);
		var bScheme=parts[0], bAuth=parts[1], bPath=parts[2], bQuery=parts[3], bFrag=parts[4];
		// if the reference is a net-path, just eliminate '.' and '..' path
		// segments; no other changes needed.
		if (typeof rAuth != "undefined") {
			tAuth = rAuth;
			tPath = URI.uriPathRemoveDotSegments(rPath);
			tQuery = rQuery;
		// if it's not a net-path, we need to inherit pieces of the base URI
		} else {
			// use base URI's path if the reference's path is empty
			if (! rPath) {
				tPath = bPath;
				// use the reference's query, if any, or else the base URI's,
				tQuery = (typeof rQuery != "undefined" && rQuery || bQuery);
			// the reference's path is not empty
			} else {
				// just use the reference's path if it's absolute
				if (rPath.charAt(0) == "/") {
					tPath = URI.uriPathRemoveDotSegments(rPath);
				// merge the reference's relative path with the base URI's path
				} else {
					if (typeof bAuth != "undefined" && ! bPath) {
						tPath = "/" + rPath;
					} else {
						tPath = bPath.substring(0, bPath.lastIndexOf("/") + 1) + rPath;
					}
					tPath = URI.uriPathRemoveDotSegments(tPath);
				}
				// use the reference's query
				tQuery = rQuery;
			}
			// since the reference isn't a net-path,
			// use the authority from the base URI
			tAuth = bAuth;
		}
		// inherit the scheme from the base URI
		tScheme = bScheme;
	}
	// always use the reference's fragment (but no need to define another var)
	//tFrag = rFrag;
	// now compose the target URI (RFC 3986 sec. 5.3)
	var result = URI.unsplitUriRef([tScheme, tAuth, tPath, tQuery, rFrag]);
	return result;
};

//--------------------------------------------------------------------------------

/* bootstrap.js */


/*- Appcelerator + jQuery - a match made in heaven */

//
// App is the private namespace that is used internally. This API is not stable and should not be used.
// AppC is the semi-stable API that can be used externally.
//
App = AppC = {};

AppC.Version = 
{
	value: '3.0.0',
	major: parseInt('3'),
	minor: parseInt('0'),
	revision: parseInt('0'),
	date: '11/04/2008',
	toString:function()
	{
		return this.value;
	}
};

var started = new Date;
var compileTime;
var loadTime;

AppC.LicenseType = 'Apache License Version 2.0 - see http://license.appcelerator.org';
AppC.Copyright = 'Copyright (c) 2006-'+(1900+started.getYear())+' by Appcelerator, Inc. All Rights Reserved.';
AppC.LicenseMessage = 'Appcelerator is licensed under ' + AppC.LicenseType;


//
// these are parameters that can be set by the developer to customize appcelerator based on app needs
//
AppC.config = 
{
	track_stats:true,    /* true to turn on simple usage tracking to help us improve product */
	report_stats:true,   /* true to send a remote message with client stats to server on page load */
	browser_check:true,  /* true to check for valid grade-A browser support when document is loaded */
	auto_locale:false    /* true to attempt to auto load localization bundle based on users locale when page is loaded */
};

//
// these are parameters that can be used to customize appcelerator from a users perspective
//
AppC.params = 
{
	debug: 0                 /* set to 1 to turn on verbose logging, 2 to turn on only pub/sub logging */,
	delayCompile: false      /* generally don't touch this unless you really know why */
};

function queryString(uri,params)
{
	idx = uri.indexOf('?');
	params = params || {};
	if (idx > 0)
	{
		var qs = uri.substring(idx+1);
		$.each(qs.split('&'),function()
		{
			var e = this.split('=');
			var v = decodeURIComponent(e[1]||'');
			var k = decodeURIComponent(e[0]);
			switch(v)
			{
				case '1':
				case 'true':
				case 'yes':
				{
					v = true;
					break;
				}
				case '0':
				case 'false':
				case 'no':
				{
					v = false;
					break;
				}
			}
			params[k]=v;
		});
	}
	return params;
}


// get config parameters for app from the URI of the page
queryString(window.document.location.href,AppC.params);

var removeLastElement = function(uri) {
    var idx = uri.lastIndexOf('/');
    if (idx != 1)
    {
        uri = uri.substring(0, idx) + "/";
    }
    return uri;
}

// top is important such that if the JS file is in a different location (hosted)
// than the primary document, we use the primary document's path (cross site scripting)
var documentRoot = removeLastElement(top.window.document.location.href);

// get appcelerator.js and base paths
// and ensure these uris are absolute
var jsLocation = $('script[@src~=appcelerator]').get(0).src;
var baseLocation = $('base[@href]').attr('href');
baseLocation = baseLocation ? URI.absolutizeURI(baseLocation, documentRoot) : "";
jsLocation = jsLocation ? URI.absolutizeURI(jsLocation, documentRoot) : "";

if (jsLocation)
{
	AppC.sdkJS = URI.absolutizeURI(jsLocation,documentRoot);
    AppC.sdkRoot = removeLastElement(jsLocation); // parent directory of js
    var docHost = URI.splitUriRef(documentRoot)[1];
    var jsHost = URI.splitUriRef(jsLocation)[1];

    // we need to know where appcelerator.xml is located
    if (docHost == jsHost) // locally hosted
    {
        AppC.docRoot = URI.absolutizeURI(".", AppC.sdkRoot + "..");
    }
    else if (docHost != jsHost && baseLocation) // remote js -- use base location
    {
        AppC.docRoot = baseLocation;
    }
    else
    {
        AppC.docRoot = URI.absolutizeURI(".", documentRoot);
    }
}
else
{
    $.error("Can't find appcelerator.js or appcelerator-debug.js");
	return false;
}

// add a slash if the path is missing one
if (!AppC.sdkRoot.charAt(AppC.sdkRoot.length - 1) == '/')
{
    AppC.sdkRoot += '/'; 
}
if (!AppC.docRoot.charAt(AppC.docRoot.length - 1) == '/')
{
    AppC.docRoot += '/'; 
}

AppC.compRoot = AppC.sdkRoot + 'components/';
AppC.pluginRoot = AppC.sdkRoot + 'plugins/';

// override the configuration for appcelerator from the appcelerator JS query string
queryString(jsLocation, AppC.config);


var appid = 0;

App.ensureId=function(el)
{
	var rootEl = el.nodeType ? el : $(el).get(0);
	var id = rootEl.id;
	if (!id)
	{
		rootEl.id = rootEl.nodeName == 'BODY' ? 'app_body' : 'app_' + (appid++);
	}
	return el;
};

$.fn.compile = function()
{
	if (arguments.length == 2 && typeof(arguments[0])=='object')
	{
		var state = arguments[1];
		$.each(arguments[0],function()
		{
			$(this).compile(state);
		});
	}
	else if (arguments.length == 1 && typeof(arguments[0].count)=='number')
	{
		// compile a single element
		var state = arguments[0];
		var node = $(this).get(0);
		var el = App.ensureId(node);
		var e = $(el);
		App.incState(state);
		var myid = e.attr('id');
		var compiled = App.runProcessors(el,state);
		$.debug(' + compiled #'+myid+' ('+getTagName(node)+') => '+compiled);
		// if false, means that the attribute processor will call
		// checkState when he's done
		if (compiled)
		{
			App.checkState(state,el);
		}
	}
	return this;
};

$.fn.compileChildren = function(state,self)
{
	var node = $(this).get(0);
	App.ensureId(node);
	var set = getTargetCompileSet(node,self);
	this.compile(set,state);
	return this;
}

var state = function(el)
{
	this.count = 1;
	this.el = el;
	this.completed = [];
};

App.createState = function(el)
{
	return new state(el)
};

App.incState=function(state)
{
	if (state)
	{
		var count = ++state.count;
		return count;
	}
};

var bodyCompiled = false;

App.checkState=function(state,el)
{
	if (state)
	{
		if (el) state.completed.push($(el).get(0));
		var count = --state.count;
		if (count == 0)
		{
			$.each($.unique(state.completed),function()
			{
				if (this != document.body)
				{
					$(this).trigger('compiled');
				}
			});
			// we must always fire compiled on body and do it last
			// but we only ever fire it once for a document load
			if (!bodyCompiled)
			{
				bodyCompiled=true;
				$(document.body).trigger('compiled');
			}
		}
	}
};

function getTargetCompileSet(node,self)
{
	var expr = null, filter = null;
	
	if (node!=null)
	{
		node = typeof(node.nodeType)=='undefined' ? node.get(0) : node;
		var parent = node.nodeName == 'BODY' ? 'body' : '#'+node.id;
		expr = (self ? (parent + ',') : '')  + parent + ' ' + App.selectors.join(', ' + parent + ' ');
	}
	else
	{
		expr = App.selectors.join(',');
		filter = function()
		{
			// this filter prevents us from compiling an element has is child of
			// any parent where it has set attribute
			var exclude = App.delegateCompilers.join(',');
			return !$(this).parents(exclude).length;
		};
	}
	
	if (filter)
	{
		return $.unique($(expr).filter(filter));
	}
	
	return $.unique($(expr));
};

var beforeCompilers = [];

AppC.beforeCompile = function(f)
{
	if (!beforeCompilers)
	{
		f();
	}
	else
	{
		beforeCompilers.push(f);
	}
	return AppC;
};

AppC.compileDocument = function()
{
	var compileStarted = new Date;
	var body = $(document.body);
	
	// call any pending guys waiting for us to get 
	// started (means they're waiting for document.ready)
	if (beforeCompilers)
	{
		$.each(beforeCompilers,function()
		{
			this(body);
		});
		beforeCompilers=null;
	}
	
	body.bind('compiled',function()
	{
		body.pub('l:app.compiled',{
			event:{id:document.body.id||'body'}
		});
		$(document).trigger('compiled');
		compileFinished = new Date;
		loadTime = compileFinished - started;
		compileTime = compileFinished - compileStarted;
		
		if (top.window === window)
		{
			$.info(AppC.Copyright);
			$.info(AppC.LicenseMessage);
			$.info('loaded in ' + (loadTime) + ' ms, compiler took ~'+(compileTime)+' ms');
			$.info('Appcelerator is ready!');
		}
	});
	
	var s = new state(body);
	$(document).compile(getTargetCompileSet(),s);
	App.checkState(s); // state starts at 1, call to dec
};

if (!AppC.params.delayCompile) $(document).ready(AppC.compileDocument);


//--------------------------------------------------------------------------------

/* jquery.js */


var debug = AppC.params.debug && AppC.params.debug == '1' || AppC.params.debug == true;
var hasConsole = typeof(console)!='undefined';
var keyStr = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

$.extend(
{
	domText:function(dom)
	{
		var str = '';
		for (var c=0;c<dom.childNodes.length;c++)
		{
			str+=dom.childNodes[c].nodeValue||'';
		}
		return $.trim(str);
	},
	gsub:function(source,pattern,replacement)
	{
		if (typeof(replacement)=='string')
		{
			var r = String(replacement);
			replacement = function()
			{
				return r;
			}
		}
	 	var result = '', match;
	    while (source.length > 0) {
	      if (match = source.match(pattern)) {
	        result += source.slice(0, match.index);
	        result += this.string(replacement(match));
	        source  = source.slice(match.index + match[0].length);
	      } else {
	        result += source, source = '';
	      }
	    }
		return result;
	},
	camel: function(value)
	{
	    var parts = value.split('-'), len = parts.length;
	    if (len == 1) return parts[0];

	    var camelized = value.charAt(0) == '-'
	      ? parts[0].charAt(0).toUpperCase() + parts[0].substring(1)
	      : parts[0];

	    for (var i = 1; i < len; i++)
	      camelized += parts[i].charAt(0).toUpperCase() + parts[i].substring(1);

	    return camelized;
	},
	string: function(value)
	{
	    return value == null ? '' : String(value);
	},
	proper: function(value)
	{
		return value.charAt(0).toUpperCase() + value.substring(1);
	},
	smartSplit: function(value,splitter)
	{
		if (typeof(value)!='string')
		{
			//throw "Invalid parameter passed to smartSplit: "+typeof(value)+", value: "+value;
			$.error("Invalid parameter passed to smartSplit: "+typeof(value)+", value: "+value);
		}
		value = this.trim(value);
		var tokens = value.split(splitter);
		if(tokens.length == 1) return tokens;
		var array = [];
		var current = null;
		for (var c=0;c<tokens.length;c++)
		{
			var line = tokens[c];
			if (!current && line.charAt(0)=='(')
			{
				current = line + ' or ';
				continue;
			}
			else if (current && current.charAt(0)=='(')
			{
				if (line.indexOf(') ')!=-1)
				{
					array.push(current+line);
					current = null;
				}
				else
				{
					current+=line + ' or ';
				}
				continue;
			}
			if (!current && line.indexOf('[')>=0 && line.indexOf(']')==-1)
			{
				if (current)
				{
					current+=splitter+line;
				}
				else
				{
					current = line;
				}
			}
			else if (current && line.indexOf(']')==-1)
			{
				current+=splitter+line;
			}
			else
			{
				if (current)
				{
					array.push(current+splitter+line)
					current=null;
				}
				else
				{
					array.push(line);
				}
			}
		}
		return array;
	},
	escapeHTML: function(value)
	{
		// idea from prototype
		var div = document.createElement('div');
		var text = document.createTextNode(value);
		div.appendChild(text);
		return div.innerHTML;
	},
	escapeXML: function(value)
	{
		if (!value) return null;
	    return value.replace(
	    /&/g, "&amp;").replace(
	    /</g, "&lt;").replace(
	    />/g, "&gt;").replace(
	    /"/g, "&quot;").replace(
	    /'/g, "&apos;");
	},
	unescapeXML: function(value)
	{
	    if (!value) return null;
	    return value.replace(
		/&lt;/g,   "<").replace(
		/&gt;/g,   ">").replace(
		/&apos;/g, "'").replace(
		/&amp;/g,  "&").replace(
		/&quot;/g, "\"");
	},
	emptyFunction: function(){},
	toFunction: function (str,dontPreProcess)
    {
        var str = $.trim(str);
        if (str.length == 0)
        {
            return this.emptyFunction;
        }
        if (!dontPreProcess)
        {
            if (str.match(/^function\(/))
            {
                str = 'return ' + this.unescapeXML(str) + '()';
            }
            else if (!str.match(/return/))
            {
                str = 'return ' + this.unescapeXML(str);
            }
            else if (str.match(/^return function/))
            {
                // invoke it as the return value
                str = this.unescapeXML(str) + ' ();';
            }
        }
        var code = 'var f = function(){ var args = $.makeArray(arguments); ' + str + '}; f;';
        var func = eval(code);
        if (this.isFunction(func))
        {
            return func;
        }
        throw Error('code was not a function: ' + this);
    },
	/**
	 * simple function will walk the properties of an object
	 * based on dotted notatation. example:
	 *
	 *  var obj = {
	 *        foo: {
	 *           bar: 'a',
	 *           foo: {
	 *              bar: 1,
	 *              jeff: 'haynie'
	 *           }
	 *        }
	 *  };
	 *
	 *  var value = $.getNestedProperty(obj,'foo.foo.jeff')
	 *  
	 * The value variable should equal 'haynie'
	 */
	getNestedProperty: function (obj, prop, def)
	{
	    if (obj!=null && prop!=null)
	    {
	        var props = prop.split('.');
	        if (props.length != -1)
	        {
		        var cur = obj;
		        this.each(props,function()
		        {
					var p = this;
		            if (null != cur[p])
		            {
		                cur = cur[p];
		            }
		            else
		            {
		                cur = null;
		                return false;
		            }
		        });
		        return cur == null ? def : cur;
		     }
		     else
		     {
		     	  return obj[prop] == null ? def : obj[prop];
		     }
	    }
	    return def;
	},
    startsWith: function(a,b)
    {
    	return a.indexOf(b) === 0;
    },
	error:function()
	{
		var log = $.makeArray(arguments).join(' ');
		if (hasConsole)
		{
			if ($.isFunction(console.error))
			{
				console.error(log);
			}
			else if ($.isFunction(console.log))
			{
				console.log(log);
			}
		}
	},
	info:function()
	{
		var log = $.makeArray(arguments).join(' ');
		if (hasConsole)
		{
			if ($.isFunction(console.info))
			{
				console.info(log);
			}
			else if ($.isFunction(console.log))
			{
				console.log(log);
			}
		}
	},
	debug:function()
	{
		if (debug)
		{
			var log = $.makeArray(arguments).join(' ');
			if (hasConsole)
			{
				if ($.isFunction(console.debug))
				{
					console.debug(log);
				}
				else if ($.isFunction(console.log))
				{
					console.log(log);
				}
			}
		}
	},
	encode64: function(input) 
	{
	   var output = "";
	   var chr1, chr2, chr3;
	   var enc1, enc2, enc3, enc4;
	   var i = 0;
	
	   do {
	      chr1 = input.charCodeAt(i++);
	      chr2 = input.charCodeAt(i++);
	      chr3 = input.charCodeAt(i++);
	
	      enc1 = chr1 >> 2;
	      enc2 = ((chr1 & 3) << 4) | (chr2 >> 4);
	      enc3 = ((chr2 & 15) << 2) | (chr3 >> 6);
	      enc4 = chr3 & 63;
	
	      if (isNaN(chr2)) {
	         enc3 = enc4 = 64;
	      } else if (isNaN(chr3)) {
	         enc4 = 64;
	      }
	
	      output = output + keyStr.charAt(enc1) + keyStr.charAt(enc2) + 
	         keyStr.charAt(enc3) + keyStr.charAt(enc4);
	   } while (i < input.length);
	   
	   return output;
	},
	decode64: function (input) 
	{
	   var output = "";
	   var chr1, chr2, chr3;
	   var enc1, enc2, enc3, enc4;
	   var i = 0;
	   
	   // remove all characters that are not A-Z, a-z, 0-9, +, /, or =
	   input = input.replace(/[^A-Za-z0-9\+\/\=]/g, "");
	
	   do {
	      enc1 = keyStr.indexOf(input.charAt(i++));
	      enc2 = keyStr.indexOf(input.charAt(i++));
	      enc3 = keyStr.indexOf(input.charAt(i++));
	      enc4 = keyStr.indexOf(input.charAt(i++));
	
	      chr1 = (enc1 << 2) | (enc2 >> 4);
	      chr2 = ((enc2 & 15) << 4) | (enc3 >> 2);
	      chr3 = ((enc3 & 3) << 6) | enc4;
	
	      output = output + String.fromCharCode(chr1);
	
	      if (enc3 != 64) {
	         output = output + String.fromCharCode(chr2);
	      }
	      if (enc4 != 64) {
	         output = output + String.fromCharCode(chr3);
	      }
	   } while (i < input.length);
	
	   return output;
	},
	toQueryString:function(obj)
	{
		var qs = [];
		for (var k in obj)
		{
			var v = obj[k];
			var t = typeof(v);
			switch(t)
			{
				case 'number':
				case 'string':
				case 'boolean':
				{
					qs.push(encodeURIComponent(k)+'='+encodeURIComponent(String(v)));
					break;
				}
				case 'object':
				{
					qs.push(encodeURIComponent(k)+'='+encodeURIComponent($.toJSON(v)));
					break;
				}
			}
		}
		return qs.join('&');
	},
	loadCSS: function(url)
	{
		var head = document.getElementsByTagName("head")[0];
		var link = document.createElement("link");
		link.rel = 'stylesheet';
		link.type = 'text/css';
		link.href = url;
		head.appendChild(link);
	}
});


/* Based on Alex Arnell's inheritance implementation. */
$.Class = {
  create: function() {
    var parent = null, properties = $.makeArray(arguments);
    if ($.isFunction(properties[0]))
      parent = properties.shift();

    function klass() {
      this.initialize.apply(this, arguments);
    }

    $.Class.extend(klass, $.Class.Methods);
    klass.superclass = parent;
    klass.subclasses = [];

    if (parent) {
      var subclass = function() { };
      subclass.prototype = parent.prototype;
      klass.prototype = new subclass;
      parent.subclasses.push(klass);
    }

    for (var i = 0; i < properties.length; i++)
      klass.addMethods(properties[i]);

    if (!klass.prototype.initialize)
      klass.prototype.initialize = $.emptyFunction;

    klass.prototype.constructor = klass;

    return klass;
  }
};

$.Class.Methods = {
  addMethods: function(source) {
    var ancestor   = this.superclass && this.superclass.prototype;
    var properties = [];
	for (var key in source)
	{
		properties.push(key);
	}

    for (var i = 0, length = properties.length; i < length; i++) {
      var property = properties[i], value = source[property];
      this.prototype[property] = value;
    }

    return this;
  }
};

$.Class.extend = function(destination, source) {
  for (var property in source)
    destination[property] = source[property];
  return destination;
};

// public API support
AppC.create = $.Class.create;
AppC.extend = $.Class.extend;

$.fn.delay = function(fn,delay)
{
	var scope = this;
	// support either first argument is number and second is function -or-
	// first argument is function and second is number ... sometimes more readable is number first
	var f = typeof(fn)=='number' ? delay : fn;
	var n = typeof(fn)=='function' ? delay : fn;
	setTimeout(function(){ 
		f.call(scope);
	},(n||0.1)*1000);
	return this;
};

$.fn.defer = function(fn,delay)
{
	$(this).delay(fn,0.001);
	return this;
};


//--------------------------------------------------------------------------------

/* types.js */

/**
 * Used for defining metadata of widgets,
 * and maybe of conditions and actions.
 * 
 * Some of these types we won't define checkers for,
 * but will use them instead for auto-completion in the IDE.
 *
 */
AppC.Types = {};

AppC.Types.enumeration = function()
{
    var pattern = '^('+ $.makeArray(arguments).join(')|(') +')$';
    return {name: 'Enumeration', values: $.makeArray(arguments), regex: RegExp(pattern)};
};
AppC.Types.openEnumeration = function()
{
	// accepts anything, suggests some things that it knows will work
    var pattern = '^('+ $.makeArray(arguments).join(')|(') +')|(.*)$';
    return {name: 'Enumeration', values: $.makeArray(arguments), regex: RegExp(pattern)};
};
AppC.Types.pattern = function(regex, name)
{
	name = name || 'pattern';
    return {name: name, regex: regex};
};

AppC.Types.bool = AppC.Types.enumeration('true','false');
AppC.Types.bool.name = "Boolean"
AppC.Types.number = AppC.Types.pattern(/^-?[0-9]+(\.[0-9]+)$/, 'Number');
AppC.Types.naturalNumber = AppC.Types.pattern(/[0-9]+/, 'Natural Number');

AppC.Types.cssDimension = AppC.Types.pattern(
    /^-?[0-9]+(\.[0-9]+)(%|(em)|(en)|(px)|(pc)|(pt))?$/, 'CSS Dimension');
	
AppC.Types.identifier = AppC.Types.pattern(
    /^[a-zA-Z_][a-zA-Z0-9_.]*$/, 'Identifier');

/*
 * Message sends can only be literal message names,
 * while message receptions can include a matching regex.
 * The distinction between these can also be used by tools
 * to detect messages sent but not received, and vice-versa
 * (probably indicating a typo).
 */
AppC.Types.messageSend = AppC.Types.pattern(
    /^((l:)|(r:)|(local:)|(remote:))[a-zA-Z0-9_.]+/, 'AppC Message Send');
AppC.Types.messageReceive = AppC.Types.pattern(
    /^((l:)|(r:)|(local:)|(remote:))((~.+)|([a-zA-Z0-9_.]+))/, 'AppC Message Reception');

AppC.Types.onExpr          = {name: 'On Expression'};
AppC.Types.fieldset        = {name: 'Fieldset'};
AppC.Types.json            = {name: 'JSON Object'};
AppC.Types.javascriptExpr  = {name: 'Javascript Expression'}
AppC.Types.pathOrUrl       = {name: 'Path or url to resource'};
AppC.Types.cssClass        = {name: 'CSS Class name'}; 
AppC.Types.color           = {name: 'Color value'};
AppC.Types.time            = {name: 'Time value'};
AppC.Types.elementId       = {name: 'Element Id'};
AppC.Types.commaSeparated  = {name: "Comma Separated Values"};
AppC.Types.languageId      = {name: "Localization String Id"};
AppC.Types.string          = {name: "Javascript String"};
AppC.Types.object          = {name: "Javascript Object"};

/**
 * these are specific types used by UI controls to indicate special properties
 */
AppC.Types.condition       = {name:'Component Condition'};
AppC.Types.action          = {name:'Component Action'};

/**
 * Checks if a value conforms to some type specification.
 * Can be used for error checking of on expressions,
 * and of the parameters passed to widgets.
 * 
 * This code is primarily for use by the IDE,
 * and tools that want to provide more feedback to the user than:
 * "Compilation Failed!"
 * 
 * @param {Object} value
 * @param {AppC.Type} type
 */
AppC.Types.isInstance = function(value,type)
{
	if(type.regex)
	{
		return type.regex.test(value);
	}
	
	switch(type)
	{
		case AppC.Types.onExpr:
		  try
		  {
		      var thens = App.parseExpression(value);
			  return thens && thens.length > 0;
		  }
		  catch(e)
		  {
		  	   return false;
		  }
		case AppC.Types.time:
		  return value && !isNaN(App.timeFormat(value));
		  
		case AppC.Types.fieldset:
		  // this could check for the names that are currently defined,
		  // but would have of timing issues if used during compilation,
		  // we should at least make a regex that matches only valid identifiers
		  return true;
		  
		default:
		  return true;
	}
	
};

/*
 * Not the name, but what the symbolic identifier used to reference the type from this namespace.
 */
AppC.Types.getTypeId = function(type) 
{
	throw new Error("this method is no longer supported");
};

//--------------------------------------------------------------------------------

/* action.js */

App.selectors = [];
App.delegateCompilers = [];
var actions = {};

function addProcessor(tag,attr,handler,delegate,priority)
{
	var wildcard = tag=='*';
	tag = wildcard ? tag.toLowerCase() : tag;
	var found = actions[tag];
	if (!found)
	{
		found = [];
		actions[tag]=found;
	}
	var isExpr = attr.indexOf('=') > 0;
	found[priority?'unshift':'push']({tag:tag,wildcard:wildcard,attr:attr,expr:isExpr,fn:handler,delegate:delegate});
	var expr = tag + '[' + (!isExpr ? '@' : '') + attr + ']';
	App.selectors[priority?'unshift':'push'](expr);
	if (delegate) App.delegateCompilers[priority?'unshift':'push'](expr);
}

function getTagName(el)
{
	var element = $(el).get(0);
	if (AppC.UA.IE)
	{
		if (element.scopeName && element.tagUrn)
		{
			return (element.scopeName + ':' + element.nodeName).toLowerCase();
		}
	}
	return element.nodeName.toLowerCase();
}

function iterateProcessors(f,el,tag,state)
{
	if (f)
	{
		var e = $(el);
		var delegateCompile = false;
		$.each(f,function()
		{
			if (this.wildcard || tag == this.tag)
			{
				if (!this.expr)
				{
					var v = e.attr(this.attr);
					if (v)
					{
						var r = this.fn.apply(e,[v,state,tag]);
						if (typeof(r)=='undefined') r=true;
						if (r && this.delegate) delegateCompile = true;
					}
				}
				else
				{
					var r = this.fn.apply(e,[tag,state]);
					if (typeof(r)=='undefined') r=true;
					if (r && this.delegate) delegateCompile = true;
				}
			}
			if (delegateCompile) return false;
		});
		return delegateCompile; 
	}
	return false;
}

function getTarget(params,t)
{
	if (!params) return t;
	if (params.target)
	{
		return $(params.target)
	}
	var nt = params.id ? $('#'+params.id) : null;
	return nt && nt.length > 0 ? nt : t;
}

function regCSSAction(name,key,value)
{
	App.regAction(evtRegex(name),function(params)
	{
		if (typeof(key)=='function')
		{
			key.call(getTarget(params,this),params);
		}
		else
		{
			return getTarget(params,this).css(key,value||name);
		}
	});
}

App.runProcessors = function(el,state)
{
	var tag = getTagName(el);
	var r1 = iterateProcessors(actions[tag],el,tag,state);
	var r2 = iterateProcessors(actions['*'],el,tag,state);
	return !(r1 || r2);
};

App.reg = function(name,el,handler,delegateCompile,priority)
{
	if (typeof(el)=='string')
	{
		el = $.makeArray(el);
	}

	$.each(el,function()
	{
		addProcessor(this,name,handler,delegateCompile,priority);
	});
};

var regp = {conds:{},actions:{},ractions:[]};
var dyn  = {conds:{},actions:{}};

App.dynloadAction = function(name,fn)
{
	// register it
	App.regAction(name,fn);

	// call any pending actions
	$.each(dyn.actions[name],function()
	{
		App.invokeAction(this.scope,name,this.params);
	});

	// remove it once loaded
	try { delete dyn.actions[name] } catch (E) { dyn.actions[name]=null; }
};

var actionRE = /\^([\w]+)\(/;
var actionUnparsers = {};
App.parseParams = function(name)
{
	var f = actionUnparsers[name];
	return typeof(f)=='undefined';
};

App.regAction = function(name,fn,dontparse)
{
	dontparse = typeof(dontparse)=='undefined' ? false : (dontparse===true);

	$.debug('adding action ' + name+', dontparse = '+dontparse);
	
	var m = actionRE.exec(String(name));
	if (m && m.length > 1 || typeof(name)=='string')
	{
		var fnName = m.length > 1 ? m[1] : name;
		if (dontparse) actionUnparsers[fnName]=1;
		var f = $.fn[fnName];
		if (typeof(f)!='function')
		{
			//FIXME
			//throw "attempt to register action which doesn't have a registered plugin with same name: "+fnName;
		}
		if (f)
		{
			// remap it
			var mapFnName = '_'+fnName;
			$.fn[mapFnName] = f;
			$.fn[fnName] = function()
			{
				if (!f) return; //FIXME
				var r = f.apply(this,arguments);
				this.trigger(fnName); // trigger an event when action is invoked
				return r || this;
			};
		}
		
		/*
		if (typeof($.fn[fnName])!='function' && /\w/.test(fnName))
		{
			$.debug('registering plugin '+fnName);
			//
			// attempt to create a plugin with the same name of the action
			// so that you can programatically call the same action such
			// as: $('#foo').highlight()
			//
			$.fn[fnName] = function(params)
			{
				var r = fn.call(this,params||{});
				this.trigger(fnName); // trigger an event when action is invoked
				if (typeof(r)!='undefined') return r;
				return this;
			};
		}*/
	}
	regp.ractions.push({re:name,fn:fn,dontparse:dontparse});
};

App.makeCustomAction=function(el,value)
{
	var p = App.extractParameters(value);
	var meta = 
	{
		action: p.name,
		actionParams: p.params,
		delay:0,
		ifCond:null
	};
	actions[meta.action]=meta;
	return meta;
};

App.invokeAction=function(name,data,params)
{
	$.debug("invokeAction called with "+name);
	var scope = this;
	var found = false;
	$.each(regp.ractions,function()
	{
		if (this.re.test(name))
		{
			found = true;

			var newparams = params || {};
			if (data && !this.dontparse)
			{
				for (var x=0;x<data.length;x++)
				{
					var entry = data[x];
					var key = entry.key, value = entry.value;
					if (entry.keyExpression)
					{
						key = App.getEvaluatedValue(entry.key,null,params,entry.keyExpression);
					}
					else if (entry.valueExpression)
					{
						value = App.getEvaluatedValue(entry.value,null,params,entry.valueExpression);
					}
					else if (entry.empty)
					{
						value = App.getEvaluatedValue(entry.key,null,params);
					}
					else
					{
						key = App.getEvaluatedValue(entry.key);
						value = App.getEvaluatedValue(entry.value,null,params);
					}
					newparams[key]=value;
				}		
			}
			else if (data && this.donparse)
			{
				newparams = params;
			}
			$.debug('invoking action: '+name+' with: '+$.toJSON(newparams)+", scope="+$(scope).attr('id'));
			this.fn.apply(scope,[newparams,name,data]);
			return false;
		}
	});
	if (!found)
	{
		var fn = typeof(name)=='function' ? name : $(this)[name];
		if (fn)
		{
			var newparams = params || {};
			if (data)
			{
				for (var x=0;x<data.length;x++)
				{
					var entry = data[x];
					var key = entry.key, value = entry.value;
					if (entry.keyExpression)
					{
						key = App.getEvaluatedValue(entry.key,null,params,entry.keyExpression);
					}
					else if (entry.valueExpression)
					{
						value = App.getEvaluatedValue(entry.value,null,params,entry.valueExpression);
					}
					else if (entry.empty)
					{
						value = App.getEvaluatedValue(entry.key,null,params);
					}
					else
					{
						key = App.getEvaluatedValue(entry.key);
						value = App.getEvaluatedValue(entry.value,null,params);
					}
					newparams[key]=value;
				}		
			}
			$.debug('invoking action: '+name+' with: '+$.toJSON(newparams)+", scope="+$(scope).attr('id'));
			fn.apply(scope,[newparams,name,params]);
		}
		else
		{
			$.error("couldn't find an action named: "+name+" for target: "+$(this).attr('id'));
		}
	}
};

App.triggerElseAction = function(scope,params,meta)
{
	App.triggerAction(scope,params,
	{
		action: meta.elseAction,
		actionParams: meta.elseActionParams,
		delay:0
	});
};
App.triggerAction = function(scope,params,meta)
{
	var data = meta.actionParams;
	var action = meta.action;
	if (meta.ifCond)
	{
		var r = eval(meta.ifCond);
		if (r)
		{
			if (typeof(r)=='boolean')
			{
				r = (r===true);
			}
		}
		if (!r)
		{	
			action = meta.elseAction;
			data = meta.elseActionParams;
		}
	}
	if (action)
	{
		if (meta.delay > 0)
		{
			$(scope).delay(function(){ App.invokeAction.apply(scope,[action,data,params]) },meta.delay/1000);
		}
		else
		{
			App.invokeAction.apply(scope,[action,data,params]);
		}
	}
};

App.dynregAction = function(actions)
{
	$.each($.makeArray(actions),function()
	{
		var name = $.string(this);
		var path = AppC.pluginRoot + '/' + name + '_action.js';
		var found = regp.actions[path];
		if (found)
		{
			return App.regAction(name,found);
		}
		App.regAction(evtRegex(name),function(params)
		{
			var c = dyn.actions[name];
			var scope = getTarget(params,this);
			if (c)
			{
				// already pending
				return c.push({scope:scope,params:params});
			}

			dyn.actions[name]=[{scope:scope,params:params}];

			$.debug('remote loading action = '+path);
			$.getScript(path);
		});
	});
};


function convertParams(params)
{
	if (arguments.length == 2 && typeof(params)=='string')
	{
		// allow $('div').set('background-color','blue');
		var key = arguments[0];
		var value = arguments[1];
		params = {
			key: value
		};
	}
	return params;
}

//--------------------------------------------------------------------------------

/* parser.js */

var parameterSeparatorRE = /[\$=:><!]+/;
var parameterRE = /(.*?)\[(.*)?\]/i;
var expressionRE = /^expr\((.*?)\)$/;

var numberRe = /^[-+]{0,1}[0-9]+$/;
var floatRe = /^[0-9]*[\.][0-9]*[f]{0,1}$/;
var booleanRe = /^(true|false)$/;
var quotedRe =/^['"]{1}|['"]{1}$/;
var jsonRe = /^\{(.*)?\}$/;

var STATE_LOOKING_FOR_VARIABLE_BEGIN = 0;
var STATE_LOOKING_FOR_VARIABLE_END = 1;
var STATE_LOOKING_FOR_VARIABLE_VALUE_MARKER = 2;
var STATE_LOOKING_FOR_VALUE_BEGIN = 3;
var STATE_LOOKING_FOR_VALUE_END = 4;
var STATE_LOOKING_FOR_VALUE_AS_JSON_END = 5;

function dequote (value)
{
	if (value && typeof value == 'string')
	{
		if (value.charAt(0)=="'" || value.charAt(0)=='"')
		{
			value = value.substring(1);
		}
		if (value.charAt(value.length-1)=="'" || value.charAt(value.length-1)=='"')
		{
			value = value.substring(0,value.length-1);
		}
	}
	return value;
}

function convertInt (value)
{
	if (value.charAt(0)=='0')
	{
		if (value.length==1)
		{
			return 0;
		}
		return convertInt(value.substring(1));
	}
	return parseInt(value);
}

function decodeParameterValue (token,wasquoted)
{
	var value = null;
	if (token!=null && token.length > 0 && !wasquoted)
	{
		var match = jsonRe.exec(token);
		if (match)
		{
			value = $.evalJSON(match[0]);
		}
		if (!value)
		{
			var quoted = quotedRe.test(token);
			if (quoted)
			{
				value = dequote(token);
			}
			else if (floatRe.test(token))
			{
				value = parseFloat(token);
			}
			else if (numberRe.test(token))
			{
				value = convertInt(token);
			}
			else if (booleanRe.test(token))
			{
				value = (token == 'true');
			}
			else
			{
				value = token;
			}
		}
	}
	if (token == 'null' || value == 'null')
	{
		return null;
	}
	return value == null ? token : value;
}

App.getTagname = function (element)
{
	if (!element) throw "element cannot be null";
	if (element.jquery) element=$(element).get(0);
	if (element.nodeType!=1) throw "node: "+element.nodeName+" is not an element, was nodeType: "+element.nodeType+", type="+(typeof element);

	//FIXME: review this
	// used by the compiler to mask a tag
	if (element._tagName) return element._tagName;

	if ($.browser.msie)
	{
		if (element.scopeName && element.tagUrn)
		{
			return element.scopeName + ':' + element.nodeName.toLowerCase();
		}
	}
	return String(element.nodeName.toLowerCase());
}

function formatValue (value,quote)
{
	quote = (quote == null) ? true : quote;

	if (value!=null)
	{
		var type = typeof(value);
		if (type == 'boolean' || type == 'array' || type == 'object')
		{
			return value;
		}
		if (value == 'true' || value == 'false')
		{
			return value == 'true';
		}
		if (value.charAt(0)=="'" && quote)
		{
			return value;
		}
		if (value.charAt(0)=='"')
		{
			value = value.substring(1,value.length-1);
		}
		if (quote)
		{
			return "'" + value + "'";
		}
		return value;
	}
	return '';
}

function getInputFieldValue (elem,dequote,local)
{
	elem = $(elem);
	
	var tagname = App.getTagname(elem);
	if (tagname != 'input' && tagname != 'textarea' && tagname != 'select')
	{
		return null;
	}

	local = local==null ? true : local;
	dequote = (dequote==null) ? false : dequote;

	var type = elem.attr('type') || 'text';
	var v = elem.val();

	switch(type)
	{
		case 'checkbox':
			return (v == 'on' || v == 'checked');
	}
	return formatValue(v,!dequote);
}

function getElementValue (element, dequote, local)
{
    var el = typeof(element)=='string' ? $('#'+element) : $(element);
    dequote = (dequote==null) ? true : dequote;
	var elem = el.get(0);

	if (el.is(':input'))
	{
		return el.val();
	}
	else if (el.is('form'))
	{
		var obj = {};
		$.each(el.find(":input").filter(function(){
			return !this.disabled &&
				(this.checked || /select|textarea/i.test(this.nodeName) ||
					/text|hidden|password/i.test(this.type));
		})
		.map(function(i, elem){
			var val = $(this).val();
			return val == null ? null :
				val.constructor == Array ?
					$.map( val, function(val, i){
						return {name: elem.name || elem.id, value: val};
					}) :
					{name: elem.name || elem.id, value: val};
		}).get(),function()
		{
			obj[this.name]=this.value;
		});
		return obj;
	}
	else if (el.is('img,iframe'))
	{
		return elem.src;
	}

    // allow the element to set the value otherwise use the
    // innerHTML of the component
    if (elem.value != undefined)
    {
        return elem.value;
    }
    return elem.html();
}

App.getEvaluatedValue = function (v,data,scope,isExpression)
{
	if (v && typeof(v) == 'string')
	{
		if (!isExpression && v.charAt(0)=='$')
		{
			var varName = v.substring(1);
			var elem = $('#'+varName);
			if (elem)
			{
				// dynamically substitute the value
				return getElementValue(elem,true);
			}
		}
        else if(!isExpression && !isNaN(parseFloat(v)))
        {
            //Assume that if they provided a number, they want the number back
            //this is important because in IE window[1] returns the first iframe
            return v;
        }
		else
		{
			// determine if this is a dynamic javascript
			// expression that needs to be executed on-the-fly
			var match = isExpression || expressionRE.exec(v);
			if (match)
			{
				var expr = isExpression ? v : match[1];
				var func = $.toFunction(expr);
				var s = scope ? scope : {};
				if (data)
				{
					for (var k in data)
					{
						if (typeof(k)  == 'string')
						{
							s[k] = data[k];
						}
					}
				}
				return func.call(s);
			}

			if (scope)
			{
				var result = $.getNestedProperty(scope,v,null);
				if (result)
				{
					return result;
				}
			}

			if (data)
			{
				return $.getNestedProperty(data,v,v);
			}
		}
	}
	return v;
}

App.extractParameters = function(value,scope)
{
	var idx = value.indexOf('[');
	if (idx != -1)
	{
		var endidx = value.lastIndexOf(']');
		var p = value.substring(idx+1,endidx);
		var action = value.substring(0,idx);
		var canParse = App.parseParams(action); 
		var params = null;
		if (canParse)
		{
			params = App.getParameters(p,false);
			if (params && scope)
			{
				var newparams = {};
				for (var x=0;x<params.length;x++)
				{
					var entry = params[x];
					var key = entry.key, value = entry.value;
					if (entry.keyExpression)
					{
						key = App.getEvaluatedValue(entry.key,null,scope,entry.keyExpression);
					}
					else if (entry.valueExpression)
					{
						value = App.getEvaluatedValue(entry.value,null,scope,entry.valueExpression);
					}
					else if (entry.empty)
					{
						value = App.getEvaluatedValue(entry.key,null,scope);
					}
					else
					{
						key = App.getEvaluatedValue(entry.key);
						value = App.getEvaluatedValue(entry.value,null,scope);
					}
					newparams[key]=value;
				}
				params = newparams;
			}		
		}
		return {name:action,params:params||p};
	}
	return {name:value,params:null};	
};

/**
 * method will parse out a loosely typed json like structure
 * into either an array of json objects or a json object
 *
 * @param {string} string of parameters to parse
 * @param {boolean} asjson return it as json object
 * @return {object} value
 */
App.getParameters = function(str,asjson)
{
	if (str==null || str.length == 0)
	{
		return asjson ? {} : [];
	}

	var exprRE = /expr\((.*?)\)/;
	var containsExpr = exprRE.test(str);

	// this is just a simple optimization to 
	// check and make sure we have at least a key/value
	// separator character before we continue with this
	// inefficient parser
	if (!parameterSeparatorRE.test(str) && !containsExpr)
	{
		if (asjson)
		{
			var valueless_key = {};
			valueless_key[str] = '';
			return valueless_key;
		}
		else
		{
			return [{key:str,value:'',empty:true}];
		}
	}
	var state = 0;
	var currentstr = '';
	var key = null;
	var data = asjson ? {} : [];
	var quotedStart = false, tickStart = false;
	var operator = null;
	var expressions = containsExpr ? {} : null;
	if (containsExpr)
	{
		var expressionExtractor = function(e)
		{
			var start = e.indexOf('expr(');
			if (start < 0) return null;
			var p = start + 5;
			var end = e.length-1;
			var value = '';
			while ( true )
			{
				var idx = e.indexOf(')',p);
				if (idx < 0) break;
				value+=e.substring(p,idx);
				if (idx == e.length-1)
				{
					end = idx+1;
					break;
				}
				var b = false;
				var x = idx + 1;
				for (;x<e.length;x++)
				{
					switch(e.charAt(x))
					{
						case ',':
						{
							end = x;
							b = true;
							break;
						}
						case ' ':
						{
							break;
						}
						default:
						{
							p = idx+1;
							break;
						}
					}
				}
				if (x==e.length-1)
				{
					end = x;
					break;
				}
				if (b) break;
				value+=')';
			}
			var fullexpr = e.substring(start,end);
			return [fullexpr,value];
		};

		var ec = 0;
		while(true)
		{
			var m = expressionExtractor(str);
			if (!m)
			{
				break;
			}
			var k = '__E__'+(ec++);
			expressions[k] = m[1];
			str = str.replace(m[0],k);
		}
	}

	function transformValue(k,v,tick)
	{
		if (k && $.startsWith(k,'__E__'))
		{
			if (!asjson)
			{
				return {key:expressions[k],value:v,keyExpression:true,valueExpression:false};
			}
			else
			{
				return eval(expressions[k]);
			}
		}
		if (v && $.startsWith(v,'__E__'))
		{
			if (!asjson)
			{
				return {key:k,value:expressions[v],valueExpression:true,keyExpression:false};
			}
			else
			{
				return eval(expressions[v]);
			}
		}
		var s = decodeParameterValue(v,tick);
		if (!asjson)
		{
			return {key:k,value:s};
		}
		return s;
	}

	for (var c=0,len=str.length;c<len;c++)
	{
		var ch = str.charAt(c);
		var append = true;

		switch (ch)
		{
			case '"':
			case "'":
			{
				switch (state)
				{
					case STATE_LOOKING_FOR_VARIABLE_BEGIN:
					{
						quoted = true;
						append = false;
						state = STATE_LOOKING_FOR_VARIABLE_END;
						quotedStart = ch == '"';
						tickStart = ch=="'";
						break;
					}
					case STATE_LOOKING_FOR_VARIABLE_END:
					{
						var previous = str.charAt(c-1);
						if (quotedStart && ch=="'" || tickStart && ch=='"')
						{
							// these are OK inline
						}
						else if (previous != '\\')
						{
							state = STATE_LOOKING_FOR_VARIABLE_VALUE_MARKER;
							append = false;
							key = $.trim(currentstr);
							currentstr = '';
						}
						break;
					}
					case STATE_LOOKING_FOR_VALUE_BEGIN:
					{
						append = false;
						quotedStart = ch == '"';
						tickStart = ch=="'";
						state = STATE_LOOKING_FOR_VALUE_END;
						break;
					}
					case STATE_LOOKING_FOR_VALUE_END:
					{
						var previous = str.charAt(c-1);
						if (quotedStart && ch=="'" || tickStart && ch=='"')
						{
							// these are OK inline
						}
						else if (previous != '\\')
						{
							state = STATE_LOOKING_FOR_VARIABLE_BEGIN;
							append = false;
							if (asjson)
							{
								data[key]=transformValue(key,currentstr,quotedStart||tickStart);
							}
							else
							{
								data.push(transformValue(key,currentstr,quotedStart||tickStart));
							}
							key = null;
							quotedStart = false, tickStart = false;
							currentstr = '';
						}
						break;
					}
				}
				break;
			}
			case '>':
			case '<':
			case '=':
			case ':':
			{
				if (state == STATE_LOOKING_FOR_VARIABLE_END)
				{
					if (ch == '<' || ch == '>')
					{
						key = $.trim(currentstr);
						currentstr = '';
						state = STATE_LOOKING_FOR_VARIABLE_VALUE_MARKER;
					}
				}
				switch (state)
				{
					case STATE_LOOKING_FOR_VARIABLE_END:
					{
						append = false;
						state = STATE_LOOKING_FOR_VALUE_BEGIN;
						key = $.trim(currentstr);
						currentstr = '';
						operator = ch;
						break;
					}
					case STATE_LOOKING_FOR_VARIABLE_VALUE_MARKER:
					{
						append = false;
						state = STATE_LOOKING_FOR_VALUE_BEGIN;
						operator = ch;
						break;
					}
				}
				break;
			}
			case ',':
			{
				switch (state)
				{
					case STATE_LOOKING_FOR_VARIABLE_BEGIN:
					{
						append = false;
						state = STATE_LOOKING_FOR_VARIABLE_BEGIN;
						break;
					}
					case STATE_LOOKING_FOR_VARIABLE_END:
					{
						// we got to the end (single parameter with no value)
						state=STATE_LOOKING_FOR_VARIABLE_BEGIN;
						append=false;
						if (asjson)
						{
							data[currentstr]=null;
						}
						else
						{
							var entry = transformValue(key,currentstr);
							entry.operator = operator;
							entry.key = entry.value;
							entry.empty = true;
							data.push(entry);
						}
						key = null;
						quotedStart = false, tickStart = false;
						currentstr = '';
						break;
					}
					case STATE_LOOKING_FOR_VALUE_END:
					{
						if (!quotedStart && !tickStart)
						{
							state = STATE_LOOKING_FOR_VARIABLE_BEGIN;
							append = false;
							if (asjson)
							{
								data[key]=transformValue(key,currentstr,quotedStart||tickStart);
							}
							else
							{
								var entry = transformValue(key,currentstr);
								entry.operator = operator;
								data.push(entry);
							}
							key = null;
							quotedStart = false, tickStart = false;
							currentstr = '';
						}
						break;
					}
				}
				break;
			}
			case ' ':
			{
			    break;
			}
			case '\n':
			case '\t':
			case '\r':
			{
				append = false;
				break;
			}
			case '{':
			{
				switch (state)
				{
					case STATE_LOOKING_FOR_VALUE_BEGIN:
					{
						state = STATE_LOOKING_FOR_VALUE_AS_JSON_END;
					}
				}
				break;
			}
			case '}':
			{
				if (state == STATE_LOOKING_FOR_VALUE_AS_JSON_END)
				{
					state = STATE_LOOKING_FOR_VARIABLE_BEGIN;
					append = false;
					currentstr+='}';
					if (asjson)
					{
						data[key]=transformValue(key,currentstr,quotedStart||tickStart);
					}
					else
					{
						var entry = transformValue(key,currentstr);
						entry.operator = operator;
						data.push(entry);
					}
					key = null;
					quotedStart = false, tickStart = false;
					currentstr = '';
				}
				break;
			}
			default:
			{
				switch (state)
				{
					case STATE_LOOKING_FOR_VARIABLE_BEGIN:
					{
						state = STATE_LOOKING_FOR_VARIABLE_END;
						break;
					}
					case STATE_LOOKING_FOR_VALUE_BEGIN:
					{
						state = STATE_LOOKING_FOR_VALUE_END;
						break;
					}
				}
			}
		}
		if (append)
		{
			currentstr+=ch;
		}
		if (c + 1 == len && key)
		{
			//at the end
			currentstr = $.trim(currentstr);
			if (asjson)
			{
				data[key]=transformValue(key,currentstr,quotedStart||tickStart);
			}
			else
			{
				var entry = transformValue(key,currentstr);
				entry.operator = operator;
				data.push(entry);
			}
		}
	}

	if (currentstr && !key)
	{
		if (asjson)
		{
			data[key]=null;
		}
		else
		{
			var entry = transformValue(key,currentstr);
			entry.empty = true;
			entry.key = entry.value;
			entry.operator = operator;
			data.push(entry);
		}
	}
	return data;
};

function isIDRef(value)
{
	if (value)
	{
		if (typeof(value) == 'string')
		{
			return value.charAt(0)=='$';
		}
	}
	return false;
}

App.parseConditionCondition = function(actionParams,data) 
{
    var ok = true;

    if (actionParams)
    {
    	for (var c=0,len=actionParams.length;c<len;c++)
    	{
    		var p = actionParams[c];
			var negate = false, regex = false;
			if (p.empty && p.value)
			{
				// swap these out
				p.key = p.value;
				p.keyExpression = p.valueExpression;
				p.value = null;
			}
			var lhs = p.key, rhs = p.value, operator = p.operator||'';
			if (p.key && p.key.charAt(0)=='!')
			{
				negate = true;
				lhs = p.key.substring(1);
			}
			else if (p.key && p.key.charAt(p.key.length-1)=='!')
			{
				negate = true;
				lhs = p.key.substring(0,p.key.length-1);
			}
			var preLHS = lhs;
			if (p.keyExpression || isIDRef(lhs))
			{
				var out = App.getEvaluatedValue(lhs,data,data,p.keyExpression);
				if (!p.keyExpression && isIDRef(lhs) && lhs == out)
				{
					lhs = null;
				}
				else
				{
					lhs = out;
				}
			}
			else
			{
				lhs = App.getEvaluatedValue(lhs,data);
			}
			if (lhs == preLHS)
			{
				// left hand side must evaluate to a value -- if we get here and it's the same, that 
				// means we didn't find it
				lhs = null;
			}
			// mathematics
			if ((operator == '<' || operator == '>') && (rhs && typeof(rhs)=='string' && rhs.charAt(0)=='='))
			{
				operator += '=';
				rhs = rhs.substring(1);
			}
			if (rhs && typeof(rhs)=='string' && rhs.charAt(0)=='~')
			{
				regex = true;
				rhs = rhs.substring(1);
			}
			if (p.empty)
			{
				rhs = lhs;
			}
			else if (p.keyExpression || isIDRef(rhs))
			{
				var out = App.getEvaluatedValue(rhs,data,data,p.valueExpression);
				if (!p.valueExpression && isIDRef(rhs) && rhs == out)
				{
					rhs = null;
				}
				else
				{
					rhs = out;
				}
			}
			else
			{
				rhs = App.getEvaluatedValue(rhs,data);
			}
			if (regex)
			{
				var r = new RegExp(rhs);
				ok = r.test(lhs);
			}
			else if (!operator && p.empty && rhs == null)
			{
				ok = lhs;
			}
			else
			{
				switch(operator||'=')
				{
					case '<':
					{
						ok = parseInt(lhs) < parseInt(rhs);
						break;
					}
					case '>':
					{
						ok = parseInt(lhs) > parseInt(rhs);
						break;
					}
					case '<=':
					{
						ok = parseInt(lhs) <= parseInt(rhs);
						break;
					}
					case '>=':
					{
						ok = parseInt(lhs) >= parseInt(rhs);
						break;
					}
					default:
					{
						ok = String(lhs) == String(rhs);
						break;
					}
				}
			}
			if (negate)
			{
				ok = !ok;
			}
			if (!ok)
			{
				break;
			}
		}
	}
	return ok;
};

App.getJsonTemplateVar=function(namespace,var_expr,template_var)
{
    var def = {};
    var o = $.getNestedProperty(namespace,var_expr,def);

    if (o == def) // wasn't found in template context
    {
        try
        {
            with(namespace) { o = eval(var_expr) };
        }
        catch (e) // couldn't be evaluated either
        {
            return template_var; // maybe a nested template replacement will catch it
        }
    }
    
    if (typeof(o) == 'object')
    {
        o = $.toJSON(o).replace(/"/g,'&quot;');
    }
    return o;
};

var templateRE = /#\{(.*?)\}/g;
AppC.compileTemplate = function(html,htmlonly,varname,re)
{
	html = html || '';
	varname = varname==null ? 'f' : varname;
	re = re || templateRE;

	var fn = function(m, name, format, args)
	{
		return "', jtv(values,'"+name+"','#{"+name+"}'),'";
	};
	var body = "var "+varname+" = function(values){ var jtv = App.getJsonTemplateVar; return ['" +
            html.replace(/(\r\n|\n)/g, '').replace(/\t/g,' ').replace(/'/g, "\\'").replace(re, fn) +
            "'].join('');};" + (htmlonly?'':varname);

	var result = htmlonly ? body : eval(body);

	return result;
};

App.componentData = {};

App.getData = function(id,key)
{
	var d = App.componentData[id];
	if (!d) return null;
	if (!key)
	{
		return d;
	}
	return d[key];
};

App.setData = function(id,data)
{
	var d = App.componentData[id];
	if (!d)
	{
		App.componentData[id]=data;
	}
	else
	{
		for (var p in data)
		{
			d[p]=data[p];
		}
	}
};

var conds = [];

App.regCond = function(re,fn)
{
	conds.push({re:re,fn:fn});
};  

App.processCond = function(el,info)
{
	var f = false;
	$.each(conds,function()
	{
		if (this.re.test(info.cond))
		{
			f = true;
			this.fn.call(el,info);
			return false;
		}
	});
	if (!f)
	{
		$.error('not match for cond = '+info.cond+' for element with id: '+$(el).attr('id'));
	}
};

function smartTokenSearch(searchString, value)
{
	var validx = -1;
	if (searchString.indexOf('[') > -1 && searchString.indexOf(']')> -1)
	{
		var possibleValuePosition = searchString.indexOf(value);
		if (possibleValuePosition > -1)
		{
			var in_left_bracket = false;
			for (var i = possibleValuePosition; i > -1; i--)
			{
				if (searchString.charAt(i) == ']')
				{
					break;
				}
				if (searchString.charAt(i) == '[')
				{
					in_left_bracket = true;
					break;
				}
			}
			var in_right_bracket = false;
			for (var i = possibleValuePosition; i < searchString.length; i++)
			{
				if (searchString.charAt(i) == '[')
				{
					break;
				}
				if (searchString.charAt(i) == ']')
				{
					in_right_bracket = true;
					break;
				}
			}

			if (in_left_bracket && in_right_bracket)
			{
				validx = -1;
			} else
			{
				validx = searchString.indexOf(value);
			}
		} else validx = possibleValuePosition;
	}
	else
	{
		validx = searchString.indexOf(value);
	}
	return validx;
};


var compoundCondRE = /^\((.*)?\) then$/;

App.parseExpression = function(value,element)
{
	if (!value)
	{
		return [];
	}

	if (typeof(value)!='string')
	{
		alert('framework error: value was '+value+' -- unexpected type: '+typeof(value));
	    throw "value: "+value+" is not a string!";
	}
	value = $.gsub(value,'\n',' ');
	value = $.gsub(value,'\r',' ');
	value = $.gsub(value,'\t',' ');
	value = $.trim(value);

	var thens = [];
	var ors = $.smartSplit(value,' or ');

	for (var c=0,len=ors.length;c<len;c++)
	{
		var expression = $.trim(ors[c]);
		var thenidx = expression.indexOf(' then ');
		if (thenidx <= 0)
		{
			//FIXME
			// we allow widgets to have a short-hand syntax for execute
			// if (Appcelerator.Compiler.getTagname(element).indexOf(':'))
			// {
			// 	expression = expression + ' then execute';
			// 	thenidx = expression.indexOf(' then ');
			// }
			// else
			// {
			// 	throw "syntax error: expected 'then' for expression: "+expression;
			// }
			throw "syntax error: expected 'then' for expression: "+expression;
		}
		var condition = expression.substring(0,thenidx);

		// check to see if we have compound conditions - APPSDK-597
		var testExpr = expression.substring(0,thenidx+5);
		var condMatch = compoundCondRE.exec(testExpr);
		if (condMatch)
		{
			var expressions = condMatch[1];
			// turn it into an array of conditions
			condition = $.smartSplit(expressions,' or ');
		}

		var elseAction = null;
		var nextstr = expression.substring(thenidx+6);
		var elseidx = smartTokenSearch(nextstr, 'else');

		var increment = 5;
		if (elseidx == -1)
		{
			elseidx = nextstr.indexOf('otherwise');
			increment = 10;
		}
		var action = null;
		if (elseidx > 0)
		{
			action = nextstr.substring(0,elseidx-1);
			elseAction = nextstr.substring(elseidx + increment);
		}
		else
		{
			action = nextstr;
		}

		var nextStr = elseAction || action;
		var ifCond = null;
		var ifIdx = nextStr.indexOf(' if expr[');

		if (ifIdx!=-1)
		{
			var ifStr = nextStr.substring(ifIdx + 9);
			var endP = ifStr.indexOf(']');
			if (endP==-1)
			{
				throw "error in if expression, missing end parenthesis at: "+action;
			}
			ifCond = ifStr.substring(0,endP);
			if (elseAction)
			{
				elseAction = nextStr.substring(0,ifIdx);
			}
			else
			{
				action = nextStr.substring(0,ifIdx);
			}
			nextStr = ifStr.substring(endP+2);
		}

		var delay = 0;
		var afterIdx =  smartTokenSearch(nextstr, 'after ');

		if (afterIdx!=-1)
		{
			var afterStr = nextstr.substring(afterIdx+6);
			delay = App.timeFormat(afterStr);
			if (!ifCond)
			{
				if (elseAction)
				{
					elseAction = nextStr.substring(0,afterIdx-1);
				}
				else
				{
					action = nextStr.substring(0,afterIdx-1);
				}
			}
		}

		thens.push([null,condition,action,elseAction,delay,ifCond]);
	}
	return thens;
};

var ONE_SECOND = 1000;
var ONE_MINUTE = 60000;
var ONE_HOUR = 3600000;
var ONE_DAY = 86400000;
var ONE_WEEK = 604800000;
var ONE_MONTH = 18748800000; // this is rough an assumes 31 days
var ONE_YEAR = 31536000000;

App.timeFormat = function(value)
{
	if (typeof(value)=='number') return value;
	var str = '';
	var time = 0;

	for (var c=0,len=value.length;c<len;c++)
	{
		var ch = value.charAt(c);
		switch (ch)
		{
			case ',':
			case ' ':
			{
				str = '';
				break;
			}
			case 'm':
			{
				if (c + 1 < len)
				{
					var nextch = value.charAt(c+1);
					if (nextch == 's')
					{
						time+=parseInt(str);
						c++;
					}
				}
				else
				{
					time+=parseInt(str) * ONE_MINUTE;
				}
				str = '';
				break;
			}
			case 's':
			{
				time+=parseInt(str) * ONE_SECOND;
				str = '';
				break;
			}
			case 'h':
			{
				time+=parseInt(str) * ONE_HOUR;
				str = '';
				break;
			}
			case 'd':
			{
				time+=parseInt(str) * ONE_DAY;
				str = '';
				break;
			}
			case 'w':
			{
				time+=parseInt(str) * ONE_WEEK;
				str = '';
				break;
			}
			case 'y':
			{
				time+=parseInt(str) * ONE_YEAR;
				str = '';
				break;
			}
			default:
			{
				str+=ch;
				break;
			}
		}
	}

	if (str.length > 0)
	{
		time+=parseInt(str);
	}

	return time;
};

//--------------------------------------------------------------------------------

/* event.js */

var events = ['blur','click','change','dblclick','focus','keydown','keyup','keypress','mousedown','mousemove','mouseout', 'mouseover', 'mouseup', 'resize', 'scroll','select','submit'];

function evtRegex(name)
{
	return new RegExp('^'+name+'(\\[(.*)?\\])?$');
}

App.regCond(new RegExp('^('+events.join('|')+')[!]?$'),function(meta)
{
	var stop = false;
	var cond = meta.cond;
	if (cond.charAt(cond.length-1)=='!')
	{
		cond = cond.substring(0,cond.length-1);
		stop = true;
	}
	var fn = function(e)
	{
		var scope = $(this);
		var data = App.getFieldsetData(scope);
		data.event = {id:$(this).attr('id'),x:e.pageX,y:e.pageY};
		$.debug('sending '+cond+', data = '+$.toJSON(data));
		App.triggerAction(scope,data,meta);
		if (stop)
		{
			e.stopPropagation();
			return false;
		}
	};
	this.bind(cond,fn);
	this.trash(function()
	{
		this.unbind(cond,fn);
	});
});

App.regCond(/^compiled$/,function(meta)
{
	var fn = function()
	{
		var scope = $(this);
		var data = {event:{id:$(this).attr('id')}};
		App.triggerAction(scope,data,meta);
	};
	this.bind('compiled',fn);
	this.trash(function()
	{
		this.unbind('compiled',fn);
	});
});

//--------------------------------------------------------------------------------

/* cookie.js */

/*!
 * Cookie plugin
 *
 * Copyright (c) 2006 Klaus Hartl (stilbuero.de)
 * Dual licensed under the MIT and GPL licenses:
 * http://www.opensource.org/licenses/mit-license.php
 * http://www.gnu.org/licenses/gpl.html
 *
 */

/* Slide modification by Jeff Haynie to make it compatible as an action */

/**
 * Create a cookie with the given name and value and other optional parameters.
 *
 * @example $.cookie('the_cookie', 'the_value');
 * @desc Set the value of a cookie.
 * @example $.cookie('the_cookie', 'the_value', { expires: 7, path: '/', domain: 'jquery.com', secure: true });
 * @desc Create a cookie with all available options.
 * @example $.cookie('the_cookie', 'the_value');
 * @desc Create a session cookie.
 * @example $.cookie('the_cookie', null);
 * @desc Delete a cookie by passing null as value. Keep in mind that you have to use the same path and domain
 *       used when the cookie was set.
 *
 * @param String name The name of the cookie.
 * @param String value The value of the cookie.
 * @param Object options An object literal containing key/value pairs to provide optional cookie attributes.
 * @option Number|Date expires Either an integer specifying the expiration date from now on in days or a Date object.
 *                             If a negative value is specified (e.g. a date in the past), the cookie will be deleted.
 *                             If set to null or omitted, the cookie will be a session cookie and will not be retained
 *                             when the the browser exits.
 * @option String path The value of the path atribute of the cookie (default: path of page that created the cookie).
 * @option String domain The value of the domain attribute of the cookie (default: domain of page that created the cookie).
 * @option Boolean secure If true, the secure attribute of the cookie will be set and the cookie transmission will
 *                        require a secure protocol (like HTTPS).
 * @type undefined
 *
 * @name $.cookie
 * @cat Plugins/Cookie
 * @author Klaus Hartl/klaus.hartl@stilbuero.de
 */

/**
 * Get the value of a cookie with the given name.
 *
 * @example $.cookie('the_cookie');
 * @desc Get the value of a cookie.
 *
 * @param String name The name of the cookie.
 * @return The value of the cookie.
 * @type String
 *
 * @name $.cookie
 * @cat Plugins/Cookie
 * @author Klaus Hartl/klaus.hartl@stilbuero.de
 */

$.cookie = function(name, value, options) {
	
    if (typeof value != 'undefined') { // name and value given, set cookie
        options = options || {};
        if (value === null) {
            value = '';
            options.expires = -1;
        }
        var expires = '';
        if (options.expires && (typeof options.expires == 'number' || options.expires.toUTCString)) {
            var date;
            if (typeof options.expires == 'number') {
                date = new Date();
                date.setTime(date.getTime() + (options.expires * 24 * 60 * 60 * 1000));
            } else {
                date = options.expires;
            }
            expires = '; expires=' + date.toUTCString(); // use expires attribute, max-age is not supported by IE
        }
        // CAUTION: Needed to parenthesize options.path and options.domain
        // in the following expressions, otherwise they evaluate to undefined
        // in the packed version for some reason...
        var path = options.path ? '; path=' + (options.path) : '';
        var domain = options.domain ? '; domain=' + (options.domain) : '';
        var secure = options.secure ? '; secure' : '';
        document.cookie = [name, '=', encodeURIComponent(value), expires, path, domain, secure].join('');
    } else { // only name given, get cookie
        var cookieValue = null;
        if (document.cookie && document.cookie != '') {
            var cookies = document.cookie.split(';');
            for (var i = 0; i < cookies.length; i++) {
                var cookie = jQuery.trim(cookies[i]);
                // Does this cookie string begin with the name we want?
                if (cookie.substring(0, name.length + 1) == (name + '=')) {
                    cookieValue = decodeURIComponent(cookie.substring(name.length + 1));
                    break;
                }
            }
        }
        return cookieValue;
    }
};

// alias it
$.fn.cookie = $.cookie;

//--------------------------------------------------------------------------------

/* trash.js */

$.fn.trash = function(fn)
{
	var trash = this.data('trash');
	if (!trash)
	{
		trash = [];
		this.data('trash',trash);
	}
	if (arguments.length == 0 || typeof(fn)=='undefined')
	{
		return trash;
	}
	trash.push(fn);
	return this;
};

$.fn.destroy = function()
{
	var scope = $(this);
	if (!scope.attr('id')) return this; // we always add id, ignore if we don't have one
	$.each(this,function()
	{
		var el = $(this);
		var trash = el.trash();
		if (trash && trash.length > 0)
		{
			$.each(trash,function()
			{
				try { this.call(scope) } catch (E) { } 
			});
			el.trigger('destroyed');
		}
		el.removeData('trash');
	});
	return this;
};

var oldEmpty = $.fn.empty;

// remap to make sure we destroy
$.fn.empty = function()
{
	var el = $(this).get(0);
	if (el)
	{
		var set = getTargetCompileSet(el,true);
		$.each(set,function()
		{
			$(this).destroy();
		});
	}
	return oldEmpty.apply(this,arguments);
};

var oldRemove = $.fn.remove;

// // remap to make sure we destroy
// $.fn.remove = function()
// {
// 	$.each(this,function()
// 	{
// 		var scope = $(this);
// 		scope.destroy();
// 		oldRemove.call(scope);
// 	});
// 	return this;
// };


//--------------------------------------------------------------------------------

/* img.js */

App.reg('srcexpr','img',function(params)
{	
	var value = null;
	for (var p in params)
	{
		switch(p)
		{
			case 'id':
			case 'event':
			{
				break;
			}
			default:
			{
				value = params[p];
				break;
			}
		}
	}
	getTarget(params,this).srcexpr(value);
});

//--------------------------------------------------------------------------------

/* json.js */


/*! some of this code came from Prototype and has been adapted to jQuery
*  Prototype is (c) 2005-2007 Sam Stephenson
*  Prototype is freely distributable under the terms of an MIT-style license.
*  For details, see the Prototype web site: http://www.prototypejs.org/
*--------------------------------------------------------------------------*/

Date.prototype.toJSON = function() {
  return '"' + this.getUTCFullYear() + '-' +
    (this.getUTCMonth() + 1).toPaddedString(2) + '-' +
    this.getUTCDate().toPaddedString(2) + 'T' +
    this.getUTCHours().toPaddedString(2) + ':' +
    this.getUTCMinutes().toPaddedString(2) + ':' +
    this.getUTCSeconds().toPaddedString(2) + 'Z"';
};

String.specialChar = {
  '\b': '\\b',
  '\t': '\\t',
  '\n': '\\n',
  '\f': '\\f',
  '\r': '\\r',
  '\\': '\\\\'
};

$.times = function(ch,len)
{
	var str = '';
	for (var c=0;c<len;c++)
	{
		str+=ch;
	}
	return str;
};

Number.prototype.toPaddedString= function(length, radix) {
  var string = this.toString(radix || 10);
  return $.times('0',length - string.length) + string;
};

String.prototype.toJSON = function() {
	var useDoubleQuotes = true;
    var escapedString = $.gsub(this,/[\x00-\x1f\\]/, function(match) {
      var character = String.specialChar[match[0]];
      return character ? character : '\\u00' + match[0].charCodeAt().toPaddedString(2, 16);
    });
    if (useDoubleQuotes) return '"' + escapedString.replace(/"/g, '\\"') + '"';
    return "'" + escapedString.replace(/'/g, '\\\'') + "'";
};

Array.prototype.toJSON = function(){
    var results = [];
    $.each(this,function(object) {
      var value = $.toJSON(this);
      if (value !== undefined) results.push(value);
    });
    return '[' + results.join(', ') + ']';
};

Number.prototype.toJSON = function(){
    return isFinite(this) ? this.toString() : 'null';
};

Boolean.prototype.toJSON = function(){
	return String(this);
};

$.extend(
{
	toJSON:function(object)
	{
		var type = typeof object;
		switch (type) {
		  case 'undefined':
		  case 'function':
		  case 'unknown': return 'null';
		}

		if (object === null) return 'null';
		if (typeof(object.jquery)=='string') return null;
		if (object.toJSON) return object.toJSON();
		if (object.nodeType == 1) return;

		var results = [];

		for (var property in object) 
		{
		   var value = object[property];
		   if (value !== undefined)
		   {
		   	  results.push($.toJSON(property) + ': ' + $.toJSON(value));
		   }
		}

		return '{' + results.join(', ') + '}';
	},

	JSONFilter: /^\/\*-secure-([\s\S]*)\*\/\s*$/,

	unfilterJSON: function(str,filter) {
		var m = (filter || this.JSONFilter).exec(str);
		return m ? m[1] : str;
  	},

  	isJSON: function(s) {
    	var str = s.replace(/\\./g, '@').replace(/"[^"\\\n\r]*"/g, '');
    	return (/^[,:{}\[\]0-9.\-+Eaeflnr-u \n\r\t]*$/).test(str);
  	},

  	evalJSON: function(str,sanitize) {
    	var json = this.unfilterJSON(str);
    	try {
      		if (!sanitize || $.isJSON(json)) return eval('(' + json + ')');
    	} catch (e) { }
    	throw new SyntaxError('Badly formed JSON string: ' + str);
  	}
});



//--------------------------------------------------------------------------------

/* md5.js */



//NOTE: this code has been adapted to load inside the 
//namespace

/**
 * MD5 cryptographic utils
 */
App.MD5 = 
{
	/*
	 * Configurable variables. You may need to tweak these to be compatible with
	 * the server-side, but the defaults work in most cases.
	 */
	hexcase: 0,  /* hex output format. 0 - lowercase; 1 - uppercase        */
	b64pad: "",  /* base-64 pad character. "=" for strict RFC compliance   */
	chrsz: 8,    /* bits per input character. 8 - ASCII; 16 - Unicode      */

	/*
	 * These are the functions you'll usually want to call
	 * They take string arguments and return either hex or base-64 encoded strings
	 */
	hex_md5: function (s){ return this.binl2hex(this.core_md5(this.str2binl(s), s.length * this.chrsz));},
	b64_md5: function(s){ return this.binl2b64(this.core_md5(this.str2binl(s), s.length * this.chrsz));},
	str_md5: function(s){ return this.binl2str(this.core_md5(this.str2binl(s), s.length * this.chrsz));},
	hex_hmac_md5: function(key, data) { return this.binl2hex(this.core_hmac_md5(key, data)); },
	b64_hmac_md5: function(key, data) { return this.binl2b64(this.core_hmac_md5(key, data)); },
	str_hmac_md5: function(key, data) { return this.binl2str(this.core_hmac_md5(key, data)); },
	
	/*
	 * Perform a simple self-test to see if the VM is working
	 */
	md5_vm_test:function()
	{
	  return this.hex_md5("abc") == "900150983cd24fb0d6963f7d28e17f72";
	},
	
	/*
	 * Calculate the MD5 of an array of little-endian words, and a bit length
	 */
	core_md5: function(x, len)
	{
	  /* append padding */
	  x[len >> 5] |= 0x80 << ((len) % 32);
	  x[(((len + 64) >>> 9) << 4) + 14] = len;
	
	  var a =  1732584193;
	  var b = -271733879;
	  var c = -1732584194;
	  var d =  271733878;
	
	  for(var i = 0; i < x.length; i += 16)
	  {
	    var olda = a;
	    var oldb = b;
	    var oldc = c;
	    var oldd = d;
	
	    a = this.md5_ff(a, b, c, d, x[i+ 0], 7 , -680876936);
	    d = this.md5_ff(d, a, b, c, x[i+ 1], 12, -389564586);
	    c = this.md5_ff(c, d, a, b, x[i+ 2], 17,  606105819);
	    b = this.md5_ff(b, c, d, a, x[i+ 3], 22, -1044525330);
	    a = this.md5_ff(a, b, c, d, x[i+ 4], 7 , -176418897);
	    d = this.md5_ff(d, a, b, c, x[i+ 5], 12,  1200080426);
	    c = this.md5_ff(c, d, a, b, x[i+ 6], 17, -1473231341);
	    b = this.md5_ff(b, c, d, a, x[i+ 7], 22, -45705983);
	    a = this.md5_ff(a, b, c, d, x[i+ 8], 7 ,  1770035416);
	    d = this.md5_ff(d, a, b, c, x[i+ 9], 12, -1958414417);
	    c = this.md5_ff(c, d, a, b, x[i+10], 17, -42063);
	    b = this.md5_ff(b, c, d, a, x[i+11], 22, -1990404162);
	    a = this.md5_ff(a, b, c, d, x[i+12], 7 ,  1804603682);
	    d = this.md5_ff(d, a, b, c, x[i+13], 12, -40341101);
	    c = this.md5_ff(c, d, a, b, x[i+14], 17, -1502002290);
	    b = this.md5_ff(b, c, d, a, x[i+15], 22,  1236535329);
	
	    a = this.md5_gg(a, b, c, d, x[i+ 1], 5 , -165796510);
	    d = this.md5_gg(d, a, b, c, x[i+ 6], 9 , -1069501632);
	    c = this.md5_gg(c, d, a, b, x[i+11], 14,  643717713);
	    b = this.md5_gg(b, c, d, a, x[i+ 0], 20, -373897302);
	    a = this.md5_gg(a, b, c, d, x[i+ 5], 5 , -701558691);
	    d = this.md5_gg(d, a, b, c, x[i+10], 9 ,  38016083);
	    c = this.md5_gg(c, d, a, b, x[i+15], 14, -660478335);
	    b = this.md5_gg(b, c, d, a, x[i+ 4], 20, -405537848);
	    a = this.md5_gg(a, b, c, d, x[i+ 9], 5 ,  568446438);
	    d = this.md5_gg(d, a, b, c, x[i+14], 9 , -1019803690);
	    c = this.md5_gg(c, d, a, b, x[i+ 3], 14, -187363961);
	    b = this.md5_gg(b, c, d, a, x[i+ 8], 20,  1163531501);
	    a = this.md5_gg(a, b, c, d, x[i+13], 5 , -1444681467);
	    d = this.md5_gg(d, a, b, c, x[i+ 2], 9 , -51403784);
	    c = this.md5_gg(c, d, a, b, x[i+ 7], 14,  1735328473);
	    b = this.md5_gg(b, c, d, a, x[i+12], 20, -1926607734);
	
	    a = this.md5_hh(a, b, c, d, x[i+ 5], 4 , -378558);
	    d = this.md5_hh(d, a, b, c, x[i+ 8], 11, -2022574463);
	    c = this.md5_hh(c, d, a, b, x[i+11], 16,  1839030562);
	    b = this.md5_hh(b, c, d, a, x[i+14], 23, -35309556);
	    a = this.md5_hh(a, b, c, d, x[i+ 1], 4 , -1530992060);
	    d = this.md5_hh(d, a, b, c, x[i+ 4], 11,  1272893353);
	    c = this.md5_hh(c, d, a, b, x[i+ 7], 16, -155497632);
	    b = this.md5_hh(b, c, d, a, x[i+10], 23, -1094730640);
	    a = this.md5_hh(a, b, c, d, x[i+13], 4 ,  681279174);
	    d = this.md5_hh(d, a, b, c, x[i+ 0], 11, -358537222);
	    c = this.md5_hh(c, d, a, b, x[i+ 3], 16, -722521979);
	    b = this.md5_hh(b, c, d, a, x[i+ 6], 23,  76029189);
	    a = this.md5_hh(a, b, c, d, x[i+ 9], 4 , -640364487);
	    d = this.md5_hh(d, a, b, c, x[i+12], 11, -421815835);
	    c = this.md5_hh(c, d, a, b, x[i+15], 16,  530742520);
	    b = this.md5_hh(b, c, d, a, x[i+ 2], 23, -995338651);
	
	    a = this.md5_ii(a, b, c, d, x[i+ 0], 6 , -198630844);
	    d = this.md5_ii(d, a, b, c, x[i+ 7], 10,  1126891415);
	    c = this.md5_ii(c, d, a, b, x[i+14], 15, -1416354905);
	    b = this.md5_ii(b, c, d, a, x[i+ 5], 21, -57434055);
	    a = this.md5_ii(a, b, c, d, x[i+12], 6 ,  1700485571);
	    d = this.md5_ii(d, a, b, c, x[i+ 3], 10, -1894986606);
	    c = this.md5_ii(c, d, a, b, x[i+10], 15, -1051523);
	    b = this.md5_ii(b, c, d, a, x[i+ 1], 21, -2054922799);
	    a = this.md5_ii(a, b, c, d, x[i+ 8], 6 ,  1873313359);
	    d = this.md5_ii(d, a, b, c, x[i+15], 10, -30611744);
	    c = this.md5_ii(c, d, a, b, x[i+ 6], 15, -1560198380);
	    b = this.md5_ii(b, c, d, a, x[i+13], 21,  1309151649);
	    a = this.md5_ii(a, b, c, d, x[i+ 4], 6 , -145523070);
	    d = this.md5_ii(d, a, b, c, x[i+11], 10, -1120210379);
	    c = this.md5_ii(c, d, a, b, x[i+ 2], 15,  718787259);
	    b = this.md5_ii(b, c, d, a, x[i+ 9], 21, -343485551);
	
	    a = this.safe_add(a, olda);
	    b = this.safe_add(b, oldb);
	    c = this.safe_add(c, oldc);
	    d = this.safe_add(d, oldd);
	  }
	  return Array(a, b, c, d);
	
	},
	
	/*
	 * These functions implement the four basic operations the algorithm uses.
	 */
	md5_cmn: function(q, a, b, x, s, t)
	{
	  return this.safe_add(this.bit_rol(this.safe_add(this.safe_add(a, q), this.safe_add(x, t)), s),b);
	},
	
	md5_ff: function (a, b, c, d, x, s, t)
	{
	  return this.md5_cmn((b & c) | ((~b) & d), a, b, x, s, t);
	},
	
	md5_gg: function (a, b, c, d, x, s, t)
	{
	  return this.md5_cmn((b & d) | (c & (~d)), a, b, x, s, t);
	},
	
    md5_hh: function(a, b, c, d, x, s, t)
	{
	  return this.md5_cmn(b ^ c ^ d, a, b, x, s, t);
	},
	
	md5_ii: function (a, b, c, d, x, s, t)
	{
	  return this.md5_cmn(c ^ (b | (~d)), a, b, x, s, t);
	},
	
	/*
	 * Calculate the HMAC-MD5, of a key and some data
	 */
	core_hmac_md5: function (key, data)
	{
	  var bkey = this.str2binl(key);
	  if(bkey.length > 16)
	  {
	     bkey = this.core_md5(bkey, key.length * this.chrsz);
	  }
	
	  var ipad = Array(16), opad = Array(16);
	  for(var i = 0; i < 16; i++)
	  {
	    ipad[i] = bkey[i] ^ 0x36363636;
	    opad[i] = bkey[i] ^ 0x5C5C5C5C;
	  }
	
	  var hash = this.core_md5(ipad.concat(this.str2binl(data)), 512 + data.length * this.chrsz);
	  return this.core_md5(opad.concat(hash), 512 + 128);
	},
	
	/*
	 * Add integers, wrapping at 2^32. This uses 16-bit operations internally
	 * to work around bugs in some JS interpreters.
	 */
	safe_add: function (x, y)
	{
	  var lsw = (x & 0xFFFF) + (y & 0xFFFF);
	  var msw = (x >> 16) + (y >> 16) + (lsw >> 16);
	  return (msw << 16) | (lsw & 0xFFFF);
	},
	
	/*
	 * Bitwise rotate a 32-bit number to the left.
	 */
	bit_rol: function (num, cnt)
	{
	  return (num << cnt) | (num >>> (32 - cnt));
	},
	
	/*
	 * Convert a string to an array of little-endian words
	 * If chrsz is ASCII, characters >255 have their hi-byte silently ignored.
	 */
	str2binl: function (str)
	{
	  var bin = Array();
	  var mask = (1 << this.chrsz) - 1;
	  for(var i = 0; i < str.length * this.chrsz; i += this.chrsz)
	    bin[i>>5] |= (str.charCodeAt(i / this.chrsz) & mask) << (i%32);
	  return bin;
	},
	
	/*
	 * Convert an array of little-endian words to a string
	 */
	binl2str: function (bin)
	{
	  var str = "";
	  var mask = (1 << this.chrsz) - 1;
	  for(var i = 0; i < bin.length * 32; i += this.chrsz)
	    str += String.fromCharCode((bin[i>>5] >>> (i % 32)) & mask);
	  return str;
	},
	
	/*
	 * Convert an array of little-endian words to a hex string.
	 */
	binl2hex: function (binarray)
	{
	  var hex_tab = this.hexcase ? "0123456789ABCDEF" : "0123456789abcdef";
	  var str = "";
	  for(var i = 0; i < binarray.length * 4; i++)
	  {
	    str += hex_tab.charAt((binarray[i>>2] >> ((i%4)*8+4)) & 0xF) +
	           hex_tab.charAt((binarray[i>>2] >> ((i%4)*8  )) & 0xF);
	  }
	  return str;
	},
	
	/*
	 * Convert an array of little-endian words to a base-64 string
	 */
	binl2b64: function (binarray)
	{
	  var tab = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	  var str = "";
	  for(var i = 0; i < binarray.length * 4; i += 3)
	  {
	    var triplet = (((binarray[i   >> 2] >> 8 * ( i   %4)) & 0xFF) << 16)
	                | (((binarray[i+1 >> 2] >> 8 * ((i+1)%4)) & 0xFF) << 8 )
	                |  ((binarray[i+2 >> 2] >> 8 * ((i+2)%4)) & 0xFF);
	    for(var j = 0; j < 4; j++)
	    {
	      if(i * 8 + j * 6 > binarray.length * 32) 
	      {
	         str += this.b64pad;
	      }
	      else 
	      {
	         str += tab.charAt((triplet >> 6*(3-j)) & 0x3F);
	      }
	    }
	  }
	  return str;
	}
    
};


//--------------------------------------------------------------------------------

/* on.js */


//--------------------------------------------------------------------------------

/* plugins.js */

//
// support for dynamically loading plugins as they 
// are invoked as a plugin stub
//
var loadedPlugins = {};
var dynamicPlugins = 
{
	'example':null
};


function installDynPlugin(name,fn)
{
	var l = loadedPlugins[name];
	if (l) return fn();
	var path = AppC.pluginRoot+name+'.js';
	$.getScript(path,function()
	{
		loadedPlugins[name]=1;
		fn();
	});
};

for (var name in dynamicPlugins)
{
	(function()
	{
		var found = $.fn[name];
		// allow them to be pre-loaded (such as in <script>) and in this
		// case, we assume it's already loaded and doesn't need this at all
		if (typeof(found)=='function') return;
		$.fn[name]=function()
		{
			var arguments = arguments;
			var scope = this;

			var depends = dynamicPlugins[name] || [];
			depends.push(name);
			var length = depends.length;
			var count = 0;

			var f = function()
			{
				var n = depends[count];
				if (count++ < length)
				{
					installDynPlugin(n,f);
				}
				else
				{
					// at this point, the plugin should have installed itself over top of us
					// so we can now invoke it
					$.fn[name].apply(scope,arguments);
				}
			};

			f();

			return this;
		}
	})();
}

//--------------------------------------------------------------------------------

/* scriptlet.js */

// Simple JavaScript Templating
// John Resig - http://ejohn.org/ - MIT Licensed
(function(){
  var cache = {};
 
  $.scriptlet = function scriptlet(str, data){
    // Figure out if we're getting a template, or if we need to
    // load the template - and be sure to cache the result.
    var fn = !/\W/.test(str) ?
      cache[str] = cache[str] ||
        scriptlet($(str).html()) :
     
      // Generate a reusable function that will serve as a template
      // generator (and which will be cached).
      new Function("obj",
        "var p=[],print=function(){p.push.apply(p,arguments);};" +
       
        // Introduce the data as local variables using with(){}
        "with(obj){p.push('" +
       
        // Convert the template into pure JavaScript
        str
          .replace(/[\r\t\n]/g, " ")
          .split("<%").join("\t")
          .replace(/((^|%>)[^\t]*)'/g, "$1\r")
          .replace(/\t=(.*?)%>/g, "',$1,'")
          .split("\t").join("');")
          .split("%>").join("p.push('")
          .split("\r").join("\\'")
      + "');}return p.join('');");
   
    // Provide some basic currying to the user
    return data ? fn( data ) : fn;
  };
})(jQuery);

//--------------------------------------------------------------------------------

/* set.js */

var components = 
{
	controls:{},
	themes:{},
	behaviors:{},
	layouts:{}
};

AppC.register = function(type,name,fn)
{
	var e = components[type+'s'][name];
	if (!e)
	{
		// in case you call directly
		e={pend:[],factory:fn};
		components[type+'s'][name]=e;
	}
	e.factory = fn;
	$.each(e.pend,function()
	{
		var instance = createControl(this.el,name,this.opts,this.fn);
		if (!instance) $.error("framework error - instance not returned from factory for: "+name);
		var render = instance.render;
		var el = this.el;
		var opts = this.opts;
		instance.render = function()
		{
			if (arguments.length == 1)
			{
				render.apply(instance,[el,arguments[0]]);
			}
			else
			{
				render.apply(instance,arguments);
			}
			el.trigger('rendered',instance);
		};
		el.data('control',instance);
		el.trigger('created',[instance,this.opts]);
		$.each(instance.getAttributes(),function()
		{
			switch (this.type)
			{
				case AppC.Types.condition:
				{
					var name = 'on' + $.proper(this.name);
					App.regCond(new RegExp('^('+this.name+')$'),function(meta)
					{
						var bindFn = function(args)
						{
							var scope = $(this);
							args = args || {};
							args.id = $(this).attr('id');
							App.triggerAction(scope,args,meta);
						};
						el.bind(name,bindFn);
						el.trash(function()
						{
							el.unbind(name,bindFn);
						});
					});
					break;
				}
				case AppC.Types.action:
				{
					//FIXME
					break;
				}
				default:
				{
					var v = opts[this.name] || this.defaultValue;
					if (typeof(v)=='undefined' && !this.optional)
					{
						el.trigger('onError',"required property '"+this.name+"' not found or missing value");
						//FIXME
					}
					opts[this.name]=v;
				}
			}
		});
		
		// call the function callback if passed in
		if (this.fn) this.fn.call(instance,opts);

		instance.render.apply(instance,[this.el,opts]);
	});
};

function createControl(el,name,opts,fn)
{
	var e = components.controls[name];
	opts = opts || {};
	if (e)
	{
		if (!e.factory)
		{
			e.pend.push({el:el,fn:fn,opts:opts});
			return;
		}
		var instance = e.factory.create();
		return instance;
	}
	e = {pend:[{el:el,fn:fn,opts:opts}],factory:null}
	components.controls[name]=e;
	load('control',name,e);
}

function load(type,name,e)
{
	var uri = AppC.sdkPath + 'components/'+type+'s/'+name+'/'+name+'.js';
	$.getScript(uri);
}

$.fn.control = function(name,opts,fn)
{
	if (arguments.length == 0)
	{
		return this.data('control');
	}
	// 2nd argument can be the callback function, in which case
	// we're not passing in parameters
	if (typeof(opts)=='function')
	{
		fn = opts;
		opts = {};
	}
	createControl($(this),name,opts,fn);
	return this;
};

$.fn.theme = function(name,options)
{
	return this;
};

$.fn.behavior = function(name,options)
{
	return this;
};

$.fn.layout = function(name,options)
{
	return this;
};


App.reg('set','*',function(value,state)
{
	var el = $(this);
	var visibility = el.css('visibility') || 'visible';
	var show = false, initial = true;

	if (visibility == 'visible')
	{
		el.css('visibility','hidden');
		show = true;
	}

	el.addClass('container');

	var bindFn = function()
	{
		el.compileChildren(state,false);
		if (show)
		{
			$(this).css('visibility',visibility);
			show=false;
		}
		if (initial)
		{
			initial=false;
			App.checkState(state,el);
		}
	};
	el.bind('rendered',bindFn);
	el.trash(function()
	{
		el.unbind('rendered',bindFn);
	});
	
	$.each($.smartSplit(value,' and '),function()
	{
		var idx = this.indexOf('[');
		if (idx < 0)
		{
			throw new "invalid set expression. must be in the form: control[type]";
		}
		var lastIdx = this.lastIndexOf(']');
		var ui = this.substring(0,idx);
		var params = this.substring(idx+1,lastIdx);
		var comma = params.indexOf(',');
		var type = null, args = {};
		if (comma < 0)
		{
			type = params;
		}
		else
		{
			type = params.substring(0,comma);
			args = App.getParameters(params.substring(comma+1),true);
			for (var p in args)
			{
				args[p] = App.getEvaluatedValue(args[p]);
			}
		}
		$.info('creating component of type='+type+',ui='+ui+',args='+$.toJSON(args));
		el[ui](type,args);
	});

},true);



//--------------------------------------------------------------------------------

/* useragent.js */


var ua = navigator.userAgent.toLowerCase();

AppC.UA = 
{
	supported:false,
	opera: (ua.indexOf('opera') > -1),
	safari: (ua.indexOf('safari') > -1),
	safari2: false,
	safari3: false,
	IE: !!(window.ActiveXObject),
	IE6: false,
	IE7: false,
	IE8: false,
	windows: false,
	mac: false,
	linux: false,
	sunOS: false,
	platform: 'unknown',
	flash:false,
	flashVersion:0,
	silverlight:false,
	sliverlightVersion:0
};

if (AppC.UA.IE)
{
	var arVersion = navigator.appVersion.split("MSIE");
	var version = parseFloat(arVersion[1]);
	AppC.UA.IE6 = version >= 6.0 && version < 7;
	AppC.UA.IE7 = version >= 7.0 && version < 8;
	AppC.UA.IE8 = version >= 8.0 && version < 9;
}
else if (AppC.UA.safari)
{
	
	var webKitFields = RegExp("( applewebkit/)([^ ]+)").exec(ua);
	if (webKitFields[2] > 400 && webKitFields[2] < 500)
	{
		AppC.UA.safari2 = true;
	}
	else if (webKitFields[2] > 500 && webKitFields[2] < 600)
	{
		AppC.UA.safari3 = true;
	}
}

AppC.UA.gecko = !AppC.UA.safari && (ua.indexOf('gecko') > -1);
AppC.UA.camino = AppC.UA.gecko && ua.indexOf('camino') > -1;
AppC.UA.firefox = AppC.UA.gecko && (ua.indexOf('firefox') > -1 || AppC.UA.camino || ua.indexOf('minefield') > -1 || ua.indexOf('granparadiso') > -1 || ua.indexOf('bonecho') > -1);
AppC.UA.IPhone = AppC.UA.safari && ua.indexOf('iphone') > -1;
AppC.UA.mozilla = AppC.UA.gecko && ua.indexOf('mozilla/') > -1;
AppC.UA.webkit = AppC.UA.mozilla && AppC.UA.gecko && ua.indexOf('applewebkit') > 0;
AppC.UA.seamonkey = AppC.UA.mozilla && ua.indexOf('seamonkey') > -1;
AppC.UA.prism = AppC.UA.mozilla && ua.indexOf('prism/') > 0;
AppC.UA.iceweasel = AppC.UA.mozilla && ua.indexOf('iceweasel') > 0;
AppC.UA.epiphany = AppC.UA.mozilla && ua.indexOf('epiphany') > 0;
AppC.UA.fluid = (window.fluid != null);
AppC.UA.gears = (window.google && google.gears) != null;
AppC.UA.chromium = AppC.UA.webkit && ua.indexOf('chrome/') > 0;


if(ua.indexOf("windows") != -1 || ua.indexOf("win32") != -1)
{
    AppC.UA.windows = true;
	AppC.UA.platform = 'win32';
}
else if(ua.indexOf("macintosh") != -1 || ua.indexOf('mac os x') != -1)
{
	AppC.UA.mac = true;
	AppC.UA.platform = 'mac';
}
else if (ua.indexOf('linux')!=-1)
{
	AppC.UA.linux = true;
	AppC.UA.platform = 'linux';
}
else if (ua.indexOf('sunos')!=-1)
{
	AppC.UA.sunOS = true;
	AppC.UA.platform = 'sun';
}

// silverlight detection
// thanks to http://www.nikhilk.net/Silverlight-Analytics.aspx
AppC.UA.silverlight = false;
AppC.UA.silverlightVersion = 0;

function checkForSilverlight()
{
    var container = null;
    try {
        var control = null;
        if (window.ActiveXObject) {
            control = new ActiveXObject('AgControl.AgControl');
        }
        else {
            if (navigator.plugins['Silverlight Plug-In']) {
                container = document.createElement('div');
                document.body.appendChild(container);
                container.innerHTML= '<embed type="application/x-silverlight" src="data:," />';
                control = container.childNodes[0];
            }
        }
        if (control) {
            if (control.isVersionSupported('2.0')) 
			{ 
				AppC.UA.silverlightVersion = 2.0; 
			}
            else if (control.isVersionSupported('1.0')) 
			{ 
				AppC.UA.silverlightVersion = 1.0; 
			}
			AppC.UA.silverlight = AppC.UA.silverlightVersion > 0;
        }
    }
    catch (e) { }
    if (container) {
        try { document.body.removeChild(container) } catch(E){ }
    }
}

// flash detection
if (AppC.UA.IE)
{
		try
		{
			var flash = new ActiveXObject("ShockwaveFlash.ShockwaveFlash.7");
			var ver = flash.GetVariable("$version");
			var idx = ver.indexOf(' ');
			var tokens = ver.substring(idx+1).split(',');
			var version = tokens[0];
			AppC.UA.flashVersion = parseInt(version);
			AppC.UA.flash = true;
		}
		catch(e)
		{
			// we currently don't support lower than 7 anyway
		}
}
else
{
	var plugin = navigator.plugins && navigator.plugins.length;
	if (plugin)
	{
		 plugin = navigator.plugins["Shockwave Flash"] || navigator.plugins["Shockwave Flash 2.0"];
		 if (plugin)
		 {
			if (plugin.description)
			{
				var ver = plugin.description;
				AppC.UA.flashVersion = parseInt(ver.charAt(ver.indexOf('.')-1));
				AppC.UA.flash = true;
			}			 	
			else
			{
				// not sure what version... ?
				AppC.UA.flashVersion = 7;
				AppC.UA.flash = true;
			}
		 }
	}
	else
	{
		plugin = (navigator.mimeTypes && 
	                    navigator.mimeTypes["application/x-shockwave-flash"] &&
	                    navigator.mimeTypes["application/x-shockwave-flash"].enabledPlugin) ?
	                    navigator.mimeTypes["application/x-shockwave-flash"].enabledPlugin : 0;
		if (plugin && plugin.description) 
		{
			AppC.UA.flash = true;
	    	AppC.UA.flashVersion = parseInt(plugin.description.substring(plugin.description.indexOf(".")-1));
		}
	}
}

$.each('firefox IE6 IE7 IE8 safari chromium webkit opera camino seamonkey prism fluid iceweasel epiphany'.split(' '),function()
{
	if (AppC.UA[this]===true)
	{
		AppC.UA.supported = true;
		var name = this.toLowerCase();
		AppC.beforeCompile(function(body)
		{
			checkForSilverlight();
			if (AppC.UA.platform) body.addClass(AppC.platform);
			body.addClass(name);
			for (var p in AppC.UA)
			{
				var v = AppC.UA[p];
				if (typeof(v)=='boolean' && v===true && p!='supported' && p!='flash' && p!='silverlight')
				{
					body.addClass(p.toLowerCase());
				}
			}
			if (AppC.UA.IPhone)
			{
				body.addClass('width_narrow');
				body.addClass('height_short');
			}
			else
			{
				function calcDim()
				{
					var cn = body.attr('class');
					if (cn)
					{
						$.each(cn.split(' '),function()
						{
							if (/^(height|width)_/.test(this))
							{
								body.removeClass(this);
							}
						});
					}
                    var width = $(document).width();
                    var height = $(document).height();

					if (height < 480)
					{
						body.addClass('height_tiny');
					}
					else if (height >= 480 && height <= 768)
					{
						body.addClass('height_small');
					}
					else if (height > 768  && height < 1100)
					{
						body.addClass('height_medium');
					}
					else if (height >= 1100)
					{
						body.addClass('height_large');
					}
					if (width <= 640)
					{
						body.addClass('width_tiny');
					}
					else if (width > 640 && width <= 1024)
					{
						body.addClass('width_small');
					}
					else if (width > 1024 && width <=1280 )
					{
						body.addClass('width_medium');
					}
					else if (width > 1280)
					{
						body.addClass('width_large');
					}
				}
				$(window).bind('resize',calcDim);
				calcDim();
			}
		});
		return false; // once we have a supported browser, just break out
	}
});

//FIXME: browser not supported work

//--------------------------------------------------------------------------------

/* uuid.js */

/**
 * utility function to generate a semi-random uuid
 * which is good enough as a unique id for what we normally want
 */
App.UUID =
{
    dateSeed: (started || new Date).getTime(),
	convert: ['0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'],

	// Numeric Base Conversion algorithm from irt.org
	// In base 16: 0=0, 5=5, 10=A, 15=F
 	base16: function(number)
	{
		//
		// Copyright 1996-2006 irt.org, All Rights Reserved.	
		//
		// Downloaded from: http://www.irt.org/script/146.htm	
		// slight modifications by Jeff Haynie/Appcelerator
		// you should be able to use String.toString(16) but 
		// apparently not reliable on all browsers (hint IE)
		//
		var output = null;
	    if (number < 16)
		{
			output = this.convert[number];
		}
	    else 
		{
	        var MSD = '' + Math.floor(number / 16);
	        var LSD = number - MSD*16;
	        if (MSD >= 16)
			{
				output = this.base16(MSD) + this.convert[LSD];
			}
	        else
			{
				output = this.convert[MSD] + this.convert[LSD];
			}
	    }
	    return output;
	},
    newID: function()
    {
		var dg = new Date(1970, 9, 22, 0, 0, 0, 0);
		var t = this.base16(this.dateSeed - dg.getTime());
        var a = this.base16(Math.floor(999999999999 * Math.random()));
        var _b = App.MD5.hex_md5(window.location.pathname);
		var b = $.gsub((_b.length > 10 ? _b.substring(0,10) : _b),/[^a-zA-Z0-9]/,'0');
        var c = this.base16(Math.round(this.dateSeed * Math.random()));
        return t + '-' + a + '-' + b + '-' + c;
    }
};
//--------------------------------------------------------------------------------

/* add_remove.js */

var currentAddFn = $.fn.add;

$.fn.add = function(prop,value)
{
	if (arguments.length == 2 && typeof(prop)=='string')
	{
		switch(prop)
		{
			case 'class':
			case 'className':
			{
				this.addClass(value);
				break;
			}
			default:
			{
				this.attr(prop,value);
			}
		}
		return this;
	}
	else
	{
		return currentAddFn.apply(this,arguments);
	}
};

var currentRemoveFn = $.fn.remove;

$.fn.remove = function(prop,value)
{
	if (!prop)
	{
		return currentRemoveFn.apply(this,arguments);
	}
	
	$.each(this,function()
	{
		switch(prop)
		{
			case 'class':
			case 'className':
			{
				$(this).removeClass(value);
				break;
			}
			default:
			{
				if ($(prop).length == 0)
				{
					$(this).removeAttr(prop);			
				}
				else
				{
					currentRemoveFn.apply(this, arguments);
				}
			}
		}
	});
	return this;
};

//--------------------------------------------------------------------------------

/* after.js */

$.fn.after = function(t,f)
{
	var time = App.timeFormat(t);
	var scope = this;
	setTimeout(function()
	{
		f.call(scope);
	},time);
	return this;
};

//--------------------------------------------------------------------------------

/* animate.js */

(function(jQuery){

	// We override the animation for all of these color styles
	jQuery.each(['backgroundColor', 'borderBottomColor', 'borderLeftColor', 'borderRightColor', 'borderTopColor', 'color', 'outlineColor'], function(i,attr){
		jQuery.fx.step[attr] = function(fx){
			if (fx.state == 0 ) {
				fx.start = getColor( fx.elem, attr );
				fx.end = getRGB( fx.end );
			}
			
			fx.elem.style[attr] = "rgb(" + [
				Math.max(Math.min( parseInt((fx.pos * (fx.end[0] - fx.start[0])) + fx.start[0]), 255), 0),
				Math.max(Math.min( parseInt((fx.pos * (fx.end[1] - fx.start[1])) + fx.start[1]), 255), 0),
				Math.max(Math.min( parseInt((fx.pos * (fx.end[2] - fx.start[2])) + fx.start[2]), 255), 0)
			].join(",") + ")";
		}
	});

	// Color Conversion functions from highlightFade
	// By Blair Mitchelmore
	// http://jquery.offput.ca/highlightFade/

	// Parse strings looking for color tuples [255,255,255]
	function getRGB(color) {
		var result;
		
		// Check if we're already dealing with an array of colors
		if ( color && color.constructor == Array && color.length == 3 )
			return color;
			
		// transparent -- assume white....
		if (color == 'transparent') return [255,255,255];

		// Look for rgb(num,num,num)
		if (result = /rgb\(\s*([0-9]{1,3})\s*,\s*([0-9]{1,3})\s*,\s*([0-9]{1,3})\s*\)/.exec(color))
			return [parseInt(result[1]), parseInt(result[2]), parseInt(result[3])];

		// Look for rgb(num%,num%,num%)
		if (result = /rgb\(\s*([0-9]+(?:\.[0-9]+)?)\%\s*,\s*([0-9]+(?:\.[0-9]+)?)\%\s*,\s*([0-9]+(?:\.[0-9]+)?)\%\s*\)/.exec(color))
			return [parseFloat(result[1])*2.55, parseFloat(result[2])*2.55, parseFloat(result[3])*2.55];

		// Look for #a0b1c2
		if (result = /#([a-fA-F0-9]{2})([a-fA-F0-9]{2})([a-fA-F0-9]{2})/.exec(color))
			return [parseInt(result[1],16), parseInt(result[2],16), parseInt(result[3],16)];

		// Look for #fff
		if (result = /#([a-fA-F0-9])([a-fA-F0-9])([a-fA-F0-9])/.exec(color))
			return [parseInt(result[1]+result[1],16), parseInt(result[2]+result[2],16), parseInt(result[3]+result[3],16)];

		// Otherwise, we're most likely dealing with a named color
		return colors[jQuery.trim(color).toLowerCase()];
	}

	function getColor(elem, attr) {
		var color;

		do {
			color = jQuery.curCSS(elem, attr);
			
			// Keep going until we find an element that has color, or we hit the body
			if ( color != '' && color != 'transparent' || jQuery.nodeName(elem, "body") )
				break; 

			attr = "backgroundColor";
		} while ( elem = elem.parentNode );

		return getRGB(color);
	};

	// Some named colors to work with
	// From Interface by Stefan Petre
	// http://interface.eyecon.ro/

	var colors = {
		aqua:[0,255,255],
		azure:[240,255,255],
		beige:[245,245,220],
		black:[0,0,0],
		blue:[0,0,255],
		brown:[165,42,42],
		cyan:[0,255,255],
		darkblue:[0,0,139],
		darkcyan:[0,139,139],
		darkgrey:[169,169,169],
		darkgreen:[0,100,0],
		darkkhaki:[189,183,107],
		darkmagenta:[139,0,139],
		darkolivegreen:[85,107,47],
		darkorange:[255,140,0],
		darkorchid:[153,50,204],
		darkred:[139,0,0],
		darksalmon:[233,150,122],
		darkviolet:[148,0,211],
		fuchsia:[255,0,255],
		gold:[255,215,0],
		green:[0,128,0],
		indigo:[75,0,130],
		khaki:[240,230,140],
		lightblue:[173,216,230],
		lightcyan:[224,255,255],
		lightgreen:[144,238,144],
		lightgrey:[211,211,211],
		lightpink:[255,182,193],
		lightyellow:[255,255,224],
		lime:[0,255,0],
		magenta:[255,0,255],
		maroon:[128,0,0],
		navy:[0,0,128],
		olive:[128,128,0],
		orange:[255,165,0],
		pink:[255,192,203],
		purple:[128,0,128],
		violet:[128,0,128],
		red:[255,0,0],
		silver:[192,192,192],
		white:[255,255,255],
		yellow:[255,255,0]
	};

})($);
//--------------------------------------------------------------------------------

/* bind.js */


var oldBind = $.fn.bind;

$.fn.bind = function()
{
	if (arguments.length > 1)
	{
		// jQuery bind
		return oldBind.apply(this,arguments);
	}
	else
	{
		alert('foo')
		var obj = arguments[0];
		$.each(this,function(idx)
		{
			var t = $(this).get(idx);
			$.each($(t).children(),function()
			{
				var child = $(this);
				var name = child.attr('name');
				var value = name ? obj[name] : obj[child.attr('id')];
				if (value)
				{
					//REVIEW: value?
					child.val(value);
				}
			});
		});
	}
	return this;
};




//--------------------------------------------------------------------------------

/* clearform.js */


//--------------------------------------------------------------------------------

/* disable.js */

$.fn.disable = function()
{
	$.each(this,function(idx)
	{
		$(this).attr('disabled',true);
	});
};
//--------------------------------------------------------------------------------

/* enable.js */

$.fn.enable = function()
{
	$.each(this,function()
	{
		$(this).removeAttr('disabled');
	});
}
//--------------------------------------------------------------------------------

/* form.js */

var validators = {}, decorators = {}, fieldsets = {};
var validatorMonitor = null;
var validatorMonitors = null;
var validatorMonitorRate = 250;

AppC.setFieldMonitorInterval = function(rate)
{
	validatorMonitorRate = rate;
	if (validatorMonitor)
	{
		clearInterval(validatorMonitor);
		validatorMonitor = setInterval(fieldMonitor,validatorMonitorRate);
	}
}

AppC.addValidator = function(name,fn)
{
	validators[name]=fn;
};

AppC.getValidator = function(name)
{
	if (name)
	{
		return validators[name];
	}
}

AppC.validators = function(iter)
{
	for (var p in validators)
	{
		iter.call({name:p,fn:validators[p]});
	}
};

AppC.addDecorator = function(name,fn)
{
	decorators[name]=fn;
};

App.getDecorator = function(name)
{
	if (name)
	{
		return decorators[name];
	}
}

AppC.decorators = function(iter)
{
	for (var p in decorators)
	{
		iter.call({name:p,fn:decorators[p]});
	}
};

AppC.addValidator('required',function(el,value)
{
	return (typeof(value)!='undefined' && value);
});

function installDecorator(el,target)
{
	if (!target)
	{
		var img = AppC.sdkRoot + 'images/exclamation.png';
		var id = el.attr('id') + '_decorator';
		var html = '<span id="'+id+'" class="decorator"><img src="' + img + '"/></span>';
		el.after(html);
		target = $('#'+id);
		el.data('decoratorTarget',target);
	}
	return target;
}

AppC.addDecorator('required',function(el,valid,value,target)
{
	target = installDecorator(el,target);
	
	if (valid)
	{
		el.css('background-color','');
		target.css('visibility','hidden');
	}
	else
	{
		el.css('background-color','#FFEEEE');
		target.css('visibility','visible');
	}
});

AppC.addDecorator('custom',function(el,valid,value,target)
{
	target.css('visibility',valid ? 'hidden' : 'visible');
});

function fieldMonitor()
{
	if (validatorMonitors && validatorMonitors.length > 0)
	{
		$.each(validatorMonitors,function()
		{
			$(this).revalidate(true);
		});
	}
}

function startFieldMonitor(el)
{
	if (!validatorMonitors)
	{
		validatorMonitors = [];
		validatorMonitor = setInterval(fieldMonitor,validatorMonitorRate);
	}
	validatorMonitors.push(el);
	var fn = function()
	{
		stopFieldMonitor(el);
	};
	el.data('fieldMonitor',fn);
	el.bind('blur',fn);
}


function stopFieldMonitor(el)
{
	if (validatorMonitors)
	{
		var idx = validatorMonitors.indexOf(el);
		var idx = $.inArray(el,validatorMonitors);
		if (idx != -1)
		{
			validatorMonitors.splice(idx,1);
		}
		var fn = el.data('fieldMonitor');
		if (fn) el.unbind('blur',fn);
		el.removeData('fieldMonitor');
	}
}

function makeFormEntry(array,name,e,fn)
{
	$.each(array,function(idx)
	{
		var el = $(this);
		el.data(name,e);
		if (fn) fn(el);
		el.revalidate();
	});
	return array;
}

function fieldDecorate(el,valid,value)
{
	if (!value)
	{
		el.addClass('validator_empty').removeClass('validator_value');
	}
	else
	{
		el.removeClass('validator_empty').addClass('validator_value');
	}
	if (valid)
	{
		el.removeClass('validator_invalid').addClass('validator_valid');
	}
	else
	{
		el.addClass('validator_invalid').removeClass('validator_valid');
	}
	var dec = el.data('decorator');
	var target = el.data('decoratorTarget');
	var fn = typeof(dec)=='function' ? dec : App.getDecorator(dec);
	return fn ? fn(el,valid,value,target) : null;
}

function fieldActivate(el,activator,activators)
{
	activator = activator ? activator : el.data('activator');
	if (activator)
	{
		var valid = true;
		var array = activators ? activators : activator.data('activators');
		$.each(array,function()
		{
			var r = $('#'+this).data('validatorResult');
			if (r===false)
			{
				valid = false;
				return false;
			}
		});
		if (valid)
		{
			activator.removeAttr('disabled');
		}
		else
		{
			activator.attr('disabled',true);
		}
	}
}

function fieldRevalidate(el,changeOnly,ignoreActivate)
{
	var v = el.data('validator');
	if (v)
	{
		var result = false;
		var validator = typeof(v)=='function' ? v : AppC.getValidator(v);
		if (validator)
		{
			try
			{
				var value = getElementValue(el);
				if (changeOnly && el.data('validatorData')===value)
				{
					// ignore if the same and its a change only trigger
					return;
				}
				el.data('validatorData',value);
				result = validator(el,value);
				// turn it into a boolean and assume a result of undefined is the same as positive
				result = (typeof(result)=='undefined' ? true : result) ? true : false;
				el.data('validatorResult',result);
				fieldDecorate(el,result,value);
				if (!ignoreActivate) fieldActivate(el);
			}
			catch (E)
			{
				$.error("error in validation = "+E+" for element = "+el.attr('id'));
			}
		}
	}
	else
	{
		var activators = el.data('activators');
		if (activators)
		{
			fieldActivate(null,el,activators);
		}
	}
}

$.fn.revalidate = function(changeOnly)
{
	return $.each(this,function(idx)
	{
		fieldRevalidate($(this),changeOnly);
	});
	return this;
};

$.fn.validator = function(v)
{
	makeFormEntry(this,'validator',v,function(el)
	{
		if (el.is(':text,textarea'))
		{
			el.bind('focus',function()
			{
				startFieldMonitor(el);
			});
		}
		else if (el.is('select,:radio,:checkbox'))
		{
			el.bind('click',function()
			{
				el.revalidate();
			});
		}
	});
	return this;
};

$.fn.decorator = function(d,decId)
{
	var dec = decId ? (typeof(decId)=='string' ? $('#'+decId) : $(decId)) : null;
	var fn = dec ? function(el) { el.data('decoratorTarget', dec) } : null;
	return makeFormEntry(this,'decorator',d,fn);
};

$.fn.activators = function(a) 
{
	var self = this;
	var array = (typeof(a)=='string' ? a.split(/[ ,]/) : $.makeArray(a)).map(function(e){return $.trim(e)});
	this.data('activators',array);
	$.each(array,function(idx)
	{
		var el = $('#'+$.trim(this));
		if (!el)
		{
			$.error('Error adding activator field with id: '+this+', not found');
			return;
		}
		el.data('activator',self);
	});
	this.revalidate();
	return this;
};


$.fn.fieldset = function(fs)
{
	var array = (typeof(fs)=='string' ? fs.split(/[ ,]/) : $.makeArray(fs)).map(function(e){return $.trim(e)});
	$.each(this,function(idx)
	{
		var el = $(this);
		if (!el)
		{
			$.error('Error adding fieldset field with id: '+$(this).attr('id')+', not found');
			return;
		}
		el.data('fieldsets',array);
		$.each(array,function()
		{
			App.addToFieldset(el,$.trim(this));
		});
	});
};

App.addToFieldset = function(el,fs)
{
	var elements = fieldsets[fs];
	if (!elements)
	{
		elements = [];
		fieldsets[fs]=elements;
	}
	elements.push(el);
}

App.getFieldsetData = function(fs,obj)
{
	obj = obj || {};
	var array = typeof(fs)=='string' ? [fs] : fs.data('fieldsets');
	if (array && array.length > 0)
	{
		$.each(array,function()
		{
			var elements = fieldsets[this];
			if (elements)
			{
				$.each(elements,function()
				{
					var el = this;
					// we include if not a button
					if (!el.is(':button'))
					{
						var value = getElementValue(el);
						var key = el.is('form') ? el.attr('name') || el.attr('fieldset') || el.attr('id') : el.attr('name') || el.attr('id');
						obj[key]=value;
					}
				});
			}
		});
	}
	return obj;
};


App.reg('validator',['input','button','select','textarea'],function(value,state)
{
	$(this).validator(value);
});

App.reg('decorator',['input','button','select','textarea'],function(value,state)
{
	$(this).decorator(value,$(this).attr('decoratorId'));
});

App.reg('activators',['div','input','button'],function(value,state)
{
	$(this).activators(value);
});

App.reg('fieldset',['form','input','button','select','textarea'],function(value,state)
{
	$(this).fieldset(value);
});


//
// remap val function such that when it's called, it will always revalidate
// the field
//
var oldVal = $.fn.val;
$.fn.val = function()
{
	// we need to make sure that we don't 
	// call revalidate from an existing revalidate
	// in the call stack - which case we'll get into
	// an infinite loop... this just puts a guard in
	// to let us know that we're in this state
	var inr = this.data('revalidating');
	var rev = false;
	if (!inr)
	{
		rev=true;
		this.data('revalidating',true);
	}
	var r = oldVal.apply(this,arguments);
	if (rev)
	{
		this.revalidate();
		this.removeData('revalidating');
	}
	return r;
};

//--------------------------------------------------------------------------------

/* hidden.js */

regCSSAction('hidden','visibility');
//--------------------------------------------------------------------------------

/* highlight.js */

$.fn.highlight = function(bgColor)
{
	bgColor = bgColor || '#ffffcc';
	$.each(this,function()
	{
		var curBgColor = $(this).css('backgroundColor');
		$(this).animate({'backgroundColor':bgColor},50).animate({'backgroundColor':curBgColor},1000);
	});
	return this;
};


//--------------------------------------------------------------------------------

/* history.js */

App.dynregAction('history');

//--------------------------------------------------------------------------------

/* locale.js */


AppC.currentLocale = window.navigator.language || 'en';
var bundles = {};

//
// called to dynamically load a language locale.
//
// the path of the bundle is:
//
// AppC.sdkPath + 'localization/' + locale.properties
//
// will attempt to load specific locale is fully specified (such as en-UK)
// and will attempt to fall back to short version (such as en) if not found
//
AppC.locale = function(lang)
{
	if (lang==AppC.currentLocale)
	{
		return;
	}
	AppC.currentLocale = lang;
	if (!bundles[lang])
	{
		try
		{
			var url = AppC.sdkPath + 'localization/' + lang.toLowerCase() + '.properties';
			$.debug('attempting to fetch '+url);

			$.ajax({
				url: url,
				cache:true,
				dataType: 'text',
				type: 'GET',
				async:true,
				success:function(prop)
				{
					var endRE = /\\$/;
					var cont = false, key = null, value = null;
					var map = {};
					$.each(prop.split("\n"),function()
					{
						if (cont)
						{
							value+=value.substring(0,value.length-1);
							if (endRE.test(this))
							{
								return;
							}
							map[key]=value;
							cont=false;
							key=value=null;
							return;
						}
						var line = $.trim(this);
						if (line == '' || line.charAt(0)=='#')
						{
							return;
						}
						var tokens = line.split('=');
						var k = $.trim(tokens[0]);
						if (k.charAt(0)=='#') return;
						var v = $.trim(tokens.length > 1 ? tokens[1] : '');
						if (endRE.test(v))
						{
							cont = true;
							key = k;
							value = v.substring(0,v.length-1);
							return;
						}
						map[k]=v;
					});
					bundles[lang]=map;
					AppC.beforeCompile(function()
					{
						$('[@id]').localize(lang);
					});
					$(document).trigger('localized',lang);
				}
			});
		}
		catch (E)
		{
			if (lang.indexOf('-') > 0)
			{
				// if you specify en-US, fall back to just en if not found
				return AppC.locale(lang.split('-')[0]);
			}
			$.error('error loading language bundle for language = '+lang+', Exception = '+E);
		}
	}
	else
	{
		AppC.beforeCompile(function()
		{
			$('[@id]').localize(lang);
		});
		$(document).trigger('localized',lang);
	}
}

//
// plugin to localize one or more elements for a given language (or the default if not passed)
//
$.fn.localize = function(lang)
{
	if (this.length > 1)
	{
		$.each(this,function(){
			$(this).localize(lang);
		});
		return this;
	}
	var id = $(this).attr('id');
	if (id != null)
	{
		var m = bundles[lang||AppC.currentLocale];
		if (m)
		{
			var value = m[id];
			if (value)
			{
				var el = $(this).get(0);
				switch(el.nodeName)
				{
					//FIXME - support more obvious types
					case 'INPUT':
					case 'BUTTON':
					{
						$(this).val(value);
						break;
					}
					default:
					{
						$(this).html(value);
						break;
					}
				}
			}
		}
	}
	return this;
};


//
// if set, will attempt to auto load the localization bundle based on the users locale setting
//
if (AppC.config.auto_locale)
{
	AppC.locale(AppC.currentLocale);	
}

//--------------------------------------------------------------------------------

/* move.js */

$.fn.move = function(params)
{
	$.each(this,function()
	{
		$(this).css('position','relative').animate({
			left:params.x||0,
			top:params.y||0
		},params.duration||1000);
	});
}	

	


//--------------------------------------------------------------------------------

/* on.js */


$.fn.on = function(value,state)
{
	var el = this;
	var isFn = typeof(state)=='function';
	if (isFn)
	{
		// if it's a function, we just fact the action so it'll parse
		// this means you can pass in a function and when the condition
		// is triggered, we'll call you function 
		value = value + ' then script[true]';
	}
	var expr = App.parseExpression(value);
	$.each(expr,function()
	{
		var p = App.extractParameters(this[2]);
		var ep = isFn ? null : this[3] ? App.extractParameters(this[3]) : null;
		var param = 
		{
			cond: this[1],
			action: isFn ? state : p.name,
			actionParams: isFn ? null : p.params,
			elseAction: ep ? ep.name : null,
			elseActionParams: ep ? ep.params : null,
			delay: this[4],
			ifCond: this[5],
			state: state
		};
		App.processCond(el,param);
	});
	return this;
};


//--------------------------------------------------------------------------------

/* opacity.js */

regCSSAction('opacity',function(params)
{
	var value = (typeof(params)=='string'||typeof(params)=='number') ? params : (params ? params.value : null);
	if (typeof(value)=='undefined' && params)
	{
		for (var p in params)
		{
			if (p=='id' || p=='source') continue;
			value = params[p];
			break;
		}
	}
	if (typeof(value)=='undefined') value=1;
	if (typeof(value)!='number')
	{
		value = parseFloat(value);
	}
	this.css('opacity',(value==1||value=='') ? '1.0' : (value < 0.00001) ? 0 : value);
});



//--------------------------------------------------------------------------------

/* pubsub.js */


var subs = {local:[], remote:[]};
var re = /^(l|local|both|r|remote|\*)\:(.*)$/;
var localRe = /^l|local|both|\*/;
var pubdebug = AppC.params.debug=='2' || AppC.params.debug==true;
var queue = [];
var remoteDisabled = true;
var queueInit = false;
var processingQueue = false;

$.fn.sub = function(name,fn,params)
{
	var p = App.extractParameters(name);
	params = params || p.params;
	name = App.normalizePub(p.name);

	var regexp = null;
	var m = re.exec(name);
	var type = m[2];
	
	if (type.charAt(0)=='~')
	{
		type = type.substring(1);
		regexp = new RegExp(type);
	}

	// unsub, but only if not for document
	if (this.get(0)!=document)
	{
		$(this).trash(function()
		{
			$(this).unsub(name,fn);
		});
	}
	
	$.debug('subscribing type='+type+', regexp='+regexp);
	
	if (localRe.test(m[1]))
	{
		subs.local.push({scope:this,fn:fn,name:type,params:params,regexp:regexp});
	}
	else
	{
		subs.remote.push({scope:this,fn:fn,name:type,params:params,regexp:regexp});
	}
	return this;
};

$.fn.unsub = function(name,fn)
{
	name = App.normalizePub(name);
	var match = re.exec(name);
	if (match)
	{
		var array = match[1]=='remote' ? subs.remote : subs.local;
		var type = match[2];
		var found = [];
		for (var c=0;c<array.length;c++)
		{
			var entry = array[c];
			if (entry.name == type && entry.fn == fn)
			{
				found.push(c);
			}
		}
		for (var c=0;c<found.length;c++)
		{
			array.splice(found[c],1);
		}
	}
};

App.normalizePub = function(name)
{
	var idx = name.indexOf('[');
	if (idx > 0)
	{
		name = name.substring(0,idx);
	}

	var m = re.exec(name);
	if (!m)
	{
		return 'both:'+name;
	}
	switch(m[1])
	{
		case 'l':
		case 'local':
			return 'local:'+m[2];
		case 'r':
		case 'remote':
			return 'remote:'+m[2];
	}
	return 'both:'+m[2];
};

App.pubQueue = function(name,data,local,scope,version)
{
	if (pubdebug) $.info('publish '+(local?'l:':'r:')+name+' with '+$.toJSON(data)+', local:'+subs.local.length+'/remote:'+subs.remote.length);
	
	// optimize if no listeners at all
	if (subs.local.length == 0 && subs.remote.length == 0) return;

 	queue.push({
		data:data||{},
		name:name,
		local:local,
		scope:scope,
		version:version
	});

	if (remoteDisabled && queueInit && !processingQueue)
	{
		processQueue();		
	}
};

$.fn.pub = function(name,data,scope,version)
{
	var p = App.extractParameters(name,data||{});
	data = data || p.params;
	name = p.name;
	var m = re.exec(name);
	var isLocal = localRe.test(m[1]);
	data = data || {};
	App.getFieldsetData(this,data);

	if (isLocal && !data.event) data.event = {id:$(this).attr('id')};
	if (!isLocal && data.event) delete data.event;
	
	App.pubQueue(m[2],data,isLocal,scope,version);

	return this;
};


//
// wait until we've initially compiled before we 
// start delivering events -- will queue until then
//
$(document).bind('compiled',function()
{
	queueInit=true;
	processQueue();
});

App.regCond(re,function(meta)
{
	$(this).sub(meta.cond,function(data,scope,version,name,direction,params)
	{
		if (App.parseConditionCondition(params,data))
		{
			App.triggerAction(this,data,meta);
		}
		else
		{
			App.triggerElseAction(this,data,meta);
		}
	});
});

App.regAction(/^(l|local|both|\*|r|remote)\:/,function(params,action)
{
	// TODO: parse our params from action
	$(this).pub(action,params);
});

function deliverRemoteMessages(msgs)
{
	$.debug('remote messages received = '+$.toJSON(msgs));
	$.each(msgs,function()
	{
		var msg = this;
		$.each(subs.remote,function()
		{
			if ((this.regexp && this.regexp.test(msg.name)) || (!this.regexp && this.name == msg.name))
			{
				this.fn.apply(this.scope,[msg.data,msg.scope,msg.version,msg.name,'remote']);
			}
		});
	});
}

var instanceid = AppC.params.instanceid || App.MD5.hex_md5(String(new Date().getTime()) + String(Math.round(9999*Math.random())));

function getServiceURL()
{
    var token = App.MD5.hex_md5(sessionCookie + instanceid);
    return serviceBroker + "?instanceid="+instanceid+'&auth='+token+'&ts='+String(new Date().getTime());
}

var marshallers={};
var currentRequestId = 0;

marshallers['xml/json']=function(q)
{
    var requestid = currentRequestId++;
	var xml = '';
    var time = new Date;
    var timestamp = time.getTime();
    xml = "<?xml version='1.0' encoding='UTF-8'?>\n";
    var tz = time.getTimezoneOffset()/60;
    var idleMs = 0;
    xml += "<request version='1.0' idle='" + idleMs + "' timestamp='"+timestamp+"' tz='"+tz+"'>\n";
	$.each(q,function()
	{
	    xml += "<message requestid='" + requestid + "' type='" + this.name + "' datatype='JSON' scope='"+(this.scope||'appcelerator')+"' version='"+(this.version||'1.0')+"'>";
	    xml += '<![CDATA[' + $.toJSON(this.data) + ']]>';
	    xml += "</message>";
	});
	xml += '</request>';
	
	$.ajax({
		type:'POST',
		url:getServiceURL(),
		data:xml,
		cache:false,
		dataType:'xml',
		contentType:'text/xml',
		success:function(xml)
		{
	        var children = xml.documentElement.childNodes;
	        if (children && children.length > 0)
	        {
	            var msgs = [];
	            for (var c = 0; c < children.length; c++)
	            {
	                var child = children.item(c);
	                if (child.nodeType == 1)
	                {
	                    var requestid = child.getAttribute('requestid');
                        var type = child.getAttribute('type');
                        var datatype = child.getAttribute('datatype');
                        var scope = child.getAttribute('scope') || 'appcelerator';
                        var text = $.domText(child);
                        var data = $.evalJSON(text);
						msgs.push({
							name:type,
							data:data,
							datatype:datatype,
							scope:scope,
							requestid:requestid
						});
	                }
	            }
				deliverRemoteMessages(msgs);
	        }
		}
	});
};

marshallers['application/json']=function(q)
{
    var requestid = this.currentRequestId++;
    var request = {};
    var time = new Date;
	var json = {
		timestamp: time.getTime()  + (time.getTimezoneOffset()*60*1000),
		version: '1.0', //protocol version
		messages: []
	};
	
    for (var c = 0,len = q.length; c < len; c++)
    {
		var e = q[c];
		json.messages.push({
			type: e.name,
			data: e.data,
			version: e.version,
			scope: e.scope
		});
    }

	$.ajax({
		type:'POST',
		url:getServiceURL(),
		data:$.toJSON(json),
		cache:false,
		dataType:'json',
		contentType:'application/json',
		success:function(result)
		{
	        var msgs = [];
	        for (var c = 0; c < result.messages.length; c++)
	        {
                var message = result.messages[c];
                var type = message.type
                var datatype = message.datatype;
                var scope = message.scope || 'appcelerator';
                var data = message.data;
                message.datatype = 'JSON'; // always JSON
                msgs.push({
					name:type,
					data:data,
					datatype:datatype,
					scope:scope
				});
	        }
			deliverRemoteMessages(msgs);
		}
	});
};

function remoteDelivery(q)
{
	if (!remoteDisabled) marshaller(q);
}

function processQueue()
{
	if (queue.length < 1) return;
	
	var rq = remoteDisabled ? null : [];
	processingQueue = true;
	
	// process message queue
	for (var i=0;i<queue.length;i++)
	{
		var a = queue[i].local ? subs.local : subs.remote;
		var name = queue[i].name;
		var data = queue[i].data;
		var scope = queue[i].scope || 'appcelerator';
		var version = queue[i].version || '1.0';
		var direction = queue[i].local ? 'local' : 'remote';
		
		// process subs
		for (var j=0;j<a.length;j++)
		{
			if ((a[j].regexp && a[j].regexp.test(name)) || (!a[j].regexp && a[j].name == name))
			{
				if (pubdebug) $.info('dispatching '+(queue[i].local?'l:':'r:')+name+' to '+a[j].scope.attr('id'));
				a[j].scope.direction = direction;
				a[j].scope.version = version;
				a[j].scope.scope = scope;
				a[j].scope.name = name;
				a[j].fn.apply(a[j].scope,[data,scope,version,name,direction,a[j].params]);
			}
		}
	}

	if (rq && rq.length > 0) remoteDelivery(rq);
	
	// clear the queue
	queue = [];
	processingQueue = false;
}

var queueTimer;
var serviceBroker;
var marshaller;
var sessionCookie;

function startDelivery(config)
{
	$.debug('remote config => ' + $.toJSON(config));

	remoteDisabled = false;
	var sb = config.servicebroker;

	if (!sb || sb.disabled=='true')
	{
		remoteDisabled = true;
		$.info('Appcelerator remote services disabled');
		return;
	}
	serviceBroker = sb.value;
	if (!serviceBroker)
	{
		$.error("Error loading service broker! not specified in appcelerator.xml");
		remoteDisabled=true;
		return;
	}
	if (!remoteDisabled)
	{
		marshaller = marshallers[sb.marshaller];
		if (!marshaller)
		{
			$.error("Error loading marshaller = "+sb.marshaller);
			remoteDisabled=true;
			return;
		}
		
        var cookieName = config['sessionid'].value || 'unknown_cookie_name';
        var cookieValue = $.cookie(cookieName);
        
        if (!cookieValue)
        {
			$.ajax({
				type:'GET',
				url:serviceBroker,
				data:"initial=1",
				async:true,
				success:function()
				{
			        sessionCookie = $.cookie(cookieName);
					$.debug('sessionCookie = '+sessionCookie);
					queueTimer = setInterval(processQueue,10);
				}
			});
        }
		else
		{
			sessionCookie = cookieValue;
			$.debug('sessionCookie = '+sessionCookie);
			queueTimer = setInterval(processQueue,10);
		}
	}
}

//
// fetch our appcelerator.xml
//
try
{
	AppC.serverConfig = {};
	// only fetch XML if you are not running from file URL
	if (AppC.docRoot.indexOf('file:/')<0)
	{
		var url = AppC.docRoot+'appcelerator.xml';
		$.ajax({
			async:true,
			cache:true,
			dataType:'xml',
			type:'GET',
			url:url,
			success:function(data)
			{
				var re = /@\{(.*?)\}/g;
				var map = {rootPath:AppC.docRoot};
				var children = data.documentElement.childNodes;
				for (var c=0;c<children.length;c++)
				{
					var child = children[c];
					if (child.nodeType == 1)
					{
						var service = child.nodeName.toLowerCase();
						var config = {};
						var path = $.domText(child);
						var template = AppC.compileTemplate(path,false,null,re);
						for (var x=0;x<child.attributes.length;x++)
						{
							var attr = child.attributes[x];
							config[attr.name]=attr.value;
						}
						config.value = template(map);
						AppC.serverConfig[service]=config;
					}
				}
				$(document).trigger('serverConfig',AppC.serverConfig);
				startDelivery(AppC.serverConfig);
			},
			error:function(xhr,text,error)
			{
				$.error('error retrieving appcelerator.xml, remote services are disabled. error = '+text);
			}
		});
	}
}
catch(e)
{
	$.error('error loading '+docRoot+'appcelerator.xml, remote services are disabled. error = '+e);
}

//--------------------------------------------------------------------------------

/* pulsate.js */

$.fn.pulsate = function(count)
{
	for (var i=0;i<(count||4);i++)
	{
		this.fadeOut('fast').fadeIn('fast');
	}
	return this;
};

regCSSAction('pulsate',function(params)
{
	return getTarget(params,this).pulsate(params.count);
});

//--------------------------------------------------------------------------------

/* reset.js */

$.each(['clear','reset','clearform'],function()
{
	$.fn[this] = function()
	{
		$.each(this,function()
		{
			var target = $(this);
			var tag = App.getTagname(this);

			switch (tag)
			{
				case 'a':
				{
				    target.attr('href','#');
					break;
				}
				case 'form':
				{
					this.reset();
					target.find(":input").revalidate();
		            break;
				}
				default:
				{
					target.val('');
					break;
				}
			}		
		});
		return this;
	}
});



//--------------------------------------------------------------------------------

/* script.js */


$.each(['script','function','javascript'],function()
{
	$.fn[this]=function(code,scope)
	{
		scope = scope || {};
		var js = code;
		if (typeof(js)=='string')
		{
			js = $.toFunction(js,true);
		}
		else if (code.nodeType==1)
		{
			js = $.toFunction($(code).html(),true);
		}
		else if (typeof(code.jquery)=='string')
		{
			js = $.toFunction(code.get(0).html(),true);
		}
		else
		{
			throw "I don't know what this object is: "+(typeof(code))+" for "+$(this).attr('id');
		}
		scope.window = window;
		this.result = js.call(scope);
		return this;
	};
});


//--------------------------------------------------------------------------------

/* scrollto.js */

/**
 * jQuery.ScrollTo
 * Copyright (c) 2007-2008 Ariel Flesler - aflesler(at)gmail(dot)com | http://flesler.blogspot.com
 * Dual licensed under MIT and GPL.
 * Date: 9/11/2008
 *
 * @projectDescription Easy element scrolling using jQuery.
 * http://flesler.blogspot.com/2007/10/jqueryscrollto.html
 * Tested with jQuery 1.2.6. On FF 2/3, IE 6/7, Opera 9.2/5 and Safari 3. on Windows.
 *
 * @author Ariel Flesler
 * @version 1.4
 *
 * @id jQuery.scrollTo
 * @id jQuery.fn.scrollTo
 * @param {String, Number, DOMElement, jQuery, Object} target Where to scroll the matched elements.
 *	  The different options for target are:
 *		- A number position (will be applied to all axes).
 *		- A string position ('44', '100px', '+=90', etc ) will be applied to all axes
 *		- A jQuery/DOM element ( logically, child of the element to scroll )
 *		- A string selector, that will be relative to the element to scroll ( 'li:eq(2)', etc )
 *		- A hash { top:x, left:y }, x and y can be any kind of number/string like above.
 * @param {Number} duration The OVERALL length of the animation, this argument can be the settings object instead.
 * @param {Object,Function} settings Optional set of settings or the onAfter callback.
 *	 @option {String} axis Which axis must be scrolled, use 'x', 'y', 'xy' or 'yx'.
 *	 @option {Number} duration The OVERALL length of the animation.
 *	 @option {String} easing The easing method for the animation.
 *	 @option {Boolean} margin If true, the margin of the target element will be deducted from the final position.
 *	 @option {Object, Number} offset Add/deduct from the end position. One number for both axes or { top:x, left:y }.
 *	 @option {Object, Number} over Add/deduct the height/width multiplied by 'over', can be { top:x, left:y } when using both axes.
 *	 @option {Boolean} queue If true, and both axis are given, the 2nd axis will only be animated after the first one ends.
 *	 @option {Function} onAfter Function to be called after the scrolling ends. 
 *	 @option {Function} onAfterFirst If queuing is activated, this function will be called after the first scrolling ends.
 * @return {jQuery} Returns the same jQuery object, for chaining.
 *
 * @desc Scroll to a fixed position
 * @example $('div').scrollTo( 340 );
 *
 * @desc Scroll relatively to the actual position
 * @example $('div').scrollTo( '+=340px', { axis:'y' } );
 *
 * @dec Scroll using a selector (relative to the scrolled element)
 * @example $('div').scrollTo( 'p.paragraph:eq(2)', 500, { easing:'swing', queue:true, axis:'xy' } );
 *
 * @ Scroll to a DOM element (same for jQuery object)
 * @example var second_child = document.getElementById('container').firstChild.nextSibling;
 *			$('#container').scrollTo( second_child, { duration:500, axis:'x', onAfter:function(){
 *				alert('scrolled!!');																   
 *			}});
 *
 * @desc Scroll on both axes, to different values
 * @example $('div').scrollTo( { top: 300, left:'+=200' }, { axis:'xy', offset:-20 } );
 */
;(function( $ ){
	
	var $scrollTo = $.scrollTo = function( target, duration, settings ){
		$(window).scrollTo( target, duration, settings );
	};

	$scrollTo.defaults = {
		axis:'xy',
		duration:1
	};

	// Returns the element that needs to be animated to scroll the window.
	// Kept for backwards compatibility (specially for localScroll & serialScroll)
	$scrollTo.window = function( scope ){
		return $(window).scrollable();
	};

	// Hack, hack, hack... stay away!
	// Returns the real elements to scroll (supports window/iframes, documents and regular nodes)
	$.fn.scrollable = function(){
		return this.map(function(){
			// Just store it, we might need it
			var win = this.parentWindow || this.defaultView,
				// If it's a document, get its iframe or the window if it's THE document
				elem = this.nodeName == '#document' ? win.frameElement || win : this,
				// Get the corresponding document
				doc = elem.contentDocument || (elem.contentWindow || elem).document,
				isWin = elem.setInterval;

			return elem.nodeName == 'IFRAME' || isWin && $.browser.safari ? doc.body
				: isWin ? doc.documentElement
				: this;
		});
	};

	$.fn.scrollTo = function( target, duration, settings ){
		if( typeof duration == 'object' ){
			settings = duration;
			duration = 0;
		}
		if( typeof settings == 'function' )
			settings = { onAfter:settings };
			
		settings = $.extend( {}, $scrollTo.defaults, settings );
		// Speed is still recognized for backwards compatibility
		duration = duration || settings.speed || settings.duration;
		// Make sure the settings are given right
		settings.queue = settings.queue && settings.axis.length > 1;
		
		if( settings.queue )
			// Let's keep the overall duration
			duration /= 2;
		settings.offset = both( settings.offset );
		settings.over = both( settings.over );

		return this.scrollable().each(function(){
			var elem = this,
				$elem = $(elem),
				targ = target, toff, attr = {},
				win = $elem.is('html,body');

			switch( typeof targ ){
				// A number will pass the regex
				case 'number':
				case 'string':
					if( /^([+-]=)?\d+(px)?$/.test(targ) ){
						targ = both( targ );
						// We are done
						break;
					}
					// Relative selector, no break!
					targ = $(targ,this);
				case 'object':
					// DOMElement / jQuery
					if( targ.is || targ.style )
						// Get the real position of the target 
						toff = (targ = $(targ)).offset();
			}
			$.each( settings.axis.split(''), function( i, axis ){
				var Pos	= axis == 'x' ? 'Left' : 'Top',
					pos = Pos.toLowerCase(),
					key = 'scroll' + Pos,
					old = elem[key],
					Dim = axis == 'x' ? 'Width' : 'Height',
					dim = Dim.toLowerCase();

				if( toff ){// jQuery / DOMElement
					attr[key] = toff[pos] + ( win ? 0 : old - $elem.offset()[pos] );

					// If it's a dom element, reduce the margin
					if( settings.margin ){
						attr[key] -= parseInt(targ.css('margin'+Pos)) || 0;
						attr[key] -= parseInt(targ.css('border'+Pos+'Width')) || 0;
					}
					
					attr[key] += settings.offset[pos] || 0;
					
					if( settings.over[pos] )
						// Scroll to a fraction of its width/height
						attr[key] += targ[dim]() * settings.over[pos];
				}else
					attr[key] = targ[pos];

				// Number or 'number'
				if( /^\d+$/.test(attr[key]) )
					// Check the limits
					attr[key] = attr[key] <= 0 ? 0 : Math.min( attr[key], max(Dim) );

				// Queueing axes
				if( !i && settings.queue ){
					// Don't waste time animating, if there's no need.
					if( old != attr[key] )
						// Intermediate animation
						animate( settings.onAfterFirst );
					// Don't animate this axis again in the next iteration.
					delete attr[key];
				}
			});			
			animate( settings.onAfter );			

			function animate( callback ){
				$elem.animate( attr, duration, settings.easing, callback && function(){
					callback.call(this, target, settings);
				});
			};
			function max( Dim ){
				var attr ='scroll'+Dim,
					doc = elem.ownerDocument;
				
				return win
						? Math.max( doc.documentElement[attr], doc.body[attr]  )
						: elem[attr];
			};
		}).end();
	};

	function both( val ){
		return typeof val == 'object' ? val : { top:val, left:val };
	};

})( jQuery );



//--------------------------------------------------------------------------------

/* select_option.js */

App.regAction(evtRegex('selectOption'),function(params)
{
	$.error('not implemented')
});



//--------------------------------------------------------------------------------

/* set.js */

var cssProperties = [
	'border', 'padding', 'margin', 'color', 'cursor', 'font', 'fontFamily', 'visibility', 'position', 'overflow', 'filter', 'display', 
	'backgroundColor', 'backgroundPosition', 'backgroundAttachment', 'borderBottomColor', 'borderBottomStyle', 
	'borderBottomWidth', 'borderLeftColor', 'borderLeftStyle', 'borderLeftWidth',
	'borderRightColor', 'borderRightStyle', 'borderRightWidth', 'borderSpacing',
	'borderTopColor', 'borderTopStyle', 'borderTopWidth', 'bottom', 'clip', 'color',
	'fontSize', 'fontWeight', 'height', 'left', 'letterSpacing', 'lineHeight',
	'marginBottom', 'marginLeft', 'marginRight', 'marginTop', 'markerOffset', 'maxHeight',
	'maxWidth', 'minHeight', 'minWidth', 'opacity', 'outlineColor', 'outlineOffset',
	'outlineWidth', 'paddingBottom', 'paddingLeft', 'paddingRight', 'paddingTop',
	'right', 'textIndent', 'top', 'width', 'wordSpacing', 'zIndex'];


$.fn.set = function(params)
{
	params = convertParams(params);
	
	$.each(this,function()
	{
		var target = $(this);
		var tag = App.getTagname(this);
		
		for (var p in params)
		{
			switch(p)
			{
				case 'id':
				case 'event':
				{
					break;
				}
				case 'class':
				case 'className':
				{
					target.attr('class',params[p]);
					break;
				}
				case 'style':
				{
					//ex: set[style=foo:bar;]
					$.each(params[p].split(';'),function()
					{
						var t = this.split(':');
						if (t && t.length > 1)
						{
							target.css(t[0],t[1]);
							return;
						}
					});
					break;
				}
				case 'href':
				{
					switch(tag)
					{
						case 'a':
						case 'link':
						{
							target.attr('href',URI.absolutizeURI(params[p],AppC.docRoot));
							break;
						}
					}
				}
				case 'src':
				{
					switch(tag)
					{
						case 'iframe':
						case 'script':
						case 'image':
						{
							var onload = target.attr('onloaded');
						    if (onload)
						    {
								var done = false;
								el.onload = el.onreadystatechange = function()
								{
									if ( !done && (!this.readyState || this.readyState == "loaded" || this.readyState == "complete"))
									{
										done = true;
										target.pub(onload,{source:{id:el.id}},target.attr('scope'));
										el.onload = null;
									}
								}
					        }
							if (AppC.UA.opera)
							{
	    				        el.location.href = URI.absolutizeURI(params[p],AppC.docRoot);
							}
							else
							{
								el.src = URI.absolutizeURI(params[p],AppC.docRoot);
							}
							break;
						}
						default:
						{
							target.attr(p,params[p]);
							break;
						}
					}
					break;
				}

				default:
				{
					// check to see if a css property
					if (cssProperties.indexOf(p)!=-1 || cssProperties.indexOf($.camel(p))!=-1)
					{
						target.css(p,params[p]);
					}
					// set it as an attribute
					else
					{
						target.attr(p,params[p]);
					}
				}
			}
		}
	});
};



//--------------------------------------------------------------------------------

/* srcexpr.js */


$.fn.srcexpr = function(value)
{
	var srcvalue = eval($.unescapeXML(value));
	if (AppC.UA.IE6)
	{
		//FIXME
		// img.onload = function()
		// {
		// 	img.addBehavior(AppC.compRoot + '/images/appcelerator/iepngfix.htc');
		// };
	}
	$(this).get(0).src = srcvalue;
	return this;
};
//--------------------------------------------------------------------------------

/* statemachine.js */

App.regAction(evtRegex('statechange'),function(params)
{
	$.error('not implementing');
});


//--------------------------------------------------------------------------------

/* test.js */


var testCases = [];
var testPassed = 0;
var testFailed = 0;
var testErrored = 0;
var testListener = null;

$.fn.assert = function()
{
	var passed = true, error = null, stop = false, message = String(arguments[0]), expr = arguments[0];
	if (typeof(arguments[0])=='string' && arguments.length==2)
	{
		// first is a message explanation of test failure and 2nd is the assertion
		expr = arguments[1];
	}
	var type = typeof(expr);
	if (type=='boolean')
	{
		passed = expr;
		stop = true;
	}
	else if (type=='string')
	{
		expr = $.toFunction(expr);
	}
	else
	{
		passed = typeof(expr)!='undefined' ? true : false;
		stop = true;
	}
	if (!stop)
	{
		$.each(this,function()
		{
			try
			{
				var result = expr.call($(this));
				if (result!==true)
				{
					passed = false;
					return false;
				}
			}
			catch(E)
			{
				passed=false;
				error = E;
				return false;
			}
		});
	}
	var result = {
		passed: passed,
		expr: String(expr),
		message: message,
		error: error
	};
	testCases.push(result);
	if (passed) testPassed++;
	if (error) testErrored++;
	if (!passed && !error) testFailed++;
	if (testListener) testListener.result(result);
	return this;
};

$.fn.assertEnabled = function()
{
	var passed = true;
	$.each(this,function()
	{
		var v = $(this).attr('disabled');
		if (v)
		{
			passed=false;
			return false;
		}
	});
	return this.assert(passed);
};

$.fn.assertDisabled = function()
{
	var passed = true;
	$.each(this,function()
	{
		var v = $(this).attr('disabled');
		if (!v)
		{
			passed=false;
			return false;
		}
	});
	return this.assert(passed);
};

$.fn.assertCSS = function(key,val)
{
	var passed = true;
	$.each(this,function()
	{
		var v = $(this).css(key);
		if (v!=val)
		{
			passed=false;
			return false;
		}
	});
	return this.assert(passed);
};
$.fn.assertClass = function(className)
{
	var passed = true;
	$.each(this,function()
	{
		if ($(this).hasClass(className)==false)
		{
			passed=false;
			return false;
		}
	});
	return this.assert(passed)
};
$.fn.assertAttr = function(attr)
{
	var passed = true;
	$.each(this,function()
	{
		if (!$(this).attr(attr))
		{
			passed=false;
			return false;
		}
	});
	return this.assert(passed)
};

$.fn.assertValid = function()
{
	var passed = true;
	$.each(this,function()
	{
		var v = $(this).data('validatorResult');
		if (!v)
		{
			passed=false;
			return false;
		}
	});
	return this.assert(passed);
};

$.fn.assertInvalid = function()
{
	var passed = true;
	$.each(this,function()
	{
		var v = $(this).data('validatorResult');
		if (v)
		{
			passed=false;
			return false;
		}
	});
	return this.assert(passed);
};

var oldPub = App.pubQueue;
var lastPubType = null, lastPubData = null;

// re-define to allow us to remember the last message and data payload
// for a publish so we can assert against it
App.pubQueue = function(name,data,local,scope,version)
{
	lastPubType = (local ? 'local' : 'remote') + ':' + name;
	lastPubData = data;
	return oldPub.apply(this,arguments);
};

$.fn.assertPub = function(name,data)
{
	name = App.normalizePub(name);
	if (typeof(data)=='undefined')
	{
		return this.assert(name+" was not correct. expected: "+name+", was: "+lastPubType,lastPubType === name);
	}
	if (name!==lastPubType)
	{
		return this.assert(name+" was not correct. expected: "+name+", was: "+lastPubType,name===lastPubType);
	}
	if (!lastPubData)
	{
		return this.assert(name+" missing data payload: "+$.toJSON(data),false);
	}
	for (var key in data)
	{
		var v1 = lastPubData[key];
		var v2 = data[key];
		if (v1!==v2)
		{
			return this.assert(name+" has incorrect data payload entry for key: "+key+", expected: "+v2+", was: "+v1,false);
		}
	}
	return this.assert(name,true);
};

var TestGuard = function(timeout,fn,cb)
{
	var count = 0;
	var done = false;
	var timer = setTimeout(function()
	{
		if (count!=0)
		{
			done=true;
			$(document).assert('test failed because it timed out',false);
			if (cb.timeout) cb.timeout();
			$.error("test timed out");
			fn();
		}
	},timeout);
	this.begin = function()
	{
		if (done) return;
		count++;
		if (cb && cb.begin) cb.begin(count);
	}
	this.end = function()
	{
		if (done) return;
		count--;
		if (cb && cb.end) cb.end(count);
		if (count == 0)
		{
			clearTimeout(timer);
			done = true;
			timer = null;
			fn();
		}
	}
	this.assert = function(a,b)
	{
		if (done) return;
		return $(document).assert(a,b);
	}
	this.assertPub = function(name,data)
	{
		if (done) return;
		return $(document).assertPub(name,data);
	}
};

AppC.runTests = function(timeout,begin,end,cb)
{
	var timeout = typeof(timeout)!='number' ? 10000 : timeout;
	begin = typeof(timeout)!='number' ? timeout : begin;
	end = typeof(timeout)!='number' ? begin : end;
	cb = typeof(timeout)!='number' ? end : cb;
	testCases = [];
	testPassed = 0;
	testFailed = 0;
	testErrored = 0;
	testListener = cb;
	var complete = function()
	{
		if (end)
		{
			end({
				passed: testPassed,
				failed: testFailed,
				errored: testErrored,
				tests: testCases
			});
		}
		else
		{
			var count = testPassed + testFailed + testErrored;
			var msg = 'TEST RESULTS: ' + count + ' test' + (count > 1 ? 's' : '') + ' executed: ';
			if (testFailed > 0 || testErrored > 0)
			{
				var r = [];
				if (testFailed) r.push(testFailed+' failed');
				if (testErrored)
				{
					r.push(testErrored+' errored');
				}
				r.push(testPassed+' passed');
				msg+=r.join(', ');
			}
			else
			{
				msg+='All Passed! Now go take a nice break.';
			}
			$.info(msg);
			if (testFailed || testErrored)
			{
				$.each(testCases,function()
				{
					if (this.error)
					{
						$.error("ERROR: "+this.error+", message: "+this.message);
					}
					else
					{
						$.error("FAILED: "+this.expr+", message: "+this.message);
					}
				});
			}
		}
	};
	var guard = new TestGuard(timeout,complete,cb);
	guard.begin();
	begin(guard);
	guard.end();
};


//--------------------------------------------------------------------------------

/* toggle.js */

$.fn.toggle = function(params)
{
	params = convertParams(params);
	$.each(this,function()
	{
		var target = $(this);
		for (var p in params)
		{
			switch(p)
			{
				case 'id':
				case 'event':
				{
					break;
				}
				// toggle CSS class
				case 'className':
				case 'class':
				{
					target.toggleClass(params[p])
					break;				
				}
				default:
				{
					// toggle CSS property
					if (cssProperties.indexOf(p)!=-1 || cssProperties.indexOf($.camel(p))!=-1)
					{
						var currentVal = target.css(p)
						var opposites = {'inline':'none', 'block':'none','none':'block','hidden':'visible','visible':'hidden'};					
						var opposite = opposites[currentVal] || '';
						if (currentVal == params[p])
						{
							target.css(p,opposite);	
							break;					
						}
						else
						{
							target.css(p,params[p])
							break;
						}

					}
					// toggle element attribute
					else
					{
						if (target.attr(p))
						{
							target.removeAttr(p)
							break;
						}
						else
						{
							target.attr(p,params[p]);
							break;
						}
					}

				}
			}
		}
	});
	return this;
};

//--------------------------------------------------------------------------------

/* value.js */

$.fn.value = function(object,property,defValue)
{
	var value = $.getNestedProperty(object,property);

	if (!value)
	{
		value = $.getNestedProperty(object,'value');
	}

	if (!value)
	{
		value = defValue||property;
	}
	
	if (this.is(':input'))
	{
		this.val(value);
	}
	else
	{
		this.html(value);
	}
	
	return this;
};


//--------------------------------------------------------------------------------

/* visible.js */

regCSSAction('visible','visibility');

//--------------------------------------------------------------------------------

/* action_adapters.js */

$.each(['add','remove'],function()
{
	var action = this;
	App.regAction(evtRegex(action),function(params,name)
	{
		var target = getTarget(params,this);

		for (var p in params)
		{
			switch(p)
			{
				case 'id':
				case 'event':
				{
					break;
				}
				default:
				{
					target[action](p,params[p]);
					break;
				}
			}
		}
		return this;
	});
});


App.regAction(evtRegex('cookie'),function(params)
{
	$.cookie(params.name,params.value,params);
});

App.regAction(evtRegex('disable'),function(params)
{
	return getTarget(params,this).disable();
});


App.handleBasicEffect = function(obj, action, params)
{	
	// target element
	var target = obj;
	var opts = {}, speed = 0, easing=null;
	
	for (var p in params)
	{
		switch(p)
		{
			case 'event':
			{
				// from built-in events like click
				// and they aren't appropriate to pass in
				continue;
			}
			case 'id':
			{
				target = $('#'+params[p]);
				break;
			}
			case 'speed':
			{
				speed = params[p];
				break;
			}
			case 'easing':
			{
				easing = params[p];
				break;
			}
			default:
			{
				// must make camel properties like background-color read like backgroundColor
				opts[$.camel(p)]=params[p];
				break;
			}
		}
	}
	$.debug('effect='+action+',target='+target.attr('id')+',opts='+$.toJSON(opts)+',speed='+speed+',easing='+easing);
	switch (action)
	{
		case 'animate':
		{
			//position
			var position = target.css('position');
			if (position!='relative' && position!='absolute')
			{
				target.css('position','relative');
			}
			return target.animate(opts,speed,easing);
		}
		case 'fadeTo':
		{
			return target.fadeTo(target,[speed,opts.opacity||opts.to||1.0]);
		}
		default:
		{
			return target[action].apply(target,[speed]);
		}
	}
};

$.fn.appear = function()
{
	$.each(this,function()
	{
		$(this).fadeIn();
	});
	return this;
};

$.fn.fade = function()
{
	$.each(this,function()
	{
		$(this).fadeOut();
	});
	return this;
};


$.each(['show','hide','slideToggle','slideUp','slideDown','fadeIn','fadeOut','fadeTo','animate','appear','fade'],function()
{
	var name = $.string(this);
	App.regAction(evtRegex(name),function(params)
	{
		return App.handleBasicEffect(getTarget(params,this),name,params);
	});
});

App.regAction(evtRegex('enable'),function(params)
{
	return getTarget(params,this).enable();
});

regCSSAction('move',function(params)
{
	return getTarget(params,this).move(params);
});

$.each(['clear','reset','clearform'],function()
{
	var eventName = this;
	App.regAction(evtRegex(eventName),function(params,name,data)
	{
		return getTarget(params,this)[eventName]();
	});
});

$.each(['script','function','javascript'],function()
{
	var type = this;
	App.regAction(evtRegex(this),function(params,name,data)
	{
		if (this.direction)
		{
			params.direction = this.direction;
			params.version = this.version;
			params.type = params.name = this.name;
		}
		return getTarget(params,this)[type](data,params);
	},true);
});

App.regAction(evtRegex('value'),function(params,name,data)
{
	if (this.direction)
	{
		params.direction = this.direction;
		params.version = this.version;
		params.type = params.name = this.name;
	}
	var p = App.extractParameters('['+data+']',params);
	return getTarget(params,this).value(p.params,data);
},true);


App.regAction(evtRegex('bind'),function(params)
{
	var target = getTarget(params,this);
//	var fieldset = target.attr('fieldset');
//	if (!fieldset)
//	{
//		$.error('bind action requires fieldset attribute');
//		return this;
//	}
//	this.find('[fieldset='+fieldset+']').bind(params);
	this.bind(params);

	return this;
});


App.regAction(evtRegex('scrollTo'),function(params)
{
	var scrollTo = 0;
	var duration = 1000;
	var scrollToObj = {};
	var useObj = false;
	var options = {};
	var target = null;
	
	for (var p in params)
	{
		switch(p)
		{
			case 'event':
			{
				break;
			}
			case 'id':
			{
				scrollTo = $('#' + params[p]);
				break;
			}
			case 'duration':
			{
				duration = parseInt(params[p]);
				break;
			}
			case 'axis':
			{
				options.axis = params[p];
				break;
			}
			case 'queue':
			{
				options.queue = params[p];
				break;
			}

			case 'top':
			{
				scrollToObj.top = params[p];
				useObj = true;
				break;
			}
			case 'left':
			{
				scrollToObj.left = params[p];
				useObj = true;
				break;
			}
			case 'target':
			{
				target = params[p];
				break;
			}
			default:
			{
				break;
			}
		}
	}
	if (target != null)
	{
		$('#'+target).scrollTo((useObj==true)?scrollToObj:scrollTo,duration,options);
	}
	else
	{
		$.scrollTo((useObj==true)?scrollToObj:scrollTo,duration,options);				
	}

});


App.regAction(evtRegex('set'),function()
{
	getTarget(params,this).set.apply(this,arguments);
	return this;
});


App.regAction(evtRegex('toggle'),function(params)
{
	return getTarget(params,this).toggle(params);
});

regCSSAction('highlight',function(params)
{
	var bgColor = params['background-color'] || params['backgroundColor'] || '#ffffcc';
	return getTarget(params,this).highlight(bgColor);
});
	

App.reg('on','*',function(value,state)
{
	$(this).on(value,state);
});




//--------------------------------------------------------------------------------

/* tracker.js */

// this will simply ping the remote JS but not do anything with it - used for collecting simple usage data (but no personal information)
// and to help us better refine problems, usage data and make the product better - turn it off by simply
// setting AppC.config['track_stats'] = false after loading your appcelerator JS file
var appuid = $.cookie('appuid');
var staturi = (('https:' == document.location.protocol) ? 'https://s3.amazonaws.com/tracker.appcelerator.org/' : 'http://tracker.appcelerator.org/' ) + 'app.js';
if (!appuid)
{
	appuid = App.UUID.newID();
	$.cookie('appuid',appuid,{expires:31536000000 * 5,path:'/',domain:document.domain});
}

var _onerror = window.onerror, _sending = false;

function TrackStat (evt,extra)
{
	if (AppC.config.track_stats)
	{
		_sending = true;
		var d = new Date().getTime() - (started || new Date).getTime();
		var uri = staturi + '?t='+Number(new Date)+'&dur=' + d + '&evt=' + evt + '&appuid=' + appuid + '&tid=' + started.getTime() + '&' + (extra || ''); 
		// this should never be allowed to fail
		try { $.getScript(uri); } catch (E) {}
		_sending = false;
	}
};
var errorCount = 0;
var errorMax = 5;
window.onerror = function(msg,url,line)
{
	// refuse to track our own errors
	if (_sending) return;
	try
	{
		if (url && url.indexOf(staturi)!=-1)
		{
			// don't let tracker errors be a problem either
			return;
		}
		if (errorCount++ > errorMax)
		{
			// refuse to send more than errorMax
			return;
		}
		// squash this event... an event object not an error and we can safely ignore
		if (!url && !line && typeof(msg)=='object' && typeof(msg.stopPropagation)=='function')
		{
			_sending = false;
			return;
		}
		
	    $.error('generic uncaught error = '+String(msg)+', url = '+url+', line = '+line);
		
		// track app errors to improve common issues
		var s = 'msg=' + encodeURIComponent($.encode64(String(msg))) + '&url='+encodeURIComponent($.encode64(String(url||''))) + '&line='+encodeURIComponent(line||-1);
		
		TrackStat(2,s);
		
		// call next guy in chain if one exists
		if (_onerror)
		{
			_onerror(msg,url,line);
		}
	}
	catch(e)
	{
		$.error('caught error in window.onerror = '+e);
		_sending = false;
		return false;
	}
};
/*** FIXME
// install logging handlers that will help track common app problems
(function()
{
	var oldError = Logger.error;
	var oldFatal = Logger.fatal;
	Logger.error = function(msg)
	{
		var m = (String(Object.isArray(msg) ? msg.join(',') : msg)).encode64();
		var s = 'x-lvl=e&x-msg=' + encodeURIComponent(m);
		Appcelerator.TrackStat(3,s);
		return oldError(msg);
	};
	Logger.fatal = function(msg)
	{
		var m = (String(Object.isArray(msg) ? msg.join(',') : msg)).encode64();
		var s = 'x-lvl=f&x-msg=' + encodeURIComponent(m);
		Appcelerator.TrackStat(3,s);
		return oldFatal(msg);
	};
})();
***/

$(document).bind('compiled',function()
{
	var sendRemote = window.location.href.indexOf('file:/')!=-1 && AppC.config.report_stats;
    var screenHeight = screen.height;
    var screenWidth = screen.width;
    var colorDepth = screen.colorDepth || -1;
	var tz = started.getTimezoneOffset()/60;
	var cookies = [];
	
	$.each((document.cookie||'').split(';'),function()
	{
		var t = this.split('=');
		if (t.length > 0) cookies.push({name:t[0],value:t[1]});
	});
	
	var data = 
	{
		'userAgent': navigator.userAgent,
		'flash': AppC.UA.flash,
		'flashver': AppC.UA.flashVersion,
		'silverlight': AppC.UA.silverlight,
		'silverlightver': AppC.UA.silverlightVersion,
		'gears': AppC.UA.gears,
		'fluid': AppC.UA.fluid,
		'screen': {
		    'height':screenHeight,
		    'width':screenWidth,
		    'color':colorDepth
		 },
		'os': AppC.UA.platform,
		'referrer': document.referrer,
		'path': window.location.href,
		'cookies' : cookies,
		'tz' : tz,
		'uid': appuid
	};
	setTimeout(function()
	{
		if (sendRemote)
		{
		    //
		    // if being loaded from an IFrame - don't do the report
		    //
		    if (window.parent == null || window.parent == window)
		    {
	            $(document).pub('r:appcelerator.status.report',data);
		    }
		}
		var a = 0, s = 0, v = 1, c = null, l = null, svc = null;
	
		c = AppC.serverConfig['aid'];
	    if (c) a = c.value;
	
		c = AppC.serverConfig['sid'];
	    if (c) s = c.value;
	
		c = AppC.serverConfig['language'];
		if (c) l = c.value;
	
		c = AppC.serverConfig['service'];
		if (c) svc = c.value;
		
	    var p = AppC.UA.platform || 'unknown';
		var f = AppC.UA.flashVersion;
		var sic = (AppC.serverConfig['sessionid']||{}).value;
	    var si = sic ? $.cookie(sic) : null;

		var qs = $.toQueryString({
			'wv': String(AppC.Version),
			'v': v,
			'a': a,
			's': s,
			'gg': Number(AppC.UA.gears),
			'fd': Number(AppC.UA.fluid),
			'dm': data.screen.width+','+data.screen.height+','+data.screen.color,
			'p': AppC.UA.platform,
			'tz': tz,
			'fv': data.flashver,
			'sv': data.silverlightver,
			'r': $.encode64(document.referrer||''),
			't': $.encode64(document.title||''),
			'si': si,
			'sct': compileTime,
			'slt': loadTime,
			'bl': window.navigator.language,
			'lng': l,
			'svc': svc,
			'js': $.encode64(jsLocation)
		});
		TrackStat(1,qs);
	},2000 + Math.round(1999*Math.random()));
	
	$(window).unload(function()
	{
		TrackStat(0);
	});
});

//--------------------------------------------------------------------------------

/* epilog.js */


})(jQuery);

//--------------------------------------------------------------------------------
