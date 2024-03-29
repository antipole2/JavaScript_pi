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
#include <wx/wxprec.h>
#include "duktape.h"
#include "JavaScriptgui_impl.h"
#include <wx/msgdlg.h>
#include <wx/url.h>
#include <wx/clipbrd.h>
#include "trace.h"
#include "algorithm"

extern JavaScript_pi *pJavaScript_pi;

bool isURLfileString(wxString fileString){
	if ((fileString.substr(0, 6) == "https:") /* || (fileString.substr(0, 5) == "http:") */) return true;	// presently recognising https only
	else return false;
	}

void clearMessageCntlEntries(std::vector<streamMessageCntl>* pv, STREAM_MESSAGE_TYPES which ){
	// remove all entries in vector of type which
	if (pv->empty()) return;
/*	this is the best solution and requires C++17 not yet available on some platforms, so we have to do it the hard way
	pv->erase(
		std::remove_if(pv->begin(),
			 pv->end(),
			 [which](streamMessageCntl x) {return(x.messageType == which);}),
			 pv->end()
		);
*/		 
	//  hard way - reverse down vector as we remove elements
	for (int i = pv->size() - 1; i >= 0; i-- ){
			if (pv->at(i).messageType == which) pv->erase(pv->begin()+i);
			}			 
	}

void fatal_error_handler(void *udata, const char *msg) {
     // Provide for handling fatal error while running duktape
     (void) udata;  /* ignored in this case, silence warning */

     /* Note that 'msg' may be NULL. */
     wxMessageBox((msg ? msg : "(no message)"), "Fatal JavaScript error");
     fprintf(stderr, "Causing intentional segfault...\n");
     fflush(stderr);
     *((volatile unsigned int *) 0) = (unsigned int) 0xdeadbeefUL;
     abort();
 }


 wxString JScleanString(wxString given){ // cleans script string of unacceptable characters
     const wxString reverseQuote {L"\u201B"};
     const wxString leftQuote    { "\u201C"};
     const wxString rightQuote   {"\u201D"};
     const wxString leftDQuote   {L"\u201F"};
     
 //  const wxString quote        {"\u0022"};
     const wxString quote        {"\""};
     const wxString accute       {"\u00B4"};
     const wxString rightSquote  {"\u2019"};    // right single quote
     const wxString leftSquote  {"\u2018"};    // left single quote
 //  const wxString apostrophe   {"\u0027"};
     const wxString apostrophe   {"\'"};
     const wxString ordinal      {"\u00BA"};  // masculine ordinal indicator - like degree
     const wxString degree       {"\u00B0"};
     const wxString superScript0 {"\u2070"};	// superscript 0 pretending to be degrees  
     const wxString degreeText   {"&#xb0"};
     const wxString backprime    {"\u0060"};
//     const wxString bell		{"\u0007"};		// Bell used for internal representation of º
 #ifndef __WXMSW__   // Don't try this one on Windows
     const wxString prime        {"\u2032"};
     given.Replace(prime, apostrophe, true);
 #endif  // __WXMSW__
 #ifdef __WINDOWS__
	 const wxString o_umlaut	{"\u00F6"};		// ö
	 const wxString O_umlaut	{"\u00D6"};		// ö
	 const wxString u_umlaut	{"\u00FC"};	// ü
	 const wxString U_umlaut	{"\u00DC"};	// ü
	 given.Replace(o_umlaut, "oe", true);
	 given.Replace(O_umlaut, "OE", true);
	 given.Replace(u_umlaut, "ue", true);
	 given.Replace(U_umlaut, "UE", true);
 #endif
 
     given.Replace(leftQuote, quote, true);
     given.Replace(reverseQuote, apostrophe, true);
     given.Replace(leftDQuote, quote, true);
     given.Replace(rightQuote, quote, true);
     given.Replace(accute, apostrophe, true);
     given.Replace(rightSquote, apostrophe, true);
     given.Replace(leftSquote, apostrophe, true);
     given.Replace(ordinal, DEGREE, true);
#ifndef	__WINDOWS__
	 given.Replace(superScript0, DEGREE, true);
#endif
	 given.Replace(DEGREE, PSEUDODEGREE, true);	// use substitute character
//   given.Replace(degree, degreeText, true);
     given.Replace(backprime, apostrophe, true);
     given.Replace("\r\n", "\n", true);	// drop CR inserted by Windows for new line  v3
     return (given);
     }

 // This function only needed with Windose

 wxString JScleanOutput(wxString given){ // clean unacceptable characters in output
#ifdef __WINDOWS__
     // As far as we know this only occurs with º symbol on Windows
     const wxString A_stringDeg{ "\u00C2\u00b0"};    // Âº
     const wxString A_stringOrd{ "\u00C2\u00ba"};    // Â ordinal
     given.Replace(A_stringDeg, "\u00b0", true);
#endif     
     given.Replace(PSEUDODEGREE, DEGREE);  
     return (given);
     }


