var localized_strings = {
  "en-US": {
    "string-no": "No",
    "string-header-desktop": "This website wants to create a shortcut on your computer. Do you want to allow this?",
    "string-no-accesskey": "N",
    "string-header-wince": "This website wants to create a shortcut in your list of programs. Do you want to allow this?",
    "string-title-simple": "Create Application Shortcuts",
    "string-ok-accesskey": "O",
    "string-ok": "OK",
    "string-cancel-accesskey": "C",
    "string-cancel": "Cancel",
    "string-startmenu": "Start menu",
    "string-yes": "Yes",
    "string-never-allow": "Never allow this shortcut",
    "string-yes-accesskey": "Y",
    "string-quicklaunch": "Quick launch bar",
    "string-never-allow-wince": "Never allow",
    "string-desktop": "Desktop",
    "string-title-default": "Gears - Create Desktop Shortcut",
    "string-location-query": "Create shortcuts in the following locations:"
  }
};

// Insert all localized strings for the specified locale into the div or span
// matching the id.
function loadI18nStrings(locale) {
  var rtl_languages = ['he', 'ar', 'fa', 'ur'];

  if (!locale) {
    locale = 'en-US';
  } else {
    if (!localized_strings[locale]) {
      // For xx-YY locales, determine what the base locale is.
      var base_locale = locale.split('-')[0];

      if (localized_strings[base_locale]) {
        locale = base_locale;
      } else {
        locale = 'en-US';
      }
    }
  }

  var strings = localized_strings[locale];

  // If the specified locale is an right to left language, change the direction
  // of the page.
  for (index in rtl_languages) {
    if (locale == rtl_languages[index]) {
      document.body.dir = "rtl";
      break;
    }
  }

  // Copy each string to the proper UI element, if it exists.
  for (name in strings) {
    if (name == 'string-html-title') {
      if (!browser.ie_mobile) {
        // IE Mobile dialogs don't have a title bar.
        // Furthermore, document.title is undefined in IE Mobile on WinMo 5.
        // It's also impossible to add properties to the window object.
        // (see http://code.google.com/apis/gears/mobile.html)
        document.title = strings[name];
      }
    } else {
      var element = dom.getElementById(name);
      if (element) {
        element.innerHTML = strings[name];
      }
    }
  }
}

