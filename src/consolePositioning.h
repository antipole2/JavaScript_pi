/**************************************************************************
* Project:  OpenCPN
* Purpose:  JavaScript Plugin
* Author:   Tony Voss 25/04/2022
*
* Copyright Ⓒ 2025 by Tony Voss
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, under which
* you must including a copy of these terms in all copies
* https://www.gnu.org/licenses/gpl-3.0.en.html
***************************************************************************
*/
#ifndef consolePositioning_h
#define consolePositioning_h

/* herewith constants used when handling minimised or parked consoles
CONSOLE_STUB		Space to be left for things other than console name
PARK_LEVEL			Level at which to park consoles in distance down on screen
PARK_FIRST_X		Indent of first parked console from left-hand edge of screen
PARK_SEP			Horizontal separation of parked consoles

These constants are platform dependent
See the Technical Guide for instructions for configuring for a given platform
THese positions and sizes are all in DIP
*/
#ifdef __WXMSW__
#define CONSOLE_STUB 63L
#define PARK_LEVEL 35L
#define PARK_FIRST_X 85L
#define PARK_SEP 2L

#elif defined(__LINUX__)
#define CONSOLE_STUB 152L
#define PARK_LEVEL 69L
#define PARK_FIRST_X 76L
#define PARK_SEP 10L

#else
//includes MacOS
#define CONSOLE_STUB 79L
#define PARK_LEVEL 23L
#define PARK_FIRST_X 40L
#define PARK_SEP 2L
#endif

#endif		// consolePositioning_h

