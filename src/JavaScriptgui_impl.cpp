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

#include <iostream>
#include "JavaScript_pi.h"
#include "JavaScriptgui_impl.h"
#include <wx/wx.h>
#include <wx/arrimpl.cpp>
#include <wx/button.h>
#include "ocpn_plugin.h"
#include "trace.h"
#include <wx/event.h>
#include "wx/window.h"

#define FAIL(X) do { error = X; goto failed; } while(0)

extern JavaScript_pi *pJavaScript_pi;

void updateRecentFiles(wxString fileString){
    //  Updates the array of recent file names
    const int maxFiles {10}; // maximum number of recents to save
    if (pJavaScript_pi->favouriteFiles.Index(fileString) == wxNOT_FOUND){  // not in favourites, so revise recents
        pJavaScript_pi->recentFiles.Remove(fileString); // remove any existing matching entry
        pJavaScript_pi->recentFiles.Insert(fileString, 0);  // insert this one as the first
        int newCount = pJavaScript_pi->recentFiles.GetCount();
        // trim list if over maximum
        if (newCount > maxFiles) pJavaScript_pi->recentFiles.RemoveAt(maxFiles, newCount-maxFiles);
        }
    }

int messageComp(MessagePair** arg1, MessagePair** arg2) {   // used when sorting messages
    return (strcmp((*arg1)->messageName, (*arg2)->messageName));}

WX_DEFINE_OBJARRAY(MessagesArray);
WX_DEFINE_OBJARRAY(MenusArray);
#ifdef SOCKETS
WX_DEFINE_OBJARRAY(SocketRecordsArray);
#endif
WX_DEFINE_OBJARRAY(ConsoleRepliesArray);

/*
void Console::OnActivate(wxActivateEvent& event){
	if (event.GetActive()){
	    TRACE(110, "Activated");
		if (m_clickFunction != wxEmptyString){
			wxString function = m_clickFunction;
			m_clickFunction = wxEmptyString;
			Completions outcome = executeFunctionNargs(function, 0);
			if (!isBusy()) wrapUp(outcome);
			}
		}
	wxTextCtrl* what = (wxTextCtrl*) event.GetEventObject();
	what->SetFocus();
	event.Skip();
 	return;
    wxFrame* pConsole = wxDynamicCast(event.GetEventObject(), wxFrame);
    long int style = pConsole->GetWindowStyle();
    if (event.GetActive()) pConsole->SetWindowStyle(style | wxSTAY_ON_TOP); // bring console on top
	pConsole->SetWindowStyle(style ^ wxSTAY_ON_TOP);    // but do not undo from v2.0.3
    };
*/
    
/*
void Console::OnLeftDClick(wxMouseEvent& event){
    TRACE(110, "LeftEvent");
	if (m_clickFunction != wxEmptyString){
		wxString function = m_clickFunction;
		m_clickFunction = wxEmptyString;
		Completions outcome = executeFunctionNargs(function, 0);
		if (!isBusy()) wrapUp(outcome);
		}
	// event.Skip();	// do rest of focussing
	}
*/

void Console::OnLoad( wxCommandEvent& event ) { // we are to load a script
    wxString fileString;
    wxTextFile ourFile;
    wxString lineOfData, script, result;
    wxString JScleanString(wxString line);
    wxString chooseFileString(Console*);
    wxString chooseLoadFile(Console*);
    bool isURLfileString(wxString fileString);
    
    fileString = chooseFileString(this);
	if (fileString == "**cancel**"){
	TRACE(3, "Load cancelled");
	return;
	}
    if (fileString == wxEmptyString) fileString = chooseLoadFile(this);  // if no favourite or recent ask for new
    else {   // only if something now chosen
    	fileString.Trim(true);	// trim any whitespace both ends
    	fileString.Trim(false);
    	if (isURLfileString(fileString)){
    		// it's a URL
    		if (!OCPN_isOnline()) {
    			message(STYLE_RED, "OpenCPN not on-line");
    			return;
    			}
    		wxString getTextFile(wxString fileString, wxString* pText);
    		result = getTextFile(fileString, &script);
    		if (result != wxEmptyString){
    		    TRACE(6, "URL not yielded file result:" + result + "\n" + script + "\n");
    			message(STYLE_RED, result);
    			pJavaScript_pi->recentFiles.Remove(fileString); // remove from recents
    			return;
    			}
    		}
    	else {
			bool OK = ourFile.Open(fileString);
			if (!OK){ // can't read file - it may have gone away
				message(STYLE_RED, "File '" + fileString + "' not found or unreadable");
				pJavaScript_pi->recentFiles.Remove(fileString); // remove from recents
				return;
				}
			script = wxEmptyString;
			for (lineOfData = ourFile.GetFirstLine(); !ourFile.Eof(); lineOfData = ourFile.GetNextLine()){
				script += lineOfData + "\n";
				}
			}
		}
	script = JScleanString(script);
	m_Script->ClearAll();   // clear old content
	m_Script->AppendText(script);
	m_fileStringBox->SetValue(wxString(fileString));
	auto_run->Show();
	auto_run->SetValue(false);
	updateRecentFiles(fileString);
    }

