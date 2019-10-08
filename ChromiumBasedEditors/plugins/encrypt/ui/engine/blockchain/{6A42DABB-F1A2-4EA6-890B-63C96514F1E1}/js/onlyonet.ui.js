if (typeof ONLYONET === "undefined") {
    ONLYONET = {};
}

const ASC_DESKTOP_EDITOR_DEFAULT_MODE = 0;
const ASC_DESKTOP_EDITOR_CRYPTO_MODE = 3;
var _lockObject = false;

ONLYONET.UI = (function() {
    function _initVaultDialog() {
        $("#dlg-vault img.img-loader").hide();
        $("#dlg-vault textarea").removeClass("error");
        $("#dlg-vault textarea").val("");
        $("#dlg-vault input:password:first").val("");
        $("#dlg-vault input:password:eq(1)").val("");
        $("#dlg-vault input:password:first").removeClass("error");
        $("#dlg-vault input:password:eq(1)").removeClass("error");
        $("#dlg-vault input:password:first").removeAttr("readonly");
        $("#dlg-vault input:password:eq(1)").removeAttr("readonly");
        $("#dlg-vault .lr-flex .error-box p.msg-error").hide();
        $("#dlg-vault button.primary").removeAttr("disabled");
        $("#dlg-vault textarea").removeAttr("readonly");
    }

    function _initSwitchOnDialog() {
        $("#dlg-onoffswitch .error-box p").hide();
        $("#dlg-onoffswitch img.img-loader").hide();
        $("#dlg-onoffswitch input:password").val("");	
        $("#dlg-onoffswitch input:password").attr("placeholder", ONLYONET.Resources["dlg-onoffswitch-password"]);
        $("#dlg-onoffswitch input:text").removeClass("error");	
        $("#dlg-onoffswitch button.primary").removeAttr("disabled");					
    }

    function _blockchainSwitchOn(cnf, dlgButtonClose) { 						
        
        AscDesktopEditor.SetCryptoMode("", ASC_DESKTOP_EDITOR_CRYPTO_MODE, async function(retCode) {
            switch (retCode)
            {
                case 0: // OK
                {
                    $("div.info-box.excl").hide();	

                    await _renderBlockChainInfo();

                    $("#enc-mode-switch").prop("checked", true);
                    $("#box-blockchain-connect").hide();
                    $("#box-blockchain-info").show();

                    $(dlgButtonClose).trigger("click");

                break;
            }
            case 1:
            {
                
                $(dlgButtonClose).trigger("click");
                $("div.info-box.excl").show();
                
                break;
            }
            default:
                break;
            }					
        });
    }

    async function _renderBlockChainInfo() {					
        if (_lockObject) return;

        _lockObject = true;   

        const dataset = await ONLYONET.getBlockchainInfo();

        $("#box-network-info dd:eq(0)").text(dataset.channelName);
        $("#box-network-info dd:eq(1)").text(dataset.blockNumber);        
        $("#box-network-info dd:eq(2)").text(ONLYONET.AppSettings.CurrentUser.mspId);
        $("#box-network-info dd:eq(3)").text(dataset.clientId);    
        $("#box-network-info dd:eq(4)").text(ONLYONET.AppSettings.CurrentUser.enrollmentID);
        $("#box-network-info dd:eq(5)").text(1);

        _lockObject = false;
    }
   
    function _getUrlVars()
    {
        var vars = [], hash;
        var hashes = window.location.href.slice(window.location.href.indexOf('?') + 1).split('&');
        for(var i = 0; i < hashes.length; i++)
        {
            hash = hashes[i].split('=');
            vars.push(hash[0]);
            vars[hash[0]] = hash[1];
        }
        return vars;
    }

    return {
        // Public Fields
        isEditorsPresent: false,
        init: function(opts) {
            var lang = _getUrlVars()["lang"];         
            
            if (typeof lang == 'undefined')
                lang = "en-US";

            const successLoadLangScript = function() {
                                                for (var key in ONLYONET.Resources){
                                                    $('*[data-t="' + key + '"]').html(ONLYONET.Resources[key]);
                                                }};              

       //     if (typeof ONLYONET.Resources == 'undefined') $.loadScript('./lang/lang_' + lang + '.js',successLoadLangScript);
            if (typeof ONLYONET.Resources == 'undefined') $.loadScript('./lang/lang_en-US.js', successLoadLangScript);

            $("#box-blockchain-info-btn-refresh").attr("alt", ONLYONET.Resources["box-blockchain-info-btn-refresh"]);
            $("#box-blockchain-info-btn-refresh").attr("title", ONLYONET.Resources["box-blockchain-info-btn-refresh"]);

            $("#box-blockchain-connect a.text-sub").click(function(){
                _initSwitchOnDialog();
               
                $("#dlg-onoffswitch")[0].showModal();
                window.parent.postMessage(JSON.stringify({type:'plugin', event: 'modal:open'}), '*');
            });          

            $("#enc-mode-switch").click(function () {	

                if (ONLYONET.UI.isEditorsPresent) {

                    if ($("#enc-mode-switch").prop("checked")) {
                        $("div.info-box.excl").html(ONLYONET.Resources["info-box-off"]);   
                    }
                    else {
                        $("div.info-box.excl").html(ONLYONET.Resources["info-box-on"]);   
                    }
                                                 
                    $("div.info-box.excl").show();   

                    return false;
                }
                else
                {
                    $("div.info-box.excl").hide();  
                }          
                
                if($(this).is(":checked")) {                    
                    _initSwitchOnDialog();

                    $("#dlg-onoffswitch")[0].showModal();
                    window.parent.postMessage(JSON.stringify({type:'plugin', event: 'modal:open'}), '*');

                    return false;
                }
                else {
                    AscDesktopEditor.SetCryptoMode("", ASC_DESKTOP_EDITOR_DEFAULT_MODE, function(retCode) {								
                        switch (retCode)
                        {								
                            case 0: // OK
                            {
                                $("#box-blockchain-connect").show();
                                $("#box-blockchain-info").hide();
                                $("div.info-box.excl").html(ONLYONET.Resources["info-box-off"]);

                                $("div.info-box.excl").hide();  

                                $("#enc-mode-switch").prop("checked", false);

                                break;
                            }
                            case 1: 
                            {
                                $("div.info-box.excl").html(ONLYONET.Resources["info-box-on"]);
                                
                                $("div.info-box.excl").show();   
                                
                                $("#enc-mode-switch").prop("checked", true);

                                break;                                
                             }
                            default: 
                            {

                                break;
                            }

                        }
                    });

                    return false;
                }	
            });  

            $("#box-blockchain-connect button.primary").click(function () {
                _initVaultDialog();

                $("#dlg-vault div.title label.caption").text(ONLYONET.Resources["dlg-vault-mnemonic-caption"]);
                $("#dlg-vault textarea").removeAttr("placeholder");
                $("#dlg-vault button.btn.primary").text( ONLYONET.Resources["dlg-vault-description-btn"]);
                $("#dlg-vault textarea").attr("readonly","readonly");

               // $("#dlg-vault textarea").val(ONLYONET.generateRandomSeed());
                           
                
                $("#dlg-vault")[0].showModal();	
                window.parent.postMessage(JSON.stringify({type:'plugin', event: 'modal:open'}), '*');					
            });		
            
            $("#box-blockchain-info-btn-refresh").click(function() {
                _renderBlockChainInfo();
            });

            $("#dlg-onoffswitch button.primary").click(function () {
                $("#dlg-onoffswitch button.primary").attr("disabled","disabled");
                $("#dlg-onoffswitch img.img-loader").show();
                
                let pwd = $("#dlg-onoffswitch input:password").val();									
                
                AscDesktopEditor.GetAdvancedEncryptedData(pwd, function (result) { 
                    let data = JSON.parse(Base64.decode(result));
            
                    if (!data) {
                        $("#dlg-onoffswitch .error-box p").show();
                        $("#dlg-onoffswitch input:text").addClass("error");

                        $("#dlg-onoffswitch button.primary").removeAttr("disabled");
                        $("#dlg-onoffswitch img.img-loader").hide();

                        return false;	
                    }
                    
                    ONLYONET.AppSettings.CurrentUser = {
                        "enrollmentID": data.enrollmentID,
                        "enrollmentSecret": data.enrollmentSecret,
                        "mspId": data.mspId,
                        "publicKeyECDSA": data.publicKeyECDSA,
                        "privateKeyECDSA": data.privateKeyECDSA,
                        "publicKeyECIES": data.publicKeyECIES,
                        "privateKeyECIES": data.privateKeyECIES
                     };                           

                    _blockchainSwitchOn(ONLYONET.AppSettings.CurrentUser, "#dlg-onoffswitch .tool.close");
                });  

            });	
    
            $("#dlg-onoffswitch a.text-sub").click(function () {
                $("#dlg-onoffswitch .tool.close").trigger("click");	

                _initVaultDialog();
                
                $("#dlg-vault textarea").removeAttr("readonly");
                $("#dlg-vault div.title label.caption").text(ONLYONET.Resources["dlg-vault-mnemonic-restore-caption"]);
                $("#dlg-vault textarea").attr("placeholder", ONLYONET.Resources["dlg-vault-mnemonic-restore-placeholder"]);
                $("#dlg-vault button.btn.primary").text(ONLYONET.Resources["dlg-vault-restore-description-btn"]);					
            
                
                $("#dlg-vault")[0].showModal();		
                window.parent.postMessage(JSON.stringify({type:'plugin', event: 'modal:open'}), '*');		
            });

            $("#dlg-vault .tool.close").click(function(){						
                $("#dlg-vault")[0].close();	
                window.parent.postMessage(JSON.stringify({type:'plugin', event: 'modal:close'}), '*')				
            });

            $("#dlg-vault, #dlg-onoffswitch, #dlg-private-key").each(function() {
                $(this)[0].addEventListener('cancel', function() {
                    window.parent.postMessage(JSON.stringify({type:'plugin', event: 'modal:close'}), '*')			
                });
            });
            
            $("#dlg-vault a.text-sub").click(function () {
                $("#dlg-vault .tool.close").trigger("click");
            });
            
            $(document).keydown(function(event) {					
                if (event.which == 13 ) { // enter
                    if ($("#dlg-onoffswitch").prop("open")) {
                        $("#dlg-onoffswitch button.primary").trigger("click");	
                    } 

                    if ($("#dlg-vault").prop("open")) {
                        $("#dlg-vault button.primary").trigger("click");	
                    }
                }
            });

            $("#dlg-vault button.primary").click(function() {
                $("#dlg-vault img.img-loader").show();
                $("#dlg-vault button.primary").attr("disabled", "disabled");

                let pwd = $("#dlg-vault input:password:first").val();	
                let pwdRepeat = $("#dlg-vault input:password:eq(1)").val();	
            
                $("#dlg-vault .error-box:eq(0) p.msg-error").hide();
                $("#dlg-vault .error-box:eq(1) p.msg-error").hide();

                $("#dlg-vault input:password:first").removeClass("error");
                $("#dlg-vault input:password:eq(1)").removeClass("error");

                if ((pwd !== pwdRepeat) || (pwd.length < ONLYONET.AppSettings.passwordLength))
                {
                    if (pwd.length < ONLYONET.AppSettings.passwordLength) {
                        $("#dlg-vault .lr-flex .error-box:eq(0) p.msg-error").text(ONLYONET.Resources["dlg-vault-new-password-msg-error-1"]);
                        $("#dlg-vault input:password:first").addClass("error");
                        $("#dlg-vault .lr-flex .error-box:eq(0) p.msg-error").show();
                    } 
                    else if (pwd !== pwdRepeat) {                   
                        $("#dlg-vault .lr-flex .error-box:eq(1) p.msg-error").text(ONLYONET.Resources["dlg-vault-new-password-msg-error"]);
                        $("#dlg-vault input:password:eq(0)").addClass("error");
                        $("#dlg-vault .lr-flex .error-box:eq(1) p.msg-error").show();    
                    }												
                
                    $("#dlg-vault img.img-loader").hide();
                    $("#dlg-vault button.primary").removeAttr("disabled");
                    
                    return false;
                }
                
                $("#dlg-vault input:password:first").attr("readonly","readonly");
                $("#dlg-vault input:password:eq(1)").attr("readonly","readonly");
               
                let enrollmentID = ONLYONET.generateRandomEnrollmentId();

                ONLYONET.registerAndEnrollUser(enrollmentID).then((data) => {
                    const encodedData = Base64.encode(data);
                    
                    AscDesktopEditor.SetAdvancedEncryptedData(pwd, encodedData, (written_data) => {
                        if (encodedData != written_data) // error
                        {
                            $("#dlg-vault img.img-loader").hide();
                            $("#dlg-vault button.primary").removeAttr("disabled");
                            $("#dlg-vault input:password:first").removeAttr("readonly");
                            $("#dlg-vault input:password:eq(1)").removeAttr("readonly");
           

                            return false;
                        }
                        else // OK 
                        {								
                            let cnf = jQuery.extend(true, {}, ONLYONET.AppSettings);	
                            let data = JSON.parse(Base64.decode(encodedData));

                            ONLYONET.AppSettings.CurrentUser = {
                                "enrollmentID": data.enrollmentID,
                                "enrollmentSecret": data.enrollmentSecret,
                                "mspId": data.mspId,
                                "publicKeyECDSA": data.publicKeyECDSA,
                                "privateKeyECDSA": data.privateKeyECDSA,
                                "publicKeyECIES": data.publicKeyECIES,
                                "privateKeyECIES": data.privateKeyECIES
                            };

                            _blockchainSwitchOn(cnf, "#dlg-vault .tool.close");
                        }
                    });
                }, (error) => {
                    
                    $("#dlg-vault img.img-loader").hide();
                    $("#dlg-vault button.primary").removeAttr("disabled");
                    $("#dlg-vault input:password:first").removeAttr("readonly");
                    $("#dlg-vault input:password:eq(1)").removeAttr("readonly");   

                    return false;                                    
                });                               
            });

            $("#dlg-onoffswitch .tool.close").click(function() {
                $("#dlg-onoffswitch")[0].close();
                window.parent.postMessage(JSON.stringify({type:'plugin', event: 'modal:close'}), '*')			
            });

            $("#dlg-private-key .tool.close").click(function () {
                $("#dlg-private-key")[0].close();
                window.parent.postMessage(JSON.stringify({type:'plugin', event: 'modal:close'}), '*')			
            });

            $("#dlg-private-key button.primary").click(function () {
                $("#dlg-private-key .tool.close").trigger("click");
            });

            $("#tbl-acount-info tfoot a").click(function() {
                let privateKey = ONLYONET.getPrivateKey();
                
                $("#dlg-private-key div.body button.primary").attr("data-clipboard-text",privateKey);
                $("#dlg-private-key div.body span").text(privateKey);

                let clip = new ClipboardJS('#dlg-private-key div.body button.primary');

                clip.on('success', function(e) {		
                        e.clearSelection();
                      });

                
                $("#dlg-private-key")[0].showModal();	
                window.parent.postMessage(JSON.stringify({type:'plugin', event: 'modal:open'}), '*');											
            });
        }
    }
})();