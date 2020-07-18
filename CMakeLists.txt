##---------------------------------------------------------------------------
## Author:      Tony Voss
## Copyright:
## License:     wxWidgets License
##---------------------------------------------------------------------------

# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)

PROJECT(JavaScript_pi)

SET(PACKAGE_NAME JavaScript_pi)
SET(VERBOSE_NAME JavaScript)
SET(TITLE_NAME JS)
SET(CPACK_PACKAGE_CONTACT "Tony Voss")

SET(VERSION_MAJOR "0")
SET(VERSION_MINOR "0.0")

SET(NAME_SUFFIX "ov50")

set (CMAKE_CXX_STANDARD 11)

#SET(CMAKE_BUILD_TYPE Release)
SET( wxWidgets_USE_DEBUG OFF)
SET( wxWidgets_USE_UNICODE ON)
SET( wxWidgets_USE_UNIVERSAL OFF)
SET( wxWidgets_USE_STATIC OFF)

#SET(CMAKE_BUILD_TYPE Release)
OPTION(JavaScript_USE_SVG "Use SVG graphics" ON)

INCLUDE("cmake/PluginConfigure.cmake")

IF(JavaScript_USE_SVG)
  ADD_DEFINITIONS(-DJavaScript_USE_SVG)
ENDIF(JavaScript_USE_SVG)

SET(SRC_JavaScript
        src/JavaScript_pi.h
        src/JavaScript_pi.cpp
        src/icons.h
        src/icons.cpp
	    src/JavaScriptgui.h
	    src/JavaScriptgui.cpp
	    src/JavaScriptgui_impl.cpp
	    src/JavaScriptgui_impl.h
	    src/NavFunc.h
	    src/NavFunc.cpp
		src/ocpn_plugin.h
		src/tinyxml.cpp
        src/tinyxmlerror.cpp
        src/tinyxmlparser.cpp
        src/tinyxml.h
	)


ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_JavaScript})

ADD_DEFINITIONS(-DTIXML_USE_STL)

INCLUDE("cmake/PluginInstall.cmake")
INCLUDE("cmake/PluginLocalization.cmake")
INCLUDE("cmake/PluginPackage.cmake")
