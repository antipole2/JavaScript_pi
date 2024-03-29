/**************************************************************************
* Project:  OpenCPN
* Purpose:  JavaScript Plugin
* Author:   Tony Voss 16/05/2020
*
* Copyright (C) 2022 by Tony Voss
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, under which
* you must including a copy of these terms in all copies
* https://www.gnu.org/licenses/gpl-3.0.en.html
***************************************************************************
* updated Oct 2021 to use extended waypoint API 17
*/

#include "duktape.h"
#include "JavaScript_pi.h"
#include "JavaScriptgui_impl.h"
#include <wx/listimpl.cpp>
#include <stdarg.h>
#include <unordered_map>
#include <vector>

WX_DEFINE_LIST(Plugin_HyperlinkList);
WX_DEFINE_LIST(Plugin_WaypointList);		// used by API16 for tracks
WX_DEFINE_LIST(Plugin_WaypointExList);		// used for API17 for waypoints and routes

const char* driverErrorStrings[] = {
	"RESULT_COMM_NO_ERROR",
	"Invalid driver handle",
	"RESULT_COMM_INVALID_PARMS",
	"RESULT_COMM_TX_ERROR",
	"RESULT_COMM_REGISTER_GATEWAY_ERROR",
	"RESULT_COMM_REGISTER_PGN_ERROR"
	};

/* here define parameters for the OPCNgetGUID options */
/* Not using this at present
 enum {
    NEW,
    WAYPOINTS_ARRAY,
    WAYPOINT_SELECTED,
    ROUTE_SELECTED,
    END_OF_OPCN_GETGUID_OPTIONS
    
};
wxString getNames[] = {
    "NEW",
    "WAYPOINTS_ARRAY",
    "WAYPOINT_SELECTED",
    "ROUTE_SELECTED",
};
 */
/* end of OPCNgetGUID options */

Console *findConsoleByCtx(duk_context *ctx);
wxString extractFunctionName(duk_context *ctx, duk_idx_t idx);
void throwErrorByCtx(duk_context *ctx, wxString message);
extern JavaScript_pi *pJavaScript_pi;


wxString getContextDump(duk_context *ctx){ // return duktape context dump as string
    duk_push_context_dump(ctx);
    wxString result = duk_to_string(ctx, -1);
    duk_pop(ctx);
    return result;
    }
    
/*
void checkHandle(duk_context *ctx, DriverHandle handle, wxString protocol, wxString APIname){	// checks for correct driver
	    std::unordered_map<std::string, std::string> attributes = GetAttributes(handle);
	    if(attributes.empty()) throwErrorByCtx(ctx, wxString::Format("%s: No such handle %s", APIname, handle));
    	auto protocol_it = attributes.find("protocol");
    	if (protocol_it == attributes.end()) throwErrorByCtx(ctx, wxString::Format("%s: Handle %s does not have a protocol", APIname, handle));
	    wxString driverProtocol = protocol_it->second;
		if (protocol.compare(driverProtocol) != 0) throwErrorByCtx(ctx, wxString::Format("%s: Handle has protocol %s not %s", APIname, driverProtocol, protocol));
		}
*/
		
wxString getHandleProtocol(duk_context *ctx, DriverHandle handle, wxString APIname){
		std::unordered_map<std::string, std::string> attributes = GetAttributes(handle);
	    if(attributes.empty()) throwErrorByCtx(ctx, wxString::Format("%s: No such handle %s", APIname, handle));
    	auto protocol_it = attributes.find("protocol");
    	if (protocol_it == attributes.end()) throwErrorByCtx(ctx, wxString::Format("%s: Handle %s does not have a protocol", APIname, handle));
	   return protocol_it->second;
	}

PlugIn_Waypoint_Ex * js_duk_to_opcn_waypoint(duk_context *ctx){
    // returns an opcn waypoint constructed from js waypoint on top of duk stack
    duk_size_t listLength, i;
    wxString dump;
    void throwErrorByCtx(duk_context *ctx, wxString message);
    PlugIn_Waypoint_Ex *p_waypoint = new PlugIn_Waypoint_Ex;
    // indenting here represents stack hight - do not reformat
    if (duk_get_prop_string(ctx, -1, "position")){
        if (!duk_get_prop_string(ctx, -1, "latitude")) throwErrorByCtx(ctx, "addSingleWaypoint error: no latitude");
            p_waypoint->m_lat = duk_to_number(ctx, -1);
            duk_pop(ctx);
        if (!duk_get_prop_string(ctx, -1, "longitude"))throwErrorByCtx(ctx, "addSingleWaypoint error: no longitude");
            p_waypoint->m_lon = duk_to_number(ctx, -1);
            duk_pop(ctx);
        duk_pop(ctx);   // done with position
        }
    else throwErrorByCtx(ctx, "addSingleWaypoint error: no position");
    if (duk_get_prop_string(ctx, -1, "markName")) p_waypoint->m_MarkName = duk_to_string(ctx, -1);
        duk_pop(ctx);
    duk_get_prop_string(ctx, -1, "GUID");
        p_waypoint->m_GUID = duk_to_string(ctx, -1);
        if ((p_waypoint->m_GUID == "undefined") || wxIsEmpty(p_waypoint->m_GUID)) p_waypoint->m_GUID = GetNewGUID();
        duk_pop(ctx);
    if (wxIsEmpty(p_waypoint->m_GUID)) p_waypoint->m_GUID = GetNewGUID();  // if no GUID, provide one
    if (duk_get_prop_string(ctx, -1, "description")) p_waypoint->m_MarkDescription = duk_to_string(ctx, -1);
        duk_pop(ctx);
    if (duk_get_prop_string(ctx, -1, "iconName")) p_waypoint->IconName = duk_to_string(ctx, -1);
        duk_pop(ctx);
    if (duk_get_prop_string(ctx, -1, "minScale")) p_waypoint->scamin = duk_to_number(ctx, -1);
        duk_pop(ctx);
    if (duk_get_prop_string(ctx, -1, "useMinScale")) p_waypoint->b_useScamin = duk_to_boolean(ctx, -1);
        duk_pop(ctx);
    if (duk_get_prop_string(ctx, -1, "isVisible")) p_waypoint->IsVisible = duk_to_boolean(ctx, -1);
        duk_pop(ctx);
    if (duk_get_prop_string(ctx, -1, "isNameVisible")) p_waypoint->IsNameVisible = duk_to_boolean(ctx, -1);
        duk_pop(ctx);
	if ( duk_get_prop_string(ctx, -1, "nRanges")) p_waypoint->nrange_rings = duk_to_int(ctx, -1);
        duk_pop(ctx);            
    if (duk_get_prop_string(ctx, -1, "rangeRingSpace")) p_waypoint->RangeRingSpace = duk_to_number(ctx, -1);
        duk_pop(ctx);    
    if (duk_get_prop_string(ctx, -1, "rangeRingColour")) p_waypoint->RangeRingColor.Set(duk_to_string(ctx, -1));
        duk_pop(ctx);
    if (duk_get_prop_string(ctx, -1, "creationDateTime")) p_waypoint->m_CreateTime.Set((time_t)(duk_to_number(ctx, -1))); // seconds
        duk_pop(ctx);    
    // hyperlink processing
    p_waypoint->m_HyperlinkList = new Plugin_HyperlinkList; // need to initialise to empty list
    if (duk_get_prop_string(ctx, -1, "hyperlinkList")){
        // we do have a hyperlinkList - this is hard work!
        if (duk_is_array(ctx, -1) && (listLength = duk_get_length(ctx, -1)) > 0){   // only process if non-empty array of hyperlinks
            duk_to_object(ctx, -1);
                for (i = 0; i < listLength; i++){
                    Plugin_Hyperlink *p_hyperlink{new Plugin_Hyperlink};
                    duk_get_prop_index(ctx, -1, (unsigned int) i);
                        duk_get_prop_string(ctx, -1, "description");
                        p_hyperlink->DescrText = duk_get_string(ctx, -1);
                        duk_pop(ctx);
                    duk_get_prop_string(ctx, -1, "link");
                        p_hyperlink->Link = duk_get_string(ctx, -1);
                        duk_pop(ctx);
                    duk_get_prop_string(ctx, -1, "type");
                        duk_pop(ctx);
                    p_waypoint->m_HyperlinkList->Append(p_hyperlink);
                    duk_pop(ctx);
                }
            }
        }
        duk_pop(ctx);
    return(p_waypoint);
    }
     
    PlugIn_Waypoint * js_duk_to_opcn_trackpoint(duk_context *ctx){
    // returns an opcn trackpoint constructed from js waypoint on top of duk stack
    void throwErrorByCtx(duk_context *ctx, wxString message);
        
    PlugIn_Waypoint *p_waypoint = new PlugIn_Waypoint;
    // indenting here represents stack hight - do not reformat
        if (duk_get_prop_string(ctx, -1, "position")){
            if (!duk_get_prop_string(ctx, -1, "latitude")) throwErrorByCtx(ctx, "add/upateTrack error: no latitude");
                p_waypoint->m_lat = duk_to_number(ctx, -1);
                duk_pop(ctx);
            if (!duk_get_prop_string(ctx, -1, "longitude"))throwErrorByCtx(ctx, "add/updateTrack error: no longitude");
                p_waypoint->m_lon = duk_to_number(ctx, -1);
                duk_pop(ctx);
            duk_pop(ctx);   // done with position
            }
        else throwErrorByCtx(ctx, "addtrackpoint error: no position");
        if (duk_get_prop_string(ctx, -1, "creationDateTime")) {
            p_waypoint->m_CreateTime.Set((time_t) duk_to_number(ctx, -1));
            }
        duk_pop(ctx);
    return(p_waypoint);
    }

