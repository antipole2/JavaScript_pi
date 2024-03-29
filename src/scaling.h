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
#ifndef scaling_h
#define scaling_h

// defaults in DIP
#define NEW_CONSOLE_POSITION	wxPoint(300,100)
#define NEW_CONSOLE_SIZE		wxSize(738,600)
#define DEFAULT_DIALOG_POSITION	wxPoint(150,100)
#define DEFAULT_ALERT_POSITION	wxPoint(90,20)

/* Handling screens of different resolutions is platform dependent in some respects
*/

#if SCREEN_RESOLUTION_AVAILABLE
	#ifdef __WXMSW__
		#define SCALE(window) (window->GetDPIScaleFactor())
	#elif defined(__LINUX__)
		#define SCALE(window) (window->GetDPIScaleFactor())
	#else
		//includes MacOS
		#define SCALE(window) (1e0)
	#endif
#else
	#define SCALE(window) (1e0)
#endif

#endif		// scaling_h

