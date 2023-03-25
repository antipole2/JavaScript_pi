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

#include <wx/wx.h>
#include "JavaScript_pi.h"
#include "JavaScriptgui_impl.h"
#include "wx/dirdlg.h"

Console* pTestConsole1 = nullptr;
Console* pTestConsole2 = nullptr;

void ToolsClass::setConsoleChoices(){
    Console *pConsole;
    m_oldNames->Clear();
    for (pConsole = pJavaScript_pi->mpFirstConsole; pConsole != nullptr; pConsole = pConsole->mpNextConsole){
        m_oldNames->Append(pConsole->mConsoleName);
        }
    }

void ToolsClass::onClose( wxCloseEvent& event ){
    extern JavaScript_pi* pJavaScript_pi;
/*
    *pPointerToThisInJavaScript_pi = nullptr;
    TRACE(4, "In Tools close");
    this->Destroy();
 */
    this->Hide();
    cleanupParking();
    Destroy();
    pJavaScript_pi->pTools = nullptr;
    m_parkingMessage->Clear();
    }

void ToolsClass::onPageChanged( wxNotebookEvent& event ) {
    // The different pages need to be different sizes - this does it
    int pageNumber;

    pageNumber = event.GetSelection();
    setupPage(pageNumber);
    }
    
void ToolsClass::cleanupParking(){
		m_customiseButton->SetLabel("Start");
		if (pTestConsole1 != nullptr){ pTestConsole1->bin(); pTestConsole1 = nullptr; }
		if (pTestConsole2 != nullptr){ pTestConsole2->bin(); pTestConsole2 = nullptr; }
		}

void ToolsClass::setupPage(int pageNumber){	// display this page of tools
    	extern JavaScript_pi* pJavaScript_pi;
        wxWindow *page;
        setConsoleChoices();
        cleanupParking();
        m_parkingMessage->Clear();
        m_customiseButton->SetLabel("Start");
        wxString currentDirectory = pJavaScript_pi->mCurrentDirectory;
    	mCurrentDirectoryString->SetLabel((currentDirectory == wxEmptyString)?"(Not yet set)":currentDirectory);
		m_notebook->ChangeSelection(pageNumber);
        page = m_notebook->GetPage(pageNumber);
        page->Fit();
        Show();
        Raise();
        wxSize pageSize = ToDIP(page->GetSize());
        TRACE(6, wxString::Format("Dialogue GetSize gave DIP %d x %d", pageSize.x, pageSize.y));
        pageSize.x = 600;	// force width
		this->SetSize(FromDIP(pageSize));	// allow for screen resolution
        }        

void ToolsClass::onAddConsole( wxCommandEvent& event ){
    wxString newConsoleName, outcome;
    Console *pConsole;
    wxString checkConsoleName(wxString name, Console* pConsole);
    int x, y;
    
    this->m_ConsolesMessage->Clear();
    newConsoleName = this->m_newConsoleName->GetValue();
    if (newConsoleName == wxEmptyString){
        m_ConsolesMessage->AppendText("Must specify a name for the new console");
        return;
        }
    outcome = checkConsoleName(newConsoleName, nullptr);
    if (outcome != ""){
        m_ConsolesMessage->AppendText(outcome);
        return;
        }
	pConsole = new Console(pJavaScript_pi->m_parent_window, newConsoleName);
/*
//x 	,
		pJavaScript_pi->m_parent_window->FromDIP(NEW_CONSOLE_POSITION),
		pJavaScript_pi->m_parent_window->FromDIP(NEW_CONSOLE_SIZE),
		pJavaScript_pi->m_parent_window->FromDIP(DEFAULT_DIALOG_POSITION),
		pJavaScript_pi->m_parent_window->FromDIP(DEFAULT_ALERT_POSITION),
		wxEmptyString, false, wxEmptyString);
*/
    pConsole->GetPosition(&x, &y);
    x += - 25 + rand()%50; y += - 25 + rand()%50;
    pConsole->SetPosition(wxPoint(x, y));
    pConsole->setConsoleMinSize();
    setConsoleChoices();    // update
    pConsole->Show();
    m_ConsolesMessage->AppendText(_("Console " + newConsoleName + " created"));
    }

