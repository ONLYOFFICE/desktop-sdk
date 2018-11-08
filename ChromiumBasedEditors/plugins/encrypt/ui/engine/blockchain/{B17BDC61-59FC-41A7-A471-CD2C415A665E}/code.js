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
	
	window.onSystemMessage = function(obj)
	{	
			if (!obj) return;
			
			let isConnected = ONLYONET.isConnected();
		
			switch (obj.type)
			{
				case "generatePassword":
				{
					let _pass = _generatePassword();

					if (!isConnected)
						_pass = "";

					let docinfo =  nacl.util.encodeBase64(nacl.randomBytes(nacl.box.nonceLength));
					
					AscDesktopEditor.sendSystemMessage({ type : "generatePassword", password : _pass,  docinfo : docinfo });
					
					break;					
				};
				  
				case "getPasswordByFile":  
				{
					if (!isConnected)
					{
						AscDesktopEditor.sendSystemMessage({ type : "getPasswordByFile", password : "" });
						
						break;
					}
	
					let _hashAsHex = "0x" + _convertToHex(atob(obj.hash)); 

			 		fileHash = _hashAsHex;
				
					let _pass = ONLYONET.getFilePassword(_hashAsHex);

					if (null == _pass) _pass = "";

					AscDesktopEditor.sendSystemMessage({ type : "getPasswordByFile", password : _pass });

					break;				
				}
				case "setPasswordByFile":
				{
					if (!isConnected)
					{
						AscDesktopEditor.sendSystemMessage(obj);	
						
						break;	
					}					

				 	if (obj.isNeedMessage)
					{
						obj.message = ONLYONET.Resources["system-message-save-to-blockchain"];
						AscDesktopEditor.sendSystemMessage(obj);					
					}
					else
					{
						let _hashAsHex = "0x" + _convertToHex(atob(obj.hash));

						fileHash = _hashAsHex;
	
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

					for (var i = 0; i < obj.data.length; i++)
					{
						obj.data[i] = ONLYONET.encryptData(obj.data[i], obj.password, obj.docinfo);
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

					for (var i = 0; i < obj.data.length; i++)
					{
						if (obj.data[i]["change"])
						{
							obj.data[i]["change"] = ONLYONET.decryptData(obj.data[i]["change"], obj.password, obj.docinfo );
						}
						else
						{
							obj.data[i] = ONLYONET.decryptData(obj.data[i], obj.password, obj.docinfo);
						}
					}	
					
					AscDesktopEditor.sendSystemMessage(obj);

					break;
				}
				case "user":
				{
					obj.account = {};
					obj.account.address = ONLYONET.getAddress();
					obj.account.publicKey = ONLYONET.getPublicKey();	
					
					AscDesktopEditor.sendSystemMessage(obj);

					break;
				}
				case "share":
				{					
					let _hashAsHex = "0x" + _convertToHex(atob(obj.hash));

					let _pass = ONLYONET.getFilePassword(_hashAsHex);
					
					if (_pass != "") {

						let _accounts = [];

						let _addresses = obj.accounts.addresses;

						if (!_addresses)
						_addresses = obj.accounts;

						$.each(_addresses, function(index, account) {
							if (!ONLYONET.isFilePasswordExist(_hashAsHex, account.address)) 
							{
								_accounts.push(account);
							}
						});


						ONLYONET.saveFilePassword(_hashAsHex, _pass, _accounts).then(result => {								
								obj.result = "OK";
								AscDesktopEditor.sendSystemMessage(obj);
							},
							error => {
								obj.result = error;
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