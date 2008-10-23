m4_changequote(`~',`~')m4_dnl
<!DOCTYPE html>

<!--
Copyright 2007, Google Inc.

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
-->

<html>
<head>
  <style type="text/css">

m4_ifelse(PRODUCT_OS,~android~,~~,~m4_dnl
m4_include(ui/common/button.css)~)
m4_include(ui/common/html_dialog.css)
    h1 {
      font-size:1.2em;
      margin:0;
    }

    h2 {
      font-size:1.1em;
      margin:0;
    }

    /* TODO(aa): Move into standard stylesheet if used in all dialogs? */
    #icon {
      margin-right:0.5em;
    }

    #content {
      overflow-x:hidden;
      overflow-y:auto;
m4_ifelse(PRODUCT_OS,~wince~,m4_dnl
~
      margin:0 4px;
~,
~
      margin:0 1em;
~)
    }

    #content table {
      border: 1px #ccc;
      /* Only set the sides and bottom border of the table because
      border-colapse doesn't work on WinCE. */
      border-style: none solid;
      border-collapse: collapse;
      /* A bug in Mozilla with border-collapse:collapse along with the overflow
      settings we have on the scroll element causes the left border to be
      placed 1px to the left, making the element invisible. */
      margin-left:1px;
    }

    #content td {
m4_ifelse(PRODUCT_OS,~wince~,m4_dnl
~
      padding: 2px;
~,
~
      padding: 4px; 
~)
    }

    #content table.permissions {
      border-bottom-style: solid;
    }

    #version {
      font-size:0.8em;
      font-style:italic;
    }

    #content td.left {
      width: 100%;
m4_ifelse(PRODUCT_OS,~wince~,m4_dnl
~~,~
      padding-left: 20px;
~)
    }

    #content td.right {
m4_ifelse(PRODUCT_OS,~wince~,m4_dnl
~
      width: 120px;
~,~
      padding-right: 20px;
~)
    }

    .app-icon {
      width: 30px;
    }

    .app-name {
      width: 150px;
    }

    .app-origin {
      width: 70px;
    }

m4_ifelse(PRODUCT_OS,~wince~,m4_dnl
~
    /* 
     * On Windows Mobile, we hide the div containing the buttons
     * by default, to only show the correct one (ie smartphone or not)
     */
    #button-row {
      display:none;
    }
~,~~)

    td.origin-name {
      font-weight: bold;
      width: 100%;
    }

m4_ifelse(PRODUCT_OS,~wince~,m4_dnl
~~,~
    div.radio-buttons {
m4_ifelse(PRODUCT_OS,~android~,m4_dnl
~
      width: 150px;
~,~
      width: 130px;
~)
    }
~)

m4_ifelse(PRODUCT_OS,~wince~,m4_dnl
~
    body, td {
      font-size: 10px;
    }
~,~~)

  </style>
</head>
<body>
  <div id="strings" style="display:none;">
    <div id="string-cancel"></div>
    <div id="string-cancel-accesskey"></div>
    <div id="string-apply"></div>
    <div id="string-apply-accesskey"></div>
    <div id="string-nosites"></div>
    <div id="string-allowed"></div>
    <div id="string-denied"></div>
    <div id="string-local-storage"></div>
    <div id="string-location-data"></div>
    <div id="string-remove"></div>
  </div>
  
  <div id="head">
    <table width="100%" cellpadding="0" cellspacing="0" border="0">
      <tr>
        <td valign="middle">
m4_ifelse(PRODUCT_OS,~android~,m4_dnl
~
          <img id="icon" src="data:image/png;base64,
m4_include(genfiles/icon_32x32.png.base64)m4_dnl
           ">
~,~
          <img id="icon" src="icon_32x32.png" width="32" height="32">
~)
          <!-- Some browsers automatically focus the first focusable item. We
          don't want anything focused, so we add this fake item. -->
          <a href="#" id="focus-thief"></a>
        </td>
        <td width="100%" valign="middle">
          <h1>
          <span id="string-gears-settings"></span>
          </h1>
        </td>
      </tr>
    </table>
  </div>
  <div id="content">
    <p>
      <span id="string-permissions-description"></span>
    </p>
    <div id='div-permissions'>
    </div>
    <br>
    <br>
    <div id="string-version"></div>
  </div>
  <div id="foot">
    <div id="button-row">
      <table width="100%" cellpadding="0" cellspacing="0" border="0">
        <tr>
m4_ifelse(PRODUCT_OS,~wince~,m4_dnl
~
          <div id="div-buttons">
            <td width="50%" valign="middle">
              <input type="BUTTON" id="cancel-button"
               onclick="saveAndClose(null); return false;"></input>
            </td>
            <td width="50%" align="right" valign="middle">
              <input type="BUTTON" id="confirm-button"
               onclick="saveAndClose(calculateDiff()); return false;"></input>
            </td>
          </div>
~,m4_dnl
~
          <td nowrap="true" align="right" valign="middle">
            <button id="confirm-button" class="custom"
                onclick="saveAndClose(calculateDiff()); return false;"
              ></button
            ><button id="cancel-button" accesskey="C" class="custom"
                onclick="saveAndClose(null); return false;"
              ></button>
          </td>
~)
        </tr>
      </table>
    </div>
  </div>


