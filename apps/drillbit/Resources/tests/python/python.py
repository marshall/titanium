from foo import Foo

def external_document():
	return document.getElementById('a')
	
def require_file_module():
	f = Foo()
	return f.bar()
	
def require_sub_file_module():
	f = Foo()
	return f.yum()
	
def test_window_global():
	return my_global_foo('suck ass')
	
def what_is_love():
	f = Foo()
	return f.what_is_love(my_global_var)

def test_document_title():
	return document.title

def test_external_module():
	return Foo().go_google()
	
def test_type_string():
	return 'i am %s' % 'string'	

def test_type_int():
	return 1
	
def test_type_boolean_false():
	return False

def test_type_boolean_true():
	return True
	
def test_type_map():
	return {'a':'b'}
	
def test_type_tuple():
	return ('a','b')

def test_type_none():
	return None
	
def test_type_dict():
	return dict(one=2,two=3)
	
def test_type_function():
	return test_type_dict
	
					