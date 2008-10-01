Title: Simple Example

This is a simple example that uses the `<app:message>`.

++example	
<app:message on="l:message.trigger then execute" 
	name="l:message.example" args="{'message':'You will enjoy financial success','answer':'haha. just kidding.'}">
</app:message>
<span on="click then l:message.trigger
	or l:message.example then hide
	or l:reset.message.example then show">
	<a style="text-decoration: underline">Click me to trigger your fortune</a>
</span>
<span on="l:reset.message.example then hide or l:message.example then show" style="display:none">
	<span on="l:message.example then value[message] or l:reset.message.example then clear"></span>
	<span style="color:red" on="l:message.example then value[answer] after 1s
		or l:reset.message.example then clear"></span>
	<a on="click then l:reset.message.example">Reset Example</a>
</span>
--example
	