/*
wxString getStringFromDuk(duk_context *ctx){
     // gets a string safely from top of duk stack and fixes º-symbol for Windose
     wxString string = wxString(duk_to_string(ctx, -1));
 #ifdef __WXMSW__
     const wxString A_stringDeg{ "\u00C2\u00b0"};    // Âº
     string.Replace(A_stringDeg, "\u00b0", true);
 #endif
     return string;
     }
*/

wxString ptrToString(Console* address){
    // format pointer to string
    if (address == nullptr) return "nullptr";
    return wxString::Format("%#012x", address);
}

Console* pConsoleBeingTimed {nullptr};  // no other way of finding which console - only one at a time?

duk_bool_t JSduk_timeout_check(void *udata) {
    (void) udata;  /* not needed */
    
    	TRACE(50, wxString::Format( "exec timeout check %ld: now=%lld, allocate=%ld, end=%lld, left=%lld\nstatus=%s\n",
        (long) pConsoleBeingTimed->m_timeout_check_counter, wxGetUTCTimeMillis(), pConsoleBeingTimed->m_time_to_allocate,
        
        pConsoleBeingTimed->m_pcall_endtime, pConsoleBeingTimed->m_pcall_endtime-wxGetUTCTimeMillis(), pConsoleBeingTimed->consoleDump()));

    if (pConsoleBeingTimed == nullptr) return 0; // nothing being timed
	if (pConsoleBeingTimed->mStatus.test(TIMEOUT)) return 1;	// already backing out    
    if (pConsoleBeingTimed->m_pcall_endtime == 0) return(0); // we are not timing at this moment
    pConsoleBeingTimed->m_timeout_check_counter++;
    if (wxGetUTCTimeMillis() > pConsoleBeingTimed->m_pcall_endtime) {
		if (!pConsoleBeingTimed->mStatus.test(TIMEOUT)){
            pConsoleBeingTimed->mStatus.set(TIMEOUT);
            pConsoleBeingTimed->m_result = wxEmptyString;    // supress result
            }
        return 1;
    }
    return 0;
}

wxPoint checkPointOnScreen(wxPoint point){ // fix a point to actually be on the screen
	// NB This works in Logical Pixels, not DIP    
    if ((point.x < 0 || point.x > pJavaScript_pi->m_display_width)) point.x = wxDefaultPosition.x;
    if ((point.y < 0 || point.y > pJavaScript_pi->m_display_height)) point.y = wxDefaultPosition.y;
    return point;
    }

Console* findConsoleByCtx(duk_context *ctx){
    // given a duktape context, return pointer to the console
    void throwErrorByCtx(duk_context *ctx, wxString message);
    Console* pConsole;
    
    for (pConsole = pJavaScript_pi->mpFirstConsole; pConsole != nullptr; pConsole = pConsole->mpNextConsole){
        if (pConsole->mpCtx == ctx) return pConsole;
        }
    // failed to match - emit an error message
    wxMessageBox( wxT("findConsoleByCtx failed to match console"), wxT("JavaScript_pi program error"), wxICON_ERROR);
    // or maybe we will return the first console anyway
    return pJavaScript_pi->mpFirstConsole;	//to avoid crash
}

void throwErrorByCtx(duk_context *ctx, wxString message){ // given ctx, throw error message
    Console *pConsole = findConsoleByCtx(ctx);
    pConsole->prep_for_throw(ctx, message);
    duk_throw(ctx);
    }

