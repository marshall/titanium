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
 * Initialize the base functionality of the dialog.
 */
function initDialog() {
  if (!browser.ie_mobile) {
    dom.addEvent(document, "keyup", handleKeyUp);
  } else {
    var buttonRowElem = null;
    if (window.pie_dialog.IsSmartPhone()) {
      buttonRowElem = dom.getElementById("button-row-smartphone");
    } else {
      buttonRowElem = dom.getElementById("button-row");
    }
    if (buttonRowElem) {
      buttonRowElem.style.display = 'block';
    }
  }
  if (browser.ie_mobile) {
    window.pie_dialog.SetScriptContext(window);
    window.pie_dialog.ResizeDialog();
  }
}

/**
 * Set the label of an input button, and optionally, its accesskey.
 */
function setButtonLabel(textID, elemID, accessKeyID) {
  var textElem = dom.getElementById(textID);
  var buttonElem = dom.getElementById(elemID);
  if (textElem == null || buttonElem == null) {
    return;
  }

  var accessKeyElem = null;
  if (isDefined(typeof accessKeyID)) {
    accessKeyElem = dom.getElementById(accessKeyID);
  }

  // This function works for two different kinds of buttons. Simple buttons
  // based on the <input type="button"> tag, and custom buttons based on a
  // <button> tag with the css class "custom".
  // Note that on Windows Mobile 5, the tagName property is not defined.
  // We can therefore safely assume that the converse is also true:
  // if tagName is not defined, then the platform must be Windows Mobile 5.  
  if (!isDefined(typeof buttonElem.tagName) ||
      buttonElem.tagName.toLowerCase() == "input") {
    buttonElem.value = dom.getTextContent(textElem).trim();
    if (accessKeyElem != null) {
      buttonElem.accessKey = dom.getTextContent(accessKeyElem).trim();
    }
  } else if (buttonElem.tagName.toLowerCase() == "button") {
    var text = dom.getTextContent(textElem).trim();

    if (accessKeyElem != null) {
      // Some browsers use the accessKey attribute of the the anchor tag.
      var accessKey = dom.getTextContent(accessKeyElem).trim();
      buttonElem.accessKey = accessKey;

      // Find the first matching character in the text and mark it.
      // Note: this form of String.replace() only replaces the first occurence.
      text = text.replace(accessKey,
                          "<span class='accesskey'>" + accessKey + "</span>");
    }

    buttonElem.innerHTML = text;
  } else {
    throw new Error("Unexpected button tag name: " + buttonElem.tagName);
  }
}

/**
 * Allows a dialog to do custom layout when the window changes sizes. The
 * provided function will be called with the height of the content area when the
 * dialog should relayout.
 */
function initCustomLayout(layoutFunction) {
  function doLayout() {
    layoutFunction(getContentHeight());
  }

  doLayout();

  // We do an additional layout in onload because sometimes things aren't
  // stabilized when the first doLayout() is called above.
  dom.addEvent(window, "load", doLayout);
  dom.addEvent(window, "resize", doLayout);

  // Mozilla doesn't fire continuous events during resize, so if we want to get
  // somewhat smooth resizing, we need to run our own timer loop. This still
  // doesn't look perfect, because the timer goes off out of sync with the
  // browser's internal reflow, but I think it's better than nothing.
  // TODO(aa): Keep looking for a way to get an event for each reflow, like IE.
  if (browser.mozilla) {
    var lastHeight = -1;

    function maybeDoLayout() {
      var currentHeight = dom.getWindowInnerHeight();
      if (currentHeight != lastHeight) {
        lastHeight = currentHeight;
        doLayout();
      }
    }

    window.setInterval(maybeDoLayout, 30);
  }
}

/**
 * Get the JSON-formatted arguments that were passed by the caller.
 */
function getArguments() {
  var argsString;
  if (browser.ie_mobile) {
    argsString = window.pie_dialog.GetDialogArguments();
  } else if (browser.ie) {
    argsString = window.external.GetDialogArguments();
  } else if (browser.mozilla) {
    argsString = getFirefoxArguments(window.arguments[0]);
  } else if (browser.safari) {
    argsString = window.gears_dialogArguments;
  } else if (browser.android) {
    argsString = "" + window.bridge.getDialogArguments();
  } else if (browser.chrome) {
    argsString = chrome.dialogArguments;
  }

  if (typeof argsString == "string") {
    return JSON.parse(argsString);
  } else {
    return null;
  }
}

/**
 * Helper used by getArguments(). Getting the arguments in the right type is
 * more involved in Firefox.
 */
function getFirefoxArguments(windowArguments) {
  var Ci = Components.interfaces;
  windowArguments.QueryInterface(Ci.nsIProperties);
  var supportsString = 
    windowArguments.get("dialogArguments", Ci.nsISupportsString);
  return supportsString.data;
}

/**
 * Close the dialog, sending the specified result back to the caller.
 */