PlugIn_Route_Ex * js_duk_to_opcn_route(duk_context *ctx, bool createGUID){
    // returns an opcn route constructed from js route on top of duk stack
    // get a GUID if none provided and createGUID is true
    void throwErrorByCtx(duk_context *ctx, wxString message);
    duk_size_t listLength, i;
    bool ret;
    PlugIn_Route_Ex *p_route = new PlugIn_Route_Ex();
    // indenting here represents stack hight - do not reformat
    duk_get_prop_string(ctx, -1, "name");
        p_route->m_NameString = duk_to_string(ctx, -1);
        duk_pop(ctx);
    duk_get_prop_string(ctx, -1, "from");
        p_route->m_StartString = duk_to_string(ctx, -1);
        duk_pop(ctx);
    duk_get_prop_string(ctx, -1, "to");
        p_route->m_EndString = duk_to_string(ctx, -1);
        duk_pop(ctx);
    duk_get_prop_string(ctx, -1, "description");
        p_route->m_Description = duk_to_string(ctx, -1);
        duk_pop(ctx);
    duk_get_prop_string(ctx, -1, "isVisible");
        p_route->m_isVisible = duk_to_boolean(ctx, -1);
        duk_pop(ctx); 
    ret = duk_get_prop_string(ctx, -1, "GUID");
        p_route->m_GUID = duk_to_string(ctx, -1);
        if (ret == 0 || wxIsEmpty(p_route->m_GUID) || createGUID) {
            p_route->m_GUID = GetNewGUID(); // if no GUID, provide one if allowed
            }
        duk_pop(ctx);
    if (duk_get_prop_string(ctx, -1, "waypoints")){
        if (duk_is_array(ctx, -1)){
            // and it is an array
            listLength = duk_get_length(ctx, -1);
            if (listLength < 2) throwErrorByCtx(ctx, "OCPNadd/add/updateupdateRoute error: less than two routepoints");
            p_route->pWaypointList = new Plugin_WaypointExList; // need to initialise to empty list
            duk_to_object(ctx, -1);
                for (i = 0; i < listLength; i++){   // do waypoints array
                    duk_get_prop_index(ctx, -1, (unsigned int) i);
                    PlugIn_Waypoint_Ex *p_waypoint = js_duk_to_opcn_waypoint(ctx);
                        p_route->pWaypointList->Append(p_waypoint);
                        duk_pop(ctx);
                }
            }
        }
    else throwErrorByCtx(ctx, "OCPNupdateRoute error: called without routepoints");
    duk_pop(ctx);
    return(p_route);
    }
    
PlugIn_Track * js_duk_to_opcn_track(duk_context *ctx, bool createGUID){
    // returns an opcn track constructed from js track on top of duk stack
    // get a GUID if none provided and createGUID is true
    void throwErrorByCtx(duk_context *ctx, wxString message);
    duk_size_t listLength, i;
    bool ret;
    PlugIn_Track *p_track = new PlugIn_Track();
    // indenting here represents stack hight - do not reformat
    duk_get_prop_string(ctx, -1, "name");
        p_track->m_NameString = duk_to_string(ctx, -1);
        duk_pop(ctx);
    duk_get_prop_string(ctx, -1, "from");
        p_track->m_StartString = duk_to_string(ctx, -1);
        duk_pop(ctx);
    duk_get_prop_string(ctx, -1, "to");
        p_track->m_EndString = duk_to_string(ctx, -1);
        duk_pop(ctx);
    ret = duk_get_prop_string(ctx, -1, "GUID");
        p_track->m_GUID = duk_to_string(ctx, -1);
        if (ret == 0 || wxIsEmpty(p_track->m_GUID) || createGUID) {
            p_track->m_GUID = GetNewGUID();  // if no GUID, provide one if allowed
            }
        duk_pop(ctx);
    if (duk_get_prop_string(ctx, -1, "waypoints")){
        if (duk_is_array(ctx, -1)){
            // and it is an array
            listLength = duk_get_length(ctx, -1);
            if (listLength < 2) throwErrorByCtx(ctx, "OCPNadd/updateTrack error: less than two trackpoints");
            p_track->pWaypointList = new Plugin_WaypointList; // need to initialise to empty list
            duk_to_object(ctx, -1);
                for (i = 0; i < listLength; i++){   // do waypoints array
                    duk_get_prop_index(ctx, -1, (unsigned int) i);
                    PlugIn_Waypoint *p_waypoint = js_duk_to_opcn_trackpoint(ctx);
                        p_track->pWaypointList->Append(p_waypoint);
                        duk_pop(ctx);
                }
            }
        }
    else throwErrorByCtx(ctx, "OCPNadd/updateTrack error: called without trackpoints");
    duk_pop(ctx);
    return(p_track);
    }


void ocpn_waypoint_to_js_duk(duk_context *ctx, PlugIn_Waypoint_Ex *p_waypoint){
    // constructs a JavaScript waypoint from an ocpn waypoint leaving it on the top of the stack
    Plugin_Hyperlink *p_hyperlink = new Plugin_Hyperlink();
    // extra indentation here shows stack depth - do not reformat!
    duk_push_object(ctx); // construct the waypoint object
        duk_push_object(ctx); // construct the waypoint object
            duk_push_number(ctx, p_waypoint->m_lat);
                duk_put_prop_literal(ctx, -2, "latitude");
            duk_push_number(ctx, p_waypoint->m_lon);
                duk_put_prop_literal(ctx, -2, "longitude");
            duk_put_prop_literal(ctx, -2, "position");
        duk_push_string(ctx, p_waypoint->m_MarkName);
            duk_put_prop_literal(ctx, -2, "markName");
        duk_push_string(ctx, p_waypoint->m_GUID);
            duk_put_prop_literal(ctx, -2, "GUID");
        duk_push_string(ctx, p_waypoint->m_MarkDescription);
            duk_put_prop_literal(ctx, -2, "description");
        duk_push_boolean(ctx, p_waypoint->IsVisible);
            duk_put_prop_literal(ctx, -2, "isVisible");
        duk_push_string(ctx, p_waypoint->IconName);
            duk_put_prop_literal(ctx, -2, "iconName");
        duk_push_string(ctx, p_waypoint->IconDescription);
            duk_put_prop_literal(ctx, -2, "iconDescription");
        duk_push_boolean(ctx, p_waypoint->IsNameVisible);
            duk_put_prop_literal(ctx, -2, "isNameVisible");
        duk_push_int(ctx, p_waypoint->nrange_rings);
        	duk_put_prop_literal(ctx, -2, "nRanges");
        duk_push_number(ctx, p_waypoint->RangeRingSpace);
        	duk_put_prop_literal(ctx, -2, "rangeRingSpace");
        duk_push_string(ctx, p_waypoint->RangeRingColor.GetAsString(wxC2S_HTML_SYNTAX));
        	duk_put_prop_literal(ctx, -2, "rangeRingColour");       	
        duk_push_number(ctx, p_waypoint->scamin);
        	duk_put_prop_literal(ctx, -2, "minScale");
        duk_push_boolean(ctx, p_waypoint->b_useScamin);
        	duk_put_prop_literal(ctx, -2, "useMinScale");		        	
        duk_push_boolean(ctx, p_waypoint->GetFSStatus());
        	duk_put_prop_literal(ctx, -2, "isFreeStanding");
        duk_push_boolean(ctx, p_waypoint->IsActive);
        	duk_put_prop_literal(ctx, -2, "isActive");
        duk_push_int(ctx, p_waypoint->GetRouteMembershipCount());
        	duk_put_prop_literal(ctx, -2, "routeCount");         	 	
	    duk_push_number(ctx, p_waypoint->m_CreateTime.GetTicks());	// seconds since 1st Jan 1970
            duk_put_prop_literal(ctx, -2, "creationDateTime");
        duk_idx_t arr_idx = duk_push_array(ctx); // the hyperlinkList array
        if (p_waypoint->m_HyperlinkList ){  // only attempt this if list of not empty
            wxPlugin_HyperlinkListNode *linknode = p_waypoint->m_HyperlinkList->GetFirst();
            for (duk_idx_t i = 0; linknode; linknode = linknode->GetNext(), i++){
                    p_hyperlink = linknode->GetData();
                    duk_push_object(ctx); // the link object
                        duk_push_string(ctx, p_hyperlink->DescrText);
                            duk_put_prop_literal(ctx, -2, "description");
                        duk_push_string(ctx, p_hyperlink->Link);
                            duk_put_prop_literal(ctx, -2, "link");
                        duk_push_string(ctx, p_hyperlink->Type);
                            duk_put_prop_literal(ctx, -2, "type");
                        duk_put_prop_index(ctx, arr_idx, i);
                    }
            }
        duk_put_prop_literal(ctx, -2, "hyperlinkList");
    delete p_hyperlink;
}

void ocpn_trackpoint_to_js_duk(duk_context *ctx, PlugIn_Waypoint *p_waypoint){
    // constructs a JavaScript trackpoint from an ocpn waypoint leaving it on the top of the stack
    // extra indentation here shows stack depth - do not reformat!
    duk_push_object(ctx); // construct the waypoint object
        duk_push_object(ctx); // construct the waypoint object
            duk_push_number(ctx, p_waypoint->m_lat);
                duk_put_prop_literal(ctx, -2, "latitude");
            duk_push_number(ctx, p_waypoint->m_lon);
                duk_put_prop_literal(ctx, -2, "longitude");
            duk_put_prop_literal(ctx, -2, "position");
	    duk_push_number(ctx, p_waypoint->m_CreateTime.GetTicks());
            duk_put_prop_literal(ctx, -2, "creationDateTime");
    }
    
static duk_ret_t getMessageNames(duk_context *ctx) {  // get message names seen
    Console *pConsole = findConsoleByCtx(ctx);
    
    duk_push_string(ctx, pConsole->getMessagesString());
    return 1;  // returns one arg
    }

