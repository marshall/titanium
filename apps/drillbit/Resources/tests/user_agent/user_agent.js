describe("user agent tests",
{
	validate_name_and_version: function()
	{
		value_of(Titanium.userAgent).should_contain('Titanium/');
		value_of(Titanium.userAgent).should_contain('Titanium/'+Titanium.version);
	}
});