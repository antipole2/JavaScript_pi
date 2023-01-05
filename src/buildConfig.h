/**************************************************************************
* Project:  OpenCPN
* Purpose:  JavaScript Plugin
* Author:   Tony Voss 16/05/2020
*
* Copyright Ⓒ 2023 by Tony Voss
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, under which
* you must including a copy of these terms in all copies
* https://www.gnu.org/licenses/gpl-3.0.en.html
***************************************************************************
*/

#ifndef buildConfig_h
#define buildConfig_h

// sets  configuration to compile - whether to include certain options
// if the component is defined, it will be included
// if undefine it will be omitted

// #define		SOCKETS					// sockets support (only experimented with so far)

// #define			IPC						// Inter Process Communication
#ifdef			IPC
#define			wxUSE_DDE_FOR_IPC  0	// do not use DDE even on Windows
#endif

#endif // buildConfig_h

