ti.App.include("ti://plugin/jquery/jquery-1.2.6.js");

// we need to make sure jQuery is in no conflict mode
// but we want to re-assign it to our local ti scope
// so we can use it
ti.jQuery = ti.$ = jQuery.noConflict();
