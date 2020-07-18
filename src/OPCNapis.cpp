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

extern JS_control_class JS_control;

static duk_ret_t getMessageNames(duk_context *ctx) {  // get message names seen
    // JS_control_class JS_control;
    
    duk_push_string(ctx, JS_control.getMessagesString());
    return 1;  // returns one arg
    }

static duk_ret_t onMessageName(duk_context *ctx) {  // to wait for message - save function to call
    int index = JS_control.messageIndex(wxString(duk_to_string(ctx, 1)));
    JS_control.m_messages[index].functionName = JS_control.getFunctionName();
    JS_control.m_JSactive = true;
    return 0;  // returns no arg
}

static duk_ret_t onNMEAsentence(duk_context *ctx) {  // to wait for NMEA message - save function to call
    if (JS_control.m_NMEAmessageFunction != wxEmptyString){
        // request already outstanding
        cout << "JavaScript onNMEAsentence called with call outstanding - this ignored\n";
        }
    else{
        JS_control.m_NMEAmessageFunction = JS_control.getFunctionName();
        JS_control.m_JSactive = true;
        }
    return 0;  // returns no arg
    }

static duk_ret_t getNavigation(duk_context *ctx) {  // gets latest navigation data and constructs navigation object
    wxString thisTime = wxDateTime(JS_control.m_positionFix.FixTime).FormatISOCombined().c_str();
    // ****  Indenting here shows stack depth - do not re-indent this section ****
    duk_push_object(ctx);
        duk_push_object(ctx);                               // start of SOG
            duk_push_number(ctx, JS_control.m_positionFix.Sog);
            duk_put_prop_literal(ctx, -2, "value");
            duk_push_string(ctx, "OCPN");
            duk_put_prop_literal(ctx, -2, "sentence");
            duk_push_string(ctx, thisTime);
            duk_put_prop_literal(ctx, -2, "timestamp");
            duk_put_prop_literal(ctx, -2, "speedOverGround"); // end of SOG
        duk_push_object(ctx);                               // start of COG
            duk_push_number(ctx, JS_control.m_positionFix.Cog);
            duk_put_prop_literal(ctx, -2, "value");
            duk_push_string(ctx, "OCPN");
            duk_put_prop_literal(ctx, -2, "sentence");
            duk_push_string(ctx, thisTime);
            duk_put_prop_literal(ctx, -2, "timestamp");
            duk_put_prop_literal(ctx, -2, "courseOverGround"); // end of COG
        duk_push_object(ctx);                                // start of position
            duk_push_object(ctx);                              // start of value
                duk_push_number(ctx, JS_control.m_positionFix.Lat);
                duk_put_prop_literal(ctx, -2, "latitude");
                duk_push_number(ctx, JS_control.m_positionFix.Lon);
                duk_put_prop_literal(ctx, -2, "longitude");
            duk_put_prop_literal(ctx, -2, "value");             // end of value
            duk_push_string(ctx, "OCPN");                       // start of nSats
            duk_put_prop_literal(ctx, -2, "sentence");
            duk_push_int(ctx, JS_control.m_positionFix.nSats);
            duk_put_prop_literal(ctx, -2, "numberSatellites");  // end of nSats
            duk_push_string(ctx, thisTime);
            duk_put_prop_literal(ctx, -2, "timestamp");
        duk_put_prop_literal(ctx, -2, "position");              // end of position
    duk_push_object(ctx);                                   // start of heading
        duk_push_object(ctx);                                // start of heading true
            duk_push_number(ctx, JS_control.m_positionFix.Hdt);
            duk_put_prop_literal(ctx, -2, "value");
            duk_push_string(ctx, "OCPN");
            duk_put_prop_literal(ctx, -2, "sentence");
            duk_push_string(ctx, thisTime);
            duk_put_prop_literal(ctx, -2, "timestamp");
            duk_put_prop_literal(ctx, -2, "headingTrue");
        duk_push_object(ctx);                                   // start of heading magnetic
            duk_push_number(ctx, JS_control.m_positionFix.Hdm);
            duk_put_prop_literal(ctx, -2, "value");
            duk_push_string(ctx, "OCPN");
            duk_put_prop_literal(ctx, -2, "sentence");
            duk_push_string(ctx, thisTime);
            duk_put_prop_literal(ctx, -2, "timestamp");
            duk_put_prop_literal(ctx, -2, "headingMagnetic");
        duk_put_prop_literal(ctx, -2, "heading");              // end of heading
    duk_push_object(ctx);                                   // start of variation
        duk_push_number(ctx, JS_control.m_positionFix.Var);
        duk_put_prop_literal(ctx, -2, "value");
        duk_push_string(ctx, "OCPN");
        duk_put_prop_literal(ctx, -2, "sentence");
        duk_push_string(ctx, thisTime);
        duk_put_prop_literal(ctx, -2, "timestamp");
        duk_put_prop_literal(ctx, -2, "variation");
    MAYBE_DUK_DUMP
    return 1;  // returns one arg
}

