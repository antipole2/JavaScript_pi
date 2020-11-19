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

void JS_dk_error(duk_context *ctx, wxString message){
    //throw an error from within c++ code
    JS_control.m_pJSconsole->Show(); // make sure console is visible
    JS_control.m_result = wxEmptyString;    // supress result
    JS_control.m_explicitResult = true;    // supress result
    JS_control.clear();
    duk_push_error_object(ctx, DUK_ERR_ERROR, message);
    duk_throw(ctx);
    return;
    }

duk_bool_t JS_exec(duk_context *ctx){
    // execute the code on the stack and report any error
    // returns true if success with result on stack, else false with error left on stack
    bool result;
    void JSduk_start_exec_timeout();
    void JSduk_clear_exec_timeout();
    
    JSduk_start_exec_timeout(); // start timer
    result = duk_pcall(ctx, 1);    // run code
    JSduk_clear_exec_timeout(); // cancel time-out
    if (result != 0){
        // an error return
        JS_control.m_pJSconsole->Show(); // make sure console is visible
        JS_control.m_result = wxEmptyString;    // supress result
        JS_control.m_explicitResult = true;    // supress result
        jsMessage(ctx, STYLE_RED, wxEmptyString, duk_safe_to_string(ctx, -1));
//        JS_control.clear();
        return false;
        }
    return true;
    }
    
wxString js_formOutput(duk_context *ctx){
    wxString JScleanOutput(wxString given);
    duk_idx_t nargs;  // number of args in call
    wxString output = "";
    
    nargs = duk_get_top(ctx);
    for(int i = 0; i<nargs; i++){
        switch (duk_int_t type = duk_get_type(ctx, i)){
            case DUK_TYPE_STRING:
            case DUK_TYPE_NUMBER:
                output = output + duk_to_string(ctx,i);  // mangles º chatcater on Windows!
                break;
            case DUK_TYPE_BOOLEAN:
                output = output + ((duk_to_boolean(ctx,i) == 1) ? "true":"false");
                break;
            case DUK_TYPE_OBJECT:
                duk_json_encode(ctx, i);
                output = output + duk_to_string(ctx,i);
                break;
            default:
                JS_dk_error(ctx, "print - arg " + to_string(i) + " of unexpected type " + to_string(type));
        }
    }
#ifdef __WXMSW__
    output = JScleanOutput(output); // clean for Windows only
#endif
    return(output);
}

duk_ret_t print_coloured(duk_context *ctx, int colour) {   // print in colour
    duk_ret_t result = 0;
    JS_control.m_pJSconsole->Show(); // make sure console is visible
    int beforeLength = JS_control.m_pJSconsole->m_Output->GetTextLength(); // where we are before adding text
    JS_control.m_pJSconsole->m_Output->AppendText(js_formOutput(ctx));
    int afterLength = JS_control.m_pJSconsole->m_Output->GetTextLength(); // where we are before adding text
    JS_control.m_pJSconsole->m_Output->StartStyling(beforeLength);
    JS_control.m_pJSconsole->m_Output->SetStyling(afterLength-beforeLength-1, colour);
    return (result);
}

static duk_ret_t duk_alert(duk_context *ctx) {   // alert
    duk_ret_t result = 0;
    
#ifdef IN_HARNESS
    wxMessageBox(js_formOutput(ctx), _("Alert"));
#else   // IN_HARNESS
    OCPNMessageBox_PlugIn(JS_control.m_pJSconsole->pParent, js_formOutput(ctx), _("Alert"),
                         wxOK, -1, -1);
#endif  // IN_HARNESS
    return (result);
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
// On hold for now - variable argumenst of variable types defeated me
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
            JS_dk_error(ctx, "readTextFile " + filePath.GetFullPath() + " not found");
            }
        if (!filePath.IsFileReadable()) JS_dk_error(ctx, "readTextFile " + filePath.GetFullPath() + " not readable");
        }
    else JS_dk_error(ctx, "readTextFile " + filePath.GetFullPath() + " does not make sense");
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
                JS_dk_error(ctx, "require - " + filePath.GetFullName() + " not in scripts library");
            }
        if (!filePath.IsFileReadable()) JS_dk_error(ctx, "readTextFile " + filePath.GetFullPath() + " not readable");
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
                JS_dk_error(ctx, "require - " + filePath.GetFullPath() + " not found");
                }
            if (!filePath.IsFileReadable()) JS_dk_error(ctx, "require - " + filePath.GetFullPath() + " not readable");
            }
        else JS_dk_error(ctx, "require - " + filePath.GetFullPath() + " does not make sense");
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
        JS_dk_error(ctx, duk_safe_to_string(ctx, -1));
        }
    return(1);
    };

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

static duk_ret_t duk_consoleHide(duk_context *ctx) {    // hide console if arg true else show
    duk_idx_t nargs;  // number of args in call
    bool arg;

    nargs = duk_get_top(ctx);
    if (nargs > 1) JS_dk_error(ctx, "consoleHide can have no more than 1 argument");
    if (nargs == 1){
        duk_require_boolean(ctx, 0);
        arg = duk_get_boolean(ctx, 0);
        duk_pop(ctx);
        if (arg) JS_control.m_pJSconsole->Hide();
        else JS_control.m_pJSconsole->Show();
        }
    duk_push_boolean(ctx,!JS_control.m_pJSconsole->IsShown());  // return true if hidden, even with no args
    return 1;
}

static duk_ret_t duk_mainThread(duk_context *ctx){ // check if in main thread
    duk_push_boolean(ctx, wxThread::IsMain());
    return 1;
}

/*
int pjstest(duk_context *ctx){
    int a, b;
    MAYBE_DUK_DUMP
    a = duk_get_int(ctx, 0);
    b = duk_get_int(ctx, 1);

     if (a == b){
//        jsMessage(ctx, STYLE_GREEN, wxEmptyString, _("args match"));
 //        duk_push_error_object(ctx, 1, "args match");
 //        duk_throw(ctx);
         duk_error(ctx, DUK_ERR_TYPE_ERROR, "args match");
         
         
        }
   duk_pop_2(ctx);
    duk_push_int(ctx, a+b);
     MAYBE_DUK_DUMP
    return (1);
    }

static duk_ret_t jstest(duk_context *ctx){
    duk_int_t rc;
    MAYBE_DUK_DUMP
    duk_push_c_function(ctx, pjstest, 2);
    duk_insert(ctx, -3);
    MAYBE_DUK_DUMP
    rc = duk_pcall(ctx, 2);
    if (rc != 0) {
        MAYBE_DUK_DUMP
        jsMessage(ctx, STYLE_GREEN, wxEmptyString, duk_safe_to_string(ctx, -1));
        return(-1);
        }
    MAYBE_DUK_DUMP
    return 1;
    }
 */

static duk_ret_t jstest(duk_context *ctx){
    int a, b;
    MAYBE_DUK_DUMP
    a = duk_get_int(ctx, 0);
    b = duk_get_int(ctx, 1);

    if (a == b){
        duk_push_error_object(ctx, DUK_ERR_TYPE_ERROR, _("the arguments match"));
        duk_throw(ctx);
        }
    duk_push_int(ctx, a + b);
    return 1;
    }
    



void duk_extensions_init(duk_context *ctx) {
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
    
     duk_push_string(ctx, "result");
    duk_push_c_function(ctx, duk_result, DUK_VARARGS /*variable number of arguments*/);
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
    
    duk_push_string(ctx, "mainThread");
    duk_push_c_function(ctx, duk_mainThread, 0 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "jstest");
    duk_push_c_function(ctx, jstest, 2 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_pop(ctx);
}
