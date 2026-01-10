/**************************************************************************
* Project:  OpenCPN
* Purpose:  JavaScript Plugin
* Author:   Tony Voss 16/05/2020
*
* Copyright Ⓒ 2025 by Tony Voss
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
#include "wx/socket.h"

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

#ifdef SOCKETS
WX_DEFINE_OBJARRAY(SocketRecordsArray);
#endif

void Console::OnLoad( wxCommandEvent& event ) { // we are to load a script
    wxString fileString;
    wxTextFile ourFile;
    wxString lineOfData, script, result;
    wxString chooseFileString(Console*);
    wxString chooseLoadFile(Console*);
    bool isURLfileString(wxString fileString);
    
    if (m_Script->IsModified() && !wxGetKeyState(WXK_SHIFT)){
    	wxMessageDialog dialog(this, "(Shift key down supresses this check)\n\nScript pane has unsaved script - overwrite?", "Load script",wxYES_NO | wxICON_QUESTION); 
    	if (dialog.ShowModal() == wxID_NO) return;
    	}
    
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
    		wxString getTextFile(wxString fileString, wxString* pText, int timeout);
    		result = getTextFile(fileString, &script, 10);
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
	m_Script->ClearAll();   // clear old content
	m_Script->AppendText(script);
	m_Script->DiscardEdits();
	m_fileStringBox->SetValue(wxString(fileString));
	auto_run->Show();
	auto_run->SetValue(false);
	updateRecentFiles(fileString);
    }

void Console::OnSaveAs( wxCommandEvent& event ) {
    int response = wxID_CANCEL;
    wxArrayString file_array;
    wxString fileName, filePath;
    wxTextFile ourFile;
    wxString lineOfData;
    wxFileName fileDirectory("");
    wxFileName savedPath(m_fileStringBox->GetValue());
    wxDialog query;
    
    if (m_Script->IsEmpty()) return;    
    fileName = savedPath.GetName();	// pick up filename if one exists, else em empty string
	if (wxGetKeyState(WXK_SHIFT) && wxGetKeyState(WXK_ALT)){	// To save to built-in directory
		fileDirectory.SetPath(GetPluginDataDir("JavaScript_pi"));
		fileDirectory.AppendDir("data");
		fileDirectory.AppendDir("scripts");
		}		
       
    wxFileDialog SaveAsConsole( this, _( "Saving your script" ), fileDirectory.GetPath(), fileName,
    				"JavaScript files (*.js)|*.js",
                    wxFD_SAVE|wxFD_OVERWRITE_PROMPT|wxDD_NEW_DIR_BUTTON);
    response = SaveAsConsole.ShowModal();
    if( response == wxID_OK ) {
        filePath = SaveAsConsole.GetPath();
        if (!filePath.EndsWith(".js")) filePath += ".js";
        m_Script->SaveFile(filePath, wxTEXT_TYPE_ANY);
        m_Script->DiscardEdits();	// mark as saved
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
    wxString fileName;
    wxTextFile ourFile;
    wxString lineOfData;
    wxDialog query;
	if (m_Script->IsEmpty()) return;
	mFileString = m_fileStringBox->GetValue();
    if ((   mFileString != "") && wxFileExists(mFileString) && !isURLfileString(mFileString)) {
        // Have a 'current' file, so can just save to it
        m_Script->SaveFile(mFileString);
        m_Script->DiscardEdits();	// mark as saved
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
    if (m_Script->IsModified() && !wxGetKeyState(WXK_SHIFT)){
    	wxMessageDialog dialog(this, "(Shift key down supresses this check)\n\nScript pane has unsaved script - overwrite?", "Clear script",wxYES_NO | wxICON_QUESTION); 
    	if (dialog.ShowModal() == wxID_NO) return;
    	}
    m_Script->ClearAll();
    m_Script->DiscardEdits();
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
    if (!pJavaScript_pi->mTraceLevelStated){
    	wxString traceMessage = "Tracing levels";
    	if (TRACE_LEVEL_0) traceMessage += " 0";
    	if (TRACE_THIS != 0) traceMessage +=  wxString::Format(" %d", TRACE_THIS);
    	if (TRACE_MAX >= TRACE_MIN) traceMessage += wxString::Format(" %d - %d",  TRACE_MIN, TRACE_MAX);
        message(STYLE_ORANGE,traceMessage);
        }
    pJavaScript_pi->mTraceLevelStated = true;
#endif
	if (m_Script->IsEmpty() && (run_button->GetLabel() == "Run")) {
		message(STYLE_RED, "No script to run");
		return;
		}
    clearBrief();
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
    if (event.CanVeto()){	// only if can take control
    	if ( wxGetKeyState(WXK_CONTROL)){
    		std::shared_ptr<callbackEntry> pEntry;
    		for (auto it = mCallbacks.begin(); it != mCallbacks.end(); /* ++it */) {
				pEntry = *it;	
				if (pEntry && pEntry->type == CB_CLOSE) {
					mWaitingCached = false;
					if (!pEntry->persistant) mCallbacks.erase(it);
					else it++;	// persistant, so move on to next entry
					Completions outcome = executeCallableNargs(pEntry->func_heapptr, 0);
					if (!isBusy()) {
						wrapUp(outcome);
						return;
						}
					continue; // more to do and it now points to next entry so just interate
					}
				else {	// not CB_CLOSE
					it++;	// so move onto the next
					}
				}
    		event.Veto(true);
			return;
			}
    		