static duk_ret_t NMEApush(duk_context *ctx) {  // pushes NMEA sentence on stack out through OpenCPN
    // props to Dirk Smits for the checksum calculation lifted from his NMEAConverter plugin
    duk_idx_t nargs;  // number of args in call
    duk_ret_t result = 0;
    wxString sentence;  // the NMEA sentence
    auto ComputeChecksum{       // Using Lambda function here to keep it private to this function
        [](wxString sentence) {
             unsigned char calculated_checksum = 0;
             for(wxString::const_iterator i = sentence.begin()+1; i != sentence.end() && *i != '*'; ++i)
                 calculated_checksum ^= static_cast<unsigned char> (*i);

            return( wxString::Format(_("%02X"), calculated_checksum) );
        }
    };
    
    nargs = duk_get_top(ctx);
    if ((nargs == 1) &&  (duk_get_type(ctx, 0) == DUK_TYPE_STRING)){
        // we have a single string - good to go
        sentence = wxString(duk_to_string(ctx,0));
        sentence.Trim();
        // we will drop any existing checksum
        int starPos = sentence.Find("*");
        if (starPos != wxNOT_FOUND){ // yes there is one
            sentence = sentence.SubString(0, starPos-1); // truncate at * onwards
            }
        wxString Checksum = ComputeChecksum(sentence);
        sentence = sentence.Append(wxT("*"));
        sentence = sentence.Append(Checksum);
        sentence = sentence.Append(wxT("\r\n"));
        PushNMEABuffer(sentence);
        return(result);
    }
    else {
        cout << "JavaScript OCPNpushNMEA error: called without single string argument\n";
        return (result);
    }
}

static duk_ret_t sendMessage(duk_context *ctx) {  // sends message to OpenCPN
    duk_idx_t nargs;  // number of args in call
    duk_ret_t result = 0;
    wxString message_body = wxEmptyString;
    
    nargs = duk_get_top(ctx);
    if (nargs < 1){
        cout << "JavaScript OCPNsendMessage error: called without at least one argument\n";
        return (result);
        }
    if ((nargs >= 1) &&  (duk_get_type(ctx, 1) == DUK_TYPE_STRING)){
        // we have a message body
        message_body = wxString(duk_to_string(ctx,1));
        }
    wxString message_id = wxString(duk_to_string(ctx,0));
    message_id.Trim();
    SendPluginMessage(message_id, message_body);
    return(result);
}

static duk_ret_t OCPNonSeconds(duk_context *ctx) {  // call function after milliseconds elapsed
    duk_ret_t result = 0;
    duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
    TimeActions ourTimeAction;
    wxString argument = wxEmptyString;
    
    if ((int)JS_control.m_times.GetCount() > 10){
        cout << "JavaScript OCPNonSeconds error: already have 10 calls outstanding\n";
        return (result);
        }
    if ((nargs < 2) || (nargs > 3)){
        cout << "JavaScript OCPNonSeconds error: requires two or three arguments\n";
        return(result);
        }
    wxTimeSpan delay(0, 0, duk_to_number(ctx,1), 0);
    ourTimeAction.timeToCall = wxDateTime::Now() + delay;
    ourTimeAction.functionName = JS_control.getFunctionName();
    if (nargs > 2) argument = wxString(duk_to_string(ctx,2));  //if user included 3rd argument, use it
    ourTimeAction.argument = argument;
    JS_control.m_times.Add(ourTimeAction);  // add this action to array
    return(result);
    }

const unsigned int GPXbufferLength {1500};
char GPXbuffer[GPXbufferLength];
static duk_ret_t getARPgpx(duk_context *ctx) {  // get Active Route Point as GPX
    bool result = GetActiveRoutepointGPX( GPXbuffer, GPXbufferLength);
    duk_push_string(ctx, result ? GPXbuffer : "");
    return 1;  // returns one arg
    }

static duk_ret_t cancelAll(duk_context *ctx) {  // cancel all call-backs
    duk_ret_t result = 0;
    JS_control.clear();
    return(result);
    }

void ocpn_apis_init(duk_context *ctx) {
    duk_idx_t duk_push_c_function(duk_context *ctx, duk_c_function func, duk_idx_t nargs);
    duk_push_global_object(ctx);
    
    duk_push_string(ctx, "OCPNpushNMEA");
    duk_push_c_function(ctx, NMEApush, 1 /*number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNsendMessage");
    duk_push_c_function(ctx, sendMessage, DUK_VARARGS /*variable number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNgetMessageNames");
    duk_push_c_function(ctx, getMessageNames, 0 /* no args */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "OCPNonMessageName");
    duk_push_c_function(ctx, onMessageName, 2 /* args */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "OCPNonNMEAsentence");
    duk_push_c_function(ctx, onNMEAsentence, 1 /* args */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNonSeconds");
    duk_push_c_function(ctx, OCPNonSeconds, DUK_VARARGS);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNgetARPgpx");
    duk_push_c_function(ctx, getARPgpx, 0 /* no args */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "OCPNgetNavigation");
    duk_push_c_function(ctx, getNavigation, 0 /* no args */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNcancelAll");
    duk_push_c_function(ctx, cancelAll, 0 /* no args */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_pop(ctx);
}
