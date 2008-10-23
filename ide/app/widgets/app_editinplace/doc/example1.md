Title: Simple Example

This is a simple example that uses the `<app:editinplace>`.
	
	<style>
	<!--
	input[type=text]
	{
		border-color:#7C7C7C rgb(195, 195, 195) rgb(221, 221, 221);
		border-style:solid;
		border-width:1px;
		font-size:26px;
		margin:1px;
		padding:3px 3px 3px 10px;
		width:350px;
	}

	-->
	</style>	
	<div style="padding:20px;border:1px dashed #bbb;background-color:#f6f6f6;">
		<app:editinplace type="text" id="foo" saveOn="click then l:save" cancelOn="click then l:cancel" 
			validator="required" position="right" defaultClassName="bold_value_grey_lg" defaultValue="Click Me or Take a Hike">
		</app:editinplace>
	</div>
	<div style="padding:10px;padding-left:none"></div>
	<span class="bold_value" style="display:none" on="l:save then show or l:save then effect[Fade] after 2s ">
		You clicked saved
	</span>
	<span class="bold_value" style="display:none"  on="l:cancel then show or l:cancel then effect[Fade] after 2s ">
		You clicked cancel
	</span>
