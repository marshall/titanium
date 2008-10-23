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
// Various basic JavaScript utilities.
//=============================================================================

/**
 * Check that the type is not undefined (we do it here as on
 * some devices typeof returns unknown instead of undefined...).
 * We have to pass the evaluation of (typeof elem) (i.e., a string)
 * to the function rather than simply (element) -- passing an 
 * undefined object would make the function crash on PIE.
 */
function isDefined(type) {
  return type != 'undefined' && type != 'unknown';
}

/**
 * Returns true if val is a function.
 */
function isFunction(val) {
  return typeof val == "function";
}

// TODO(aa): more basic type checking here.


/**
 * Creates a 'bound' function that calls the current function with a preset
 * 'this' object and set of arguments.
 */
Function.prototype.bind = function(obj) {
  var preArgs = Array.prototype.slice.call(arguments, 1);
  var self = this;
  return function() {
    var postArgs = Array.prototype.slice.call(arguments, 0);
    var totalArgs = preArgs.concat(postArgs);
    return self.apply(obj, totalArgs);
  }
};

/**
 * Creates a partially applied function that calls the current function with
 * a preset set of arguments.
 */
Function.prototype.partial = function() {
  var args = Array.prototype.slice.call(arguments, 0);
  return this.bind(null, args);
};

/**
 * Binds all the methods in obj to obj.
 */
function bindMethods(obj) {
  for (var p in obj) {
    if (isFunction(obj[p])) {
      obj[p] = obj[p].bind(obj);
    }
  }
}


/**
 * Trim leading and trailing whitespace from a string.
 */
String.prototype.trim = function(str) {
  return this.replace(/^\s+/, "").replace(/\s+$/, "");
};


/**
 * Find the index of a given element in an array. Returns -1 if the item does
 * not exist.
 */
Array.prototype.indexOf = function(item) {
  for (var i = 0; i < this.length; i++) {
    if (this[i] === item) {
      return i;
    }
  }
  return -1;
};

/**
 * Makes a deep copy of an object.
 */
function cloneObject(original) {
  var newObject = new Object();
  for (i in original) {
    if (typeof original[i] == 'object') {
      newObject[i] = cloneObject(original[i]);
    }
    else {
      newObject[i] = original[i];
    }
  }
  newObject.length = original.length;
  return newObject;
}

var browser = {};

(function() {
  var ua = navigator.userAgent;
  browser.opera = typeof opera != "undefined";
  browser.ie = !browser.opera && ua.indexOf("MSIE") > -1;
  browser.ie_mobile = (ua.indexOf("Windows CE") > -1) &&
      (ua.indexOf("MSIE") > -1);
  browser.webkit = ua.indexOf("WebKit") > -1;
  // WebKit also gives product == "gecko".
  browser.mozilla = !browser.webkit && navigator.product == "Gecko";
  browser.safari = ua.indexOf("Safari") > -1;
  browser.mac = navigator.platform.indexOf("Mac") > -1;
  browser.linux = navigator.platform.indexOf("Linux") > -1;
  browser.windows = navigator.platform.indexOf("Win") > -1;
  browser.android = ua.indexOf("Android") > -1;
  browser.chrome = window.chrome;  // TODO(mpcomplete): use ua string
  if (browser.android || browser.chrome) {
    browser.safari = false;
  }
  // TODO(aa): Add detection for more browsers, as necessary.
})();
