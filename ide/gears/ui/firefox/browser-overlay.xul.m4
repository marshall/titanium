<?xml version="1.0"?>
<!DOCTYPE overlay SYSTEM "chrome://PRODUCT_SHORT_NAME_UQ/locale/i18n.dtd">

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

<overlay xmlns="http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul">

  <script type="application/x-javascript" 
    src="chrome://PRODUCT_SHORT_NAME_UQ/content/browser-overlay.js" />

  <menupopup id="menu_ToolsPopup">
    <!--
    Unfortuantely, the place we want to put this doesn't have any ID, so we
    can't use insertbefore. On a default install, 5 is correct.
    -->
    <menuitem id="PRODUCT_SHORT_NAME_UQ-settings" label="&PRODUCT_SHORT_NAME_UQ.browser.settings;" 
      accesskey="&PRODUCT_SHORT_NAME_UQ.browser.settings.accesskey;" position="5" 
      oncommand="PRODUCT_SHORT_NAME_UQ$openSettingsDialog()"/>
  </menupopup>

</overlay>
