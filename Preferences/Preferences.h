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
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/panel.h>
#include <wx/notebook.h>
#include <wx/valtext.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class SettingsClass
///////////////////////////////////////////////////////////////////////////////
class SettingsClass : public wxDialog
{
	private:

	protected:
		wxNotebook* m_notebook;
		wxPanel* Consoles;
		wxStaticText* m_prompt;
		wxTextCtrl* m_newConsoleName;
		wxButton* m_addButton;
		wxPanel* Utilities;
		wxStaticText* m_sendMessageText;
		wxTextCtrl* m_sendMessage;
		wxButton* m_sendMessageButton;
		wxButton* m_closeButton;

		// Virtual event handlers, overide them in your derived class
		virtual void onAddConsole( wxCommandEvent& event ) { event.Skip(); }
		virtual void onSendMessage( wxCommandEvent& event ) { event.Skip(); }


	public:
		wxTextCtrl* m_resultMessage;

		SettingsClass( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,165 ), long style = wxCAPTION|wxCLOSE_BOX|wxDEFAULT_DIALOG_STYLE|wxSTAY_ON_TOP, const wxString& name = wxT("JavaScript settings") );
		~SettingsClass();

};

