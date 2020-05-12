(function(window, undefined){
	
	window.DocInfoWorker = {};
	window.DocInfoWorker.isUseOnePasswordOnAllVersions = false;
	window.DocInfoWorker.TmpPassword = "";
	window.DocInfoWorker.User = null;
	
	// work with docinfo
	// create from passwords
	window.DocInfoWorker.generateDocInfo = function(users, password)
	{
		var result = "";
		for (var i = 0, len = users.length; i < len; i++)
		{
			if (users[i].publicKey)
			{
				result += (users[i].userId + "\n");
				result += (window.AscDesktopEditor.CryproRSA_EncryptPublic(users[i].publicKey, password) + "\n\n");
			}				
		}
		return result;
	};
	// decrypt password from docinfo
	window.DocInfoWorker.readPassword = function(docinfo)
	{
		var user = window.DocInfoWorker.User[2];
		var index = docinfo.indexOf(user);
		
		if (-1 == index)
			return "";
		
		var delimeter = "<!--break-->";
		var start = docinfo.indexOf(delimeter, index);
		if (-1 == index)
			return "";
		
		var end = docinfo.indexOf(delimeter, start + 1);
		if (-1 == end)
			return "";
		
		var encPassword = docinfo.substring(start + delimeter.length, end);
		return window.AscDesktopEditor.CryproRSA_DecryptPrivate(window.DocInfoWorker.User[0], encPassword);
	};
	
	// генерация нового пароля
	window.DocInfoWorker.createPasswordNew = function() {
		function s4() {
			return Math.floor((1 + Math.random()) * 0x10000).toString(16).substring(1);
		}
		return s4() + s4() + '-' + s4() + '-' + s4() + '-' + s4() + '-' + s4() + s4() + s4();
	}
	// запрос на новый пароль
	window.DocInfoWorker.createPassword = function()
	{
		var password = window.Asc.plugin.documentPassword;
		if (null == password || "" == password || true !== this.isUseOnePasswordOnAllVersions)
			password = window.DocInfoWorker.createPasswordNew();
		return password;
	};
	
	DocInfoWorker.generatePassword = function()
	{
		this.TmpPassword = this.createPassword();
				
		window.AscDesktopEditor.cloudCryptoCommandMainFrame({ name: "window.cloudCryptoCommand", type: "getsharingkeys" }, function(obj){
			
			window.Asc.plugin.onEngineSystemMessage({ 
				type : "generatePassword", 
				password : window.DocInfoWorker.TmpPassword, 
				docinfo : window.DocInfoWorker.generateDocInfo(obj.keys, window.DocInfoWorker.TmpPassword)
			});
			
		});
	};
	
	window.engine = {};

	window.engine.sendSystemMessage = function(obj)
	{
		if (!window.DocInfoWorker.User)
			window.DocInfoWorker.User = window.AscDesktopEditor.CryptoCloud_GetUserInfo();
		
		switch (obj.type)
		{
			case "generatePassword":
			{
				window.DocInfoWorker.generatePassword();
				break;
			}
			case "getPasswordByFile":
			{
				window.Asc.plugin.onEngineSystemMessage({ 
					type : "getPasswordByFile", 
					password : window.DocInfoWorker.readPassword(obj.docinfo)
				});
 				break;
			}
			case "setPasswordByFile":
			{
				if (obj.isNeedMessage)
				{
					obj.message = "Save to storage...";
					window.Asc.plugin.onEngineSystemMessage(obj);					
				}
				else
				{
					// send message on end
					setTimeout(function() {
						// timer only for visibility saving message (delay)
						window.Asc.plugin.onEngineSystemMessage(obj);
					}, 500);
				}
				break;
			}
			case "encryptData":
			{
				if (obj.password)
				{
					for (var i = 0; i < obj.data.length; i++)
					{
						obj.data[i] = window.AscDesktopEditor.CryptoAES_Encrypt(obj.data[i]);
					}
				}
				
				window.Asc.plugin.onEngineSystemMessage(obj);
				break;
			}
			case "decryptData":
			{
				if (obj.password)
				{
					for (var i = 0; i < obj.data.length; i++)
					{
						obj.data[i] = window.AscDesktopEditor.CryptoAES_Decrypt(obj.data[i]);
					}
				}
				
				window.Asc.plugin.onEngineSystemMessage(obj);
				break;
			}
			default:
			{
				window.Asc.plugin.onEngineSystemMessage(obj);
				break;
			}
		}
	};
	
})(window, undefined);
