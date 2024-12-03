/**************************************************************************
* Project:  OpenCPN
* Purpose:  JavaScript Plugin
* Author:   Tony Voss 16/05/2020
*
* Copyright Ⓒ 2024 by Tony Voss
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

   	if (pTestConsole1 != nullptr){ pTestConsole1->bin(); pTestConsole1 = nullptr; }
	if (pTestConsole2 != nullptr){ pTestConsole2->bin(); pTestConsole2 = nullptr; }
	m_parkingMessage->Clear();
	pJavaScript_pi->pTools = nullptr;
   //  this->Hide();
    Destroy();
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
        wxSize size;
        setConsoleChoices();
        cleanupParking();
        m_parkingMessage->Clear();
        m_customiseButton->SetLabel("Start");
        wxString currentDirectory = pJavaScript_pi->mCurrentDirectory;
    	mCurrentDirectoryString->SetLabel((currentDirectory == wxEmptyString)?"(Not yet set)":currentDirectory);
		m_floatOnParent->Show();	
    	m_floatOnParent->SetValue(pJavaScript_pi->m_floatOnParent);
    	m_rememberToggleStatus->SetValue(pJavaScript_pi->mRememberToggleStatus);
		m_notebook->ChangeSelection(pageNumber);
        page = m_notebook->GetPage(pageNumber);
		page->Fit();

        wxSize pageClientSize = ToDIP(page->GetClientSize());
        
        TRACE(6, wxString::Format("Tools Page %d GetClientSize gave DIP %d x %d", pageNumber, pJavaScript_pi->pTools->Diagnostics->GetId(), pageClientSize.x, pageClientSize.y));
        pageClientSize.x = 590;	// force width
		SetClientSize(FromDIP(pageClientSize));	// allow for screen resolution
//		page->Fit(); // Adjusts to page size but this makes window too tight, so...
		size = ToDIP(GetSize());
		size.x = 600;	// Force the window size
		size.y += 10;
		SetSize(FromDIP(size));
		TRACE(6, wxString::Format("Tools Page %d final size DIP %d x %d", pageNumber, size.x, size.y));
		Show();
        Raise();
        TRACE(6, wxString::Format("Page %d of %d", pageNumber, pJavaScript_pi->pTools->m_notebook->GetPageCount()));
        if (pageNumber == pJavaScript_pi->pTools->m_notebook->GetPageCount()-1){	// on last panel - assumed to be diagnostics
        	// following to be set to ‟Fancy quotes” degrees°º⁰ primes ‘’‛’′´` but Windows screws this up, so will set explicitly here
        	m_charsToClean->ChangeValue("\u201FFancy quotes\u201D degrees\u00B0\u00BA\u2070 primes \u2018\u2019\u201B\u2019\u2032\u00B4\u0060");
        	}
        
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
	pConsole->setup();
    pConsole->GetPosition(&x, &y);
    x += - 25 + rand()%50; y += - 25 + rand()%50;
    pConsole->SetPosition(wxPoint(x, y));
    pConsole->setConsoleMinClientSize();
    setConsoleChoices();    // update
    pConsole->Show();
    m_ConsolesMessage->AppendText(_("Console " + newConsoleName + " created"));
    }

void ToolsClass::onChangeName( wxCommandEvent& event ){
    wxString oldConsoleName, newConsoleName, outcome;
    wxSize minSize, oldSize, newSize;
    wxString checkConsoleName(wxString name, Console* pConsole);
    Console* findConsoleByName(wxString name);
    void reviewParking();
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
//    wxSize oldClientMinSize = pConsole->GetMinClientSize();
    pConsole->setConsoleMinClientSize();
	wxSize newMinSize = pConsole->GetMinSize();
	if (pConsole->m_parkedLocation.set){
		pConsole->m_parkedLocation.size.x = newMinSize.x;
		}
	if (pConsole->isParked()) pConsole->SetSize(newMinSize);	// shrink it
	reviewParking();
    m_ConsolesMessage->AppendText(_("Console " + oldConsoleName + " changed to " + newConsoleName));
    setConsoleChoices();    // update
    }
    
void ToolsClass::onFindAllConsoles( wxCommandEvent& event ){	// make all consoles visible
	wxPoint position = FromDIP(wxPoint(NEW_CONSOLE_POSITION));
	wxSize  size = FromDIP(wxSize(NEW_CONSOLE_SIZE));
	for (Console* pCon = pJavaScript_pi->mpFirstConsole; pCon != nullptr; pCon = pCon->mpNextConsole){	// work though all consoles
		if (pCon->isParked()) pCon->unPark();
		// shift by random amount
	    pCon->SetPosition(position);
		pCon->Move(position);		
		pCon->SetSize(size);
		pCon->Show();
		pCon->Raise();
		// now shift so next console does not exactly overlap
		int x, y;
		pCon->GetPosition(&x, &y);
		x += 20; y+= 25;
		position = wxPoint(x, y);
		}
	}
    
void ToolsClass::onFloatOnParent(wxCommandEvent& event) {
    Console *pConsole;
	pJavaScript_pi->m_floatOnParent = m_floatOnParent->GetValue();
	for (pConsole = pJavaScript_pi->mpFirstConsole; pConsole != nullptr; pConsole = pConsole->mpNextConsole){
		pConsole->floatOnParent(pJavaScript_pi->m_floatOnParent);
        }
	} 
	
void ToolsClass::onToggleStatus( wxCommandEvent& event ){
	pJavaScript_pi->mRememberToggleStatus = m_rememberToggleStatus->GetValue();
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
    wxFrame* dumpWindow;
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
    dumpWindow = new wxFrame(this /*pJavaScript_pi->m_parent_window */, wxID_ANY, "JavaScript plugin dump", dumpPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT  | wxRESIZE_BORDER);
    dumpTextCtrl = new wxTextCtrl(dumpWindow, wxID_NEW,
                          wxEmptyString, wxDefaultPosition, wxSize(240, 100),
                          wxTE_MULTILINE | wxTE_WORDWRAP);

    dump += (wxString::Format("JavaScript plugin version %d.%d\n", PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR));
    dump += (wxString::Format("JavaScript patch %d\n", PLUGIN_VERSION_PATCH));
    dump += (wxString::Format("JavaScript tools window DPI scaling factor %f\n", SCALE(this)));
    dump += (wxString::Format("Platform\t%s\n", wxGetOsDescription()));