/*    		
    		for (
    			pEntry = getCallbackEntryByType(CB_CLOSE, false);
    			(pEntry);
    			pEntry = getCallbackEntryByType(CB_CLOSE, false)){
    			Completions outcome = executeCallableNargs(pEntry->func_heapptr, 0);
    			mWaitingCached = false;
    			if (!isBusy()) wrapUp(outcome);
    			}
*/

		if (isParked()){	// hit close button when parked and minimised
			TRACE(25, wxString::Format("%s->onClose unparking", mConsoleName));
			unPark();
        	Raise();
        	event.Veto(true);
        	return;
        	}
        if (pJavaScript_pi->m_consoles.size() < 2) {
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
            this->message(STYLE_RED, "Console close: clear the script first\nOr if you want to call a close button handler, hold down command key");
            event.Veto(true);
            return;
            }
        if ((this->m_remembered != wxEmptyString) && (this->m_remembered != "{}")){
        	wxString message = "You will lose what you have in _remember\nProceed anyway?";
        	int choice = wxMessageBox(message, "Close console", wxYES_NO | wxYES_DEFAULT);
        	if (choice == wxNO){
        		event.Veto(true);
        		return;
        		}
        	}
        deleteMe();
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
	TRACE(66, wxString::Format("in HandleTimer"));
	int id = event.GetId();
	if (mCallbacks.empty()) return;
	std::shared_ptr<callbackEntry> pEntry = getCallbackEntry(id, false);
	if (!pEntry){
		message(STYLE_RED, "HandleTimer - program error - failed to match id");
		return;
		}
	void* heapptr = pEntry->func_heapptr;
	wxString parameter = pEntry->parameter;
	mWaitingCached = false;
	duk_push_string(mpCtx, parameter.c_str());
	Completions outcome = executeCallableNargs(heapptr, 1);
	TRACE(75, wxString::Format("mCallbacks entry IsOneShot %s", pEntry->timer->IsOneShot()?"True":"False"));
	if (!isBusy()) wrapUp(outcome);
	return;
	};
    
