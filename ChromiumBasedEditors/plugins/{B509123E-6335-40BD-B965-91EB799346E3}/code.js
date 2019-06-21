(function(window, undefined){

	window.Asc.plugin.init = function(text)
	{
		window["AscDesktopEditor"]["SendByMail"]();

		setTimeout(function(){
			window.Asc.plugin.executeCommand("close", "");
		}, 100);
	};
	
	window.Asc.plugin.button = function(id)
	{
		this.executeCommand("close", "");
	};
	
})(window, undefined);