duk_ret_t onMessageNameGuts(duk_context *ctx , bool persist) {  // to wait for message - save function to call
    duk_idx_t nargs = duk_get_top(ctx);  // number of args in JS call
    Console *pConsole = findConsoleByCtx(ctx);
    if (pConsole->mStatus.test(INEXIT)) {
		pConsole->prep_for_throw(ctx, "onMessageName within onExit function");
		duk_throw(ctx);
		}
    if (nargs > 2) throwErrorByCtx(ctx, "OCPNonMessageName bad call");
    if (nargs == 0) { // empty call - cancel any waiting callback
        size_t messageCount = pConsole->mMessages.GetCount();
        if (messageCount > 0){
            for(unsigned int index = 0; index < messageCount; index++){
                pConsole->mMessages[index].functionName = wxEmptyString;
                }
            }
        pConsole->mWaitingCached = false;
        return(0);
        }
    duk_require_function(ctx, 0);
    duk_require_string(ctx, 1);
    TRACE(5, pConsole->dukDump());
    TRACE(5, "About to register waiting for message");
    TRACE (5, findConsoleByCtx(ctx)->dukDump());
    int index = pConsole->OCPNmessageIndex(wxString(duk_to_string(ctx, 1)));
    pConsole->mMessages[index].functionName = extractFunctionName(ctx, 0);
    pConsole->mMessages[index].persist = persist;
    TRACE(5, "Registered");
    duk_pop_2(ctx);
    pConsole->mWaitingCached = pConsole->mWaiting = true;
    return 0;  // returns no arg
	}
	
static duk_ret_t onMessageName(duk_context *ctx){  // to wait for message - save function to call
	onMessageNameGuts(ctx, false);
	return 0;
	}
	
static duk_ret_t onMessageNamePersist(duk_context *ctx){  // to wait for message - save function to call
	onMessageNameGuts(ctx, true);
	return 0;
	}	

static duk_ret_t onNMEA0183(duk_context *ctx) {  // to wait for NMEA0183 message - save function to call
    Console* pConsole = findConsoleByCtx(ctx);
    pConsole->setupNMEA0183(ctx, false);   // needs to be in a method - not persistent
    return 0;
	};
	
static duk_ret_t onNMEA0183Persist(duk_context *ctx) {  // to wait for NMEA0183 message - save function to call
    Console* pConsole = findConsoleByCtx(ctx);
    pConsole->setupNMEA0183(ctx, true);   // needs to be in a method - persistent
    return 0;
	};
	
static duk_ret_t onNMEA2k(duk_context *ctx) {  // to wait for NMEA2k message - save function to call
    Console* pConsole = findConsoleByCtx(ctx);
    pConsole->setupNMEA2k(ctx, false);   // needs to be in a method - non-persistent
    return 0;
	};
	
static duk_ret_t onNMEA2kPersist(duk_context *ctx) {  // to wait for NMEA2k message - save function to call
    Console* pConsole = findConsoleByCtx(ctx);
    pConsole->setupNMEA2k(ctx, true);   // needs to be in a method - persistent
    return 0;
	};
	
/*	for testing in development
    static const char hex_digits[] = "0123456789ABCDEF";
	auto payload = make_shared<std::vector<uint8_t>>();
	wxString source = wxEmptyString;
	
	payload->clear();
	for (int i = 0; i < 20; i++){
		uint8_t x = rand() % 512;
		payload->push_back(x);
		source +=  hex_digits[x >> 4];
		source +=  hex_digits[x & 15];
		}
		
	// now have a payload to return

	duk_push_object(ctx);
		duk_push_string(ctx, source.c_str());
			duk_put_prop_literal(ctx, -2, "Source");
		duk_push_array(ctx);
			for (int i = 0; i < payload->size(); i++){
				duk_push_uint(ctx, payload->at(i));
				duk_put_prop_index(ctx, -2, i);
				}
			duk_put_prop_literal(ctx, -2, "Payload");
	return 1;
	
//	int argType = duk_get_type(ctx, 0);
//	bool isArray = duk_is_array(ctx, 0);
//	duk_pop(ctx);
	duk_get_prop_index(ctx, -1, 5);
	int num = duk_get_number(ctx, -1);
	duk_pop_2(ctx);
	duk_push_number(ctx, num);
	return 1;
//	duk_pop(ctx);
	duk_push_string(ctx, duk_get_string(ctx, 0));
//	duk_push_number(ctx, argType);
	return 1;
	};
*/

	
	
static duk_ret_t onNavigation(duk_context *ctx) {  // to wait for navigation - save function to call
    Console* pConsole = findConsoleByCtx(ctx);
    pConsole->setupNavigationStream(ctx, false);   // needs to be in a method - non-persistent
    return 0;
	};
	
static duk_ret_t onNavigationAll(duk_context *ctx) {  // to wait for navigation - save function to call
    Console* pConsole = findConsoleByCtx(ctx);
    pConsole->setupNavigationStream(ctx, true);   // needs to be in a method - persistant
    return 0;
	};	

static duk_ret_t onActiveLeg(duk_context *ctx) {  // to wait for active leg message - save function to call
    duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
    Console *pConsole = findConsoleByCtx(ctx);
    if (pConsole->mStatus.test(INEXIT)) {
		pConsole->prep_for_throw(ctx, "OCPNonActiveLeg within onExit function");
		duk_throw(ctx);
		}
    if (nargs == 0) { // empty call - cancel any waiting callback
        pConsole->m_activeLegFunction = wxEmptyString;
        pConsole->mWaitingCached = false;
        return(0);
        }

    if (pConsole->m_activeLegFunction != wxEmptyString){
        // request already outstanding
        throwErrorByCtx(ctx, "OCPNonActiveLeg called with call outstanding");
        }
    else{
        duk_require_function(ctx, 0);
        pConsole->m_activeLegFunction = extractFunctionName(ctx,0);
        pConsole->mWaitingCached = pConsole->mWaiting = true;
        }
    return 0;  // returns no arg
    }

static duk_ret_t onContextMenu(duk_context *ctx) {  // call function on context menu
    extern JavaScript_pi *pJavaScript_pi;
    wxString extractFunctionName(duk_context *ctx, duk_idx_t idx);
    duk_ret_t result = 0;
    duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
    MenuActions menuAction;
    wxString argument = wxEmptyString;
    wxString menuName;
    int menuCount;
    wxMenu dummy_menu;
    Console *pConsole = findConsoleByCtx(ctx);
    
    if (nargs == 0) { // empty call - cancel all menus
        pConsole->clearMenus();
        pConsole->mWaitingCached = false;   // force full isWaiting check
        return(result);
        }
    if (pConsole->mStatus.test(INEXIT)) {
		pConsole->prep_for_throw(ctx, "OCPNonContextMenu within onExit function");
		duk_throw(ctx);
		}
    if (!duk_is_function(ctx, 0)) {
		pConsole->prep_for_throw(ctx, "OCPNonContextMenu first argument must be function");
		duk_throw(ctx);
		}
    menuCount = pConsole->mMenus.GetCount();
    if (menuCount >= MAX_MENUS){
        pConsole->prep_for_throw(ctx, "OCPNonContextMenu already have maximum menus outstanding");
        }
    if ((nargs < 2) || (nargs > 3)){
        pConsole->prep_for_throw(ctx, "OCPNonContextMenu requires two or three arguments");
        }
    menuAction.functionName = extractFunctionName(ctx, 0);
    if (nargs > 2) argument = wxString(duk_to_string(ctx,2));  //if user included 3rd argument, use it
    menuName = wxString(duk_to_string(ctx,1));
    // check menuName not already in use
    for (int i = 0; i < menuCount; i++){
         if (pConsole->mMenus[i].menuName == menuName)
             pConsole->prep_for_throw(ctx, "OCPNonContextMenu menu name '" + menuName + "' already in use");
        }
    menuAction.pmenuItem = new wxMenuItem(&dummy_menu, -1, menuName);
    menuAction.menuID = AddCanvasContextMenuItem(menuAction.pmenuItem, pJavaScript_pi);
    menuAction.menuName = menuName;
    menuAction.argument = argument;
    pConsole->mMenus.Add(menuAction);  // add this menu to array
    pConsole->mWaitingCached = pConsole->mWaiting = true;
    return(result);
    }

static duk_ret_t getNavigation(duk_context *ctx) {  // gets latest navigation data and constructs navigation object
    // ****  Indenting here shows stack depth - do not re-indent this section ****
    duk_push_object(ctx);
        duk_push_number(ctx, pJavaScript_pi->m_positionFix.FixTime);
            duk_put_prop_literal(ctx, -2, "fixTime");
        duk_push_object(ctx);                                  // start of position
            duk_push_number(ctx, pJavaScript_pi->m_positionFix.Lat);
                duk_put_prop_literal(ctx, -2, "latitude");
            duk_push_number(ctx, pJavaScript_pi->m_positionFix.Lon);
                duk_put_prop_literal(ctx, -2, "longitude");
            duk_put_prop_literal(ctx, -2, "position");             // end of position
        duk_push_number(ctx, pJavaScript_pi->m_positionFix.Sog);
            duk_put_prop_literal(ctx, -2, "SOG");
        duk_push_number(ctx, pJavaScript_pi->m_positionFix.Cog);
            duk_put_prop_literal(ctx, -2, "COG");
        duk_push_number(ctx, pJavaScript_pi->m_positionFix.Var);
            duk_put_prop_literal(ctx, -2, "variation");
        duk_push_number(ctx, pJavaScript_pi->m_positionFix.Hdm);
            duk_put_prop_literal(ctx, -2, "HDM");
        duk_push_number(ctx, pJavaScript_pi->m_positionFix.Hdt);
            duk_put_prop_literal(ctx, -2, "HDT");
        duk_push_int(ctx, pJavaScript_pi->m_positionFix.nSats);
            duk_put_prop_literal(ctx, -2, "nSats");
    return 1;  // returns one arg
}

