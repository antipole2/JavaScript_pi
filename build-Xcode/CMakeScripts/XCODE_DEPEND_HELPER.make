# DO NOT EDIT
# This makefile makes sure all linkable targets are
# up-to-date with anything they link to
default:
	echo "Do not invoke directly"

# Rules to remove targets that are older than anything to which they
# link.  This forces Xcode to relink the targets from scratch.  It
# does not seem to check these dependencies itself.
PostBuild.JavaScript_pi.Debug:
/Users/Tony/OpenCPN_project/JavaScript_pi/build-Xcode/Debug/libJavaScript_pi.dylib:\
	/usr/lib/libz.dylib
	/bin/rm -f /Users/Tony/OpenCPN_project/JavaScript_pi/build-Xcode/Debug/libJavaScript_pi.dylib


PostBuild.JavaScript_pi.Release:
/Users/Tony/OpenCPN_project/JavaScript_pi/build-Xcode/Release/libJavaScript_pi.dylib:\
	/usr/lib/libz.dylib
	/bin/rm -f /Users/Tony/OpenCPN_project/JavaScript_pi/build-Xcode/Release/libJavaScript_pi.dylib


PostBuild.JavaScript_pi.MinSizeRel:
/Users/Tony/OpenCPN_project/JavaScript_pi/build-Xcode/MinSizeRel/libJavaScript_pi.dylib:\
	/usr/lib/libz.dylib
	/bin/rm -f /Users/Tony/OpenCPN_project/JavaScript_pi/build-Xcode/MinSizeRel/libJavaScript_pi.dylib


PostBuild.JavaScript_pi.RelWithDebInfo:
/Users/Tony/OpenCPN_project/JavaScript_pi/build-Xcode/RelWithDebInfo/libJavaScript_pi.dylib:\
	/usr/lib/libz.dylib
	/bin/rm -f /Users/Tony/OpenCPN_project/JavaScript_pi/build-Xcode/RelWithDebInfo/libJavaScript_pi.dylib




# For each target create a dummy ruleso the target does not have to exist
/usr/lib/libz.dylib:
