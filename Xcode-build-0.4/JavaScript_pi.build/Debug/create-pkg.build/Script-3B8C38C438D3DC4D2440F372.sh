#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4
  /usr/local/bin/packagesbuild -F /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4 /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/JavaScript.pkgproj
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4
  /usr/local/bin/packagesbuild -F /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4 /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/JavaScript.pkgproj
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4
  /usr/local/bin/packagesbuild -F /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4 /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/JavaScript.pkgproj
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4
  /usr/local/bin/packagesbuild -F /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4 /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/JavaScript.pkgproj
fi

