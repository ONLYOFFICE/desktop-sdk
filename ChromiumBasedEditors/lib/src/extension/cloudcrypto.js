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
					var isSupportCrypt = window.AscDesktopEditor.isFileSupportCloudCrypt(file);
					var isCrypto = false;

					if (isSupportCrypt)
					{
						var docinfo = window.AscDesktopEditor.getDocumentInfo(file);
						var param = window.cloudCryptoCommandParam;					
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
		case "upload":
		{
			var filter = param.filter || "any";
			window.AscDesktopEditor.OpenFilenameDialog(filter, true, function(files) {
				if (!Array.isArray(files))
					files = [files];

				window.cloudCryptoCommandCount = files.length;
				for (var i = 0; i < files.length; i++)
				{
					var file = files[i];
					var isSupportCrypt = window.AscDesktopEditor.isFileSupportCloudCrypt(file);
					var isOfficeFileCrypt = window.AscDesktopEditor.isFileCrypt(file);
					var isCrypto = false;

					if (isSupportCrypt && !isOfficeFileCrypt)
					{
						var password = window.AscCrypto.CryptoWorker.createPassword();
						var user = window.AscCrypto.CryptoWorker.User;
						docinfo = window.AscCrypto.CryptoWorker.generateDocInfo([{ userId : user[2], publicKey : user[1] }], password);
						isCrypto = window.AscDesktopEditor.setDocumentInfo(file, password, docinfo);
					}

					var curIndex = i;
					window.AscDesktopEditor.loadLocalFile(file, function(data){
						
						var fileName = file;
						var index = fileName.lastIndexOf("/");
						if (index != -1) fileName = fileName.substring(index + 1);
						index = fileName.lastIndexOf("\\");
						if (index != -1) fileName = fileName.substring(index + 1);

						window.cloudCryptoCommandCallback({
							bytes : data,
							isCrypto : isCrypto,
							name : fileName,
							index : curIndex,
							count : window.cloudCryptoCommandCount
						});
						
						window.cloudCryptoCommandCounter++;
						if (window.cloudCryptoCommandCounter == window.cloudCryptoCommandCount)
						{
							window.cloudCryptoCommandCount = 0;
							delete window.cloudCryptoCommandParam;
							delete window.cloudCryptoCommandCallback;
						}
					});
				}
			});
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
