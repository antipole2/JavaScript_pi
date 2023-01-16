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
CONSOLE_CHAR_WIDTH	Width to allow for average 
PARK_FRAME_HEIGHT	Height of canvas window top frame			
PARK_CILL			Space between bottom of minimised console and bottom of canvas top bar
PARK_FIRST_X		Indent of first parked console from left-hand edge of canvas window
PARK_SEP			Horizontal separation of parked consoles

These constants are platform dependent
See the Technical Guide for instructions for configuring for a given platform
*/
#ifdef __WXMSW__
#define CONSOLE_MIN_HEIGHT 20
#define CONSOLE_STUB 50

#define PARK_LEVEL 25
#define PARK_FIRST_X 70
#define PARK_SEP 8

#else
//includes MacOS
#define CONSOLE_MIN_HEIGHT 29L
#define CONSOLE_STUB 77L
#define PARK_LEVEL 25L
#define PARK_FIRST_X 3L
#define PARK_SEP 6L

#endif

#endif		// consolePositioning_h

