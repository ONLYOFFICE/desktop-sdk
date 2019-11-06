(function(window, undefined){

	function _generatePassword()
	{
		function s4() {
			return Math.floor((1 + Math.random()) * 0x10000).toString(16).substring(1);
		}
		return s4() + s4() + '-' + s4() + '-' + s4() + '-' + s4() + '-' + s4() + s4() + s4();
	}

	function _convertToHex(str) {
		var hex = '';
		
		for(var i=0;i<str.length;i++) {
			hex += ''+str.charCodeAt(i).toString(16);
		}

		return hex;
	}

	function _byteToHexString(buffer) {
		return Array
			.from (new Uint8Array (buffer))
			.map (b => b.toString (16).padStart (2, "0"))
			.join ("");
	}
	

	function _getLocation(href) {
		let match = href.match(/^(https?\:)\/\/(([^:\/?#]*)(?:\:([0-9]+))?)([\/]{0,1}[^?#]*)(\?[^#]*|)(#.*|)$/);

		return match && {
			href: href,
			protocol: match[1],
			host: match[2],
			hostname: match[3],
			port: match[4],
			pathname: match[5],
			search: match[6],
			hash: match[7]
		}
	}

	function _getRequestUrl(href) {
		let parts = _getLocation(href);

		let result = parts.protocol + "//" + parts.hostname;

		if (typeof parts.port !== 'undefined')
			result += parts.port;

		return result;		
	}

	function _guidsAreEqual(left, right) {
		var rgx = /[\{\-\}]/g;
		var txtLeft = left.replace(rgx, '').toUpperCase();
		var txtRight = right.replace(rgx, '').toUpperCase();
		return txtLeft === txtRight;
	};

	window.onSystemMessage = function(obj)
	{	
			if (!obj) return;
		
			switch (obj.type)
			{
				case "generatePassword":
				{
					let _pass = "";
					let docinfo = obj.docinfo;

					let nonce = nacl.util.encodeBase64(nacl.randomBytes(nacl.box.nonceLength));
					let version = ONLYONET.AppSettings.version;
					let _request_url = _getRequestUrl(obj.url);
			
					if (!docinfo) {
						_pass = _generatePassword();

						let documentRandomHash256 = _byteToHexString(nacl.randomBytes(32));

						docinfo = documentRandomHash256 + "|" + nonce + "|" + version;
					}
					else
					{
						let parts = docinfo.split("|");						
						let _documentHash256 =  parts[0];

						if (parts.length >= 3)
							version = parts[2];

						_pass = ONLYONET.getFilePassword(_request_url, "0x" + _documentHash256);

						docinfo = _documentHash256 + "|" + nonce + "|" + version;
					}
					
					AscDesktopEditor.sendSystemMessage({ type : "generatePassword", password : _pass,  docinfo : docinfo });
					
					break;					
				}

				case "getPasswordByFile":  
				{	
					let _request_url = _getRequestUrl(obj.url);
					let _hashAsHex = "0x" + obj.docinfo.split("|")[0];				
					let _pass = ONLYONET.getFilePassword(_request_url, _hashAsHex);

					if (null == _pass) _pass = "";
			
					AscDesktopEditor.sendSystemMessage({ type : "getPasswordByFile", password : _pass });

					break;				
				}

				case "setPasswordByFile":
				{
				 	if (obj.isNeedMessage)
					{
						obj.message = ONLYONET.Resources["system-message-save-to-database"];
						AscDesktopEditor.sendSystemMessage(obj);					
					}
					else
					{
						let _hashAsHex = "0x" + obj.docinfo.split("|")[0];
						let _request_url = _getRequestUrl(obj.url);
						let _passByHash	= ONLYONET.getFilePassword(_request_url, _hashAsHex);
					
						if (_passByHash) {
							if (!_guidsAreEqual(obj.password,_passByHash))
								throw "temp";
							 
							AscDesktopEditor.sendSystemMessage(obj);
		
							break;
						}

						let _pass = encodeURIComponent(obj.password);	
						let _publicKey = ONLYONET.getPublicKey();

						ONLYONET.saveFilePassword(_request_url, _hashAsHex, _pass, [ _publicKey ]).then(result => {							
							AscDesktopEditor.sendSystemMessage(obj);
						},
						error => {							
							obj.error = error.message;
							AscDesktopEditor.sendSystemMessage(obj);
						});					
					}

					break;					
				}

				case "encryptData":
				{	
					if (!obj.password)
					{
						AscDesktopEditor.sendSystemMessage(obj);
						break;
					}
					
					let docInfoParts = obj.docinfo.split("|");
					let nonce = "";

					if (docInfoParts.length >= 2)
						nonce = docInfoParts[1];
					else
						nonce = docInfoParts[0];	

					for (var i = 0; i < obj.data.length; i++)
					{
						obj.data[i] = ONLYONET.encryptData(obj.data[i], obj.password, nonce);
					}	
					
					AscDesktopEditor.sendSystemMessage(obj);

					break;
				}
				case "decryptData":
				{
					if (!obj.password)
					{
						AscDesktopEditor.sendSystemMessage(obj);
						
						break;
					}	
					
					let docInfoParts = obj.docinfo.split("|");
					let nonce = "";

					if (docInfoParts.length >= 2)
						nonce = docInfoParts[1];
					else
						nonce = docInfoParts[0];		

					for (var i = 0; i < obj.data.length; i++)
					{
						if (obj.data[i]["change"])
						{
							obj.data[i]["change"] = ONLYONET.decryptData(obj.data[i]["change"], obj.password, nonce);
						}
						else
						{
							obj.data[i] = ONLYONET.decryptData(obj.data[i], obj.password, nonce);
						}
					}	
					
					AscDesktopEditor.sendSystemMessage(obj);

					break;
				}
				case "user":
				{
					obj.account = {};
					obj.account.address = ONLYONET.getPublicKey();
					obj.account.publicKey = ONLYONET.getPublicKey();

					AscDesktopEditor.sendSystemMessage(obj);

					break;
				}

				case "share":
				{	
					let _hashAsHex = "0x" + obj.docinfo.split("|")[0];				
					let _requestUrl = _getRequestUrl(obj.url);
					let _pass = ONLYONET.getFilePassword(_requestUrl, _hashAsHex);
										
					if (_pass != "") {

						let _publicKeys = [];

						$.each(obj.accounts.addresses, function(index, account) {
							let _publicKey =  account.publicKey;
							
							if (ONLYONET.getFilePassword(_requestUrl, _hashAsHex, _publicKey) == "") 
							{
								_publicKeys.push(_publicKey);
							}
						});

						if (_publicKeys.length == 0) {
							obj.result = "OK";
							AscDesktopEditor.sendSystemMessage(obj);
						}							 

						ONLYONET.saveFilePassword(_requestUrl, _hashAsHex, _pass, _publicKeys).then(result => {								
								obj.result = "OK";
								AscDesktopEditor.sendSystemMessage(obj);
							},
							error => {
								obj.result = JSON.stringify(error);
								AscDesktopEditor.sendSystemMessage(obj);
							}						 
						);						
					}
					else {						
						obj.result = "[Error]: Could not get password";
						AscDesktopEditor.sendSystemMessage(obj);
					}

					break;
				}
				default:
					break;
			}		
	};
	
	window.onChangeEditorsCount = function(isEditorsPresent)
	{
		ONLYONET.UI.isEditorsPresent = isEditorsPresent;
	};

})(window, undefined);