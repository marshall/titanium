// fix a limitation in using custom targets for chromium/win32
// just append the custom target to the query string of the URL
if (ti.platform == "win32") {
	$.each($('a[target^=ti:]'), function()
	{
		var href = $(this).attr('href');
		var target = $(this).attr('target');
		target = target.replace("ti:", "");
		
		if (href.indexOf('?') == -1) {
			href += '?ti_Target_Win32=' + target;	
		} else {
			href += '&ti_Target_Win32=' + target;
		}
		
		$(this).attr('href', href);
		$(this).attr('target', null);
	});
}


// 
// if we have transparency and we don't have a background
// color at the body or parent element level, we set the 
// background to white
//

//FIXME temporarily see if we can work around this stuff

// if (windowTransparency < 1.0)
// {
// 	if (!document.body.style.backgroundColor)
// 	{
// 		var top = $('body > *:first');
// 		if (!top.css('backgroundColor'))
// 		{
// 			top.css('backgroundColor','white');
// 		}
// 	}
// }
// 
// //
// // we need to fix up anchors so that they will open correctly
// //
// $.each($('a[href]:not(a[target])'),function()
// {
// 	$(this).attr('target','_new');
// });

