const { Api, JsonRpc, RpcError } = require("eosjs");
const { JsSignatureProvider } = require("eosjs/dist/eosjs-jssig"); // development only
const fetch = require("node-fetch"); // node only; not needed in browsers
const { TextEncoder, TextDecoder } = require("util"); // node only; native TextEncoder/Decoder

const defaultPrivateKey = "your private key"; // 
const signatureProvider = new JsSignatureProvider([defaultPrivateKey]);
let rpc;
let nonce =0;
const rpc0 = new JsonRpc("https://eos.newdex.one", { fetch });
const rpc1 = new JsonRpc("https://eospush.tokenpocket.pro", { fetch });
const rpc2 = new JsonRpc("http://openapi.eos.ren", { fetch });
const rpc3 = new JsonRpc("https://eos.greymass.com", { fetch });
const rpc4 = new JsonRpc("http://api.hkeos.com", { fetch });
const rpc5 = new JsonRpc("https://nodes.get-scatter.com", { fetch });
const rpc6 = new JsonRpc("https://api.eossweden.se", { fetch });
const rpc7 = new JsonRpc("https://api.eoslaomao.com", { fetch });
const rpc8 = new JsonRpc("https://api.eosn.io", { fetch });
const rpc9 = new JsonRpc("https://api.main.alohaeos.com", { fetch });

const http = require("./framework/httpClient");
const _ = require("lodash");
const moment = require("moment");

global.sleep = async (timeout) => {
  return new Promise((res, rej) =>
    setTimeout(() => {
      return res();
    }, timeout)
  );
};

class Runner {
 
   run(change) {
      if(change ==0){rpc =rpc0;}
      if(change ==1){rpc =rpc1;}
      if(change ==2){rpc =rpc2;}
      if(change ==3){rpc =rpc3;}
      if(change ==4){rpc =rpc4;}
      if(change ==5){rpc =rpc5;}
      if(change ==6){rpc =rpc6;}
      if(change ==7){rpc =rpc7;} 
      if(change ==8){rpc =rpc8;} 
      if(change ==9){rpc =rpc9;} 
      if(change ==10){rpc =rpc10;} 
      if(change ==11){rpc =rpc11;}
      if(change ==12){rpc =rpc12;}
      if(change ==13){rpc =rpc13;}
      if(change ==14){rpc =rpc14;}
      if(change ==15){rpc =rpc15;}
      console.log(`\n[${moment().format("YYYY-MM-DD HH:mm:ss")}]`+ JSON.stringify(rpc));

      const api = new Api({
        rpc,
        signatureProvider,
        textDecoder: new TextDecoder(),
        textEncoder: new TextEncoder(),
      }); 
      let time =  parseInt(Math.random()*moment().format('X'),10);
      const result =  api.transact(
        {
          actions: [

            { account: "youraccount",
              name: "detect",
              authorization: [{ actor: "youraccount",permission: "active",}],
              data: {time},
            },
          ],
        },
        { blocksBehind: 3,
          expireSeconds: 30,
        }
      );      
      nonce++;
  }
  async start() {
    while (true) {      

      for(let i =0;i<10;i++){
        this.run(i);
        // try{this.run(i);}
        // catch(e){}
      }
      console.log(`-----------------------------------`);
      await sleep(100);
    }
  }
}
new Runner().start();



  

