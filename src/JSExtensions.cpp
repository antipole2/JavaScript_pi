/**************************************************************************
* Project:  OpenCPN
* Purpose:  JavaScript Plugin
* Author:   Tony Voss 16/05/2020
*
* Copyright Ⓒ 2020 by Tony Voss
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, under which
* you must including a copy of these terms in all copies
* https://www.gnu.org/licenses/gpl-3.0.en.html
***************************************************************************
*/
#include "stdio.h"
#include "duktape.h"
#include "JavaScript_pi.h"
#include "JavaScriptgui.h"
#include "JavaScriptgui_impl.h"
#include <stdarg.h>
#include "ocpn_duk.h"
#include <string>

/*  On hold for now
 // sprintf function
 static duk_ret_t duk_sprintf(duk_context *ctx) {
 const char *duk_push_string(duk_context *ctx, const char *str);
 const char *duk_to_string(duk_context *ctx, duk_idx_t idx);
 const char *duk_push_sprintf(duk_context *ctx, const char *fmt, ...);
 duk_int_t duk_get_type(duk_context *ctx, duk_idx_t idx);
 duk_idx_t nargs;  // number of args in call
 
 nargs = duk_get_top(ctx);
 cout << "Inside sprintf - nargs=" << nargs <<"\n";
 cout << "arg0 is type:" << duk_get_type(ctx, 0) << " and value:" << duk_to_string(ctx,0) << "\n";
 cout << "arg1 is type:" << duk_get_type(ctx, 1) << " and value:" << duk_to_string(ctx,1) << "\n";
 cout << "sprintf gives:" << duk_push_sprintf(ctx, duk_to_string(ctx,0), duk_to_string(ctx,1)) << "\n";
 
 return (1);
 //    return (*duk_push_sprintf(ctx, "meaning of life: %d, name", 42));
 }
 */
extern JS_control_class JS_control;

duk_bool_t JS_exec(duk_context *ctx){
    // execute the code on the stack and report any error
    // returns true if success with result on stack, else false with error left on stack
    bool result;
    void JSduk_start_exec_timeout();
    void JSduk_clear_exec_timeout();
    
    if (ctx == nullptr){
        JS_control.message(STYLE_RED, _("JS_exec"), _("called with no ctx - logic eror"));
        return false;
        }
    JSduk_start_exec_timeout(); // start timer
    result = duk_pcall(ctx, 1);    // run code
    JSduk_clear_exec_timeout(); // cancel time-out
    if (JS_control.m_stopScriptCalled) return (false);
    if (result != DUK_EXEC_SUCCESS){
        JS_control.display_error(ctx, duk_safe_to_string(ctx, -1));
        return false;
        }
    return true;
    }
    
wxString js_formOutput(duk_context *ctx){
    duk_idx_t nargs;  // number of args in call
    wxString output = "";
    
    nargs = duk_get_top(ctx);
    for(int i = 0; i<nargs; i++){
        switch (duk_int_t type = duk_get_type(ctx, i)){
            case DUK_TYPE_STRING:
            case DUK_TYPE_NUMBER:
                output = output + duk_to_string(ctx,i);  // mangles º character on Windows!
                break;
            case DUK_TYPE_BOOLEAN:
                output = output + ((duk_to_boolean(ctx,i) == 1) ? "true":"false");
                break;
            case DUK_TYPE_OBJECT:
                duk_json_encode(ctx, i);
                output = output + duk_to_string(ctx,i);
                break;
            default:
                JS_control.throw_error(ctx, "print - arg " + to_string(i) + " of unexpected type " + to_string(type));
        }
    }
#ifdef __WXMSW__
    wxString JScleanOutput(wxString given);
    output = JScleanOutput(output); // clean for Windows only
#endif
    return(output);
}

duk_ret_t print_coloured(duk_context *ctx, int colour) {   // print arguments on stack in colour
    JS_control.message(colour,wxEmptyString, js_formOutput(ctx));
    return (0);
}

void onDismissAlert(wxCommandEvent & event){
    // here when alert to be dismissed
#ifdef IN_HARNESS
    cout << "In alertDismissed\n";
#endif  // IN_HARNESS
     JS_control.clearAlert();
    if (!JS_control.waiting()) {
        JS_control.clearAndDestroy();
        }
    }