static duk_ret_t getNavigationK(duk_context *ctx) {  // gets latest navigation data and constructs navigation object Signak K formatted
    wxString thisTime = wxDateTime(pJavaScript_pi->m_positionFix.FixTime).FormatISOCombined().c_str();

    // ****  Indenting here shows stack depth - do not re-indent this section ****
    duk_push_object(ctx);
        duk_push_object(ctx);                               // start of SOG
            duk_push_number(ctx, pJavaScript_pi->m_positionFix.Sog);
            duk_put_prop_literal(ctx, -2, "value");
            duk_push_string(ctx, "OCPN");
            duk_put_prop_literal(ctx, -2, "sentence");
            duk_push_string(ctx, thisTime);
            duk_put_prop_literal(ctx, -2, "timestamp");
            duk_put_prop_literal(ctx, -2, "speedOverGround"); // end of SOG
        duk_push_object(ctx);                               // start of COG
            duk_push_number(ctx, pJavaScript_pi->m_positionFix.Cog);
            duk_put_prop_literal(ctx, -2, "value");
            duk_push_string(ctx, "OCPN");
            duk_put_prop_literal(ctx, -2, "sentence");
            duk_push_string(ctx, thisTime);
            duk_put_prop_literal(ctx, -2, "timestamp");
            duk_put_prop_literal(ctx, -2, "courseOverGround"); // end of COG
        duk_push_object(ctx);                                // start of position
            duk_push_object(ctx);                              // start of value
                duk_push_number(ctx, pJavaScript_pi->m_positionFix.Lat);
                duk_put_prop_literal(ctx, -2, "latitude");
                duk_push_number(ctx, pJavaScript_pi->m_positionFix.Lon);
                duk_put_prop_literal(ctx, -2, "longitude");
            duk_put_prop_literal(ctx, -2, "value");             // end of value
            duk_push_string(ctx, "OCPN");                       // start of nSats
            duk_put_prop_literal(ctx, -2, "sentence");
            duk_push_int(ctx, pJavaScript_pi->m_positionFix.nSats);
            duk_put_prop_literal(ctx, -2, "numberSatellites");  // end of nSats
            duk_push_string(ctx, thisTime);
            duk_put_prop_literal(ctx, -2, "timestamp");
        duk_put_prop_literal(ctx, -2, "position");              // end of position
    duk_push_object(ctx);                                   // start of heading
        duk_push_object(ctx);                                // start of heading true
            duk_push_number(ctx, pJavaScript_pi->m_positionFix.Hdt);
            duk_put_prop_literal(ctx, -2, "value");
            duk_push_string(ctx, "OCPN");
            duk_put_prop_literal(ctx, -2, "sentence");
            duk_push_string(ctx, thisTime);
            duk_put_prop_literal(ctx, -2, "timestamp");
            duk_put_prop_literal(ctx, -2, "headingTrue");
        duk_push_object(ctx);                                   // start of heading magnetic
            duk_push_number(ctx, pJavaScript_pi->m_positionFix.Hdm);
            duk_put_prop_literal(ctx, -2, "value");
            duk_push_string(ctx, "OCPN");
            duk_put_prop_literal(ctx, -2, "sentence");
            duk_push_string(ctx, thisTime);
            duk_put_prop_literal(ctx, -2, "timestamp");
            duk_put_prop_literal(ctx, -2, "headingMagnetic");
        duk_put_prop_literal(ctx, -2, "heading");              // end of heading
    duk_push_object(ctx);                                   // start of variation
        duk_push_number(ctx, pJavaScript_pi->m_positionFix.Var);
        duk_put_prop_literal(ctx, -2, "value");
        duk_push_string(ctx, "OCPN");
        duk_put_prop_literal(ctx, -2, "sentence");
        duk_push_string(ctx, thisTime);
        duk_put_prop_literal(ctx, -2, "timestamp");
        duk_put_prop_literal(ctx, -2, "variation");
    return 1;  // returns one arg
}

/* static duk_ret_t getActiveLeg(duk_context *ctx) {  // gets active leg data and constructs object
    // ****  Indenting here shows stack depth - do not re-indent this section ****
    duk_push_object(ctx);
        duk_push_string(ctx, pJavaScript_pi->Plugin_Active_Leg_Info.wp_name);
            duk_put_prop_literal(ctx, -2, "markName");
        duk_push_number(ctx, pJavaScript_pi->Plugin_Active_Leg_Info.Btw);
            duk_put_prop_literal(ctx, -2, "bearing");
        duk_push_number(ctx, pJavaScript_pi->Plugin_Active_Leg_Info.Dtw;
            duk_put_prop_literal(ctx, -2, "distance");
        duk_push_number(ctx, pJavaScript_pi->Plugin_Active_Leg_Info.Xte);
            duk_put_prop_literal(ctx, -2, "xte");
        duk_push_boolean(ctx, pJavaScript_pi->Plugin_Active_Leg_Info.arrival);
            duk_put_prop_literal(ctx, -2, "arrived");
    return 1;  // returns one arg
}
 */
 
wxString addNMEA0183CheckSum(duk_context *ctx, wxString sentence){// add checksum to NMEA sentence
	// props to Dirk Smits for the checksum calculation lifted from his NMEAConverter plugin
	wxString NMEAchecksum(wxString sentence);
	sentence.Trim();       
	// we will drop any existing checksum
	int starPos = sentence.Find("*");
	if (starPos != wxNOT_FOUND){ // yes there is one
		sentence = sentence.SubString(0, starPos-1); // truncate at * onwards
		}
	if  (!(((sentence[0] == '$') || (sentence[0] == '!')) && (sentence[6] == ',')))
		throwErrorByCtx(ctx, "NMEA0183 sentence does not start $....., or !.....,");
	wxString sum = NMEAchecksum(sentence);
	sentence = sentence.Append("*");
	sentence = sentence.Append(sum);
	sentence = sentence.Append("\r\n");
	return sentence;
	}
	
static duk_ret_t pushText(duk_context *ctx) {  // pushes string on stack out through OpenCPN
	// called with OCPNpushText(data, handle)
    duk_require_string(ctx, 0);	// data argument must be string
    duk_require_string(ctx, 1);	// handle argument must be string   
    wxString data = wxString(duk_to_string(ctx,0));
	DriverHandle handle = duk_to_string(ctx,1);
	duk_pop_2(ctx);
	wxString protocol = getHandleProtocol(ctx, handle, "OCPNpushText");
	if (protocol == "nmea0183"){
		data = addNMEA0183CheckSum(ctx, data);
		}
	auto payload = make_shared<std::vector<uint8_t>>();
	for (const auto& ch : data) payload->push_back(ch);
	CommDriverResult outcome = WriteCommDriver(handle, payload);
	if (outcome == RESULT_COMM_NO_ERROR) duk_push_string(ctx, "OK");
	else throwErrorByCtx(ctx, wxString(wxString::Format("OCPNpush driver error: %s", driverErrorStrings[outcome])));
	return 1; // keep compiler happy
	}


static duk_ret_t NMEA0183push(duk_context *ctx) {  // pushes NMEA sentence on stack out through OpenCPN
    duk_idx_t nargs = duk_get_top(ctx);	// number of arguments
    if (nargs == 1)	{ // old-style Call
    	duk_require_string(ctx, 0);	// first and only argument must be string
    	wxString sentence = wxString(duk_to_string(ctx,0));  // the NMEA sentence
    	duk_pop(ctx);
    	sentence = addNMEA0183CheckSum(ctx, sentence);
    	PushNMEABuffer(sentence);
    	return 0;
    	}
    // must have handle - use new method
    pushText(ctx);
    return 1;
    }

static duk_ret_t NMEA2kPush(duk_context *ctx) {  // pushes NMEA2k sentence on stack out through OpenCPN
	// OCPNpushNMEA2000(handle, pgn, destination, priority, payload)
	// sanity checks
	duk_require_string(ctx, 0);		// handle
	duk_require_int(ctx, 1);		// pgn
	duk_require_int(ctx, 2);		// destination
	duk_require_int(ctx, 3);		// priority
	duk_require_object(ctx, 4);		// payload array
	
	// deal with pgn registration if needed
	DriverHandle handle = duk_get_string(ctx, 0);
	pgn_t pgn = duk_get_int(ctx, 1);
	bool matchedHandle = false;
	bool matchedPgn = false;
	for (int h = 0; h < pJavaScript_pi->m_pgnRegistrations.size(); h++){	// look for existing handle
		if (pJavaScript_pi->m_pgnRegistrations[h].handle == handle){	// matched handle
			matchedHandle = true;
			for (int p = 0; p < pJavaScript_pi->m_pgnRegistrations[h].pgns.size(); p++){
				if (pJavaScript_pi->m_pgnRegistrations[h].pgns[p] == pgn){	//matched pgn
					matchedPgn = true;
					break;
					}
				}
			if (!matchedPgn){	// new pgn in familiar handle
				pJavaScript_pi->m_pgnRegistrations[h].pgns.emplace_back(pgn);
				// need to register here
				matchedPgn = true;
				break;
				}
			}
		
		}
	if (!matchedHandle) {	// new handle - need to register it and remember it
		wxString protocol = getHandleProtocol(ctx, handle, "OCPNpushNMEA2000");
		if (protocol != "nmea2000") throwErrorByCtx(ctx, wxString(wxString::Format("OCPNpushNMEA2000 handle &s not NMEA2000", handle)));
		std::vector<int> pgn_list;	// pgn needs to be in list
		pgn_list.emplace_back(pgn);
		RegisterTXPGNs(handle, pgn_list);
		pgn_registration newreg;
		newreg.handle = handle;
		newreg.pgns.emplace_back(pgn);
		pJavaScript_pi->m_pgnRegistrations.emplace_back(newreg);
		};	
		
	// all good to go - prepare payload
	int destinationCANAddress = duk_get_int(ctx, 2);
	int priority = duk_get_int(ctx, 3);
	std::shared_ptr <std::vector<uint8_t>> payload = std::make_shared<std::vector<uint8_t>>();
	duk_size_t length = duk_get_length(ctx, 4);
	duk_uint_t val;
	for (duk_idx_t i = 0; i < length; i++) {
		duk_get_prop_index(ctx, 4, i);
		val = duk_get_uint(ctx, -1);
		payload->push_back(val);
		}
	duk_pop_n(ctx, 4);	// finished with arguments	
	CommDriverResult outcome = WriteCommDriverN2K(handle, pgn, destinationCANAddress, priority, payload);
	if (outcome == RESULT_COMM_NO_ERROR) duk_push_string(ctx, "OK");
	else throwErrorByCtx(ctx, wxString(wxString::Format("OCPNpushNMEA2000 driver error: %s\n", driverErrorStrings[outcome])));
	return 1;
	}

