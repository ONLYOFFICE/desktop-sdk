(function(window, undefined){

	window.Asc.plugin.init = function(text)
	{
		window.Asc.plugin.executeMethod("AddVideo", [], function() {
			this.executeCommand("close", "");
		});
	};
	
})(window, undefined);
