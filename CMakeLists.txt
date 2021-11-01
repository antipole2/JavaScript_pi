##---------------------------------------------------------------------------
## Author:      Tony Voss
## Copyright:
## License:     wxWidgets License
##---------------------------------------------------------------------------

# -------- Cmake setup ---------
#
cmake_minimum_required(VERSION 3.12.0)

cmake_policy(SET CMP0042 NEW)

if (POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW)
endif ()

if (POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif ()

# Locations where cmake looks for cmake modules.
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/build ${CMAKE_SOURCE_DIR}/cmake)

if (WIN32)
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/buildwin)
endif ()
message(STATUS "Cmake version: ${CMAKE_VERSION}.")


# -------- Build setup (mostly in PluginCompiler) --------
#
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
message(STATUS "Building: ${BUILD_TYPE}")

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

set(OCPN_TARGET_TUPLE "" CACHE STRING
  "Target spec: \"platform;version;arch\""
)

string(TOLOWER "${OCPN_TARGET_TUPLE}" _lc_target)
if ("${_lc_target}" MATCHES "android*")
  set(QT_ANDROID ON)
  # Until we have a proper toolchain file:
  set(CMAKE_CROSSCOMPILING ON)
else ()
  set(QT_ANDROID OFF)
  add_definitions(-D__OCPN_USE_CURL__)
endif ()
#
#
# -------  Plugin setup --------
#
project(javascript_pi VERSION 0.5.0.0)
set(PKG_PRERELEASE "")  # Empty, or a tag like 'beta'

set(DISPLAY_NAME javascript)    # Dialogs, installer artifacts, ...
set(PLUGIN_API_NAME javascript) # As of GetCommonName() in plugin API
set(CPACK_PACKAGE_CONTACT "Mike Rossiter")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Run JavaScripts in the plugin")
set(CPACK_PACKAGE_DESCRIPTION [=[
This plugin for OpenCPN allows you to run JavaScript and to interact with OpenCPN.
You can use it to script your own enhancements to standard OpenCPN functionality.
]=])