void Console::HandleNMEA0183(ObservedEvt& ev, int id) {
	bool checkNMEAsum(wxString sentence);
    
    if (!isWaiting()) return;  // ignore if we are not waiting on something.  Should not be - being safe
    std::shared_ptr<callbackEntry> pEntry = getCallbackEntry(id, false);
	// It turns out that this method is sometimes called even after the entry has been removed
	// Probably a thread sequence phenomenon.
	// So we will ignore the entry not being found - no message unless debugging
	if (pEntry->id == 0){
		message(STYLE_RED, wxString::Format("HandleNMEA0183 - program error - failed to match id %i", id));
		return;
		}
	void* heapptr = pEntry->func_heapptr;
	NMEA0183Id nmeaId = pEntry->_IDENT.ToStdString();
    wxString sentence = wxString(GetN0183Payload(nmeaId, ev));
	bool OK = checkNMEAsum(sentence);
	size_t starPos = sentence.find("*");
	sentence = sentence.SubString(0, starPos-1); // truncate at * onwards
	duk_push_object(mpCtx);
	duk_push_string(mpCtx, sentence.c_str());
		duk_put_prop_literal(mpCtx, -2, "value");
	duk_push_boolean(mpCtx, OK);
		duk_put_prop_literal(mpCtx, -2, "OK");
	Completions outcome = executeCallableNargs(heapptr, 1);	// only one object on stack
	if (!isBusy()) wrapUp(outcome);
	return;
    }
    
void Console::HandleNMEA2k(ObservedEvt& ev, int id) {
    if (!isWaiting()) return;  // ignore if we are not waiting on something.  Should not be - being safe
	std::shared_ptr<callbackEntry> pEntry = getCallbackEntry(id, false);
	// It turns out that this method is sometimes called even after the entry has been removed
	// Probably a thread sequence phenomenon.
	// So we will ignore the entry not being found - no message unless debugging
	if (pEntry->id == 0){
		message(STYLE_RED, wxString::Format("HandleMessage - program error - failed to match id %i", id));
		return;
		}
	void* heapptr = pEntry->func_heapptr;
	NMEA2000Id nmea2kId(pEntry->_PGN);
	std::string source = GetN2000Source(nmea2kId, ev);
	std::vector<uint8_t> payload = GetN2000Payload(nmea2kId, ev);
	unsigned int pgn = payload[3] | (payload[4] << 8) | (payload[5] << 16);
	duk_push_array(mpCtx);	// 1st arg will be data array
	int j = 0;
	int count = payload.size();
	if (((count - payload.at(12) - 13) == 1) && (payload.back() == 85)) count--;	// drop any crc if present
	for (int i = 0; i < count; i++){
		duk_push_uint(mpCtx, payload.at(i));
		duk_put_prop_index(mpCtx, -2, j++);
		}
	duk_push_uint(mpCtx, pgn);	// 2nd arg is pgn
	duk_push_string(mpCtx, source.c_str());	// 3rd is source
	Completions outcome = executeCallableNargs(heapptr, 3);
	if (!isBusy()) wrapUp(outcome);
    }  
    
void Console::HandleNavdata(ObservedEvt& ev, int messageId) {
    if (!isWaiting()) return;  // ignore if we are not waiting on something.  Should not be - being safe
	std::shared_ptr<callbackEntry> pEntry = getCallbackEntry(messageId, false);
	if (!pEntry){
		message(STYLE_RED, wxString::Format("HandleNavigation - program error - failed to match id %i", messageId));
		return;
		}
	void* heapptr = pEntry->func_heapptr;
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
	Completions outcome = executeCallableNargs(heapptr, 1);
	if (!isBusy()) wrapUp(outcome);
	return;
	}
 
 void Console::HandleNotificationAction(ObservedEvt& ev, int messageId) {
    if (!isWaiting()) return;  // ignore if we are not waiting on something.  Should not be - just being safe
    NotificationMsgId id;
	std::shared_ptr<callbackEntry> pEntry = getCallbackEntry(messageId, false);
	// It turns out that this method is sometimes called even after the entry has been removed
	// Probably a thread sequence phenomenon.
	// So we will ignore the entry not being found - no message unless debugging
	if (!pEntry){
		message(STYLE_RED, wxString::Format("HandleMessage - program error - failed to match id %i", messageId));
		return;
		}
	void* heapptr = pEntry->func_heapptr;
	std::shared_ptr<PI_Notification> notification = GetNotificationMsgPayload(id, ev);
	duk_push_object(mpCtx);
	duk_push_string(mpCtx, wxString(notification->guid));
	duk_put_prop_literal(mpCtx, -2, "GUID");
	duk_push_int(mpCtx, (int)notification->severity);
	duk_put_prop_literal(mpCtx, -2, "severity");
	duk_push_string(mpCtx, wxString(notification->message));
	duk_put_prop_literal(mpCtx, -2, "message");
	duk_push_int(mpCtx,(int)notification->auto_timeout_start);
	duk_put_prop_literal(mpCtx, -2, "timeStart");
	duk_push_int(mpCtx,(int)notification->auto_timeout_left);
	duk_put_prop_literal(mpCtx, -2, "timeRemaining");
	duk_push_string(mpCtx, wxString(notification->action_verb));
	duk_put_prop_literal(mpCtx, -2, "action");
	Completions outcome = executeCallableNargs(heapptr, 1);
	if (!isBusy()) wrapUp(outcome);
	return;
    }
    
