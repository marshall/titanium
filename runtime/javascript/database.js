if (gearsFactory)
{
	ti.Database = function()
	{
		return gearsFactory.create('beta.database');
	}
}