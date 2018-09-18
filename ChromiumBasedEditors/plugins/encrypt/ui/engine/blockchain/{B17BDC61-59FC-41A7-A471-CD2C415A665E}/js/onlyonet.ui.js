if (typeof ONLYONET === "undefined") {
    ONLYONET = {};
}

const ASC_DESKTOP_EDITOR_DEFAULT_MODE = 0;
const ASC_DESKTOP_EDITOR_CRYPTO_MODE = 3;

ONLYONET.UI = (function() {
    function _initVaultDialog() {
        $("#dlg-vault img.img-loader").hide();
        $("#dlg-vault .error-box:last p").hide();
        $("#dlg-vault textarea").removeClass("error");
        $("#dlg-vault textarea").val("");
        $("#dlg-vault input:password:first").val("");
        $("#dlg-vault input:password:eq(1)").val("");
        $("#dlg-vault input:password:first").removeClass("error");
        $("#dlg-vault input:password:eq(1)").removeClass("error");
        $("#dlg-vault .error-box:eq(1) p.msg-error").hide();
        $("#dlg-vault .error-box:eq(2) p.msg-error").hide();
        $("#dlg-vault button.primary").removeAttr("disabled");
    }

    function _initSwitchOnDialog() {
        $("#dlg-onoffswitch .error-box p").hide();
        $("#dlg-onoffswitch img.img-loader").hide();
        $("#dlg-onoffswitch input:password").val("");	
        $("#dlg-onoffswitch input:text").removeClass("error");	
        $("#dlg-onoffswitch button.primary").removeAttr("disabled");					
    }

    function _blockchainSwitchOn(cnf, dlgButtonClose) { 						
        
        AscDesktopEditor.SetCryptoMode("", ASC_DESKTOP_EDITOR_CRYPTO_MODE, function(retCode) {
            switch (retCode)
            {
                case 0: // OK
                {
                    $("div.info-box.excl").hide();						
                
                    ONLYONET.init(cnf).then(function(isConnected) {	

                        _renderBlockChainInfo();
                        
                        $("#enc-mode-switch").prop("checked", true);
                        $("#box-blockchain-connect").hide();
                        $("#box-blockchain-info").show();

                        $(dlgButtonClose).trigger("click");									

                        let address = ONLYONET.getAddress();
                        
                        _requestEtherFromFaucet(address);						
                    });

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

    function _renderBlockChainInfo() {					

        let address = ONLYONET.getAddress();

        $("#tbl-acount-info tbody td:eq(0) span").text(address);	
        $("#box-network-info dd:eq(0)").text(ONLYONET.getNetworkId());
        $("#box-network-info dd:eq(1)").text(ONLYONET.getBlockNumber());
        $("#tbl-acount-info tbody td:eq(2)").text(ONLYONET.getTransactionCount());	

        $("#box-network-info dd:eq(2)").text(ONLYONET.getAvarageGasPriceBlock().toString());
        $("#box-network-info dd:eq(3)").text(ONLYONET.getBlockGasLimit());

        $("#tbl-acount-info tbody td:eq(1)").text(ONLYONET.getBalance("ether") + " ETH");
        $("#box-network-selector span:eq(1)").text(ONLYONET.getProviderUri());

        $("#tbl-acount-info tbody td:eq(0) a").attr("data-clipboard-text",address);

        let clip = new ClipboardJS($("#tbl-acount-info tbody td:eq(0) a")[0]);

        clip.on('success', function(e) {							
            e.clearSelection();
        });
    }

    function _requestEtherFromFaucet(address) {
        let balance = ONLYONET.getBalance("ether");			

        if (balance >= 5) return;
            
        let signMsg = ONLYONET.sign(address);	
        
        $.ajax({						
            url: ONLYONET.getProviderUri() + "/faucet",
            contentType:"application/json; charset=utf-8",
            method: 'POST',
            async: true,
            data: JSON.stringify({
                           "address":address,
                           "signature": signMsg
                       })
        }).done(function() {
            let balance  = ONLYONET.getBalance("ether");

            $("#tbl-acount-info tbody td:eq(1)").text(balance + " ETH");
        }).fail(function() {

        });
    }

    return {
        // Public Fields
        init: function(opts) {
            $("#box-blockchain-connect a.text-sub").click(function(){
                _initSwitchOnDialog();
                $("#dlg-onoffswitch")[0].showModal();
            });

            $("#box-connect-to-cloud a.link").click(function() {
                parent.postMessage(window.JSON.stringify({type:'plugin', data:{command:'panel:select', params:'connect'}}), "*");
            });

            $("#enc-mode-switch").click(function () {						
                if($(this).is(":checked")) {
                    _initSwitchOnDialog();
                    $("#dlg-onoffswitch")[0].showModal();

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

                                break;
                            }
                            case 1: 
                            {									
                                alert("[ERROR] FAILED TO SET CRYPTO MODE AS " + ASC_DESKTOP_EDITOR_DEFAULT_MODE);

                                break;
                            }
                            default: 
                            {

                                break;
                            }

                        }
                    });
                }	
            });  

            $("#box-blockchain-connect button.primary").click(function () {
                _initVaultDialog();

                $("#dlg-vault div.title label.caption").text("Create your Account with Seed Phrase");
                $("#dlg-vault textarea").removeAttr("placeholder");
                $("#dlg-vault button.btn.primary").text("I've copied my mnemonice to a safe place");
                $("#dlg-vault textarea").attr("readonly","readonly");

                $("#dlg-vault textarea").val(ONLYONET.generateRandomSeed());
                                                
                $("#dlg-vault")[0].showModal();						
            });				

            $("#dlg-onoffswitch button.primary").click(function () {
                $("#dlg-onoffswitch button.primary").attr("disabled","disabled");
                $("#dlg-onoffswitch img.img-loader").show();
                
                let pwd = $("#dlg-onoffswitch input:password").val();									
                

                ONLYONET.readSeedPhraseFromFile(pwd, function(seedPhrase) {
                
                    if (!ONLYONET.isSeedPhraseValid(seedPhrase)) {
                        $("#dlg-onoffswitch .error-box p").show();
                        $("#dlg-onoffswitch input:text").addClass("error");

                        $("#dlg-onoffswitch button.primary").removeAttr("disabled");
                        $("#dlg-onoffswitch img.img-loader").hide();
                                                
                        return false;							
                    }							
                    
                    var cnf = jQuery.extend(true, {}, ONLYONET.defaultConfig);					

                    cnf.seedPhrase = seedPhrase;
                    cnf.password = pwd;

                    _blockchainSwitchOn(cnf, "#dlg-onoffswitch .tool.close");
                });

            });	
    
            $("#dlg-onoffswitch a.text-sub").click(function () {
                $("#dlg-onoffswitch .tool.close").trigger("click");	

                _initVaultDialog();
                
                $("#dlg-vault textarea").removeAttr("readonly");
                $("#dlg-vault div.title label.caption").text("Restore your Account with Seed Phrase");
                $("#dlg-vault textarea").attr("placeholder", "Enter your secret twelve word phrase here to restore your account");
                $("#dlg-vault button.btn.primary").text("Restore");					
            
                $("#dlg-vault")[0].showModal();				
            });

            $("#dlg-vault .tool.close").click(function(){						
                $("#dlg-vault")[0].close();					
            });
            
            $("#dlg-vault a.text-sub").click(function () {
                $("#dlg-vault .tool.close").trigger("click");
            });
            
            $(document).keydown(function(event) {					
                if (event.which == 13 ) { // enter
                    if ($("#dlg-onoffswitch").prop("open")) {
                        $("#dlg-onoffswitch button.primary").trigger("click");	
                    } 
                }
            });

            $("#dlg-vault button.primary").click(function() {
                $("#dlg-vault img.img-loader").show();
                $("#dlg-vault button.primary").attr("disabled", "disabled");

                let pwd = $("#dlg-vault input:password:first").val();	
                let pwdRepeat = $("#dlg-vault input:password:eq(1)").val();	
            
                if ((pwd !== pwdRepeat) || (pwd.length < ONLYONET.defaultConfig.passwordLength))
                {
                    if (pwd !== pwdRepeat) {
                        $("#dlg-vault .error-box:eq(1) p.msg-error").text("Don't Match");
                        $("#dlg-vault .error-box:eq(2) p.msg-error").text("Don't Match");
                    }
                
                    if (pwd.length < ONLYONET.defaultConfig.passwordLength) {
                        $("#dlg-vault .error-box:eq(1) p.msg-error").text("Password not long enough");
                        $("#dlg-vault .error-box:eq(2) p.msg-error").text("Password not long enough");							
                    }													

                    $("#dlg-vault input:password:first").addClass("error");
                    $("#dlg-vault input:password:eq(1)").addClass("error");
                
                    $("#dlg-vault .error-box:eq(1) p.msg-error").show();
                    $("#dlg-vault .error-box:eq(2) p.msg-error").show();

                    $("#dlg-vault img.img-loader").hide();
                    $("#dlg-vault button.primary").removeAttr("disabled");
                    
                    return false;
                }

                $("#dlg-vault .error-box:eq(1) p.msg-error").hide();
                $("#dlg-vault .error-box:eq(2) p.msg-error").hide();

                let seedPhrase = $("#dlg-vault textarea").val();
                
                if (!ONLYONET.isSeedPhraseValid(seedPhrase)) {
                    $("#dlg-vault .error-box:first p").show();
                    $("#dlg-vault textarea").addClass("error");
                
                    $("#dlg-vault img.img-loader").hide();
                    $("#dlg-vault button.primary").removeAttr("disabled");

                    return false;							
                }

                $("#dlg-vault .error-box:first p").hide();
                
                ONLYONET.storeSeedPhraseToFile(seedPhrase, pwd, function(written_data) {
                    if (seedPhrase != written_data) // error
                    {
                        $("#dlg-vault img.img-loader").hide();
                        $("#dlg-vault button.primary").removeAttr("disabled");

                        return false;
                    }
                    else // OK 
                    {								
                        let cnf = jQuery.extend(true, {}, ONLYONET.defaultConfig);					

                        cnf.seedPhrase = seedPhrase;
                        cnf.password = pwd;							

                        _blockchainSwitchOn(cnf, "#dlg-vault .tool.close");
                    }
                });
            });

            $("#dlg-onoffswitch .tool.close").click(function() {
                $("#dlg-onoffswitch")[0].close();
            });

            $("#dlg-private-key .tool.close").click(function () {
                $("#dlg-private-key")[0].close();
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
            });
        }
    }
})();