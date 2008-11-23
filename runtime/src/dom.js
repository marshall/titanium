// 
// if we have transparency and we don't have a background
// color at the body or parent element level, we set the 
// background to white
//
if (windowTransparency < 1.0)
{
	if (!document.body.style.backgroundColor)
	{
		var top = $('body > *:first');
		if (!top.css('backgroundColor'))
		{
			top.css('backgroundColor','white');
		}
	}
}

//
// we need to fix up anchors so that they will open correctly
//
$.each($('a[href]:not(a[target])'),function()
{
	$(this).attr('target','_new');
});

