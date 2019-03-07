// @flow

import * as FirebaseApi from './firebase-api.js';
import * as ossl from './openssl_utils';
import * as debug from './debug';
import {prompt} from './command_handler';

import * as Logger from './logger';
let logger = Logger.getInst();

function VerifyOpenssl(){
  let version = ossl.version();
  let verIdx = version.indexOf('1.1.');
  if ((verIdx >= 0) && (verIdx < version.length)){

  }
  else {
    console.log("This tool requires openssl 1.1.x");
    debug.assert(verIdx >= 0);
    debug.assert(verIdx < version.length);
  }
}

function trimQuotes(input:string):string{
  let out = input;
  let quote = '"'.charCodeAt(0);
  let b = out.charCodeAt(0);
  if (b === quote){
    out = out.substr(1);
  }
  let l = out.charCodeAt(out.length-1)
  if (l === quote){
    out = out.substr(0, out.length-1);
  }
  return out;
}

export function go() {
  // $FlowFixMe
  console.log = logger.log;
// $FlowFixMe
  console.error = logger.error;

  if (process.argv.length >= 4){

    VerifyOpenssl();
    let firebaseApi = new FirebaseApi.FirebaseApi();
    let usr = trimQuotes(process.argv[2]);
    let pass = trimQuotes(process.argv[3]);

    firebaseApi.start(usr,pass, (started:boolean)=>{
      if (started){
        FirebaseApi.getLastProvisionedSN(
          (sn:number)=>{
            ossl.init(sn+1);
          }
        );
      }
      else {
        console.log("Failed to log into firebase");
      }
    });

    prompt(process.argv[4]);
  }
  else {
    console.log("Enter username and password when starting process.")
    console.log('fact_init <username> "<password>" "<port>"');
    console.log('Ex: fact_init altran@altran.com password "/dev/tty.usbserial-FT9ZOUGE"');
    console.log('<port> is optional.');
  }
}
