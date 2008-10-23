Title: Simple Example

This is a simple example that uses the `<app:folder>`.
	
++example
<app:folder id="folder_example">
    <folder name="Overview">
        <item onopen="l:test.folder[name=my]">My Documents</item>
        <item onopen="l:test.folder[name=your]">Your Documents</item>
    </folder>
</app:folder>

<div style="padding:10px;border:1px solid #999;background-color:#ffffcc;margin-top:20px">
    <span on="l:test.folder then hide" >
        Click a folder item to see a message here
    </span>
    <span on="l:test.folder[name=my] then show else hide" style="color:green;display:none">
        You clicked "My Documents"
    </span>
    <span on="l:test.folder[name=your] then show else hide" style="color:green;display:none">
        You click "Your Documents"
    </span>
</div>
--example
