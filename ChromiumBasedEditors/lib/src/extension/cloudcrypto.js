window.AscDesktopEditor.cloudCryptoCommandMainFrame = function(obj, callback){
	window.cloudCryptoCommandMainFrame_callback = callback;
	window.AscDesktopEditor._cloudCryptoCommandMainFrame(window.AscDesktopEditor.GetFrameId(), JSON.stringify(obj));
};
window.AscDesktopEditor.cloudCryptoCommand = function(type, param, callback)
{
	window.AscDesktopEditor.initCryptoWorker(param.cryptoEngineId);
	window.cloudCryptoCommandCounter = 0;
	window.cloudCryptoCommandCount = 0;
	window.cloudCryptoCommandParam = param;
	window.cloudCryptoCommandCallback = callback;
	switch (type)
	{
		case "share":
		{
			var filesInput = Array.isArray(param.file) ? param.file : [param.file];
			window.cloudCryptoCommandCount = filesInput.length;
			window.AscDesktopEditor.DownloadFiles(filesInput, [], function(files) {				
				
				for (var fileItem in files)
				{
					var file = files[fileItem];
					var docinfo = window.AscDesktopEditor.getDocumentInfo(file);
					var param = window.cloudCryptoCommandParam;
					var isCrypto = false;
					if (docinfo == "")
					{
						var password = window.AscCrypto.CryptoWorker.createPassword();
						docinfo = window.AscCrypto.CryptoWorker.generateDocInfo(param.keys, password);
						isCrypto = window.AscDesktopEditor.setDocumentInfo(file, password, docinfo);
					}
					else
					{
						var password = window.AscCrypto.CryptoWorker.readPassword(docinfo);						
						docinfo = window.AscCrypto.CryptoWorker.generateDocInfo(param.keys, password);						
						isCrypto = window.AscDesktopEditor.setDocumentInfo(file, password, docinfo);						
					}

					window.AscDesktopEditor.loadLocalFile(file, function(data){
						window.cloudCryptoCommandCallback({
							bytes : data,
							isCrypto : isCrypto,
							url : fileItem
						});
						
						window.AscDesktopEditor.RemoveFile(file);

						window.cloudCryptoCommandCounter++;
						if (window.cloudCryptoCommandCounter == window.cloudCryptoCommandCount)
						{
							window.cloudCryptoCommandCount = 0;
							delete window.cloudCryptoCommandParam;
							delete window.cloudCryptoCommandCallback;
						}
					});
				}
				
			}, 1);
			
			break;
		}
		default:
		{
			callback(null);
			delete window.cloudCryptoCommandParam;
			delete window.cloudCryptoCommandCallback;
			break;
		}
	}
};
