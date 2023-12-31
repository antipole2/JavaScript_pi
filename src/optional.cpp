/**************************************************************************
* Project:  OpenCPN
* Purpose:  JavaScript Plugin
* Author:   Tony Voss 25/02/2021
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
#include "wx/wx.h"
#include "wx/fileconf.h"
#include "duktape.h"
// #include <dukglue/dukglue.h>
#include <iostream>
#include "JavaScriptgui_impl.h"
//#include "wx/arrimpl.cpp"

extern JavaScript_pi* pJavaScript_pi;
extern Console* pConsoleBeingTimed;
void throwErrorByCtx(duk_context *ctx, wxString message);
Console* findConsoleByName(wxString name);

duk_ret_t console_get_details(duk_context *ctx){
	// return structure giving details of Console or parent frame
	// if 2nd present and is true, minimise minSize, else set as normal
	wxSize size, minSize, clientSize;
	wxPoint pos;
	wxPoint screenToFrame(wxPoint);
	
	duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
	if ((nargs != 1) && (nargs != 2)) throwErrorByCtx(ctx, "consoleGetDetails number of args not 1 or 2");
	wxString name = duk_get_string(ctx, 0);
	Console* pConsole = findConsoleByName(name);
	if (!pConsole) throwErrorByCtx(ctx, "consoleDetails console " + name + " does not exist");
	pos = screenToFrame(pConsole->GetPosition());
	size = pConsole->GetSize();
	minSize = pConsole->GetMinSize();
	clientSize = pConsole->GetClientSize();
	name = pConsole->mConsoleName;
	if (nargs == 2){
		if (duk_get_boolean(ctx, 1)){	// set minSize small
			pConsole->SetMinSize(wxSize(1,1));
			minSize = pConsole->GetMinSize();
			}
		else pConsole->setConsoleMinClientSize();
		minSize = pConsole->GetMinSize();
		duk_pop(ctx);	//pop off arg 2
		}
	duk_pop(ctx);	//pop off arg 1
    
	// indentation here shows stack depth
    duk_push_object(ctx); // construct the details object
        duk_push_string(ctx, name);
    		duk_put_prop_literal(ctx, -2, "name");
	    duk_push_object(ctx); // construct the position object
		    duk_push_int(ctx, pos.x);
		    	duk_put_prop_literal(ctx, -2, "x");
		    duk_push_int(ctx, pos.y);
		    	duk_put_prop_literal(ctx, -2, "y");
		    duk_put_prop_literal(ctx, -2, "position");
		duk_push_object(ctx); // construct the size object
		    duk_push_int(ctx, size.x);
		    	duk_put_prop_literal(ctx, -2, "x");
		    duk_push_int(ctx, size.y);
		    	duk_put_prop_literal(ctx, -2, "y");
		    duk_put_prop_literal(ctx, -2, "size");
		duk_push_object(ctx); // construct the minSize object
		    duk_push_int(ctx, minSize.x);
		    	duk_put_prop_literal(ctx, -2, "x");
		    duk_push_int(ctx, minSize.y);
		    	duk_put_prop_literal(ctx, -2, "y");
		    duk_put_prop_literal(ctx, -2, "minSize");
		duk_push_object(ctx); // construct the clientSize object
		    duk_push_int(ctx, clientSize.x);
		    	duk_put_prop_literal(ctx, -2, "x");
		    duk_push_int(ctx, clientSize.y);
		    	duk_put_prop_literal(ctx, -2, "y");
		    duk_put_prop_literal(ctx, -2, "clientSize");		    		    
	return 1;
	};
	
duk_ret_t console_exists(duk_context *ctx){
    //test if console exists
    duk_require_string(ctx, 0);
    Console* pConsole;
    wxString name = duk_get_string(ctx, 0);
    duk_pop(ctx);
    pConsole = findConsoleByName(name);
    duk_push_boolean(ctx, (pConsole != nullptr)?true:false);
    return 1;
    };

duk_ret_t console_busy(duk_context *ctx){
    //test if console busy
    duk_require_string(ctx, 0);
    Console* pConsole;
    wxString name = duk_get_string(ctx, 0);
    duk_pop(ctx);
    pConsole = findConsoleByName(name);
    if (!pConsole) throwErrorByCtx(ctx, "Console " + name + " does not exist");
    duk_push_boolean(ctx, pConsole->isBusy());
    return 1;
    };

duk_ret_t console_clearOutput(duk_context *ctx){
    // clear output pane
    Console* pConsole;
    Console *findConsoleByCtx(duk_context *ctx);
    duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
    if (nargs>1) throwErrorByCtx(ctx, "consoleClearOutput bad call (too many arguments)");
    if (nargs == 0){    // clearing our own output
        pConsole = findConsoleByCtx(ctx);
        }
    else {
        duk_require_string(ctx, 0);
        wxString name = duk_get_string(ctx, 0);
        duk_pop(ctx);
        pConsole = findConsoleByName(name);
        if (!pConsole) throwErrorByCtx(ctx, "Console " + name + " does not exist");
        }
    pConsole->m_Output->ClearAll();
    return 0;
    };

duk_ret_t console_getOutput(duk_context *ctx){
    // clear output pane
    duk_require_string(ctx, 0);
    Console* pConsole;
    Console *findConsoleByCtx(duk_context *ctx);
    duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
    if (nargs>1) throwErrorByCtx(ctx, "consoleClearOutput bad call (too many arguments)");
    if (nargs == 0){    // clearing our own output
        pConsole = findConsoleByCtx(ctx);
        }
    else {
        duk_require_string(ctx, 0);
        wxString name = duk_get_string(ctx, 0);
        duk_pop(ctx);
        pConsole = findConsoleByName(name);
        if (!pConsole) throwErrorByCtx(ctx, "Console " + name + " does not exist");
        }
    duk_push_string(ctx, pConsole->m_Output->GetText());
    return 1;
    };

duk_ret_t console_add(duk_context *ctx){
    // add new console
    wxString checkConsoleName(wxString name, Console* pConsole);
    Console* pConsole;
    int x, y;
    wxString outcome;
    
    duk_require_string(ctx, 0);
    wxString name = duk_get_string(ctx, 0);
    duk_pop(ctx);
    outcome = checkConsoleName(name, nullptr);
    if (outcome !="") throwErrorByCtx(ctx, outcome);
    pConsole = new Console(pJavaScript_pi->m_parent_window, name);
    pConsole->CenterOnScreen();
    pConsole->setConsoleMinClientSize();
    // to prevent multiple new consoles hiding eachother completely, we will shift each randomly
    pConsole->GetPosition(&x, &y);
    x += - 25 + rand()%50; y += - 25 + rand()%50;
    pConsole->SetPosition(wxPoint(x, y));
    pConsole->Show();
    return 0;
    };

duk_ret_t console_close(duk_context* ctx){
    Console *findConsoleByCtx(duk_context *ctx);
    duk_require_string(ctx, 0);
    wxString name = duk_get_string(ctx, 0);
    duk_pop(ctx);
    Console* pConsole = findConsoleByName(name);
    if (!pConsole) throwErrorByCtx(ctx, "Console " + name + " does not exist");
    if (pConsole == findConsoleByCtx(ctx)) throwErrorByCtx(ctx, "Console " + name + " cannot close yourself");
    if (pConsole->mpMessageDialog != nullptr){ 
    	pConsole->mpMessageDialog->EndModal(-1);	// close modal with special outcome
    	}
	else {
		pConsole->clearCallbacks();
		}
	pConsole->bin();
    return 0;
    }

duk_ret_t console_load(duk_context* ctx){
    // consoleLoad(consoleName,  script) into console
    wxString fileString, script, consoleName, outcome;
    wxString getTextFile(wxString fileString, wxString* fetched);
	wxString resolveFileName(wxString inputName, Console* pConsole, FileOptions options);
    wxString JScleanString(wxString given);
    Console* pConsole;
    
    duk_require_string(ctx, 0);
    duk_require_string(ctx, 1);
    consoleName = wxString(duk_get_string(ctx, 0));
    fileString = wxString(duk_get_string(ctx, 1));
    duk_pop_2(ctx);
    pConsole = findConsoleByName(consoleName);
    if (!pConsole) throwErrorByCtx(ctx, "Console " + consoleName + " does not exist");
    if (pConsole == pConsoleBeingTimed)
        pConsole->throw_error(pConsoleBeingTimed->mpCtx, "Load console " + pConsole->mConsoleName + " cannot load into own console");
    if (pConsole->mpCtx) pConsoleBeingTimed->throw_error(pConsoleBeingTimed->mpCtx, "Load console " + pConsole->mConsoleName + " is busy");
    if (fileString.EndsWith(".js")){   // we are to try and load a file
        fileString = resolveFileName(fileString, pConsole, MUST_EXIST);
        outcome = getTextFile( fileString, &script);
        if (outcome != wxEmptyString)  throwErrorByCtx(ctx, outcome);
        script = JScleanString(script);
        pConsole->m_Script->ClearAll();
        pConsole->m_Script->AppendText(	script + "\n");
        pConsole->m_fileStringBox->SetValue(fileString);
        pConsole->auto_run->Show();
        }
    else {
    	pConsole->m_Script->SetText(fileString);   // we were passed a script
    	pConsole->m_fileStringBox->SetValue(wxEmptyString);
        pConsole->auto_run->Hide();
		}
    pConsole->auto_run->SetValue(false);
    return 0;
    }

duk_ret_t console_run(duk_context* ctx){
    // consoleRun(consoleName,  brief)
    wxString consoleName, brief;
    Console* pConsole;
    bool haveBrief {false};
    duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
    
    duk_require_string(ctx, 0);
    consoleName = wxString(duk_get_string(ctx, 0));
    if (nargs>1){
        duk_require_string(ctx, 1);
        brief = wxString(duk_get_string(ctx, 1));
        haveBrief = true;
        duk_pop(ctx);
        }
    duk_pop(ctx);
    pConsole = findConsoleByName(consoleName);
    if (pConsole == pConsoleBeingTimed)
        pConsole->throw_error(pConsoleBeingTimed->mpCtx, "Run console " + pConsole->mConsoleName + " cannot run own console");
    if (pConsole->mpCtx) pConsoleBeingTimed->throw_error(pConsoleBeingTimed->mpCtx, "Run console " + pConsole->mConsoleName + " is busy");
    pConsole->mBrief.reply = false;
    if (haveBrief){
        pConsole->mBrief.theBrief = brief;
        pConsole->mBrief.hasBrief = true;
        }
    pConsole->mscriptToBeRun = true;
    TRACE(5, "console_run about to run " + consoleName + "with haveBrief " + (haveBrief?("true and brief: " + pConsole->mBrief.theBrief):"false"));
    pConsole->CallAfter(&Console::doRunCommand, pConsole->mBrief);
    return 0;
    }
    
duk_ret_t onConsoleResult(duk_context* ctx){
    // onConsoleResult(consoleName, function [, brief])
    Console* findConsoleByCtx(duk_context* ctx);
    wxString extractFunctionName(duk_context *ctx, duk_idx_t idx);
    wxString consoleName, functionName, brief;
    Console* pConsole, *pCallingConsole;
    bool haveBrief {false};
    duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
    pCallingConsole = pConsoleBeingTimed;
 
    if (nargs < 2) pCallingConsole->throw_error(ctx, "onConsoleResult called with insufficient args");
    duk_require_string(ctx, 0);
    duk_require_object(ctx, 1);
    consoleName = wxString(duk_get_string(ctx, 0));
    functionName = extractFunctionName(ctx, 1);
    if (nargs>2){
    	brief = duk_get_string(ctx, 2);
    	haveBrief = true;
    	duk_pop(ctx);   // the third arg
    	}
    duk_pop_2(ctx); // first and second args
    pConsole = findConsoleByName(consoleName);
    if (pConsole->mRunningMain || pConsole->isWaiting())
        pCallingConsole->throw_error(ctx, "onConsoleResult target console " + pConsole->mConsoleName + " is busy");
    if (pCallingConsole->mConsoleRepliesAwaited > MAX_TIMERS){
        pCallingConsole->throw_error(ctx, "onConsoleResult error: already have maximum callbacks outstanding");
        }
    // OK - ready to go
    pConsole->mBrief.briefingConsoleName = pCallingConsole->mConsoleName;
    pConsole->mBrief.reply = true;
    pConsole->mBrief.function = functionName;
    if (haveBrief){
        pConsole->mBrief.theBrief = brief;
        pConsole->mBrief.hasBrief = true;
        }
    else pConsole->mBrief.hasBrief = false;
    pConsole->mscriptToBeRun = true;
    pCallingConsole->mConsoleRepliesAwaited++;
    pCallingConsole->setWaiting();
    TRACE(4, "onConsoleResult about to run " + consoleName + " with haveBrief " + (haveBrief?("true and brief: " + pConsole->mBrief.theBrief):"false"));
    pConsole->CallAfter(&Console::doRunCommand, pConsole->mBrief);
    return 0;
    }

void register_console(duk_context *ctx){
    duk_push_global_object(ctx);
    
    duk_push_string(ctx, "consoleExists");
    duk_push_c_function(ctx, console_exists, 1 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "consoleBusy");
    duk_push_c_function(ctx, console_busy, 1 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "consoleClearOutput");
    duk_push_c_function(ctx, console_clearOutput, DUK_VARARGS /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);;
    
    duk_push_string(ctx, "consoleGetOutput");
    duk_push_c_function(ctx, console_getOutput, DUK_VARARGS /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "consoleAdd");
    duk_push_c_function(ctx, console_add, 1 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "consoleClose");
    duk_push_c_function(ctx, console_close, 1 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "consoleLoad");
    duk_push_c_function(ctx, console_load, 2 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "consoleRun");
    duk_push_c_function(ctx, console_run, DUK_VARARGS /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "onConsoleResult");
    duk_push_c_function(ctx, onConsoleResult, DUK_VARARGS /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "consoleDetails");
    duk_push_c_function(ctx, console_get_details, DUK_VARARGS /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    };

void register_drivers(duk_context *ctx);

bool loadComponent(duk_context *ctx, wxString name) {
    bool result {false};
    
    if (name == "Consoles") {register_console(ctx); result = true;}
    if (name == "Drivers") {register_drivers(ctx); result = true;}
    
#ifdef SOCKETS
    void register_sockets(duk_context *ctx);
	if (name == "Sockets") {register_sockets(ctx); result = true;}
#endif

#ifdef IPC
	void register_tcp(duk_context *ctx);
	if (name == "IPC") {register_tcp(ctx); result = true;}
#endif
	
    return result;
    };
