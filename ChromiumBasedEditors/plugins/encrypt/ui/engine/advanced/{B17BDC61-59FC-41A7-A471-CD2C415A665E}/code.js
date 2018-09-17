(function(window, undefined){

	window.uniAlgCommonPhrase = "ONLYOFFICE Desktop Advanced Encryption";
	
	// passwords generate functions
	// 1st variang for password generation
	function generate_password_guid_random()
	{
		function s4() {
			return Math.floor((1 + Math.random()) * 0x10000).toString(16).substring(1);
		}
		return s4() + s4() + '-' + s4() + '-' + s4() + '-' + s4() + '-' + s4() + s4() + s4();
	}

	// 2nd variang for password generation
	function generate_password_string_random()
	{
		function generatePassword(length) {
			var password = '', character; 
			while (length > password.length) 
			{
				if (password.indexOf(character = String.fromCharCode(Math.floor(Math.random() * 94) + 33), Math.floor(password.length / 94) * 94) < 0) 
				{				
					password += character;
				}
			}
			return password;
		}
		
		return generatePassword(32);
	}

	function generate_password() 
	{
		var _pass = generate_password_guid_random();
		return { pass : _pass, docinfo : Aes.Ctr.encrypt(_pass, window.uniAlgCommonPhrase, 256) };
	}
	
	window.onSystemMessage = function(obj)
	{
		switch (obj.type)
		{
			case "generatePassword":
			{
				var _pass = generate_password();
				AscDesktopEditor.sendSystemMessage({ type : "generatePassword", password : _pass.pass, docinfo : _pass.docinfo });
				break;
			}
			case "getPasswordByFile":
			{
				AscDesktopEditor.sendSystemMessage({ type : "getPasswordByFile", password : Aes.Ctr.decrypt(obj.docinfo, window.uniAlgCommonPhrase, 256) });
				break;
			}
			case "setPasswordByFile":
			{
				if (obj.isNeedMessage)
				{
					obj.message = "Save to storage...";
					AscDesktopEditor.sendSystemMessage(obj);					
				}
				else
				{
					// saving...
					
					// send message on end
					setTimeout(function() {
						// timer only for visibility saving message (delay)
						AscDesktopEditor.sendSystemMessage(obj);
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
						obj.data[i] = Aes.Ctr.encrypt(obj.data[i], obj.password, 256);
					}
				}
				
				AscDesktopEditor.sendSystemMessage(obj);
				break;
			}
			case "decryptData":
			{
				if (obj.password)
				{
					for (var i = 0; i < obj.data.length; i++)
					{
						obj.data[i] = Aes.Ctr.decrypt(obj.data[i], obj.password, 256);
					}
				}
				
				AscDesktopEditor.sendSystemMessage(obj);
				break;
			}
			default:
			{
				AscDesktopEditor.sendSystemMessage(obj);
				break;
			}
		}
	};
	
})(window, undefined);
