#!/bin/sh
make -C /Users/Tony/OpenCPN_project/JavaScript-project/Xcode-build-0.3 -f /Users/Tony/OpenCPN_project/JavaScript-project/Xcode-build-0.3/CMakeScripts/JavaScript_pi_postBuildPhase.make$CONFIGURATION OBJDIR=$(basename "$OBJECT_FILE_DIR_normal") all
cd Xcode-build-0.4
cp Debug/libJavaScript_pi.dylib libJavaScript_pi.dylib
make -C . -f CMakeScripts/create-pkg_cmakeRulesBuildPhase.make$CONFIGURATION OBJDIR=$(basename "$OBJECT_FILE_DIR_normal") all


