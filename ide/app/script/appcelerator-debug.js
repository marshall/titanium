/*!
 * This file is part of Appcelerator.
 *
 * Copyright 2006-2008 Appcelerator, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/

/* The following files are subject to license agreements by their respective license owners */
/**
 * this file is included *before* any other thirdparty libraries or 
 * SDK files.  you must assume that you have no external capabilities in this file
 */
 
/*  Prototype JavaScript framework, version 1.6.0
 *  (c) 2005-2007 Sam Stephenson
 *
 *  Prototype is freely distributable under the terms of an MIT-style license.
 *  For details, see the Prototype web site: http://www.prototypejs.org/
 *
 *--------------------------------------------------------------------------*/

var Prototype = {
  Version: '1.6.0',

  Browser: {
    IE:     !!(window.attachEvent && !window.opera),
    Opera:  !!window.opera,
    WebKit: navigator.userAgent.indexOf('AppleWebKit/') > -1,
    Gecko:  navigator.userAgent.indexOf('Gecko') > -1 && navigator.userAgent.indexOf('KHTML') == -1,
    MobileSafari: !!navigator.userAgent.match(/Apple.*Mobile.*Safari/)
  },

  BrowserFeatures: {
    XPath: !!document.evaluate,
    ElementExtensions: !!window.HTMLElement,
    SpecificElementExtensions:
      document.createElement('div').__proto__ &&
      document.createElement('div').__proto__ !==
        document.createElement('form').__proto__
  },

  ScriptFragment: '<script[^>]*>([\\S\\s]*?)<\/script>',
  JSONFilter: /^\/\*-secure-([\s\S]*)\*\/\s*$/,

  emptyFunction: function() { },
  K: function(x) { return x }
};

if (Prototype.Browser.MobileSafari)
  Prototype.BrowserFeatures.SpecificElementExtensions = false;

if (Prototype.Browser.WebKit)
  Prototype.BrowserFeatures.XPath = false;

/* Based on Alex Arnell's inheritance implementation. */
var Class = {
  create: function() {
    var parent = null, properties = $A(arguments);
    if (Object.isFunction(properties[0]))
      parent = properties.shift();

    function klass() {
      this.initialize.apply(this, arguments);
    }

    Object.extend(klass, Class.Methods);
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
      klass.prototype.initialize = Prototype.emptyFunction;

    klass.prototype.constructor = klass;

    return klass;
  }
};

Class.Methods = {
  addMethods: function(source) {
    var ancestor   = this.superclass && this.superclass.prototype;
    var properties = Object.keys(source);

    if (!Object.keys({ toString: true }).length)
      properties.push("toString", "valueOf");

    for (var i = 0, length = properties.length; i < length; i++) {
      var property = properties[i], value = source[property];
      if (ancestor && Object.isFunction(value) &&
          value.argumentNames().first() == "$super") {
        var method = value, value = Object.extend((function(m) {
          return function() { return ancestor[m].apply(this, arguments) };
        })(property).wrap(method), {
          valueOf:  function() { return method },
          toString: function() { return method.toString() }
        });
      }
      this.prototype[property] = value;
    }

    return this;
  }
};

var Abstract = { };

Object.extend = function(destination, source) {
  for (var property in source)
    destination[property] = source[property];
  return destination;
};

Object.extend(Object, {
  inspect: function(object) {
    try {
      if (object === undefined) return 'undefined';
      if (object === null) return 'null';
      return object.inspect ? object.inspect() : object.toString();
    } catch (e) {
      if (e instanceof RangeError) return '...';
      throw e;
    }
  },

  toJSON: function(object) {
    var type = typeof object;
    switch (type) {
      case 'undefined':
      case 'function':
      case 'unknown': return;
      case 'boolean': return object.toString();
    }

    if (object === null) return 'null';
    if (object.toJSON) return object.toJSON();
    if (Object.isElement(object)) return;

    var results = [];
    for (var property in object) {
      var value = Object.toJSON(object[property]);
      if (value !== undefined)
        results.push(property.toJSON() + ': ' + value);
    }

    return '{' + results.join(', ') + '}';
  },

  toQueryString: function(object) {
    return $H(object).toQueryString();
  },

  toHTML: function(object) {
    return object && object.toHTML ? object.toHTML() : String.interpret(object);
  },

  keys: function(object) {
    var keys = [];
    for (var property in object)
      keys.push(property);
    return keys;
  },

  values: function(object) {
    var values = [];
    for (var property in object)
      values.push(object[property]);
    return values;
  },

  clone: function(object) {
    return Object.extend({ }, object);
  },

  isElement: function(object) {
    return object && object.nodeType == 1;
  },

  isArray: function(object) {
    return object && object.constructor === Array;
  },

  isHash: function(object) {
    return object instanceof Hash;
  },

  isFunction: function(object) {
    return typeof object == "function";
  },

  isString: function(object) {
    return typeof object == "string";
  },

  isNumber: function(object) {
    return typeof object == "number";
  },

  isUndefined: function(object) {
    return typeof object == "undefined";
  }
});

Object.extend(Function.prototype, {
  argumentNames: function() {
    var names = this.toString().match(/^[\s\(]*function[^(]*\((.*?)\)/)[1].split(",").invoke("strip");
    return names.length == 1 && !names[0] ? [] : names;
  },

  bind: function() {
    if (arguments.length < 2 && arguments[0] === undefined) return this;
    var __method = this, args = $A(arguments), object = args.shift();
    return function() {
      return __method.apply(object, args.concat($A(arguments)));
    }
  },

  bindAsEventListener: function() {
    var __method = this, args = $A(arguments), object = args.shift();
    return function(event) {
      return __method.apply(object, [event || window.event].concat(args));
    }
  },

  curry: function() {
    if (!arguments.length) return this;
    var __method = this, args = $A(arguments);
    return function() {
      return __method.apply(this, args.concat($A(arguments)));
    }
  },

  delay: function() {
    var __method = this, args = $A(arguments), timeout = args.shift() * 1000;
    return window.setTimeout(function() {
      return __method.apply(__method, args);
    }, timeout);
  },

  wrap: function(wrapper) {
    var __method = this;
    return function() {
      return wrapper.apply(this, [__method.bind(this)].concat($A(arguments)));
    }
  },

  methodize: function() {
    if (this._methodized) return this._methodized;
    var __method = this;
    return this._methodized = function() {
      return __method.apply(null, [this].concat($A(arguments)));
    };
  }
});

Function.prototype.defer = Function.prototype.delay.curry(0.01);

Date.prototype.toJSON = function() {
  return '"' + this.getUTCFullYear() + '-' +
    (this.getUTCMonth() + 1).toPaddedString(2) + '-' +
    this.getUTCDate().toPaddedString(2) + 'T' +
    this.getUTCHours().toPaddedString(2) + ':' +
    this.getUTCMinutes().toPaddedString(2) + ':' +
    this.getUTCSeconds().toPaddedString(2) + 'Z"';
};

var Try = {
  these: function() {
    var returnValue;

    for (var i = 0, length = arguments.length; i < length; i++) {
      var lambda = arguments[i];
      try {
        returnValue = lambda();
        break;
      } catch (e) { }
    }

    return returnValue;
  }
};

RegExp.prototype.match = RegExp.prototype.test;

RegExp.escape = function(str) {
  return String(str).replace(/([.*+?^=!:${}()|[\]\/\\])/g, '\\$1');
};

/*--------------------------------------------------------------------------*/

var PeriodicalExecuter = Class.create({
  initialize: function(callback, frequency) {
    this.callback = callback;
    this.frequency = frequency;
    this.currentlyExecuting = false;

    this.registerCallback();
  },

  registerCallback: function() {
    this.timer = setInterval(this.onTimerEvent.bind(this), this.frequency * 1000);
  },

  execute: function() {
    this.callback(this);
  },

  stop: function() {
    if (!this.timer) return;
    clearInterval(this.timer);
    this.timer = null;
  },

  onTimerEvent: function() {
    if (!this.currentlyExecuting) {
      try {
        this.currentlyExecuting = true;
        this.execute();
      } finally {
        this.currentlyExecuting = false;
      }
    }
  }
});
Object.extend(String, {
  interpret: function(value) {
    return value == null ? '' : String(value);
  },
  specialChar: {
    '\b': '\\b',
    '\t': '\\t',
    '\n': '\\n',
    '\f': '\\f',
    '\r': '\\r',
    '\\': '\\\\'
  }
});

Object.extend(String.prototype, {
  gsub: function(pattern, replacement) {
    var result = '', source = this, match;
    replacement = arguments.callee.prepareReplacement(replacement);

    while (source.length > 0) {
      if (match = source.match(pattern)) {
        result += source.slice(0, match.index);
        result += String.interpret(replacement(match));
        source  = source.slice(match.index + match[0].length);
      } else {
        result += source, source = '';
      }
    }
    return result;
  },

  sub: function(pattern, replacement, count) {
    replacement = this.gsub.prepareReplacement(replacement);
    count = count === undefined ? 1 : count;

    return this.gsub(pattern, function(match) {
      if (--count < 0) return match[0];
      return replacement(match);
    });
  },

  scan: function(pattern, iterator) {
    this.gsub(pattern, iterator);
    return String(this);
  },

  truncate: function(length, truncation) {
    length = length || 30;
    truncation = truncation === undefined ? '...' : truncation;
    return this.length > length ?
      this.slice(0, length - truncation.length) + truncation : String(this);
  },

  strip: function() {
    return this.replace(/^\s+/, '').replace(/\s+$/, '');
  },

  stripTags: function() {
    return this.replace(/<\/?[^>]+>/gi, '');
  },

  stripScripts: function() {
    return this.replace(new RegExp(Prototype.ScriptFragment, 'img'), '');
  },

  extractScripts: function() {
    var matchAll = new RegExp(Prototype.ScriptFragment, 'img');
    var matchOne = new RegExp(Prototype.ScriptFragment, 'im');
    return (this.match(matchAll) || []).map(function(scriptTag) {
      return (scriptTag.match(matchOne) || ['', ''])[1];
    });
  },

  evalScripts: function() {
    return this.extractScripts().map(function(script) { return eval(script) });
  },

  escapeHTML: function() {
    var self = arguments.callee;
    self.text.data = this;
    return self.div.innerHTML;
  },

  unescapeHTML: function() {
    var div = new Element('div');
    div.innerHTML = this.stripTags();
    return div.childNodes[0] ? (div.childNodes.length > 1 ?
      $A(div.childNodes).inject('', function(memo, node) { return memo+node.nodeValue }) :
      div.childNodes[0].nodeValue) : '';
  },

  toQueryParams: function(separator) {
    var match = this.strip().match(/([^?#]*)(#.*)?$/);
    if (!match) return { };

    return match[1].split(separator || '&').inject({ }, function(hash, pair) {
      if ((pair = pair.split('='))[0]) {
        var key = decodeURIComponent(pair.shift());
        var value = pair.length > 1 ? pair.join('=') : pair[0];
        if (value != undefined) value = decodeURIComponent(value);

        if (key in hash) {
          if (!Object.isArray(hash[key])) hash[key] = [hash[key]];
          hash[key].push(value);
        }
        else hash[key] = value;
      }
      return hash;
    });
  },

  toArray: function() {
    return this.split('');
  },

  succ: function() {
    return this.slice(0, this.length - 1) +
      String.fromCharCode(this.charCodeAt(this.length - 1) + 1);
  },

  times: function(count) {
    return count < 1 ? '' : new Array(count + 1).join(this);
  },

  camelize: function() {
    var parts = this.split('-'), len = parts.length;
    if (len == 1) return parts[0];

    var camelized = this.charAt(0) == '-'
      ? parts[0].charAt(0).toUpperCase() + parts[0].substring(1)
      : parts[0];

    for (var i = 1; i < len; i++)
      camelized += parts[i].charAt(0).toUpperCase() + parts[i].substring(1);

    return camelized;
  },

  capitalize: function() {
    return this.charAt(0).toUpperCase() + this.substring(1).toLowerCase();
  },

  underscore: function() {
    return this.gsub(/::/, '/').gsub(/([A-Z]+)([A-Z][a-z])/,'#{1}_#{2}').gsub(/([a-z\d])([A-Z])/,'#{1}_#{2}').gsub(/-/,'_').toLowerCase();
  },

  dasherize: function() {
    return this.gsub(/_/,'-');
  },

  inspect: function(useDoubleQuotes) {
    var escapedString = this.gsub(/[\x00-\x1f\\]/, function(match) {
      var character = String.specialChar[match[0]];
      return character ? character : '\\u00' + match[0].charCodeAt().toPaddedString(2, 16);
    });
    if (useDoubleQuotes) return '"' + escapedString.replace(/"/g, '\\"') + '"';
    return "'" + escapedString.replace(/'/g, '\\\'') + "'";
  },

  toJSON: function() {
    return this.inspect(true);
  },

  unfilterJSON: function(filter) {
    return this.sub(filter || Prototype.JSONFilter, '#{1}');
  },

  isJSON: function() {
    var str = this.replace(/\\./g, '@').replace(/"[^"\\\n\r]*"/g, '');
    return (/^[,:{}\[\]0-9.\-+Eaeflnr-u \n\r\t]*$/).test(str);
  },

  evalJSON: function(sanitize) {
    var json = this.unfilterJSON();
    try {
      if (!sanitize || json.isJSON()) return eval('(' + json + ')');
    } catch (e) { }
    throw new SyntaxError('Badly formed JSON string: ' + this.inspect());
  },

  include: function(pattern) {
    return this.indexOf(pattern) > -1;
  },

  startsWith: function(pattern) {
    return this.indexOf(pattern) === 0;
  },

  endsWith: function(pattern) {
    var d = this.length - pattern.length;
    return d >= 0 && this.lastIndexOf(pattern) === d;
  },

  empty: function() {
    return this == '';
  },

  blank: function() {
    return /^\s*$/.test(this);
  },

  interpolate: function(object, pattern) {
    return new Template(this, pattern).evaluate(object);
  }
});

if (Prototype.Browser.WebKit || Prototype.Browser.IE) Object.extend(String.prototype, {
  escapeHTML: function() {
    return this.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');
  },
  unescapeHTML: function() {
    return this.replace(/&amp;/g,'&').replace(/&lt;/g,'<').replace(/&gt;/g,'>');
  }
});

String.prototype.gsub.prepareReplacement = function(replacement) {
  if (Object.isFunction(replacement)) return replacement;
  var template = new Template(replacement);
  return function(match) { return template.evaluate(match) };
};

String.prototype.parseQuery = String.prototype.toQueryParams;

Object.extend(String.prototype.escapeHTML, {
  div:  document.createElement('div'),
  text: document.createTextNode('')
});

with (String.prototype.escapeHTML) div.appendChild(text);

var Template = Class.create({
  initialize: function(template, pattern) {
    this.template = template.toString();
    this.pattern = pattern || Template.Pattern;
  },

  evaluate: function(object) {
    if (Object.isFunction(object.toTemplateReplacements))
      object = object.toTemplateReplacements();

    return this.template.gsub(this.pattern, function(match) {
      if (object == null) return '';

      var before = match[1] || '';
      if (before == '\\') return match[2];

      var ctx = object, expr = match[3];
      var pattern = /^([^.[]+|\[((?:.*?[^\\])?)\])(\.|\[|$)/, match = pattern.exec(expr);
      if (match == null) return before;

      while (match != null) {
        var comp = match[1].startsWith('[') ? match[2].gsub('\\\\]', ']') : match[1];
        ctx = ctx[comp];
        if (null == ctx || '' == match[3]) break;
        expr = expr.substring('[' == match[3] ? match[1].length : match[0].length);
        match = pattern.exec(expr);
      }

      return before + String.interpret(ctx);
    }.bind(this));
  }
});
Template.Pattern = /(^|.|\r|\n)(#\{(.*?)\})/;

var $break = { };

var Enumerable = {
  each: function(iterator, context) {
    var index = 0;
    iterator = iterator.bind(context);
    try {
      this._each(function(value) {
        iterator(value, index++);
      });
    } catch (e) {
      if (e != $break) throw e;
    }
    return this;
  },

  eachSlice: function(number, iterator, context) {
    iterator = iterator ? iterator.bind(context) : Prototype.K;
    var index = -number, slices = [], array = this.toArray();
    while ((index += number) < array.length)
      slices.push(array.slice(index, index+number));
    return slices.collect(iterator, context);
  },

  all: function(iterator, context) {
    iterator = iterator ? iterator.bind(context) : Prototype.K;
    var result = true;
    this.each(function(value, index) {
      result = result && !!iterator(value, index);
      if (!result) throw $break;
    });
    return result;
  },

  any: function(iterator, context) {
    iterator = iterator ? iterator.bind(context) : Prototype.K;
    var result = false;
    this.each(function(value, index) {
      if (result = !!iterator(value, index))
        throw $break;
    });
    return result;
  },

  collect: function(iterator, context) {
    iterator = iterator ? iterator.bind(context) : Prototype.K;
    var results = [];
    this.each(function(value, index) {
      results.push(iterator(value, index));
    });
    return results;
  },

  detect: function(iterator, context) {
    iterator = iterator.bind(context);
    var result;
    this.each(function(value, index) {
      if (iterator(value, index)) {
        result = value;
        throw $break;
      }
    });
    return result;
  },

  findAll: function(iterator, context) {
    iterator = iterator.bind(context);
    var results = [];
    this.each(function(value, index) {
      if (iterator(value, index))
        results.push(value);
    });
    return results;
  },

  grep: function(filter, iterator, context) {
    iterator = iterator ? iterator.bind(context) : Prototype.K;
    var results = [];

    if (Object.isString(filter))
      filter = new RegExp(filter);

    this.each(function(value, index) {
      if (filter.match(value))
        results.push(iterator(value, index));
    });
    return results;
  },

  include: function(object) {
    if (Object.isFunction(this.indexOf))
      if (this.indexOf(object) != -1) return true;

    var found = false;
    this.each(function(value) {
      if (value == object) {
        found = true;
        throw $break;
      }
    });
    return found;
  },

  inGroupsOf: function(number, fillWith) {
    fillWith = fillWith === undefined ? null : fillWith;
    return this.eachSlice(number, function(slice) {
      while(slice.length < number) slice.push(fillWith);
      return slice;
    });
  },

  inject: function(memo, iterator, context) {
    iterator = iterator.bind(context);
    this.each(function(value, index) {
      memo = iterator(memo, value, index);
    });
    return memo;
  },

  invoke: function(method) {
    var args = $A(arguments).slice(1);
    return this.map(function(value) {
      return value[method].apply(value, args);
    });
  },

  max: function(iterator, context) {
    iterator = iterator ? iterator.bind(context) : Prototype.K;
    var result;
    this.each(function(value, index) {
      value = iterator(value, index);
      if (result == undefined || value >= result)
        result = value;
    });
    return result;
  },

  min: function(iterator, context) {
    iterator = iterator ? iterator.bind(context) : Prototype.K;
    var result;
    this.each(function(value, index) {
      value = iterator(value, index);
      if (result == undefined || value < result)
        result = value;
    });
    return result;
  },

  partition: function(iterator, context) {
    iterator = iterator ? iterator.bind(context) : Prototype.K;
    var trues = [], falses = [];
    this.each(function(value, index) {
      (iterator(value, index) ?
        trues : falses).push(value);
    });
    return [trues, falses];
  },

  pluck: function(property) {
    var results = [];
    this.each(function(value) {
      results.push(value[property]);
    });
    return results;
  },

  reject: function(iterator, context) {
    iterator = iterator.bind(context);
    var results = [];
    this.each(function(value, index) {
      if (!iterator(value, index))
        results.push(value);
    });
    return results;
  },

  sortBy: function(iterator, context) {
    iterator = iterator.bind(context);
    return this.map(function(value, index) {
      return {value: value, criteria: iterator(value, index)};
    }).sort(function(left, right) {
      var a = left.criteria, b = right.criteria;
      return a < b ? -1 : a > b ? 1 : 0;
    }).pluck('value');
  },

  toArray: function() {
    return this.map();
  },

  zip: function() {
    var iterator = Prototype.K, args = $A(arguments);
    if (Object.isFunction(args.last()))
      iterator = args.pop();

    var collections = [this].concat(args).map($A);
    return this.map(function(value, index) {
      return iterator(collections.pluck(index));
    });
  },

  size: function() {
    return this.toArray().length;
  },

  inspect: function() {
    return '#<Enumerable:' + this.toArray().inspect() + '>';
  }
};

Object.extend(Enumerable, {
  map:     Enumerable.collect,
  find:    Enumerable.detect,
  select:  Enumerable.findAll,
  filter:  Enumerable.findAll,
  member:  Enumerable.include,
  entries: Enumerable.toArray,
  every:   Enumerable.all,
  some:    Enumerable.any
});
function $A(iterable) {
  if (!iterable) return [];
  if (iterable.toArray) return iterable.toArray();
  var length = iterable.length, results = new Array(length);
  while (length--) results[length] = iterable[length];
  return results;
}

if (Prototype.Browser.WebKit) {
  function $A(iterable) {
    if (!iterable) return [];
    if (!(Object.isFunction(iterable) && iterable == '[object NodeList]') &&
        iterable.toArray) return iterable.toArray();
    var length = iterable.length, results = new Array(length);
    while (length--) results[length] = iterable[length];
    return results;
  }
}

Array.from = $A;

Object.extend(Array.prototype, Enumerable);

if (!Array.prototype._reverse) Array.prototype._reverse = Array.prototype.reverse;

Object.extend(Array.prototype, {
  _each: function(iterator) {
    for (var i = 0, length = this.length; i < length; i++)
      iterator(this[i]);
  },

  clear: function() {
    this.length = 0;
    return this;
  },

  first: function() {
    return this[0];
  },

  last: function() {
    return this[this.length - 1];
  },

  compact: function() {
    return this.select(function(value) {
      return value != null;
    });
  },

  flatten: function() {
    return this.inject([], function(array, value) {
      return array.concat(Object.isArray(value) ?
        value.flatten() : [value]);
    });
  },

  without: function() {
    var values = $A(arguments);
    return this.select(function(value) {
      return !values.include(value);
    });
  },

  reverse: function(inline) {
    return (inline !== false ? this : this.toArray())._reverse();
  },

  reduce: function() {
    return this.length > 1 ? this : this[0];
  },

  uniq: function(sorted) {
    return this.inject([], function(array, value, index) {
      if (0 == index || (sorted ? array.last() != value : !array.include(value)))
        array.push(value);
      return array;
    });
  },

  intersect: function(array) {
    return this.uniq().findAll(function(item) {
      return array.detect(function(value) { return item === value });
    });
  },

  clone: function() {
    return [].concat(this);
  },

  size: function() {
    return this.length;
  },

  inspect: function() {
    return '[' + this.map(Object.inspect).join(', ') + ']';
  },

  toJSON: function() {
    var results = [];
    this.each(function(object) {
      var value = Object.toJSON(object);
      if (value !== undefined) results.push(value);
    });
    return '[' + results.join(', ') + ']';
  }
});

// use native browser JS 1.6 implementation if available
if (Object.isFunction(Array.prototype.forEach))
  Array.prototype._each = Array.prototype.forEach;

if (!Array.prototype.indexOf) Array.prototype.indexOf = function(item, i) {
  i || (i = 0);
  var length = this.length;
  if (i < 0) i = length + i;
  for (; i < length; i++)
    if (this[i] === item) return i;
  return -1;
};

if (!Array.prototype.lastIndexOf) Array.prototype.lastIndexOf = function(item, i) {
  i = isNaN(i) ? this.length : (i < 0 ? this.length + i : i) + 1;
  var n = this.slice(0, i).reverse().indexOf(item);
  return (n < 0) ? n : i - n - 1;
};

Array.prototype.toArray = Array.prototype.clone;

function $w(string) {
  if (!Object.isString(string)) return [];
  string = string.strip();
  return string ? string.split(/\s+/) : [];
}

if (Prototype.Browser.Opera){
  Array.prototype.concat = function() {
    var array = [];
    for (var i = 0, length = this.length; i < length; i++) array.push(this[i]);
    for (var i = 0, length = arguments.length; i < length; i++) {
      if (Object.isArray(arguments[i])) {
        for (var j = 0, arrayLength = arguments[i].length; j < arrayLength; j++)
          array.push(arguments[i][j]);
      } else {
        array.push(arguments[i]);
      }
    }
    return array;
  };
}
Object.extend(Number.prototype, {
  toColorPart: function() {
    return this.toPaddedString(2, 16);
  },

  succ: function() {
    return this + 1;
  },

  times: function(iterator) {
    $R(0, this, true).each(iterator);
    return this;
  },

  toPaddedString: function(length, radix) {
    var string = this.toString(radix || 10);
    return '0'.times(length - string.length) + string;
  },

  toJSON: function() {
    return isFinite(this) ? this.toString() : 'null';
  }
});

$w('abs round ceil floor').each(function(method){
  Number.prototype[method] = Math[method].methodize();
});
function $H(object) {
  return new Hash(object);
};

var Hash = Class.create(Enumerable, (function() {
  if (function() {
    var i = 0, Test = function(value) { this.key = value };
    Test.prototype.key = 'foo';
    for (var property in new Test('bar')) i++;
    return i > 1;
  }()) {
    function each(iterator) {
      var cache = [];
      for (var key in this._object) {
        var value = this._object[key];
        if (cache.include(key)) continue;
        cache.push(key);
        var pair = [key, value];
        pair.key = key;
        pair.value = value;
        iterator(pair);
      }
    }
  } else {
    function each(iterator) {
      for (var key in this._object) {
        var value = this._object[key], pair = [key, value];
        pair.key = key;
        pair.value = value;
        iterator(pair);
      }
    }
  }

  function toQueryPair(key, value) {
    if (Object.isUndefined(value)) return key;
    return key + '=' + encodeURIComponent(String.interpret(value));
  }

  return {
    initialize: function(object) {
      this._object = Object.isHash(object) ? object.toObject() : Object.clone(object);
    },

    _each: each,

    set: function(key, value) {
      return this._object[key] = value;
    },

    get: function(key) {
      return this._object[key];
    },

    unset: function(key) {
      var value = this._object[key];
      delete this._object[key];
      return value;
    },

    toObject: function() {
      return Object.clone(this._object);
    },

    keys: function() {
      return this.pluck('key');
    },

    values: function() {
      return this.pluck('value');
    },

    index: function(value) {
      var match = this.detect(function(pair) {
        return pair.value === value;
      });
      return match && match.key;
    },

    merge: function(object) {
      return this.clone().update(object);
    },

    update: function(object) {
      return new Hash(object).inject(this, function(result, pair) {
        result.set(pair.key, pair.value);
        return result;
      });
    },

    toQueryString: function() {
      return this.map(function(pair) {
        var key = encodeURIComponent(pair.key), values = pair.value;

        if (values && typeof values == 'object') {
          if (Object.isArray(values))
            return values.map(toQueryPair.curry(key)).join('&');
        }
        return toQueryPair(key, values);
      }).join('&');
    },

    inspect: function() {
      return '#<Hash:{' + this.map(function(pair) {
        return pair.map(Object.inspect).join(': ');
      }).join(', ') + '}>';
    },

    toJSON: function() {
      return Object.toJSON(this.toObject());
    },

    clone: function() {
      return new Hash(this);
    }
  }
})());

Hash.prototype.toTemplateReplacements = Hash.prototype.toObject;
Hash.from = $H;
var ObjectRange = Class.create(Enumerable, {
  initialize: function(start, end, exclusive) {
    this.start = start;
    this.end = end;
    this.exclusive = exclusive;
  },

  _each: function(iterator) {
    var value = this.start;
    while (this.include(value)) {
      iterator(value);
      value = value.succ();
    }
  },

  include: function(value) {
    if (value < this.start)
      return false;
    if (this.exclusive)
      return value < this.end;
    return value <= this.end;
  }
});

var $R = function(start, end, exclusive) {
  return new ObjectRange(start, end, exclusive);
};

var Ajax = {
  getTransport: function() {
    return Try.these(
      function() {return new XMLHttpRequest()},
      function() {return new ActiveXObject('Msxml2.XMLHTTP')},
      function() {return new ActiveXObject('Microsoft.XMLHTTP')}
    ) || false;
  },

  activeRequestCount: 0
};

Ajax.Responders = {
  responders: [],

  _each: function(iterator) {
    this.responders._each(iterator);
  },

  register: function(responder) {
    if (!this.include(responder))
      this.responders.push(responder);
  },

  unregister: function(responder) {
    this.responders = this.responders.without(responder);
  },

  dispatch: function(callback, request, transport, json) {
    this.each(function(responder) {
      if (Object.isFunction(responder[callback])) {
        try {
          responder[callback].apply(responder, [request, transport, json]);
        } catch (e) { }
      }
    });
  }
};

Object.extend(Ajax.Responders, Enumerable);

Ajax.Responders.register({
  onCreate:   function() { Ajax.activeRequestCount++ },
  onComplete: function() { Ajax.activeRequestCount-- }
});

Ajax.Base = Class.create({
  initialize: function(options) {
    this.options = {
      method:       'post',
      asynchronous: true,
      contentType:  'application/x-www-form-urlencoded',
      encoding:     'UTF-8',
      parameters:   '',
      evalJSON:     true,
      evalJS:       true
    };
    Object.extend(this.options, options || { });

    this.options.method = this.options.method.toLowerCase();
    if (Object.isString(this.options.parameters))
      this.options.parameters = this.options.parameters.toQueryParams();
  }
});

Ajax.Request = Class.create(Ajax.Base, {
  _complete: false,

  initialize: function($super, url, options) {
    $super(options);
    this.transport = Ajax.getTransport();
    this.request(url);
  },

  request: function(url) {
    this.url = url;
    this.method = this.options.method;
    var params = Object.clone(this.options.parameters);

    if (!['get', 'post'].include(this.method)) {
      // simulate other verbs over post
      params['_method'] = this.method;
      this.method = 'post';
    }

    this.parameters = params;

    if (params = Object.toQueryString(params)) {
      // when GET, append parameters to URL
      if (this.method == 'get')
        this.url += (this.url.include('?') ? '&' : '?') + params;
      else if (/Konqueror|Safari|KHTML/.test(navigator.userAgent))
        params += '&_=';
    }

    try {
      var response = new Ajax.Response(this);
      if (this.options.onCreate) this.options.onCreate(response);
      Ajax.Responders.dispatch('onCreate', this, response);

      this.transport.open(this.method.toUpperCase(), this.url,
        this.options.asynchronous);

      if (this.options.asynchronous) this.respondToReadyState.bind(this).defer(1);

      this.transport.onreadystatechange = this.onStateChange.bind(this);
      this.setRequestHeaders();

      this.body = this.method == 'post' ? (this.options.postBody || params) : null;
      this.transport.send(this.body);

      /* Force Firefox to handle ready state 4 for synchronous requests */
      if (!this.options.asynchronous && this.transport.overrideMimeType)
        this.onStateChange();

    }
    catch (e) {
      this.dispatchException(e);
    }
  },

  onStateChange: function() {
    var readyState = this.transport.readyState;
    if (readyState > 1 && !((readyState == 4) && this._complete))
      this.respondToReadyState(this.transport.readyState);
  },

  setRequestHeaders: function() {
    var headers = {
      'X-Requested-With': 'XMLHttpRequest',
      'X-Prototype-Version': Prototype.Version,
      'Accept': 'text/javascript, text/html, application/xml, text/xml, */*'
    };

    if (this.method == 'post') {
      headers['Content-type'] = this.options.contentType +
        (this.options.encoding ? '; charset=' + this.options.encoding : '');

      /* Force "Connection: close" for older Mozilla browsers to work
       * around a bug where XMLHttpRequest sends an incorrect
       * Content-length header. See Mozilla Bugzilla #246651.
       */
      if (this.transport.overrideMimeType &&
          (navigator.userAgent.match(/Gecko\/(\d{4})/) || [0,2005])[1] < 2005)
            headers['Connection'] = 'close';
    }

    // user-defined headers
    if (typeof this.options.requestHeaders == 'object') {
      var extras = this.options.requestHeaders;

      if (Object.isFunction(extras.push))
        for (var i = 0, length = extras.length; i < length; i += 2)
          headers[extras[i]] = extras[i+1];
      else
        $H(extras).each(function(pair) { headers[pair.key] = pair.value });
    }

    for (var name in headers)
      this.transport.setRequestHeader(name, headers[name]);
  },

  success: function() {
    var status = this.getStatus();
    return !status || (status >= 200 && status < 300);
  },

  getStatus: function() {
    try {
      return this.transport.status || 0;
    } catch (e) { return 0 }
  },

  respondToReadyState: function(readyState) {
    var state = Ajax.Request.Events[readyState], response = new Ajax.Response(this);

    if (state == 'Complete') {
      try {
        this._complete = true;
        (this.options['on' + response.status]
         || this.options['on' + (this.success() ? 'Success' : 'Failure')]
         || Prototype.emptyFunction)(response, response.headerJSON);
      } catch (e) {
        this.dispatchException(e);
      }

      var contentType = response.getHeader('Content-type');
      if (this.options.evalJS == 'force'
          || (this.options.evalJS && contentType
          && contentType.match(/^\s*(text|application)\/(x-)?(java|ecma)script(;.*)?\s*$/i)))
        this.evalResponse();
    }

    try {
      (this.options['on' + state] || Prototype.emptyFunction)(response, response.headerJSON);
      Ajax.Responders.dispatch('on' + state, this, response, response.headerJSON);
    } catch (e) {
      this.dispatchException(e);
    }

    if (state == 'Complete') {
      // avoid memory leak in MSIE: clean up
      this.transport.onreadystatechange = Prototype.emptyFunction;
    }
  },

  getHeader: function(name) {
    try {
      return this.transport.getResponseHeader(name);
    } catch (e) { return null }
  },

  evalResponse: function() {
    try {
      return eval((this.transport.responseText || '').unfilterJSON());
    } catch (e) {
      this.dispatchException(e);
    }
  },

  dispatchException: function(exception) {
    (this.options.onException || Prototype.emptyFunction)(this, exception);
    Ajax.Responders.dispatch('onException', this, exception);
  }
});

Ajax.Request.Events =
  ['Uninitialized', 'Loading', 'Loaded', 'Interactive', 'Complete'];

Ajax.Response = Class.create({
  initialize: function(request){
    this.request = request;
    var transport  = this.transport  = request.transport,
        readyState = this.readyState = transport.readyState;

    if((readyState > 2 && !Prototype.Browser.IE) || readyState == 4) {
      this.status       = this.getStatus();
      this.statusText   = this.getStatusText();
      this.responseText = String.interpret(transport.responseText);
      this.headerJSON   = this._getHeaderJSON();
    }

    if(readyState == 4) {
      var xml = transport.responseXML;
      this.responseXML  = xml === undefined ? null : xml;
      this.responseJSON = this._getResponseJSON();
    }
  },

  status:      0,
  statusText: '',

  getStatus: Ajax.Request.prototype.getStatus,

  getStatusText: function() {
    try {
      return this.transport.statusText || '';
    } catch (e) { return '' }
  },

  getHeader: Ajax.Request.prototype.getHeader,

  getAllHeaders: function() {
    try {
      return this.getAllResponseHeaders();
    } catch (e) { return null }
  },

  getResponseHeader: function(name) {
    return this.transport.getResponseHeader(name);
  },

  getAllResponseHeaders: function() {
    return this.transport.getAllResponseHeaders();
  },

  _getHeaderJSON: function() {
    var json = this.getHeader('X-JSON');
    if (!json) return null;
    json = decodeURIComponent(escape(json));
    try {
      return json.evalJSON(this.request.options.sanitizeJSON);
    } catch (e) {
      this.request.dispatchException(e);
    }
  },

  _getResponseJSON: function() {
    var options = this.request.options;
    if (!options.evalJSON || (options.evalJSON != 'force' &&
      !(this.getHeader('Content-type') || '').include('application/json')))
        return null;
    try {
      return this.transport.responseText.evalJSON(options.sanitizeJSON);
    } catch (e) {
      this.request.dispatchException(e);
    }
  }
});

Ajax.Updater = Class.create(Ajax.Request, {
  initialize: function($super, container, url, options) {
    this.container = {
      success: (container.success || container),
      failure: (container.failure || (container.success ? null : container))
    };

    options = options || { };
    var onComplete = options.onComplete;
    options.onComplete = (function(response, param) {
      this.updateContent(response.responseText);
      if (Object.isFunction(onComplete)) onComplete(response, param);
    }).bind(this);

    $super(url, options);
  },

  updateContent: function(responseText) {
    var receiver = this.container[this.success() ? 'success' : 'failure'],
        options = this.options;

    if (!options.evalScripts) responseText = responseText.stripScripts();

    if (receiver = $(receiver)) {
      if (options.insertion) {
        if (Object.isString(options.insertion)) {
          var insertion = { }; insertion[options.insertion] = responseText;
          receiver.insert(insertion);
        }
        else options.insertion(receiver, responseText);
      }
      else receiver.update(responseText);
    }

    if (this.success()) {
      if (this.onComplete) this.onComplete.bind(this).defer();
    }
  }
});

Ajax.PeriodicalUpdater = Class.create(Ajax.Base, {
  initialize: function($super, container, url, options) {
    $super(options);
    this.onComplete = this.options.onComplete;

    this.frequency = (this.options.frequency || 2);
    this.decay = (this.options.decay || 1);

    this.updater = { };
    this.container = container;
    this.url = url;

    this.start();
  },

  start: function() {
    this.options.onComplete = this.updateComplete.bind(this);
    this.onTimerEvent();
  },

  stop: function() {
    this.updater.options.onComplete = undefined;
    clearTimeout(this.timer);
    (this.onComplete || Prototype.emptyFunction).apply(this, arguments);
  },

  updateComplete: function(response) {
    if (this.options.decay) {
      this.decay = (response.responseText == this.lastText ?
        this.decay * this.options.decay : 1);

      this.lastText = response.responseText;
    }
    this.timer = this.onTimerEvent.bind(this).delay(this.decay * this.frequency);
  },

  onTimerEvent: function() {
    this.updater = new Ajax.Updater(this.container, this.url, this.options);
  }
});

function $(element) {
  if (arguments.length > 1) {
    for (var i = 0, elements = [], length = arguments.length; i < length; i++)
      elements.push($(arguments[i]));
    return elements;
  }
  if (Object.isString(element))
    element = document.getElementById(element);
  return Element.extend(element);
}

if (Prototype.BrowserFeatures.XPath) {
  document._getElementsByXPath = function(expression, parentElement) {
    var results = [];
    var query = document.evaluate(expression, $(parentElement) || document,
      null, XPathResult.ORDERED_NODE_SNAPSHOT_TYPE, null);
    for (var i = 0, length = query.snapshotLength; i < length; i++)
      results.push(Element.extend(query.snapshotItem(i)));
    return results;
  };
}

/*--------------------------------------------------------------------------*/

if (!window.Node) var Node = { };

if (!Node.ELEMENT_NODE) {
  // DOM level 2 ECMAScript Language Binding
  Object.extend(Node, {
    ELEMENT_NODE: 1,
    ATTRIBUTE_NODE: 2,
    TEXT_NODE: 3,
    CDATA_SECTION_NODE: 4,
    ENTITY_REFERENCE_NODE: 5,
    ENTITY_NODE: 6,
    PROCESSING_INSTRUCTION_NODE: 7,
    COMMENT_NODE: 8,
    DOCUMENT_NODE: 9,
    DOCUMENT_TYPE_NODE: 10,
    DOCUMENT_FRAGMENT_NODE: 11,
    NOTATION_NODE: 12
  });
}

(function() {
  var element = this.Element;
  this.Element = function(tagName, attributes) {
    attributes = attributes || { };
    tagName = tagName.toLowerCase();
    var cache = Element.cache;
    if (Prototype.Browser.IE && attributes.name) {
      tagName = '<' + tagName + ' name="' + attributes.name + '">';
      delete attributes.name;
      return Element.writeAttribute(document.createElement(tagName), attributes);
    }
    if (!cache[tagName]) cache[tagName] = Element.extend(document.createElement(tagName));
    return Element.writeAttribute(cache[tagName].cloneNode(false), attributes);
  };
  Object.extend(this.Element, element || { });
}).call(window);

Element.cache = { };

Element.Methods = {
  visible: function(element) {
    return $(element).style.display != 'none';
  },

  toggle: function(element) {
    element = $(element);
    Element[Element.visible(element) ? 'hide' : 'show'](element);
    return element;
  },

  hide: function(element) {
    $(element).style.display = 'none';
    return element;
  },

  show: function(element) {
    $(element).style.display = '';
    return element;
  },

  remove: function(element) {
    element = $(element);
    element.parentNode.removeChild(element);
    return element;
  },

  update: function(element, content) {
    element = $(element);
    if (content && content.toElement) content = content.toElement();
    if (Object.isElement(content)) return element.update().insert(content);
    content = Object.toHTML(content);
    element.innerHTML = content.stripScripts();
    content.evalScripts.bind(content).defer();
    return element;
  },

  replace: function(element, content) {
    element = $(element);
    if (content && content.toElement) content = content.toElement();
    else if (!Object.isElement(content)) {
      content = Object.toHTML(content);
      var range = element.ownerDocument.createRange();
      range.selectNode(element);
      content.evalScripts.bind(content).defer();
      content = range.createContextualFragment(content.stripScripts());
    }
    element.parentNode.replaceChild(content, element);
    return element;
  },

  insert: function(element, insertions) {
    element = $(element);

    if (Object.isString(insertions) || Object.isNumber(insertions) ||
        Object.isElement(insertions) || (insertions && (insertions.toElement || insertions.toHTML)))
          insertions = {bottom:insertions};

    var content, t, range;

    for (position in insertions) {
      content  = insertions[position];
      position = position.toLowerCase();
      t = Element._insertionTranslations[position];

      if (content && content.toElement) content = content.toElement();
      if (Object.isElement(content)) {
        t.insert(element, content);
        continue;
      }

      content = Object.toHTML(content);

      range = element.ownerDocument.createRange();
      t.initializeRange(element, range);
      t.insert(element, range.createContextualFragment(content.stripScripts()));

      content.evalScripts.bind(content).defer();
    }

    return element;
  },

  wrap: function(element, wrapper, attributes) {
    element = $(element);
    if (Object.isElement(wrapper))
      $(wrapper).writeAttribute(attributes || { });
    else if (Object.isString(wrapper)) wrapper = new Element(wrapper, attributes);
    else wrapper = new Element('div', wrapper);
    if (element.parentNode)
      element.parentNode.replaceChild(wrapper, element);
    wrapper.appendChild(element);
    return wrapper;
  },

  inspect: function(element) {
    element = $(element);
    var result = '<' + element.tagName.toLowerCase();
    $H({'id': 'id', 'className': 'class'}).each(function(pair) {
      var property = pair.first(), attribute = pair.last();
      var value = (element[property] || '').toString();
      if (value) result += ' ' + attribute + '=' + value.inspect(true);
    });
    return result + '>';
  },

  recursivelyCollect: function(element, property) {
    element = $(element);
    var elements = [];
    while (element = element[property])
      if (element.nodeType == 1)
        elements.push(Element.extend(element));
    return elements;
  },

  ancestors: function(element) {
    return $(element).recursivelyCollect('parentNode');
  },

  descendants: function(element) {
    return $A($(element).getElementsByTagName('*')).each(Element.extend);
  },

  firstDescendant: function(element) {
    element = $(element).firstChild;
    while (element && element.nodeType != 1) element = element.nextSibling;
    return $(element);
  },

  immediateDescendants: function(element) {
    if (!(element = $(element).firstChild)) return [];
    while (element && element.nodeType != 1) element = element.nextSibling;
    if (element) return [element].concat($(element).nextSiblings());
    return [];
  },

  previousSiblings: function(element) {
    return $(element).recursivelyCollect('previousSibling');
  },

  nextSiblings: function(element) {
    return $(element).recursivelyCollect('nextSibling');
  },

  siblings: function(element) {
    element = $(element);
    return element.previousSiblings().reverse().concat(element.nextSiblings());
  },

  match: function(element, selector) {
    if (Object.isString(selector))
      selector = new Selector(selector);
    return selector.match($(element));
  },

  up: function(element, expression, index) {
    element = $(element);
    if (arguments.length == 1) return $(element.parentNode);
    var ancestors = element.ancestors();
    return expression ? Selector.findElement(ancestors, expression, index) :
      ancestors[index || 0];
  },

  down: function(element, expression, index) {
    element = $(element);
    if (arguments.length == 1) return element.firstDescendant();
    var descendants = element.descendants();
    return expression ? Selector.findElement(descendants, expression, index) :
      descendants[index || 0];
  },

  previous: function(element, expression, index) {
    element = $(element);
    if (arguments.length == 1) return $(Selector.handlers.previousElementSibling(element));
    var previousSiblings = element.previousSiblings();
    return expression ? Selector.findElement(previousSiblings, expression, index) :
      previousSiblings[index || 0];
  },

  next: function(element, expression, index) {
    element = $(element);
    if (arguments.length == 1) return $(Selector.handlers.nextElementSibling(element));
    var nextSiblings = element.nextSiblings();
    return expression ? Selector.findElement(nextSiblings, expression, index) :
      nextSiblings[index || 0];
  },

  select: function() {
    var args = $A(arguments), element = $(args.shift());
    return Selector.findChildElements(element, args);
  },

  adjacent: function() {
    var args = $A(arguments), element = $(args.shift());
    return Selector.findChildElements(element.parentNode, args).without(element);
  },

  identify: function(element) {
    element = $(element);
    var id = element.readAttribute('id'), self = arguments.callee;
    if (id) return id;
    do { id = 'anonymous_element_' + self.counter++ } while ($(id));
    element.writeAttribute('id', id);
    return id;
  },

  readAttribute: function(element, name) {
    element = $(element);
    if (Prototype.Browser.IE) {
      var t = Element._attributeTranslations.read;
      if (t.values[name]) return t.values[name](element, name);
      if (t.names[name]) name = t.names[name];
      if (name.include(':')) {
        return (!element.attributes || !element.attributes[name]) ? null :
         element.attributes[name].value;
      }
    }
    return element.getAttribute(name);
  },

  writeAttribute: function(element, name, value) {
    element = $(element);
    var attributes = { }, t = Element._attributeTranslations.write;

    if (typeof name == 'object') attributes = name;
    else attributes[name] = value === undefined ? true : value;

    for (var attr in attributes) {
      var name = t.names[attr] || attr, value = attributes[attr];
      if (t.values[attr]) name = t.values[attr](element, value);
      if (value === false || value === null)
        element.removeAttribute(name);
      else if (value === true)
        element.setAttribute(name, name);
      else element.setAttribute(name, value);
    }
    return element;
  },

  getHeight: function(element) {
    return $(element).getDimensions().height;
  },

  getWidth: function(element) {
    return $(element).getDimensions().width;
  },

  classNames: function(element) {
    return new Element.ClassNames(element);
  },

  hasClassName: function(element, className) {
    if (!(element = $(element))) return;
    var elementClassName = element.className;
    return (elementClassName.length > 0 && (elementClassName == className ||
      new RegExp("(^|\\s)" + className + "(\\s|$)").test(elementClassName)));
  },

  addClassName: function(element, className) {
    if (!(element = $(element))) return;
    if (!element.hasClassName(className))
      element.className += (element.className ? ' ' : '') + className;
    return element;
  },

  removeClassName: function(element, className) {
    if (!(element = $(element))) return;
    element.className = element.className.replace(
      new RegExp("(^|\\s+)" + className + "(\\s+|$)"), ' ').strip();
    return element;
  },

  toggleClassName: function(element, className) {
    if (!(element = $(element))) return;
    return element[element.hasClassName(className) ?
      'removeClassName' : 'addClassName'](className);
  },

  // removes whitespace-only text node children
  cleanWhitespace: function(element) {
    element = $(element);
    var node = element.firstChild;
    while (node) {
      var nextNode = node.nextSibling;
      if (node.nodeType == 3 && !/\S/.test(node.nodeValue))
        element.removeChild(node);
      node = nextNode;
    }
    return element;
  },

  empty: function(element) {
    return $(element).innerHTML.blank();
  },

  descendantOf: function(element, ancestor) {
    element = $(element), ancestor = $(ancestor);

    if (element.compareDocumentPosition)
      return (element.compareDocumentPosition(ancestor) & 8) === 8;

    if (element.sourceIndex && !Prototype.Browser.Opera) {
      var e = element.sourceIndex, a = ancestor.sourceIndex,
       nextAncestor = ancestor.nextSibling;
      if (!nextAncestor) {
        do { ancestor = ancestor.parentNode; }
        while (!(nextAncestor = ancestor.nextSibling) && ancestor.parentNode);
      }
      if (nextAncestor) return (e > a && e < nextAncestor.sourceIndex);
    }

    while (element = element.parentNode)
      if (element == ancestor) return true;
    return false;
  },

  scrollTo: function(element) {
    element = $(element);
    var pos = element.cumulativeOffset();
    window.scrollTo(pos[0], pos[1]);
    return element;
  },

  getStyle: function(element, style) {
    element = $(element);
    style = style == 'float' ? 'cssFloat' : style.camelize();
    var value = element.style[style];
    if (!value) {
      var css = document.defaultView.getComputedStyle(element, null);
      value = css ? css[style] : null;
    }
    if (style == 'opacity') return value ? parseFloat(value) : 1.0;
    return value == 'auto' ? null : value;
  },

  getOpacity: function(element) {
    return $(element).getStyle('opacity');
  },

  setStyle: function(element, styles) {
    element = $(element);
    var elementStyle = element.style, match;
    if (Object.isString(styles)) {
      element.style.cssText += ';' + styles;
      return styles.include('opacity') ?
        element.setOpacity(styles.match(/opacity:\s*(\d?\.?\d*)/)[1]) : element;
    }
    for (var property in styles)
      if (property == 'opacity') element.setOpacity(styles[property]);
      else
        elementStyle[(property == 'float' || property == 'cssFloat') ?
          (elementStyle.styleFloat === undefined ? 'cssFloat' : 'styleFloat') :
            property] = styles[property];

    return element;
  },

  setOpacity: function(element, value) {
    element = $(element);
    element.style.opacity = (value == 1 || value === '') ? '' :
      (value < 0.00001) ? 0 : value;
    return element;
  },

  getDimensions: function(element) {
    element = $(element);
    var display = $(element).getStyle('display');
    if (display != 'none' && display != null) // Safari bug
      return {width: element.offsetWidth, height: element.offsetHeight};

    // All *Width and *Height properties give 0 on elements with display none,
    // so enable the element temporarily
    var els = element.style;
    var originalVisibility = els.visibility;
    var originalPosition = els.position;
    var originalDisplay = els.display;
    els.visibility = 'hidden';
    els.position = 'absolute';
    els.display = 'block';
    var originalWidth = element.clientWidth;
    var originalHeight = element.clientHeight;
    els.display = originalDisplay;
    els.position = originalPosition;
    els.visibility = originalVisibility;
    return {width: originalWidth, height: originalHeight};
  },

  makePositioned: function(element) {
    element = $(element);
    var pos = Element.getStyle(element, 'position');
    if (pos == 'static' || !pos) {
      element._madePositioned = true;
      element.style.position = 'relative';
      // Opera returns the offset relative to the positioning context, when an
      // element is position relative but top and left have not been defined
      if (window.opera) {
        element.style.top = 0;
        element.style.left = 0;
      }
    }
    return element;
  },

  undoPositioned: function(element) {
    element = $(element);
    if (element._madePositioned) {
      element._madePositioned = undefined;
      element.style.position =
        element.style.top =
        element.style.left =
        element.style.bottom =
        element.style.right = '';
    }
    return element;
  },

  makeClipping: function(element) {
    element = $(element);
    if (element._overflow) return element;
    element._overflow = Element.getStyle(element, 'overflow') || 'auto';
    if (element._overflow !== 'hidden')
      element.style.overflow = 'hidden';
    return element;
  },

  undoClipping: function(element) {
    element = $(element);
    if (!element._overflow) return element;
    element.style.overflow = element._overflow == 'auto' ? '' : element._overflow;
    element._overflow = null;
    return element;
  },

  cumulativeOffset: function(element) {
    var valueT = 0, valueL = 0;
    do {
      valueT += element.offsetTop  || 0;
      valueL += element.offsetLeft || 0;
      element = element.offsetParent;
    } while (element);
    return Element._returnOffset(valueL, valueT);
  },

  positionedOffset: function(element) {
    var valueT = 0, valueL = 0;
    do {
      valueT += element.offsetTop  || 0;
      valueL += element.offsetLeft || 0;
      element = element.offsetParent;
      if (element) {
        if (element.tagName == 'BODY') break;
        var p = Element.getStyle(element, 'position');
        if (p == 'relative' || p == 'absolute') break;
      }
    } while (element);
    return Element._returnOffset(valueL, valueT);
  },

  absolutize: function(element) {
    element = $(element);
    if (element.getStyle('position') == 'absolute') return;
    // Position.prepare(); // To be done manually by Scripty when it needs it.

    var offsets = element.positionedOffset();
    var top     = offsets[1];
    var left    = offsets[0];
    var width   = element.clientWidth;
    var height  = element.clientHeight;

    element._originalLeft   = left - parseFloat(element.style.left  || 0);
    element._originalTop    = top  - parseFloat(element.style.top || 0);
    element._originalWidth  = element.style.width;
    element._originalHeight = element.style.height;

    element.style.position = 'absolute';
    element.style.top    = top + 'px';
    element.style.left   = left + 'px';
    element.style.width  = width + 'px';
    element.style.height = height + 'px';
    return element;
  },

  relativize: function(element) {
    element = $(element);
    if (element.getStyle('position') == 'relative') return;
    // Position.prepare(); // To be done manually by Scripty when it needs it.

    element.style.position = 'relative';
    var top  = parseFloat(element.style.top  || 0) - (element._originalTop || 0);
    var left = parseFloat(element.style.left || 0) - (element._originalLeft || 0);

    element.style.top    = top + 'px';
    element.style.left   = left + 'px';
    element.style.height = element._originalHeight;
    element.style.width  = element._originalWidth;
    return element;
  },

  cumulativeScrollOffset: function(element) {
    var valueT = 0, valueL = 0;
    do {
      valueT += element.scrollTop  || 0;
      valueL += element.scrollLeft || 0;
      element = element.parentNode;
    } while (element);
    return Element._returnOffset(valueL, valueT);
  },

  getOffsetParent: function(element) {
    if (element.offsetParent) return $(element.offsetParent);
    if (element == document.body) return $(element);

    while ((element = element.parentNode) && element != document.body)
      if (Element.getStyle(element, 'position') != 'static')
        return $(element);

    return $(document.body);
  },

  viewportOffset: function(forElement) {
    var valueT = 0, valueL = 0;

    var element = forElement;
    do {
      valueT += element.offsetTop  || 0;
      valueL += element.offsetLeft || 0;

      // Safari fix
      if (element.offsetParent == document.body &&
        Element.getStyle(element, 'position') == 'absolute') break;

    } while (element = element.offsetParent);

    element = forElement;
    do {
      if (!Prototype.Browser.Opera || element.tagName == 'BODY') {
        valueT -= element.scrollTop  || 0;
        valueL -= element.scrollLeft || 0;
      }
    } while (element = element.parentNode);

    return Element._returnOffset(valueL, valueT);
  },

  clonePosition: function(element, source) {
    var options = Object.extend({
      setLeft:    true,
      setTop:     true,
      setWidth:   true,
      setHeight:  true,
      offsetTop:  0,
      offsetLeft: 0
    }, arguments[2] || { });

    // find page position of source
    source = $(source);
    var p = source.viewportOffset();

    // find coordinate system to use
    element = $(element);
    var delta = [0, 0];
    var parent = null;
    // delta [0,0] will do fine with position: fixed elements,
    // position:absolute needs offsetParent deltas
    if (Element.getStyle(element, 'position') == 'absolute') {
      parent = element.getOffsetParent();
      delta = parent.viewportOffset();
    }

    // correct by body offsets (fixes Safari)
    if (parent == document.body) {
      delta[0] -= document.body.offsetLeft;
      delta[1] -= document.body.offsetTop;
    }

    // set position
    if (options.setLeft)   element.style.left  = (p[0] - delta[0] + options.offsetLeft) + 'px';
    if (options.setTop)    element.style.top   = (p[1] - delta[1] + options.offsetTop) + 'px';
    if (options.setWidth)  element.style.width = source.offsetWidth + 'px';
    if (options.setHeight) element.style.height = source.offsetHeight + 'px';
    return element;
  }
};

Element.Methods.identify.counter = 1;

Object.extend(Element.Methods, {
  getElementsBySelector: Element.Methods.select,
  childElements: Element.Methods.immediateDescendants
});

Element._attributeTranslations = {
  write: {
    names: {
      className: 'class',
      htmlFor:   'for'
    },
    values: { }
  }
};


if (!document.createRange || Prototype.Browser.Opera) {
  Element.Methods.insert = function(element, insertions) {
    element = $(element);

    if (Object.isString(insertions) || Object.isNumber(insertions) ||
        Object.isElement(insertions) || (insertions && (insertions.toElement || insertions.toHTML)))
          insertions = { bottom: insertions };

    var t = Element._insertionTranslations, content, position, pos, tagName;

    for (position in insertions) {
      content  = insertions[position];
      position = position.toLowerCase();
      pos      = t[position];

      if (content && content.toElement) content = content.toElement();
      if (Object.isElement(content)) {
        pos.insert(element, content);
        continue;
      }

      content = Object.toHTML(content);
      tagName = ((position == 'before' || position == 'after')
        ? element.parentNode : element).tagName.toUpperCase();

      if (t.tags[tagName]) {
        var fragments = Element._getContentFromAnonymousElement(tagName, content.stripScripts());
        if (position == 'top' || position == 'after') fragments.reverse();
        fragments.each(pos.insert.curry(element));
      }
      else element.insertAdjacentHTML(pos.adjacency, content.stripScripts());

      content.evalScripts.bind(content).defer();
    }

    return element;
  };
}

if (Prototype.Browser.Opera) {
  Element.Methods._getStyle = Element.Methods.getStyle;
  Element.Methods.getStyle = function(element, style) {
    switch(style) {
      case 'left':
      case 'top':
      case 'right':
      case 'bottom':
        if (Element._getStyle(element, 'position') == 'static') return null;
      default: return Element._getStyle(element, style);
    }
  };
  Element.Methods._readAttribute = Element.Methods.readAttribute;
  Element.Methods.readAttribute = function(element, attribute) {
    if (attribute == 'title') return element.title;
    return Element._readAttribute(element, attribute);
  };
}

else if (Prototype.Browser.IE) {
  $w('positionedOffset getOffsetParent viewportOffset').each(function(method) {
    Element.Methods[method] = Element.Methods[method].wrap(
      function(proceed, element) {
        element = $(element);
        var position = element.getStyle('position');
        if (position != 'static') return proceed(element);
        element.setStyle({ position: 'relative' });
        var value = proceed(element);
        element.setStyle({ position: position });
        return value;
      }
    );
  });

  Element.Methods.getStyle = function(element, style) {
    element = $(element);
    style = (style == 'float' || style == 'cssFloat') ? 'styleFloat' : style.camelize();
    var value = element.style[style];
    if (!value && element.currentStyle) value = element.currentStyle[style];

    if (style == 'opacity') {
      if (value = (element.getStyle('filter') || '').match(/alpha\(opacity=(.*)\)/))
        if (value[1]) return parseFloat(value[1]) / 100;
      return 1.0;
    }

    if (value == 'auto') {
      if ((style == 'width' || style == 'height') && (element.getStyle('display') != 'none'))
        return element['offset' + style.capitalize()] + 'px';
      return null;
    }
    return value;
  };

  Element.Methods.setOpacity = function(element, value) {
    function stripAlpha(filter){
      return filter.replace(/alpha\([^\)]*\)/gi,'');
    }
    element = $(element);
    var currentStyle = element.currentStyle;
    if ((currentStyle && !currentStyle.hasLayout) ||
      (!currentStyle && element.style.zoom == 'normal'))
        element.style.zoom = 1;

    var filter = element.getStyle('filter'), style = element.style;
    if (value == 1 || value === '') {
      (filter = stripAlpha(filter)) ?
        style.filter = filter : style.removeAttribute('filter');
      return element;
    } else if (value < 0.00001) value = 0;
    style.filter = stripAlpha(filter) +
      'alpha(opacity=' + (value * 100) + ')';
    return element;
  };

  Element._attributeTranslations = {
    read: {
      names: {
        'class': 'className',
        'for':   'htmlFor'
      },
      values: {
        _getAttr: function(element, attribute) {
          return element.getAttribute(attribute, 2);
        },
        _getAttrNode: function(element, attribute) {
          var node = element.getAttributeNode(attribute);
          return node ? node.value : "";
        },
        _getEv: function(element, attribute) {
          var attribute = element.getAttribute(attribute);
          return attribute ? attribute.toString().slice(23, -2) : null;
        },
        _flag: function(element, attribute) {
          return $(element).hasAttribute(attribute) ? attribute : null;
        },
        style: function(element) {
          return element.style.cssText.toLowerCase();
        },
        title: function(element) {
          return element.title;
        }
      }
    }
  };

  Element._attributeTranslations.write = {
    names: Object.clone(Element._attributeTranslations.read.names),
    values: {
      checked: function(element, value) {
        element.checked = !!value;
      },

      style: function(element, value) {
        element.style.cssText = value ? value : '';
      }
    }
  };

  Element._attributeTranslations.has = {};

  $w('colSpan rowSpan vAlign dateTime accessKey tabIndex ' +
      'encType maxLength readOnly longDesc').each(function(attr) {
    Element._attributeTranslations.write.names[attr.toLowerCase()] = attr;
    Element._attributeTranslations.has[attr.toLowerCase()] = attr;
  });

  (function(v) {
    Object.extend(v, {
      href:        v._getAttr,
      src:         v._getAttr,
      type:        v._getAttr,
      action:      v._getAttrNode,
      disabled:    v._flag,
      checked:     v._flag,
      readonly:    v._flag,
      multiple:    v._flag,
      onload:      v._getEv,
      onunload:    v._getEv,
      onclick:     v._getEv,
      ondblclick:  v._getEv,
      onmousedown: v._getEv,
      onmouseup:   v._getEv,
      onmouseover: v._getEv,
      onmousemove: v._getEv,
      onmouseout:  v._getEv,
      onfocus:     v._getEv,
      onblur:      v._getEv,
      onkeypress:  v._getEv,
      onkeydown:   v._getEv,
      onkeyup:     v._getEv,
      onsubmit:    v._getEv,
      onreset:     v._getEv,
      onselect:    v._getEv,
      onchange:    v._getEv
    });
  })(Element._attributeTranslations.read.values);
}

else if (Prototype.Browser.Gecko && /rv:1\.8\.0/.test(navigator.userAgent)) {
  Element.Methods.setOpacity = function(element, value) {
    element = $(element);
    element.style.opacity = (value == 1) ? 0.999999 :
      (value === '') ? '' : (value < 0.00001) ? 0 : value;
    return element;
  };
}

else if (Prototype.Browser.WebKit) {
  Element.Methods.setOpacity = function(element, value) {
    element = $(element);
    element.style.opacity = (value == 1 || value === '') ? '' :
      (value < 0.00001) ? 0 : value;

    if (value == 1)
      if(element.tagName == 'IMG' && element.width) {
        element.width++; element.width--;
      } else try {
        var n = document.createTextNode(' ');
        element.appendChild(n);
        element.removeChild(n);
      } catch (e) { }

    return element;
  };

  // Safari returns margins on body which is incorrect if the child is absolutely
  // positioned.  For performance reasons, redefine Position.cumulativeOffset for
  // KHTML/WebKit only.
  Element.Methods.cumulativeOffset = function(element) {
    var valueT = 0, valueL = 0;
    do {
      valueT += element.offsetTop  || 0;
      valueL += element.offsetLeft || 0;
      if (element.offsetParent == document.body)
        if (Element.getStyle(element, 'position') == 'absolute') break;

      element = element.offsetParent;
    } while (element);

    return Element._returnOffset(valueL, valueT);
  };
}

if (Prototype.Browser.IE || Prototype.Browser.Opera) {
  // IE and Opera are missing .innerHTML support for TABLE-related and SELECT elements
  Element.Methods.update = function(element, content) {
    element = $(element);

    if (content && content.toElement) content = content.toElement();
    if (Object.isElement(content)) return element.update().insert(content);

    content = Object.toHTML(content);
    var tagName = element.tagName.toUpperCase();

    if (tagName in Element._insertionTranslations.tags) {
      $A(element.childNodes).each(function(node) { element.removeChild(node) });
      Element._getContentFromAnonymousElement(tagName, content.stripScripts())
        .each(function(node) { element.appendChild(node) });
    }
    else element.innerHTML = content.stripScripts();

    content.evalScripts.bind(content).defer();
    return element;
  };
}

if (document.createElement('div').outerHTML) {
  Element.Methods.replace = function(element, content) {
    element = $(element);

    if (content && content.toElement) content = content.toElement();
    if (Object.isElement(content)) {
      element.parentNode.replaceChild(content, element);
      return element;
    }

    content = Object.toHTML(content);
    var parent = element.parentNode, tagName = parent.tagName.toUpperCase();

    if (Element._insertionTranslations.tags[tagName]) {
      var nextSibling = element.next();
      var fragments = Element._getContentFromAnonymousElement(tagName, content.stripScripts());
      parent.removeChild(element);
      if (nextSibling)
        fragments.each(function(node) { parent.insertBefore(node, nextSibling) });
      else
        fragments.each(function(node) { parent.appendChild(node) });
    }
    else element.outerHTML = content.stripScripts();

    content.evalScripts.bind(content).defer();
    return element;
  };
}

Element._returnOffset = function(l, t) {
  var result = [l, t];
  result.left = l;
  result.top = t;
  return result;
};

Element._getContentFromAnonymousElement = function(tagName, html) {
  var div = new Element('div'), t = Element._insertionTranslations.tags[tagName];
  div.innerHTML = t[0] + html + t[1];
  t[2].times(function() { div = div.firstChild });
  return $A(div.childNodes);
};

Element._insertionTranslations = {
  before: {
    adjacency: 'beforeBegin',
    insert: function(element, node) {
      element.parentNode.insertBefore(node, element);
    },
    initializeRange: function(element, range) {
      range.setStartBefore(element);
    }
  },
  top: {
    adjacency: 'afterBegin',
    insert: function(element, node) {
      element.insertBefore(node, element.firstChild);
    },
    initializeRange: function(element, range) {
      range.selectNodeContents(element);
      range.collapse(true);
    }
  },
  bottom: {
    adjacency: 'beforeEnd',
    insert: function(element, node) {
      element.appendChild(node);
    }
  },
  after: {
    adjacency: 'afterEnd',
    insert: function(element, node) {
      element.parentNode.insertBefore(node, element.nextSibling);
    },
    initializeRange: function(element, range) {
      range.setStartAfter(element);
    }
  },
  tags: {
    TABLE:  ['<table>',                '</table>',                   1],
    TBODY:  ['<table><tbody>',         '</tbody></table>',           2],
    TR:     ['<table><tbody><tr>',     '</tr></tbody></table>',      3],
    TD:     ['<table><tbody><tr><td>', '</td></tr></tbody></table>', 4],
    SELECT: ['<select>',               '</select>',                  1]
  }
};

(function() {
  this.bottom.initializeRange = this.top.initializeRange;
  Object.extend(this.tags, {
    THEAD: this.tags.TBODY,
    TFOOT: this.tags.TBODY,
    TH:    this.tags.TD
  });
}).call(Element._insertionTranslations);

Element.Methods.Simulated = {
  hasAttribute: function(element, attribute) {
    attribute = Element._attributeTranslations.has[attribute] || attribute;
    var node = $(element).getAttributeNode(attribute);
    return node && node.specified;
  }
};

Element.Methods.ByTag = { };

Object.extend(Element, Element.Methods);

if (!Prototype.BrowserFeatures.ElementExtensions &&
    document.createElement('div').__proto__) {
  window.HTMLElement = { };
  window.HTMLElement.prototype = document.createElement('div').__proto__;
  Prototype.BrowserFeatures.ElementExtensions = true;
}

Element.extend = (function() {
  if (Prototype.BrowserFeatures.SpecificElementExtensions)
    return Prototype.K;

  var Methods = { }, ByTag = Element.Methods.ByTag;

  var extend = Object.extend(function(element) {
    if (!element || element._extendedByPrototype ||
        element.nodeType != 1 || element == window) return element;

    var methods = Object.clone(Methods),
      tagName = element.tagName, property, value;

    // extend methods for specific tags
    if (ByTag[tagName]) Object.extend(methods, ByTag[tagName]);

    for (property in methods) {
      value = methods[property];
      if (Object.isFunction(value) && !(property in element))
        element[property] = value.methodize();
    }

    element._extendedByPrototype = Prototype.emptyFunction;
    return element;

  }, {
    refresh: function() {
      // extend methods for all tags (Safari doesn't need this)
      if (!Prototype.BrowserFeatures.ElementExtensions) {
        Object.extend(Methods, Element.Methods);
        Object.extend(Methods, Element.Methods.Simulated);
      }
    }
  });

  extend.refresh();
  return extend;
})();

Element.hasAttribute = function(element, attribute) {
  if (element.hasAttribute) return element.hasAttribute(attribute);
  return Element.Methods.Simulated.hasAttribute(element, attribute);
};

Element.addMethods = function(methods) {
  var F = Prototype.BrowserFeatures, T = Element.Methods.ByTag;

  if (!methods) {
    Object.extend(Form, Form.Methods);
    Object.extend(Form.Element, Form.Element.Methods);
    Object.extend(Element.Methods.ByTag, {
      "FORM":     Object.clone(Form.Methods),
      "INPUT":    Object.clone(Form.Element.Methods),
      "SELECT":   Object.clone(Form.Element.Methods),
      "TEXTAREA": Object.clone(Form.Element.Methods)
    });
  }

  if (arguments.length == 2) {
    var tagName = methods;
    methods = arguments[1];
  }

  if (!tagName) Object.extend(Element.Methods, methods || { });
  else {
    if (Object.isArray(tagName)) tagName.each(extend);
    else extend(tagName);
  }

  function extend(tagName) {
    tagName = tagName.toUpperCase();
    if (!Element.Methods.ByTag[tagName])
      Element.Methods.ByTag[tagName] = { };
    Object.extend(Element.Methods.ByTag[tagName], methods);
  }

  function copy(methods, destination, onlyIfAbsent) {
    onlyIfAbsent = onlyIfAbsent || false;
    for (var property in methods) {
      var value = methods[property];
      if (!Object.isFunction(value)) continue;
      if (!onlyIfAbsent || !(property in destination))
        destination[property] = value.methodize();
    }
  }

  function findDOMClass(tagName) {
    var klass;
    var trans = {
      "OPTGROUP": "OptGroup", "TEXTAREA": "TextArea", "P": "Paragraph",
      "FIELDSET": "FieldSet", "UL": "UList", "OL": "OList", "DL": "DList",
      "DIR": "Directory", "H1": "Heading", "H2": "Heading", "H3": "Heading",
      "H4": "Heading", "H5": "Heading", "H6": "Heading", "Q": "Quote",
      "INS": "Mod", "DEL": "Mod", "A": "Anchor", "IMG": "Image", "CAPTION":
      "TableCaption", "COL": "TableCol", "COLGROUP": "TableCol", "THEAD":
      "TableSection", "TFOOT": "TableSection", "TBODY": "TableSection", "TR":
      "TableRow", "TH": "TableCell", "TD": "TableCell", "FRAMESET":
      "FrameSet", "IFRAME": "IFrame"
    };
    if (trans[tagName]) klass = 'HTML' + trans[tagName] + 'Element';
    if (window[klass]) return window[klass];
    klass = 'HTML' + tagName + 'Element';
    if (window[klass]) return window[klass];
    klass = 'HTML' + tagName.capitalize() + 'Element';
    if (window[klass]) return window[klass];

    window[klass] = { };
    window[klass].prototype = document.createElement(tagName).__proto__;
    return window[klass];
  }

  if (F.ElementExtensions) {
    copy(Element.Methods, HTMLElement.prototype);
    copy(Element.Methods.Simulated, HTMLElement.prototype, true);
  }

  if (F.SpecificElementExtensions) {
    for (var tag in Element.Methods.ByTag) {
      var klass = findDOMClass(tag);
      if (Object.isUndefined(klass)) continue;
      copy(T[tag], klass.prototype);
    }
  }

  Object.extend(Element, Element.Methods);
  delete Element.ByTag;

  if (Element.extend.refresh) Element.extend.refresh();
  Element.cache = { };
};

document.viewport = {
  getDimensions: function() {
    var dimensions = { };
    $w('width height').each(function(d) {
      var D = d.capitalize();
      dimensions[d] = self['inner' + D] ||
       (document.documentElement['client' + D] || document.body['client' + D]);
    });
    return dimensions;
  },

  getWidth: function() {
    return this.getDimensions().width;
  },

  getHeight: function() {
    return this.getDimensions().height;
  },

  getScrollOffsets: function() {
    return Element._returnOffset(
      window.pageXOffset || document.documentElement.scrollLeft || document.body.scrollLeft,
      window.pageYOffset || document.documentElement.scrollTop || document.body.scrollTop);
  }
};
/* Portions of the Selector class are derived from Jack Slocumâ€™s DomQuery,
 * part of YUI-Ext version 0.40, distributed under the terms of an MIT-style
 * license.  Please see http://www.yui-ext.com/ for more information. */

var Selector = Class.create({
  initialize: function(expression) {
    this.expression = expression.strip();
    this.compileMatcher();
  },

  compileMatcher: function() {
    // Selectors with namespaced attributes can't use the XPath version
    if (Prototype.BrowserFeatures.XPath && !(/(\[[\w-]*?:|:checked)/).test(this.expression))
      return this.compileXPathMatcher();

    var e = this.expression, ps = Selector.patterns, h = Selector.handlers,
        c = Selector.criteria, le, p, m;

    if (Selector._cache[e]) {
      this.matcher = Selector._cache[e];
      return;
    }

    this.matcher = ["this.matcher = function(root) {",
                    "var r = root, h = Selector.handlers, c = false, n;"];

    while (e && le != e && (/\S/).test(e)) {
      le = e;
      for (var i in ps) {
        p = ps[i];
        if (m = e.match(p)) {
          this.matcher.push(Object.isFunction(c[i]) ? c[i](m) :
    	      new Template(c[i]).evaluate(m));
          e = e.replace(m[0], '');
          break;
        }
      }
    }

    this.matcher.push("return h.unique(n);\n}");
    eval(this.matcher.join('\n'));
    Selector._cache[this.expression] = this.matcher;
  },

  compileXPathMatcher: function() {
    var e = this.expression, ps = Selector.patterns,
        x = Selector.xpath, le, m;

    if (Selector._cache[e]) {
      this.xpath = Selector._cache[e]; return;
    }

    this.matcher = ['.//*'];
    while (e && le != e && (/\S/).test(e)) {
      le = e;
      for (var i in ps) {
        if (m = e.match(ps[i])) {
          this.matcher.push(Object.isFunction(x[i]) ? x[i](m) :
            new Template(x[i]).evaluate(m));
          e = e.replace(m[0], '');
          break;
        }
      }
    }

    this.xpath = this.matcher.join('');
    Selector._cache[this.expression] = this.xpath;
  },

  findElements: function(root) {
    root = root || document;
    if (this.xpath) return document._getElementsByXPath(this.xpath, root);
    return this.matcher(root);
  },

  match: function(element) {
    this.tokens = [];

    var e = this.expression, ps = Selector.patterns, as = Selector.assertions;
    var le, p, m;

    while (e && le !== e && (/\S/).test(e)) {
      le = e;
      for (var i in ps) {
        p = ps[i];
        if (m = e.match(p)) {
          // use the Selector.assertions methods unless the selector
          // is too complex.
          if (as[i]) {
            this.tokens.push([i, Object.clone(m)]);
            e = e.replace(m[0], '');
          } else {
            // reluctantly do a document-wide search
            // and look for a match in the array
            return this.findElements(document).include(element);
          }
        }
      }
    }

    var match = true, name, matches;
    for (var i = 0, token; token = this.tokens[i]; i++) {
      name = token[0], matches = token[1];
      if (!Selector.assertions[name](element, matches)) {
        match = false; break;
      }
    }

    return match;
  },

  toString: function() {
    return this.expression;
  },

  inspect: function() {
    return "#<Selector:" + this.expression.inspect() + ">";
  }
});

Object.extend(Selector, {
  _cache: { },

  xpath: {
    descendant:   "//*",
    child:        "/*",
    adjacent:     "/following-sibling::*[1]",
    laterSibling: '/following-sibling::*',
    tagName:      function(m) {
      if (m[1] == '*') return '';
      return "[local-name()='" + m[1].toLowerCase() +
             "' or local-name()='" + m[1].toUpperCase() + "']";
    },
    className:    "[contains(concat(' ', @class, ' '), ' #{1} ')]",
    id:           "[@id='#{1}']",
    attrPresence: "[@#{1}]",
    attr: function(m) {
      m[3] = m[5] || m[6];
      return new Template(Selector.xpath.operators[m[2]]).evaluate(m);
    },
    pseudo: function(m) {
      var h = Selector.xpath.pseudos[m[1]];
      if (!h) return '';
      if (Object.isFunction(h)) return h(m);
      return new Template(Selector.xpath.pseudos[m[1]]).evaluate(m);
    },
    operators: {
      '=':  "[@#{1}='#{3}']",
      '!=': "[@#{1}!='#{3}']",
      '^=': "[starts-with(@#{1}, '#{3}')]",
      '$=': "[substring(@#{1}, (string-length(@#{1}) - string-length('#{3}') + 1))='#{3}']",
      '*=': "[contains(@#{1}, '#{3}')]",
      '~=': "[contains(concat(' ', @#{1}, ' '), ' #{3} ')]",
      '|=': "[contains(concat('-', @#{1}, '-'), '-#{3}-')]"
    },
    pseudos: {
      'first-child': '[not(preceding-sibling::*)]',
      'last-child':  '[not(following-sibling::*)]',
      'only-child':  '[not(preceding-sibling::* or following-sibling::*)]',
      'empty':       "[count(*) = 0 and (count(text()) = 0 or translate(text(), ' \t\r\n', '') = '')]",
      'checked':     "[@checked]",
      'disabled':    "[@disabled]",
      'enabled':     "[not(@disabled)]",
      'not': function(m) {
        var e = m[6], p = Selector.patterns,
            x = Selector.xpath, le, m, v;

        var exclusion = [];
        while (e && le != e && (/\S/).test(e)) {
          le = e;
          for (var i in p) {
            if (m = e.match(p[i])) {
              v = Object.isFunction(x[i]) ? x[i](m) : new Template(x[i]).evaluate(m);
              exclusion.push("(" + v.substring(1, v.length - 1) + ")");
              e = e.replace(m[0], '');
              break;
            }
          }
        }
        return "[not(" + exclusion.join(" and ") + ")]";
      },
      'nth-child':      function(m) {
        return Selector.xpath.pseudos.nth("(count(./preceding-sibling::*) + 1) ", m);
      },
      'nth-last-child': function(m) {
        return Selector.xpath.pseudos.nth("(count(./following-sibling::*) + 1) ", m);
      },
      'nth-of-type':    function(m) {
        return Selector.xpath.pseudos.nth("position() ", m);
      },
      'nth-last-of-type': function(m) {
        return Selector.xpath.pseudos.nth("(last() + 1 - position()) ", m);
      },
      'first-of-type':  function(m) {
        m[6] = "1"; return Selector.xpath.pseudos['nth-of-type'](m);
      },
      'last-of-type':   function(m) {
        m[6] = "1"; return Selector.xpath.pseudos['nth-last-of-type'](m);
      },
      'only-of-type':   function(m) {
        var p = Selector.xpath.pseudos; return p['first-of-type'](m) + p['last-of-type'](m);
      },
      nth: function(fragment, m) {
        var mm, formula = m[6], predicate;
        if (formula == 'even') formula = '2n+0';
        if (formula == 'odd')  formula = '2n+1';
        if (mm = formula.match(/^(\d+)$/)) // digit only
          return '[' + fragment + "= " + mm[1] + ']';
        if (mm = formula.match(/^(-?\d*)?n(([+-])(\d+))?/)) { // an+b
          if (mm[1] == "-") mm[1] = -1;
          var a = mm[1] ? Number(mm[1]) : 1;
          var b = mm[2] ? Number(mm[2]) : 0;
          predicate = "[((#{fragment} - #{b}) mod #{a} = 0) and " +
          "((#{fragment} - #{b}) div #{a} >= 0)]";
          return new Template(predicate).evaluate({
            fragment: fragment, a: a, b: b });
        }
      }
    }
  },

  criteria: {
    tagName:      'n = h.tagName(n, r, "#{1}", c);   c = false;',
    className:    'n = h.className(n, r, "#{1}", c); c = false;',
    id:           'n = h.id(n, r, "#{1}", c);        c = false;',
    attrPresence: 'n = h.attrPresence(n, r, "#{1}"); c = false;',
    attr: function(m) {
      m[3] = (m[5] || m[6]);
      return new Template('n = h.attr(n, r, "#{1}", "#{3}", "#{2}"); c = false;').evaluate(m);
    },
    pseudo: function(m) {
      if (m[6]) m[6] = m[6].replace(/"/g, '\\"');
      return new Template('n = h.pseudo(n, "#{1}", "#{6}", r, c); c = false;').evaluate(m);
    },
    descendant:   'c = "descendant";',
    child:        'c = "child";',
    adjacent:     'c = "adjacent";',
    laterSibling: 'c = "laterSibling";'
  },

  patterns: {
    // combinators must be listed first
    // (and descendant needs to be last combinator)
    laterSibling: /^\s*~\s*/,
    child:        /^\s*>\s*/,
    adjacent:     /^\s*\+\s*/,
    descendant:   /^\s/,

    // selectors follow
    tagName:      /^\s*(\*|[\w\-]+)(\b|$)?/,
    id:           /^#([\w\-\*]+)(\b|$)/,
    className:    /^\.([\w\-\*]+)(\b|$)/,
    pseudo:       /^:((first|last|nth|nth-last|only)(-child|-of-type)|empty|checked|(en|dis)abled|not)(\((.*?)\))?(\b|$|(?=\s)|(?=:))/,
    attrPresence: /^\[([\w]+)\]/,
    attr:         /\[((?:[\w-]*:)?[\w-]+)\s*(?:([!^$*~|]?=)\s*((['"])([^\4]*?)\4|([^'"][^\]]*?)))?\]/
  },

  // for Selector.match and Element#match
  assertions: {
    tagName: function(element, matches) {
      return matches[1].toUpperCase() == element.tagName.toUpperCase();
    },

    className: function(element, matches) {
      return Element.hasClassName(element, matches[1]);
    },

    id: function(element, matches) {
      return element.id === matches[1];
    },

    attrPresence: function(element, matches) {
      return Element.hasAttribute(element, matches[1]);
    },

    attr: function(element, matches) {
      var nodeValue = Element.readAttribute(element, matches[1]);
      return Selector.operators[matches[2]](nodeValue, matches[3]);
    }
  },

  handlers: {
    // UTILITY FUNCTIONS
    // joins two collections
    concat: function(a, b) {
      for (var i = 0, node; node = b[i]; i++)
        a.push(node);
      return a;
    },

    // marks an array of nodes for counting
    mark: function(nodes) {
      for (var i = 0, node; node = nodes[i]; i++)
        node._counted = true;
      return nodes;
    },

    unmark: function(nodes) {
      for (var i = 0, node; node = nodes[i]; i++)
        node._counted = undefined;
      return nodes;
    },

    // mark each child node with its position (for nth calls)
    // "ofType" flag indicates whether we're indexing for nth-of-type
    // rather than nth-child
    index: function(parentNode, reverse, ofType) {
      parentNode._counted = true;
      if (reverse) {
        for (var nodes = parentNode.childNodes, i = nodes.length - 1, j = 1; i >= 0; i--) {
          var node = nodes[i];
          if (node.nodeType == 1 && (!ofType || node._counted)) node.nodeIndex = j++;
        }
      } else {
        for (var i = 0, j = 1, nodes = parentNode.childNodes; node = nodes[i]; i++)
          if (node.nodeType == 1 && (!ofType || node._counted)) node.nodeIndex = j++;
      }
    },

    // filters out duplicates and extends all nodes
    unique: function(nodes) {
      if (nodes.length == 0) return nodes;
      var results = [], n;
      for (var i = 0, l = nodes.length; i < l; i++)
        if (!(n = nodes[i])._counted) {
          n._counted = true;
          results.push(Element.extend(n));
        }
      return Selector.handlers.unmark(results);
    },

    // COMBINATOR FUNCTIONS
    descendant: function(nodes) {
      var h = Selector.handlers;
      for (var i = 0, results = [], node; node = nodes[i]; i++)
        h.concat(results, node.getElementsByTagName('*'));
      return results;
    },

    child: function(nodes) {
      var h = Selector.handlers;
      for (var i = 0, results = [], node; node = nodes[i]; i++) {
        for (var j = 0, children = [], child; child = node.childNodes[j]; j++)
          if (child.nodeType == 1 && child.tagName != '!') results.push(child);
      }
      return results;
    },

    adjacent: function(nodes) {
      for (var i = 0, results = [], node; node = nodes[i]; i++) {
        var next = this.nextElementSibling(node);
        if (next) results.push(next);
      }
      return results;
    },

    laterSibling: function(nodes) {
      var h = Selector.handlers;
      for (var i = 0, results = [], node; node = nodes[i]; i++)
        h.concat(results, Element.nextSiblings(node));
      return results;
    },

    nextElementSibling: function(node) {
      while (node = node.nextSibling)
	      if (node.nodeType == 1) return node;
      return null;
    },

    previousElementSibling: function(node) {
      while (node = node.previousSibling)
        if (node.nodeType == 1) return node;
      return null;
    },

    // TOKEN FUNCTIONS
    tagName: function(nodes, root, tagName, combinator) {
      tagName = tagName.toUpperCase();
      var results = [], h = Selector.handlers;
      if (nodes) {
        if (combinator) {
          // fastlane for ordinary descendant combinators
          if (combinator == "descendant") {
            for (var i = 0, node; node = nodes[i]; i++)
              h.concat(results, node.getElementsByTagName(tagName));
            return results;
          } else nodes = this[combinator](nodes);
          if (tagName == "*") return nodes;
        }
        for (var i = 0, node; node = nodes[i]; i++)
          if (node.tagName.toUpperCase() == tagName) results.push(node);
        return results;
      } else return root.getElementsByTagName(tagName);
    },

    id: function(nodes, root, id, combinator) {
      var targetNode = $(id), h = Selector.handlers;
      if (!targetNode) return [];
      if (!nodes && root == document) return [targetNode];
      if (nodes) {
        if (combinator) {
          if (combinator == 'child') {
            for (var i = 0, node; node = nodes[i]; i++)
              if (targetNode.parentNode == node) return [targetNode];
          } else if (combinator == 'descendant') {
            for (var i = 0, node; node = nodes[i]; i++)
              if (Element.descendantOf(targetNode, node)) return [targetNode];
          } else if (combinator == 'adjacent') {
            for (var i = 0, node; node = nodes[i]; i++)
              if (Selector.handlers.previousElementSibling(targetNode) == node)
                return [targetNode];
          } else nodes = h[combinator](nodes);
        }
        for (var i = 0, node; node = nodes[i]; i++)
          if (node == targetNode) return [targetNode];
        return [];
      }
      return (targetNode && Element.descendantOf(targetNode, root)) ? [targetNode] : [];
    },

    className: function(nodes, root, className, combinator) {
      if (nodes && combinator) nodes = this[combinator](nodes);
      return Selector.handlers.byClassName(nodes, root, className);
    },

    byClassName: function(nodes, root, className) {
      if (!nodes) nodes = Selector.handlers.descendant([root]);
      var needle = ' ' + className + ' ';
      for (var i = 0, results = [], node, nodeClassName; node = nodes[i]; i++) {
        nodeClassName = node.className;
        if (nodeClassName.length == 0) continue;
        if (nodeClassName == className || (' ' + nodeClassName + ' ').include(needle))
          results.push(node);
      }
      return results;
    },

    attrPresence: function(nodes, root, attr) {
      if (!nodes) nodes = root.getElementsByTagName("*");
      var results = [];
      for (var i = 0, node; node = nodes[i]; i++)
        if (Element.hasAttribute(node, attr)) results.push(node);
      return results;
    },

    attr: function(nodes, root, attr, value, operator) {
      if (!nodes) nodes = root.getElementsByTagName("*");
      var handler = Selector.operators[operator], results = [];
      for (var i = 0, node; node = nodes[i]; i++) {
        var nodeValue = Element.readAttribute(node, attr);
        if (nodeValue === null) continue;
        if (handler(nodeValue, value)) results.push(node);
      }
      return results;
    },

    pseudo: function(nodes, name, value, root, combinator) {
      if (nodes && combinator) nodes = this[combinator](nodes);
      if (!nodes) nodes = root.getElementsByTagName("*");
      return Selector.pseudos[name](nodes, value, root);
    }
  },

  pseudos: {
    'first-child': function(nodes, value, root) {
      for (var i = 0, results = [], node; node = nodes[i]; i++) {
        if (Selector.handlers.previousElementSibling(node)) continue;
          results.push(node);
      }
      return results;
    },
    'last-child': function(nodes, value, root) {
      for (var i = 0, results = [], node; node = nodes[i]; i++) {
        if (Selector.handlers.nextElementSibling(node)) continue;
          results.push(node);
      }
      return results;
    },
    'only-child': function(nodes, value, root) {
      var h = Selector.handlers;
      for (var i = 0, results = [], node; node = nodes[i]; i++)
        if (!h.previousElementSibling(node) && !h.nextElementSibling(node))
          results.push(node);
      return results;
    },
    'nth-child':        function(nodes, formula, root) {
      return Selector.pseudos.nth(nodes, formula, root);
    },
    'nth-last-child':   function(nodes, formula, root) {
      return Selector.pseudos.nth(nodes, formula, root, true);
    },
    'nth-of-type':      function(nodes, formula, root) {
      return Selector.pseudos.nth(nodes, formula, root, false, true);
    },
    'nth-last-of-type': function(nodes, formula, root) {
      return Selector.pseudos.nth(nodes, formula, root, true, true);
    },
    'first-of-type':    function(nodes, formula, root) {
      return Selector.pseudos.nth(nodes, "1", root, false, true);
    },
    'last-of-type':     function(nodes, formula, root) {
      return Selector.pseudos.nth(nodes, "1", root, true, true);
    },
    'only-of-type':     function(nodes, formula, root) {
      var p = Selector.pseudos;
      return p['last-of-type'](p['first-of-type'](nodes, formula, root), formula, root);
    },

    // handles the an+b logic
    getIndices: function(a, b, total) {
      if (a == 0) return b > 0 ? [b] : [];
      return $R(1, total).inject([], function(memo, i) {
        if (0 == (i - b) % a && (i - b) / a >= 0) memo.push(i);
        return memo;
      });
    },

    // handles nth(-last)-child, nth(-last)-of-type, and (first|last)-of-type
    nth: function(nodes, formula, root, reverse, ofType) {
      if (nodes.length == 0) return [];
      if (formula == 'even') formula = '2n+0';
      if (formula == 'odd')  formula = '2n+1';
      var h = Selector.handlers, results = [], indexed = [], m;
      h.mark(nodes);
      for (var i = 0, node; node = nodes[i]; i++) {
        if (!node.parentNode._counted) {
          h.index(node.parentNode, reverse, ofType);
          indexed.push(node.parentNode);
        }
      }
      if (formula.match(/^\d+$/)) { // just a number
        formula = Number(formula);
        for (var i = 0, node; node = nodes[i]; i++)
          if (node.nodeIndex == formula) results.push(node);
      } else if (m = formula.match(/^(-?\d*)?n(([+-])(\d+))?/)) { // an+b
        if (m[1] == "-") m[1] = -1;
        var a = m[1] ? Number(m[1]) : 1;
        var b = m[2] ? Number(m[2]) : 0;
        var indices = Selector.pseudos.getIndices(a, b, nodes.length);
        for (var i = 0, node, l = indices.length; node = nodes[i]; i++) {
          for (var j = 0; j < l; j++)
            if (node.nodeIndex == indices[j]) results.push(node);
        }
      }
      h.unmark(nodes);
      h.unmark(indexed);
      return results;
    },

    'empty': function(nodes, value, root) {
      for (var i = 0, results = [], node; node = nodes[i]; i++) {
        // IE treats comments as element nodes
        if (node.tagName == '!' || (node.firstChild && !node.innerHTML.match(/^\s*$/))) continue;
        results.push(node);
      }
      return results;
    },

    'not': function(nodes, selector, root) {
      var h = Selector.handlers, selectorType, m;
      var exclusions = new Selector(selector).findElements(root);
      h.mark(exclusions);
      for (var i = 0, results = [], node; node = nodes[i]; i++)
        if (!node._counted) results.push(node);
      h.unmark(exclusions);
      return results;
    },

    'enabled': function(nodes, value, root) {
      for (var i = 0, results = [], node; node = nodes[i]; i++)
        if (!node.disabled) results.push(node);
      return results;
    },

    'disabled': function(nodes, value, root) {
      for (var i = 0, results = [], node; node = nodes[i]; i++)
        if (node.disabled) results.push(node);
      return results;
    },

    'checked': function(nodes, value, root) {
      for (var i = 0, results = [], node; node = nodes[i]; i++)
        if (node.checked) results.push(node);
      return results;
    }
  },

  operators: {
    '=':  function(nv, v) { return nv == v; },
    '!=': function(nv, v) { return nv != v; },
    '^=': function(nv, v) { return nv.startsWith(v); },
    '$=': function(nv, v) { return nv.endsWith(v); },
    '*=': function(nv, v) { return nv.include(v); },
    '~=': function(nv, v) { return (' ' + nv + ' ').include(' ' + v + ' '); },
    '|=': function(nv, v) { return ('-' + nv.toUpperCase() + '-').include('-' + v.toUpperCase() + '-'); }
  },

  matchElements: function(elements, expression) {
    var matches = new Selector(expression).findElements(), h = Selector.handlers;
    h.mark(matches);
    for (var i = 0, results = [], element; element = elements[i]; i++)
      if (element._counted) results.push(element);
    h.unmark(matches);
    return results;
  },

  findElement: function(elements, expression, index) {
    if (Object.isNumber(expression)) {
      index = expression; expression = false;
    }
    return Selector.matchElements(elements, expression || '*')[index || 0];
  },

  findChildElements: function(element, expressions) {
    var exprs = expressions.join(','), expressions = [];
    exprs.scan(/(([\w#:.~>+()\s-]+|\*|\[.*?\])+)\s*(,|$)/, function(m) {
      expressions.push(m[1].strip());
    });
    var results = [], h = Selector.handlers;
    for (var i = 0, l = expressions.length, selector; i < l; i++) {
      selector = new Selector(expressions[i].strip());
      h.concat(results, selector.findElements(element));
    }
    return (l > 1) ? h.unique(results) : results;
  }
});

function $$() {
  return Selector.findChildElements(document, $A(arguments));
}
var Form = {
  reset: function(form) {
    $(form).reset();
    return form;
  },

  serializeElements: function(elements, options) {
    if (typeof options != 'object') options = { hash: !!options };
    else if (options.hash === undefined) options.hash = true;
    var key, value, submitted = false, submit = options.submit;

    var data = elements.inject({ }, function(result, element) {
      if (!element.disabled && element.name) {
        key = element.name; value = $(element).getValue();
        if (value != null && (element.type != 'submit' || (!submitted &&
            submit !== false && (!submit || key == submit) && (submitted = true)))) {
          if (key in result) {
            // a key is already present; construct an array of values
            if (!Object.isArray(result[key])) result[key] = [result[key]];
            result[key].push(value);
          }
          else result[key] = value;
        }
      }
      return result;
    });

    return options.hash ? data : Object.toQueryString(data);
  }
};

Form.Methods = {
  serialize: function(form, options) {
    return Form.serializeElements(Form.getElements(form), options);
  },

  getElements: function(form) {
    return $A($(form).getElementsByTagName('*')).inject([],
      function(elements, child) {
        if (Form.Element.Serializers[child.tagName.toLowerCase()])
          elements.push(Element.extend(child));
        return elements;
      }
    );
  },

  getInputs: function(form, typeName, name) {
    form = $(form);
    var inputs = form.getElementsByTagName('input');

    if (!typeName && !name) return $A(inputs).map(Element.extend);

    for (var i = 0, matchingInputs = [], length = inputs.length; i < length; i++) {
      var input = inputs[i];
      if ((typeName && input.type != typeName) || (name && input.name != name))
        continue;
      matchingInputs.push(Element.extend(input));
    }

    return matchingInputs;
  },

  disable: function(form) {
    form = $(form);
    Form.getElements(form).invoke('disable');
    return form;
  },

  enable: function(form) {
    form = $(form);
    Form.getElements(form).invoke('enable');
    return form;
  },

  findFirstElement: function(form) {
    var elements = $(form).getElements().findAll(function(element) {
      return 'hidden' != element.type && !element.disabled;
    });
    var firstByIndex = elements.findAll(function(element) {
      return element.hasAttribute('tabIndex') && element.tabIndex >= 0;
    }).sortBy(function(element) { return element.tabIndex }).first();

    return firstByIndex ? firstByIndex : elements.find(function(element) {
      return ['input', 'select', 'textarea'].include(element.tagName.toLowerCase());
    });
  },

  focusFirstElement: function(form) {
    form = $(form);
    form.findFirstElement().activate();
    return form;
  },

  request: function(form, options) {
    form = $(form), options = Object.clone(options || { });

    var params = options.parameters, action = form.readAttribute('action') || '';
    if (action.blank()) action = window.location.href;
    options.parameters = form.serialize(true);

    if (params) {
      if (Object.isString(params)) params = params.toQueryParams();
      Object.extend(options.parameters, params);
    }

    if (form.hasAttribute('method') && !options.method)
      options.method = form.method;

    return new Ajax.Request(action, options);
  }
};

/*--------------------------------------------------------------------------*/

Form.Element = {
  focus: function(element) {
    $(element).focus();
    return element;
  },

  select: function(element) {
    $(element).select();
    return element;
  }
};

Form.Element.Methods = {
  serialize: function(element) {
    element = $(element);
    if (!element.disabled && element.name) {
      var value = element.getValue();
      if (value != undefined) {
        var pair = { };
        pair[element.name] = value;
        return Object.toQueryString(pair);
      }
    }
    return '';
  },

  getValue: function(element) {
    element = $(element);
    var method = element.tagName.toLowerCase();
    return Form.Element.Serializers[method](element);
  },

  setValue: function(element, value) {
    element = $(element);
    var method = element.tagName.toLowerCase();
    Form.Element.Serializers[method](element, value);
    return element;
  },

  clear: function(element) {
    $(element).value = '';
    return element;
  },

  present: function(element) {
    return $(element).value != '';
  },

  activate: function(element) {
    element = $(element);
    try {
      element.focus();
      if (element.select && (element.tagName.toLowerCase() != 'input' ||
          !['button', 'reset', 'submit'].include(element.type)))
        element.select();
    } catch (e) { }
    return element;
  },

  disable: function(element) {
    element = $(element);
    element.blur();
    element.disabled = true;
    return element;
  },

  enable: function(element) {
    element = $(element);
    element.disabled = false;
    return element;
  }
};

/*--------------------------------------------------------------------------*/

var Field = Form.Element;
var $F = Form.Element.Methods.getValue;

/*--------------------------------------------------------------------------*/

Form.Element.Serializers = {
  input: function(element, value) {
    switch (element.type.toLowerCase()) {
      case 'checkbox':
      case 'radio':
        return Form.Element.Serializers.inputSelector(element, value);
      default:
        return Form.Element.Serializers.textarea(element, value);
    }
  },

  inputSelector: function(element, value) {
    if (value === undefined) return element.checked ? element.value : null;
    else element.checked = !!value;
  },

  textarea: function(element, value) {
    if (value === undefined) return element.value;
    else element.value = value;
  },

  select: function(element, index) {
    if (index === undefined)
      return this[element.type == 'select-one' ?
        'selectOne' : 'selectMany'](element);
    else {
      var opt, value, single = !Object.isArray(index);
      for (var i = 0, length = element.length; i < length; i++) {
        opt = element.options[i];
        value = this.optionValue(opt);
        if (single) {
          if (value == index) {
            opt.selected = true;
            return;
          }
        }
        else opt.selected = index.include(value);
      }
    }
  },

  selectOne: function(element) {
    var index = element.selectedIndex;
    return index >= 0 ? this.optionValue(element.options[index]) : null;
  },

  selectMany: function(element) {
    var values, length = element.length;
    if (!length) return null;

    for (var i = 0, values = []; i < length; i++) {
      var opt = element.options[i];
      if (opt.selected) values.push(this.optionValue(opt));
    }
    return values;
  },

  optionValue: function(opt) {
    // extend element because hasAttribute may not be native
    return Element.extend(opt).hasAttribute('value') ? opt.value : opt.text;
  }
};

/*--------------------------------------------------------------------------*/

Abstract.TimedObserver = Class.create(PeriodicalExecuter, {
  initialize: function($super, element, frequency, callback) {
    $super(callback, frequency);
    this.element   = $(element);
    this.lastValue = this.getValue();
  },

  execute: function() {
    var value = this.getValue();
    if (Object.isString(this.lastValue) && Object.isString(value) ?
        this.lastValue != value : String(this.lastValue) != String(value)) {
      this.callback(this.element, value);
      this.lastValue = value;
    }
  }
});

Form.Element.Observer = Class.create(Abstract.TimedObserver, {
  getValue: function() {
    return Form.Element.getValue(this.element);
  }
});

Form.Observer = Class.create(Abstract.TimedObserver, {
  getValue: function() {
    return Form.serialize(this.element);
  }
});

/*--------------------------------------------------------------------------*/

Abstract.EventObserver = Class.create({
  initialize: function(element, callback) {
    this.element  = $(element);
    this.callback = callback;

    this.lastValue = this.getValue();
    if (this.element.tagName.toLowerCase() == 'form')
      this.registerFormCallbacks();
    else
      this.registerCallback(this.element);
  },

  onElementEvent: function() {
    var value = this.getValue();
    if (this.lastValue != value) {
      this.callback(this.element, value);
      this.lastValue = value;
    }
  },

  registerFormCallbacks: function() {
    Form.getElements(this.element).each(this.registerCallback, this);
  },

  registerCallback: function(element) {
    if (element.type) {
      switch (element.type.toLowerCase()) {
        case 'checkbox':
        case 'radio':
          Event.observe(element, 'click', this.onElementEvent.bind(this));
          break;
        default:
          Event.observe(element, 'change', this.onElementEvent.bind(this));
          break;
      }
    }
  }
});

Form.Element.EventObserver = Class.create(Abstract.EventObserver, {
  getValue: function() {
    return Form.Element.getValue(this.element);
  }
});

Form.EventObserver = Class.create(Abstract.EventObserver, {
  getValue: function() {
    return Form.serialize(this.element);
  }
});
if (!window.Event) var Event = { };

Object.extend(Event, {
  KEY_BACKSPACE: 8,
  KEY_TAB:       9,
  KEY_RETURN:   13,
  KEY_ESC:      27,
  KEY_LEFT:     37,
  KEY_UP:       38,
  KEY_RIGHT:    39,
  KEY_DOWN:     40,
  KEY_DELETE:   46,
  KEY_HOME:     36,
  KEY_END:      35,
  KEY_PAGEUP:   33,
  KEY_PAGEDOWN: 34,
  KEY_INSERT:   45,

  cache: { },

  relatedTarget: function(event) {
    var element;
    switch(event.type) {
      case 'mouseover': element = event.fromElement; break;
      case 'mouseout':  element = event.toElement;   break;
      default: return null;
    }
    return Element.extend(element);
  }
});

Event.Methods = (function() {
  var isButton;

  if (Prototype.Browser.IE) {
    var buttonMap = { 0: 1, 1: 4, 2: 2 };
    isButton = function(event, code) {
      return event.button == buttonMap[code];
    };

  } else if (Prototype.Browser.WebKit) {
    isButton = function(event, code) {
      switch (code) {
        case 0: return event.which == 1 && !event.metaKey;
        case 1: return event.which == 1 && event.metaKey;
        default: return false;
      }
    };

  } else {
    isButton = function(event, code) {
      return event.which ? (event.which === code + 1) : (event.button === code);
    };
  }

  return {
    isLeftClick:   function(event) { return isButton(event, 0) },
    isMiddleClick: function(event) { return isButton(event, 1) },
    isRightClick:  function(event) { return isButton(event, 2) },

    element: function(event) {
      var node = Event.extend(event).target;
      return Element.extend(node.nodeType == Node.TEXT_NODE ? node.parentNode : node);
    },

    findElement: function(event, expression) {
      var element = Event.element(event);
      return element.match(expression) ? element : element.up(expression);
    },

    pointer: function(event) {
      return {
        x: event.pageX || (event.clientX +
          (document.documentElement.scrollLeft || document.body.scrollLeft)),
        y: event.pageY || (event.clientY +
          (document.documentElement.scrollTop || document.body.scrollTop))
      };
    },

    pointerX: function(event) { return Event.pointer(event).x },
    pointerY: function(event) { return Event.pointer(event).y },

    stop: function(event) {
      Event.extend(event);
      event.preventDefault();
      event.stopPropagation();
      event.stopped = true;
    }
  };
})();

Event.extend = (function() {
  var methods = Object.keys(Event.Methods).inject({ }, function(m, name) {
    m[name] = Event.Methods[name].methodize();
    return m;
  });

  if (Prototype.Browser.IE) {
    Object.extend(methods, {
      stopPropagation: function() { this.cancelBubble = true },
      preventDefault:  function() { this.returnValue = false },
      inspect: function() { return "[object Event]" }
    });

    return function(event) {
      if (!event) return false;
      if (event._extendedByPrototype) return event;

      event._extendedByPrototype = Prototype.emptyFunction;
      var pointer = Event.pointer(event);
      Object.extend(event, {
        target: event.srcElement,
        relatedTarget: Event.relatedTarget(event),
        pageX:  pointer.x,
        pageY:  pointer.y
      });
      return Object.extend(event, methods);
    };

  } else {
    Event.prototype = Event.prototype || document.createEvent("HTMLEvents").__proto__;
    Object.extend(Event.prototype, methods);
    return Prototype.K;
  }
})();

Object.extend(Event, (function() {
  var cache = Event.cache;

  function getEventID(element) {
    if (element._eventID) return element._eventID;
    arguments.callee.id = arguments.callee.id || 1;
    return element._eventID = ++arguments.callee.id;
  }

  function getDOMEventName(eventName) {
    if (eventName && eventName.include(':')) return "dataavailable";
    return eventName;
  }

  function getCacheForID(id) {
    return cache[id] = cache[id] || { };
  }

  function getWrappersForEventName(id, eventName) {
    var c = getCacheForID(id);
    return c[eventName] = c[eventName] || [];
  }

  function createWrapper(element, eventName, handler) {
    var id = getEventID(element);
    var c = getWrappersForEventName(id, eventName);
    if (c.pluck("handler").include(handler)) return false;

    var wrapper = function(event) {
      if (!Event || !Event.extend ||
        (event.eventName && event.eventName != eventName))
          return false;

      Event.extend(event);
      handler.call(element, event)
    };

    wrapper.handler = handler;
    c.push(wrapper);
    return wrapper;
  }

  function findWrapper(id, eventName, handler) {
    var c = getWrappersForEventName(id, eventName);
    return c.find(function(wrapper) { return wrapper.handler == handler });
  }

  function destroyWrapper(id, eventName, handler) {
    var c = getCacheForID(id);
    if (!c[eventName]) return false;
    c[eventName] = c[eventName].without(findWrapper(id, eventName, handler));
  }

  function destroyCache() {
    for (var id in cache)
      for (var eventName in cache[id])
        cache[id][eventName] = null;
  }

  if (window.attachEvent) {
    window.attachEvent("onunload", destroyCache);
  }

  return {
    observe: function(element, eventName, handler) {
      element = $(element);
      var name = getDOMEventName(eventName);

      var wrapper = createWrapper(element, eventName, handler);
      if (!wrapper) return element;

      if (element.addEventListener) {
        element.addEventListener(name, wrapper, false);
      } else {
        element.attachEvent("on" + name, wrapper);
      }

      return element;
    },

    stopObserving: function(element, eventName, handler) {
      element = $(element);
      var id = getEventID(element), name = getDOMEventName(eventName);

      if (!handler && eventName) {
        getWrappersForEventName(id, eventName).each(function(wrapper) {
          element.stopObserving(eventName, wrapper.handler);
        });
        return element;

      } else if (!eventName) {
        Object.keys(getCacheForID(id)).each(function(eventName) {
          element.stopObserving(eventName);
        });
        return element;
      }

      var wrapper = findWrapper(id, eventName, handler);
      if (!wrapper) return element;

      if (element.removeEventListener) {
        element.removeEventListener(name, wrapper, false);
      } else {
        element.detachEvent("on" + name, wrapper);
      }

      destroyWrapper(id, eventName, handler);

      return element;
    },

    fire: function(element, eventName, memo) {
      element = $(element);
      if (element == document && document.createEvent && !element.dispatchEvent)
        element = document.documentElement;

      if (document.createEvent) {
        var event = document.createEvent("HTMLEvents");
        event.initEvent("dataavailable", true, true);
      } else {
        var event = document.createEventObject();
        event.eventType = "ondataavailable";
      }

      event.eventName = eventName;
      event.memo = memo || { };

      if (document.createEvent) {
        element.dispatchEvent(event);
      } else {
        element.fireEvent(event.eventType, event);
      }

      return event;
    }
  };
})());

Object.extend(Event, Event.Methods);

Element.addMethods({
  fire:          Event.fire,
  observe:       Event.observe,
  stopObserving: Event.stopObserving
});

Object.extend(document, {
  fire:          Element.Methods.fire.methodize(),
  observe:       Element.Methods.observe.methodize(),
  stopObserving: Element.Methods.stopObserving.methodize()
});

(function() {
  /* Support for the DOMContentLoaded event is based on work by Dan Webb,
     Matthias Miller, Dean Edwards and John Resig. */

  var timer, fired = false;

  function fireContentLoadedEvent() {
    if (fired) return;
    if (timer) window.clearInterval(timer);
    document.fire("dom:loaded");
    fired = true;
  }

  if (document.addEventListener) {
    if (Prototype.Browser.WebKit) {
      timer = window.setInterval(function() {
        if (/loaded|complete/.test(document.readyState))
          fireContentLoadedEvent();
      }, 0);

      Event.observe(window, "load", fireContentLoadedEvent);

    } else {
      document.addEventListener("DOMContentLoaded",
        fireContentLoadedEvent, false);
    }

  } else {
    document.write("<script id=__onDOMContentLoaded defer src=//:><\/script>");
    $("__onDOMContentLoaded").onreadystatechange = function() {
      if (this.readyState == "complete") {
        this.onreadystatechange = null;
        fireContentLoadedEvent();
      }
    };
  }
})();
/*------------------------------- DEPRECATED -------------------------------*/

Hash.toQueryString = Object.toQueryString;

var Toggle = { display: Element.toggle };

Element.Methods.childOf = Element.Methods.descendantOf;

var Insertion = {
  Before: function(element, content) {
    return Element.insert(element, {before:content});
  },

  Top: function(element, content) {
    return Element.insert(element, {top:content});
  },

  Bottom: function(element, content) {
    return Element.insert(element, {bottom:content});
  },

  After: function(element, content) {
    return Element.insert(element, {after:content});
  }
};

var $continue = new Error('"throw $continue" is deprecated, use "return" instead');

// This should be moved to script.aculo.us; notice the deprecated methods
// further below, that map to the newer Element methods.
var Position = {
  // set to true if needed, warning: firefox performance problems
  // NOT neeeded for page scrolling, only if draggable contained in
  // scrollable elements
  includeScrollOffsets: false,

  // must be called before calling withinIncludingScrolloffset, every time the
  // page is scrolled
  prepare: function() {
    this.deltaX =  window.pageXOffset
                || document.documentElement.scrollLeft
                || document.body.scrollLeft
                || 0;
    this.deltaY =  window.pageYOffset
                || document.documentElement.scrollTop
                || document.body.scrollTop
                || 0;
  },

  // caches x/y coordinate pair to use with overlap
  within: function(element, x, y) {
    if (this.includeScrollOffsets)
      return this.withinIncludingScrolloffsets(element, x, y);
    this.xcomp = x;
    this.ycomp = y;
    this.offset = Element.cumulativeOffset(element);

    return (y >= this.offset[1] &&
            y <  this.offset[1] + element.offsetHeight &&
            x >= this.offset[0] &&
            x <  this.offset[0] + element.offsetWidth);
  },

  withinIncludingScrolloffsets: function(element, x, y) {
    var offsetcache = Element.cumulativeScrollOffset(element);

    this.xcomp = x + offsetcache[0] - this.deltaX;
    this.ycomp = y + offsetcache[1] - this.deltaY;
    this.offset = Element.cumulativeOffset(element);

    return (this.ycomp >= this.offset[1] &&
            this.ycomp <  this.offset[1] + element.offsetHeight &&
            this.xcomp >= this.offset[0] &&
            this.xcomp <  this.offset[0] + element.offsetWidth);
  },

  // within must be called directly before
  overlap: function(mode, element) {
    if (!mode) return 0;
    if (mode == 'vertical')
      return ((this.offset[1] + element.offsetHeight) - this.ycomp) /
        element.offsetHeight;
    if (mode == 'horizontal')
      return ((this.offset[0] + element.offsetWidth) - this.xcomp) /
        element.offsetWidth;
  },

  // Deprecation layer -- use newer Element methods now (1.5.2).

  cumulativeOffset: Element.Methods.cumulativeOffset,

  positionedOffset: Element.Methods.positionedOffset,

  absolutize: function(element) {
    Position.prepare();
    return Element.absolutize(element);
  },

  relativize: function(element) {
    Position.prepare();
    return Element.relativize(element);
  },

  realOffset: Element.Methods.cumulativeScrollOffset,

  offsetParent: Element.Methods.getOffsetParent,

  page: Element.Methods.viewportOffset,

  clone: function(source, target, options) {
    options = options || { };
    return Element.clonePosition(target, source, options);
  }
};

/*--------------------------------------------------------------------------*/

if (!document.getElementsByClassName) document.getElementsByClassName = function(instanceMethods){
  function iter(name) {
    return name.blank() ? null : "[contains(concat(' ', @class, ' '), ' " + name + " ')]";
  }

  instanceMethods.getElementsByClassName = Prototype.BrowserFeatures.XPath ?
  function(element, className) {
    className = className.toString().strip();
    var cond = /\s/.test(className) ? $w(className).map(iter).join('') : iter(className);
    return cond ? document._getElementsByXPath('.//*' + cond, element) : [];
  } : function(element, className) {
    className = className.toString().strip();
    var elements = [], classNames = (/\s/.test(className) ? $w(className) : null);
    if (!classNames && !className) return elements;

    var nodes = $(element).getElementsByTagName('*');
    className = ' ' + className + ' ';

    for (var i = 0, child, cn; child = nodes[i]; i++) {
      if (child.className && (cn = ' ' + child.className + ' ') && (cn.include(className) ||
          (classNames && classNames.all(function(name) {
            return !name.toString().blank() && cn.include(' ' + name + ' ');
          }))))
        elements.push(Element.extend(child));
    }
    return elements;
  };

  return function(className, parentElement) {
    return $(parentElement || document.body).getElementsByClassName(className);
  };
}(Element.Methods);

/*--------------------------------------------------------------------------*/

Element.ClassNames = Class.create();
Element.ClassNames.prototype = {
  initialize: function(element) {
    this.element = $(element);
  },

  _each: function(iterator) {
    this.element.className.split(/\s+/).select(function(name) {
      return name.length > 0;
    })._each(iterator);
  },

  set: function(className) {
    this.element.className = className;
  },

  add: function(classNameToAdd) {
    if (this.include(classNameToAdd)) return;
    this.set($A(this).concat(classNameToAdd).join(' '));
  },

  remove: function(classNameToRemove) {
    if (!this.include(classNameToRemove)) return;
    this.set($A(this).without(classNameToRemove).join(' '));
  },

  toString: function() {
    return $A(this).join(' ');
  }
};

Object.extend(Element.ClassNames.prototype, Enumerable);

/*--------------------------------------------------------------------------*/

Element.addMethods();
// script.aculo.us scriptaculous.js v1.8.1, Thu Jan 03 22:07:12 -0500 2008

// Copyright (c) 2005-2007 Thomas Fuchs (http://script.aculo.us, http://mir.aculo.us)
// 
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// For details, see the script.aculo.us web site: http://script.aculo.us/

var Scriptaculous = {
  Version: '1.8.1',
  require: function(libraryName) {
    // inserting via DOM fails in Safari 2.0, so brute force approach
    document.write('<script type="text/javascript" src="'+libraryName+'"><\/script>');
  },
  REQUIRED_PROTOTYPE: '1.6.0',
  load: function() {
    function convertVersionString(versionString){
      var r = versionString.split('.');
      return parseInt(r[0])*100000 + parseInt(r[1])*1000 + parseInt(r[2]);
    }
 
    if((typeof Prototype=='undefined') || 
       (typeof Element == 'undefined') || 
       (typeof Element.Methods=='undefined') ||
       (convertVersionString(Prototype.Version) < 
        convertVersionString(Scriptaculous.REQUIRED_PROTOTYPE)))
       throw("script.aculo.us requires the Prototype JavaScript framework >= " +
        Scriptaculous.REQUIRED_PROTOTYPE);
    
    $A(document.getElementsByTagName("script")).findAll( function(s) {
      return (s.src && s.src.match(/scriptaculous\.js(\?.*)?$/))
    }).each( function(s) {
      var path = s.src.replace(/scriptaculous\.js(\?.*)?$/,'');
      var includes = s.src.match(/\?.*load=([a-z,]*)/);
      (includes ? includes[1] : 'builder,effects,dragdrop,controls,slider,sound').split(',').each(
       function(include) { Scriptaculous.require(path+include+'.js') });
    });
  }
}

// script.aculo.us effects.js v1.8.1, Thu Jan 03 22:07:12 -0500 2008

// Copyright (c) 2005-2007 Thomas Fuchs (http://script.aculo.us, http://mir.aculo.us)
// Contributors:
//  Justin Palmer (http://encytemedia.com/)
//  Mark Pilgrim (http://diveintomark.org/)
//  Martin Bialasinki
// 
// script.aculo.us is freely distributable under the terms of an MIT-style license.
// For details, see the script.aculo.us web site: http://script.aculo.us/ 

// converts rgb() and #xxx to #xxxxxx format,  
// returns self (or first argument) if not convertable  
String.prototype.parseColor = function() {  
  var color = '#';
  if (this.slice(0,4) == 'rgb(') {  
    var cols = this.slice(4,this.length-1).split(',');  
    var i=0; do { color += parseInt(cols[i]).toColorPart() } while (++i<3);  
  } else {  
    if (this.slice(0,1) == '#') {  
      if (this.length==4) for(var i=1;i<4;i++) color += (this.charAt(i) + this.charAt(i)).toLowerCase();  
      if (this.length==7) color = this.toLowerCase();  
    }  
  }  
  return (color.length==7 ? color : (arguments[0] || this));  
};

/*--------------------------------------------------------------------------*/

Element.collectTextNodes = function(element) {  
  return $A($(element).childNodes).collect( function(node) {
    return (node.nodeType==3 ? node.nodeValue : 
      (node.hasChildNodes() ? Element.collectTextNodes(node) : ''));
  }).flatten().join('');
};

Element.collectTextNodesIgnoreClass = function(element, className) {  
  return $A($(element).childNodes).collect( function(node) {
    return (node.nodeType==3 ? node.nodeValue : 
      ((node.hasChildNodes() && !Element.hasClassName(node,className)) ? 
        Element.collectTextNodesIgnoreClass(node, className) : ''));
  }).flatten().join('');
};

Element.setContentZoom = function(element, percent) {
  element = $(element);  
  element.setStyle({fontSize: (percent/100) + 'em'});   
  if (Prototype.Browser.WebKit) window.scrollBy(0,0);
  return element;
};

Element.getInlineOpacity = function(element){
  return $(element).style.opacity || '';
};

Element.forceRerendering = function(element) {
  try {
    element = $(element);
    var n = document.createTextNode(' ');
    element.appendChild(n);
    element.removeChild(n);
  } catch(e) { }
};

/*--------------------------------------------------------------------------*/

var Effect = {
  _elementDoesNotExistError: {
    name: 'ElementDoesNotExistError',
    message: 'The specified DOM element does not exist, but is required for this effect to operate'
  },
  Transitions: {
    linear: Prototype.K,
    sinoidal: function(pos) {
      return (-Math.cos(pos*Math.PI)/2) + 0.5;
    },
    reverse: function(pos) {
      return 1-pos;
    },
    flicker: function(pos) {
      var pos = ((-Math.cos(pos*Math.PI)/4) + 0.75) + Math.random()/4;
      return pos > 1 ? 1 : pos;
    },
    wobble: function(pos) {
      return (-Math.cos(pos*Math.PI*(9*pos))/2) + 0.5;
    },
    pulse: function(pos, pulses) { 
      pulses = pulses || 5; 
      return (
        ((pos % (1/pulses)) * pulses).round() == 0 ? 
              ((pos * pulses * 2) - (pos * pulses * 2).floor()) : 
          1 - ((pos * pulses * 2) - (pos * pulses * 2).floor())
        );
    },
    spring: function(pos) { 
      return 1 - (Math.cos(pos * 4.5 * Math.PI) * Math.exp(-pos * 6)); 
    },
    none: function(pos) {
      return 0;
    },
    full: function(pos) {
      return 1;
    }
  },
  DefaultOptions: {
    duration:   1.0,   // seconds
    fps:        100,   // 100= assume 66fps max.
    sync:       false, // true for combining
    from:       0.0,
    to:         1.0,
    delay:      0.0,
    queue:      'parallel'
  },
  tagifyText: function(element) {
    var tagifyStyle = 'position:relative';
    if (Prototype.Browser.IE) tagifyStyle += ';zoom:1';
    
    element = $(element);
    $A(element.childNodes).each( function(child) {
      if (child.nodeType==3) {
        child.nodeValue.toArray().each( function(character) {
          element.insertBefore(
            new Element('span', {style: tagifyStyle}).update(
              character == ' ' ? String.fromCharCode(160) : character), 
              child);
        });
        Element.remove(child);
      }
    });
  },
  multiple: function(element, effect) {
    var elements;
    if (((typeof element == 'object') || 
        Object.isFunction(element)) && 
       (element.length))
      elements = element;
    else
      elements = $(element).childNodes;
      
    var options = Object.extend({
      speed: 0.1,
      delay: 0.0
    }, arguments[2] || { });
    var masterDelay = options.delay;

    $A(elements).each( function(element, index) {
      new effect(element, Object.extend(options, { delay: index * options.speed + masterDelay }));
    });
  },
  PAIRS: {
    'slide':  ['SlideDown','SlideUp'],
    'blind':  ['BlindDown','BlindUp'],
    'appear': ['Appear','Fade']
  },
  toggle: function(element, effect) {
    element = $(element);
    effect = (effect || 'appear').toLowerCase();
    var options = Object.extend({
      queue: { position:'end', scope:(element.id || 'global'), limit: 1 }
    }, arguments[2] || { });
    Effect[element.visible() ? 
      Effect.PAIRS[effect][1] : Effect.PAIRS[effect][0]](element, options);
  }
};

Effect.DefaultOptions.transition = Effect.Transitions.sinoidal;

/* ------------- core effects ------------- */

Effect.ScopedQueue = Class.create(Enumerable, {
  initialize: function() {
    this.effects  = [];
    this.interval = null;    
  },
  _each: function(iterator) {
    this.effects._each(iterator);
  },
  add: function(effect) {
    var timestamp = new Date().getTime();
    
    var position = Object.isString(effect.options.queue) ? 
      effect.options.queue : effect.options.queue.position;
    
    switch(position) {
      case 'front':
        // move unstarted effects after this effect  
        this.effects.findAll(function(e){ return e.state=='idle' }).each( function(e) {
            e.startOn  += effect.finishOn;
            e.finishOn += effect.finishOn;
          });
        break;
      case 'with-last':
        timestamp = this.effects.pluck('startOn').max() || timestamp;
        break;
      case 'end':
        // start effect after last queued effect has finished
        timestamp = this.effects.pluck('finishOn').max() || timestamp;
        break;
    }
    
    effect.startOn  += timestamp;
    effect.finishOn += timestamp;

    if (!effect.options.queue.limit || (this.effects.length < effect.options.queue.limit))
      this.effects.push(effect);
    
    if (!this.interval)
      this.interval = setInterval(this.loop.bind(this), 15);
  },
  remove: function(effect) {
    this.effects = this.effects.reject(function(e) { return e==effect });
    if (this.effects.length == 0) {
      clearInterval(this.interval);
      this.interval = null;
    }
  },
  loop: function() {
    var timePos = new Date().getTime();
    for(var i=0, len=this.effects.length;i<len;i++) 
      this.effects[i] && this.effects[i].loop(timePos);
  }
});

Effect.Queues = {
  instances: $H(),
  get: function(queueName) {
    if (!Object.isString(queueName)) return queueName;
    
    return this.instances.get(queueName) ||
      this.instances.set(queueName, new Effect.ScopedQueue());
  }
};
Effect.Queue = Effect.Queues.get('global');

Effect.Base = Class.create({
  position: null,
  start: function(options) {
    function codeForEvent(options,eventName){
      return (
        (options[eventName+'Internal'] ? 'this.options.'+eventName+'Internal(this);' : '') +
        (options[eventName] ? 'this.options.'+eventName+'(this);' : '')
      );
    }
    if (options && options.transition === false) options.transition = Effect.Transitions.linear;
    this.options      = Object.extend(Object.extend({ },Effect.DefaultOptions), options || { });
    this.currentFrame = 0;
    this.state        = 'idle';
    this.startOn      = this.options.delay*1000;
    this.finishOn     = this.startOn+(this.options.duration*1000);
    this.fromToDelta  = this.options.to-this.options.from;
    this.totalTime    = this.finishOn-this.startOn;
    this.totalFrames  = this.options.fps*this.options.duration;
    
    eval('this.render = function(pos){ '+
      'if (this.state=="idle"){this.state="running";'+
      codeForEvent(this.options,'beforeSetup')+
      (this.setup ? 'this.setup();':'')+ 
      codeForEvent(this.options,'afterSetup')+
      '};if (this.state=="running"){'+
      'pos=this.options.transition(pos)*'+this.fromToDelta+'+'+this.options.from+';'+
      'this.position=pos;'+
      codeForEvent(this.options,'beforeUpdate')+
      (this.update ? 'this.update(pos);':'')+
      codeForEvent(this.options,'afterUpdate')+
      '}}');
    
    this.event('beforeStart');
    if (!this.options.sync)
      Effect.Queues.get(Object.isString(this.options.queue) ? 
        'global' : this.options.queue.scope).add(this);
  },
  loop: function(timePos) {
    if (timePos >= this.startOn) {
      if (timePos >= this.finishOn) {
        this.render(1.0);
        this.cancel();
        this.event('beforeFinish');
        if (this.finish) this.finish(); 
        this.event('afterFinish');
        return;  
      }
      var pos   = (timePos - this.startOn) / this.totalTime,
          frame = (pos * this.totalFrames).round();
      if (frame > this.currentFrame) {
        this.render(pos);
        this.currentFrame = frame;
      }
    }
  },
  cancel: function() {
    if (!this.options.sync)
      Effect.Queues.get(Object.isString(this.options.queue) ? 
        'global' : this.options.queue.scope).remove(this);
    this.state = 'finished';
  },
  event: function(eventName) {
    if (this.options[eventName + 'Internal']) this.options[eventName + 'Internal'](this);
    if (this.options[eventName]) this.options[eventName](this);
  },
  inspect: function() {
    var data = $H();
    for(property in this)
      if (!Object.isFunction(this[property])) data.set(property, this[property]);
    return '#<Effect:' + data.inspect() + ',options:' + $H(this.options).inspect() + '>';
  }
});

Effect.Parallel = Class.create(Effect.Base, {
  initialize: function(effects) {
    this.effects = effects || [];
    this.start(arguments[1]);
  },
  update: function(position) {
    this.effects.invoke('render', position);
  },
  finish: function(position) {
    this.effects.each( function(effect) {
      effect.render(1.0);
      effect.cancel();
      effect.event('beforeFinish');
      if (effect.finish) effect.finish(position);
      effect.event('afterFinish');
    });
  }
});

Effect.Tween = Class.create(Effect.Base, {
  initialize: function(object, from, to) {
    object = Object.isString(object) ? $(object) : object;
    var args = $A(arguments), method = args.last(), 
      options = args.length == 5 ? args[3] : null;
    this.method = Object.isFunction(method) ? method.bind(object) :
      Object.isFunction(object[method]) ? object[method].bind(object) : 
      function(value) { object[method] = value };
    this.start(Object.extend({ from: from, to: to }, options || { }));
  },
  update: function(position) {
    this.method(position);
  }
});

Effect.Event = Class.create(Effect.Base, {
  initialize: function() {
    this.start(Object.extend({ duration: 0 }, arguments[0] || { }));
  },
  update: Prototype.emptyFunction
});

Effect.Opacity = Class.create(Effect.Base, {
  initialize: function(element) {
    this.element = $(element);
    if (!this.element) throw(Effect._elementDoesNotExistError);
    // make this work on IE on elements without 'layout'
    if (Prototype.Browser.IE && (!this.element.currentStyle.hasLayout))
      this.element.setStyle({zoom: 1});
    var options = Object.extend({
      from: this.element.getOpacity() || 0.0,
      to:   1.0
    }, arguments[1] || { });
    this.start(options);
  },
  update: function(position) {
    this.element.setOpacity(position);
  }
});

Effect.Move = Class.create(Effect.Base, {
  initialize: function(element) {
    this.element = $(element);
    if (!this.element) throw(Effect._elementDoesNotExistError);
    var options = Object.extend({
      x:    0,
      y:    0,
      mode: 'relative'
    }, arguments[1] || { });
    this.start(options);
  },
  setup: function() {
    this.element.makePositioned();
    this.originalLeft = parseFloat(this.element.getStyle('left') || '0');
    this.originalTop  = parseFloat(this.element.getStyle('top')  || '0');
    if (this.options.mode == 'absolute') {
      this.options.x = this.options.x - this.originalLeft;
      this.options.y = this.options.y - this.originalTop;
    }
  },
  update: function(position) {
    this.element.setStyle({
      left: (this.options.x  * position + this.originalLeft).round() + 'px',
      top:  (this.options.y  * position + this.originalTop).round()  + 'px'
    });
  }
});

// for backwards compatibility
Effect.MoveBy = function(element, toTop, toLeft) {
  return new Effect.Move(element, 
    Object.extend({ x: toLeft, y: toTop }, arguments[3] || { }));
};

Effect.Scale = Class.create(Effect.Base, {
  initialize: function(element, percent) {
    this.element = $(element);
    if (!this.element) throw(Effect._elementDoesNotExistError);
    var options = Object.extend({
      scaleX: true,
      scaleY: true,
      scaleContent: true,
      scaleFromCenter: false,
      scaleMode: 'box',        // 'box' or 'contents' or { } with provided values
      scaleFrom: 100.0,
      scaleTo:   percent
    }, arguments[2] || { });
    this.start(options);
  },
  setup: function() {
    this.restoreAfterFinish = this.options.restoreAfterFinish || false;
    this.elementPositioning = this.element.getStyle('position');
    
    this.originalStyle = { };
    ['top','left','width','height','fontSize'].each( function(k) {
      this.originalStyle[k] = this.element.style[k];
    }.bind(this));
      
    this.originalTop  = this.element.offsetTop;
    this.originalLeft = this.element.offsetLeft;
    
    var fontSize = this.element.getStyle('font-size') || '100%';
    ['em','px','%','pt'].each( function(fontSizeType) {
      if (fontSize.indexOf(fontSizeType)>0) {
        this.fontSize     = parseFloat(fontSize);
        this.fontSizeType = fontSizeType;
      }
    }.bind(this));
    
    this.factor = (this.options.scaleTo - this.options.scaleFrom)/100;
    
    this.dims = null;
    if (this.options.scaleMode=='box')
      this.dims = [this.element.offsetHeight, this.element.offsetWidth];
    if (/^content/.test(this.options.scaleMode))
      this.dims = [this.element.scrollHeight, this.element.scrollWidth];
    if (!this.dims)
      this.dims = [this.options.scaleMode.originalHeight,
                   this.options.scaleMode.originalWidth];
  },
  update: function(position) {
    var currentScale = (this.options.scaleFrom/100.0) + (this.factor * position);
    if (this.options.scaleContent && this.fontSize)
      this.element.setStyle({fontSize: this.fontSize * currentScale + this.fontSizeType });
    this.setDimensions(this.dims[0] * currentScale, this.dims[1] * currentScale);
  },
  finish: function(position) {
    if (this.restoreAfterFinish) this.element.setStyle(this.originalStyle);
  },
  setDimensions: function(height, width) {
    var d = { };
    if (this.options.scaleX) d.width = width.round() + 'px';
    if (this.options.scaleY) d.height = height.round() + 'px';
    if (this.options.scaleFromCenter) {
      var topd  = (height - this.dims[0])/2;
      var leftd = (width  - this.dims[1])/2;
      if (this.elementPositioning == 'absolute') {
        if (this.options.scaleY) d.top = this.originalTop-topd + 'px';
        if (this.options.scaleX) d.left = this.originalLeft-leftd + 'px';
      } else {
        if (this.options.scaleY) d.top = -topd + 'px';
        if (this.options.scaleX) d.left = -leftd + 'px';
      }
    }
    this.element.setStyle(d);
  }
});

Effect.Highlight = Class.create(Effect.Base, {
  initialize: function(element) {
    this.element = $(element);
    if (!this.element) throw(Effect._elementDoesNotExistError);
    var options = Object.extend({ startcolor: '#ffff99' }, arguments[1] || { });
    this.start(options);
  },
  setup: function() {
    // Prevent executing on elements not in the layout flow
    if (this.element.getStyle('display')=='none') { this.cancel(); return; }
    // Disable background image during the effect
    this.oldStyle = { };
    if (!this.options.keepBackgroundImage) {
      this.oldStyle.backgroundImage = this.element.getStyle('background-image');
      this.element.setStyle({backgroundImage: 'none'});
    }
    if (!this.options.endcolor)
      this.options.endcolor = this.element.getStyle('background-color').parseColor('#ffffff');
    if (!this.options.restorecolor)
      this.options.restorecolor = this.element.getStyle('background-color');
    // init color calculations
    this._base  = $R(0,2).map(function(i){ return parseInt(this.options.startcolor.slice(i*2+1,i*2+3),16) }.bind(this));
    this._delta = $R(0,2).map(function(i){ return parseInt(this.options.endcolor.slice(i*2+1,i*2+3),16)-this._base[i] }.bind(this));
  },
  update: function(position) {
    this.element.setStyle({backgroundColor: $R(0,2).inject('#',function(m,v,i){
      return m+((this._base[i]+(this._delta[i]*position)).round().toColorPart()); }.bind(this)) });
  },
  finish: function() {
    this.element.setStyle(Object.extend(this.oldStyle, {
      backgroundColor: this.options.restorecolor
    }));
  }
});

Effect.ScrollTo = function(element) {
  var options = arguments[1] || { },
    scrollOffsets = document.viewport.getScrollOffsets(),
    elementOffsets = $(element).cumulativeOffset(),
    max = (window.height || document.body.scrollHeight) - document.viewport.getHeight();  

  if (options.offset) elementOffsets[1] += options.offset;

  return new Effect.Tween(null,
    scrollOffsets.top,
    elementOffsets[1] > max ? max : elementOffsets[1],
    options,
    function(p){ scrollTo(scrollOffsets.left, p.round()) }
  );
};

/* ------------- combination effects ------------- */

Effect.Fade = function(element) {
  element = $(element);
  var oldOpacity = element.getInlineOpacity();
  var options = Object.extend({
    from: element.getOpacity() || 1.0,
    to:   0.0,
    afterFinishInternal: function(effect) { 
      if (effect.options.to!=0) return;
      effect.element.hide().setStyle({opacity: oldOpacity}); 
    }
  }, arguments[1] || { });
  return new Effect.Opacity(element,options);
};

Effect.Appear = function(element) {
  element = $(element);
  var options = Object.extend({
  from: (element.getStyle('display') == 'none' ? 0.0 : element.getOpacity() || 0.0),
  to:   1.0,
  // force Safari to render floated elements properly
  afterFinishInternal: function(effect) {
    effect.element.forceRerendering();
  },
  beforeSetup: function(effect) {
    effect.element.setOpacity(effect.options.from).show(); 
  }}, arguments[1] || { });
  return new Effect.Opacity(element,options);
};

Effect.Puff = function(element) {
  element = $(element);
  var oldStyle = { 
    opacity: element.getInlineOpacity(), 
    position: element.getStyle('position'),
    top:  element.style.top,
    left: element.style.left,
    width: element.style.width,
    height: element.style.height
  };
  return new Effect.Parallel(
   [ new Effect.Scale(element, 200, 
      { sync: true, scaleFromCenter: true, scaleContent: true, restoreAfterFinish: true }), 
     new Effect.Opacity(element, { sync: true, to: 0.0 } ) ], 
     Object.extend({ duration: 1.0, 
      beforeSetupInternal: function(effect) {
        Position.absolutize(effect.effects[0].element)
      },
      afterFinishInternal: function(effect) {
         effect.effects[0].element.hide().setStyle(oldStyle); }
     }, arguments[1] || { })
   );
};

Effect.BlindUp = function(element) {
  element = $(element);
  element.makeClipping();
  return new Effect.Scale(element, 0,
    Object.extend({ scaleContent: false, 
      scaleX: false, 
      restoreAfterFinish: true,
      afterFinishInternal: function(effect) {
        effect.element.hide().undoClipping();
      } 
    }, arguments[1] || { })
  );
};

Effect.BlindDown = function(element) {
  element = $(element);
  var elementDimensions = element.getDimensions();
  return new Effect.Scale(element, 100, Object.extend({ 
    scaleContent: false, 
    scaleX: false,
    scaleFrom: 0,
    scaleMode: {originalHeight: elementDimensions.height, originalWidth: elementDimensions.width},
    restoreAfterFinish: true,
    afterSetup: function(effect) {
      effect.element.makeClipping().setStyle({height: '0px'}).show(); 
    },  
    afterFinishInternal: function(effect) {
      effect.element.undoClipping();
    }
  }, arguments[1] || { }));
};

Effect.SwitchOff = function(element) {
  element = $(element);
  var oldOpacity = element.getInlineOpacity();
  return new Effect.Appear(element, Object.extend({
    duration: 0.4,
    from: 0,
    transition: Effect.Transitions.flicker,
    afterFinishInternal: function(effect) {
      new Effect.Scale(effect.element, 1, { 
        duration: 0.3, scaleFromCenter: true,
        scaleX: false, scaleContent: false, restoreAfterFinish: true,
        beforeSetup: function(effect) { 
          effect.element.makePositioned().makeClipping();
        },
        afterFinishInternal: function(effect) {
          effect.element.hide().undoClipping().undoPositioned().setStyle({opacity: oldOpacity});
        }
      })
    }
  }, arguments[1] || { }));
};

Effect.DropOut = function(element) {
  element = $(element);
  var oldStyle = {
    top: element.getStyle('top'),
    left: element.getStyle('left'),
    opacity: element.getInlineOpacity() };
  return new Effect.Parallel(
    [ new Effect.Move(element, {x: 0, y: 100, sync: true }), 
      new Effect.Opacity(element, { sync: true, to: 0.0 }) ],
    Object.extend(
      { duration: 0.5,
        beforeSetup: function(effect) {
          effect.effects[0].element.makePositioned(); 
        },
        afterFinishInternal: function(effect) {
          effect.effects[0].element.hide().undoPositioned().setStyle(oldStyle);
        } 
      }, arguments[1] || { }));
};

Effect.Shake = function(element) {
  element = $(element);
  var options = Object.extend({
    distance: 20,
    duration: 0.5
  }, arguments[1] || {});
  var distance = parseFloat(options.distance);
  var split = parseFloat(options.duration) / 10.0;
  var oldStyle = {
    top: element.getStyle('top'),
    left: element.getStyle('left') };
    return new Effect.Move(element,
      { x:  distance, y: 0, duration: split, afterFinishInternal: function(effect) {
    new Effect.Move(effect.element,
      { x: -distance*2, y: 0, duration: split*2,  afterFinishInternal: function(effect) {
    new Effect.Move(effect.element,
      { x:  distance*2, y: 0, duration: split*2,  afterFinishInternal: function(effect) {
    new Effect.Move(effect.element,
      { x: -distance*2, y: 0, duration: split*2,  afterFinishInternal: function(effect) {
    new Effect.Move(effect.element,
      { x:  distance*2, y: 0, duration: split*2,  afterFinishInternal: function(effect) {
    new Effect.Move(effect.element,
      { x: -distance, y: 0, duration: split, afterFinishInternal: function(effect) {
        effect.element.undoPositioned().setStyle(oldStyle);
  }}) }}) }}) }}) }}) }});
};

Effect.SlideDown = function(element) {
  element = $(element).cleanWhitespace();
  // SlideDown need to have the content of the element wrapped in a container element with fixed height!
  var oldInnerBottom = element.down().getStyle('bottom');
  var elementDimensions = element.getDimensions();
  return new Effect.Scale(element, 100, Object.extend({ 
    scaleContent: false, 
    scaleX: false, 
    scaleFrom: window.opera ? 0 : 1,
    scaleMode: {originalHeight: elementDimensions.height, originalWidth: elementDimensions.width},
    restoreAfterFinish: true,
    afterSetup: function(effect) {
      effect.element.makePositioned();
      effect.element.down().makePositioned();
      if (window.opera) effect.element.setStyle({top: ''});
      effect.element.makeClipping().setStyle({height: '0px'}).show(); 
    },
    afterUpdateInternal: function(effect) {
      effect.element.down().setStyle({bottom:
        (effect.dims[0] - effect.element.clientHeight) + 'px' }); 
    },
    afterFinishInternal: function(effect) {
      effect.element.undoClipping().undoPositioned();
      effect.element.down().undoPositioned().setStyle({bottom: oldInnerBottom}); }
    }, arguments[1] || { })
  );
};

Effect.SlideUp = function(element) {
  element = $(element).cleanWhitespace();
  var oldInnerBottom = element.down().getStyle('bottom');
  var elementDimensions = element.getDimensions();
  return new Effect.Scale(element, window.opera ? 0 : 1,
   Object.extend({ scaleContent: false, 
    scaleX: false, 
    scaleMode: 'box',
    scaleFrom: 100,
    scaleMode: {originalHeight: elementDimensions.height, originalWidth: elementDimensions.width},
    restoreAfterFinish: true,
    afterSetup: function(effect) {
      effect.element.makePositioned();
      effect.element.down().makePositioned();
      if (window.opera) effect.element.setStyle({top: ''});
      effect.element.makeClipping().show();
    },  
    afterUpdateInternal: function(effect) {
      effect.element.down().setStyle({bottom:
        (effect.dims[0] - effect.element.clientHeight) + 'px' });
    },
    afterFinishInternal: function(effect) {
      effect.element.hide().undoClipping().undoPositioned();
      effect.element.down().undoPositioned().setStyle({bottom: oldInnerBottom});
    }
   }, arguments[1] || { })
  );
};

// Bug in opera makes the TD containing this element expand for a instance after finish 
Effect.Squish = function(element) {
  return new Effect.Scale(element, window.opera ? 1 : 0, { 
    restoreAfterFinish: true,
    beforeSetup: function(effect) {
      effect.element.makeClipping(); 
    },  
    afterFinishInternal: function(effect) {
      effect.element.hide().undoClipping(); 
    }
  });
};

Effect.Grow = function(element) {
  element = $(element);
  var options = Object.extend({
    direction: 'center',
    moveTransition: Effect.Transitions.sinoidal,
    scaleTransition: Effect.Transitions.sinoidal,
    opacityTransition: Effect.Transitions.full
  }, arguments[1] || { });
  var oldStyle = {
    top: element.style.top,
    left: element.style.left,
    height: element.style.height,
    width: element.style.width,
    opacity: element.getInlineOpacity() };

  var dims = element.getDimensions();    
  var initialMoveX, initialMoveY;
  var moveX, moveY;
  
  switch (options.direction) {
    case 'top-left':
      initialMoveX = initialMoveY = moveX = moveY = 0; 
      break;
    case 'top-right':
      initialMoveX = dims.width;
      initialMoveY = moveY = 0;
      moveX = -dims.width;
      break;
    case 'bottom-left':
      initialMoveX = moveX = 0;
      initialMoveY = dims.height;
      moveY = -dims.height;
      break;
    case 'bottom-right':
      initialMoveX = dims.width;
      initialMoveY = dims.height;
      moveX = -dims.width;
      moveY = -dims.height;
      break;
    case 'center':
      initialMoveX = dims.width / 2;
      initialMoveY = dims.height / 2;
      moveX = -dims.width / 2;
      moveY = -dims.height / 2;
      break;
  }
  
  return new Effect.Move(element, {
    x: initialMoveX,
    y: initialMoveY,
    duration: 0.01, 
    beforeSetup: function(effect) {
      effect.element.hide().makeClipping().makePositioned();
    },
    afterFinishInternal: function(effect) {
      new Effect.Parallel(
        [ new Effect.Opacity(effect.element, { sync: true, to: 1.0, from: 0.0, transition: options.opacityTransition }),
          new Effect.Move(effect.element, { x: moveX, y: moveY, sync: true, transition: options.moveTransition }),
          new Effect.Scale(effect.element, 100, {
            scaleMode: { originalHeight: dims.height, originalWidth: dims.width }, 
            sync: true, scaleFrom: window.opera ? 1 : 0, transition: options.scaleTransition, restoreAfterFinish: true})
        ], Object.extend({
             beforeSetup: function(effect) {
               effect.effects[0].element.setStyle({height: '0px'}).show(); 
             },
             afterFinishInternal: function(effect) {
               effect.effects[0].element.undoClipping().undoPositioned().setStyle(oldStyle); 
             }
           }, options)
      )
    }
  });
};

Effect.Shrink = function(element) {
  element = $(element);
  var options = Object.extend({
    direction: 'center',
    moveTransition: Effect.Transitions.sinoidal,
    scaleTransition: Effect.Transitions.sinoidal,
    opacityTransition: Effect.Transitions.none
  }, arguments[1] || { });
  var oldStyle = {
    top: element.style.top,
    left: element.style.left,
    height: element.style.height,
    width: element.style.width,
    opacity: element.getInlineOpacity() };

  var dims = element.getDimensions();
  var moveX, moveY;
  
  switch (options.direction) {
    case 'top-left':
      moveX = moveY = 0;
      break;
    case 'top-right':
      moveX = dims.width;
      moveY = 0;
      break;
    case 'bottom-left':
      moveX = 0;
      moveY = dims.height;
      break;
    case 'bottom-right':
      moveX = dims.width;
      moveY = dims.height;
      break;
    case 'center':  
      moveX = dims.width / 2;
      moveY = dims.height / 2;
      break;
  }
  
  return new Effect.Parallel(
    [ new Effect.Opacity(element, { sync: true, to: 0.0, from: 1.0, transition: options.opacityTransition }),
      new Effect.Scale(element, window.opera ? 1 : 0, { sync: true, transition: options.scaleTransition, restoreAfterFinish: true}),
      new Effect.Move(element, { x: moveX, y: moveY, sync: true, transition: options.moveTransition })
    ], Object.extend({            
         beforeStartInternal: function(effect) {
           effect.effects[0].element.makePositioned().makeClipping(); 
         },
         afterFinishInternal: function(effect) {
           effect.effects[0].element.hide().undoClipping().undoPositioned().setStyle(oldStyle); }
       }, options)
  );
};

Effect.Pulsate = function(element) {
  element = $(element);
  var options    = arguments[1] || { };
  var oldOpacity = element.getInlineOpacity();
  var transition = options.transition || Effect.Transitions.sinoidal;
  var reverser   = function(pos){ return transition(1-Effect.Transitions.pulse(pos, options.pulses)) };
  reverser.bind(transition);
  return new Effect.Opacity(element, 
    Object.extend(Object.extend({  duration: 2.0, from: 0,
      afterFinishInternal: function(effect) { effect.element.setStyle({opacity: oldOpacity}); }
    }, options), {transition: reverser}));
};

Effect.Fold = function(element) {
  element = $(element);
  var oldStyle = {
    top: element.style.top,
    left: element.style.left,
    width: element.style.width,
    height: element.style.height };
  element.makeClipping();
  return new Effect.Scale(element, 5, Object.extend({   
    scaleContent: false,
    scaleX: false,
    afterFinishInternal: function(effect) {
    new Effect.Scale(element, 1, { 
      scaleContent: false, 
      scaleY: false,
      afterFinishInternal: function(effect) {
        effect.element.hide().undoClipping().setStyle(oldStyle);
      } });
  }}, arguments[1] || { }));
};

Effect.Morph = Class.create(Effect.Base, {
  initialize: function(element) {
    this.element = $(element);
    if (!this.element) throw(Effect._elementDoesNotExistError);
    var options = Object.extend({
      style: { }
    }, arguments[1] || { });
    
    if (!Object.isString(options.style)) this.style = $H(options.style);
    else {
      if (options.style.include(':'))
        this.style = options.style.parseStyle();
      else {
        this.element.addClassName(options.style);
        this.style = $H(this.element.getStyles());
        this.element.removeClassName(options.style);
        var css = this.element.getStyles();
        this.style = this.style.reject(function(style) {
          return style.value == css[style.key];
        });
        options.afterFinishInternal = function(effect) {
          effect.element.addClassName(effect.options.style);
          effect.transforms.each(function(transform) {
            effect.element.style[transform.style] = '';
          });
        }
      }
    }
    this.start(options);
  },
  
  setup: function(){
    function parseColor(color){
      if (!color || ['rgba(0, 0, 0, 0)','transparent'].include(color)) color = '#ffffff';
      color = color.parseColor();
      return $R(0,2).map(function(i){
        return parseInt( color.slice(i*2+1,i*2+3), 16 ) 
      });
    }
    this.transforms = this.style.map(function(pair){
      var property = pair[0], value = pair[1], unit = null;

      if (value.parseColor('#zzzzzz') != '#zzzzzz') {
        value = value.parseColor();
        unit  = 'color';
      } else if (property == 'opacity') {
        value = parseFloat(value);
        if (Prototype.Browser.IE && (!this.element.currentStyle.hasLayout))
          this.element.setStyle({zoom: 1});
      } else if (Element.CSS_LENGTH.test(value)) {
          var components = value.match(/^([\+\-]?[0-9\.]+)(.*)$/);
          value = parseFloat(components[1]);
          unit = (components.length == 3) ? components[2] : null;
      }

      var originalValue = this.element.getStyle(property);
      return { 
        style: property.camelize(), 
        originalValue: unit=='color' ? parseColor(originalValue) : parseFloat(originalValue || 0), 
        targetValue: unit=='color' ? parseColor(value) : value,
        unit: unit
      };
    }.bind(this)).reject(function(transform){
      return (
        (transform.originalValue == transform.targetValue) ||
        (
          transform.unit != 'color' &&
          (isNaN(transform.originalValue) || isNaN(transform.targetValue))
        )
      )
    });
  },
  update: function(position) {
    var style = { }, transform, i = this.transforms.length;
    while(i--)
      style[(transform = this.transforms[i]).style] = 
        transform.unit=='color' ? '#'+
          (Math.round(transform.originalValue[0]+
            (transform.targetValue[0]-transform.originalValue[0])*position)).toColorPart() +
          (Math.round(transform.originalValue[1]+
            (transform.targetValue[1]-transform.originalValue[1])*position)).toColorPart() +
          (Math.round(transform.originalValue[2]+
            (transform.targetValue[2]-transform.originalValue[2])*position)).toColorPart() :
        (transform.originalValue +
          (transform.targetValue - transform.originalValue) * position).toFixed(3) + 
            (transform.unit === null ? '' : transform.unit);
    this.element.setStyle(style, true);
  }
});

Effect.Transform = Class.create({
  initialize: function(tracks){
    this.tracks  = [];
    this.options = arguments[1] || { };
    this.addTracks(tracks);
  },
  addTracks: function(tracks){
    tracks.each(function(track){
      track = $H(track);
      var data = track.values().first();
      this.tracks.push($H({
        ids:     track.keys().first(),
        effect:  Effect.Morph,
        options: { style: data }
      }));
    }.bind(this));
    return this;
  },
  play: function(){
    return new Effect.Parallel(
      this.tracks.map(function(track){
        var ids = track.get('ids'), effect = track.get('effect'), options = track.get('options');
        var elements = [$(ids) || $$(ids)].flatten();
        return elements.map(function(e){ return new effect(e, Object.extend({ sync:true }, options)) });
      }).flatten(),
      this.options
    );
  }
});

Element.CSS_PROPERTIES = $w(
  'backgroundColor backgroundPosition borderBottomColor borderBottomStyle ' + 
  'borderBottomWidth borderLeftColor borderLeftStyle borderLeftWidth ' +
  'borderRightColor borderRightStyle borderRightWidth borderSpacing ' +
  'borderTopColor borderTopStyle borderTopWidth bottom clip color ' +
  'fontSize fontWeight height left letterSpacing lineHeight ' +
  'marginBottom marginLeft marginRight marginTop markerOffset maxHeight '+
  'maxWidth minHeight minWidth opacity outlineColor outlineOffset ' +
  'outlineWidth paddingBottom paddingLeft paddingRight paddingTop ' +
  'right textIndent top width wordSpacing zIndex');
  
Element.CSS_LENGTH = /^(([\+\-]?[0-9\.]+)(em|ex|px|in|cm|mm|pt|pc|\%))|0$/;

String.__parseStyleElement = document.createElement('div');
String.prototype.parseStyle = function(){
  var style, styleRules = $H();
  if (Prototype.Browser.WebKit)
    style = new Element('div',{style:this}).style;
  else {
    String.__parseStyleElement.innerHTML = '<div style="' + this + '"></div>';
    style = String.__parseStyleElement.childNodes[0].style;
  }
  
  Element.CSS_PROPERTIES.each(function(property){
    if (style[property]) styleRules.set(property, style[property]); 
  });
  
  if (Prototype.Browser.IE && this.include('opacity'))
    styleRules.set('opacity', this.match(/opacity:\s*((?:0|1)?(?:\.\d*)?)/)[1]);

  return styleRules;
};

if (document.defaultView && document.defaultView.getComputedStyle) {
  Element.getStyles = function(element) {
    var css = document.defaultView.getComputedStyle($(element), null);
    return Element.CSS_PROPERTIES.inject({ }, function(styles, property) {
      styles[property] = css[property];
      return styles;
    });
  };
} else {
  Element.getStyles = function(element) {
    element = $(element);
    var css = element.currentStyle, styles;
    styles = Element.CSS_PROPERTIES.inject({ }, function(results, property) {
      results[property] = css[property];
      return results;
    });
    if (!styles.opacity) styles.opacity = element.getOpacity();
    return styles;
  };
};

Effect.Methods = {
  morph: function(element, style) {
    element = $(element);
    new Effect.Morph(element, Object.extend({ style: style }, arguments[2] || { }));
    return element;
  },
  visualEffect: function(element, effect, options) {
    element = $(element)
    var s = effect.dasherize().camelize(), klass = s.charAt(0).toUpperCase() + s.substring(1);
    new Effect[klass](element, options);
    return element;
  },
  highlight: function(element, options) {
    element = $(element);
    new Effect.Highlight(element, options);
    return element;
  }
};

$w('fade appear grow shrink fold blindUp blindDown slideUp slideDown '+
  'pulsate shake puff squish switchOff dropOut').each(
  function(effect) { 
    Effect.Methods[effect] = function(element, options){
      element = $(element);
      Effect[effect.charAt(0).toUpperCase() + effect.substring(1)](element, options);
      return element;
    }
  }
);

$w('getInlineOpacity forceRerendering setContentZoom collectTextNodes collectTextNodesIgnoreClass getStyles').each( 
  function(f) { Effect.Methods[f] = Element[f]; }
);

Element.addMethods(Effect.Methods);
// script.aculo.us dragdrop.js v1.8.1, Thu Jan 03 22:07:12 -0500 2008

// Copyright (c) 2005-2007 Thomas Fuchs (http://script.aculo.us, http://mir.aculo.us)
//           (c) 2005-2007 Sammi Williams (http://www.oriontransfer.co.nz, sammi@oriontransfer.co.nz)
// 
// script.aculo.us is freely distributable under the terms of an MIT-style license.
// For details, see the script.aculo.us web site: http://script.aculo.us/

if(Object.isUndefined(Effect))
  throw("dragdrop.js requires including script.aculo.us' effects.js library");

var Droppables = {
  drops: [],

  remove: function(element) {
    this.drops = this.drops.reject(function(d) { return d.element==$(element) });
  },

  add: function(element) {
    element = $(element);
    var options = Object.extend({
      greedy:     true,
      hoverclass: null,
      tree:       false
    }, arguments[1] || { });

    // cache containers
    if(options.containment) {
      options._containers = [];
      var containment = options.containment;
      if(Object.isArray(containment)) {
        containment.each( function(c) { options._containers.push($(c)) });
      } else {
        options._containers.push($(containment));
      }
    }
    
    if(options.accept) options.accept = [options.accept].flatten();

    Element.makePositioned(element); // fix IE
    options.element = element;

    this.drops.push(options);
  },
  
  findDeepestChild: function(drops) {
    deepest = drops[0];
      
    for (i = 1; i < drops.length; ++i)
      if (Element.isParent(drops[i].element, deepest.element))
        deepest = drops[i];
    
    return deepest;
  },

  isContained: function(element, drop) {
    var containmentNode;
    if(drop.tree) {
      containmentNode = element.treeNode; 
    } else {
      containmentNode = element.parentNode;
    }
    return drop._containers.detect(function(c) { return containmentNode == c });
  },
  
  isAffected: function(point, element, drop) {
    return (
      (drop.element!=element) &&
      ((!drop._containers) ||
        this.isContained(element, drop)) &&
      ((!drop.accept) ||
        (Element.classNames(element).detect( 
          function(v) { return drop.accept.include(v) } ) )) &&
      Position.within(drop.element, point[0], point[1]) );
  },

  deactivate: function(drop) {
    if(drop.hoverclass)
      Element.removeClassName(drop.element, drop.hoverclass);
    this.last_active = null;
  },

  activate: function(drop) {
    if(drop.hoverclass)
      Element.addClassName(drop.element, drop.hoverclass);
    this.last_active = drop;
  },

  show: function(point, element) {
    if(!this.drops.length) return;
    var drop, affected = [];
    
    this.drops.each( function(drop) {
      if(Droppables.isAffected(point, element, drop))
        affected.push(drop);
    });
        
    if(affected.length>0)
      drop = Droppables.findDeepestChild(affected);

    if(this.last_active && this.last_active != drop) this.deactivate(this.last_active);
    if (drop) {
      Position.within(drop.element, point[0], point[1]);
      if(drop.onHover)
        drop.onHover(element, drop.element, Position.overlap(drop.overlap, drop.element));
      
      if (drop != this.last_active) Droppables.activate(drop);
    }
  },

  fire: function(event, element) {
    if(!this.last_active) return;
    Position.prepare();

    if (this.isAffected([Event.pointerX(event), Event.pointerY(event)], element, this.last_active))
      if (this.last_active.onDrop) {
        this.last_active.onDrop(element, this.last_active.element, event); 
        return true; 
      }
  },

  reset: function() {
    if(this.last_active)
      this.deactivate(this.last_active);
  }
}

var Draggables = {
  drags: [],
  observers: [],
  
  register: function(draggable) {
    if(this.drags.length == 0) {
      this.eventMouseUp   = this.endDrag.bindAsEventListener(this);
      this.eventMouseMove = this.updateDrag.bindAsEventListener(this);
      this.eventKeypress  = this.keyPress.bindAsEventListener(this);
      
      Event.observe(document, "mouseup", this.eventMouseUp);
      Event.observe(document, "mousemove", this.eventMouseMove);
      Event.observe(document, "keypress", this.eventKeypress);
    }
    this.drags.push(draggable);
  },
  
  unregister: function(draggable) {
    this.drags = this.drags.reject(function(d) { return d==draggable });
    if(this.drags.length == 0) {
      Event.stopObserving(document, "mouseup", this.eventMouseUp);
      Event.stopObserving(document, "mousemove", this.eventMouseMove);
      Event.stopObserving(document, "keypress", this.eventKeypress);
    }
  },
  
  activate: function(draggable) {
    if(draggable.options.delay) { 
      this._timeout = setTimeout(function() { 
        Draggables._timeout = null; 
        window.focus(); 
        Draggables.activeDraggable = draggable; 
      }.bind(this), draggable.options.delay); 
    } else {
      window.focus(); // allows keypress events if window isn't currently focused, fails for Safari
      this.activeDraggable = draggable;
    }
  },
  
  deactivate: function() {
    this.activeDraggable = null;
  },
  
  updateDrag: function(event) {
    if(!this.activeDraggable) return;
    var pointer = [Event.pointerX(event), Event.pointerY(event)];
    // Mozilla-based browsers fire successive mousemove events with
    // the same coordinates, prevent needless redrawing (moz bug?)
    if(this._lastPointer && (this._lastPointer.inspect() == pointer.inspect())) return;
    this._lastPointer = pointer;
    
    this.activeDraggable.updateDrag(event, pointer);
  },
  
  endDrag: function(event) {
    if(this._timeout) { 
      clearTimeout(this._timeout); 
      this._timeout = null; 
    }
    if(!this.activeDraggable) return;
    this._lastPointer = null;
    this.activeDraggable.endDrag(event);
    this.activeDraggable = null;
  },
  
  keyPress: function(event) {
    if(this.activeDraggable)
      this.activeDraggable.keyPress(event);
  },
  
  addObserver: function(observer) {
    this.observers.push(observer);
    this._cacheObserverCallbacks();
  },
  
  removeObserver: function(element) {  // element instead of observer fixes mem leaks
    this.observers = this.observers.reject( function(o) { return o.element==element });
    this._cacheObserverCallbacks();
  },
  
  notify: function(eventName, draggable, event) {  // 'onStart', 'onEnd', 'onDrag'
    if(this[eventName+'Count'] > 0)
      this.observers.each( function(o) {
        if(o[eventName]) o[eventName](eventName, draggable, event);
      });
    if(draggable.options[eventName]) draggable.options[eventName](draggable, event);
  },
  
  _cacheObserverCallbacks: function() {
    ['onStart','onEnd','onDrag'].each( function(eventName) {
      Draggables[eventName+'Count'] = Draggables.observers.select(
        function(o) { return o[eventName]; }
      ).length;
    });
  }
}

/*--------------------------------------------------------------------------*/

var Draggable = Class.create({
  initialize: function(element) {
    var defaults = {
      handle: false,
      reverteffect: function(element, top_offset, left_offset) {
        var dur = Math.sqrt(Math.abs(top_offset^2)+Math.abs(left_offset^2))*0.02;
        new Effect.Move(element, { x: -left_offset, y: -top_offset, duration: dur,
          queue: {scope:'_draggable', position:'end'}
        });
      },
      endeffect: function(element) {
        var toOpacity = Object.isNumber(element._opacity) ? element._opacity : 1.0;
        new Effect.Opacity(element, {duration:0.2, from:0.7, to:toOpacity, 
          queue: {scope:'_draggable', position:'end'},
          afterFinish: function(){ 
            Draggable._dragging[element] = false 
          }
        }); 
      },
      zindex: 1000,
      revert: false,
      quiet: false,
      scroll: false,
      scrollSensitivity: 20,
      scrollSpeed: 15,
      snap: false,  // false, or xy or [x,y] or function(x,y){ return [x,y] }
      delay: 0
    };
    
    if(!arguments[1] || Object.isUndefined(arguments[1].endeffect))
      Object.extend(defaults, {
        starteffect: function(element) {
          element._opacity = Element.getOpacity(element);
          Draggable._dragging[element] = true;
          new Effect.Opacity(element, {duration:0.2, from:element._opacity, to:0.7}); 
        }
      });
    
    var options = Object.extend(defaults, arguments[1] || { });

    this.element = $(element);
    
    if(options.handle && Object.isString(options.handle))
      this.handle = this.element.down('.'+options.handle, 0);
    
    if(!this.handle) this.handle = $(options.handle);
    if(!this.handle) this.handle = this.element;
    
    if(options.scroll && !options.scroll.scrollTo && !options.scroll.outerHTML) {
      options.scroll = $(options.scroll);
      this._isScrollChild = Element.childOf(this.element, options.scroll);
    }

    Element.makePositioned(this.element); // fix IE    

    this.options  = options;
    this.dragging = false;   

    this.eventMouseDown = this.initDrag.bindAsEventListener(this);
    Event.observe(this.handle, "mousedown", this.eventMouseDown);
    
    Draggables.register(this);
  },
  
  destroy: function() {
    Event.stopObserving(this.handle, "mousedown", this.eventMouseDown);
    Draggables.unregister(this);
  },
  
  currentDelta: function() {
    return([
      parseInt(Element.getStyle(this.element,'left') || '0'),
      parseInt(Element.getStyle(this.element,'top') || '0')]);
  },
  
  initDrag: function(event) {
    if(!Object.isUndefined(Draggable._dragging[this.element]) &&
      Draggable._dragging[this.element]) return;
    if(Event.isLeftClick(event)) {    
      // abort on form elements, fixes a Firefox issue
      var src = Event.element(event);
      if((tag_name = src.tagName.toUpperCase()) && (
        tag_name=='INPUT' ||
        tag_name=='SELECT' ||
        tag_name=='OPTION' ||
        tag_name=='BUTTON' ||
        tag_name=='TEXTAREA')) return;
        
      var pointer = [Event.pointerX(event), Event.pointerY(event)];
      var pos     = Position.cumulativeOffset(this.element);
      this.offset = [0,1].map( function(i) { return (pointer[i] - pos[i]) });
      
      Draggables.activate(this);
      Event.stop(event);
    }
  },
  
  startDrag: function(event) {
    this.dragging = true;
    if(!this.delta)
      this.delta = this.currentDelta();
    
    if(this.options.zindex) {
      this.originalZ = parseInt(Element.getStyle(this.element,'z-index') || 0);
      this.element.style.zIndex = this.options.zindex;
    }
    
    if(this.options.ghosting) {
      this._clone = this.element.cloneNode(true);
      this.element._originallyAbsolute = (this.element.getStyle('position') == 'absolute');
      if (!this.element._originallyAbsolute)
        Position.absolutize(this.element);
      this.element.parentNode.insertBefore(this._clone, this.element);
    }
    
    if(this.options.scroll) {
      if (this.options.scroll == window) {
        var where = this._getWindowScroll(this.options.scroll);
        this.originalScrollLeft = where.left;
        this.originalScrollTop = where.top;
      } else {
        this.originalScrollLeft = this.options.scroll.scrollLeft;
        this.originalScrollTop = this.options.scroll.scrollTop;
      }
    }
    
    Draggables.notify('onStart', this, event);
        
    if(this.options.starteffect) this.options.starteffect(this.element);
  },
  
  updateDrag: function(event, pointer) {
    if(!this.dragging) this.startDrag(event);
    
    if(!this.options.quiet){
      Position.prepare();
      Droppables.show(pointer, this.element);
    }
    
    Draggables.notify('onDrag', this, event);
    
    this.draw(pointer);
    if(this.options.change) this.options.change(this);
    
    if(this.options.scroll) {
      this.stopScrolling();
      
      var p;
      if (this.options.scroll == window) {
        with(this._getWindowScroll(this.options.scroll)) { p = [ left, top, left+width, top+height ]; }
      } else {
        p = Position.page(this.options.scroll);
        p[0] += this.options.scroll.scrollLeft + Position.deltaX;
        p[1] += this.options.scroll.scrollTop + Position.deltaY;
        p.push(p[0]+this.options.scroll.offsetWidth);
        p.push(p[1]+this.options.scroll.offsetHeight);
      }
      var speed = [0,0];
      if(pointer[0] < (p[0]+this.options.scrollSensitivity)) speed[0] = pointer[0]-(p[0]+this.options.scrollSensitivity);
      if(pointer[1] < (p[1]+this.options.scrollSensitivity)) speed[1] = pointer[1]-(p[1]+this.options.scrollSensitivity);
      if(pointer[0] > (p[2]-this.options.scrollSensitivity)) speed[0] = pointer[0]-(p[2]-this.options.scrollSensitivity);
      if(pointer[1] > (p[3]-this.options.scrollSensitivity)) speed[1] = pointer[1]-(p[3]-this.options.scrollSensitivity);
      this.startScrolling(speed);
    }
    
    // fix AppleWebKit rendering
    if(Prototype.Browser.WebKit) window.scrollBy(0,0);
    
    Event.stop(event);
  },
  
  finishDrag: function(event, success) {
    this.dragging = false;
    
    if(this.options.quiet){
      Position.prepare();
      var pointer = [Event.pointerX(event), Event.pointerY(event)];
      Droppables.show(pointer, this.element);
    }

    if(this.options.ghosting) {
      if (!this.element._originallyAbsolute)
        Position.relativize(this.element);
      delete this.element._originallyAbsolute;
      Element.remove(this._clone);
      this._clone = null;
    }

    var dropped = false; 
    if(success) { 
      dropped = Droppables.fire(event, this.element); 
      if (!dropped) dropped = false; 
    }
    if(dropped && this.options.onDropped) this.options.onDropped(this.element);
    Draggables.notify('onEnd', this, event);

    var revert = this.options.revert;
    if(revert && Object.isFunction(revert)) revert = revert(this.element);
    
    var d = this.currentDelta();
    if(revert && this.options.reverteffect) {
      if (dropped == 0 || revert != 'failure')
        this.options.reverteffect(this.element,
          d[1]-this.delta[1], d[0]-this.delta[0]);
    } else {
      this.delta = d;
    }

    if(this.options.zindex)
      this.element.style.zIndex = this.originalZ;

    if(this.options.endeffect) 
      this.options.endeffect(this.element);
      
    Draggables.deactivate(this);
    Droppables.reset();
  },
  
  keyPress: function(event) {
    if(event.keyCode!=Event.KEY_ESC) return;
    this.finishDrag(event, false);
    Event.stop(event);
  },
  
  endDrag: function(event) {
    if(!this.dragging) return;
    this.stopScrolling();
    this.finishDrag(event, true);
    Event.stop(event);
  },
  
  draw: function(point) {
    var pos = Position.cumulativeOffset(this.element);
    if(this.options.ghosting) {
      var r   = Position.realOffset(this.element);
      pos[0] += r[0] - Position.deltaX; pos[1] += r[1] - Position.deltaY;
    }
    
    var d = this.currentDelta();
    pos[0] -= d[0]; pos[1] -= d[1];
    
    if(this.options.scroll && (this.options.scroll != window && this._isScrollChild)) {
      pos[0] -= this.options.scroll.scrollLeft-this.originalScrollLeft;
      pos[1] -= this.options.scroll.scrollTop-this.originalScrollTop;
    }
    
    var p = [0,1].map(function(i){ 
      return (point[i]-pos[i]-this.offset[i]) 
    }.bind(this));
    
    if(this.options.snap) {
      if(Object.isFunction(this.options.snap)) {
        p = this.options.snap(p[0],p[1],this);
      } else {
      if(Object.isArray(this.options.snap)) {
        p = p.map( function(v, i) {
          return (v/this.options.snap[i]).round()*this.options.snap[i] }.bind(this))
      } else {
        p = p.map( function(v) {
          return (v/this.options.snap).round()*this.options.snap }.bind(this))
      }
    }}
    
    var style = this.element.style;
    if((!this.options.constraint) || (this.options.constraint=='horizontal'))
      style.left = p[0] + "px";
    if((!this.options.constraint) || (this.options.constraint=='vertical'))
      style.top  = p[1] + "px";
    
    if(style.visibility=="hidden") style.visibility = ""; // fix gecko rendering
  },
  
  stopScrolling: function() {
    if(this.scrollInterval) {
      clearInterval(this.scrollInterval);
      this.scrollInterval = null;
      Draggables._lastScrollPointer = null;
    }
  },
  
  startScrolling: function(speed) {
    if(!(speed[0] || speed[1])) return;
    this.scrollSpeed = [speed[0]*this.options.scrollSpeed,speed[1]*this.options.scrollSpeed];
    this.lastScrolled = new Date();
    this.scrollInterval = setInterval(this.scroll.bind(this), 10);
  },
  
  scroll: function() {
    var current = new Date();
    var delta = current - this.lastScrolled;
    this.lastScrolled = current;
    if(this.options.scroll == window) {
      with (this._getWindowScroll(this.options.scroll)) {
        if (this.scrollSpeed[0] || this.scrollSpeed[1]) {
          var d = delta / 1000;
          this.options.scroll.scrollTo( left + d*this.scrollSpeed[0], top + d*this.scrollSpeed[1] );
        }
      }
    } else {
      this.options.scroll.scrollLeft += this.scrollSpeed[0] * delta / 1000;
      this.options.scroll.scrollTop  += this.scrollSpeed[1] * delta / 1000;
    }
    
    Position.prepare();
    Droppables.show(Draggables._lastPointer, this.element);
    Draggables.notify('onDrag', this);
    if (this._isScrollChild) {
      Draggables._lastScrollPointer = Draggables._lastScrollPointer || $A(Draggables._lastPointer);
      Draggables._lastScrollPointer[0] += this.scrollSpeed[0] * delta / 1000;
      Draggables._lastScrollPointer[1] += this.scrollSpeed[1] * delta / 1000;
      if (Draggables._lastScrollPointer[0] < 0)
        Draggables._lastScrollPointer[0] = 0;
      if (Draggables._lastScrollPointer[1] < 0)
        Draggables._lastScrollPointer[1] = 0;
      this.draw(Draggables._lastScrollPointer);
    }
    
    if(this.options.change) this.options.change(this);
  },
  
  _getWindowScroll: function(w) {
    var T, L, W, H;
    with (w.document) {
      if (w.document.documentElement && documentElement.scrollTop) {
        T = documentElement.scrollTop;
        L = documentElement.scrollLeft;
      } else if (w.document.body) {
        T = body.scrollTop;
        L = body.scrollLeft;
      }
      if (w.innerWidth) {
        W = w.innerWidth;
        H = w.innerHeight;
      } else if (w.document.documentElement && documentElement.clientWidth) {
        W = documentElement.clientWidth;
        H = documentElement.clientHeight;
      } else {
        W = body.offsetWidth;
        H = body.offsetHeight
      }
    }
    return { top: T, left: L, width: W, height: H };
  }
});

Draggable._dragging = { };

/*--------------------------------------------------------------------------*/

var SortableObserver = Class.create({
  initialize: function(element, observer) {
    this.element   = $(element);
    this.observer  = observer;
    this.lastValue = Sortable.serialize(this.element);
  },
  
  onStart: function() {
    this.lastValue = Sortable.serialize(this.element);
  },
  
  onEnd: function() {
    Sortable.unmark();
    if(this.lastValue != Sortable.serialize(this.element))
      this.observer(this.element)
  }
});

var Sortable = {
  SERIALIZE_RULE: /^[^_\-](?:[A-Za-z0-9\-\_]*)[_](.*)$/,
  
  sortables: { },
  
  _findRootElement: function(element) {
    while (element.tagName.toUpperCase() != "BODY") {  
      if(element.id && Sortable.sortables[element.id]) return element;
      element = element.parentNode;
    }
  },

  options: function(element) {
    element = Sortable._findRootElement($(element));
    if(!element) return;
    return Sortable.sortables[element.id];
  },
  
  destroy: function(element){
    var s = Sortable.options(element);
    
    if(s) {
      Draggables.removeObserver(s.element);
      s.droppables.each(function(d){ Droppables.remove(d) });
      s.draggables.invoke('destroy');
      
      delete Sortable.sortables[s.element.id];
    }
  },

  create: function(element) {
    element = $(element);
    var options = Object.extend({ 
      element:     element,
      tag:         'li',       // assumes li children, override with tag: 'tagname'
      dropOnEmpty: false,
      tree:        false,
      treeTag:     'ul',
      overlap:     'vertical', // one of 'vertical', 'horizontal'
      constraint:  'vertical', // one of 'vertical', 'horizontal', false
      containment: element,    // also takes array of elements (or id's); or false
      handle:      false,      // or a CSS class
      only:        false,
      delay:       0,
      hoverclass:  null,
      ghosting:    false,
      quiet:       false, 
      scroll:      false,
      scrollSensitivity: 20,
      scrollSpeed: 15,
      format:      this.SERIALIZE_RULE,
      
      // these take arrays of elements or ids and can be 
      // used for better initialization performance
      elements:    false,
      handles:     false,
      
      onChange:    Prototype.emptyFunction,
      onUpdate:    Prototype.emptyFunction
    }, arguments[1] || { });

    // clear any old sortable with same element
    this.destroy(element);

    // build options for the draggables
    var options_for_draggable = {
      revert:      true,
      quiet:       options.quiet,
      scroll:      options.scroll,
      scrollSpeed: options.scrollSpeed,
      scrollSensitivity: options.scrollSensitivity,
      delay:       options.delay,
      ghosting:    options.ghosting,
      constraint:  options.constraint,
      handle:      options.handle };

    if(options.starteffect)
      options_for_draggable.starteffect = options.starteffect;

    if(options.reverteffect)
      options_for_draggable.reverteffect = options.reverteffect;
    else
      if(options.ghosting) options_for_draggable.reverteffect = function(element) {
        element.style.top  = 0;
        element.style.left = 0;
      };

    if(options.endeffect)
      options_for_draggable.endeffect = options.endeffect;

    if(options.zindex)
      options_for_draggable.zindex = options.zindex;

    // build options for the droppables  
    var options_for_droppable = {
      overlap:     options.overlap,
      containment: options.containment,
      tree:        options.tree,
      hoverclass:  options.hoverclass,
      onHover:     Sortable.onHover
    }
    
    var options_for_tree = {
      onHover:      Sortable.onEmptyHover,
      overlap:      options.overlap,
      containment:  options.containment,
      hoverclass:   options.hoverclass
    }

    // fix for gecko engine
    Element.cleanWhitespace(element); 

    options.draggables = [];
    options.droppables = [];

    // drop on empty handling
    if(options.dropOnEmpty || options.tree) {
      Droppables.add(element, options_for_tree);
      options.droppables.push(element);
    }

    (options.elements || this.findElements(element, options) || []).each( function(e,i) {
      var handle = options.handles ? $(options.handles[i]) :
        (options.handle ? $(e).select('.' + options.handle)[0] : e); 
      options.draggables.push(
        new Draggable(e, Object.extend(options_for_draggable, { handle: handle })));
      Droppables.add(e, options_for_droppable);
      if(options.tree) e.treeNode = element;
      options.droppables.push(e);      
    });
    
    if(options.tree) {
      (Sortable.findTreeElements(element, options) || []).each( function(e) {
        Droppables.add(e, options_for_tree);
        e.treeNode = element;
        options.droppables.push(e);
      });
    }

    // keep reference
    this.sortables[element.id] = options;

    // for onupdate
    Draggables.addObserver(new SortableObserver(element, options.onUpdate));

  },

  // return all suitable-for-sortable elements in a guaranteed order
  findElements: function(element, options) {
    return Element.findChildren(
      element, options.only, options.tree ? true : false, options.tag);
  },
  
  findTreeElements: function(element, options) {
    return Element.findChildren(
      element, options.only, options.tree ? true : false, options.treeTag);
  },

  onHover: function(element, dropon, overlap) {
    if(Element.isParent(dropon, element)) return;

    if(overlap > .33 && overlap < .66 && Sortable.options(dropon).tree) {
      return;
    } else if(overlap>0.5) {
      Sortable.mark(dropon, 'before');
      if(dropon.previousSibling != element) {
        var oldParentNode = element.parentNode;
        element.style.visibility = "hidden"; // fix gecko rendering
        dropon.parentNode.insertBefore(element, dropon);
        if(dropon.parentNode!=oldParentNode) 
          Sortable.options(oldParentNode).onChange(element);
        Sortable.options(dropon.parentNode).onChange(element);
      }
    } else {
      Sortable.mark(dropon, 'after');
      var nextElement = dropon.nextSibling || null;
      if(nextElement != element) {
        var oldParentNode = element.parentNode;
        element.style.visibility = "hidden"; // fix gecko rendering
        dropon.parentNode.insertBefore(element, nextElement);
        if(dropon.parentNode!=oldParentNode) 
          Sortable.options(oldParentNode).onChange(element);
        Sortable.options(dropon.parentNode).onChange(element);
      }
    }
  },
  
  onEmptyHover: function(element, dropon, overlap) {
    var oldParentNode = element.parentNode;
    var droponOptions = Sortable.options(dropon);
        
    if(!Element.isParent(dropon, element)) {
      var index;
      
      var children = Sortable.findElements(dropon, {tag: droponOptions.tag, only: droponOptions.only});
      var child = null;
            
      if(children) {
        var offset = Element.offsetSize(dropon, droponOptions.overlap) * (1.0 - overlap);
        
        for (index = 0; index < children.length; index += 1) {
          if (offset - Element.offsetSize (children[index], droponOptions.overlap) >= 0) {
            offset -= Element.offsetSize (children[index], droponOptions.overlap);
          } else if (offset - (Element.offsetSize (children[index], droponOptions.overlap) / 2) >= 0) {
            child = index + 1 < children.length ? children[index + 1] : null;
            break;
          } else {
            child = children[index];
            break;
          }
        }
      }
      
      dropon.insertBefore(element, child);
      
      Sortable.options(oldParentNode).onChange(element);
      droponOptions.onChange(element);
    }
  },

  unmark: function() {
    if(Sortable._marker) Sortable._marker.hide();
  },

  mark: function(dropon, position) {
    // mark on ghosting only
    var sortable = Sortable.options(dropon.parentNode);
    if(sortable && !sortable.ghosting) return; 

    if(!Sortable._marker) {
      Sortable._marker = 
        ($('dropmarker') || Element.extend(document.createElement('DIV'))).
          hide().addClassName('dropmarker').setStyle({position:'absolute'});
      document.getElementsByTagName("body").item(0).appendChild(Sortable._marker);
    }    
    var offsets = Position.cumulativeOffset(dropon);
    Sortable._marker.setStyle({left: offsets[0]+'px', top: offsets[1] + 'px'});
    
    if(position=='after')
      if(sortable.overlap == 'horizontal') 
        Sortable._marker.setStyle({left: (offsets[0]+dropon.clientWidth) + 'px'});
      else
        Sortable._marker.setStyle({top: (offsets[1]+dropon.clientHeight) + 'px'});
    
    Sortable._marker.show();
  },
  
  _tree: function(element, options, parent) {
    var children = Sortable.findElements(element, options) || [];
  
    for (var i = 0; i < children.length; ++i) {
      var match = children[i].id.match(options.format);

      if (!match) continue;
      
      var child = {
        id: encodeURIComponent(match ? match[1] : null),
        element: element,
        parent: parent,
        children: [],
        position: parent.children.length,
        container: $(children[i]).down(options.treeTag)
      }
      
      /* Get the element containing the children and recurse over it */
      if (child.container)
        this._tree(child.container, options, child)
      
      parent.children.push (child);
    }

    return parent; 
  },

  tree: function(element) {
    element = $(element);
    var sortableOptions = this.options(element);
    var options = Object.extend({
      tag: sortableOptions.tag,
      treeTag: sortableOptions.treeTag,
      only: sortableOptions.only,
      name: element.id,
      format: sortableOptions.format
    }, arguments[1] || { });
    
    var root = {
      id: null,
      parent: null,
      children: [],
      container: element,
      position: 0
    }
    
    return Sortable._tree(element, options, root);
  },

  /* Construct a [i] index for a particular node */
  _constructIndex: function(node) {
    var index = '';
    do {
      if (node.id) index = '[' + node.position + ']' + index;
    } while ((node = node.parent) != null);
    return index;
  },

  sequence: function(element) {
    element = $(element);
    var options = Object.extend(this.options(element), arguments[1] || { });
    
    return $(this.findElements(element, options) || []).map( function(item) {
      return item.id.match(options.format) ? item.id.match(options.format)[1] : '';
    });
  },

  setSequence: function(element, new_sequence) {
    element = $(element);
    var options = Object.extend(this.options(element), arguments[2] || { });
    
    var nodeMap = { };
    this.findElements(element, options).each( function(n) {
        if (n.id.match(options.format))
            nodeMap[n.id.match(options.format)[1]] = [n, n.parentNode];
        n.parentNode.removeChild(n);
    });
   
    new_sequence.each(function(ident) {
      var n = nodeMap[ident];
      if (n) {
        n[1].appendChild(n[0]);
        delete nodeMap[ident];
      }
    });
  },
  
  serialize: function(element) {
    element = $(element);
    var options = Object.extend(Sortable.options(element), arguments[1] || { });
    var name = encodeURIComponent(
      (arguments[1] && arguments[1].name) ? arguments[1].name : element.id);
    
    if (options.tree) {
      return Sortable.tree(element, arguments[1]).children.map( function (item) {
        return [name + Sortable._constructIndex(item) + "[id]=" + 
                encodeURIComponent(item.id)].concat(item.children.map(arguments.callee));
      }).flatten().join('&');
    } else {
      return Sortable.sequence(element, arguments[1]).map( function(item) {
        return name + "[]=" + encodeURIComponent(item);
      }).join('&');
    }
  }
}

// Returns true if child is contained within element
Element.isParent = function(child, element) {
  if (!child.parentNode || child == element) return false;
  if (child.parentNode == element) return true;
  return Element.isParent(child.parentNode, element);
}

Element.findChildren = function(element, only, recursive, tagName) {   
  if(!element.hasChildNodes()) return null;
  tagName = tagName.toUpperCase();
  if(only) only = [only].flatten();
  var elements = [];
  $A(element.childNodes).each( function(e) {
    if(e.tagName && e.tagName.toUpperCase()==tagName &&
      (!only || (Element.classNames(e).detect(function(v) { return only.include(v) }))))
        elements.push(e);
    if(recursive) {
      var grandchildren = Element.findChildren(e, only, recursive, tagName);
      if(grandchildren) elements.push(grandchildren);
    }
  });

  return (elements.length>0 ? elements.flatten() : []);
}

Element.offsetSize = function (element, type) {
  return element['offset' + ((type=='vertical' || type=='height') ? 'Height' : 'Width')];
}
// Copyright (c) 2005 Thomas Fakes (http://craz8.com)
// 
// This code is substantially based on code from script.aculo.us which has the 
// following copyright and permission notice
//
// Copyright (c) 2005 Thomas Fuchs (http://script.aculo.us, http://mir.aculo.us)
// 
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

var Resizeable = Class.create();
Resizeable.prototype = {
  initialize: function(element) {
    var options = Object.extend({
      top: 6,
      bottom: 6,
      left: 6,
      right: 6,
      minHeight: 0,
      minWidth: 0,
      zindex: 1000,
      resize: null
    }, arguments[1] || {});

    this.element      = $(element);
    this.handle 	  = this.element;

    Element.makePositioned(this.element); // fix IE    

    this.options      = options;

    this.active       = false;
    this.resizing     = false;   
    this.currentDirection = '';

    this.eventMouseDown = this.startResize.bindAsEventListener(this);
    this.eventMouseUp   = this.endResize.bindAsEventListener(this);
    this.eventMouseMove = this.update.bindAsEventListener(this);
    this.eventCursorCheck = this.cursor.bindAsEventListener(this);
    this.eventKeypress  = this.keyPress.bindAsEventListener(this);
    
    this.registerEvents();
  },
  destroy: function() {
    Event.stopObserving(this.handle, "mousedown", this.eventMouseDown);
    this.unregisterEvents();
  },
  registerEvents: function() {
    Event.observe(document, "mouseup", this.eventMouseUp);
    Event.observe(document, "mousemove", this.eventMouseMove);
    Event.observe(document, "keypress", this.eventKeypress);
    Event.observe(this.handle, "mousedown", this.eventMouseDown);
    Event.observe(this.element, "mousemove", this.eventCursorCheck);
  },
  unregisterEvents: function() {
    //if(!this.active) return;
    //Event.stopObserving(document, "mouseup", this.eventMouseUp);
    //Event.stopObserving(document, "mousemove", this.eventMouseMove);
    //Event.stopObserving(document, "mousemove", this.eventCursorCheck);
    //Event.stopObserving(document, "keypress", this.eventKeypress);
  },
  startResize: function(event) {
    if(Event.isLeftClick(event)) {
      
      // abort on form elements, fixes a Firefox issue
      var src = Event.element(event);
      if(src.tagName && (
        src.tagName=='INPUT' ||
        src.tagName=='SELECT' ||
        src.tagName=='BUTTON' ||
        src.tagName=='TEXTAREA')) return;

	  var dir = this.directions(event);
	  if (dir.length > 0) {      
	      this.active = true;
    	  var offsets = Position.cumulativeOffset(this.element);
	      this.startTop = offsets[1];
	      this.startLeft = offsets[0];
	      this.startWidth = parseInt(Element.getStyle(this.element, 'width'));
	      this.startHeight = parseInt(Element.getStyle(this.element, 'height'));
	      this.startX = event.clientX + document.body.scrollLeft + document.documentElement.scrollLeft;
	      this.startY = event.clientY + document.body.scrollTop + document.documentElement.scrollTop;
	      
	      this.currentDirection = dir;
	      Event.stop(event);
	  }
    }
  },
  finishResize: function(event, success) {
    // this.unregisterEvents();

    this.active = false;
    this.resizing = false;

    if(this.options.zindex)
      this.element.style.zIndex = this.originalZ;
      
    if (this.options.resize) {
    	this.options.resize(this.element);
    }
  },
  keyPress: function(event) {
    if(this.active) {
      if(event.keyCode==Event.KEY_ESC) {
        this.finishResize(event, false);
        Event.stop(event);
      }
    }
  },
  endResize: function(event) {
    if(this.active && this.resizing) {
      this.finishResize(event, true);
      Event.stop(event);
    }
    this.active = false;
    this.resizing = false;
  },
  draw: function(event) {
    var pointer = [Event.pointerX(event), Event.pointerY(event)];
    var style = this.element.style;
    if (this.currentDirection.indexOf('n') != -1) {
    	var pointerMoved = this.startY - pointer[1];
    	var margin = Element.getStyle(this.element, 'margin-top') || "0";
    	var newHeight = this.startHeight + pointerMoved;
    	if (newHeight > this.options.minHeight) {
    		style.height = newHeight + "px";
    		style.top = (this.startTop - pointerMoved - parseInt(margin)) + "px";
    	}
    }
    if (this.currentDirection.indexOf('w') != -1) {
    	var pointerMoved = this.startX - pointer[0];
    	var margin = Element.getStyle(this.element, 'margin-left') || "0";
    	var newWidth = this.startWidth + pointerMoved;
    	if (newWidth > this.options.minWidth) {
    		style.left = (this.startLeft - pointerMoved - parseInt(margin))  + "px";
    		style.width = newWidth + "px";
    	}
    }
    if (this.currentDirection.indexOf('s') != -1) {
    	var newHeight = this.startHeight + pointer[1] - this.startY;
    	if (newHeight > this.options.minHeight) {
    		style.height = newHeight + "px";
    	}
    }
    if (this.currentDirection.indexOf('e') != -1) {
    	var newWidth = this.startWidth + pointer[0] - this.startX;
    	if (newWidth > this.options.minWidth) {
    		style.width = newWidth + "px";
    	}
    }
    if(style.visibility=="hidden") style.visibility = ""; // fix gecko rendering
  },
  between: function(val, low, high) {
  	return (val >= low && val < high);
  },
  directions: function(event) {
    var pointer = [Event.pointerX(event), Event.pointerY(event)];
    var offsets = Position.cumulativeOffset(this.element);
    
	var cursor = '';
	if (this.between(pointer[1] - offsets[1], 0, this.options.top)) cursor += 'n';
	if (this.between((offsets[1] + this.element.offsetHeight) - pointer[1], 0, this.options.bottom)) cursor += 's';
	if (this.between(pointer[0] - offsets[0], 0, this.options.left)) cursor += 'w';
	if (this.between((offsets[0] + this.element.offsetWidth) - pointer[0], 0, this.options.right)) cursor += 'e';

	return cursor;
  },
  cursor: function(event) {
  	var cursor = this.directions(event);
	if (cursor.length > 0) {
		cursor += '-resize';
	} else {
		cursor = '';
	}
	this.element.style.cursor = cursor;		
  },
  update: function(event) {
   if(this.active) {
      if(!this.resizing) {
        var style = this.element.style;
        this.resizing = true;
        
        if(Element.getStyle(this.element,'position')=='') 
          style.position = "relative";
        
        if(this.options.zindex) {
          this.originalZ = parseInt(Element.getStyle(this.element,'z-index') || 0);
          style.zIndex = this.options.zindex;
        }
      }
      this.draw(event);

      // fix AppleWebKit rendering
      if(navigator.appVersion.indexOf('AppleWebKit')>0) window.scrollBy(0,0); 
      Event.stop(event);
      return false;
   }
  }
}/*

Copyright (c) 2007 Travis Hensgen, http://mondea.com.au

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

*/

var Jel = {
    version:    1.0
};

/*

Class: Jel.Lang
    *Language Resources* for the Jel library which allow it to be internationalised.
    
*/

Jel.Lang = 
{
    KEY:                            "en.us", 
    DATE_FORMAT:                    "m/d/Y",
    DATE_12_FORMAT:                 "m/d/Y g:i A"
};

Jel.Lang.String = 
{

};


/*

Class: Jel.Lang.Date
    Language resources for <Jel.Date>
    
*/


/* 
    Property: DAYS 
        an *array* of *long day name* string constants for the current language build. e.g. for English ["Sunday", ..., "Saturday"]. These are also used by <Jel.Date.format> and <Jel.Date.parse>.
    
    Example: 
        > Jel.Lang.Date.DAYS[4]; // Thursday
        > Jel.Lang.Date.DAYS[1]; // Monday
*/

/* 
    Property: DAYS_SHORT
        an *array* of *short day name* string constants for the current language build. e.g. for English ["Sun", ..., "Sat"]. These are also used by <Jel.Date.format> and <Jel.Date.parse>.
    
    Example: 
        > Jel.Lang.Date.DAYS_SHORT[4]; // Thu
        > Jel.Lang.Date.DAYS_SHORT[1]; // Mon
*/

/* 
    Property: MONTHS
        an *array* of *long month names* string constants for the current language build. e.g. for English ["January", ..., "December"]. These are also used by <Jel.Date.format> and <Jel.Date.parse>.
    
    Example: 
        > Jel.Lang.Date.MONTHS[4]; // May
        > Jel.Lang.Date.MONTHS[1]; // Februrary
*/

/* 
    Property: MONTHS_SHORT
        an array of *short month names* string constants for the current language build. e.g. for English ["Jan", ..., "Dec"]. These are also used by <Jel.Date.format> and <Jel.Date.parse>.

    Example: 
        > Jel.Lang.Date.MONTHS_SHORT[4]; // May
        > Jel.Lang.Date.MONTHS_SHORT[1]; // Feb
*/



Jel.Lang.Date = 
{
    DAYS: ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"],
    DAYS_SHORT: ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"],
    MONTHS: ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"],
    MONTHS_SHORT: ["Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"],
    MONTHS_LCASE: ["january", "february", "march", "april", "may", "june", "july", "august", "september", "october", "november", "december"],
    MONTHS_SHORT_LCASE: ["jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"]
};
  
/*
Class: Jel.String
    *Utility Methods* for manipulating JavaScript's *built-in String class*
*/

Jel.String = 
{
    /*
        Method: repeat
            gets the *repeat* of a string for a specified *count*

        Arguments:
            str - string, the string to repeat
            count - integer, the delimiter to use between words. If not specified, a dash ("-") is used
    
        Returns: 
            string
        
        Examples:
            > String.repeat("-", 5);     // "-----"
            > String.repeat("hello", 3); // "hellohellohello"
    */

    repeat: function(str, count)
    {
    	var ret = '';

    	for (var i=0; i<count; i++)
    		ret += str;
	
    	return ret;
    },
    
    wrapToLines: function(str, lineLength)
    {
        var lines = [];
        var line = '';
        var words = str.split(" ");
        var appended;
        var word;
        
        for (var i=0; i<words.length; i++)
        {
            word = words[i];
            appended = line + ( line.length == '' ? '' : ' ' ) + word;
            
            
            if ( appended.length > lineLength )
            {
                lines[lines.length] = appended;
                line = '';
            }
            else
            {
                line = appended;
            }
        }
        
        if (line != '')
            lines[lines.length] = appended;
        
        return lines;
    },

    /*
        Method: right
            gets the *specified number of characters from the end* of a given string
            
        Arguments:
            str - String, a given string 
            length - the number of characters to return
        
        Example:
            > Jel.String.right("wicked", 3); // "ked"
    */
            
    right: function(str, length)
    {
	    return str.substring(str.length - length, str.length);
	},

    /*
        Method: left
            gets the *specified number of characters from the beginning* of a given string
            
        Arguments:
            str - String, a given string 
            length - the number of characters to return
        
        Example:
            > Jel.String.left("wicked", 4); // "wick"
    */
	
	left: function(str, length)
    {
    	return str.substring(0, length);
    },
    
    /*
        Method: ltrim
            *removes whitespace* characters *from the beginning* of a given string

        Arguments:
            str - String, a given string 
        
        Examples:
            > Jel.String.lrim("  wicked");      // "wicked"
            > Jel.String.lrim("  wicked  ");    // "wicked  "
    */
	
    ltrim: function(str)
    {
    	return str.replace(/^\s+/,'');
    },

    /*
        Method: rtrim
            *removes whitespace* characters *from the end* of a given string

        Arguments:
            str - String, a given string 
        
        Examples:
            > Jel.String.rtrim("wicked");       // "wicked"
            > Jel.String.rtrim("  wicked  ");   // "  wicked"
    */
    
    rtrim: function(str)
    {
    	return str.replace(/\s+$/,'');
    },

    /*
        Method: trim
            *removes whitespace* characters *from the beginning and end* of a given string

        Arguments:
            str - String, a given string 
        
        Example:
            > Jel.String.trim("   wicked  "); // "wicked"
    */
    
    trim: function(str) 
    {
    	return Jel.String.ltrim(Jel.String.rtrim(str));
    },
    
    /*
        Method: toFloat
            *gets the float value* of a given string in manner *safe for arithmetic expressions*

        Arguments:
            str - String, a given string 
        
        Returns: 
            float - the float value of the string, if it can be converted into one 
            0.0 - if the string does not represent a float

        Examples:
            > Jel.String.toFloat("0.5");                                  // 0.5
            > Jel.String.toFloat("word");                                 // 0.0
            > Jel.String.toFloat("0.6") + Jel.String.toFloat("word");     // 0.6

    */
    
    toFloat: function(str)
    {
    	if (isNaN(str))
    		return 0.0;
    	else
    		if (Jel.String.trim(str) == '')
    			return 0.0;
    		else
    			return parseFloat(str);
    },

    /*
        Method: toInt
            *gets the integer value* of a given string in a manner that is *safe for arithmetic expressions*

        Arguments:
            str - String, a given string 
        
        Returns: 
            integer - the integer value of the string, if it can be converted into one 
            0 - if the string does not represent an integer

        Examples:
            > Jel.String.toInt("4");          // 4
            > Jel.String.toInt("word");       // 0
            > Jel.String.toInt(8 + "word");   // 8

    */
    
    toInt: function(str)
    {
    	if (isNaN(str))
    		return 0;
    	else
    	{
    		if (Jel.String.trim(str) == '')
    			return 0;
    		else
    			return parseInt(str);
    	}
    },

    /*
        Method: extractInt
            *extracts the digits in a given string*, returning the *integer value* of them *joined together in sequence*

        Arguments:
            str - String, a given string 
        
        Returns: 
            integer - the integer value of the all of the digits joined together in a given string, if any are present 
            0 - otherwise                           

        Examples:
            > Jel.String.extractInt("4jkbn45");   // 445
            > Jel.String.extractInt("word");      // 0
            > Jel.String.extractInt("ff88f999");  // 88999

    */
    
    extractInt: function(str)
    {
        var ret = "";

        var matches = str.match(/[0-9]+/ig);
    
        if (matches)
        {
            for (var i=0; i<matches.length; i++)
            {
                var part = "";
                
                for (var j=0; j<matches[i].length; j++)
                {
                    if (!(part.length == 0 && matches[i][j] == '0'))
                    {
                        // ignore leading zeros
                        part += matches[i][j];
                    }
                }
                
                ret += part;
            }
        }

        return ret == "" ? 0 : parseInt(ret);
    },

    /*
        Method: eq
            checks if a given string *is equal to another string*, with the *optional case-insensitive comparison*
        
        Arguments:
            str - String, a given string 
            strCompare - string, the string to compare to
            ignoreCase - boolean, whether the comparison is case sensitive or not
        
        Returns: 
            true - if they are equal 
            false - otherwise                           

        Examples:
            > Jel.String.equals("clay", "blah");        // false
            > Jel.String.equals("word", "WORD", true); // true
    */
    
    equals: function(str, strCompare, ignoreCase)
    {
        if (!ignoreCase)
            return str == strCompare;
        else
            return str.toLowerCase() == strCompare.toLowerCase();
    },
    
    /*
        Method: startsWith
            checks if a given string *begins with another string*, with the *optional case-insensitive comparison*

        Arguments:
            str - String, a given string 
            strCompare - string, the string to compare to
            ignoreCase - boolean, whether the comparison is case sensitive or not
        
        Returns: 
            true - if a given string begins with *str* 
            false - otherwise                    

        Examples:
            > Jel.String.startsWith("word", "w");                       // true
            > Jel.String.startsWith("field-container", "FIELD", false); // true
            > Jel.String.startsWith("word", "p");                       // false
    */

    startsWith: function(str, strCompare, ignoreCase)
    {   
        return Jel.String.equals(Jel.String.left(str, strCompare.length), strCompare, ignoreCase);
    },
 
    /*
        Method: endsWith
            checks if a given string *ends with another string*, with the option of a case-insensitive comparison

        Arguments:
            str - String, a given string 
            strCompare - string, the string to compare to
            ignoreCase - boolean, whether the comparison is case sensitive or not
        
        Returns: 
            true - if a given string ends with *str* 
            false - otherwise                    

        Examples:
            > Jel.String.endsWith("word", "w");                           // false
            > Jel.String.endsWith("field-container", "CONTAINER", false); // true
    */
    
    endsWith: function(str, strCompare, ignoreCase)
    {
        return Jel.String.equals(Jel.String.right(str, strCompare.length), strCompare, ignoreCase);
    },

    /*
        Method: decamelize
            *breaks a given string up into* a string of *lowercase delimited words*, where an uppercase letter in *a given string* denotes a new word (*camel case*)

        Arguments:
            str - String, the string to decamelize
            delimiter - String, the delimiter to use between words. If not specified, a dash ("-") is used
        
        Returns: 
            string - the final delimited words string
            
        Examples:
            > Jel.String.decamelize("fieldValidator");   // "field-validator"
            > Jel.String.decamelize("helloThere", " ");  // "hello there"
            > Jel.String.decamelize("getValue", "_");    // "get_value"
    */

    decamelize: function(str, delimiter)
    {
        var ret = str.replace(/([A-Z0-9])/g, (delimiter ? delimiter : '-') + '$1').toLowerCase();

        var re = new RegExp('([0-9])' + (delimiter ? delimiter : '-') + '([0-9])', 'g');

        while (ret.match(re))
        {
            ret = ret.replace(re, "$1$2");
        }

        return ret;    
    },

    /*
        Method: camelize
            converts a string of dash-delimited words into a camelized version

        Arguments:
            delimiter - the delimiter to use between words. If not specified, a dash ("-") is used
            
        Returns: 
            string - the final camel cased words string
        
        Credit:
            code taken from Prototype.js 1.5 (c) 2005-2007 Sam Stephenson
            
        Example:
            > Jel.String.camelize("field-validator"); // "fieldValidator"
    */
    
    camelize: function(str, delimiter) 
    {
        if (!delimiter)
            delimiter = '-';
            
        var parts = str.split(delimiter), len = parts.length;
        
        if (len == 1) 
            return parts[0];

        var camelized = str.charAt(0) == delimiter ? parts[0].charAt(0).toUpperCase() + parts[0].substring(1) : parts[0];

        for (var i = 1; i < len; i++)
        {
            camelized += parts[i].charAt(0).toUpperCase() + parts[i].substring(1);
        }

        return camelized;
    },

    /*
        Method: titleCase
            converts a string of words into the equivalent title cased word

        Arguments:
            str - String, the string of words to convert
            
        Returns: 
            string - the final title-cased words string
            
        Example:
            > Jel.String.titleCase("field validator"); // "Field Validator"
    */

    titleCase: function(str)
    {
        var words = str.split(" ");
        var titleWords = [];
        
        for (var i=0; i<words.length; i++)
        {
            titleWords[titleWords.length] = words[i].substr(0, 1).toUpperCase() + words[i].substr(1, words[i].length - 1);
        }
        
        return titleWords.join(" ");
    },
    
    /*
        Method: normalize
            converts a string of words which are camel-cased, dash-delimited, underscore delimted, or a combination into a string of dash-delimited words

        Arguments:
            str - String, the string of words to convert
            
        Returns: 
            string - the final dash-delimited words string
            
        Example:
            > Jel.String.normalize("fieldValidator_1"); // "field-validator-1"
    */
    
    normalize: function(str)
    {
        var ret = str;
        
        ret = ret.replace(/\_/g, "-");
        ret = Jel.String.camelize(ret);
        ret = Jel.String.decamelize(ret);
        
        return ret;
    },

    /*
        Method: constant
            converts a string of words which are camel-cased, dash-delimited, underscore delimted, or a combination into a style approprite for constant values, that is, a string of underscore-delimited uppercase words

        Arguments:
            str - String, the string of words to convert
            
        Returns: 
            string - the final uppercase underscore-delimited words string
            
        Example:
            > Jel.String.normalize("fieldValidator_1"); // "FIELD_VALIDATOR_1"
    */
    
    constant: function(str)
    {
        return Jel.String.normalize(str).toUpperCase().replace(/\-/g, "_");
    }
};/*

Class: Jel.Date
    *Utility Methods* for manipulating JavaScript's *built-in Date class*
*/

Jel.Date = {};

Jel.Date.DATE_NOW = "now";

Jel.Date.PARSE_REG_EXP_COMMON =
{
    LEADING_1_12:       "10|11|12|(?:0[1-9])",
    NO_LEADING_1_12:    "10|11|12|(?:[1-9])",
    BOTH_0_59:          "[0-5][0-9]" 
};

    
/* 
    Property: FORMAT 
        a *hash* collection of *common formatting string constants* to be used with <Jel.Date.format> and <Jel.Date.parse>
        
    Examples:
        > Jel.Date.format(Jel.Date.parse("23/02/2006", Jel.Date.FORMAT.UK), Jel.Date.FORMAT.US);
        > // "02/23/2006"
        >
        > Jel.Date.format(Jel.Date.parse("23/02/2006 11:50 PM", Jel.Date.FORMAT.UK_12), Jel.Date.FORMAT.UTC);
        > // "2006-23-02 23:50:00"

    Available constants:
        > T_12:                     'g:i A' 
        > T_24:                     'G:i' 
        > T_MILITARY:               'Gi'
        > UK:                       'd/m/Y'
        > US:                       'm/d/Y'
        > UK_12:                    'd/m/Y g:i A'
        > US_12:                    'm/d/Y g:i A'
        > UK_24:                    'd/m/Y G:i'
        > US_24:                    'm/d/Y G:i'
        > UK_12_SHORT:              'd/m/Y g A'
        > US_12_SHORT:              'm/d/Y g A'
        > UK_24_SHORT:              'd/m/Y G'
        > US_24_SHORT:              'm/d/Y G'
        > UK_12_LONG:               'd/m/Y g:i:s A'
        > US_12_LONG:               'm/d/Y g:i:s A'
        > UK_24_LONG:               'd/m/Y G:i:s'
        > US_24_LONG:               'm/d/Y G:i:s'
        > UTC:                      'Y-m-d G:i:s'
        > UTC_T:                    'Y-m-dTG:i:s'
        > UTC_Y:                    'Y'
        > UTC_YM:                   'Y-m'
        > UTC_YMD:                  'Y-m-d'
        > UTC_YMDHM:                'Y-m-d g:i'
        > UTC_YMDHMS:               'Y-m-d g:i:s'
        > SHORT_MONTH:              'd M Y',
        > SHORT_MONTH_12:           'd M Y g:i A'
        > SHORT_MONTH_24:           'd M Y G:i'
        > SHORT_MONTH_PHRASE:       'jS M Y'
        > SHORT_MONTH_PHRASE_12:    'jS M Y g:i A'
        > SHORT_MONTH_PHRASE_24:    'jS M Y G:i'

    See also:
        <http://www.w3.org/TR/NOTE-datetime> has more information about the UTC (Coordinated Universal Time) standard formats
*/

Jel.Date.FORMAT =
{ 
    T_12:           'g:i A', 
    T_24:           'G:i', 
    T_MILITARY:     'Gi',
    UK:             'd/m/Y',
    US:             'm/d/Y',
    UK_12:          'd/m/Y g:i A',
    US_12:          'm/d/Y g:i A',
    UK_24:          'd/m/Y G:i',
    US_24:          'm/d/Y G:i',
    UK_12_SHORT:    'd/m/Y g A',
    US_12_SHORT:    'm/d/Y g A',
    UK_24_SHORT:    'd/m/Y G',
    US_24_SHORT:    'm/d/Y G',
    UK_12_LONG:     'd/m/Y g:i:s A',
    US_12_LONG:     'm/d/Y g:i:s A',
    UK_24_LONG:     'd/m/Y G:i:s',
    US_24_LONG:     'm/d/Y G:i:s',
    UTC:            'Y-m-d G:i:s',
    UTC_T:          'Y-m-dTG:i:s',
    UTC_Y:          'Y',
    UTC_YM:         'Y-m',
    UTC_YMD:        'Y-m-d',
    UTC_YMDHM:      'Y-m-d g:i',
    UTC_YMDHMS:     'Y-m-d g:i:s',
    SHORT_MONTH:    "d M Y",
    SHORT_MONTH_12: "d M Y g:i A",
    SHORT_MONTH_24: "d M Y G:i",
    SHORT_MONTH_PHRASE: "jS M Y",
    SHORT_MONTH_PHRASE_12: "jS M Y g:i A",
    SHORT_MONTH_PHRASE_24: "jS M Y G:i"
};

/* 
    Property: HUMAN_FORMAT 
        a hash collection of *common date format string constants* as usually *expressed by humans*, with each constant being equivalent to those in <Jel.Date.FORMAT>

    Available constants:
        > T_12:                     'h:mm AM/PM' 
        > T_24:                     'hh:mm (24 hour)'
        > T_MILITARY:               'hmm (military time)' 
        > UK:                       'dd/mm/yyyy'
        > US:                       'mm/dd/yyyy'
        > UK_12:                    'dd/mm/yyyy h:mm AM/PM'
        > US_12:                    'mm/dd/yyyy h:mm AM/PM'
        > UK_24:                    'dd/mm/yyyy hh:mm (24 hour)'
        > US_24:                    'mm/dd/yyyy hh:mm (24 hour)'
        > UK_12_SHORT:              'dd/mm/yyyy h AM/PM'
        > US_12_SHORT:              'mm/dd/yyyy h AM/PM'
        > UK_24_SHORT:              'dd/mm/yyyy h (24 hour)'
        > US_24_SHORT:              'mm/dd/yyyy h (24 hour)'
        > UK_12_LONG:               'dd/mm/yyyy h:mm:ss AM/PM'
        > US_12_LONG:               'mm/dd/yyyy h:mm:ss AM/PM'
        > UK_24_LONG:               'dd/mm/yyyy h:mm:ss (24 hour)'
        > US_24_LONG:               'mm/dd/yyyy h:mm:ss (24 hour)'
        > UTC:                      'yyyy-mm-dd hh:mm:ss (24 hour)'
        > UTC:                      'yyyy-mm-dd hh:mm:ss (24 hour)'
        > UTC_T:                    'yyyy-mm-ssThh:mm:ss (24 hour)'
        > UTC_Y:                    'yyyy'
        > UTC_YM:                   'yyyy-mm'
        > UTC_YMD:                  'yyyy-mm-dd'
        > UTC_YMDHM:                'yyyy-mm-dd hh:mm (24 hour)'
        > UTC_YMDHMS:               'yyyy-mm-dd hh:mm:ss (24 hour)'
        > SHORT_MONTH:              "dd mmm yyyy"
        > SHORT_MONTH_12:           "dd mmm yyyy hh:mm AM/PM"
        > SHORT_MONTH_24:           "dd mmm yyyy hh:mm (24 hour)"
        > SHORT_MONTH_PHRASE:       "d(th) mmm yyyy"
        > SHORT_MONTH_PHRASE_12:    "d(th) mmm yyyy hh:mm AM/PM"
        > SHORT_MONTH_PHRASE_24:    "d(th) mmm yyyy hh:mm (24 hour)"

*/
    
Jel.Date.HUMAN_FORMAT = 
{
    T_12:           'h:mm AM/PM', 
    T_24:           'h:mm (24 hour)', 
    T_MILITARY:     'hmm (military time)',
    UK:             'dd/mm/yyyy',
    US:             'mm/dd/yyyy',
    UK_12:          'dd/mm/yyyy h:mm AM/PM',
    US_12:          'mm/dd/yyyy h:mm AM/PM',
    UK_24:          'dd/mm/yyyy hh:mm (24 hour)',
    US_24:          'mm/dd/yyyy hh:mm (24 hour)',
    UK_12_SHORT:    'dd/mm/yyyy h AM/PM',
    US_12_SHORT:    'mm/dd/yyyy h AM/PM',
    UK_24_SHORT:    'dd/mm/yyyy h (24 hour)',
    US_24_SHORT:    'mm/dd/yyyy h (24 hour)',
    UK_12_LONG:     'dd/mm/yyyy h:mm:ss AM/PM',
    US_12_LONG:     'mm/dd/yyyy h:mm:ss AM/PM',
    UK_24_LONG:     'dd/mm/yyyy h:mm:ss (24 hour)',
    US_24_LONG:     'mm/dd/yyyy h:mm:ss (24 hour)',
    UTC:            'yyyy-mm-dd hh:mm:ss (24 hour)',
    UTC:            'yyyy-mm-dd hh:mm:ss (24 hour)',
    UTC_T:          'yyyy-mm-ssThh:mm:ss (24 hour)',
    UTC_Y:          'yyyy',
    UTC_YM:         'yyyy-mm',
    UTC_YMD:        'yyyy-mm-dd',
    UTC_YMDHM:      'yyyy-mm-dd hh:mm (24 hour)',
    UTC_YMDHMS:     'yyyy-mm-dd hh:mm:ss (24 hour)',
    SHORT_MONTH:    "dd mmm yyyy",
    SHORT_MONTH_12: "dd mmm yyyy hh:mm AM/PM",
    SHORT_MONTH_24: "dd mmm yyyy hh:mm (24 hour)",
    SHORT_MONTH_PHRASE: "d(th) mmm yyyy",
    SHORT_MONTH_PHRASE_12: "d(th) mmm yyyy hh:mm AM/PM",
    SHORT_MONTH_PHRASE_24: "d(th) mmm yyyy hh:mm (24 hour)"
};

Jel.Date.PARSE_REG_EXP = 
{
    d: "30|31|(?:[0-2][0-9])",
    D: Jel.Lang.Date.DAYS_SHORT.join("|"),
    j: "30|31|(?:[12]?[0-9])",
    l: Jel.Lang.Date.DAYS.join("|"),
    N: "[1-7]",
    S: "st|nd|rd|th",
    W: "50|51|52|(?:[1234]?[0-9])",
    F: Jel.Lang.Date.MONTHS.join("|"),
    m: Jel.Date.PARSE_REG_EXP_COMMON.LEADING_1_12,
    M: Jel.Lang.Date.MONTHS_SHORT.join("|"),
    n: Jel.Date.PARSE_REG_EXP_COMMON.NO_LEADING_1_12,
    t: "28|29|30|31",
    Y: "[0-9]{4}",
    y: "[0-9]{2}",
    a: "am|pm",
    A: "AM|PM",
    B: "[0-9]{1,3}",
    g: Jel.Date.PARSE_REG_EXP_COMMON.NO_LEADING_1_12,
    G: "20|21|22|23|(?:[1]?[1-9])",
    h: Jel.Date.PARSE_REG_EXP_COMMON.LEADING_1_12,
    H: "20|21|22|23|(?:[01]?[1-9])",
    i: Jel.Date.PARSE_REG_EXP_COMMON.BOTH_0_59,
    s: Jel.Date.PARSE_REG_EXP_COMMON.BOTH_0_59
};

Jel.Date.PARSE_REG_EXP.c = Jel.Date.PARSE_REG_EXP.Y + "\-" + Jel.Date.PARSE_REG_EXP.m + "\-" + Jel.Date.PARSE_REG_EXP.d + "T" + Jel.Date.PARSE_REG_EXP.H + ":" + Jel.Date.PARSE_REG_EXP.i + ":" + Jel.Date.PARSE_REG_EXP.s;
        
/*
    Method: parse
        *parses a date string* into a JavaScript Date object *assuming a specified date format*.
        
    Arguments:
        str - string, the string to parse.
        format - string, describes the format of the input string.
                Note that this can contain any arbritrary characters except the reserved formatting characters listed below 
                (these characters are based on formatting characters used for in the *PHP date function*)
    
    Example:
        > Jel.Date.parse("23/04/2006", "d/m/Y");
        > // [Date object with day = 23, month = 3, year = 2006]
        >  
        > Jel.Date.parse("23 Feb 2006", "d/m/Y");
        > // false, not in expected format  
    
    Returns:
        Date object - if the date string is in the specified format, and it is a real date in the Gregorian calendar
        false - otherwise
        
    Formatting characters:
    >   d  Day of the month, 2 digits with leading zeros:                       01 - 31
    >   D  A textual representation of a day, three letters:                    Mon - Sun
    >   j  Day of the month without leading zeros:                              1 - 31
    >   l  A full textual representation of the day of the week:                Sunday - Saturday
    >   S  English ordinal suffix for the day of the month, 2 characters:       st, nd, rd or th.
    >   F  A full textual representation of a month, such as January or March:  January - December
    >   m  Numeric representation of a month, with leading zeros:               01 - 12
    >   M  A short textual representation of a month, three letters:            Jan - Dec
    >   n  Numeric representation of a month, without leading zeros:            1 - 12
    >   Y  A full numeric representation of a year, 4 digits: Examples:         1999 or 2003
    >   y  A two digit representation of a year:                                Examples:  99 or 03
    >   a  Lowercase Ante meridiem and Post meridiem:                           am / pm 
    >   A  Uppercase Ante meridiem and Post meridiem:                           AM / PM
    >   g  12-hour format of an hour without leading zeros:                     1 - 12
    >   G  24-hour format of an hour without leading zeros:                     0 - 23
    >   h  12-hour format of an hour with leading zeros:                        01 - 12 
    >   H  24-hour format of an hour with leading zeros:                        00 - 23 
    >   i  Minutes with leading zeros:                                          00 - 59 
    >   s  Seconds, with leading zeros:                                         00 - 59

    See also: 
        <Jel.Date.format>, <Jel.Date.FORMAT>
*/

Jel.Date.parse = function(str, format)
{
    // first task is to check that the date is formatted as specified in format
    // any deviations from this format will result in parse failure
    
    // build the equivalent regular expression pattern from the format string
    
    var matchIndices = new Object(); 
    
    var now = new Date();
    
    var day = now.getDate();
    var month = now.getMonth();
    var year = Jel.Date.fullYear(now);
    
    var hour = 0;
    var minute = 0;
    var second = 0;
    var meridiem = 'AM';
    
    var pattern = '';
    
    for (var i=0; i<format.length; i++)
    {
        var part = format.charAt(i);
        
        var matchKey = '';
        
        // record parts of the string that represent day, month, year, so that we can check combinational validity
        switch (part)
        {
            case 'd':
            {
            }
            case 'j':
            {
                matchKey = 'day';
                break;
            }
            case 'm':
            case 'F':
            case 'M':
            case 'n':
            {
                matchKey = 'month';
                break;
            }
            case 'y':
            case 'Y':
            {
                matchKey = 'year';
                break;
            }
            case 'g':
            case 'G':
            case 'h':
            case 'H':
            {
                matchKey = 'hour';
                break;
            }
            case 'i':
            {
                matchKey = 'minute';
                break;
            }
            case 's':
            {
                
                matchKey = 'second';
                break;
            }
            case 'a':
            case 'A':
            {
                matchKey = 'meridiem';
                break;
            }
            
        }

        //debugger;
        
        if (matchKey)
            matchIndices[matchKey] = { index: i + 1, format: part };
    
        var pre = Jel.Date.PARSE_REG_EXP[part];
    
        if (pre)
            pattern = pattern + "(" + pre + ")";
        else
            pattern = pattern + "(" + part + ")";
    }
    
    var matches = str.match(new RegExp(pattern, "i"));
    
        
    if (matches)
    {
        // only check further for a real date if our format appears correct
        
        if (matchIndices['day'] && matchIndices['month'] && matchIndices['year'])
        {
            // check that the date is valid by extracting all parts
            day = Jel.String.extractInt(matches[matchIndices['day'].index]);
            
            if (matchIndices['month'].format == 'F')
                month = Jel.Lang.Date.MONTHS_LCASE.indexOf(matches[matchIndices['month'].index].toLowerCase());
            else if (matchIndices['month'].format == 'M')
                month = Jel.Lang.Date.MONTHS_SHORT_LCASE.indexOf(matches[matchIndices['month'].index].toLowerCase());
            else
                month = Jel.String.extractInt(matches[matchIndices['month'].index]) - 1;
        
            if (matchIndices['year'].format == 'Y')
                year = Jel.String.extractInt(matches[matchIndices['year'].index]);
            else
            {
                var yearValue = Jel.String.extractInt(matches[matchIndices['year'].index]);
            
                if (yearValue >= 69)
                {
                    year = Jel.String.extractInt("19" + Jel.Number.leadingZero(yearValue));
                }
                else
                {
                    year = Jel.String.extractInt("20" + Jel.Number.leadingZero(yearValue));
                }
            }    

            if (!Jel.Date.validDayMonthYear(day, month, year))
                return false;
        }
        
        // now check for any time components
        
        if (matchIndices['meridiem'])
            meridiem = matches[matchIndices['meridiem'].index].toUpperCase();
        
        if (matchIndices['hour'])
        {
            hour = Jel.String.extractInt(matches[matchIndices['hour'].index]);

            if (matchIndices['hour'].format == 'g' || matchIndices['hour'].format == 'h')
            {
                // consider the meridiem
                
                if (meridiem == 'AM')
                {
                    if (hour == 12)
                        hour = 0;
                }
                else
                {
                    if (hour != 12)
                        hour = hour + 12;
                }
            }   
        }
        
        
        if (matchIndices['minute'])
            minute = Jel.String.extractInt(matches[matchIndices['minute'].index]);

        if (matchIndices['second'])
            second = Jel.String.extractInt(matches[matchIndices['second'].index]);
        
        return new Date(year, month, day, hour, minute, second);
    }

    return false; 
};

/*
    Method: format
        formats the given date as a string using the date format in *format*.
        
    Arguments:
        date   - Date, the date to format
        format - string, describes the format of the output string using the reserved formatting characters listed below. Any other
                characters present will simply appear in the output string in the same place. 
    
    Returns:
        string - containing the formatted date
    
    Example:
        > Jel.Date.format(new Date(2007, 2, 2), "d/m/Y"); // 02/03/2007  
        > Jel.Date.format(new Date(2007, 2, 2), "jS M Y"); // 2nd March 2007  
    
    Formatting characters (based on formatting characters used in the *PHP date function*):

    >   d	Day of the month, 2 digits with leading zeros:                      01 - 31
    >   D	A textual representation of a day, three letters:                   Mon - Sun
    >   j	Day of the month without leading zeros:                             1 - 31
    >   l   A full textual representation of the day of the week:               Sunday - Saturday
    >   N   ISO-8601 numeric representation of the day of the week:             1 (for Monday) - 7 (for Sunday)
    >   S	English ordinal suffix for the day of the month, 2 characters:      st, nd, rd or th.
    >   w	Numeric representation of the day of the week:                      0 (for Sunday) - 6 (for Saturday)
    >   z	The day of the year (starting from 0):                              0 - 365
    >   F	A full textual representation of a month:	                        January - December
    >   m	Numeric representation of a month, with leading zeros:              01 - 12
    >   M	A short textual representation of a month, three letters:           Jan - Dec
    >   n	Numeric representation of a month, without leading zeros:           1 - 12
    >   t	Number of days in the given month:                                  28 - 31
    >   L	Whether it's a leap year:                                           1 (leap year), 0 (otherwise). 
    >   Y	A full numeric representation of a year, 4 digits:                  Examples: 1999 or 2003
    >   y	A two digit representation of a year:                               Examples: 99 or 03
    >   a	Lowercase Ante meridiem and Post meridiem:                          am / pm 
    >   A	Uppercase Ante meridiem and Post meridiem:                          AM / PM
    >   B	Swatch Internet time:                                               000 through 999
    >   g	12-hour format of an hour without leading zeros:                    1 - 12
    >   G	24-hour format of an hour without leading zeros:                    0 - 23
    >   h	12-hour format of an hour with leading zeros:                       01 - 12 
    >   H	24-hour format of an hour with leading zeros:                       00 - 23 
    >   i	Minutes with leading zeros:                                         00 - 59 
    >   s	Seconds, with leading zeros:                                        00 - 59
    >   c	ISO 8601 date:                                                      Example: 2004-02-12T15:19:21+00:00 
    >   r	RFC 2822 formatted date:                                            Example - Thu, 21 Dec 2000 16:01:07 +0200

    See also: <Jel.Date.parse>, <Jel.Date.FORMAT>
*/

Jel.Date.format = function(date, format)
{
	var ret = '';
    
	for (var i=0; i<format.length; i++)
	{	
		var chr = format.charAt(i);

		switch (chr)
		{
			case 'd' :
			{
				ret += Jel.Number.leadingZero(date.getDate());
				break;
			}
			case 'D' :
			{
				ret += Jel.Lang.Date.DAYS_SHORT[date.getDay()];
				break;
			}	
			case 'j' :
			{
				ret += date.getDate();
				break;
			}
			case 'l' :
			{
				ret += Jel.Lang.Date.DAYS[date.getDay()];
				break;
			}
			case 'N' :
			{
				var date = date.getDate();
				ret += (date == 0 ? 7 : date);
				break;
			}	
			case 'S' :
			{
				ret += Jel.Date.ordinalSuffix(date.getDate());
				break;
			}	
			case 'w' :
			{
				ret += date.getDate();
				break;
			}
			case 'z' :
			{
				ret += Jel.Date.dayOfYear(date);
				break;
			}
			case 'F' :
			{
				ret += Jel.Lang.Date.MONTHS[date.getMonth()];
				break;
			}
			case 'm' :
			{
				ret += Jel.Number.leadingZero(date.getMonth() + 1);
				break;
			}
			case 'M' :
			{
				ret += Jel.Lang.Date.MONTHS_SHORT[date.getMonth()];
				break;
			}
			case 'n' :
			{
				ret += date.getMonth() + 1;
				break;
			}
			case 't' :
			{
				ret += Jel.Date.daysInMonth(date.getMonth(), Jel.Date.fullYear(date));
				break;
			}
			case 'L' :
			{
				ret += Jel.Date.isLeapYear(Jel.Date.fullYear(date)) ? 1 : 0;
				break;
			}
			case 'Y' :
			{
				ret += Jel.Date.fullYear(date);
				break;
			}
			case 'y' :
			{
				ret += Jel.Date.fullYear(date).toString().substr(2, 2);
				break;
			}
			case 'a' :
			{
				ret += Jel.Date.meridiem(date.getHours());
				break;
			}
			case 'A' :
			{
				ret += Jel.Date.meridiem(date.getHours()).toUpperCase();
				break;
			}
			case 'B' :
			{
				ret += Jel.Date.internetBeat(date);
				break;
			}
			case 'g' :
			{
				ret += Jel.Date.twelveHour(date.getHours());
				break;
			}
			case 'G' :
			{
				ret += date.getHours();
				break;
			}
			case 'h' :
			{
				ret += Jel.Number.leadingZero(Jel.Date.twelveHour(date.getHours()));
				break;
			}
			case 'H' :
			{
				ret += Jel.Number.leadingZero(date.getHours());
				break;
			}
			case 'i' :
			{
				ret += Jel.Number.leadingZero(date.getMinutes());
				break;
			}
			case 's' :
			{
				ret += Jel.Number.leadingZero(date.getSeconds());
				break;
			}
			case 'c' :
			{
				ret += Jel.Date.format(date, 'Y-m-dTH:i:s');
				break;
			}
			case 'r' :
			{
				ret += date.toString();
				break;
			}

			default: 
				ret += chr;
    	}
    }
    
	return ret;
};

/*
    Method: convert
        converts a *date string from one format to another*. Essentially performs <Jel.Date.parse> on a given string, followed by <Jel.Date.format> on the return date object
        
    Arguments:
        str - string, the original date string to parse
        fromFormat - the expected format of the date in *str* (refer to <Jel.Date.parse> for valid formatter characters)
        toFormat - the desired output format  (refer to <Jel.Date.format> for valid formatter characters)
    
    Returns:
        string - the formatted date, if both the original date string was in the expected format, and the parsed date was a real date
        false - otherwise 
    
    Examples:
        > Jel.Date.convert("28 Feb 2006 2PM", "j M Y gA", Jel.Date.FORMAT.UK_12);        
        > // "28/02/2006 2:00 PM"
        >
        > Jel.Date.convert("30/04/2007 10:00 PM", Jel.Date.FORMAT.US_12, "js M Y, gA");  
        > // "30th April 2007, 10PM"
        >
        > Jel.Date.convert("31/02/2007 10:00 PM", Jel.Date.FORMAT.US_12, "js M Y, gA");  
        > // false, form format correct, but not a real date
        >
        > Jel.Date.convert("31st March 2006", "d/m/Y", "js M Y, gA");                
        > // false, from format is incorrect
*/

Jel.Date.convert = function(str, fromFormat, toFormat)
{
    var date = Jel.Date.parse(str, fromFormat);
    
    if (date)
        return Jel.Date.format(date, toFormat);
        
    return false;
};

 /*
    Method: now
        gets a *Date object* representing the *current date*, with an optional *format* string. 
        
    Arguments:
        format - String (optional), if provided, will return a string of the current date in this format , using <Jel.Date.format>
    
    Returns:
        Date - if format is not specified
        String - otherwise
*/

Jel.Date.now = function(format)
{
    if (format)
    {
        return Jel.Date.format(new Date(), format);
    }
    else
    {
        return new Date();
    }
};

 /*
    Method: daysInMonth
        gets the *number of days* for a *given month* and *year*.

    Arguments:
        month - integer, the month of the year 0-11 (not 1-12, as standard in JavaScript)
        year - integer, the 4 digit year
        
    Returns:
        integer
    
    Examples: 
        > Jel.Date.daysInMonth(2, 2006); // 31
*/

Jel.Date.daysInMonth = function(month, year)
{
    switch (month)
	{
		case 8:
		case 3:
		case 5:
		case 10:
		{
			return 30;
			break;
		}
		case 1:
	    {
			return (Jel.Date.isLeapYear(year) ? 29 : 28);
			break;
		}
		default:
		{
			return 31;
		}
	}
};

 /*
    Method: isLeapYear
        checks *if a given year* is a *leap year*

    Arguments:
        year - integer, the 4 digit year
        
    Returns:
        true - if the year is a leap year
        false - otherwise
    
    Examples: 
        > Jel.Date.isLeapYear(2004); // true
        > Jel.Date.isLeapYear(2006); // false
*/

Jel.Date.isLeapYear = function(year)
{
    return year % 4 == 0;
};

 /*
    Method: validDayMonthYear
        checks *if a given day, month, and year* combination is a *valid date* in the Gregorian calendar

    Arguments:
        month - integer, the month of the year 0-11 (not 1-12, as standard in JavaScript)
        year - integer, the 4 digit year
        
    Returns:
        true - if the combination is a valid day, month, and year combination
        false - otherwise
*/

Jel.Date.validDayMonthYear = function(day, month, year)
{
    if (isNaN(day) || isNaN(month) || isNaN(year))
        throw("day, month, and year must all be integer values");
        
    return day > 0 && day <= Jel.Date.daysInMonth(month, year);
};

 /*
    Method: meridiem
        gets the *ante/post meridiem* (*am* or *pm*) for a *given hour*

    Arguments:
        hour - integer, the hour of the day in 24-hour time (0-23) 
        
    Returns:
        string - "am" or "pm"
    
    Examples:
        > Jel.Date.meridiem(0); // "am"
        > Jel.Date.meridiem(5); // "am"
        > Jel.Date.meridiem(12); // "pm"
        > Jel.Date.meridiem(17); // "pm"
*/

Jel.Date.meridiem = function(hour)
{
	return hour < 12 ? 'am' : 'pm';    
};

/*
    Method: twelveHour
        gets the twelve hour value for the given *hour*
        
    Arguments:
        hour - integer an hour from 0-23 (24 hour time)
    
    Examples:
        >Jel.Date.twelveHour(0); // 12
        >Jel.Date.twelveHour(4); // 4
        >Jel.Date.twelveHour(12); // 12
        >Jel.Date.twelveHour(13); // 1
        >Jel.Date.twelveHour(18); // 6
*/

Jel.Date.twelveHour = function(hour)
{
    if (hour == 0)
        return 12;
    else if (hour > 12)
        return hour - 12;
    else
        return hour;
}; 

/*
    Method: ordinalSuffix
        gets the *English ordinal suffix* for a given day *(st, nd, rd, th)*
    
    Arguments:
        day - Integer

    Returns: 
        string
        
    See also: 
        <Jel.Number.ordinalSuffix>
    
    Example:
        Jel.Date.ordinalSuffix(5) // th
*/

Jel.Date.ordinalSuffix = function(day)
{
	return Jel.Number.ordinalSuffix(day);
};

/*
    Method: fullYear
        gets the *full 4-digit year* for *this* Date object

    Arguments:
        date - Date object

    Returns: integer
    
    Example:
        > Jel.Date.fullYear(new Date(2007, 6, 30));    // 2007

    Credit:
        Thanks to Peter-Paul Koch of http://www.quirksmode.org/ for the basis of this code
*/

Jel.Date.fullYear = function(date) 
{
    var x = date.getYear();

    var y = x % 100;

    y += (y < 38) ? 2000 : 1900;

    return y;
};

/*
    Method: dayOfYear
        gets the *day of the year* for the given date

    Arguments:
        date - Date object

    Returns: integer
        
    Example:
        > Jel.Date.dayOfYear(new Date(2007, 6, 30));    // 211
        
*/

Jel.Date.dayOfYear = function(date)
{
    var month = date.getMonth();
    var day = date.getDate();
    var daysElapsed = 0;
    
    for (var i=0; i<month; i++)
    {
        daysElapsed += Jel.Date.daysInMonth(i);
    }   
    
    return daysElapsed + day;
};

/*
    Method: internetBeat
        gets the *Swatch internet time* for the given date object 

    Arguments:
        date - Date object

    Returns: integer
    
    Example:
        > Jel.Date.internetBeat(new Date(2007, 06, 30, 20, 40, 0));    // 486

    Credit:
        Thanks to Peter-Paul Koch of http://www.quirksmode.org/ for this code
*/

Jel.Date.internetBeat = function(date)
{
	var off = (date.getTimezoneOffset() + 60)*60;

	var theSeconds = (date.getHours() * 3600) + (date.getMinutes() * 60) + date.getSeconds() + off;

	var beat = Math.floor(theSeconds/86.4);

	if (beat > 1000) 
		beat -= 1000;

	if (beat < 0) 
		beat += 1000;

	return beat;
};

/*
Class: Jel.Number
    *Utility Methods* for manipulating JavaScript's *built-in Number class*
*/

Jel.Number = 
{
    /*
        Method: leadingZero
            gets a *string representation* for a given number *padded out with leading zeros to a given length*

        Arguments:
            number - Number, the number to append leading zeros to
            toLength - integer, optional, the total length of the final leading-zero-padded string. 
                      If not specified, defaults to a length of 2

        Examples:
            > Jel.Number.leadingZero(1);        // 01
            > Jel.Number.leadingZero(1, 3);     // 001
            > Jel.Number.leadingZero(10, 3);    // 010
            > Jel.Number.leadingZero(1000, 3);  // 1000

    */
    
    leadingZero: function(number, toLength)
    {
        return Jel.String.repeat('0', (toLength || 2) - number.toString().length) + number.toString();
    },

    /*
        Method: ordinalSuffix
            gets the English ordinal suffix for a given number *(st, nd, rd, th)*
    
        Arguments:
            number - Number, the value to get the ordinal suffix for
            
        Example:
            > Jel.Number.ordinalSuffix(2);    // "st"
            > Jel.Number.ordinalSuffix(2);    // "nd"
            > Jel.Number.ordinalSuffix(23);   // "rd"
            > Jel.Number.ordinalSuffix(11);   // "th"
            > Jel.Number.ordinalSuffix(14);   // "th"
    */

    ordinalSuffix: function(number)
    {
        var str = Math.round(number).toString();
        var rem = Math.round(number) % 10;
    
        // first, special case for teen numbers (which are always "th")
    
        if (Jel.String.right(str, 2))
            if (Jel.String.right(str, 2).length == 2 && Jel.String.left(Jel.String.right(str, 2), 1) == "1")
                return "th";
    
        switch (rem)
        {
            case 1:
            {
                return "st";
                break;
            }
            case 2:
            {
                return "nd";
                break;
            }
            case 3:
            {
                return "rd";
                break;
            }
            default:
                return "th";
        }
    },

    /*
        Method: format
            *formats a given number* in a *specified display format*

        Arguments:
            number - Number object, the number to format
            format - string, describes the format of the output string (see examples below)
    
        Returns:
            string - the formatted output string
        
        Examples:
            > Jel.Number.format(3129.95, "$#,###.##");  // $3,129.95   
            > Jel.Number.format(3129.95, "$####.##");   // $3129.95 	  
            > Jel.Number.format(329.95, "$#####.##");   // $329.95 	  
            > Jel.Number.format(329, "$###");           // $329 		  
            > Jel.Number.format(-329, "(###)");         // (329)		  
            > Jel.Number.format(-1234.95, "#,##0.##");  // -1,234.95	  
            > Jel.Number.format(0.01, "#.##");          // 01		  
            > Jel.Number.format(0.01, "#.###");         // 010		  
            > Jel.Number.format(0.01, "0.##");          // 0.01		  
            > Jel.Number.format(2, "00");               // 02		  
            > Jel.Number.format(2345, "00000");         // 02345		  
            > Jel.Number.format(45, "00000");           // 00045
        
        Formatting Rules:
            > # - substitutes for a number, but only if this position has a definite non-zero number here
            > 0 - substitutes for a number always, using zero if this position has no definite non-zero number here
        
    
    */

    format: function(number, format)
    {
        var formatted;
    	var formattedDec = '';
    	var formattedWhole = '';

    	var strWhole;

    	var value = Math.abs(number);
    	var valueWhole = Math.floor(value);

    	var formatter = format;

    	var parenthesis = false;

    	// check if negative values should use parenthesis formatting

    	var matches = formatter.match(/\((.*?)\)/, "ig");

    	if (matches && matches.length > 0)
    	{
    		parenthesis = true;

    		// take the rest of the string as the actual formatter
    		formatter = matches[1];
    	}

    	var formatterWhole = formatter;

    	var parts = formatter.split(".");

    	if (parts.length > 1)
    	{
    		// the string has a decimal part
    		value = value.toFixed(parts[1].length);	

    		formatterWhole = parts[0];
    	}
    	else
    	{
    		valueWhole = Math.round(value);	
    	}

    	// now work out how to format the whole number part
    	formatted = value.toString();

    	if (parts.length > 1)
    	{
    		formattedDec = "." + formatted.split(".")[1];
    	}

    	strWhole = Math.abs(valueWhole).toString();

    	// first, pad out formatterWhole up to the length of valueWhole, with #  

    	var count = 0;

    	formatterWhole.toArray().each
    	(
    		function (chr)
    		{
    			if (chr == '#' || chr == '0')
    				count++;
    		}
    	);

    	matches = formatterWhole.match(/[^#0,]*?([#0,]+)[^#0,]*?/);

    	if (matches.length > 1)
    	{
    		formatterWhole = formatterWhole.replace(matches[1], String.stringOfChar('#', strWhole.length - count) + matches[1]);
    	}

    	var formatterChars = formatterWhole.toArray();

    	var digitIndex = strWhole.length - 1;

    	for (var i = formatterChars.length - 1; i>=0; i--)
    	{
    		// process each character in the formatter string 

    		var chr = formatterChars[i];
    		var ten = Math.pow(10, strWhole.length - 1 - digitIndex);


    		if (chr == '#')
    		{
    			if (valueWhole >= ten)
    			{
    				formattedWhole = strWhole.substr(digitIndex, 1) + formattedWhole;
    			} 
    			// otherwise add nothing
    			digitIndex = digitIndex - 1;	
    		}
    		else if (chr == '0')
    		{
    			if (valueWhole >= ten)
    			{
    				formattedWhole = strWhole.substr(digitIndex, 1) + formattedWhole;
    			} 
    			else
    			{
    				// otherwise add a 0
    				formattedWhole = '0' + formattedWhole;
    			}
	
    			digitIndex = digitIndex - 1;
    		}
    		else if (chr == ',')
    		{
    			if (valueWhole >= ten)
    			{
    				formattedWhole = chr + formattedWhole;
    			}
    		}
    		else
    		{
    			formattedWhole = chr + formattedWhole;
    		}
    	}

    	// apply the parenthesis if the original value is negative

    	if (number < 0)
    	{
    		if (parenthesis)
    			return '(' + formattedWhole + formattedDec + ')';
    		else
    			return '-' + formattedWhole + formattedDec;
    	}

    	return (formattedWhole + formattedDec);
    }
};

/* END THIRD PARTY SOURCE */
/*!
 * This file is part of Appcelerator.
 *
 * Copyright 2006-2008 Appcelerator, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/
/**
 * Appcelerator bootstrap loader
 */

/** THESE CHECKS ARE NEEDED IN CASE THE NON-BUNDLED VERSION OF PROTOTYPE/SCRIPTACULOUS IS USED **/
 
if (typeof Prototype=='undefined')
{
    var msg = 'Required javascript library "Prototype" not found';
    alert(msg);
    throw msg;
}

if (typeof Effect=='undefined')
{
    var msg = 'Required javascript library "Scriptaculous" not found';
    alert(msg);
    throw msg;
}
        

if (Object.isUndefined(window['$sl']))
{
	/**
	 * create a non-conflicting alias to $$
	 */
	window.$sl = function()
	{
	    return Selector.findChildElements(document, $A(arguments));
	}
}
if (Object.isUndefined(window['$el']))
{
	/**
	 * create a non-conflicting alias to $
	 */
	window.$el = eval('window["$"]');
}

var Appcelerator = {};
Appcelerator.Util={};
Appcelerator.Browser={};
Appcelerator.Compiler={};
Appcelerator.Config={};
Appcelerator.Core={};
Appcelerator.Localization={};
Appcelerator.Validator={};
Appcelerator.Decorator={};
Appcelerator.Module={};
Appcelerator.Widget={};
Appcelerator.Shortcuts={}; // please do not touch this

Appcelerator.started = new Date;
Appcelerator.loadTime = -1;
Appcelerator.compileTime = -1;

Appcelerator.Version = 
{
	major: parseInt('2'),
	minor: parseInt('2'),
	revision: parseInt('2'),
	toString:function()
	{
		return this.major + "." + this.minor + '.' + this.revision;
	}
};

Appcelerator.LicenseType = 'Apache License Version 2.0 - see http://license.appcelerator.org';
Appcelerator.Copyright = 'Copyright (c) 2006-2008 by Appcelerator, Inc. All Rights Reserved.';
Appcelerator.LicenseMessage = 'Appcelerator is licensed under ' + Appcelerator.LicenseType;
Appcelerator.Parameters = $H({});



var APPCELERATOR_DEBUG_LEVEL = Appcelerator.Parameters.get('debug');
var APPCELERATOR_DEBUG = window.location.href.indexOf('debug=1') > 0 || APPCELERATOR_DEBUG_LEVEL=='1';
var log4javascript_threshold = Appcelerator.Parameters.get('log4javascript');

var Logger = Class.create();
$$l = Logger;
var _logAppender = null;
var _logEnabled = true;
Logger.toLevel = function(value, logger)
{
	if (!value)
		return log4javascript.Level.INFO;
	value = value.toUpperCase();
	if (value==log4javascript.Level.INFO.toString())
		return log4javascript.Level.INFO;
	else if (value==log4javascript.Level.WARN.toString())
		return log4javascript.Level.WARN;
	else if (value==log4javascript.Level.ERROR.toString())
		return log4javascript.Level.ERROR;
	else if (value==log4javascript.Level.FATAL.toString())
		return log4javascript.Level.FATAL;
	else if (value==log4javascript.Level.TRACE.toString())
		return log4javascript.Level.TRACE;
	else if (value==log4javascript.Level.DEBUG.toString())
		return log4javascript.Level.DEBUG;

	return logger.getLevel();
}

if (log4javascript_threshold && log4javascript_threshold!='')
{
	_log = log4javascript.getDefaultLogger();
	var Level = Logger.toLevel(log4javascript_threshold, _log);
	Logger.infoEnabled = log4javascript.Level.INFO.isGreaterOrEqual(Level);
	Logger.warnEnabled = log4javascript.Level.WARN.isGreaterOrEqual(Level);
	Logger.errorEnabled = log4javascript.Level.ERROR.isGreaterOrEqual(Level);
	Logger.fatalEnabled = log4javascript.Level.FATAL.isGreaterOrEqual(Level);
	Logger.traceEnabled = log4javascript.Level.TRACE.isGreaterOrEqual(Level);
	Logger.debugEnabled = log4javascript.Level.DEBUG.isGreaterOrEqual(Level);
}
else
{
	_logAppender = true;
	_logEnabled = _logEnabled && typeof(console)!='undefined';
	_log =
	{
		console:function(type,msg)
		{
			try
			{
				if (console && console[type]) console[type](msg);
			}
			catch(e)
			{

			}
		},
		debug:function(msg) { if (APPCELERATOR_DEBUG) this.console('debug',msg);  },
		info:function(msg)  { this.console('info',msg);   },
		error:function(msg) { this.console('error',msg);  },
		warn:function(msg)  { this.console('warn',msg);   },
		trace:function(msg) { if (APPCELERATOR_DEBUG) this.console('debug',msg);  },
		fatal:function(msg) { this.console('error',msg);  }
	};

	Logger.infoEnabled = _logEnabled && console.info;
	Logger.warnEnabled = _logEnabled && console.warn;
	Logger.errorEnabled = _logEnabled && console.error;
	Logger.fatalEnabled = _logEnabled && console.error;
	Logger.traceEnabled = _logEnabled && APPCELERATOR_DEBUG && console.debug;
	Logger.debugEnabled = _logEnabled && APPCELERATOR_DEBUG && console.debug;
}


Logger.info = function(msg)
{
    if (Logger.infoEnabled) _log.info(msg);
};
Logger.warn = function(msg)
{
    if (Logger.warnEnabled) _log.warn(msg);
};
Logger.debug = function(msg)
{
    if (Logger.debugEnabled) _log.debug(msg);
};
Logger.error = function(msg)
{
    if (Logger.errorEnabled) _log.error(msg);
};
Logger.trace = function(msg)
{
    if (Logger.traceEnabled) _log.trace(msg);
}
Logger.fatal = function(msg)
{
    if (Logger.fatalEnabled) _log.fatal(msg);
}


/**
 * define a convenience shortcut for debugging messages
 */
function $D()
{
    if (Logger.debugEnabled)
    {
        Logger.debug(Logger.buildMessage(arguments));
    }
}

function $E()
{
    if (Logger.errorEnabled)
    {
        Logger.error(Logger.buildMessage(arguments));
    }
}

// prettier debug output
Logger.buildMessage = function(args)
{
    args = $A(args)
    for(var i = 0; i<args.length; i++)
    {
        var arg = args[i];
        if(arg)
        {
            if(args[i].constructor == Object)
            {   // for simple objects
                args[i] = Object.toJSON(args[i]);
            }
            else
            {   // for dom elements, builtin types, etc
                args[i] = args[i].toString();
            }
        }
    }
    return args.join('');
}

if(typeof err == 'undefined') {
    var err = {
        println: function(msg) {
            $E(msg);
        }
    };
}
if(typeof out == 'undefined') {
    var out = {
        println: function(msg) {
            $D(msg);
        }
    };
}


Object.extend(String.prototype,
{
	/**
	 * trims leading and trailing spaces
	 */
    trim: function()
    {
        return this.replace(/^\s+/g, '').replace(/\s+$/g, '');
    },

	/**
	 * return true if this string starts with value
	 */
    startsWith: function(value)
    {
        if (value.length <= this.length)
        {
            return this.substring(0, value.length) == value;
        }
        return false;
    },

	/**
	 * eval the contents of the string as a javascript function
	 * and return the function reference
	 */
    toFunction: function (dontPreProcess)
    {
        var str = this.trim();
        if (str.length == 0)
        {
            return Prototype.emptyFunction;
        }
        if (!dontPreProcess)
        {
            if (str.match(/^function\(/))
            {
                str = 'return ' + String.unescapeXML(this) + '()';
            }
            else if (!str.match(/return/))
            {
                str = 'return ' + String.unescapeXML(this);
            }
            else if (str.match(/^return function/))
            {
                // invoke it as the return value
                str = String.unescapeXML(this) + ' ();';
            }
        }
        var code = 'var f = function(){ var args = $A(arguments); ' + str + '}; f;';
        var func = eval(code);
        if (typeof(func) == 'function')
        {
            return func;
        }
        throw Error('code was not a function: ' + this);
    },

	toFunctionString: function (functionId)
	{
		var str = this.trim();
        var code = 'var functionString_'+functionId+' = function(){ var args = $A(arguments); ' + str + '}; functionString_'+functionId+'();';
		return code;
	}

});


/**
 * escape XML entities in the value passed
 */
String.escapeXML = function(value)
{
	if (!value) return null;
    return value.replace(
    /&/g, "&amp;").replace(
    /</g, "&lt;").replace(
    />/g, "&gt;").replace(
    /"/g, "&quot;").replace(
    /'/g, "&apos;");
}

/**
 * unescape XML entities back into their normal values
 */
String.unescapeXML = function(value)
{
    if (!value) return null;
    return value.replace(
	/&lt;/g,   "<").replace(
	/&gt;/g,   ">").replace(
	/&apos;/g, "'").replace(
	/&amp;/g,  "&").replace(
	/&quot;/g, "\"");
};


// This code was written by Tyler Akins and has been placed in the
// public domain.  It would be nice if you left this header intact.
// Base64 code from Tyler Akins -- http://rumkin.com

(function()
{
    var keyStr = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
	String.prototype.encode64 = function() 
	{
	   var input = this;
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
	};
	String.prototype.decode64 = function () {
	   var output = "";
	   var chr1, chr2, chr3;
	   var enc1, enc2, enc3, enc4;
	   var i = 0;
	   
	   var input = this;
	
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
	};
})();

String.stringValue = function(str) 
{
	if (str)
	{
		return '"'+str.replace(/"/g, '\\"')+'"';
	}
	else
	{
		return null;
	}
};

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

Appcelerator.URI = Class.create();

/**
 * This function determines whether the given URI reference is absolute
 * (has a scheme).
 */
Appcelerator.URI.isAbsolute = function(uriRef) 
{
	return absoluteUriRefRegex.test(uriRef);
};

/*
splitUriRef(uriRef)

This function splits a URI reference into an Array of its principal components,
[scheme, authority, path, query, fragment] as per STD 66 / RFC 3986 appendix B.
*/

Appcelerator.URI.splitUriRef = function(uriRef) 
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
Appcelerator.URI.unsplitUriRef=function(uriRefSeq) 
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
Appcelerator.URI.uriPathRemoveDotSegments = function (path) 
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
Appcelerator.URI.absolutizeURI = function(uriRef, baseUri)
{
	// Ensure base URI is absolute
	if (! baseUri || ! Appcelerator.URI.isAbsolute(baseUri)) {
		 throw Error("baseUri '" + baseUri + "' is not absolute");
	}
	// shortcut for the simplest same-document reference cases
	if (uriRef == "" || uriRef.charAt(0) == "#") {
		return baseUri.split('#')[0] + uriRef;
	}
	var tScheme, tAuth, tPath, tQuery;
	// parse the reference into its components
	var parts = Appcelerator.URI.splitUriRef(uriRef);
	var rScheme=parts[0], rAuth=parts[1], rPath=parts[2], rQuery=parts[3], rFrag=parts[4];
	// if the reference is absolute, eliminate '.' and '..' path segments
	// and skip to the end
	if (typeof rScheme != "undefined") {
		var tScheme = rScheme;
		var tAuth = rAuth;
		var tPath = Appcelerator.URI.uriPathRemoveDotSegments(rPath);
		var tQuery = rQuery;
	} else {
		// the base URI's scheme, and possibly more, will be inherited
		parts = Appcelerator.URI.splitUriRef(baseUri);
		var bScheme=parts[0], bAuth=parts[1], bPath=parts[2], bQuery=parts[3], bFrag=parts[4];
		// if the reference is a net-path, just eliminate '.' and '..' path
		// segments; no other changes needed.
		if (typeof rAuth != "undefined") {
			tAuth = rAuth;
			tPath = Appcelerator.URI.uriPathRemoveDotSegments(rPath);
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
					tPath = Appcelerator.URI.uriPathRemoveDotSegments(rPath);
				// merge the reference's relative path with the base URI's path
				} else {
					if (typeof bAuth != "undefined" && ! bPath) {
						tPath = "/" + rPath;
					} else {
						tPath = bPath.substring(0, bPath.lastIndexOf("/") + 1) + rPath;
					}
					tPath = Appcelerator.URI.uriPathRemoveDotSegments(tPath);
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
	var result = Appcelerator.URI.unsplitUriRef([tScheme, tAuth, tPath, tQuery, rFrag]);
	return result;
};/**
 * Appcelerator Core
 */

// 
// basic initialization for the core
// 
(function()
{
	var baseHref = null;
	var documentRoot = null;
	
	//
	// top is important such that if the JS file is in a different location (hosted)
	// than the primary document, we use the primary document's path (cross site scripting)
	//
	var idx = top.window.document.location.href.lastIndexOf('/');
    if (idx == top.window.document.location.href.length - 1)
    {
    	documentRoot = top.window.document.location.href;
    }
    else
    {
        documentRoot  = top.window.document.location.href.substr(0, idx);
        if (documentRoot.substring(documentRoot.length - 1) != '/')
        {
            documentRoot  = documentRoot + '/';
        }
    }

	$A(document.getElementsByTagName("script")).findAll( function(s) 
	{
	    if (s.src && s.src.match(/appcelerator(-debug){0,1}\.js(\?.*)?$/))
	    {
	    	Appcelerator.jsFileLocation = Appcelerator.URI.absolutizeURI(s.src,documentRoot);
	    	return true;
	    }
	    return false;
	}).each( function(s) 
	{
		Appcelerator.Parameters = $H(s.src.toQueryParams());
	});	

	$A(document.getElementsByTagName("base")).each( function(s) 
	{
		if (s.href)
		{
			baseHref = Appcelerator.URI.absolutizeURI(s.href,documentRoot);
			throw $break;
		}
	});

	if (baseHref)
	{
		Appcelerator.DocumentPath = baseHref;
	}
	else
	{	
		Appcelerator.DocumentPath = documentRoot;
	}
	
	if (Appcelerator.jsFileLocation)
	{
		if (!baseHref)
		{
			// see if it's a full URI
			var hostIdx = Appcelerator.jsFileLocation.indexOf(':/');
			if (hostIdx > 0)
			{
				var jsHostPath = Appcelerator.jsFileLocation.substring(hostIdx + 3, Appcelerator.jsFileLocation.indexOf('/',hostIdx + 4));
				var docIdx = Appcelerator.DocumentPath.indexOf(':/');
				if (docIdx > 0)
				{
					var docHostPath = Appcelerator.DocumentPath.substring(docIdx + 3, Appcelerator.DocumentPath.indexOf('/',docIdx+4));
					if (docHostPath == jsHostPath)
					{
						// if on the same host then always prefer the JS location (one directory up) as the base href
						// such that we can have multiple content directories that include the JS relatively from the top
						Appcelerator.DocumentPath = Appcelerator.URI.absolutizeURI(Appcelerator.jsFileLocation.substring(0,Appcelerator.jsFileLocation.lastIndexOf('/')) + '/../',Appcelerator.DocumentPath);
					}
				}
			}
			else
			{
				// relative URI we need to adjust the DocumentPath
				if (Appcelerator.jsFileLocation.startsWith('/') || Appcelerator.jsFileLocation.startsWith('.'))
				{
					var idx = Appcelerator.jsFileLocation.lastIndexOf('/');
					if (idx!=-1)
					{
						Appcelerator.DocumentPath = Appcelerator.URI.absolutizeURI(Appcelerator.jsFileLocation.substring(0,idx+1) + '../',Appcelerator.DocumentPath);
					}
				}
			}
		}
	}
	else
	{
		Appcelerator.ScriptNotFound = true;
	}
	
    Appcelerator.ScriptPath = Appcelerator.DocumentPath + 'javascripts/';
    Appcelerator.ImagePath = Appcelerator.DocumentPath + 'images/';
    Appcelerator.StylePath = Appcelerator.DocumentPath + 'stylesheets/';
    Appcelerator.ContentPath = Appcelerator.DocumentPath + 'content/';
    Appcelerator.ModulePath = Appcelerator.DocumentPath + 'widgets/';
    Appcelerator.WidgetPath = Appcelerator.DocumentPath + 'widgets/';

	if (Appcelerator.jsFileLocation.indexOf('code.appcelerator.org') != -1)
	{
		var codepath = (('https:' == document.location.protocol) ? 'https://s3.amazonaws.com/code.appcelerator.org' : 'http://code.appcelerator.org' );
		Appcelerator.ModulePath = codepath + Appcelerator.Version + '/widgets/';
		Appcelerator.WidgetPath = Appcelerator.ModulePath;
	}

    Appcelerator.Parameters = Appcelerator.Parameters.merge(window.location.href.toQueryParams());

	if (Appcelerator.Parameters.get('instanceid'))
	{
		Appcelerator.instanceid = Appcelerator.Parameters.get('instanceid');
	}
	else
	{
		Appcelerator.instanceid = Math.round(9999*Math.random()) + '-' + Math.round(999*Math.random());
	}

	var ua = navigator.userAgent.toLowerCase();
	Appcelerator.Browser.isPreCompiler = (ua.indexOf('Appcelerator Compiler') > -1);
	Appcelerator.Browser.isOpera = (ua.indexOf('opera') > -1);
	Appcelerator.Browser.isSafari = (ua.indexOf('safari') > -1);
	Appcelerator.Browser.isSafari2 = false;
	Appcelerator.Browser.isSafari3 = false;
	Appcelerator.Browser.isIE = !!(window.ActiveXObject);
	Appcelerator.Browser.isIE6 = false;
	Appcelerator.Browser.isIE7 = false;
	Appcelerator.Browser.isIE8 = false;

	if (Appcelerator.Browser.isIE)
	{
		var arVersion = navigator.appVersion.split("MSIE");
		var version = parseFloat(arVersion[1]);
		Appcelerator.Browser.isIE6 = version >= 6.0 && version < 7;
		Appcelerator.Browser.isIE7 = version >= 7.0 && version < 8;
		Appcelerator.Browser.isIE8 = version >= 8.0 && version < 9;
	}

	if (Appcelerator.Browser.isSafari)
	{
		var webKitFields = RegExp("( applewebkit/)([^ ]+)").exec(ua);
		if (webKitFields[2] > 400 && webKitFields[2] < 500)
		{
			Appcelerator.Browser.isSafari2 = true;
		}
		else if (webKitFields[2] > 500 && webKitFields[2] < 600)
		{
			Appcelerator.Browser.isSafari3 = true;
		}
	}

	Appcelerator.Browser.isGecko = !Appcelerator.Browser.isSafari && (ua.indexOf('gecko') > -1);
	Appcelerator.Browser.isCamino = Appcelerator.Browser.isGecko && ua.indexOf('camino') > -1;
	Appcelerator.Browser.isFirefox = Appcelerator.Browser.isGecko && (ua.indexOf('firefox') > -1 || Appcelerator.Browser.isCamino || ua.indexOf('minefield') > -1 || ua.indexOf('granparadiso') > -1 || ua.indexOf('bonecho') > -1);
	Appcelerator.Browser.isIPhone = Appcelerator.Browser.isSafari && ua.indexOf('iphone') > -1;
	Appcelerator.Browser.isMozilla = Appcelerator.Browser.isGecko && ua.indexOf('mozilla/') > -1;
	Appcelerator.Browser.isWebkit = Appcelerator.Browser.isMozilla && Appcelerator.Browser.isGecko && ua.indexOf('applewebkit') > 0;
	Appcelerator.Browser.isSeamonkey = Appcelerator.Browser.isMozilla && ua.indexOf('seamonkey') > -1;
	Appcelerator.Browser.isPrism = Appcelerator.Browser.isMozilla && ua.indexOf('prism/') > 0;
    Appcelerator.Browser.isIceweasel = Appcelerator.Browser.isMozilla && ua.indexOf('iceweasel') > 0;
    Appcelerator.Browser.isEpiphany = Appcelerator.Browser.isMozilla && ua.indexOf('epiphany') > 0;
	Appcelerator.Browser.isFluid = (window.fluid != null);
	Appcelerator.Browser.isGears = (window.google && google.gears) != null;
	Appcelerator.Browser.isChromium = Appcelerator.Browser.isWebkit && ua.indexOf('chrome/') > 0;

	Appcelerator.Browser.isWindows = false;
	Appcelerator.Browser.isMac = false;
	Appcelerator.Browser.isLinux = false;
	Appcelerator.Browser.isSunOS = false;

	var platform = null;

	if(ua.indexOf("windows") != -1 || ua.indexOf("win32") != -1)
	{
	    Appcelerator.Browser.isWindows = true;
		platform = 'win32';
	}
	else if(ua.indexOf("macintosh") != -1 || ua.indexOf('mac os x') != -1)
	{
		Appcelerator.Browser.isMac = true;
		platform = 'mac';
	}
	else if (ua.indexOf('linux')!=-1)
	{
		Appcelerator.Browser.isLinux = true;
		platform = 'linux';
	}
	else if (ua.indexOf('sunos')!=-1)
	{
		Appcelerator.Browser.isSunOS = true;
		platform = 'sun';
	}

	// silverlight detection
	// thanks to http://www.nikhilk.net/Silverlight-Analytics.aspx
    Appcelerator.Browser.isSilverlight = false;
	Appcelerator.Browser.silverlightVersion = 0;
	Event.observe(window,'load',function()
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
					Appcelerator.Browser.silverlightVersion = 2.0; 
				}
	            else if (control.isVersionSupported('1.0')) 
				{ 
					Appcelerator.Browser.silverlightVersion = 1.0; 
				}
				Appcelerator.Browser.isSilverlight = Appcelerator.Browser.silverlightVersion > 0;
	        }
	    }
	    catch (e) { }
	    if (container) {
	        document.body.removeChild(container);
	    }
	});

	// flash detection
	Appcelerator.Browser.isFlash = false;
	Appcelerator.Browser.flashVersion = 0;
	if (Appcelerator.Browser.isIE)
	{
			try
			{
				var flash = new ActiveXObject("ShockwaveFlash.ShockwaveFlash.7");
				var ver = flash.GetVariable("$version");
				var idx = ver.indexOf(' ');
				var tokens = ver.substring(idx+1).split(',');
				var version = tokens[0];
				Appcelerator.Browser.flashVersion = parseInt(version);
				Appcelerator.Browser.isFlash = true;
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
					Appcelerator.Browser.flashVersion = parseInt(ver.charAt(ver.indexOf('.')-1));
					Appcelerator.Browser.isFlash = true;
				}			 	
				else
				{
					// not sure what version... ?
					Appcelerator.Browser.flashVersion = 7;
					Appcelerator.Browser.isFlash = true;
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
				Appcelerator.Browser.isFlash = true;
		    	Appcelerator.Browser.flashVersion = parseInt(plugin.description.substring(plugin.description.indexOf(".")-1));
			}
		}
	}
	Appcelerator.Browser.isBrowserSupported = false;
	$w('Firefox IE6 IE7 IE8 Safari Camino Opera Webkit Seamonkey Prism Iceweasel Epiphany').each(function(name)
	{
        if (Appcelerator.Browser['is'+name]===true)
        {
            Appcelerator.Browser.isBrowserSupported=true;
			Event.observe(window,'load',function()
			{
				if (platform) Element.addClassName(document.body,platform);
				Element.addClassName(document.body,name.toLowerCase());
				if (Appcelerator.Browser.isMozilla)
				{
					Element.addClassName(document.body,'mozilla');
				}
				if (Appcelerator.Browser.isIPhone)
				{
					Element.addClassName(document.body,'iphone');
					Element.addClassName(document.body,'webkit');
					Element.addClassName(document.body,'safari');
				}
				if (Appcelerator.Browser.isChromium)
				{
					Element.addClassName(document.body,'chromium');
				}
				if (Appcelerator.Browser.isSafari)
				{
					Element.addClassName(document.body,'webkit');
					if (Appcelerator.Browser.isSafari2)
					{
						Element.addClassName(document.body,'safari2');
					}
					else if (Appcelerator.Browser.isSafari3)
					{
						Element.addClassName(document.body,'safari3');
					}
				}
				else if (Appcelerator.Browser.isGecko)
				{
					Element.addClassName(document.body,'gecko');
				}
				if (Appcelerator.Browser.isFirefox)
				{
					if (ua.indexOf('firefox/3')>0)
					{
						Element.addClassName(document.body,'firefox3');
					}
					else if (ua.indexOf('firefox/2')>0)
					{
						Element.addClassName(document.body,'firefox2');
					}
				}
				else if (Appcelerator.Browser.isIE)
				{
					Element.addClassName(document.body,'msie');
				}
				if (Appcelerator.Browser.isIPhone)
				{
					Element.addClassName(document.body,'width_narrow');
					Element.addClassName(document.body,'height_short');
				}
				else
				{
					function calcDim()
					{
						var cn = Element.classNames(document.body);
						if (cn)
						{
							cn._each(function(name)
							{
								if (name.startsWith('height_') || name.startsWith('width_'))
								{
									cn.remove(name);
								}
							});
						}
                        var width = document.documentElement.clientWidth || window.screen.width;
                        var height = document.documentElement.clientHeight || window.screen.height;

						if (height < 480)
						{
							Element.addClassName(document.body,'height_tiny');
						}
						else if (height >= 480 && height <= 768)
						{
							Element.addClassName(document.body,'height_small');
						}
						else if (height > 768  && height < 1100)
						{
							Element.addClassName(document.body,'height_medium');
						}
						else if (height >= 1100)
						{
							Element.addClassName(document.body,'height_large');
						}
						if (width <= 640)
						{
							Element.addClassName(document.body,'width_tiny');
						}
						else if (width > 640 && width <= 1024)
						{
							Element.addClassName(document.body,'width_small');
						}
						else if (width > 1024 && width <=1280 )
						{
							Element.addClassName(document.body,'width_medium');
						}
						else if (width > 1280)
						{
							Element.addClassName(document.body,'width_large');
						}
					}
					Event.observe(window,'resize',calcDim);
					calcDim();
				}
			});
            throw $break;
        }
	});
	Appcelerator.Browser.unsupportedBrowserMessage = "<h1>Browser Upgrade Required</h1><p>We're sorry, but your browser version is not supported by this application.</p><p>This application requires a modern browser, such as <a href='http://www.getfirefox.com'>Firefox 2.0+</a>, <a href='http://www.apple.com/safari/'>Safari 2.0+</a>, <a href='http://www.microsoft.com/windows/products/winfamily/ie/default.mspx'>Internet Explorer 6.0+</a> or <a href='http://www.opera.com'>Opera 9.0+</a>.</p><p>Your browser reported: <font face='courier'>" + ua + "</font></p>";
	Appcelerator.Browser.upgradePath = Appcelerator.DocumentPath + 'upgrade.html';
})();


Appcelerator.Core.usedModules = {};
Appcelerator.Core.modules = [];
Appcelerator.Core.loadedFiles = {};
Appcelerator.Core.fetching = {};
Appcelerator.Core.widgets = {};
Appcelerator.Core.widgets_css = {};
Appcelerator.Core.widgets_js = {};
Appcelerator.Core.moduleLoaderCallbacks = {};
Appcelerator.Core.script_count = 0;
Appcelerator.Core.scriptWithDependenciesQueue = [];
Appcelerator.Core.scriptWithDependenciesCallback;

Appcelerator.Core.HeadElement = document.getElementsByTagName('head')[0];

/**
 * dynamically load CSS from common CSS path and call onload once 
 * loaded (or immediately if already loaded)
 *
 * @param {string} name of the common file
 * @param {function} onload function to invoke
 */
Appcelerator.Core.requireCommonCSS = function(name,onload,onerror)
{
    var srcpath = Appcelerator.Core.getModuleCommonDirectory()+'/css/'
            
    Appcelerator.Core.requireMultiple(function(path,action)
    {
        Appcelerator.Core.remoteLoadCSS(path,onload,onerror);
    },srcpath,name);
};


/**
 * dynamically load JS from common JS path and call onload once 
 * loaded (or immediately if already loaded)
 *
 * @param {string} name of the common js
 * @param {function} onload function to callback upon load
 */
Appcelerator.Core.requireCommonJS = function(name,onload,onerror)
{
    var srcpath = Appcelerator.Core.getModuleCommonDirectory()+'/js/'
            
    Appcelerator.Core.requireMultiple(function(path,action)
    {
        Appcelerator.Core.remoteLoadScript(path,onload,onerror);
    },srcpath,name);
};

/**
 * internal method for loading multiple files
 */
Appcelerator.Core.requireMultiple = function(invoker,srcpath,name,onload,onerror)
{
    if (Object.isUndefined(name))
    {
        if (Object.isFunction(onload)) onload();
        return;
    }
    if (Object.isArray(name))
    {
        name = name.compact();
        var idx = 0;
        var loader = function()
        {
            idx++;
            if (idx == name.length)
            {
                if (Object.isFunction(onload)) onload();
            }
            else
            {
                Appcelerator.Core.requireMultiple(invoker,srcpath,name[idx],loader,onerror);                    
            }
        };
        Appcelerator.Core.requireMultiple(invoker,srcpath,name[idx],loader,onerror);                    
    }
    else
    {
        var path = srcpath+name;
        var loaded = Appcelerator.Core.loadedFiles[path];
        if (loaded)
        {
            if (Object.isFunction(onload)) onload();
        }
        else
        {
            invoker(path,function()
            {
                Appcelerator.Core.loadedFiles[path]=true;
                if (Object.isFunction(onload)) onload();
            });
        }
    }
};

/**
 * dynamically load a javascript file
 *
 * @param {string} path to resource
 * @param {function} onload function to execute once loaded
 */
Appcelerator.Core.remoteLoadScript = function(path,onload,onerror)
{
    Appcelerator.Core.remoteLoad('script','text/javascript',path,onload,onerror);  
};

/**
 * dynamically laod a javascript file with dependencies
 * this will not call the callback until all resources are loaded
 * multiple calls to this method are queued
 * 
 * @param {string} path to resource
 * @param {function} the string representation of the callback
 */
Appcelerator.Core.queueRemoteLoadScriptWithDependencies = function(path, onload) 
{
    Appcelerator.Core.scriptWithDependenciesQueue.push({'path': path, 'onload': onload});
    Appcelerator.Core.remoteLoadScriptWithDependencies();
};

Appcelerator.Core.remoteLoadScriptWithDependencies = function() 
{
    if(0 < Appcelerator.Core.scriptWithDependenciesQueue.length) 
    {
        var script = Appcelerator.Core.scriptWithDependenciesQueue[0];
        Appcelerator.Core.remoteLoad('script', 'text/javascript', script.path, null);
        Appcelerator.Core.scriptWithDependenciesCallback = function() 
        {
            script.onload();
            Appcelerator.Core.scriptWithDependenciesQueue.shift();
            Appcelerator.Core.remoteLoadScriptWithDependencies();
        }
    }
};

/**
 * dynamically load a css file
 *
 * @param {string} path to resource
 * @param {function} onload function to execute once loaded
 */
Appcelerator.Core.remoteLoadCSS = function(path,onload,onerror)
{
    Appcelerator.Core.remoteLoad('link','text/css',path,onload,onerror);  
};

/**
 * dynamically load a remote file
 *
 * @param {string} name of the tag to insert into the DOM
 * @param {string} type as in content type
 * @param {string} full path to the resource
 * @param {function} onload to invoke upon load
 * @param {function} onerror to invoke upon error
 */
Appcelerator.Core.remoteLoad = function(tag,type,path,onload,onerror)
{
	$D('remoteLoad '+tag+',type='+type+',path='+path+',onload='+onload+',onerror='+onerror);

	// fixup the URI
	path = Appcelerator.URI.absolutizeURI(path,Appcelerator.DocumentPath);
	
    var array = Appcelerator.Core.fetching[path];
    if (array)
    {
        if (onload)
        {
            array.push(onload);
        }
        return;
    }
    if (onload)
    {
        Appcelerator.Core.fetching[path]=[onload];
    }
    var element = document.createElement(tag);
    element.setAttribute('type',type);

    switch(tag)
    {
        case 'script':
            element.setAttribute('src',path);
            break;
        case 'link':
            element.setAttribute('href',path);
            element.setAttribute('rel','stylesheet');
            break;
    }
	var timer = null;
    var loader = function()
    {
	   $D('loaded '+path);
	   if (timer) clearTimeout(timer);
       var callbacks = Appcelerator.Core.fetching[path];
       if (callbacks)
       {
           for (var c=0;c<callbacks.length;c++)
           {
               try { callbacks[c](); } catch (E) { }
           }
           delete Appcelerator.Core.fetching[path];
       }
    };    
    if (tag == 'script')
    {
	    if (Appcelerator.Browser.isSafari2)
	    {
	        //this is a hack because we can't determine in safari 2
	        //when the script has finished loading
	        loader.delay(1.5);
	    }
	    else
	    {
	        (function()
	        {
	            var loaded = false;
	            element.onload = loader;
				if (onerror)
				{
					if (!loaded)
					{
						// max time to determine if we've got an error
						// obviously won't work if takes long than 3.5 secs to load script
						timer=setTimeout(onerror,3500);
					}
					element.onerror = function()
					{
						// for browsers that support onerror
						if (timer) clearTimeout(timer);
						onerror();
					};
				}
	            element.onreadystatechange = function()
	            {
	                switch(this.readyState)
	                {
	                    case 'loaded':   // state when loaded first time
	                    case 'complete': // state when loaded from cache
	                        break;
	                    default:
	                        return;
	                }
	                if (loaded) return;
	                loaded = true;
	                
	                // prevent memory leak
	                this.onreadystatechange = null;
	                loader();
	            }   
	        })();
	    }   
	}
	else
	{
	   loader.defer();
	}
    Appcelerator.Core.HeadElement.appendChild(element);
};

//
// dynamically load JS from path and call onload once 
// loaded (or immediately if already loaded)
//
Appcelerator.Core.loadJS = function (path, onload, track, onerror)
{
    Appcelerator.Core.remoteLoadScript(path,onload,onerror);
};

//
// return the module common directory
//
Appcelerator.Core.getModuleCommonDirectory = function ()
{
    return Appcelerator.ModulePath + 'common';
};

//
// called to load a css file relative to the modules/common/css directory
//
Appcelerator.Core.loadModuleCommonCSS = function(moduleName,css)
{
    Appcelerator.Core.loadModuleCSS(moduleName,css,Appcelerator.Core.getModuleCommonDirectory()+'/css')
};

//
// called to load a css file relative to the module css directory
//
Appcelerator.Core.loadModuleCSS = function(moduleName,css,csspath)
{
	moduleName = Appcelerator.Core.getModuleNameFromTag(moduleName);
	
	var loaded = Appcelerator.Core.modules[moduleName];
	if (loaded)
	{
	   throw 'module already loaded when loadModuleCSS is called. Call loadModuleCSS *before* you call registerModule for '+moduleName+' and css: '+css;
	}
	
	var path = csspath ? csspath + '/' + css : Appcelerator.ModulePath + moduleName + '/css/' + css;

	var loaded = Appcelerator.Core.widgets_css[path];
	if (!loaded)
	{
		var link = document.createElement('link');
		link.setAttribute('type','text/css');
		link.setAttribute('rel','stylesheet');
		link.setAttribute('href',path);
		Appcelerator.Compiler.setElementId(link, 'css_'+moduleName);
		
		var refPoint = null;
		for (var c=0;c<Appcelerator.Core.HeadElement.childNodes.length;c++)
		{
			var element = Appcelerator.Core.HeadElement.childNodes[c];
			if (element.nodeType == 1 && element.nodeName == "LINK")
			{
				var src = element.getAttribute("href");
				var type = element.getAttribute("type");
				if (src && type && type.indexOf('css') > 0)
				{
					refPoint = element;
					break;
				}
			}
		}
		if (refPoint)
		{
			// insert it before the first css so that it won't override applications css
			Appcelerator.Core.HeadElement.insertBefore(link,refPoint);
		}
		else
		{
			Appcelerator.Core.HeadElement.appendChild(link);
		}
		Appcelerator.Core.widgets_css[path]=moduleName;

		//Refresh css in IE 6/7 because the give priority to css in load order, not document order
		/*
		if (Appcelerator.Browser.isIE)
		{
		    // fun with IE, crashes in IE6 is you do this on the same thread so we 
		    // have to give it up
		    setTimeout(function()
		    {
	            var link = document.styleSheets[document.styleSheets.length-1].owningElement;
	            if (link)
	            {
	                link.moduleCSS = 1;
	            }
	            
	            var ss = document.styleSheets;
	            var arr = [];
	            var modarr = [];
	            
	            try
	            {
	                for (var i = 0; i < ss.length; i++)
	                {
	                    if (ss[i].owningElement.moduleCSS)
	                    {
	                        modarr.push ([ss[i].owningElement,ss[i].owningElement.outerHTML]);
	                    }
	                    else
	                    {
	                        arr.push ([ss[i].owningElement,ss[i].owningElement.outerHTML]);
	                    }
	                }
	            } 
	            catch (e) 
	            { 
	                throw 'Failed to gather CSS: ' + e.message; 
	            }
	                    
	            try
	            {
	                for (var i = arr.length-1; i >= 0; i--)
	                {
	                    Element.remove(arr[i][0]);
	                }
	                
	                for (var i = modarr.length-1; i >= 0; i--)
	                {
	                    Element.remove(modarr[i][0]);
	                }
	                
	                for (var i = 0; i < modarr.length; i++)
	                {
	                    var elem = document.createElement(modarr[i][1]);
	                    elem.moduleCSS = 1;
	                    Appcelerator.Core.HeadElement.appendChild(elem);
	                }
	                            
	                for (var i = 0; i < arr.length; i++)
	                {
	                    if(arr[i][0].tagName == 'STYLE')
	                    {
	                        var style = document.createStyleSheet();
	                        style.cssText = arr[i][0].styleSheet.cssText;
	                    }
	                    else 
	                    {
	                        var elem = document.createElement(arr[i][1]);
	                        Appcelerator.Core.HeadElement.appendChild(elem);
	                    }
	                }
	            } 
	            catch (e) 
	            { 
	                throw 'Failed to refresh CSS: ' + e.message; 
	            }
		    },100);
		}*/
	}
};


Appcelerator.Core.getModuleNameFromTag=function(moduleName)
{
	return moduleName.replace(':','_');
};

/**
 * require a module and call onload when it is loaded and registered
 * 
 * @param {string} name of the module (for example, app:script)
 * @param {function} function to call upon loading *and* registering the module
 */
Appcelerator.Core.requireModule = function(moduleName,onload,onerror)
{
	var moduleName = Appcelerator.Core.getModuleNameFromTag(moduleName);
	var module = Appcelerator.Core.modules[moduleName];
	
	// already loaded
	if (module)
	{
		onload();
		return;
	}
	
	// already in the process of loading
	var callbacks = Appcelerator.Core.moduleLoaderCallbacks[moduleName];
	if (!callbacks)
	{
		callbacks=[];
		Appcelerator.Core.moduleLoaderCallbacks[moduleName]=callbacks;
		callbacks.push(onload);
	}
	else
	{
		callbacks.push(onload);
		return;
	}
	
	// module needs to be loaded
	var path = Appcelerator.ModulePath + moduleName + '/' + moduleName + '.js';
	Appcelerator.Core.loadJS(path);
};

// map forward to widget
Appcelerator.Core.requireWidget = Appcelerator.Core.requireModule;

/**
 * Modules must call this to register themselves with the framework
 *
 * @param {string} modulename
 * @param {object} module object
 * @param {boolean} dynamic
 */
Appcelerator.Core.registerModule = function (moduleName,module,dynamic)
{
	moduleName = Appcelerator.Core.getModuleNameFromTag(moduleName);
	
	Appcelerator.Core.modules[moduleName] = module;
	
	//
	// determine if the module supports widget and if it does
	// register the widget name
	//
	if (module.isWidget)
	{
		var widgetName = module.getWidgetName().toLowerCase();
		if (Appcelerator.Core.widgets[widgetName])
		{
			throw "duplicate widget name detected: "+widgetName;
		}
		Appcelerator.Core.widgets[widgetName] = module;
	}
	
    //
    //setup unload handler if found in the module
    //
    if (module.onUnload)
    {
        window.observe(window,'unload',module.onUnload);
    }

	//
	// give the module back his path
	// 
	if (typeof(module.setPath)=='function')
	{
		var path = Appcelerator.ModulePath + moduleName + '/';
		module.setPath(path);
	}
	
	var path = Appcelerator.ModulePath + moduleName + '/' + moduleName + '.js';	
	
	var listeners = (Appcelerator.Core.fetching[path] || [] ).concat(Appcelerator.Core.moduleLoaderCallbacks[moduleName]||[]);
	
	if (listeners)
	{
		// notify any pending listeners
		for (var c=0;c<listeners.length;c++)
		{
			listeners[c]();
		}
	}

	delete Appcelerator.Core.moduleLoaderCallbacks[moduleName];
    delete Appcelerator.Core.fetching[path];
};

if (Appcelerator.Browser.isIE)
{
    //
    // special loader function for IE which seems to load async when
    // you loop over multiple scripts and add to DOM - this will ensure
    // that only one JS is loaded in order 
    // 
	Appcelerator.Core.registerModuleWithJSInIE = function(moduleName,module,js,idx,jspath)
	{
	    var file = js[idx];
	    moduleName = Appcelerator.Core.getModuleNameFromTag(moduleName);
	    var path = jspath ? jspath + '/' + file : Appcelerator.ModulePath + moduleName + '/js/' + file;

	    var script = document.createElement('script');
	    script.setAttribute('type','text/javascript');
	    script.setAttribute('src',path);
	    script.onerror = function(e)
	    {
	        $E('Error loading '+path+'\n Exception: '+Object.getExceptionDetail(e));
	    };
	    var loaded = false;
	    script.onreadystatechange = function()
	    {
	        switch(this.readyState)
	        {
	            case 'loaded':   // state when loaded first time
	            case 'complete': // state when loaded from cache
	                break;
	            default:
	                return;
	        }
	        if (loaded) return;
	        loaded = true;
	        
	        // prevent memory leak
	        this.onreadystatechange = null;

	        idx=idx+1;
	        
	        if (idx == js.length)
	        {
	            // do something
	            Appcelerator.Core.registerModule(moduleName, module);
	        }
	        else
	        {
	            // continue to the next one
	            Appcelerator.Core.registerModuleWithJSInIE(moduleName,module,js,idx,jspath);
	        }
	    };
        Appcelerator.Core.HeadElement.appendChild(script);
	};
}
// map forward
Appcelerator.Core.registerWidget = Appcelerator.Core.registerModule; 

/**
 * called to load a js file relative to the modules/common/js directory
 *
 * @param {string} moduleName
 * @param {object} module object
 * @param {string} js file(s)
 */
Appcelerator.Core.registerModuleWithCommonJS = function (moduleName,module,js)
{
    Appcelerator.Core.registerModuleWithJS(moduleName,module,js,Appcelerator.Core.getModuleCommonDirectory()+'/js');
};

// map forward
Appcelerator.Core.registerWidgetWithCommonJS = Appcelerator.Core.registerModuleWithCommonJS;

/**
 * called to load a js file relative to the module js directory
 *
 * @param {string} moduleName
 * @param {object} module object
 * @param {string} js files(s)
 * @param {string} js path
 */
Appcelerator.Core.registerModuleWithJS = function (moduleName,module,js,jspath)
{
    moduleName = Appcelerator.Core.getModuleNameFromTag(moduleName);
    
    if (Appcelerator.Browser.isIE)
    {
        Appcelerator.Core.registerModuleWithJSInIE(moduleName,module,js,0,jspath);
        return;
    }
    
    var state = 
    {
        count : js.length
    };
    
    var checkState = function()
    {
        state.count--;
        if (state.count==0)
        {
            Appcelerator.Core.registerModule(moduleName, module);
        }
    };
    
    var orderedLoad = function(i)
    {
        var file = js[i];
        var path = !Object.isUndefined(jspath) ? (jspath + '/' + file) : Appcelerator.ModulePath + moduleName + '/js/' + file;

        var script = document.createElement('script');
        script.setAttribute('type','text/javascript');
        script.setAttribute('src',path);
        script.onerror = function(e)
        {
            $E('Error loading '+path+'\n Exception: '+Object.getExceptionDetail(e));
        };
        if (Appcelerator.Browser.isSafari2)
        {
            //this is a hack because we can't determine in safari 2
            //when the script has finished loading
            checkState.delay(2);
        }
        else
        {
	        var loaded = false;
	        
	        if(!Appcelerator.Browser.isOpera) 
            {
                script.onload = checkState; 
            }
            
	        script.onreadystatechange = function()
	        {
	            switch(this.readyState)
	            {
	                case 'loaded':   // state when loaded first time
	                case 'complete': // state when loaded from cache
	                    break;
	                default:
	                    return;
	            }
	            if (loaded) return;
	            loaded = true;
	            
	            // prevent memory leak
	            this.onreadystatechange = null;
	            checkState.defer();
            }	
        }
        Appcelerator.Core.HeadElement.appendChild(script);
        
        if(i+1 < js.length)
        {
            orderedLoad.delay(0, i+1);
        }
    };
    orderedLoad(0);
};

Appcelerator.Core.registerWidgetWithJS = Appcelerator.Core.registerModuleWithJS;


Appcelerator.Core.getLoadedModulesAndAttributes = function() 
{
    return $H(Appcelerator.Module).map(function(kv){
		return [kv[1].getWidgetName(), kv[1].getAttributes().pluck('name')];
	});
};

//
// handlers for when document is loaded or unloaded
//
Appcelerator.Core.onloaders = [];
Appcelerator.Core.onunloaders = [];

/**
 * function for adding f as listener for when the document is loaded
 *
 * @param {function} function to call onload
 * @param {boolean} add to the head or at the end of the stack
 */
Appcelerator.Core.onload = function(f,first)
{
	if (first)
	{
		Appcelerator.Core.onloaders.unshift(f);
	}
	else
	{
		Appcelerator.Core.onloaders.push(f);
	}
};

/**
 * function for adding f as unload listener when the document is unloaded
 *
 * @param {function} function to call on unload
 */
Appcelerator.Core.onunload = function(f)
{
	Appcelerator.Core.onunloaders.push(f);
};

Appcelerator.Core.onloadInvoker = function()
{
	for (var c=0;c<Appcelerator.Core.onloaders.length;c++)
	{
		Appcelerator.Core.onloaders[c]();
	} 
	Appcelerator.Core.onloaders = null;
	
	Logger.info('Appcelerator v'+Appcelerator.Version+' ... loaded in '+(new Date().getTime()-Appcelerator.started.getTime())+' ms');
	Logger.info(Appcelerator.Copyright);
	Logger.info(Appcelerator.LicenseMessage);
	Logger.info('More App. Less Code.');
};
Appcelerator.Core.onunloadInvoker = function()
{
	for (var c=0;c<Appcelerator.Core.onunloaders.length;c++)
	{
		Appcelerator.Core.onunloaders[c]();
	}
	Appcelerator.Core.onunloaders = null;
};

Event.observe(window,'load',Appcelerator.Core.onloadInvoker);
Event.observe(window,'unload',Appcelerator.Core.onunloadInvoker);

/**
 * check the browser support before continuing
 */
Appcelerator.Core.onload(function()
{
    Appcelerator.TEMPLATE_VARS =
    {
        rootPath: Appcelerator.DocumentPath,
        scriptPath: Appcelerator.ScriptPath,
        imagePath: Appcelerator.ImagePath,
        cssPath: Appcelerator.StylePath,
        contentPath: Appcelerator.ContentPath,
        modulePath: Appcelerator.ModulePath,
        instanceid: Appcelerator.instanceid,
		version: Appcelerator.Version,
		codeServer: 'http://code.appcelerator.org/'
    };
 

	if (Appcelerator.Browser.autocheckBrowserSupport && !Appcelerator.Browser.isBrowserSupported)
	{
		document.body.style.display = 'none';
		try
		{
			// attempt to see if we have an upgrade file and if we do
			// just go to it - otherwise, default to just the unsupported text
			new Ajax.Request(Appcelerator.Browser.upgradePath,
			{
				asynchronous:true,
				method:'get',
				onFailure: function(e)
				{
					document.open();
					document.write("<html><head><meta http-equiv='pragma' content='no-cache'></head><body>"+Appcelerator.Browser.unsupportedBrowserMessage+"</body></html>");
					document.close();
					document.body.style.display='';
				},
				onSuccess:function(r)
				{
					// just go to the page directly - makes it cleaner
					document.open();
					document.write(r.responseText);
					document.close();
					document.body.style.display='';
				}
			});
		}
		catch(e)
		{
			document.open();
			document.write("<html><head><meta http-equiv='pragma' content='no-cache'></head><body>"+Appcelerator.Browser.unsupportedBrowserMessage+"</body></html>");
			document.close();
			document.body.style.display='';
		}
	}
    
    //Override document.write
    Appcelerator.Core.old_document_write = document.write;
    document.write = function(src) {
        var re = /src=('([^']*)'|"([^"]*)")/gm;
        re.lastIndex = 0;
        var match = re.exec(src);
        if(match) {
            match = match[2] || match[3];
            //This is for some thing that prototype does.  Without it, 
            //IE6 will crash
            if (match == "//:") 
            {
                Appcelerator.Core.old_document_write(src);
            }
            else 
            {
                Appcelerator.Core.script_count++;
                Appcelerator.Core.remoteLoadScript(match, function(){
                    Appcelerator.Core.script_count--;
                    if (0 == Appcelerator.Core.script_count) {
                        Appcelerator.Core.scriptWithDependenciesCallback();
                    }
                });
            }
        }
    };
});


/**
 * simple function that will set a dotted notation
 * property to the value specified.
 */
Object.setNestedProperty = function (obj, prop, value)
{
    var props = prop.split('.');
    var cur = obj;
    var prev = null;
    var last = null;
    props.each(function(p)
    {
        last = p;
        if (cur[p])
        {
            prev = cur;
            cur = cur[p];
        }
        else
        {
            prev = cur;
            cur = cur[p] = {};
        }
    });
    prev[last] = value;
    return obj;
};


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
 *  var value = Object.getNestedProperty(obj,'foo.foo.jeff')
 *  
 * The value variable should equal 'haynie'
 */
Object.getNestedProperty = function (obj, prop, def)
{
    if (obj!=null && prop!=null)
    {
        var props = prop.split('.');
        if (props.length != -1)
        {
	        var cur = obj;
	        props.each(function(p)
	        {
	            if (null != cur[p])
	            {
	                cur = cur[p];
	            }
	            else
	            {
	                cur = null;
	                throw $break;
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
};

/**
 * copy the properties only (exclude native and functions)
 * from the source to the target. this is very similiar
 * to prototype's extend, except for the exclusion principle
 */
Object.copy = function (target, source)
{
    if (source)
    {
        for (var p in source)
        {
            var obj = source[p];
            var type = typeof(obj);
            switch (type)
            {
                case 'string':
                case 'number':
                case 'boolean':
                case 'object':
                case 'array':
                {
                    target[p] = obj;
                    break;
                }
            }
        }
    }
    return target;
};

/**
 * do an eval with code in the scope putting scope as the 
 * this reference
 */
Object.evalWithinScope = function (code, scope)
{
    if (code == '{}') return {};

	// make sure we escape any quotes given we're building a string with quotes
	var expr = code.gsub('"',"\\\"");
	
    // create the function
    var func = eval('var f = function(){return eval("(' + expr + ')")}; f;');

    // now invoke our scoped eval with scope as the this reference
    return func.call(scope);
};

/**
 * return a formatted message detail for an exception object
 */
Object.getExceptionDetail = function (e,format)
{
    if (!e) return 'No Exception Object';

	if (typeof(e) == 'string')
	{
		return 'message: ' + e;
	}

    if (Appcelerator.Browser.isIE)
    {
        return 'message: ' + e.message + ', location: ' + e.location || e.number || 0;
    }
    else
    {
		var line = 0;
		try
		{
			line = e.lineNumber || 0;
		}
		catch(x)
		{
			// sometimes you'll get a PermissionDenied on certain errors
		}
        return 'message: ' + (e.message || e) + ', location: ' + line + ', stack: ' + (format?'<pre>':'') +(e.stack || 'not specified') + (format?'</pre>':'');
    }
};

/**
 * returns true if object passed in as a boolean
 */
Object.isBoolean = function(object)
{
    return typeof(object)=='boolean';
};

/**
 * Create an object with the given parameter as its prototype.
 * 
 * @param {Object} obj	object to mirror
 */
Object.mirror = function(obj) {
    var mirrorer = (function() {});
    mirrorer.prototype = obj;
    return new mirrorer();
}

/**
 * Clone an object and hide all the fields shared with the given namespace.
 * 
 * @param {Object} obj		object to clone		
 * @param {Object} without	fields to hide
 */
Object.cloneWithout = function(obj, without) {
	var clone = Object.clone(obj);
	for(var name in without) {
		clone[name] = undefined;
	}
	return clone;
}

Appcelerator.Util.makeSet = function(array) {
    var set = {};
    var arrayLen = array.length;
    for(var i = 0; i < arrayLen; i++)
    {
        set[array[i]] = true;
    }
    return set;
};


Appcelerator.Util.DateTime =
{
    ONE_SECOND:1000,
    ONE_MINUTE: 60000,
    ONE_HOUR: 3600000,
    ONE_DAY: 86400000,
    ONE_WEEK: 604800000,
    ONE_MONTH: 18748800000, // this is rough an assumes 31 days
    ONE_YEAR: 31536000000,

	/**
	 * Convert a duration from the format: "2y 3w 5d 27m 13s" into milliseconds
	 */
	timeFormat: function (value)
	{
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
						time+=parseInt(str) * this.ONE_MINUTE;
					}
					str = '';
					break;
				}
				case 's':
				{
					time+=parseInt(str) * this.ONE_SECOND;
					str = '';
					break;
				}
				case 'h':
				{
					time+=parseInt(str) * this.ONE_HOUR;
					str = '';
					break;
				}
				case 'd':
				{
					time+=parseInt(str) * this.ONE_DAY;
					str = '';
					break;
				}
				case 'w':
				{
					time+=parseInt(str) * this.ONE_WEEK;
					str = '';
					break;
				}
				case 'y':
				{
					time+=parseInt(str) * this.ONE_YEAR;
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
	},
    getDurationNoFormat: function (begin, end)
	{
        end = end || new Date();
        var amount = end.getTime() - begin.getTime();

		var hours = 0
		var minutes = 0;
		var seconds = 0;

		if (amount > this.ONE_HOUR)
		{
			hours = Math.round(amount/this.ONE_HOUR);
			amount = amount - (this.ONE_HOUR * hours);
		}
		if (amount == this.ONE_HOUR)
		{
			hours = 1;
			amount = amount - this.ONE_HOUR;
		}
		if (amount > this.ONE_MINUTE)
		{
			minutes = Math.round(amount/this.ONE_MINUTE);
			amount = amount - (this.ONE_MINUTE * minutes);
		}
		if (amount == this.ONE_MINUTE)
		{
			minutes = 1;
			amount = amount - this.ONE_MINUTE;
		}
		if (amount > this.ONE_SECOND)
		{
			seconds = Math.round(amount/this.ONE_SECOND);
			amount = amount - (this.ONE_SECOND * seconds);
		}
		if (amount == this.ONE_SECOND)
		{
			seconds = 1;
		}
		if (seconds <10)
		{
			seconds = "0" + seconds;
		}

		if (hours > 0)
		{
			return hours + ":" + minutes + ":" + seconds;
		}

		if (minutes > 0)
		{
			return minutes + ":" + seconds;
		}

		if (seconds > 0)
		{
			return "0:" + seconds;
		}

		else return ":00";
	},
	getDuration: function (begin, end)
    {
        end = end || new Date();
        var amount = end.getTime() - begin.getTime();

		return this.toDuration(amount);
    },
    toDuration: function (amount)
    {
        amount = amount || 0;

		if (amount < 0)
        {
            return '0';
        }
        else if (amount < this.ONE_SECOND)
        {
            var calc = Math.round(amount / this.ONE_SECOND);
            return calc + ' ms';
        }
        else if (amount == this.ONE_SECOND)
        {
            return '1 second';
        }
        else if (amount < this.ONE_MINUTE)
        {
            var calc = Math.round(amount / this.ONE_SECOND);
            return calc + ' second' + (calc > 1 ? 's' : '');
        }
        if (amount == this.ONE_MINUTE)
        {
            return '1 minute';
        }
        else if (amount == this.ONE_HOUR)
        {
            return '1 hour';
        }
        else if (amount < this.ONE_HOUR)
        {
            var calc = Math.round(amount / this.ONE_MINUTE);
            return calc + ' minute' + (calc > 1 ? 's' : '');
        }
        else if (amount > this.ONE_HOUR && amount < this.ONE_DAY)
        {
            var calc = Math.round(amount / this.ONE_HOUR);
            return calc + ' hour' + (calc > 1 ? 's' : '');
        }
        else if (amount == this.ONE_DAY)
        {
            return '1 day';
        }
        else if (amount > this.ONE_DAY && amount < this.ONE_YEAR)
        {
            if (amount > this.ONE_MONTH)
            {
                var calc = Math.round(amount / this.ONE_MONTH);
                return calc + ' month' + (calc > 1 ? 's' : '');
            }
            else if (amount > this.ONE_WEEK)
            {
                var calc = Math.round(amount / this.ONE_WEEK);
                return calc + ' week' + (calc > 1 ? 's' : '');
            }
            else
            {
                var calc = Math.round(amount / this.ONE_DAY);
                return calc + ' day' + (calc > 1 ? 's' : '');
            }
        }
        else if (amount == this.ONE_YEAR)
        {
            return '1 year';
        }
        else
        {
            var calc = Math.round(amount / this.ONE_DAY);
            return calc + ' years';
        }
    },

    friendlyDiff: function (date, end, shortstr)
    {
        if (!date) return null;
        end = end || new Date();
        var amount = end.getTime() - date.getTime();
        if (amount <= this.ONE_MINUTE)
        {
            if (shortstr)
            {
                return 'a few secs ago';
            }
            return 'a few seconds ago';
        }
        else if (amount < this.ONE_HOUR)
        {
            var calc = Math.round(amount / this.ONE_MINUTE);
            if (calc < 10)
            {
                if (shortstr)
                {
                    return 'a few mins ago';
                }
                return 'a few minutes ago';
            }
            if (shortstr)
            {
                return calc + ' min' + (calc > 1 ? 's' : '') + ' ago';
            }
            return calc + ' minute' + (calc > 1 ? 's' : '') + ' ago';
        }
        else if (amount == this.ONE_HOUR)
        {
            return 'an hour ago';
        }
        else if (amount < this.ONE_DAY)
        {
            var calc = Math.round(amount / this.ONE_HOUR);
            return calc + ' hour' + (calc > 1 ? 's' : '') + ' ago';
        }
        else if (amount == this.ONE_DAY)
        {
            return 'yesterday';
        }
        else if (amount > this.ONE_DAY && amount < this.ONE_YEAR)
        {
            if (amount > this.ONE_MONTH)
            {
                var calc = Math.round(amount / this.ONE_MONTH);
                return calc + ' month' + (calc > 1 ? 's' : '') + ' ago';
            }
            else if (amount > this.ONE_WEEK)
            {
                var calc = Math.round(amount / this.ONE_WEEK);
                return calc + ' week' + (calc > 1 ? 's' : '') + ' ago';
            }
            else
            {
                var calc = Math.round(amount / this.ONE_DAY);
                if (calc == 1)
                {
                    return 'yesterday';
                }
                return calc + ' day' + (calc > 1 ? 's' : '') + ' ago';
            }
        }
        else if (amount > this.ONE_YEAR)
        {
            if (shortstr)
            {
                return '>1 year ago';
            }
            return 'more than a year ago';
        }
        return amount;
    },

    getMonthIntValue: function(s)
    {
        switch (s)
                {
            case "Jan":
                return 0;
            case "Feb":
                return 1;
            case "Mar":
                return 2;
            case "Apr":
                return 3;
            case "May":
                return 4;
            case "Jun":
                return 5;
            case "Jul":
                return 6;
            case "Aug":
                return 7;
            case "Sep":
                return 8;
            case "Oct":
                return 9;
            case "Nov":
                return 10;
            case "Dec":
                return 11;
        }
    },
    intval: function(s)
    {
        if (s)
        {
            if (s.charAt(0) == '0')
            {
                s = s.substring(1);
            }
            return parseInt(s);
        }
        return 0;
    },
/**
 * return a friendly date - can pass a string or date object. if string, must be in the
 * RFC2822 format
 */
    getFriendlyDate: function (date, qualifiers)
    {
        //allow specifiers to be appended before returning based on format
        var q = Object.extend(
        {
            today: '',
            yesterday: '',
            past: '',
            lowercase: false
        }, qualifiers || "");

        date = date || new Date();
        var localDate = typeof(date) == "string" ? Appcelerator.Util.DateTime.parseRFC2822Date(date) : date;
        if (Appcelerator.Util.DateTime.isToday(localDate))
        {
            return q['today'] + Appcelerator.Util.DateTime.get12HourTime(localDate);
        }
        else if (Appcelerator.Util.DateTime.isYesterday(localDate))
        {
            return q['yesterday'] + (q['lowercase'] ? "yesterday" : "Yesterday");
        }
        return q['past'] + localDate.getDay() + " " + Appcelerator.Util.DateTime.getShortMonthName(localDate);
    },
/**
 * given a date object, return true if the
 * date is today (not necessary in the same
 * hour, but in the same day period)
 */
    isToday: function (date)
    {
        var now = new Date();
        return (now.getFullYear() == date.getFullYear() &&
                now.getMonth() == date.getMonth() &&
                now.getDay() == date.getDay());
    },
/**
 * given a date object, return true if the
 * date is yesterday (not necessary in the same
 * hour, but in the same day period)
 */
    isYesterday: function (date)
    {
        var now = new Date();
        return (now.getFullYear() == date.getFullYear() &&
                now.getMonth() == date.getMonth() &&
                now.getDay() - 1 == date.getDay());
    },
    getShortMonthName: function (date)
    {
        var month = date.getMonth();
        switch (month)
                {
            case 0:
                return "Jan";
            case 1:
                return "Feb";
            case 2:
                return "Mar";
            case 3:
                return "Apr";
            case 4:
                return "May";
            case 5:
                return "Jun";
            case 6:
                return "Jul";
            case 7:
                return "Aug";
            case 8:
                return "Sep";
            case 9:
                return "Oct";
            case 10:
                return "Nov";
            case 11:
                return "Dec";
        }
    },
    get12HourTime: function (d, seconds, milli)
    {
        var date = d;
        if (date == null)
        {
            date = new Date();
        }
        var hour = date.getHours();
        var str = (hour == 0) ? 12 : hour;
        var ampm = "AM";
        if (hour >= 12)
        {
            // convert to 12-hour clock
            str = (hour == 12) ? 12 : (hour - 12);
            ampm = "PM";
        }
        var m = date.getMinutes();
        var s = date.getSeconds();
        str += ":" + (m < 10 ? "0" : "") + m;
        if (seconds)
        {
            str += ":" + (s < 10 ? "0" : "") + s;
        }
        if (milli)
        {
            var ms = date.getMilliseconds();
            str += "." + (ms < 10 ? "0" : "") + ms;
        }
        str += " " + ampm;
        return str;
    },
    getShortDateTime: function (d)
    {
        if (d && typeof(d) == 'string')
        {
            d = this.parseJavaDate(d);
        }
        var date = d || new Date();
        return this.getShortMonthName(date) + " " + date.getDate() + " " + this.get12HourTime(date);
    },
    parseInt: function (x)
    {
        if (x)
        {
            // parseInt with a leading 0 returns 0 instead of the value
            // so we chop it off
            if (x.charAt(0) == '0')
            {
                return this.parseInt(x.substring(1));
            }
            return parseInt(x);
        }
        return 0;
    },
/**
 * given a date in the format: 2006-09-21 22:47:20.0 return a javascript
 * date object. the string format is the same as return when you return
 * a Java Date object with toString()
 *
 */
    parseJavaDate: function (d)
    {
        if (!d || d == '') return null;

        //012345678901234567890
        //2006-09-21 22:47:20.0
        var year = parseInt(d.substring(0, 4));
        // note - javascript month is 0-11 not 1-12
        var month = this.parseInt(d.substring(5, 7)) - 1;
        var day = this.parseInt(d.substring(8, 10));
        var hour = this.parseInt(d.substring(11, 13));
        var minute = this.parseInt(d.substring(14, 16));
        var second = this.parseInt(d.substring(17, 19));
        var milli = this.parseInt(d.substring(20));
        var date = new Date();
        date.setFullYear(year);
        date.setMonth(month);
        date.setDate(day);
        date.setHours(hour);
        date.setMinutes(minute);
        date.setSeconds(second);
        date.setMilliseconds(milli);
        return date;
    },
/**
 * given an RFC 2822 formatted date string, return
 * a converted Date object.
 */
    parseRFC2822Date: function (d)
    {
        // EXAMPLE: Wed, 07 Jun 2006 09:03:53 -0700

        var tokens = d.split(" ");

        var day = this.intval(tokens[1]);
        var month = tokens[2];
        var year = tokens[3];

        var timetokens = tokens[4].split(":");
        var hour = this.intval(timetokens[0]);
        var min = this.intval(timetokens[1]);
        var sec = this.intval(timetokens[2]);

        // example: -0700
        // The first two digits indicate the number of hours
        // difference from Universal Time, and the last two digits indicate the
        // number of minutes difference from Universal Time.
        var hourOffset = this.intval(tokens[5].substring(0, 3));

        // convert to GMT (if we're behind GMT, we need to add
        // and if we're ahead of GMT, we need to subtract)
        var gmtHour = 0;
        if (hourOffset < 0)
        {
            gmtHour = hour - (hourOffset);
        }
        else
        {
            gmtHour = hour + (hourOffset);
        }
        if (gmtHour > 23)
        {
            // we've lapsed into the next day
            gmtHour = gmtHour - 24;
            day += 1;
        }
        else if (gmtHour < 0)
        {
            // we've lapsed into the previous day
            gmtHour = 24 - gmtHour;
            day -= 1;
        }
        var gmtDate = new Date();
        gmtDate.setUTCDate(day);
        gmtDate.setUTCMonth(this.getMonthIntValue(month));
        gmtDate.setUTCFullYear(year);
        gmtDate.setUTCHours(gmtHour, min, sec);

        return gmtDate;
    },
    //Returns the client machine's timezone offset from GMT
    //Format is something like -0400 or +0815
    getTimezoneOffset: function()
	{
		var curdate = new Date();
		var offset = curdate.getTimezoneOffset();
		var hours = Math.floor(offset/60);
		var modMin = Math.abs(offset%60);
		var s = new String();
		s += (hours > 0) ? "-" : "+";
		var absHours = Math.abs(hours);
		s += (absHours < 10) ? "0" + absHours :absHours;
		s += ((modMin == 0) ? "00" : modMin);
		return s;
	},
	
	parse: function(date, format)
	{
	    return Jel.Date.parse(date,format);
	},
	format: function(date, format)
	{
	    return Jel.Date.format(date, format);
	}
};Appcelerator.ServerConfig = {};

Appcelerator.Util.ServerConfig = Class.create();
Appcelerator.Util.ServerConfig.listeners = [];

Appcelerator.Util.ServerConfig.addConfigListener = function(listener)
{
	if (Appcelerator.Util.ServerConfig.listeners)
	{
		Appcelerator.Util.ServerConfig.listeners.push(listener);
	}
	else
	{
		listener();
	}
};

/**
 * if this value is set, we will not auto fetch the appcelerator.xml
 * and you will need to call Appcelerator.Util.ServerConfig.set 
 * programatically
 */
Appcelerator.Util.ServerConfig.disableRemoteConfig = false;

/**
 * call this function when you want to manually configure the server
 * config from javascript vs. fetching automatically from AJAX request
 */
Appcelerator.Util.ServerConfig.set = function(config)
{
    Appcelerator.ServerConfig = config;
    Appcelerator.Util.ServerConfig.loadComplete();
};

Appcelerator.Util.ServerConfig.load = function()
{
	if (window.location.href.indexOf('file:/')==-1)
	{
	    if (Appcelerator.Util.ServerConfig.disableRemoteConfig)
	    {
	       return;
	    }
		var xmlPath = Appcelerator.DocumentPath + 'appcelerator.xml';
			
		new Ajax.Request(xmlPath,
		{
		 	asynchronous: true,
		    method: 'get',
			onSuccess:function (resp)
			{
				var xml = resp.responseXML.documentElement;
				var children = xml.childNodes;
				for (var c=0;c<children.length;c++)
				{
					var child = children[c];
					if (child.nodeType == Appcelerator.Util.Dom.ELEMENT_NODE)
					{
						var service = child.nodeName.toLowerCase();
						var config = {};
						var path = Appcelerator.Util.Dom.getText(child);
						var template = new Template(path,/(^|.|\r|\n)(@\{(.*?)\})/);
						config.path = template.evaluate(Appcelerator.TEMPLATE_VARS);
						// keep path for backwards compatability
						config.value = config.path;
						Appcelerator.Util.Dom.eachAttribute(child,function(k,v)
						{
							config[k]=v;
						},['id'],true);
						Appcelerator.ServerConfig[service] = config;
					}
				}
				Appcelerator.Util.ServerConfig.loadComplete();
			},
			onFailure:function(r)
			{
				$E('Failed to load configuration from '+xmlPath);
				Appcelerator.Util.ServerConfig.loadComplete();
			}
		});
	}
	else
	{
		Appcelerator.Util.ServerConfig.loadComplete();
	}
};

Appcelerator.Util.ServerConfig.loadComplete = function()
{
	Appcelerator.Util.ServerConfig.listeners.each (function(l)
	{	
		try 
		{
			l();
		}
		catch (e)
		{
			$E('error calling server config listener = '+l+', Exception = '+Object.getExceptionDetail(e));
		}
	});
	Appcelerator.Util.ServerConfig.listeners = null;
};

Appcelerator.Util.ServerConfig.addConfigListener(function ()
{
    Appcelerator.Util.ServerConfig.setValues('cookie_check', true);
    Appcelerator.Util.ServerConfig.setValues('browser_check', true);
    Appcelerator.Util.ServerConfig.setValues('hide_body', false);
    Appcelerator.Util.ServerConfig.setValues('perfmon', false);
    Appcelerator.Util.ServerConfig.setValues('usegears', true);
    Appcelerator.Util.ServerConfig.setValues('report_stats', true);
    Appcelerator.Util.ServerConfig.setValues('track_stats', true);

	// allow the user to override our defaults
	if (window.AppceleratorConfig)
	{
		for (var key in window.AppceleratorConfig)
		{
			var value = window.AppceleratorConfig[key];
			Appcelerator.Util.ServerConfig.setValues(key,value);
		}
	}

	Appcelerator.Browser.autocheckBrowserSupport = Appcelerator.Config['browser_check'];
	Appcelerator.Browser.autoReportStats = Appcelerator.Config['report_stats'];
});

Appcelerator.Util.ServerConfig.setValues = function(key, def)
{
    if (Appcelerator.ServerConfig[key])
    {
        Appcelerator.Config[key] = Appcelerator.ServerConfig[key].value;
    }
    else if (Appcelerator.Parameters.get(key))
    {
        var param = Appcelerator.Parameters.get(key);
        switch (param)
        {
            case 'true':
            {
                Appcelerator.Config[key] = true;
                break;
            }
            case 'false':
            {
                Appcelerator.Config[key] = false;
                break;
            }
            default:
            {
                Appcelerator.Config[key] = Appcelerator.Parameters.get(key);
            }
        }
    }
    else
    {
        Appcelerator.Config[key] = def;
    }
}

Appcelerator.Core.onload(Appcelerator.Util.ServerConfig.load);/**
 * Traverses the document, starting at the body node.
 *
 * As it encounters widget tags, it fires off async requests for
 * the modules' code. As modules finish fetching they callback
 * listeners which call Appcelerator.Compiler.compileWidget
 * to construct the widget in HTML.
 *
 * A.C.compileWidget dispatches widget construction to each module,
 * and then uses flags (called 'instructions') returned by the module
 * to complete the widget building/compiling process.
 *
 * @fileOverview a set of functions related to compiling a DOM with web expressions into resulting DHTML/javascript/AJAX
 * @name Appcelerator.Compiler
 */

/**
 * this should be set if you want the document to be
 * compiled when loaded - otherwise, it must be manually compiled
 */
Appcelerator.Compiler.compileOnLoad = true;

/**
 * returns true if running in interpretive mode
 * compilation is done at runtime in the browser
 *
 * @deprecated no longer used
 */
Appcelerator.Compiler.isInterpretiveMode = true;

/**
 * returns true if running in compiled mode
 * compilation is done at deployment time
 *
 * @deprecated no longer used
 */
Appcelerator.Compiler.isCompiledMode = false;


Appcelerator.Compiler.POSITION_REMOVE = -1;
Appcelerator.Compiler.POSITION_REPLACE = 0;
Appcelerator.Compiler.POSITION_TOP = 1;
Appcelerator.Compiler.POSITION_BOTTOM = 2;
Appcelerator.Compiler.POSITION_BEFORE = 3;
Appcelerator.Compiler.POSITION_AFTER = 4;
Appcelerator.Compiler.POSITION_BODY_TOP = 5;
Appcelerator.Compiler.POSITION_BODY_BOTTOM = 6;
Appcelerator.Compiler.POSITION_HEAD_TOP = 7;
Appcelerator.Compiler.POSITION_HEAD_BOTTOM = 8;

Appcelerator.Compiler.nextId = 0;
Appcelerator.Compiler.functionId = 1;

/**
 * check an element for an ID and ensure that if it doesn't have one,
 * it will automatically generate a system-generated unique ID
 *
 * @param {element} element element to check
 * @return {string} return the id of the element
 */
Appcelerator.Compiler.getAndEnsureId = function(element)
{
	if (!element.id)
	{
		element.id = Appcelerator.Compiler.generateId();
	}
	if (!element._added_to_cache)
	{
	    Appcelerator.Compiler.setElementId(element,element.id);
    }
	return element.id;
};

/**
 * set an elements ID attribute. an variable in the global scope
 * will be created that references the element object in the format
 * $ID such as if you have an element named foo you can reference the
 * element directly with the global variable named $foo.
 *
 * @param {element} element to set ID
 * @param {string} id of the element
 * @return {element} element
 */
Appcelerator.Compiler.setElementId = function(element, id)
{
	Appcelerator.Compiler.removeElementId(element.id);
    element.id = id;
    element._added_to_cache = true;
    // set a global variable to a reference to the element
    // which now allows you to do something like $myid in javascript
    // to reference the element
    window['$'+id]=element;
    return element;
};

/**
 * removes an element ID attribute from the global cache and
 * delete the auto-generated global variable
 *
 * @param {string} id id of the element to delete
 * @return {boolean} true if found or false if not found
 */
Appcelerator.Compiler.removeElementId = function(id)
{
	if (id)
	{
		var element_var = window['$'+id];
		if (element_var)
		{
			try
			{
				delete window['$'+id];
			}
			catch(e)
			{
				window['$'+id] = 0;
			}
			if (element_var._added_to_cache)
			{
				try
				{
				    delete element_var._added_to_cache;
				}
				catch (e)
				{
					element_var._added_to_cache = 0;
				}
			}
			return true;
		}
	}
	return false;
};

/**
 * we're doing to redefine prototype's $ function to do some special
 * processing - we delete it first
 *
 * @private
 */
(function()
{
	if (Object.isFunction(window['$']))
	{
	    try
	    {
	        delete window['$'];
	    }
	    catch (e)
	    {
	    }
	}
})();


/**
 * redefined $ function from prototype which is optimized to lookup
 * the element first by checking the global variable for the element
 *
 * @param {string} element can either by array of string ids, single strip or element
 * @return {element} element object or null if not found
 */
function $(element)
{
	if (arguments.length > 1)
	{
	    var args = $A(arguments);
    	return args.collect(function(a)
		{
			return $(a);
		});
 	}

	if (Object.isString(element))
	{
	    if(element == '')
	    {
	        // otherwise $('') == $, which breaks code doing null testing
	        return null;
	    }
		var id = element;
		element = window['$'+id];
		if (!element)
		{
			element = document.getElementById(id);
		}
	}

	return element ? Element.extend(element) : null;
}

/**
 * generate a unique ID
 *
 * @return {string} id that can be used only once
 */
Appcelerator.Compiler.generateId = function()
{
	return 'app_' + (Appcelerator.Compiler.nextId++);
};

/**
 * @property {hash} has of key which is name of element (or * for all elements) and array
 * of attribute processors that should be called when element is encountered
 */
Appcelerator.Compiler.attributeProcessors = {'*':[]};

/**
 * Register an object that has a <b>handle</b> method which takes
 * an element, attribute name, and attribute value of the processed element.
 *
 * This method takes the name of the element (or optionally, null or * as
 * a wildcard) and an attribute (required) value to look for on the element
 * and a listener.
 *
 * @param {string} name of attribute processor. can be array of strings for multiple elements or * for wildcard.
 * @param {string} attribute to check when matching element
 * @param {function} listener to call when attribute is matched on element
 */
Appcelerator.Compiler.registerAttributeProcessor = function(name,attribute,listener)
{
	if (typeof name == 'string')
	{
		name = name||'*';
		var a = Appcelerator.Compiler.attributeProcessors[name];
		if (!a)
		{
			a = [];
			Appcelerator.Compiler.attributeProcessors[name]=a;
		}
		// always push to the end such that custom attribute processors will be 
		// processed before internal ones so that they can overwrite builtins
		a.unshift([attribute,listener]);
	}
	else
	{
		for (var c=0,len=name.length;c<len;c++)
		{
			var n = name[c]||'*';
			var a = Appcelerator.Compiler.attributeProcessors[n];
			if (!a)
			{
				a = [];
				Appcelerator.Compiler.attributeProcessors[n]=a;
			}
			// always push to the end such that custom attribute processors will be 
			// processed before internal ones so that they can overwrite builtins
			a.unshift([attribute,listener]);
		}
	}
};

/**
 * called internally by compiler to dispatch details to attribute processors
 *
 * @param {element} element
 * @param {array} array of processors
 */
Appcelerator.Compiler.forwardToAttributeListener = function(element,array)
{
    for (var i=0;i<array.length;i++)
	{
		var entry = array[i];
		var attributeName = entry[0];
		var listener = entry[1];
		var value = element.getAttribute(attributeName);
        if (value) // optimization to avoid adding listeners if the attribute isn't present
        {
            listener.handle(element,attributeName,value);
        }
    }
};

/**
 * internal method called to process each element and potentially one or
 * more attribute processors
 *
 * @param {element} element
 */
Appcelerator.Compiler.delegateToAttributeListeners = function(element)
{
	var tagname = Appcelerator.Compiler.getTagname(element);
	var p = Appcelerator.Compiler.attributeProcessors[tagname];
	if (p && p.length > 0)
	{
		Appcelerator.Compiler.forwardToAttributeListener(element,p,tagname);
	}
	p = Appcelerator.Compiler.attributeProcessors['*'];
	if (p && p.length > 0)
	{
		Appcelerator.Compiler.forwardToAttributeListener(element,p,'*');
	}
};

Appcelerator.Compiler.containerProcessors=[];

/**
 * add a listener that is fired when a container is created
 *
 * @param {function} listener which is a container processor
 */
Appcelerator.Compiler.addContainerProcessor = function(listener)
{
	Appcelerator.Compiler.containerProcessors.push(listener);
};

/**
 * remove container listener
 *
 * @param {function} listener to remove
 */
Appcelerator.Compiler.removeContainerProcessor = function(listener)
{
	Appcelerator.Compiler.containerProcessors.remove(listener);
};

/**
 * called when a container is created
 *
 * @param {element} element being compiled
 * @param {element} container
 */
Appcelerator.Compiler.delegateToContainerProcessors = function(element,container)
{
	if (Appcelerator.Compiler.containerProcessors.length > 0)
	{
		for (var c=0,len=Appcelerator.Compiler.containerProcessors.length;c<len;c++)
		{
			var processor = Appcelerator.Compiler.containerProcessors[c];
			var r = processor.process(element,container);
			if (r)
			{
				container = r;
			}
		}
	}
	return container;
};

/**
 * List of attributes to copy from the original <app:widget_name> element
 * onto the <div> that replaces that widget.
 *
 * Needed for the 'bind' action and 'selectable' attribute.
 */
Appcelerator.Compiler.retainedWidgetAttributes = ['name'];
Appcelerator.Compiler.addContainerProcessor(
{
	process: function(element,container)
	{
	    var attrs = Appcelerator.Compiler.retainedWidgetAttributes;
	    for(var i = 0; i < attrs.length; i++)
	    {
	        var attr = attrs[i];
    		var v = element.getAttribute(attr);
    		if (v)
    		{
    			container.setAttribute(attr,v);
    		}
    	}
	}
});


Appcelerator.Compiler.checkLoadState = function (element)
{
	var state = element.state;
	if (state && state.pending==0 && state.scanned)
	{
		if (typeof(state.onfinish)=='function')
		{
			state.onfinish(code);
		}

		if (typeof(state.onafterfinish)=='function')
		{
			state.onafterfinish();
		}
		Appcelerator.Compiler.removeState(element);
		return true;
	}
	return false;
};

/**
 * call this to dynamically compile a widget on-the-fly and evaluate
 * any widget JS code as compiled
 *
 * @param {element} element to compile
 * @param {boolean} notimeout immediately process or false (default) to process later
 * @param {boolean} recursive compile child elements
 */
Appcelerator.Compiler.dynamicCompile = function(element,notimeout,recursive)
{
	if (!element) return;

    $D('dynamic compile called for ',element,' - id=',element.id);

    Appcelerator.Compiler.doCompile(element,recursive);
};

Appcelerator.Compiler.doCompile = function(element,recursive)
{
    var state = Appcelerator.Compiler.createCompilerState();
    Appcelerator.Compiler.compileElement(element,state,recursive);
    state.scanned = true;
    Appcelerator.Compiler.checkLoadState(element);
};

Appcelerator.Compiler.removeState = function(element)
{
	if (element.state)
	{
		try 
		{
			delete element.state;
		}
		catch (e)
		{
			element.state = null;
		}
	}
};

Appcelerator.Compiler.createCompilerState = function ()
{
	return {pending:0,scanned:false};
};

Appcelerator.Compiler.onbeforecompileListeners = [];
Appcelerator.Compiler.oncompileListeners = [];
Appcelerator.Compiler.beforeDocumentCompile = function(l)
{
	Appcelerator.Compiler.onbeforecompileListeners.push(l);
};
Appcelerator.Compiler.afterDocumentCompile = function(l)
{
    Appcelerator.Compiler.oncompileListeners.push(l);
};

/**
 * main entry point for compiler to compile document DOM
 *
 * @param {function} onFinishCompiled function to call (or null) when document is finished compiling
 */
Appcelerator.Compiler.compileDocument = function(onFinishCompiled)
{
    $D('compiled document called');

    if (Appcelerator.Compiler.onbeforecompileListeners)
    {
       for (var c=0;c<Appcelerator.Compiler.onbeforecompileListeners.length;c++)
       {
           Appcelerator.Compiler.onbeforecompileListeners[c]();
       }
       delete Appcelerator.Compiler.onbeforecompileListeners;
    }

    var container = document.body;
    var originalVisibility = container.style.visibility || 'visible';

	if (Appcelerator.Config['hide_body'])
	{
	    container.style.visibility = 'hidden';
	}

    if (!document.body.id)
    {
        Appcelerator.Compiler.setElementId(document.body, 'app_body');
    }

    var state = Appcelerator.Compiler.createCompilerState();
	container.state = state;
	
    // start scanning at the body
    Appcelerator.Compiler.compileElement(container,state);

    // mark it as complete and check the loading state
    state.scanned = true;
    state.onafterfinish = function(code)
    {
        (function()
        {
		    if (typeof(onFinishCompiled)=='function') onFinishCompiled();
		    if (originalVisibility!=container.style.visibility)
		    {
		       container.style.visibility = originalVisibility;
		    }
			Appcelerator.Compiler.compileDocumentOnFinish();
        }).defer();
    };
    Appcelerator.Compiler.checkLoadState(container);
};

Appcelerator.Compiler.compileDocumentOnFinish = function ()
{
    if (Appcelerator.Compiler.oncompileListeners)
    {
        for (var c=0;c<Appcelerator.Compiler.oncompileListeners.length;c++)
        {
            Appcelerator.Compiler.oncompileListeners[c]();
        }
        delete Appcelerator.Compiler.oncompileListeners;
    }
    $MQ('l:app.compiled');
}

Appcelerator.Compiler.compileInterceptors=[];

Appcelerator.Compiler.addCompilationInterceptor = function(interceptor)
{
	Appcelerator.Compiler.compileInterceptors.push(interceptor);
};

Appcelerator.Compiler.onPrecompile = function (element)
{
	if (Appcelerator.Compiler.compileInterceptors.length > 0)
	{
		for (var c=0,len=Appcelerator.Compiler.compileInterceptors.length;c<len;c++)
		{
			var interceptor = Appcelerator.Compiler.compileInterceptors[c];
			if (interceptor.onPrecompile(element)==false)
			{
				return false;
			}
		}
	}
	return true;
};

Appcelerator.Compiler.compileElement = function(element,state,recursive)
{
    recursive = recursive==null ? true : recursive;

	Appcelerator.Compiler.getAndEnsureId(element);
	Appcelerator.Compiler.determineScope(element);

    $D('compiling element => '+element.id);

	if (typeof(state)=='undefined')
	{
		throw "compileElement called without state for "+element.id;
	}

	Appcelerator.Compiler.onPrecompile(element);

	// check to see if we should compile
	var doCompile = element.getAttribute('compile') || 'true';
	if (doCompile == 'false')
	{
		return;
	}

    if (Appcelerator.Compiler.isInterpretiveMode)
    {
        if (element.compiled)
        {
           Appcelerator.Compiler.destroy(element);
        }
        element.compiled = 1;
    }

	//TODO: fix this - we need to remove this from element
	
	element.state = state;

	try
	{
		var name = Appcelerator.Compiler.getTagname(element);
		var kind = element.getAttribute('kind');  //FIXME: deprecate this
		if (name.indexOf(':')>0)
		{
			element.style.originalDisplay = element.style.display || 'block';

	        state.pending+=1;
			Appcelerator.Core.requireModule(name,function()
			{
				Appcelerator.Compiler.compileWidget(element,state);
				state.pending-=1;
				Appcelerator.Compiler.checkLoadState(element);
				Element.fire(element,'element:compiled:'+element.id,{id:element.id});
			});
		}
		else if(kind && kind.length > 0)
		{
			var name = 'app:'+kind;
			state.pending += 1;
			Appcelerator.Core.requireModule(name,function()
			{
				var widgetJS = Appcelerator.Compiler.compileWidget(element,state,name);
				state.pending -= 1;
				Appcelerator.Compiler.checkLoadState(state);
				Element.fire(element,'element:compiled:'+element.id,{id:element.id});
			});
		}
		else
		{
			Appcelerator.Compiler.delegateToAttributeListeners(element);

			if (recursive && !element.stopCompile)
	        {
				Appcelerator.Compiler.compileElementChildren(element);
				Element.fire(element,'element:compiled:'+element.id,{id:element.id});
			}
		}
	}
	catch(e)
	{
		Appcelerator.Compiler.handleElementException(element, e, 'compiling ' + element.id);
	}
};

Appcelerator.Compiler.compileElementChildren = function(element)
{
	if (element && element.nodeType == 1)
	{
		if (element.nodeName.toLowerCase() != 'textarea')
		{
			var elementChildren = Appcelerator.Compiler.getElementChildren(element);
			for (var i=0,len=elementChildren.length;i<len;i++)
			{
	            Appcelerator.Compiler.compileElement(elementChildren[i],element.state);
			}
		}
		Appcelerator.Compiler.checkLoadState(element);
		Element.fire(element,'element:compiled:'+element.id,{id:element.id});
	}
};

Appcelerator.Compiler.getElementChildren = function (element)
{
    var elementChildren = [];
	if (element && element.nodeType == 1)
	{
		for (var i = 0, length = element.childNodes.length; i < length; i++)
		{
		    if (element.childNodes[i].nodeType == 1)
		    {
	    	     elementChildren.push(element.childNodes[i]);
	    	}
		}
	}
	return elementChildren;
}

/**
 * method should be called to clean up any listeners or internally added stuff
 * the compiler places into the element
 *
 * @param {element} element to destroy
 * @param {boolean} recursive should be destroy element's children as well (defaults to true)
 */
Appcelerator.Compiler.destroy = function(element, recursive)
{
	if (!element) return;
	recursive = recursive==null ? true : recursive;

	element.compiled = 0;

	Appcelerator.Compiler.removeElementId(element.id);

	if (Object.isArray(element.trashcan))
	{
		for (var c=0,len=element.trashcan.length;c<len;c++)
		{
			try
			{
				element.trashcan[c]();
			}
			catch(e)
			{
				$D(e);
			}
		}
		try
		{
			delete element.trashcan;
		}
		catch(e)
		{
			$D(e);
		}
	}

	if (recursive)
	{
		if (element.nodeType == 1 && element.childNodes && element.childNodes.length > 0)
		{
			for (var c=0,len=element.childNodes.length;c<len;c++)
			{
				var node = element.childNodes[c];
				if (node && node.nodeType && node.nodeType == 1)
				{
					try
					{
						Appcelerator.Compiler.destroy(node,true);
					}
					catch(e)
					{
					    $E(e);
					}
				}
			}
		}
	}
};

Appcelerator.Compiler.destroyContent = function(element)
{
	var elementChildren = Appcelerator.Compiler.getElementChildren(element);
	for (var i=0,len=elementChildren.length;i<len;i++)
	{
	    Appcelerator.Compiler.destroy(elementChildren[i], true);
	}
};

Appcelerator.Compiler.addTrash = function(element,trash)
{
	if (!element.trashcan)
	{
		element.trashcan = [];
	}
	element.trashcan.push(trash);
};

Appcelerator.Compiler.getJsonTemplateVar = function(namespace,var_expr,template_var)
{
    var def = {};
    var o = Object.getNestedProperty(namespace,var_expr,def);

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
        o = Object.toJSON(o).replace(/"/g,'&quot;');
    }
    return o;
}

Appcelerator.Compiler.templateRE = /#\{(.*?)\}/g;
Appcelerator.Compiler.compileTemplate = function(html,htmlonly,varname)
{
	varname = varname==null ? 'f' : varname;

	var fn = function(m, name, format, args)
	{
		return "', jtv(values,'"+name+"','#{"+name+"}'),'";
	};
	var body = "var "+varname+" = function(values){ var jtv = Appcelerator.Compiler.getJsonTemplateVar; return ['" +
            html.replace(/(\r\n|\n)/g, '').replace(/\t/g,' ').replace(/'/g, "\\'").replace(Appcelerator.Compiler.templateRE, fn) +
            "'].join('');};" + (htmlonly?'':varname);

	var result = htmlonly ? body : eval(body);

	return result;
};

Appcelerator.Compiler.tagNamespaces = [];
for (var x=0;x<document.documentElement.attributes.length;x++)
{
    var attr = document.documentElement.attributes[x];
    var name = attr.name;
    var value = attr.value;
    var idx = name.indexOf(':');
    if (idx > 0)
    {
        Appcelerator.Compiler.tagNamespaces.push(name.substring(idx+1));
    }
}

if (Appcelerator.Compiler.tagNamespaces.length == 0)
{
	Appcelerator.Compiler.tagNamespaces.push('app');
}

Appcelerator.Compiler.removeHtmlPrefix = function(html)
{
    if (Appcelerator.Browser.isIE)
    {
        html = html.gsub(/<\?xml:namespace(.*?)\/>/i,'');
    }
    return html.gsub(/html:/i,'').gsub(/><\/img>/i,'/>');
};

/**
 * this super inefficient but nifty function will
 * parse our HTML: namespace tags required when HTML is
 * included in APP: tags but as long as they are
 * not within a APP: tags content.  The browser
 * doesn't like HTML: (he'll think it's a non-HTML tag)
 * so we need to strip the HTML: before passing to browser
 * as long as it's outside a appcelerator widget
 *
 * @param {string} html string to parse
 * @param {prefix} prefix to remove
 * @return {string} content with prefixes properly removed
 */
Appcelerator.Compiler.specialMagicParseHtml = function(html,prefix)
{
    var newhtml = html;
    for (var c=0;c<Appcelerator.Compiler.tagNamespaces.length;c++)
    {
        newhtml = Appcelerator.Compiler.specialMagicParseTagSet(newhtml,Appcelerator.Compiler.tagNamespaces[c]);
    }
    return newhtml;
};
Appcelerator.Compiler.specialMagicParseTagSet = function(html,prefix)
{
    var beginTag = '<'+prefix+':';
    var endTag = '</'+prefix+':';

    var idx = html.indexOf(beginTag);
    if (idx < 0)
    {
        return Appcelerator.Compiler.removeHtmlPrefix(html);
    }

    var myhtml = Appcelerator.Compiler.removeHtmlPrefix(html.substring(0,idx));

    var startIdx = idx + beginTag.length;

    var tagEnd = html.indexOf('>',startIdx);

    var tagSpace = html.indexOf(' ',startIdx);
    if (tagSpace<0 || tagEnd<tagSpace)
    {
        tagSpace=tagEnd;
    }
    var tagName = html.substring(startIdx,tagSpace);
    var endTagName = endTag+tagName+'>';

    while ( true )
    {
        var lastIdx = html.indexOf(endTagName,startIdx);
        var endTagIdx = html.indexOf('>',startIdx);
        var lastTagIdx = html.indexOf('>',lastIdx);
        var content = html.substring(endTagIdx+1,lastIdx);
        // check to see if we're within a nested element of the same name
        var dupidx = content.indexOf(beginTag+tagName);
        if (dupidx!=-1)
        {
            startIdx=lastIdx+endTagName.length;
            continue;
        }
        var specialHtml = html.substring(idx,lastIdx+endTagName.length);
        if (Appcelerator.Browser.isIE)
        {
            specialHtml = Appcelerator.Compiler.removeHtmlPrefix(specialHtml);
        }
        myhtml+=specialHtml;
        break;
    }

    myhtml+=Appcelerator.Compiler.specialMagicParseHtml(html.substring(lastTagIdx+1),prefix);
    return myhtml;
};

Appcelerator.Compiler.copyAttributes = function (sourceElement, targetElement)
{
	Appcelerator.Util.Dom.eachAttribute(sourceElement,function(name,value,specified,idx,len)
	{
		if (specified) targetElement.setAttribute(name,value);
	},['style','class','id']);
};

Appcelerator.Compiler.getHtml = function (element,convertHtmlPrefix)
{
	convertHtmlPrefix = (convertHtmlPrefix==null) ? true : convertHtmlPrefix;

	var html = element.innerHTML || Appcelerator.Util.Dom.getText(element);

	return Appcelerator.Compiler.convertHtml(html, convertHtmlPrefix);
};

Appcelerator.Compiler.addIENameSpace = function (html)
{
	if (Appcelerator.Browser.isIE)
	{
		html = '<?xml:namespace prefix = app ns = "http://www.appcelerator.org" /> ' + html;
	}
	return html;
};


Appcelerator.Compiler.convertHtml = function (html, convertHtmlPrefix)
{
	// convert funky url-encoded parameters escaped
	if (html.indexOf('#%7B')!=-1)
	{
	   html = html.gsub('#%7B','#{').gsub('%7D','}');
    }

	// IE/Opera unescape XML in innerHTML, need to escape it back
	html = html.gsub(/\\\"/,'&quot;');

	if (convertHtmlPrefix)
	{
		return (html!=null) ? Appcelerator.Compiler.specialMagicParseHtml(html) : '';
	}
	else
	{
		return html;
	}
};

Appcelerator.Compiler.getWidgetHTML = function(element,stripHTMLPrefix)
{
	//TODO: remove this method in favor of the mapped method
	return Appcelerator.Compiler.getHtml(element,stripHTMLPrefix);
};

Appcelerator.Compiler.isHTMLTag = function(element)
{
	if (Appcelerator.Browser.isIE)
	{
		return element.scopeName=='HTML' || !element.tagUrn;
	}
	var tagName = Appcelerator.Compiler.getTagName(element,true);
	return !tagName.startsWith('app:');
};

Appcelerator.Compiler.getTagname = function(element)
{
	if (!element) throw "element cannot be null";
	if (element.nodeType!=1) throw "node: "+element.nodeName+" is not an element, was nodeType: "+element.nodeType+", type="+(typeof element);

	// used by the compiler to mask a tag
	if (element._tagName) return element._tagName;

	if (Appcelerator.Browser.isIE)
	{
		if (element.scopeName && element.tagUrn)
		{
			return element.scopeName + ':' + element.nodeName.toLowerCase();
		}
	}
	if (Appcelerator.Compiler.isInterpretiveMode)
	{
		return element.nodeName.toLowerCase();
	}
	return String(element.nodeName.toLowerCase());
};

/**
 * internal method to add event listener
 *
 * @param {element} element to add event to
 * @param {string} event name
 * @param {function} action to invoke when event is fired
 * @param {integer} amount of time to delay before calling action after event fires
 * @return {element} returns element for chaining
 */
Appcelerator.Compiler.addEventListener = function (element,event,action,delay)
{
	var logWrapper = function()
	{
		var args = $A(arguments);
		$D('on '+element.id+'.'+event+' => invoking action '+action);
		return action.apply({data:{}},args);
	};

	var functionWrapper = delay > 0 ? (function()
	{
		var args = $A(arguments);

		// IE destroys the keyCode when there's a delay
		var event = args[0];
		if (event.keyCode)
		{
		    args[0] = {keyCode: event.keyCode};
		}

		var a = function()
		{
			return logWrapper.apply(logWrapper,args);
		};
        a.delay(delay/1000);
	}) : logWrapper;

	Event.observe(element,event,functionWrapper,false);

	Appcelerator.Compiler.addTrash(element,function()
	{
		Event.stopObserving(element,event,functionWrapper);
	});

	return element;
}

/**
 * called to install a change listener
 *
 * @param {element} element to add change listener
 * @param {function} action function to call when change is detected in element
 * @return {element} element
 */
Appcelerator.Compiler.installChangeListener = function (element, action)
{
    (function()
    {
        if(element.type == 'checkbox')
	    {
	        // Safari is finicky
	        element._validatorObserver = new Form.Element.Observer(
	            element, 0.1, action
	        );
	    }
	    else
	    {
	        element._focusChangeListener = function()
    	    {
    	        element._validatorObserver = new Form.Element.Observer(element,.5,function(element, value)
                {
    	            action(element,value);
    	        });
    	    };
    	    Event.observe(element,'focus',element._focusChangeListener);

	        element._blurChangeListener = function()
    	    {
    	        if (element._validatorObserver)
    	        {
    	            element._validatorObserver.stop();
    	            try { delete element._validatorObserver; } catch (e) { element._validatorObserver = null; }
                    action(element,Field.getValue(element));
    	        }
    	    };
    	    Event.observe(element,'blur',element._blurChangeListener);
    	}
    }).defer();

    return element;
};

Appcelerator.Compiler.removeChangeListener = function (element)
{
    (function()
    {
        if (element._focusChangeListener)
        {
            Event.stopObserving(element, 'focus', element._focusChangeListener);
            try { delete element._focusChangeListener; } catch (e) { element._focusChangeListener = null; }
        }
        if (element._blurChangeListener)
        {
            Event.stopObserving(element, 'blur', element._blurChangeListener);
            try { delete element._blurChangeListener; } catch (e) { element._blurChangeListener = null; }
        }
        if (element._validatorObserver)
        {
            element._validatorObserver.stop();
            try { delete element._validatorObserver; } catch (e) { element._validatorObserver = null; }
        }
    }).defer();
}

Appcelerator.Compiler.ElementFunctions = {};

/**
 * get a function attached to element
 *
 * @param {element} element that has attached function
 * @param {string} name of function
 * @return {function} function attached or null if none found with name
 */
Appcelerator.Compiler.getFunction = function(element,name)
{
	var id = (typeof element == 'string') ? element : element.id;
	var key = id + '_' + name;

	var f = Appcelerator.Compiler.ElementFunctions[key];
	if (f)
	{
		return f;
	}

	element = $(id);
	if (element)
	{
		return element[name];
	}
	return null;
};

/**
 * attach a special function to element which can be invoked (such as a special action)
 * by system
 *
 * @param {element} element to attach function to
 * @param {string} name of the function
 * @param {function} function to invoke
 * @return {element} element
 */
Appcelerator.Compiler.attachFunction = function(element,name,f)
{
	var id = (typeof element == 'string') ? element : element.id;
	var key = id + '_' + name;
	Appcelerator.Compiler.ElementFunctions[key]=f;
	return element;
};

/**
 * attempt to first execute a special attached function by name or if not found
 * attempt to execute a function already part of elements prototype
 *
 * @param {element} element to invoke against
 * @param {string} name of the function
 * @param {array} arguments to pass
 * @param {boolean} required to be there or if not - throw exception
 * @return {object} value returned from invocation
 */
Appcelerator.Compiler.executeFunction = function(element,name,args,required)
{
	required = (required==null) ? false : required;
	args = (args==null) ? [] : args;
	var id = (typeof element == 'string') ? element : element.id;
	element = $(id);

	var key = id + '_' + name;
	var f = Appcelerator.Compiler.ElementFunctions[key];
	if (f)
	{
        //
        // NOTE: you must call the function as below since
        // natively wrapped methods like focus won't work if you
        // try and use the normal javascript prototype call/apply
        // methods on them
        //
		switch(args.length)
		{
			case 0:
				return f();
			case 1:
				return f(args[0]);
			case 2:
				return f(args[0],args[1]);
			case 3:
				return f(args[0],args[1],args[2]);
			case 4:
				return f(args[0],args[1],args[2],args[3]);
			case 5:
				return f(args[0],args[1],args[2],args[3],args[4]);
			case 6:
				return f(args[0],args[1],args[2],args[3],args[4],args[5]);
			case 7:
				return f(args[0],args[1],args[2],args[3],args[4],args[5],args[6]);
			case 8:
				return f(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7]);
			case 9:
				return f(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8]);
			case 10:
				return f(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9]);
			case 11:
				return f(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10]);
			default:
				throw "too many arguments - only 11 supported currently for method: "+name+", you invoked with "+args.length+", args was: "+Object.toJSON(args);
		}
	}
	if (element)
	{
		var f = element[name];
		var tf = typeof(f);
		if (f && (tf == 'function' || (Appcelerator.Browser.isIE && tf == 'object')))
		{
			return element[name]();
		}
	}
	else if (required)
	{
		throw "element with ID: "+id+" doesn't have a function named: "+name;
	}
};




/**
 * called by compiler to compile a widget
 *
 * @param {element} element object to compile which must be a widget
 * @param {object} state object
 * @param {name} name of the widget which can override what the element actually is
 * @return {string} compiled code or null if none generated
 */
Appcelerator.Compiler.customConditionObservers = {};

Appcelerator.Compiler.compileWidget = function(element,state,name)
{
	name = name || Appcelerator.Compiler.getTagname(element);
	var module = Appcelerator.Core.widgets[name];
	var compiledCode = '';
	
    $D('compiled widget '+element+', id='+element.id+', tag='+name+', module='+module);

	if (module)
	{
		if (Appcelerator.Compiler.isInterpretiveMode && module.flashRequired)
		{
			var version = module.flashVersion || 9.0;
			var error = null;

			if (!Appcelerator.Browser.isFlash)
			{
				error = 'Flash version ' + version + ' or greater is required';
			}
			else if (Appcelerator.Browser.flashVersion < version)
			{
				error = 'Flash version ' + version + ' or greater is required. Your version is: '+Appcelerator.Browser.flashVersion;
			}

			if (error)
			{
				error = error + '. <a href="http://www.adobe.com/products/flashplayer/" target="_NEW">Download Flash Now</a>'
				var html = '<div class="flash_error" style="border:1px solid #c00;padding:5px;background-color:#fcc;text-align:center;margin:5px;">' + error + '</div>';
				new Insertion.Before(element,html);
				Element.remove(element);
				return;
			}
		}

		var id = Appcelerator.Compiler.getAndEnsureId(element);

		var moduleAttributes = module.getAttributes();
		var widgetParameters = {};
		for (var i = 0; i < moduleAttributes.length; i++)
		{
			var error = false;
			(function(){
				var modAttr = moduleAttributes[i];
				var value = element.getAttribute(modAttr.name) || modAttr.defaultValue;
				// check and make sure the value isn't a function as what will happen in certain
				// situations because of prototype's fun feature of attaching crap on to the Object prototype
				if (Object.isFunction(value))
				{
					value = modAttr.defaultValue;
				}
				if (!value && !modAttr.optional)
				{
					Appcelerator.Compiler.handleElementException(element, null, 'required attribute "' + modAttr.name + '" not defined for ' + id);
					error = true;
				}
				widgetParameters[modAttr.name] = value;
			})();
			if (error)
			{
				return;
			}
		}
		widgetParameters['id'] = id;

		//
		// building custom functions
		//
		var functions = null;
		if (module.getActions)
		{
			functions = module.getActions();
			for (var c=0;c<functions.length;c++)
			{
				Appcelerator.Compiler.buildCustomAction(functions[c]);
			}
		}

        // grab any custom conditions before attempting to parse on attributes
        if(module.getConditions)
        {
            Appcelerator.Compiler.customConditionObservers[id] = {};
            var customConditions = module.getConditions();
            for (var i = 0; i < customConditions.length; i++)
            {
                var custCond = customConditions[i];
                var condFunct = Appcelerator.Compiler.customConditionFunctionCallback(custCond);
                Appcelerator.Compiler.registerCustomCondition({conditionNames: [custCond]}, 
                    condFunct, element.id);
            }
        }

        //
        // parse on attribute
        //
        if (Object.isFunction(module.dontParseOnAttributes))
        {
            if (!module.dontParseOnAttributes())
            {
                Appcelerator.Compiler.parseOnAttribute(element);
            }
        }
        else
        {
            Appcelerator.Compiler.parseOnAttribute(element);
        }
        
		//
		// hand off widget for building
		//
		var instructions = null;
		try
		{
			instructions = module.buildWidget(element,widgetParameters,state);
		}
		catch (exxx)
		{
			Appcelerator.Compiler.handleElementException(element, exxx, 'building widget ' + element.id);
			return;
		}

		//
		// allow the widget to change its id
		//
		if (element.id != id)
		{
			Appcelerator.Compiler.removeElementId(id);
			id = element.id;
			Appcelerator.Compiler.getAndEnsureId(element);
		}

		var added = false;
		if (instructions)
		{
			var position = instructions.position || Appcelerator.Compiler.POSITION_REPLACE;
			var removeElement = position == Appcelerator.Compiler.POSITION_REMOVE;

			if (!removeElement)
			{
				//
				// now handle presentation details
				//
				var html = instructions.presentation;
				if (html!=null)
				{
					// rename the real ID
					Appcelerator.Compiler.setElementId(element, id+'_widget');
					// widgets can define the tag in which they should be wrapped
					if(instructions.parent_tag != 'none' && instructions.parent_tag != '')
					{
					   var parent_tag = instructions.parent_tag || 'div';
					   html = '<'+parent_tag+' id="'+id+'_temp" style="margin:0;padding:0;display:none">'+html+'</'+parent_tag+'>';
					}

					// add the XML namespace IE thing but only if you have what looks to
					// be a widget that requires namespace - otherwise, it will causes issues like when
					// you include a single <img>
					if (Appcelerator.Browser.isIE && html.indexOf('<app:') != -1)
					{
						html = Appcelerator.Compiler.addIENameSpace(html);
					}

					added = true;
					switch(position)
					{
						case Appcelerator.Compiler.POSITION_REPLACE:
						{
							new Insertion.Before(element,html);
							removeElement = true;
							break;
						}
						case Appcelerator.Compiler.POSITION_TOP:
						{
							new Insertion.Top(element,html);
							break;
						}
						case Appcelerator.Compiler.POSITION_BOTTOM:
						{
							new Insertion.Bottom(element,html);
							break;
						}
						case Appcelerator.Compiler.POSITION_BEFORE:
						{
							new Insertion.Before(element,html);
							break;
						}
						case Appcelerator.Compiler.POSITION_AFTER:
						{
							new Insertion.After(element,html);
							break;
						}
						case Appcelerator.Compiler.POSITION_BODY_TOP:
						{
							new Insertion.Top(document.body,html);
							break;
						}
						case Appcelerator.Compiler.POSITION_BODY_BOTTOM:
						{
							new Insertion.Bottom(document.body,html);
							break;
						}
						case Appcelerator.Compiler.POSITION_HEAD_TOP:
						{
							new Insertion.Top(Appcelerator.Core.HeadElement,html);
							break;
						}
						case Appcelerator.Compiler.POSITION_HEAD_BOTTOM:
						{
							new Insertion.Bottom(Appcelerator.Core.HeadElement,html);
							break;
						}
					}
				}
			}

			var outer = null;
			if (added)
			{
				outer = $(id+'_temp');
				if (!outer)
				{
					// in case we're in a content file or regular unattached DOM
					outer = element.ownerDocument.getElementById(id+'_temp');
				}

				Appcelerator.Compiler.delegateToContainerProcessors(element, outer);
			}

			var compileId = id;
			var fieldset = element.getAttribute('fieldset');

			//
			// remove element
			//
			var removeId = element.id;
			if (removeElement)
			{
			    Appcelerator.Compiler.removeElementId(removeId);
				Element.remove(element);
			}

			if (outer)
			{
    			if (added && !$(id))
    			{
    				// set outer div only if widget id was not used in presentation
    				Appcelerator.Compiler.setElementId(outer, id);
    				compileId = id;
    				widgetParameters['id']=id;
    			}
			    outer.widget = module;
                outer.widgetParameters = widgetParameters;
			}

			//
			// attach any special widget functions
			//
			if (functions)
			{
				for (var c=0;c<functions.length;c++)
				{
					var methodname = functions[c];
					var method = module[methodname];
					if (!method) throw "couldn't find method named: "+methodname+" for module = "+module;
					(function()
					{
						var attachMethodName = functions[c];
						var attachMethod = module[methodname];
						var f = function(id,m,data,scope,version,customActionArguments,direction,type)
						{
							try
							{
								attachMethod(id,widgetParameters,data,scope,version,customActionArguments,direction,type);
							}
							catch (e)
							{
								$E('Error executing '+attachMethodName+' in module '+module.getWidgetName()+'. Error '+Object.getExceptionDetail(e)+', stack='+e.stack);
							}
						};
						Appcelerator.Compiler.attachFunction(id,attachMethodName,f);
					})();
				}
			}

			if(fieldset && !module.ignoreFieldset)
			{
			    Appcelerator.Compiler.addFieldSet(outer, false, fieldset);
			}

            //
            // run initialization
            //
            if (instructions.compile)
            {
                try
                {
                	module.compileWidget(widgetParameters,outer);
                }
                catch (exxx)
                {
                	Appcelerator.Compiler.handleElementException($(id), exxx, 'compiling widget ' + id + ', type ' + element.nodeName);
                	return;
                }
            }

            if (added && instructions.wire && outer)
            {
				Appcelerator.Compiler.compileElement(outer, state);
            }

            // reset the display for the widget
			if (outer)
			{
                outer.style.display='';
			}
		}
	}
	else
	{
		// reset to the original
		if (element.style && element.style.display != element.style.originalDisplay)
		{
		  element.style.display = element.style.originalDisplay;
		}
	}

	return compiledCode;
};

/**
 * fire an custom condition from within the widget.  
 */
Appcelerator.Compiler.fireCustomCondition = function(id, name, data)
{
	// let any listeners have at it
	Appcelerator.Compiler.fireConditionEvent(id,name);

	
    var observersForElement = Appcelerator.Compiler.customConditionObservers[id];
    if(observersForElement == null) {
        $D('no custom condition found for id="'+id+'"');
    }
    else if (observersForElement[name] == null)
    {
        $D('no custom condition found for id="'+id+'" condition="'+name+'"');
    }
    else
    {
        var entries = observersForElement[name];
        for(var i = 0; i < entries.length; i++) 
        {   
            var entry = entries[i];
            params = entry.params;
            var actionParams = params ? Appcelerator.Compiler.getParameters(params,false) : null;
        	var paramsStr = (actionParams) ? Object.toJSON(actionParams) : null;
            var ok = Appcelerator.Compiler.parseConditionCondition(paramsStr, data);
            
            var actionFunc;
            if (ok)
        	{
        	    actionFunc = Appcelerator.Compiler.makeConditionalAction(id,entry.action,entry.ifCond,data);
        	}
        	else if (elseaction)
        	{
        	    actionFunc = Appcelerator.Compiler.makeConditionalAction(id,entry.elseAction,entry.ifCond,data);
        	}
            Appcelerator.Compiler.executeAfter(actionFunc,entry.delay);
        }
    }
};


Appcelerator.Compiler.customConditionFunctionCallback = function(custCond)
{
    return function (element, condition, action, elseAction, delay, ifCond)
    {
        var id = element.id;
        var actionParams = Appcelerator.Compiler.parameterRE.exec(condition);
        var type = (actionParams ? actionParams[1] : condition);
        var params = actionParams ? actionParams[2] : null;
        if (type == custCond)
        {
            var entry =
            {
                'action': action,
                'delay': delay,
                'elseAction': elseAction,
                'ifCond': ifCond,
                'params': params
            }

            if(Appcelerator.Compiler.customConditionObservers[id] == null) {
                Appcelerator.Compiler.customConditionObservers[id] = {};
            }

            if (Appcelerator.Compiler.customConditionObservers[id][custCond])
            {
                Appcelerator.Compiler.customConditionObservers[id][custCond].push(entry);
            }
            else
            {
                Appcelerator.Compiler.customConditionObservers[id][custCond] = [entry];
            }
            return true;
        }
        return false;
    }
}
Appcelerator.Compiler.determineScope = function(element)
{
	var scope = element.getAttribute('scope');

	if (!scope)
	{
		var p = element.parentNode;
		if (p)
		{
			scope = p.scope;
		}

		if (!scope)
		{
			scope = 'appcelerator';
		}
	}
	element.scope = scope;
};

Appcelerator.Compiler.parseOnAttribute = function(element)
{
    try
    {
    	var on = element.getAttribute('on');
    	if (on && Object.isString(on))
    	{
		    $D('parseOnAttribute ',element.id,' on=',on);
    		Appcelerator.Compiler.compileExpression(element,on,false);
    		return true;
    	}
    }
	catch (exxx)
	{
		Appcelerator.Compiler.handleElementException(element, exxx, 'compiling "on" attribute for element ' + element.id);
	}
	return false;
};

Appcelerator.Compiler.smartTokenSearch = function(searchString, value)
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

Appcelerator.Compiler.compoundCondRE = /^\((.*)?\) then$/;

Appcelerator.Compiler.parseExpression = function(value,element)
{
	if (!value)
	{
		return [];
	}

	if (!Object.isString(value))
	{
		alert('framework error: value was '+value+' -- unexpected type: '+typeof(value));
	    throw "value: "+value+" is not a string!";
	}
	value = value.gsub('\n',' ');
	value = value.gsub('\r',' ');
	value = value.gsub('\t',' ');
	value = value.trim();

	var thens = [];
	var ors = Appcelerator.Compiler.smartSplit(value,' or ');
	
	for (var c=0,len=ors.length;c<len;c++)
	{
		var expression = ors[c].trim();
		var thenidx = expression.indexOf(' then ');
		if (thenidx <= 0)
		{
			// we allow widgets to have a short-hand syntax for execute
			if (Appcelerator.Compiler.getTagname(element).indexOf(':'))
			{
				expression = expression + ' then execute';
				thenidx = expression.indexOf(' then ');
			}
			else
			{
				throw "syntax error: expected 'then' for expression: "+expression;
			}
		}
		var condition = expression.substring(0,thenidx);
		
		// check to see if we have compound conditions - APPSDK-597
		var testExpr = expression.substring(0,thenidx+5);
		var condMatch = Appcelerator.Compiler.compoundCondRE.exec(testExpr);
		if (condMatch)
		{
			var expressions = condMatch[1];
			// turn it into an array of conditions
			condition = Appcelerator.Compiler.smartSplit(expressions,' or ');
		}
		
		var elseAction = null;
		var nextstr = expression.substring(thenidx+6);
		var elseidx = Appcelerator.Compiler.smartTokenSearch(nextstr, 'else');

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
		var afterIdx =  Appcelerator.Compiler.smartTokenSearch(nextstr, 'after ');

		if (afterIdx!=-1)
		{
			var afterStr = nextStr.substring(afterIdx+6);
			delay = Appcelerator.Util.DateTime.timeFormat(afterStr);
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

Appcelerator.Compiler.compileExpression = function (element,value,notfunction)
{
	value = Appcelerator.Compiler.processMacros(value,element.id);
	if (!value)
	{
		alert('value returned null for '+element.id);
	}
	var clauses = Appcelerator.Compiler.parseExpression(value,element);
	$D('on expression for ',element.id,' has ',clauses.length,' condition/action pairs');
	for(var i = 0; i < clauses.length; i++)
	{
		var clause = clauses[i];
        $D('compiling expression for ',element.id,' => condition=[',clause[1],'], action=[',clause[2],'], elseAction=[',clause[3],'], delay=[',clause[4],'], ifCond=[',clause[5],']');

        clause[0] = element;
		var handled = false;
		
		if (Object.isArray(clause[1]))
		{
			for (var c=0;c<clause[1].length;c++)
			{
				var cl = clause[1][c];
				var copy = [element,cl,clause[2],clause[3],clause[4],clause[5]];

		        handled = Appcelerator.Compiler.handleCondition.call(this, copy);
		        if (!handled)
		        {
		            throw "syntax error: unknown condition type: "+clause[1]+" for "+value;
		        }
			}
			continue;
		}
		
		
        handled = Appcelerator.Compiler.handleCondition.call(this, clause);

        if (!handled)
        {
            throw "syntax error: unknown condition type: "+clause[1]+" for "+value;
        }
	}
};

Appcelerator.Compiler.isIDRef = function(value)
{
	if (value)
	{
		if (Object.isString(value))
		{
			return value.charAt(0)=='$';
		}
	}
	return false;
};

Appcelerator.Compiler.parseConditionCondition = function(actionParamsStr,data) 
{
    var ok = true;
    var actionParams = actionParamsStr ? actionParamsStr.evalJSON() : null;

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
			if (p.keyExpression || Appcelerator.Compiler.isIDRef(lhs))
			{
				var out = Appcelerator.Compiler.getEvaluatedValue(lhs,data,data,p.keyExpression);
				if (!p.keyExpression && Appcelerator.Compiler.isIDRef(lhs) && lhs == out)
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
				lhs = Appcelerator.Compiler.getEvaluatedValue(lhs,data);
			}
			if (lhs == preLHS)
			{
				// left hand side must evaluate to a value -- if we get here and it's the same, that 
				// means we didn't find it
				lhs = null;
			}
			// mathematics
			if ((operator == '<' || operator == '>') && (rhs && Object.isString(rhs) && rhs.charAt(0)=='='))
			{
				operator += '=';
				rhs = rhs.substring(1);
			}
			if (rhs && Object.isString(rhs) && rhs.charAt(0)=='~')
			{
				regex = true;
				rhs = rhs.substring(1);
			}
			if (p.empty)
			{
				rhs = lhs;
			}
			else if (p.keyExpression || Appcelerator.Compiler.isIDRef(rhs))
			{
				var out = Appcelerator.Compiler.getEvaluatedValue(rhs,data,data,p.valueExpression);
				if (!p.valueExpression && Appcelerator.Compiler.isIDRef(rhs) && rhs == out)
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
				rhs = Appcelerator.Compiler.getEvaluatedValue(rhs,data);
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

/*
 * Conditions trigger the execution of on expressions,
 * customConditions is a list of parsers that take the left-hand-side
 * of an on expression (before the 'then') and register event listeners
 * to be called when the condition is true.
 *
 * Parsers registered with registerCustomCondition are called in order
 * until one of them successfully parses the condition and returns true.
 */
Appcelerator.Compiler.customConditions = [];
Appcelerator.Compiler.customElementConditions = [];

Appcelerator.Compiler.registerCustomCondition = function(metadata, condition, elementid)
{
	condition.metadata = metadata;
	if (!elementid)
	{
    	Appcelerator.Compiler.customConditions.push(condition);
	}
	else
	{
    	Appcelerator.Compiler.customElementConditions.push({elementid: elementid, condition: condition});
	}
};

Appcelerator.Compiler.conditionListeners = {};

/**
 * register for when a condition is begin fired for an element
 */
Appcelerator.Compiler.registerConditionListener = function(element,condition,callback)
{
	var key = $(element).id + "__" + condition;
	var listeners = Appcelerator.Compiler.conditionListeners[key];
	if (!listeners)
	{
		listeners = [];
		Appcelerator.Compiler.conditionListeners[key]=listeners;
	}
	listeners.push(callback);
};

Appcelerator.Compiler.fireConditionEvent = function(element,condition)
{
	var key = $(element).id + "__" + condition;
	var listeners = Appcelerator.Compiler.conditionListeners[key];
	if (listeners)
	{
		for (var c=0;c<listeners.length;c++)
		{
			listeners[c]($(element),condition);
		}
	}
};

Appcelerator.Compiler.handleCondition = function(clause)
{
    var element = clause[0];
    $D('handleCondition called for ',element);

	if (clause[1] && Object.isBoolean(clause[1]))
	{
	    var f = Appcelerator.Compiler.makeAction(element.id,clause[2]);
		return f.call(this,clause[3]);
	}
	
    //first loop through custom conditions defined by the widget
    for (var f=0;f<Appcelerator.Compiler.customElementConditions.length;f++)
    {
        var cond = Appcelerator.Compiler.customElementConditions[f];
        if (cond.elementid == element.id)
        {
            var condFunction = cond.condition;
            var processed = condFunction.apply(condFunction,clause);
     		if (processed)
     		{
     			return true;
     		}
        }
    }

	for (var f=0;f<Appcelerator.Compiler.customConditions.length;f++)
	{
		var condFunction = Appcelerator.Compiler.customConditions[f];
		var processed = condFunction.apply(condFunction,clause);
 		if (processed)
 		{
 			return true;
 		}
 	}
 	return false;
};

Appcelerator.Compiler.getConditionsMetadata = function() {
	return Appcelerator.Compile.customConditions.pluck('metadata');
}

// TODO: Appcelerator.Compiler.getConditionsRegex


Appcelerator.Compiler.parameterRE = /(.*?)\[(.*)?\]/i;
Appcelerator.Compiler.expressionRE = /^expr\((.*?)\)$/;

Appcelerator.Compiler.customActions = {};
Appcelerator.Compiler.customElementActions = {};
Appcelerator.Compiler.registerCustomAction = function(name,callback,element)
{
	//
	// create a wrapper that will auto-publish events for each
	// action that can be subscribed to
	//
	var action = Object.clone(callback);
	action.build = function(id,action,params)
	{
		return [
			'try {',
			callback.build(id,action,params),
			'; }catch(exxx){Appcelerator.Compiler.handleElementException',
			'($("',id,'"),exxx,"Executing:',action,'");}'
		].join('');

	};

	if (callback.parseParameters)
	{
		action.parseParameters = callback.parseParameters;
	}
	if (!element)
	{
    	Appcelerator.Compiler.customActions[name] = action;
	}
	else
	{
    	Appcelerator.Compiler.customElementActions[name] = action;
	}
};

Appcelerator.Compiler.properCase = function (value)
{
	return value.charAt(0).toUpperCase() + value.substring(1);
};

Appcelerator.Compiler.smartSplit = function(value,splitter)
{
	value = value.trim();
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
};

Appcelerator.Compiler.makeConditionalAction = function(id, action, ifCond, additionalParams)
{
	var actionFunc = function(scope)
	{
	    var f = Appcelerator.Compiler.makeAction(id,action,additionalParams);
	    if (ifCond)
	    {
			if (Object.isUndefined(scope.id))
			{
				scope.id = id;
			}
			if (Object.evalWithinScope(ifCond,scope))
			{
	            f(scope);
			}
	    }
	    else
	    {
	        f(scope);
	    }
	};
	return actionFunc;
};

/**
 * make an valid javascript function for executing the
 * action - this string must be converted to a function
 * object before executing
 *
 * @param {string} id of the element
 * @param {string} value of the action string
 * @param {object} optional parameters to pass to action
 * @return {string} action as javascript
 */
Appcelerator.Compiler.makeAction = function (id,value,additionalParams)
{
    var actionFuncs = [];
	var actions = Appcelerator.Compiler.smartSplit(value.trim(),' and ');

	for (var c=0,len=actions.length;c<len;c++)
	{
        (function()
        {
    		var actionstr = actions[c].trim();
			var wildcard = actionstr.startsWith('both:') || actionstr.startsWith('*:');
    		var remote_msg = !wildcard && actionstr.startsWith('remote:') || actionstr.startsWith('r:');
    		var local_msg = !remote_msg && (actionstr.startsWith('local:') || actionstr.startsWith('l:'));
    		var actionParams = Appcelerator.Compiler.parameterRE.exec(actionstr);
    		var params = actionParams!=null ? Appcelerator.Compiler.getParameters(actionParams[2].trim(),false) : null;
    		var action = actionParams!=null ? actionParams[1] : actionstr;

			params = params || [];
			if (additionalParams)
			{
				for (var p in additionalParams)
				{
					params.push({key:p,value:additionalParams[p]});
				}
			}
    		if (local_msg || remote_msg || wildcard)
    		{
    			var f = function(scope)
    			{
					var newparams = {};
					for (var x=0;x<params.length;x++)
					{
						var entry = params[x];
						var key = entry.key, value = entry.value;
						if (entry.keyExpression)
						{
							key = Appcelerator.Compiler.getEvaluatedValue(entry.key,null,scope,entry.keyExpression);
						}
						else if (entry.valueExpression)
						{
							value = Appcelerator.Compiler.getEvaluatedValue(entry.value,null,scope,entry.valueExpression);
						}
						else if (entry.empty)
						{
							value = Appcelerator.Compiler.getEvaluatedValue(entry.key,null,scope);
						}
						else
						{
							key = Appcelerator.Compiler.getEvaluatedValue(entry.key);
							value = Appcelerator.Compiler.getEvaluatedValue(entry.value,null,scope);
						}
						newparams[key]=value;
					}
    			    Appcelerator.Compiler.fireServiceBrokerMessage(id, action, newparams, scope);
    			}
    			actionFuncs.push({func: f, action: action});
    		}
    		else
    		{
    		    var builder = Appcelerator.Compiler.customElementActions[action];
                if (!builder)
                {
        			builder = Appcelerator.Compiler.customActions[action];
                }
    			if (!builder)
    			{
    				throw "syntax error: unknown action: "+action+" for "+id;
    			}

    			//
    			// see if the widget has its own parameter parsing routine
    			//
    			var f = builder.parseParameters;

    			if (f && Object.isFunction(f))
    			{
    				// this is called as a function to custom parse parameters in the action between brackets []
    				params = f(id,action,actionParams?actionParams[2]||actionstr:actionstr);
    			}

    			//
    			// delegate to our pluggable actions to make it easy
    			// to extend the action functionality
    			//
    			var f = function(scope)
    			{
					scope = scope || window;
					if (Object.isArray(params))
					{
						for (var x=0;x<params.length;x++)
						{
							var entry = params[x];
							if (entry.keyExpression)
							{
								entry.key = Appcelerator.Compiler.getEvaluatedValue(entry.key,scope.data,scope,entry.keyExpression);
							 	entry.keyExpression = false;
							}
							else if (entry.valueExpression)
							{
								entry.value = Appcelerator.Compiler.getEvaluatedValue(entry.value,scope.data,scope,entry.valueExpression);
								entry.valueExpression = false;
								if (entry.empty)
								{
									entry.key = entry.value;
								}
							}
							else if (entry.empty)
							{
								entry.value = Appcelerator.Compiler.getEvaluatedValue(entry.key,scope.data,scope);
							}
							else
							{
								entry.key = Appcelerator.Compiler.getEvaluatedValue(entry.key);
								entry.value = Appcelerator.Compiler.getEvaluatedValue(entry.value,scope.data,scope);
							}
						}
					}
    			    builder.execute(id, action, params, scope);
    			}
    			actionFuncs.push({func: f, action: action});
    		}
        })();
	}
    var actionFunction = function(scope)
    {
		var perf = Appcelerator.Config['perfmon'];
        for (var i=0; i < actionFuncs.length; i++)
        {
            actionFunc = actionFuncs[i];
            var timeStart = null;
            if (perf)
            {
                timeStart = new Date;
            }
            actionFunc.func(scope);
            if (perf)
            {
                var time = (new Date).getTime() - timeStart.getTime();
                $MQ('l:perfmon.action', {id: id, action: actionFunc.action, time: time});
            }
        }
    }
	return actionFunction;
};

Appcelerator.Compiler.convertMessageType = function(type)
{
	return Appcelerator.Util.ServiceBroker.convertType(type);
};

Appcelerator.Compiler.getMessageType = function (value)
{
	var actionParams = Appcelerator.Compiler.parameterRE.exec(value);
	return Appcelerator.Compiler.convertMessageType(actionParams && actionParams.length > 0 ? actionParams[1] : value);
};

Appcelerator.Compiler.fireServiceBrokerMessage = function (id, type, args, scopedata)
{
    (function()
    {
		var data = args || {};
		var element = $(id);
		var fieldset = null;
		var scope = null;
		if (element)
		{
			fieldset = element.getAttribute('fieldset');
			scope = element.scope;
		}
		
		for (var p in data)
		{
			var entry = data[p];
			data[p] = Appcelerator.Compiler.getEvaluatedValue(entry,data,scopedata);
		}

		var localMode = type.startsWith('local:') || type.startsWith('l:');

		if (fieldset)
		{
            Appcelerator.Compiler.fetchFieldset(fieldset, localMode, data);
		}

		if (localMode)
		{
			if (data['id'] == null)
			{
                data['id'] = id;
			}

		    if (data['element'] == null)
            {
				// this might not be the element that triggered the message,
				// but the element corresponding to an explicit id parameter
                data['element'] = $(data['id']);
            }
		}

		if (!scope || scope == '*')
		{
			scope = 'appcelerator';
		}

		$MQ(type,data,scope);
	}).defer();
};


/*
 Pluck the current values from a fieldset and return in a hash/dict/object.
 If a third argument is passed, the key/value pairs from the fieldset will be added to that object. 
*/
Appcelerator.Compiler.fetchFieldset = function(fieldset, localMode, data) {
    if(!data) {
        data = {};
    }
    
    var fields = Appcelerator.Compiler.fieldSets[fieldset];
	if (fields && fields.length > 0)
	{
		for (var c=0,len=fields.length;c<len;c++)
		{
			var fieldid = fields[c];
			var field = $(fieldid);
			var name = field.getAttribute('name') || fieldid;
            
            // don't overwrite other values in the payload
			if (data[name] == null)
			{
				// special case type field we only want to add
				// the value if it's checked
				if (field.type == 'radio' && !field.checked)
				{
					continue;
				}
				var newvalue = Appcelerator.Compiler.getElementValue(field,true,localMode);
				var valuetype = typeof(newvalue);
				if (newvalue != null && (valuetype=='object' || newvalue.length > 0 || valuetype=='boolean'))
				{
					data[name] = newvalue;
				}
				else
				{
					data[name] = '';
				}
			}
			else
			{
			    if(field.type != 'radio')
			    {
			        Logger.warn('fieldset value for "'+name+'" ignored because it conflicts with existing data payload value');
		        }
			}
		}
	}
	return data;
};

/**
 * return the elements value depending on the type of
 * element it is
 *
 * @param {element} element to get value from
 * @param {boolean} dequote should we automatically dequote value
 * @return {string} value from element
 */
Appcelerator.Compiler.getElementValue = function (elem, dequote, local)
{
    elem = $(elem);
    dequote = (dequote==null) ? true : dequote;

    var widget = elem.widget
    if (widget)
    {
        if(elem.widget.getValue)
        {
            return elem.widget.getValue(elem.id, elem.widgetParameters);
        }
    }
    else
    {
        switch (Appcelerator.Compiler.getTagname(elem))
        {
            case 'input':
            {
                return Appcelerator.Compiler.getInputFieldValue(elem,true,local);
            }
            case 'select':
            {
                if(elem.hasAttribute('multiple'))
                {
                    var selected = [];
                    var options = elem.options;
                    var optionsLen = elem.options.length;
                    for(var i = 0; i < optionsLen; i++)
                    {
                        if(options[i].selected)
                        {
                            selected.push(options[i].value);
                        }
                    }
                    return selected;
                }
                break; // if not multi-select, we use 
            }
            case 'img':
            case 'iframe':
            {
                return elem.src;
            }
            case 'form':
            {
                //TODO
                return '';
            }
        }
        // allow the element to set the value otherwise use the
        // innerHTML of the component
        if (elem.value != undefined)
        {
            return elem.value;
        }
        return elem.innerHTML;
    }
};

/**
 * get the value of the input, suitable for messaging
 *
 * @param {element} element
 * @param {boolean} dequote
 * @param {boolean} local true if it is for a local message
 * @return {string} value
 */
Appcelerator.Compiler.getInputFieldValue = function(elem,dequote,local)
{
	var tagname = Appcelerator.Compiler.getTagname(elem);
	if (tagname != 'input' && tagname != 'textarea' && tagname != 'select')
	{
		return null;
	}

	local = local==null ? true : local;
	dequote = (dequote==null) ? false : dequote;
	var type = elem.getAttribute('type') || 'text';

	var v = Form.Element.Methods.getValue(elem);

	switch(type)
	{
		case 'checkbox':
			return (v == 'on' || v == 'checked');

		case 'password':
		{
			if (!local)
			{
				//
				// support hashing of one or more elements for the password
				//
				var hashElemId = elem.getAttribute('hash');
				if (hashElemId)
				{
					var hashValues = '';
					hashElemId.split(',').each(function(t)
					{
						var hashElem = $(hashElemId);
						if (hashElem)
						{
							hashValues += Appcelerator.Compiler.getInputFieldValue(hashElem,false,true);
						}
					});
					var hash = Appcelerator.Util.MD5.hex_md5(v + hashValues);
					return {
						'hash' : hash,
						'auth' : Appcelerator.Util.MD5.hex_md5(hash + Appcelerator.Util.Cookie.GetCookie('appcelerator-auth'))
					};
				}
			}
		}
	}
	return Appcelerator.Compiler.formatValue(v,!dequote);
};

Appcelerator.Compiler.getEvaluatedValue = function(v,data,scope,isExpression)
{
	if (v && typeof(v) == 'string')
	{
		if (!isExpression && v.charAt(0)=='$')
		{
			var varName = v.substring(1);
			var elem = $(varName);
			if (elem)
			{
				// dynamically substitute the value
				return Appcelerator.Compiler.getElementValue(elem,true);
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
			var match = isExpression || Appcelerator.Compiler.expressionRE.exec(v);
			if (match)
			{
				var expr = isExpression ? v : match[1];
				var func = expr.toFunction();
				var s = scope ? Object.clone(scope) : {};
				if (data)
				{
					for (var k in data)
					{
						if (Object.isString(k))
						{
							s[k] = data[k];
						}
					}
				}
				return func.call(s);
			}

			if (scope)
			{
				var result = Object.getNestedProperty(scope,v,null);
				if (result)
				{
					return result;
				}
			}

			if (data)
			{
				return Object.getNestedProperty(data,v,v);
			}
		}
	}
	return v;
};

Appcelerator.Compiler.formatValue = function (value,quote)
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
};

Appcelerator.Compiler.dequote = function(value)
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
};

Appcelerator.Compiler.convertInt = function(value)
{
	if (value.charAt(0)=='0')
	{
		if (value.length==1)
		{
			return 0;
		}
		return Appcelerator.Compiler.convertInt(value.substring(1));
	}
	return parseInt(value);
};

Appcelerator.Compiler.convertFloat = function(value)
{
	return parseFloat(value);
}

Appcelerator.Compiler.numberRe = /^[-+]{0,1}[0-9]+$/;
Appcelerator.Compiler.floatRe = /^[0-9]*[\.][0-9]*[f]{0,1}$/;
Appcelerator.Compiler.booleanRe = /^(true|false)$/;
Appcelerator.Compiler.quotedRe =/^['"]{1}|['"]{1}$/;
Appcelerator.Compiler.jsonRe = /^\{(.*)?\}$/;

var STATE_LOOKING_FOR_VARIABLE_BEGIN = 0;
var STATE_LOOKING_FOR_VARIABLE_END = 1;
var STATE_LOOKING_FOR_VARIABLE_VALUE_MARKER = 2;
var STATE_LOOKING_FOR_VALUE_BEGIN = 3;
var STATE_LOOKING_FOR_VALUE_END = 4;
var STATE_LOOKING_FOR_VALUE_AS_JSON_END = 5;

Appcelerator.Compiler.decodeParameterValue = function(token,wasquoted)
{
	var value = null;
	if (token!=null && token.length > 0 && !wasquoted)
	{
		var match = Appcelerator.Compiler.jsonRe.exec(token);
		if (match)
		{
			value = String(match[0]).evalJSON();
		}
		if (!value)
		{
			var quoted = Appcelerator.Compiler.quotedRe.test(token);
			if (quoted)
			{
				value = Appcelerator.Compiler.dequote(token);
			}
			else if (Appcelerator.Compiler.floatRe.test(token))
			{
				value = Appcelerator.Compiler.convertFloat(token);
			}
			else if (Appcelerator.Compiler.numberRe.test(token))
			{
				value = Appcelerator.Compiler.convertInt(token);
			}
			else if (Appcelerator.Compiler.booleanRe.test(token))
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
};

Appcelerator.Compiler.parameterSeparatorRE = /[\$=:><!]+/;

/**
 * method will parse out a loosely typed json like structure
 * into either an array of json objects or a json object
 *
 * @param {string} string of parameters to parse
 * @param {boolean} asjson return it as json object
 * @return {object} value
 */
Appcelerator.Compiler.getParameters = function(str,asjson)
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
	if (!Appcelerator.Compiler.parameterSeparatorRE.test(str) && !containsExpr)
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
		if (k && k.startsWith('__E__'))
		{
			if (!asjson)
			{
				return {key:expressions[k],value:v,keyExpression:true,valueExpression:false};
			}
			else
			{
				return expressions[k];
			}
		}
		if (v && v.startsWith('__E__'))
		{
			if (!asjson)
			{
				return {key:k,value:expressions[v],valueExpression:true,keyExpression:false};
			}
			else
			{
				return expressions[v];
			}
		}
		var s = Appcelerator.Compiler.decodeParameterValue(v,tick);
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
							key = currentstr.trim();
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
						key = currentstr.trim();
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
						key = currentstr.trim();
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
			currentstr = currentstr.strip();
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
	// alert('=>'+Object.toJSON(data));
	return data;
};


/**
 * potentially delay execution of function if delay argument is specified
 *
 * @param {function} action to execute
 * @param {integer} delay value to execute in ms
 * @param {object} scope to invoke function in
 */
Appcelerator.Compiler.executeAfter = function(action,delay,scope)
{
	var f = (scope!=null) ? function() { action(scope); } : action;

	if (delay > 0)
	{
		f.delay(delay/1000);
	}
	else
	{
		f();
	}
};

/**
 * generic routine for handling painting error into element upon error
 *
 * @param {element} element
 * @param {exception} exception object
 * @param {string} context of error
 */
Appcelerator.Compiler.handleElementException = function(element,e,context)
{
	var tag = element ? Appcelerator.Compiler.getTagname(element) : document.body;
	var msg = '<strong>Appcelerator Processing Error:</strong><div>Element ['+tag+'] with ID: '+(element.id||element)+' has an error: <div>'+Object.getExceptionDetail(e,true)+'</div>' + (context ? '<div>in <pre>'+context+'</pre></div>' : '') + '</div>';
	$E(msg);
	if (tag == 'IMG')
	{
		new Insertion.Before(element,msg);
	}
	else
	{
		element.innerHTML = '<div style="border:4px solid #777;padding:30px;background-color:#fff;color:#e00;font-family:sans-serif;font-size:18px;margin-left:20px;margin-right:20px;margin-top:100px;text-align:center;">' + msg + '</div>'
	}
	Element.show(element);
};

Appcelerator.Compiler.fieldSets = {};

/**
 * add a fieldset
 *
 * @param {element} element to add to fieldset
 * @param {boolean} excludeself
 * @param {string}  optional name for the fieldset, overrides attribute on element
 *
 * @return {string} fieldset name or null if none found
 */
Appcelerator.Compiler.addFieldSet = function(element,excludeSelf,fieldsetName)
{
	excludeSelf = (excludeSelf==null) ? false : excludeSelf;
	fieldsetName = fieldsetName || element.getAttribute('fieldset');
	if (fieldsetName)
	{
		var fieldset = Appcelerator.Compiler.fieldSets[fieldsetName];
		if (!fieldset)
		{
			fieldset=[];
			Appcelerator.Compiler.fieldSets[fieldsetName] = fieldset;
		}
		if (false == excludeSelf)
		{
			$D('adding = ',element.id,' to field = ',fieldsetName,', values=',fieldset);
			fieldset.push(element.id);
		}
		return fieldset;
	}
	return null;
};

Appcelerator.Compiler.removeFieldSet = function(element)
{
	var fieldsetName = element.getAttribute('fieldset');
	if (fieldsetName)
	{
		var fieldset = Appcelerator.Compiler.fieldSets[fieldsetName];
		if (fieldset)
		{
			fieldset.remove(element.id);
		}
	}
};

Appcelerator.Compiler.updateFieldsetValues = function(fieldset, values, key)
{
	var data = Object.getNestedProperty(values, key, null);
	if (!data)
	{
		data = values;
	}

	if (fieldset)
	{
		var fields = Appcelerator.Compiler.fieldSets[fieldset];
		if (fields && fields.length > 0)
		{
			for (var c=0,len=fields.length;c<len;c++)
			{
				var fieldid = fields[c];
				var field = $(fieldid);
				var name = field.name || field.getAttribute('name') || fieldid;
				var val = Object.getNestedProperty(data, name, null);
				if (val != null)
				{
					Appcelerator.Compiler.setElementValue(field, val);
				}
			}
		}
	}
}

Appcelerator.Compiler.setElementValue = function (element, value)
{
    var revalidate = false;
    var fireChange = false;

    if (element && value != null)
    {
        var widget = element.widget;
        if (widget)
        {
            if (widget.setValue)
            {
                try
                {
                    widget.setValue(element.id, element.widgetParameters, value);
                }
                catch(e)
                {
                    $E(e);
                }
            }
        }
        else
        {
            switch (Appcelerator.Compiler.getTagname(element))
            {
                case 'input':
                {
                    var type = element.getAttribute('type') || 'text';
                    switch (type)
                    {
                        case 'password':
                        case 'hidden':
                        case 'text':
                        {
                            revalidate = true;
                            element.value = value;
                            break;
                        }
                        case 'button':
                        case 'submit':
                        {
                            element.value = value;
                            break;
                        }
                        case 'checkbox':
                        {
                            revalidate = true;
                            if (value == true || value == 'true')
                            {
                                element.checked = true;
                            }
                            else
                            {
                                element.checked = false;
                            }
                            break;
                        }
                        case 'radio':
                        {
                            revalidate = true;
                            if (value == element.getAttribute('value'))
                            {
                                element.checked = true;
                            }
                            else
                            {
                                element.checked = false;
                            }
                            break;
                        }
                    }
                    break;
                }
                case 'textarea':
                {
                    revalidate = true;
                    element.value = value;
                    break;
                }
                case 'select':
                {
                    if(element.hasAttribute('multiple'))
                    {
                        var values = Appcelerator.Util.makeSet(value)
                        var options = element.options;
                        for (var i = 0; i < options.length; i++)
                        {
                            var option = options[i];
                            if (values[option.value])
                            {
                                if(!option.selected)
                                {
                                    option.selected = true;
                                    fireChange = true;
                                    revalidate = true;
                                }
                                // else ignore, already selected
                            }
                            else
                            {
                                if(option.selected)
                                {
                                    option.selected = false;
                                    fireChange = true;
                                    revalidate = true;
                                }
                                // else ignore, already un-selected
                            }
                        }                        
                    }
                    else
                    {
                        var options = element.options;

                        for (var i = 0; i < options.length; i++)
                        {
                            if (options[i].value == value)
                            {
                                element.selectedIndex = i;
                                fireChange = true;
                                revalidate = true;
                                break;
                            }
                        }
                    }
                    break;
                }
                case 'a':
                {
                    element.href = value;
                    break;
                }
                case 'img':
                case 'iframe':
                {
                    element.src = value;
                    break;
                }
                default:
                {
                    element.innerHTML = value;
                    break;
                }
            }
        }
    }

    if(fireChange)
    {
        try {
            Event.cache[element._eventID]['change'][0]({});
        } catch(e) {
            $D('failed to fire change listener for element: '+element.id);
        }
    }

    if (revalidate)
    {
        Appcelerator.Compiler.executeFunction(element, "revalidate");
    }
}

Appcelerator.Compiler.CSSAttributes =
[
	'color',
	'cursor',
	'font',
	'font-family',
	'font-weight',
	'border',
	'border-right',
	'border-bottom',
	'border-left',
	'border-top',
	'border-color',
	'border-style',
	'border-width',
	'background',
	'background-color',
	'background-attachment',
	'background-position',
	'position',
	'display',
	'visibility',
	'overflow',
	'opacity',
	'filter',
	'top',
	'left',
	'right',
	'bottom',
	'width',
	'height',
	'margin',
	'margin-left',
	'margin-right',
	'margin-bottom',
	'margin-top',
	'padding',
	'padding-left',
	'padding-right',
	'padding-bottom',
	'padding-top'
];

Appcelerator.Compiler.isCSSAttribute = function (name)
{
	if (name == 'style') return true;

	for (var c=0,len=Appcelerator.Compiler.CSSAttributes.length;c<len;c++)
	{
		if (Appcelerator.Compiler.CSSAttributes[c] == name)
		{
			return true;
		}

		var css = Appcelerator.Compiler.CSSAttributes[c];
		var index = css.indexOf('-');
		if (index > 0)
		{
			var converted = css.substring(0,index) + css.substring(index+1).capitalize();
			if (converted == name)
			{
				return true;
			}
		}
	}
	return false;
};

Appcelerator.Compiler.convertCSSAttribute = function (css)
{
	var index = css.indexOf('-');
	if (index > 0)
	{
		var converted = css.substring(0,index) + css.substring(index+1).capitalize();
		return converted;
	}
	return css;
}

/**
 * start the compile once the document is loaded - we need to run this each
 * time and not just check when this file is loaded in case an external script
 * decides to turn off compiling on the fly
 */
Appcelerator.Util.ServerConfig.addConfigListener(function()
{
    if (Appcelerator.Compiler.compileOnLoad)
	{
        var outputHandler = Prototype.K;
        Appcelerator.Compiler.compileDocument(outputHandler);
	}
});


Appcelerator.Compiler.setHTML = function(element,html)
{
	$(element).innerHTML = html;
};


/**
 * attach a method which can dynamically compile an expression passed in
 * for a given element. this allows a more unobstrutive or standards based
 * approach for purists such as
 *
 * $('myelement').on('click then l:foo')
 */
var AppceleratorCompilerMethods =
{
    on: function(re,webexpr,parameters)
    {
        Appcelerator.Compiler.destroy(re);
        if (parameters)
        {
            for (var key in parameters)
            {
                if (Object.isString(key))
                {
                    var value = parameters[key];
                    if (Object.isString(value) || Object.isNumber(value) || Object.isBoolean(value))
                    {
                        re.setAttribute(key,value);
                    }
                }
            }
            Appcelerator.Compiler.delegateToAttributeListeners(re);
        }
        Appcelerator.Compiler.compileExpression(re,webexpr,false);
        return re;
    },
    get: function(e)
    {
        return $el(e);
    }
};
Element.addMethods(AppceleratorCompilerMethods);

/**
 * extend an array to support passing multiple elements with a single expression
 *
 * for example: $$('.myclass').on('click then l:foo')
 */
Object.extend(Array.prototype,
{
    on:function(webexpr,parameters)
    {
        for (var c=0;c<this.length;c++)
        {
           var e = this[c];
           if (e && Object.isElement(e) && Object.isFunction(e.on)) e.on(webexpr,parameters);
        }
        return this;
    },

	withoutAll:function(vals)
	{
		return this.without.apply(this, vals);
	}
});


Appcelerator.Compiler.macros = {};
Appcelerator.Compiler.macroRE = /(#[A-Za-z0-9_-]+(\[(.*)?\])?)/;

Appcelerator.Compiler.processMacros = function(expression,id,scope)
{
	return expression.gsub(Appcelerator.Compiler.macroRE,function(match)
	{
		var expr = match[0].substring(1);
		var key = expr;
		var idx1 = key.indexOf('[');
		var idx2 = idx1 > 0 ? key.lastIndexOf(']') : -1;
		if (idx1>0 && idx2>0)
		{
			key = key.substring(0,idx1);
		}
		var template = Appcelerator.Compiler.macros[key];
		if (template)
		{
			scope = scope || {};

			if (idx1>0 && idx2>0)
			{
				var options = expr.substring(idx1+1,idx2);
				options.split(',').each(function(p)
				{
					var tok = p.split('=');
					scope[tok[0].trim()]=tok[1].trim();
				});
			}
			if (id)
			{
				var idvalue = scope['id'];
				if (Object.isUndefined(idvalue))
				{
					scope['id'] = id;
				}
			}
			// recursive in case you reference a macro in a macro
			return Appcelerator.Compiler.processMacros(template(scope),id,scope);
		}
		return match[0];
	});
};

function $WEM(config)
{
	for (var name in config)
	{
		var value = config[name];
		if (Object.isString(value))
		{
			Appcelerator.Compiler.macros[name]=Appcelerator.Compiler.compileTemplate(value);
		}
	}
};



/**
 * DOM utils
 */
Appcelerator.Util.Dom =
{
    ELEMENT_NODE: 1,
    ATTRIBUTE_NODE: 2,
    TEXT_NODE: 3,
    CDATA_SECTION_NODE: 4,
    ENTITY_REFERENCE_NODE: 5,
    ENTITY_NODE: 6,
    PROCESSING_INSTRUCTION_NODE: 7,
    COMMENT_NODE: 8,
    DOCUMENT_NODE: 9,
    DOCUMENT_TYPE_NODE: 10,
    DOCUMENT_FRAGMENT_NODE: 11,
    NOTATION_NODE: 12,

	/**
	 * iterator for node attributes
	 * pass in an iterator function and optional specified (was explicit
	 * placed on node or only inherited via #implied)
	 */
    eachAttribute: function (node, iterator, excludes, specified)
    {
        specified = specified == null ? true : specified;
        excludes = excludes || [];

        if (node.attributes)
        {
            var map = node.attributes;
            for (var c = 0,len = map.length; c < len; c++)
            {
                var item = map[c];
                if (item && item.value != null && specified == item.specified)
                {
                    var type = typeof(item);
                    if (item.value.startsWith('function()'))
                    {
                       continue;
                    }
                    if (type == 'function' || type == 'native' || item.name.match(/_moz\-/) ) continue;
                	if (excludes.length > 0)
                	{
                	  	  var cont = false;
                	  	  for (var i=0,l=excludes.length;i<l;i++)
                	  	  {
                	  	  	  if (excludes[i]==item.name)
                	  	  	  {
                	  	  	  	  cont = true;
                	  	  	  	  break;
                	  	  	  }
                	  	  }
                	  	  if (cont) continue;
                	}
                    iterator(item.name, item.value, item.specified, c, map.length);
                }
            }
            return c > 0;
        }
        return false;
    },

    getTagAttribute: function (element, tagname, key, def)
    {
        try
        {
            var attribute = element.getElementsByTagName(tagname)[0].getAttribute(key);
            if (null != attribute)
            {
                return attribute;
            }
        }
        catch (e)
        {
            //squash...
        }

        return def;
    },

    each: function(nodelist, nodeType, func)
    {
        if (typeof(nodelist) == "array")
        {
            nodelist.each(function(n)
            {
                if (n.nodeType == nodeType)
                {
                    func(n);
                }
            });
        }
        //Safari returns "function" as the NodeList object from a DOM
        else if (typeof(nodelist) == "object" || typeof(nodelist) == "function" && navigator.userAgent.match(/WebKit/i))
        {
            for (var p = 0, len = nodelist.length; p < len; p++)
            {
                var obj = nodelist[p];
                if (typeof obj.nodeType != "undefined" && obj.nodeType == nodeType)
                {
                    try
                    {
                        func(obj);
                    }
                    catch(e)
                    {
                        if (e == $break)
                        {
                            break;
                        }
                        else if (e != $continue)
                        {
                            throw e;
                        }
                    }
                }
            }
        }
        else
        {
            throw ("unsupported dom nodelist type: " + typeof(nodelist));
        }
    },

	/**
 	 * return the text value of a node as XML
 	 */
    getText: function (n,skipHTMLStyles,visitor,addbreaks,skipcomments)
    {
		var text = [];
        var children = n.childNodes;
        var len = children ? children.length : 0;
        for (var c = 0; c < len; c++)
        {
            var child = children[c];
            if (visitor)
            {
            	child = visitor(child);
            }
            if (child.nodeType == this.COMMENT_NODE)
            {
            	if (!skipcomments)
            	{
                	text.push("<!-- " + child.nodeValue + " -->");
            	}
                continue;
            }
            if (child.nodeType == this.ELEMENT_NODE)
            {
                text.push(this.toXML(child, true, null, null, skipHTMLStyles, addbreaks,skipcomments));
            }
            else
            {
                if (child.nodeType == this.TEXT_NODE)
                {
                	var v = child.nodeValue;
                	if (v)
                	{
                    	text.push(v);
                    	if (addbreaks) text.push("\n");
                	}
                }
                else if (child.nodeValue == null)
                {
                    text.push(this.toXML(child, true, null, null, skipHTMLStyles, addbreaks,skipcomments));
                }
                else
                {
                    text.push(child.nodeValue || '');
                }
            }
        }
        return text.join('');
    },

/**
 * IE doesn't have an hasAttribute when you dynamically
 * create an element it appears
 */
    hasAttribute: function (e, n, cs)
    {
        if (!e.hasAttribute)
        {
            if (e.attributes)
            {
                for (var c = 0, len = e.attributes.length; c < len; c++)
                {
                    var item = e.attributes[c];
                    if (item && item.specified)
                    {
                        if (cs && item.name == n || !cs && item.name.toLowerCase() == n.toLowerCase())
                        {
                            return true;
                        }
                    }
                }
            }
            return false;
        }
        else
        {
            return e.hasAttribute(n);
        }
    },

    getAttribute: function (e, n, cs)
    {
        if (cs)
        {
            return e.getAttribute(n);
        }
        else
        {
            for (var c = 0, len = e.attributes.length; c < len; c++)
            {
                var item = e.attributes[c];
                if (item && item.specified)
                {
                    if (item.name.toLowerCase() == n.toLowerCase())
                    {
                        return item.value;
                    }
                }
            }
            return null;
        }
    },

	/**
	 * given an XML element node, return a string representing
	 * the XML
	 */
    toXML: function(e, embed, nodeName, id, skipHTMLStyles, addbreaks, skipcomments)
    {
    	nodeName = (nodeName || e.nodeName.toLowerCase());
        var xml = [];

		xml.push("<" + nodeName);
        
        if (id)
        {
            xml.push(" id='" + id + "' ");
        }
        if (e.attributes)
        {
	        var x = 0;
            var map = e.attributes;
            for (var c = 0, len = map.length; c < len; c++)
            {
                var item = map[c];
                if (item && item.value != null && item.specified)
                {
                    var type = typeof(item);
                    if (item.value && item.value.startsWith('function()'))
                    {
                       continue;
                    }
                    if (type == 'function' || type == 'native' || item.name.match(/_moz\-/)) continue;
                    if (id != null && item.name == 'id')
                    {
                        continue;
                    }
                    
                    // special handling for IE styles
                    if (Appcelerator.Browser.isIE && !skipHTMLStyles && item.name == 'style' && e.style && e.style.cssText)
                    {
                       var str = e.style.cssText;
                       xml.push(" style=\"" + str+"\"");
                       x++;
                       continue;
                    }
                    
                    var attr = String.escapeXML(item.value);
					if (Object.isUndefined(attr) || (!attr && nodeName=='input' && item.name == 'value'))
					{
						attr = '';
					}
                    xml.push(" " + item.name + "=\"" + attr + "\"");
                    x++;
                }
            }
        }
        xml.push(">");

        if (embed && e.childNodes && e.childNodes.length > 0)
        {
        	xml.push("\n");
            xml.push(this.getText(e,skipHTMLStyles,null,addbreaks,skipcomments));
        }
		xml.push("</" + nodeName + ">" + (addbreaks?"\n":""));
		
        return xml.join('');
    },

    getAndRemoveAttribute: function (node, name)
    {
        var value = node.getAttribute(name);
        if (value)
        {
            node.removeAttribute(name);
        }
        return value;
    },

    getAttributesString: function (element, excludes)
    {
        var html = '';
        this.eachAttribute(element, function(name, value)
        {
            if (false == (excludes && excludes.indexOf(name) > -1))
            {
				html += name + '="' + String.escapeXML(value||'') + '" ';                
            }
        }, null, true);
        return html;
    },
	createElement: function (type, options)
	{
	    var elem = document.createElement(type);
	    if (options)
	    {
	        if (options['parent'])
	        {
	            options['parent'].appendChild(elem);
	        }
	        if (options['className'])
	        {
	            elem.className = options['className'];
	        }
	        if (options['html'])
	        {
	            elem.innerHTML = options['html'];
	        }
	        if (options['children'])
	        {
	            options['children'].each(function(child)
	            {
	                elem.appendChild(child);
	            });
	        }
	    }
	    return elem;
	}
};

try
{
    if (typeof(DOMNodeList) == "object") DOMNodeList.prototype.length = DOMNodeList.prototype.getLength;
    if (typeof(DOMNode) == "object")
    {
        DOMNode.prototype.childNodes = DOMNode.prototype.getChildNodes;
        DOMNode.prototype.parentNode = DOMNode.prototype.getParentNode;
        DOMNode.prototype.nodeType = DOMNode.prototype.getNodeType;
        DOMNode.prototype.nodeName = DOMNode.prototype.getNodeName;
        DOMNode.prototype.nodeValue = DOMNode.prototype.getNodeValue;
    }
}
catch(e)
{
}
Appcelerator.Util.Cookie = Class.create();
Appcelerator.Util.Cookie = 
{
	toString: function ()
	{
		return "[Appcelerator.Util.Cookie]";
	},
	
    // ---------------------------------------------------------------
    //  Cookie Functions - Second Helping  (21-Jan-96)
    //  Written by:  Bill Dortch, hIdaho Design <BDORTCH@NETW.COM>
    //  The following functions are released to the public domain.
    //
    //  The Second Helping version of the cookie functions dispenses with
    //  my encode and decode functions, in favor of JavaScript's new built-in
    //  escape and unescape functions, which do more complete encoding, and
    //  which are probably much faster.
    //
    //  The new version also extends the SetCookie function, though in
    //  a backward-compatible manner, so if you used the First Helping of
    //  cookie functions as they were written, you will not need to change any
    //  code, unless you want to take advantage of the new capabilities.
    //
    //  The following changes were made to SetCookie:
    //
    //  1.  The expires parameter is now optional - that is, you can omit
    //      it instead of passing it null to expire the cookie at the end
    //      of the current session.
    //
    //  2.  An optional path parameter has been added.
    //
    //  3.  An optional domain parameter has been added.
    //
    //  4.  An optional secure parameter has been added.
    //
    //  For information on the significance of these parameters, and
    //  and on cookies in general, please refer to the official cookie
    //  spec, at:
    //
    //      http://www.netscape.com/newsref/std/cookie_spec.html    
    //
    //
    // "Internal" function to return the decoded value of a cookie
    //
    getCookieVal: function (offset) {
      var endstr = document.cookie.indexOf (";", offset);
      if (endstr == -1)
      {
        endstr = document.cookie.length;
      } 
      return unescape(document.cookie.substring(offset, endstr));
    },

    //
    //  Function to return the value of the cookie specified by "name".
    //    name - String object containing the cookie name.
    //    returns - String object containing the cookie value, or null if
    //      the cookie does not exist.
    //
    GetCookie: function (name) {
      var arg = escape(name) + "=";
      var alen = arg.length;
      var clen = document.cookie.length;
      var i = 0;
      while (i < clen) {
        var j = i + alen; 
        var value = document.cookie.substring(i,j);
        if (value == arg)
		{
            var cookieval = this.getCookieVal (j);
            if (cookieval!=null && cookieval!='')
            {
      			$D(this,"GetCookie",name+"="+cookieval);
      			return cookieval;
      		}
      		else
      		{
			  	$D(this,"GetCookie",name+"=null");
      			return null;
      		}
        }
        i = document.cookie.indexOf(" ", i) + 1;
        if (i == 0) break; 
      }
  	  $D(this,"GetCookie",name+"=null");
      return null;
    },

    //
    //  Function to create or update a cookie.
    //    name - String object object containing the cookie name.
    //    value - String object containing the cookie value.  May contain
    //      any valid string characters.
    //    [expires] - Date object containing the expiration data of the cookie.  If
    //      omitted or null, expires the cookie at the end of the current session.
    //    [path] - String object indicating the path for which the cookie is valid.
    //      If omitted or null, uses the path of the calling document.
    //    [domain] - String object indicating the domain for which the cookie is
    //      valid.  If omitted or null, uses the domain of the calling document.
    //    [secure] - Boolean (true/false) value indicating whether cookie transmission
    //      requires a secure channel (HTTPS).  
    //
    //  The first two parameters are required.  The others, if supplied, must
    //  be passed in the order listed above.  To omit an unused optional field,
    //  use null as a place holder.  For example, to call SetCookie using name,
    //  value and path, you would code:
    //
    //      SetCookie ("myCookieName", "myCookieValue", null, "/");
    //
    //  Note that trailing omitted parameters do not require a placeholder.
    //
    //  To set a secure cookie for path "/myPath", that expires after the
    //  current session, you might code:
    //
    //      SetCookie (myCookieVar, cookieValueVar, null, "/myPath", null, true);
    //
    SetCookie: function (name, value) {
      var argv = this.SetCookie.arguments;
      var argc = this.SetCookie.arguments.length;
      var expires = (argc > 2) ? argv[2] : null;
      var path = (argc > 3) ? argv[3] : null;
      var domain = (argc > 4) ? argv[4] : null;
      var secure = (argc > 5) ? argv[5] : false;
      var cookie = escape(name) + "=" + escape (value) +
        ((expires == null) ? "" : ("; expires=" + expires.toGMTString())) +
        ((path == null) ? "" : ("; path=" + path)) +
        ((domain == null) ? "" : ("; domain=" + domain)) +
        ((secure) ? "; secure" : "");
      $D(this,"SetCookie",cookie);
      document.cookie = cookie;
    },

    //  Function to delete a cookie. (Sets expiration date to current date/time)
    //    name - String object containing the cookie name
    // 
    // modified by Jeff Haynie to allow multiple deletions by passing in multiple
    // cookies names in the function call
    //
    DeleteCookie: function () 
    {
		var date = new Date();
		date.setTime(date.getTime()+(-1*24*60*60*1000));
		var expires = "; expires="+date.toGMTString();

		var args=$A(arguments);
      	var self = this;
      	args.each(function(name)
      	{
	      $D(self,"DeleteCookie",name);
	      // delete both local path and the root path (seems to be the only sure way)
		  document.cookie = name+"="+expires+"; path=/";
		  document.cookie = name+"="+expires+";";
		});
    },
    
    //
    // returns true if a cookie with name exists, or false if not
    //
    HasCookie: function (name) 
    {
    	return this.GetCookie(name) != null;
    }
};    

Appcelerator.Core.onload(function()
{
    Appcelerator.Util.ServerConfig.addConfigListener(function()
    {
        if (window.location.protocol!='file:')
        {
        	(function()
        	{
        		if (Appcelerator.Config['cookie_check'])
        		{
        			Appcelerator.Util.Cookie.SetCookie('CookieCheck','1');
        			var cookie = Appcelerator.Util.Cookie.GetCookie('CookieCheck');
        			if (!cookie)
        			{
        			   // cookies not working
        			   window.location = Appcelerator.DocumentPath + 'upgrade_cookies.html';
        			}
        			else
        			{
        			   // just delete it
        			   Appcelerator.Util.Cookie.DeleteCookie('CookieCheck');
        			}
        		}
        	})();
        }
    });
});Appcelerator.Util.ServiceBroker = 
{
    DEBUG:false,
    init:false,
    // this is a randomly generated id used to identify all messages from a page
    instanceid: Appcelerator.instanceid,
    // this default location is overwritten by values in appcelerator.xml
    serverPath: Appcelerator.DocumentPath + "servicebroker",
    interceptors: [],
    messageQueue: [],
    localMessageQueue: [],
    localTimer: null,
    localTimerPoll: 10,
    initQueue:[],
    timer: null,
    time: null,
    serverDown: false,
    poll: false,
    fetching: false,
    localDirectListeners: {},
    remoteDirectListeners: {},
    localPatternListeners: [],
    remotePatternListeners: [],
    devmode: (window.location.href.indexOf('devmode=1') > 0),
    disabled: this.devmode || (window.location.href.indexOf('file:/') != -1) || Appcelerator.Parameters.get('mbDisabled')=='1',
    remoteDisabled: this.disabled || Appcelerator.Parameters.get('remoteDisabled')=='1',
    marshaller:'xml/json',
    transport:'appcelerator',
    multiplex:true,

    toString: function ()
    {
        return '[ServiceBroker]';
    },

    addInterceptor: function (interceptor)
    {
         $D(this.toString() + ' Adding interceptor: ' + interceptor);
        this.interceptors.push(interceptor);
    },

    removeInterceptor: function (interceptor)
    {
        $D(this.toString() + 'Removing interceptor: ' + interceptor);
        this.interceptors.remove(interceptor);
    },

    convertType: function (t)
    {
        if (t.startsWith('l:')) return 'local:'+t.substring(2);
        if (t.startsWith('r:')) return 'remote:'+t.substring(2);
        if (t.startsWith('*:')) return 'both:'+t.substring(2);
        return t;
    },

    addListenerByType: function (t, callback)
    {
        t = this.convertType(t.trim());
        // note: don't use split since regex has :
        var idx = t.indexOf(':');
        var direction = (idx != -1) ? (t.substring(0, idx)) : 'both';
        var type = (idx != -1) ? t.substring(idx + 1) : t;
        var regex = type.charAt(0) == '~';

        if (direction == 'local' || direction == 'both')
        {
            var array = null;
            if (regex)
            {
                this.localPatternListeners.push([callback,new RegExp(type.substring(1))]);
                var removePatternListenerArrays = callback['_removePatternListenerArrays'];
                if (!removePatternListenerArrays)
                {
                    removePatternListenerArrays = [];
                    callback['_removePatternListenerArrays'] = removePatternListenerArrays;
                }
                removePatternListenerArrays.push(this.localPatternListeners);
            }
            else
            {
                array = this.localDirectListeners[type];
                if (!array)
                {
                    array = [];
                    this.localDirectListeners[type] = array;
                }
                var removeListenerArrays = callback['_removeListenerArrays'];
                if (!removeListenerArrays)
                {
                    removeListenerArrays = [];
                    callback['_removeListenerArrays'] = removeListenerArrays;
                }
                removeListenerArrays.push(array);
                array.push(callback);
            }
        }
        if (direction == 'remote' || direction == 'both')
        {
            var array = null;
            if (regex)
            {
                this.remotePatternListeners.push([callback,new RegExp(type.substring(1))]);
                var removePatternListenerArrays = callback['_removePatternListenerArrays'];
                if (!removePatternListenerArrays)
                {
                    removePatternListenerArrays = [];
                    callback['_removePatternListenerArrays'] = removePatternListenerArrays;
                }
                removePatternListenerArrays.push(this.remotePatternListeners);
            }
            else
            {
                array = this.remoteDirectListeners[type];
                if (!array)
                {
                    array = [];
                    this.remoteDirectListeners[type] = array;
                }
                var removeListenerArrays = callback['_removeListenerArrays'];
                if (!removeListenerArrays)
                {
                    removeListenerArrays = [];
                    callback['_removeListenerArrays'] = removeListenerArrays;
                }
                removeListenerArrays.push(array);
                array.push(callback);
            }
        }
    },

    addListener: function (callback)
    {
        var startTime = new Date().getTime();

        var accept = callback['accept'];
        if (!accept)
        {
            alert('add listener problem, callback incorrect and has no accept function\n' + callback);
            return;
        }
        var types = accept.call(callback);
        if (types.length == 1)
        {
            this.addListenerByType(types[0], callback);
        }
        else
        {
            for (var c = 0, len = types.length; c < len; c++)
            {
                this.addListenerByType(types[c], callback);
            }
        }
        if (this.DEBUG) $D(this.toString() + ' Added listener: ' + callback + ' (' + (new Date().getTime() - startTime) + ' ms)');
    },
    removeListener: function (callback)
    {
        var startTime = new Date().getTime();
        var removeListenerArrays = callback['_removeListenerArrays'];
        if (removeListenerArrays && removeListenerArrays.length > 0)
        {
            for (var c = 0,len = removeListenerArrays.length; c < len; c++)
            {
                removeListenerArrays[c].remove(callback);
            }
            delete callback['_removeListenerArrays'];
        }
        var removePatternListenerArrays = callback['_removePatternListenerArrays'];
        if (removePatternListenerArrays && removePatternListenerArrays.length > 0)
        {
            for (var c = 0,len = removePatternListenerArrays.length; c < len; c++)
            {
                var array = removePatternListenerArrays[c];
                if (array && array.length > 0)
                {
                    var found = null;
                    for (var x = 0, xlen = array.length; x < xlen; x++)
                    {
                        var e = array[x];
                        if (e[0] == callback)
                        {
                            found = e;
                            break;
                        }
                    }
                    if (found) array.remove(found);
                }
            }
        }
        if (this.DEBUG) $D(this.toString() + ' Removed listener: ' + callback + ' (' + (new Date().getTime() - startTime) + ' ms)');
    },
    triggerComplete:function()
    {
        this.compileComplete = true;
        this.runInitQueue();
        this.startLocalTimer();
    },
    triggerConfig:function()
    {
        this.configComplete = true;
        this.runInitQueue();
    },
    runInitQueue:function()
    {
        // we need both of these conditions to be true before we can complete initialization
        if (this.compileComplete && this.configComplete)
        {
            this.init = true;
            if (this.initQueue && this.initQueue.length > 0)
            {
                for (var c=0;c<this.initQueue.length;c++)
                {
                    this.queue(this.initQueue[c][0],this.initQueue[c][1]);
                }
                this.initQueue = null;
            }
        }
    },
    /**
     * enqueue an event into the broker
     *
     * the msg must contain a 'type' property
     * which is in the format:
     *
     * [destination:message_type]
     *
     * where destination is either:
     *
     * local - deliver the message to client listeners (don't go to the server)
     * remote - deliver the message to the server
     *
     *
     */
    queue: function (msg, callback)
    {
        if (!Appcelerator.Util.ServiceBroker.init)
        {
            $D(msg.type+' will be queued, not yet initialized');
            this.initQueue.push([msg,callback]);
            return;
        }

        var type = msg['type'];

        if (!type)
        {
            throw "type must be specified on the message";
        }

        type = this.convertType(type);

        var scope = msg['scope'] || 'appcelerator';
        var version = msg['version'] || '1.0';

        // let the interceptors have at it
        if (this.interceptors.length > 0)
        {
            var send = true;
            for (var c = 0, len = this.interceptors.length; c < len; c++)
            {
                var interceptor = this.interceptors[c];
                var func = interceptor['interceptQueue'];
                if (func)
                {
                    var result = func.apply(interceptor, [msg,callback,type,scope,version]);
                    if (this.DEBUG) $D(self.toString() + ' Invoked interceptor: ' + interceptor + ', returned: ' + result + ' for message: ' + type);
                    if (result != null && !result)
                    {
                        send = false;
                        break;
                    }
                }
            }

            if (!send)
            {
                // allow the interceptor the ability to squash it
                $D(this + ' interceptor squashed event: ' + msg['type']);
                return;
            }
        }

        var a = type.indexOf(':');

        var dest = a != -1 ? type.substring(0, a) : 'local';
        var name = a != -1 ? type.substring(a + 1) : type;

        // replace the destination
        msg['type'] = name;

        var data = (msg['data']) ? msg['data'] : {};

        if(Logger.debugEnabled)
        {
            var json = null;
            switch (typeof(data))
            {
                case 'object':
                case 'array':
                json = Object.toJSON(data);
                break;
                default:
                json = data.toString();
                break;
            }
            $D(this + ' message queued: ' + name + ', data: ' + json+', version: '+version+', scope: '+scope);
        }

        this.localMessageQueue.push([name,data,dest,scope,version]);

        if(dest == 'remote')
        {
            // send remote
            if (this.messageQueue)
            {
                // in devmode, we don't actually send remote events
                if (!this.devmode && !this.remoteDisabled)
                {
					this.queueRemote(msg,callback,scope,version);
                }
            }
            else
            {
                $E(this + ' message:' + name + " ignored since we don't have a messageQueue!");
            }
        }
    },
    queueRemote:function(msg,callback,scope,version)
	{
        // place in the outbound message queue for delivery
        this.messageQueue.push([msg,callback,scope,version]);

        // the remote message can be forced to be sent immediate
        // by setting this property, otherwise, it will be queued
        this.startTimer(msg['immediate'] || false);
	},
    dispatch: function (msg)
    {
        var requestid = msg.requestid;
        var type = msg.type;
        var datatype = msg.datatype;
        var scope = msg.scope;
        var data = msg.data;

        // let the interceptors have at it
        if (this.interceptors.length > 0)
        {
            var send = true;
            for (var c = 0, len = this.interceptors.length; c < len; c++)
            {
                var interceptor = this.interceptors[c];
                var func = interceptor['interceptDispatch'];
                if (func)
                {
                    var result = func.apply(interceptor, [requestid,type,msg,datatype,scope]);
                    if (result != null && !result)
                    {
                        send = false;
                        break;
                    }
                }
            }

            if (!send)
            {
                // allow the interceptor the ability to squash it
                return;
            }
        }
        var stat = Appcelerator.Util.Performance.logStats ? Appcelerator.Util.Performance.createStat() : null;

        var array = this.remoteDirectListeners[type];
        if (array && array.length > 0)
        {
            for (var c = 0, len = array.length; c < len; c++)
            {
                this.sendToListener(array[c], type, data, datatype, 'remote', scope);
            }
        }
        if (this.remotePatternListeners.length > 0)
        {
            for (var c = 0,len = this.remotePatternListeners.length; c < len; c++)
            {
                var entry = this.remotePatternListeners[c];
                var listener = entry[0];
                var pattern = entry[1];
                if (pattern.test(type))
                {
                    this.sendToListener(listener, type, data, datatype, 'remote', scope);
                }
            }
        }
		if (Appcelerator.Util.Performance.logStats) Appcelerator.Util.Performance.endStat(stat,type,"remote");
    },
    sendToListener: function (listener, type, msg, datatype, from, scope)
    {
        // let the interceptors have at it
        if (this.interceptors.length > 0)
        {
            var send = true;
            for (var c = 0, len = this.interceptors.length; c < len; c++)
            {
                var interceptor = this.interceptors[c];
                var func = interceptor.interceptSendToListener;
                if (func)
                {
                    var result = func.apply(interceptor, [listener,type,msg,datatype,from,scope]);
                    if (result != null && !result)
                    {
                        send = false;
                        break;
                    }
                }
            }

            if (!send)
            {
                // allow the interceptor the ability to squash it
                return;
            }
        }
        
        //
        // check the scope function before we continue
        //
        var scopeCheckFunc = listener.acceptScope;
        if (scopeCheckFunc)
        {
            if (!scopeCheckFunc(scope))
            {
                return;
            }
        }
        
        $D(this.toString() + ' forwarding ' + type + ' to ' + listener + ', direction:' + from + ', datatype:' + datatype + ', data: ' + msg);
        try
        {
            listener.onMessage.apply(listener, [type,msg,datatype,from,scope]);
        }
        catch (e)
        {
            $E("Unhandled Exception dispatching:" + type + ", " + msg + ", to listener:" + listener + ", " + Object.getExceptionDetail(e));
        }
        return true;
    },

    flush: function()
    {
        this.deliver(false,true);
    },

    deliver: function (initialrequest, synchronous)
    {
        if (this.messageQueue == null)
        {
            // we're dead and destroyed, cool, just
            // return silently
            $E(this.toString()+', deliver called but no message queue');
            return;
        }
        if (this.remoteDisabled)
        {
            // remote disabled
            return;
        }

        // get the marshaller to use
        var marshaller = Appcelerator.Util.ServiceBrokerMarshaller[this.marshaller];
        if (!marshaller)
        {
            $E(this+' - no marshaller defined, will not send message to remote endpoint');
            this.remoteDisabled = true;
            return;
        }
        var transportHandler = Appcelerator.Util.ServiceBrokerTransportHandler[this.transport];
        if (!transportHandler)
        {
            $E(this+' - no transport handler defined, will not send message to remote endpoint');
            this.remoteDisabled = true;
            return;
        }
        
        this.fetching = true;
        
        var payloadObj = marshaller.serialize(this.messageQueue,this.multiplex);
        var payload = payloadObj.postBody;
        var contentType = payloadObj.contentType;
    
        var instructions = transportHandler.prepare(this,initialrequest,payload,contentType);

        if (this.multiplex)
        {
            this.messageQueue.clear();
        }
        else
        {
            if (this.messageQueue.length > 0)
            {
                this.messageQueue.removeAt(0);
            }
        }

        var url = instructions.url;
        var method = instructions.method;
        var postBody = instructions.postBody;
        var contentType = instructions.contentType;

        this.sendRequest(url,method,postBody,contentType,marshaller,transportHandler,synchronous);
        
        if (!this.multiplex && this.messageQueue.length > 0)
        {
            this.deliver.defer();
        }
    },
    sendRequest: function(url,method,body,contentType,marshaller,transportHandler,count,synchronous)
    {
        count = (count || 0) + 1;
        
        if (count > 3)
        {
          $E('failed to send request too many times to '+url);
          return;
        }
        var self = this;
        var pending = false;
        
        new Ajax.Request(url,
        {
            asynchronous: !synchronous,
            method: method,
            postBody: body,
            contentType: contentType,
            evalJSON:false,
            evalJS:false,
            onComplete: function()
            {
                self.fetching = false;
            },
            onSuccess: function (result)
            {
                (function()
                {
                    self.fetching = false;
                    self.startTimer(false);
    
                    if (result && result.status && result.status >= 200 && result.status < 300)
                    {
                        if (self.serverDown)
                        {
                            self.serverDown = false;
                            var downtime = new Date().getTime() - self.serverDownStarted;
                            if (Logger.infoEnabled) Logger.info('[' + Appcelerator.Util.DateTime.get12HourTime(new Date(), true, true) + '] ' + self.toString() + ' Server is UP at ' + self.serverPath);
                            self.queue({type:'local:appcelerator.servicebroker.server.up',data:{path:this.serverPath,downtime:downtime}});
                        }

                        var cl = parseInt(result.getResponseHeader("Content-Length") || '1');
                        var contentType = result.getResponseHeader('Content-Type') || 'unknown';
                        
                        if (cl > 0)
                        {
                            var msgs = marshaller.deserialize(result,parseInt(cl),contentType);
                            if (msgs && msgs.length > 0)
                            {
                                for (var c=0;c<msgs.length;c++)
                                {
                                  self.localMessageQueue.push([msgs[c].type,msgs[c].data,'remote',msgs[c].scope,msgs[c].version||'1.0']);
                                }
                            }
                        }
                    }
                }).defer();
            },
            onFailure: function (transport, json)
            {
                //
                // 400 in this case means security check failed
                //
                if (transport.status == 400)
                {
                    if (pending)
                    {
                        self.queue({type:'l:appcelerator.servicebroker.failed',data:{}});
                        Logger.warn("Failed authentication");
                          return;
                    }
                    else
                    {
                        // we're going to retry on a 400 (authentication error) to resend
                        // the request - in this case, we need to re-serialize/prepare the 
                        // request again since most likely our cookie has changed and we need
                        // to re-calculate the security token
                        
                        pending=true;
                        
                        var instructions = transportHandler.prepare(self,false,body,contentType);
                        var newURL = instructions.url;

                        // increment count as a failsafe guard to prevent this from happening too many times
                        self.sendRequest(newURL,method,body,contentType,marshaller,transportHandler,count+1);
                        return;
                    }
                }
                if (transport.status == 406)
                {
                      self.sendRequest(url,method,body,contentType,marshaller,count);
                    return;
                }
                self.fetching = false;
                if (transport.status >= 500 || transport.status == 404)
                {
                    self.serverDown = true;
                    self.serverDownStarted = new Date().getTime();
                    Logger.warn(self.toString() + ' Server is DOWN at ' + self.serverPath);
                    self.queue({type:'local:appcelerator.servicebroker.server.down',data:{path:self.serverPath}});
                }
                else
                {
                    $E(self.toString() + ' Failure: ' + transport.status + ' ' + transport.statusText);
                }

                // restart the timer
                self.startTimer(false);
            },
            onException: function (resp, ex)
            {
                var msg = new String(ex);
                var log = true;
                var restartTimer = true;
                self.fetching = false;

                if (msg.indexOf("NS_ERROR_NOT_AVAILABLE") != -1 && msg.indexOf("nsIXMLHttpRequest.status") != -1)
                {
                    // this is a firefox exception when you call status on the XMLHttpRequest object when
                    // the backend closed the connection, such as in a logout situation.. you can safely
                    // ignore this
                    log = false;
                }
                if (msg.indexOf("NS_ERROR_FILE_NOT_FOUND") != -1)
                {
                    // we might be running local for example
                    restartTimer = false;
                }

                if (log)
                {
                    $E(self.toString() + ' Exception: ' + msg);
                }

                if (restartTimer)
                {
                    // restart timer
                    self.startTimer(false);
                }
            }
        });
    },

    destroy: function ()
    {
        this.cancelTimer();
        this.cancelLocalTimer();
        if (this.messageQueue) this.messageQueue.clear();
        if (this.localMessageQueue) this.localMessageQueue.clear();
        this.messageQueue = null;
        this.time = null;
        if (this.localDirectListeners)
        {
            for (var p in this.localDirectListeners)
            {
                var a = this.localDirectListeners[p];
                if (Object.isArray(a))
                {
                    a.clear();
                }
            }
            this.localDirectListeners = null;
        }
        if (this.remoteDirectListeners)
        {
            for (var p in this.remoteDirectListeners)
            {
                var a = this.remoteDirectListeners[p];
                if (Object.isArray(a))
                {
                    a.clear();
                }
            }
            this.remoteDirectListeners = null;
        }
        if (this.localPatternListeners) this.localPatternListeners.clear();
        this.localPatternListeners = null;
        if (this.remotePatternListeners) this.remotePatternListeners.clear();
        this.remotePatternListeners = null;
    },

    cancelLocalTimer: function ()
    {
        if (this.locaTimer)
        {
            clearInterval(this.locaTimer);
            this.locaTimer = 0;
        }
    },

    cancelTimer: function ()
    {
        if (this.timer)
        {
            clearTimeout(this.timer);
            this.timer = 0;
        }
    },

    onServiceBrokerInvalidContentType: function (type)
    {
        // the default is to redirect to the landing page, but you
        // can set this to do something different if you want
        if (Logger.fatalEnabled) Logger.fatal(this + ', received an invalid content type ('+(type||'none')+'), probably need to re-login, redirecting to landing page');
    },

    onServiceBrokerInvalidLogin: function ()
    {
        // the default is to redirect to the landing page, but you
        // can set this to do something different if you want
        if (Logger.fatalEnabled) Logger.fatal(this + ', received an invalid credentials, probably need to re-login, redirecting to landing page');
    },

    startLocalTimer: function ()
    {        
        this.localTimer = setInterval(function()
        {
            var queue = this.localMessageQueue; 
            if (queue && queue.length > 0)
            {
                var message = queue[0];
                var name = message[0];
                var data = message[1];
                var dest = message[2];
                var scope = message[3];
                var version = message[4];
                var stat = Appcelerator.Util.Performance.logStats ? Appcelerator.Util.Performance.createStat() : null;

                switch (dest)
                {
                    case 'remote':
                    {
                        var arraydirect = this.remoteDirectListeners[name];
                        var arraypattern = this.remotePatternListeners;
                        if (arraydirect)
                        {
                            for (var c = 0, len = arraydirect.length; c < len; c++)
                            {
                                this.sendToListener(arraydirect[c], name, data, 'JSON', dest, scope, version);
                            }
                        }
                        if (arraypattern)
                        {
                            for (var c = 0, len = arraypattern.length; c < len; c++)
                            {
                                var entry = arraypattern[c];
                                var listener = entry[0];
                                var pattern = entry[1];
                                if (pattern.test(name))
                                {
                                    this.sendToListener(listener, name, data, 'JSON', dest, scope, version);
                                }
                            }
                        }
                        break;
                    }
                    case 'local':
                    {
                        arraydirect = this.localDirectListeners[name];
                        arraypattern = this.localPatternListeners;

                        if (arraydirect)
                        {
                            for (var c = 0, len = arraydirect.length; c < len; c++)
                            {
                                this.sendToListener(arraydirect[c], name, data, 'JSON', dest, scope, version);
                            }
                        }
                        if (arraypattern)
                        {
                            for (var c = 0, len = arraypattern.length; c < len; c++)
                            {
                                var entry = arraypattern[c];
                                var listener = entry[0];
                                var pattern = entry[1];
                                if (pattern.test(name))
                                {
                                    this.sendToListener(listener, name, data, 'JSON', dest, scope, version);
                                }
                            }
                        }
                        break;
                    }
                }

                if (Appcelerator.Util.Performance.logStats) Appcelerator.Util.Performance.endStat(stat,name,dest);
                
                queue.remove(message);
            }
        }.bind(this), this.localTimerPoll);
    },
    
    startTimer: function (force)
    {
        if (!this.init)
        {
              // make sure we've initialized
              $D(this.toString()+' - startTimer called but not running yet');
            return;
        }
        
        if (this.disabled)
        {
            return;
        }
        
        // stop the existing timer
        this.cancelTimer();

        // queue could be destroyed
        if (this.messageQueue)
        {
            if (!this.poll)
            {
                if (this.messageQueue.length > 0 && !this.fetching)
                {
                    this.emptyQueueHits = 0;
                    this.deliver(true);
                    return;
                }
                return;
            }
            if (force && !this.fetching)
            {
                this.emptyQueueHits = 0;
                this.deliver(force);
                return;
            }
            // determine how long we've been waiting
            // so on a rapid succession of messages we
            // starve the timer and never send
            var now = new Date().getTime();
            if (this.time)
            {
                // ok, too long, go ahead and send if we have something
                // in the queue
                if (this.messageQueue.length > 0 && now - this.time >= 150 && !this.fetching)
                {
                    this.emptyQueueHits = 0;
                    this.deliver(false);
                    return;
                }
            }
            // start our inactive timer, and fire it even if we
            // don't have messages so we can get messages coming
            // to us
            this.time = now;
            var self = this;
            var qtime = this.downServerPoll;
            if (!this.serverDown)
            {
                if (this.messageQueue.length > 1)
                {
                    qtime = this.moreThanOneEntryQueuePoll;
                    this.emptyQueueHits = 0;
                }
                else
                {
                    if (this.messageQueue.length > 0)
                    {
                        qtime = this.oneEntryQueuePoll;
                        this.emptyQueueHits = 0;
                    }
                    else
                    {
                        // use a backoff decay as we continue to poll with empty messages such that the longer we poll with no data
                        // the longer we wait between polls - this somewhat ensures that the more active we are, the faster we are
                        // polling and the less active, the slower - but with the ability to reset and get faster as we start sending
                        // more messages
                        qtime = Math.min(this.emptyQueuePollMax, this.emptyQueuePoll + (this.emptyQueueHits * this.emptyQueueDecay));
                        this.emptyQueueHits++;
                    }
                }
            }
            this.timer = setTimeout(function()
            {
                if (!self.fetching)
                {
                    self.deliver(false);
                }
            }, qtime);
        }
        else
        {
            $E(toString() + ' startTimer called and we have no message queue');
        }
    },
    maxWaitPollTime: 200,   /*how long to poll the message queue on the server */
    maxWaitPollTimeWhenSending:0, /* how long to poll the message queue on the server when we're sending data */
    moreThanOneEntryQueuePoll: 50, /* how long to wait before sending when we have >1 entry in our send queue */
    oneEntryQueuePoll: 200, /* how long to wait before sending when we have one entry in our send queue */
    emptyQueuePoll: 2000, /* how long to wait before polling when we have an empty queue */
    emptyQueuePollMax: 30000, /* max time we are willing wait before polling */
    emptyQueueDecay: 500, /* how long to decay in ms when we have an empty queue */
    emptyQueueHits: 0, /* use this as the decay multiplier */
    downServerPoll: 10000 /* when the server is down, how long to wait before retrying */
};


//
// convenience macro for doing a message queue request
//
function $MQ (type,data,scope,version)
{
    Appcelerator.Util.ServiceBroker.queue({
        type: type,
        data: data || {},
        scope: scope,
        version: version || '1.0'
    });
}

//
// convenience macro for adding a message queue listener
//
function $MQL (type,f,myscope,element)
{
    var listener = 
    {
        accept: function ()
        {
            if (Object.isArray(type))
            {
                return type;
            }
            return [type];
        },
        acceptScope: function (scope)
        {
            if (myscope)
            {
                return myscope == '*' || scope == myscope;
            }
            return true;
        },
        onMessage: function (type,msg,datatype,from,scope)
        {
            try
            {
                f.apply(f,[type,msg,datatype,from,scope]);
            }
            catch(e)
            {
                Appcelerator.Compiler.handleElementException(element,e,'service broker handling => '+from+':'+type+' ('+Object.toJSON(msg)+'), tried invoking function:'+f);
            }
        }
    };
    
    Appcelerator.Util.ServiceBroker.addListener(listener);

    if (element)
    {
        Appcelerator.Compiler.addTrash(element,function()
        {
            Appcelerator.Util.ServiceBroker.removeListener(listener);
        });
    }

    return listener;
}

if (APPCELERATOR_DEBUG_LEVEL == '2')
{
	//Can do w/ 1 $MQ but we need to know whether it's remote or local..so we do two..
	$MQL('*:~.*',function(type,msg,datatype,from,scope)
	{
	    Logger.info('Message Type: '+from+':' + type + '\nMessage Data: ' + Object.toJSON(msg));
	});
}


Appcelerator.Util.ServiceBrokerMarshaller={};
Appcelerator.Util.ServiceBrokerMarshaller['xml/json'] = 
{
    currentRequestId:1,
    jsonify:function(msg)
    {
        var requestid = this.currentRequestId++;
        var scope = msg[2];
        var version = msg[3];
        var datatype = 'JSON';
        var data = msg[0]['data'];
        var xml = "<message requestid='" + requestid + "' type='" + msg[0]['type'] + "' datatype='" + datatype + "' scope='"+scope+"' version='"+version+"'>";
        xml += '<![CDATA[' + Object.toJSON(data) + ']]>';
        xml += "</message>";
        return xml;
    },
    serialize: function(messageQueue,multiplex)
    {
        var xml = null;
        if (messageQueue.length > 0)
        {
            xml = '';
            var time = new Date();
            var timestamp = time.getTime();
            xml = "<?xml version='1.0' encoding='UTF-8'?>\n";
            var tz = time.getTimezoneOffset()/60;
            var idleMs = Appcelerator.Util.IdleManager.getIdleTimeInMS();
            xml += "<request version='1.0' idle='" + idleMs + "' timestamp='"+timestamp+"' tz='"+tz+"'>\n";
            if (multiplex)
            {
                for (var c = 0,len = messageQueue.length; c < len; c++)
                {
                    xml+=this.jsonify(messageQueue[c]);
                }
            }
            else
            {
                xml+=this.jsonify(messageQueue[0]);
            }
            xml += "</request>";
        }
        return {
            'postBody': xml,
            'contentType':'text/xml'
        };
    },
    deserialize: function(response,length,contentType)
    {
        if (response.status == 202)
        {
            return null;
        }
        if (contentType.indexOf('text/xml')==-1 && contentType.indexOf('application/xml')==-1)
        {
            $E(this+', invalid content type: '+contentType+', expected: text/xml or application/xml');
            return null;
        }
        var xml = response.responseXML;
        if (!xml)
        {
            return null;
        }
        var children = xml.documentElement.childNodes;
        var msgs = null;
        if (children && children.length > 0)
        {
            msgs = [];
            for (var c = 0; c < children.length; c++)
            {
                var child = children.item(c);
                if (child.nodeType == Appcelerator.Util.Dom.ELEMENT_NODE)
                {
                    var requestid = child.getAttribute("requestid");
                    try
                    {
                        var type = child.getAttribute("type");
                        var datatype = child.getAttribute("datatype");
                        var scope = child.getAttribute("scope") || 'appcelerator';
                        var data, text;
                        try
                        {
                            text = Appcelerator.Util.Dom.getText(child);
                            data = text.evalJSON();
                            data.toString = function () { return Object.toJSON(this); };
                        }
                        catch (e)
                        {
                            $E('Error received evaluating: ' + text + ' for type: ' + type + ", error: " + Object.getExceptionDetail(e));
                            return;
                        }
                        $D(this.toString() + ' received remote message, type:' + type + ',data:' + data);
                        msgs.push({type:type,data:data,datatype:datatype,scope:scope,requestid:requestid});
                    }
                    catch (e)
                    {
                        $E(this + ' - Error in dispatch of message. ' + Object.getExceptionDetail(e));
                    }
                }
            }
        }
        return msgs;
    }
};

Appcelerator.Util.ServiceBrokerMarshaller['application/json'] = 
{
    prepareMsg: function(msg)
    {
        var result = {};
        result['type'] = msg[0]['type'];
        result['scope'] = msg[2];
        result['version'] = msg[3];
        result['data'] = msg[0]['data'];
        return result;
    },
    serialize: function(messageQueue,multiplex)
    {
        var json = {};
        if (messageQueue.length > 0)
        {
            var request = {};

            var time = new Date();
            json['timestamp'] = time.getTime()  + (time.getTimezoneOffset()*60*1000);
            json['version'] = '1.0';
            json['messages'] = [];

            if (multiplex)
            {
                for (var c = 0,len = messageQueue.length; c < len; c++)
                {
                    json['messages'].push(this.prepareMsg(messageQueue[c]));
                }
            }
            else
            {
                json['messages'].push(this.prepareMsg(messageQueue[0]));
            }
        }
        return {
            'postBody': Object.toJSON(json),
            'contentType':'application/json'
        };
    },
    deserialize: function(response,length,contentType)
    {
        if (response.status == 202)
        {
            return null;
        }
        if (contentType.indexOf('application/json')==-1)
        {
            $E(this+', invalid content type: '+contentType+', expected: application/json');
            return null;
        }
        
        var result = response.responseText.evalJSON();
        var messages = result['messages'];
        var msgs = [];
        for (var c = 0; c < messages.length; c++)
        {
            try
            {
                var message = messages[c];

                var type = message['type'];
                var datatype = message['datatype'];
                var scope = message['scope'] || 'appcelerator';
                var data = message['data'];
                message['datatype'] = "JSON"; // always JSON
                $D(this.toString() + ' received remote message, type:' + type + ',data:' + data);
                msgs.push({type:type,data:data,datatype:datatype,scope:scope});
            }
            catch (e)
            {
                $E(this + ' - Error in dispatch of message. ' + Object.getExceptionDetail(e));
            }
        }
        return msgs;
    }
};

function jsonParameterEncode (key, value, array)
{
    switch(typeof(value))
    {
        case 'string':
        case 'number':
        case 'boolean':
        {
            array.push(key+'='+encodeURIComponent(value));
            break;
        }
        case 'array':
        case 'object':
        {
            // check to see if the object is an array
            if (Object.isArray(value))
            {
                for (var c=0;c<value.length;c++)
                {
                    jsonParameterEncode(key+'.'+c,value[c],array);
                }
            }
            else
            {
                for (var p in value)
                {
                    jsonParameterEncode(key+'.'+p,value[p],array);
                }
            }
            break;
        }
        case 'function':
        {
            break;
        }
        default:
        {
            array.push(encodeURIComponent(key)+'=');
            break;
        }
    }
}

function jsonToQueryParams(json)
{
    var parameters = [];

    for (var key in json)
    {
        var value = json[key];
        jsonParameterEncode(key,value,parameters);
    }
    
    return parameters.join('&');
};

/**
 * given an element will encode the element and it's children to 
 * json
 */
function json_encode_xml (node,json)
{
   var obj = {};
   var found = json[node.nodeName];
   if (found)
   {
       if (Object.isArray(found))
       {
          found.push(obj);
       } 
       else
       {
          json[node.nodeName] = [found,obj];
       }
   }
   else
   {
       json[node.nodeName] = obj;
   }
 
   Appcelerator.Util.Dom.eachAttribute(node,function(name,value)
   {
       obj[name]=value;
   });
   var added = false;
   Appcelerator.Util.Dom.each(node.childNodes,Appcelerator.Util.Dom.ELEMENT_NODE,function(child)
   {
       json_encode_xml(child,obj);
       added = true;
   });
   if (!added)
   {
     var text = Appcelerator.Util.Dom.getText(node);
     json[node.nodeName] = text;
   }
}


/**
 * Form URL encoded service marshaller
 */
Appcelerator.Util.ServiceBrokerMarshaller['application/x-www-form-urlencoded'] = 
{
    currentRequestId:1,
    parameterize: function(msg)
    {
        var requestid = this.currentRequestId++;
        var scope = msg[2];
        var version = msg[3];
        var datatype = 'JSON';
        var data = msg[0]['data'];

        var str = '$messagetype='+msg[0]['type']+'&$requestid='+requestid+'&$datatype='+datatype+'&$scope='+scope+'&$version='+version;
        
        if (data)
        {
            str+='&' + jsonToQueryParams(data);
        }
        return str;
    },
    serialize: function(messageQueue,multiplex)
    {
        var xml = null;
        if (messageQueue.length > 0)
        {
            xml = '';
            if (multiplex)
            {
                for (var c = 0,len = messageQueue.length; c < len; c++)
                {
                    xml+=this.parameterize(messageQueue[c]);
                }
            }
            else
            {
                xml+=this.parameterize(messageQueue[0]);
            }
        }
        return {
            'postBody': xml,
            'contentType':'application/x-www-form-urlencoded'
        };
    },
    deserialize: function(response,length,contentType)
    {
        if (response.status == 202)
        {
            return null;
        }
        if (contentType.indexOf('/json')==-1)
        {
            $E(this+', invalid content type: '+contentType+', expected json mimetype');
            return null;
        }
        return response.responseText.evalJSON(true);
    }
};

Appcelerator.Util.ServiceBrokerTransportHandler = {};

/**
 * Appcelerator based protocol transport handler
 */
Appcelerator.Util.ServiceBrokerTransportHandler['appcelerator'] = 
{
    prepare: function(serviceBroker,initialrequest,payload,contentType)
    {
        // get the auth token
        var cookieName = Appcelerator.ServerConfig['sessionid'].value||'JSESSIONID';
        var authToken = Appcelerator.Util.Cookie.GetCookie(cookieName);
        
        // calculate the token we send back to the server
        var token = (authToken) ? Appcelerator.Util.MD5.hex_md5(authToken+serviceBroker.instanceid) : '';
        
        // create parameters for URL
        var parameters = "?maxwait=" + ((!initialrequest && payload) ? serviceBroker.maxWaitPollTime : serviceBroker.maxWaitPollTimeWhenSending)+"&instanceid="+serviceBroker.instanceid+'&auth='+token+'&ts='+new Date().getTime();
        var url = serviceBroker.serverPath + parameters;
        var method = payload ? 'post' : 'get';
        
        return { 
            'url':url, 
            'method': method, 
            'postBody': payload||'', 
            'contentType':contentType 
        };
    }
};


Appcelerator.Util.ServiceBrokerInterceptor = Class.create();
/**
 * ServiceBrokerInterceptor interceptor prototype class
 */
Object.extend(Appcelerator.Util.ServiceBrokerInterceptor.prototype, {

    initialize: function ()
    {
    },

    toString: function ()
    {
        return '[Appcelerator.Util.ServiceBrokerInterceptor]';
    },

    interceptQueue: function (msg, callback)
    {
        return true;
    },

    interceptDispatch: function (requestid, type, data, datatype)
    {
        return true;
    },

    interceptSendToListener: function (listener, type, msg, datatype)
    {
        return true;
    }
});

if (Appcelerator.Util.ServiceBroker.disabled || Appcelerator.Util.ServiceBroker.remoteDisabled)
{
    Appcelerator.Util.ServiceBroker.triggerConfig();
    Logger.warn('[ServiceBroker] remote delivery of messages is disabled');
}
else
{
    Appcelerator.Util.ServerConfig.addConfigListener(function()
    {
        var config = Appcelerator.ServerConfig['servicebroker'];
        if (!config || config['disabled']=='true')
        {
            Appcelerator.Util.ServiceBroker.disabled = true;
            Appcelerator.Util.ServiceBroker.remoteDisabled = true;
            Appcelerator.Util.ServiceBroker.triggerConfig();
            Logger.warn('[ServiceBroker] remote delivery of messages is disabled');
            return;
        }
        Appcelerator.Util.ServiceBroker.serverPath = config.value;
        Appcelerator.Util.ServiceBroker.poll = (config.poll == 'true');
        Appcelerator.Util.ServiceBroker.multiplex = config.multiplex ? (config.multiplex == 'true') : true;
        Appcelerator.Util.ServiceBroker.transport = config.transport || Appcelerator.Util.ServiceBroker.transport;
        Appcelerator.Util.ServiceBroker.marshaller = config.marshaller || Appcelerator.Util.ServiceBroker.marshaller;
        
        var cookieName = Appcelerator.ServerConfig['sessionid'].value || 'unknown_cookie_name';
        var cookieValue = Appcelerator.Util.Cookie.GetCookie(cookieName);
        
        if (!cookieValue)
        {
            new Ajax.Request(Appcelerator.Util.ServiceBroker.serverPath+'?initial=1',
            {
                asynchronous: true,
                method: 'get',
                evalJSON:false,
                evalJS:false,
                onSuccess:function(r)
                {
                    Appcelerator.Util.ServiceBroker.triggerConfig();
                    Appcelerator.Util.ServiceBroker.startTimer();
                    Logger.info('ServiceBroker ready');
                }
            });
            return;
        }
        
        
        Appcelerator.Util.ServiceBroker.triggerConfig();
        Appcelerator.Util.ServiceBroker.startTimer();
        Logger.info('ServiceBroker ready');
    });
}


Appcelerator.Core.onunload(Appcelerator.Util.ServiceBroker.destroy.bind(Appcelerator.Util.ServiceBroker));
Appcelerator.Compiler.afterDocumentCompile(function()
{
    Appcelerator.Util.ServiceBroker.triggerComplete();
});


Appcelerator.Util.Performance = Class.create();
Appcelerator.Util.Performance = 
{
    logStats: (window.location.href.indexOf('logStats=1') > 0),
    stats: $H({}),
    createStat: function ()
    {
        if (this.logStats)
            return new Date();
    },
    endStat: function (start,type,category,data)
    {
        if (this.logStats)
        {
            var id = type;
            if (category)
                id = id+'.'+category
            else
                category='';
            var end = new Date();
            var stat = this.stats.get(id);
            var diff = (end.getTime() - start.getTime());
            if (!stat)
            {
                var stat = {'type':type,'category':category,'hits':0,'mean':0,'min':diff,'max':diff,'total':0};
                this.stats.set(id,stat);
            }
            stat.hits++;
            stat.last = diff;
            stat.total +=diff;
            stat.mean = stat.total/stat.hits;
            stat.max = (stat.last > stat.max ? stat.last : stat.max); 
            stat.min = (stat.last < stat.min ? stat.last : stat.min); 
            Logger.info('stats: ' + type + ' last:' + stat.last + 'ms mean:'+stat.mean+'ms hits:'+stat.hits + ' min:'+stat.min+'ms max:'+stat.max+'ms total:' + stat.total+'ms');
        }
    },
    reset: function (start,type,data)
    {
        this.stats = $H({});
    }
};
$MQL('l:get.performance.request',function(type,msg,datatype,from)
{
    $MQ('l:get.performance.response',{'stats':Appcelerator.Util.Performance.stats.values()});
});
$MQL('l:reset.performance.request',function(type,msg,datatype,from)
{
    Appcelerator.Util.Performance.reset();
});


/**
 * Used for defining metadata of widgets,
 * and maybe of conditions and actions.
 * 
 * Some of these types we won't define checkers for,
 * but will use them instead for auto-completion in the IDE.
 *
 */
Appcelerator.Types = {};

Appcelerator.Types.enumeration = function()
{
    var pattern = '^('+ $A(arguments).join(')|(') +')$';
    return {name: 'Enumeration', values: $A(arguments), regex: RegExp(pattern)};
};
Appcelerator.Types.openEnumeration = function()
{
	// accepts anything, suggests some things that it knows will work
    var pattern = '^('+ $A(arguments).join(')|(') +')|(.*)$';
    return {name: 'Enumeration', values: $A(arguments), regex: RegExp(pattern)};
};
Appcelerator.Types.pattern = function(regex, name)
{
	name = name || 'pattern';
    return {name: name, regex: regex};
};

Appcelerator.Types.bool = Appcelerator.Types.enumeration('true','false');
Appcelerator.Types.bool.name = "Boolean"
Appcelerator.Types.number = Appcelerator.Types.pattern(/^-?[0-9]+(\.[0-9]+)$/, 'Number');
Appcelerator.Types.naturalNumber = Appcelerator.Types.pattern(/[0-9]+/, 'Natural Number');

Appcelerator.Types.cssDimension = Appcelerator.Types.pattern(
    /^-?[0-9]+(\.[0-9]+)(%|(em)|(en)|(px)|(pc)|(pt))?$/, 'CSS Dimension');
	
Appcelerator.Types.identifier = Appcelerator.Types.pattern(
    /^[a-zA-Z_][a-zA-Z0-9_.]*$/, 'Identifier');

/*
 * Message sends can only be literal message names,
 * while message receptions can include a matching regex.
 * The distinction between these can also be used by tools
 * to detect messages sent but not received, and vice-versa
 * (probably indicating a typo).
 */
Appcelerator.Types.messageSend = Appcelerator.Types.pattern(
    /^((l:)|(r:)|(local:)|(remote:))[a-zA-Z0-9_.]+/, 'Appcelerator Message Send');
Appcelerator.Types.messageReceive = Appcelerator.Types.pattern(
    /^((l:)|(r:)|(local:)|(remote:))((~.+)|([a-zA-Z0-9_.]+))/, 'Appcelerator Message Reception');

Appcelerator.Types.onExpr          = {name: 'On Expression'};
Appcelerator.Types.fieldset        = {name: 'Fieldset'};
Appcelerator.Types.json            = {name: 'JSON Object'};
Appcelerator.Types.javascriptExpr  = {name: 'Javascript Expression'}
Appcelerator.Types.pathOrUrl       = {name: 'Path or url to resource'};
Appcelerator.Types.cssClass        = {name: 'CSS Class name'}; 
Appcelerator.Types.color           = {name: 'Color value'};
Appcelerator.Types.time            = {name: 'Time value'};
Appcelerator.Types.elementId       = {name: 'Element Id'};
Appcelerator.Types.commaSeparated  = {name: "Comma Separated Values"};
Appcelerator.Types.languageId      = {name: "Localization String Id"};

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
 * @param {Appcelerator.Type} type
 */
Appcelerator.Types.isInstance = function(value,type)
{
	if(type.regex)
	{
		return type.regex.match(value);
	}
	
	switch(type)
	{
		case Appcelerator.Types.onExpr:
		  try
		  {
		      var thens = Appcelerator.Compiler.parseExpression(value);
			  return thens && thens.length > 0;
		  }
		  catch(e)
		  {
		  	   return false;
		  }
		case Appcelerator.Types.time:
		  return value && !isNaN(Appcelerator.Util.DateTime.timeFormat(value));
		  
		case Appcelerator.Types.fieldset:
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
Appcelerator.Types.getTypeId = function(type) {
	try {
	   return $H(Appcelerator.Types).find(function(kv) {
		  return kv[1] == type;
	   })[0];
	} catch(e) {
		// the given type was not defined in the Appcelerator.Types namespace
		return null;
	}
};
Appcelerator.Compiler.registerCustomAction('show',
{
	execute: function(id, action, params)
	{
		if (params && params.length > 0)
		{
			var obj = params[0];
			var key = obj.key;
			Element.show(key);
		}
		else
		{
		    Element.show(id);
		}
	}
});

Appcelerator.Compiler.registerCustomAction('hide',
{
	execute: function(id,action,params)
	{
		if (params && params.length > 0)
		{
			var optional = false;
			var key = null;
			for (var c=0;c<params.length;c++)
			{
				if (params[c].key=='optional')
				{
					optional = params[c].value;
					continue;
				}
				if (params[c].key=='id')
				{
					key = params[c].value;
					continue;
				}
				if (!key)
				{
					key = params[c].key;
				}
			}
			if (!key && !optional)
			{
				throw "no key specified for hide action";
			}
			var el = $(key);
			if (!el && !optional)
			{
				throw "no element with id: "+key+" found for hide action";
			}
			if (el) Element.hide(el);
		}
		else
		{
		    Element.hide(id);
		}
	}
});

Appcelerator.Compiler.registerCustomAction('visible',
{
	execute: function(id,action,params)
	{
		if (params && params.length > 0)
		{
			var obj = params[0];
			var key = obj.key;
			Element.setStyle(key, {visibility:'visible'});
		}
		else
		{
			Element.setStyle(id, {visibility:'visible'});
		}
	}
});

Appcelerator.Compiler.registerCustomAction('hidden',
{
	execute: function(id,action,params)
	{
		if (params && params.length > 0)
		{
			var obj = params[0];
			var key = obj.key;
			Element.setStyle(key, {visibility:'hidden'});
		}
		else
		{
			Element.setStyle(id, {visibility:'hidden'});
		}
	}
});

// get the effects
Appcelerator.Compiler.effects = $H(Effect).select(function(kv) {
    var name = kv[0];
    var val = kv[1];
    var lowerName = name.charAt(0).toLowerCase() + name.substring(1);
    return Effect.Methods[lowerName] || val.superclass == Effect.Base;
}).pluck('0');

// push ScrollTo since it's valid but doesn't extend Effect.Base
Appcelerator.Compiler.effects.push('ScrollTo');



Appcelerator.Compiler.customEffects = {};

Appcelerator.Compiler.registerCustomEffect = function(effect,callback)
{
	Appcelerator.Compiler.customEffects[effect] = callback;
};

Appcelerator.Compiler.getMoveByOpts = function(params)
{
	var x = Appcelerator.Compiler.findParameter(params,'x');
	x  = (x==null)?0:x;
	var y = Appcelerator.Compiler.findParameter(params,'y');
	y  = (y==null)?0:y;
	var duration = Appcelerator.Compiler.findParameter(params,'duration');
	duration  = (duration==null)?2:duration;
	return {'x':x,'y':y,'duration':duration};
}

Appcelerator.Compiler.registerCustomEffect('Bounce', function(id,params)
{
	opts = Appcelerator.Compiler.getMoveByOpts(params)
	new Effect.MoveBy( $(id), opts['x'], opts['y'], {duration: opts['duration'], transition: Effect.Transitions.SwingTo});
});

Appcelerator.Compiler.registerCustomEffect('Bang', function(id,params)
{
	opts = Appcelerator.Compiler.getMoveByOpts(params)
	new Effect.MoveBy( $(id), opts['x'], opts['y'], {duration: opts['duration'], transition: Effect.Transitions.Bounce});
		
});

Appcelerator.Compiler.registerCustomAction('effect',
{
	metadata:
	{
		requiresParameters: true,
		shorthandParameters: true,
		optionalParameterKeys: Appcelerator.Compiler.effects,
	    description: "Invokes a visual effect on this element"
	},

	execute: function(id,action,params)
	{
		if (params && params.length > 0)
		{
			// split first param to get effect name
			var arg1= params[0].key.split(",");
			var effectName = arg1[0];
			effectName = effectName.dasherize().camelize();
		  	effectName = effectName.charAt(0).toUpperCase() + effectName.substring(1);

			var options = {};
			var target=id

			// get remaining options
			if (params.length > 1)
			{
				for (var c=1;c<params.length;c++)
				{
					// if option is id, set target element for effect
					if (params[c].key=="id")
					{
						target = params[c].value;
						options['id']=target;
					}
					// is it a transition
					else if (params[c].key == "transition")
					{
						var t = params[c].value;
						if (!t.startsWith('Effect.Transitions.'))
						{
							// this will simply first see if the effect is standalong (outside of effects package)
							// and if not, attempt to add the package and try again... this allows us to use 
							// shorthand transitions for core such as SwingTo
							var error = false;
							var x = 0;
							while (x < 1)
							{
								x=1;
								// we first check to see if this is a valid object
								eval("try { " + t + "; error = false;} catch(e) { error = true; }");
								if (error)
								{
									t = 'Effect.Transitions.' + t;
								}
								else
								{
									break;
								}
							}
						}
						var f = Object.evalWithinScope(t,{});
						if (Object.isFunction(f))
						{
						    options[params[c].key] = f;
						}
					}
					// otherwise, its an effect option
					else
					{
					    options[params[c].key] = params[c].value;
					}
				}
			}

			// first allow overriden built-in effects 
			if (Appcelerator.Compiler.customEffects[effectName])
			{
				var f = Appcelerator.Compiler.customEffects[effectName];
				f(id,options);
			}
			// and now check for standard effects
			else if (Appcelerator.Compiler.effects.indexOf(effectName) != -1)
			{
				Element.visualEffect(target,effectName,options);
			}
			else
			{
				throw "syntax error: unsupported effect type: "+effectName;				
			}
		}
		else
		{
			throw "syntax error: effect action must have parameters.";
		}
	}
});


Appcelerator.Compiler.registerCustomAction('toggle',
{
	metadata:
	{
		requiresParameters: true,
		description: "Toggles a CSS property, CSS Class, or boolean attribute on this element"
	},
	execute: function(id,action,params)
	{
		if (params && params.length > 0)
		{
			var obj = params[0];
			var key = obj.key;
			var val = obj.value;
			var code = null;
			if (key == 'class')
			{
			    if (Element.hasClassName(id,val))
			    {
			        Element.removeClassName(id,val);
			    }
			    else
			    {
			        Element.addClassName(id,val);
		        }
			}
			else
			{
				if (Appcelerator.Compiler.isCSSAttribute(key))
				{
					key = Appcelerator.Compiler.convertCSSAttribute(key);
					switch (key)
					{
						case 'display':
						case 'visibility':
						{
							var opposite = '';
							if (!val)val = (key =='display') ? 'block': 'visible';
							switch(val)
							{
								case 'inline':
									opposite='none';break;
								case 'block':
									opposite='none'; break;
								case 'none':
									opposite='block'; break;
								case 'hidden':
									opposite='visible'; break;
								case 'visible':
									opposite='hidden'; break;
							}
							var a = Element.getStyle(id,key);
						    var params = {};
							if (a!=opposite)
							{
							    params[key] = opposite;
							}
							else
							{
							    params[key] = val;
						    }
						    Element.setStyle(id,params);
							break;
						}
						default:
						{
							var a = Element.getStyle(id, key);
						    var params = {};
							if (a)
							{
							    params[key] = '';
							}
							else
							{
							    params[key] = val;
						    }
						    Element.setStyle(id,params);
							break;
						}
					}
				}
				else
				{
					var a = $(id);
					if (!a)
					{
					    throw "no element with ID: "+id;
					}
					var v = a.getAttribute(key);
					if (v)
					{
					    a.removeAttribute(key);
					}
					else
					{
					    a.setAttribute(key,val);
				    }
				}
			}
		}
		else
		{
			throw "syntax error: toggle action must have parameters";
		}
	}
});

Appcelerator.Compiler.generateSetter = function(value,scope)
{
    if (scope)
    {
    	return Appcelerator.Compiler.getEvaluatedValue(value,scope.data||{});
    }
    else
    {
    	return Appcelerator.Compiler.getEvaluatedValue(value,{});
    }
};

Appcelerator.Compiler.findTargetFromParams = function(id,params)
{
	var target = id;
	if (params && params.length > 0)
	{
		for (var c=0;c<params.length;c++)
		{
			var entry = params[c];
			if (entry.key == 'id')
			{
				target = entry.value;
				break;
			}
		}
	}
	return $(target);
};

(function()
{
	var addsetBuildFunction = function(id,action,params,scope)
	{
		if (params.length == 0)
		{
			throw "syntax error: expected parameter key:value for action: "+action;
		}
		
		var setStyles = null;
		var target = Appcelerator.Compiler.findTargetFromParams(id,params);
		
		for (var c=0;c<params.length;c++)
		{
			var obj = params[c];
			var key = obj.key;
			if (!Object.isString(key)) continue;
			var value = obj.value;
			if (Appcelerator.Compiler.isCSSAttribute(key))
			{
				key = Appcelerator.Compiler.convertCSSAttribute(key);
				if (!setStyles) setStyles = {};
				setStyles[key] = Appcelerator.Compiler.generateSetter(value,scope);
				continue;
			}
			else if (key == 'class')
			{
				if (action=='set')
				{
					$(target).className = Appcelerator.Compiler.generateSetter(value,scope);
				}
				Element.addClassName(id, Appcelerator.Compiler.generateSetter(value,scope));
			}
			else if (key.startsWith('style'))
			{
			    $(target)[key] = Appcelerator.Compiler.generateSetter(value,scope);
		    }
		    else
		    {
				var e = $(target);
				if (!e)
				{
				    throw "syntax error: element with ID: "+target+" doesn't exist";
				}
				if (e.nodeName=='IFRAME' && key=='src')
				{
				    var onload = e.getAttribute('onloaded');
				    if (onload)
				    {
				        Appcelerator.Util.IFrame.monitor(e, function()
				        {
				            $MQ(onload,{},e.scope);
				        });
			        }
			    }
				if (e[key]!=null)
				{
	    			switch(key)
	    			{
	    				case 'checked':
	    				case 'selected':
	    				case 'disabled':
						case 'defaultChecked':
	    				{
							var value = Appcelerator.Compiler.generateSetter(value,scope);
							if (value)
							{
								e.setAttribute(key,value);
							}
							else
							{
								e.removeAttribute(key);
							}
	    					break;
						}
	    				default:
	    				{
	            			var isOperaSetIframe = Appcelerator.Browser.isOpera && e.nodeName=='IFRAME' && key=='src';
	    				    if (isOperaSetIframe)
	    				    {
	    				        e.location.href = Appcelerator.Compiler.generateSetter(value,scope);
	    				    }
	    				    else
	    				    {
	    				        e[key] = Appcelerator.Compiler.generateSetter(value,scope);
	    				    }
						}
	    			}
			    }
			    else
			    {
			        e.setAttribute(key, Appcelerator.Compiler.generateSetter(value,scope));
			    }
			}
		}
		if (setStyles)
		{
			Element.setStyle(target, setStyles);
		}
	}

    Appcelerator.Compiler.registerCustomAction('add',
	{
		metadata:
		{
			description: "Add a CSS property or attribute on this element"
		},
		execute: addsetBuildFunction
	});
    Appcelerator.Compiler.registerCustomAction('set',
    {
        metadata:
        {
            description: "Set a CSS property or attribute on this element"
        },
        execute: addsetBuildFunction
    });
})();

Appcelerator.Compiler.registerCustomAction('remove',
{
	execute: function(id,action,params)
	{
		if (params.length == 0)
		{
			throw "syntax error: expected parameter for action: "+action;
		}
		var target = Appcelerator.Compiler.findTargetFromParams(id,params);
		var key = null;
		var value = null;
		var optional = false;
		for (var c=0;c<params.length;c++)
		{
			if (params[c].key == 'id') continue;
			if (params[c].key == 'optional') 
			{
				optional = params[c].value;
				continue;
			}
			if (!key)
			{
				key = params[c].key;
				value = params[c].value;
			}
		}
		if (!key && !optional)
		{
			throw "couldn't find key to remove";
		}
		if (key)
		{
			switch (key)
			{
				case 'class':
				    Element.removeClassName(target,Appcelerator.Compiler.formatValue(value,false));
				    break;
				default:
	    			target.removeAttribute(key);
			}
		}
	}
});

Appcelerator.Compiler.registerCustomAction('statechange',
{
	metadata:
	{
		requiresParameters: true
	},
	execute: function(id,action,params)
	{
		if (params.length == 0)
		{
			throw "syntax error: expected parameters in format 'statechange[statemachine=state]'";
		}

		var changes = params.map(function(obj)
		{
			var statemachine = obj.key;
			var state = obj.value;
			Appcelerator.Compiler.StateMachine.fireStateMachineChange(statemachine,state,true,true,false);
		});
	}
});

(function()
{
	var scriptBuilderAction =
	{
		metadata:
        {
            requiresParameters: true,
			description: "Executes a line of javascript"
        },
	    //
	    // define a custom parsing routine for parameters
	    // that will just take in as-is everything inside
	    // [ ] as the code to execute
	    //
	    parseParameters: function (id,action,params)
	    {
	        return params;
	    },
	    execute: function (id,action,params,scope)
	    {   
	        var f = function() { eval(params); }.bind(scope);
	        f();
	    }
	};


	Appcelerator.Compiler.registerCustomAction('javascript',scriptBuilderAction);
	Appcelerator.Compiler.registerCustomAction('function',scriptBuilderAction);
	Appcelerator.Compiler.registerCustomAction('script',scriptBuilderAction);
})();


Appcelerator.Compiler.findParameter = function(params,key)
{
	if (params)
	{
		if (params[key])
		{
			return params[key];
		}
		else
		{
			if (params.length > 0)
			{
				for (var c=0;c<params.length;c++)
				{
					var obj = params[c];
					if (obj.key == key)
					{
						return obj.value;
					}
				}
			}
		}
	}
	return null;
}

Appcelerator.Compiler.registerCustomAction('selectOption',
{
	execute: function(id,action,params,scope)
	{
		if (params.length == 0)
		{
			throw "syntax error: expected parameter property for action: "+action;
		}
		var select = $(id);

		if (!select.options)
		{
			throw "syntax error: selectOption must apply to a select tag";
		}

		var key = params[0].key;
		var def = params[0].value || key;
		if (def=='$null')
		{
			def = '';
		}

		var selectedValue = Object.getNestedProperty(scope.data, key, def);
		var targetSelect = $(id);
		var isArray = Object.isArray(selectedValue);

        targetSelect.selectedIndex = -1;

		for (var j=0;j<targetSelect.options.length;j++)
		{
			if (isArray)
			{
				targetSelect.options[j].selected = selectedValue.include(targetSelect.options[j].value);
			}
			else
			{
			    if (targetSelect.options[j].value == selectedValue)
			    {
			        targetSelect.selectedIndex = j;
			        break;
			    }
			}
		}
		Appcelerator.Compiler.executeFunction(targetSelect,'revalidate');
	}
});

var ResetAction =
{
	execute: function(id,action,params)
	{
		var target = Appcelerator.Compiler.findParameter(params,'id') || id;
		var element = $(target);
		var revalidate = false;
		var code = null;

		var element = $(target);
		var variable = '';
		var value = '';

		switch (Appcelerator.Compiler.getTagname(element))
		{
			case 'input':
			case 'textarea':
			{
			    element.value = value;
				revalidate=true;
				break;
			}
			case 'select':
			{
			    element.selectedIndex = 0;
				revalidate=true;
				break;
			}
			case 'img':
			{
			    element.src = '';
				break;
			}
			case 'a':
			{
			    element.href = '#';
				break;
			}
			case 'form':
			{
				Form.reset(target);
				Form.Methods.getInputs(target).each(function(i)
				{
				    Appcelerator.Compiler.executeFunction(i,'revalidate');
				});
                return;
			}
			default:
			{
				element.update(value);
				return;
			}
		}

		if (revalidate)
		{
		    Appcelerator.Compiler.executeFunction(element,revalidate);
		}
	}
};
Appcelerator.Compiler.registerCustomAction('clear',ResetAction);
Appcelerator.Compiler.registerCustomAction('reset',ResetAction);

var ResetFormAction =
{
	execute: function(id,action,params)
	{
		var target = Appcelerator.Compiler.findParameter(params,'id') || id;
		var element = $(target);
		var code = null;
		var form = null;

		switch (Appcelerator.Compiler.getTagname(element))
		{
			case 'form':
			{
			    $(target).reset();
				break;
			}
			case 'input':
			case 'select':
			case 'textarea':
			default:
			{
			    $(target).form.reset();
				break;
			}
		}
	}
};
Appcelerator.Compiler.registerCustomAction('clearform',ResetFormAction);

Appcelerator.Compiler.registerCustomAction('value',
{
	metadata:
    {
        requiresParameters: true,
		requiredParameterKeys: ['value'],
		optionalParameterKeys: ['text','append','id','property'],
		description: "Sets the value of the current element with data from the message payload"
    },
	execute: function(id,action,params,scope)
	{
		var element = Appcelerator.Compiler.findTargetFromParams(id,params);
		var valueHtml = null;
		var append = false;
		var form = false;
		var key = null;
		var value = null;
		
		if (params)
		{
			for (var c=0,len=params.length;c<len;c++)
			{
				var param = params[c];
				switch(param.key)
				{
					case 'append':
					{
						append=true;
						break;
					}
					case 'value':
					{
						valueHtml = param.value;
						break;
					}
					default:
					{
						key = param.key;
						value = param.value;
						if (param.empty)
						{
							if (key.startsWith("'") && key.endsWith("'"))
							{
								value = Appcelerator.Compiler.dequote(param.key);
								value = null;
							}
						}
					}
				}
			}
		}
		
		if (!key && !valueHtml)
		{
			key = params[0].key;
			value = params[0].value;
		}
		
		if (!valueHtml)
		{
			if (!value && key && key.startsWith("'") && key.endsWith("'"))
			{
				valueHtml = Appcelerator.Compiler.dequote(key);
			}
			else if (!value)
			{
				valueHtml = Object.getNestedProperty(scope,key);
			}
			else if (value)
			{
				if (typeof(value)=='object')
				{
					valueHtml = Object.getNestedProperty(value,key);
				}
				else
				{
					valueHtml = value;
				}
			}
		}

		var html = '';
		var variable = '';
		var expression = '';

		var revalidate = false;

		switch (Appcelerator.Compiler.getTagname(element))
		{
			case 'input':
			{
				revalidate = true;
				var type = element.getAttribute('type') || 'text';
				switch (type)
				{
					case 'password':
					case 'hidden':
					case 'text':
					{
						variable='value';
						break;
					}
/*					case 'radio': TODO- fix me */
					case 'checkbox':
					{
						variable='checked';
						append=false;
						expression = "==true || " + valueHtml + "=='true'";
						break;
					}
					case 'button':
					case 'submit':
					{
						variable='value';
						break;
					}
				}
				break;
			}
			case 'textarea':
			{
				revalidate = true;
				variable = 'value';
				break;
			}
			case 'select':
			{
				// select is a special beast
				var code = '';
				var property = Appcelerator.Compiler.findParameter(params,'property');
				var row = Appcelerator.Compiler.findParameter(params,'row');
				var value = Appcelerator.Compiler.findParameter(params,'value');
				var text = Appcelerator.Compiler.findParameter(params,'text');
				if (!property) throw "required parameter named 'property' not found in value parameter list";
				if (!value) throw "required parameter named 'value' not found in value parameter list";
				if (!text) text = value;
				if (!append)
				{
				    element.options.length = 0;
				}
				var ar = Appcelerator.Compiler.generateSetter(property,scope);
				if (ar)
				{
				    for (var c=0;c<ar.length;c++)
				    {
				        if (row)
				        {
				            var rowData = Object.getNestedProperty(ar[c],row);
				        }
				        else
				        {
				            var rowData = ar[c];
				        }
				        if (rowData)
				        {
				            element.options[element.options.length] = new Option(Object.getNestedProperty(rowData, text), Object.getNestedProperty(rowData, value));
				        }
				    }
				}
                Appcelerator.Compiler.executeFunction(element,'revalidate');
				return;
			}
			case 'div':
			case 'span':
			case 'p':
			case 'a':
			case 'h1':
			case 'h2':
			case 'h3':
			case 'h4':
			case 'h5':
			case 'td':
			case 'code':
			case 'li':
			case 'blockquote':
			{
				variable = 'innerHTML';
				break;
			}
			case 'img':
			case 'iframe':
			{
				append=false;
				variable = 'src';
				break;
			}
			case 'form':
			{
				//Guarantee that the form will not auto-submit when someone hits enter by adding 1 display:none text field
				var new_input_id = id+'_no_submit';
				if (!$(new_input_id)) 
				{
					var new_input = document.createElement('input');
					new_input.id = new_input_id;
					new_input.type = 'text';
					new_input.style.display = 'none';
					new_input.name = 'no_submit_guarantee';
					element.appendChild(new_input);
				}

				//Set form to true so we clear html var below -- we delegate to subsequent calls to handleCondition
				form = true;

				//e.g. value[bar]
				var elementAction = 'value['+key+']';
				//find the matching clause (in case the form has several actions in its on expression); e.g. r:foo
				var clause = this.findMatchingFormClause(element,elementAction);

				var descendants = element.descendants();
				
				for (var c = 0; c < descendants.length; c++)
				{
					var child = descendants[c];
					
					//need an id to handle the condition later and probably need one anyway so make sure it's there
					Appcelerator.Compiler.getAndEnsureId(child);
					var child_parameter;
					switch(Appcelerator.Compiler.getTagname(child))
					{
						 case 'select':
						 case 'textarea':
						 case 'input':
						 {
							  child_parameter = child.getAttribute('name') || child.id || ''
							  break;
						 }
						 default:
						 {
							  /**
							   * We don't look for an id as the value to read out on normal elements since divs, spans, etc.
							   * may have ids for styling, etc. but we do not want to overwrite text for labels etc.
							   * For divs, spans, etc. we require the name attribute if they are to be populated with data
							   * without their own explicit on expression (that is when the on expression is on a form tag).
							   */
							   child_parameter = child.getAttribute('name') || '';
						 }
					}
					
					if (child_parameter)
					{
						//e.g. value[bar.idx]
						var action = 'value['+ key + '.' + child_parameter+']';
						Appcelerator.Compiler.handleCondition.call(this, [child,true,action,scope,null,null]);
					}
				}
				break;
			}
			default:
			{
				throw "syntax error: " + element.nodeName+' not supported for value action';
			}
		}

		if (!form)
		{
			if (append)
			{
			    var val = element[variable];
			    element[variable] = val + valueHtml + expression;
			}
			else
			{
			    element[variable] = valueHtml + expression;
			}
			if (revalidate)
			{
			    Appcelerator.Compiler.executeFunction(element, 'revalidate');
			}
		}
	},
	findMatchingFormClause: function(element, params)
	{
		//iterate over the clauses and find the appropriate clause to return
		//(the one with the appropriate action being handled by the cal for registerCustomAction('value'))
		var clauses = Appcelerator.Compiler.parseExpression(element.getAttribute('on'));

		for (var i = 0; i < clauses.length; i++)
		{
			var condition = clauses[i][2];
			if (condition == params)
			{
				return clauses[i];
			}
		}
		return [];
	}
});

Appcelerator.Compiler.registerCustomAction('bind',
{
	metadata:
    {
        requiresParameters: false,
		description: "Sets the value to all elements with the same fieldset to the data from the payload"
    },
	parseParameters: function (id,action,params,scope)
	{
		return params;
	},
	execute: function(id,action,parameters,scope)
	{
		var element = $(id);
		var fieldset = element.getAttribute('fieldset');

		if (!fieldset || fieldset == '')
		{
			throw "syntax error: element has no field set attribute "  + element;
		}

		var key = parameters;

		Appcelerator.Compiler.updateFieldsetValues(fieldset, scope.data, key);
	}
});

Appcelerator.Compiler.executeActionFunction = function(id,method,params,checkenabled,scope)
{
	var target = Appcelerator.Compiler.findParameter(params,'id') || id;
	if (checkenabled)
	{
	    try
	    {
	        var e = $(target);
	        if (e && !e.disabled && Element.showing(e))
	        {
	            Appcelerator.Compiler.executeFunction(target, method, [target, method, scope.data, scope.scope, scope.version, params, scope.direction, scope.type]);
	        }
	    }
	    catch (xxx_)
	    {
        }
	}
	else
	{
		scope = (!scope) ? {} : scope;
        Appcelerator.Compiler.executeFunction(target, method, [target, method, scope.data, scope.scope, scope.version, params, scope.direction, scope.type]);
	}
};

var GenericActionFunction = Class.create();
Object.extend(GenericActionFunction.prototype,
{
	initialize: function(check, widgetAction)
	{
		this.checkenabled = check;
		this.widgetAction = !!widgetAction;
	},
	execute: function(id,action,params,scope)
	{
        Appcelerator.Compiler.executeActionFunction(id,action,params,this.checkenabled,scope);
	}
});

Appcelerator.Compiler.GenericFunctions =
[
	['execute',false],
	['enable',false],
	['disable',false],
	['focus',true],
	['blur',true],
	['select',false],
	['selected',false],
	['unselect',false],
	['click',false],
	['submit',false]
];

for (var c=0,len=Appcelerator.Compiler.GenericFunctions.length;c<len;c++)
{
	var gf = Appcelerator.Compiler.GenericFunctions[c];
	var f = new GenericActionFunction(gf[1]);
	Appcelerator.Compiler.registerCustomAction(gf[0],f);
}

Appcelerator.Compiler.buildCustomAction = function (name)
{
	var action = Appcelerator.Compiler.customActions[name];

	if (!action)
	{
		var f = new GenericActionFunction(false, true);
		Appcelerator.Compiler.registerCustomAction(name,f);
	}
};

Appcelerator.Compiler.buildCustomElementAction = function (name, element, callback)
{
    var f = new GenericActionFunction(false, true);
    Appcelerator.Compiler.registerCustomAction(name, f, element);
    Appcelerator.Compiler.attachFunction(element.id, name, callback);
};
Appcelerator.Compiler.registerCustomAction('history',
{
	execute: function(id,action,params)
	{
		if (params && params.length > 0)
		{
			var obj = params[0];
			var key = obj.key;
			Appcelerator.History.go(key);
		}
		else
		{
			throw "required parameter for history action";
		}	
	}
});
Object.extend(Array.prototype,
{
	remove: function(obj) 
	{
		var a = [];
  		for (var i=0; i<this.length; i++)
		{
    		if (this[i] != obj) 
			{
      			a.push(this[i]);
    		}
  		}
		this.clear();
		for (var i=0; i<a.length; i++)
		{
			this.push(a[i]);
		}
  		return a;
	}
});Appcelerator.Compiler.registerCustomCondition(
{
	conditionNames: ['dragstart', 'drag', 'dragend', 'dragover', 'drop'],
	description: "Respond to drag-and-drop events on the element"
},
function(element,condition,action,elseAction,delay,ifCond)
{
	var eventName = null;
	
	switch (condition)
	{
		case 'dragstart':
		{
			eventName = 'onStart';
			break;
		}
		case 'drag':
		{
			eventName = 'onDrag';
			break;
		}
		case 'dragend':
		{
			eventName = 'onEnd';
			break;
		}
		case 'dragover':
		{
			if (!element.getAttribute('droppable'))
			{
				throw "dragover condition only applies to elements that have droppable attribute";
			}
			
			if (!element.hoverListeners)
			{
				element.hoverListeners = [];
			}
			
			var actionFunc = Appcelerator.Compiler.makeConditionalAction(element.id,action,ifCond);
			element.hoverListeners.push(
			{
				onHover: function(e)
				{
					Appcelerator.Compiler.executeAfter(actionFunc,delay,{id:element.id,dragged:e.id});
				}
			});
			return true;
		}
		case 'drop':
		{
			if (!element.getAttribute('droppable'))
			{
				throw "drop condition only applies to elements that have droppable attribute";
			}
			
			if (!element.dropListeners)
			{
				element.dropListeners = [];
			}
			
			var actionFunc = Appcelerator.Compiler.makeConditionalAction(element.id,action,ifCond);
			element.dropListeners.push(
			{
				onDrop: function(e)
				{
					Appcelerator.Compiler.executeAfter(actionFunc,delay,{id:element.id,dropped:e.id});
				}
			});
			return true;
		}
	}
	
	if (eventName)
	{
		var id = element.id;
		var actionFunc = Appcelerator.Compiler.makeConditionalAction(element.id,action,ifCond);
		var observer = {};
		observer[eventName] = function(type, draggable)
		{
			if (draggable.element.id == id)
			{
				Appcelerator.Compiler.executeAfter(actionFunc,delay,{id:id});
			}
		;}
		Draggables.addObserver(observer);
		
		Appcelerator.Compiler.addTrash(element, function()
		{
			Draggables.removeObserver(element);
		});
		return true;
	}
	
	return false;
});
Appcelerator.Compiler.registerCustomCondition(
{
	conditionNames: ['enter','enter!'],
	description: "Respond to 'enter' keypress on a form"
},
function(element,condition,action,elseAction,delay,ifCond)
{
	switch(condition)
	{
		case 'enter':
		case 'enter!':
		{
			if (Appcelerator.Compiler.getTagname(element)!='form')
			{
				throw "invalid condition: "+condition+" on element of type: "+element.tagName+", only supported on form elements";
			}
			var id = element.id;
			var actionFunc = Appcelerator.Compiler.makeConditionalAction(id,action,ifCond,{id:id});
			var action = function(e)
			{
				Appcelerator.Compiler.executeAfter(actionFunc,delay,{id:id});
				if (condition=='enter!')
				{
					Event.stop(Event.getEvent(e));
				}
			};
			
			var inputs = Form.Methods.getInputs(element);
			for (var c=0;c<inputs.length;c++)
			{
				var input = inputs[c];
				Appcelerator.Compiler.getAndEnsureId(input);
				Appcelerator.Util.KeyManager.installHandler(Appcelerator.Util.KeyManager.KEY_ENTER,input,action);
				Appcelerator.Compiler.addTrash(input,function()
				{
					Appcelerator.Util.KeyManager.removeHandler(Appcelerator.Util.KeyManager.KEY_ENTER,input);
				});
			}
			return true;
		}
	}
	return false;
});Appcelerator.Compiler.Events = 
[
	'click',
	'focus',
	'blur',
	'dblclick',
	'mousedown',
	'mouseout',
	'mouseover',
	'mousemove',
	'change',
	'scroll',
	'contextmenu',
	'mousewheel',
	'input',
	'paste',
	'orientationchange'
];

Appcelerator.Compiler.EventTargets =
[
    'this',
	'parent',
    'down',
    'after',
    'next',
    'sibling',
    'next-sibling',
    'before',
    'up',
    'prev-sibling',
    'previous-sibling',
    'child',
    'children',
    'window',
    'body'
];

Appcelerator.Compiler.actionId = 0;

Appcelerator.Compiler.isEventSelector = function (token)
{
    if (token.indexOf(':')!=-1)
    {
        return false;
    }
    
	if (token.charAt(token.length-1)=='!')
	{
		token = token.substring(0,token.length-1);
	}
	for (var c=0,len=Appcelerator.Compiler.Events.length;c<len;c++)
	{
		if (token==Appcelerator.Compiler.Events[c])
		{
			return true;
		}
		
		var i = token.indexOf('.');
		realtoken = token.substring(i+1);
		realtoken = Appcelerator.Compiler.parseEventConditionName(realtoken);
		if (realtoken==Appcelerator.Compiler.Events[c])
		{
			return true;
		}
	}
	return false;
};
Appcelerator.Compiler.filteredEvent = function(event, filters) 
{
	if (filters!=null) {
		for(var i=0;i<filters.length;i++) 
		{
			filter = filters[i];
			var expected = filter.value;
			var actual = "" + event[filter.name];
			if (actual != expected)
				return true;
		}
	}
	return false;
};
Appcelerator.Compiler.parseEventConditionName = function(condition) 
{
	if (condition.indexOf('[')==-1)
		return condition;
	else
		return condition.substring(0,condition.indexOf('['));
};
Appcelerator.Compiler.parseEventConditionFilters = function(condition) 
{
	if (condition.indexOf('[')!=-1) 
	{
		filters = new Array();
		var start = (condition.indexOf('[')+1);
		var end = (condition.indexOf(']'));
		var namevalue = condition.substring(start,end);
		var pairs = (namevalue.split(','));
		for (var i=0;i<pairs.length;i++)
		{
			var pair = pairs[i].split('=');
			filters.push({name:pair[0],value:pair[1]});
		}
		return filters;
	}
	else
	{
		return null;
	}
};
Appcelerator.Compiler.registerCustomCondition(
{
	conditionPrefixes: Appcelerator.Compiler.EventTargets.map(function(targetName){ return targetName+'.'}),
	conditionSuffixes: Appcelerator.Compiler.Events,
	prefixOptional: true,
	description: "Respond to standard DOM event"
},
function(element,condition,action,elseAction,delay,ifCond)
{
	if (!Appcelerator.Compiler.isEventSelector(condition))
	{
		return false;
	}

	if (elseAction)
	{
		throw "syntax error: 'else' action not supported on event conditions for: "+condition;
	}

	var stopEvent = false;
	if (condition.charAt(condition.length-1)=='!')
	{
		stopEvent = true;
		condition = condition.substring(0,condition.length-1);
	}

	var filters = Appcelerator.Compiler.parseEventConditionFilters(condition);
	condition = Appcelerator.Compiler.parseEventConditionName(condition);

	var i = condition.indexOf('.');
	var event = condition, target = element.id, children = false;
	if (i > 0)
	{
		var tid = condition.substring(0,i);
		switch (tid)
		{
			case 'this':
			{
				target = element.id;
				break;
			}
			case 'parent':
			{
				target = element.parentNode.id;
				break;
			}
			case 'down':
			case 'after':
			case 'next':
			case 'sibling':
			case 'next-sibling':
			{
				target = element.nextSibling.id;
				break;
			}
			case 'before':
			case 'up':
			case 'prev-sibling':
			case 'previous-sibling':
			{
				target = element.previousSibling.id;
				break;
			}
			case 'child':
			case 'children':
			{
				children = true;
				break;
			}
			case 'window':
			{
				target = 'window';
				break;
			}
			case 'body':
			{
				target = document.body.id;
				break;
			}
			default:
			{
				target = tid;
			}
		}
		event = condition.substring(i+1);
	}

	// target the body with the iPhone's orientationchange event
	if (event == 'orientationchange') target = document.body.id;

	//
	// be smart condition. if we attach change event to select - then include in the result
	// the value and text properties of the option selected
	// 
	if (Appcelerator.Compiler.getTagname(element) == 'select' && event == 'change' && (children || target==element.id))
	{
		var f = function(e)
		{
			if (Appcelerator.Compiler.filteredEvent(e,filters))
				return;
			var child = element.options[element.selectedIndex||0];
			var me = child;
			if (Element.isDisabled(me) || Element.isDisabled(me.parentNode)) return;
			var actionFunc = Appcelerator.Compiler.makeConditionalAction(element.id,action,ifCond,{value:child.value,text:child.text});
			actionFunc();
			if (stopEvent)
			{
				Event.stop(e);
				return false;
			}
		};
		Appcelerator.Compiler.addEventListener(element,event,f,delay);		
	}
    else if(event == 'change')
    {
        var actionFunc = Appcelerator.Compiler.makeConditionalAction(element.id,action,ifCond);
        
        element._validatorObserver = new Form.Element.Observer(
            element,
            .5,  
            function(element, value)
            {
                actionFunc(element,value);
            }
        );
    }
	else
	{
		if (children)
		{
			for (var c=0;c<element.childNodes.length;c++)
			{
				(function(){
					var child = element.childNodes[c];
					if (child.nodeType == 1)
					{
						Appcelerator.Compiler.getAndEnsureId(child);
						$D('adding listener to '+child.id+' for event: '+event);
						var cf = function(event)
						{
							if (Appcelerator.Compiler.filteredEvent(event,filters))
								return;
							var me = child;
							if (Element.isDisabled(me) || Element.isDisabled(me.parentNode)) return;
							var actionFunc = Appcelerator.Compiler.makeConditionalAction(Appcelerator.Compiler.getAndEnsureId(child),action,ifCond);
						    var __method = actionFunc, args = $A(arguments);
						    return __method.apply(this, [event || window.event].concat(args));
						};
						var f = cf;
						if (stopEvent)
						{
							f = function(e)
							{
								if (Appcelerator.Compiler.filteredEvent(e,filters))
									return;
								cf(e);
								Event.stop(e);
								return false;
							};
						}
						Appcelerator.Compiler.addEventListener(child,event,f,delay);					
					}
				})();
			}
		}
		else
		{
			if (Object.isString(target))
			{
				target = target == 'window' ? window : $(target);
			}

			if (!target)
			{
				throw "syntax error: target not found for "+condition;
			}

			$D('adding listener to '+target+', name: '+target.nodeName+', id: '+target.id+' for event: '+event);
			var actionFunc = Appcelerator.Compiler.makeConditionalAction(target.id,action,ifCond);

			var scope = {id:target.id};
			var cf = function(event)
			{
				if (Appcelerator.Compiler.filteredEvent(event,filters))
					return;
				var me = $(scope.id);
				if (Element.isDisabled(me) || Element.isDisabled(me.parentNode)) return;
			    var __method = actionFunc, args = $A(arguments);
			    return __method.apply(scope, [event || window.event].concat(args));
			};
			var f = cf;
			if (stopEvent)
			{
				f = function(e)
				{
					if (Appcelerator.Compiler.filteredEvent(e,filters))
						return;
					cf(e);
					Event.stop(e);
					return false;
				};
			}

			Appcelerator.Compiler.addEventListener(target,event,f,delay);
		}
	}
	return true;
});

//handle the iPhone's originationchange event by dispatching our own custom event called orientationchange
if (Appcelerator.Browser.isIPhone)
{
	window.onorientationchange = function() 
	{
		if (!Element.hasClassName('orientation_'+window.orientation))
		{
			var cn = Element.classNames(document.body);
			if (cn)
			{
				cn._each(function(name)
				{
					if (name.startsWith('orientation_'))
					{
						cn.remove(name);
					}
				});
			}
			Element.addClassName(document.body,'orientation_'+window.orientation);
		}
		var evt = document.createEvent("Events");
		evt.initEvent('orientationchange', true, true); //true for can bubble, true for cancelable
		document.body.dispatchEvent(evt);
	}
}//
// this is a custom condition for handling executing actions based on a message condition
//
Appcelerator.Compiler.registerCustomCondition(
{
	conditionPrefixes: ['history:==', 'history:!='],
	suffixOptional: true,
	description: "Respond to back/forward navigation"
},
function(element,condition,action,elseAction,delay,ifCond)
{
    if (!condition.startsWith('history:') && !condition.startsWith('history['))
	{
		return false;
	}

    var token = null;

    if (condition.startsWith('history:'))
    {
        token = condition.substring(8);
    }
    
    if (condition.startsWith('history['))
    {
        token = condition.substring(8,condition.indexOf(']'));
    }

	// allow token to be empty string which is essentially wildcard
    token = token || '';

	var id = element.id;
	var actionFunc = Appcelerator.Compiler.makeConditionalAction(id,action,ifCond);
	var elseActionFunc = elseAction ? Appcelerator.Compiler.makeConditionalAction(id,elseAction,null) : null;
	var operator = '==';
	var regex = null;

	if (token.charAt(0)=='!')
	{
		token = token.substring(1);
		operator = '!=';
	}
	else if (token == '*')
	{
	    operator = '*';
	}
	else if (token.charAt(0)=='~')
	{
		operator = 'regex';
		token = token.substring(1);
	}
	else if (token.indexOf('*') > -1)
	{
		operator = 'regex';
		token = token.gsub(/\*/,'(.*)?');
	}
	
	if (operator == 'regex')
	{
		regex = new RegExp(token);
	}
	
	// support a null (no history) history
	token = token.length == 0 || token=='_none_' || token==='null' ? null : token;
	
	Appcelerator.History.onChange(function(newLocation,data,scope)
	{
		var fire = false;
		switch (operator)
		{
			case 'regex':
			{
				fire = regex.test(newLocation);
				break;
			}
			case '==':
			{
				fire = (newLocation == token);
				break;
			}
			case '!=':
			{
				fire = (newLocation != token);
				break;
			}
			case '*':
			{
				fire = !Object.isUndefined(newLocation);
				break;
			}
		}
		if (fire)
		{
			Appcelerator.Compiler.executeAfter(actionFunc,delay,{data:data});
		}
		else if (elseActionFunc)
		{
			Appcelerator.Compiler.executeAfter(elseActionFunc,delay,{data:data});
		}
	});
	return true;
});



Appcelerator.Compiler.registerCustomCondition(
{
	conditionNames: ['keypress', 'keyup', 'keydown'],
	description: "Respond to key events on the element"
},
function(element,condition,action,elseAction,delay,ifCond)
{
    if (condition.indexOf('keypress')==-1 && condition.indexOf('keyup')==-1 && condition.indexOf('keydown')==-1)
    {
        return false;
    }
    
    if (elseAction)
	{
		throw "syntax error: 'else' action not supported on event conditions for: "+condition;
	}
	
    var token = condition;
	var stopEvent = false;

	if (condition.charAt(token.length-1)=='!')
	{
		condition = condition.substring(0,condition.length-1);
		stopEvent = true;
	}
	
	var i = condition.indexOf('.');
	var event = condition, target = element.id, children = false;
	if (i > 0)
	{
		var tid = condition.substring(0,i);
		switch (tid)
		{
			case 'this':
			{
				target = element.id;
				break;
			}
			case 'parent':
			{
				target = element.parentNode.id;
				break;
			}
			case 'down':
			case 'after':
			case 'next':
			case 'sibling':
			case 'next-sibling':
			{
				target = element.nextSibling.id;
				break;
			}
			case 'before':
			case 'up':
			case 'prev-sibling':
			case 'previous-sibling':
			{
				target = element.previousSibling.id;
				break;
			}
			case 'child':
			case 'children':
			{
				children = true;
				break;
			}
			case 'window':
			{
				target = 'window';
				break;
			}
			case 'body':
			{
				target = document.body.id;
				break;
			}
			default:
			{
				target = tid;
			}
		}
		event = condition.substring(i+1);
	}
	
	var i = condition.indexOf('[');
	var attribute = null;
	if (i > 0)
	{
	    attribute = condition.substring(i+1,condition.indexOf(']'));
	    event = event.substring(0,i);
	}

	if (event == 'keypress' || event == 'keydown' || event == 'keyup')
	{
        if (Object.isString(target))
        {
            target = target == 'window' ? window : $(target);
        }
        
        if (!target)
        {
            throw "syntax error: target not found for "+condition;
        }
        
        $D('adding listener to '+target+', name: '+target.nodeName+', id: '+target.id+' for event: '+event);
        var actionFunc = Appcelerator.Compiler.makeConditionalAction(target.id,action,ifCond);

        var scope = {id:target.id};
        var cf = function(e)
        {
            var me = $(scope.id);
            if (Element.isDisabled(me) || Element.isDisabled(me.parentNode)) return;
            var key = e.keyCode;
            var data = {key: key, event: e};

            if (attribute)
            {
                var mods = attribute.split('+');
                var code = mods[mods.length-1];
                
                switch (code)
                {
                    case 'enter':
                    {
                        if (key != Event.KEY_RETURN) return;
                        break;
                    }
                    case 'esc':
                    {
                        if (key != Event.KEY_ESC) return;
                        break;
                    }
                    case 'left':
                    {
                        if (key != Event.KEY_LEFT) return;
                        break;
                    }
                    case 'right':
                    {
                        if (key != Event.KEY_RIGHT) return;
                        break;
                    }
                    case 'up':
                    {
                        if (key != Event.KEY_UP) return;
                        break;
                    }
                    case 'down':
                    {
                        if (key != Event.KEY_DOWN) return;
                        break;
                    }
                    case 'tab':
                    {
                        if (key != Event.KEY_TAB) return;
                        break;
                    }
                    case 'delete':
                    {
                        if (key != Event.KEY_DELETE) return;
                        break;
                    }
                    case 'backspace':
                    {
                        if (key != Event.KEY_BACKSPACE) return;
                        break;
                    }
                    default:
                    {
                        if (key != code) return;
                        break;
                    }
                }
                
                if (mods.length > 1)
                {
                    for (var i=0; i<(mods.length-1); i++)
                    {
                        var mod = mods[i];
                        switch (mod)
                        {
                            case 'ctrl':
                            {
                                if (!e.ctrlKey) return;
                                break;
                            }
                            case 'alt':
                            {
                                if (!e.altKey) return;
                                break;
                            }
                            case 'shift':
                            {
                                if (!e.shiftKey) return;
                                break;
                            }
                            case 'meta':
                            {
                                if (!e.metaKey) return;
                                break;
                            }
                        }
                    }
                }
            }
            
			Appcelerator.Compiler.executeAfter(actionFunc,0,data);
        };
        var f = cf;
        if (stopEvent)
        {
            f = function(e)
            {
                cf(e);
                Event.stop(e);
                return false;
            };
        }

        Appcelerator.Compiler.addEventListener(target,event,f,delay);
        
        return true;
	}
	return false;
});

//
// this is a custom condition for handling executing actions based on a message condition
//
Appcelerator.Compiler.registerCustomCondition(
{
	conditionPrefixes: ['local:', 'remote:', 'l:', 'r:', 'both:', '*:'],
	freeformSuffix: true,
	description: "Respond to a local or remote message"
},
function(element,condition,action,elseAction,delay,ifCond)
{
	if (condition.startsWith('local:') ||
	       condition.startsWith('l:') || 
	       condition.startsWith('remote:') ||
	       condition.startsWith('r:') ||
		   condition.startsWith('both:') ||
		   condition.startsWith('*:') )
	{
		var id = element.id;
		var actionParams = Appcelerator.Compiler.parameterRE.exec(condition);
		var type = (actionParams ? actionParams[1] : condition);
		var params = actionParams ? actionParams[2] : null;
		var actionFunc = Appcelerator.Compiler.makeConditionalAction(id,action,ifCond);
		var elseActionFunc = (elseAction ? Appcelerator.Compiler.makeConditionalAction(id,elseAction,null) : null);
		return Appcelerator.Compiler.MessageAction.makeMBListener(element,type,actionFunc,params,delay,elseActionFunc);
	}
	return null;
});

Appcelerator.Compiler.MessageAction = {};

Appcelerator.Compiler.MessageAction.onMessage = function(type,data,datatype,direction,scope,actionParamsStr,action,delay,elseaction)
{
	var ok = Appcelerator.Compiler.parseConditionCondition(actionParamsStr, data);

	var obj = {type:type,datatype:datatype,direction:direction,data:data,scope:scope};
	if (ok)
	{
		Appcelerator.Compiler.executeAfter(action,delay,obj);
	}
	else if (elseaction)
	{
		Appcelerator.Compiler.executeAfter(elseaction,delay,obj);
	}
};

Appcelerator.Compiler.MessageAction.makeMBListener = function(element,type,action,params,delay,elseaction)
{
	var actionParams = params ? Appcelerator.Compiler.getParameters(params,false) : null;
	var i = actionParams ? type.indexOf('[') : 0;
	if (i>0)
	{
		type = type.substring(0,i);
	}
	
	var paramsStr = (actionParams) ? Object.toJSON(actionParams) : null;
	$MQL(type,function(type,data,datatype,direction,scope)
	{
		Appcelerator.Compiler.MessageAction.onMessage(type,data,datatype,direction,scope,paramsStr,action,delay,elseaction);
	},element.scope,element);
	
	return true;
};
Appcelerator.Compiler.registerCustomCondition(
{
	conditionNames: ['resize'],
	description:
	("Respond to resizing of an element, "+
	 "requires that the <i>resizable</i> attribute be set"
	)
},
function(element,condition,action,elseAction,delay,ifCond)
{
	if (condition == 'resize')
	{
		if (!element.getAttribute('resizable'))
		{
			throw "resize condition only applies to elements that have resizable=\"true\"";
		}
		
		if (!element.resizeListeners)
		{
			element.resizeListeners = [];
		}
		
		var actionFunc = Appcelerator.Compiler.makeConditionalAction(element.id,action,ifCond);
		element.resizeListeners.push(
		{
			onResize: function(e)
			{
				Appcelerator.Compiler.executeAfter(actionFunc,delay,{id:element.id});
			}
		});
		return true;		
	}
	
	return false;
});
Appcelerator.Compiler.Selected = {};
Appcelerator.Compiler.Selected.makeSelectedListener = function(element,condition,action,elseAction,delay,ifCond)
{
	var id = Appcelerator.Compiler.getAndEnsureId(element);
	Appcelerator.Compiler.attachFunction(element,'selected',function(selected,count,unselect)
	{
		if (selected)
		{
			var actionFunc = Appcelerator.Compiler.makeConditionalAction(id,action,ifCond,{count:count,unselect:unselect});
			Appcelerator.Compiler.executeAfter(actionFunc,delay,{count:count,id:id,unselect:unselect});
		}
		else if (elseAction)
		{
			var elseActionFunc = Appcelerator.Compiler.makeConditionalAction(id,elseAction,null,{count:count,unselect:unselect});
			Appcelerator.Compiler.executeAfter(elseActionFunc,delay,{count:count,id:id,unselect:unselect});
		}
	});
};

Appcelerator.Compiler.registerCustomCondition(
{
	conditionPrefixes: ['parent.', 'child.', 'children.'],
	conditionSuffixes: ['selected'],
	prefixOptional: true,
	description:
	("Respond to selection of this element, "+
	 "requires the <i>selectable</i> attribute be set"
	)
},
function(element,condition,action,elseAction,delay,ifCond)
{
	switch (condition)
	{
		case 'selected':
		{
			Element.addClassName(element,'app_selectable');
			Appcelerator.Compiler.Selected.makeSelectedListener(element,condition,action,elseAction,delay,ifCond);
			break;
		}
		case 'parent.selected':
		{
			Element.addClassName(element,'app_selectable');
			Appcelerator.Compiler.Selected.makeSelectedListener(element.parentNode,condition,action,elseAction,delay,ifCond);
			break;
		}
		case 'children.selected':
		case 'child.selected':
		{
			Element.addClassName(element,'app_selectgroup');
			for (var c=0;c<element.childNodes.length;c++)
			{
				var child = element.childNodes[c];
				if (child.nodeType == 1)
				{
					Element.addClassName(child,'app_selectable');
					Appcelerator.Compiler.Selected.makeSelectedListener(child,condition,action,elseAction,delay,ifCond);
				}
			}
			break;
		}
		default:
		{
			return false;
		}
	}
	return true;
});Appcelerator.Compiler.registerCustomCondition(
{
	conditionNames: ['sortupdate', 'sortchange'],
	description: "Respond to sortable events on the element"
},
function(element,condition,action,elseAction,delay,ifCond)
{
	var eventName = null;
	
	switch (condition)
	{
		case 'sortupdate':
		{
			if (!element.getAttribute('sortable'))
			{
				throw "sortupdate condition only applies to elements that have sortable attribute";
			}
			
			if (!element.updateListeners)
			{
				element.updateListeners = [];
			}
			
			var actionFunc = Appcelerator.Compiler.makeConditionalAction(element.id,action,ifCond);
			element.updateListeners.push(
			{
				onUpdate: function(e)
				{
					Appcelerator.Compiler.executeAfter(actionFunc,delay,{id:e.id});
				}
			});
			return true;
		}
		case 'sortchange':
		{
			if (!element.getAttribute('sortable'))
			{
				throw "sortchange condition only applies to elements that have sortable attribute";
			}
			
			if (!element.changeListeners)
			{
				element.changeListeners = [];
			}
			
			
			var actionFunc = Appcelerator.Compiler.makeConditionalAction(element.id,action,ifCond);
			element.changeListeners.push(
			{
				onChange: function(e)
				{
					Appcelerator.Compiler.executeAfter(actionFunc,delay,{id:e.id});
				}
			});
			return true;
		}
	}
	
	return false;
});
Appcelerator.Compiler.StateMachine = {};
Appcelerator.Compiler.StateMachine.APP_STATES = {};
Appcelerator.Compiler.StateMachine.CompiledStateConditionCache = {};

Appcelerator.Compiler.StateMachine.isStateCondition = function (token)
{
	return Appcelerator.Compiler.parameterRE.test(token);
};
Appcelerator.Compiler.getTransitionSource = function(moniker) {
	if (moniker.indexOf(':')==-1)
		return '';
	else {
		value =  moniker.split(':')[0];
		return ''+value;
	}
};
Appcelerator.Compiler.getTransitionTarget = function(moniker) {
	if (moniker.indexOf(':')==-1)
		return moniker;
	else {
		value =  moniker.split(':')[1];
		return ''+value;
	}
};

Appcelerator.Compiler.registerCustomCondition(
{
	pattern: /([^\[]*)\[([^\]]*)\]/
},
function(element,condition,action,elseAction,delay,ifCond)
{
	if (!Appcelerator.Compiler.StateMachine.isStateCondition(condition))
	{
		return false;
	}
	var id = element.id;
	
	//TODO: add not condition
	
	// statemachine[state] and statemachine[state]
	var tokens = condition.split(' ');
	var condition = '';
	var statemachines = [];
	
	for (var c=0,len=tokens.length;c<len;c++)
	{
		var token = tokens[c].trim();
		if (token.length == 0)
		{
			continue;
		}
		switch(token)
		{
			case 'and':
			{
				condition+=' && ';
				continue;
			}
			case '||':
			case '&&':
			{
				condition+=' '+token+' ';
				continue;
			}
			default:break;
		}
		
		var cond = '';
		
		if (token.charAt(0)=='!')
		{
			cond = '!';
			token = token.substring(1);
		}
		else if (token.startsWith('not '))
		{
			cond = '!';
			token = token.substring(4);
		}
		
		var actionParams = Appcelerator.Compiler.parameterRE.exec(token);
		var statemachine = actionParams[1];
		var state = Appcelerator.Compiler.getTransitionTarget(actionParams[2]);
		var oldstate = Appcelerator.Compiler.getTransitionSource(actionParams[2]);
		statemachines.push([statemachine,state,oldstate]);
		if (state !='' && state)
			condition += cond + 'Appcelerator.Compiler.StateMachine.isStateValid("'+statemachine+'","'+state+'")';
	}
	
	// pre-compile conditions
	var actionFunc = Appcelerator.Compiler.makeConditionalAction(id,action,ifCond);
	var elseActionFunc = (elseAction ? Appcelerator.Compiler.makeConditionalAction(id,elseAction,null) : null);
	var compiledCondition = Appcelerator.Compiler.StateMachine.CompiledStateConditionCache[condition];
	
	// if we didn't find it, cache it for re-use
	if (!compiledCondition)
	{
		compiledCondition = condition.toFunction();
		Appcelerator.Compiler.StateMachine.CompiledStateConditionCache[condition] = compiledCondition;
		$D('compiled state => '+condition);
	}

	for (var c=0,len=statemachines.length;c<len;c++)
	{
		var statemachine = statemachines[c][0];
		var curstate = statemachines[c][1];
		var curoldstate = statemachines[c][2];
		$D('adding state change listener for '+statemachine+'['+curstate+']');
	
		Appcelerator.Compiler.StateMachine.registerStateListener(statemachine,function(statemachinein,statechange,valid, old_state)
		{
			var result=false;
			if (curoldstate!='') {
				var validold = curoldstate == (''+old_state) || curoldstate=='';
				var validnew = curstate == (''+statechange) || curstate=='';
				result = validold && validnew;
			} else {
				result = compiledCondition();
				$D('statemachine: '+statemachinein+'['+statechange+'] logic returned => '+result);
			}
			if (result)
			{
				Appcelerator.Compiler.executeAfter(actionFunc,delay);
			}
			else if (elseActionFunc)
			{
				Appcelerator.Compiler.executeAfter(elseActionFunc,delay);
			}
		});
		Appcelerator.Compiler.StateMachine.addOnLoadStateCheck(statemachine);
	}

	return true;
});

//
// returns true if state machine is found or false if not registered
//
Appcelerator.Compiler.StateMachine.isStateMachine = function (machine)
{
	return Appcelerator.Compiler.StateMachine.APP_STATES[machine] != null;
};

//
// returns true if a state for machine is set
//
Appcelerator.Compiler.StateMachine.isStateValid = function (machine,state)
{
	var m = Appcelerator.Compiler.StateMachine.APP_STATES[machine];
	if (m)
	{
		if (m.active)
		{
			return (state == '*' || m.states['state_'+state]==true);
		}
	}
	return false;
};

//
// disable all states
//
Appcelerator.Compiler.StateMachine.disableStateMachine = function(statemachine)
{
	var state = Appcelerator.Compiler.StateMachine.getActiveState(statemachine,true);
	var m = Appcelerator.Compiler.StateMachine.APP_STATES[statemachine];
	if (m)
	{
		m.active=false;
		m.init=true;
		$D('disable state machine='+statemachine+'['+state+']');
		if (state)
		{
			Appcelerator.Compiler.StateMachine.fireStateMachineChange(statemachine,state,false);
		}
		m.activeState=state;
	}
}

//
// enable all states
//
Appcelerator.Compiler.StateMachine.enableStateMachine = function(statemachine)
{
	var m = Appcelerator.Compiler.StateMachine.APP_STATES[statemachine];
	if (m)
	{
		m.active=true;
		m.init=true;
		var state = m.activeState;
		$D('enable state machine='+statemachine+'['+state+']');
		if (state)
		{
			m.activeState = null;
			Appcelerator.Compiler.StateMachine.fireStateMachineChange(statemachine,state,true,true);
		}
	}
}

//
// add a state to a state machine 
//
Appcelerator.Compiler.StateMachine.addState = function (statemachine,state,on_off)
{
	var m = Appcelerator.Compiler.StateMachine.APP_STATES[statemachine];
	if (!m)
	{
		m = Appcelerator.Compiler.StateMachine.createStateMachine(statemachine);
	}
	m.states['state_'+state]=on_off;
};

//
// create a statemachine by id. warning: this method will
// not check for the existing of statemachine already created
// and will replace it if one already exists
// 
Appcelerator.Compiler.StateMachine.createStateMachine = function(statemachine)
{
	var m = {'states':{},'listeners':[], 'active':true, 'activeState':null, 'init':false};
	Appcelerator.Compiler.StateMachine.APP_STATES[statemachine] = m;
	return m;
};

//
// register a state machine listener for changes
//
Appcelerator.Compiler.StateMachine.registerStateListener = function (statemachine,listener)
{
	var m = Appcelerator.Compiler.StateMachine.APP_STATES[statemachine];
	if (!m)
	{
		m = Appcelerator.Compiler.StateMachine.createStateMachine(statemachine);
	}
	m.listeners.push(listener);
};

//
// unregister a listener
//
Appcelerator.Compiler.StateMachine.unregisterStateListener = function(statemachine,listener)
{
	var m = Appcelerator.Compiler.StateMachine.APP_STATES[statemachine];
	if (m)
	{
		var idx = m.indexOf(listener);
		if (idx >= 0)
		{
			m.removeAt(idx);
			return true;
		}
	}
	return false;
};

//
// given a statemachine, return the activate state name
//
Appcelerator.Compiler.StateMachine.getActiveState = function(statemachine,force)
{
	var m = Appcelerator.Compiler.StateMachine.APP_STATES[statemachine];
	if (m && (m.active || force==true))
	{
		if (m.activeState!=null) 
		{
			return m.activeState;
		}
		for (var s in m.states)
		{
			if (s.startsWith('state_'))
			{
				if (true == m.states[s])
				{
					return s.substring(6);
				}
			}
		}
	}
	return null;
};

//
// set the new state and de-activate other states and then 
// fire events to any listeners
//
Appcelerator.Compiler.StateMachine.fireStateMachineChange = function (statemachine,state,on_off,force,init)
{
	$D('fireStateMachineChange => '+statemachine+'['+state+'] = '+on_off+', force='+(force==true)+',init='+init);
	var m = Appcelerator.Compiler.StateMachine.APP_STATES[statemachine];
	if (m)
	{
		var different = false;
		var old_state = 'state_'+m.activeState;
		var old_state_simple = m.activeState;
				
		for (var s in m.states)
		{
			if (s.startsWith('state_'))
			{
				if (state == s.substring(6))
				{
					var old = m.states[s];
					
					if (init==true || force==true || on_off == null || old==null || (old!=null && old!=on_off))
					{
						m.states[s] = on_off;
						m.activeState = on_off ? s.substring(6) : null;
						different = old_state == s ? false : true;
					}
				}
				else if (on_off!=null && on_off==true)
				{
					// you can only have one state active
					m.states[s]=false;
				}
			}
		}
		
		$D('setting activeState = '+m.activeState+' for '+statemachine);
		
		m.init = true;
		
		// force potentially, check it
		different = force==true ? true : different ;
		
		//
		// now fire state change event to listeners
		// 
		if (different && m.listeners.length > 0)
		{
			for (var c=0,len=m.listeners.length;c<len;c++)
			{
				var listener = m.listeners[c];
				listener.apply(listener,[statemachine,state,on_off, old_state_simple]);
			}
		}
	}
};

Appcelerator.Compiler.StateMachine.addOnLoadStateCheck = function (statemachine)
{
	if (Appcelerator.Compiler.StateMachine.initialStateInit)
	{
		if (Appcelerator.Compiler.StateMachine.initialStateInit.indexOf(statemachine) < 0)
		{
			Appcelerator.Compiler.StateMachine.initialStateInit.push(statemachine);	
		}
	}
};

Appcelerator.Compiler.StateMachine.resetOnStateListeners = function()
{
	if (!Appcelerator.Compiler.StateMachine.initialStateLoaders)
	{
		Appcelerator.Compiler.StateMachine.initialStateLoaders = [];
		Appcelerator.Compiler.StateMachine.initialStateInit = [];
	}
};

Appcelerator.Compiler.StateMachine.fireOnStateListeners = function()
{
	if (Appcelerator.Compiler.StateMachine.initialStateLoaders && Appcelerator.Compiler.StateMachine.initialStateLoaders.length > 0)
	{
		var statesFired = [];
		for(var c=0,len=Appcelerator.Compiler.StateMachine.initialStateLoaders.length;c<len;c++)
		{
			var entry = Appcelerator.Compiler.StateMachine.initialStateLoaders[c];
			$D('firing initial state change = '+entry[0]+'['+entry[1]+']');
			Appcelerator.Compiler.StateMachine.fireStateMachineChange(entry[0],entry[1],true,false,true);
			statesFired.push(entry[0]);
		}
		if (Appcelerator.Compiler.StateMachine.initialStateInit && Appcelerator.Compiler.StateMachine.initialStateInit.length > 0)
		{
			for (var c=0;c<Appcelerator.Compiler.StateMachine.initialStateInit.length;c++)
			{
				var statemachine = Appcelerator.Compiler.StateMachine.initialStateInit[c];
				if (statesFired.indexOf(statemachine) < 0)
				{
					var activeState = Appcelerator.Compiler.StateMachine.getActiveState(statemachine);
					if (activeState)
					{
						Appcelerator.Compiler.StateMachine.fireStateMachineChange(statemachine,activeState,true,true,false);
						statesFired.push(statemachine);
					}
				}
			}
		}
	}
	Appcelerator.Compiler.StateMachine.initialStateLoaders = null;
	Appcelerator.Compiler.StateMachine.initialStateInit = null;
};

Appcelerator.Compiler.StateMachine.re = /[\s\n\r]+or[\s\n\r]+/;
Appcelerator.Compiler.StateMachine.compileStateCondition = function (name,value)
{
	var state = 
	{
		'name':name,
		'states':{},
		'types':[]
	};

	if (value)
	{
		var tokens = value.split(Appcelerator.Compiler.StateMachine.re);

		for (var c=0,len=tokens.length;c<len;c++)
		{
			var token = tokens[c];
			var parts = Appcelerator.Compiler.parameterRE.exec(token);
			var type = Appcelerator.Compiler.convertMessageType(parts ? parts[1] : token);
			var params = parts ? parts[2] : null;
			var parameters = params ? Appcelerator.Compiler.getParameters(params,false) : null;
			var cond = parameters ? Appcelerator.Compiler.StateMachine.compileMessageConditionExpression(parameters) : null;
			var obj = state.states[type];
			if (!obj)
			{
				state.types.push(type);
				obj=[];
				state.states[type]=obj;
			}
			// add the condition (if we have one) or 
			// create an implicit condition that is always true
			obj.push(cond||'1==1');
		}
	}
	return state;
};

Appcelerator.Compiler.StateMachine.compileMessageConditionExpression = function(parameters)
{
	var code = '';
	for (var c=0,len=parameters.length;c<len;c++)
	{
		var parameter = parameters[c];
		var value = null;
		var condition = '==';
		var key = parameter.key;
		if (key.endsWith("!"))
		{
			condition='!=';
			key = key.substring(0,key.length-1);
		}
		switch(typeof(parameter.value))
		{
			case 'string':
			{
				value = '"'+parameter.value+'"';
				break;
			}
			case 'number':
			case 'boolean':
			{
				value = parameter.value;
				break;
			}
		}
		code+= 'Object.getNestedProperty(this.data,"'+key+'")' + condition + value;
		if (c + 1 < len)
		{
			code+=' && ';
		}
	}
	return code;
};

Appcelerator.Compiler.StateMachine.buildConditions = function(contexts)
{
	var byType = {};

	for (var c=0,len=contexts.length;c<len;c++)
	{
		var context = contexts[c];
		for (var x=0,xlen=context.types.length;x<xlen;x++)
		{
			var type = context.types[x];
			var obj = byType[type];
			if (!obj)
			{
				obj = [];
				obj.push('case "'+type+'": { ');
				byType[type]=obj;
			}
			var states = context.states[type];
			obj.push('if ('+states.join(' || ')+') return "' + context.name + '";');
		}
	}

	var code = [];
	var types = [];
	for (var type in byType)
	{
		types.push(type);
		var obj = byType[type];
		code.push(obj.join('') + 'break; } ');
	}

	return {
		types:types.uniq(),
		code:'switch(this.messagetype) { ' + code.join('') + ' default: return null; } '
	};
};

Appcelerator.Compiler.StateMachine.initialStateLoaders = [];
Appcelerator.Compiler.StateMachine.initialStateInit = [];

Appcelerator.Compiler.afterDocumentCompile(Appcelerator.Compiler.StateMachine.fireOnStateListeners);
Object.extend(Appcelerator.Decorator,
{
	invalidImage: Appcelerator.ImagePath + 'warning.png',
	validImage: Appcelerator.ImagePath + 'confirm.png',
	
    toString: function ()
    {
        return '[Appcelerator.Decorator]';
    },

    decoratorId: 0,
    names: [],

	addDecorator: function(name, decorator)
	{
		Appcelerator.Decorator[name] = decorator;
		Appcelerator.Decorator.names.push(name);
	},

    checkInvalid: function (element, valid, decId, msg, showValid)
    {
        if (decId)
        {
            var div = $(decId);
            if (div)
            {
                if (showValid)
                {
                    if (valid)
                    {
                    	Appcelerator.Compiler.setHTML(div,'<img src="' + Appcelerator.Decorator.validImage + '"/> <span class="success_color">' + msg + "</span>");
						Appcelerator.Compiler.dynamicCompile(div);
                    }
                }
                else
                {
                    if (!valid)
                    {
                        Appcelerator.Compiler.setHTML(div,'<img src="' + Appcelerator.Decorator.invalidImage + '" style="margin: 3px 0 -3px 0"/> ' + msg);
						Appcelerator.Compiler.dynamicCompile(div);
                    }
                    Element.setStyle(decId, {visibility:(valid ? 'hidden' : 'visible')});
                }
            }
        }
        else
        {
            var id = 'decorate_' + element.id;
            var div = $(id);

            if (!div)
            {
                // only insert the first time through
                new Insertion.After(element.id, '<span id="' + id + '" style="font-size:11px;color:#ff0000"></span>');
                div = $(id);
            }

            if (showValid)
            {
                Appcelerator.Compiler.setHTML(div,'<img src="' + Appcelerator.Decorator.validImage + '"/> <span class="success_color">' + msg + "</span>");
				Appcelerator.Compiler.dynamicCompile(div);
            }
            else
            {
                if (!valid)
                {
                    // update the message if not valid
                    Appcelerator.Compiler.setHTML(div,'<img src="' + Appcelerator.Decorator.invalidImage + '" style="margin: 3px 0 -3px 0"/> ' + msg);
					Appcelerator.Compiler.dynamicCompile(div);
                }

                // just flip the visibility flag is valid or invalid - helps with shifting box when you remove/add
                Element.setStyle(id, {visibility:(valid ? 'hidden' : 'visible')});
            }
        }
    }	
});

(function(){
	
	var addDecorator = Appcelerator.Decorator.addDecorator;
	
	addDecorator('defaultDecorator', function(element, valid)
	{
		// do nothing
	});
	
	addDecorator('custom', function(element, valid, decId)
	{
		if (!decId)
		{
			throw "invalid custom decorator, decoratorId attribute must be specified";
		}
		var dec = $(decId);
		if (!dec)
		{
			throw "invalid custom decorator, decorator with ID: "+decId+" not found";
		}
		if (!valid)
		{
			if (dec.style.display=='none')
			{
				dec.style.display='';
			}
			if (dec.style.visibility=='hidden')
			{
				dec.style.visibility='';
			}
		}
		else
		{
			if (dec.style.display!='none')
			{
				dec.style.display='none';
			}
			if (dec.style.visibility!='hidden')
			{
				dec.style.visibility='hidden';
			}
		}
	});
	
    addDecorator('termsAccepted', function(element, valid, decId)
    {
        this.checkInvalid(element, valid, decId, 'must accept terms and conditions');
    });

    addDecorator('required', function(element, valid, decId)
    {
        this.checkInvalid(element, valid, decId, 'required');
    });

    addDecorator('zipcode_5', function(element, valid, decId)
    {
        this.checkInvalid(element, valid, decId, '5 digit zipcode required');
    });
    addDecorator('phone_us', function(element, valid, decId)
    {
        this.checkInvalid(element, valid, decId, '10 digit phone number required (###-###-####)');
    });
    addDecorator('ssn', function(element, valid, decId)
    {
        this.checkInvalid(element, valid, decId, '9 digit ssn required (###-##-####)');
    });

    addDecorator('email', function(element, valid, decId)
    {
        this.checkInvalid(element, valid, decId, 'enter a valid email address');
    });
	addDecorator('date', function(element, valid, decId)
	{
       this.checkInvalid(element, valid, decId, 'invalid date');	
	});
	addDecorator('number', function(element, valid, decId)
	{
       this.checkInvalid(element, valid, decId, 'invalid number');			
	});
    addDecorator('fullname', function(element, valid, decId)
    {
        this.checkInvalid(element, valid, decId, 'enter first and last name');
    });

	addDecorator('alphanumeric', function(element,valid,decId)
	{
        this.checkInvalid(element, valid, decId, 'enter an alphanumeric value');
	});

    addDecorator('noSpaces', function(element, valid, decId)
    {
        this.checkInvalid(element, valid, decId, 'value must contain no spaces');
    });

    addDecorator('password', function(element, valid, decId)
    {
        this.checkInvalid(element, valid, decId, 'password must be at least 6 characters');
    });

    addDecorator('url', function (element, valid, decId)
    {
        this.checkInvalid(element, valid, decId, 'enter a valid URL');
    });

   	addDecorator('checked', function (element, valid, decId)
    {
        this.checkInvalid(element, valid, decId, 'item must be checked');
    });

  	addDecorator('wholenumber', function (element, valid, decId)
    {
        this.checkInvalid(element, valid, decId, 'enter a whole number');
    });

    addDecorator('length', function (element, valid, decId)
    {
        if (!valid)
        {
            var min = element.getAttribute('validatorMinLength') || '0';
            var max = element.getAttribute('validatorMaxLength') || '999999';
            this.checkInvalid(element, valid, decId, 'value must be between ' + min + '-' + max + ' characters');
        }
        else
        {
            this.checkInvalid(element, valid, decId, element.value.length + ' characters', true);
        }
    });
})();

$MQL('appcelerator.download',function(type,msg,datatype,from)
{
	var name = msg['name'];
	var ticket = msg['ticket'];
	var d = Appcelerator.ServerConfig['download'];
	var url = d ? d.path : Appcelerator.DocumentPath + '/download';
	var id = Appcelerator.Util.UUID.generateNewId();
	url = url+'?ticket='+encodeURIComponent(ticket)+'&name='+encodeURIComponent(name)+'&reqid='+id;
	var html = '<iframe id="'+id+'" src="' + url + '" height="1" width="1" style="position:absolute;left:-400px;top:-400px;height:1px;width:1px;"></iframe>';
	new Insertion.Bottom(document.body,html);	
});
/*
Based on Easing Equations v2.0
(c) 2003 Robert Penner, all rights reserved.
This work is subject to the terms in http://www.robertpenner.com/easing_terms_of_use.html

Adapted for Scriptaculous by Ken Snyder (kendsnyder ~at~ gmail ~dot~ com) June 2006
*/
Object.extend(Effect.Transitions, 
{
  elastic: function(pos) {
    return -1 * Math.pow(4,-8*pos) * Math.sin((pos*6-1)*(2*Math.PI)/2) + 1;
  },
  
  swingFromTo: function(pos) {
    var s = 1.70158;
    return ((pos/=0.5) < 1) ? 0.5*(pos*pos*(((s*=(1.525))+1)*pos - s)) :
      0.5*((pos-=2)*pos*(((s*=(1.525))+1)*pos + s) + 2);
  },
  
  swingFrom: function(pos) {
    var s = 1.70158;
    return pos*pos*((s+1)*pos - s);
  },
  
  swingTo: function(pos) {
    var s = 1.70158;
    return (pos-=1)*pos*((s+1)*pos + s) + 1;
  },
  
  bounce: function(pos) {
    if (pos < (1/2.75)) {
        return (7.5625*pos*pos);
    } else if (pos < (2/2.75)) {
        return (7.5625*(pos-=(1.5/2.75))*pos + .75);
    } else if (pos < (2.5/2.75)) {
        return (7.5625*(pos-=(2.25/2.75))*pos + .9375);
    } else {
        return (7.5625*(pos-=(2.625/2.75))*pos + .984375);
    }
  },
  
  bouncePast: function(pos) {
    if (pos < (1/2.75)) {
        return (7.5625*pos*pos);
    } else if (pos < (2/2.75)) {
        return 2 - (7.5625*(pos-=(1.5/2.75))*pos + .75);
    } else if (pos < (2.5/2.75)) {
        return 2 - (7.5625*(pos-=(2.25/2.75))*pos + .9375);
    } else {
        return 2 - (7.5625*(pos-=(2.625/2.75))*pos + .984375);
    }
  },
  
  easeFromTo: function(pos) {
    if ((pos/=0.5) < 1) return 0.5*Math.pow(pos,4);
    return -0.5 * ((pos-=2)*Math.pow(pos,3) - 2);   
  },
  
  easeFrom: function(pos) {
    return Math.pow(pos,4);
  },
  
  easeTo: function(pos) {
    return Math.pow(pos,0.25);
  }
});

for (var p in Effect.Transitions)
{
	var v = Effect.Transitions[p];
	if (Object.isFunction(v))
	{
		Effect.Transitions[String(p.charAt(0)).toUpperCase()+p.substring(1)] = v;
	}
}


// taken from http://www.huddletogether.com/projects/lightbox2
// getPageSize()
// Returns array with page width, height and window width, height
// Core code from - quirksmode.org
// Edit for Firefox by pHaez
//
Element.getDocumentSize = function ()
{
   var xScroll, yScroll;
   var pageHeight, pageWidth;

    if (window.innerHeight && window.scrollMaxY) {
		xScroll = document.body.scrollWidth;
		yScroll = window.innerHeight + window.scrollMaxY;
	} else if (document.body.scrollHeight > document.body.offsetHeight){ // all but Explorer Mac
		xScroll = document.body.scrollWidth;
		yScroll = document.body.scrollHeight;
	} else { // Explorer Mac...would also work in Explorer 6 Strict, Mozilla and Safari
		xScroll = document.body.offsetWidth;
		yScroll = document.body.offsetHeight;
	}
	
	var windowWidth, windowHeight;
	if (self.innerHeight) {	// all except Explorer
		windowWidth = self.innerWidth;
		windowHeight = self.innerHeight;
	} else if (document.documentElement && document.documentElement.clientHeight) { // Explorer 6 Strict Mode
		windowWidth = document.documentElement.clientWidth;
		windowHeight = document.documentElement.clientHeight;
	} else if (document.body) { // other Explorers
		windowWidth = document.body.clientWidth;
		windowHeight = document.body.clientHeight;
	}	
	
	// for small pages with total height less then height of the viewport
	if(yScroll < windowHeight){
		pageHeight = windowHeight;
	} else { 
		pageHeight = yScroll;
	}

	// for small pages with total width less then width of the viewport
	if(xScroll < windowWidth){	
		pageWidth = windowWidth;
	} else {
		pageWidth = xScroll;
	}

	return [pageWidth,pageHeight,windowWidth,windowHeight];
};

//
// getPageScroll()
// Returns array with x,y page scroll values.
// Core code from - quirksmode.org
//
Element.getPageScroll = function ()
{
	var yScroll;

	if (self.pageYOffset) {
		yScroll = self.pageYOffset;
	} else if (document.documentElement && document.documentElement.scrollTop){	 // Explorer 6 Strict
		yScroll = document.documentElement.scrollTop;
	} else if (document.body) {// all other Explorers
		yScroll = document.body.scrollTop;
	}

	return yScroll;
}

Element.getCenteredTopPosition = function (relative,name)
{
      var dim = Element.getDimensions(relative);
      var pos = Element.getDimensions(name).width;
      return (dim.height - pos) / 2;
};
   
Element.center = function(relative, name, top)
{
      var element = $(name);
      var dim = Element.getDimensions(relative);
      var width = dim.width;
      var height = dim.height;
      var pos = Element.getDimensions(name).width;
      
      var left = (width - pos) / 2;
      element.style.position = "absolute";
      element.style.left = left + "px";
      element.style.right = left + width +"px";
      
      if (top)
      {
         element.style.top = top +"px";
      }
      else
      {
         var hw = element.offsetHeight || parseInt(element.style.height) || 200;
         element.style.top = (height - hw ) / 2 + "px";
      }
};

Element.findParentForm = function (element)
{
	var found = null;
	
	Element.ancestors(element).each(function(n)
	{
		if (n.nodeName == 'FORM')
		{
			found = n;
			throw $break;
		}
	});
	
	return found;
};

/**
 * determine if our element is showing, regardless of its display property,
 * by walking up the ancestory to determine if *any* of the parents are display:none
 */
Element.showing = function (element, ancestors)
{
	if (!Element.visible(element)) return false;
	
     ancestors = ancestors || element.ancestors();
     
     var visible = true;
     
     ancestors.each(function(ancestor)
     {
     	if (!Element.visible(ancestor))
     	{
     		visible = false;
     		throw $break;
     	}
     });
     
     return visible;
};

/**
 * return the non-showing ancestor or null if all are displaying
 */
Element.findNonShowingAncestor = function (element, ancestors)
{
	if (!Element.visible(element)) return element;
	
     ancestors = ancestors || element.ancestors();
     
     var found = null;
     
     ancestors.each(function(ancestor)
     {
     	if (!Element.visible(ancestor))
     	{
     		found = ancestor;
     		throw $break;
     	}
     });
     
     return found;
};

Element.setHeight = function(element,h) 
{
   	element = $(element);
    element.style.height = h +"px";
};

Element.setWidth = function (element, w)
{
	element = $(element);
	element.style.width = w +"px";
};

Element.setTop = function(element,t) 
{
	element = $(element);
    element.style.top = t +"px";
};

Element.isVisible = function(el)
{
	return el.style.visibility != 'hidden' && el.style.display != 'none';
};

Element.ensureVisible=function(element)
{
	element = $(element);
	Position.prepare();
	var pos = Position.cumulativeOffset(element);

	var docsize = Element.getDocumentSize();
	var top = Element.getPageScroll();
	var bottom = top+docsize[3];

	var belowTop = pos[1]>=top+25;
	var aboveBottom = pos[1]<=bottom-25;

	var within = (belowTop&&aboveBottom);

	if (!within)
	{
	   var y=Math.max(0,pos[1]-30);
	   window.scrollTo(0,y);
	}
};


Element.isDisabled = function(element)
{
	element = $(element);
	return (element && (element.disabled || element.getAttribute('disabled') == 'true'));
};




Object.extend(Event, {
    getEvent: function (e)
    {
    	e = e || window.event;
    	return e ? typeof(e.inspect)=='function' ? e : Event.extend(e) : null;
    },
    isEventObject: function (e)
	{
	   e = this.getEvent(e);
	   if (e)
	   {
		   // IE
		   if (typeof(e.cancelBubble)!='undefined' && typeof(e.qualifier)!='undefined')
		   {
		      return true;
		   }
		   // the others (FF, safari, opera, etc)
		   if (typeof(e.stopPropagation)!='undefined' && typeof(e.preventDefault)!='undefined')
		   {
		      return true;
		   }
	   }
	   return false;
	},    
    getKeyCode: function (event)
    {
        var pK;
        if (event)
        {
            if (typeof(event.keyCode) != "undefined")
            {
                pK = event.keyCode;
            }
            else if (typeof(event.which) != "undefined")
            {
                pK = event.which;
            }
        }
        else
        {
            pK = window.event.keyCode;
        }
        return pK;
    },
    isKey: function (event, code)
    {
        return Event.getKeyCode(event) == code;
    },
    isEscapeKey: function(event)
    {
        return Event.isKey(event, Event.KEY_ESC) || Event.isKey(event, event.DOM_VK_ESCAPE);
    },
    isEnterKey: function(event)
    {
        return Event.isKey(event, Event.KEY_RETURN) || Event.isKey(event, event.DOM_VK_ENTER);
    },
    isSpaceBarKey: function (event)
    {
        return Event.isKey(event, Event.KEY_SPACEBAR) || Event.isKey(event, event.DOM_VK_SPACE);
    },
    isPageUpKey: function (event)
    {
        return Event.isKey(event, Event.KEY_PAGEUP) || Event.isKey(event, event.DOM_VK_PAGE_UP);
    },
    isPageDownKey: function (event)
    {
        return Event.isKey(event, Event.KEY_PAGEDOWN) || Event.isKey(event, event.DOM_VK_PAGE_DOWN);
    },
    isHomeKey: function (event)
    {
        return Event.isKey(event, Event.KEY_HOME) || Event.isKey(event, event.DOM_VK_HOME);
    },
    isEndKey: function (event)
    {
        return Event.isKey(event, Event.KEY_END) || Event.isKey(event, event.DOM_VK_END);
    },
    isDeleteKey: function (event)
    {
        return Event.isKey(event, Event.KEY_DELETE) || Event.isKey(event, event.DOM_VK_DELETE);
    },
    isLeftKey: function (event)
    {
        return Event.isKey(event, Event.KEY_LEFT) || Event.isKey(event, event.DOM_VK_LEFT);
    },
    isRightKey: function (event)
    {
        return Event.isKey(event, Event.KEY_RIGHT) || Event.isKey(event, event.DOM_VK_RIGHT);
    },

    KEY_SPACEBAR: 0,
    KEY_PAGEUP: 33,
    KEY_PAGEDOWN: 34,
    KEY_END: 35,
    KEY_HOME: 36,
    KEY_DELETE: 46
});


// Copyright 2007, Google Inc.
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
//
// Sets up google.gears.*, which is *the only* supported way to access Gears.
//
// Circumvent this file at your own risk!
//
// In the future, Gears may automatically define google.gears.* without this
// file. Gears may use these objects to transparently fix bugs and compatibility
// issues. Applications that use the code below will continue to work seamlessly
// when that happens.

(function() {
	
  //
  // APPCELERATOR CODE TO CHECK TO SEE IF WE'RE CONFIGURED BEFORE DOING GOOGLE INIT
  // 	
  if (!Appcelerator.Config['usegears'])
  {
	return;
  }	
	
  // We are already defined. Hooray!
  if (window.google && google.gears) {
    return;
  }

  var factory = null;

  // Firefox
  if (typeof GearsFactory != 'undefined') {
    factory = new GearsFactory();
  } else {
    // IE
    try {
      factory = new ActiveXObject('Gears.Factory');
      // privateSetGlobalObject is only required and supported on WinCE.
      if (factory.getBuildInfo().indexOf('ie_mobile') != -1) {
        factory.privateSetGlobalObject(this);
      }
    } catch (e) {
      // Safari
      if ((typeof navigator.mimeTypes != 'undefined')
           && navigator.mimeTypes["application/x-googlegears"]) {
        factory = document.createElement("object");
        factory.style.display = "none";
        factory.width = 0;
        factory.height = 0;
        factory.type = "application/x-googlegears";
        document.documentElement.appendChild(factory);
      }
    }
  }

  // *Do not* define any objects if Gears is not installed. This mimics the
  // behavior of Gears defining the objects in the future.
  if (!factory) {
    return;
  }

  // Now set up the objects, being careful not to overwrite anything.
  //
  // Note: In Internet Explorer for Windows Mobile, you can't add properties to
  // the window object. However, global objects are automatically added as
  // properties of the window object in all browsers.
  if (!window.google) {
    google = {};
  }

  if (!google.gears) {
    google.gears = {factory: factory};
  }


  // APPCELERATOR BOOTSTRAP CODE START
  google.gears.appcelerator = {};
  google.gears.appcelerator.workerPool = google.gears.factory.create('beta.workerpool');	

  google.gears.appcelerator.workerPool.onmessage = function(a, b, message) 
  {
	Appcelerator.Util.ServiceBroker.gearsDispatch(message.text);
  };

  var baseScriptPath = (Appcelerator.WidgetPath + 'common/js/gears/').replace(window.location,'');
  google.gears.appcelerator.serviceBrokerId = google.gears.appcelerator.workerPool.createWorkerFromUrl(baseScriptPath + 'servicebroker_remote.js');

  Appcelerator.Core.requireCommonJS('gears/servicebroker_local.js');
})();

Appcelerator.History = {};

Appcelerator.History.changeListeners = [];
Appcelerator.History.currentState = false;

Appcelerator.History.onChange = function(listener)
{
	Appcelerator.History.changeListeners.push(listener);	
};

Appcelerator.History.go = function(historyToken)
{
	document.location.hash = historyToken;
};

Appcelerator.History.fireChange = function(newState)
{
	if (newState && newState.charAt(0)=='#')
    {
        newState = newState.substring(1);
    }
    
    if (newState === '')
    {
    	newState = null;
    }
    
    if (Appcelerator.History.currentState!=newState)
    {
	    Appcelerator.History.currentState = newState;
	    
	    var data = 
	    {
	    	state:newState
	    };
	    
	    for (var c=0;c<Appcelerator.History.changeListeners.length;c++)
	    {
	        var listener = Appcelerator.History.changeListeners[c];
	        listener(newState,data);
	    }
    }
};

if (Appcelerator.Browser.isIE)
{
	Appcelerator.History.loadIE = function()
	{
	    var iframe = document.createElement('iframe');
	    iframe.id='app_hist_frame';
	    iframe.style.position='absolute';
	    iframe.style.left='-10px';
	    iframe.style.top='-10px';
	    iframe.style.width='1px';
	    iframe.style.height='1px';
		iframe.src='javascript:false';	
	    document.body.appendChild(iframe);
	
	    var frame = $('app_hist_frame');
	    var stateField = null;
	    var state = null;
	    var initial = false;
	    
	    setInterval(function()
	    {
	        var doc = frame.contentWindow.document;
	        if (!doc)
	        {
	            return;
	        }
	
	        stateField = doc.getElementById('state');
	        var cur = document.location.hash;
	        
	        if (cur!==initial)
	        {
	            initial = cur;
	            doc.open();
	            doc.write( '<html><body><div id="state">' + cur + '</div></body></html>' );
	            doc.close();
	            Appcelerator.History.fireChange(cur);
	        }
	        else
	        {
	            // check for state
	            if (stateField)
	            {
	                var newState = stateField.innerText;
	                if (state!=newState)
	                {
	                    state = newState;
	                    if (newState==null || newState==='')
	                    {
	                        if (document.location.hash)
	                        {
	                            initial = '#';
	                            document.location.hash='';
	                            Appcelerator.History.fireChange('#');
	                        }
	                    }
	                    else
	                    {
	                        if (newState!=document.location.hash)
	                        {
	                            document.location.hash=newState;
	                            Appcelerator.History.fireChange(document.location.hash);
	                        }
	                    }
	                }
	            }
	            else
	            {
	                if (initial)
	                {
	                    initial = false;
	                }
	            }
	        }
	    },50);	
	};
}

Appcelerator.Compiler.afterDocumentCompile(function()
{
    if (Appcelerator.Browser.isIE)
    {
    	Appcelerator.History.loadIE();
    }
    else if (Appcelerator.Compiler.isCompiledMode)
    {
        // init.js does not define history, maybe it should
        return;
    }
    else
    {
    	// THIS TRICK CAME FROM YUI's HISTORY COMPONENT
    	//
    	// On Safari 1.x and 2.0, the only way to catch a back/forward
        // operation is to watch history.length... We basically exploit
        // what I consider to be a bug (history.length is not supposed
        // to change when going back/forward in the history...) This is
        // why, in the following thread, we first compare the hash,
        // because the hash thing will be fixed in the next major
        // version of Safari. So even if they fix the history.length
        // bug, all this will still work!
        var counter = history.length;
        
        // On Gecko and Opera, we just need to watch the hash...
        var hash = null; // set it to null so we can start off in a null state to check for first change
        
        setInterval( function () 
        {
            var newHash;
            var newCounter;

            newHash = document.location.hash;
            newCounter = history.length;

            if ( newHash !== hash ) 
            {
                hash = newHash;
                counter = newCounter;
                Appcelerator.History.fireChange(newHash);
            } 
            else if ( newCounter !== counter ) 
            {
                // If we ever get here, we should be on Safari...
                hash = newHash;
                counter = newCounter;
                Appcelerator.History.fireChange(newHash);
            }
        }, 50 );
    }
});

Appcelerator.Util.IdleManager = Class.create();

/**
 * idle manager is a simple manager that will track
 * movements on the mouse and keystrokes to determine (infer)
 * when the user is "IDLE" or "ACTIVE"
 *
 */
Appcelerator.Util.IdleManager =
{
    IDLE_TIME: 60000, // 1 min of no activity is considered default idle
    timestamp: null,
    activeListeners:[],
    idleListeners:[],
    idle: false,
    started: false,

    install: function ()
    {
    	if (this.started) return;
    	
    	this.started = true;
        this.event = this.onActivity.bind(this);
        this.timestamp = new Date().getTime();

        // track keyboard and mouse move events to determine
        // when the user is idle
        Event.observe(window.document, 'keypress', this.event, false);
        Event.observe(window.document, 'mousemove', this.event, false);

        var self = this;
        Event.observe(window, 'unload', function()
        {
            Event.stopObserving(window.document, 'keypress', self.event, false);
            Event.stopObserving(window.document, 'mousemove', self.event, false);
            self.event = null;
        }, false);

        // start idle timer monitor
        this.startTimer();
    },

    startTimer: function ()
    {
        this.stopTimer();

        // precision on idle is 1/2 second which should be perfectly acceptable
        this.timer = setInterval(this.onTimer.bind(this), 500);
    },

    stopTimer: function ()
    {
        if (this.timer)
        {
            clearInterval(this.timer);
            this.timer = 0;
        }
    },

    onTimer: function ()
    {
        if (this.isIdle())
        {
            // stop the timer until we're active again
            this.stopTimer();

            this.idle = true;

            // notify listeners
            if (this.idleListeners.length > 0)
            {
                this.idleListeners.each(function(l)
                {
                    l.call(l);
                });
            }
        }
    },

    registerIdleListener: function (l)
    {
    	Appcelerator.Util.IdleManager.install();
        this.idleListeners.push(l);
    },

    unregisterIdleListener: function (l)
    {
        this.idleListeners.remove(l);
    },

    registerActiveListener: function (l)
    {
    	Appcelerator.Util.IdleManager.install();
        this.activeListeners.push(l);
    },

    unregisterActiveListener: function (l)
    {
        this.activeListeners.remove(l);
    },

    isIdle: function ()
    {
        return this.getIdleTimeInMS() >= this.IDLE_TIME;
    },

    getIdleTimeInMS: function ()
    {
        return (new Date().getTime() - this.timestamp);
    },

    onActivity: function ()
    {
        this.timestamp = new Date().getTime();

        // not idle anymore...
        if (this.idle)
        {
            this.idle = false;

            // notify any listeners
            if (this.activeListeners.length > 0)
            {
                this.activeListeners.each(function(l)
                {
                    l.call(l);
                });
            }

            // restart timer
            this.startTimer();
        }

        return true;
    }
};


//Correctly handle PNG transparency in Win IE 5.5 & 6.
//http://homepage.ntlworld.com/bobosola. Updated 18-Jan-2006.
//modified to work with delayed loading for Appcelerator by JHaynie
if (Appcelerator.Browser.isIE6)
{
	document.getElementsByTagNameNS = function(xmlns, tag)
	{
		var ar = $A(document.getElementsByTagName('*'));
		// IE doesn't under default - of course
		if(xmlns == "http://www.w3.org/1999/xhtml") 
		{
			xmlns = "";
		}
		var found = [];
		for (var c=0,len=ar.length;c<len;c++)
		{
			var elem = ar[c];
			if (elem.tagUrn == xmlns && (tag == '*' || elem.nodeName == tag))
			{
				found.push(elem);
			}
		}
		return found;
	};

	try
	{
		// remove CSS background image flicker and cache images if
		// the server says so
		// http://evil.che.lu/2006/9/25/no-more-ie6-background-flicker
   		document.execCommand("BackgroundImageCache", false, true);
    }
	catch(e){}
}/**
 * this is a simple utility for dynamically loading iframes and returning the 
 * content from the iframe's <body> in the callback
 */
Appcelerator.Util.IFrame = 
{
	fetch: function(src,onload,removeOnLoad,copyContent)
	{
		src = Appcelerator.URI.absolutizeURI(src,Appcelerator.DocumentPath);
		
	    setTimeout(function()
	    {
	        copyContent = (copyContent==null) ? false : copyContent;
	        var frameid = 'frame_'+new Date().getTime()+'_'+Math.round(Math.random() * 99);
	        var frame = document.createElement('iframe');
	        Appcelerator.Compiler.setElementId(frame, frameid);
	        //This prevents Firefox 1.5 from getting stuck while trying to get the contents of the new iframe
	        if(!Appcelerator.Browser.isFirefox)
	        {
	            frame.setAttribute('name', frameid);
	        }
            frame.setAttribute('src', src);
	        frame.style.position = 'absolute';
	        frame.style.width = frame.style.height = frame.borderWidth = '1px';
	        // in Opera and Safari you'll need to actually show it or the frame won't load
	        // so we just put it off screen
	        frame.style.left = "-50px";
	        frame.style.top = "-50px";
	        var iframe = document.body.appendChild(frame);
	        // this is a IE speciality
	        if (window.frames && window.frames[frameid]) iframe = window.frames[frameid];
	        iframe.name = frameid;
	        var scope = {iframe:iframe,frameid:frameid,onload:onload,removeOnLoad:(removeOnLoad==null)?true:removeOnLoad,src:src,copyContent:copyContent};
	        if (!Appcelerator.Browser.isFirefox)
	        {
	            setTimeout(Appcelerator.Util.IFrame.checkIFrame.bind(scope),10);
	        }
	        else
	        {
	            iframe.onload = Appcelerator.Util.IFrame.doIFrameLoad.bind(scope);
	        }
	    },0);
	},
	monitor: function(frame,onload)
	{
        var scope = {iframe:frame,frameid:frame.id,onload:onload,removeOnLoad:false};
        if (!Appcelerator.Browser.isFirefox)
        {
            setTimeout(Appcelerator.Util.IFrame.checkIFrame.bind(scope),10);
        }
        else
        {
            frame.onload = Appcelerator.Util.IFrame.doIFrameLoad.bind(scope);
        }
	},
	doIFrameLoad: function()
	{
		var doc = this.iframe.contentDocument || this.iframe.document;
		var body = doc.documentElement.getElementsByTagName('body')[0];
		
		if (Appcelerator.Browser.isSafari && Appcelerator.Browser.isWindows && body.childNodes.length == 0)
		{
			Appcelerator.Util.IFrame.fetch(this.src, this.onload, this.removeOnLoad);
			return;
		}
		
		if (this.copyContent)
		{
	        var div = document.createElement('div');
	        
	        Appcelerator.Util.IFrame.loadStyles(doc.documentElement);
	        
	        var bodydiv = document.createElement('div');
	        bodydiv.innerHTML = body.innerHTML;
	        div.appendChild(bodydiv);
	        
	        this.onload(div);
		}
		else
		{
            this.onload(body);
		}
		if (this.removeOnLoad)
		{
			var f = this.frameid;
			if (Appcelerator.Browser.isFirefox)
			{
				// firefox won't stop spinning with Loading... message
				// if you remove right away
				setTimeout(function(){Element.remove(f)},50);
			}
			else
			{
				Element.remove(f);
			}
		}
	},
	checkIFrame:function()
	{
		var doc = this.iframe.contentDocument || this.iframe.document;
		var dr = doc.readyState;
		if (dr == 'complete' || (!document.getElementById && dr == 'interactive'))
	 	{
	 		Appcelerator.Util.IFrame.doIFrameLoad.call(this);
	 	}
	 	else
	 	{
	  		setTimeout(Appcelerator.Util.IFrame.checkIFrame.bind(this),10);
	 	}
	},
	loadStyles:function(element)
	{
		for (var i = 0; i < element.childNodes.length; i++)
		{
			var node = element.childNodes[i];

			if (node.nodeName == 'STYLE')
			{
				if (Appcelerator.Browser.isIE)
				{
					var style = document.createStyleSheet();
					style.cssText = node.styleSheet.cssText;
				}
				else
				{
					var style = document.createElement('style');
					style.setAttribute('type', 'text/css');
					try
					{
						style.appendChild(document.createTextNode(node.innerHTML));
					}
					catch (e)
					{
						style.cssText = node.innerHTML;
					}				
					Appcelerator.Core.HeadElement.appendChild(style);
				}
			}
			else if (node.nodeName == 'LINK')
			{
				var link = document.createElement('link');
				link.setAttribute('type', node.type);
				link.setAttribute('rel', node.rel);
				link.setAttribute('href', node.getAttribute('href'));
				Appcelerator.Core.HeadElement.appendChild(link);
			}
			
			Appcelerator.Util.IFrame.loadStyles(node);
		}
	}
};
Appcelerator.Util.EventBroadcaster = Class.create();

Object.extend(Appcelerator.Util.EventBroadcaster.prototype,
{
    listeners: [],

    addListener: function(listener)
    {
        this.listeners.push(listener);
    },

    removeListener: function(listener)
    {
        if (!this.listeners)
        {
            return false;
        }
        return this.listeners.remove(listener);
    },

    dispose: function ()
    {
        if (this.listeners)
        {
            this.listeners.clear();
        }
    },

    fireEvent: function()
    {
        if (!this.listeners || this.listeners.length == 0)
        {
            $D(this + ' not firing event: registered no listeners');
            return;
        }

        var args = $A(arguments);

        if (undefined == args || 0 == args.length)
        {
            throw "fire event not correct for " + this + ", requires at least a event name parameter";
        }

        // name is always the first parameter
        var name = args.shift();

        // format is always onXXXX
        var ch = name.charAt(0).toUpperCase();
        var method = "on" + ch + name.substring(1);

        $D(this + ' firing event: ' + method + ', listeners: ' + this.listeners.length);

        this.listeners.each(function(listener)
        {
            var f = listener[method];
            if (f)
            {
                f.apply(listener, args);
            }
            else if (typeof(listener) == 'function')
            {
                listener.apply(listener, args);
            }
        });
    }
});

Appcelerator.Util.KeyManager = Class.create();

Object.extend(Appcelerator.Util.KeyManager,
{
    monitors: [],
    KEY_ENTER: 1,
    KEY_ESCAPE: 2,
    KEY_SPACEBAR: 3,
    disabled: false,
    installed:false,

    toString: function ()
    {
        return '[Appcelerator.Util.KeyManager]';
    },

    disable: function ()
    {
        this.disabled = true;
    },

    enable: function ()
    {
        this.disabled = false;
    },

    install: function ()
    {
        this.installed = true;
        this.keyPressFunc = this.onkeypress.bindAsEventListener(this);
        // NOTE: you *must* not use Event.observe here for this or you
        // won't be able to stop propogration (and you'll get a page reload on
        // hitting the enter key) of the event
        document.body.onkeypress = this.keyPressFunc;
        Appcelerator.Core.onunload(this.dispose.bind(this));
    },

//FIXME FIXME - install the blur/focus handler on body and then just iterate over monitors

    dispose: function ()
    {
        document.body.onkeypress = null;
        this.keyPressFunc = null;
        var self = this;
        this.monitors.each(function(m)
        {
            self.removeHandler(m[0], m[1]);
        });
        this.monitors = null;
    },

    removeHandler: function (key, element)
    {
        if (element)
        {
            var focusHandler = element['_focusHandler_' + key];
            var blurHandler = element['_blurHandler_' + key];

            if (focusHandler)
            {
                Event.stopObserving(element, 'focus', focusHandler, false);
                try
                {
                    delete element['_focusHandler_' + key];
                }
                catch (E)
                {
                }
            }
            if (blurHandler)
            {
                Event.stopObserving(element, 'blur', blurHandler, false);
                try
                {
                    delete element['_blurHandler_' + key];
                }
                catch (E)
                {
                }
            }
        }
        if (this.monitors.length > 0)
        {
            var found = null;
            for (var c = 0,len = this.monitors.length; c < len; c++)
            {
                var a = this.monitors[c];
                if (a[0] == key && a[1] == element)
                {
                    found = a;
                    break;
                }
            }
            if (found)
            {
                this.monitors.remove(found);
            }
        }
    },

    installHandler: function (key, element, handler)
    {
        if (!this.installed) this.install.bind(this);
        
        var self = this;

        this.removeHandler(key, element);

        var array = [key,element,handler];

        if (element)
        {
            var focusHandler = function(e)
            {
                e = Event.getEvent(e);
                element.focused = true;
                self.monitors.remove(array);
                self.monitors.push(array);
                return true;
            };

            var blurHandler = function(e)
            {
                e = Event.getEvent(e);
                element.focused = false;
                self.monitors.remove(array);
                return true;
            };

            element['_focusHandler_' + key] = focusHandler;
            element['_blurHandler_' + key] = blurHandler;

            Event.observe(element, 'focus', focusHandler, false);
            Event.observe(element, 'blur', blurHandler, false);
        }
        else
        {
            this.monitors.push(array);
        }
    },

    onkeypress: function (e)
    {
        if (this.monitors.length > 0 && !this.disabled)
        {
        	e = Event.getEvent(e);
            var stop = false;
            for (var c = 0,len = this.monitors.length; c < len; c++)
            {
                var m = this.monitors[c];
                if ((Event.isEnterKey(e) && m[0] == Appcelerator.Util.KeyManager.KEY_ENTER) ||
                    (Event.isEscapeKey(e) && m[0] == Appcelerator.Util.KeyManager.KEY_ESCAPE) ||
                    (Event.isSpaceBarKey(e) && m[0] == Appcelerator.Util.KeyManager.KEY_SPACEBAR))
                {
                    var target = Event.element(e);
                    if (m[1] == null || target == m[1])
                    {
                        // NOTE: this only allows one handler to be registered for
                        // a field+event combo, we might want to allow multiples here
                        // and just stop below
                        m[2](e);
                        stop = true;
                        break;
                    }
                }
            }
            if (stop)
            {
                Event.stop(e);
                return false;
            }
        }
        return true;
    }
});

Appcelerator.Core.onload(Appcelerator.Util.KeyManager.install.bind(Appcelerator.Util.KeyManager));
if (window.navigator.language != undefined)
{
	Appcelerator.Localization.currentLanguage = window.navigator.language;
}
else
{
    Appcelerator.Localization.currentLanguage = 'en-US';
}

Appcelerator.Localization.LanguageMap = {};

//
// add a language bundle, replacing an existing on if found
// the map should be the 
//
Appcelerator.Localization.addLanguageBundle = function(lang,displayName,map)
{
    map = map==null ? $H() : typeof(map.get)=='function' ? map : $H(map);
	if (Object.isArray(lang))
	{
		for (var c=0;c<lang.length;c++)
		{
			Appcelerator.Localization.addLanguageBundle(lang[c],displayName,map);
		}
	}
	else
	{
		Appcelerator.Localization.LanguageMap['language_'+lang] = {'map':map,'display':displayName,'lang':lang};
		var idx = lang.indexOf('-');
		if (idx > 0)
		{
			Appcelerator.Localization.LanguageMap['language_'+lang.substring(0,idx)] = {'map':map,'display':displayName,'lang':lang.substring(0,idx)};
		}
	}
};

//
// update changes (merge them) with an existing language bundle possibly overwriting
// existing keys found
//
Appcelerator.Localization.updateLanguageBundle = function(lang,displayName,map)
{
	var bundle = Appcelerator.Localization.getBundle(lang);
    if (!bundle)
    {
        Appcelerator.Localization.addLanguageBundle(lang,displayName,map);
    }
    else
    {
        bundle.map = bundle.map.merge(map);
    }
	var idx = lang.indexOf('-');
	if (idx > 0)
	{
		Appcelerator.Localization.updateLanguageBundle(lang.substring(0,idx),displayName,map);
	}
};

//
// get an array of languages. the entry in the array is an
// object with two properties:
// 
// - id - the language code (such as "en")
// - name - the registered language name such as "English"
// 
//
Appcelerator.Localization.getLanguages = function()
{
	var langs = [];
	
	for (var key in Appcelerator.Localization.LanguageMap)
	{
		if (key.startsWith('language_'))
		{
			langs.push({
				id:key.substring(9),
				name:Appcelerator.Localization.LanguageMap[key].display
			});
		}
	}
	return langs;
};

Appcelerator.Localization.getBundle = function(lang)
{
	lang = (lang==null) ? Appcelerator.Localization.currentLanguage : lang;
	var map = Appcelerator.Localization.LanguageMap['language_'+lang];

	if (!map)
	{
		// we don't have a specific language bundle like en-US so we now
		// just look for en part
		var idx = lang.indexOf('-');
		if (idx > 0)
		{
			return Appcelerator.Localization.getBundle(lang.substring(0,idx));
		}
	}
	return map;
}

//
// set a language bundle key for a given language - the language
// bundle must have already been registered or an exception will be raised
//
Appcelerator.Localization.set = function(key,value,lang)
{
	var bundle = Appcelerator.Localization.getBundle(lang);
	if (!bundle || !bundle.map)
	{
		throw "language bundle not found for language: "+lang;
	}
	bundle.map.set(key,value);
}

//
// get a language bundle key value or return defValue if not found
//
Appcelerator.Localization.get = function(key,defValue,lang)
{
	var bundle = Appcelerator.Localization.getBundle(lang);
	if (bundle && bundle.map)
	{
		var value = bundle.map.get(key);
		if (!value)
		{
			var idx = bundle.lang.indexOf('-');
			if (idx > 0)
			{
				return Appcelerator.Localization.get(key,defValue,bundle.lang.substring(0,idx));
			}
		}
		return value || defValue;
	}
	return defValue;
};

Appcelerator.Localization.compiledTemplates = {};

//
// get a language bundle string that is formatted by using args as a json
// array where each key in the template is in the format #{keyname}
//
Appcelerator.Localization.getWithFormat = function (key, defValue, lang, args)
{
	lang = (lang==null) ? Appcelerator.Localization.currentLanguage : lang;
	var cacheKey = key + ':' + lang;
	var cachedCopy = Appcelerator.Localization.compiledTemplates[cacheKey];
	if (!cachedCopy)
	{
		var template = Appcelerator.Localization.get(key,defValue,lang);
		if (!template)
		{
			return defValue;
		}
		cachedCopy = Appcelerator.Compiler.compileTemplate(template);
		Appcelerator.Localization.compiledTemplates[cacheKey]=cachedCopy;
	}
	return cachedCopy.call(cachedCopy,args);
};


//
// default supported tags that can be localized
//
Appcelerator.Localization.supportedTags = 
[
	'div',
	'span',
	'input',
	'a',
	'td',
	'select',
	'option',
	'li',
	'h1',
	'h2',
	'h3',
	'h4',
	'ol',
	'legend',
    'img'
];

//
// register a supported tag
//
Appcelerator.Localization.registerTag = function(tag)
{
	if (Appcelerator.Localization.supportedTags.indexOf(tag)==-1)
	{
		Appcelerator.Localization.supportedTags.push(tag);
	}
};

//
// unregister a supported tag
//
Appcelerator.Localization.unregisterTag = function(tag)
{
	var i = Appcelerator.Localization.supportedTags.indexOf(tag);
	if (i!=-1)
	{
		Appcelerator.Localization.supportedTags.removeAt(i);
	}
};


// 
// create the default english bundle and make it empty
//
Appcelerator.Localization.addLanguageBundle(['en','en-US'],'English',{});



//NOTE: this code has been adapted to load inside the 
//namespace

/**
 * MD5 cryptographic utils
 */
Appcelerator.Util.MD5 = 
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


// 
//
if(!Appcelerator.Browser.isSafari)
{
    String.prototype.gsub = function(pattern, replacement) {
        var result = [], source = this, match;

        if(typeof pattern == 'string') {
            pattern = new RegExp(pattern,'g');
        } else {
            var flags = (pattern.multiline? 'm':'')+(pattern.ignoreCase? 'i':'')+'g';
            pattern = new RegExp(pattern.source,flags);
        }

        replacement = arguments.callee.prepareReplacement(replacement);

        var prevIndex = 0;
        var match = pattern.exec(source);
        while(match)
        {
            result.push(source.slice(prevIndex, match.index));
            prevIndex = pattern.lastIndex;
            result.push(String.interpret(replacement(match)));
            match = pattern.exec(source);
        }
        result.push(source.slice(prevIndex, source.length));

        return result.join('');
    };
    
    String.prototype.gsub.prepareReplacement = function(replacement) {
      if (Object.isFunction(replacement)) return replacement;
      var template = new Template(replacement);
      return function(match) { return template.evaluate(match) };
    };
	
}// 
// register our input button listener for handling
// activators
// 
Appcelerator.Compiler.registerAttributeProcessor(['div','input','button'],'activators',
{
	handle: function(element,attribute,value)
	{
		if (value && (element.nodeName == 'DIV' || element.getAttribute('type') == 'button' || element.nodeName == 'BUTTON'))
		{
			// see if we're part of a field set and if so, add
			// our reference
			//
			Appcelerator.Compiler.addFieldSet(element,true);
			var fields = value.split(',');
			if (fields.length > 0)
			{
				var activator = function()
				{
					var valid = true;
					for (var c=0,len=fields.length;c<len;c++)
					{
						var field = $(fields[c]);
					    var fieldvalid = field.validatorValid;
						if (!fieldvalid)
						{
							valid = false;
							break;
						}
					}
					element.setAttribute('disabled',!valid);
					element.disabled = !valid;
					
					if (element.disabled && element.onActivatorsDisable)
					{
						element.onActivatorsDisable();
					}
					if (!element.disabled && element.onActivatorsEnable)
					{
						element.onActivatorsEnable();
					}
				};
				for (var c=0,len=fields.length;c<len;c++)
				{
					var fieldid = fields[c];
					var field = $(fieldid);
					if (!field)
					{
						throw "syntax error: invalid field: "+fieldid+" specified for activator on field: "+element.id;
					}
					
					if (!Appcelerator.Compiler.getFunction(field,'addValidationListener'))
					{
						throw "syntax error: non-validator field: "+fieldid+" specified for activator on field: "+element.id;
					}
					Appcelerator.Compiler.executeFunction(field,'addValidationListener',[activator]);
				}
				
				activator();
			}
		}
	},
	metadata:
	{
		description: (
		""
		)
	}
});
Appcelerator.Compiler.registerAttributeProcessor('form','disablesubmit',
{
	handle: function(element,attribute,value)
	{
		if (value == "true")
		{
			var input = document.createElement('input');
			input.type = 'text';
			input.style.display = 'none';
			input.name = 'disablesubmit';
			element.appendChild(input);
		}
	},
	metadata:
	{
		description: (
		"When set to true, then it will disable the form from auto-submit when a user hits enter."
		)
	}
});
//
// register our drag-n-drop Draggable attribute listener
//
Appcelerator.Compiler.registerAttributeProcessor('div','draggable',
{
	handle: function(element,attribute,value)
	{
		// activate the element
		if (value && value!='false')
		{
			var options = value == 'true' ? {} : value.evalJSON();
			var d = new Draggable(element.id,options);
			Appcelerator.Compiler.addTrash(element,function()
			{
				d.destroy();
			});
		}
	},
	metadata:
	{
		description: (
		"Make this element draggable. Dragged elements can be dropped on a droppable."
		)
	}
});

//
// register our drag-n-drop Droppable attribute listener
//
Appcelerator.Compiler.registerAttributeProcessor('div','droppable',
{
	handle: function(element,attribute,value)
	{
		if (value && value!='false')
		{
			var options = value == "true" ? {} : value.evalJSON();
			options.onHover = function(e)
			{
				var listeners = element.hoverListeners;
				if (listeners && listeners.length > 0)
				{
					for (var c=0;c<listeners.length;c++)
					{
						var cb = listeners[c];
						cb.onHover(e);
					}
				}
			};
			
			options.onDrop = function(e)
			{
				var listeners = element.dropListeners;
				if (listeners && listeners.length > 0)
				{
					for (var c=0;c<listeners.length;c++)
					{
						var cb = listeners[c];
						cb.onDrop(e);
					}
				}
			};
			
			Droppables.add(element.id,options);
			
			Appcelerator.Compiler.addTrash(element, function()
			{
				Droppables.remove(element);
			});
		}
	},
	metadata:
	{
		description: (
		"Make this element a possible drop target."
		)
	}
});
// 
// register our fieldsets
// 
Appcelerator.Compiler.registerAttributeProcessor('*','fieldset',
{
	handle: function(element,attribute,value)
	{
		if (value && element.getAttribute('type')!='button')
		{
			// see if we're part of a field set and if so, add
			// our reference
			//
			Appcelerator.Compiler.addFieldSet(element,false);

			Appcelerator.Compiler.addTrash(element, function()
			{
			    Appcelerator.Compiler.removeFieldSet(element);
			});
		}
	},
	metadata:
	{
		description: (
		"Groups fields together. Messages sent from an element with a <i>fieldset</i> "+
		"will contain a payload built from all the elements in the fieldset.<br/>"+
		"The keys of the payload will be the names (or, lacking names, the ids) of the fieldset elements "+
		"and the payload values will be the result of Appcelerator.Compiler.getInputFieldValue() on each element."
		)
	}
});
//
// for IE6, we need to apply a PNG transparency fix
//

Appcelerator.Compiler.Image = {};

Appcelerator.Compiler.registerAttributeProcessor('img','srcexpr',
{
	handle: function(img,attribute,value)
	{
		if (value)
		{
			try
			{
				var srcvalue = eval(String.unescapeXML(value));
				if (Appcelerator.Browser.isIE6)
				{
					img.onload = function()
					{
						img.addBehavior(Appcelerator.Core.getModuleCommonDirectory() + '/images/appcelerator/iepngfix.htc');
					};
				}
				img.src = srcvalue;
			}
			catch(e)
			{
				Appcelerator.Compiler.handleElementException(img, e, 'setting img srcexpr using expression = ' + value);
			}
		}
	}
});//
// register an attribute processor for langid that will replace
// the value of the element with the language bundle key value
// the langid attribute should equal the language bundle key
//
Appcelerator.Localization.AttributeProcessor = 
{
	handle: function(element,attribute,value)
	{
		if (value)
		{
			var v = Appcelerator.Localization.get(value);
			if (!v) 
			{
			     Logger.error("couldn't find localization key for "+value);
			     return;
			}
			switch (Appcelerator.Compiler.getTagname(element))
			{
				case 'select':
				{
					element.options.length = 0;
					for (var c=0;c<v.length;c++)
					{
						var o = v[c];
						element.options[element.options.length]=new Option(o.value,o.id);
					}
					break;
				}
				case 'option':
				{
					if (typeof v == 'object')
					{
						element.text = v.value;
						element.value = v.id;
					}
					else
					{
						element.value = element.text = v;
					}
					break;
				}
				case 'input':
				{
					element.value = v;
					break;
				}
				case 'img':
				{
					element.setAttribute("title", v);
					element.setAttribute("alt", v);
					break;
				}
				default:
				{
					element.innerHTML = v;
					break;
				}
			}
		}
	},
	metadata:
	{
		description: (
		"Specify a language bundle key that will be used to lookup localized text for this element."
		)
	}
};

Appcelerator.Compiler.registerAttributeProcessor(Appcelerator.Localization.supportedTags,'langid',Appcelerator.Localization.AttributeProcessor);
Appcelerator.Compiler.registerAttributeProcessor('*','on',
{
	handle: function(element,attribute,value)
	{
		if (value)
		{
			var value = element.getAttribute('set');
			if (value)
			{
				// set calls parse after its done, let it win
				return;
			}
			Appcelerator.Compiler.parseOnAttribute(element);
		}
	},
	metadata:
	{
		description: (
	 	"Contains a Web Expression: a mapping between event triggers and actions. "+
		"Independent trigger/action pairs can be join with the <b>or</b> keyword.<br/> "+
		"Multiple actions for a single trigger can be combined with the <b>and</b> keyword.<br/> "+
		""
		)
	}
});//
// register our resizable attribute listener
//
Appcelerator.Compiler.registerAttributeProcessor(['img'],'reflectable',
{
	handle: function(element,attribute,value)
	{
		if (value && value!='false')
		{
			var options = value == "true" ? {} : value.evalJSON();
			Reflection.add(element, options);
		}
	},
	metadata:
	{
		description: (
		"Add reflection to any image across all browsers."
		)
	}
});

/**
 * Reflection code from reflection.js 1.8 
 *
 * http://cow.neondragon.net/stuff/reflection/
 */

var Reflection = {
	defaultHeight : 0.5,
	defaultOpacity: 0.5,
	
	add: function(image, options) {
		Reflection.remove(image);
		
		doptions = { "height" : Reflection.defaultHeight, "opacity" : Reflection.defaultOpacity }
		if (options) {
			for (var i in doptions) {
				if (!options[i]) {
					options[i] = doptions[i];
				}
			}
		} else {
			options = doptions;
		}
	
		try {
			var d = document.createElement('div');
			var p = image;
			
			var classes = p.className.split(' ');
			var newClasses = '';
			for (j=0;j<classes.length;j++) {
				if (classes[j] != "reflect") {
					if (newClasses) {
						newClasses += ' '
					}
					
					newClasses += classes[j];
				}
			}

			var reflectionHeight = Math.floor(p.height*options['height']);
			var divHeight = Math.floor(p.height*(1+options['height']));
			
			var reflectionWidth = p.width;
			
			if (document.all && !window.opera) {
				/* Fix hyperlinks */
                if(p.parentElement.tagName == 'A') {
	                var d = document.createElement('a');
	                d.href = p.parentElement.href;
                }  
                    
				/* Copy original image's classes & styles to div */
				d.className = newClasses;
				p.className = 'reflected';
				
				d.style.cssText = p.style.cssText;
				p.style.cssText = 'vertical-align: bottom';
			
				var reflection = document.createElement('img');
				reflection.src = p.src;
				reflection.style.width = reflectionWidth+'px';
				reflection.style.display = 'block';
				reflection.style.height = p.height+"px";
				
				reflection.style.marginBottom = "-"+(p.height-reflectionHeight)+'px';
				reflection.style.filter = 'flipv progid:DXImageTransform.Microsoft.Alpha(opacity='+(options['opacity']*100)+', style=1, finishOpacity=0, startx=0, starty=0, finishx=0, finishy='+(options['height']*100)+')';
				
				d.style.width = reflectionWidth+'px';
				d.style.height = divHeight+'px';
				p.parentNode.replaceChild(d, p);
				
				d.appendChild(p);
				d.appendChild(reflection);
			} else {
				var canvas = document.createElement('canvas');
				if (canvas.getContext) {
					/* Copy original image's classes & styles to div */
					d.className = newClasses;
					p.className = 'reflected';
					
					d.style.cssText = p.style.cssText;
					p.style.cssText = 'vertical-align: bottom';
			
					var context = canvas.getContext("2d");
				
					canvas.style.height = reflectionHeight+'px';
					canvas.style.width = reflectionWidth+'px';
					canvas.height = reflectionHeight;
					canvas.width = reflectionWidth;
					
					d.style.width = reflectionWidth+'px';
					d.style.height = divHeight+'px';
					p.parentNode.replaceChild(d, p);
					
					d.appendChild(p);
					d.appendChild(canvas);
					
					context.save();
					
					context.translate(0,image.height-1);
					context.scale(1,-1);
					
					context.drawImage(image, 0, 0, reflectionWidth, image.height);
	
					context.restore();
					
					context.globalCompositeOperation = "destination-out";
					var gradient = context.createLinearGradient(0, 0, 0, reflectionHeight);
					
					gradient.addColorStop(1, "rgba(255, 255, 255, 1.0)");
					gradient.addColorStop(0, "rgba(255, 255, 255, "+(1-options['opacity'])+")");
		
					context.fillStyle = gradient;
					if (navigator.appVersion.indexOf('WebKit') != -1) {
						context.fill();
					} else {
						context.fillRect(0, 0, reflectionWidth, reflectionHeight*2);
					}
				}
			}
		} catch (e) {
	    }
	},
	
	remove : function(image) {
		if (image.className == "reflected") {
			image.className = image.parentNode.className;
			image.parentNode.parentNode.replaceChild(image, image.parentNode);
		}
	}
}//
// register our resizable attribute listener
//
Appcelerator.Compiler.registerAttributeProcessor(['div','img', 'table'],'resizable',
{
	handle: function(element,attribute,value)
	{
		if (value && value!='false')
		{
			var options = value == "true" ? {} : value.evalJSON();
			options.resize = function(e)
			{
				var listeners = element.resizeListeners;
				if (listeners && listeners.length > 0)
				{
					for (var c=0;c<listeners.length;c++)
					{
						var cb = listeners[c];
						cb.onResize(e);
					}
				}
			};
			
			element.resizable = new Resizeable(element.id, options);
			
			Appcelerator.Compiler.addTrash(element, function()
			{
				element.resizable.destroy();
			});
		}
	},
	metadata:
	{
		description: (
		""
		)
	}
});
//
// add a selectable attribute which implements a single state selection
// for all children of element passed in
//
Appcelerator.Compiler.SelectableGroups = {};

Appcelerator.Compiler.retainedWidgetAttributes.push('selectable');

Appcelerator.Compiler.wireSelectable = function(element,value)
{
	element = $(element);
	
	var name = value;
	var multiselect = false;
	var tokens = name.split(',');
	if (tokens.length > 1)
	{
		multiselect = tokens[1]=='multiselect';
	}
	var id = element.id;
	var dontAddStyles = element.getAttribute('supressAutoStyles')=='true';
	
	var members = Appcelerator.Compiler.SelectableGroups[name];
	if (!members)
	{
		var v = multiselect ? [] : null;
		members = {selected:v,members:[]};
		Appcelerator.Compiler.SelectableGroups[name] = members;
	}
	
	var selectFunc = function(element,selected,count,unselect)
	{
		count = count || 0;
		if (!dontAddStyles)
		{
			if (selected)
			{
				element.setAttribute('selected','true');
				Element.addClassName(element,'selected');
			}
			else
			{
				element.removeAttribute('selected');
				Element.removeClassName(element,'selected');
			}
		}
		Appcelerator.Compiler.executeFunction(element,'selected',[selected,count,unselect]);
	}

	for (var c=0,len=element.childNodes.length;c<len;c++)
	{
		var child = element.childNodes[c];
		if (child && child.nodeType && child.nodeType == 1)
		{
		    (function()
		    {
	            var childid = Appcelerator.Compiler.getAndEnsureId(child);
	            members.members.push(childid);
	            var scope = {};
	            scope.element = child;
	            child.app_selectable = true;
	            var selectListener = function(e,unselectOnly)
	            {
	                e = Event.getEvent(e);
	                var target = this.element;
	                if (e)
	                {
	                    var t = Event.element(e);
	                    
	                    if (t == this.element)
	                    {
	                        if (e._selectedEventSeen)
	                        {
	                            return;
	                        }
	                        e._selectedEventSeen = true;
	                    }
	                    else
	                    {
	                        // TODO: walk up to find the selectable div and then
	                        // make sure he's selected if not
	                        var p = t.parentNode;
	                        while (p && p!=document.body)
	                        {
	                            if (p.app_selectable)
	                            {
	                                target = p;
	                                if (e._selectedEventSeen)
	                                {
	                                    return;
	                                }
	                                if (target == members.selected)
	                                {
	                                    return;
	                                }
	                                e._selectedEventSeen = true;
	                                break;
	                            }
	                            else
	                            {
	                                p = p.parentNode;
	                            }
	                        }
	                    }
	                }
	                unselectOnly = unselectOnly!=null ? unselectOnly==true : false;
	                var fired = false;
	                var unselect = unselectOnly;
	                var reselect = (!unselectOnly && members.selected && members.selected!=target);
	                if (members.selected || (unselectOnly && !members.selected))
	                {
	                    if (multiselect)
	                    {
	                        var idx = members.selected.indexOf(target.id);
	                        if ( idx >=0 )
	                        {
	                            // unselect
	                            unselect = true;
	                            selectFunc(target,false,members.selected.length-1,unselect);
	                            members.selected.removeAt(idx);
	                            fired=true;
	                            return;
	                        }
	                    }
	                    else
	                    {
	                        var t = members.selected;
	                        selectFunc(members.selected,false,reselect?1:0,true);
	                        fired=true;
	                        members.selected = null;
	                        if (t == target)
	                        {
	                            return;
	                        }
	                    }
	                }
	                if (!unselectOnly)
	                {
	                    var count = 1;
	                    if (multiselect)
	                    {
	                        members.selected.push(target.id);
	                        count = members.selected.length;
	                    }
	                    else
	                    {
	                        members.selected = target;
	                    }
	                    selectFunc(target,true,count,false);
	                    fired=true;
	                }
	                else if (!fired)
	                {
	                    var idx = members.selected.indexOf(target.id);
	                    if ( idx >=0 )
	                    {
	                        // unselect
	                        selectFunc(target,false,members.selected.length-1,unselect);
	                        members.selected.removeAt(idx);
	                        fired=true;
	                        return;
	                    }
	                    Logger.warn('not removed on unselected '+target.id+',current='+members.selected.join(','));
	                }
	            };
	            var f = selectListener.bind(scope);
	            Appcelerator.Compiler.attachFunction(child,'unselect',function()
                {
                    selectListener.apply(this,[null,true]);
                }.bind(scope));
                Appcelerator.Compiler.attachFunction(child,'select',function()
                {
                    selectListener.apply(this,[null,false]);
                }.bind(scope));
	            Event.observe(child,'click',f,false);
	            Appcelerator.Compiler.addTrash(child,function()
	            {
	                selectListener.apply(this,[null,true]);
	                members.members.remove(this.element.id);
	                Event.stopObserving(this.element,'click',f);
	            }.bind(scope));
		    })();
		}
	}
};

Appcelerator.Compiler.registerAttributeProcessor(['div','ul','ol','tr'],'selectable',
{
	handle: function(element,attribute,value)
	{
		if (value)
		{
			Appcelerator.Compiler.wireSelectable(element,value);
		}
	}
});

//
// register our drag-n-drop Sortable attribute listener
//
Appcelerator.Compiler.registerAttributeProcessor(['div','ul','ol'],'sortable',
{
	handle: function(element,attribute,value)
	{
		// activate the element
		if (value && value!='false')
		{
			var options = value == 'true' ? {} : value.evalJSON();
			
			// let's be smart here and go ahead and set the child tag type
			// if not already set
			if (!options.tag)
			{
				Element.cleanWhitespace(element);
				var child = element.down();
				if (child)
				{
					options.tag = Appcelerator.Compiler.getTagname(child);
				}
			}
			
			options.onUpdate = function(e)
			{
				var listeners = element.updateListeners;
				if (listeners && listeners.length > 0)
				{
					for (var c=0;c<listeners.length;c++)
					{
						var cb = listeners[c];
						cb.onUpdate(e);
					}
				}
			};
			
			options.onChange = function(e)
			{
				var listeners = element.changeListeners;
				if (listeners && listeners.length > 0)
				{
					for (var c=0;c<listeners.length;c++)
					{
						var cb = listeners[c];
						cb.onChange(e);
					}
				}
			};
			
			Sortable.create(element.id,options);

			Appcelerator.Compiler.addTrash(element,function()
			{
				Sortable.destroy(element);
			});
		}
	}
});// 
// register our input fields listener
// 
Appcelerator.Compiler.registerAttributeProcessor(['textarea','input','select'],'validator',
{
	handle: function(element,attribute,value)
	{
		if (value && element.getAttribute('type')!='button')
		{
			// get the validator
			var validatorFunc = Appcelerator.Validator[value];
			if (!validatorFunc)
			{
				throw "syntax error: validator specified is not registered: "+value;
			}

			var validatorFunc = Appcelerator.Validator[value];
			var value = Appcelerator.Compiler.getInputFieldValue(element,true,true);
			element.validatorValid = validatorFunc(value, element) || false;
			
			// get the decorator
			var decoratorValue = element.getAttribute('decorator');
			var decorator = null, decoratorId = null;

			if (decoratorValue)
			{
				decorator = Appcelerator.Decorator[decoratorValue];
				if (!decorator)
				{
					throw "syntax error: decorator specified is not registered: "+decoratorValue;
				}
				decoratorId = decorator ? element.getAttribute('decoratorId') : null;
			}
						
			// optimize the revalidate by only running the validator/decorator logic after a period
			// of no changes for 100 ms - such that continous keystrokes won't continue
			// to cause revalidations -- or -- if we reach 10 keystrokes (which is arbitrary
			// but helps have more instant validation for longer values)
			var timer = null;
			var keystrokeCount = 0;
			var timerFunc = function()
			{
				keystrokeCount=0;
				Appcelerator.Compiler.executeFunction(element,'revalidate');
			};
			
			Appcelerator.Compiler.installChangeListener(element,function()
			{
				if (timer)
				{
					clearTimeout(timer);
					timer=null;
				}
				if (keystrokeCount++ < 10)
				{
					timer = setTimeout(timerFunc,100);
				}
				else
				{
					timerFunc();
				}
				return true;
			});
			
			var validationListeners = [];
			
			Appcelerator.Compiler.attachFunction(element,'revalidate',function()
			{
				// FIXME : this was needed for IE/Safari port but break firefox of course...				
				// if (!Element.showing(element))
				// {
				// 	return element.validatorValid;
				// }
				var value = Appcelerator.Compiler.getInputFieldValue(element,true,true);
				var valid = validatorFunc(value, element);
				var same = valid == element.validatorValid;
							
				element.validatorValid = valid;
				if (decorator)
				{
					decorator.apply(Appcelerator.Decorator,[element,element.validatorValid,decoratorId]);
				}
				if (!value)
				{
					Element.addClassName(element,'validator_empty');
					Element.removeClassName(element,'validator_value');
				}
				else
				{
					Element.addClassName(element,'validator_value');
					Element.removeClassName(element,'validator_empty');
				}
				if (valid)
				{
					Element.removeClassName(element,'validator_invalid');
					Element.addClassName(element,'validator_valid');
				}
				else
				{
					Element.removeClassName(element,'validator_valid');
					Element.addClassName(element,'validator_invalid');
				}
				if (same)
				{
					// if the same, don't refire events
					return valid;
				}
				if (validationListeners.length > 0)
				{
					for (var c=0,len=validationListeners.length;c<len;c++)
					{
						var listener = validationListeners[c];
						listener.apply(listener,[element,element.validatorValid]);
					}
				}
				return element.validatorValid;
			});
			
			Appcelerator.Compiler.attachFunction(element,'removeValidationListener',function(listener)
			{
				var idx = validationListeners.indexOf(listener);
				if (idx >= 0)
				{
					validationListeners.removeAt(idx);
				}
			});
			
			Appcelerator.Compiler.attachFunction(element,'addValidationListener',function(listener)
			{
				validationListeners.push(listener);
			});
			
			Appcelerator.Compiler.addTrash(element, function()
			{
			    Appcelerator.Compiler.removeChangeListener(element);
				validationListeners = null;
			});
			
			Appcelerator.Compiler.executeFunction(element,'revalidate');
		}
	}
});
Appcelerator.UI = Class.create();
Appcelerator.UI.UIManager = {managers:{}};
Appcelerator.UI.UIComponents = {};
Appcelerator.UI.UIListeners = {};

Appcelerator.UI.registerListener = function(type,name,event,callback)
{
	var f = function()
	{
		if (this.name == name || name == '*')
		{
			if (this.event == event || event == '*')
			{
				var scope = this.data || {};
				scope.type = this.type;
				scope.name = this.name;
				scope.event = this.event;
				callback.call(scope);
			}
		}
	};
	var listeners = Appcelerator.UI.UIListeners[type];
	if (!listeners)
	{
		listeners=[];
		Appcelerator.UI.UIListeners[type] = listeners;
	}
	listeners.push(f);
};

Appcelerator.UI.fireEvent = function(type,name,event,data)
{
	var listeners = Appcelerator.UI.UIListeners[type];
	if (listeners && listeners.length > 0)
	{
		var scope = {type:type,name:name,event:event,data:data};
		for (var c=0;c<listeners.length;c++)
		{
			listeners[c].call(scope);
		}
	}
};

/**
 * called by an UI manager implementation to register itself by type
 */
Appcelerator.UI.registerUIManager = function(ui,impl)
{
	Appcelerator.UI.UIManager.managers[ui] = impl;
};

/**
 * called by UI manager to register itself
 */
Appcelerator.UI.registerUIComponent = function(type,name,impl)
{
	try
	{
		var f = Appcelerator.UI.UIComponents[type+':'+name];

		if (!f)
		{
			f = {};
			Appcelerator.UI.UIComponents[type+':'+name]=f;
		}

		f.impl = impl;
		f.loaded = true;

		if (impl.setPath)
		{
			impl.setPath.call(impl,f.dir);
		}

		if (f.elements)
		{
			for (var c=0;c<f.elements.length;c++)
			{
				var obj = f.elements[c];
				Appcelerator.UI.activateUIComponent(f.impl,f.dir,obj.type,obj.name,obj.element,obj.options,obj.callback);
			}

			f.elements = null;
		}
	}
	catch(e)
	{
		Appcelerator.Compiler.handleElementException(null,e,'registerUIComponent for '+type+':'+name);
	}
};

Appcelerator.UI.activateUIComponent = function(impl,setdir,type,name,element,options,callback)
{
	var formattedOptions = Appcelerator.UI.UIManager.parseAttributes(element,impl,options);
	if (formattedOptions!=false)
	{
		try
		{
			impl.build(element,formattedOptions);

			// keep track of elements and their UI attributes
			Appcelerator.UI.addElementUI(element,type,name);
		}
		catch (e)
		{
			Appcelerator.Compiler.handleElementException(element,e,'activateUIComponent for '+type+':'+name);
		}
	}
	if (impl.getActions)
	{
		var actions = impl.getActions();
		var id = element.id;
		for (var c=0;c<actions.length;c++)
		{
			(function()
			{
				var actionName = actions[c];
				var action = impl[actionName];
				if (action)
				{
					var xf = function(id,m,data,scope,version,customActionArguments,direction,type)
					{
						try
						{
							action.apply(impl,[id,formattedOptions,data,scope,version,customActionArguments,direction,type]);
						}
						catch (e)
						{
							$E('Error executing '+actionName+' in container type: '+type+'. Error '+Object.getExceptionDetail(e)+', stack='+e.stack);
						}
					};
					Appcelerator.Compiler.buildCustomElementAction(actionName, element, xf);
				}
			})();
		}
	}

	if (impl.getConditions)
	{
        Appcelerator.Compiler.customConditionObservers[element.id] = {};
        var customConditions = impl.getConditions();
        for (var i = 0; i < customConditions.length; i++)
        {
            var custCond = customConditions[i];
            var condFunct = Appcelerator.Compiler.customConditionFunctionCallback(custCond);
            Appcelerator.Compiler.registerCustomCondition({conditionNames: [custCond]}, 
                condFunct, element.id);
        }
	}
	
	Appcelerator.Compiler.parseOnAttribute(element);
	
	if (callback)
	{
		callback();
	}
};

/**
 * called to load UI component by UI manager
 */ 
Appcelerator.UI.loadUIComponent = function(type,name,element,options,callback)
{
	var f = Appcelerator.UI.UIComponents[type+':'+name];
	if (f)
	{
		if (f.loaded)
		{
			Appcelerator.UI.activateUIComponent(f.impl,f.dir,type,name,element,options,callback);
		}
		else
		{
			f.elements.push({type:type,name:name,element:element,options:options,callback:callback});
		}
	}
	else
	{
		// added for API calls
		if (!element.state)element.state = {pending:0};
		
		element.state.pending+=1;
		var dir = Appcelerator.DocumentPath + '/components/'+type+'s/'+name;
		var path = dir+'/'+name+'.js';
		Appcelerator.UI.UIComponents[type+':'+name] = {dir:dir,loaded:false,elements:[{type:type,name:name,element:element,options:options,callback:callback}]};

		Appcelerator.Core.remoteLoadScript(path,function()
		{
			element.state.pending-=1;
			Appcelerator.Compiler.checkLoadState(element);
		},function()
		{
			Appcelerator.UI.UIManager.handleLoadError(element,type,name,null,path);
			element.state.pending-=1;
			Appcelerator.Compiler.checkLoadState(element);
		});
	}
};

/**
 * called to handle load error
 */
Appcelerator.UI.UIManager.handleLoadError = function(element,type,name,subtype,path)
{
	$E("error loading - type:"+type+",name:"+name+",subtype:"+subtype+"\nfor "+element.id+' from url='+path);
	Appcelerator.Compiler.handleElementException(element,null,"couldn't load "+type+":"+name+" for : "+path);

	//TODO: determine if we're online or offline to determine action here
	//FIXME: add widget error handling
	//top.document.location.href = Appcelerator.DocumentPath + 'component_notfound.html?type='+encodeURIComponent(type)+'&name='+encodeURIComponent(name)+'&url='+encodeURIComponent(top.document.location.href)+'&'+(subtype ? ('&subtype='+encodeURIComponent(subtype)) : '');
};

/************************************
 *  API FUNCTIONS
 ***********************************/
Appcelerator.UI.createControl = function (element,type,args,callback)
{
	Appcelerator.loadUIManager('control',type,element,args||{},callback);
};
Appcelerator.UI.addBehavior = function(element,type,args,callback)
{
	Appcelerator.loadUIManager('behavior',type,element,args||{},callback);
};
Appcelerator.UI.createLayout = function(element,type,args,callback)
{
	Appcelerator.loadUIManager('layout',type,element,args||{},callback);
};

/****************************************************
  HANDLE CROSS-CONTROL/BEHAVIOR/LAYOUT DEPENDENCIES
*****************************************************/
Appcelerator.UI.dependencyMap = [];
Appcelerator.UI.elementMap = {};

//
// allow components to register their dependencies for an element
//
Appcelerator.UI.addElementUIDependency = function(element,ui,type,dependencyUI, dependencyType, callback)
{

	// see if element already has UI attribute that is a dependency
	if (Appcelerator.UI.elementMap[element.id + "_" + dependencyUI +"_" + dependencyType])
	{
		callback(element);
	}
	
	// otherwise store it for later
	else
	{
		Appcelerator.UI.dependencyMap.push({element:element,ui:ui,type:type,dependencyUI:dependencyUI,dependencyType:dependencyType,callback:callback});	
	}
};

// 
// Keep track of an element's UI attributes (controls, behaviors, layouts, etc)
// 
Appcelerator.UI.addElementUI = function(element, ui, type)
{
	// is UI attribute combo part of an existing dependency
	var map = Appcelerator.UI.dependencyMap;
	for (var i=0;i<map.length;i++)
	{
		if (map[i].element.id == element.id)
		{
			// new UI + TYPE has a dependency for this element
			if ((map[i].dependencyUI == ui) && (map[i].dependencyType == type))
			{
				// see if element already has UI + TYPE 
				if (Appcelerator.UI.elementMap[element.id + "_" + map[i].ui + "_" + map[i].type])
				{
					map[i].callback(element);
				}
			}
		}
	}
	Appcelerator.UI.elementMap[element.id + "_" + ui + "_" + type] = {element:element};
	
};

/**
 * called by a UI to load a UI manager
 */
Appcelerator.loadUIManager=function(ui,type,element,args,failIfNotFound,callback)
{
	var f = Appcelerator.UI.UIManager.managers[ui];
	if (f)
	{
		var data = {element:element,args:args};
		Appcelerator.UI.fireEvent(ui,type,'beforeBuild',data);
		var afterBuild = function()
		{
			Appcelerator.UI.fireEvent(ui,type,'afterBuild',data);
			if (callback) callback();
		};
		f(type,element,args,afterBuild);
	} 
	else
	{
		if (failIfNotFound==true)
		{
			$E('UI not found for '+ui+', type: '+type);
		}
		else
		{
			element.state.pending+=1;
			Appcelerator.Core.requireCommonJS('appcelerator/'+ui+'s/'+ui+'s.js',function()
			{
				Appcelerator.UI.fireEvent(ui,type,'register');
				Appcelerator.loadUIManager(ui,type,element,args,true,callback);
				element.state.pending-=1;
				Appcelerator.Compiler.checkLoadState(element.state);
			},function()
			{
				Appcelerator.Compiler.handleElementException(element,null,'error loading '+type+'['+name+']');
				element.state.pending-=1;
				Appcelerator.Compiler.checkLoadState(element.state);
			});
		}
	}
};

Appcelerator.Compiler.registerAttributeProcessor('*','set',
{
	queue:[],
	handle: function(element,attribute,value)
	{
		Element.addClassName(element,'container');

		// parse value
		var expressions = Appcelerator.Compiler.smartSplit(value,' and ');
		var count = 0;
		var compiler = function()
		{
			count++;
			if (count == expressions.length)
			{
				Appcelerator.Compiler.compileElementChildren(element);
			}
		};
		for (var i=0;i<expressions.length;i++)
		{
			// turn into comma-delimited string
			var delimitedString = expressions[i].replace("[",",").replace("]","");
			var a = delimitedString.split(",");
			
			// syntax: attribute[attributeType,arg1=val1,arg2=val2]
			var ui;
			var type;
			var args = {};

			for (var j=0;j<a.length;j++)
			{
				if (j==0)ui = a[0].trim();
				else if (j==1)type = a[1].trim();
				else
				{
					var pair = a[j].split("=");
					args[pair[0].trim()] = pair[1].trim();
				} 
			}
			if (i == 0) element.stopCompile=true;
			Appcelerator.loadUIManager(ui,type,element,args,false,compiler);
		}
	},
	metadata:
	{
		description: (
			"set visual properties for an element"
		)
	}
});

Appcelerator.UI.UIManager.defaultThemes = 
{
	'panel':'basic',
	'shadow':'basic',
	'button':'white_gradient',
	'input':'white_gradient',
	'textarea':'white_gradient',
	'select':'thinline',
	'tabpanel':'white',
	'accordion':'basic'
};

Appcelerator.UI.UIManager.getDefaultTheme = function(type)
{
	return Appcelerator.UI.UIManager.defaultThemes[type];
};

Appcelerator.UI.UIManager.setDefaultThemes = function(type,theme)
{
	Appcelerator.UI.UIManager.defaultThemes[type] = theme;
};

Appcelerator.UI.UIManager.attrToJSON = function(attrs)
{
	var a = attrs.split(",");
	
	var args = {};

	for (var j=0;j<a.length;j++)
	{
		var pair = a[j].split("=");
		args[pair[0].trim()] = pair[1].trim();
	}

	return args;
};

Appcelerator.UI.UIManager.parseAttributes = function(element,f,options)
{
	var moduleAttributes = f.getAttributes();
	for (var i = 0; i < moduleAttributes.length; i++)
	{
		var error = false;
		var modAttr = moduleAttributes[i];
		var value =  options[modAttr.name] || element.style[modAttr.name] || modAttr.defaultValue;
		// check and make sure the value isn't a function as what will happen in certain
		// situations because of prototype's fun feature of attaching crap on to the Object prototype
		if (Object.isFunction(value))
		{
			value = modAttr.defaultValue;
		}
		if (!value && !modAttr.optional)
		{
			Appcelerator.Compiler.handleElementException(element, null, 'required attribute "' + modAttr.name + '" not defined for ' + element.id);
			error = true;
		}
		options[modAttr.name] = value;
		if (error == true)
		{
			$E('error parsing attributes for '+element);
			return false;
		}
	}
	return options;
};

Appcelerator.UI.themes = {};

Appcelerator.Core.registerTheme = function(type,container,theme,impl)
{
	var key = Appcelerator.Core.getThemeKey(type,container,theme);
	var themeImpl = Appcelerator.UI.themes[key];
	if (!themeImpl)
	{
		themeImpl = {};
		Appcelerator.UI.themes[key] = themeImpl;
	}
	themeImpl.impl = impl;
	themeImpl.loaded = true;
	// trigger on registration any pending guys
	Appcelerator.Core.loadTheme(type,container,theme,null,null);
};

Appcelerator.Core.getThemeKey = function(pkg,container,theme)
{
	return pkg + ':' + container + ':' + theme;
};

Appcelerator.Core.loadTheme = function(pkg,container,theme,element,options)
{
	theme = theme || Appcelerator.UI.UIManager.getDefaultTheme(container);
	var key = Appcelerator.Core.getThemeKey(pkg,container,theme);
	var themeImpl = Appcelerator.UI.themes[key];
	var fetch = false;
	var path = Appcelerator.DocumentPath + '/components/' + pkg + 's/' + container + '/themes/' +theme;

	if (!themeImpl)
	{
		themeImpl = { callbacks: [], impl: null, loaded: false, path: path };
		Appcelerator.UI.themes[key] = themeImpl;
		fetch = true;
	}
	
	if (themeImpl.loaded)
	{
		if (themeImpl.callbacks && themeImpl.callbacks.length > 0 && themeImpl.impl && themeImpl.impl.build)
		{
			for (var c=0;c<themeImpl.callbacks.length;c++)
			{
				var callback = themeImpl.callbacks[c];
				themeImpl.impl.build(callback.element,callback.options);
			}
		}
		if (element!=null && options!=null && themeImpl.impl && themeImpl.impl.build)
		{
			if (themeImpl.impl.setPath)
			{
				themeImpl.impl.setPath.call(themeImpl.impl,path);
			}
			themeImpl.impl.build(element,options);
		}
		themeImpl.callbacks = null;
	}
	else
	{
		themeImpl.callbacks.push({element:element,options:options});
	}
	
	if (fetch)
	{
		var css_path =  path + '/' +theme+  '.css';
		Appcelerator.Core.remoteLoadCSS(css_path);

		var js_path = path + '/' +theme+  '.js';
		Appcelerator.Core.remoteLoadScript(js_path,null,function()
		{
			Appcelerator.UI.UIManager.handleLoadError(element,pkg,theme,container,js_path);
		});
	}
};

//
// this is a special type of UI manager always available
//
Appcelerator.UI.registerUIManager('theme', function(theme,element,options,callback)
{
	// is this a default setting
	if (theme == 'defaults')
	{
		for (var key in options)
		{
			Appcelerator.UI.UIManager.setDefaultThemes(key,options[key])
		}
		Appcelerator.Compiler.compileElementChildren(element);
	}
	else
	{
		Element.addClassName(element,'themed');
		var type = element.nodeName.toLowerCase();
		options['theme']=theme;
		Appcelerator.UI.loadUIComponent('control',type,element,options,callback);		
	}
});

Appcelerator.UI.ContainerManager = {};
Appcelerator.UI.widgetRegex = /^app:/

Appcelerator.UI.registerUIManager('control',function(type,element,options,callback)
{
	// check for backwards-porting of widgets
	// this will eventually be deprecated
	if (Appcelerator.UI.widgetRegex.test(type))
	{
	    var state = Appcelerator.Compiler.createCompilerState();
		state.pending+=1;
		state.scanned = true;
		Appcelerator.Core.requireModule(type,function()
		{
			var widgetJS = Appcelerator.Compiler.compileWidget(element,state,type);
			state.pending -= 1;
			Appcelerator.Compiler.checkLoadState(state);
			Element.fire(element,'element:compiled:'+element.id,{id:element.id});
		});
		return;
	}
    Element.addClassName(element,type);
    Appcelerator.UI.loadUIComponent('control',type,element,options,callback);
});

Appcelerator.UI.LayoutManager = {};
Appcelerator.UI.LayoutManager._formatTable = function(options)
{
   return '<table width="'+options['width']+'" cellspacing="'+(options['cellspacing'] || '') +'" cellpadding="'+ (options['cellpadding'] || '0') + '">';
};

Appcelerator.UI.LayoutManager._buildForm = function(options)
{
	var childNodes = options['childNodes'];
	var html = options['html'];
	var align = options['align'];
	var colspan = options['colspan'];
	var hintPos = options['hintPos'];
	var errorPos = options['errorPos'];
	var buttonPos = options['buttonPos'];
	var labelWidth = options['labelWidth'];
	var formElement = options['element'];
	
	var defaultFieldset = formElement.getAttribute('fieldset') || formElement.id+'_fieldset';
	
	var inputHTML = [];
	var labelHTML = [];
	var buttonHTML = [];
	var hintHTML = [];
	var errorHTML = [];
	for (var c=0,len=childNodes.length;c<len;c++)
	{
		var node = childNodes[c];
		if (node.nodeType == 1)
		{
			switch (node.tagName.toLowerCase())
			{
				case 'input':
				case 'select':
				case 'textarea':
				{
					if (node.getAttribute('type') == 'button')
					{
						if (Appcelerator.Browser.isIE)
						{
							buttonHTML.push(node.outerHTML);
						}
						else
						{
							buttonHTML.push(Appcelerator.Util.Dom.toXML(node,true,Appcelerator.Compiler.getTagname(node)));	
						}
					}
					else
					{
						var fs = node.getAttribute('fieldset');
						if (!fs)
						{
							node.setAttribute('fieldset',defaultFieldset);
						}
						inputHTML.push({'element':node});
					}
					break;
				}
				case 'label':
				{
					if (Appcelerator.Browser.isIE)
					{
						if (node.getAttribute("type") == "hint")
						{
							hintHTML.push({'id':node.htmlFor,'element':node,'html':node.outerHTML});
						}
						else if (node.getAttribute("type") == "error")
						{
							errorHTML.push({'id':node.htmlFor,'element':node,'html':node.outerHTML});
						}
						else
						{
							labelHTML.push({'id':node.htmlFor,'element':node,'html':node.outerHTML});						
						}
					}
					else
					{
						if (node.getAttribute("type") == "hint")
						{
							hintHTML.push({'id':node.getAttribute('for'),'element':node,'html':Appcelerator.Util.Dom.toXML(node,true,Appcelerator.Compiler.getTagname(node))});
						}
						else if (node.getAttribute("type") == "error")
						{
							errorHTML.push({'id':node.getAttribute('for'),'element':node,'html':Appcelerator.Util.Dom.toXML(node,true,Appcelerator.Compiler.getTagname(node))});
						}
						else
						{
							labelHTML.push({'id':node.getAttribute('for'),'element':node,'html':Appcelerator.Util.Dom.toXML(node,true,Appcelerator.Compiler.getTagname(node))});						
						}
					}
					break;
				}
				case 'button':
				{
					if (Appcelerator.Browser.isIE)
					{
						buttonHTML.push(node.outerHTML);
					}
					else
					{
						buttonHTML.push(Appcelerator.Util.Dom.toXML(node,true,Appcelerator.Compiler.getTagname(node)));	
					}
					break;
				}
			}
		}	
	}
	// 
	// horizontal: hint (top, right, bottom, input), error (top, right, bottom)
	// vertical: hint (top, right, bottom, input), error (top, right, bottom)
	// 
	for (var x=0;x<inputHTML.length;x++)
	{
		(function()
		{
			var label = '';
			var error = ''
			var hint = '';
			var input = '';
			input = inputHTML[x].element;

			// define label for this input
			label = '';
			for (var i=0;i<labelHTML.length;i++)
			{
				if (labelHTML[i].id == input.id)
				{
					label = labelHTML[i].html;
					break;
				}
			}

			// define error for this input
			error = ''
			for (var i=0;i<errorHTML.length;i++)
			{
				if (errorHTML[i].id == input.id)
				{
					error = errorHTML[i].html;
					break;
				}
			}

			// define hint for this input
			hint = '';
			for (var i=0;i<hintHTML.length;i++)
			{
				if (hintHTML[i].id == input.id)
				{
					hint = (hintPos == 'input')?hintHTML[i].element.innerHTML:hintHTML[i].html;
					break;
				}
			}

			if (align=='horizontal')
			{
				var valign = 'middle';
				var labelPadding = "5px";
				var inputPadding = "5px";
				var topPadding = "0px";
				if ((errorPos == 'top' || errorPos == 'bottom')&&((hintPos == 'top' || hintPos == 'bottom')))
				{
					valign = "middle";				
					labelPadding = "15px"
				}
				
				else if (errorPos == 'top' || hintPos == 'top')
				{
					valign = "bottom";
				}
				else if (errorPos=='bottom' || hintPos=='bottom')
				{
					valign = "top";
					topPadding = "4px"
				}
				// create form
				if ((labelPadding == "5px")  && (hintPos != 'bottom') && (errorPos != 'bottom'))
				{
					labelPadding = "9px";
				}
				
				html += '<tr><td align="left" valign="'+valign+'" width="'+labelWidth+'" style="padding-bottom:'+labelPadding+';padding-top:'+topPadding+'" >' + label + '</td>';
				html += '<td align="left" style="padding-bottom:'+inputPadding+'">';
				html += (hintPos == "top")?'<div>'+hint+'</div>':'';
				html += (errorPos == "top")?'<div>'+error+'</div>':'';
				html += Appcelerator.Util.Dom.toXML(input,true,Appcelerator.Compiler.getTagname(input));
				html += (hintPos == "right")?'<span style="padding-left:5px">'+hint+'</span>':'';
				html += (errorPos == "right")?'<span style="padding-left:5px">'+error+'</span>':'';
				html += (hintPos == 'bottom')?'<div style="margin-bottom:10px;position:relative;top:-1px">'+hint + '</div>':'';				
				html += (errorPos == 'bottom')?'<div style="margin-bottom:10px;position:relative;top:-1px">'+error + '</div>':'';
				html += '</td></tr>';
			}
			else
			{
				// create form
				html += '<tr><td align="left">' + label;
				html += (hintPos == "top")?'<span style="padding-left:5px">'+hint+'</span>':'';
				html += (errorPos == "top")?'<span style="padding-left:5px">'+error+'</span>':'';
				html += '</td></tr><tr>';
				html += (errorPos != 'bottom' && hintPos != 'bottom')?'<td align="left" style="padding-bottom:5px">':'<td align="left">';
				html += Appcelerator.Util.Dom.toXML(input,true,Appcelerator.Compiler.getTagname(input));
				html += (hintPos == "right")?'<span style="padding-left:5px">'+hint+'</span>':'';
				html += (errorPos == "right")?'<span style="padding-left:5px">'+error+'</span>':'';
				html += (hintPos == 'bottom')?'<div style="margin-bottom:5px;position:relative;top:-1px">'+hint + '</div>':'';				
				html += (errorPos == 'bottom')?'<div style="margin-bottom:5px;position:relative;top:-1px">'+error + '</div>':'';
				html += '</td></tr>';

			}
			if (options['hintPos'] == "input")
			{
				formElement.observe('element:compiled:'+formElement.id,function(a)
				{
					if (hintPos == "input")
					{
						$(input.id).value = hint;
						Element.addClassName($(input.id),'layout_form_hinttext');
					}

					Event.observe(input.id,'click',function(e)
					{
						if ($(input.id).value == hint)
						{
							$(input.id).value = '';
							Element.removeClassName($(input.id),'layout_form_hinttext');
						}
					});
					Event.observe(input.id,'blur',function(e)
					{
						if ($(input.id).value == '')
						{
							$(input.id).value = hint;
							Element.addClassName($(input.id),'layout_form_hinttext');

						}
					});
				});
			}
		})();
	}
	if (buttonHTML.length > 0)
	{
		var buttonPadding = (errorPos == 'bottom' || hintPos == 'bottom')?"0px":"5px";
		var paddingBottom = "5px";
		if (buttonPos == "right")
		{
			html += '<tr><td></td><td align="left" colspan="1" style="padding-top:'+buttonPadding+';padding-bottom:'+paddingBottom+'">';
		}
		else
		{
			html += '<tr><td align="left" colspan='+colspan+' style="padding-top:'+buttonPadding+';padding-bottom:'+paddingBottom+'">';		
		}
		for (var y=0;y<buttonHTML.length;y++)
		{
			html += buttonHTML[y] + '<span style="padding-right:10px"></span>';
		}
		html += '</td></tr>';
		
	}
	html +="</table>";
	return html;
};

Appcelerator.UI.registerUIManager('layout', function(type,element,options,callback)
{
   Element.addClassName(element,'layout');
   Element.addClassName(element,type);
   Appcelerator.UI.loadUIComponent('layout',type,element,options,callback);
});

Appcelerator.UI.registerUIManager('behavior', function(type,element,options,callback)
{
   Appcelerator.UI.loadUIComponent('behavior',type,element,options,callback);
});



/**
 * utility function to generate a semi-random uuid
 * which is good enough as a unique id for what we normally want
 */
Appcelerator.Util.UUID =
{
    dateSeed: (Appcelerator.started || new Date).getTime(),
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
    generateNewId: function()
    {
		var dg = new Date(1970, 9, 22, 0, 0, 0, 0);
		var t = this.base16(this.dateSeed - dg.getTime());
        var a = this.base16(Math.floor(999999999999 * Math.random()));
        var _b = String(window.location.pathname).encode64();
		var b = (_b.length > 10 ? _b.substring(0,10) : _b).gsub(/[^a-zA-Z0-9]/,'0');
        var c = this.base16(Math.round(this.dateSeed * Math.random()));
        return t + '-' + a + '-' + b + '-' + c;
    }
};Object.extend(Appcelerator.Validator,
{
    toString: function ()
    {
        return '[Appcelerator.Validator]';
    },

    uniqueId: 0,
	names: [],
	
	addValidator: function(name, validator)
	{
		Appcelerator.Validator[name] = validator;
		Appcelerator.Validator.names.push(name);
	},

	URI_REGEX: /^((([hH][tT][tT][pP][sS]?|[fF][tT][pP])\:\/\/)?([\w\.\-]+(\:[\w\.\&%\$\-]+)*@)?((([^\s\(\)\<\>\\\"\.\[\]\,@;:]+)(\.[^\s\(\)\<\>\\\"\.\[\]\,@;:]+)*(\.[a-zA-Z]{2,4}))|((([01]?\d{1,2}|2[0-4]\d|25[0-5])\.){3}([01]?\d{1,2}|2[0-4]\d|25[0-5])))(\b\:(6553[0-5]|655[0-2]\d|65[0-4]\d{2}|6[0-4]\d{3}|[1-5]\d{4}|[1-9]\d{0,3}|0)\b)?((\/[^\/][\w\.\,\?\'\\\/\+&%\$#\=~_\-@]*)*[^\.\,\?\"\'\(\)\[\]!;<>{}\s\x7F-\xFF])?)$/,
    ALPHANUM_REGEX: /^[0-9a-zA-Z]+$/,
    DECIMAL_REGEX: /^[-]?([1-9]{1}[0-9]{0,}(\.[0-9]{0,2})?|0(\.[0-9]{0,2})?|\.[0-9]{1,2})$/,
 	EMAIL_REGEX: /^([a-zA-Z0-9_\-\.]+)@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.)|(([a-zA-Z0-9\-]+\.)+))([a-zA-Z]{2,4}|[0-9]{1,3})(\]?)$/,
	PHONE_REGEX: /^(?:\([2-9]\d{2}\)\ ?|[2-9]\d{2}(?:\-?|\ ?))[2-9]\d{2}[- ]?\d{4}$/,
	SSN_REGEX: /(^|\s)(00[1-9]|0[1-9]0|0[1-9][1-9]|[1-6]\d{2}|7[0-6]\d|77[0-2])(-?|[\. ])([1-9]0|0[1-9]|[1-9][1-9])\3(\d{3}[1-9]|[1-9]\d{3}|\d[1-9]\d{2}|\d{2}[1-9]\d)($|\s|[;:,!\.\?])/,

	//
	// DATE VALIDATION UTILS
	//
	dtCh:"/",
	minYear:1000,
	maxYear:3000,
	stripCharsInBag: function(s, bag)
	{
		var i;
	    var returnString = "";
	    for (i = 0; i < s.length; i++)
		{   
	        var c = s.charAt(i);
	        if (bag.indexOf(c) == -1) returnString += c;
	    }
	    return returnString;
	},
	daysInFebruary: function (year)
	{
	    return (((year % 4 == 0) && ( (!(year % 100 == 0)) || (year % 400 == 0))) ? 29 : 28 );
	},
 	DaysArray: function(n) 
	{
		for (var i = 1; i <= n; i++) 
		{
			this[i] = 31
			if (i==4 || i==6 || i==9 || i==11) {this[i] = 30}
			if (i==2) {this[i] = 29}
	   } 
	   return this;
	}
	
});

(function(){
	
	var addValidator = Appcelerator.Validator.addValidator;
	
    addValidator('required', function(value)
    {
       if (null == value)
        {
            return false;
        }
        if (typeof(value) == 'boolean')
        {
            return value;
        }
		value = ''+value;
        return value.trim().length > 0;
    });

	addValidator('email_optional', function(value)
	{
		if (!value || value.trim().length == 0) return true;
		return Appcelerator.Validator.EMAIL_REGEX.test(value);		
	});

    addValidator('email', function(value)
    {
        return Appcelerator.Validator.EMAIL_REGEX.test(value);
    });

    addValidator('zipcode_5', function(value)
    {
		return (value.length == 5 && Appcelerator.Validator.number(value)==true)?true:false
    });

    addValidator('zipcode_5_optional', function(value)
    {
	 	if (!value || value.trim().length == 0) return true;
        return Appcelerator.Validator.zipcode_5(value);
    });

    addValidator('ssn', function(value)
    {
        return Appcelerator.Validator.SSN_REGEX.test(value);
    });
    addValidator('ssn_optional', function(value)
    {
	 	if (!value || value.trim().length == 0) return true;
        return Appcelerator.Validator.ssn(value);
    });

    addValidator('phone_us', function(value)
    {
        return Appcelerator.Validator.PHONE_REGEX.test(value);
    });

    addValidator('phone_us_optional', function(value)
    {
	 	if (!value || value.trim().length == 0) return true;
        return Appcelerator.Validator.phone_us(value);
    });

    addValidator('fullname_optional', function(value)
    {
        if (!value || value.trim().length == 0) return true;
        return Appcelerator.Validator.fullname(value);
    });

    addValidator('fullname', function(value)
    {
        // allow Jeffrey George Haynie or Jeff Haynie or Jeff Smith, Jr.
        return ((value.split(" ")).length > 1);
    });

    addValidator('noSpaces_optional', function(value)
    {
       if (!value) return true;
       return Appcelerator.Validator.noSpaces(value);
    });

    addValidator('noSpaces', function(value)
    {
        // also must have a value
        // check before we check for spaces
        if (!Appcelerator.Validator.required(value))
        {
            return false;
        }
        return value.indexOf(' ') == -1;
    });
 
    addValidator('password_optional', function (value)
    {
	   if (!value || value.trim().length == 0) return true;
       return Appcelerator.Validator.password(value);
    });

    addValidator('password', function (value)
    {
        return (value.length >= 6);
    });

    addValidator('number', function (value)
    {
		if (!value || value.trim().length == 0 || value < 0)return false;
		return Appcelerator.Validator.DECIMAL_REGEX.test(value);
		
	});

    addValidator('number_optional', function (value)
    {
		if (!value || value.trim().length == 0) return true;
		return Appcelerator.Validator.number(value);
	});

    addValidator('wholenumber_optional', function (value)
    {
		if (!value || value.trim().length == 0) return true;
		return Appcelerator.Validator.wholenumber(value);
	});
		
    addValidator('wholenumber', function (value)
    {
		if (!value || value < 0) return false;
		
		for (var i = 0; i < value.length; i++)
		{   
			var c = value.charAt(i);
		    if (((c < "0") || (c > "9"))) return false;
		}
		return true;
    });

    addValidator('url_optional', function (value)
    {
		if (!value || value.trim().length == 0)return true;
        return Appcelerator.Validator.url(value);
    });

    addValidator('url', function (value)
    {
      	return Appcelerator.Validator.URI_REGEX.test(value);
    });

    addValidator('checked', function (value)
    {
        return value || value == 'on';
    });

    addValidator('length', function (value, element)
    {
        if (value)
        {
            try
            {
                var min = parseInt(element.getAttribute('validatorMinLength') || '1');
                var max = parseInt(element.getAttribute('validatorMaxLength') || '999999');
                var v = value.length;
                return v >= min && v <= max;
            }
            catch (e)
            {
            }
        }
        return false;
    });
    
    addValidator('alphanumeric_optional', function (value,element)
    {
    	if (!value || value.trim().length ==0)return true;
		return Appcelerator.Validator.ALPHANUM_REGEX.test(value)==true;
    });
	
    addValidator('alphanumeric', function (value,element)
    {
    	return Appcelerator.Validator.ALPHANUM_REGEX.test(value)==true;
    });

	addValidator('date_optional', function(value)
	{
		if (!value || value.trim().length == 0)return true;
		return Appcelerator.Validator.date(value);
		
	});
	
	addValidator('date', function(value)
	{
		
		var daysInMonth = Appcelerator.Validator.DaysArray(12);
		var pos1=value.indexOf(Appcelerator.Validator.dtCh);
		var pos2=value.indexOf(Appcelerator.Validator.dtCh,pos1+1);
		var strMonth=value.substring(0,pos1);
		var strDay=value.substring(pos1+1,pos2);
		var strYear=value.substring(pos2+1);
		strYr=strYear;
		if (strDay.charAt(0)=="0" && strDay.length>1) 
			strDay=strDay.substring(1);
		if (strMonth.charAt(0)=="0" && strMonth.length>1) 
			strMonth=strMonth.substring(1);
		for (var i = 1; i <= 3; i++) 
		{
			if (strYr.charAt(0)=="0" && strYr.length>1) strYr=strYr.substring(1);
		}
		month=parseInt(strMonth);
		day=parseInt(strDay);
		year=parseInt(strYr);
		if (pos1==-1 || pos2==-1)
		{
			return false;
		}
		if (strMonth.length<1 || month<1 || month>12)
		{
			return false;
		}
		if (strDay.length<1 || day<1 || day>31 || (month==2 && day>Appcelerator.Validator.daysInFebruary(year)) || day > daysInMonth[month])
		{
			return false;
		}
		if (strYear.length != 4 || year==0 || year<Appcelerator.Validator.minYear || year>Appcelerator.Validator.maxYear)
		{
			return false;
		}
		var numberTest = month + "/" + day + "/" + year;
		if (value.indexOf(Appcelerator.dtCh,pos2+1)!=-1 || Appcelerator.Validator.number(Appcelerator.Validator.stripCharsInBag(numberTest, Appcelerator.Validator.dtCh))==false)
		{
			return false;
		}
		return true;
	});
})();

/***************************************************************************/
/*                                                                         */
/*  These are the public widget APIs that should be accessible by a user-  */
/*  defined widget.                                                        */
/*                                                                         */
/*                                                                         */
/***************************************************************************/


/**
 * called by a widget to register itself
 *
 * @param {string} modulename
 * @param {object} module object
 * @param {boolean} dynamic
 * @since 2.1.0
 */
Appcelerator.Widget.register = function(moduleName,module,dynamic)
{
	Appcelerator.Core.registerModule(moduleName,module,dynamic);
};

/**
 * called to load a js file relative to the module js directory
 *
 * @param {string} moduleName
 * @param {object} module object
 * @param {string} js files(s)
 * @param {string} js path
 */
Appcelerator.Widget.registerWithJS = function (moduleName,module,js,jspath)
{
	Appcelerator.Core.registerModuleWithJS(moduleName,module,js,jspath);
};

/**
 * called to load a js file relative to the modules/common/js directory
 *
 * @param {string} moduleName
 * @param {object} module object
 * @param {string} js file(s)
 * @deprecated use registerWidgetWithCommonJS
 */
Appcelerator.Widget.registerModuleWithCommonJS = function(moduleName,module,js)
{
	Appcelerator.Core.registerModuleWithCommonJS(moduleName,module,js);
};

/**
 * called to load a js file relative to the modules/common/js directory
 *
 * @param {string} moduleName
 * @param {object} module object
 * @param {string} js file(s)
 * @since 2.1.0
 */
Appcelerator.Widget.registerWidgetWithCommonJS = function(moduleName,module,js)
{
	Appcelerator.Core.registerModuleWithCommonJS(moduleName,module,js);
};

/**
 * called to load a js file relative to the module js directory
 *
 * @param {string} moduleName
 * @param {object} module object
 * @param {string} js files(s)
 * @param {string} js path
 * @deprecated use registerWidgetWithJS
 */
Appcelerator.Widget.registerModuleWithJS = function (moduleName,module,js,jspath)
{
	Appcelerator.Core.registerModuleWithJS(moduleName,module,js,jspath);
};

/**
 * called to load a js file relative to the module js directory
 *
 * @param {string} moduleName
 * @param {object} module object
 * @param {string} js files(s)
 * @param {string} js path
 * @since 2.1.0
 */
Appcelerator.Widget.registerWidgetWithJS = function (moduleName,module,js,jspath)
{
	Appcelerator.Core.registerModuleWithJS(moduleName,module,js,jspath);
};

/**
 * load widget's css from widget/common
 * @param {string} moduleName
 * @param {string} css path
 */
Appcelerator.Widget.loadWidgetCommonCSS = function(moduleName,css)
{
	Appcelerator.Core.loadModuleCommonCSS (moduleName,css);
};

/**
 * dynamically load CSS from common CSS path and call onload once 
 * loaded (or immediately if already loaded)
 *
 * @param {string} name of the common file
 * @param {function} onload function to invoke
 * @since 2.1.0
 */
Appcelerator.Widget.requireCommonCSS = function(name,onload)
{
	Appcelerator.Core.requireCommonCSS(name,onload);
};

/**
 * dynamically load JS from common JS path and call onload once 
 * loaded (or immediately if already loaded)
 *
 * @param {string} name of the common js
 * @param {function} onload function to callback upon loaded
 * @since 2.1.0
 */
Appcelerator.Widget.requireCommonJS = function(name,onload)
{
	Appcelerator.Core.requireCommonJS(name,onload);
};

/**
 * dynamically laod a javascript file with dependencies
 * this will not call the callback until all resources are loaded
 * multiple calls to this method are queued
 * 
 * @param {string} path to resource
 * @param {function} the string representation of the callback
 * @since 2.1.0
 */
Appcelerator.Widget.queueRemoteLoadScriptWithDependencies = function(path, onload) 
{
	Appcelerator.Core.queueRemoteLoadScriptWithDependencies(path, onload);
};


/**
 * dynamically load a widget specific CSS
 */
Appcelerator.Widget.loadWidgetCSS = function(name,css)
{
	Appcelerator.Core.loadModuleCSS(name,css);
};

/**
 * fire a custom condition from within the widget.  
 */
Appcelerator.Widget.fireWidgetCondition = function(id, name, data)
{
    Appcelerator.Compiler.fireCustomCondition(id,name,data);
};
Appcelerator.appuid = Appcelerator.Util.Cookie.GetCookie('appuid');
Appcelerator.StatsURI = (('https:' == document.location.protocol) ? 'https://s3.amazonaws.com/tracker.appcelerator.org/' : 'http://tracker.appcelerator.org/' ) + 'app.gif';

if (!Appcelerator.appuid)
{
	Appcelerator.appuid = Appcelerator.Util.UUID.generateNewId();
	var e = new Date(new Date().getTime() + (Appcelerator.Util.DateTime.ONE_YEAR * 5));
	Appcelerator.Util.Cookie.SetCookie('appuid',Appcelerator.appuid,e,'/');
}

Appcelerator.Compiler.beforeDocumentCompile(function()
{
	Appcelerator.compileStarted = new Date;
});

Appcelerator.Compiler.afterDocumentCompile(function()
{
	var D = new Date().getTime();
	Appcelerator.compileTime = D - Appcelerator.compileStarted.getTime();
	Appcelerator.loadTime = D - Appcelerator.started.getTime(); 
});

if (window.onerror)
{
	Appcelerator._onerror = window.onerror;
}

Appcelerator.TrackStat = function(evt,extra)
{
	if (Appcelerator.Config['track_stats'])
	{
		try
		{
			var i = new Image;
			var d = new Date().getTime() - (Appcelerator.started || new Date).getTime();
			i.src = Appcelerator.StatsURI + '?t='+Number(new Date)+'&dur=' + d + '&evt=' + evt + '&appuid=' + Appcelerator.appuid + '&tid=' + Appcelerator.started.getTime() + '&' + (extra || ''); 
		}
		catch(e)
		{
		}	
	}
};

window.onerror = function(msg,url,line)
{
	try
	{
		Logger.error('generic uncaught error = '+msg+', url = '+url+', line = '+line);
		
		// track app errors to improve common issues
		var s = 'msg=' + encodeURIComponent(String(msg).encode64()) + '&url='+encodeURIComponent(String(url||'').encode64()) + '&line='+encodeURIComponent(line||-1);
		
		(function() { Appcelerator.TrackStat(2,s) }).defer();
		
		// call next guy in chain if one exists
		if (Appcelerator._onerror)
		{
			Appcelerator._onerror(msg,url,line);
		}
	}
	catch(e)
	{
		return false;
	}
};

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


Appcelerator.Core.onload(function()
{
	var sendRemote = window.location.href.indexOf('file:/')!=-1;
    var screenHeight = screen.height;
    var screenWidth = screen.width;
    var colorDepth = screen.colorDepth || -1;
	var tz = Appcelerator.started.getTimezoneOffset()/60;
    var platform = Appcelerator.Browser.isWindows ? 'win' : Appcelerator.Browser.isMac ? 'mac' : Appcelerator.Browser.isLinux ? 'linux' : Appcelerator.Browser.isSunOS ? 'sunos' : 'unknown';
    var data = 
    {
        'userAgent': navigator.userAgent,
        'flash': Appcelerator.Browser.isFlash,
        'flashver': Appcelerator.Browser.flashVersion,
		'silverlight': Appcelerator.Browser.isSilverlight,
		'silverlightver': Appcelerator.Browser.silverlightVersion,
		'gears': Appcelerator.Browser.isGears,
		'fluid': Appcelerator.Browser.isFluid,
        'screen': {
            'height':screenHeight,
            'width':screenWidth,
            'color':colorDepth
         },
        'os': platform,
        'referrer': document.referrer,
        'path': window.location.href,
        'cookies' : (document.cookie||'').split(';').collect(function(f){ var t = f.split('='); return t && t.length > 0 ? {name:t[0],value:t[1]} : {name:null,value:null}}),
        'tz' : tz,
		'uid': Appcelerator.appuid
    };
	if (sendRemote)
	{
		Appcelerator.Util.ServerConfig.addConfigListener(function()
		{
		    //
		    // if being loaded from an IFrame - don't do the report
		    //
		    if (window.parent == null || window.parent == window && Appcelerator.Browser.autoReportStats)
		    {
	            $MQ('remote:appcelerator.status.report',data);
		    }
		});
	}
	setTimeout(function()
	{
		var a = 0, s = 0, v = 1, c = null, l = null, svc = null;

		c = Appcelerator.ServerConfig['aid'];
        if (c) a = c.value;

		c = Appcelerator.ServerConfig['sid'];
        if (c) s = c.value;

		c = Appcelerator.ServerConfig['language'];
		if (c) l = c.value;

		c = Appcelerator.ServerConfig['service'];
		if (c) svc = c.value;
		
        var p = Appcelerator.Browser.isWindows ? 'win' : Appcelerator.Browser.isMac ? 'mac' : Appcelerator.Browser.isLinux ? 'linux' : Appcelerator.Browser.isSunOS ? 'sunos' : 'unknown';
		var f = Appcelerator.Browser.flashVersion;
		var sic = (Appcelerator.ServerConfig['sessionid']||{}).value;
        var si = Appcelerator.Util.Cookie.GetCookie(sic);
		var i = new Image;
		var qs = $H({
			'wv': String(Appcelerator.Version),
			'v': v,
			'a': a,
			's': s,
			'gg': Number(Appcelerator.Browser.isGears),
			'fd': Number(Appcelerator.Browser.isFluid),
			'dm': data.screen.width+','+data.screen.height+','+data.screen.color,
			'p': platform,
			'tz': tz,
			'fv': data.flashver,
			'sv': data.silverlightver,
			'r': String(document.referrer||'').encode64(),
			't': String(document.title||'').encode64(),
			'si': si,
			'sct': Appcelerator.compileTime,
			'slt': Appcelerator.loadTime,
			'bl': window.navigator.language,
			'lng': l,
			'svc': svc,
			'js': String(Appcelerator.jsFileLocation).encode64()
		}).toQueryString();
		Appcelerator.TrackStat(1,qs);
		
	},2000 + Math.round(1999*Math.random()));
	Event.observe(window,'unload',function()
	{
		Appcelerator.TrackStat(0);
	});
});

if (typeof(Appcelerator.Util)!='undefined' && typeof($$AU)=='undefined')
{
	$$AU = Appcelerator.Util;
	$$AC = Appcelerator.Compiler;
	$$AV = Appcelerator.Validator;
	$$AD = Appcelerator.Decorator;
	$$AR = Appcelerator.Core;
	$$AM = Appcelerator.Module;
	$$AL = Appcelerator.Localization;
	$$AF = Appcelerator.Config;
	$$AB = Appcelerator.Browser;
}