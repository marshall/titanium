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

// The 'name' of the managed store used by this demo
var STORE_NAME = "helloworld-managedstore";

function canDemo() {
  if (!window.google || !google.gears) {
    setError('Gears is not installed');
    return false;
  } else if (location.protocol.indexOf("http") != 0) {
    setError('This sample must be hosted on an HTTP server');
    return false;
  } else {
    return true;
  }
}

// The remaining functions are unrelated to Gears APIs

function httpGet(url) {
  if (!checkProtocol()) {
    return 'This sample must be hosted on an HTTP server';
  }
  if (window.XMLHttpRequest) {
    xmlhttp = new XMLHttpRequest();
  } else if (window.ActiveXObject) {
    xmlhttp = new ActiveXObject('Msxml2.XMLHTTP');
  }
  xmlhttp.open('GET', url, false);
  xmlhttp.send(null);
  return xmlhttp.responseText;
}

function showSourceFileInline(url, divId) {
  var source = httpGet(url);
  var html = '<pre>' + source + '</pre>';
  var div = getElementById(divId);
  div.innerHTML = html;
}

function textOut(s) {
  var elm = getElementById('textOut');
  setTextContent(elm, s);
}
