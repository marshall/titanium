Title: Dynamic Example

This is a dynamic example that uses the `<app:datatable>`.

++example
	<style>
	<!--
	.table_cell
	{
		padding: 3px;
		border-right: 1px solid #999;
		border-left: 1px solid #999;
	}
	.table_cell_header
	{
		padding-top: 3px;
		padding-bottom: 3px;
		padding-left: 5px;
		padding-right: 5px;
		background-color: #999;
		color:white;
		border: 1px solid #555;
		cursor: pointer;
	}
	.table_row_even
	{
		padding: 3px;
		border-right: 1px solid #999;
		border-left: 1px solid #999;
		border-bottom:1px solid #999;
		background-color: #fff;
	}
	.table_row_odd
	{
		padding: 3px;
		border-right: 1px solid #999;
		border-left: 1px solid #999;
		border-bottom:1px solid #999;
		background-color: #ddd;		
	}
	-->
	</style>
	<app:datatable on="l:load.datatable then execute" width="50%" property="rows" sort="client" 
		rowEvenClass="table_row_even" rowOddClass="table_row_odd">
		<header property="col1" align="left" width="40%" title="Greetings">hello there #{col1} how are you?</header>
		<header property="col2" align="center" width="30%">Age</header>
		<header property="col3" align="center" width="30%">Height</header>
		<header property="col4" align="center" width="30%">Epidermis Showing?</header>
		<header align="center" width="30%" sortable="false" title="action"><a on="click then l:test[name=#{col1}]" style="color:blue;cursor:pointer">save</a></header>
	</app:datatable>
	<app:script on="l:test then execute">
		alert("Do you really want to save  "+this.data.name+"?");
	</app:script>
	<app:script style="display:none;">
		$MQ('l:load.datatable',{'rows':[{'col1':'Sam','col2':'18','col3':'5.5 feet','col4':'Yes'},
		{'col1':'Vanessa','col2':'22','col3':'6.5 feet','col4':'No'},
		{'col1':'Joe','col2':'18','col3':'5.0 feet','col4':'Yes'},
		{'col1':'Bill','col2':'40','col3':'5.2 feet','col4':'Yes'},
		{'col1':'James','col2':'33','col3':'6.1 feet','col4':'No'},
		{'col1':'Mary','col2':'25','col3':'5.7 feet','col4':'Yes'}]});
	</app:script>
--example