void ToolsClass::onChangeName( wxCommandEvent& event ){
    wxString oldConsoleName, newConsoleName, outcome;
    wxSize minSize, oldSize, newSize;
    wxString checkConsoleName(wxString name, Console* pConsole);
    Console* findConsoleByName(wxString name);
    Console *pConsole;
 
    this->m_ConsolesMessage->Clear();
    newConsoleName = this->m_changedName->GetValue();
    oldConsoleName = m_oldNames->GetStringSelection();
    if (oldConsoleName.IsSameAs(newConsoleName, false)){ // Check with case insenstive
        m_ConsolesMessage->AppendText("New name same as old name");
        return;
        }
    if (newConsoleName == wxEmptyString){
        m_ConsolesMessage->AppendText("Must specify a new name");
        return;
        }
    outcome = checkConsoleName(newConsoleName, nullptr);
    if (outcome != ""){
        m_ConsolesMessage->AppendText(outcome);
        return;
        }
    pConsole = findConsoleByName(oldConsoleName);
    if (pConsole == nullptr){
    	m_ConsolesMessage->AppendText("Change name error - cannot find old console");
    	return;} 
    pConsole->SetLabel(newConsoleName);
    pConsole->mConsoleName = newConsoleName;
    TRACE(17, wxString::Format("onChangeName for parked console %s size was x:%i y:%i", oldConsoleName, GetSize().x, GetSize().y ));
    pConsole->setConsoleMinSize();
    if (pConsole->isParked()){	// shrink it
    	wxSize size = pConsole->GetMinSize();
    	TRACE(17, wxString::Format("onChangeName for parked console new name %s setting new size to x:%i y:%i", newConsoleName, size.x, size.y ));
    	pConsole->SetSize(size);
    	}
    m_ConsolesMessage->AppendText(_("Console " + oldConsoleName + " changed to " + newConsoleName));
    setConsoleChoices();    // update
    }

wxString NMEAsentence;  // to hold NMEA sentence as enduring string
void ToolsClass::onRecieveNMEAmessage(wxCommandEvent& event ){
    void SetNMEASentence(wxString &sentence);
    
    NMEAsentence = this->m_NMEAmessage->GetValue();
    pJavaScript_pi->SetNMEASentence(NMEAsentence);
     return;
    }

wxString MessageID;  // to hold message ID as enduring string
wxString MessageBody;  // to hold message body as enduring string
void ToolsClass::onRecieveMessage(wxCommandEvent& event ){
    void SetNMEASentence(wxString &sentence);
    
    MessageID = m_MessageID->GetValue();
    MessageBody = m_MessageBody->GetValue();
    pJavaScript_pi->SetPluginMessage(MessageID, MessageBody);
    }

void ToolsClass::onChangeDirectory( wxCommandEvent& event ){
    wxDirDialog dirDialog(this, "Select a current directory", pJavaScript_pi->mCurrentDirectory, wxDD_NEW_DIR_BUTTON);
    if (dirDialog.ShowModal() == wxID_OK){
        pJavaScript_pi->mCurrentDirectory = dirDialog.GetPath();
        mCurrentDirectoryString->SetLabel(pJavaScript_pi->mCurrentDirectory);
        }
    }

