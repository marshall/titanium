def rubyalert2
	Window.alert("hello from external ruby")
end

Window.rubyFunction2 = method(:rubyalert2)
