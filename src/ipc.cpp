/**************************************************************************
* Project:  OpenCPN
* Purpose:  JavaScript Plugin
* Author:   Tony Voss 25/09/2022
*
* Copyright Ⓒ 2025 by Tony Voss
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, under which
* you must including a copy of these terms in all copies
* https://www.gnu.org/licenses/gpl-3.0.en.html
***************************************************************************
*/

#include "buildConfig.h"

#ifdef IPC	// Inter Process Communication
// This feature is compiled optionally
// As of plugin V2.0 it is incomplete and should not be included

#include "wx/wx.h"
#include "wx/ipc.h"
#include "duktape.h"
#include "JavaScriptgui_impl.h"
#include "ipc.h"

// temp stuff during experimentation
jsTCPServer *tcpServer;

jsTCPServer::jsTCPServer() : wxTCPServer(){
	m_connection = nullptr;
	}
	
jsTCPServer::~jsTCPServer() {
	Disconnect();
	}
	
wxConnectionBase *jsTCPServer::OnAcceptConnection(const wxString& topic){
	};
	
void jsTCPServer::Disconnect(){
	};
	
void jsTCPServer::Advise(){
	};

duk_ret_t JStcpOpenServer(duk_context *ctx){ // JStcpAddServer(eventHandler, address, port) - returns socketID ???
	// at present, called with 0 args
	tcpServer = new jsTCPServer;
	wxString service = "9000";
	bool outcome = tcpServer->Create(service);
	duk_push_boolean(ctx, outcome);
	return 1;
	}
	
duk_ret_t JStcpCloseServer(duk_context *ctx){ // JStcpAddServer(eventHandler, address, port) - returns socketID ???
	// at present, called with 0 args
	bool outcome;
	
	if (tcpServer != nullptr){
		tcpServer->Disconnect();
		delete tcpServer;
		tcpServer = nullptr;
		outcome = true;
		}
	else outcome = false;
	duk_push_boolean(ctx, outcome);
	return 1;
	}