void ToolsClass::onDump( wxCommandEvent& event ){
    cout << "Dumping\n";
    wxString ptrToString(Console* address);
    Console *pConsole;
    wxDialog* dumpWindow;
    wxTextCtrl *dumpTextCtrl;
    extern JavaScript_pi *pJavaScript_pi;
    wxString dump {wxEmptyString};
    // position dump window just to left of tools - calculate in DIP
    wxPoint toolsPosition = ToDIP(GetPosition());
    wxSize toolsSize = ToDIP(GetSize());
    wxPoint dumpPosition = toolsPosition;
    dumpPosition.x -= toolsSize.x;	// shift left to be on left of tools
    dumpPosition.y = toolsPosition.y;	// and at same height
    dumpPosition = FromDIP(dumpPosition);
    dumpWindow = new wxDialog(this /*pJavaScript_pi->m_parent_window */, wxID_ANY, "JavaScript plugin dump", dumpPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxSTAY_ON_TOP | wxRESIZE_BORDER);
    dumpTextCtrl = new wxTextCtrl(dumpWindow, wxID_NEW,
                          wxEmptyString, wxDefaultPosition, wxSize(240, 100),
                          wxTE_MULTILINE);

    dump += (wxString::Format("JavaScript plugin version %d.%d\n", PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR));
    dump += (wxString::Format("JavaScript patch %d\n", PLUGIN_VERSION_PATCH));
    dump += (wxString::Format("JavaScript tools window DPI scaling factor %f\n", SCALE(this)));
    dump += (wxString::Format("wxWidgets version %d.%d.%d.%d or %d\n", wxMAJOR_VERSION, wxMINOR_VERSION, wxRELEASE_NUMBER, wxSUBRELEASE_NUMBER, wxVERSION_NUMBER));
    dump += (wxString::Format("OCPN API version %d.%d\n", API_VERSION_MAJOR, API_VERSION_MINOR));
    dump += (wxString::Format("Duktape version %d\n", DUK_VERSION));
    wxString svg {"Not using svg"};
#ifdef JAVASCRIPT_USE_SVG
    svg = "Using svg";
#endif
    dump += (svg + "\n");
    dump += "pJavaScript_pi->m_pconfig\t\t\t" + ptrToString((Console *)pJavaScript_pi->m_pconfig) + "\n";
    dump += "pJavaScript_pi->m_parent_window\t\t" + ptrToString((Console *)pJavaScript_pi->m_parent_window) + "\n"; 

    dump += "favouriteFiles:\n";
    for (int i = 0; i < pJavaScript_pi->favouriteFiles.GetCount(); i++)
        dump += ("\t" + pJavaScript_pi->favouriteFiles[i] + "\n");
    dump += "recentFiles:\n";
    for (int i = 0; i < pJavaScript_pi->recentFiles.GetCount(); i++)
        dump += ("\t" + pJavaScript_pi->recentFiles[i] + "\n");
    dump += "pJavaScript_pi->mpFirstConsole\t" + ptrToString(pJavaScript_pi->mpFirstConsole) + "\n";
    for (pConsole = pJavaScript_pi->mpFirstConsole; pConsole != nullptr; pConsole = pConsole->mpNextConsole){
        dump += ("\n————————————Console " + pConsole->mConsoleName + "————————————\n");
        dump += (pConsole->consoleDump());
        }
    dump += "\n----------------------------------------------------\n";
    dump += "\npJavaScript_pi->mpBin\t\t" + ptrToString(pJavaScript_pi->mpBin) + "\n";
    for (pConsole = pJavaScript_pi->mpBin; pConsole != nullptr; pConsole = pConsole->mpNextConsole){
        dump += ("\n————————————Console in bin " + pConsole->mConsoleName + "————————————\n");
        dump += (pConsole->consoleDump());
        }
    dump += ("\nEnd of dump\n");
    dumpTextCtrl->AppendText(dump);
    dumpWindow->SetSize(FromDIP(wxSize(600, 900)));
    dumpWindow->Show();
    }

wxString JS_dumpString(wxString identifier, wxString string){
    // dumps string to output window
    wxString::const_iterator i;
    int j;
    wxString dumpTextCtrl = "\n" + identifier + "\n" + string + "\n";
    for (j = 0, i = string.begin(); i != string.end(); ++i, j++)
        {
        wxUniChar uni_ch = *i;
        dumpTextCtrl += wxString::Format("[%02d]%02X ", j, uni_ch);
        if ((j > 0) && ((j+1)%10 == 0)) dumpTextCtrl += "\n";
        }
    return dumpTextCtrl;
    }

