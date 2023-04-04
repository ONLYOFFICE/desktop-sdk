(function(){

	window.AscSimpleRequest = window.AscSimpleRequest || {};

	/*
	{
		url: "url",
		crossOrigin: true,
		crossDomain: true,
		timeout: 10000,
		headers: {
			"key" : "value"
		},
		complete: function(e, status) {},
		error: function(e, status, error) {}
	}
	*/

	var simpleRequestCounter = 0;
	var simpleRequestCallback = {};

	window.AscSimpleRequest.createRequest = function(obj) 
	{
		++simpleRequestCounter;

		simpleRequestCallback[simpleRequestCounter] = {
			id : simpleRequestCounter,
			complete : obj.complete,
			error : obj.error
		};

		if (obj.timeout)
		{
			let cSS = simpleRequestCounter;
			simpleRequestCallback[simpleRequestCounter].timer = setTimeout(function(){
				if (simpleRequestCallback[cSS])
				{
					if (simpleRequestCallback[cSS].error)
					{
						simpleRequestCallback[cSS].error({
							status : "error",
							statusCode : 404
						}, "error");
					}

					delete simpleRequestCallback[cSS];
				}

			}, obj.timeout);
		}

		window.AscDesktopEditor.sendSimpleRequest(simpleRequestCounter, obj);
	};

	window.AscSimpleRequest._onSuccess = function(counter, e)
	{
		let obj = simpleRequestCallback[counter];
		if (obj)
		{
			if (obj.timer)
				clearTimeout(obj.timer);

			if (obj.complete)
				obj.complete(e, e.status);

			delete simpleRequestCallback[counter];
		}
	};

	window.AscSimpleRequest._onError = function(counter, e)
	{
		let obj = simpleRequestCallback[counter];
		if (obj)
		{
			if (obj.timer)
				clearTimeout(obj.timer);

			if (obj.error)
				obj.error(e, e.status);

			delete simpleRequestCallback[counter];
		}
	};

})();