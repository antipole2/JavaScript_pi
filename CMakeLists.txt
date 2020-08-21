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
SET(PROJECT_NAME JavaScript)

SET(TITLE_NAME JS)
SET(CPACK_PACKAGE_CONTACT "Tony Voss")

SET(VERSION_MAJOR "0")
SET(VERSION_MINOR "2.0")

SET(NAME_SUFFIX "ov50")

set (CMAKE_CXX_STANDARD 11)
set(CMAKE_MACOSX_RPATH 1)

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

SET(JAVASCRIPT
    src/version.h
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
    src/runJS.cpp
    src/harness.h
    src/ocpn_plugin.h
    src/duk_config.h
    src/duktape.h
    src/duktape.cpp
    )
    
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
ADD_library(${PACKAGE_NAME}
	${JAVASCRIPT}
    ${SCINTILLA}
    )

ADD_DEFINITIONS(-DSCI_LEXER)

INCLUDE("cmake/PluginInstall.cmake")
INCLUDE("cmake/PluginLocalization.cmake")
INCLUDE("cmake/PluginPackage.cmake")
