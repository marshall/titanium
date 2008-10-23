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

// Some WinCE devices use 'unknown' rather than 'undefined'. If an element has
// type 'unknown', we can not pass it to a function, so we must pass its type
// instead.
function isDefined(type) {
  return (type != 'undefined' && type != 'unknown');
}

function childNodes(element) {
  if (isDefined(typeof element.childNodes)) {
    return element.childNodes;
  } else if (isDefined(typeof element.children)) {
    return element.children;
  }
}

function getElementById(element_name) {
  if (isDefined(typeof document.getElementById)) {
    return document.getElementById(element_name);
  } else if(typeof isDefined(document.all)) {
    return document.all[element_name];
  }
}

function setTextContent(elem, content) {
  if (isDefined(typeof elem.innerText)) {
    elem.innerText = content; 
  } else if (isDefined(typeof elem.textContent)) {
    elem.textContent = content;
  }
}

function setupSample() {
  // Make sure we have Gears. If not, tell the user.
  if (!window.google || !google.gears) {
    if (confirm("This demo requires Gears to be installed. Install now?")) {
      var spliceStart = location.href.indexOf("/samples");
      location.href =
        location.href.substring(0, spliceStart) + "/install.html";
      return;
    }
  }

  var viewSourceElem = getElementById("view-source");
  if (!viewSourceElem) {
    return;
  }
  var elm;
  if (navigator.product == "Gecko") {
    // If we're gecko, we can show the source of the application with the
    // view-source protocol.
    elm = "<a href='view-source:" + location.href + "'>" +
          "View Demo Source" +
          "</a>";
  } else {
    // Otherwise, just tell users how to do it manually.
    elm = "<em>" +
          "To see how this works, use the <strong>view " +
          "source</strong> feature of your browser" +
          "</em>";
  }
  viewSourceElem.innerHTML += elm;
}

function checkProtocol() {
  if (location.protocol.indexOf('http') != 0) {
    setError('This sample must be hosted on an HTTP server');
    return false;
  } else {
    return true;
  }
}

function addStatus(message, opt_class) {
  var elm = getElementById('status');
  var id = 'statusEntry' + (childNodes(elm).length + 1);
  if (!elm) return;
  if (opt_class) {
    elm.innerHTML += '<span id="' + id + '" class="' + opt_class + '"></span>';
  } else {
    elm.innerHTML += '<span id="' + id + '"></span>';
  }
  elm.innerHTML += '<br>';
  setTextContent(getElementById(id), message);
}

function clearStatus() {
  var elm = getElementById('status');
  elm.innerHTML = '';
}

function setError(s) {
  clearStatus();
  addStatus(s, 'error');
}

setupSample();
