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

// Functions for putting the labels of input boxes in the boxes themselves.

function setupLabels(fields) {
  for (var i = 0, elm; elm = fields[i]; i++) {
    if (elm.type == "text" || elm.type == "password") {
      if (elm.value == "") {
        elm._type = elm.type;
        setDynamicLabel(elm);
      }
      
      if (!DOM.is_pocket_ie) {
        DOM.listen(elm, "focus", focusDynamicLabel);
        DOM.listen(elm, "blur", blurDynamicLabel);
      }
    }
  }
}

function resetLabels(fields) {
  for (var i = 0, elm; elm = fields[i]; i++) {
    if (elm.type == "text" || elm.type == "password") {
      if (elm.value == elm.getAttribute("label")) {
        elm.value = "";
      }
    }
  }
}

function focusDynamicLabel(event) {
  var elm = getEventSrc(event);
  if (elm.value == elm.getAttribute("label")) {
    if (elm._type == "password") {
      elm = setInputType(elm, "password", true);
    }
    elm.value = "";
  }
}

function blurDynamicLabel(event) {
  // wierd... sometimes, it seems like the blur gets fired after unload, so 
  // this function is gone.
  if (typeof window.getEventSrc == "undefined") {
    return;
  }

  var elm = getEventSrc(event);
  setDynamicLabel(elm);
}

function setDynamicLabel(elm) {
  if ("" == elm.value) {
    if (elm.type == "password") {
      elm = setInputType(elm, "text", false);
    }

    elm.value = elm.getAttribute("label");
  }
}

function getEventSrc(e) {
  if (!e) e = window.event;

  if (e.originalTarget)
    return e.originalTarget;
  else if (e.srcElement)
    return e.srcElement;
}

function setInputType(el, type, focus) {
  if (DOM.is_pocket_ie) {
    // I could not get Pocket IE to respond to focus/blur events,
    // so this manipulation is not necessary for that platform
    return el;
  }
  if (DOM.is_ie) {
    var span = document.createElement("SPAN");
    span.innerHTML = 
      '<input id="' + el.id + '" type="' + type + '" class="' + 
      el.className + '" label="' + el.getAttribute("label") + '">';

    var newEl = span.firstChild;
    el.parentNode.replaceChild(newEl, el);

    newEl._type = el._type;

    if (focus) {
      window.setTimeout(function() { newEl.focus(); }, 0);
    }

    DOM.listen(newEl, "focus", focusDynamicLabel);
    DOM.listen(newEl, "blur", blurDynamicLabel);

    return newEl;
  } else {
    el.type = type;
    return el;
  }
}