void Console::OnSaveAs( wxCommandEvent& event ) {
    int response = wxID_CANCEL;
    wxArrayString file_array;
    wxString filename, filePath;
    wxTextFile ourFile;
    wxString lineOfData;
    wxDialog query;
    
    wxFileDialog SaveAsConsole( this, _( "Saving your script" ), wxEmptyString, wxEmptyString,
                               _T("js files (*.js)|*.js|All files (*.*)|*.*"),
                               wxFD_SAVE|wxFD_OVERWRITE_PROMPT|wxDD_NEW_DIR_BUTTON);
    response = SaveAsConsole.ShowModal();
    if( response == wxID_OK ) {
        filePath = SaveAsConsole.GetPath();
        if (!filePath.EndsWith(".js")) filePath += ".js";
        m_Script->SaveFile(filePath, wxTEXT_TYPE_ANY);
        m_fileStringBox->SetValue(wxString(filePath));
        auto_run->Show();
        updateRecentFiles(filePath);
        return;
    }
    else if(response == wxID_CANCEL){
        return;
        }
    }

void Console::OnSave( wxCommandEvent& event ) {
    wxArrayString file_array;
    wxString filename;
    wxTextFile ourFile;
    wxString lineOfData;
    wxDialog query;
    
    mFileString = m_fileStringBox->GetValue();
    if ((   mFileString != "") && wxFileExists(   mFileString) && !isURLfileString(mFileString)) {
        // Have a 'current' file, so can just save to it
        m_Script->SaveFile(mFileString);
        TRACE(3, wxString::Format("Saved to  %s",mFileString));
        auto_run->Show();
        updateRecentFiles(mFileString);
        return;
        }
    else OnSaveAs(event);  // No previous save, so do Save As
    return;
    }

void Console::OnCopyAll(wxCommandEvent& event) {
	TRACE(4, "OnCopyAll");
    int currentPosition = m_Script->GetCurrentPos();
    m_Script->SelectAll();
    m_Script->Copy();
    m_Script->GotoPos(currentPosition);
    }

void Console::OnClearScript( wxCommandEvent& event ){
    m_Script->ClearAll();
    m_Script->SetFocus();
    m_fileStringBox->SetValue(wxEmptyString);
    mFileString = wxEmptyString;
    auto_run->SetValue(false);
    auto_run->Hide();
    }

void Console::OnClearOutput( wxCommandEvent& event ){
    m_Output->ClearAll();
    }

void Console::OnRun( wxCommandEvent& event ) {
#if TRACE_YES
    extern JavaScript_pi *pJavaScript_pi;
    if (!pJavaScript_pi->mTraceLevelStated)
        message(STYLE_ORANGE, wxString::Format("Tracing levels %d - %d",TRACE_MIN, TRACE_MAX));
    pJavaScript_pi->mTraceLevelStated = true;
#endif
	if (m_Script->IsEmpty()) {
		message(STYLE_RED, "No script to run");
		return;
		}
	wxString script = m_Script->GetText();
	if (script == wxEmptyString)
    clearBrief();
    mConsoleRepliesAwaited = 0;
    doRunCommand(mBrief);
    }

void Console::OnAutoRun(wxCommandEvent& event){   // Auto run tick box
    if (this->auto_run->GetValue()){
        // box has been ticked
        mFileString  = m_fileStringBox->GetValue();
        if ((   mFileString != "") && wxFileExists(   mFileString /*FilePath*/)){
            // Have a 'current' file we can auto-run
            this->m_autoRun = true;
            }
        else {
            this->message(STYLE_RED, wxString("No script loaded to auto-run"));
            this->auto_run->SetValue(false);
            this->m_autoRun = false;
            }
        }
    else this->m_autoRun = false;
    }
    
void Console::OnPark( wxCommandEvent& event ){
	TRACE(25, mConsoleName + " OnPark start");
    park();
    TRACE(25, mConsoleName + " OnPark finished");
    }

