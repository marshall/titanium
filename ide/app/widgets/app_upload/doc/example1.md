Title: Simple Example

You can use upload to upload a file along with other fields to be processed by a service `<app:upload>`.

    <app:upload action="-/fileupload" maxsize="3145728" on="l:save.picture.request then submit" 
    	service="save.picture.request" id="picture_upload">
	
    	<html:input type="file" fieldset="my_profile" name="profile_photo" id="profile_photo"></html:input>
    	<html:input type="text" fieldset="my_profile" name="profile_name" id="profile_name"></html:input>
    	<html:button  id="profile_button" fieldset="my_profile"
    		on="click then l:save.account.request">
    		Save my changes
    	</html:button>
    </app:upload>