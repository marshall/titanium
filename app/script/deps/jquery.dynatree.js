/*************************************************************************
	jquery.dynatree.js
	Dynamic tree view control, with support for lazy loading of branches.

	Copyright (c) 2008  Martin Wendt (http://wwWendt.de)
	Licensed under the MIT License (MIT-License.txt)

	A current version and some documentation should be available at
		http://dynatree.googlecode.com/

	Let me know, if you find bugs or improvements (martin at domain wwWendt.de).
	
	$Rev$
	$Date$

 	@depends: jquery.js
 	@depends: ui.core.js
*************************************************************************/

/*************************************************************************
 *	Common functions for extending classes etc.
 *	Borrowed from prototype.js
 */
// TODO: remove Class
var Class = {
	create: function() {
		return function() {
			this.initialize.apply(this, arguments);
		}
	}
}

/*************************************************************************
 *	Debug functions
 */
var _bDebug = false;
_bDebug = true;

function logMsg(msg) {
	// Usage: logMsg("%o was toggled", this);
	// See http://michaelsync.net/2007/09/09/firebug-tutorial-logging-profiling-and-commandline-part-i
	// for details (logInfo, logWarning, logGroup, ...)
	if ( _bDebug  && window.console && window.console.log ) {
		var dt = new Date();
		var tag = dt.getHours()+':'+dt.getMinutes()+':'+dt.getSeconds()+'.'+dt.getMilliseconds();
		arguments[0] = tag + " - " + arguments[0];
		try {
			// Safari gets here, but fails
			window.console.log.apply(this, arguments);
		} catch(e) {
		}
	}
}

/*************************************************************************
 *	DynaTreeNode
 */
var DTNodeStatus_Error   = -1;
var DTNodeStatus_Loading = 1;
var DTNodeStatus_Ok      = 0;

var DynaTreeNode = Class.create();

