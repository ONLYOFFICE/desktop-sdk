var deleteGuids = {};

function deletePlugin(id) 
{
    deleteGuids[id] = true;
    loadPlugins();
    updateList();
};

function addPlugin() 
{
    window["AscDesktopEditor"]["OpenFilenameDialog"]("plugin", false, function(_file){
		var file = _file;
		if (Array.isArray(file))
			file = file[0];
		
		window["AscDesktopEditor"]["PluginInstall"](file);
		loadPlugins();
		updateList();
	});
};

document.getElementById("button_add").onclick = function() {
    addPlugin();
};

Ps.initialize(document.getElementById("plugins-list-box"), { theme : 'custom-theme' });
function updateScroll()
{
    Ps.update(document.getElementById("plugins-list-box"));
    if($('.ps__scrollbar-y').height() === 0)
    {
        $('.ps__scrollbar-y').css('border-width', '0px');
    }
    else
    {
        $('.ps__scrollbar-y').css('border-width', '1px');
    }
	
	if($('.ps__scrollbar-x').width() === 0)
	{
		$('.ps__scrollbar-x').css('border-width', '0px');
	}
	else
	{
		$('.ps__scrollbar-x').css('border-width', '1px');
	}
}

window.onresize = function()
{
    var _separator = document.getElementById("id_separator");
    var _divMain = document.getElementById("plugins-list-box");
    _divMain.style.top = _separator.offsetTop + "px";
    updateScroll();
}

window.onresize();
document.getElementById("mask").style.display = "none";

var EditorPlugins = [];

function loadPlugins()
{
    var _pluginsTmp = JSON.parse(window["AscDesktopEditor"]["GetInstallPlugins"]());
	_pluginsTmp[0]["url"] = _pluginsTmp[0]["url"].replace(" ", "%20");
	_pluginsTmp[1]["url"] = _pluginsTmp[1]["url"].replace(" ", "%20");

	var _plugins = { "url" : _pluginsTmp[0]["url"], "pluginsData" : [] };
	for (var k = 0; k < 2; k++)
	{
		var _pluginsCur = _pluginsTmp[k];

		var _len = _pluginsCur["pluginsData"].length;
		for (var i = 0; i < _len; i++)
		{
            _pluginsCur["pluginsData"][i]["baseUrl"] = _pluginsCur["url"] + _pluginsCur["pluginsData"][i]["guid"].substring(4) + "/";
            _pluginsCur["pluginsData"][i]["isSystemInstall"] = (k == 0) ? true : false;

            if ((true !== deleteGuids[_pluginsCur["pluginsData"][i]["guid"]]) &&
                (window.Asc.plugin.guid != _pluginsCur["pluginsData"][i]["guid"]) && 
                (_pluginsCur["pluginsData"][i]["variations"]) && 
                (_pluginsCur["pluginsData"][i]["variations"][0]) && 
                (_pluginsCur["pluginsData"][i]["variations"][0]["EditorsSupport"]))
            {
                _plugins["pluginsData"].push(_pluginsCur["pluginsData"][i]);
            }
		}
	}

	for (var i = 0; i < _plugins["pluginsData"].length; i++)
	{
		var _plugin = _plugins["pluginsData"][i];
		//_plugin["baseUrl"] = _plugins["url"] + _plugin["guid"].substring(4) + "/";

		var isSystem = false;
		for (var j = 0; j < _plugin["variations"].length; j++)
		{
			var _variation = _plugin["variations"][j];
			if (_variation["initDataType"] == "desktop")
			{
				isSystem = true;
				break;
			}
		}

		if (isSystem)
		{
			_plugins["pluginsData"].splice(i, 1);
			--i;
		}
	}

    EditorPlugins = _plugins;
    
    EditorPlugins["pluginsData"].sort(function(a, b) {
        if (a.isSystemInstall === b.isSystemInstall)
            return 0;
        if (a.isSystemInstall && !b.isSystemInstall)
            return -1;
        return 1;
    });
}

