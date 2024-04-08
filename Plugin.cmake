# ~~~
# Summary:      Local, non-generic plugin setup
# Copyright (c) 2020-2022 Tony Voss
# License:      GPLv3+
# ~~~

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.


# -------- Options ----------

set(OCPN_TEST_REPO
    "opencpn/javascript-alpha"
    CACHE STRING "Default repository for untagged builds"
)
set(OCPN_BETA_REPO
    "opencpn/javascript-beta"
    CACHE STRING
    "Default repository for tagged builds matching 'beta'"
)
set(OCPN_RELEASE_REPO
    "opencpn/javascript-prod"
    CACHE STRING
    "Default repository for tagged builds not matching 'beta'"
)

option(JavaScript_USE_SVG "Use SVG graphics" ON)

#
#
# -------  Plugin setup --------
#
set(PKG_NAME JavaScript_pi)
set(PKG_VERSION  3.0.1)
set(PKG_PRERELEASE "")  # Empty, or a tag like 'beta'

set(DISPLAY_NAME javascript)    # Dialogs, installer artifacts, ...
set(PLUGIN_API_NAME javascript) # As of GetCommonName() in plugin API
set(PKG_SUMMARY "Run JavaScripts in the plugin")
set(PKG_DESCRIPTION [=[
This plugin for OpenCPN allows you to run JavaScript and to interact with OpenCPN.
You can use it to script your own enhancements to standard OpenCPN functionality.
See help menu ? for documentation details
]=])

set(PKG_AUTHOR "Tony Voss")
set(PKG_IS_OPEN_SOURCE "yes")
set(PKG_HOMEPAGE https://github.com/Antipole2/JavaScript_pi)
set(PKG_INFO_URL https://opencpn-manuals.github.io/main/javascript/index.html)

SET(SRC
	src/bitmaps.cpp
	src/consolePositioning.h
    src/duk_config.h
    src/duktape.cpp
    src/duktape.h
    src/fileOrganiser.cpp    
    src/functions.cpp
    src/icons.cpp
    src/icons.h
    src/ipc.cpp
    src/JavaScript_pi.cpp
    src/JavaScript_pi.h
    src/JavaScriptgui_impl.cpp
    src/JavaScriptgui_impl.h
    src/JavaScriptgui.cpp
    src/JavaScriptgui.h
    src/JSdialog.cpp
    src/JSExtensions.cpp
    src/JSlexer.cpp
    src/menus.cpp
    src/ocpn_duk.h
    src/OPCNapis.cpp
    src/optional.cpp
    src/drivers.cpp
    src/sockets.cpp
    src/toolsDialogGui.cpp
    src/toolsDialogGui.h
    src/toolsDialogImp.cpp
    src/toolsDialogImp.h
    src/drivers.cpp
    src/trace.h
    src/version.h
    )

set(PKG_API_LIB api-18)  #  A directory in libs/ e. g., api-17 or api-16

macro(late_init)
  # Perform initialization after the PACKAGE_NAME library, compilers
  # and ocpn::api is available.
  if (JavaScript_USE_SVG)
    add_definitions(-DJavaScript_USE_SVG)
  endif ()
endmacro ()

macro(add_plugin_libraries)
  # Add libraries required by this plugin
  add_subdirectory("libs/tinyxml")
  target_link_libraries(${PACKAGE_NAME} ocpn::tinyxml)

  add_subdirectory("libs/wxJSON")
  target_link_libraries(${PACKAGE_NAME} ocpn::wxjson)
  
#  add_subdirectory("libs/plugingl")
#  target_link_libraries(${PACKAGE_NAME} ocpn::plugingl)


#   add_subdirectory("libs/jsoncpp")
#   target_link_libraries(${PACKAGE_NAME} ocpn::jsoncpp)
endmacro ()
 