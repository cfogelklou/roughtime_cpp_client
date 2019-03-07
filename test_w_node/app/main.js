// @flow
import * as Roughtime from 'roughtime'


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
  Roughtime('roughtime.cloudflare.com', (err, result) => {
    if (err) throw err
    const {midpoint, radius} = result
    console.log(midpoint, radius) // ex. "1537907399109000 1000000"
    console.log(new Date(midpoint / 1e3)) // ex. "2018-09-25T20:29:59.109Z"
  })
}
