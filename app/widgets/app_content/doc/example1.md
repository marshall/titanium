Title: Simple Example

This is a simple example that uses the `<app:content>`.
	
++example
<div id="styled">
	<app:tabpanel id="contentmenu" initial="movies">
		<tab name="movies">Favorite Movies</tab>
		<tab name="music">Favorite Music</tab>
		<tab name="banjo">Banjo Sonatas in D</tab>
	</app:tabpanel>
</div>
<div id="my_tab_divider"></div>
<app:content
	lazy="false"
	args="{'title':'These movies do not suck'}"
	src="../../widgets/app_content/doc/movies.html" 
	on="contentmenu[movies] then show else hide">
</app:content>
<app:content
	lazy="false"
	args="{'title':'These albums do not suck'}"
	src="../../widgets/app_content/doc/music.html" 
	on="contentmenu[music] then show else hide">
</app:content>
<app:content
	lazy="false"
	args="{'title':'Banjo - the new Cow Bell'}"
	src="../../widgets/app_content/doc/banjo.html" 
	on="contentmenu[banjo] then show else hide">
</app:content>			
--example
