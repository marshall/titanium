/*
Copyright 2008, Google Inc.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, 
    this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
 3. Neither the name of Google Inc. nor the names of its contributors may be
    used to endorse or promote products derived from this software without
    specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// List of available tools (used to construct header links)
// Format: [<tool friendly name>, <url relative to /inspector/>]
var tools = [
  ['Console', 'console.html'],
  ['Database', 'database.html'],
  ['LocalServer', 'localserver.html']
  //['WorkerPool', 'workerpool.html']
];

// Initialise tools menu
(function () {
  // Determine current page
  var current_page = location.href;
  current_page = current_page.substring(current_page.lastIndexOf('/') + 1,
                                        current_page.length);
  // Construct menu
  var links = document.createElement('div');
  links.id = 'inspector-links';
 
  // IE fix: absolutely positioned elements in IE are shrink-wrapped,
  // causing the links bar to be squished to the left of the window.
  // Floating an arbitrary element to the right fixes this behavior.
  var right_div = document.createElement('div');
  right_div.innerHTML = '&nbsp;';
  right_div.style.styleFloat = 'right';  // IE way of accessing float
  right_div.style.cssFloat = 'right';    // W3C way of accessing float
                                         // Set them both just to be safe
  links.appendChild(right_div);
  
  for (var i = 0; i < tools.length; i++) {
    var link;
    if (current_page == tools[i][1]) {
      link = document.createElement('span');
    } else {
      link = document.createElement('a');
      link.href = tools[i][1];
    }
    link.innerHTML = tools[i][0];
    links.appendChild(link);
  }
  document.body.insertBefore(links, document.body.firstChild);
})();
