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
#include "scaling.h"

class ToolsClass: public ToolsClassBase {
public:

    void onPageChanged( wxNotebookEvent& event );
    void onAddConsole( wxCommandEvent& event );
    void onChangeName( wxCommandEvent& event );
    void onFloatOnParent( wxCommandEvent& event );
    void onToggleStatus( wxCommandEvent& event );
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
	
    ToolsClass( wxWindow *parent,  wxWindowID id = wxID_ANY)
        :ToolsClassBase(parent, id)
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
        
#if 0 	//this for simulating Hi Res screen on non-hires screen
#define simScale 2
    wxSize FromDIP(wxSize size){
    	return wxSize(size.x*simScale, size.y*simScale);
   		 }   		 
   	 wxPoint FromDIP(wxPoint size){
    	return wxPoint(size.x*simScale, size.y*simScale);
   		 }
   	 wxSize ToDIP(wxSize size){
    	return wxSize(size.x/simScale, size.y/simScale);
   		 }   		 
   	 wxPoint ToDIP(wxPoint size){
    	return wxPoint(size.x/simScale, size.y/simScale);
   		}	    		 
#endif
        

	void fixForScreenRes(){	// fix up sizes according to screen resolution
		double scale = SCALE(this);
		if (scale == 1) return;	// nothing to do
		wxSize size;
		
		m_notebook->SetSize(FromDIP(m_notebook->GetSize()));

		// Consoles tab
		Consoles->SetSize(FromDIP(Consoles->GetSize()));
		m_newConsoleName->SetMinSize(FromDIP(m_newConsoleName->GetSize()));
		m_oldNames->SetMinSize(FromDIP(m_oldNames->GetSize()));
		m_changedName->SetMinSize(FromDIP(m_changedName->GetSize()));		
		m_ConsolesMessage->SetMinSize(FromDIP(m_ConsolesMessage->GetSize()));

		// Directory tab
		Directory->SetSize(FromDIP(Directory->GetSize()));
		mCurrentDirectory->SetMinSize(FromDIP(mCurrentDirectory->GetSize()));
		
		// NMEA tab
		NMEA->SetSize(FromDIP(NMEA->GetSize()));
		m_NMEAmessage->SetMinSize(FromDIP(m_NMEAmessage->GetSize()));

		// Message tab
		Message->SetSize(FromDIP(Message->GetSize()));
		m_MessageID->SetMinSize(FromDIP(m_MessageID->GetSize()));
		m_MessageBody->SetMinSize(FromDIP(m_MessageBody->GetSize()));

		//Parking tab
		Parking->SetSize(FromDIP(Parking->GetSize()));
		m_parkingMessage->SetMinSize(FromDIP(m_parkingMessage->GetSize()));
		

		//Help tab
		// NB The wrap with overides that set in wxForBuilder as there is no way of getting the value set
		Help->SetSize(FromDIP(Help->GetSize()));
		HelpTopText1->Wrap( 550*scale );
		HelpTopText11->Wrap( 550*scale );
		HelpTopText111->Wrap( 550*scale );
		HelpTopText1111->Wrap( 550*scale );
		HelpTopText11111->Wrap( 550*scale );
		
		// Diagnostics tab
		Diagnostics->SetSize(FromDIP(Diagnostics->GetSize()));
		m_charsToClean->SetMinSize(FromDIP(m_charsToClean->GetSize()));
		}
    	    
	};

#endif /* ToolsDialog_h */
