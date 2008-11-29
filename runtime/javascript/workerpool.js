if (gearsFactory)
{
	ti.Workerpool = function()
	{
		return gearsFactory.create('beta.workerpool');
	}
}