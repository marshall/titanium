describe("ti.App tests",
{
	validate_properties: function()
	{
		value_of(Titanium.platform).should_be_one_of(['win32','osx','linux']);
		value_of(Titanium.version).should_be_number();
		
		value_of(Titanium.App.getID()).should_be('com.titaniumapp.unittest');
		value_of(Titanium.App.getName()).should_be('foobar');
		value_of(Titanium.App.getVersion()).should_be('1.2');
		value_of(Titanium.App.getPublisher()).should_be('yoy');
		value_of(Titanium.App.getURL()).should_be('blah.com');
		value_of(Titanium.App.getCopyright()).should_be('2010');
		value_of(Titanium.App.getDescription()).should_be('cool like dat');
		value_of(Titanium.App.getGUID()).should_be('D83B08F4-B43B-4909-9FEE-336CDB44750B');

		value_of(Titanium.App.exit).should_be_function();
		value_of(Titanium.App.loadProperties).should_be_function();
		value_of(Titanium.App.path).should_not_be_null();
		value_of(Titanium.App.arguments).should_not_be_null();
		value_of(Titanium.App.Properties).should_be_object();
		
		// this is specific to the test harness args
		value_of(Titanium.App.arguments.length).should_be(4); 
		
		value_of(Titanium.App.stdout).should_be_function();
		value_of(Titanium.App.stderr).should_be_function();
		
		// this should be the default stream if not specified in the manifest
		value_of(Titanium.App.getStreamURL()).should_be('http://api.appcelerator.net/p/v1');
		// test passing arg
		value_of(Titanium.App.getStreamURL('foo')).should_be('http://api.appcelerator.net/p/v1/foo');
		// test passing multiple args
		value_of(Titanium.App.getStreamURL('foo','bar')).should_be('http://api.appcelerator.net/p/v1/foo/bar');
	}
});