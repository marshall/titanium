describe("user agent tests",
{
	validate_name_and_version: function()
	{
		value_of(Titanium.userAgent).should_contain('Titanium/');
		value_of(Titanium.userAgent).should_contain('Titanium/'+Titanium.version);
		
		// for now, we're going to simulate the near later version
		// this addresses TI-303
		value_of(Titanium.userAgent).should_contain('Version/4.0 Safari/528.16');
	}
});