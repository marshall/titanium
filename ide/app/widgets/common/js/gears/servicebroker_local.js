/**
 * this code will execute inside the browser and serves as the local implementation changes
 * to our normal ServiceBroker to support google gears dispatching and reception of remote messages 
 * from a worker pool worker
 */


/**
 * override queue remote to send to workpool
 */
Appcelerator.Util.ServiceBroker.queueRemote = function(msg,callback,scope,version,initial)
{
    var marshaller = Appcelerator.Util.ServiceBrokerMarshaller[Appcelerator.Util.ServiceBroker.marshaller];
    var transportHandler = Appcelerator.Util.ServiceBrokerTransportHandler[Appcelerator.Util.ServiceBroker.transport];
    var payloadObj = marshaller.serialize(msg ? [[msg,null,scope,version]] : [],false);
    var payload = payloadObj.postBody;
    var contentType = payloadObj.contentType;
    var instructions = transportHandler.prepare(Appcelerator.Util.ServiceBroker,initial,payload,contentType);
	google.gears.appcelerator.workerPool.sendMessage(Object.toJSON(instructions),google.gears.appcelerator.serviceBrokerId);
};

Appcelerator.Util.ServiceBroker.$triggerConfig=Appcelerator.Util.ServiceBroker.triggerConfig;
Appcelerator.Util.ServiceBroker.triggerConfig = function()
{
	Appcelerator.Util.ServiceBroker.$triggerConfig();
	/**
	 * setup polling information if polling if configured and the poll will be controlled
	 * directly by the workerpool out of process
	 */
	if (Appcelerator.Util.ServiceBroker.poll && !Appcelerator.Util.ServiceBroker.disabled)
	{
		Appcelerator.Util.ServiceBroker.poll = false; // turn it off since we're going to poll from within gears
        var config = Appcelerator.ServerConfig['servicebroker'];
		var interval = Appcelerator.Util.DateTime.timeFormat(config['poll_interval'] || '30s');
	    var transportHandler = Appcelerator.Util.ServiceBrokerTransportHandler[Appcelerator.Util.ServiceBroker.transport];
	    var instructions = transportHandler.prepare(Appcelerator.Util.ServiceBroker,false,null,'text/xml');
		google.gears.appcelerator.workerPool.sendMessage(Object.toJSON(
			{
				'type':'gears.init',
				'poll':true,
				'interval':interval,
				'url':instructions.url
			}
		),google.gears.appcelerator.serviceBrokerId);
	}
};

Appcelerator.Util.ServiceBroker.gearsResponseRE = new RegExp('<message (.*)\'>(.*)?</message>','g');

/**
 * method will receive worker pool response based on servicebroker remote (received inside worker)
 * and forward to normal servicebroker queue for delivery
 */ 
Appcelerator.Util.ServiceBroker.gearsDispatch = function(msg)
{
	$D('[Gears] receiving from gears remote => '+msg);

	var tok = msg.split('|||');
	var contentType = tok[0];
	var status = tok[1];
	var payload = tok[2];
	
	if (status == 202)
	{
		// this means no content
		return;
	}
	
    var marshaller = Appcelerator.Util.ServiceBroker.marshaller;

	switch (marshaller)
	{
		case 'xml/json':
		{
			var arrMatch = null;

			// we have to use regular expressions to parse response since we don't have XML parser we can use
			// in gears return responseXML for their XHR

			while (arrMatch = Appcelerator.Util.ServiceBroker.gearsResponseRE.exec(payload))
			{
				var parameters={};
				arrMatch[1].split(' ').each(function(a,b)
				{
					var s = a.split('=');
					parameters[s[0]] = s[1].gsub("'",'');
				});
				var cdata = arrMatch[2];
				// strip out <![CDATA[]]> to get JSON payload
				var json = cdata.substring(9,cdata.length-3);
		        var type = parameters['type'];
		        var datatype = parameters['datatype'];
		        var scope = parameters['scope'] || 'appcelerator';
				var version = parameters['version'] || '1.0';
		        var data = null;
		        try
		        {
		            data = json.evalJSON();
		            data.toString = function () { return Object.toJSON(this); };
		        }
		        catch (e)
		        {
		            $E('Error received evaluating: ' + text + ' for type: ' + type + ", error: " + Object.getExceptionDetail(e));
		            return;
		        }
		        $D(this.toString() + ' received remote message, type:' + type + ',data:' + data);
				Appcelerator.Util.ServiceBroker.localMessageQueue.push([type,data,'remote',scope,version]);
			}
			break;
		}
		case 'application/json':
		{
			var response = eval('('+ payload + ')');
			var array = response.messages;
			if (array && array.length > 0)
			{
				for (var c=0;c<array.length;c++)
				{
					var entry = array[c];
					var type = entry.type;
					var data = entry.data;
					var scope = entry.scope;
					var version = entry.version;
					Appcelerator.Util.ServiceBroker.localMessageQueue.push([type,data,'remote',scope,version]);
				}
			}
			break;
		}
	}

};

