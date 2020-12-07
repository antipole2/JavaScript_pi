/************************************************************************
* Project:  OpenCPN
* Purpose:  JavaScript Plugin
* Author:   Tony Voss
*
* Copyright Ⓒ 2020 by Tony Voss
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, under which
* you must including a copy of these terms in all copies
* https://www.gnu.org/licenses/gpl-3.0.en.html
***************************************************************************
*/
// Runs some JavaScript and returns any result

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include "duktape.h"
#include "stdio.h"
#include <wx/arrimpl.cpp>
#include <string>
#include "wx/strconv.h"
#include <stdarg.h>
#include "JavaScript_pi.h"
#include "JavaScriptgui.h"
#include "JavaScriptgui_impl.h"
#include "ocpn_duk.h"
#include "wx/stc/stc.h"

extern JS_control_class JS_control;

void fatal_error_handler(void *udata, const char *msg) {
    // Provide for handling fatal error while running duktape
    (void) udata;  /* ignored in this case, silence warning */
    
    JS_control.m_result = wxEmptyString;    // supress result
    JS_control.m_explicitResult = true;    // supress result
    JS_control.message(STYLE_RED, "type", msg);

    return;
    
#if 0
    /* Note that 'msg' may be NULL. */
    wxMessageBox((msg ? msg : "(no message)"), "Fatal JavaScript error");
    fprintf(stderr, "Causing intentional segfault...\n");
    fflush(stderr);
    *((volatile unsigned int *) 0) = (unsigned int) 0xdeadbeefUL;
    abort();
#endif
}
wxString JScleanString(wxString given){ // cleans script string of unacceptable characters
    const wxString leftQuote    { _("\u201C")};
    const wxString rightQuote   {_("\u201D")};
//  const wxString quote        {_("\u0022")};
    const wxString quote        {_("\"")};
    const wxString accute       {_("\u00B4")};
    const wxString rightSquote  {_("\u2019")};    // right single quote
    const wxString leftSquote  {_("\u2018")};    // right single quote
//  const wxString apostrophe   {_("\u0027")};
    const wxString apostrophe   {_("\'")};
    const wxString ordinal      {_("\u00BA")};  // masculine ordinal indicator - like degree
    const wxString degree       {_("\u00B0")};
#ifndef __WXMSW__   // Don't try this one on Windows
    const wxString prime        {_("\u2032")};
    given.Replace(prime, apostrophe, true);
#endif  // __WXMSW__
    given.Replace(leftQuote, quote, true);
    given.Replace(rightQuote, quote, true);
    given.Replace(accute, apostrophe, true);
    given.Replace(rightSquote, apostrophe, true);
    given.Replace(leftSquote, apostrophe, true);
    given.Replace(ordinal, degree, true);
    return (given);
    }


// This function only needed with Windose
#ifdef __WXMSW__
wxString JScleanOutput(wxString given){ // clean unacceptable characters in output
    // As far as we know this only occurs with º symbol on Windows
    const wxString A_stringDeg{ _("\u00C2\u00b0")};    // Âº
    const wxString A_stringOrd{ _("\u00C2\u00ba")};    // Â ordinal
    given.Replace(A_stringDeg, _("\u00b0"), true);
    return (given);
    }
#endif

wxString getStringFromDuk(duk_context *ctx){
    // gets a string safely from top of duk stack and fixes º-symbol for Windose
    wxString string = duk_to_string(ctx, -1);
#ifdef __WXMSW__
    const wxString A_stringDeg{ _("\u00C2\u00b0")};    // Âº
    string.Replace(A_stringDeg, _("\u00b0"), true);
#endif
    return string;
    }


bool jsCheckFunctionMissing(duk_context *ctx, wxString function){
    // script set call back to this function - check it exists
    // returns true if missing, else false
    if (function == wxEmptyString) return false;
    if (!duk_get_prop_string(ctx, -1, function.c_str())){  // Does function exist in there?
            JS_control.display_error(ctx, "call-back function " + function + " not provided");
            duk_pop(ctx);   // pop off function or undefined
            return true;
            }
    duk_pop(ctx);   // pop off function or undefined
    return false;
    }

bool compileJS(wxString script, Console* console){
    // compiles and runs supplied JavaScript script and returns true if succesful
    wxString result;
    jsFunctionNameString_t function;
    bool more;   // set to true if follow-up functions to be run
    bool JSresult;
    void duk_extensions_init(duk_context *ctx);
    void ocpn_apis_init(duk_context *ctx);
    void JSduk_start_exec_timeout(void);
    void JSduk_clear_exec_timeout(void);
    
    JS_control.m_pJSconsole = console;  // remember our console
    
    // clean up fatefull characters in script
    script = JScleanString(script);
    script = script.mb_str(wxConvUTF8);

    duk_context *ctx = duk_create_heap(NULL, NULL, NULL, NULL, fatal_error_handler);  // create the context
    duk_extensions_init(ctx);  // register our own extensions
    ocpn_apis_init(ctx);       // register our API extensions
    more = false;      // start with no call-backs - will be set true in 'on' callback APIs

    duk_push_string(ctx, script.mb_str());   // load the script
    JS_control.init(ctx);
    JSduk_start_exec_timeout(); // start timer
    JSresult = duk_peval(ctx);    // run code
    JSduk_clear_exec_timeout(); // cancel time-out
    if (JS_control.m_stopScriptCalled) return (0);
    if (JSresult != DUK_EXEC_SUCCESS){
        JS_control.display_error(ctx, duk_safe_to_string(ctx, -1));
        duk_pop(ctx);   // pop off the error message
        return false;
        }
    result = getStringFromDuk(ctx);
    if (!JS_control.m_explicitResult) JS_control.m_result = result; // for when not explicit result
    duk_pop(ctx);  // pop result

    // if script has set up call-backs with nominated functions, check they exist
    duk_push_global_object(ctx);  // get our compiled script
    if (
        jsCheckFunctionMissing(ctx, JS_control.m_NMEAmessageFunction) ||
        jsCheckFunctionMissing(ctx, JS_control.m_dialog.functionName)
        ) return false;
    if(JS_control.waiting()) return true;
    else return false;
    }

