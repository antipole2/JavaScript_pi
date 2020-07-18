///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/gdicmn.h>
#include <wx/button.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/toolbar.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/stc/stc.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class m_Console
///////////////////////////////////////////////////////////////////////////////
class m_Console : public wxDialog
{
	private:

	protected:
		wxToolBar* m_toolBar1;
		wxButton* m_clearScript;
		wxButton* load_button;
		wxButton* save_button;
		wxButton* save_as_button;
		wxButton* run_button;
		wxButton* testA_button;
		wxButton* TestB_button;
		wxButton* m_clearOutput;

		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnClearScript( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnLoad( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSave( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSaveAs( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRun( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnTestA( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnTestB( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnClearOutput( wxCommandEvent& event ) { event.Skip(); }


	public:
		wxTextCtrl* m_fileString;
		wxStyledTextCtrl* m_Script;
		wxTextCtrl* m_Output;

		m_Console( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("JavaScript"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );
		~m_Console();

};

