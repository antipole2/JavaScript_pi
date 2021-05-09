# Install script for directory: /Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/SharedSupport/plugins/libJavaScript_pi.dylib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/SharedSupport/plugins" TYPE SHARED_LIBRARY FILES "/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/Debug/libJavaScript_pi.dylib")
    if(EXISTS "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/SharedSupport/plugins/libJavaScript_pi.dylib" AND
       NOT IS_SYMLINK "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/SharedSupport/plugins/libJavaScript_pi.dylib")
      execute_process(COMMAND /usr/bin/install_name_tool
        -delete_rpath "/usr/local/lib"
        "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/SharedSupport/plugins/libJavaScript_pi.dylib")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip" -x "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/SharedSupport/plugins/libJavaScript_pi.dylib")
      endif()
    endif()
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/SharedSupport/plugins/libJavaScript_pi.dylib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/SharedSupport/plugins" TYPE SHARED_LIBRARY FILES "/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/Release/libJavaScript_pi.dylib")
    if(EXISTS "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/SharedSupport/plugins/libJavaScript_pi.dylib" AND
       NOT IS_SYMLINK "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/SharedSupport/plugins/libJavaScript_pi.dylib")
      execute_process(COMMAND /usr/bin/install_name_tool
        -delete_rpath "/usr/local/lib"
        "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/SharedSupport/plugins/libJavaScript_pi.dylib")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip" -x "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/SharedSupport/plugins/libJavaScript_pi.dylib")
      endif()
    endif()
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/SharedSupport/plugins/libJavaScript_pi.dylib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/SharedSupport/plugins" TYPE SHARED_LIBRARY FILES "/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/MinSizeRel/libJavaScript_pi.dylib")
    if(EXISTS "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/SharedSupport/plugins/libJavaScript_pi.dylib" AND
       NOT IS_SYMLINK "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/SharedSupport/plugins/libJavaScript_pi.dylib")
      execute_process(COMMAND /usr/bin/install_name_tool
        -delete_rpath "/usr/local/lib"
        "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/SharedSupport/plugins/libJavaScript_pi.dylib")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip" -x "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/SharedSupport/plugins/libJavaScript_pi.dylib")
      endif()
    endif()
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/SharedSupport/plugins/libJavaScript_pi.dylib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/SharedSupport/plugins" TYPE SHARED_LIBRARY FILES "/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/RelWithDebInfo/libJavaScript_pi.dylib")
    if(EXISTS "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/SharedSupport/plugins/libJavaScript_pi.dylib" AND
       NOT IS_SYMLINK "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/SharedSupport/plugins/libJavaScript_pi.dylib")
      execute_process(COMMAND /usr/bin/install_name_tool
        -delete_rpath "/usr/local/lib"
        "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/SharedSupport/plugins/libJavaScript_pi.dylib")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip" -x "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/SharedSupport/plugins/libJavaScript_pi.dylib")
      endif()
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/PlugIns/libJavaScript_pi.dylib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/PlugIns" TYPE SHARED_LIBRARY FILES "/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/Debug/libJavaScript_pi.dylib")
    if(EXISTS "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/PlugIns/libJavaScript_pi.dylib" AND
       NOT IS_SYMLINK "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/PlugIns/libJavaScript_pi.dylib")
      execute_process(COMMAND /usr/bin/install_name_tool
        -delete_rpath "/usr/local/lib"
        "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/PlugIns/libJavaScript_pi.dylib")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip" -x "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/PlugIns/libJavaScript_pi.dylib")
      endif()
    endif()
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/PlugIns/libJavaScript_pi.dylib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/PlugIns" TYPE SHARED_LIBRARY FILES "/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/Release/libJavaScript_pi.dylib")
    if(EXISTS "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/PlugIns/libJavaScript_pi.dylib" AND
       NOT IS_SYMLINK "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/PlugIns/libJavaScript_pi.dylib")
      execute_process(COMMAND /usr/bin/install_name_tool
        -delete_rpath "/usr/local/lib"
        "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/PlugIns/libJavaScript_pi.dylib")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip" -x "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/PlugIns/libJavaScript_pi.dylib")
      endif()
    endif()
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/PlugIns/libJavaScript_pi.dylib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/PlugIns" TYPE SHARED_LIBRARY FILES "/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/MinSizeRel/libJavaScript_pi.dylib")
    if(EXISTS "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/PlugIns/libJavaScript_pi.dylib" AND
       NOT IS_SYMLINK "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/PlugIns/libJavaScript_pi.dylib")
      execute_process(COMMAND /usr/bin/install_name_tool
        -delete_rpath "/usr/local/lib"
        "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/PlugIns/libJavaScript_pi.dylib")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip" -x "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/PlugIns/libJavaScript_pi.dylib")
      endif()
    endif()
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/PlugIns/libJavaScript_pi.dylib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/PlugIns" TYPE SHARED_LIBRARY FILES "/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/RelWithDebInfo/libJavaScript_pi.dylib")
    if(EXISTS "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/PlugIns/libJavaScript_pi.dylib" AND
       NOT IS_SYMLINK "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/PlugIns/libJavaScript_pi.dylib")
      execute_process(COMMAND /usr/bin/install_name_tool
        -delete_rpath "/usr/local/lib"
        "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/PlugIns/libJavaScript_pi.dylib")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip" -x "$ENV{DESTDIR}/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/OpenCPN.app/Contents/PlugIns/libJavaScript_pi.dylib")
      endif()
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Xcode-build-0.4/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
