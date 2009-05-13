require 'foo'
require 'markaby/lib/markaby'

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
