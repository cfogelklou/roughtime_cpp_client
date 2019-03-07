#!/bin/bash
#You need to have run build_for_node.sh to "compile" to raw js, first.
echo To debug this, open chrome://inspect
#node --inspect compiled/app/main_compiled.js
node --inspect-brk compiled/app/main_compiled.js