#include "wx/tokenzr.h"
jsFunctionNameString_t extractFunctionName(duk_context *ctx, duk_idx_t idx){
    // extract function name from call on JS stack
    // This does not work if in method in class not substantiated, so is here    
    wxStringTokenizer tokens( wxString(duk_to_string(ctx, idx)), " (");
    if (tokens.GetNextToken() != "function") {
        throwErrorByCtx(ctx, "on.. error: must supply function name");
        }
    return (tokens.GetNextToken());
    }

#if TRACE_YES
#if TRACE_TO_WINDOW 
//wxWindow *JStraceWindow;
//wxTextCtrl *JStraceTextCtrl {nullptr};
void windowTrace(int level, wxString text){
    // implements tracing to a separate window
    if (pJavaScript_pi->mpTraceWindow == nullptr){
        // the first time to trace
#if SCREEN_RESOLUTION_AVAILABLE
        wxPoint position = pJavaScript_pi->m_parent_window->FromDIP(wxPoint(800, 100));
        wxSize size = pJavaScript_pi->m_parent_window->FromDIP(wxSize(700, 500));
#else
        wxPoint position = wxPoint(800, 100);
        wxSize size = wxSize(700, 500);
#endif
        pJavaScript_pi->mpTraceWindow = new wxDialog(pJavaScript_pi->m_parent_window, wxID_ANY,"JavaScript plugin trace", position, size,
         	wxDEFAULT_FRAME_STYLE | wxSTAY_ON_TOP|wxRESIZE_BORDER);
        pJavaScript_pi->mpTraceTextCtrl = new wxTextCtrl( pJavaScript_pi->mpTraceWindow, wxID_NEW,
                              wxEmptyString, wxDefaultPosition, wxSize(240, 100),
                              wxTE_MULTILINE);
        }
    try{ 	// Use try in case window has been closed
	    pJavaScript_pi->mpTraceTextCtrl->AppendText(text + "\n");
	    pJavaScript_pi->mpTraceWindow->Raise();
	     pJavaScript_pi->mpTraceWindow->Show();
	    }
	catch(int i) {};
    return;
    }
#endif  // TRACE_TO_WINDOW
#endif	// TRACE_YES