void Console::HandleMessage(ObservedEvt& ev, int messageId) {
    wxString ptrToString(Console* address);
	std::shared_ptr<callbackEntry> pEntry = getCallbackEntry(messageId, false);
	// It turns out that this method is sometimes called even after the entry has been removed
	// Probably a thread sequence phenomenon.
	// So we will ignore the entry not being found - no message unless debugging
	if (!pEntry){
		// message(STYLE_RED, wxString::Format("HandleMessage - program error - failed to match id %i", messageId));  // Only during testing
		return;
		}
	void* heapptr = pEntry->func_heapptr;
	PluginMsgId id(pEntry->parameter.ToStdString());
	wxString message = GetPluginMsgPayload(id, ev);
	duk_push_string(mpCtx, message);
	mWaitingCached = false;
	Completions outcome = executeCallableNargs(heapptr, 1);
	if (!isBusy()) wrapUp(outcome);
	return;
    }

void Console::PollSocket(wxTimerEvent& event){
	int id = event.GetId();
	if (mCallbacks.empty()) return;
	std::shared_ptr<callbackEntry> pEntry = getCallbackEntry(id, false);
	if (!pEntry){
		message(STYLE_RED, "PollSocket - program error - failed to match id");
		return;
		}
	if (!pEntry->datagram.socket->IsData()) return;// no data
	uint8_t buf[2048];
	wxIPV4address sender;
	int errorNumber {0};	
	pEntry->datagram.socket->RecvFrom(sender, buf, sizeof(buf));
	if (pEntry->datagram.socket->Error()) {
		errorNumber = pEntry->datagram.socket->LastError();
		message(STYLE_RED, wxString::Format("PollSocket - RecvFrom error %d", errorNumber));
		}
	pEntry->datagram.lastSender = sender; 	// Cache sender for replies
	// Process buf[0..len)
	duk_push_number(mpCtx, errorNumber);
	if (errorNumber == 0){	// no error - have data
		size_t len = pEntry->datagram.socket->LastCount();
		uint8_t first = static_cast<uint8_t>(buf[0]);	// might this be CBOR data?	
		if (first == MAGIC_CBOR){// yes
			const uint8_t* cborData = buf + 1;
			size_t cborlen = len - 1;
			void* dst = duk_push_fixed_buffer(mpCtx, cborlen);
			memcpy(dst, cborData, cborlen);
			}
		else {	// assume string
			wxString payload = wxString::FromUTF8(reinterpret_cast<const char*>(buf), len);
			duk_push_string(mpCtx, payload);
			}		
		}
	mWaitingCached = false;
	Completions outcome = executeCallableNargs(pEntry->func_heapptr, (errorNumber == 0) ? 2 : 1);
	if (!isBusy()) wrapUp(outcome);
	return;
    }
    
// Smart indenting & re-flowing starts here

struct BraceDelta {
    int curly = 0;   // { }
    int round = 0;   // ( )
	};
    
