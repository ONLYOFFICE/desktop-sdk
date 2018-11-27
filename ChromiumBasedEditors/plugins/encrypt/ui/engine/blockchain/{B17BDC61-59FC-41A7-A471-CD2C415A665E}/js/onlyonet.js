if (typeof ONLYONET === "undefined") {
    ONLYONET = {};
}

ONLYONET = (function() {
    // Private Fields
    var _contractCode = '608060405260043610610057576000357c0100000000000000000000000000000000000000000000000000000000900463ffffffff1680638eaa6ac01461005c57806395bc267314610106578063fff6e61514610137575b600080fd5b34801561006857600080fd5b5061008b60048036038101908080356000191690602001909291905050506101ce565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156100cb5780820151818401526020810190506100b0565b50505050905090810190601f1680156100f85780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34801561011257600080fd5b5061013560048036038101908080356000191690602001909291905050506102c7565b005b34801561014357600080fd5b506101cc6004803603810190808035600019169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001908201803590602001908080601f016020809104026020016040519081016040528093929190818152602001838380828437820191505050505050919291929050505061032d565b005b6060600080836000191660001916815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000208054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156102bb5780601f10610290576101008083540402835291602001916102bb565b820191906000526020600020905b81548152906001019060200180831161029e57829003601f168201915b50505050509050919050565b600080826000191660001916815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600061032a9190610620565b50565b60006001028360001916141515156103ad576040517f08c379a00000000000000000000000000000000000000000000000000000000081526004018080602001828103825260118152602001807f46696c652068617368207265717569726500000000000000000000000000000081525060200191505060405180910390fd5b6000815114151515610427576040517f08c379a000000000000000000000000000000000000000000000000000000000815260040180806020018281038252601f8152602001807f456e637279707465642066696c652070617373776f726420726571756972650081525060200191505060405180910390fd5b600073ffffffffffffffffffffffffffffffffffffffff168273ffffffffffffffffffffffffffffffffffffffff16141515156104cc576040517f08c379a00000000000000000000000000000000000000000000000000000000081526004018080602001828103825260148152602001807f5369676e6572206669656c64207265717569726500000000000000000000000081525060200191505060405180910390fd5b6000806000856000191660001916815260200190815260200160002060008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000208054600181600116156101000203166002900490501415156105af576040517f08c379a00000000000000000000000000000000000000000000000000000000081526004018080602001828103825260078152602001807f726571756972650000000000000000000000000000000000000000000000000081525060200191505060405180910390fd5b80600080856000191660001916815260200190815260200160002060008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020908051906020019061061a929190610668565b50505050565b50805460018160011615610100020316600290046000825580601f106106465750610665565b601f01602090049060005260206000209081019061066491906106e8565b5b50565b828054600181600116156101000203166002900490600052602060002090601f016020900481019282601f106106a957805160ff19168380011785556106d7565b828001600101855582156106d7579182015b828111156106d65782518255916020019190600101906106bb565b5b5090506106e491906106e8565b5090565b61070a91905b808211156107065760008160009055506001016106ee565b5090565b905600a165627a7a72305820f858846dad90b2cc7f224b6fd9db7316a8cb690638ad4f80e7b5fa35bc5db58d0029';
    var _contractAddress = "0x9039e72fd686c381e1f22e78c404954dbe58cbf3";
    var _contractABI = [{"constant":true,"inputs":[{"name":"_fileHash","type":"bytes32"}],"name":"get","outputs":[{"name":"","type":"bytes"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[{"name":"_fileHash","type":"bytes32"}],"name":"remove","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":false,"inputs":[{"name":"_fileHash","type":"bytes32"},{"name":"_signer","type":"address"},{"name":"_pwdSig","type":"bytes"}],"name":"add","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"}];
    var _contractInstance;
    var _providerUri;
    var _web3;  

    var _txutils;
    var _signing;
    var _encryption;
    var _keyStore;

    var _publicKey;    
    var _pwDerivedKey;
    var _address;
    
    var _addressIndex = 0;
    //Math.floor(Math.random() * Math.floor(10));;
    
    var _gasPrice;
    var _gasLimit;    
   
    var _getTransactionReceiptMined = function(txHash, interval) {
        const self = this;
        const transactionReceiptAsync = function(resolve, reject) {
            self.getTransactionReceipt(txHash, (error, receipt) => {
                if (error) {
                    reject(error);
                } else if (receipt == null || receipt.blockHash == null) {
                    setTimeout(
                        () => transactionReceiptAsync(resolve, reject),
                        interval ? interval : 500);
                } else {
                    resolve(receipt);
                }
            });
        };
    
        if (Array.isArray(txHash)) {
            return Promise.all(txHash.map(
                oneTxHash => self.getTransactionReceiptMined(oneTxHash, interval)));
        } else if (typeof txHash === "string") {
            return new Promise(transactionReceiptAsync);
        } else {
            throw new Error("Invalid Type: " + txHash);
        }
    }
    
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

return {
    // Public Fields
    init: function(opts) {
        _gasPrice = opts.gasPrice;
        _gasLimit = opts.gasLimit;        
        _providerUri = opts.protocol + "://" + opts.host + ":" + opts.port;

        _web3 = new Web3();

        _web3.setProvider(new _web3.providers.HttpProvider(_providerUri));
        
        var version = _web3.version.api;
        console.log(version); // "0.2.0"

        if(!_web3.isConnected()) {
            return Promise.reject(false);
        }

        _web3.eth.getTransactionReceiptMined = _getTransactionReceiptMined;

        _contractInstance = _web3.eth.contract(_contractABI).at(_contractAddress);  
     
        _txutils = lightwallet.txutils;
        _signing = lightwallet.signing;
        _encryption = lightwallet.encryption;

        var promise = new Promise((resolve, reject) => {
            lightwallet.keystore.createVault({
                password: opts.password,
                seedPhrase: opts.seedPhrase, 
                hdPathString: "m/44'/60'/0'/0"    
           }, function(err, ks) {

                _keyStore = ks;
                _keyStore.keyFromPassword(opts.password, function(err, pwDerivedKey) {
                    if (err) reject(err);
                    _pwDerivedKey = pwDerivedKey; // scrypt password
                    _keyStore.generateNewAddress(_pwDerivedKey, 10);
                    _address = _keyStore.getAddresses()[_addressIndex];                    
                    _publicKey = _encryption.addressToPublicEncKey(_keyStore, _pwDerivedKey, _address);

                    resolve(true);
                });
           });
        });

        return promise;    
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
    saveFilePassword: function (fileHash, filePassword, accounts) {        
        var txn = [];
    
        var catchError;

        if (accounts == null)
        {
            accounts = [ { "address": _address, "publicKey": _publicKey } ];
        }

        let prevNonce = 0;
         
        $.each(accounts, function(index, account) {
        
            let sealed = sealedBox.seal(nacl.util.decodeUTF8(filePassword), _hexStringToByte(account.publicKey));

            let encryptedPwd = nacl.util.encodeBase64(sealed);

            let txOptions = {
                gasPrice: _gasPrice,
                gasLimit: _web3.toHex(_web3.eth.getBlock("latest").gasLimit),
                value: 0,
                nonce: _web3.eth.getTransactionCount(_address),
                data: _contractCode,
                to: _contractAddress
            };

            if (prevNonce == txOptions.nonce)
                txOptions.nonce++;

            prevNonce = txOptions.nonce;

            let pwdHex = _web3.toHex(encryptedPwd);
            let registerTx = _txutils.functionTx(_contractABI, 'add', [fileHash, account.address, pwdHex ], txOptions);
            
            let signedRegisterTx = _signing.signTx(_keyStore, _pwDerivedKey, registerTx, _address);
            
            try {
                var txId = _web3.eth.sendRawTransaction('0x' + signedRegisterTx.toString('hex'));                           
             }
            catch(err) {
                catchError = err;
                return false;                
            }

            txn.push(txId);             
        }); 

        if (catchError)
        {
            return Promise.reject(catchError);
        }       

        return _web3.eth.getTransactionReceiptMined(txn, 300);
    },
    isFilePasswordExist: function(fileHash, address) {
        let pwdHex =  _web3.toUtf8(_contractInstance.get.call(fileHash, { "from": address}));

        if (!pwdHex) return false;

        return true;

    },
    getFilePassword: function (fileHash) {
        let pwdHex =  _contractInstance.get.call(fileHash, { "from": _address});

        let encMsg = _web3.toUtf8(pwdHex);

        if (!encMsg) return "";
       
        let publicKeyUInt8Array = _hexStringToByte(_publicKey);
        let privateKeyUInt8Array = _hexStringToByte(_keyStore.exportPrivateKey(_address, _pwDerivedKey));

        let _pass = nacl.util.encodeUTF8(sealedBox.open(nacl.util.decodeBase64(encMsg), publicKeyUInt8Array, privateKeyUInt8Array));

        if (!_pass)
            return "";

        return _pass;
    },
    storeSeedPhraseToFile: function(seedPhrase, password, callback) {
        AscDesktopEditor.SetAdvancedEncryptedData(password, seedPhrase, callback);
    }, 
    readSeedPhraseFromFile: function(password, callback) {
        AscDesktopEditor.GetAdvancedEncryptedData(password, callback);       
    },     
    getPublicKey: function() {
        return _publicKey;
    },
    getPrivateKey: function() {
        return _keyStore.exportPrivateKey(_address, _pwDerivedKey);
    },  
    isSeedPhraseValid: function (seedPhrase) {
        if (!seedPhrase) return false;
        return lightwallet.keystore.isSeedValid(seedPhrase);
    },    
    getAddress: function() {
        return _address;
    },    
    generateRandomSeed: function() {
        return lightwallet.keystore.generateRandomSeed();
    }, 
    generatePassword: function(length) {
        var password = '', character; 

        while (length > password.length) 
        {
            if (password.indexOf(character = String.fromCharCode(Math.floor(Math.random() * 94) + 33), Math.floor(password.length / 94) * 94) < 0) 
            {				
                password += character;
            }
        }
        
        return password;
    },        
    getBalance: function(unit) {
        return _web3.fromWei(_web3.eth.getBalance(_address), unit);
    },
    getNetworkId: function() {
        return _web3.version.network;
    },
    getTransactionCount: function() {
        return _web3.eth.getTransactionCount(_address);
    },
    getBlockNumber: function () { 
        return _web3.eth.blockNumber;
    },
    sign: function(dataToSign) {
        let signedMsg = _signing.signMsg(_keyStore, _pwDerivedKey, dataToSign, _address);
              
        let wrapper = {r: nacl.util.encodeBase64(signedMsg.r), s: nacl.util.encodeBase64(signedMsg.s), v: signedMsg.v};
       
        return _web3.toHex(JSON.stringify(wrapper));
    },
    isConnected: function () {
        if (!_web3)
            return false;

        return _web3.isConnected();
    },
    getProviderUri: function() {
        return _providerUri;
    },
    getBlockGasLimit: function() {
        return _web3.eth.getBlock("latest").gasLimit; 
    },
    getAvarageGasPriceBlock: function () {
        return _web3.eth.gasPrice; //This property is read only and returns the current gas price. The gas price is determined by the x latest blocks median gas price.
    }
  }
})();