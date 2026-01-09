/**************************************************************************
* Project:  OpenCPN
* Purpose:  JavaScript Plugin
* Author:   Tony Voss 25/02/2021
*
* Copyright Ⓒ 2025 by Tony Voss
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, under which
* you must including a copy of these terms in all copies
* https://www.gnu.org/licenses/gpl-3.0.en.html
***************************************************************************
*/

#if 0	// not being used at presnt
#include <iostream>
#include "wx/wx.h"
#include "duktape.h"
#include "JavaScriptgui_impl.h"
#include <unordered_map>
#include <vector>
#include <wx/arrstr.h>

extern JavaScript_pi* pJavaScript_pi;
void throwErrorByCtx(duk_context *ctx, wxString message);

const char* errorStrings[] = {
	"RESULT_COMM_NO_ERROR",
	"Invalid driver handle",
	"RESULT_COMM_INVALID_PARMS",
	"RESULT_COMM_TX_ERROR",
	"RESULT_COMM_REGISTER_GATEWAY_ERROR"
	};
    


duk_ret_t writeDriver(duk_context* ctx){
	wxString NMEAchecksum(wxString sentence);
	DriverHandle handle;
	wxString sentence;
	
	handle = (DriverHandle) duk_get_string(ctx, 0);
	sentence = wxString(duk_to_string(ctx,1));
	duk_pop_2(ctx);
	
	// Determine protocol
	std::unordered_map<std::string, std::string> attributes = GetAttributes(handle);	
  	auto protocol_it = attributes.find("protocol");
  	if (protocol_it == attributes.end()) throwErrorByCtx(ctx, wxString(wxString::Format("writeDriver error: %s\n", "Handle does not have protocol")));
    wxString protocol = protocol_it->second;
	if (!protocol.compare("nmea0183")){ // if NMEA0183, clean up sentence and add checksum
		sentence.Trim();        
		// we will drop any existing checksum
		int starPos = sentence.Find("*");
		if (starPos != wxNOT_FOUND){ // yes there is one
			sentence = sentence.SubString(0, starPos-1); // truncate at * onwards
			}
		if  (!(((sentence[0] == '$') || (sentence[0] == '!')) && (sentence[6] == ',')))
			throwErrorByCtx(ctx, "OCPNwriteDriverNMEA sentence does not start $....., or !.....,");
		sentence = sentence.Append("*" + NMEAchecksum(sentence) + "\r\n");
		}
	else if (!protocol.compare("SignalK"))((void)0);	// no action required	
	else throwErrorByCtx(ctx, "OCPNWriteNMEAdriver error: protocol not NMEA0183 or Signal K");

	auto payload = make_shared<std::vector<uint8_t>>();
	for (const auto& ch : sentence) payload->push_back(ch);
	CommDriverResult outcome = WriteCommDriver(handle, payload);
	if (outcome == RESULT_COMM_NO_ERROR) duk_push_string(ctx, "OK");
	else throwErrorByCtx(ctx, wxString(wxString::Format("OCPNWriteDriver error: %s\n", errorStrings[outcome])));
	return 1;	
	}


void register_drivers(duk_context *ctx){
    duk_push_global_object(ctx);
    

    duk_push_string(ctx, "OCPNwriteDriver");
    duk_push_c_function(ctx, writeDriver, 2 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    };

#endif