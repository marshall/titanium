var workerPool = google.gears.factory.create('beta.workerpool');

var workerActions = {
	"confirm": function(body) { Appcelerator.Titanium.Boot.confirm(body.question, body.canForce); },
	"confirm_install_components": function(body) { Appcelerator.Titanium.Boot.confirmInstallComponents(body.components); },
	"show_login_form": function(body) { Appcelerator.Titanium.Boot.showLoginForm(); },
	"show_login_form_error": function(body) { Appcelerator.Titanium.Boot.showLoginFormError(body.error); },
	"show_verification_form": function(body) { Appcelerator.Titanium.Boot.showVerificationForm(); },
	"show_verification_form_error": function(body) { Appcelerator.Titanium.Boot.showVerificationFormError(body.error); },
	"finished_creating_project": function(body) { Appcelerator.Titanium.Boot.finishedCreatingProject(body.projectName, body.directory); },
	"info": function(body) { console.info(body.text); },
	"debug": function(body) { console.debug(body.text); },
	"error": function(body) { console.error(body.text); },
	"open_progress": function(body) { Appcelerator.Titanium.Boot.openProgress(body.id, body.max); },
	"set_progress_message": function(body) { Appcelerator.Titanium.Boot.setProgressMessage(body.id, body.message); },
	"increment_progress": function(body) { Appcelerator.Titanium.Boot.incrementProgress(body.id, body.amount); },
	"close_progress": function(body) { Appcelerator.Titanium.Boot.closeProgress(body.id); }
};

workerPool.onmessage = function(a, b, message) {
	var boot = Appcelerator.Titanium.Boot;
	
	if ("action" in message.body) {
		var action = workerActions[message.body.action];
		action(message.body);
	}
};

var bootWorkerId = workerPool.createWorkerFromUrl('script/titanium_boot_worker.js');
$("#confirm_dialog,#login_dialog").dialog();
$("#confirm_dialog,#login_dialog").dialog("close");

function sendResponse (action, response)
{
	workerPool.sendMessage({action: action, response: response}, bootWorkerId);
}

var progress = {};

Appcelerator.Titanium.Boot = {
	die: function (message) {
		Appcelerator.Titanium.Core.Console.appendError(message);
	},
	
	debug: function (message) {
		if (Appcelerator.Titanium.isDebugging)
		{
			Appcelerator.Titanium.Core.Console.appendDebug(message);
		}
	},
	
	confirmResponse: function(response) {	
		$("#confirm_dialog").dialog("close");
		console.debug("send confirm_response to: " + bootWorkerId);
		
		sendResponse("confirm_response", response);
	},
	
	confirm: function (question, canForce) {
		console.debug("confirm dialog: " + question);
		var options = {
			modal: true,
   			buttons: { 
				"Yes": function() {
					Appcelerator.Titanium.Boot.confirmResponse("Yes");
				}, 
				"No": function() {
					Appcelerator.Titanium.Boot.confirmResponse("No");
				},
				"Always": function() {
					Appcelerator.Titanium.Boot.confirmResponse("Always");
				}
			},
			overlay: { 
	        	opacity: 0.5, 
	        	background: "black" 
	    	}
		};
	
		//alert(question);
		$("#confirm_dialog").dialog(options);
		$("#confirm_dialog").html(question);
		$("#confirm_dialog").dialog("open");
		
	},
	
	askForProxy: function () {
		
	},
	
	loginResponse: function(response) {
		$("#login_dialog").dialog("close");
		var r = { response: response };
		
		if (response == "Login")
		{
			r.username = $("#login_username").val();
			r.password = $("#login_password").val();
		}
		
		sendResponse("login_form_response", r);
	},
	
	showLoginForm: function () {
		var options = {
			modal: true,
   			buttons: { 
				"Login": function() {
					Appcelerator.Titanium.Boot.loginResponse("Login");
				}, 
				"Cancel": function() {
					Appcelerator.Titanium.Boot.loginResponse("Cancel");
				}
			},
			overlay: { 
	        	opacity: 0.5, 
	        	background: "black" 
	    	}
		};
		
		$("#login_dialog").dialog(options);
		$("#login_dialog").dialog("open");
	},
	
	showSignupForm: function () {
	},
	
	finishedCreatingProject: function (projectName, directory) {

		$('#new_project_create_button').attr('disabled', null);
		$('#new_project_dialog').dialog("close");
		
		var project = new TiProject(projectName, directory, "ruby");
		if (! ('projects' in Appcelerator.Titanium)) {
			Appcelerator.Titanium.projects = [];
		}
		
		Appcelerator.Titanium.projects.push(project);
		project.addToTree();
		
		tiEditor.openFile(project.getFileAtPath('public/index.html'));
		
		TiProject.saveProjects();
	},
	
	openProgress: function (id, max)
	{
		$("#"+id).progressBar(0);
		$("#"+id).fadeIn();
		
		progress[id] = {current: 0, max: max};
	},
	
	setProgressMessage: function (id, message)
	{
		if (message != null) {
			$("#"+id+"_message").html(message);
		}
	},
	
	incrementProgress: function (id, amount)
	{
		progress[id].current += amount;
		var percent = Math.floor((progress[id].current/progress[id].max) * 100);
		
		console.debug("id="+id+",amount="+amount+",current="+progress[id].current+",percent="+percent);
		$("#"+id).progressBar(percent);
	},
	
	closeProgress: function (id)
	{
		progress[id] = null;
		$("#" + id).fadeOut();
		//$("#"+id).css('display', 'none');
	}
};