wxString resolveFileName(wxString inputName, Console* pConsole, int mode){
	// if inputName is URL does not change anything
	// if inputName starts "??" uses as prompt for dialogue according to options
    // else if inputName is relative, makes it absolute by the current directory
    // if inputName starts with single "?" and there is an error,  a prompt dialogue is used els the ? is dropped
    // if it is badly formed or other error returns error message else empty string
    // checks existence according to mode
    if ((mode < 0) || (mode > 4)) {
    		pConsole->prep_for_throw(pConsole->mpCtx, "resolveFileNaame invalid mode");
    		duk_throw(pConsole->mpCtx);
    		}
	if ((inputName.substr(0, 6) == "https:")|| (inputName.substr(0, 5) == "http:")){ 	// If URL, don't do anything
		return(inputName);
		}
	int toPrompt {0};	// whether to prompt 0 never; -1 always; +1 if file not valid/found	
	wxString prompt;	// string to use for any prompt
	bool fileExists;
	if (inputName.substr(0, 2) == "??") {	// must prompt
		inputName = inputName.substr(2);	// drop '??'
		TRACE(101, "?? found - inputName now '" + inputName + "'");
		toPrompt = -1;
		prompt = pConsole->mConsoleName + " " + inputName;
		}
	else if (inputName.substr(0, 1) == "?") { // prompt if file missing
		inputName = inputName.substr(1);	// drop '?'
		TRACE(101, "? found - inputName now '" + inputName + "'");
		toPrompt = 1;
		prompt = pConsole->mConsoleName + " " + inputName + " not found - choose alternative";
		}
	wxFileName filePath = wxFileName(inputName);
	std::vector<wxFile::OpenMode> openModes {wxFile::read, wxFile::write, wxFile::read_write, wxFile::write_append, wxFile::write_excl};
	wxFile::OpenMode wxMode = openModes[mode];
	TRACE(101, wxString::Format("resolveFileName mode %i toPrompt %i prompt %s", mode, toPrompt, prompt));
	if (toPrompt != -1){	// try and locate file
	    if (!filePath.IsAbsolute()){ // relative  - so will prepend current working directory and do again
        filePath.MakeAbsolute(pJavaScript_pi->mCurrentDirectory);
	        }
	    }
    if ((toPrompt == 0) && !filePath.IsOk()){
    	pConsole->prep_for_throw(pConsole->mpCtx, "File path " + filePath.GetFullPath() + " does not make sense");
    	duk_throw(pConsole->mpCtx);
    	}
    wxString fullPath = filePath.GetFullPath();
    TRACE(101, wxString::Format("resolveFileName fullPath:%s",fullPath));
    fileExists = filePath.FileExists();
    prompt += " ";	// this to silence compiler warning
    if (
    	(toPrompt == -1)																// always prompt
    	|| (!fileExists && (toPrompt == 1))												// Prompt if no file
    	|| (fileExists && (toPrompt == 1) && (wxMode == wxFile::wxFile::write_excl))	// prompt if exits when it should not
    	)
    	{	// need dialogue
    	TRACE(101, wxString::Format("resolveFileName prompting mode %i toPrompt %i prompt %s", mode, toPrompt, prompt));
		auto style = wxDEFAULT_DIALOG_STYLE;
//		if ((wxMode == wxFile::write) || (wxMode == wxFile::write_append))	// need save dialogue
		if (mode == 4)	style |= wxFD_SAVE | wxFD_OVERWRITE_PROMPT; // need save dialogue			
		wxFileDialog dialog(pConsole, prompt, pJavaScript_pi->mCurrentDirectory, wxEmptyString, wxFileSelectorDefaultWildcardStr, style);
		if (dialog.ShowModal() == wxID_CANCEL){
			pConsole->prep_for_throw(pConsole->mpCtx, "Open dialogue cancelled");
			duk_throw(pConsole->mpCtx);
			}
		return dialog.GetPath();
    	}
    // file exists and we did not use dialogue - check access mode
    if (wxMode ==  wxFile::write_excl){
    	pConsole->prep_for_throw(pConsole->mpCtx, "File " + fullPath + " already exists");
    	duk_throw(pConsole->mpCtx);
    	}
    if (((wxMode == wxFile::write) || (wxMode == wxFile::read_write)) && !filePath.IsFileWritable ()){
    	pConsole->prep_for_throw(pConsole->mpCtx, "File " + fullPath + " no write access");
    	duk_throw(pConsole->mpCtx);
    	}
    return fullPath;
    };
    
wxString NMEAchecksum(wxString sentence){
	// given an NMEA sentence, returns the checksum as a string
	// props to Dirk Smits for the checksum calculation lifted from his NMEAConverter plugin
    // we will drop any existing checksum
    if (sentence.length() < 7) return wxEmptyString;	// cannot be sentence - avoid crash ahead
    int starPos = sentence.Find("*");
    if (starPos != wxNOT_FOUND) // yes there is one
	    sentence = sentence.SubString(0, starPos-1); // truncate at * onwards
    unsigned char calculated_checksum = 0;
	for(wxString::const_iterator i = sentence.begin()+1; i != sentence.end() && *i != '*'; ++i)
		 calculated_checksum ^= static_cast<unsigned char> (*i);
	return( wxString::Format("%02X", calculated_checksum) );
	};
	
wxString getClipboardString(Console* pConsole){  // return string from clipboard
	if (!wxTheClipboard->Open())return "getClipboardString clipboard is busy - logic error?";
	wxTextDataObject dataObject;
	wxTheClipboard->GetData(dataObject);
	wxTheClipboard->Close();
	wxString data = dataObject.GetText();
	data =  JScleanString(data);
	return data;
	}