static duk_ret_t duk_alert(duk_context *ctx) {   // create or add to an alert  box with dismiss button
    duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
    wxDialog *alert = JS_control.m_alert.palert;    // any existing alert
    
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
            JS_control.clearAlert();
            duk_push_boolean(ctx, true);
            }
        else duk_push_boolean(ctx, false);
        return 1;
        }
    
    // we have alert text to display
    JS_control.m_alert.alertText += js_formOutput(ctx); // add text to any existing
    if (alert != nullptr){  // clear any existing alert being displayed
        JS_control.m_alert.palert->Close();
        JS_control.m_alert.palert->Destroy();
        }
    alert = new wxDialog(NULL,  wxID_ANY, _("JavaScript alert"), JS_control.m_alert.position, wxDefaultSize,
                         wxCAPTION | wxSTAY_ON_TOP | wxBORDER_RAISED);
    alert->SetBackgroundColour(*wxYELLOW);
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);  // A top-level sizer
    alert->SetSizer(boxSizer);
    wxStaticText* staticText = new wxStaticText( alert, wxID_STATIC, JS_control.m_alert.alertText, wxDefaultPosition, wxDefaultSize, 0 );
    boxSizer->Add(staticText, 0, wxALIGN_LEFT|wxALL, 5);
    wxBoxSizer* buttonBox = new wxBoxSizer(wxHORIZONTAL);
    boxSizer->Add(buttonBox, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxButton* button = new wxButton ( alert, wxNewId(), _("Dismiss"), wxDefaultPosition, wxDefaultSize, 0 );
    buttonBox->Add(button, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    alert->Bind(wxEVT_BUTTON, &onDismissAlert, wxID_ANY);
    alert->Fit();
    alert->Show(true);
    JS_control.m_alert.palert = alert;  // remember
    duk_pop_n(ctx, nargs);  // pop all the arguments
    duk_push_boolean(ctx, true);
    return 1;
    }

static duk_ret_t duk_result(duk_context *ctx) {   // result
    JS_control.m_explicitResult = true;
    if (duk_get_top(ctx) > 0) {// called with args
        JS_control.m_result = js_formOutput(ctx);
        duk_pop(ctx);
        }
    duk_push_string(ctx, JS_control.m_result);
    return (1); // return present result
    }

static duk_ret_t duk_print(duk_context *ctx) {   // print
    duk_ret_t result = 0;
    JS_control.m_pJSconsole->Show(); // make sure console is visible
    JS_control.m_pJSconsole->m_Output->AppendText(js_formOutput(ctx));
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

static duk_ret_t duk_log(duk_context *ctx) {   // log message
    duk_ret_t result = 0;
    wxLogMessage(js_formOutput(ctx));
    return (result);
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
    wxString fileNameGiven, fileString, lineOfFile, text;
    wxFileName filePath;
    wxTextFile inputFile;
    wxFileName homeDirectory;
    bool ok;
    wxString JScleanString(wxString line);
    
    fileNameGiven = duk_to_string(ctx,0);
    duk_pop(ctx);  // finished with that
    filePath = wxFileName(fileNameGiven);
    if (filePath.IsOk()){
        if (!filePath.FileExists()) {
            if (filePath.IsRelative()){  // Let's try relative to home directory
                homeDirectory.AssignHomeDir();
                fileString = homeDirectory.GetFullPath();
                filePath = wxFileName(homeDirectory.GetFullPath() + fileNameGiven);
                fileString = filePath.GetFullPath();
                }
            }
        if (!filePath.FileExists()) {  // try again
            JS_control.throw_error(ctx, "readTextFile " + filePath.GetFullPath() + " not found");
            }
        if (!filePath.IsFileReadable()) JS_control.throw_error(ctx, "readTextFile " + filePath.GetFullPath() + " not readable");
        }
    else JS_control.throw_error(ctx, "readTextFile " + filePath.GetFullPath() + " does not make sense");
    fileString = filePath.GetFullPath();
    ok = inputFile.Open(fileString);
    for (lineOfFile = inputFile.GetFirstLine(); !inputFile.Eof(); lineOfFile = inputFile.GetNextLine()){
        text += lineOfFile + "\n";
        }
    text = JScleanString(text);
    duk_push_string(ctx, text);
    return 1;
    };

duk_ret_t duk_require(duk_context *ctx){ // the module search function
    wxString fileNameGiven, fileString, lineOfData, script;
    wxFileName filePath;
    wxTextFile inputFile;
    wxFileName homeDirectory;
    wxString JScleanString(wxString line);
    bool ok;
    
    fileNameGiven = duk_to_string(ctx,0);
    duk_pop(ctx);  // finished with that
    filePath = wxFileName(fileNameGiven);
    if ((filePath.GetDirCount() == 0) && !filePath.HasExt()){
        // simple file name - we will look in the plugin library
        filePath = *GetpSharedDataLocation() +
            _T("plugins") + wxFileName::GetPathSeparator() +
            _T("JavaScript_pi") + wxFileName::GetPathSeparator() +
            _T("data") + wxFileName::GetPathSeparator() +
            _T("scripts") + wxFileName::GetPathSeparator() +
            fileNameGiven;
        if (!filePath.FileExists()){
                JS_control.throw_error(ctx, "require - " + filePath.GetFullName() + " not in scripts library");
            }
        if (!filePath.IsFileReadable()) JS_control.throw_error(ctx, "readTextFile " + filePath.GetFullPath() + " not readable");
        // ready to go
        }
    
    else{   // not a library script}
        if (filePath.IsOk()){
            if (!filePath.FileExists()) {
                if (filePath.IsRelative()){  // Let's try relative to home directory
                    homeDirectory.AssignHomeDir();
                    fileString = homeDirectory.GetFullPath();
                    filePath = wxFileName(homeDirectory.GetFullPath() + fileNameGiven);
                    fileString = filePath.GetFullPath();
                    }
                }
            if (!filePath.FileExists()) {  // try again
                JS_control.throw_error(ctx, "require - " + filePath.GetFullPath() + " not found");
                }
            if (!filePath.IsFileReadable()) JS_control.throw_error(ctx, "require - " + filePath.GetFullPath() + " not readable");
            }
        else JS_control.throw_error(ctx, "require - " + filePath.GetFullPath() + " does not make sense");
        }
    fileString = filePath.GetFullPath();
    ok = inputFile.Open(fileString);
    for (lineOfData = inputFile.GetFirstLine(); !inputFile.Eof(); lineOfData = inputFile.GetNextLine()){
        script += lineOfData + "\n";
        }
    script = JScleanString(script);
    duk_push_string(ctx, script);
    duk_push_string(ctx, "position");
    if (duk_pcompile(ctx, DUK_COMPILE_FUNCTION)){
        JS_control.throw_error(ctx, duk_safe_to_string(ctx, -1));
        }
    return(1);
    };

static duk_ret_t onSeconds(duk_context *ctx) {  // call function after milliseconds elapsed
    duk_ret_t result = 0;
    duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
    TimeActions ourTimeAction;
    wxString argument = wxEmptyString;
    
    if (nargs == 0) { // empty call - cancel all timers
        JS_control.m_times.Clear();
        JS_control.m_timerActionBusy = false;
        return(result);
        }
    
    duk_require_function(ctx, 0);
    if ((int)JS_control.m_times.GetCount() > 10){
        JS_control.throw_error(ctx, "onSeconds error: already have 10 calls outstanding");
        }
    if ((nargs < 2) || (nargs > 3)){
        JS_control.throw_error(ctx, "onSeconds error: requires two or three arguments");

        }
    wxTimeSpan delay(0, 0, duk_to_number(ctx,1), 0);
    ourTimeAction.timeToCall = wxDateTime::Now() + delay;
    ourTimeAction.functionName = JS_control.getFunctionName();
    if (nargs > 2) argument = wxString(duk_to_string(ctx,2));  //if user included 3rd argument, use it
    ourTimeAction.argument = argument;
    JS_control.m_times.Add(ourTimeAction);  // add this action to array
    JS_control.m_JSactive = true;
    return(result);
    }

static duk_ret_t duk_timeAlloc(duk_context *ctx) {   // time allocation
    wxLongLong timeLeft =  JS_control.m_pcall_endtime - wxGetUTCTimeMillis();
    if (timeLeft < 0)  timeLeft = 0;    // have captured time left
        if (duk_get_top(ctx) > 0 ){
        // was called with argument - us to set new allocatins
        JS_control.m_time_to_allocate = duk_get_int(ctx, -1);
        duk_pop(ctx);
        }
    JS_control.m_pcall_endtime = wxGetUTCTimeMillis() + JS_control.m_time_to_allocate;  // grant more time
    duk_push_number(ctx, timeLeft.GetValue());   // will return time that was left
    return (1);
}

static duk_ret_t duk_stopScript(duk_context *ctx) {
    duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
    
    if (nargs > 0){
        JS_control.m_result = duk_safe_to_string(ctx, -1);
        JS_control.m_explicitResult = true;
        duk_pop(ctx);
        }
    JS_control.m_stopScriptCalled = true;
    duk_push_error_object(ctx, DUK_ERR_ERROR, _("stopScript called"));
    duk_throw(ctx);
    return 1;  //never executed but keeps compiler happy
    }

static duk_ret_t duk_consoleHide(duk_context *ctx) {    // hide console if arg true else show
    bool arg;
    duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
 
    if (nargs > 1) JS_control.throw_error(ctx, "consoleHide can have no more than 1 argument");
    if (nargs == 1){
        duk_require_boolean(ctx, 0);
        arg = duk_get_boolean(ctx, 0);
        duk_pop(ctx);
        if (arg){
            JS_control.m_pJSconsole->Hide();
            JS_control.m_pJSconsole->pPlugIn->m_bShowJavaScript = false;
            }
        else {
            JS_control.m_pJSconsole->Show();
            JS_control.m_pJSconsole->pPlugIn->m_bShowJavaScript = true;
            }
        }
    duk_push_boolean(ctx,!JS_control.m_pJSconsole->IsShown());  // return true if hidden, even with no args
    return 1;
    }

#ifdef DUK_DUMP

static duk_ret_t duk_mainThread(duk_context *ctx){ // check if in main thread
    duk_push_boolean(ctx, wxThread::IsMain());
    return 1;
    
    }
static duk_ret_t jstest(duk_context *ctx){  // for testing when in harness
    int a = duk_get_int(ctx, 0);
    int b = duk_get_int(ctx, 1);
    if (a == b){
        duk_push_error_object(ctx, DUK_ERR_TYPE_ERROR, _("the arguments match"));
        duk_throw(ctx);
        }
    duk_push_int(ctx, a + b);
    return 1;
    }

static duk_ret_t dukDump(duk_context *ctx){  // for testing when in harness
    ERROR_DUMP
    return 0;
    }

static duk_ret_t JSdump(duk_context *ctx){  // dumps useful stuff from the JS-control structure
    duk_idx_t nargs = duk_get_top(ctx);
    wxString attribute = wxEmptyString;
    if (nargs > 0){
        attribute = duk_safe_to_string(ctx, -1);
        duk_pop(ctx);
    }
    JS_control.message(STYLE_ORANGE, attribute, JS_control.dump());
    return 0;
    }
#endif
    
void duk_extensions_init(duk_context *ctx) {
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

    duk_push_string(ctx, "printLog");
    duk_push_c_function(ctx, duk_log, DUK_VARARGS /*variable number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "alert");
    duk_push_c_function(ctx, duk_alert, DUK_VARARGS /*variable number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
     duk_push_string(ctx, "scriptResult");
    duk_push_c_function(ctx, duk_result, DUK_VARARGS /*variable number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "onSeconds");
    duk_push_c_function(ctx, onSeconds, DUK_VARARGS);
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

    duk_push_string(ctx, "require");
    duk_push_c_function(ctx, duk_require, 1 /* number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "timeAlloc");
    duk_push_c_function(ctx, duk_timeAlloc, DUK_VARARGS /* variable arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "consoleHide");
    duk_push_c_function(ctx, duk_consoleHide, DUK_VARARGS /* variable arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "stopScript");
    duk_push_c_function(ctx, duk_stopScript , DUK_VARARGS /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    
#ifdef DUK_DUMP
    duk_push_string(ctx, "JS_throw_test");
    duk_push_c_function(ctx, jstest, 2 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "JS_duk_dump");
    duk_push_c_function(ctx, dukDump, 0 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "JS_control_dump");
    duk_push_c_function(ctx, JSdump , DUK_VARARGS /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "JS_mainThread");
    duk_push_c_function(ctx, duk_mainThread, 0 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

#endif

    duk_pop(ctx);
}
