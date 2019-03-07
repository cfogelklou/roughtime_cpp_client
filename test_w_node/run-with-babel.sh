#!/bin/bash
cp .babelrc.bak .babelrc
babel-node app/main_babel_node.js
rm .babelrc

