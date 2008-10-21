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
// Implements the behavior of our custom buttons. There is no native support in
// IE for rounded corners on HTML elements, so this script also implements a
// hack to simulate them in that browser.
//
// Requires: base.js, dom.js
//=============================================================================

/**
 * Constructor.
 */
function CustomButton(htmlButton) {
  bindMethods(this);
  
  this._focused = false;
  this._htmlButton = htmlButton;

  if (!document.all) {  // not IE
    dom.addEvent(htmlButton, "mouseover", this._handleMouseOver);
    dom.addEvent(htmlButton, "mouseout", this._handleMouseOut);
  } else {  // IE
    // mouseover/mouseout behave badly with nested elements in IE. Luckily,
    // mouseenter/mouseleave are provided and have the equivalent semantics.
    dom.addEvent(htmlButton, "mouseenter", this._handleMouseOver);
    dom.addEvent(htmlButton, "mouseleave", this._handleMouseOut);

    // We also handle focus and blur in IE so that we can update the corners
    // to match the black border IE provides.
    dom.addEvent(htmlButton, "focusin", this._handleFocus);
    dom.addEvent(htmlButton, "blur", this._handleBlur);

    dom.addClass(htmlButton, "ie");
    this._createCorners();
  }

  // Limit button widths to a minimum of 60px. They look funny skinnier.  
  if (htmlButton.offsetWidth < 60) {
    htmlButton.style.width = "60px";
  }

  htmlButton.style.visibility = "visible";
}

/**
 * Initializes all the custom buttons on the page.
 */
CustomButton.initializeAll = function() {
  var buttons = document.getElementsByTagName("button");
  for (var i = 0, button; button = buttons[i]; i++) {
    if (dom.hasClass(button, "custom")) {
      new CustomButton(button);
    }
  }
};

/**
 * Highlights the button on mouseover. Note that this gets called multiple
 * times while the mouse goes over sub elements inside the button.
 */
CustomButton.prototype._handleMouseOver = function() {
  if (!this._htmlButton.disabled && !this._focused) {
    dom.addClass(this._htmlButton, "hover");
    this._setCornerColor("blue");
  }
};

/**
 * Undoes the highlighting of the button on mouseout. Note that this gets called
 * multiple times when the mouse moves out of sub elements, even if it is still
 * over the button. That is OK, as long as there is immediately a mouseover
 * event.
 */
CustomButton.prototype._handleMouseOut = function() {
  if (!this._htmlButton.disabled && !this._focused) {
    dom.removeClass(this._htmlButton, "hover");
    this._setCornerColor("grey");
  }
};

/**
 * Highlights the button on focus. Currently only used for IE.
 * TODO(aa): The black highlight looks cool. Consider for other browsers.
 */
CustomButton.prototype._handleFocus = function() {
  dom.removeClass(this._htmlButton, "hover");
  this._setCornerColor("black");
  this._focused = true;
};

/**
 * Undoes the highlighting of the button on blur.
 */
CustomButton.prototype._handleBlur = function() {
  this._setCornerColor("grey");
  this._focused = false;
};

/**
 * Helper to create the graphics that make the button have rounded corners in
 * IE.
 */
CustomButton.prototype._createCorners = function() {
  // Making the button position:relative makes it possible to position things
  // inside it relative to it's border.
  this._htmlButton.style.position = "relative";

  var verticalEdges = ["left", "right"];
  var horizontalEdges = ["top", "bottom"];
  var colors = ["grey", "blue", "black"];

  for (var i = 0; i < verticalEdges.length; i++) {
    for (var j = 0; j < horizontalEdges.length; j++) {
      for (var k = 0; k < colors.length; k++) {
        var img = document.createElement("img");
        img.src = "button_corner_" + colors[k]  + ".gif";
        img.color  = colors[k];
        img.style.position = "absolute";
        img.style[verticalEdges[i]] = "-2px";
        img.style[horizontalEdges[j]] = "-2px";
        img.style.filter =
            "progid:DXImageTransform.Microsoft.BasicImage(Rotation=" + 
            (i + j) + ")";
        this._htmlButton.appendChild(img);
      }
    }
  }

  this._setCornerColor("grey");
};

/**
 * Sets the color of the corners in IE by changing the stack order of the corner
 * images.
 */
CustomButton.prototype._setCornerColor = function(newColor) {
  var imgs = this._htmlButton.getElementsByTagName("img");
  for (var i = 0, img; img = imgs[i]; i++) {
    img.style.zIndex = Number(img.color == newColor);
  }
};
