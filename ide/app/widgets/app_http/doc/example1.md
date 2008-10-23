Title: Simple Example

This is a simple example that uses the `<app:http>`.
	
Here's an example of how to display photos from Flickr (using JSON).

	<app:http on="l:get.pictures then get"> 
	       <uri method="get" responseRegex="jsonFlickrApi\((.*)\)" uri="http://api.flickr.com/services/rest/?method=flickr.photos.getRecent&amp;format=json&amp;api_key=adfc38a29c02d72e21d6f71ffcae264d" response="l:get.pictures.response"></uri>
	</app:http>
	<app:iterator on="l:get.pictures.response then execute" property="photos.photo">
		<html:div style="float:left"><html:img src="http://farm#{farm}.static.flickr.com/#{server}/#{id}_#{secret}.jpg"></html:img></html:div>
	</app:iterator>
	<app:message name="l:get.pictures"></app:message>			

Here's an example of how to get RSS articles from Techcrunch (using XML).

    <app:http on="l:get.techcrunch then get">
      <uri method="get" uri=" http://feeds.feedburner.com/Techcrunch" response="l:get.techcrunch.response"></uri>
    </app:http>
    <app:iterator on="l: get.techcrunch.response then execute" property="rss.channel.item">
       <html:div><html:a href="#{link}">#{title}</html:a></html:div>
       <html:div>#{content:encoded}</html:div> 
    </app:iterator>
    <app:message name="l:get.techcrunch"></app:message>		