wxString getTextFile(wxString fileString, wxString* pText){
    // gets contents of a text file
    // if error, returns message, else empty string
    wxFileName filePath;
    wxString tmp_file_name = wxEmptyString;
    wxString fileStringCopy = fileString;	// for later error messages
    if (isURLfileString(fileString)){
		TRACE(64, "Trying for URL: " + fileString);
		// its a URL - let's try and get it
		if (!OCPN_isOnline()) return("readTextFile " + filePath.GetFullPath() + " failed - not on-line");
		wxURI url(fileString);
		wxString tmp_file_name = wxFileName::CreateTempFileName("");
		_OCPN_DLStatus ret = OCPN_downloadFile(url.BuildURI(), tmp_file_name,
		"", "", wxNullBitmap, pJavaScript_pi->m_parent_window,
		OCPN_DLDS_ELAPSED_TIME | OCPN_DLDS_ESTIMATED_TIME | OCPN_DLDS_REMAINING_TIME | OCPN_DLDS_SPEED | OCPN_DLDS_SIZE | OCPN_DLDS_CAN_PAUSE | OCPN_DLDS_CAN_ABORT | OCPN_DLDS_AUTO_CLOSE,
		10);
		if (ret == OCPN_DL_ABORTED) return( "readTextFile " + filePath.GetFullPath() + " download aborted");
		if (ret == OCPN_DL_FAILED) return("readTextFile " + filePath.GetFullPath() + " failed - invalid URL?");
		fileString = tmp_file_name;
		}
    wxFile inputFile;
    if (!inputFile.Exists(fileString)) return("readTextFile " + fileStringCopy + " not found");
    bool ok = inputFile.Open(fileString);
    if (!ok) return(wxString::Format("readTextFile %s cannot be opened - error code %d", fileStringCopy, inputFile.GetLastError()));
    // the file can be big, so we will allocated buffer space
    wxFileOffset length = inputFile.Length();
#if TRACE_YES
    wxString message = wxString::Format("geTextFile file %s is length %d", fileString, length);
    TRACE(64, message);
#endif
	wxStringBuffer(*pText, length+10);
	ok = inputFile.ReadAll(pText);
	if (tmp_file_name != wxEmptyString) wxRemoveFile(tmp_file_name);	// if created one
    if (!ok) return(wxString::Format("readTextFile %s cannot read file - error code %d", fileStringCopy, inputFile.GetLastError()));
    if (*pText == wxEmptyString) return("readTextFile " + fileStringCopy + " is empty");
    return wxEmptyString;
    }

Console* findConsoleByName(wxString name){
    // find console by console name - return nullptr if none
    Console* pConsole;
    pConsole = pJavaScript_pi->mpFirstConsole;
    for (pConsole = pJavaScript_pi->mpFirstConsole; pConsole; pConsole = pConsole->mpNextConsole)
    if (pConsole->mConsoleName.IsSameAs(name, false)) return pConsole;
    return nullptr;
    }

wxString statusesToString(status_t mStatus){
    // returns the statuses in status as string
    // statuses defined in JavaScript_pi.h
    wxString result =
       (
        (mStatus.test(HAD_ERROR)?_("HAD_ERROR "):_("")) +
        (mStatus.test(TIMEOUT)?_("TIMEOUT "):_("")) +
        (mStatus.test(DONE)?_("DONE "):_("")) +
        (mStatus.test(INEXIT)?_("INEXIT "):_("")) +        
        (mStatus.test(STOPPED)?_("STOPPED "):_("")) +
        (mStatus.test(MORETODO)?_("MORETODO "):_("")) +
        (mStatus.test(INMAIN)?_("INMAIN "):_("")) +
        (mStatus.test(TOCHAIN)?_("TOCHAIN "):_("")) +
        (mStatus.test(CLOSED)?_("CLOSED "):_("")) +
        (mStatus.test(CLOSING)?_("CLOSING "):_(""))
        );
    if (result == wxEmptyString) result = "(no status)";
    return result;
    }

