if (typeof ONLYONET === "undefined") {
    ONLYONET = {};
}

ONLYONET = (function() {

var _publicKey;    
var _privateKey;    

function _hexStringToByte(str) {
    if (!str) {
        return new Uint8Array();
    }
    
    var a = [];
    for (var i = 0, len = str.length; i < len; i+=2) {
        a.push(parseInt(str.substr(i,2),16));
    }
    
    return new Uint8Array(a);
}

function _byteToHexString(buffer) {
    return Array
        .from (new Uint8Array (buffer))
        .map (b => b.toString (16).padStart (2, "0"))
        .join ("");
}

return {
    encryptData: function (data, key, nonce) {     
        let dataUInt8Array = nacl.util.decodeUTF8(data);
        let keyUInt8Array = nacl.util.decodeUTF8(key.replace(/-/g,''));
        let nonceUInt8Array = nacl.util.decodeBase64(nonce);
      
        return nacl.util.encodeBase64(nacl.secretbox(dataUInt8Array, nonceUInt8Array, keyUInt8Array));        
    },  
    decryptData: function (data, key, nonce) {
        let dataUInt8Array = nacl.util.decodeBase64(data);
        let keyUInt8Array = nacl.util.decodeUTF8(key.replace(/-/g,''));
        let nonceUInt8Array = nacl.util.decodeBase64(nonce);

        return nacl.util.encodeUTF8(nacl.secretbox.open(dataUInt8Array, nonceUInt8Array, keyUInt8Array));    
    },                 
    saveFilePassword: function (request_url, fileHash, filePassword, publicKeys) {   
        let promises = [];
        
        $.each(publicKeys, function(index, publicKey) {
            let sealed = sealedBox.seal(nacl.util.decodeUTF8(filePassword), _hexStringToByte(publicKey));
            let encryptedPwd = nacl.util.encodeBase64(sealed);

            promises.push(new Promise(function(resolve, reject) {     
                    $.ajax({
                        method: "POST",
                        url: `${request_url}/api/2.0/encryption/store`,
                        contentType: "application/json", 
                        dataType: 'json',           
                        data: JSON.stringify({"publicKey": publicKey, "fileHash": fileHash, "data": encryptedPwd }),
                        success: function(result) {
                            resolve(result);
                        },
                        error: function(result) {
                            reject(result.responseJSON.error);
                        }
                    })
            }));
        });

        return Promise.all(promises);
    },
    getFilePassword: function (request_url, fileHash, publicKey) {
        if (typeof publicKey == 'undefined') {
            publicKey = _publicKey;
        }
          
        let result  = $.ajax({
            method: "GET",
            async: false,
            url: `${request_url}/api/2.0/encryption/store?publicKey=${publicKey}&fileHash=${fileHash}`,
            contentType: "application/json"
        }).responseJSON;
       
        if (jQuery.isEmptyObject(result.response)) return "";

        let encMsg = result.response;

        let _pass = nacl.util.encodeUTF8(sealedBox.open(nacl.util.decodeBase64(encMsg), _hexStringToByte(_publicKey), _hexStringToByte(_privateKey)));

        if (!_pass) return "";

        return _pass;
    },  
    generatePassword: function(length) {
        let password = '', character; 

        while (length > password.length) 
        {
            if (password.indexOf(character = String.fromCharCode(Math.floor(Math.random() * 94) + 33), Math.floor(password.length / 94) * 94) < 0) 
            {				
                password += character;
            }
        }

        return password;
    },
    setPublicKey: function(publicKey) {
        _publicKey = publicKey;                   
    },
    getPublicKey: function() {
        return _publicKey;                   
    },
    setPrivateKey: function(privateKey) {
        _privateKey = privateKey;
    },
    getPrivateKey: function() {
        return _privateKey;
    },
    generateKeyPair: function() {
        let result = nacl.box.keyPair();        

        _privateKey = _byteToHexString(result.secretKey);
        _publicKey =  _byteToHexString(result.publicKey);

        return {"publicKey": _publicKey, "privateKey": _privateKey };
    }
  }
})();