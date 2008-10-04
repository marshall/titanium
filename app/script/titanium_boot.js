
function sleep(millis)
{
	var date = new Date();
	var curDate = null;

	do { curDate = new Date(); }
	while(curDate-date < millis);
} 

Appcelerator.Titanium.Boot = {
	die: function (message) {
		Appcelerator.Titanium.Core.Console.appendError(message);
	},
	
	confirm: function (question, canForce){
		var options = {
			response: null,
			modal: true,
    		buttons: { 
				"Yes": function() {
					this.response = "Yes";
				}, 
				"No": function() {
					this.response = "No";
				},
				"Always": function() {
					this.response = "Always";	
				}
			}
		};
		
		//alert(question);
		$("#confirm_dialog").html(question);
		$("#confirm_dialog").dialog(options);
	},
	
	askForProxy: function () {
		
	},
	
	showLoginForm: function () {
		
	},
	
	showSignupForm: function () {
		var title = 'Welcome to the Appcelerator RIA Platform';
		var message = 'Before we can continue, you will need your Appcelerator Developer Network login ';
		message += 'credentials.  If you have not yet created a (free) developer account, you can ';
		message += 'either visit <a href="http://www.appcelerator.org">http://www.appcelerator.org</a> to create one and return. ';
		message += 'Or, you can create an account now.';
		
		
	}
};