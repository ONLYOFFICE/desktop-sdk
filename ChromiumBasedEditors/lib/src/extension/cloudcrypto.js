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
					let file = files[fileItem];
					let isSupportCrypt = window.AscDesktopEditor.isFileSupportCloudCrypt(file);
					let isCrypto = false;

					if (isSupportCrypt)
					{
						let docinfo = window.AscDesktopEditor.getDocumentInfo(file);
						let param = window.cloudCryptoCommandParam;					
						if (docinfo == "")
						{
							let password = window.AscCrypto.CryptoWorker.createPassword();
							docinfo = window.AscCrypto.CryptoWorker.generateDocInfo(param.keys, password);
							isCrypto = window.AscDesktopEditor.setDocumentInfo(file, password, docinfo);
						}
						else
						{
							let password = window.AscCrypto.CryptoWorker.readPassword(docinfo);						
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
			var keys = param.keys || [];
			var user = window.AscCrypto.CryptoWorker.User;
			keys.push({ userId : user[2], publicKey : user[1] });
			window.AscDesktopEditor.OpenFilenameDialog(filter, true, function(files) {
				if (!Array.isArray(files))
					files = [files];

				window.cloudCryptoCommandCount = files.length;
				for (var i = 0; i < files.length; i++)
				{
					let file = files[i];
					let isSupportCrypt = window.AscDesktopEditor.isFileSupportCloudCrypt(file);
					let isOfficeFileCrypt = window.AscDesktopEditor.isFileCrypt(file);
					let cryptoFile = "";

					if (isSupportCrypt && !isOfficeFileCrypt)
					{
						let password = window.AscCrypto.CryptoWorker.createPassword();						
						docinfo = window.AscCrypto.CryptoWorker.generateDocInfo(keys, password);
						cryptoFile = window.AscDesktopEditor.setDocumentInfo(file, password, docinfo, true);
					}
					let isCrypto = ("" != cryptoFile) ? true : false;
					let curIndex = i;
					window.AscDesktopEditor.loadLocalFile(isCrypto ? cryptoFile : file, function(data){
						
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

						if (isCrypto)
							window.AscDesktopEditor.RemoveFile(cryptoFile);
						
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
