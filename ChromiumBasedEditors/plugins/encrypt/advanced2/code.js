(function(window, undefined){

	const decryptHeader = "DECRYPTED;";
	const encryptHeader = window.AscDesktopEditor.GetEncryptedHeader();

	const decryptHeaderLen = decryptHeader.length;
	const encryptHeaderLen = encryptHeader.length;	
	
	window.Asc.plugin.init = function(obj)
    {
    	if (!obj)
    		return;

    	switch (obj.type)
		{
			case "setPassword":
			{			
				this.documentHash = obj.hash;
				this.documentInfo = obj.docinfo;
				this.documentPassword = obj.password;
				window.AscDesktopEditor.CryptoAES_Init(this.documentPassword);
				break;
			}
			case "generatePassword":
			{
				obj.hash = this.documentHash;
				obj.docinfo = this.documentInfo;
				this.sendSystemMessage(obj);
				break;
			}
			case "getPasswordByFile":
			{
				this.documentHash = obj.hash;
				this.documentInfo = obj.docinfo;
				this.sendSystemMessage(obj);
				break;
			}
			case "setPasswordByFile":
			{
				this.documentHash = obj.hash;
				obj.docinfo = this.documentInfo;
				obj.isNeedMessage = true;
				this.sendSystemMessage(obj);
				break;
			}
			case "encryptData":
			{
				this.encryptData = obj;
				for (var i = 0; i < obj.data.length; i++)
					obj.data[i] = decryptHeader + obj.data[i];
				
				obj.hash = this.documentHash;
				obj.docinfo = this.documentInfo;
				obj.password = this.documentPassword;
				this.sendSystemMessage(obj);
				break;
			}
			case "decryptData":
			{
				this.decryptData = obj;
				this.isQuoted = false;
				this.isChange = false;
				if (obj.data.length > 0)
				{
					this.isChange = obj.data[0]["change"] ? true : false;
					
					if (this.isChange)
						this.isQuoted = (obj.data[0]["change"].charCodeAt(0) == 34) ? true : false;
					else
						this.isQuoted = (obj.data[0].charCodeAt(0) == 34) ? true : false;
				}
				
				var nOffset = this.isQuoted ? 1 : 0;
				
				var objSend = { type : "decryptData", data : [] };
				var isPassword = true;
				for (var i = 0; i < obj.data.length; i++)
				{
					var _data = this.isChange ? obj.data[i]["change"] : obj.data[i];
					if (_data.length < (encryptHeaderLen + 1) || _data.substr(nOffset, encryptHeaderLen) != encryptHeader)
					{
						isPassword = false;
					}
					
					if (isPassword)
					{
						// изменения зашифрованы
						objSend.data.push(_data.substr(encryptHeaderLen + nOffset, _data.length - encryptHeaderLen - 2 * nOffset));
					}
					else
					{
						// пришли незашифрованные данные (даже без хедера - т.е. из клиента без включенного crypto mode)
						// добавим хедер и отправим расшифровку без пароля (на onSystemMessage этот хедер удалится)
						objSend.data.push(decryptHeader + _data.substr(nOffset, _data.length - 2 * nOffset));
					}
				}
		
				objSend.hash = this.documentHash;
				objSend.docinfo = this.documentInfo;
				objSend.password = isPassword ? this.documentPassword : "";
				this.sendSystemMessage(objSend);
				break;
			}
			default:
				break;
		}
    };
	
	window.Asc.plugin.sendSystemMessage = function(obj)
	{
		switch (obj.type)
		{
			case "generatePassword":
			{
				// async... callback inside worker (get sharing info)
				window.AscCrypto.CryptoWorker.generatePassword(this.documentPassword);
				break;
			}
			case "getPasswordByFile":
			{
				this.onSystemMessage({ 
					type : "getPasswordByFile", 
					password : window.AscCrypto.CryptoWorker.readPassword(obj.docinfo)
				});
 				break;
			}
			case "setPasswordByFile":
			{
				if (obj.isNeedMessage)
				{
					obj.message = "Save to storage...";
					this.onSystemMessage(obj);					
				}
				else
				{
					// send message on end
					setTimeout(function() {
						// timer only for visibility saving message (delay)
						window.Asc.plugin.onSystemMessage(obj);
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
						obj.data[i] = window.AscCrypto.CryptoWorker.encrypt(obj.data[i]);
					}
				}
				
				this.onSystemMessage(obj);
				break;
			}
			case "decryptData":
			{
				if (obj.password)
				{
					for (var i = 0; i < obj.data.length; i++)
					{
						obj.data[i] = window.AscCrypto.CryptoWorker.decrypt(obj.data[i]);
					}
				}
				
				this.onSystemMessage(obj);
				break;
			}
			default:
			{
				this.onSystemMessage(obj);
				break;
			}
		}
	};
	
	window.Asc.plugin.executeMethodSync = function(name, arg)
	{
		if (undefined == this.stackMethods)
			this.stackMethods = [];
			
		if (this.isWaitMethod)
		{
			this.stackMethods.push([name, arg]);
			return;
		}
		
		this.executeMethod(name, arg, this.executeMethodSyncCallback);
	};
	
	window.Asc.plugin.executeMethodSyncCallback = function()
	{
		var _this = window.Asc.plugin;
		if (_this.stackMethods.length > 0)
		{
			var _ret = _this.stackMethods[0];
			_this.stackMethods.splice(0, 1);
			
			_this.executeMethod(_ret[0], _ret[1], _this.executeMethodSyncCallback);
		}
	};		
	
	window.Asc.plugin.onSystemMessage = function(e)
    {
        switch (e.type)
		{
			case "generatePassword":
			{
				this.documentInfo = e.docinfo;
				this.documentPassword = e.password;
				window.AscCrypto.CryptoWorker.cryptInit(this.documentPassword);
				this.executeMethodSync("OnEncryption", [{ type : "generatePassword", password : e.password, docinfo : e.docinfo, error : (e.docinfo !== "") ? "" : "no_build" }]);
				break;
			}
			case "getPasswordByFile":
			{
				this.documentPassword = e.password;
				window.AscCrypto.CryptoWorker.cryptInit(this.documentPassword);
				this.executeMethodSync("OnEncryption", [{ type : "getPasswordByFile", password : e.password, docinfo : this.documentInfo, hash : this.documentHash }]);
				break;
			}
			case "setPasswordByFile":
			{
				if (e.isNeedMessage)
				{
					this.message = e.message;
					this.executeMethodSync("StartAction", ["Block", this.message]);
					
					delete e.isNeedMessage;
					delete e.message;
					
					this.sendSystemMessage(e);
				}
				else
				{
					this.executeMethodSync("EndAction", ["Block", this.message, e.error]);
				}				
				break;
			}
			case "encryptData":
			{
				for (var i = 0; i < e.data.length; i++)
				{
					if (e.data[i].length >= decryptHeaderLen && e.data[i].substr(0, decryptHeaderLen) == decryptHeader)
					{
						// если начало - с decryptHeader - то не зашифровалось ничего (нет пароля).
						// поэтому убираем все дописки, чтобы поняли все клиенты
						this.encryptData.data[i] = e.data[i].substr(decryptHeaderLen);
					}
					else
					{
						this.encryptData.data[i] = encryptHeader + e.data[i];
					}
				}
				
				this.executeMethodSync("OnEncryption", [{ type : "encryptData", data : this.encryptData.data, check: true }]);
				this.encryptData = null;
				break;
			}
			case "decryptData":
			{
				var check = true;
				for (var i = 0; i < e.data.length; i++)
				{
					if (e.data[i].length < decryptHeaderLen || e.data[i].substr(0, decryptHeaderLen) != decryptHeader)
					{
						check = false;
						break;
					}

					if (!this.isQuoted)
					{
						if (this.isChange)
							this.decryptData.data[i]["change"] = e.data[i].substr(decryptHeaderLen);
						else
							this.decryptData.data[i] = e.data[i].substr(decryptHeaderLen);
					}
					else
					{
						if (this.isChange)
							this.decryptData.data[i]["change"] = ("\"" + e.data[i].substr(decryptHeaderLen) + "\"");
						else
							this.decryptData.data[i] = ("\"" + e.data[i].substr(decryptHeaderLen) + "\"");
					}
				}
				
				this.executeMethodSync("OnEncryption", [{ type : "decryptData", data : this.decryptData.data, check: check }]);
				this.decryptData = null;
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
