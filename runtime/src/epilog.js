	}
	catch(E)
	{
		alert("Caught Exception in Framework: "+E);
	}
	
// this ends the variable hiding and also 
// ensures that jQuery only is available inside our
// local scope (pass true to ensure jQuery is removed too)
})(window.jQuery.noConflict(true));