#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4
  /usr/local/Cellar/cmake/3.19.4/bin/cmake -E cmake_symlink_library /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/Debug/libJavaScript_pi.dylib /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/Debug/libJavaScript_pi.dylib /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/Debug/libJavaScript_pi.dylib
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4
  /usr/local/Cellar/cmake/3.19.4/bin/cmake -E cmake_symlink_library /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/Release/libJavaScript_pi.dylib /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/Release/libJavaScript_pi.dylib /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/Release/libJavaScript_pi.dylib
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4
  /usr/local/Cellar/cmake/3.19.4/bin/cmake -E cmake_symlink_library /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/MinSizeRel/libJavaScript_pi.dylib /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/MinSizeRel/libJavaScript_pi.dylib /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/MinSizeRel/libJavaScript_pi.dylib
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4
  /usr/local/Cellar/cmake/3.19.4/bin/cmake -E cmake_symlink_library /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/RelWithDebInfo/libJavaScript_pi.dylib /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/RelWithDebInfo/libJavaScript_pi.dylib /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/RelWithDebInfo/libJavaScript_pi.dylib
fi
cd /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4
cp Debug/libJavaScript_pi.dylib libJavaScript_pi.dylib