set(PKG_AUTHOR "Tony Voss")
set(PKG_IS_OPEN_SOURCE "yes")
set(CPACK_PACKAGE_HOMEPAGE_URL https://github.com/antipole2/javascript_pi)
set(PKG_INFO_URL https://opencpn-manuals.github.io/plugins/javascript/0.1/index.html)


SET(SRC
    src/version.h
    src/trace.h
    src/JavaScript_pi.h
    src/JavaScript_pi.cpp
    src/icons.h
    src/icons.cpp
    src/JavaScriptgui.h
    src/JavaScriptgui.cpp
    src/JavaScriptgui_impl.cpp
    src/JavaScriptgui_impl.h
    src/JSExtensions.cpp
    src/ocpn_duk.h
    src/OPCNapis.cpp
    src/duk_config.h
    src/duktape.h
    src/duktape.cpp
    src/JSlexer.cpp
    src/JSdialog.cpp
    src/functions.cpp
    src/optional.cpp
    src/toolsDialogGui.h
    src/toolsDialogGui.cpp
    src/toolsDialogImp.h
    src/toolsDialogImp.cpp
    )

#IF 0
SET(SCINTILLA
    scintilla/ScintillaWX.h
    scintilla/ScintillaWX.cpp
    scintilla/stc.cpp
    scintilla/include/ILexer.h
    scintilla/include/Platform.h
    scintilla/include/SciLexer.h
    scintilla/include/Sci_Position.h
    scintilla/include/Scintilla.h
    scintilla/include/Scintilla.iface
    scintilla/include/ScintillaWidget.h
    scintilla/lexers/ILexer.h
    scintilla/lexers/LexA68k.cxx
    scintilla/lexers/LexAPDL.cxx
    scintilla/lexers/LexASY.cxx
    scintilla/lexers/LexAU3.cxx
    scintilla/lexers/LexAVE.cxx
    scintilla/lexers/LexAVS.cxx
    scintilla/lexers/LexAbaqus.cxx
    scintilla/lexers/LexAda.cxx
    scintilla/lexers/LexAsm.cxx
    scintilla/lexers/LexAsn1.cxx
    scintilla/lexers/LexBaan.cxx
    scintilla/lexers/LexBash.cxx
    scintilla/lexers/LexBasic.cxx
    scintilla/lexers/LexBatch.cxx
    scintilla/lexers/LexBibTeX.cxx
    scintilla/lexers/LexBullant.cxx
    scintilla/lexers/LexCLW.cxx
    scintilla/lexers/LexCOBOL.cxx
    scintilla/lexers/LexCPP.cxx
    scintilla/lexers/LexCSS.cxx
    scintilla/lexers/LexCaml.cxx
    scintilla/lexers/LexCmake.cxx
    scintilla/lexers/LexCoffeeScript.cxx
    scintilla/lexers/LexConf.cxx
    scintilla/lexers/LexCrontab.cxx
    scintilla/lexers/LexCsound.cxx
    scintilla/lexers/LexD.cxx
    scintilla/lexers/LexDMAP.cxx
    scintilla/lexers/LexDMIS.cxx
    scintilla/lexers/LexDiff.cxx
    scintilla/lexers/LexECL.cxx
    scintilla/lexers/LexEDIFACT.cxx
    scintilla/lexers/LexEScript.cxx
    scintilla/lexers/LexEiffel.cxx
    scintilla/lexers/LexErlang.cxx
    scintilla/lexers/LexErrorList.cxx
    scintilla/lexers/LexFlagship.cxx
    scintilla/lexers/LexForth.cxx
    scintilla/lexers/LexFortran.cxx
    scintilla/lexers/LexGAP.cxx
    scintilla/lexers/LexGui4Cli.cxx
    scintilla/lexers/LexHTML.cxx
    scintilla/lexers/LexHaskell.cxx
    scintilla/lexers/LexHex.cxx
    scintilla/lexers/LexInno.cxx
    scintilla/lexers/LexJSON.cxx
    scintilla/lexers/LexKVIrc.cxx
    scintilla/lexers/LexKix.cxx
    scintilla/lexers/LexLaTeX.cxx
    scintilla/lexers/LexLisp.cxx
    scintilla/lexers/LexLout.cxx
    scintilla/lexers/LexLua.cxx
    scintilla/lexers/LexMMIXAL.cxx
    scintilla/lexers/LexMPT.cxx
    scintilla/lexers/LexMSSQL.cxx
    scintilla/lexers/LexMagik.cxx
    scintilla/lexers/LexMake.cxx
    scintilla/lexers/LexMarkdown.cxx
    scintilla/lexers/LexMatlab.cxx
    scintilla/lexers/LexMetapost.cxx
    scintilla/lexers/LexModula.cxx
    scintilla/lexers/LexMySQL.cxx
    scintilla/lexers/LexNimrod.cxx
    scintilla/lexers/LexNsis.cxx
    scintilla/lexers/LexNull.cxx
    scintilla/lexers/LexOScript.cxx
    scintilla/lexers/LexOpal.cxx
    scintilla/lexers/LexPB.cxx
    scintilla/lexers/LexPLM.cxx
    scintilla/lexers/LexPO.cxx
    scintilla/lexers/LexPOV.cxx
    scintilla/lexers/LexPS.cxx
    scintilla/lexers/LexPascal.cxx
    scintilla/lexers/LexPerl.cxx
    scintilla/lexers/LexPowerPro.cxx
    scintilla/lexers/LexPowerShell.cxx
    scintilla/lexers/LexProgress.cxx
    scintilla/lexers/LexProps.cxx
    scintilla/lexers/LexPython.cxx
    scintilla/lexers/LexR.cxx
    scintilla/lexers/LexRebol.cxx
    scintilla/lexers/LexRegistry.cxx
    scintilla/lexers/LexRuby.cxx
    scintilla/lexers/LexRust.cxx
    scintilla/lexers/LexSML.cxx
    scintilla/lexers/LexSQL.cxx
    scintilla/lexers/LexSTTXT.cxx
    scintilla/lexers/LexScriptol.cxx
    scintilla/lexers/LexSmalltalk.cxx
    scintilla/lexers/LexSorcus.cxx
    scintilla/lexers/LexSpecman.cxx
    scintilla/lexers/LexSpice.cxx
    scintilla/lexers/LexTACL.cxx
    scintilla/lexers/LexTADS3.cxx
    scintilla/lexers/LexTAL.cxx
    scintilla/lexers/LexTCL.cxx
    scintilla/lexers/LexTCMD.cxx
    scintilla/lexers/LexTeX.cxx
    scintilla/lexers/LexTxt2tags.cxx
    scintilla/lexers/LexVB.cxx
    scintilla/lexers/LexVHDL.cxx
    scintilla/lexers/LexVerilog.cxx
    scintilla/lexers/LexVisualProlog.cxx
    scintilla/lexers/LexYAML.cxx
    scintilla/lexlib/Accessor.cxx
    scintilla/lexlib/Accessor.h
    scintilla/lexlib/CharacterCategory.cxx
    scintilla/lexlib/CharacterCategory.h
    scintilla/lexlib/CharacterSet.cxx
    scintilla/lexlib/CharacterSet.h
    scintilla/lexlib/LexAccessor.h
    scintilla/lexlib/LexerBase.cxx
    scintilla/lexlib/LexerBase.h
    scintilla/lexlib/LexerModule.cxx
    scintilla/lexlib/LexerModule.h
    scintilla/lexlib/LexerNoExceptions.cxx
    scintilla/lexlib/LexerNoExceptions.h
    scintilla/lexlib/LexerSimple.cxx
    scintilla/lexlib/LexerSimple.h
    scintilla/lexlib/OptionSet.h
    scintilla/lexlib/PropSetSimple.cxx
    scintilla/lexlib/PropSetSimple.h
    scintilla/lexlib/SparseState.h
    scintilla/lexlib/StringCopy.h
    scintilla/lexlib/StyleContext.cxx
    scintilla/lexlib/StyleContext.h
    scintilla/lexlib/SubStyles.h
    scintilla/lexlib/WordList.cxx
    scintilla/lexlib/WordList.h
    scintilla/src/AutoComplete.cxx
    scintilla/src/AutoComplete.h
    scintilla/src/CallTip.cxx
    scintilla/src/CallTip.h
    scintilla/src/CaseConvert.cxx
    scintilla/src/CaseConvert.h
    scintilla/src/CaseFolder.cxx
    scintilla/src/CaseFolder.h
    scintilla/src/Catalogue.cxx
    scintilla/src/Catalogue.h
    scintilla/src/CellBuffer.cxx
    scintilla/src/CellBuffer.h
    scintilla/src/CharClassify.cxx
    scintilla/src/CharClassify.h
    scintilla/src/ContractionState.cxx
    scintilla/src/ContractionState.h
    scintilla/src/Decoration.cxx
    scintilla/src/Decoration.h
    scintilla/src/Document.cxx
    scintilla/src/Document.h
    scintilla/src/EditModel.cxx
    scintilla/src/EditModel.h
    scintilla/src/EditView.cxx
    scintilla/src/EditView.h
    scintilla/src/Editor.cxx
    scintilla/src/Editor.h
    scintilla/src/ExternalLexer.cxx
    scintilla/src/ExternalLexer.h
    scintilla/src/FontQuality.h
    scintilla/src/Indicator.cxx
    scintilla/src/Indicator.h
    scintilla/src/KeyMap.cxx
    scintilla/src/KeyMap.h
    scintilla/src/LineMarker.cxx
    scintilla/src/LineMarker.h
    scintilla/src/MarginView.cxx
    scintilla/src/MarginView.h
    scintilla/src/Partitioning.h
    scintilla/src/PerLine.cxx
    scintilla/src/PerLine.h
    scintilla/src/PlatWX.cpp
    scintilla/src/PlatWX.h
    scintilla/src/Position.h
    scintilla/src/PositionCache.cxx
    scintilla/src/PositionCache.h
    scintilla/src/RESearch.cxx
    scintilla/src/RESearch.h
    scintilla/src/RunStyles.cxx
    scintilla/src/RunStyles.h
    scintilla/src/SciTE.properties
    scintilla/src/ScintillaBase.cxx
    scintilla/src/ScintillaBase.h
    scintilla/src/Selection.cxx
    scintilla/src/Selection.h
    scintilla/src/SparseVector.h
    scintilla/src/SplitVector.h
    scintilla/src/Style.cxx
    scintilla/src/Style.h
    scintilla/src/UniConversion.cxx
    scintilla/src/UniConversion.h
    scintilla/src/UnicodeFromUTF8.h
    scintilla/src/ViewStyle.cxx
    scintilla/src/ViewStyle.h
    scintilla/src/XPM.cxx
    scintilla/src/XPM.h
	)

#IF(APPLE)

#    add_library(${CMAKE_PROJECT_NAME}
#        ${SCINTILLA}
#        )

#ENDIF(APPLE)
#ENDIF #0

ADD_DEFINITIONS(-DSCI_LEXER)

# -------- Setup completed, build the plugin --------
#
add_library(${CMAKE_PROJECT_NAME} SHARED ${SRC})
include_directories(BEFORE ${CMAKE_BINARY_DIR}/include)

add_subdirectory("libs/api-17")
target_link_libraries(${CMAKE_PROJECT_NAME} ocpn::api)

set(VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(VERSION_PATCH ${PROJECT_VERSION_PATCH})

set(PACKAGE_NAME ${CMAKE_PROJECT_NAME})

# Create the minimim ocpn version id string like ov50
string(REGEX REPLACE "\([0-9]\)\.\([0-9]\).*" "ov\\1\\2"
       OCPN_MIN_VERSION ${API_OCPN_MIN_VERSION})

if (JavaScript_USE_SVG)
  add_definitions(-DJavaScript_USE_SVG)
endif ()

# Set up targets. Targets sets up a recursive call with BUILD_TYPE set to
# 'flatpak', 'pkg' or 'tarball'. The initial call without BUILD_TYPE ends
# here.
#
include(Targets)
create_targets(
  ${PROJECT_SOURCE_DIR}/flatpak/org.opencpn.OpenCPN.Plugin.javascript.yaml
)
if ("${BUILD_TYPE}" STREQUAL "")
  return ()
endif ()

if (NOT ${BUILD_TYPE} STREQUAL "flatpak")
  # Build package as required (flatpak already dealt with).
  #
  include(PluginCompiler)
  include(PluginInstall)
  include(PluginPackage)
  if (QT_ANDROID)
    include(AndroidLibs)
  else ()
    include(PluginLibs)
  endif ()
  include(PluginLocalization)

  include_directories(BEFORE ${CMAKE_BINARY_DIR}/include)

  add_subdirectory("libs/jsoncpp")
  target_link_libraries(${PACKAGE_NAME} ocpn::jsoncpp)

  add_subdirectory("libs/tinyxml")
  target_link_libraries(${PACKAGE_NAME} ocpn::tinyxml)

  add_subdirectory("libs/wxJSON")
  target_link_libraries(${PACKAGE_NAME} ocpn::wxjson)
endif ()

configure_file(
  ${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/include//config.h
  @ONLY
)

configure_file(
  # The cloudsmith upload script
  ${CMAKE_SOURCE_DIR}/ci/upload.sh.in ${CMAKE_BINARY_DIR}/upload.sh
  @ONLY
)
configure_file(
  # The cloudsmith upload script, windows bat file.
  ${CMAKE_SOURCE_DIR}/ci/upload.bat.in ${CMAKE_BINARY_DIR}/upload.bat
  @ONLY
)


set(checksum "@checksum@")
configure_file(
  # The XML metadata file
  ${CMAKE_SOURCE_DIR}/plugin.xml.in
  ${CMAKE_BINARY_DIR}/${pkg_displayname}.xml.in
  @ONLY
)

