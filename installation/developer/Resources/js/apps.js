TiDeveloper.Apps = {};

//
// Rating Function
//
TiDeveloper.Apps.setupRating = function(data)
{
	// set rating
	for (var i=0;i<data.length;i++)
	{
		var value = data[i].value
		var id = data[i].id
		for (var j=5;j>value;j--)
		{
			$('#rating_' + id + '_' + j).addClass('rating_off');
		}
	}
	
	// capture rating on click
	$('.rating').click(function()
	{
		var id = $(this).attr('appid')
		var ratingStr = $('#rating_string_' + id);
		var hasVoted = ratingStr.attr('hasVoted');
		var vote = parseInt($(this).attr('star'));
		var votes = parseInt(ratingStr.attr('votes'));
		var rating = parseFloat(ratingStr.attr('rating'));
		
		
		// we only count vote
		if (hasVoted == "true")
		{
			ratingStr.css('display','none');
			$('#already_voted_'+id).fadeIn();
			setTimeout(function()
			{
				$('#already_voted_'+id).css('display','none');
				ratingStr.fadeIn();
				
			},1500)
			
		}
		else
		{
			var totalRating = (votes * rating) + vote;
			votes++
			var newRating = String(totalRating/votes).split('.');
			newRating = newRating[0] + "." + newRating[1].substring(0,2);
			
			ratingStr.css('display','none');
			ratingStr.html(newRating+' rating from '+votes+' votes')
			$('#voted_success_'+id).fadeIn();
			setTimeout(function()
			{
				$('#voted_success_'+id).css('display','none');
				ratingStr.fadeIn();
				
			},1500)

			// do we need to update stars
			if (parseFloat(newRating) > (parseInt(rating) + 1))
			{
				$('#rating_' + id + '_' + (parseInt(rating) +1)).removeClass('rating_off');
			}
			else if (parseFloat(newRating) < parseInt(rating))
			{
				$('#rating_' + id + '_' + rating).addClass('rating_off');
			}
			ratingStr.attr('rating',newRating);
			ratingStr.attr('hasVoted','true');
			
		}
		
	});
	
	// enable mouseover
	$('.rating').mouseover(function()
	{
		var id = $(this).attr('appid');
		var star = parseInt($(this).attr('star'));
		for (var i=star;i>1;i--)
		{
			$('#rating_' + id + '_' + i).removeClass('rating_off');

		}
	})

	// enable mouseout
	$('.rating').mouseout(function()
	{
		var id = $(this).attr('appid');
		var rating = parseInt($('#rating_string_'+id).attr('rating'))
		rating++;
		for (var i=rating;i<6;i++)
		{
			$('#rating_' + id + '_' + i).addClass('rating_off');

		}
	})
	
}

//
// listen for app search results
// and format star ratings
//
$MQL('l:applist',function(msg)
{
	setTimeout(function()
	{
		TiDeveloper.Apps.setupRating(msg.payload['rows']);

	},100);
})

//
// Mock Service
//
$MQL('l:menu',function(msg)
{
	if (msg.payload.val == 'apps')
	{
		$MQ('l:applist',{rows:[
				{image:'images/bigimage2.jpg',title:'Tweeter',pubdate:'2/1/2008',author:'Joe Smith',url:'http://joe.com',desc:'This is yet another twitter app written with titanium.  please enjoy.  It was develop in less than two weeks using turtles',id:1,value:3.6,votes:12,hasVoted:true,downloads:0},
				{image:'images/bigimage2.jpg',title:'Tweeter',pubdate:'2/1/2008',author:'Joe Smith',url:'http://joe.com',desc:'This is yet another twitter app written with titanium.  please enjoy  It was develop in less than two weeks using turtles',id:2,value:5.0,votes:23,hasVoted:false,downloads:2003},
				{image:'images/bigimage2.jpg',title:'Tweeter',pubdate:'2/1/2008',author:'Joe Smith',url:'http://joe.com',desc:'This is yet another twitter app written with titanium.  please enjoy  It was develop in less than two weeks using turtles',id:3,value:2.96,votes:32,hasVoted:false,downloads:403},
				{image:'images/bigimage2.jpg',title:'Tweeter',pubdate:'2/1/2008',author:'Joe Smith',url:'http://joe.com',desc:'This is yet another twitter app written with titanium.  please enjoy  It was develop in less than two weeks using turtles',id:4,value:2.96,votes:32,hasVoted:false,downloads:22212},
				{image:'images/bigimage2.jpg',title:'Tweeter',pubdate:'2/1/2008',author:'Joe Smith',url:'http://joe.com',desc:'This is yet another twitter app written with titanium.  please enjoy  It was develop in less than two weeks using turtles',id:5,value:2.96,votes:32,hasVoted:false,downloads:100},
				{image:'images/bigimage2.jpg',title:'Tweeter',pubdate:'2/1/2008',author:'Joe Smith',url:'http://joe.com',desc:'This is yet another twitter app written with titanium.  please enjoy  It was develop in less than two weeks using turtles',id:6,value:2.96,votes:32,hasVoted:false,downloads:201},
				{image:'images/bigimage2.jpg',title:'Tweeter',pubdate:'2/1/2008',author:'Joe Smith',url:'http://joe.com',desc:'This is yet another twitter app written with titanium.  please enjoy  It was develop in less than two weeks using turtles',id:7,value:2.96,votes:32,hasVoted:false,downloads:10},
				{image:'images/bigimage2.jpg',title:'Tweeter',pubdate:'2/1/2008',author:'Joe Smith',url:'http://joe.com',desc:'This is yet another twitter app written with titanium.  please enjoy  It was develop in less than two weeks using turtles',id:8,value:2.96,votes:32,hasVoted:false,downloads:120},
				{image:'images/bigimage2.jpg',title:'Tweeter',pubdate:'2/1/2008',author:'Joe Smith',url:'http://joe.com',desc:'This is yet another twitter app written with titanium.  please enjoy  It was develop in less than two weeks using turtles',id:9,value:3.6,votes:12,hasVoted:false,downloads:301}

		]})
	}
});