void ToolsClass::onClean( wxCommandEvent& event ){
    wxWindow *stringWindow;
    wxTextCtrl *dumpTextCtrl;
    int j;
    wxString::const_iterator i;
    wxString JScleanString(wxString given);
    wxString text = this->m_charsToClean->GetValue();
    if (text == wxEmptyString) return;
    stringWindow = new wxDialog(this /* pJavaScript_pi->m_parent_window */, wxID_ANY, "JavaScript text cleaning", wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE );

    dumpTextCtrl = new wxTextCtrl(stringWindow, wxID_NEW,
                          wxEmptyString, wxDefaultPosition, wxSize(240, 100),
                          wxTE_MULTILINE);
    for (j = 0, i = text.begin(); i != text.end(); ++i, j++)
        {
        wxUniChar uni_ch = *i;
            dumpTextCtrl->AppendText(wxString::Format("[%02d]%c ", j, uni_ch));
        if ((j > 0) && ((j+1)%10 == 0)) dumpTextCtrl->AppendText("\n");
        }
    dumpTextCtrl->AppendText(JS_dumpString("\nRaw", text));
    text = JScleanString(text);
    dumpTextCtrl->AppendText(JS_dumpString("\nCleaned", text));
    stringWindow->SetSize(FromDIP(wxSize(500, 500)));
    stringWindow->Show();
    }
    
void ToolsClass::onParkingRevert(wxCommandEvent& event){
	// revert parking parameters to platform default
	pJavaScript_pi->m_parkingBespoke = false;
	pJavaScript_pi->m_parkingMinHeight = CONSOLE_MIN_HEIGHT;
	pJavaScript_pi->m_parkingStub = CONSOLE_STUB;
	pJavaScript_pi->m_parkingLevel = PARK_LEVEL;
	pJavaScript_pi->m_parkFirstX = PARK_FIRST_X;
	pJavaScript_pi->m_parkSep = PARK_SEP;
	m_parkingMessage->SetValue("Parking parameters reset to platform defaults");
	cleanupParking();
	pJavaScript_pi->SaveConfig();
	}