void Console::OnClose(wxCloseEvent& event) {
    extern JavaScript_pi *pJavaScript_pi;
    void reviewParking();
    TRACE(1, "Closing console " + this->mConsoleName + " Can veto is " + (event.CanVeto()?"true":"false"));
    if (event.CanVeto()){
		if (m_closeButtonFunction != wxEmptyString){
			wxString function = m_closeButtonFunction;
			m_closeButtonFunction = wxEmptyString;
			Completions outcome = executeFunctionNargs(function, 0);
			if (!isBusy()) wrapUp(outcome);
			return;
			}
		if (isParked()){	// hit close button when parked and minimised
			TRACE(25, wxString::Format("%s->onClose unparking", mConsoleName));
			unPark();
        	Raise();
        	return;
        	}

        if ((this == pJavaScript_pi->mpFirstConsole) && (this->mpNextConsole == nullptr)) {
            // This is only console - decline
            this->message(STYLE_RED, "Console close: You cannot close the only console");
            event.Veto(true);
            return;
            }
        if (this->isBusy()) {
            // We will not delete running console, although it is safe to do so
            wxString text = "Console close: stop";
            if (!this->m_Script->IsEmpty()) text += " and clear";
            text +=  " the script first";
            this->message(STYLE_RED, text);
            event.Veto(true);
            return;
            }
        if (!this->m_Script->IsEmpty()) {
            // We will not delete a console with a script
            this->message(STYLE_RED, "Console close: clear the script first");
            event.Veto(true);
            return;
            }
        if (this->m_remembered != wxEmptyString){
        	wxString message = "You will lose what you have in _remember\nProceed anyway?";
        	int choice = wxMessageBox(message, "Close console", wxYES_NO | wxYES_DEFAULT);
        	if (choice == wxNO){
        		event.Veto(true);
        		return;
        		}
        	}
		this->bin();
		reviewParking();
		// take care to remove from tools, if we have them open
		ToolsClass *pTools = pJavaScript_pi->pTools;
		if (pTools != nullptr) pTools->setConsoleChoices();
		TRACE(3, "Binning console " + this->mConsoleName);
		event.Veto(true);
		}
    }

static wxString dummyMessage, message_id;
wxDialog* dialog;

void Console::OnHelp( wxCommandEvent& event){
    extern JavaScript_pi *pJavaScript_pi;
    
    pJavaScript_pi->ShowTools(pJavaScript_pi->m_parent_window, -1);
    return;
    }

void Console::OnTools( wxCommandEvent& event){
    extern JavaScript_pi *pJavaScript_pi;
    
    pJavaScript_pi->ShowTools(pJavaScript_pi->m_parent_window, 0);
    return;
    }
    
void Console::HandleTimer(wxTimerEvent& event){
	TRACE(66, wxString::Format("in HandleTimer "));
	int id = event.GetId();
	bool matched = false;
	for (auto it = mpTimersVector.cbegin(); it != mpTimersVector.cend(); ++it){
		auto entry = (*it);
		if (entry.timer->GetId() == id){
			wxString functionToCall = entry.functionName;
			wxString parameter = entry.parameter;
	    	if (entry.timer->IsOneShot()) mpTimersVector.erase(it);
	    	duk_push_string(mpCtx, parameter.c_str());
	    	Completions outcome = executeFunctionNargs(functionToCall, 1);
	    	if (!isBusy()) wrapUp(outcome);
	    	matched = true;
	    	break;
	    	}	    
		}
	if (!matched) message(STYLE_RED, "HandleTimer prog error: failed to match timer ID");
	};
    

void Console::HandleNMEA0183(ObservedEvt& ev, int messageCntlId) {
    Completions outcome;
    TRACE(23, wxString::Format("Starting HandleNMEAstream messageCntlId is %d", messageCntlId));
    if (!isWaiting()) return;  // ignore if we are not waiting on something.  Should not be - being safe
    // look for our messageCntl entry
    for (auto it = m_streamMessageCntlsVector.cbegin(); it != m_streamMessageCntlsVector.cend(); ++it){
        auto entry = *it;
        if (entry.messageCntlId == messageCntlId) {
            NMEA0183Id nmeaId(entry.id0183.ToStdString());
            wxString sentence = wxString(GetN0183Payload(nmeaId, ev));
            // check the checksum
            wxString NMEAchecksum(wxString sentence);
            wxUniChar star = '*';
            wxString checksum;
            sentence.Trim();
            size_t starPos = sentence.find(star); // position of *
            if (starPos != wxNOT_FOUND){ // yes there is one
                checksum = sentence.Mid(starPos + 1, 2);
                sentence = sentence.SubString(0, starPos-1); // truncate at * onwards
                }
            wxString correctChecksum = NMEAchecksum(sentence);
            bool OK = (checksum == correctChecksum) ? true : false;
            duk_push_object(mpCtx);
                duk_push_string(mpCtx, sentence.c_str());
                    duk_put_prop_literal(mpCtx, -2, "value");
                duk_push_boolean(mpCtx, OK);
                    duk_put_prop_literal(mpCtx, -2, "OK");
            // maybe drop this element of vector before executing function
            if (!entry.persist) m_streamMessageCntlsVector.erase(it);
            outcome = executeFunction(entry.functionName);
            if (!isBusy()) wrapUp(outcome);
            return;
            }
        }
    }
    
