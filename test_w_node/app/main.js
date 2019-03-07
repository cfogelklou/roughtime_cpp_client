// @flow
const roughtime = require('roughtime')

export function go() {
  roughtime('roughtime.cloudflare.com', (err, result) => {
    if (err) throw err
    const {midpoint, radius} = result
    console.log(midpoint, radius) // ex. "1537907399109000 1000000"
    console.log(new Date(midpoint / 1e3)) // ex. "2018-09-25T20:29:59.109Z"
  })
}
