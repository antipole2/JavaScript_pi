///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 3.10.1-0-g8feb16b)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/button.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/stc/stc.h>
#include <wx/statline.h>
#include <wx/panel.h>
#include <wx/splitter.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class m_Console
///////////////////////////////////////////////////////////////////////////////
class m_Console : public wxFrame
{
	private:

	protected:
		wxSplitterWindow* m_splitter;
		wxPanel* m_scriptSizer;
		wxButton* m_clearScript;
		wxButton* copy_button;
		wxButton* load_button;
		wxButton* save_button;
		wxButton* save_as_button;
		wxButton* park_button;
		wxButton* tools_button;
		wxButton* Help;
		wxStaticLine* m_staticline1;
		wxPanel* m_outputSizer;
		wxStaticLine* m_staticline2;
		wxButton* m_clearOutput;

		// Virtual event handlers, override them in your derived class
		virtual void OnActivate( wxActivateEvent& event ) { event.Skip(); }
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnMove( wxMoveEvent& event ) { event.Skip(); }
		virtual void onMouse( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnClearScript( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCopyAll( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnLoad( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSave( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSaveAs( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRun( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAutoRun( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnPark( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnTools( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnHelp( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnClearOutput( wxCommandEvent& event ) { event.Skip(); }


	public:
		wxButton* run_button;
		wxCheckBox* auto_run;
		wxTextCtrl* m_fileStringBox;
		wxStyledTextCtrl* m_Script;
		wxStyledTextCtrl* m_Output;

		m_Console( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 600,300 ), long style = wxCAPTION|wxCLOSE_BOX|wxFRAME_FLOAT_ON_PARENT|wxRESIZE_BORDER|wxTAB_TRAVERSAL );

		~m_Console();

		void m_splitterOnIdle( wxIdleEvent& )
		{
			m_splitter->SetSashPosition( 600 );
			m_splitter->Disconnect( wxEVT_IDLE, wxIdleEventHandler( m_Console::m_splitterOnIdle ), NULL, this );
		}

};

