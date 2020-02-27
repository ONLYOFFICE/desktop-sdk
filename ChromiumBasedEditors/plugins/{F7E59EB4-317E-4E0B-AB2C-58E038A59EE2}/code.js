(function(window, undefined){

	window.Asc.plugin.init = function(text)
	{
		window.Asc.plugin.executeMethod("AddVideo", [], function() {
			window.Asc.plugin.executeCommand("close", "");
		});
	};
	
})(window, undefined);
