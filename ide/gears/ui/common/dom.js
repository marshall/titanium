// Copyright 2008, Google Inc.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. Neither the name of Google Inc. nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

//=============================================================================
// DOM manipulation utilities.
// Requires: base.js
//=============================================================================

var dom = {};

/**
 * Provides a cross-browser way of getting an element by its id
 */
dom.getElementById = function(id) {
  if (isDefined(typeof document.getElementById)) {
    return document.getElementById(id);
  } else if (isDefined(typeof document.all)) {
    return document.all[id];
  }
  throw new Error("Failed to get element by ID.");
};

/**
 * Gets the position and dimensions of an element in pixels in relation
 * to the window origin.
 */
dom.getPosition = function(elem) {
  var ret = { 'left'   : 0,
              'top'    : 0,
              'width'  : elem.width,
              'height' : elem.height };
  
  if (!elem.offsetParent) return ret;
  
  var left = 0;
  var top = 0;
  while (elem) {
    left += elem.offsetLeft;
    top += elem.offsetTop;
    elem = elem.offsetParent
  }
  ret.left = left;
  ret.top = top;
  return ret;
};

/**
 * Returns the height of the inside of the window.
 */
dom.getWindowInnerHeight = function() {
  if (isDefined(typeof window.innerHeight)) { // Firefox
    return window.innerHeight;
  } else if (isDefined(typeof document.body.offsetHeight)) { // IE
    return document.body.offsetHeight;
  }

  throw new Error("Could not get windowInnerHeight.");
};

/**
 * Add an event listener to an element cross-browser.
 */
dom.addEvent = function(element, eventName, handler) {
  var wrapper = dom._callEventHandler.bind(dom, handler);

  if (isDefined(typeof element.addEventListener)) {
    // Standards-compatible browsers
    element.addEventListener(eventName, wrapper, false);
  } else if (isDefined(typeof element.attachEvent)) {
    // IE
    element.attachEvent("on" + eventName, wrapper);
  } else {
    throw new Error('Failed to attach event.');
  }
};

/**
 * Private helper for calling event handlers compatibly across browsers.
 */
dom._callEventHandler = function(handler, e) {
  // Older versions of IE don't pass event to the handler.
  if (!e) e = window.event;

  // TODO(aa): Normalize event object properties.

  return handler(e);
};

/**
 * Gets the CSS classes for an element.
 */
dom.getClasses = function(elm) {
  return elm.className.split(/\s+/);
};

/**
 * Check is an element has a particular CSS class name.
 */
dom.hasClass = function(elm, className) {
  return this.getClasses(elm).indexOf(className) > -1;
};

/**
 * Adds a CSS class to an element. Do nothing if the class already exists.
 */
dom.addClass = function(elm, className) {
  var classes = this.getClasses(elm);
  if (classes.indexOf(className) > -1) {
    return;
  }
  classes.push(className);
  elm.className = classes.join(" ");
};

/**
 * Removes a CSS class from an element. Do nothing if the class doesn't exist.
 */
dom.removeClass = function(elm, className) {
  var classes = this.getClasses(elm);
  var index = classes.indexOf(className);
  if (index == -1) {
    return;
  }
  classes.splice(index, 1);
  elm.className = classes.join(" ");
};

/**
 * Gets the text (non-html) content of an element.
 */
dom.getTextContent = function(elm) {
  if (isDefined(typeof elm.textContent)) {
    return elm.textContent;
  } else if (isDefined(typeof elm.innerText)) {
    return elm.innerText;
  } else {
    throw new Error("Could not find a property to get text content.");
  }
};

/**
 * Sets the text (non-html) contents of an element.
 */
dom.setTextContent = function(elm, text) {
  if (isDefined(typeof elm.textContent)) {
    elm.textContent = text;
  } else if (isDefined(typeof elm.innerText)) {
    elm.innerText = text;
  } else {
    throw new Error("Could not find a property to set text content.");
  }
};
