/**************************************************************************
* Project:  OpenCPN
* Purpose:  JavaScript Plugin
* Author:   Tony Voss 16/05/2020
*
* Copyright Ⓒ 2024 by Tony Voss
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, under which
* you must including a copy of these terms in all copies
* https://www.gnu.org/licenses/gpl-3.0.en.html
***************************************************************************
*/

#ifndef icp_h
#define icp_h

// sets  configuration to compile - whether to include certain options
// if the component is defined, it will be included
// if undefine it will be omitted

#include "wx/ipc.h"
// #include "wx/ipcsetup.h"

class jsConnection : public wxConnection {
    // adds pointer to console to a connection so we can find it from an event
public:
	Console* m_pConsole;
	
	jsConnection(Console* pConsole) {
		m_pConsole = pConsole;
		};
		
	~jsConnection(void) { };
	
    virtual bool OnExecute(const wxString& topic, const void *data, size_t size, wxIPCFormat format);
    virtual const void *OnRequest(const wxString& topic, const wxString& item, size_t *size, wxIPCFormat format);
    virtual bool OnPoke(const wxString& topic, const wxString& item, const void *data, size_t size, wxIPCFormat format);
    virtual bool OnStartAdvise(const wxString& topic, const wxString& item);
    virtual bool OnStopAdvise(const wxString& topic, const wxString& item);
    virtual bool DoAdvise(const wxString& item, const void *data, size_t size, wxIPCFormat format);
    virtual bool OnDisconnect();

    // topic for which we advise the client or empty if none
    wxString m_advise;

protected:
    // the data returned by last OnRequest(): we keep it in this buffer to
    // ensure that the pointer we return from OnRequest() stays valid
    wxCharBuffer m_requestData;
};

class jsTCPServer : public wxTCPServer {
public:
    jsTCPServer();
    virtual ~jsTCPServer();

    void Disconnect();
    bool IsConnected() { return m_connection != NULL; }
    jsConnection *GetConnection() { return m_connection; }

    void Advise();
    bool CanAdvise() { return m_connection && !m_connection->m_advise.empty(); }

    virtual wxConnectionBase *OnAcceptConnection(const wxString& topic);

protected:
    jsConnection *m_connection;
};

class JsConnection: public wxConnection
{
public:
    JsConnection(void);
    ~JsConnection(void);
 
    bool OnAdvise(const wxString& topic, const wxString& item, char *data,
                  int size, wxIPCFormat format)
    {
        wxMessageBox(topic, data);
    }
};
 
class JsClient: public wxClient
{
public:
    JsClient(void) { }
 
    wxConnectionBase* MakeConnection(	const wxString &host, const wxString & 	service, const wxString & topic )
    {
        return new JsConnection;
    }
};
	
#endif // icp_h

