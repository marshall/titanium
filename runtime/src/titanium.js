function debug (x) {
	TiNative.debug(x);
}

titanium = {};

titanium.include = function(x) {
	TiNative.include(x);
}

debug("loading..");


titanium.appName = TiNative.getAppName();
titanium.endPoint = TiNative.getEndpoint();

debug("set vars..");