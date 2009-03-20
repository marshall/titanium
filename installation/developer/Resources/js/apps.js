TiDeveloper.Apps = {};
TiDeveloper.Apps.app_list_url = 'http://localhost/~jhaynie/dist/services/app-list';
TiDeveloper.Apps.app_rate_url = 'http://localhost/~jhaynie/dist/services/app-rate';

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
		var id = $(this).attr('appid');
		var guid = $(this).attr('guid');
		var ratingStr = $('#rating_string_' + id);
		var hasVoted = ratingStr.attr('hasVoted');
		var vote = parseInt($(this).attr('star'));
		var votes = parseInt(ratingStr.attr('votes'));
		var rating = parseInt(ratingStr.attr('rating'));
		
		
		// we only count vote
		if (hasVoted == "true")
		{
			ratingStr.css('display','none');
			$('#already_voted_'+id).fadeIn();
			setTimeout(function()
			{
				$('#already_voted_'+id).css('display','none');
				ratingStr.fadeIn();
				
			},1500);
		}
		else
		{
			var totalRating = (votes * rating) + vote;
			votes++;
			var newRating = String(totalRating/votes).split('.');
			newRating = newRating[0] + "." + newRating[1].substring(0,2);
			
			ratingStr.css('display','none');
			ratingStr.html(newRating+' rating from '+votes+' votes');
			$('#voted_success_'+id).fadeIn();
			setTimeout(function()
			{
				$('#voted_success_'+id).css('display','none');
				ratingStr.fadeIn();
				
			},1500);

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

			//TODO NOTE: rating should be 1-5
			var url = TiDeveloper.make_url(TiDeveloper.Apps.app_rate_url,{
				'rating':vote,
				'guid':guid,
				'mid':Titanium.Platform.id
			});
			
			// submit the new rating
			$.get(url);
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
	});

	// enable mouseout
	$('.rating').mouseout(function()
	{
		var id = $(this).attr('appid');
		var rating = parseInt($('#rating_string_'+id).attr('rating'));
		rating++;
		for (var i=rating;i<6;i++)
		{
			$('#rating_' + id + '_' + i).addClass('rating_off');
		}
	});
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
		var url = TiDeveloper.make_url(TiDeveloper.Apps.app_list_url,{
			'f':'json'
		});
		
		$.getJSON(url,function(result)
		{
			$MQ('l:applist',{'rows':result});
		});
	}
});
