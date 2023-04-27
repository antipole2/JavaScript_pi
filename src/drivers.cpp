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
#include "duktape.h"
#include <iostream>
#include "JavaScriptgui_impl.h"
#include <unordered_map>
#include <vector>
#include <wx/arrstr.h>
//#include "stream_events.h"

extern JavaScript_pi* pJavaScript_pi;
void throwErrorByCtx(duk_context *ctx, wxString message);
Console *findConsoleByCtx(duk_context *ctx);
wxString extractFunctionName(duk_context *ctx, duk_idx_t idx);

const char* errorStrings[] = {
	"RESULT_COMM_NO_ERROR",
	"Invalid driver handle",
	"RESULT_COMM_INVALID_PARMS",
	"RESULT_COMM_TX_ERROR",
	"RESULT_COMM_REGISTER_GATEWAY_ERROR"
	};
    
duk_ret_t getDriverHandles(duk_context* ctx){
	std::vector<DriverHandle> handlesVector = GetActiveDrivers();
	duk_idx_t arr_idx = duk_push_array(ctx);
	int count = handlesVector.size();
    if (count > 0){
        for (int i = 0; i < count; i++){
            duk_push_string(ctx, handlesVector[i].c_str());
            duk_put_prop_index(ctx, arr_idx, i);
            }
        }
	return 1;
	}
	
//std::unordered_map<std::string, std::string> attributes;
duk_ret_t getDriverAttributes(duk_context* ctx){
	// OCPNgetDriverAttributes(driverHandle)
	DriverHandle handle;
	
	duk_require_string(ctx, 0);
	handle = duk_get_string(ctx, 0);
	duk_pop(ctx);		
	std::unordered_map<std::string, std::string> attributes = GetAttributes(handle);
	duk_push_object(ctx);

	for (auto it = attributes.begin(); it != attributes.end(); it++){
		duk_push_string(ctx, it->first.c_str());
		duk_push_string(ctx, it->second.c_str());
		duk_put_prop(ctx, -3);
		}
	return 1;			
	}

/*
duk_ret_t writeDriver(duk_context* ctx){
	// OCPNwriteDriver(driverHandle, payload)
	DriverHandle handle;
	const std::shared_ptr <std::vector<uint8_t>> payload {};
	const char * data;
	duk_size_t dataLength;	// length of payload
	CommDriverResult result;
	
	handle = (driverHandle) duk_get_string(ctx, 0);
	data = duk_get_lstring(ctx, 1, &dataLength);
	duk_pop_2(ctx);
	if (dataLength == 0) throwErrorByCtx(ctx, "OCPNwriteDriver error");
	payload->resize(dataLength);
	for (int i = 0; i < dataLength; i++) payload->at(i) = (uint8_t)data[i];
	result = WriteCommDriver(handle, payload);
	if (result == RESULT_COMM_NO_ERROR) return 0;
	else throwErrorByCtx(ctx, wxString("OCPNWriteDriver ", errorStrings[result]));	
	}
*/
	
static duk_ret_t onNavData(duk_context *ctx) {  // callback with nav data
	// OCPNonNavData(function)
	// NB presently using a single event - need to allocate so can have several calls open
	Console *pConsole = findConsoleByCtx(ctx);
	duk_require_c_function(ctx, 0);
	jsFunctionNameString_t functionName = extractFunctionName(ctx, 0);
	duk_pop(ctx);

//	listener.pConsole = pConsole;
//	listener.functionToCall = functionName;
//	Bind(EVT_NAVDATA, [&](JSnavdataEvt) { pJavaScript_pi->HandleNavData(ev);}
	return 0;
	}
	

void register_drivers(duk_context *ctx){
    duk_push_global_object(ctx);
    
    duk_push_string(ctx, "OCPNgetActiveDriverHandles");
    duk_push_c_function(ctx, getDriverHandles, 0 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "OCPNgetDriverAttributes");
    duk_push_c_function(ctx, getDriverAttributes, 1 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

#if 0
    duk_push_string(ctx, "OCPNwriteDriver");
    duk_push_c_function(ctx, writeDriver, 2 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
#endif
        
    duk_push_string(ctx, "OCPNonNavData");
    duk_push_c_function(ctx, onNavData, 1 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE);

    };

