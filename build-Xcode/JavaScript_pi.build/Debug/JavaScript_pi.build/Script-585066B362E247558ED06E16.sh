#!/bin/sh
make -C /Users/Tony/OpenCPN_project/JavaScript_pi/build-Xcode -f /Users/Tony/OpenCPN_project/JavaScript_pi/build-Xcode/CMakeScripts/JavaScript_pi_postBuildPhase.make$CONFIGURATION OBJDIR=$(basename "$OBJECT_FILE_DIR_normal") all
cd build-Xcode
cp Debug/libJavaScript_pi.dylib libJavaScript_pi.dylib
make -C . -f CMakeScripts/create-pkg_cmakeRulesBuildPhase.make$CONFIGURATION OBJDIR=$(basename "$OBJECT_FILE_DIR_normal") all