wxString checkConsoleName(wxString newName, Console* pConsole){
    // validate proposed name - alphanumeric  or '_' and not more than 14 chars
    // checks if name already taken but does not check pConsole to allow keeping same name
    // returns error message or empty string if OK
    if ((newName.Length() > 14) || (newName.Length() < 1)) return("New console name must be 1-14 chars");
    wxString::const_iterator i;
    for (i = newName.begin(); i != newName.end(); ++i) {
        wxUniChar code = *i;
        if (((code  < 48) || (code >  122) || ((code > 57) && (code <65)) || ((code >90) && (code < 97)))
            && (code != 95))
        return("consoleName new name must only be alphanumeric or _");
    }
    //check for existing console with this name (ignoring case)
    for (Console* pCon = pJavaScript_pi->mpFirstConsole; pCon != nullptr; pCon = pCon->mpNextConsole){
        if (pCon == pConsole) continue;    // don't check this console to allow repeated renaming
        if (newName.IsSameAs(pCon->mConsoleName, false)) return("New console name " + newName + " already taken by another console");
        }
    return(wxEmptyString);
    }
    
void reviewParking(){	// adjust parking space sizes and remove any gaps (console deleted)
	struct lot{	// a parking lot
		Console* pConsole;
		location place;
		};
	std::vector<lot> lots;
	lot aLot;	// working space
	// build array of places and sort
	for (Console* pCon = pJavaScript_pi->mpFirstConsole; pCon != nullptr; pCon = pCon->mpNextConsole){
		if (pCon->m_parkedLocation.set){	// this one has a lot
			aLot.pConsole = pCon;
			aLot.place = pCon->m_parkedLocation;
			lots.push_back(aLot);			
			}
		}
	if (lots.size() < 1) return;
	std::sort(lots.begin(),lots.end(), [](lot &a, lot &b){ return a.place.position.x < b.place.position.x; });
	
	// now adjust location of each lot, shuffling left it needed
	int xPos = pJavaScript_pi->m_parkFirstX;
	for (int i = 0; i < lots.size(); i++){
		aLot = lots[i];
		aLot.pConsole->m_parkedLocation.position.x = xPos;
		xPos += aLot.pConsole->m_parkedLocation.size.x + pJavaScript_pi->m_parkSep;	// next place
		if (aLot.pConsole->m_parked) aLot.pConsole->setLocation(aLot.pConsole->m_parkedLocation);
		}
	}
    
#include "wx/regex.h"
// The following could be Duktape release dependent
wxRegEx parse(" *at ([^ ]*).*:([0-9]*)"); // parses function and line number
wxString formErrorMessage(duk_context *ctx){
    // form error message from error object
    wxString message, part;
    duk_dup_top(ctx);
    wxString error = duk_safe_to_string(ctx, 1);    // the error itself
    duk_pop(ctx);
    if (duk_is_error(ctx, -1)){  // can only do more if error object
        wxString line, func, lineNumber;
        message = error;    // just for now
        duk_get_prop_string(ctx, -1, "stack");
        wxString stack = wxString(duk_safe_to_string(ctx, -1));
        TRACE(4, "Call stack: " + stack);
        wxStringTokenizer tokenizer(stack, "\n");
        line = tokenizer.GetNextToken();   //skip line 1
        line = tokenizer.GetNextToken();   //skip line 2
        line = tokenizer.GetNextToken();   //skip line 3
        line = tokenizer.GetNextToken();   //the line throwing error
        if (parse.Matches(line)){   // identfy the place
            TRACE(4, "wholeLine:" + parse.GetMatch(line, 0));
            func = parse.GetMatch(line, 1);
            lineNumber = parse.GetMatch(line, 2);
            func = (func == "eval")?"":(func+" "); // drop eval as function name
            message = func + "line " + lineNumber + " " + error;
            }
        while ( tokenizer.HasMoreTokens()){ // prepare tracebck if any
            line = tokenizer.GetNextToken();
            if (parse.Matches(line)){
                func = parse.GetMatch(line, 1);
                lineNumber = parse.GetMatch(line, 2);
                func = (func == "eval")?"":(" " + func); // drop eval as function name
                message += "\ncalled from" + func + " line " + lineNumber;
                }
            }
        }
    else message = error;   // only the error itself available
    duk_pop(ctx);   // the error text/object
    return message;
}
    
//#if DUKDUMP
wxString dukdump_to_string(duk_context* ctx){
    duk_push_context_dump(ctx);
    wxString result = "Duktape context dump:\n"+wxString(duk_to_string(ctx, -1))+"\n";
    duk_pop(ctx);
    return result;
    }
//#endif // DUKDUMP