DynaTreeNode.prototype = {
	initialize: function(tree, data) {
		this.tree    = tree;
		if ( typeof data == 'string' ) {
			this.data = { title: data };
		} else {
			this.data = $.extend({}, $.ui.dynatree.nodedatadefaults, data);
		}
		this.parent = null; // not yet added to a parent
		this.div = null; // not yet created
		this.span = null; // not yet created
		this.aChilds = null; // no subnodes yet
		this.bRead = false; // Lazy content not yet read
		this.bExpanded = false; // Collapsed by default
	},

	toString: function() {
		return "DynaTreeNode '" + this.data.title + "', key=" + this.data.key;
	},

	getInnerHtml: function() {
		var res = '';

		// parent connectors
		var bIsRoot = (this.parent==null);
		var bHideFirstConnector = ( !this.tree.options.rootVisible || !this.tree.options.rootCollapsible );

		var p = this.parent;
		var cache = this.tree.cache;
		while ( p ) {
			if ( ! (bHideFirstConnector && p.parent==null ) )
				res = ( p.isLastSibling() ? cache.tagL_ : cache.tagL_ns) + res ;
			p = p.parent;
		}

		// connector (expanded, expandable or simple)
		if ( bHideFirstConnector && bIsRoot  ) {
			// skip connector
		} else if ( this.aChilds && this.bExpanded  ) {
			res += ( this.isLastSibling() ? cache.tagM_ne : cache.tagM_nes );
		} else if (this.aChilds) {
			res += ( this.isLastSibling() ? cache.tagP_ne : cache.tagP_nes );
		} else if (this.data.isLazy) {
			res += ( this.isLastSibling() ? cache.tagD_ne : cache.tagD_nes );
		} else {
			res += ( this.isLastSibling() ? cache.tagL_ne : cache.tagL_nes );
		}
		// folder or doctype icon
   		if ( this.data && this.data.icon ) {
    		res += '<img src="' + this.tree.options.imagePath + this.data.icon + '" alt="" />';
		} else if ( this.data.isFolder ) {
	    	res += ( this.bExpanded ? cache.tagFld_o : cache.tagFld );
		} else {
	    	res += cache.tagDoc;
		}

		// node name
		var tooltip = ( this.data && typeof this.data.tooltip == 'string' ) ? ' title="' + this.data.tooltip + '"' : '';
		res +=  '<a href="#" ' + tooltip + '>' + this.data.title + '</a>';
		return res;
	},

	render: function (bDeep, bHidden) {
		/*
			Called by
		*/
//		logMsg('render '+this.data.title+', expanded='+this.bExpanded + ', aChilds='+(this.aChilds?this.aChilds.length:'0'));
		// --- create <div><span>..</span></div> tags for this node
		if ( ! this.div ) {
//			logMsg('render: create '+this.data.title+', expanded='+this.bExpanded + ', aChilds='+(this.aChilds?this.aChilds.length:'0'));
			this.div  = document.createElement ('div');
			this.span = document.createElement ('span');
			this.span.dtnode = this;
			if( this.data.key )
				this.span.id = this.tree.options.idPrefix + this.data.key;
			this.span.className = ( this.data.isFolder ) ? this.tree.options.classnames.folder : this.span.className = this.tree.options.classnames.document;

			this.div.appendChild(this.span);
			if ( this.parent )
				this.parent.div.appendChild(this.div);
		}
		// hide root?
		if ( this.parent==null )
			this.span.style.display = ( this.tree.options.rootVisible ? '' : 'none');
		// hide this node, if parent is collapsed
		this.div.style.display = ( this.parent==null || this.parent.bExpanded ? '' : 'none');

		// set node connector images, links and text
		this.span.innerHTML = this.getInnerHtml();

		if ( bDeep && (this.aChilds != null ) && (bHidden || this.bExpanded) ) {
			for (var i=0; i<this.aChilds.length; i++) {
				this.aChilds[i].render (bDeep, bHidden)
			}
		}
	},

	isLastSibling: function() {
		var p = this.parent;
		if ( !p ) return true;
		return p.aChilds[p.aChilds.length-1] === this;
	},

	prevSibling: function() {
		if( !this.parent ) return null;
		var ac = this.parent.aChilds;
		for(var i=1; i<ac.length; i++) // start with 1, so prev(first) = null
			if( ac[i] === this )
				return ac[i-1];
		return null;
	},

	nextSibling: function() {
		if( !this.parent ) return null;
		var ac = this.parent.aChilds;
		for(var i=0; i<ac.length-1; i++) // up to length-2, so next(last) = null
			if( ac[i] === this )
				return ac[i+1];
		return null;
	},

	_setStatusNode: function(data) {
		// Create, modify or remove the status child node (pass 'null', to remove it).
		var firstChild = ( this.aChilds ? this.aChilds[0] : null );
		if( !data ) {
			if ( firstChild ) {
				this.div.removeChild(firstChild.div);
				if( this.aChilds.length == 1 )
					this.aChilds = null;
				else
					this.aChilds.shift();
			}
		} else if ( firstChild ) {
			firstChild.data = data;
			firstChild.render (false, false);
		} else {
			firstChild = this._addChildNode (new DynaTreeNode (this.tree, data));
			firstChild.data.isStatusNode = true;
		}
	},

	setLazyNodeStatus: function (lts) {
		switch( lts ) {
			case DTNodeStatus_Ok:
				this._setStatusNode(null);
				this.bRead = true;
				if( this === this.tree.tnRoot && this.tree.options.focusRoot 
					&& !this.tree.options.rootVisible && this.aChilds.length > 0 ) {
					// special case: using ajaxInit	
					this.aChilds[0].focus();
				} else {
					this.focus();
				}
				break;
			case DTNodeStatus_Loading:
				this._setStatusNode({
					title: this.tree.options.strings.loading,
					icon: "ltWait.gif"
				});
				break;
			case DTNodeStatus_Error:
				this._setStatusNode({
					title: this.tree.options.strings.loadError,
					icon: "ltError.gif"
				});
				break;
			default:
				throw "Bad LazyNodeStatus: '" + lts + "'.";
		}
	},

	_parentList: function(includeRoot, includeSelf) {
		var l = new Array();
		var dtn = includeSelf ? this : this.parent;
		while( dtn ) {
			if( includeRoot || dtn.parent )
				l.unshift(dtn);
			dtn = dtn.parent;
		};
		return l;
	},

	isVisible: function() {
		// Return true, if all parents are expanded.
		var parents = this._parentList(true, false);
		for(var i=0; i<parents.length; i++) 
			if( ! parents[i].bExpanded ) return false;
		return true;
	},

	makeVisible: function() {
		// Make sure, all parents are expanded
		var parents = this._parentList(true, false);
		for(var i=0; i<parents.length; i++) 
			parents[i]._expand(true);
	},

	focus: function() {
//		logMsg("dtnode.focus(): %o", this);
		this.makeVisible();
		$(this.span).find(">a").focus();
	},

	select: function() {
		// Select - but not focus - this node.
//		logMsg("dtnode.select(): %o", this);
		if( this.tree.isDisabled || this.data.isStatusNode )
			return;
		if( this.tree.tnSelected ) {
			if( this.tree.tnSelected === this )
				return;
			this.tree.tnSelected.unselect();
		}
		if( this.tree.options.selectionVisible )
			this.makeVisible();
		this.tree.tnSelected = this;
		$(this.span).addClass(this.tree.options.classnames.selected);
		if ( this.tree.options.onSelect ) // Pass element as 'this' (jQuery convention)
			this.tree.options.onSelect.call(this.span, this);
	},

	unselect: function() {
//		logMsg("dtnode.unselect(): %o", this);
		$(this.span).removeClass(this.tree.options.classnames.selected);
		if( this.tree.tnSelected === this ) {
			this.tree.tnSelected = null;
			if ( this.tree.options.onUnselect )
				this.tree.options.onUnselect.call(this.span, this);
		}
	},

	_expand: function (bExpand) {
//		logMsg("dtnode._expand(%s): %o", bExpand, this);
		if ( this.bExpanded == bExpand )
			return;
		this.bExpanded = bExpand;
		// Auto-collapse mode: collapse all siblings
		if ( this.bExpanded && this.parent && this.tree.options.autoCollapse ) {
			var parents = this._parentList(false, true);
			for(var i=0; i<parents.length; i++) 
				parents[i].collapseSiblings();
		}
		// If current focus is now hidden, focus the first visible parent.
		// TODO: doesn't make sense here(?) we should check if the currently focused node (not <this>) is visible.
		// At the moment, _expand gets only called, after focus was set to <this>.
		if ( ! this.bExpanded && ! this.isVisible() ) { 
			logMsg("Focus became invisible: setting to this.");
			this.focus();
		}
		// If current selection is now hidden, unselect it  
		if( this.tree.options.selectionVisible && this.tree.tnSelected && ! this.tree.tnSelected.isVisible() ) {
			this.tree.tnSelected.unselect();
		}
		// Expanding a lazy node: set 'loading...' and call callback
		if ( bExpand && this.data.isLazy && !this.bRead ) {
			try {
				this.setLazyNodeStatus(DTNodeStatus_Loading);
				if( true == this.tree.options.onLazyRead.call(this.span, this) ) {
					// If function returns 'true', we assume that the loading is done:
					this.setLazyNodeStatus(DTNodeStatus_Ok);
					// Otherwise (i.e. if the loading was started as an asynchronous process)
					// the onLazyRead(dtnode) handler is expected to call dtnode.setLazyNodeStatus(DTNodeStatus_Ok/_Error) when done.
				}
			} catch(e) {
				this.setLazyNodeStatus(DTNodeStatus_Error);
			}
			return;
		}
		// render expanded nodes
		this.render (true, false);
		// we didn't render collapsed nodes, so we have to update the visibility of direct childs
		if ( this.aChilds ) {
			for (var i=0; i<this.aChilds.length; i++) {
				this.aChilds[i].div.style.display = (this.bExpanded ? '' : 'none');
			}
		}
	},

	toggleExpand: function() {
		logMsg('toggleExpand('+this.data.title+')...');
		this._expand ( ! this.bExpanded);
		logMsg('toggleExpand('+this.data.title+') done.');
	},

	collapseSiblings: function() {
		if ( this.parent==null )
			return;
		var ac = this.parent.aChilds;
		for (var i=0; i<ac.length; i++) {
			if ( ac[i] !== this && ac[i].bExpanded )
				ac[i]._expand(false);
		}
	},

	onClick: function(event) {
		/*
		this is the <div> element
		event:.target: <a>
		event.type: 'click'
			.which: 1
			.shiftKey: false, .ctrlKey:  false, . metaKey: false
			.buton: 0
			. currentTargte: div#tree
		*/
//		logMsg("dtnode.onClick(" + event.type + "): dtnode:" + this + ", button:" + event.button + ", which: " + event.which);

//		if( $(event.target).parent(".ui-dynatree-expander").length ) {
		if( $(event.target).hasClass(this.tree.options.classnames.expander) ) {
			// Clicking the [+] icon always expands
			this.toggleExpand();
		} else if ( this.data.isFolder 
			&& this.tree.options.selectExpandsFolders 
			&& (this.aChilds || this.data.isLazy) 
			&& (this.parent != null || this.tree.options.rootCollapsible)
			) {
			// Clicking a non-empty folder, when selectExpandsFolders is on, will expand
			this.toggleExpand();
		} else if ( !this.data.isFolder ) {
			// Clicking a document will select it
			this.select();
		} else {
			// Folders cannot be selected TODO: really?
		}

		this.focus();
		
		// Make sure that clicks stop
		return false;
	},

	onKeypress: function(event) {
		/*
		this is the <div> element
		event:.target: <a>
		event.type: 'keypress'
			.which: 1
			.shiftKey: false, 	.ctrlKey:  false, 	. metaKey: false
			.charCode:  '+':43, '-':45, '*':42, '/':47, ' ':32
			.keyCode:   left:37, right:39, up:38 , down: 40, <Enter>:13
			. currentTargte: div#tree
		*/
//		logMsg("dtnode.onKeyPress(" + event.type + "): dtnode:" + this + ", charCode:" + event.charCode + ", keyCode: " + event.keyCode + ", which: " + event.which);
		var code = ( ! event.charCode ) ? 1000+event.keyCode : event.charCode;
		var handled = true;

		switch( code ) {
			// charCodes:
			case 43: // '+'
				if( !this.bExpanded ) this.toggleExpand();
				break;
			case 45: // '-'
				if( this.bExpanded ) this.toggleExpand();
				break;
			//~ case 42: // '*'
				//~ break;
			//~ case 47: // '/'
				//~ break;
			case 1032: // <space>
			case 32: // <space>
				this.select();
				break;
			// keyCodes
			case 1008: // <backspace>
				if( this.parent )
					this.parent.focus();
				break;
			case 1037: // <left>
				if( this.bExpanded ) {
					this.toggleExpand();
					this.focus();
				} else if( this.parent && (this.tree.options.rootVisible || this.parent.parent) ) {
					this.parent.focus();
				}
				break;
			case 1039: // <right>
				if( !this.bExpanded && (this.aChilds || this.data.isLazy) ) {
					this.toggleExpand();
					this.focus();
				} else if( this.aChilds ) {
					this.aChilds[0].focus();
				}
				break;
			case 1038: // <up>
				var sib = this.prevSibling();
				while( sib && sib.bExpanded )
					sib = sib.aChilds[sib.aChilds.length-1];
				if( !sib && this.parent && (this.tree.options.rootVisible || this.parent.parent) ) 
					sib = this.parent;
				if( sib ) sib.focus();
				break;
			case 1040: // <down>
				var sib;
				if( this.bExpanded ) {
					sib = this.aChilds[0];
				} else {
					var parents = this._parentList(false, true);
					for(var i=parents.length-1; i>=0; i--) { 
						sib = parents[i].nextSibling();
						if( sib ) break;
					}
				}
				if( sib ) sib.focus();
				break;
			//~ case 1013: // <enter>
				//~ this.select();
				//~ break;
			default:
				handled = false;
		}
		if( handled )
			return false;
	},

	onFocus: function(event) {
		// Handles blur and focus events.
//		logMsg("dtnode.onFocus(%o): %o", event, this);
		if ( event.type=="blur" || event.type=="focusout" ) {
			if ( this.tree.options.onBlur ) // Pass element as 'this' (jQuery convention)
				this.tree.options.onBlur.call(this.span, this);
			if( this.tree.tnFocused )
				$(this.tree.tnFocused.span).removeClass(this.tree.options.classnames.focused);
			this.tree.tnFocused = null;
		} else if ( event.type=="focus" || event.type=="focusin") {
			// Fix: sometimes the blur event is not generated
			if( this.tree.tnFocused && this.tree.tnFocused !== this ) { 
				logMsg("dtnode.onFocus: out of sync: curFocus: %o", this.tree.tnFocused);
				$(this.tree.tnFocused.span).removeClass(this.tree.options.classnames.focused);
			}
			this.tree.tnFocused = this;
			if ( this.tree.options.onFocus ) // Pass element as 'this' (jQuery convention)
				this.tree.options.onFocus.call(this.span, this);
			$(this.tree.tnFocused.span).addClass(this.tree.options.classnames.focused);
		}
		// TODO: return anything?
//		return false;
	},

/*
	visit: function (cb, bSelf, bDeep) { // TODO: better name: each(fn)
		var n = 0;
		if ( bSelf ) { cb (this); n++; }
		if ( this.aChilds ) {
			for (var i=0; i<this.aChilds.length; i++) {
				if ( bDeep ) {
					n += this.aChilds[i].visit (cb, true, bDeep);
				} else {
					cb (this.aChilds[i]);
					n++;
				}
			}
		}
		return n;
	},
*/

	_addChildNode: function (dtnode) {
//		logMsg ('_addChildNode '+dtnode);
		if ( this.aChilds==null )
			this.aChilds = new Array();
		this.aChilds.push (dtnode);
		dtnode.parent = this;

		if ( this.tree.options.expandOnAdd || ( (!this.tree.options.rootCollapsible || !this.tree.options.rootVisible) && this.parent==null ) )
			this.bExpanded = true;
		if ( this.tree.bEnableUpdate )
//			this.render (true, false);
			this.render (true, true); // Issue #4
			
		return dtnode;
	},

	_addNode: function(data) {
		var dtnode = new DynaTreeNode (this.tree, data);
		return this._addChildNode(dtnode);
	},

	append: function(obj) {
		/*
		Data format: array of node objects, with optional 'children' attributes.
		[
			{ title: "t1", isFolder: true, ... }
			{ title: "t2", isFolder: true, ...,
				children: [
					{title: "t2.1", ..},
					{..}
					]
			}
		]
		A simple object is also accepted instead of an array.
		*/
		if( !obj ) return;
		if( !obj.length ) // Passed a single node 
			return this._addNode(obj);
		
		var prevFlag = this.tree.enableUpdate(false);

		var tnFirst = null;
		for (var i=0; i<obj.length; i++) {
			var data = obj[i];
			var dtnode = this._addNode(data);
			if( !tnFirst ) tnFirst = dtnode;
			if( data.children )
				for(var j=0; j<data.children.length; j++)
					dtnode.append(data.children[j]);
		}
		this.tree.enableUpdate(prevFlag);
		return tnFirst;
	},
	
	appendAjax: function(ajaxOptions) {
		this.setLazyNodeStatus(DTNodeStatus_Loading);
		// Ajax option inheritance: $.ajaxSetup < $.ui.dynatree.defaults.ajaxDefaults < tree.options.ajaxDefaults < ajaxOptions
		var self = this;
		var ajaxOptions = $.extend({}, this.tree.options.ajaxDefaults, ajaxOptions, {
       		success: function(data, textStatus){
				self.append(data);
				self.setLazyNodeStatus(DTNodeStatus_Ok);
       			},
       		error: function(XMLHttpRequest, textStatus, errorThrown){
				self.setLazyNodeStatus(DTNodeStatus_Error);
       			}
		});
       	$.ajax(ajaxOptions);
	},
	// --- end of class
	lastentry: undefined
}

