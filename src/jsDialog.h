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
#ifndef jsDialog_h
#define jsDialog_h

// dialogues storage
struct dialogElement{
    wxString    type;
    wxString    stringValue;
    wxString    label;
    wxString    textValue;
    int         numberValue;
    int         width;
    int         height;
    int         multiLine;
    wxString    suffix;
    int         itemID; // The ID of the window element if anyitemID
    };
    
class JsDialog: public wxDialog{
public:
	JsDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos=wxDefaultPosition,
		const wxSize &size=wxDefaultSize, long style=wxDEFAULT_DIALOG_STYLE, const wxString &name=wxDialogNameStr)
		:wxDialog(parent, id, title, pos, size, style, name)
		{};
		
	std::vector<dialogElement> dialogElementsArray;	// we store the dialogue design as in the JavaScript
    bool customPosition = false;					// true of this dialogue was a custom position
		
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

class jsButton : public wxButton {
    // adds pointer to console to button so we can find it from button event
public:
    Console *pConsole;   // pointer to the console associated with this button
    jsButton (Console *buttonConsole, wxWindow *parent, wxWindowID id = 0, const wxString &label = wxEmptyString, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = 0, const wxValidator &validator = wxDefaultValidator,const wxString &name = wxButtonNameStr) :
            wxButton(parent, id, label, pos, size, style, validator, name ),
            pConsole(buttonConsole)
    {};
    int entryId;	// the id of the callback entry
	};



#endif // jsDialog_h