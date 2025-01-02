/**************************************************************************
* Project:  OpenCPN
* Purpose:  JavaScript Plugin
* Author:   Tony Voss 25/04/2022
*
* Copyright Ⓒ 2024 by Tony Voss
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
#define     PLUGIN_VERSION_COMMENT "beta build 3"

#define     PLUGIN_FIRST_TIME_WELCOME "Welcome to the JavaScript plugin\n\nRun your first script by clicking on the run button above.\n\nSee the Help panel right for documentation"
#define     PLUGIN_UPDATE_WELCOME "Plugin has been updated.\n\n\
Release notice at https://github.com/antipole2/JavaScript_pi/discussions\n\nSee the Help panel right for documentation.\n\
Changes are highlighted in the User Guide and listed in its Appendix A"

// screen resolution support only available in wxWidgets 3.2 or later
#if wxCHECK_VERSION(3,2,0)
#define SCREEN_RESOLUTION_AVAILABLE true
#else
#define SCREEN_RESOLUTION_AVAILABLE false
#endif

#define	DEGREE		  "\u00B0"	// real degree character
#ifdef DUK_F_WINDOWS
#define PSEUDODEGREE  "\u0007"	// substitute character for degree - bell
#define SUBDEGREE(what, in, out) what.Replace(in, out, true)
#else
#define SUBDEGREE(what, in, out)
#endif

#endif		// version_h