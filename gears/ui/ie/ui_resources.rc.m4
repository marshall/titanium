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

#ifdef WINCE
  #include "aygshell.h"
  #include "afxres.h"
  #include "genfiles/product_constants.h"
#else
  #include "WinResrc.h"
#endif
#include "ui/ie/ui_resources.h"
#include "ui/ie/string_table.h"

//-----------------------------------------------------------------------------
// HTML
// Note: We break with convention on the naming of these resources so that
// they can be addressed the same way on all platforms.
// TODO(aa): What effect (if any) does the HTML tag on these have? Should I be
// using something else? It seems to work as is.
//-----------------------------------------------------------------------------

button.css                   HTML  "ui/common/button.css"
button.css.end               HTML  {"\0END\0"}
button_bg.gif                HTML  "ui/common/button_bg.gif"
button_bg.gif.end            HTML  {"\0END\0"}
button_corner_black.gif      HTML  "ui/common/button_corner_black.gif"
button_corner_black.gif.end  HTML  {"\0END\0"}
button_corner_blue.gif       HTML  "ui/common/button_corner_blue.gif"
button_corner_blue.gif.end   HTML  {"\0END\0"}
button_corner_grey.gif       HTML  "ui/common/button_corner_grey.gif"
button_corner_grey.gif.end   HTML  {"\0END\0"}
html_dialog.css              HTML  "ui/common/html_dialog.css"
html_dialog.css.end          HTML  {"\0END\0"}
icon_32x32.png               HTML  "ui/common/icon_32x32.png"
icon_32x32.png.end           HTML  {"\0END\0"}
local_data.png               HTML  "ui/common/local_data.png"
local_data.png.end           HTML  {"\0END\0"}
location_data.png            HTML  "ui/common/location_data.png"
location_data.png.end        HTML  {"\0END\0"}

permissions_dialog.html      HTML "genfiles/permissions_dialog.html"
permissions_dialog.html.end  HTML {"\0END\0"}
settings_dialog.html         HTML "genfiles/settings_dialog.html"
settings_dialog.html.end     HTML {"\0END\0"}
shortcuts_dialog.html        HTML "genfiles/shortcuts_dialog.html"
shortcuts_dialog.html.end    HTML {"\0END\0"}

//-----------------------------------------------------------------------------
// Dialogs
//-----------------------------------------------------------------------------

#ifdef WINCE

// NOTE: Resources files aren't run through the translation
// console, so we can't localize the strings here.
// The js bridge used with the HTML dialog define two methods
// SetButton() and SetCancelButton() which we use to set the
// buttons labels; the translation is thus done in the HTML files.

IDR_HTML_DIALOG_MENU MENU
BEGIN
  POPUP ""
  BEGIN
    MENUITEM "Cancel" ID_CANCEL
    MENUITEM "Save" ID_ALLOW
  END
END

IDR_HTML_DIALOG_MENUBAR RCDATA
BEGIN
  IDR_HTML_DIALOG_MENU,
  2,
  I_IMAGENONE, ID_CANCEL, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE,
  0, 0, NOMENU,
  I_IMAGENONE, ID_ALLOW, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE,
  0, 0, NOMENU,
END

IDD_GENERIC_HTML DIALOG -5000, -5000, 500, 500
STYLE DS_SETFONT | DS_MODALFRAME | DS_FIXEDSYS | DS_CENTER | WS_POPUP | 
      WS_DLGFRAME | WS_CAPTION | WS_NONAVDONEBUTTON 
CAPTION PRODUCT_FRIENDLY_NAME_ASCII
FONT 8, "MS Sans Serif"
BEGIN
END

#else

IDD_GENERIC_HTML DIALOGEX -5000, -5000, 500, 500
STYLE DS_SETFONT | DS_MODALFRAME | DS_FIXEDSYS | DS_CENTER | WS_POPUP |
    WS_CAPTION | WS_SYSMENU | WS_THICKFRAME
CAPTION ""
FONT 8, "MS Shell Dlg", 400, 0, 0x1
BEGIN
    // Note: This is the GUID of Microsoft's reusable browser control:
    // http://msdn2.microsoft.com/en-us/library/aa752040.aspx
    CONTROL         "",IDC_GENERIC_HTML,"{8856F961-340A-11D0-A96B-00C04FD705A2}",WS_TABSTOP,7,7,486,486
END
#endif

//-----------------------------------------------------------------------------
// Strings
//-----------------------------------------------------------------------------

// The string resources must follow the format below for i18n.
// * If a string resource is preceded by one or more comment lines, those
//   comments will be associated with the string for translation purposes.
// * To add a comment not tied to any particular string, follow it with
//   a blank line.
// * It is also permissible to list string resources consecutively, without
//   any comments or blank lines in-between.

// Leave commented out until we have a real entry; table cannot be empty.
//STRINGTABLE
//BEGIN
//
//  // Example:
//  // IDS_CAPABILITIES_HELP_URL  "http://www.google.com/"
//
//END

