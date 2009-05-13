describe("Ruby Tests",
{
	test_inline: function()
	{
		value_of(window.inline_test_result).should_be('A');
	},
	test_external_file: function()
	{
		value_of(window.external_test_result).should_be('A');
	},
	test_require_file_module: function()
	{
		value_of(window.require_file_module_result).should_be('hello,world');
	},
	test_require_file_module_and_sub_file: function()
	{
		value_of(window.require_file_sub_file_module_result).should_be('yah');
	},
	test_window_global_from_ruby: function()
	{
		// test to make sure that we can access a function defined
		// in normal javascript block from within ruby 
		value_of(window.test_window_global_result).should_be('you suck ass');
	},
	test_window_global_var_from_ruby: function()
	{
		// test passing global variable from JS and getting it back
		value_of(window.what_is_love_result).should_be('i love you');
	},
	test_document_title_from_ruby: function()
	{
		value_of(window.test_document_title_result).should_be('I love Matz');
	},
	test_gem_include: function()
	{
		value_of(window.test_gem_result).should_be('<html><head><meta content="text/html; charset=utf-8" http-equiv="Content-Type"/><title>Boats.com</title></head></html>');
	}
});