function updateList()
{
    let _content = "<ul>";

    for (let i = 0; i < EditorPlugins.pluginsData.length; i++)
    {
        let url = EditorPlugins.url;
        if (EditorPlugins.pluginsData[i].baseUrl != null && EditorPlugins.pluginsData[i].baseUrl != "")
            url = EditorPlugins.pluginsData[i].baseUrl;

        let iconIndex = 0;
        let iconsArray = (EditorPlugins.pluginsData[i].variations[0] && EditorPlugins.pluginsData[i].variations[0].icons && EditorPlugins.pluginsData[i].variations[0].icons.length) ? 
                                    EditorPlugins.pluginsData[i].variations[0].icons : [];
        if (iconsArray.length >= 4)
        {
            iconIndex = (window.devicePixelRatio >= 2) ? 3 : 2;
        }
        else
        {
            iconIndex = (window.devicePixelRatio >= 2) ? 1 : 0;
        }
        let iconSrc = iconsArray[iconIndex];
        if (iconSrc)
            iconSrc = url + iconSrc;
        if (!iconSrc)
            iconSrc = (window.devicePixelRatio >= 2) ? "defaulticon2@2x.png" : "defaulticon2.png";

        let item = "<li><a class=\"item\">";
        item += ("<img class=\"icon\" src=\"" + iconSrc + "\"></img>");
        item += ("<span class=\"defaultlable\" style=\"flex-grow: 1;margin-left: 10px; line-height: 20px;\">" + EditorPlugins.pluginsData[i].name + "</span>");

        let classRemove = (window.devicePixelRatio >= 2) ? "del2" : "del";
        if (!EditorPlugins.pluginsData[i].isSystemInstall)
        {
            item += ("<i class=\"btn " + classRemove + "\" onclick=\"deletePlugin('" + EditorPlugins.pluginsData[i].guid + "');\" title=\"remove plugin\"></i>");
        }

        item += "</a></li>";
        _content += item;
    }

    _content += "</ul>";

    let _list1 = document.getElementById('content');
    _list1.innerHTML = _content;

    updateScroll();
};

(function(){
    if ("undefined" != typeof(FileReader))
    {
        var viewportElem = document.getElementById("viewport");
        viewportElem["ondragover"] = function(e)
        {
            e.stopPropagation();
            e.preventDefault();

            let _files = window["AscDesktopEditor"]["GetDropFiles"]();
            for (let i = _files.length - 1; i >= 0; i--)
            {
                if (_files[i].indexOf(".plugin") != -1)
                {
                    e.dataTransfer.dropEffect = "copy";
                    return false;
                }
            }

            e.dataTransfer.dropEffect = "none";
            return false;
        };
        viewportElem["ondrop"] = function(e) 
        {
            e.stopPropagation();
            e.preventDefault();
            
            let _isUpdate = false;
            let _files = window["AscDesktopEditor"]["GetDropFiles"]();
            for (let i = _files.length - 1; i >= 0; i--)
            {
                if (_files[i].indexOf(".plugin") != -1)
                {
                    window["AscDesktopEditor"]["PluginInstall"](_files[i]);
                    _isUpdate = true;
                }
            }
            
            if (_isUpdate)
            {
                loadPlugins();
                updateList();
            }
        };
    }
})();

(function(window, undefined){        

    window.Asc.plugin.init = function(text)
    {
        loadPlugins();
        updateList();        
    };
    
    window.Asc.plugin.button = function(id)
    {
        if (0 == id)
        {
            for (let guid in deleteGuids)
                window["AscDesktopEditor"]["PluginUninstall"](guid);
        }
        this.executeCommand("close", "");
    };
	
	window.Asc.plugin.onExternalMouseUp = function()
    {
		var userAgent = navigator.userAgent.toLowerCase();
		var isIE =  ((userAgent.indexOf("msie") > -1 || userAgent.indexOf("trident") > -1) && (userAgent.indexOf("edge") < 0));
    	if (!isIE)
		{
			var event = new MouseEvent('mouseup', {
				'view':       window,
				'bubbles':    true,
				'cancelable': true
			});

			document.dispatchEvent(event);
		}
		else
		{
			var mouseUpEvent = document.createEvent ('MouseEvents');
			mouseUpEvent.initEvent("mouseup", true, true);
			document.dispatchEvent(mouseUpEvent);
		}
    };

})(window, undefined);