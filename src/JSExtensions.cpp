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

#include "duktape.h"
#include "JavaScript_pi.h"
#include "JavaScriptgui_impl.h"
#include "ocpn_duk.h"
#include <string>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/event.h>
#include <wx/textfile.h>
#include <wx/file.h>
#include <wx/clipbrd.h>
#include <wx/dataobj.h>
#include <wx/arrstr.h>
#include <wx/utils.h>
#include "jsDialog.h"

/*  On hold for now - cannot find a way of handling the variable arguments
 // sprintf function
 static duk_ret_t duk_sprintf(duk_context *ctx) {
 const char *duk_push_string(duk_context *ctx, const char *str);
 const char *duk_to_string(duk_context *ctx, duk_idx_t idx);
 const char *duk_push_sprintf(duk_context *ctx, const char *fmt, ...);
 duk_int_t duk_get_type(duk_context *ctx, duk_idx_t idx);
 duk_idx_t nargs;  // number of args in call
 
 nargs = duk_get_top(ctx);
 cout << "  sprintf - nargs=" << nargs <<"\n";
 cout << "arg0 is type:" << duk_get_type(ctx, 0) << " and value:" << duk_to_string(ctx,0) << "\n";
 cout << "arg1 is type:" << duk_get_type(ctx, 1) << " and value:" << duk_to_string(ctx,1) << "\n";
 cout << "sprintf gives:" << duk_push_sprintf(ctx, duk_to_string(ctx,0), duk_to_string(ctx,1)) << "\n";
 
 return (1);
 //    return (*duk_push_sprintf(ctx, "meaning of life: %d, name", 42));
 }
 */

extern JavaScript_pi *pJavaScript_pi;
Console* findConsoleByCtx(duk_context *ctx);
Console* findConsoleByName(wxString name);
void throwErrorByCtx(duk_context *ctx, wxString message);
wxString resolveFileName(wxString inputName, Console* pConsole, int options);
wxString getTextFile(wxString fileString, wxString* text, int timeOut);
wxString JScleanOutput(wxString given);

void limitOutput(wxStyledTextCtrl* pText){
	// given output text area, ensure does not exceed size limit and scroll to end
	int maxLength = 100000;	// max chars allowed
	int textLength = pText->GetTextLength();
	if (textLength > maxLength) {
		wxString deleted = "<Text has been deleted>\n";
		pText->DeleteRange(0, textLength - maxLength);
		// to avoid leaving a part tine, we will look for a new line near top
		int newlinePos = pText->FindText(0, 100, wxString("\n"));
		if (newlinePos != wxSTC_INVALID_POSITION) pText->DeleteRange(0, newlinePos+1);
		// now we will insert a coloured note at the start
		pText->InsertText(0, deleted);
#if (wxVERSION_NUMBER < 3100)
        pText->StartStyling(0,255);   // 2nd parameter included Linux still using wxWidgets v3.0.2
#else
		pText->StartStyling(0);
#endif
		pText->SetStyling(deleted.Length(), STYLE_BLUE);
		}
	pText->ScrollToEnd();
	}
    
wxString js_formOutput(duk_context *ctx){
    duk_idx_t nargs;  // number of args in call
    wxString output = wxEmptyString;
    
    nargs = duk_get_top(ctx);
//   MAYBE_DUK_DUMP
    for(int i = 0; i<nargs; i++){
        switch (duk_int_t type = duk_get_type(ctx, i)){
            case DUK_TYPE_STRING:
            case DUK_TYPE_NUMBER:
                output += duk_to_string(ctx,i);  // mangles º character on Windows!
                break;
            case DUK_TYPE_BOOLEAN:
                output += ((duk_to_boolean(ctx,i) == 1) ? "true":"false");
                break;
            case DUK_TYPE_OBJECT:
                duk_json_encode(ctx, i);
                output += duk_to_string(ctx,i);
                break;
            case DUK_TYPE_NULL:
				break;
            case DUK_TYPE_UNDEFINED:
                findConsoleByCtx(ctx)->prep_for_throw(ctx, "output - arg " + to_string(i) + " is undefined");
                duk_throw(ctx);
            default:
                findConsoleByCtx(ctx)->prep_for_throw(ctx, "output - arg " + to_string(i) + " of unexpected type " + to_string(type));
                duk_throw(ctx);
	        }
    	}
	output = JScleanOutput(output);	// internally, we are using PSEUDODEGREE to represent degree - convert any back
    return(output);
}

void appendStyledText(wxString text, wxStyledTextCtrl* window, int colour){
	// Append coloured text to window
	int long beforeLength, afterLength;
	beforeLength = window->GetTextLength(); // where we are before adding text
	window->AppendText(text);
	afterLength = window->GetTextLength(); // where we are after adding text
#if (wxVERSION_NUMBER < 3100)
    window->StartStyling((int)beforeLength,255);   // 2nd parameter included Linux still using wxWidgets v3.0.2
#else
    window->StartStyling((int)beforeLength);
#endif
	window->SetStyling((int)(afterLength-beforeLength), colour);
	TRACE(24, wxString::Format("Styled text '%s' colour %i  start %li  end %li  length %i", text, colour,
		beforeLength, afterLength, (int)(afterLength-beforeLength)));
	}

duk_ret_t print_coloured(duk_context *ctx, int colour) {   // print arguments on stack in colour
    wxStyledTextCtrl* output_window;
    Console *pConsole = findConsoleByCtx(ctx);
    
    pConsole->Show(); // make sure console is visible
	pConsole->makeBigEnough();
    output_window = pConsole->m_Output;
    appendStyledText(js_formOutput(ctx),output_window, colour);
    limitOutput(output_window);
    return(0);
}

void onDismissAlert(wxCommandEvent & event){
    // here when alert to be dismissed
    jsButton *button = wxDynamicCast(event.GetEventObject(), jsButton);
    Console *pConsole = button->pConsole;
    TRACE(3,pConsole->mConsoleName + "alert dismissed ");
    pConsole->clearAlert();
    if (!pConsole->isWaiting()) {
        pConsole->wrapUp(DONE);
        }
    }

