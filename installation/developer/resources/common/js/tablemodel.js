App.TableModel = function(data)
{
	this.data = data;
	
	this.getRowCount = function()
	{
		return this.data ? this.data.length : 0;
	},
	this.getRow = function(idx)
	{
		return (this.data ? this.data[idx] : null);
	}

};

