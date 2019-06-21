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
		
		var filter = window.Asc.plugin.tr("Video Files") + " (*.webm  *.mkv  *.flv  *.ogg  *.avi  *.mov  *.wmv  *.mp4 *.m4v  *.mpg  *.mp2  *.mpeg  *.mpe  *.mpv  *.m2v  *.m4v *.3gp  *.3g2  *.f4v  *.m2ts  *.mts);";
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
			
			window.Asc.plugin.executeMethod("StartAction", ["Block", window.Asc.plugin.tr("Copying video...")], function() {
				
				setTimeout(function(){
					window["AscDesktopEditor"]["AddVideo"](file, window.Asc.plugin.guid.substr(4));
					window.Asc.plugin.executeMethod("EndAction", ["Block", window.Asc.plugin.tr("Copying video...")]);
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
