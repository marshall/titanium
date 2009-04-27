describe("ti.UI.Dialog dialog tests",
{
	create_dialog_with_no_content_as_async:function(callback)
	{
		value_of(Titanium.UI.showDialog).should_be_function();
		var dialog = Titanium.UI.showDialog({
			'url':'app://dialog_no_content.html'
		});
		value_of(dialog).should_be_object();
		value_of(dialog.close).should_be_function();
		setTimeout(function()
		{
			try
			{
				value_of(dialog.getResult()).should_be_null();
				callback.passed();
			}
			catch(e)
			{
				callback.failed(e);
			}
			dialog.close();
		},1000);
	},

	create_dialog_return_result_as_async:function(callback)
	{
		var timer = null;
		value_of(Titanium.UI.showDialog).should_be_function();
		var dialog = Titanium.UI.showDialog({
			'url':'app://dialog_return_result.html',
			'parameters':{'in':'out'},
			'onclose':function(result)
			{
				clearTimeout(timer);
				try
				{
					value_of(result).should_not_be_null();
					value_of(result).should_be_object();
					value_of(result.a).should_be('b');
					value_of(result.out).should_be('out');
					callback.passed();
				}
				catch(e)
				{
					callback.failed(e);
				}
			}
		});
		value_of(dialog).should_be_object();
		value_of(dialog.close).should_be_function();
		timer = setTimeout(function()
		{
			callback.failed('dialog timed out, should have closed by now');
			dialog.close();
		},5000);
	}
});