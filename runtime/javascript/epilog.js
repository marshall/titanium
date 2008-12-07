
			// notify our pending ready listeners
			loaded = true;
			$.each(readies,function()
			{
				var fn = this;
				try
				{
					fn.call(window,$);
				}
				catch (ReadyException)
				{
					ti.App.debug("Ready function failed: "+ReadyException+", line: "+ReadyException.line);
					ti.App.debug("Ready script was: "+String(fn));
					ti.App.debug("===============================================================");
				}
			});
			readies = null;
		}
	    catch(TiException)
	    {
			ti.App.debug("Caught Exception in Framework: "+TiException+" at line: "+TiException.line);
			alert("Caught Exception in Framework: "+TiException+" at line: "+TiException.line);
	 	}
   });
	
// this ends the variable hiding and also 
// ensures that jQuery only is available inside our
// local scope (pass true to ensure jQuery is removed too)
})(window.jQuery.noConflict(true));