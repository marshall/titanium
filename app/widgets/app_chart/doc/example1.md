Title: Simple Example

This is a simple example that uses the `<app:chart>`.
	
++example
<app:chart type="bar" chartMode="mode" on="l:load.barchart then execute" property="rows" chartTitles="titles"
	title="Happiness Meter" angle="30" color="#0D52D1,#2A0CD0,#8A0CCF"
	thickness="10" height="300"  width="600" legend="true">
</app:chart> 
<app:script style="display:none;">    	
    $MQ('l:load.barchart',
        {'rows':[
            {'name':'Appcelerator', 'value':'10,10,10'},
            {'name':'Flex', 'value':'7,5,5'},
            {'name':'Open Lazlo', 'value':'6,5,5'},
            {'name':'Google Web Toolkit', 'value':'6,6,2'}],
        'titles':[
            {'title':'Easy to Use 1'},
            {'title':'Fun to Use'},
            {'title':'Makes me coffee'}],'mode':'clustered'}); 	
</app:script>
--example