</body>
m4_ifelse(PRODUCT_OS,~wince~,m4_dnl
~<object style="display:none;" classid="clsid:134AB400-1A81-4fc8-85DD-29CD51E9D6DE" id="pie_dialog">~m4_dnl
~</object>~)
<script>
m4_include(../third_party/jsonjs/json_noeval.js)
m4_include(ui/common/base.js)
m4_include(ui/common/dom.js)
m4_include(ui/common/html_dialog.js)
m4_include(ui/common/button.js)
m4_include(genfiles/settings_dialog.js)
</script>
<script>
  var debug = false;
  var permissions;
  var originalPermissions;

  // Must match values in permission_db.h
  var PERMISSION_NOT_SET = 0;
  var PERMISSION_ALLOWED = 1;
  var PERMISSION_DENIED  = 2;

  if (debug && browser.ie_mobile) {
    // Handy for debugging layout.
    // Remember to remove the pie_dialog object tag above when debugging on
    // WinMo 5, otherwise the assignement below will fail with "the object does
    // not support this property or method".
    pie_dialog = new Object();
    pie_dialog.IsSmartPhone = function() { return false; };
    pie_dialog.SetCancelButton = function() {};
    pie_dialog.SetButton = function() {};
    pie_dialog.SetButtonEnabled = function() {};
    pie_dialog.SetScriptContext = function() {};
    pie_dialog.ResizeDialog = function() {};
  }

  initDialog();

  initSettings();

  setButtonLabel("string-cancel", "cancel-button", "string-cancel-accesskey");
  setButtonLabel("string-apply", "confirm-button", "string-apply-accesskey");

  if (!browser.ie_mobile) {
    CustomButton.initializeAll();
    if (!browser.android) {
      initCustomLayout(layoutSettings);
    }
  } else {
    var applyText = dom.getElementById("string-apply");
    if (applyText) {
      window.pie_dialog.SetButton(applyText.innerText, 
        "saveAndClose(calculateDiff());");
    }
    var cancelText = dom.getElementById("string-cancel");
    if (cancelText) {
      window.pie_dialog.SetCancelButton(cancelText.innerText);
    }
    window.pie_dialog.SetButtonEnabled(true);
  }

  // Start out with only cancel enabled, just for clarity.
  disableButton(dom.getElementById("confirm-button"));

  function initSettings() {
    var args;

    if (debug) {
      // Handy for debugging layout:
      var args = {
        locale: "en-US",
        permissions: [
          {
            name: "http://www.google.com",
            localStorage: { permissionState: 1 },
            locationData: { permissionState: 0 }
          },
          { 
            name: "http://www.aaronboodman.com",
            localStorage: { permissionState: 1 },
            locationData: { permissionState: 1 }
          },
          { 
            name: "http://www.evil.org",
            localStorage: { permissionState: 2 },
            locationData: { permissionState: 2 }
          }
        ]
      };
    } else {
      args = getArguments();
    }
    loadI18nStrings(args.locale);
    permissions = args.permissions;
    originalPermissions = cloneObject(permissions);

    updatePermissionsList();
  }

  function updatePermissionsList() {
    var table = dom.getElementById('div-permissions');

    var content = "";
    var isFirstOrigin = true;
    for (var index = 0; index < permissions.length; index++) {
      content += renderOrigin(index, permissions[index], isFirstOrigin);
      isFirstOrigin = false;
    }
    if (content == "") {
      content = "<table style=\"border-style: solid;\"><tbody>";
      content += "<tr><td class=\"left\"><em>";
      var noSitesText = dom.getElementById("string-nosites");
      if (noSitesText) {
        if (isDefined(typeof noSitesText.innerText)) {
          content += noSitesText.innerText;
        } else {
          content += noSitesText.textContent;
        }
      }
      content += "</em></td><td></td></tr></tbody></table>";
    } 
    table.innerHTML = content;
  }

  function renderOrigin(index, originData, isFirstOrigin) {
    var localStorageContent =
        renderLocalStorage(index, originData.localStorage);
    var locationDataContent =
        renderLocationData(index, originData.locationData);

    // Defend against an origin having no data for any permission class.
    if (localStorageContent == "" && locationDataContent == "") {
      return "";
    }
    var content = "<table";
    if (isFirstOrigin) {
      content += " style=\"border-top-style: solid;\"";
    }
    content += "><tbody>";
    content += "<tr><td class=\"origin-name\">";
    content += wrapDomain(originData.name);
    content += "</td><td><a href='#' onclick='handleRemoveClick(";
    content += index;
    content += ");'>";
    content += getString("string-remove");
    content += "</a></td>";
    content += "</td></tr></tbody></table>";

    content += "<table class=\"permissions\"><tbody>";
    if (localStorageContent != "") {
      content += "<tr>";
      content += localStorageContent;
      content += "</tr>";
    }
    if (locationDataContent != "") {
      content += "<tr>";
      content += locationDataContent;
      content += "</tr>";
    }
    content += "</td></tr></tbody></table>";
    return content;
  }

  function renderLocalStorage(index, data) {
    if (data == null || data.permissionState == PERMISSION_NOT_SET) {
      return "";
    }
    var content = "";
    content += "<td class=\"left\">";
    content += getString("string-local-storage");
    content += "</td><td class=\"right\">";
    content += renderRadioButtons(index, "localStorage",
                                  data.permissionState);
    content += "</td>";
    // Permission class-specific content goes here;
    return content;
  }

  function renderLocationData(index, data) {
    if (data == null || data.permissionState == PERMISSION_NOT_SET) {
      return "";
    }
    var content = "";
    content += "<td class=\"left\">";
    content += getString("string-location-data");
    content += "</td><td class=\"right\">";
    content += renderRadioButtons(index, "locationData",
                                  data.permissionState);
    content += "</td>";
    // Permission class-specific content goes here;
    return content;
  }

  function getString(stringId) {
    var textElement = dom.getElementById(stringId);
    if (!isDefined(typeof textElement)) {
      return "";
    }
    return dom.getTextContent(textElement);
  }

  function renderRadioButtons(index, permissionClass, permissionState) {
    var radioButtonName = getRadioButtonName(index, permissionClass);
    var content = "<div class=\"radio-buttons\">";
    content += "<input type=\"radio\" name=\"" + radioButtonName + "\"";
    content += " id=\"" + radioButtonName + "-allow\"";

    if (permissionState == PERMISSION_ALLOWED) {
      content += "checked=\"true\"";
    }
    content += " onclick='handleRadioClick(" + index + ", \"" +
               permissionClass + "\", " + PERMISSION_ALLOWED + ");'>";
    content += "</input>";
    content += "<label for=\"" + radioButtonName + "-allow\">";
    content += getString("string-allowed");
    content += "</label>";

    content += "<input type=\"radio\" name=\"" + radioButtonName + "\"";
    content += " id=\"" + radioButtonName + "-deny\"";

    if (permissionState == PERMISSION_DENIED) {
      content += "checked=\"true\"";
    }
    content += " onclick='handleRadioClick(" + index + ", \"" +
               permissionClass + "\", " + PERMISSION_DENIED + ");'>";
    content += "</input>";
    content += "<label for=\"" + radioButtonName + "-deny\">";
    content += getString("string-denied");
    content += "</label>";
    content += "</div>";
    return content;
  }

  function getRadioButtonName(index, permissionClass) {
    return index + "-" + permissionClass + "-radio";
  }

  function handleRadioClick(index, permissionClass, permissionState) {
    updatePermission(index, permissionClass, permissionState);

    // Return false to prevent the default link action (scrolling up to top of
    // page in this case).
    return false;
  }

  function handleRemoveClick(index) {
    updatePermission(index, "localStorage", PERMISSION_NOT_SET);
    updatePermission(index, "locationData", PERMISSION_NOT_SET);
    updatePermissionsList();

    // Return false to prevent the default link action (scrolling up to top of
    // page in this case).
    return false;
  }

  function updatePermission(index, permissionClass, permissionState) {
    if (permissions[index][permissionClass]) {
      if (permissions[index][permissionClass].permissionState !=
          permissionState) {
        permissions[index][permissionClass].permissionState =
          permissionState;
        enableButton(dom.getElementById("confirm-button"));
      }
    }
  }

  function layoutSettings(contentHeight) {
    var content = dom.getElementById("content");

    content.style.height = Math.max(contentHeight, 0) + "px";

    // If a scrollbar is going to be visible, then add some padding between it
    // and the inner right edge of the content. We don't want to have this in
    // all the time or else there will be double-padding when the scrollbar
    // isn't visible.
    if (content.scrollHeight > contentHeight) {
      content.style.paddingRight = "1em";
    } else {
      content.style.paddingRight = "";
    }
  }

  function calculateDiff() {
    var permissionClasses = ["localStorage", "locationData"];
    var result = { "modifiedOrigins": [] };
    for (var index = 0; index < originalPermissions.length; index++) {
      var originalOriginData = originalPermissions[index];
      var originData = permissions[index];
      // Defend against a mis-match between the origins in each list.
      if (isDefined(typeof originData) && 
          originData.name == originalOriginData.name) {
        var modifiedOriginData = new Object();
        var modified = false;
        for (var i = 0; i < permissionClasses.length; i++) {
          var permissionClass = permissionClasses[i];
          // The data may not include entries for all permission classes.
          if (isDefined(typeof originalOriginData[permissionClass])) {
            var originalState =
                originalOriginData[permissionClass]["permissionState"];
            var state = originData[permissionClass]["permissionState"];
            if (originalState != state) {
              modified = true;
              modifiedOriginData[permissionClass] = new Object();
              modifiedOriginData[permissionClass]["permissionState"] = state;
            }
          }
        }
        if (modified) {
          modifiedOriginData.name = originData.name;
          result.modifiedOrigins.push(modifiedOriginData);
        }
      }
    }
    return result;
  }

</script>
</html>