void ToolsClass::onParkingCustomise(wxCommandEvent& event){
	// customise parking parameters
	void appendStyledText(wxString text, wxStyledTextCtrl* window, int colour);
	wxPoint screenToFrame(wxPoint);
	m_parkingMessage->Clear();
	wxString label = m_customiseButton->GetLabel();
	TRACE(4, wxString::Format("In onParking with button %s", label));
	if (label == "Start"){
		// check if have parked consoles
		int x, y;
		Console* pConsole;
		int numberParked = 0;
		for (pConsole = pJavaScript_pi->mpFirstConsole; pConsole != nullptr; pConsole = pConsole->mpNextConsole){// for each console
			if (pConsole->isParked()){
				if (numberParked == 0) m_parkingMessage->SetValue("Unpark the parked console(s)\n");
				m_parkingMessage->AppendText(wxString::Format("%s\n", pConsole->mConsoleName));
				numberParked++;
				}
			}
		if (numberParked > 0){
			m_parkingMessage->AppendText("and then try again");
			return;
			}
		// create fist test console
		pTestConsole1 = new Console(pJavaScript_pi->m_parent_window, "M",FromDIP(wxPoint(250,250)), FromDIP(wxSize(700,300)));
		pTestConsole1->GetPosition(&x, &y);
		pTestConsole1->SetPosition(wxPoint(x, y));
		pTestConsole1->SetMinSize(wxSize(1,1));
		pTestConsole1->SetBackgroundColour(*wxBLUE);
		pTestConsole1->Show();
		pTestConsole2 = new Console(pJavaScript_pi->m_parent_window, "MMMMMMMMMMMMMM",FromDIP(wxPoint(300,600)), FromDIP(wxSize(700,300)));
		pTestConsole2->GetPosition(&x, &y);
		pTestConsole2->SetPosition(wxPoint(x, y));
		pTestConsole2->SetMinSize(wxSize(1,1));
		pTestConsole2->SetBackgroundColour(*wxGREEN);
		pTestConsole2->Show();
		m_parkingMessage->SetValue("Minimize the blue console M so that it has the minimum height and is just wide enough to see its name\n");
		m_parkingMessage->AppendText("Then move it into the desired left most parking position\n\n");
		m_parkingMessage->AppendText("Next minimize the green console MMMMMMMMMMMMMM so that it has minimum height and is just wide enough to see its name\n");
		m_parkingMessage->AppendText("Then move it into the first parking position just to the right of M\n\n");
		m_parkingMessage->AppendText("Then click Next");
		m_customiseButton->SetLabel("Next");
		}
	else if ((label == "Next") || (label == "Retry")){
		TRACE(4, "onParking next");
		// now for the calculation
		double scale = SCALE(this);	// for DIP corrections
		wxSize c1Size = pTestConsole1->GetSize();
		wxSize c2Size = pTestConsole2->GetSize();
		wxPoint c1Pos = screenToFrame(pTestConsole1->GetPosition());
		wxPoint c2Pos = screenToFrame(pTestConsole2->GetPosition());
		m_customiseButton->SetLabel("Retry");	// in case we have a problem
		if ((c1Size.y > 40*scale) || (c2Size.y > 40*scale)){
			m_parkingMessage->SetValue("You have not minimised the consoles\nDo so and try again");
			return;
			}
		if (abs(c1Pos.y - c2Pos.y) > 3*scale){
			m_parkingMessage->SetValue("Both consoles should be at the same level.\nAdjust and try again");
			return;
			}
		if ((c1Pos.x + c1Size.x) > c2Pos.x){
			m_parkingMessage->SetValue("Consoles overlap.\nAdjust and try again");
			return;
			}
		pJavaScript_pi->m_parkingMinHeight = ((c1Size.y + c2Size.y)/2)/scale;	// take average
		wxString label = pTestConsole2->GetLabel();
		wxSize labelSize = pTestConsole2->GetTextExtent(label);
		pJavaScript_pi->m_parkingStub = (c2Size.x - labelSize.x)/scale;
		pJavaScript_pi->m_parkingLevel = ((c1Pos.y + c2Pos.y)/2)/scale;
		pJavaScript_pi->m_parkFirstX = c1Pos.x/scale;
		pJavaScript_pi->m_parkSep = (c2Pos.x - (c1Pos.x + c1Size.x))/scale;
		pJavaScript_pi->m_parkingBespoke = true;
		pJavaScript_pi->SaveConfig();
		m_parkingMessage->SetValue("Custom parking parameters set and saved");
		m_customiseButton->SetLabel("Finish");		
		}
	else if (label == "Finish") {
		cleanupParking();
		m_customiseButton->SetLabel("Start");
		}
	}
	
void ToolsClass::onParkingReveal(wxCommandEvent& event) {
		m_parkingMessage->SetValue(pJavaScript_pi->m_parkingBespoke?"Bespoke settings\n\n":"Default settings\n\n");
		m_parkingMessage->AppendText(wxString::Format(
			"#define CONSOLE_MIN_HEIGHT %iL\n#define CONSOLE_STUB %iL\n#define PARK_LEVEL %iL\n#define PARK_FIRST_X %iL\n#define PARK_SEP %iL",
			pJavaScript_pi->m_parkingMinHeight, pJavaScript_pi->m_parkingStub, pJavaScript_pi->m_parkingLevel, pJavaScript_pi->m_parkFirstX, pJavaScript_pi->m_parkSep ));	
		m_parkingMessage->AppendText("\n\nThe above are all in DIP pixels");			
		}
	
void displayTools(wxWindow* parent){ // this used for testing in harness only
    ToolsClass *pToolsDialog = new ToolsClass(parent);

    pToolsDialog->Fit();
    pToolsDialog->Show();
    }
