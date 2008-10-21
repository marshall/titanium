var localized_strings = {
  "en-US": {
    "string-workerpool-desc": "Run asynchronous JavaScript to improve application responsiveness",
    "string-allow": "Allow",
    "string-query-data": "The website below wants to store information on your computer using Gears.",
    "string-deny-accesskey": "D",
    "string-query-location": "The website below wants to access information about your location using Gears.",
    "string-html-title": "Gears Security Warning",
    "string-description": "Gears is an open source browser extension that enables web applications to provide offline functionality using the following JavaScript APIs:",
    "string-database-desc": "Store data locally in a fully-searchable relational database",
    "string-allow-accesskey": "A",
    "string-cancel": "Cancel",
    "string-never-allow-link": "Never allow this site",
    "string-deny": "Deny",
    "string-never-allow-link-wince": "Never allow it",
    "string-trust-site-accesskey": "t",
    "string-localserver-desc": "Store and serve application resources locally",
    "string-location-privacy-statement": "Read the site's privacy policy to see how your location will be used.",
    "string-trust-site": "I trust this site. Allow it to use Gears."
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

