<?xml version="1.0"?>

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

<RDF xmlns="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
     xmlns:em="http://www.mozilla.org/2004/em-rdf#">

  <Description about="urn:mozilla:install-manifest">
    <!-- 
    Think very carefully before you change this ID, which uniquely identifies
    this extension across versions. If you do change it:
    * Come up with some way to remove old versions of the extension, since
      Firefox will no longer do it for you. See old versions of bootstrap.js
      which had some code for this (look for 'killOldExtension()' and 
      'killOldRegistryEntry()').
    * Keep it all lower-case. We've observed behavior in the field where URLs
      coming into Google are forced to lower-case.
    * Do not put the name of the product in it. Otherwise we are forced to
      change it when the product name changes.
    * Change all other occurences of it:
      - bootstrap.js
      - paths_ff.cc
      - win32_msi.wxs.m4
    * Consider changing other unique identifiers throughout the code, such as:
      - contract IDs
      - interface IDs
      - class names
      - names exposed to the DOM
      If you do not change these, but do change this extension ID, Firefox will
      have two extensions installed both supporting these unique IDs and will
      basically guess which one to use. This probably isn't what you want.
    * Reconsider how cool the current GUID is. It starts with three zeros *and*
      contains the quad "beef". I mean, what are the chances of that?
   -->
    <em:id>{000a9d1c-beef-4f90-9363-039d445309b8}</em:id>
    <em:version>PRODUCT_VERSION</em:version>

    <!-- 
    For Up-To-Date Documentation of this Format Please See: 
    http://www.mozilla.org/projects/firefox/extensions/packaging/extensions.html
    -->
    <em:targetApplication>
      <Description>
        <em:id>{ec8030f7-c20a-464f-9b0e-13a3a9e97384}</em:id>
        <em:minVersion>1.5</em:minVersion>
        <em:maxVersion>3.0.*</em:maxVersion>
      </Description>
    </em:targetApplication>

    <!-- Prevent this XPI from being installed on the wrong platform. -->
m4_changequote(`^|',`|^')m4_dnl
m4_ifelse(PRODUCT_OS,^|win32|^,^|m4_dnl
    <em:targetPlatform>WINNT_x86-msvc</em:targetPlatform>
|^,PRODUCT_OS,^|wince|^,^|m4_dnl
    <!-- WE DON'T CURRENTLY BUILD FOR WINCE FIREFOX. -->
|^,PRODUCT_OS,^|linux|^,^|m4_dnl
    <em:targetPlatform>Linux_x86-gcc3</em:targetPlatform>
    <!-- Ubuntu Edgy Eft requires "linux-gnu" for the OS_TARGET prefix. -->
    <em:targetPlatform>linux-gnu_x86-gcc3</em:targetPlatform>
|^,PRODUCT_OS,^|osx|^,^|m4_dnl
    <em:targetPlatform>Darwin</em:targetPlatform>
|^)
    <em:name>PRODUCT_FRIENDLY_NAME_UQ</em:name>
    <em:description>These are the gears that power the tubes! :-)</em:description>
    <em:creator>Google Inc.</em:creator>
    <em:homepageURL>http://gears.google.com/</em:homepageURL>
    <em:iconURL>chrome://PRODUCT_SHORT_NAME_UQ/content/icon_32x32.png</em:iconURL>
m4_ifelse(PRODUCT_OS,^|win32|^,^|m4_dnl
    <!-- On Win32, updates should _only_ be handled by the Google Updater.
         To completely guard against Firefox changing the same files, we
         remove the updateURL on Win32. -->
|^,^|m4_dnl // PRODUCT_OS else...
    <!-- All Google extensions use this "/update?..." suffix for updateURL,
         except for "&os=..." which was added for our product. We should
         be able to drop the os argument after the migration to the
         public autoupdate server is successful (i.e., the user-agent
         mechanism on tbhome is confirmed to be working properly).
         -->
m4_ifdef(^|OFFICIAL_BUILD|^,^|m4_dnl
    <em:updateURL><![CDATA[https://tools.google.com/service/update2/ff?guid=%ITEM_ID%&version=%ITEM_VERSION%&application=%APP_ID%&appversion=%APP_VERSION%&dist=google&os=PRODUCT_OS]]></em:updateURL>
|^,^|m4_dnl // OFFICIAL_BUILD else...
    <em:updateURL><![CDATA[https://tools.google.com/service/update2/ff?guid=%ITEM_ID%&version=%ITEM_VERSION%&application=%APP_ID%&appversion=%APP_VERSION%&dist=google&os=PRODUCT_OS&dev=1]]></em:updateURL>
|^)m4_dnl // OFFICIAL_BUILD end

|^)m4_dnl // PRODUCT_OS end

  </Description>
</RDF>