void Console::HandleNMEA2k(ObservedEvt& ev, int messageCntlId) {
    Completions outcome;
    TRACE(23, wxString::Format("Starting HandleNMEA2k messageCntlId is %d", messageCntlId));
    if (!isWaiting()) return;  // ignore if we are not waiting on something.  Should not be - being safe
    // look for our messageCntl entry
    for (auto it = m_streamMessageCntlsVector.cbegin(); it != m_streamMessageCntlsVector.cend(); ++it){
        auto entry = *it;
        if (entry.messageCntlId == messageCntlId) {
            NMEA2000Id nmea2kId(entry.id2k);
            std::string source = GetN2000Source(nmea2kId, ev);
			std::vector<uint8_t> payload = GetN2000Payload(nmea2kId, ev);
			unsigned int pgn = payload[3] | (payload[4] << 8) | (payload[5] << 16);
			duk_push_array(mpCtx);	// 1st arg will be data array
			int j = 0;
			int count = payload.size();
			if (((count - payload.at(12) - 13) == 1) && (payload.back() == 85)) count--;	// drop any crc if present
			for (int i = 0; i < count; i++){
//	was		for (int i = 0; i < payload.size()-1; i++){	// drop the dummy CRC end byte
				duk_push_uint(mpCtx, payload.at(i));
				duk_put_prop_index(mpCtx, -2, j++);
				}
			duk_push_uint(mpCtx, pgn);	// 2nd arg is pgn
			duk_push_string(mpCtx, source.c_str());	// 3rd is source
            // drop this element of vector before executing function
            if (!entry.persist) m_streamMessageCntlsVector.erase(it);		// drop if not persistent
            outcome = executeFunctionNargs(entry.functionName, 3);
            if (!isBusy()) wrapUp(outcome);
            return;	// only fulfil one entry per call
            }
        }
    }    
    
    void Console::HandleNavdata(ObservedEvt& ev, int messageCntlId) {
    Completions outcome;
//    bool matched {false};
    TRACE(23, wxString::Format("Starting HandleNavdata messageCntlId is %d", messageCntlId));
    if (!isWaiting()) return;  // ignore if we are not waiting on something.  Should not be - being safe
    // look for our messageCntl entry
    for (auto it = m_streamMessageCntlsVector.cbegin(); it != m_streamMessageCntlsVector.cend(); ++it){
        auto entry = *it;
        if (entry.messageCntlId == messageCntlId) {
//            matched = true;
            PluginNavdata navdata = GetEventNavdata(ev);
			duk_push_object(mpCtx);
				duk_push_number(mpCtx, navdata.time);
					duk_put_prop_literal(mpCtx, -2, "fixTime");
				duk_push_object(mpCtx);                                  // start of position
					duk_push_number(mpCtx, navdata.lat);
						duk_put_prop_literal(mpCtx, -2, "latitude");
					duk_push_number(mpCtx, navdata.lon);
						duk_put_prop_literal(mpCtx, -2, "longitude");
					duk_put_prop_literal(mpCtx, -2, "position");             // end of position
				duk_push_number(mpCtx, navdata.sog);
					duk_put_prop_literal(mpCtx, -2, "SOG");
				duk_push_number(mpCtx, navdata.cog);
					duk_put_prop_literal(mpCtx, -2, "COG");
				duk_push_number(mpCtx, navdata.var);
					duk_put_prop_literal(mpCtx, -2, "variation");
				duk_push_number(mpCtx, navdata.hdt);
					duk_put_prop_literal(mpCtx, -2, "HDT");
            // drop this element of vector before executing function
            if (!entry.persist) m_streamMessageCntlsVector.erase(it);		// drop if not persistent
            outcome = executeFunction(entry.functionName);
            if (!isBusy()) wrapUp(outcome);
            return;
            }
        }
    // if (!matched) this->message(STYLE_RED, "HandleNavdata prog error - failed to match messageCntl entry");
    }

