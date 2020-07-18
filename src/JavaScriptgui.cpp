///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "JavaScriptgui.h"

///////////////////////////////////////////////////////////////////////////

m_Console::m_Console( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer9111;
	bSizer9111 = new wxBoxSizer( wxVERTICAL );

	bSizer9111->SetMinSize( wxSize( 600,200 ) );
	m_toolBar1 = new wxToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL );
	m_clearScript = new wxButton( m_toolBar1, wxID_ANY, wxT("Clear"), wxDefaultPosition, wxDefaultSize, 0 );
	m_toolBar1->AddControl( m_clearScript );
	load_button = new wxButton( m_toolBar1, wxID_ANY, wxT("Load"), wxDefaultPosition, wxDefaultSize, 0 );
	m_toolBar1->AddControl( load_button );
	save_button = new wxButton( m_toolBar1, wxID_ANY, wxT("Save"), wxDefaultPosition, wxDefaultSize, 0 );
	m_toolBar1->AddControl( save_button );
	save_as_button = new wxButton( m_toolBar1, wxID_ANY, wxT("Save as…"), wxDefaultPosition, wxDefaultSize, 0 );
	m_toolBar1->AddControl( save_as_button );
	run_button = new wxButton( m_toolBar1, wxID_ANY, wxT("Run"), wxDefaultPosition, wxDefaultSize, 0 );
	m_toolBar1->AddControl( run_button );
	testA_button = new wxButton( m_toolBar1, wxID_ANY, wxT(" Test A"), wxDefaultPosition, wxDefaultSize, 0 );
	m_toolBar1->AddControl( testA_button );
	TestB_button = new wxButton( m_toolBar1, wxID_ANY, wxT("Test B"), wxDefaultPosition, wxDefaultSize, 0 );
	m_toolBar1->AddControl( TestB_button );
	m_toolBar1->Realize();

	bSizer9111->Add( m_toolBar1, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxHORIZONTAL );

	bSizer14->SetMinSize( wxSize( 600,20 ) );

	bSizer9111->Add( bSizer14, 1, 0, 5 );

	m_fileString = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 600,18 ), wxTE_READONLY );
	m_fileString->SetFont( wxFont( 10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString ) );
	m_fileString->SetHelpText( wxT("This is the current accessed file") );

	bSizer9111->Add( m_fileString, 0, wxALL, 5 );

	m_Script = new wxStyledTextCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, wxEmptyString );
	m_Script->SetUseTabs( true );
	m_Script->SetTabWidth( 5 );
	m_Script->SetIndent( 5 );
	m_Script->SetTabIndents( true );
	m_Script->SetBackSpaceUnIndents( true );
	m_Script->SetViewEOL( false );
	m_Script->SetViewWhiteSpace( false );
	m_Script->SetMarginWidth( 2, 0 );
	m_Script->SetIndentationGuides( true );
	m_Script->SetMarginType( 1, wxSTC_MARGIN_SYMBOL );
	m_Script->SetMarginMask( 1, wxSTC_MASK_FOLDERS );
	m_Script->SetMarginWidth( 1, 16);
	m_Script->SetMarginSensitive( 1, true );
	m_Script->SetProperty( wxT("fold"), wxT("1") );
	m_Script->SetFoldFlags( wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED );
	m_Script->SetMarginType( 0, wxSTC_MARGIN_NUMBER );
	m_Script->SetMarginWidth( 0, m_Script->TextWidth( wxSTC_STYLE_LINENUMBER, wxT("_99999") ) );
	{
		wxFont font = wxFont( 13, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Courier") );
		m_Script->StyleSetFont( wxSTC_STYLE_DEFAULT, font );
	}
	m_Script->MarkerDefine( wxSTC_MARKNUM_FOLDER, wxSTC_MARK_BOXPLUS );
	m_Script->MarkerSetBackground( wxSTC_MARKNUM_FOLDER, wxColour( wxT("BLACK") ) );
	m_Script->MarkerSetForeground( wxSTC_MARKNUM_FOLDER, wxColour( wxT("WHITE") ) );
	m_Script->MarkerDefine( wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_BOXMINUS );
	m_Script->MarkerSetBackground( wxSTC_MARKNUM_FOLDEROPEN, wxColour( wxT("BLACK") ) );
	m_Script->MarkerSetForeground( wxSTC_MARKNUM_FOLDEROPEN, wxColour( wxT("WHITE") ) );
	m_Script->MarkerDefine( wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_EMPTY );
	m_Script->MarkerDefine( wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_BOXPLUS );
	m_Script->MarkerSetBackground( wxSTC_MARKNUM_FOLDEREND, wxColour( wxT("BLACK") ) );
	m_Script->MarkerSetForeground( wxSTC_MARKNUM_FOLDEREND, wxColour( wxT("WHITE") ) );
	m_Script->MarkerDefine( wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_BOXMINUS );
	m_Script->MarkerSetBackground( wxSTC_MARKNUM_FOLDEROPENMID, wxColour( wxT("BLACK") ) );
	m_Script->MarkerSetForeground( wxSTC_MARKNUM_FOLDEROPENMID, wxColour( wxT("WHITE") ) );
	m_Script->MarkerDefine( wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY );
	m_Script->MarkerDefine( wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_EMPTY );
	m_Script->SetSelBackground( true, wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT ) );
	m_Script->SetSelForeground( true, wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHTTEXT ) );
	m_Script->SetFont( wxFont( 13, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Courier") ) );
	m_Script->SetMinSize( wxSize( 600,500 ) );

	bSizer9111->Add( m_Script, 1, wxEXPAND | wxALL, 5 );

	m_clearOutput = new wxButton( this, wxID_ANY, wxT("Clear output"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	m_clearOutput->SetMaxSize( wxSize( -1,10 ) );

	bSizer9111->Add( m_clearOutput, 0, wxALL, 5 );

	m_Output = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_WORDWRAP );
	m_Output->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString ) );
	m_Output->SetMinSize( wxSize( 600,200 ) );

	bSizer9111->Add( m_Output, 0, wxALL, 5 );


	this->SetSizer( bSizer9111 );
	this->Layout();
	bSizer9111->Fit( this );

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( m_Console::OnClose ) );
	m_clearScript->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_Console::OnClearScript ), NULL, this );
	load_button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_Console::OnLoad ), NULL, this );
	save_button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_Console::OnSave ), NULL, this );
	save_as_button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_Console::OnSaveAs ), NULL, this );
	run_button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_Console::OnRun ), NULL, this );
	testA_button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_Console::OnTestA ), NULL, this );
	TestB_button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_Console::OnTestB ), NULL, this );
	m_clearOutput->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_Console::OnClearOutput ), NULL, this );
}

m_Console::~m_Console()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( m_Console::OnClose ) );
	m_clearScript->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_Console::OnClearScript ), NULL, this );
	load_button->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_Console::OnLoad ), NULL, this );
	save_button->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_Console::OnSave ), NULL, this );
	save_as_button->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_Console::OnSaveAs ), NULL, this );
	run_button->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_Console::OnRun ), NULL, this );
	testA_button->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_Console::OnTestA ), NULL, this );
	TestB_button->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_Console::OnTestB ), NULL, this );
	m_clearOutput->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_Console::OnClearOutput ), NULL, this );

}