#ifndef __LINUX__
    dump += (wxString::Format("Architecture\t%s\n", wxGetNativeCpuArchitectureName()));	// not available under Linux
#endif
    dump += (wxString::Format("wxWidgets version %d.%d.%d.%d or %d\n", wxMAJOR_VERSION, wxMINOR_VERSION, wxRELEASE_NUMBER, wxSUBRELEASE_NUMBER, wxVERSION_NUMBER));
    dump += (wxString::Format("OCPN API version %d.%d\n", API_VERSION_MAJOR, API_VERSION_MINOR));
    dump += (wxString::Format("Duktape version %d\n", DUK_VERSION));
    wxString svg {"Not using svg"};
#ifdef JAVASCRIPT_USE_SVG
    svg = "Using svg";
#endif
    dump += (svg + "\n");
    
	// display location of built-in scripts
	wxFileName filePath;
	wxString fileString = GetPluginDataDir("JavaScript_pi");
    filePath.SetPath(fileString);
    filePath.AppendDir("data");
    filePath.AppendDir("scripts");
    dump += "Built in scripts folder: " + filePath.GetPath() + "\n";

    dump += "pJavaScript_pi->m_pconfig\t\t\t\t\t" + ptrToString((Console *)pJavaScript_pi->m_pconfig) + "\n";
    dump += "pJavaScript_pi->m_parent_window\t\t\t" + ptrToString((Console *)pJavaScript_pi->m_parent_window) + "\n"; 
	dump += "pJavaScript_pi->m_floatOnTop\t\t\t\t" + (pJavaScript_pi->m_floatOnParent ? _("true"):_("false")) + "\n";
	dump += "pJavaScript_pi->mRememberToggleStatus\t" + (pJavaScript_pi->mRememberToggleStatus ? _("true"):_("false")) + "\n";
    dump += "favouriteFiles:\n";
    for (int i = 0; i < pJavaScript_pi->favouriteFiles.GetCount(); i++)
        dump += ("\t" + pJavaScript_pi->favouriteFiles[i] + "\n");
    dump += "recentFiles:\n";
    for (int i = 0; i < pJavaScript_pi->recentFiles.GetCount(); i++)
        dump += ("\t" + pJavaScript_pi->recentFiles[i] + "\n");
    int pgn_reg_count = pJavaScript_pi->m_pgnRegistrations.size();
    dump += "N2K pgn registrations:";
    if (pgn_reg_count > 0){
    	dump += "\n";
    	for (int h = 0; h < pgn_reg_count; h++){
    		dump += "\tHandle\t" + pJavaScript_pi->m_pgnRegistrations[h].handle + "\n";
    		for (int p = 0; p < pJavaScript_pi->m_pgnRegistrations[h].pgns.size(); p++){
    			dump += wxString::Format("\t\t%d", pJavaScript_pi->m_pgnRegistrations[h].pgns[p]);
    			}
    		dump += "\n";
    		}
    	}
    else dump += " none\n";
    dump += "pJavaScript_pi->mpFirstConsole\t" + ptrToString(pJavaScript_pi->mpFirstConsole) + "\n";
    for (pConsole = pJavaScript_pi->mpFirstConsole; pConsole != nullptr; pConsole = pConsole->mpNextConsole){
        dump += ("\n----------Console " + pConsole->mConsoleName + "----------\n");
        dump += (pConsole->consoleDump());
        }
    dump += "\n----------------------------------------------------\n";
    dump += "\npJavaScript_pi->mpBin\t\t" + ptrToString(pJavaScript_pi->mpBin) + "\n";
    for (pConsole = pJavaScript_pi->mpBin; pConsole != nullptr; pConsole = pConsole->mpNextConsole){
        dump += ("\n----------Console in bin " + pConsole->mConsoleName + "----------\n");
        dump += (pConsole->consoleDump());
        }
    dump += ("\nEnd of dump\n");
    dumpTextCtrl->AppendText(dump);
    dumpWindow->SetSize(FromDIP(wxSize(700, 1000)));
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
    stringWindow = new wxFrame(this /* pJavaScript_pi->m_parent_window */, wxID_ANY, "JavaScript text cleaning", wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT );

    dumpTextCtrl = new wxTextCtrl(stringWindow, wxID_NEW,
                          wxEmptyString, wxDefaultPosition, wxSize(240, 100),
                          wxTE_MULTILINE);
    for (j = 0, i = text.begin(); i != text.end(); ++i, j++)
        {
        wxUniChar uni_ch = *i;
        	wxString format = (uni_ch <= 0xFF) ? "[%02d]%c " : "[%04d]%c ";
            dumpTextCtrl->AppendText(wxString::Format(format, j, uni_ch));
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
		wxSize c1Size = pTestConsole1->GetClientSize();
		wxSize c2Size = pTestConsole2->GetClientSize();
		wxPoint c1Pos = pTestConsole1->GetPosition();
		wxPoint c2Pos = pTestConsole2->GetPosition();
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
		wxString label = pTestConsole2->GetLabel();
		wxSize labelSize = pTestConsole2->GetTextExtent(label);
		pJavaScript_pi->m_parkingStub = (c2Size.x - labelSize.x)/scale;
		pJavaScript_pi->m_parkingLevel = ((c1Pos.y + c2Pos.y)/2)/scale;
		pJavaScript_pi->m_parkFirstX = c1Pos.x/scale;
		pJavaScript_pi->m_parkSep = (c2Pos.x - (c1Pos.x + c1Size.x))/scale;
		pJavaScript_pi->m_parkingBespoke = true;
		pJavaScript_pi->SaveConfig();
		m_parkingMessage->SetValue("Custom parking parameters set and saved");
		cleanupParking();
		m_customiseButton->SetLabel("Start");		
		}
	}
	
void ToolsClass::onParkingReveal(wxCommandEvent& event) {
		m_parkingMessage->SetValue(pJavaScript_pi->m_parkingBespoke?"Bespoke settings\n\n":"Default settings\n\n");
		m_parkingMessage->AppendText(wxString::Format(
			"#define CONSOLE_STUB %iL\n#define PARK_LEVEL %iL\n#define PARK_FIRST_X %iL\n#define PARK_SEP %iL",
			pJavaScript_pi->m_parkingStub, pJavaScript_pi->m_parkingLevel, pJavaScript_pi->m_parkFirstX, pJavaScript_pi->m_parkSep ));	
		m_parkingMessage->AppendText("\n\nThe above are all in DIP pixels");			
		}
	
void displayTools(wxWindow* parent){ // this used for testing in harness only
    ToolsClass *pToolsDialog = new ToolsClass(parent);
	pToolsDialog->fixForScreenRes();
    pToolsDialog->Fit();
    pToolsDialog->Show();
    }