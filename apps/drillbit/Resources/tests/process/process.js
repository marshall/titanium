describe("process tests",
{
	test_process_as_async: function(test)
	{
		value_of(Titanium.Process).should_not_be_null();
		var p = null;
		
		if (Titanium.platform == 'win32')
		{
			p = Titanium.Process.launch('cmd.exe /c dir',['/s']);
		}
		else
		{
			p = Titanium.Process.launch('/bin/ls',['-la']);
		}

		var timer = null;
		value_of(p).should_not_be_null();
		var output = '';
		
		p.onread = function(buf)
		{
			output += buf;
		};
		p.onexit = function()
		{
			clearTimeout(timer);
			// this can happen in cases where we exit before we 
			// get everything out of the buffer
			if (output.length==0)
			{
				output = p.out.read();
			}
			if (output.length > 0)
			{
				test.passed();
			}
			else
			{
				test.failed('no output received');
			}
		};
		timer = setTimeout(function()
		{
			test.failed('timed out');
		},2000);
	},
	test_process_exception_as_async: function(test)
	{
		value_of(Titanium.Process).should_not_be_null();
		var p = null;
		
		if (Titanium.platform == 'win32')
		{
			p = Titanium.Process.launch('cmd.exe /c dir',['/s']);
		}
		else
		{
			p = Titanium.Process.launch('/bin/ls',['-la']);
		}

		var timer = null;
		value_of(p).should_not_be_null();
		var output = '';
		
		p.onread = function(buf)
		{
			// test throwing exception from onread
			throw "this is an exception";
		};
		p.onexit = function()
		{
			clearTimeout(timer);
			test.passed();
		};
		timer = setTimeout(function()
		{
			test.failed('timed out');
		},2000);
	}
});
