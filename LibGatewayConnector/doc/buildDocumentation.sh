#!/bin/bash
GATEWAY_VERSION=`git describe '--match=release-*'`

which doxygen > /dev/null 2>&1
DOXYGEN_INSTALLED=$?
which dot > /dev/null 2>&1
DOT_INSTALLED=$?

if [[ ( $DOXYGEN_INSTALLED != 0 ) || ( $DOT_INSTALLED != 0 ) ]]; then
echo "Doxygen and dot must be installed to build this documentation."
echo "To install on ubuntu, run 'sudo apt-get install doxygen dot'."
exit
fi

echo "Building documentation for LibGatewayConnector"

( cat Doxyfile ; echo "PROJECT_NUMBER=${GATEWAY_VERSION}" ) | doxygen -

if [[ $? == 0 ]]; then
echo "Documentation build succeeded...  zipping up documentation."
mv html gateway-doc-$GATEWAY_VERSION
zip -r gateway-doc-$GATEWAY_VERSION.zip gateway-doc-$GATEWAY_VERSION
mv gateway-doc-$GATEWAY_VERSION html
fi
