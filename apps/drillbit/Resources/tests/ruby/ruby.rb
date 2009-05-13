require 'foo'
require 'markaby/lib/markaby'
require 'ostruct'

def external_document
	document.getElementById 'a'
end

def require_file_module
  f = Foo.new
  f.bar
end

def require_sub_file_module
  f = Foo.new
  f.yum
end

def test_window_global
  my_global_foo 'suck ass'
end

def what_is_love?
  f = Foo.new
  f.what_is_love?(my_global_var)
end

def test_document_title
  return document.title
end

def test_gem
   mab = Markaby::Builder.new
   mab.html do
     head { title "Boats.com" }
   end
   mab.to_s
end

def test_type_string
	return 'i am string'	
end

def test_type_float
  1.1
end

def test_type_long
  10**100 #<= google
end

def test_type_scientific_notation_large
  1.7e19
end

def test_type_scientific_notation_small
  3.21e-13
end

def test_type_int
	1
end
	
def test_type_boolean_false
	false
end

def test_type_boolean_true
	true
end
	
def test_type_map
	{'a'=>'b'}
end
	
def test_type_tuple
	OpenStruct.new(:one=>2,:two=>3)
end

def test_type_struct
	customer = Struct.new(:name,:address)
	customer.new("jeff","mtn view")
end

def test_type_nil
	nil
end
	
def test_type_array
	[1,2,3]
end
	
def test_type_function
	method(:test_type_array)
end

def test_type_proc
  Proc.new { 'hello world' }
end

