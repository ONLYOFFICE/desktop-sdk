(function(window, undefined){

	window.Asc.plugin.init = function(text)
	{
		if (!window.Asc.plugin.translateManager)
		{
			setTimeout(function(){
				window.Asc.plugin.init();
			}, 100);
			return;
		}
		
		var filter = window.Asc.plugin.tr("Audio Files") + " (*.flac *.mp3 *.ogg *.wav *.wma *.ape *.aac *.m4a *.alac);";
		filter += (";" + window.Asc.plugin.tr("All Files") + " (*.*)");
		
	    window["AscDesktopEditor"]["OpenFilenameDialog"](filter, false, function(_file) {
			var file = _file;
			if (Array.isArray(file))
				file = file[0];
			
			if (!file)
			{
				window.Asc.plugin.executeCommand("close", "");
				return;
			}
			
			window.Asc.plugin.executeMethod("StartAction", ["Block", window.Asc.plugin.tr("Copying audio...")], function() {
				
				setTimeout(function(){
					window["AscDesktopEditor"]["AddAudio"](file, window.Asc.plugin.guid.substr(4));
					window.Asc.plugin.executeMethod("EndAction", ["Block", window.Asc.plugin.tr("Copying audio...")]);
					window.Asc.plugin.executeCommand("close", "");
				}, 100);
				
			});
		});
	};
	
	window.Asc.plugin.button = function(id)
	{
		this.executeCommand("close", "");
	};
	
})(window, undefined);
