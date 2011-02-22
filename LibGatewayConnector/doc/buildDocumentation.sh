#!/bin/bash
( cat Doxyfile ; echo "PROJECT_NUMBER=`git describe '--match=release-*'`" ) | doxygen -
