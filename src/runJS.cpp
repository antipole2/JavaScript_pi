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

extern JS_control_class JS_control;

// Provide for handling fatal Javascript error
    void fatal_error_handler(void *udata, const char *msg) {
    (void) udata;  /* ignored in this case, silence warning */

    /* Note that 'msg' may be NULL. */
    wxMessageBox((msg ? msg : "(no message)"), "Fatal JavaScript error");
    fprintf(stderr, "Causing intentional segfault...\n");
    fflush(stderr);
    *((volatile unsigned int *) 0) = (unsigned int) 0xdeadbeefUL;
    abort();
}

void jsMessage(duk_context *ctx, wxTextAttr textColour, wxString messageAttribute, wxString message){
    JS_control.m_pJSconsole->Show(); // make sure console is visible
    wxTextCtrl* output_window = JS_control.m_pJSconsole->m_Output;
    output_window->SetDefaultStyle(wxTextAttr(textColour));
    output_window->AppendText("JavaScript ");
    output_window->AppendText(messageAttribute);
    output_window->AppendText(message);
    output_window->AppendText("\n");
    output_window->SetDefaultStyle(wxTextAttr(*wxBLACK));
}

bool compileJS(wxString script, Console* console){
    // compiles and runs supplied JavaScript script and returns true if succesful
    wxString result;
    jsFunctionNameString_t function;
    bool more, errors;   // set to true if follow-up functions to be run
    void duk_extensions_init(duk_context *ctx);
    void ocpn_apis_init(duk_context *ctx);
    
    JS_control.m_pJSconsole = console;  // remember our console
    wxTextCtrl* output_window = console->m_Output;    // where we want stdout to go

    // clean up fatefull characters in script
    script.Replace(wxString("“"),wxString('"'), true);  // opening double quote
    script.Replace(wxString("”"),wxString('"'), true);  // closing double quote
    script.Replace(wxString("’"),wxString("'"), true);  // fix opening prime ’
    script.Replace(wxString("’"),wxString("'"), true);  // fix closing prime ’

    duk_context *ctx = duk_create_heap(NULL, NULL, NULL, NULL, fatal_error_handler);  // create the context
    JS_control.m_pctx = ctx;  // remember it
    wxStreamToTextRedirector redirect(output_window);  // redirect sdout to our window pane
    duk_extensions_init(ctx);  // register our own extensions
    ocpn_apis_init(ctx);       // register our API extensions
    more = false;      // start with no call-backs - will be set true in 'on' callback APIs
    errors = false;
    JS_control.m_runCompleted = false;

    duk_push_string(ctx, script.mb_str());   // load the script
    if (duk_peval(ctx) != 0) { // compile and run - how did it go?
        jsMessage(ctx, *wxRED, _(""), duk_safe_to_string(ctx, -1));
        return false;
        }
    jsMessage(ctx, *wxBLUE, _("result: "), duk_safe_to_string(ctx, -1));
     duk_pop(ctx);  /* pop result/error */

    // check if script has set up call backs with nominated functions
    size_t count = JS_control.m_messages.GetCount();
    if (count > 0){
        more = true;
        for (unsigned int i = 0; i < count; i++){
            function = JS_control.m_messages[i].functionName;
            if (function != wxEmptyString){
                // script set call back to this function - check it exists
                duk_push_global_object(ctx);  // ready with our compiled script
                if (!duk_get_prop_string(ctx, -1, function.c_str())){  // Does function exist in there?
                    jsMessage(ctx, *wxRED, _("function ") + function + " ", duk_safe_to_string(ctx, -1));
                    errors = true;   // will not proceed with call-backs
                    }
                duk_pop(ctx);
                }
        }
    }
    // check if timers set
    count = JS_control.m_times.GetCount();
    if ((count > 0)){
        more = true;
        for (unsigned int i = 0; i < count; i++){
            function = JS_control.m_times[i].functionName;
            if (function != wxEmptyString){
                // script set call back to this function - check it exists
                duk_push_global_object(ctx);  // ready with our compiled script
                if (!duk_get_prop_string(ctx, -1, function.c_str())){  // Does function exist?
                    jsMessage(ctx, *wxRED, _("Timed function ") + function + " ", duk_safe_to_string(ctx, -1));
                    errors = true;   // will not proceed with call-backs
                    }
                duk_pop(ctx);
                }
            }
        }
    
    // check if NMEA call back set
    function = JS_control.m_NMEAmessageFunction;
    if (function != wxEmptyString){
        more = true;
        // script set call back to this function - check it exists
        duk_push_global_object(ctx);  // ready with our compiled script
        if (!duk_get_prop_string(ctx, -1, function.c_str())){  // Does function exist?
            jsMessage(ctx, *wxRED, _("function ") + function + " ", duk_safe_to_string(ctx, -1));
            errors = true;   // will not proceed with call-backs
            }
        duk_pop(ctx);
        }


    if (errors) more = false;
    JS_control.m_JSactive = more;  // remember for outside here
    return(more);
}