static duk_ret_t duk_alert(duk_context *ctx) {   // create or add to an alert  box with dismiss button
    duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
    Console *pConsole = findConsoleByCtx(ctx);
    JsDialog *alert = pConsole->mAlert.palert;    // any existing alert
    extern JavaScript_pi* pJavaScript_pi;
    
    if (nargs == 0){    // just return present state
        duk_push_boolean(ctx, (alert != nullptr) ? true : false);
        return 1;
        }
    // if called with single argument of false, is attempt to cancel any open alert
    // cancel any existing alert and return true if there was one
    if (nargs == 1 && duk_is_boolean(ctx, -1) && !duk_get_boolean(ctx, -1)){
        duk_pop(ctx);   // the argument
        if (alert != nullptr){
            // there is an alert displayed
            pConsole->clearAlert();
            duk_push_boolean(ctx, true);
            }
        else duk_push_boolean(ctx, false);
        return 1;
        }
    // we have alert text to display
    if (pConsole->mAlert.alertText.length() > 0) pConsole->mAlert.alertText += "\n";// Insert newline after any existing text
    pConsole->mAlert.alertText += js_formOutput(ctx); // add text to any existing
    if (alert != nullptr){  // clear any existing alert being displayed
        pConsole->mAlert.palert->Close();
        pConsole->mAlert.palert->Destroy();
        }
    alert = new JsDialog(pJavaScript_pi->m_parent_window,  wxID_ANY, pConsole->mConsoleName, pConsole->FromDIP(pConsole->mAlert.position),
    	wxDefaultSize, wxCAPTION | wxSTAY_ON_TOP | wxBORDER_RAISED);
    alert->SetBackgroundColour(*wxYELLOW);
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);  // A top-level sizer
    alert->SetSizer(boxSizer);
    wxStaticText* staticText = new wxStaticText( alert, wxID_STATIC, pConsole->mAlert.alertText, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
    boxSizer->Add(staticText, 0, wxALIGN_LEFT|wxALL, 5);
    wxBoxSizer* buttonBox = new wxBoxSizer(wxHORIZONTAL);
    boxSizer->Add(buttonBox, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    jsButton* button = new jsButton ( pConsole, alert, wxNewId(), _("Dismiss"), wxDefaultPosition, wxDefaultSize, 0 );
    buttonBox->Add(button, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    alert->Bind(wxEVT_BUTTON, &onDismissAlert, wxID_ANY);
    alert->Fit();
    alert->Show(true);
    pConsole->mAlert.palert = alert;  // remember
    pConsole->mWaitingCached = pConsole->mWaiting = true;
    duk_pop_n(ctx, nargs);  // pop all the arguments
    duk_push_boolean(ctx, true);
    return 1;
    }

static duk_ret_t duk_result(duk_context *ctx) {   // result
    Console *pConsole = findConsoleByCtx(ctx);
    pConsole->m_explicitResult = true;
    if (duk_get_top(ctx) > 0) {// called with args
        pConsole->m_result = js_formOutput(ctx);
        pConsole->m_explicitResult = true;
        duk_pop(ctx);
        }
    duk_push_string(ctx, pConsole->m_result);
    return (1); // return present result
    }

static duk_ret_t duk_print(duk_context *ctx) {   // print
    duk_ret_t result = 0;
    Console *pConsole = findConsoleByCtx(ctx);
    pConsole->Show(); // make sure console is visible
    pConsole->makeBigEnough();
    pConsole->m_Output->AppendText(js_formOutput(ctx));
    limitOutput(pConsole->m_Output);
    return (result);
    }

static duk_ret_t duk_print_red(duk_context *ctx) {   // print in red
    return (print_coloured(ctx, STYLE_RED));
    }

static duk_ret_t duk_print_orange(duk_context *ctx) {   // print in orange
    return (print_coloured(ctx, STYLE_ORANGE));
    }

static duk_ret_t duk_print_green(duk_context *ctx) {   // print in green
    return (print_coloured(ctx, STYLE_GREEN));
    }

static duk_ret_t duk_print_blue(duk_context *ctx) {   // print in blue
    return (print_coloured(ctx, STYLE_BLUE));
    }

/*	doesn't seem to work
    static duk_ret_t duk_print_bold(duk_context *ctx) {   // print bold
    return (print_coloured(ctx, STYLE_BOLD));
    }
*/

static duk_ret_t duk_print_ul(duk_context *ctx) {   // print underlined
   return (print_coloured(ctx, STYLE_UNDERLINE));
  }

static duk_ret_t duk_log(duk_context *ctx) {   // log message
    duk_ret_t result = 0;
    wxLogMessage(js_formOutput(ctx));
    return (result);
    }
    
void onMessageButton(wxCommandEvent & event)
	{
	jsButton* button = wxDynamicCast(event.GetEventObject(), jsButton);
    Console* pConsole = button->pConsole;
    pConsole->mpMessageDialog->EndModal(button->GetId());
	}
	
void onMove(wxMoveEvent & event){
	TRACE(29, "Move event");
	}
	
static duk_ret_t duk_message(duk_context *ctx) {   // show modal dialogue
    // messageBox(message [, caption, "YesNo"]);
    // NB This feature should be able to use wxMessageBox() and would be very much simpler
    // But wxMessageBox cannot be cancelled (EndModal()) as needed by consoleClose
    // So we implement it with wxDialog and cope with the complexity, including having to have a handler for "YesNo" above
    wxString buttonType;
    int answer;
    Console *pConsole = findConsoleByCtx(ctx);
    wxString ok = "OK";
    wxString yesNo = "YesNo";
    wxString caption = "JavaScript " + pConsole->mConsoleName;
    duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
    if (nargs > 3) {
		pConsole->prep_for_throw(ctx, "messageBox called with invalid number of args");
		duk_throw(ctx);
	}
    wxString message = duk_get_string(ctx, 0);
    message = JScleanOutput(message);	// internally, we are using PSEUDODEGREE to represent degree - convert any back
    if (nargs > 1){
        wxString arg2 = duk_get_string(ctx,1);
        if (arg2 == yesNo) buttonType = arg2;
        else if ((arg2 == ok) || (arg2 == wxEmptyString)) buttonType = ok;
        else {
        	pConsole->prep_for_throw(ctx, "messageBox invalid 2nd arg");
        	duk_throw(ctx);
        	}
        }
    else buttonType = ok;
    if (nargs > 2) caption = duk_get_string(ctx, 2);
    duk_pop_n(ctx, nargs);
    // we attach this dialogue to the canvas window so console not forced to front on dismiss
	wxDialog *dialog = new wxDialog(pJavaScript_pi->m_parent_window, wxID_ANY, caption, wxDefaultPosition, wxDefaultSize,   wxRESIZE_BORDER | wxCAPTION | wxSTAY_ON_TOP);
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);  // A top-level sizer
    dialog->SetSizer(topSizer);
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL); // A second box sizer to give more space around the controls
    topSizer->Add(boxSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxStaticText* staticText = new wxStaticText( dialog, wxID_STATIC, message, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER );
    boxSizer->Add(staticText, 0, wxALIGN_CENTER|wxALL, 5);
    wxBoxSizer* buttonBoxSizer = new  wxBoxSizer(wxHORIZONTAL);
    jsButton* cancel = new jsButton(pConsole, dialog, wxID_CANCEL, "Cancel",wxDefaultPosition);
    buttonBoxSizer->Add(cancel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    if (buttonType == ok){
	    jsButton* OK = new jsButton(pConsole, dialog, wxID_OK, ok, wxDefaultPosition);
	    OK->SetDefault();
		buttonBoxSizer->Add(OK, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
		}
	else if (buttonType == yesNo){
		jsButton* Yes = new jsButton(pConsole, dialog, wxID_YES,"Yes",wxDefaultPosition);
		buttonBoxSizer->Add(Yes, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
		jsButton* No = new jsButton(pConsole, dialog, wxID_NO, "No",wxDefaultPosition);
		buttonBoxSizer->Add(No, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
		}
	else {
		pConsole->prep_for_throw(ctx, "messageBox invalid 2nd arg");
		duk_throw(ctx);
		}
	boxSizer->Add(buttonBoxSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);
	dialog->Bind(wxEVT_BUTTON, &onMessageButton, wxID_ANY);
	dialog->Fit();
	pConsole->mpMessageDialog = dialog;
	answer = dialog->ShowModal();
    switch (answer) {
    	case -1:	// modal was closed by consoleClose()
    		pConsole->mStatus.set(CLOSING); // note special case
    		// now fall through to get out
        case wxID_CANCEL:
            pConsole->prep_for_throw(ctx, "messageBox cancelled");
            duk_throw(ctx);
        case wxID_OK:
            answer = 1;
            break;
        case wxID_YES:
        	answer = 2;
            break;
        case wxID_NO:
            answer = 3;
            break;
        default:
        	pConsole->prep_for_throw(ctx, "messageBox logic error");
        	duk_throw(ctx);
        }
	pConsole->mpMessageDialog->Destroy();
    pConsole->mpMessageDialog = nullptr;
    pConsole->startTimeout();
    duk_push_int(ctx, answer);
    return(1);
    }


#if 0
// On hold for now - variable arguments of variable types defeated me
static duk_ret_t duk_format(duk_context *ctx) {   // format text
    MAYBE_DUK_DUMP
    duk_push_sprintf(<#duk_context *ctx#>, <#const char *fmt, ...#>)
    void *ptr;
    int argCount;
    const char *fmt;
    const char *arg1s, *arg2s, *arg3s;
    int arg1i, arg2i, arg3i;
    double arg1n, arge2n, arg3n;
    bool arge1b, arge2b, arg3b;
    
    cout<< "entered duk_format\n";
    MAYBE_DUK_DUMP
    argCount = duk_get_top(ctx);
    if (argCount > 1){
        
    }
    fmt = duk_get_string(ctx, -3);
    arg1 = duk_get_string(ctx, -2);
    arg2 = duk_get_string(ctx, -1);
    duk_pop_3(ctx);
    MAYBE_DUK_DUMP
    duk_push_sprintf(ctx, fmt, arg1);
    MAYBE_DUK_DUMP
    return (1);
}
#endif

static duk_ret_t duk_read_text_file(duk_context *ctx) {  // read in a text file
    // read in a text file
    extern JavaScript_pi* pJavaScript_pi;
    wxString filePath, fileString, lineOfFile, text, result;
    wxString JScleanString(wxString line);
    Console *pConsole = findConsoleByCtx(ctx);
    
    filePath = duk_to_string(ctx,0);
    duk_pop(ctx);  // finished with that
	filePath = resolveFileName(filePath, pConsole, 0);
	if (filePath == wxEmptyString) {
		pConsole->prep_for_throw(ctx, result);
		duk_throw(ctx);
		}
    result = getTextFile(filePath, &text, 10);
    if (result != wxEmptyString) {
		pConsole->prep_for_throw(ctx, result);
		duk_throw(ctx);
		}
    text = JScleanString(text);
    duk_push_string(ctx, text);
    return 1;
    };

static duk_ret_t duk_write_text_file(duk_context *ctx) {  // write a text file
	// filewriteTextFile(text, fileName, access)
    wxString fileNameGiven,fileString, outcome, text;
    wxFileName filePath;
    Console *pConsole = findConsoleByCtx(ctx);
    
    text = duk_to_string(ctx, 0);
    text = JScleanOutput(text);
    fileNameGiven = duk_to_string(ctx,1);
    int access = (FileOptions)duk_to_int(ctx, 2);
    duk_pop_3(ctx);  // finished with arguments
    if ((access < 0) || (access > 2)) {
		pConsole->prep_for_throw(ctx, wxString::Format("writeTextFile access %i invalid", access));
		duk_throw(ctx);
		}
    std::vector<wxFile::OpenMode> modes {wxFile::write_excl, wxFile::write, wxFile::write_append};	// bespoke list
    wxFile::OpenMode mode = modes[access];
    fileString = resolveFileName(fileNameGiven, pConsole, mode);
    if ((fileNameGiven.substr(0, 1) == "?") && (access = 0)){	// write_excl && "?"
    	// if file already exists and we are here, user must have chosen to overwrite it, so we allow it to overwrite in next bit
    	mode = wxFile::write;	
    	}
//    std::vector<wxFile::OpenMode> openMode {wxFile::read, wxFile::write, wxFile::read_write, wxFile::write_append, wxFile::write_excl};	
    wxFile* theFile = new wxFile();
    bool OK;
    if (access == 0) OK = theFile->Create(fileString, false);	// must not exist
    else if (access == 1) OK = theFile->Create(fileString, true);	// may exist and overwrite
    else OK = theFile->Open(fileString, wxFile::write_append);	// open for append
    if (!OK) {
    	delete theFile;	// delete because throw seems not to clean up
		pConsole->prep_for_throw(ctx, "writeTextFile failed to open file");
		duk_throw(ctx);
		}
    OK = theFile->Write(text);
    if (!OK) {
        delete theFile;
		pConsole->prep_for_throw(ctx, "writeTextFile failed to write text");
		duk_throw(ctx);
		}
    return 0;
    };

duk_ret_t duk_require(duk_context *ctx){ // the module search function
    wxString fileNameGiven, fileString, script, outcome, resolved;
    wxFileName filePath;
//    wxTextFile inputFile;
//    wxFileName homeDirectory;
    wxString JScleanString(wxString line);
    bool loadComponent(duk_context *ctx, wxString name);
    Console *pConsole = findConsoleByCtx(ctx);
    
    if (pConsole->mStatus.test(INEXIT)) {
		pConsole->prep_for_throw(ctx, "require within onExit function");
		duk_throw(ctx);
		}
    fileNameGiven = duk_to_string(ctx,0);
    duk_pop(ctx);  // finished with that
    filePath = wxFileName(fileNameGiven);
    TRACE(45, "Require - fileGivenName: " + fileNameGiven);
    if ((filePath.GetDirCount() == 0) && !filePath.HasExt() && fileNameGiven.substr(0, 1) != "?"){ // simple file name
        if (loadComponent(ctx, fileNameGiven)) {
            // above we will have loaded a C++ component if it matches
            return 1;
            }
        else {  // we will look for it in the built-in scripts
            // this updated for ocpn v5.5
            TRACE(45, "Require - looking for built-in script");
            fileString = GetPluginDataDir("JavaScript_pi");
            filePath.SetPath(fileString);
            filePath.AppendDir("data");
            filePath.AppendDir("scripts");
            filePath.SetFullName(fileNameGiven + ".js");	// .js added Nov 2023
            TRACE(45, "Require - looking to load built-in script: " + filePath.GetFullPath());
            if (!filePath.FileExists()){
                pConsole->prep_for_throw(ctx, "require - " + fileNameGiven + " not in built-in scripts");
                duk_throw(ctx);
                }
            if (!filePath.IsFileReadable()) {
				pConsole->prep_for_throw(ctx, "readTextFile " + filePath.GetFullPath() + " not readable");
				duk_throw(ctx);
				}
            // ready to go
            resolved = filePath.GetFullPath();
            TRACE(45, "Require - found built-in script");
            }
        }
    else{   // not a built-in or library script - we will hunt for it
        TRACE(45, "Require - hunting for script");
        resolved = resolveFileName(fileNameGiven, pConsole, 0);
        }
    TRACE(45, "Require - resolved to: " + resolved);
    outcome = getTextFile(resolved, &script, 10);
    if (outcome != wxEmptyString){
    	pConsole->prep_for_throw(ctx, "require readTextFile error: " + outcome);
		duk_throw(ctx);
    	}
    script = JScleanString(script);
    duk_push_string(ctx, script);
    duk_push_string(ctx, resolved); // the file name for trace back
    if (duk_pcompile(ctx, DUK_COMPILE_FUNCTION)){
        pConsole->prep_for_throw(ctx, duk_safe_to_string(ctx, -1));
        duk_throw(ctx);
        }
    return(1);
    };

void setupSeconds(duk_context *ctx, bool persistant){
	// create a callback entry to call fuction with argument at timeToCall
	// called with onSeconds(function, time [, parameter])
	wxString extractFunctionName(duk_context *ctx, duk_idx_t idx);
	int matched = -1;
    Console* pConsole = findConsoleByCtx(ctx);
	duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
	if (nargs < 2){
		if (nargs == 0) { // empty call - cancel all timers
			pConsole->mpTimersVector.clear();
			pConsole->mTimerActionBusy = false;
			pConsole->mWaitingCached = false;   // force full isWaiting check
			matched = 0;
			}
		else if (nargs == 1){ // cancel specified timer
			duk_require_number(ctx, 0);
			int id = duk_get_int(ctx, 0);
			duk_pop(ctx);
			if (pConsole->mpTimersVector.empty()) return;
			for (auto it = pConsole->mpTimersVector.cbegin(); it != pConsole->mpTimersVector.cend(); ++it){
				auto entry = (*it);
				if (entry.timer->GetId() == id){
					pConsole->mpTimersVector.erase(it);
					matched = 1;
					break;
					}
				}
			}
		duk_push_int(ctx, matched);
		return;
		}
	if (pConsole->mpTimersVector.size() > MAX_TIMERS){
		pConsole->prep_for_throw(ctx, wxString::Format("onSeconds already have maximum %i timers outstanding", MAX_TIMERS));   // safety limit of timers);
		duk_throw(ctx);
		}
	// ready to go
	wxString argument = wxEmptyString;
	if (pConsole->mStatus.test(INEXIT)) {
		pConsole->prep_for_throw(ctx, "onSeconds within onExit function");
		duk_throw(ctx);
		}
	if (!duk_is_function(ctx, 0)) {
		pConsole->prep_for_throw(ctx, "onSeconds first argument must be function");
		duk_throw(ctx);
		}
	if (nargs > 3) {
		pConsole->prep_for_throw(ctx, "onSeconds requires no more than three arguments");
		duk_throw(ctx);
		}
	jsFunctionNameString_t functionToCall = extractFunctionName(ctx, 0);
	int timeInt = (int) (duk_get_number(ctx, 1) * 1000);	//interval in msec
	if (nargs == 3) argument = duk_to_string(ctx, 2);
	duk_pop_n(ctx, nargs);	// finished with call
	bool shots = persistant ? wxTIMER_CONTINUOUS : wxTIMER_ONE_SHOT;
	timerEntry entry;
	entry.functionName = functionToCall;
	entry.parameter = argument;
	entry.timer = make_shared<wxTimer>();
	int id = entry.timer->GetId();
	entry.timer->Bind(wxEVT_TIMER, &Console::HandleTimer, pConsole);
	entry.timer->Start(timeInt, shots);
	pConsole->mpTimersVector.push_back(entry);
	TRACE(66, wxString::Format("setTimedCallback time %i, function %s  argument %s mpTimersVector length %i",
		timeInt, functionToCall, argument, pConsole->mpTimersVector.size())); 
	duk_push_int(ctx, id); 
	}

static duk_ret_t onSeconds(duk_context *ctx) {  // call function after milliseconds elapsed
	setupSeconds(ctx, false);
    return 1;
	};
	
static duk_ret_t onAllSeconds(duk_context *ctx) {  // call function after milliseconds elapsed
	setupSeconds(ctx, true);
    return 1;
	};

static duk_ret_t duk_timeAlloc(duk_context *ctx) {   // time allocation
    Console *pConsole = findConsoleByCtx(ctx);
    wxLongLong timeLeft =  pConsole->m_pcall_endtime - wxGetUTCTimeMillis();
    if (timeLeft < 0)  timeLeft = 0;    // have captured time left
        if (duk_get_top(ctx) > 0 ){
        // was called with argument - us to set new allocatins
            pConsole->m_time_to_allocate = duk_get_int(ctx, -1);
        duk_pop(ctx);
        }
    pConsole->m_pcall_endtime = wxGetUTCTimeMillis() + pConsole->m_time_to_allocate;  // grant more time
    duk_push_number(ctx, timeLeft.GetValue());   // will return time that was left
    return (1);
	}

static duk_ret_t duk_stopScript(duk_context *ctx) {
    duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
    Console *pConsole = findConsoleByCtx(ctx);
    
    if (pConsole->mStatus.test(INEXIT)) {
		pConsole->prep_for_throw(ctx, "stopScript within onExit function");
		duk_throw(ctx);
		}
    if (nargs > 0){
        pConsole->m_result = duk_safe_to_string(ctx, -1);
        pConsole->m_explicitResult = true;
        duk_pop(ctx);
        }
    pConsole->mStatus.set(STOPPED);
    duk_throw(ctx);
    return 0;  //never executed but keeps compiler happy
    }

Console* getConsoleWithOptionalNameOnStack(duk_context *ctx){
    extern JavaScript_pi *pJavaScript_pi;
    duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
    Console* pConsole;
    wxString consoleName;
    
    if (nargs > 0){
        consoleName = duk_get_string(ctx, 0);
        duk_pop(ctx);
        pConsole = findConsoleByName(consoleName);
        if (!pConsole) throwErrorByCtx(ctx, "No console with name " + consoleName);
        }
    else pConsole = findConsoleByCtx(ctx);
    return pConsole;
    }

static duk_ret_t duk_consoleHide(duk_context *ctx) {
    // consoleHide() to hide this console
    // consoleHide(name) to hide that console
    if (duk_is_boolean(ctx, 0)){
        // old version 0.3 call
        bool which = duk_get_boolean(ctx, 0);
        duk_pop(ctx);
        Console* pConsole = findConsoleByCtx(ctx);
        if (which) pConsole->Hide();
        else pConsole->Show();
        pConsole->message(STYLE_ORANGE , "Warning - calling consoleHide with true or false deprecated");
        }
    else getConsoleWithOptionalNameOnStack(ctx)->Hide();
    return 0;
    }

static duk_ret_t duk_consoleShow(duk_context *ctx) {
    // consoleShow() to show this console
    // consoleShow(name) to show that console
    getConsoleWithOptionalNameOnStack(ctx)->Show();
    return 0;
    }
    
static duk_ret_t duk_consolePark(duk_context *ctx) {
	// park console consolePark(consolName) - if no name, park self
	Console* pConsole;
	duk_idx_t nargs = duk_get_top(ctx);  // number of args in call 
	wxString consoleName;	// console to park
	 
	if (nargs > 0) {
		consoleName = duk_get_string(ctx, 0);
		duk_pop(ctx);	// pop console name
		pConsole = findConsoleByName(consoleName);
		if (pConsole == nullptr) throwErrorByCtx(ctx, "consolPark called with non-existant console name: " + consoleName);
		}
	else pConsole = findConsoleByCtx(ctx);
	pConsole->park();
	return 0;
	}

static duk_ret_t duk_consoleName(duk_context *ctx) {
    // consoleName(newName) to rename this console
    // consoleName() no change
    // returns console name
    extern JavaScript_pi* pJavaScript_pi;
    void reviewParking();
    wxString newName, outcome;
    wxString checkConsoleName(wxString name, Console* pConsole);
    
    duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
    Console* pConsole = findConsoleByCtx(ctx);
    if (nargs > 1){pConsole->prep_for_throw(ctx, "consoleName has more than 1 argument"); duk_throw(ctx);}
    if (nargs == 1){	// have proposed new name
    	newName = duk_get_string(ctx, 0);
    	duk_pop(ctx);
        outcome = checkConsoleName(newName, pConsole);
        if (outcome != "") {
		pConsole->prep_for_throw(ctx, outcome);
		duk_throw(ctx);
		}
        // OK - ready to go
		pConsole->mConsoleName = newName;
		pConsole->SetLabel(newName);
		pConsole->setConsoleMinClientSize();
		wxSize newMinSize = pConsole->GetMinSize();
		if (pConsole->m_parkedLocation.set){
			pConsole->m_parkedLocation.size.x = newMinSize.x;
			}
		if (pConsole->isParked()) pConsole->SetSize(newMinSize);	// shrink it
		reviewParking();
    	if (pJavaScript_pi->pTools != nullptr) pJavaScript_pi->pTools->setConsoleChoices();	// rebuild in tools if open
        }

    duk_push_string(ctx, pConsole->mConsoleName);
    return 1;
    }

duk_ret_t chain_script(duk_context* ctx){
    // consoleChain(fileString,  brief)
    wxString fileString, script, brief, outcome;
    bool haveBrief {false};
    Console* pConsole = findConsoleByCtx(ctx);
    duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
    
    duk_require_string(ctx, 0);
    fileString = wxString(duk_get_string(ctx, 0));
    // the file string is relative - maybe. Ensure is absolute for record
    fileString = resolveFileName(fileString, pConsole, 0);
    if (nargs>1){
        duk_require_string(ctx, 1);
        brief = wxString(duk_get_string(ctx, 1));
        haveBrief = true;
        duk_pop(ctx);
        }
    duk_pop(ctx);
    
    outcome = getTextFile(fileString, &script, 10);
    if(outcome != wxEmptyString) {
		pConsole->prep_for_throw(ctx, _("getBrief found no brief"));
		duk_throw(ctx);
		}
    pConsole->m_Script->SetValue(script);
    pConsole->m_fileStringBox->SetValue(fileString);
    pConsole->auto_run->Show();
    pConsole->auto_run->SetValue(false);
    if (haveBrief){
        pConsole->mBrief.fresh = true;
        pConsole->mBrief.theBrief = brief;
        pConsole->mBrief.hasBrief = true;
        pConsole->mBrief.fresh = true;
        }
    else pConsole->mBrief.hasBrief = false;
    // ready to go
    pConsole->m_result = wxEmptyString;
    pConsole->m_explicitResult = true;
    pConsole->mStatus.set(TOCHAIN);
    duk_throw(ctx); // terminates this script.
    return 0;   // this to keep compiler happy
    }

static duk_ret_t duk_getBrief(duk_context *ctx) {
    // get any brief left for this script
    extern JavaScript_pi *pJavaScript_pi;
    Console *pConsole = findConsoleByCtx(ctx);
    wxString brief;
    if ( !pConsole->mBrief.hasBrief) {
		pConsole->prep_for_throw(ctx, _("getBrief found no brief"));
		duk_throw(ctx);
		}
    duk_push_string(ctx, pConsole->mBrief.theBrief);
    return 1;
    }

static duk_ret_t duk_onExit(duk_context *ctx){
    // set an onExit function
    wxString extractFunctionName(duk_context *ctx, duk_idx_t idx);
    extern JavaScript_pi *pJavaScript_pi;
    Console *pConsole = findConsoleByCtx(ctx);
    if (pConsole->mStatus.test(INEXIT)) {
		pConsole->prep_for_throw(ctx, "onExit within onExit function");
		duk_throw(ctx);
		}
    pConsole->m_exitFunction = extractFunctionName(ctx, -1);
    duk_pop(ctx);
    return 0;
    }
    
static duk_ret_t duk_copy(duk_context *ctx){
	// copy arg to clipboard
	Console *pConsole = findConsoleByCtx(ctx);
	if (!duk_is_string(ctx, 0)) {
		pConsole->prep_for_throw(ctx, "copyToClip requires string");
		duk_throw(ctx);
		}
	if (!wxTheClipboard->Open()) {
		pConsole->prep_for_throw(ctx, "copyToClip clipboard is busy");
		duk_throw(ctx);
		}
	wxString data = duk_get_string(ctx, 0);
	data = JScleanOutput(data);
	SUBDEGREE(data, PSEUDODEGREE, DEGREE);
//	data.Replace(PSEUDODEGREE, DEGREE, true);	// internally, we are using DEGREE to represent degree - convert any back
	wxTheClipboard->SetData(new wxTextDataObject(data));
	wxTheClipboard->Close();		
	return 0;
	}
	
static duk_ret_t duk_paste(duk_context *ctx){
	// get string from clipboard
	wxString getClipboardString(Console* pConsole);
	Console *pConsole = findConsoleByCtx(ctx);
	duk_push_string(ctx, getClipboardString(pConsole));
	return 1;
	}
	
static duk_ret_t duk_execute(duk_context *ctx){
	// execute(command string, env, errorOption);  returns output
	wxString command;
	wxArrayString output;
	wxArrayString errors;
	bool ignoreErrors {false};

	int flags = 0;
	wxExecuteEnv execEnv;	// execution environment variable
	execEnv.cwd = pJavaScript_pi->mCurrentDirectory;	// default to plugin current directory
	size_t i;
	
	duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
	command = duk_get_string(ctx, 0);
	duk_idx_t env_obj_idx {1};	// where the env object is if present
	if(duk_is_object_coercible(ctx, env_obj_idx)){	// yes - given environment variables
//	 	wxArrayString keys = {"PATH", "PWD"};	// possible keys supported
		wxArrayString keys;
		keys.Add("PATH");
		keys.Add("PWD");
		keys.Add("SHELL");
		for (i = 0; i < keys.GetCount(); i++){
			wxString key = keys[i];
			duk_push_string(ctx, key);
			if (duk_get_prop(ctx, env_obj_idx)){	// key exists
				wxString value = duk_get_string(ctx, -1);
				if (key == "PWD") execEnv.cwd = value;
//				else execEnv.env.SetEnv(key, value);
				else execEnv.env[key] = value;
				}
			}
	 	
	 	}
	 	
	 if (nargs >= 2) ignoreErrors = duk_get_boolean(ctx, 2);		
	duk_pop_n(ctx, nargs);
	
	long pid = wxExecute(command, output, errors, flags, &execEnv);

	wxString resultString = wxEmptyString;
	for (i = 0; i < output.GetCount(); i++) {
		if (i > 0) resultString += "\n";
		resultString += output.Item(i);
		}
	wxString errorString = wxEmptyString;
	for (i = 0; i < errors.GetCount(); i++){
		if (i > 0) errorString += "\n";
		errorString += errors.Item(i);
		}		
	duk_push_object(ctx);	// to be the result
		duk_push_string(ctx, resultString);
		duk_put_prop_literal(ctx, -2, "stdout");
		duk_push_string(ctx, errorString);
		duk_put_prop_literal(ctx, -2, "stderr");

	if (!ignoreErrors) {	// deal with any errors here
		if (pid != 0){	// there was an error
			wxString errorMessage;
			if (pid < 0) errorMessage = "execute unable to launch process";
			else errorMessage = "execute " + errorString.c_str();
			duk_push_error_object(ctx, DUK_ERR_TYPE_ERROR, errorString);
			duk_throw(ctx);
			}		
		}
	else {
		duk_push_int(ctx, pid);
		duk_put_prop_literal(ctx, -2, "errorCode");
		}
	return 1;		
	}

static duk_ret_t duk_get_fileString(duk_context *ctx){	// file dialogue to get file string
	wxString resolveFileName(wxString inputName, Console* pConsole, int mode);
    Console *pConsole = findConsoleByCtx(ctx);
	duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
	wxString file = duk_get_string(ctx, 0);
	int mode;
	if (nargs > 1) mode = duk_get_int(ctx, 1);
	else mode = 0;
	duk_pop_n(ctx, nargs);
	wxString resolved = resolveFileName(file, pConsole, mode);
	duk_push_string(ctx,resolved);
	return 1;
	}
	
int findFileIndex(Console *pConsole, int id){
	// given id, return index of entry in vector
	int index;
	int count = pConsole->m_wxFileFcbs.size();
	for (index = 0; index < count; index ++){
		if (pConsole->m_wxFileFcbs[index].id == id) return index;
		}
	pConsole->prep_for_throw(pConsole->mpCtx, "_wxFile failed to match id");
	duk_throw(pConsole->mpCtx);
	return 1;	// to keep compiler happy
	}
	
duk_ret_t clearFileEntry(duk_context *ctx){
	// finalise object by cleaning up.  Object on stack
	TRACE(99, wxString::Format("In _wxFile finaliser with ctx:%i", ctx));
	Console *pConsole = findConsoleByCtx(ctx);
	duk_get_prop_literal(ctx, -1, "id");
	int id = duk_get_int_default(ctx, -1, -1);	// index for this object
	TRACE(99, wxString::Format("Finaliser got id:%i", id));
	if (id == -1) {
		pConsole->message(STYLE_RED, "_wxFile failed to get id in finaliser");
		return 0;
		}
	int index = findFileIndex(pConsole, id);
	TRACE(99, wxString::Format("Finaliser got index:%i", index));
	pConsole->m_wxFileFcbs[index].pFile->Flush();
	delete pConsole->m_wxFileFcbs[index].pFile;
	pConsole->m_wxFileFcbs.erase(pConsole->m_wxFileFcbs.begin() + index);
	duk_pop_2(ctx);
	return 0;
	}
	
static duk_ret_t _wxFile(duk_context *ctx){
/****
When called with action 0, we create a control block which we stash in the vector pConsole->m_wxFileFcbs
This includes a wxFiie object
When the JavaScript File object is destroyed, this must all be cleared down
We achieve this by setting up a finaliser 'clearFileEntry' for the JavaScript File object which was passed by its this value. 
****/
    Console *pConsole = findConsoleByCtx(ctx);
    wxString JScleanString(wxString given);
	duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
	int action = duk_get_number(ctx, 0);
	switch (action){
		case 0:	{ // construct
			// call was ( 0, this, filesString[, mode])
//			FileOptions options;
			std::vector<wxFile::OpenMode> openMode {wxFile::read, wxFile::write, wxFile::read_write, wxFile::write_append, wxFile::write_excl};	
			wxString fileString = duk_to_string(ctx, 2);
			int mode =  duk_get_int(ctx, 3);
			wxString filePath = resolveFileName(fileString, pConsole,  mode);
			if (filePath == wxEmptyString) {
				pConsole->prep_for_throw(ctx, "_wxFile unable to resolve fileString " + fileString);
				duk_throw(ctx);
				}
			wxFileFcb control;
			control.pFile = new wxFile;
			bool ok;
			if (mode == 1) ok = control.pFile->Create(filePath, true);	// overwrite any existing file
			else if (mode == 5) ok = control.pFile->Create(filePath, false);	// create - file must not exist
			else ok = control.pFile->Open(filePath, openMode[mode]);
			if (!ok) {
				pConsole->prep_for_throw(ctx, wxString::Format("_wxFile unable to open file %s error %d", filePath, control.pFile->GetLastError()));
				duk_throw(ctx);
				}
			pConsole->m_wxFileFcbs.push_back(control);

			// set up finalisation
			duk_push_c_function(ctx, clearFileEntry, 1 /*nargs*/);
			duk_set_finalizer(ctx, 1);	// 'this' from call here
			
			duk_pop_n(ctx, nargs);	// clear call args
			duk_push_object(ctx); 	// will return the id and filePath
			duk_push_number(ctx, control.id);
			duk_put_prop_literal(ctx, -2, "id");
			duk_push_string(ctx, filePath);
			duk_put_prop_literal(ctx, -2, "filePath");
			return 1;
			}
		case 1: {	// Seek - call was (1, id, offset)
			int id = duk_get_number(ctx, 1);
			wxFileOffset offset = duk_get_int(ctx, 2);
			duk_pop_n(ctx, nargs);	// clear call args
			wxFileFcb control = pConsole->m_wxFileFcbs[findFileIndex(pConsole, id)];
			if (offset >= 0) offset =  control.pFile->Seek(offset);
			else offset =  control.pFile->SeekEnd();
			if (offset == wxInvalidOffset) {
				pConsole->prep_for_throw(ctx, "_wxFile invalid offset");
				duk_throw(ctx);
				}
			duk_push_int(ctx, offset);
			return 1;
			}
		case 2: {	// return current offset - call was (2, id)
			int id = duk_get_number(ctx, 1);
			duk_pop_n(ctx, nargs);	// clear call args
			wxFileFcb control = pConsole->m_wxFileFcbs[findFileIndex(pConsole, id)];
			wxFileOffset offset = control.pFile->Tell();
			if (offset == wxInvalidOffset) {
				pConsole->prep_for_throw(ctx, "_wxFile invalid offset");
				duk_throw(ctx);
				}
			duk_push_int(ctx, offset);
			return 1;
			}
		case 3: {	// check for eof - call was (3, id)
			int id = duk_get_number(ctx, 1);
			duk_pop_n(ctx, nargs);	// clear call args
			wxFileFcb control = pConsole->m_wxFileFcbs[findFileIndex(pConsole, id)];
			duk_push_boolean(ctx, control.pFile->Eof());
			return 1;
			}
		case 4: { // getAllText  call was (4, id)
			int id = duk_get_number(ctx, 1);
			duk_pop_n(ctx, nargs);	// clear call args
			wxFileFcb control = pConsole->m_wxFileFcbs[findFileIndex(pConsole, id)];
			wxString data;
			bool ok = control.pFile->ReadAll(&data);
			if (!ok) {
				pConsole->prep_for_throw(ctx, "_wxFile unable to read all text");
				duk_throw(ctx);
				}
			data = JScleanString(data);
			duk_push_string(ctx, data);
			return 1;
			}
		case 5:	// get text
		case 6:	// get bytes
			{ // call was (5|6, id, number)
			if (nargs < 3) {
				pConsole->prep_for_throw(ctx, "_wxFile get no count");
				duk_throw(ctx);
				}
			int id = duk_get_number(ctx, 1);
			size_t count = duk_get_number(ctx, 2);
			duk_pop_n(ctx, nargs);	// clear call args
			wxFileFcb control = pConsole->m_wxFileFcbs[findFileIndex(pConsole, id)];
			if (count < 1) {
				pConsole->prep_for_throw(ctx, "_wxFile get invalid count");
				duk_throw(ctx);
				}
			void* p = duk_push_dynamic_buffer(ctx, count);
			size_t got = control.pFile->Read(p, count);
			if (got == wxInvalidOffset) {
				pConsole->prep_for_throw(ctx, "_wxFile get invalid start with count");
				duk_throw(ctx);
				}
			if (got < count) duk_resize_buffer(ctx,-1, got);
			if (action == 5){
				duk_buffer_to_string(ctx, -1);
				wxString data = duk_get_string(ctx, -1);
				duk_pop(ctx);
				data = JScleanString(data);
				duk_push_string(ctx, data);
				}
			else duk_push_buffer_object(ctx, -1, 0, count, DUK_BUFOBJ_UINT8ARRAY);
			return 1;
			}			
		case 7: // write text
			{ // call was (7, id, string)
			int id = duk_get_number(ctx, 1);
			wxString data = duk_get_string(ctx, 2);
			duk_pop_n(ctx, nargs);	// clear call args
			data = JScleanOutput(data);
			wxFileFcb control = pConsole->m_wxFileFcbs[findFileIndex(pConsole, id)];
			bool OK = control.pFile->Write(data);
			duk_push_boolean(ctx, OK);
			return 1;
			}
		case 8:	//write bytes
			{ // call was 8, id, data)
			int id = duk_get_number(ctx, 1);
			size_t bufferSize;
			void* ptr = duk_get_buffer_data(ctx, 2, &bufferSize);	// get data
			duk_pop_n(ctx, nargs);	// clear call args
			TRACE(100, wxString::Format("Write buffer size:%i", bufferSize));
			wxFileFcb control = pConsole->m_wxFileFcbs[findFileIndex(pConsole, id)];
			if (bufferSize < 1) {
				pConsole->prep_for_throw(ctx, "_wxFile write invalid data length");
				duk_throw(ctx);
		}
			size_t wrote = control.pFile->Write(ptr, bufferSize);
			if (wrote != bufferSize) {
				pConsole->prep_for_throw(ctx, "_wxFile write wrong length");
				duk_throw(ctx);
		}
			return 0;
			}
		case -1: { // return file Length  call was (-1, id)
			int id = duk_get_number(ctx, 1);
			duk_pop_n(ctx, nargs);	// clear call args
			wxFileFcb control = pConsole->m_wxFileFcbs[findFileIndex(pConsole, id)];
			duk_push_int(ctx, control.pFile->Length());
			return 1;
			}						
		default:
			pConsole->prep_for_throw(ctx, "_wxFile invalid action");
			duk_throw(ctx);
		}
	
	duk_pop_n(ctx, nargs);
	duk_push_number(ctx, nargs);
	return 1;
	}
	

#if DUK_DUMP

static duk_ret_t duk_mainThread(duk_context *ctx){ // check if in main thread
    duk_push_boolean(ctx, wxThread::IsMain());
    return 1;
    
    }
static duk_ret_t jstest(duk_context *ctx){  // for testing when in harness
    int a = duk_get_int(ctx, 0);
    int b = duk_get_int(ctx, 1);
    if ((a == b) && (a > 0)){
        duk_push_error_object(ctx, DUK_ERR_TYPE_ERROR, _("the arguments match - duk_throw"));
        duk_throw(ctx);
        }
    else if ((a == b) && (a == 0)) {
        Console *pConsole = findConsoleByCtx(ctx);
    	pConsole->prep_for_throw(ctx, "the arguments are both zero - prep_for_throw");
    	duk_throw(ctx);
    	}
    duk_push_int(ctx, a + b);
    return 1;
    }

static duk_ret_t dukDump(duk_context *ctx){  // for testing when in harness
    Console *pConsole = findConsoleByCtx(ctx);
    duk_push_string(ctx, pConsole->dukDump());
    return 1;
    }

static duk_ret_t consoleDump(duk_context *ctx){  // dumps useful stuff from the console structure
    Console *pConsole = findConsoleByCtx(ctx);
    duk_push_string(ctx, pConsole->consoleDump());
    return 1;
    }
#endif

// following needed later for safe decoding of JSON string
typedef struct {
	wxString jsonString;
	} json_safe_args;	
duk_ret_t safe_decode_JSON(duk_context *ctx, void *udata) {	// function for safe call to decode JSON
		json_safe_args *args = (json_safe_args *) udata;
		duk_push_string(ctx, args->jsonString);	// push the JSON string being the remembred value
	    duk_json_decode(ctx, -1);	// recover original object - might have invalid JSON
	    return 1;	// returns with decoded object on stack if OK
		}
		
static duk_ret_t duk_onCloseButton(duk_context *ctx){
	wxString extractFunctionName(duk_context *ctx, duk_idx_t idx);
    Console *pConsole = findConsoleByCtx(ctx);
	duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
	if (nargs == 0) pConsole-> m_closeButtonFunction = wxEmptyString;
	else if (nargs == 1){
		if (pConsole-> m_closeButtonFunction != wxEmptyString) {
			pConsole->prep_for_throw(ctx, "onCloseButton already set");
			duk_throw(ctx);
			}
		pConsole-> m_closeButtonFunction = extractFunctionName(ctx, 0);
		duk_pop(ctx);
		}
	else {
		pConsole->prep_for_throw(ctx, "onCloseButton requires just one argument");
		duk_throw(ctx);
		}
	return 0;
	}

static duk_ret_t cleanString(duk_context *ctx){
	// string = cleanString(string)
	wxString JScleanString(wxString given);
	wxString string = duk_get_string(ctx, 0);
	duk_pop(ctx);
	duk_push_string(ctx, JScleanString(string));
	return 1;
	}
	
	
static duk_ret_t NMEAchecksum(duk_context *ctx){
	// checkChars = NMEAchecksum(sentence)
	wxString NMEAchecksum(wxString sentence);	
	wxString sentence = duk_get_string(ctx, 0);
	duk_pop(ctx);
	duk_push_string(ctx, NMEAchecksum(sentence));
	return 1;
	}

static duk_ret_t keyboardState(duk_context *ctx){
	// state = keyboardState();
    duk_push_object(ctx);
        duk_push_boolean(ctx, wxGetKeyState(WXK_SHIFT));
            duk_put_prop_literal(ctx, -2, "shift");
		// the following is a confusing mess between MacOS and Windows
/*
#ifdef __WXMAC__
		duk_push_boolean(ctx, wxGetKeyState(WXK_MENU));
            duk_put_prop_literal(ctx, -2, "control");
#endif
*/
        duk_push_boolean(ctx, wxGetKeyState(WXK_ALT));
            duk_put_prop_literal(ctx, -2, "option");
        duk_push_boolean(ctx, wxGetKeyState(WXK_CONTROL));
            duk_put_prop_literal(ctx, -2, "menu");            
    return 1;
    }
	   
void duk_extensions_init(duk_context *ctx, Console* pConsole) {
    extern duk_ret_t duk_dialog(duk_context *ctx);
    duk_idx_t duk_push_c_function(duk_context *ctx, duk_c_function func, duk_idx_t nargs);

    duk_push_global_object(ctx);
    
    duk_push_string(ctx, "print");
    duk_push_c_function(ctx, duk_print, DUK_VARARGS /*variable number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "printRed");
    duk_push_c_function(ctx, duk_print_red, DUK_VARARGS /*variable number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "printOrange");
    duk_push_c_function(ctx, duk_print_orange, DUK_VARARGS /*variable number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "printGreen");
    duk_push_c_function(ctx, duk_print_green, DUK_VARARGS /*variable number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "printBlue");
    duk_push_c_function(ctx, duk_print_blue, DUK_VARARGS /*variable number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

#if 0
    duk_push_string(ctx, "printBold");
    duk_push_c_function(ctx, duk_print_bold, DUK_VARARGS /*variable number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
#endif
    
    duk_push_string(ctx, "printUnderlined");
    duk_push_c_function(ctx, duk_print_ul, DUK_VARARGS /*variable number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "printLog");
    duk_push_c_function(ctx, duk_log, DUK_VARARGS /*variable number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "alert");
    duk_push_c_function(ctx, duk_alert, DUK_VARARGS /*variable number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "messageBox");
    duk_push_c_function(ctx, duk_message, DUK_VARARGS /*variable number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "scriptResult");
    duk_push_c_function(ctx, duk_result, DUK_VARARGS /*variable number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "NMEA0183checksum");
    duk_push_c_function(ctx, NMEAchecksum, 1);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "onSeconds");
    duk_push_c_function(ctx, onSeconds, DUK_VARARGS);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "onAllSeconds");
    duk_push_c_function(ctx, onAllSeconds, DUK_VARARGS);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "onDialogue");
    duk_push_c_function(ctx, duk_dialog, DUK_VARARGS /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

#if 0
    duk_push_string(ctx, "format");
    duk_push_c_function(ctx, duk_format, DUK_VARARGS /*variable number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
#endif
    
    duk_push_string(ctx, "readTextFile");
    duk_push_c_function(ctx, duk_read_text_file, 1 /* number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "writeTextFile");
    duk_push_c_function(ctx, duk_write_text_file, 3 /* number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "require");
    duk_push_c_function(ctx, duk_require, 1 /* number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "timeAlloc");
    duk_push_c_function(ctx, duk_timeAlloc, DUK_VARARGS /* variable arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "consoleHide");
    duk_push_c_function(ctx, duk_consoleHide, DUK_VARARGS /* variable arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "consoleShow");
    duk_push_c_function(ctx, duk_consoleShow, DUK_VARARGS /* variable arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

	duk_push_string(ctx, "consoleName");
    duk_push_c_function(ctx, duk_consoleName, DUK_VARARGS /* variable arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "consolePark");
    duk_push_c_function(ctx, duk_consolePark, DUK_VARARGS /* variable arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "stopScript");
    duk_push_c_function(ctx, duk_stopScript , DUK_VARARGS /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "chainScript");
    duk_push_c_function(ctx, chain_script, DUK_VARARGS /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "getBrief");
    duk_push_c_function(ctx, duk_getBrief , 0 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "onExit");
    duk_push_c_function(ctx, duk_onExit , 1 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "toClipboard");
    duk_push_c_function(ctx, duk_copy, 1 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "fromClipboard");
    duk_push_c_function(ctx, duk_paste, 0 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "execute");
    duk_push_c_function(ctx, duk_execute, DUK_VARARGS /* variable arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "getFileString");
    duk_push_c_function(ctx, duk_get_fileString, DUK_VARARGS /* variable arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE); 
    
    duk_push_string(ctx, "_wxFile");
    duk_push_c_function(ctx, _wxFile, DUK_VARARGS /* variable arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "onCloseButton");
    duk_push_c_function(ctx, duk_onCloseButton, DUK_VARARGS /* variable arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "cleanString");
    duk_push_c_function(ctx, cleanString, 1);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "keyboardState");
    duk_push_c_function(ctx, keyboardState, 0);	// no args
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);


    
	// set up the _remember variable
	// the JSON string could be invalid - need to catch any error	
    json_safe_args args;
    args.jsonString = pConsole->m_remembered;
    duk_int_t rc = duk_safe_call(ctx, safe_decode_JSON, (void *) &args, 0 /*nargs*/, 1 /*nrets*/);
    if (rc != DUK_EXEC_SUCCESS) {	// JSON decode error
    	duk_pop(ctx);	// pop off invalid JSON result
    	duk_push_undefined(ctx);	// replace with undefined
    	if (pConsole->m_remembered != wxEmptyString){// this was a real error in the JSON}
    		pConsole->message(STYLE_ORANGE, "_remember saved value invalid JSON - have set to undefined");
    		}
    	}	
	duk_push_string(ctx, "_remember");
	duk_swap_top(ctx, -2);		
	duk_def_prop(ctx, -3, DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_HAVE_VALUE);


#if 0	
    duk_push_string(ctx, "test");
    duk_push_c_function(ctx, test , DUK_VARARGS /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
#endif



#if DUK_DUMP
    duk_push_string(ctx, "JS_throw_test");
    duk_push_c_function(ctx, jstest, 2 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "duktapeDump");
    duk_push_c_function(ctx, dukDump, 0 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "consoleDump");
    duk_push_c_function(ctx, consoleDump , 0 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "JS_mainThread");
    duk_push_c_function(ctx, duk_mainThread, 0 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

#endif

    duk_pop(ctx);
}