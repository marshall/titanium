Title: Simple Example

This is a simple example that uses the `<app:script>`.  Please note that only C-style comments are supported (/* comment here */).

++example
<button on="click then l:get.current.datetime">
    Get time and date
</button>
<div style="border:1px solid #ccc;background-color:#f6f6f6;padding:10px;margin-top:10px;display:none"
	on="l:show.current.datetime then show and effect[Highlight] or l:reset.script.example then hide">
	<span style="color:#000">
		Current Date Time = <span on="l:show.current.datetime then value[datetime]"></span>
		<a on="click then l:reset.script.example">Reset Example</a>
	</span>
</div>

<app:script on="l:get.current.datetime then execute">
	var date = new Date();
	$MQ('l:show.current.datetime',{'datetime':date});
</app:script>
--example
