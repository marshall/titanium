if (gearsFactory)
{
	ti.Http = function()
	{
		return gearsFactory.create('beta.httprequest');
	};
}