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

	function byteArraytoHexString(byteArray) {
		return Array.from(byteArray, function(byte) {
		  return ('0' + (byte & 0xFF).toString(16)).slice(-2);
		}).join('')
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
					
					if (!docinfo) {
						_pass = _generatePassword();

						let documentRandomHash256 = byteArraytoHexString(nacl.randomBytes(32));

						docinfo = documentRandomHash256 + "|" + nonce + "|" + version;
					}
					else
					{
						let parts = docinfo.split("|");						
						let _documentHash256 = parts[0];

						if (parts.length >= 3)
							version = parts[2];

						_pass = ONLYONET.getFilePassword("0x" + _documentHash256);

						docinfo = _documentHash256 + "|" + nonce + "|" + version;
					}

					AscDesktopEditor.sendSystemMessage({ type : "generatePassword", password : _pass,  docinfo : docinfo });
					
					break;					
				};
				  
				case "getPasswordByFile":  
				{
					let _hashAsHex = "0x" + obj.docinfo.split("|")[0];
					let _pass = ONLYONET.getFilePassword(_hashAsHex);

					if (null == _pass) {
						_hashAsHex = "0x" + _convertToHex(atob(obj.hash)); 	
					 	_pass = ONLYONET.getFilePassword(_hashAsHex);
					}

					if (null == _pass) _pass = "";

					AscDesktopEditor.sendSystemMessage({ type : "getPasswordByFile", password : _pass });

					break;				
				}
				case "setPasswordByFile":
				{
				 	if (obj.isNeedMessage)
					{
						obj.message = ONLYONET.Resources["system-message-save-to-blockchain"];
						AscDesktopEditor.sendSystemMessage(obj);					
					}
					else
					{
						let _hashAsHex = "0x" + obj.docinfo.split("|")[0];

						let _passByHash	= ONLYONET.getFilePassword(_hashAsHex);

						if (_passByHash) {
							if (!_guidsAreEqual(_passByHash, obj.password))
								obj.error = "Password with this hash already exist in blockchain";
		
							AscDesktopEditor.sendSystemMessage(obj);
		
							break;
						}
						
						let _pass = encodeURIComponent(obj.password);				
				
						ONLYONET.saveFilePassword(_hashAsHex, _pass).then(result => {							
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
					obj.account.address = ONLYONET.AppSettings.CurrentUser.enrollmentID;
					obj.account.publicKey = ONLYONET.AppSettings.CurrentUser.publicKeyECDSA;
					
					AscDesktopEditor.sendSystemMessage(obj);

					break;
				}
				case "share":
				{		
					let _hashAsHex = "0x" + obj.docinfo.split("|")[0];		
					let _pass = ONLYONET.getFilePassword(_hashAsHex);
					let _accounts = [];
				
					if (null == _pass) {
						_hashAsHex = "0x" + _convertToHex(atob(obj.hash)); 	
					 	_pass = ONLYONET.getFilePassword(_hashAsHex);
					}	
					
					if (_pass != "" && obj.accounts.addresses.length == 1) {
						obj.result = "OK";
						AscDesktopEditor.sendSystemMessage(obj);

						break;
					}

					$.each(obj.accounts.addresses, function(index, account) {
						if (!ONLYONET.isFilePasswordExist(_hashAsHex, account.publicKey)) 
						{
							_accounts.push(account);
						}
					});

					if (_pass != "") {
						ONLYONET.saveFilePassword(_hashAsHex, _pass, _accounts).then(result => {								
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