BraceDelta CountBraceDelta(wxStyledTextCtrl* stc, int line,  bool& inBlockComment){ // for smart {} indenting
    int start = stc->PositionFromLine(line);
    int end   = stc->GetLineEndPosition(line);
    bool inString = false;
    char stringChar = 0;
    BraceDelta deltas;
    for (int i = start; i < end; ++i){
        char ch = stc->GetCharAt(i); 
        char next = (i + 1 < end) ? stc->GetCharAt(i + 1) : 0;
        // Handle block comments
        if (inBlockComment){
            if (ch == '*' && next == '/'){
                inBlockComment = false;
                ++i;
            	}
            continue;
        	}
        if (!inString && ch == '/' && next == '*'){
            inBlockComment = true;
            ++i;
            continue;
        	}
        // Stop at single-line comment
        if (!inString && ch == '/' && stc->GetCharAt(i + 1) == '/') break;
        if ((ch == '"' || ch == '\'')) { // String handling
            if (!inString){
                inString = true;
                stringChar = ch;
            	}
            else if (ch == stringChar)inString = false;
			continue;
 	       }
        if (inString) continue;
        if (ch == '{') deltas.curly++;
        else if (ch == '}') deltas.curly--;
        else if (ch == '(') deltas.round++;
        else if (ch == ')') deltas.round--;
    	}
    return deltas;
	}

void Console::FormatIndentation(){	// reflow entire script
    auto* stc = m_Script;
    stc->BeginUndoAction();
    int indentWidth = stc->GetIndent();
    int lines = stc->GetLineCount();
    std::vector<int> lineIndent(lines, 0);
    bool inBlockComment = false;
    int indent = 0;
    int minIndent = 0;
    BraceDelta deltasOverall;

    // First pass — compute raw indents (can go negative)
    for (int line = 0; line < lines; ++line){
        lineIndent[line] = indent;
        minIndent = std::min(minIndent, indent);

        BraceDelta deltas = CountBraceDelta(stc, line, inBlockComment);
        indent += (deltas.curly + deltas.round) * indentWidth;
        deltasOverall.curly += deltas.curly;
        deltasOverall.round += deltas.round;       
    	}
    	
    // report?
    if (deltasOverall.curly > 0) message(STYLE_RED, wxString::Format("Reformat found %d more opening curly brace(s) than closing", deltasOverall.curly));
    else if (deltasOverall.curly < 0) message(STYLE_RED, wxString::Format("Reformat found %d more closing curly brace(s) than opening", -deltasOverall.curly));
    if (deltasOverall.round > 0) message(STYLE_RED, wxString::Format("Reformat found %d more opening round bracket(s) than closing", deltasOverall.round));
    else if (deltasOverall.round < 0) message(STYLE_RED, wxString::Format("Reformat found %d more closing round bracket(s) than opening", -deltasOverall.round));
    if ((deltasOverall.curly != 0) && (deltasOverall.round != 0) && (deltasOverall.curly * deltasOverall.round < 0))
    	message(STYLE_RED, "Reformat found likely confusion between curly braces and round brackets");

    // Second pass — apply global offset
    int offset = -minIndent;
    for (int line = 0; line < lines; ++line){
        int finalIndent = lineIndent[line] + offset;
        if (finalIndent < 0) finalIndent = 0;
        stc->SetLineIndentation(line, finalIndent);
    	}
    stc->EndUndoAction();
	}
    
void Console::OnConsoleFormat(wxCommandEvent&){
    FormatIndentation();
	}
		
void Console::OnScriptCharAdded(wxStyledTextEvent& e){
    auto* stc = m_Script;
    int charCode = e.GetKey();
    if (charCode == '\n'){
        int line = stc->GetCurrentLine();
        if (line == 0) return;
        int indent = stc->GetLineIndentation(line - 1);
        bool inBlockComment = false;
        BraceDelta deltas = CountBraceDelta(stc, line - 1, inBlockComment);
        indent += (deltas.curly + deltas.round) * stc->GetIndent();
        if (indent < 0) indent = 0;
        stc->SetLineIndentation(line, indent);
        stc->GotoPos(stc->GetLineIndentPosition(line));
        }
	}
	
// End of smart indenting & re-flowing
