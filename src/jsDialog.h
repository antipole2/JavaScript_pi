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
#ifndef jsDialog_h
#define jsDialog_h

class JsDialog: public wxDialog{
public:
	JsDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos=wxDefaultPosition,
		const wxSize &size=wxDefaultSize, long style=wxDEFAULT_DIALOG_STYLE, const wxString &name=wxDialogNameStr)
		:wxDialog(parent, id, title, pos, size, style, name)
		{};
	#if !SCREEN_RESOLUTION_AVAILABLE
		// provide dummy methods for those not available
		// needs to work with wxPoint & wxSize
		template <typename T>
		T ToDIP(T point){
		return point;
		}
	
		template <typename T>
		T FromDIP(T point){
		return point;
			}
	#endif

	};


#endif // jsDialog_h