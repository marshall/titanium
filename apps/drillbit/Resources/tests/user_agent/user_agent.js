describe("user agent tests",
{
	validate: function()
	{
		value_of(Titanium.userAgent).should_contain('Titanium/');
		value_of(Titanium.userAgent).should_contain('Titanium/'+Titanium.version);
	}
});