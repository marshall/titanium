
			// notify our pending ready listeners
			loaded = true;
			$.each(readies,function()
			{
				this.call(ti,$);
			});
			readies = null;
		}
	    catch(E)
	    {
			alert("Caught Exception in Framework: "+E+" at line: "+E.line);
	 	}
   });
	
// this ends the variable hiding and also 
// ensures that jQuery only is available inside our
// local scope (pass true to ensure jQuery is removed too)
})(window.jQuery.noConflict(true));