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

/**
 * Cross-platform implementations of various methods needed in order to ensure
 * compatibility with target platforms.
 */

/**
 * Retrieve a DOM element by its ID using the best mechanism available.
 * @param nodeid the ID of the node to locate.
 */
function getDOMElementById(nodeid) {
  var node;
  if (document.getElementById) {
     node = document.getElementById(nodeid);
  } else if (document.all) {
     node = document.all[nodeid];
  } else {
    throw new Error("Finding dom nodes not supported; cannot run tests.");
  }
  if (!node || node == null) {
    throw new Error("Unable to locate node with id " + nodeid);
  }
  return node;
}

/**
 * Set the text contents of a node.
 * @param nodeid the ID of the node to update
 * @param content the content to set as the text content of the node
 */
function setDOMNodeText(nodeid, content) {
  var node = getDOMElementById(nodeid);
  if (typeof node.innerText != "undefined") {
    node.innerText = content;
  } else {
    node.textContent = content;
  }
}

/**
 * Get the DOM Frame object for a given frame ID in the current window
 * @param frameid the frame ID to locate in the current window
 */
function getDOMFrameById(frameid) {
  var frame = window.frames[frameid];
  if (!frame) {
    throw new Error("Unable to load frame with id of " + frameid);
  }
  return frame;
}

/**
 * Set the CSS class on a DOM node by id.
 * @param nodeid the id of the node to update
 * @param className the css class name to set on the node
 */
function setDOMNodeClass(nodeid, className) {
  var node = getDOMElementById(nodeid);
  node.className = className;
}

/**
 * Append an anchor tag to the contents of a node.  We use innerHTML because
 * we care about compatibility with Windows Mobile and other browsers of
 * similar levels of virility.
 *
 * This will add spacing before and after the anchor tag.
 *
 * @nodeid the node to append an anchor tag to
 * @href the URL that the anchor should link to
 * @target the target of the anchor.
 * @text the contents of the anchor tag
 */
function appendAnchor(nodeid, href, target, text) {
  var node = getDOMElementById(nodeid);
  var anchor = '&nbsp; <a href=\"' + href + '\" target=\"' + target + '\">' +
      text + '</a> ';
  node.innerHTML = node.innerHTML + anchor;
}

/**
 * Create an element under the specified node id with the specified element
 * type, id, and text contents.
 * @nodeid the node to append a new node to
 * @type the type of the child node to create and append
 * @id the id to set on the new child node
 * @text the contents of the new child node.
 */
function appendSimpleElement(nodeid, type, id, text) {
  var node = getDOMElementById(nodeid);
  var element = '<' + type;
  if (id != null) {
    element += ' id=\"' + id + '\"';
  }
  element += '>' + text + '</' + type + '> ';
  node.innerHTML = node.innerHTML + element;
}

/**
 * Set a window's scroll position to the bottom of the window.
 */
function scrollToBottom() {
  if (window.scrollHeight) {
    window.scrollTo(0, window.scrollHeight);
  } else {
    window.scrollTo(0, 200000);
  }
}
