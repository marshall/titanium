Title: Simple Example

This is a simple example that uses the `<app:if>`.
	
	<span on="click then l:message.trigger
		or l:message.example then hide">
		Click me to send a message
	</span>
	<app:iterator on="l:if.example then execute" property="results">
		<app:if expr="#{show}">
			<html:div style="padding: 5px;margin-bottom: 10px; border:1px solid #ccc;background-color:#f6f6f6">
				#{text} (shown conditionally)
			</html:div>
		</app:if>
	</app:iterator>
	<app:message on="l:message.trigger then execute" 
		name="l:if.example" args="{results: [{show: true, text: 'Example 1'},{show: false, text: 'Example 2'},{show: true, text: 'Example 3'}]}">
	</app:message>
	
