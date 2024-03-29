/**************************************************************************
* Project:  OpenCPN
* Purpose:  JavaScript Plugin
* Author:   Tony Voss 25/04/2022
*
* Copyright Ⓒ 2023 by Tony Voss
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, under which
* you must including a copy of these terms in all copies
* https://www.gnu.org/licenses/gpl-3.0.en.html
***************************************************************************
*/
#ifndef version_h
#define version_h

#include <wx/version.h>

#define     PLUGIN_VERSION_DATE __DATE__
#define     PLUGIN_VERSION_COMMENT "Build -d"

// #define     USERGUIDES "\nhttps://github.com/antipole2/JavaScript_pi/blob/master/documentation/JavaScript_plugin_user_guide.pdf\nor\nhttps://github.com/antipole2/JavaScript_pi/blob/master/documentation/JavaScript_plugin_user_guide.epub\n\n"
#define     PLUGIN_FIRST_TIME_WELCOME "Welcome to the JavaScript plugin\n\nRun your first script by clicking on the run button above.\n\nSee the Help panel right for documentation"
#define     PLUGIN_UPDATE_WELCOME "Plugin has been updated.\n\nSee the Help panel right for documentation.\n\
Changes are highlighted in the User Guide and listed in its Appendix A"

// screen resolution support only available in wxWidgets 3.2 or later
#if wxCHECK_VERSION(3,2,0)
#define SCREEN_RESOLUTION_AVAILABLE true
#else
#define SCREEN_RESOLUTION_AVAILABLE false
#endif

#endif		// version_h