/*************************************************************************
 * class DynaTree
 */
 
var DynaTree = Class.create();

DynaTree.prototype = {
	// static members
//	version: '0.3 beta',
	// constructor
	initialize: function (id, options) {
		logMsg ("DynaTree.initialize()");
		// instance members
		this.options = options;

		this.tnSelected    = null;
		this.bEnableUpdate = true;
		this.isDisabled = false;

		// Cached tags
		this.cache = {
			tagFld: "<img src='" + options.imagePath + "ltFld.gif' alt='' />",
			tagFld_o: "<img src='" + options.imagePath + "ltFld_o.gif' alt='' />",
			tagDoc: "<img src='" + options.imagePath + "ltDoc.gif' alt='' />",
			tagL_ns: "<img src='" + options.imagePath + "ltL_ns.gif' alt=' | ' />",
			tagL_: "<img src='" + options.imagePath + "ltL_.gif' alt='   ' />",
			tagL_ne: "<img src='" + options.imagePath + "ltL_ne.gif' alt=' + ' />",
			tagL_nes: "<img src='" + options.imagePath + "ltL_nes.gif' alt=' + ' />",
			tagM_ne: "<img src='" + options.imagePath + "ltM_ne.gif' alt='[-]' class='" + options.classnames.expander + "'/>",
			tagM_nes: "<img src='" + options.imagePath + "ltM_nes.gif' alt='[-]' class='" + options.classnames.expander + "'/>",
			tagP_ne: "<img src='" + options.imagePath + "ltP_ne.gif' alt='[+]' class='" + options.classnames.expander + "'/>",
			tagP_nes: "<img src='" + options.imagePath + "ltP_nes.gif' alt='[+]' class='" + options.classnames.expander + "'/>",
			tagD_ne: "<img src='" + options.imagePath + "ltD_ne.gif' alt='[?]' class='" + options.classnames.expander + "'/>",
			tagD_nes: "<img src='" + options.imagePath + "ltD_nes.gif' alt='[?]' class='" + options.classnames.expander + "'/>"
		};

		// find container element
		this.divTree   = document.getElementById (id);
		// create the root element
		this.tnRoot    = new DynaTreeNode (this, {title: this.options.title, key:"root"});
		this.tnRoot.data.isFolder   = true;
		this.tnRoot.render(false, false);
		this.divRoot   = this.tnRoot.div;
		this.divRoot.className = this.options.classnames.container;
		// add root to container
		this.divTree.appendChild (this.divRoot);
	},
	
	// member functions
	
	toString: function() {
		return "DynaTree '" + this.options.title + "'";
	},
	
	redraw: function() {
		logMsg("dynatree.redraw()...");
		this.tnRoot.render(true, true);
		logMsg("dynatree.redraw() done.");
	},
	
	getRoot: function() {
		return this.tnRoot;
	},
	
	getNodeByKey: function(key) {
		// $("#...") has problems, if the key contains '.', so we use getElementById()
//		return $("#" + this.options.idPrefix + key).attr("dtnode");
		var el = document.getElementById(this.options.idPrefix + key);
		return ( el && el.dtnode ) ? el.dtnode : null;
	},

	getSelectedNode: function() {
		return this.tnSelected;
	},
	
	selectKey: function(key) {
		var dtnode = this.getNodeByKey(key);
		if( !dtnode ) {
			this.tnSelected = null;
			return null;
		}
		dtnode.focus();
		dtnode.select();
		return dtnode;
	},
	
	enableUpdate: function(bEnable) {
		if ( this.bEnableUpdate==bEnable )
			return bEnable;
		this.bEnableUpdate = bEnable;
		if ( bEnable )
			this.redraw();
		return !bEnable; // return previous value
	},
	// --- end of class
	lastentry: undefined
};

