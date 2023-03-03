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
#ifndef consolePositioning_h
#define consolePositioning_h

/* herewith constants used when handling minimised or parked consoles
CONSOLE_MIN_HEIGHT	Minimum height of a console
CONSOLE_STUB		Space to be left for things other than console name
PARK_LEVEL			Level at which to park consoles in distance down on screen
PARK_FIRST_X		Indent of first parked console from left-hand edge of screen
PARK_SEP			Horizontal separation of parked consoles

These constants are platform dependent
See the Technical Guide for instructions for configuring for a given platform
*/
#ifdef __WXMSW__
#define CONSOLE_MIN_HEIGHT 20L
#define CONSOLE_STUB 50L
#define PARK_LEVEL 25L
#define PARK_FIRST_X 70L
#define PARK_SEP 8L

#elif defined(__LINUX__)
#define CONSOLE_MIN_HEIGHT 20L
#define CONSOLE_STUB 79L
#define PARK_LEVEL 48L
#define PARK_FIRST_X 66L
#define PARK_SEP 21L

#else
//includes MacOS
#define CONSOLE_MIN_HEIGHT 23L
#define CONSOLE_STUB 79L
#define PARK_LEVEL 23L
#define PARK_FIRST_X 40L
#define PARK_SEP 2L
#endif

#endif		// consolePositioning_h

