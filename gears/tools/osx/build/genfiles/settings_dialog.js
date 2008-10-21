var localized_strings = {
  "en-US": {
    "string-denied": "Denied",
    "string-apply-accesskey": "A",
    "string-remove": "Remove",
    "string-gears-settings": "Gears Settings",
    "string-apply": "Apply",
    "string-cancel-accesskey": "C",
    "string-cancel": "Cancel",
    "string-html-title": "Gears Settings",
    "string-allowed": "Allowed",
    "string-local-storage": "Local storage",
    "string-nosites": "No sites.",
    "string-permissions-description": "The table below shows the permissions you have granted to each site that has attempted to use Gears.",
    "string-location-data": "Location",
    "string-version": "Gears version 0.4.20.0"
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

