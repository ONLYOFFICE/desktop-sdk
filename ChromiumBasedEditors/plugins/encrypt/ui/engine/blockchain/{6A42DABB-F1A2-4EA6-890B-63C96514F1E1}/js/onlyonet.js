if (typeof ONLYONET === "undefined") {
    ONLYONET = {};
}

ONLYONET = (function() {


return {    
    generateRandomEnrollmentId: function() {
		function s4() {
			return Math.floor((1 + Math.random()) * 0x10000).toString(16).substring(1);
		}

        let guid = s4() + s4() + '-' + s4() + '-' + s4() + '-' + s4() + '-' + s4() + s4() + s4();
        
        return `${guid}@rca.onlyoffice.dev`;
    },    
    getBlockchainInfo: function () { 
        return $.ajax({
            method: "POST",
            url: ONLYONET.AppSettings.client_endpoint + "/system/blockchaininfo",
            contentType: "application/json",
            async: true,
            dataType: 'json',
            data: JSON.stringify({"enrollmentID": ONLYONET.AppSettings.CurrentUser.enrollmentID })
        });  
    },
    encryptData: function (data, key, nonce) {     
        var messageUInt8Array = nacl.util.decodeUTF8(data);
        var nonceUInt8Array = nacl.util.decodeBase64(nonce);
        var keyUInt8Array = nacl.util.decodeUTF8(key.replace(/-/g,''));
      
        return nacl.util.encodeBase64(nacl.secretbox(messageUInt8Array, nonceUInt8Array, keyUInt8Array));        
    },  
    decryptData: function (data, key, nonce) {
        var messageUInt8Array = nacl.util.decodeBase64(data);
        var keyUInt8Array = nacl.util.decodeUTF8(key.replace(/-/g,''));
        var nonceUInt8Array = nacl.util.decodeBase64(nonce);

        return nacl.util.encodeUTF8(nacl.secretbox.open(messageUInt8Array, nonceUInt8Array, keyUInt8Array));    
    },
    registerAndEnrollUser:  async function(enrollmentID, enrollmentSecret) {       
        const userInfo = await $.ajax({
                                    method: "POST",
                                    url: ONLYONET.AppSettings.server_endpoint + "/user/enroll",
                                    contentType: "application/json",                                    
                                    dataType: 'json',
                                    data: JSON.stringify({"enrollmentID": enrollmentID, 
                                                          "enrollmentSecret": enrollmentSecret})
                                }); 

        await $.ajax({
                        method: "POST",
                        url: ONLYONET.AppSettings.client_endpoint + "/user/wallet",
                        contentType: "application/json",   
                        data: JSON.stringify({"enrollmentID": enrollmentID, 
                                            "mspId": userInfo.mspId,
                                            "publicKeyECDSA": userInfo.publicKeyECDSA,
                                            "privateKeyECDSA": userInfo.privateKeyECDSA
                                            })
                    });
      
        const ECIES = await $.ajax({
                                    method: "POST",
                                    url: ONLYONET.AppSettings.client_endpoint + "/user/eccrypto/generate",
                                    contentType: "application/json", 
                                    dataType: 'json',           
                                    data: JSON.stringify({"enrollmentID": enrollmentID})
                                });
                                
        return JSON.stringify({
            enrollmentID: userInfo.enrollmentID,
            enrollmentSecret: userInfo.enrollmentSecret,
            mspId: userInfo.mspId,            
            publicKeyECDSA: userInfo.publicKeyECDSA,
            privateKeyECDSA: userInfo.privateKeyECDSA,
            publicKeyECIES: ECIES.publicKeyECIES,
            privateKeyECIES: ECIES.privateKeyECIES 
        });
        
    },
    saveFilePassword: function (fileHash, filePassword, accounts) {  
        let promises = [];
          
        if (accounts == null)
        {
            accounts = [ { "address": "", "publicKey": ONLYONET.AppSettings.CurrentUser.publicKeyECDSA } ];
        }
        
        $.each(accounts, function(index, account) {            
            promises.push(
                new Promise(function(resolve, reject) {                
                    $.ajax({
                        method: "POST",
                        url: ONLYONET.AppSettings.client_endpoint + "/document/password",
                        contentType: "application/json",                        
                        data: JSON.stringify({
                                            "enrollmentID": ONLYONET.AppSettings.CurrentUser.enrollmentID, 
                                            "fileHash": fileHash,
                                            "filePassword": filePassword, 
                                            "recipientPublicKeyECDSA": account.publicKey
                                            }),
                        success: function(result) {
                            resolve(result);
                        },
                        error: function(result) {
                            reject(result.responseJSON.error);
                        }
                    });
                })                
            );
        });

        return Promise.all(promises);
    },
    isFilePasswordExist: function (fileHash, publicKeyECDSA) {
        const resp = $.ajax({
            method: "POST",
            url: ONLYONET.AppSettings.client_endpoint + "/document/password/exist",
            contentType: "application/json",
            async: false,
            dataType: 'json',
            data: JSON.stringify({
                                   "enrollmentID": ONLYONET.AppSettings.CurrentUser.enrollmentID, 
                                   "fileHash": fileHash, 
                                   "publicKeyECDSA": publicKeyECDSA
                                })
        }).responseJSON;

        return (resp.result == "true");
    },
    getFilePassword: function (fileHash) {
        const resp = $.ajax({
            method: "POST",
            url: ONLYONET.AppSettings.client_endpoint + "/document/password/get",
            contentType: "application/json",
            async: false,
            dataType: 'json',
            data: JSON.stringify({
                                   "enrollmentID": ONLYONET.AppSettings.CurrentUser.enrollmentID, 
                                   "fileHash": fileHash, 
                                   "privateKeyECIES": ONLYONET.AppSettings.CurrentUser.privateKeyECIES
                                })
        }).responseJSON;

        return resp.password;
    }
}
})();