// Copyright 2005, Google Inc.
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

/**
 * lang.js - The missing JavaScript language features
 */

/**
 * This is a reference to the global object. It is preferred to be used in
 * places where people often use 'window' as the global object. You should of
 * course continue to use 'window' when you are accessing methods and fields of
 * the window object such as 'setTimeout' etc.
 * @type Object
 */
var global = this;

/**
 * Returns true if the specified value is null.
 */
function isNull(val) {
  return val === null;
}

/**
 * Returns true if the specified value is an array.
 */
function isArray(a) {
  // We cannot rely on constructor == Array or instanceof Array because
  // different frames have different Array objects.
  return a instanceof Array || a && typeof a == "object" &&
      typeof a.join == "function" &&
      typeof a.reverse == "function";
}

/**
 * Returns true if the specified value is a string.
 */
function isString(val) {
  return typeof val == "string";
}

/**
 * Returns true if the specified value is a boolean.
 */
function isBoolean(val) {
  return typeof val == "boolean";
}

/**
 * Returns true if the specified value is a number.
 */
function isNumber(val) {
  return typeof val == "number";
}

/**
 * Returns true if the specified value is a function.
 */
function isFunction(val) {
  return typeof val == "function";
}

/**
 * Returns true if the specified value is an object.
 */
function isObject(val) {
  return val && typeof val == "object";
}

/**
 * Returns an array of all the properties defined on an object.
 */
function getObjectProps(obj) {
  var ret = [];

  for (var p in obj) {
    ret.push(p);
  }

  return ret;
}

/**
 * Returns true if the specified value is an object which has no properties
 * defined.
 */
function isEmptyObject(val) {
  if (!isObject(val)) {
    return false;
  }

  for (var p in val) {
    return false;
  }

  return true;
}


var G_HASH_CODE_PROPERTY_ = "lang_hashCode_";
var G_hashCodeCounter_ = 0;

/**
 * Adds a lang_hashCode_ field to an object. The hash code is unique for the
 * given object.
 *
 * Warning! If getHashCode is called on a prototype object then all the
 * instances of the class that extends that will share the hash code.
 *
 * @param obj {Object} The object to get the hash code for
 * @returns {Number} The hash code for the object
 */
function getHashCode(obj) {
  // we are not using hasOwnProperty because it might lead to hard to find
  // bugs in IE 5.0 and other browsers that do not support it.
  if (!obj[G_HASH_CODE_PROPERTY_]) {
    obj[G_HASH_CODE_PROPERTY_] = ++G_hashCodeCounter_;
  }
  return obj[G_HASH_CODE_PROPERTY_];
}

/**
 * Removes the lang_hashCode_ field from an object.
 * @param obj {Object} The object to remove the field from.
 */
function removeHashCode(obj) {
  // In IE, you cannot use delete on DOM nodes so we just set the field
  // to undefined if this one fails
  obj[G_HASH_CODE_PROPERTY_] = undefined;
};


/**
 * Returns the current absolute time, in seconds.
 */
function getTimeSeconds() {
  var timeMs = new Date().getTime();
  return timeMs / 1000;
}


/**
 * Checks if a string starts with another string.
 */
String.prototype.startsWith = function(prefix) {
  return this.indexOf(prefix) == 0;
}


/**
 * Fast suffix-checker.
 * @param {String} suffix String that may appear at end
 * @return {Boolean} True/false
 */
String.prototype.endsWith = function (suffix) {
  var l = this.length - suffix.length;
  return l >= 0 && this.lastIndexOf(suffix, l) == l;
};


/**
 * Removes whitespace from the beginning and end of the string
 */
String.prototype.trim = function() {
  return this.replace(/^\s+|\s+$/g, "");
}

/**
 * Does simple python-style string substitution.
 * "foo%s hot%s".subs("bar", "dog") becomes "foobar hotdot".
 * For more fully-featured templating, see template.js.
 */
String.prototype.subs = function(var_args) {
  var ret = this;

  // this appears to be slow, but testing shows it compares more or less equiv.
  // to the regex.exec method.
  for (var i = 0; i < arguments.length; i++) {
    ret = ret.replace(/\%s/, String(arguments[i]));
  }

  return ret;
}

/**
 * Partially applies this function to a particular "this object" and zero or
 * more arguments. The result is a new function with some arguments of the first
 * function pre-filled and the value of |this| "pre-specified".
 *
 * Remaining arguments specified at call-time are appended to the pre-
 * specified ones.
 *
 * Also see: partial().
 *
 * Note that bind and partial are optimized such that repeated calls to it do
 * not create more than one function object, so there is no additional cost for
 * something like:
 *
 * var g = bind(f, obj);
 * var h = partial(g, 1, 2, 3);
 * var k = partial(h, a, b, c);
 *
 * Usage:
 * var barMethBound = bind(myFunction, myObj, "arg1", "arg2");
 * barMethBound("arg3", "arg4");
 *
 * @param self {Object} Specifies the object which |this| should point to
 * when the function is run. If the value is null or undefined, it will default
 * to the global object.
 *
 * @returns {Function} A partially-applied form of the function bind() was
 * invoked as a method of.
 */
function bind(fn, self, var_args) {
  var boundargs = fn.boundArgs_ || [];
  boundargs = boundargs.concat(Array.prototype.slice.call(arguments, 2));

  if (typeof fn.boundSelf_ != "undefined") {
    self = fn.boundSelf_;
  }

  if (typeof fn.boundFn_ != "undefined") {
    fn = fn.boundFn_;
  }

  var newfn = function() {
    // Combine the static args and the new args into one big array
    var args = boundargs.concat(Array.prototype.slice.call(arguments));
    return fn.apply(self, args);
  }

  newfn.boundArgs_ = boundargs;
  newfn.boundSelf_ = self;
  newfn.boundFn_ = fn;

  return newfn;
}

/**
 * Like bind(), except that a "this object" is not required. Useful when the
 * target function is already bound.
 *
 * Usage:
 * var g = partial(f, arg1, arg2);
 * g(arg3, arg4);
 */
function partial(fn, var_args) {
  return bind.apply(
    null, [fn, null].concat(Array.prototype.slice.call(arguments, 1)));
}

/**
 * Convenience. Binds all the methods of obj to itself. Calling this in the
 * constructor before referencing any methods makes things a little more like
 * Java or Python where methods are intrinsically bound to their instance.
 */
function bindMethods(obj) {
  for (var p in obj) {
    if (isFunction(obj[p])) {
      obj[p] = bind(obj[p], obj);
    }
  }
}

/**
 * Inherit the prototype methods from one constructor into another.
 *
 * Usage:
 * <pre>
 * function ParentClass(a, b) { }
 * ParentClass.prototype.foo = function(a) { }
 *
 * function ChildClass(a, b, c) {
 *   ParentClass.call(this, a, b);
 * }
 *
 * ChildClass.inherits(ParentClass);
 *
 * var child = new ChildClass("a", "b", "see");
 * child.foo(); // works
 * </pre>
 *
 * In addition, a superclass' implementation of a method can be invoked
 * as follows:
 *
 * <pre>
 * ChildClass.prototype.foo = function(a) {
 *   ChildClass.superClass_.foo.call(this, a);
 *   // other code
 * };
 * </pre>
 */
Function.prototype.inherits = function(parentCtor) {
  var tempCtor = function(){};
  tempCtor.prototype = parentCtor.prototype;
  this.superClass_ = parentCtor.prototype;
  this.prototype = new tempCtor();
}
