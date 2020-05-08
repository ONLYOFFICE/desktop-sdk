(function(window, undefined){
	
	window.engine = {};

	function generate_password_guid_random()
	{
		function s4() {
			return Math.floor((1 + Math.random()) * 0x10000).toString(16).substring(1);
		}
		return s4() + s4() + '-' + s4() + '-' + s4() + '-' + s4() + '-' + s4() + s4() + s4();
	}

	function generate_password() 
	{
		if (!window.engine.user)
			return { pass : "", docinfo : "" };
		
		var ret = { pass : generate_password_guid_random(), docinfo : "" };
		
		// TODO: all keys
		ret.docinfo = window.AscDesktopEditor.CryproRSA_EncryptPublic(window.engine.user[1], ret.pass);
		
		return ret;
	}
	
	window.engine.sendSystemMessage = function(obj)
	{
		if (!window.engine.user)
			window.engine.user = window.AscDesktopEditor.CryptoCloud_GetUserInfo();
		
		switch (obj.type)
		{
			case "generatePassword":
			{
				var _pass = generate_password();
				window.Asc.plugin.onEngineSystemMessage({ type : "generatePassword", password : _pass.pass, docinfo : _pass.docinfo });
				break;
			}
			case "getPasswordByFile":
			{
				var encPasswordForUser = obj.docinfo; // TODO: parse for user
				var _pass = "";
				if (window.engine.user)
					_pass = window.AscDesktopEditor.CryproRSA_DecryptPrivate(window.engine.user[0], encPasswordForUser);
				
				window.Asc.plugin.onEngineSystemMessage({ type : "getPasswordByFile", password : _pass });
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
					// saving...
					
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
