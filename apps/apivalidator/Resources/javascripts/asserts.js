

TestMonkey.installAssertionType('Function', function(win,frame,testcase,assertion,args)
{
	var obj = args[0];
	return [Object.prototype.toString.call(obj) === "[object Function]",obj];
});

TestMonkey.installAssertionType('Array', function(win,frame,testcase,assertion,args)
{
	var obj = args[0];
	return [Object.prototype.toString.call(obj) === "[object Array]",obj];
});
