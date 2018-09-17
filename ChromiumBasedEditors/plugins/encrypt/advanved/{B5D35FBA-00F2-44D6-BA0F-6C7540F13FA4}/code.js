(function(window, undefined){

	window.isUseLocalStorage = false;
	window.uniAlgCommonPhrase = "ONLYOFFICE Desktop Advanced Encryption";

	const decryptHeader = "DECRYPTED;";
	const encryptHeader = window.AscDesktopEditor.GetEncryptedHeader();

	const decryptHeaderLen = decryptHeader.length;
	const encryptHeaderLen = encryptHeader.length;

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

	// local storage version & random passwords
	function getPasswordByFile(hash, docinfo)
	{
		console.log("pass: " + docinfo);
		if (window.isUseLocalStorage || !docinfo)
		{
			try
			{
				var _ret = localStorage.getItem(hash);
				if (null == _ret)
					_ret = "";
				return decodeURIComponent(_ret);
			}
			catch (err)
			{
				return "";
			}
		}
		else
		{
			try
			{
				return Aes.Ctr.decrypt(docinfo, window.uniAlgCommonPhrase, 256);
			}
			catch (err)
			{
			}
			return "";
		}		
	}

	function setPasswordByFile(hash, pass)
	{
		if (window.isUseLocalStorage)
		{
			try
			{
				return localStorage.setItem(hash, encodeURIComponent(pass));
			}
			catch (err)
			{
				return "";
			}
		}
		else
		{
			// nothing
		}
	}
	
	var currentPassword = "";
	
	function generate_password() 
	{
		var ret = { pass : "", docinfo: "" };
		if (window.isUseLocalStorage)
		{
			currentPassword = generate_password_guid_random();
		}
		else
		{
			currentPassword = generate_password_guid_random();
			ret.docinfo = Aes.Ctr.encrypt(currentPassword, window.uniAlgCommonPhrase, 256);
		}
		ret.pass = currentPassword;
		return ret;
	}

	String.prototype.encryptData = function(objCheck)
    {
        return encryptHeader + Aes.Ctr.encrypt(decryptHeader + this, currentPassword, 256);
	};
	String.prototype.decryptData = function(objCheck)
    {
		if (this.length < (encryptHeaderLen + 1))
			return this;

		var isQuoted = (this.charCodeAt(0) == 34) ? true : false;
		var nOffset = isQuoted ? 1 : 0;

		if (this.substr(nOffset, encryptHeaderLen) != encryptHeader)
			return this;

		var retValue = "";
		try
		{
			retValue = Aes.Ctr.decrypt(this.substr(encryptHeaderLen + nOffset, this.length - encryptHeaderLen - 2 * nOffset), currentPassword, 256);
		}
		catch (err)
		{
			retValue = "";			
		}
		if (retValue.length < decryptHeaderLen || retValue.substr(0, decryptHeaderLen) != decryptHeader)
		{
			objCheck.valid = false;
			return "";
		}

		if (!isQuoted)
			return retValue.substr(decryptHeaderLen);
		
		return "\"" + retValue.substr(decryptHeaderLen) + "\"";
    };

	window.Asc.plugin.init = function(obj)
    {
    	if (!obj)
    		return;

    	switch (obj.type)
		{
			case "generatePassword":
			{
				var _pass = generate_password();
				this.executeMethod("OnEncryption", [{ type : "generatePassword", password : _pass.pass, docinfo : _pass.docinfo }]);
				break;
			}
			case "getPasswordByFile":
			{
				this.executeMethod("OnEncryption", [{ type : "getPasswordByFile", password : getPasswordByFile(obj.hash, obj.docinfo) }]);
				break;
			}
			case "setPasswordByFile":
			{
				this.executeMethod("StartAction", ["Block", "Save to localstorage..."], function() {
					setPasswordByFile(obj.hash, obj.password);
					
					setTimeout(function() {
						// send end action with delay
						window.Asc.plugin.executeMethod("EndAction", ["Block", "Save to localstorage..."]);					
					}, 200);
				});
				break;
			}
			case "encryptData":
			{
				var check = { valid : true };
				for (var i = 0; i < obj.data.length; i++)
					obj.data[i] = obj.data[i].encryptData(check);
				
				this.executeMethod("OnEncryption", [{ type : "encryptData", data : obj.data, check: check.valid }]);
				break;
			}
			case "decryptData":
			{
				var check = { valid : true };
				for (var i = 0; i < obj.data.length; i++)
				{
					if (obj.data[i]["change"])
						obj.data[i]["change"] = obj.data[i]["change"].decryptData(check);
					else
						obj.data[i] = obj.data[i].decryptData(check);
				}
				
				this.executeMethod("OnEncryption", [{ type : "decryptData", data : obj.data, check: check.valid }]);
				break;
			}
			default:
				break;
		}
    };

	window.Asc.plugin.button = function(id)
    {
        this.executeCommand("close", "");
    };

})(window, undefined);