/*************************************************************************
 * widget $(..).dynatree
 */

(function($) {

function _getNodeFromElement(el) {
	var iMax = 4;
	do {
		if( el.dtnode ) return el.dtnode;
		el = el.parentNode;
	} while( iMax-- );
	return null;
}

function fnClick(event) {
	var dtnode = _getNodeFromElement(event.target);
	if( !dtnode )
		return false; 
	return dtnode.onClick(event);
}

function fnKeyHandler(event) {
	// Handles keydown and keypressed, because IE and Safari don't fire keypress for cursor keys.
	var dtnode = _getNodeFromElement(event.target);
	if( !dtnode )
		return false; 
	// ...but Firefox does, so ignore them:
	if( event.type == "keypress" && event.charCode == 0 )
		return;
	return dtnode.onKeypress(event);
}

function fnFocusHandler(event) {
	// Handles blur and focus.
	// Fix event for IE:
	event = arguments[0] = jQuery.event.fix( event || window.event );
	var dtnode = _getNodeFromElement(event.target);
//	logMsg("fnFocusHandler(%o), dtnode=%o", event, dtnode);
	if( !dtnode )
		return false; 
	return dtnode.onFocus(event);
}

$.widget("ui.dynatree", {
	init: function() {
		// The widget framework supplies this.element and this.options.
		this.options.event += '.dynatree'; // namespace event

		// create lazytree
		var $this = this.element;
		var opts = this.options;

		// Guess skin path, if not specified
		if(!opts.imagePath) {
			$("script").each( function () {
				if( this.src.search(/.*dynatree[^/]*\.js$/i) >= 0 ) {
					opts.imagePath = this.src.slice(0, this.src.lastIndexOf("/")) + "/skin/";
					logMsg("Guessing imagePath from '%s': '%s'", this.src, opts.imagePath);
					return false; // first match
				}
			});
		}
		// Attach the tree object to parent element
		var id = $this.attr("id");
		this.tree = new DynaTree(id, opts);
		var root = this.tree.getRoot()

		// Init tree structure
		if( opts.children ) {
			// Read structure from node array
			root.append(opts.children);

		} else if( opts.initAjax && opts.initAjax.url ) {
			// Init tree from AJAX request
			root.appendAjax(opts.initAjax);
        	
		} else if( opts.initId ) {
			// Init tree from another UL element
			this.createFromTag(root, $("#"+opts.initId));

		} else {
			// Init tree from the first UL element inside the container <div>
			var $ul = $this.find(">ul").hide();
			this.createFromTag(root, $ul);
			$ul.remove();
		}
		// bind event handlers
		$this.bind("click", fnClick);
		if( opts.keyboard ) {
			$this.bind("keypress keydown", fnKeyHandler);

			// focus/blur don't bubble, i.e. are not delegated to parent <div> tags,
			// so we use the addEventListener capturing phase.
			// See http://www.howtocreate.co.uk/tutorials/javascript/domevents
			var div = this.tree.divTree;
			if( div.addEventListener ) {
				div.addEventListener("focus", fnFocusHandler, true);
				div.addEventListener("blur", fnFocusHandler, true);
			} else {
				div.onfocusin = div.onfocusout = fnFocusHandler;
			}
			if( opts.focusRoot ) {
				if( opts.rootVisible ) {
					root.focus();
				} else if( root.aChilds.length > 0 && ! (opts.initAjax && opts.initAjax.url) ) {
					// Only if not lazy initing (Will be handled by setLazyNodeStatus(DTNodeStatus_Ok)) 
					root.aChilds[0].focus();
				}
			}
		}
	},

	// --- getter methods (i.e. NOT returning a reference to $)
	getTree: function() {
		return this.tree;
	},

	getRoot: function() {
		return this.tree.getRoot();
	},

	getSelectedNode: function() {
		return this.tree.getSelectedNode();
	},

	// --- Private methods
	createFromTag: function(parentTreeNode, $ulParent) {
		// Convert a <UL>...</UL> list into children of the parent tree node.
		var self = this;
		$ulParent.find(">li").each(function() {
			var $li = $(this);
			var $liSpan = $li.find(">span:first");
			var title;
			if( $liSpan.length ) {
				// If a <li><span> tag is specified, use it literally.
				title = $liSpan.html();
			} else {
				// If only a <li> tag is specified, use the trimmed string up to the next child <ul> tag.
				title = $.trim($li.html().match(/.*(<ul)?/)[0]);
			}
			// Parse node attributes data from data="" attribute
			var data = {
				title: title,
				key: $li.attr("id"),
				isFolder: $li.hasClass("folder"),
				isLazy: $li.hasClass("lazy")
			};
			var dataAttr = $.trim($li.attr("data"));
			if( dataAttr ) {
				if( dataAttr[0] != "{" )
					dataAttr = "{" + dataAttr + "}"
				try{
					$.extend(data, eval("(" + dataAttr + ")"));
				} catch(e) {
					throw ("Error parsing node data: " + e + "\ndata:\n'" + dataAttr + "'");
				}
			}
			childNode = parentTreeNode._addNode(data);
			// Recursive reading of child nodes, if LI tag contains an UL tag
			var $ul = $li.find(">ul:first");
			if( $ul.length ) {
				if( data.expand )
					childNode.bExpanded = true;
				self.createFromTag(childNode, $ul); // must use 'self', because 'this' is the each() context
			}
		});
	},

	// ------------------------------------------------------------------------
	lastentry: undefined
});


// The following methods return a value (thus breaking the jQuery call chain):

$.ui.dynatree.getter = "getTree getRoot getSelectedNode";


// Plugin default options:

$.ui.dynatree.defaults = {
	title: "Dynatree root", // Name of the root node.
	rootVisible: false, // Set to true, to make the root node visible.
	rootCollapsible: false, // Prevent root node from being collapsed.
	imagePath: null, // Path to a folder containing icons. Defaults to 'skin/' subdirectory.
	children: null, // Init tree structure from this object array.
	initId: null, // Init tree structure from a <ul> element with this ID.
	initAjax: null, // Ajax options used to initialize the tree strucuture.
	onSelect: null, // Callback when a node is selected.
	onUnselect: null, // Callback when a node is deselected.
	onLazyRead: null, // Callback when a lazy node is expanded for the first time.
	onFocus: null, // Callback when a node receives keyboard focus.
	onBlur: null, // Callback when a node looses focus.
	focusRoot: true, // Set focus to root node on init.
	keyboard: true, // Support keyboard navigation.
	autoCollapse: false, // Automatically collapse all siblings, when a node is expanded.
	expandOnAdd: false, // Automatically expand parent, when a child is added.
	selectExpandsFolders: true, // Clicking a folder title expands the folder, instead of selecting it.
	idPrefix: 'ui-dynatree-id-', // Used to generate node id's like <span id="ui-dynatree-id-<key>">.
	ajaxDefaults: { // Used by initAjax option
		cache: false, // Append random '_' argument to url to prevent caching.
		dataType: "json" // Expect json format and pass json object to callbacks.
	},
	strings: {
		loading: "Loading&#8230;",
		loadError: "Load error!"
	},
	classnames: {
		container: "ui-dynatree-container",
		expander: "ui-dynatree-expander",
		hidden: "ui-dynatree-hidden",
		disabled: "ui-dynatree-disabled",
		selected: "ui-dynatree-selected",
		folder: "ui-dynatree-folder",
		document: "ui-dynatree-document",
		focused: "ui-dynatree-focused",
		loading: "ui-dynatree-loading"
	},
	debugLevel: 0,
	// Experimental:
	selectionVisible: true, // Make sure, selected nodes are visible (expanded).
// 	minExpandLevel: 1, // Instead of rootCollapsible
//	expandLevel: 1, // Expand all branches until level i (set to 0 to )
//	persist: "cookie",
//	fx: null, // Animations, e.g. { height: 'toggle', opacity: 'toggle', duration: 200 }

	// ### copied from ui.tabs
	// basic setup
//	cookie: null, // e.g. { expires: 7, path: '/', domain: 'jquery.com', secure: true }
//  history: false,

	// templates
	//~ tabTemplate: '<li><a href="#{href}"><span>#{label}</span></a></li>', 
	// 		var $li = $(o.tabTemplate.replace(/#\{href\}/g, url).replace(/#\{label\}/g, label));
	//~ panelTemplate: '<div></div>'
	// ------------------------------------------------------------------------
	lastentry: undefined
};

/**
 * Reserved data attributes for a tree node.
 */
$.ui.dynatree.nodedatadefaults = {
	title: null, // (required) Displayed name of the node (html is allowed here)
	key: null, // May be used with select(), find(), ...
	isFolder: false, // Use a folder icon. Also the node is expandable but not selectable.
	isLazy: false, // Call onLazyRead(), when the node is expanded for the first time to allow for delayed creation of children.
	expand: false, // Initial expand status. 
	tooltip: null, // Show this popup text.
	icon: null, // Use a custom image (filename relative to tree.options.imagePath)
	// The following attributes are only valid if passed to some functions:
	children: null, // Array of child nodes.
	// NOTE: we can also add custom attributes here.
	// This may then also be used in the onSelect() or onLazyTree() callbacks.
	// ------------------------------------------------------------------------
	lastentry: undefined
};


// ---------------------------------------------------------------------------
})(jQuery);
