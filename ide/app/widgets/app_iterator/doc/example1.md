Title: Simple Example

This is a simple example that uses the `<app:iterator>`.
	
<style>
<!--
#results a
{
	color:#0000CC;
	text-decoration:underline;
	font-size:16px;
}
.result_text
{
	color:#000;
	font-size:13px;
}
.result_link
{
	font-size:12px;
	color:green;
}
body
{
	font-family:arial,sans-serif;
	font:12px;
	color:#696969;
}

#search_table
{
	position:absolute;
	top:100px;
}

#results_top
{
	position:relative;
	top:10px;
	left:20px;
	width:60%;
}

#results_divider
{
	position:relative;
	top:10px;
	background-color:#D5DFF3;
	border-top:1px solid #3366CC;
	height:20px;
}
#results
{
	position:relative;
	top:20px;
	left:20px;
}
-->
</style>

++example
<app:iterator on="r:search.response then execute" property="rows">
	<html:div style="margin-bottom:15px">
		<html:a >#{result}</html:a>
		<html:div class="result_text">#{desc}</html:div>
		<html:div class="result_link">#{url}</html:div>
	</html:div>
</app:iterator>
<app:script>
	$MQ('r:search.response',{rows:[
		{'desc':'Ajax, DHTML and SOA all in one platform','url':'http://appcelerator.com','result':'Appcelerator - More App. Less Code.'},
		{'desc':'Ajax, or AJAX (Asynchronous JavaScript and XML), is a web development technique used for creating...','url':'http://en.wikipedia.org/wiki/Ajax_(programming)','result':'Ajax (programming) - Wikipedia, the free encyclopedia'},
		{'desc':'Provides the implementation of AJAX technology introduced by Microsoft.','url':'http://ajax.asp.net/','result':'AJAX : The Official Microsoft ASP.NET 2.0 Site'},
		{'desc':'AJAX is a type of programming made popular in 2005 by Google (with Google Suggest). AJAX is not a new programming...','url':'http://www.w3schools.com/ajax/default.asp', 'result':'AJAX Tutorial'},
		{'desc':'Seminal article which popularized "AJAX" as a term.','url':'http://www.adaptivepath.com/publications/essays/archives/000385.php','result':'adaptive path » ajax: a new approach to web applications'},
		{'desc':'A comprehensive guide on getting started with AJAX including articles, tutorials and links to other useful websites.','url':'http://developer.mozilla.org/en/docs/AJAX','result':'AJAX - MDC'},
		{'desc':"I am excited about this, as it means that you can write a rich Ajax client that doesn't need server-side...",'url':'http://ajaxian.com/ ','result':'Ajaxian'}
		]});
	$MQ('r:add.response',{rows:[
		{'desc':'Clean toilets fast with new and improved Ajax!','url':'www.ajax.com','result':'Ajax Clean Toliets'},
		{'desc':'Our Ajax improves your personal happiness by over 5%!','url':'www.ourajax.com','result':'Ajax Improves Happiness'},
		{'desc':'We do know Ajax, but we love beer','url':'www.beer.com','result':'Ajax is good. Beer is Better'},
		{'desc':'Get your Ajax on Ebay for $1.99','url':'www.ebay.com','result':'Last Call for Ajax on Ebay'},
		{'desc':'Ajax stole my car.  Can you help?','url':'www.findmycar.com','result':'Ajax: Car Thief'}
		]});
</app:script>
--example