static duk_ret_t sendMessage(duk_context *ctx) {  // sends message to OpenCPN
    duk_idx_t nargs;  // number of args in call
    duk_ret_t result = 0;
    wxString message_body = wxEmptyString;
    
    nargs = duk_get_top(ctx);
    if (nargs < 1){
        throwErrorByCtx(ctx, "OCPNsendMessage error: called without at least one argument");
        }
    duk_require_string(ctx, 0);
    if (nargs > 1){
        // we have a message body
        duk_require_string(ctx, 1);
        message_body = wxString(duk_to_string(ctx,1));
        duk_pop(ctx);
        }
    wxString message_id = wxString(duk_to_string(ctx,0));
    duk_pop(ctx);
    message_id.Trim();
    if (message_id == "OCPN_DRAW_PI") throwErrorByCtx(ctx, "OCPNsendMessage blocked: sending to OCPN_DRAW_PI not supported");
    TRACE(4, "Sending message " + message_id);
    TRACE (5, findConsoleByCtx(ctx)->dukDump());
    SendPluginMessage(message_id, message_body);
    return(result);
}

const unsigned int GPXbufferLength {1500};
char GPXbuffer[GPXbufferLength];
static duk_ret_t getARPgpx(duk_context *ctx) {  // get Active Route Point as GPX
    bool result = GetActiveRoutepointGPX( GPXbuffer, GPXbufferLength);
    duk_push_string(ctx, result ? GPXbuffer : "");
    return 1;  // returns one arg
    }

static duk_ret_t OCPNrefreshCanvas(duk_context *ctx) {  // refresh main window
    RequestRefresh(GetOCPNCanvasWindow());
    return(0);
}


static duk_ret_t getNewGUID(duk_context *ctx) {  // get new GUID
    duk_push_string(ctx, GetNewGUID());
    return(1);  // returns 1 string
    }
 
OBJECT_LAYER_REQ determinGUIDtype(duk_context *ctx){
	// for all getGUID array calls,  determine which are to be fetched
	duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
	if (nargs == 0) return(OBJECTS_NO_LAYERS);
	if (nargs != 1) throwErrorByCtx(ctx, "OCPNgetGUIDs called with more than 1 arg");
	int guidSelection = duk_get_number(ctx, 0);	// the argument
	duk_pop(ctx);
	switch (guidSelection){
		case -1:	return OBJECTS_ALL;
		case 0:		return OBJECTS_NO_LAYERS;
		case 1:		return OBJECTS_ONLY_LAYERS;
		default:	throwErrorByCtx(ctx, "OCPNgetGUIDs called with invalid arg");
					return OBJECTS_ALL;	// not reachable but avoids compiler warning
		}
	}

void clearWaypointsOutofRoute(PlugIn_Route_Ex *p_route){
// For a route structure, clear out the waypoints
// For each waypoint, need to clear out any hyperlink lists
PlugIn_Waypoint_Ex *p_waypoint;

wxPlugin_WaypointExListNode *linknode = p_route->pWaypointList->GetFirst();
for (unsigned int i = 0; linknode; linknode = linknode->GetNext(), i++){
    p_waypoint = linknode->GetData();
    p_waypoint->m_HyperlinkList->DeleteContents(true);
    p_waypoint->m_HyperlinkList->clear();
    }
p_route->pWaypointList->DeleteContents(true);
p_route->pWaypointList->clear();
}

void clearWaypointsOutofTrack(PlugIn_Track *p_track){
// For a track structure, clear out the waypoints
/*
wxPlugin_WaypointListNode *linknode = p_track->pWaypointList->GetFirst();
    for (unsigned int i = 0; linknode; linknode = linknode->GetNext(), i++){
    p_waypoint = linknode->GetData();
    p_waypoint->m_HyperlinkList->DeleteContents(true);
    p_waypoint->m_HyperlinkList->clear();
    }
 */
p_track->pWaypointList->DeleteContents(true);
p_track->pWaypointList->clear();
}

static duk_ret_t getWaypointGUIDs(duk_context *ctx){ // get waypoing GUID array
    wxArrayString guidArray;
    duk_idx_t arr_idx;
    int i;
    size_t count;
    
    guidArray = GetWaypointGUIDArray(determinGUIDtype(ctx));
    arr_idx = duk_push_array(ctx);
    if (!guidArray.IsEmpty()){
        count = guidArray.GetCount();
        for (i = 0; i < count; i++){
            duk_push_string(ctx, guidArray[i]);
            duk_put_prop_index(ctx, arr_idx, i);
            }
        }
    return(1);
    }
    
static duk_ret_t getActiveWaypointGUID(duk_context *ctx){ // get GUID of active waypoint
    duk_push_string(ctx, GetActiveWaypointGUID());
    return(1);
    }

static duk_ret_t getSingleWaypoint(duk_context *ctx) {

    PlugIn_Waypoint_Ex *p_waypoint = new PlugIn_Waypoint_Ex();
    bool result;
    wxString GUID;
    duk_require_string(ctx, 0);
    GUID = duk_get_string(ctx, 0);
    duk_pop(ctx);
    result = GetSingleWaypointEx(GUID, p_waypoint);
    if (!result){  // waypoint does not exist
        delete p_waypoint;
        throwErrorByCtx(ctx, "OCPNGetSingleWaypoint called with non-existant GUID " + GUID);
        return(1);
        }
    ocpn_waypoint_to_js_duk(ctx, p_waypoint);   // construct the waypoint
    delete p_waypoint;
    return(1);
    }

static duk_ret_t addSingleWaypoint(duk_context *ctx) {
    PlugIn_Waypoint_Ex *p_waypoint;
    bool result;
    wxString GUID;
    duk_require_object(ctx,0);
    p_waypoint = js_duk_to_opcn_waypoint(ctx);  // construct the opcn waypoint
    result = AddSingleWaypointEx(p_waypoint);
    if (!result){ // waypoint already exists?
            throwErrorByCtx(ctx, "OCPNaddSingleWaypoint called with existing GUID? " + p_waypoint->m_GUID);
            }
    duk_push_string(ctx, p_waypoint->m_GUID);  // else return the GUID
    // now waypoint safely stored in OpenCPN, clean up - list data not otherwise released
    p_waypoint->m_HyperlinkList->DeleteContents(true);
    p_waypoint->m_HyperlinkList->clear();
    return(1);  // returns result
    }

static duk_ret_t updateSingleWaypoint(duk_context *ctx) {
    PlugIn_Waypoint_Ex *p_waypoint;
    bool result;
    duk_require_object(ctx,0);
    p_waypoint = js_duk_to_opcn_waypoint(ctx);  // construct the ocpn waypoint
    result = UpdateSingleWaypointEx(p_waypoint);
    if (!result){ // waypoint does not exists?
        throwErrorByCtx(ctx, "OCPNupdateSingleWaypoint error. Non-existant GUID? " + p_waypoint->m_GUID);
        }
    // now waypoint safely stored in OpenCPN, clean up - list data not otherwise released
    p_waypoint->m_HyperlinkList->DeleteContents(true);
    p_waypoint->m_HyperlinkList->clear();
    duk_push_boolean(ctx, true);    // for compatibility with v0.2 return true
    return(1);
    }

static duk_ret_t deleteSingleWaypoint(duk_context *ctx) {  // given a GUID, deletes waypoint
    wxString GUID;
    bool result;
    duk_require_string(ctx,0);
    GUID = wxString(duk_to_string(ctx,0));
    result = DeleteSingleWaypoint(GUID);
    if (!result){  // waypoint does not exist
        throwErrorByCtx(ctx, "OCPNDeleteSingleWaypoint called with non-existant GUID " + GUID);
        }
    duk_pop(ctx);
    duk_push_boolean(ctx, true);    // for compatibility with v0.2 return true
    return(1);
    }
    
static duk_ret_t getRouteGUIDs(duk_context *ctx){ // get routes GUID array
    wxArrayString guidArray;
    duk_idx_t arr_idx;
    int i;
    size_t count;
    
    guidArray = GetRouteGUIDArray(determinGUIDtype(ctx));
    arr_idx = duk_push_array(ctx);
    if (!guidArray.IsEmpty()){
        count = guidArray.GetCount();
        for (i = 0; i < count; i++){
            duk_push_string(ctx, guidArray[i]);
            duk_put_prop_index(ctx, arr_idx, i);
            }
        }
    return(1);
    }
    
static duk_ret_t getActiveRouteGUID(duk_context *ctx){ // get GUID of active route
    duk_push_string(ctx, GetActiveRouteGUID());
    return(1);
    }

static duk_ret_t getRouteByGUID(duk_context *ctx) {

    wxString GUID;
    std::unique_ptr<PlugIn_Route_Ex> p_route;
    PlugIn_Waypoint_Ex *p_waypoint = new PlugIn_Waypoint_Ex();

    duk_require_string(ctx,0);
    GUID = duk_get_string(ctx, 0);
    duk_pop(ctx);
    p_route = GetRouteEx_Plugin(GUID);
    if (p_route == nullptr){ // no such route
        throwErrorByCtx(ctx, "OCPNgetRoute called with non-existant GUID " + GUID);
        }
    // extra indentation here shows stack depth - do not reformat!
    duk_push_object(ctx); // construct the route object
    duk_push_string(ctx, p_route->m_NameString);
        duk_put_prop_literal(ctx, -2, "name");        
    duk_push_string(ctx, p_route->m_StartString);
        duk_put_prop_literal(ctx, -2, "from");
    duk_push_string(ctx, p_route->m_EndString);
        duk_put_prop_literal(ctx, -2, "to");
    duk_push_string(ctx, p_route->m_Description);
        duk_put_prop_literal(ctx, -2, "description");
    duk_push_string(ctx, p_route->m_GUID);
        duk_put_prop_literal(ctx, -2, "GUID");
    duk_push_boolean(ctx, p_route->m_isActive);
	    duk_put_prop_literal(ctx, -2, "isActive");	    
	duk_push_boolean(ctx, p_route->m_isVisible);
	    duk_put_prop_literal(ctx, -2, "isVisible");   
    duk_idx_t arr_idx = duk_push_array(ctx); // the waypoint array
    if (p_route->pWaypointList ){  // only attempt this if waypoint list of not empty
        wxPlugin_WaypointExListNode *linknode = p_route->pWaypointList->GetFirst();
        for (duk_idx_t i = 0; linknode; linknode = linknode->GetNext(), i++){
            p_waypoint = linknode->GetData();
            ocpn_waypoint_to_js_duk(ctx, p_waypoint);   // construct this waypoint
            duk_put_prop_index(ctx, arr_idx, i);
            }
        }
    duk_put_prop_literal(ctx, -2, "waypoints");
    return(1);
    }

