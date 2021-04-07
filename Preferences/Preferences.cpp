///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "Preferences.h"

///////////////////////////////////////////////////////////////////////////

SettingsClass::SettingsClass( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) : wxDialog( parent, id, title, pos, size, style, name )
{
	this->SetSizeHints( wxDefaultSize, wxSize( 500,180 ) );
	this->Enable( false );

	wxBoxSizer* TopSizer;
	TopSizer = new wxBoxSizer( wxVERTICAL );

	m_notebook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	m_notebook->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );
	m_notebook->SetMaxSize( wxSize( -1,250 ) );

	Consoles = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	Consoles->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_INACTIVECAPTIONTEXT ) );
	Consoles->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_APPWORKSPACE ) );

	wxStaticBoxSizer* ConsolesSizer1;
	ConsolesSizer1 = new wxStaticBoxSizer( new wxStaticBox( Consoles, wxID_ANY, wxEmptyString ), wxHORIZONTAL );

	m_prompt = new wxStaticText( ConsolesSizer1->GetStaticBox(), wxID_ANY, wxT("Add new console named"), wxDefaultPosition, wxDefaultSize, 0|wxBORDER_NONE );
	m_prompt->Wrap( -1 );
	m_prompt->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_CAPTIONTEXT ) );

	ConsolesSizer1->Add( m_prompt, 0, wxALL, 5 );

	m_newConsoleName = new wxTextCtrl( ConsolesSizer1->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_newConsoleName->SetMinSize( wxSize( 150,-1 ) );

	ConsolesSizer1->Add( m_newConsoleName, 0, wxALL, 2 );


	ConsolesSizer1->Add( 0, 0, 1, wxEXPAND, 5 );

	m_addButton = new wxButton( ConsolesSizer1->GetStaticBox(), wxID_ANY, wxT("Add"), wxDefaultPosition, wxDefaultSize, 0 );
	ConsolesSizer1->Add( m_addButton, 0, wxALL, 5 );


	Consoles->SetSizer( ConsolesSizer1 );
	Consoles->Layout();
	ConsolesSizer1->Fit( Consoles );
	m_notebook->AddPage( Consoles, wxT("Consoles"), false );
	Utilities = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	Utilities->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_APPWORKSPACE ) );

	wxStaticBoxSizer* UtilitiesSizer;
	UtilitiesSizer = new wxStaticBoxSizer( new wxStaticBox( Utilities, wxID_ANY, wxEmptyString ), wxHORIZONTAL );

	m_sendMessageText = new wxStaticText( UtilitiesSizer->GetStaticBox(), wxID_ANY, wxT("OCPN message"), wxDefaultPosition, wxDefaultSize, 0 );
	m_sendMessageText->Wrap( -1 );
	m_sendMessageText->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_CAPTIONTEXT ) );

	UtilitiesSizer->Add( m_sendMessageText, 0, wxALL, 5 );

	m_sendMessage = new wxTextCtrl( UtilitiesSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_sendMessage->SetMinSize( wxSize( 250,-1 ) );

	UtilitiesSizer->Add( m_sendMessage, 0, wxALL, 5 );


	UtilitiesSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	m_sendMessageButton = new wxButton( UtilitiesSizer->GetStaticBox(), wxID_ANY, wxT("Send"), wxDefaultPosition, wxDefaultSize, 0 );
	UtilitiesSizer->Add( m_sendMessageButton, 0, wxALL, 5 );


	Utilities->SetSizer( UtilitiesSizer );
	Utilities->Layout();
	UtilitiesSizer->Fit( Utilities );
	m_notebook->AddPage( Utilities, wxT("Utility"), false );

	TopSizer->Add( m_notebook, 1, wxEXPAND | wxALL, 5 );

	wxBoxSizer* LastSizer;
	LastSizer = new wxBoxSizer( wxHORIZONTAL );

	m_resultMessage = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxBORDER_NONE );
	m_resultMessage->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_CAPTIONTEXT ) );
	m_resultMessage->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_APPWORKSPACE ) );
	m_resultMessage->SetMinSize( wxSize( 400,-1 ) );

	LastSizer->Add( m_resultMessage, 0, wxALL, 5 );

	m_closeButton = new wxButton( this, wxID_ANY, wxT("Close"), wxDefaultPosition, wxDefaultSize, 0 );
	LastSizer->Add( m_closeButton, 0, wxALL, 5 );


	TopSizer->Add( LastSizer, 1, wxEXPAND, 5 );


	this->SetSizer( TopSizer );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	m_addButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SettingsClass::onAddConsole ), NULL, this );
	m_sendMessageButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SettingsClass::onSendMessage ), NULL, this );
}

SettingsClass::~SettingsClass()
{
	// Disconnect Events
	m_addButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SettingsClass::onAddConsole ), NULL, this );
	m_sendMessageButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SettingsClass::onSendMessage ), NULL, this );

}
