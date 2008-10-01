
Appcelerator.Titanium.Boot = {
	_options: {force: false},
	dieHandler: null,
	
	die: function (message, exitValue) {
		if (Appcelerator.Titanium.Boot.dieHandler != null) {
			Appcelerator.Titanium.Boot.dieHandler(message);
		}
	},
	
	askWithValidation: function (question, message, regex, mask) {
		var response = null;
		while (true) {
			response = ask(question, mask);
			if (response.match(regex)) {
				break;
			}
			if (response != null) {
				Appcelerator.Titanium.Core.Console.appendError(message);
			}
		}
		return response;
	},
	
	ask: function (question, mask) {
		return Appcelerator.Titanium.Core.askQuestion(question, mask);
	},
	
	confirm: function (question, canForce, dieIfFails, defaultAnswer){
		var answer = Appcelerator.Titanium.Core.askYesNoQuestion(question);
		
		if (answer == null || answer == '' || !answer) {
			answer = defaultAnswer;
		}
		
		if (answer.autoConfirm) {
			_options.force = true;
		}
		
		if (dieIfFails && !answer.response) {
			Appcelerator.Titanium.Boot.die("Cancelled by User", -1);
			return false;
		}
		
		return answer.response;
	},
	
	createNetworkAccount: function () {
		var title = 'Welcome to the Appcelerator RIA Platform';
		var message = 'Before we can continue, you will need your Appcelerator Developer Network login ';
		message += 'credentials.  If you have not yet created a (free) developer account, you can ';
		message += 'either visit <a href="http://www.appcelerator.org">http://www.appcelerator.org</a> to create one and return. ';
		message += 'Or, you can create an account now.';
		
		
	}
};