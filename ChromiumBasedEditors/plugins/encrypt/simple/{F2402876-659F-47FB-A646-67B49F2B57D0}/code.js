(function(window, undefined){

	const decryptHeader = "DECRYPTED;";
	const encryptHeader = window.AscDesktopEditor.GetEncryptedHeader();

	const decryptHeaderLen = decryptHeader.length;
	const encryptHeaderLen = encryptHeader.length;	
	
	// desktop editor simple mode
	var currentPassword = "";
	function generate_password()
	{
		currentPassword = window.AscDesktopEditor.CryptoPassword;
		return currentPassword;
	}

	function getPasswordByFile(hash)
	{
		currentPassword = generate_password();
		return currentPassword;
	}

	function setPasswordByFile(hash, pass)
	{
		// none (simple password for all files)
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
				this.executeMethod("OnEncryption", [{ type : "generatePassword", password : generate_password() }]);
				break;
			}
			case "getPasswordByFile":
			{
				this.executeMethod("OnEncryption", [{ type : "getPasswordByFile", password : getPasswordByFile(obj.hash) }]);
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