static duk_ret_t addRoute(duk_context *ctx) { // add the route to OpenCPN
    PlugIn_Route_Ex *p_route;

//    bool permanent = true;  // permanent by default
    bool result;
    duk_require_object(ctx,0);
    p_route = js_duk_to_opcn_route(ctx, true);    // construct the opcn route, providing a GUID if not supplied
    result = AddPlugInRouteEx(p_route);
    if (!result){
        throwErrorByCtx(ctx, "OCPNaddRoute called with existant GUID " + p_route->m_GUID);
        }
    duk_push_string(ctx, p_route->m_GUID);  // return the GUID
    // now route safely stored in OpenCPN, clean up - lists data not otherwise released
    clearWaypointsOutofRoute(p_route);
    return(1);
    }

static duk_ret_t updateRoute(duk_context *ctx) { // update the route in OpenCPN
    PlugIn_Route_Ex *p_route;
    duk_require_object(ctx,0);
    p_route = js_duk_to_opcn_route(ctx, false);    // construct the opcn route - must have given GUID
    if(!UpdatePlugInRouteEx(p_route)) throwErrorByCtx(ctx, "OCPNupdateRoute called with non-existant GUID " + p_route->m_GUID);
    clearWaypointsOutofRoute(p_route);
    duk_push_boolean(ctx, true);    // for compatibility with v0.2 return true
    return(1);
    }

static duk_ret_t deleteRoute(duk_context *ctx) {  // given a GUID, deletes route
    wxString GUID;
    duk_require_string(ctx,0);
    GUID = wxString(duk_to_string(ctx,0));
    duk_pop(ctx);
    if (!DeletePlugInRoute(GUID)) {
        throwErrorByCtx(ctx, "OCPNdeleteRoute called with non-existant GUID " + GUID);
        }
    duk_push_boolean(ctx, true);    // for compatibility with v0.2 return true
    return(1);
    }

static duk_ret_t getTrackGUIDs(duk_context *ctx){ // get tracks GUID array
    wxArrayString guidArray;
    duk_idx_t arr_idx;
    int i;
    size_t count;
    
    guidArray = GetTrackGUIDArray(determinGUIDtype(ctx));
    arr_idx = duk_push_array(ctx);
    if (!guidArray.IsEmpty()){
        count = guidArray.GetCount();
        for (i = 0; i < count; i++){
            duk_push_string(ctx, guidArray[i]);
            duk_put_prop_index(ctx, arr_idx, i);
            }
        }
    return(1);
    }
    
static duk_ret_t getTrackByGUID(duk_context *ctx) {
    wxString GUID;
    std::unique_ptr<PlugIn_Track> p_track;
    PlugIn_Waypoint *p_waypoint = new PlugIn_Waypoint();

    GUID = duk_get_string(ctx, 0);
    duk_pop(ctx);
     //No harness equivalent of this, so omit for now
    p_track = GetTrack_Plugin(GUID);
    if (p_track == nullptr){ // no such track
        throwErrorByCtx(ctx, "OCPNgetTrack called with non-existant GUID " + GUID);
        }
    // extra indentation here shows stack depth - do not reformat!
    duk_push_object(ctx); // construct the track object
    duk_push_string(ctx, p_track->m_NameString);
        duk_put_prop_literal(ctx, -2, "name");
    duk_push_string(ctx, p_track->m_StartString);
        duk_put_prop_literal(ctx, -2, "from");
    duk_push_string(ctx, p_track->m_EndString);
        duk_put_prop_literal(ctx, -2, "to");
    duk_push_string(ctx, p_track->m_GUID);
        duk_put_prop_literal(ctx, -2, "GUID");
    duk_idx_t arr_idx = duk_push_array(ctx); // the waypoint array
    if (p_track->pWaypointList ){  // only attempt this if waypoint list is not empty
        wxPlugin_WaypointListNode *linknode = p_track->pWaypointList ->GetFirst();
        for (duk_idx_t i = 0; linknode; linknode = linknode->GetNext(), i++){
            p_waypoint = linknode->GetData();
            ocpn_trackpoint_to_js_duk(ctx, p_waypoint);   // construct this waypoint
            duk_put_prop_index(ctx, arr_idx, i);
            }
        }
    duk_put_prop_literal(ctx, -2, "waypoints");
    return(1);
    }

static duk_ret_t addTrack(duk_context *ctx) { // add the track to OpenCPN
    PlugIn_Track *p_track;

    bool permanent = true;  // permanent by default
    bool result;
    duk_idx_t nargs;
    nargs = duk_get_top(ctx);   // number of arguments in call
    if (nargs == 2) {
        permanent = duk_get_boolean(ctx, -1);   // decide on permanency
        duk_pop(ctx);
        }
    p_track = (PlugIn_Track *) js_duk_to_opcn_track(ctx, true); 
    result = AddPlugInTrack(p_track, permanent);
    if (!result) duk_push_boolean(ctx, false);  //  failed
    else duk_push_string(ctx, p_track->m_GUID);  // else return the GUID
    // now track safely stored in OpenCPN, clean up - lists data not otherwise released
    clearWaypointsOutofTrack(p_track);
    return(1);
    }

static duk_ret_t updateTrack(duk_context *ctx) { // update the track in OpenCPN
    PlugIn_Track *p_track;

    p_track = js_duk_to_opcn_track(ctx, false);
    duk_push_boolean(ctx, UpdatePlugInTrack(p_track));  // result will be returned
    clearWaypointsOutofTrack(p_track);
    return(1);
    }

static duk_ret_t deleteTrack(duk_context *ctx) {  // given a GUID, deletes track
    wxString GUID;
    GUID = wxString(duk_to_string(ctx,0));
    if (!DeletePlugInTrack(GUID)) {
        throwErrorByCtx(ctx, "OCPNdeleteTrack called with non-existant GUID " + GUID);
        }
    return(0);  // returns boolean result
    }

static duk_ret_t OCPNcentreCanvas(duk_context *ctx) { // jump to position
	// call was OCPNcentreCanvas(position [, ppm])   ppm is scale in pixels per metre
    duk_double_t lat, lon, ppm;
    duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
    if ((nargs < 1) || (nargs > 2)) throwErrorByCtx(ctx, "OCPNcentreCanvas called with wrong number of arguments");
    
//    duk_require_object(ctx, 0);	// check we have a position
    bool latOK = duk_get_prop_literal(ctx, 0, "latitude");
    lat = duk_get_number(ctx, -1);
    bool lonOK = duk_get_prop_literal(ctx, 0, "longitude");
    lon = duk_get_number(ctx, -1);
    duk_pop_2(ctx);	// pop lat and lon
    if (!latOK || !lonOK) throwErrorByCtx(ctx, "OCPNcentreCanvas called without position as first arg");
    if ((abs(lat)> 90) || (abs(lon) > 180)) throwErrorByCtx(ctx, "OCPNcentreCanvas called with invalid lat or lon");
    
    if (nargs == 2){	// ppm supplied
    	duk_require_number(ctx, 1);
    	ppm = duk_get_number(ctx,1);
    	}
	else {	// must be just position - no ppm, get current
		ppm = pJavaScript_pi->m_currentViewPort.view_scale_ppm;
		}

//    canvas = GetOCPNCanvasWindow();
    JumpToPosition(lat, lon, ppm);
    duk_pop_n(ctx, nargs);
    return(0);  // no arguments returned
}


static duk_ret_t getPluginConfig(duk_context *ctx) {  // gets plugin configuration object
    // ****  Indenting here shows stack depth - do not re-indent this section ****
#include <wx/versioninfo.h>
    duk_push_object(ctx);
        duk_push_int(ctx, PLUGIN_VERSION_MAJOR);
            duk_put_prop_literal(ctx, -2, "PluginVersionMajor");
        duk_push_int(ctx, PLUGIN_VERSION_MINOR);
            duk_put_prop_literal(ctx, -2, "PluginVersionMinor");
        duk_push_int(ctx, PLUGIN_VERSION_PATCH);
            duk_put_prop_literal(ctx, -2, "patch");
        duk_push_string(ctx, PLUGIN_VERSION_COMMENT);
            duk_put_prop_literal(ctx, -2, "comment");
        duk_push_int(ctx, API_VERSION_MAJOR);
        	duk_put_prop_literal(ctx, -2, "ApiMajor");
		duk_push_int(ctx, API_VERSION_MINOR);
        	duk_put_prop_literal(ctx, -2, "ApiMinor");
        wxVersionInfo info = wxGetLibraryVersionInfo();
        duk_push_string(ctx, info.GetVersionString());
            duk_put_prop_literal(ctx, -2, "wxWidgets");
        duk_push_int(ctx, DUK_VERSION);
        	duk_put_prop_literal(ctx, -2, "DuktapeVersion");

#ifdef IN_HARNESS
        duk_push_boolean(ctx, true);
#else
        duk_push_boolean(ctx, false);
#endif
            duk_put_prop_literal(ctx, -2, "inHarness");
    return 1;  // returns one arg
}

