;(function($){ // secure $ jQuery alias
/*******************************************************************************************/	
// jquery.event.wheel.js - rev 1 
// Copyright (c) 2008, Three Dub Media (http://threedubmedia.com)
// Liscensed under the MIT License (MIT-LICENSE.txt)
// http://www.opensource.org/licenses/mit-license.php
// Created: 2008-07-01 | Updated: 2008-07-14
/*******************************************************************************************/

// jquery method
$.fn.wheel = function( fn ){
	return this[ fn ? "bind" : "trigger" ]( "wheel", fn );
	};

// special event config
$.event.special.wheel = {
	setup: function(){
		$.event.add( this, wheelEvents, wheelHandler, {} );
		},
	teardown: function(){
		$.event.remove( this, wheelEvents, wheelHandler );
		}
	};

// events to bind ( browser sniffed... )
var wheelEvents = !$.browser.mozilla ? "mousewheel" : // IE, opera, safari
	"DOMMouseScroll"+( $.browser.version<"1.9" ? " mousemove" : "" ); // firefox

// shared event handler
function wheelHandler( event ){ 
	switch ( event.type ){
		case "mousemove": // FF2 has incorrect event positions
			return $.extend( event.data, { // store the correct properties
				clientX: event.clientX, clientY: event.clientY,
				pageX: event.pageX, pageY: event.pageY
				});			
		case "DOMMouseScroll": // firefox
			$.extend( event, event.data ); // fix event properties in FF2
			event.delta = -event.detail/3; // normalize delta
			break;
		case "mousewheel": // IE, opera, safari
			event.delta = event.wheelDelta/120; // normalize delta
			if ( $.browser.opera ) event.delta *= -1; // normalize delta
			break;
		}
	event.type = "wheel"; // hijack the event	
	return $.event.handle.call( this, event, event.delta );
	};
	
/*******************************************************************************************/
})(jQuery); // confine scope