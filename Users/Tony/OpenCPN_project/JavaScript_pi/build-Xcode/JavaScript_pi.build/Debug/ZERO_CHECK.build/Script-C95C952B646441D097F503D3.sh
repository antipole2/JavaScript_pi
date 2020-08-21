#!/bin/sh
make -C /Users/Tony/OpenCPN_project/JavaScript_pi/build -f /Users/Tony/OpenCPN_project/JavaScript_pi/build/CMakeScripts/ZERO_CHECK_cmakeRulesBuildPhase.make$CONFIGURATION OBJDIR=$(basename "$OBJECT_FILE_DIR_normal") all