static duk_ret_t getCursorPosition(duk_context *ctx) {  // gets cursor position
    // ****  Indenting here shows stack depth - do not re-indent this section ****
    duk_push_object(ctx);
    duk_push_number(ctx, pJavaScript_pi->mCursorPosition.lat);
            duk_put_prop_literal(ctx, -2, "latitude");
    duk_push_number(ctx, pJavaScript_pi->mCursorPosition.lon);
            duk_put_prop_literal(ctx, -2, "longitude");
    return 1;  // returns one arg
    }

static duk_ret_t getOCPNconfig(duk_context *ctx) {  // gets OCPN configuration as JSON
    // json = OCPNgetOCPNconfig()
    if (pJavaScript_pi->openCPNConfig == wxEmptyString) throwErrorByCtx(ctx, "OCPNgetOCPNconfig  no config available");
    duk_push_string(ctx, pJavaScript_pi->openCPNConfig);
    return 1;
    }

static duk_ret_t getAISTargetsArray(duk_context *ctx) {
    ArrayOfPlugIn_AIS_Targets *AIStargets;
    ArrayOfPlugIn_AIS_Targets::iterator it;
    PlugIn_AIS_Target *t;
    duk_idx_t arr_idx;
    int i;
    wxString temp;
    
    arr_idx = duk_push_array(ctx);  // we are going to create an array of objects
    AIStargets = GetAISTargetArray();
    for( it = (*AIStargets).begin(), i= 0; it != (*AIStargets).end(); ++it, ++i ) {
        t = *it;
        // ****  Indenting here shows stack depth - do not re-indent this section ****
        duk_push_object(ctx);
            duk_push_int(ctx, t->MMSI);
                duk_put_prop_literal(ctx, -2, "MMSI");
            duk_push_int(ctx, t->Class);
                duk_put_prop_literal(ctx, -2, "class");
            duk_push_int(ctx, t->NavStatus);
                duk_put_prop_literal(ctx, -2, "navStatus");
            duk_push_number(ctx, t->SOG);
                duk_put_prop_literal(ctx, -2, "SOG");
            duk_push_number(ctx, t->COG);
                duk_put_prop_literal(ctx, -2, "COG");
            duk_push_number(ctx, t->HDG);
                duk_put_prop_literal(ctx, -2, "HDG");
            duk_push_object(ctx);
                duk_push_number(ctx, t->Lon);
                    duk_put_prop_literal(ctx, -2, "longitude");
                duk_push_number(ctx, t->Lat);
                    duk_put_prop_literal(ctx, -2, "latitude");
                duk_put_prop_literal(ctx, -2, "position");
            duk_push_number(ctx, t->ShipType);
                duk_put_prop_literal(ctx, -2, "shipType");
            duk_push_int(ctx, t->ROTAIS);
                duk_put_prop_literal(ctx, -2, "ROTAIS");
            // callSign and shipName padded with '@' so need to remove.  t->ShipName.BeforeFirst() does not compile, so do in stages
            temp = t->CallSign;
            duk_push_string(ctx, temp.BeforeFirst('@'));
                duk_put_prop_literal(ctx, -2, "callSign");
            temp = t->ShipName;
            duk_push_string(ctx, temp.BeforeFirst('@'));
                duk_put_prop_literal(ctx, -2, "shipName");
            duk_push_int(ctx, t->IMO);
                duk_put_prop_literal(ctx, -2, "IMO");
            duk_push_number(ctx, t->Range_NM);
                duk_put_prop_literal(ctx, -2, "range");
            duk_push_number(ctx, t->Brg);
                duk_put_prop_literal(ctx, -2, "bearing");
            duk_push_boolean(ctx, t->bCPA_Valid);
                duk_put_prop_literal(ctx, -2, "CPAvalid");
            duk_push_number(ctx, t->TCPA);
                duk_put_prop_literal(ctx, -2, "CPAminutes");
            duk_push_number(ctx, t->CPA);
                duk_put_prop_literal(ctx, -2, "CPAnm");
            duk_push_int(ctx, t->alarm_state);
                duk_put_prop_literal(ctx, -2, "alarmState");
            duk_put_prop_index(ctx, arr_idx, i);
        }
    return(1);
    }

static duk_ret_t getVectorPP(duk_context *ctx) {
        // get vector between two positions
        // API seems to work in opposite sense - corrected here
        double fromLat, fromLon, toLat, toLon, bearing, distance;
        duk_idx_t obj_idx;
        duk_idx_t from_idx {0};
        duk_idx_t to_idx {1};
        duk_get_prop_literal(ctx, from_idx, "latitude");
        if (duk_is_undefined(ctx, -1)  || !duk_is_number(ctx, -1)) throwErrorByCtx(ctx, "OPNgetVectorPP from latitude is missing or invalid");
        fromLat = duk_get_number(ctx, -1);
        duk_pop(ctx);
        duk_get_prop_literal(ctx, from_idx, "longitude");
        if (duk_is_undefined(ctx, -1)  || !duk_is_number(ctx, -1)) throwErrorByCtx(ctx, "OPNgetVectorPP from longitude is missing or invalid");
        fromLon = duk_get_number(ctx, -1);
        duk_pop(ctx);
        duk_get_prop_literal(ctx, to_idx, "latitude");
        if (duk_is_undefined(ctx, -1)  || !duk_is_number(ctx, -1)) throwErrorByCtx(ctx, "OPNgetVectorPP to latitude is missing or invalid");
        toLat = duk_get_number(ctx, -1);
        duk_pop(ctx);
        duk_get_prop_literal(ctx, to_idx, "longitude");
        if (duk_is_undefined(ctx, -1)  || !duk_is_number(ctx, -1)) throwErrorByCtx(ctx, "OPNgetVectorPP to longitude is missing or invalid");
        toLon = duk_get_number(ctx, -1);
        duk_pop(ctx);
        duk_pop_2(ctx); // pop off both args
        DistanceBearingMercator_Plugin(toLat, toLon, fromLat, fromLon, &bearing, &distance);
        obj_idx = duk_push_object(ctx);
        duk_push_number(ctx, bearing);
        duk_put_prop_literal(ctx, obj_idx, "bearing");
        duk_push_number(ctx, distance);
        duk_put_prop_literal(ctx, obj_idx, "distance");
        return 1;
        }

static duk_ret_t getPositionPV(duk_context *ctx) {
        // get position after position moved by vector
        // call was (position, vector)
        double fromLat, fromLon, toLat, toLon, bearing, distance;
        duk_idx_t pos_idx {0};
        duk_idx_t vector_idx {1};

        duk_get_prop_literal(ctx, pos_idx, "latitude");
        if (duk_is_undefined(ctx, -1)  || !duk_is_number(ctx, -1)) throwErrorByCtx(ctx, "OCPNgetPositionPV latitude is missing or invalid");
        fromLat = duk_get_number(ctx, -1);
        duk_pop(ctx);
        duk_get_prop_literal(ctx, pos_idx, "longitude");
        if (duk_is_undefined(ctx, -1)  || !duk_is_number(ctx, -1)) throwErrorByCtx(ctx, "OCPNgetPositionPV  longitude is missing or invalid");
        fromLon = duk_get_number(ctx, -1);
        duk_pop(ctx);
        duk_get_prop_literal(ctx, vector_idx, "bearing");
        if (duk_is_undefined(ctx, -1)  || !duk_is_number(ctx, -1)) throwErrorByCtx(ctx, "OCPNgetPositionPV bearing is missing or invalid");
        bearing = duk_get_number(ctx, -1);
        duk_pop(ctx);
        duk_get_prop_literal(ctx, vector_idx, "distance");
        if (duk_is_undefined(ctx, -1)  || !duk_is_number(ctx, -1)) throwErrorByCtx(ctx, "OCPNgetPositionPV distance is missing or invalid");
        distance = duk_get_number(ctx, -1);
        duk_pop(ctx);
        duk_push_object(ctx);   // our new position object to return
        duk_dup(ctx, pos_idx); // duplicate start pos to top of stack above new pos
        duk_set_prototype(ctx, -2);	// set prototype of new object to that of start pos
        PositionBearingDistanceMercator_Plugin(fromLat, fromLon, bearing, distance, &toLat, &toLon);
        duk_push_number(ctx, toLat);
        duk_put_prop_literal(ctx, -2, "latitude");
        duk_push_number(ctx, toLon);
        duk_put_prop_literal(ctx, -2, "longitude");
        // value stack is now 0 original pos  1 vector  2 new pos
        duk_remove(ctx, 0);	// out with original pos
    	duk_remove(ctx, 0);	// out with vector
        return 1;	// leaving new pos to return
        }

static duk_ret_t getGCdistance(duk_context *ctx) {
        // retuns Great Circle distance between two positions
        double firstLat, firstLon, secondLat, secondLon, distance;
        duk_idx_t from_idx {0};
        duk_idx_t to_idx {1};
        duk_get_prop_literal(ctx, from_idx, "latitude");
        if (duk_is_undefined(ctx, -1)  || !duk_is_number(ctx, -1)) throwErrorByCtx(ctx, "OPNgetGCdistance first latitude is missing or invalid");
        firstLat = duk_get_number(ctx, -1);
        duk_pop(ctx);
        duk_get_prop_literal(ctx, from_idx, "longitude");
        if (duk_is_undefined(ctx, -1)  || !duk_is_number(ctx, -1)) throwErrorByCtx(ctx, "OPNgetGCdistance first longitude is missing or invalid");
        firstLon = duk_get_number(ctx, -1);
        duk_pop(ctx);
        duk_get_prop_literal(ctx, to_idx, "latitude");
        if (duk_is_undefined(ctx, -1)  || !duk_is_number(ctx, -1)) throwErrorByCtx(ctx, "OPNgetGCdistance second latitude is missing or invalid");
        secondLat = duk_get_number(ctx, -1);
        duk_pop(ctx);
        duk_get_prop_literal(ctx, to_idx, "longitude");
        if (duk_is_undefined(ctx, -1)  || !duk_is_number(ctx, -1)) throwErrorByCtx(ctx, "OPNgetGCdistance second longitude is missing or invalid");
        secondLon = duk_get_number(ctx, -1);
        duk_pop(ctx);
        duk_pop_2(ctx); // pop off both args
        distance = DistGreatCircle_Plugin(firstLat, firstLon, secondLat, secondLon);
        duk_push_number(ctx, distance);
        return 1;
        }