function saveAndClose(resultObject) {
  var resultString = JSON.stringify(resultObject);
  if (browser.ie_mobile) {
    window.pie_dialog.CloseDialog(resultString);
  } else if (browser.ie) {
    window.external.CloseDialog(resultString);
  } else if (browser.mozilla) {
    saveFirefoxResults(resultString);
    window.close();
  } else if (browser.safari) {
    window.gears_dialog.setResults(resultString);
  } else if (browser.android) {
    window.bridge.closeDialog(resultString);
  } else if (browser.chrome) {
    saveChromeResults(resultString);
    window.close();
  }
}

function saveChromeResults(resultString) {
  chrome.send("DialogClose", [resultString]);
}

/**
 * Helper used by endDialog() for Firefox.
 */
function saveFirefoxResults(resultString) {
  var Cc = Components.classes;
  var Ci = Components.interfaces;
  
  var props = window.arguments[0].QueryInterface(Ci.nsIProperties);
  var supportsString = Cc["@mozilla.org/supports-string;1"]
      .createInstance(Ci.nsISupportsString);
      
  supportsString.data = resultString;
  props.set("dialogResult", supportsString);
}

/**
 * Returns the height of the content area of the dialog.
 */
function getContentHeight() {
  var head = dom.getElementById("head");
  var foot = dom.getElementById("foot");
  return dom.getWindowInnerHeight() - head.offsetHeight - foot.offsetHeight;
}

/**
 * For some reason ESC doesn't work on HTML dialogs in either Firefox or IE. So
 * we implement it manually.
 */
function handleKeyUp(e) {
  var ESC_KEY_CODE = 27;
  
  if (e.keyCode == ESC_KEY_CODE) {
    saveAndClose(null);
  }
}

/**
 * Disables a button in the right way, whether it is normal or custom.
 */
function disableButton(buttonElem) {
  buttonElem.disabled = true;

  if (browser.ie_mobile) {
    window.pie_dialog.SetButtonEnabled(false);
  }

  if (!isDefined(typeof buttonElem.tagName) || 
      buttonElem.tagName.toLowerCase() == "input") {
    buttonElem.style.color = "gray";
  } else if (buttonElem.tagName.toLowerCase() == "button") {
    dom.addClass(buttonElem, "disabled");
  } else {
    throw new Error("Unexpected tag name: " + buttonElem.tagName);
  }
}

/**
 * Enables a button in the right way, whether it is normal or custom.
 */
function enableButton(buttonElem) {
  buttonElem.disabled = false;

  if (browser.ie_mobile) {
    window.pie_dialog.SetButtonEnabled(true);
  }
  
  if (!isDefined(typeof buttonElem.tagName) ||
      buttonElem.tagName.toLowerCase() == "input") {
    buttonElem.style.color = "black";
  } else if (buttonElem.tagName.toLowerCase() == "button") {
    dom.removeClass(buttonElem, "disabled");
  } else {
    throw new Error("Unexpected tag name: " + buttonElem.tagName);
  }
}

/**
 * Returns a wrapped domain (useful for small screens dialogs, 
 * e.g. windows mobile devices)
 */
function wrapDomain(str) {
  if (!browser.ie_mobile) {
    return str;
  }

  // Replace occurences of '.' with an image representing a dot. This allows the
  // browser to wrap the URL at these points as if there were whitespace
  // present.
  var dotImage = "<img height='2px' width='2px' " +
                 "style='background-color: black; margin: 0px 1px;'>";
  return str.replace(/[.]/g, dotImage);
}

/**
 * Resizes the dialog to fit the content size.
 */
function resizeDialogToFitContent(opt_minDialogInnerHeight) {
  // This does not work on PIE (no height measurement)
  if (browser.ie_mobile) {
    window.pie_dialog.ResizeDialog();
    return;
  }
  // window.resize() is not working on Android, we bail out.
  if (browser.android) {
    return;
  }

  // Resize the window to fit
  var contentDiv = dom.getElementById("content");
  var contentHeightProvided = getContentHeight();
  var contentHeightDesired = contentDiv.offsetHeight;

  var dialogHeightProvided = dom.getWindowInnerHeight();
  var dialogHeightDesired =
      dialogHeightProvided + (contentHeightDesired - contentHeightProvided);

  var minDialogHeight = opt_minDialogInnerHeight || 0;
  var maxDialogHeight = 400; // arbitrary max height for a dialog to resize to
  
  var targetHeight = Math.max(dialogHeightDesired, minDialogHeight);
  targetHeight = Math.min(dialogHeightDesired, maxDialogHeight);

  if (targetHeight != dialogHeightProvided) {
    var dy = targetHeight - dialogHeightProvided;
    window.resizeBy(0, dy);
  }
}

/**
 * Safari running on OSX 10.4 can't load images dynamically from the network,
 * this helper function calls gears-specific code to work around this
 * limitation.
 */
function loadImage(elem, image_url) {
  if (browser.safari) {
    var position = dom.getPosition(elem);
    window.gears_dialog.loadImageIntoElement(image_url, position.top,
                                             position.left,
                                             position.width,
                                             position.height);
  } else {
    elem.src = image_url;
  }
}
