cp .babelrc.bak .babelrc
npm run build
#cp reactnative/pakutils.js compiled/reactnative/
rm .babelrc
node compiled/app/main_compiled.js