// wxString soundFile = wxString("/Applications/OpenCPN.app/Contents/SharedSupport/sounds/beep_ssl.wav");
static duk_ret_t playSound(duk_context *ctx) {   // play alarm sound
	wxString resolveFileName(wxString inputName, Console* pConsole, int mode);
 	wxString soundFile;
	duk_idx_t nargs = duk_get_top(ctx);  // number of args in call
	Console *pConsole = findConsoleByCtx(ctx);
	if (nargs > 0) soundFile = resolveFileName(duk_get_string(ctx, 0), pConsole, 0);
	else {
	    wxString* directory = GetpSharedDataLocation();
TRACE(999, *directory);
    	soundFile = *directory   + "sounds" + wxFileName::GetPathSeparator() + "beep_ssl.wav";
TRACE(999, soundFile);
    	}
    duk_pop_n(ctx, nargs);
	bool OK = false;
	OK = PlugInPlaySoundEx(soundFile);
	duk_push_boolean(ctx, OK);
	return 1;
        }
        
static duk_ret_t isOnline(duk_context *ctx) {   // check if online
	duk_push_boolean(ctx, OCPN_isOnline());
	return 1;
	}
	
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
	
duk_ret_t getDriverAttributes(duk_context* ctx){
	// OCPNgetDriverAttributes(driverHandle)
	std::unordered_map<std::string, std::string> attributeMap;
	std::unordered_map<std::string, std::string> :: iterator i;
	DriverHandle handle;
	
//	duk_require_number(ctx, 0);
	handle = duk_get_string(ctx, 0);
	duk_pop(ctx);		
	attributeMap = GetAttributes(handle);
	duk_push_object(ctx);
	for (i = attributeMap.begin(); i != attributeMap.end(); i++){
		duk_push_string(ctx, i->first.c_str());
		duk_push_string(ctx, i->second.c_str());
		duk_put_prop(ctx, -3);
		}
	return 1;			
	}
	
duk_ret_t getCanvasView(duk_context* ctx){
	PlugIn_ViewPort vp = pJavaScript_pi->m_currentViewPort;
	duk_push_object(ctx); // construct the object
		duk_push_number(ctx, vp.view_scale_ppm);
			duk_put_prop_literal(ctx, -2, "ppm");
		duk_push_number(ctx, vp.chart_scale);
			duk_put_prop_literal(ctx, -2, "chartScale");
//		duk_push_number(ctx, vp.ref_scale);	// missing from plugin's definition of class
//			duk_put_prop_literal(ctx, -2, "refScale");			
		duk_push_object(ctx); // construct the centre point
			duk_push_number(ctx, vp.clat);
				duk_put_prop_literal(ctx, -2, "latitude");
			duk_push_number(ctx, vp.clon);
				duk_put_prop_literal(ctx, -2, "longitude");
			duk_put_prop_literal(ctx, -2, "centre");	
		duk_push_number(ctx, vp.pix_width);
			duk_put_prop_literal(ctx, -2, "pixWidth");
		duk_push_number(ctx, vp.pix_height);
			duk_put_prop_literal(ctx, -2, "pixHeight");		
	return 1;
	}
    
// ÑÑÑÑÑÑ API registrations follow

void ocpn_apis_init(duk_context *ctx) { // register the OpenCPN APIs
    duk_idx_t duk_push_c_function(duk_context *ctx, duk_c_function func, duk_idx_t nargs);
     
    duk_push_global_object(ctx);
    
    /* create the getNames objects */
/*    for (int i = 0; i < END_OF_OPCN_GETGUID_OPTIONS; i++){
        duk_push_string(ctx, getNames[i]);Ä
        duk_push_int(ctx,i);
        duk_def_prop(ctx, -3, DUK_DEFPROP_SET_ENUMERABLE);
    }
*/
    /* add the fuctions */
    
    duk_push_string(ctx, "OCPNgetActiveDriverHandles");
    duk_push_c_function(ctx, getDriverHandles, 0 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "OCPNgetDriverAttributes");
    duk_push_c_function(ctx, getDriverAttributes, 1 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNpushNMEA");
    duk_push_c_function(ctx, NMEA0183push, DUK_VARARGS /*number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNpushNMEA0183");
    duk_push_c_function(ctx, NMEA0183push, DUK_VARARGS /*number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNpushNMEA2000");
    duk_push_c_function(ctx, NMEA2kPush, 5 /*number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNpushText");
    duk_push_c_function(ctx, pushText, 2 /*number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNsendMessage");
    duk_push_c_function(ctx, sendMessage, DUK_VARARGS /*variable number of arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNgetMessageNames");
    duk_push_c_function(ctx, getMessageNames, 0 /* no args */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "OCPNonMessageName");
    duk_push_c_function(ctx, onMessageName, DUK_VARARGS /* args */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNonAllMessageName");
    duk_push_c_function(ctx, onMessageNamePersist, DUK_VARARGS /* args */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    
    duk_push_string(ctx, "OCPNonNMEAsentence");
    duk_push_c_function(ctx, onNMEA0183, DUK_VARARGS /* args */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNonNMEA0183");
    duk_push_c_function(ctx, onNMEA0183, DUK_VARARGS /* args */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNonAllNMEA0183");
    duk_push_c_function(ctx, onNMEA0183Persist, DUK_VARARGS /* args */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    
    duk_push_string(ctx, "OCPNonNMEA2000");
    duk_push_c_function(ctx, onNMEA2k, DUK_VARARGS /* args */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
     duk_push_string(ctx, "OCPNonAllNMEA2000");
    duk_push_c_function(ctx, onNMEA2kPersist, DUK_VARARGS /* args */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "OCPNonActiveLeg");
    duk_push_c_function(ctx, onActiveLeg, DUK_VARARGS /* args */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNonContextMenu");
    duk_push_c_function(ctx, onContextMenu, DUK_VARARGS);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "OCPNgetARPgpx");
    duk_push_c_function(ctx, getARPgpx, 0 /* no args */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "OCPNgetNavigation");
    duk_push_c_function(ctx, getNavigation, 0 /* no args */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "OCPNonNavigation");
    duk_push_c_function(ctx, onNavigation, DUK_VARARGS /* args */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNonAllNavigation");
    duk_push_c_function(ctx, onNavigationAll, 1 /* 1 args */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNgetNavigationK");
    duk_push_c_function(ctx, getNavigationK, 0 /* no args */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
            
    duk_push_string(ctx, "OCPNgetNewGUID");
    duk_push_c_function(ctx, getNewGUID, 0 /* 1 arg */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNgetWaypointGUIDs");
    duk_push_c_function(ctx, getWaypointGUIDs, DUK_VARARGS);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNgetActiveWaypointGUID");
    duk_push_c_function(ctx, getActiveWaypointGUID, 0 /* 0 args */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNgetSingleWaypoint");
    duk_push_c_function(ctx, getSingleWaypoint, 1 /* arg is GUID */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "OCPNaddSingleWaypoint");
    duk_push_c_function(ctx, addSingleWaypoint, DUK_VARARGS);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNupdateSingleWaypoint");
    duk_push_c_function(ctx, updateSingleWaypoint, 1);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "OCPNgetVectorPP");
    duk_push_c_function(ctx, getVectorPP, 2);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNgetGCdistance");
    duk_push_c_function(ctx, getGCdistance, 2);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNgetPositionPV");
    duk_push_c_function(ctx, getPositionPV, 2);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "OCPNdeleteSingleWaypoint");
    duk_push_c_function(ctx, deleteSingleWaypoint, 1 /* 1 arg */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNgetRouteGUIDs");
    duk_push_c_function(ctx, getRouteGUIDs, DUK_VARARGS);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "OCPNgetActiveRouteGUID");
    duk_push_c_function(ctx, getActiveRouteGUID, 0 /* 0 args */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "OCPNgetRoute");
    duk_push_c_function(ctx, getRouteByGUID, 1 /* arg is GUID */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNaddRoute");
    duk_push_c_function(ctx, addRoute, 1 /* arg is GUID */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNupdateRoute");
    duk_push_c_function(ctx, updateRoute, 1);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNdeleteRoute");
    duk_push_c_function(ctx, deleteRoute, 1);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNrefreshCanvas");
    duk_push_c_function(ctx, OCPNrefreshCanvas, 0);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNcentreCanvas");
    duk_push_c_function(ctx, OCPNcentreCanvas, DUK_VARARGS);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    duk_push_string(ctx, "OCPNgetTrackGUIDs");
    duk_push_c_function(ctx, getTrackGUIDs, DUK_VARARGS);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNgetTrack");
    duk_push_c_function(ctx, getTrackByGUID, 1 /* arg is GUID */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNaddTrack");
    duk_push_c_function(ctx, addTrack, 1 /* arg is GUID */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNupdateTrack");
    duk_push_c_function(ctx, updateTrack, 1);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNdeleteTrack");
    duk_push_c_function(ctx, deleteTrack, 1);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNgetCursorPosition");
    duk_push_c_function(ctx, getCursorPosition, 0);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNgetPluginConfig");
    duk_push_c_function(ctx, getPluginConfig, 1);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNgetOCPNconfig");
    duk_push_c_function(ctx, getOCPNconfig, 0);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNgetAISTargets");
    duk_push_c_function(ctx, getAISTargetsArray, 0);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNsoundAlarm");
    duk_push_c_function(ctx, playSound, DUK_VARARGS);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNisOnline");
    duk_push_c_function(ctx, isOnline, 0 /* 0 arg */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "OCPNgetCanvasView");
    duk_push_c_function(ctx, getCanvasView, 0 /* nargs */);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_pop(ctx);
}
