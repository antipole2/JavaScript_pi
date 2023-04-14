/**************************************************************************
* Project:  OpenCPN
* Purpose:  JavaScript Plugin
* Author:   Tony Voss 25/02/2021
*
* Copyright Ⓒ 2023 by Tony Voss
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, under which
* you must including a copy of these terms in all copies
* https://www.gnu.org/licenses/gpl-3.0.en.html
***************************************************************************
*/

#ifndef toolsDialog_h
#define toolsDialog_h

#include "wx/wx.h"
#include "wx/window.h"
#include "toolsDialogGui.h"
#include "trace.h"

class ToolsClass: public ToolsClassBase {
public:

    void onPageChanged( wxNotebookEvent& event );
    void onAddConsole( wxCommandEvent& event );
    void onChangeName( wxCommandEvent& event );
    void onRecieveNMEAmessage( wxCommandEvent& event );
    void onRecieveMessage( wxCommandEvent& event );
    void onChangeDirectory( wxCommandEvent& event );
    void onDump( wxCommandEvent& event );
    void onClean( wxCommandEvent& event );
    void onClose( wxCloseEvent& event );
    void setConsoleChoices();
    void onParkingRevert(wxCommandEvent& event);
    void onParkingCustomise(wxCommandEvent& event);
    void onParkingReveal(wxCommandEvent& event);
    void cleanupParking();
    void setupPage(int pageNumber);
	
    ToolsClass( wxWindow *parent,  wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString,
    	const wxPoint& pos = wxPoint(1000,400), const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE)
        :ToolsClassBase(parent, id, title, pos, size, style)
        {        
        // adding extra _ to list of valid chars via wxFormBuilder does not work.  Bug in wxWidgets?
        // so we will do it this way
        // this only available in wxWidgets 3.1.3 and later
#if (wxVERSION_NUMBER >= 3103)
        wxTextValidator* validator;
        validator = (wxTextValidator*)m_newConsoleName->GetValidator();
        validator->AddCharIncludes("_");   
        validator = (wxTextValidator*)m_changedName->GetValidator();
        validator->AddCharIncludes("_");
#endif
        m_notebook->SetSelection(0);	//start on required page
        };
        
 
    };

#endif /* ToolsDialog_h */