void register_tcp(duk_context *ctx){
    duk_push_global_object(ctx);
    
    duk_push_string(ctx, "JStcpOpenServer");
    duk_push_c_function(ctx, JStcpOpenServer, 3 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "JStcpCloseServer");
    duk_push_c_function(ctx, JStcpCloseServer, 3 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    }
    

#endif		// IPC

#ifdef IGNORE_REST	// the rest of this is for ideas only

#include "wx/wx.h"
#include "wx/socket.h"
#include "duktape.h"
#include "JavaScriptgui_impl.h"

extern JavaScript_pi* pJavaScript_pi;
void throwErrorByCtx(duk_context *ctx, wxString message);
Console* findConsoleByName(wxString name);

#if 0
// compiles OK

void onDialogEvent(wxCommandEvent& event){
	};
	
void testItA(){
	wxDialog *pConsole = new wxDialog(pJavaScript_pi->m_parent_window,  wxID_ANY, "dummy");
	pConsole->Bind(wxEVT_BUTTON, &onDialogEvent, wxID_ANY);
}


//does not compile

void onSocketEvent(wxSocketEvent& event){
	};
	
void testItB(){
	wxIPV4address address;
	address.AnyAddress();
	wxSocketServer *pServer = new wxSocketServer(address);
	pServer->SetEventHandler(&onSocketEvent, wxID_ANY);
	}
#endif

#if 1

#define SOCKET_ID -1
BEGIN_EVENT_TABLE(Console, m_Console)
    EVT_SOCKET(SOCKET_ID,  Console::onSocketEvent)
END_EVENT_TABLE()

void pushSocketDescriptor(duk_context *ctx, int ID, int type, int status, int lastEvent){	// puts a socket descriptor onto the duktape stack
	    duk_push_object(ctx);
        duk_push_int(ctx, ID);
            duk_put_prop_literal(ctx, -2, "ID");
        duk_push_int(ctx, type);
            duk_put_prop_literal(ctx, -2, "type");
        duk_push_int(ctx, status);
            duk_put_prop_literal(ctx, -2, "status");
        duk_push_int(ctx, lastEvent);
            duk_put_prop_literal(ctx, -2, "lastEvent");
        }
#endif

/*
duk_ret_t JSsocketAddServer(duk_context *ctx){ // JSsocketsAddServer(eventHandler, address, port) - returns socketID
	Console *findConsoleByCtx(duk_context *ctx);
	wxString extractFunctionName(duk_context *ctx, duk_idx_t idx);
	SocketRecord socket;
	wxSocketBase* pSocket;
	wxIPV4address address;
	bool result;
	Console *pConsole = findConsoleByCtx(ctx);
    if (!duk_is_function(ctx, 0)) pConsole->throw_error(ctx, "JSsocketsAddServer first argument must be function");
    socket.functionName = extractFunctionName(ctx, 0);
	result = address.LocalHost();
	if (!result) pConsole->throw_error(ctx, wxString::Format("JSsocketAddServer address invalid"));
	int port = duk_get_int(ctx, 2);
	if (port < 1) pConsole->throw_error(ctx, wxString::Format("JSsocketAddServer called with port %i - requires > 1", port));
	result = address.Service(port);
	if (!result) pConsole->throw_error(ctx, wxString::Format("JSsocketAddServer service/port invalid"));
//	pSocket = (wxSocketBase*) new wxSocketServer(address);
	pSocket = (wxSocketBase*) new wxSocketClient();
//	if (!pSocket->IsOk()){
//		pConsole->throw_error(ctx, wxString::Format("JSsocketAddServer failed to create server"));
//		}
	pSocket->SetEventHandler(*pConsole, SOCKET_ID);
	pSocket->SetNotify(wxSOCKET_CONNECTION | wxSOCKET_INPUT_FLAG | wxSOCKET_OUTPUT| wxSOCKET_LOST_FLAG);
	pSocket->Notify(true);
	if (!pSocket->IsOk()){
		pConsole->throw_error(ctx, wxString::Format("JSsocketAddSocket failed to open server"));
		}
	socket.pSocket = pSocket;
	socket.socketID = pJavaScript_pi->nextID++;
	pConsole->mSockets.Add(socket);  // add this socket to array
	duk_pop_3(ctx);
	pushSocketDescriptor(ctx, socket.socketID, 1, socket.pSocket->IsConnected()?1:0, 0);
	return 1;
	}
*/
/*	
duk_ret_t JSsocketAddServer(duk_context *ctx){ // JSsocketsAddServer(eventHandler, address, port) - returns socketID
	Console *findConsoleByCtx(duk_context *ctx);
	wxString extractFunctionName(duk_context *ctx, duk_idx_t idx);
	SocketRecord socket;
	wxSocketBase* pSocket;
	wxIPV4address address;
	bool result;
	Console *pConsole = findConsoleByCtx(ctx);
    if (!duk_is_function(ctx, 0)) pConsole->throw_error(ctx, "JSsocketAddServer first argument must be function");
    socket.functionName = extractFunctionName(ctx, 0);
	result = address.LocalHost();
	if (!result) pConsole->throw_error(ctx, wxString::Format("JSsocketAddServer address invalid"));
	int port = duk_get_int(ctx, 2);
	if (port < 1) pConsole->throw_error(ctx, wxString::Format("JSsocketAddServer called with port %i - requires > 1", port));
	result = address.Service(port);
	if (!result) pConsole->throw_error(ctx, wxString::Format("JSsocketAddServer service/port invalid"));
	pSocket = (wxSocketBase*) new wxSocketServer(address);
	pSocket->SetEventHandler(*pConsole, SOCKET_ID);
	pSocket->SetNotify(wxSOCKET_CONNECTION | wxSOCKET_INPUT_FLAG | wxSOCKET_OUTPUT| wxSOCKET_LOST_FLAG);
	pSocket->Notify(true);
//	{ wxSocketServer* pServer = (wxSocketServer*) pSocket; pServer->Connect(address, false);}

	if (!pSocket->IsOk()){
		pConsole->throw_error(ctx, wxString::Format("JSsocketAddSocket failed to open server"));
		}		
	socket.pSocket = pSocket;
	socket.socketID = pJavaScript_pi->nextID++;
	socket.type = 1;
	pConsole->mSockets.Add(socket);  // add this socket to array	
	duk_pop_3(ctx);
	pushSocketDescriptor(ctx, socket.socketID, 1, socket.pSocket->IsConnected()?1:0, 0);
	return 1;
	}
8?
	
duk_ret_t JSsocketAddClient(duk_context *ctx){ // JSsocketsAddClient(eventHandler, address, port) - returns socketID
	Console *findConsoleByCtx(duk_context *ctx);
	wxString extractFunctionName(duk_context *ctx, duk_idx_t idx);
	SocketRecord socket;
	wxSocketBase* pSocket;
	wxIPV4address address;
	bool result;
	Console *pConsole = findConsoleByCtx(ctx);
    if (!duk_is_function(ctx, 0)) pConsole->throw_error(ctx, "JSsocketAddClient first argument must be function");
    socket.functionName = extractFunctionName(ctx, 0);
	result = address.LocalHost();
	if (!result) pConsole->throw_error(ctx, wxString::Format("JSsocketAddClient address invalid"));
	int port = duk_get_int(ctx, 2);
	if (port < 1) pConsole->throw_error(ctx, wxString::Format("JSsocketAddClient called with port %i - requires > 1", port));
	result = address.Service(port);
	if (!result) pConsole->throw_error(ctx, wxString::Format("JSsocketAddClient service/port invalid"));
	pSocket = (wxSocketBase*) new wxSocketClient();
	pSocket->SetEventHandler(*pConsole, SOCKET_ID);
	pSocket->SetNotify(wxSOCKET_CONNECTION | wxSOCKET_INPUT_FLAG | wxSOCKET_OUTPUT| wxSOCKET_LOST_FLAG);
	pSocket->Notify(true);
	{ wxSocketClient* pClient = (wxSocketClient*) pSocket; pClient->Connect(address, false);}
	if (!pSocket->IsOk()){
		pConsole->throw_error(ctx, wxString::Format("JSsocketAddSocket failed to open client"));
		}
	socket.pSocket = pSocket;
	socket.socketID = pJavaScript_pi->nextID++;
	socket.type = 2;
	pConsole->mSockets.Add(socket);  // add this socket to array	
	duk_pop_3(ctx);
	pushSocketDescriptor(ctx, socket.socketID, 2, socket.pSocket->IsConnected()?1:0, 0);
	return 1;
	}
	
duk_ret_t JSsocketDestroy(duk_context *ctx){	// JSsocketsDestroy(socketID)
	Console *findConsoleByCtx(duk_context *ctx);
	int socketID;
	bool result;
	SocketRecord socket;
	wxSocketServer* pServer;
	int s;
    Console *pConsole = findConsoleByCtx(ctx);
    
    TRACE(3, "Destroying socket for console " + pConsole->mConsoleName);
    if (duk_is_object(ctx, 0)) {	// we have been given a socket descriptor
    	duk_get_prop_string(ctx, -1, "ID");
		socketID = duk_get_int(ctx, -1);
		duk_pop(ctx);
    	}
    else socketID = duk_get_int(ctx, 0);	// if not a descriptor, then expect just the ID
    int count = (int)pConsole->mSockets.GetCount();
    for (s = 0; s < count; s++){
        if (pConsole->mSockets[s].socketID == socketID){
        	socket = pConsole->mSockets[s];	// found it
        	break;
        	}
        }
    if (s >= count) pConsole->throw_error(ctx, wxString::Format("JSsocketDestroy called with unknown socketID %i", socketID));
    if (socket.pSocket->IsConnected()) socket.pSocket->Close();
    pConsole->mSockets.RemoveAt(s);
    duk_pop(ctx);
    duk_push_boolean(ctx, true);
    return 1;
    }
    
duk_ret_t JSsocketStatus(duk_context *ctx){	// JSsocketsStatus(socketID)
	Console *findConsoleByCtx(duk_context *ctx);
	int socketID;
	bool result;
	SocketRecord socket;
	wxSocketServer* pServer;
	int s;
    Console *pConsole = findConsoleByCtx(ctx);
    
    if (duk_is_object(ctx, 0)) {	// we have been given a socket descriptor
    	duk_get_prop_string(ctx, -1, "ID");
		socketID = duk_get_int(ctx, -1);
		duk_pop(ctx);
    	}
    else socketID = duk_get_int(ctx, 0);	// if not a descriptor, then expect just the ID
    int count = (int)pConsole->mSockets.GetCount();
    for (s = 0; s < count; s++){
        if (pConsole->mSockets[s].socketID == socketID){
        	socket = pConsole->mSockets[s];	// found it
        	break;
        	}
        }
    if (s >= count) pConsole->throw_error(ctx, wxString::Format("JSsocketStatus called with unknown socketID %i", socketID));	
    duk_pop(ctx);
    pushSocketDescriptor(ctx, socket.socketID, socket.type, socket.pSocket->IsConnected()?1:0, 0);
    return 1;
    }
	
#if 0	
	int op = duk_get_int(ctx, 0);	// which operation
	switch (op){
		case 0: // clear down
			{
			if (nargs != 1) pConsole->throw_error(ctx, wxString::Format("JSsocket call with op %i but %i args", op, nargs));
			if (pServer != nullptr) pServer->Destroy();
			duk_pop_n(ctx, nargs);
			duk_push_boolean(ctx, true);
			return 1;
			}
		case 1: //create server
			{
			if (nargs != 3) pConsole->throw_error(ctx, wxString::Format("JSsocket call with op %i but %i args", op, nargs));
			wxIPV4address address;
			address.AnyAddress();
			address.Service(duk_get_string(ctx, 2));
			wxMessageBox("Got op 1 with address " + address.IPAddress());
			pServer = new wxSocketServer(address);
			pServer->Accept(false /* no wait */);
			if (!pServer->IsOk()){
				pConsole->throw_error(ctx, wxString::Format("JSsocket failed to create server"));
				}
			socket.pSocket = pServer;
			pConsole->mSockets.Add(socket);  // add this socket to array
			break;
			}
		default: pConsole->throw_error(ctx, wxString::Format("JSsocket call with invalid op %i", op));
			break;
			}
	return 1;
	}

void register_sockets(duk_context *ctx){
    duk_push_global_object(ctx);
    
    duk_push_string(ctx, "JSsocketAddServer");
    duk_push_c_function(ctx, JSsocketAddServer, 3 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "JSsocketAddClient");
    duk_push_c_function(ctx, JSsocketAddClient, 3 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
  
    duk_push_string(ctx, "JSsocketDestroy");
    duk_push_c_function(ctx, JSsocketDestroy, 1 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
    
    duk_push_string(ctx, "JSsocketStatus");
    duk_push_c_function(ctx, JSsocketStatus, 1 /* arguments*/);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

    };
    
#endif