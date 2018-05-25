'use strict';

// this is how we will require our module
const blonde = require('./build/Release/blonde')

console.log(blonde.initialize());
const tokenBuffer = blonde.highlight("public struct DiskSampler2D \n\
{\n\
    private\n\
    let candidate_ring:[Math.DoubleV2]\n\
}\n\
");
const count = tokenBuffer.length >> 1;
for (let i = 0; i < count; ++i)
{
    console.log(tokenBuffer.readUInt16LE(i << 1));
}
console.log(blonde.deinitialize());
