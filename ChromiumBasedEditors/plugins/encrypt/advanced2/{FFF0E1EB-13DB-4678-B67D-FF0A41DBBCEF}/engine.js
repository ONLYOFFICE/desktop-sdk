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
		var _pass = generate_password_guid_random();
		return { pass : _pass, docinfo : "" };
	}
	
	window.engine.sendSystemMessage = function(obj)
	{
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
				// TODO:
				// 1) read docinfo & find record: publicKey & crypted password
				// 2) decrypt password with privateKey				
				window.Asc.plugin.onEngineSystemMessage({ type : "getPasswordByFile", password : "" });
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
