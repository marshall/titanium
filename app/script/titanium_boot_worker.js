var wp = google.gears.workerPool;
var appCommand = google.gears.factory.create("beta.appcommand");

function _debug (x)
{
	wp.sendMessage({action: "debug", text: x}, 0);
}

var bootWrapper = {
	waiting: true,
	response: null,
	
	waitForAction: function (message)
	{
		_debug("wait for action: " + message.action);
		
		bootWrapper.waiting = true;
		wp.sendMessage(message, 0);
		
		while (bootWrapper.waiting) {
			appCommand.Sleep(100);
			wp.processMessages();
		}
		
		return bootWrapper.response;	
	},
	
	confirm: function (question, canForce)
	{
		return bootWrapper.waitForAction({
			action: "confirm", question: question, canForce: canForce});
	},
	
	confirmInstallComponents: function (components)
	{
		return bootWrapper.waitForAction({
			action: "confirm_install_components", components: components});
	},
	
	showLoginForm: function()
	{
		return bootWrapper.waitForAction({action: "show_login_form"});
	},
	
	showLoginFormError: function(error)
	{
		wp.sendMessage({action: "show_login_form_error", error: error}, 0);
	},
	
	showVerificationForm: function ()
	{
		return bootWrapper.waitForAction({action: "show_verification_form"});
	},
	
	showVerificationFormError: function (error)
	{
		wp.sendMessage({action: "show_verification_form_error", error: error}, 0);
	},
	
	die: function (message)
	{
		bootWrapper.error(message);
	},
	
	info: function (message)
	{
		wp.sendMessage({action: "info", text: message}, 0);
	},
	
	debug: function (message)
	{
		_debug(message);
	},
	
	error: function (message)
	{
		wp.sendMessage({action: "error", text: message}, 0);
	},
	
	openProgress: function (id, max)
	{
		wp.sendMessage({action: "open_progress", id: id, max: max}, 0);
	},
	
	setProgressMessage: function (id, message)
	{
		wp.sendMessage({action: "set_progress_message", id: id, message: message}, 0);
	},
	
	incrementProgress: function (id, amount)
	{
		wp.sendMessage({action: "increment_progress", id: id, amount: amount}, 0);
	},
	
	closeProgress: function (id)
	{
		wp.sendMessage({action: "close_progress", id: id}, 0);
	},
	
	finishedCreatingProject: function (projectName, directory)
	{
		wp.sendMessage({action: "finished_creating_project", projectName: projectName, directory: directory}, 0);
	}
};

wp.onmessage = function(a, b, message) {
	var action = message.body.action;
	
	_debug("worker action: " + action);
	if (action == "app_command") {
		var options = message.body.options;
		var path = message.body.path;
		
		_debug("calling app, path="+path);
		appCommand.App(path, options, bootWrapper);
	}
	else if (action.match(/_response$/)) {
		bootWrapper.response = message.body.response;
		bootWrapper.waiting = false;
	}
}