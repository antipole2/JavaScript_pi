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

#ifndef JavaScript_impl_h
#define JavaScript_impl_h
#include "wx/window.h"
#include "wx/string.h"
#include "wx/datetime.h"
#include <wx/stc/stc.h>
#include "duktape.h"
#include "JavaScript_pi.h"
#include "JavaScriptgui.h"
#include "scaling.h"
#include <vector>
#include "buildConfig.h"
#include "consolePositioning.h"
#include <wx/event.h>
#include "wx/socket.h"
#include "wx/datetime.h"
#include "jsDialog.h"

#if DUK_DUMP
#define MAYBE_DUK_DUMP duk_push_context_dump(ctx);cout<<"Duktape context dump:\n"<<duk_to_string(ctx, -1)+_("\n");duk_pop(ctx);
//#define MAYBE_DUK_DUMP duk_push_context_dump(ctx);message(STYLE_ORANGE, _(""), _("Duktape context dump:\n")+duk_to_string(ctx, -1)+_("\n"));duk_pop(ctx);
#define ERROR_DUMP {message(STYLE_ORANGE, _(""), _("error dump\n")+(duk_is_error(ctx, -1)?(_("Error object with " )+duk_safe_to_string(ctx, -1)+_("\n")):_("No error object\n")));}\
MAYBE_DUK_DUMP
#endif  //  DUK_DUMP

using namespace std;
typedef wxString jsFunctionNameString_t;
typedef wxString messageNameString_t;
wxString JScleanOutput(wxString given);

extern JavaScript_pi *pJavaScript_pi;
bool isURLfileString(wxString);

#define MAX_SENTENCE_TYPES 50    // safety limit in case of coding error

class MessagePair  // holds OPCN messages seen, together with JS callback function name, if to be called back
    {
public:
    messageNameString_t messageName;
    jsFunctionNameString_t functionName;
    bool persist = false;	// true if to stay listening
    };
WX_DECLARE_OBJARRAY(MessagePair, MessagesArray);

// declare wxStyledText styles
enum {
    STYLE_BLACK,
    STYLE_RED,
    STYLE_BLUE,
    STYLE_ORANGE,
    STYLE_GREEN,
    STYLE_BOLD,
    STYLE_UNDERLINE
    };
    
// console short cuts    
enum {
    ID_CONSOLE_FORMAT = wxID_HIGHEST + 200
    // future:
    // ID_CONSOLE_RUN,
    // ID_CONSOLE_CLEAR
	};


/******************************** Callbacks ************************************
Starting with v3, all callbacks (apart from onExit()) are implemented through a unified approach.
This is based on callbackEntry objects pointed to in the vector
std::vector<std::shared_ptr<callbackEntry>> mCallbacks;

Each callback is one of CallbackType declared in JavaScript.h
*/

struct Datagram {
	std::unique_ptr<wxDatagramSocket> socket;
	wxIPV4address sendAddress;	// 255.255.255.255:port
	wxIPV4address lastSender;  // address of who sent last received
	};
		
struct callbackEntry {
   	callbackEntry() = default;
    ~callbackEntry() = default;

    // ❌ Disable copy
    callbackEntry(const callbackEntry&) = delete;
    callbackEntry& operator=(const callbackEntry&) = delete;
    // ✔ Enable move
    callbackEntry(callbackEntry&&) = default;
    callbackEntry& operator=(callbackEntry&&) = default;
   
	callbackID id = wxWindow::NewControlId();
	CallbackType  type;
	bool persistant = false;			// true if the callback is to be retained after a callback
	void* func_heapptr = nullptr;		// ptr to duktape handler on heap
	std::shared_ptr<wxTimer>	timer = nullptr;
	std::shared_ptr<ObservableListener> listener = nullptr;
	Datagram datagram;
//	std::shared_ptr<wxDatagramSocket> socket = nullptr;
	wxString parameter = wxEmptyString;
	wxMenuItem* pmenuItem = nullptr;
	std::vector<callbackID>	subMenuIDs = {};
	wxString string1 = wxEmptyString;
	wxString string2 = wxEmptyString;
	int integer1 = 0;
	std::unique_ptr<JsDialog> dialogue = nullptr;	
	};
		
/*
In addition to attributes required for all callbacks, the structure includes attributes needed for specific types of callback.
Some of these are used for different purposes in different types of callback.
They have been given aliases to make them meaningful for that particular callback type.
*/
// for context menus
#define _MENU_NAME string1	// menu item name
#define _MENU_TYPE string2	// type of object: "" for chart else "Waypoint", "Route", "Track" or "AIS"
#define _SUBMENU_PARENT_ID	integer1  // for a submenu, the ID of the parent entry
// for NMEA
#define _IDENT string1	// NMEA0183 identifier
#define _PGN integer1		// For N2K messages
// for onConsoleResult
#define _CONSOLE_NAME string1	// this and next may not be used any more?
#define _BRIEF string2

/*
The callbackEntry entities are managed through methods included below
and through the cancelCallbackPerCtx() function declared in function.cpp

Cleaning up when a callbackEntry is deleted is achieve through the share_ptr mechanism
but certain types need bespoke cleaning.
*/
/************************* End of callback structure declarations **************************/

#ifdef SOCKETS
#include "wx/socket.h"
class SocketRecord {
public:
    int socketID;
    int type;	// 0 not in use; 1 server; 2 client
    wxSocketBase * pSocket;
    jsFunctionNameString_t functionName;
    wxString argument;  // optional argument to pass to function
	};
#define MAX_SOCKETS 10   // safety limit of number of sockets
WX_DECLARE_OBJARRAY(SocketRecord, SocketRecordsArray);
#endif

#ifdef IPC
#include "wx/sckipc.h"
#endif	// IPC

struct ConsoleReplyResult{
    wxString        result;
    Completions     type;
    };

struct AlertDetails {// holds details of open alert dialogue
    JsDialog    *palert;    // points to open alert else nullptr
    wxPoint     position = wxPoint(wxDefaultPosition);	// in DIP
    wxString    alertText;  // the currently displayed text
    };

struct Brief  { // holds details of briefs and call-backs for this console   
	int 		callbackId = 0;				// if non-zero is the callback ID for the console to be called back
	wxString    ConsoleNameToCallback;
    bool        hasBrief = false;  // there is a brief in theBrief
    wxString    theBrief;
    };
	
struct wxFileFcb {	// to hold wxFile structures
	int id;	// identifying file control block number
	wxFile* pFile;
	wxFileFcb()
		: id(rand()) {}	// set id to random number
	};
	
struct location {	// holds location and size of Console
	bool	set {false};	// whether this has a location
	wxPoint	position;
	wxSize	size;
	};
    
class Console : public m_Console {
public:
    wxString    mConsoleName;
    duk_context* mpCtx {nullptr};      // Pointer to the Duktape context
    bool        mWaitingCached = true; // true if mWaiting valid cached state
    bool        mWaiting {false};       // true if waiting for something
    bool        mRunningMain {false}; // true while main is running
    bool        mJSrunning {false}; // true while any JS code is running
    bool        mscriptToBeRun {false};   // true if script to be run because it was chained
    status_t    mStatus;     // the status of this process
    
    // window handling
    location	m_parkedLocation;
    location 	m_notParkedLocation;

	std::vector<std::shared_ptr<callbackEntry>> mCallbacks;	// where we store callbacks
	
#ifdef SOCKETS
	SocketRecordsArray	mSockets;		// array of sockets
#endif

    int 			mModalCount;	// count of modal windows
    wxPoint			mDialogPosition;	// default dialogue position in DIP
    AlertDetails    mAlert;        // details of alert dialog
    wxDialog* 		mpMessageDialog;	// the wxWidgets message d
    void*	 		m_exitHandler;  // handler to call on exit, else nullptr

    // duktape timeout handling
    wxLongLong m_pcall_endtime;         // time we must end execution (msecs since 1970)
    long       m_time_to_allocate;      // the time we allocate (msec)
    long       m_timeout_check_counter; // counter of number of timeout checks
    
    // result handling - can be made explicit by scriptResult or stopScript
    bool        m_explicitResult;    // true if using explicit result
    wxString    m_result {wxEmptyString};   // the result if explici

    Brief       mBrief;         		// brief for this console
    
    // file handling
    wxString    mFileString;   // file loaded/saved, if any
    wxString    mFolderPath;    //  of current folder
    wxFileName  mCurrentFile;
    bool        m_autoRun;      // true if script to be auto run after activation of plugin
    bool        mWaitingToRun {false};  // true if an auto-run script has been loaded but not yet run
    
    bool		m_parked;		// true if this console parked
    
    std::vector<wxFileFcb> m_wxFileFcbs;	// will hold file fcb entries
    
    wxString	m_remembered {"{}"};	// _remember variable saved as JSON-like string - initially empty structure
    wxString	m_versionControl {"{}"};	// _versionControl variable saved as JSON-like string - initially empty structure

    void OnClearScript( wxCommandEvent& event );
    void OnCopyAll( wxCommandEvent& event );
    void OnLoad( wxCommandEvent& event );
    void OnSave (wxCommandEvent& event);
    void OnSaveAs ( wxCommandEvent& event );
    void OnRun( wxCommandEvent& event );
    void OnAutoRun(wxCommandEvent& event);
    void OnClearOutput( wxCommandEvent& event );
    void OnTools( wxCommandEvent& event );
    void OnHelp( wxCommandEvent& event );
    void OnPark( wxCommandEvent& event );
    void OnActivate(wxActivateEvent& event);
    void OnClose( wxCloseEvent& event );
    void HandleNMEA0183(ObservedEvt& ev, int id);
    void HandleNMEA2k(ObservedEvt& ev, int id);
    void HandleNavdata(ObservedEvt& ev, int id);
    void HandleNotificationAction(ObservedEvt& ev, int id);
    void HandleMessage(ObservedEvt& ev, int id);
    void HandleTimer(wxTimerEvent& event);
    void PollSocket(wxTimerEvent& event);
    void OnScriptCharAdded(wxStyledTextEvent& e);
    void FormatIndentation();
	void OnConsoleFormat(wxCommandEvent&);
    
#ifdef SOCKETS
	DECLARE_EVENT_TABLE()
#endif	// SOCKETS

#ifdef IPC
	wxTCPServer*	m_IPCserver;
#endif	// IPC
    
public:
    Console(wxWindow *parent, wxString consoleName)
    		: m_Console(parent, wxID_ANY, consoleName
    		)
        	{
			mConsoleName = consoleName;
			
			// for handling short cuts
			wxAcceleratorEntry entries[1];
#ifdef __WXMAC__
			entries[0].Set(wxACCEL_CMD, 'F', ID_CONSOLE_FORMAT);
#else
			entries[0].Set(wxACCEL_CTRL, 'F', ID_CONSOLE_FORMAT);
#endif
			wxAcceleratorTable accel(1, entries);
			SetAcceleratorTable(accel);
			Bind(wxEVT_MENU, &Console::OnConsoleFormat, this, ID_CONSOLE_FORMAT);
			// end of stuff for short cuts
			}
        
#if  !SCREEN_RESOLUTION_AVAILABLE
		// provide dummy methods for those not available
		// needs to work with wxPoint & wxSize
		template <typename T>
		T ToDIP(T point){
			return point;
			}
		
		template <typename T>
		T FromDIP(T point){
			return point;
			}
#endif
 
	void setup(	// iniset up console details
	 			wxString welcome = wxEmptyString,	// initial text for output pane
	 			wxString fileString = wxEmptyString, // for the fileString box
	 			bool autoRun = false				// whether to auto start
	 		){
	 	void JSlexit(wxStyledTextCtrl* pane);
	 	wxPoint checkPointOnScreen(wxPoint point);
 	 	TRACE(33, wxString::Format("%s->setup welcome=%s; fileString=%s; autoRun=%s",
 	 		mConsoleName, welcome, fileString, autoRun?"true":"false"));

		// position and size of this and associated windows
		location aLocation;
		aLocation.position = ToDIP(checkPointOnScreen(FromDIP(NEW_CONSOLE_POSITION))); // check position on screen;
		aLocation.size = NEW_CONSOLE_SIZE;
		aLocation.set = true;
		setLocation(aLocation);
		setConsoleMinClientSize();
		mDialogPosition = ToDIP(checkPointOnScreen(FromDIP(DEFAULT_DIALOG_POSITION)));
		mAlert.position = ToDIP(checkPointOnScreen(FromDIP(DEFAULT_ALERT_POSITION)));
		
        // script pane set up
        JSlexit(this->m_Script);  // set up lexing
        m_Script->SetWrapMode(wxSTC_WRAP_WORD);
        m_Script->SetWrapIndentMode(wxSTC_WRAPINDENT_INDENT);
        m_Script->SetWrapVisualFlags(wxSTC_WRAPVISUALFLAG_START);
        m_Script->Bind(wxEVT_STC_CHARADDED, &Console::OnScriptCharAdded, this);
        m_fileStringBox->SetValue(fileString);
        auto_run->Hide();
        if (m_fileStringBox->GetValue() != wxEmptyString) {    // we have a script to load
            wxString    script;
            wxString    fileString = this->m_fileStringBox->GetValue();
            if (isURLfileString(fileString)){
            	if (!OCPN_isOnline()) message(STYLE_RED, _("Load file " + fileString + "OpenCPN not on-line"));
				else {
					wxString getTextFile(wxString fileString, wxString* pText, int timeout);
					wxString script;
					wxString result = getTextFile(fileString, &script, 10);
					if (result == wxEmptyString){	// looking good
						m_Script->ClearAll();
						m_Script->WriteText(script);
						m_Script->DiscardEdits();	// mark as saved
						}
					else message(STYLE_RED, _("Load file cannot download " + fileString + " - " + result));
					}
				}
            else if (!wxFileExists(fileString)){
                message(STYLE_RED, _("Load file " + fileString + ": ") +_("File does not exist"));
                auto_run->SetValue(false);
                }
            else {
                m_Script->LoadFile(fileString);
                auto_run->Show();
                mWaitingToRun = this->auto_run->GetValue();
                }
            }
        else {
        	wxString patchString = wxEmptyString;
        	if (PLUGIN_VERSION_PATCH > 0) patchString = wxString(_(".")) << PLUGIN_VERSION_PATCH;
        	wxString welcome = wxString(_("print(\"Hello from the JavaScript plugin v")) << PLUGIN_VERSION_MAJOR << "." << PLUGIN_VERSION_MINOR\
        			<<  patchString  << " " << PLUGIN_VERSION_COMMENT << " " << PLUGIN_VERSION_DATE << " JS v" << DUK_VERSION << ("\\n\");\n\"All OK\";");
            m_Script->AddText(welcome); // some initial script
            m_Script->DiscardEdits();	// Let this be cleared without check
            }
        m_fileStringBox->SetValue(fileString);
        auto_run->SetValue(autoRun);
       
        // output pane set up
        m_Output->StyleSetSpec(STYLE_RED, _("fore:#FF0000"));
        m_Output->StyleSetSpec(STYLE_BLUE, _("fore:#2020FF"));
        m_Output->StyleSetSpec(STYLE_ORANGE, _("fore:#FF7F00"));
        m_Output->StyleSetSpec(STYLE_GREEN, _("fore:#00CE00"));
        m_Output->StyleSetSpec(STYLE_BOLD, _("bold"));
        m_Output->StyleSetSpec(STYLE_UNDERLINE, _("underline"));
        m_Output->SetWrapMode(wxSTC_WRAP_WORD);
        m_Output->SetWrapIndentMode(wxSTC_WRAPINDENT_INDENT);
        m_Output->SetWrapVisualFlags(wxSTC_WRAPVISUALFLAG_START);
	 	m_Output->AppendText(welcome);
	 	
        mBrief.hasBrief = false;
        mBrief.theBrief = wxEmptyString;
	 	consoleReset();	// regular initialisation	 	
	 	mWaitingToRun = autoRun;
	 	}
	 
	void consoleReset(){	// Resets console, clearing out settings
		mpCtx = nullptr;
		mRunningMain = false;
		mStatus = 0;
		mWaitingCached = false;
		m_explicitResult = false;
		m_result = wxEmptyString;
		m_time_to_allocate = 1000;   //default time allocation (msecs)
		mModalCount = 0;
		mAlert.palert = nullptr;
		mAlert.alertText = wxEmptyString;
		mpMessageDialog = nullptr;
		m_exitHandler = nullptr;
		// just in case file controls not finalised from any previous run
		{
			int count = m_wxFileFcbs.size();
			if (count > 0){
				message(STYLE_ORANGE, _("Warning: file controls not cleaned up by previous run - doing it now"));
				for (auto it = begin (m_wxFileFcbs); it != end (m_wxFileFcbs); ++it) {
					delete it->pFile;
					}
				m_wxFileFcbs.clear();
				}
			}
		#ifdef SOCKETS
			clearSockets();
		#endif
		mscriptToBeRun = false;
		run_button->SetLabel("Run");
		if (mAlert.position.y == -1){ // shift initial position of alert  away from default used by dialogue
			mAlert.position.y = 150;
			}
		m_wxFileFcbs.clear();
		return;
		}

	void clearBrief(){  // initialise brief by clearing it down
		mBrief.hasBrief = false;
		}
 
  Completions run(wxString script) { // compile and run the script
	   Completions outcome;       // result of JS run
	   wxString JScleanString(wxString given);
	   wxString result;
	   void fatal_error_handler(void *udata, const char *msg);
	   void duk_extensions_init(duk_context *ctx, Console* pConsole);
	   void ocpn_apis_init(duk_context *ctx);
	   void ConsoleHandle_init(duk_context *ctx);
   
	   TRACE(3, this->mConsoleName + "->run() entered");
	   if (script == wxEmptyString) return HAD_ERROR;  // ignore
	   script = JScleanString(script);
	   consoleReset();
	   mpCtx = duk_create_heap(NULL, NULL, NULL, NULL, fatal_error_handler);  // create the Duktape context
	   if (!mpCtx) {
		message(STYLE_RED, _("Plugin logic error: Duktape failed to create heap"));
		  return HAD_ERROR;
		  }
	   duk_extensions_init(mpCtx, this);  // register our own extensions
	   ocpn_apis_init(mpCtx);       // register our API extensions
	   run_button->SetLabel(_("Stop"));
	   mRunningMain = true;
	   duk_push_string(mpCtx, script.mb_str());   // load the script
	   startTimeout();
	   duk_int_t dukOutcome = duk_peval(mpCtx);    // run code **** this is where it all happens ****
	   clearTimeout();
	   mRunningMain = false;
	   outcome = afterwards(dukOutcome);
      TRACE(4, wxString::Format("%s->run() completed with outcome %d",mConsoleName, outcome ));
	   return outcome;
	   }
    
    void doRunCommand(Brief brief){
        // this is implemented as a method so we can lazy call it with CallAfter
        if (run_button->GetLabel() == _("Run")){
            TRACE(0, "------------------ Console " + mConsoleName + " about to run");
            Completions outcome;
            mBrief = brief;
            outcome = run(m_Script->GetText());
       		if (!isBusy()) wrapUp(outcome);
            }
        else { // Stop button clicked - we have a script running - kill it off
            TRACE(0, "------------------ Console " + mConsoleName + " stopping");
            m_explicitResult = true;
            m_result = _("script was stopped");
            mStatus = 0;
            wrapUp(STOPPED);
            }
	    }
    
    Completions executeCallableNargs(void* heapptr, int nargs){
        // execute callable on heap wih nargs arguments already on duktape stack
        // returns result with nothing on stack
        Console *findConsoleByCtx(duk_context *ctx); // ******
        duk_int_t duk_outcome;
        duk_context *ctx = mpCtx;
        TRACE(66, wxString::Format("Entered executeCallableNargs with nargs %i\n", nargs) + findConsoleByCtx(ctx)->dukDump());
        if (mStatus.test(HAD_ERROR)){ // an error has been thrown elsewhere and duktape has unwound etc.
        	return(HAD_ERROR);	// don't try and call the handler
        	}
        duk_push_heapptr(mpCtx, heapptr);
        TRACE(66, wxString::Format("Pushed heapptr\n") + findConsoleByCtx(ctx)->dukDump());
        if (!duk_is_callable(mpCtx, -1)) {
        	message(STYLE_RED, "Callback error - no longer callable");
        	duk_pop_n(ctx, nargs);
            return HAD_ERROR;
            }
        // the handler is on top above the nargs.  Need to shuffle it below them.
        duk_idx_t pos = -1;
        for (int i = nargs; i > 0; i--, pos--) duk_swap(ctx, pos, pos-1);
        TRACE(66, "Before call " + dukDump());
        startTimeout();
        duk_outcome = duk_pcall(ctx, nargs);   // call with nargs 
        clearTimeout();
        TRACE(66, "After call " + dukDump() + _("Result is ") + ((duk_outcome == DUK_EXEC_SUCCESS)?"success":"error"));
		return(afterwards(duk_outcome));
		}
    
    void callHandlerInOtherConsole(Brief brief, ConsoleReplyResult result){
    	// callback to other console with outcome of onConsoleResult
        // this is implemented as a method so we can lazy-call the handler with CallAfter
		// build object from result to give handler
		Console* findConsoleByName(wxString name);
		wxString consoleNameToCallback = brief.ConsoleNameToCallback;
		std::shared_ptr<callbackEntry> pEntry = getCallbackEntry(brief.callbackId, true);
        if (!pEntry){
        	prep_for_throw(mpCtx, wxString::Format("%s->callHandlerInOtherConsole: Console %s is not waiting", mConsoleName, consoleNameToCallback));
			duk_throw(mpCtx);
        	}
		Console* consoleToCallback = findConsoleByName(consoleNameToCallback);
		if (consoleToCallback == nullptr) {
		   	prep_for_throw(mpCtx, wxString::Format("%s->callHandlerInOtherConsole: Console to callback not known", mConsoleName));
			duk_throw(mpCtx);
			}
		if (!consoleToCallback->isWaiting()){
			prep_for_throw(mpCtx, wxString::Format("%s->callHandlerInOtherConsole: Console %s is not waiting", mConsoleName, consoleNameToCallback));
			duk_throw(mpCtx);
			}
		// build object in other console's stack and invoke its handler
        duk_push_object(consoleToCallback->mpCtx);
        duk_push_int(consoleToCallback->mpCtx, result.type);
        duk_put_prop_literal(consoleToCallback->mpCtx, -2, "type");
        duk_push_string(consoleToCallback->mpCtx , result.result.c_str());
        duk_put_prop_literal(consoleToCallback->mpCtx, -2, "value");
		Completions outcome = consoleToCallback->executeCallableNargs(pEntry->func_heapptr, 1);
		if (!isBusy()) consoleToCallback->wrapUp(outcome);
        }

/************ callbackEntry management methods start ********************/
	struct DialogueDeleter {
		void operator()(callbackEntry* p) const {
			TRACE(987, wxString::Format("callbackEntry %d deletion for CB_DIALOGUE", p->id));
			if (p->dialogue){
				// watch out for modal dialogues
				if (p->dialogue->IsModal()){
					if (p->dialogue->GetReturnCode() == wxID_NONE){ // It is still active
						p->dialogue->EndModal(wxID_CANCEL);
						}
					}
				p->dialogue->Destroy();
				}
			delete p;
		}
	};
	
	struct ContextMenuDeleter {
		Console* self;	
		void operator()(callbackEntry* p) const {
			TRACE(987, wxString::Format("callbackEntry %d deletion for CB_CONTEXT_MENU", p->id));
			RemoveCanvasContextMenuItem(p->id);	
			// clean submenu entries
			if (p->func_heapptr == nullptr) {
				std::shared_ptr<callbackEntry> waste;
				for (auto id : p->subMenuIDs) waste = self->getCallbackEntry(id, true);
				}
			delete p;
		}
	};

	// Sockets - because we are polling, the default delete is sufficient - no custom deleter
	
	std::shared_ptr<callbackEntry>newCallbackEntry(CallbackType type){
    constexpr size_t MaxCallbacks = 400;
    if (mCallbacks.size() >= MaxCallbacks) {
        clearCallbacks();
        prep_for_throw(mpCtx,
            wxString::Format("Attempted to set up more than %i callbacks", MaxCallbacks));
        duk_throw(mpCtx);
    	}
    auto* raw = new callbackEntry{};
    raw->type = type;
    std::shared_ptr<callbackEntry> pEntry;
    // we need custom deleters for some types
    switch (type) {
        case CB_DIALOGUE:
            pEntry = std::shared_ptr<callbackEntry>(
                raw,
                DialogueDeleter{}
            	);
            break;
        case CB_CONTEXT_MENU:
            pEntry = std::shared_ptr<callbackEntry>(
                raw,
                ContextMenuDeleter{this}
            	);
            break;           
        default:
            // Fallback: default deleter
            pEntry = std::shared_ptr<callbackEntry>(
                raw);
            break;
    		}

    mCallbacks.push_back(pEntry);
    return pEntry;
	}
      		
	std::shared_ptr<callbackEntry>getCallbackEntry(int id, bool forceErase){
		// given a callback id, returns the entry.
		// If the entry is not persistant, it is removed.
		// If forceErase true, it is removed regardless.		
		for (auto it = mCallbacks.begin(); it != mCallbacks.end(); ++it) {
			auto& pEntry = *it;	
			if (pEntry && pEntry->id == id) {
				auto result = pEntry;	// Save the shared_ptr to return
				TRACE(987, wxString::Format("getCallbackEntry matched entry with id %d pEntry->persistant is %s  forceErase is %s",
					id, (pEntry->persistant)?"true":"false", forceErase?"true":"false"));
				if (!pEntry->persistant || forceErase) {
					mCallbacks.erase(it);
					}
				return result;
				}
			}
		TRACE(987, wxString::Format("getCallbackEntry failed to match entry with id %d", id));
		return nullptr;		// Not found
		}
		
	std::shared_ptr<callbackEntry>getCallbackEntryByType(CallbackType type, bool forceErase){
		// given a callback type, returns the first matching entry.
		// If the entry is not persistant, it is removed.
		// If forceErase true, it is removed regardless.
		for (auto it = mCallbacks.begin(); it != mCallbacks.end(); /* ++it */) {
			auto& pEntry = *it;	
			if (pEntry && pEntry->type == type) {
				auto result = pEntry;	// Save the shared_ptr to return
				TRACE(987, wxString::Format("getCallbackEntryByType matched entry with id %d pEntry->persistant is %s  forceErase is %s",
					pEntry->id, (pEntry->persistant)?"true":"false", forceErase?"true":"false"));
				if (!pEntry->persistant || forceErase) {
					mCallbacks.erase(it);
					}
				return result;
				}
			else it++;
			}
		TRACE(987, wxString::Format("getCallbackEntryByType failed to match entry with type %d", type));	
		return nullptr;		// Not found
		}
		
	bool checkCallbackEntryMenuName(wxString MenuType, wxString menuName){
		// Checks whether menuName already in use for MenuType
		// returns true if already in use, else false
		std::shared_ptr<callbackEntry> pEntry;
		for (auto it = mCallbacks.cbegin(); it < mCallbacks.cend(); ++it){
			pEntry = (*it);
			if ((pEntry->_MENU_TYPE == MenuType) && (pEntry->_MENU_NAME == menuName)) return true;
			}
		return false;
		}
		
	void clearCallbacks(){// clears all reasons a console might be called back
		mCallbacks.clear();
		clearAlert();
        mRunningMain = false;
        if (mpMessageDialog != nullptr){
        	TRACE(4,mConsoleName + "->clearCallbacks() found mpMessageDialog - clearing");
        	mpMessageDialog->Destroy();
        	mpMessageDialog = nullptr;
        	}
#ifdef SOCKETS
		clearSockets();
#endif
		mWaitingCached = false;
		run_button->SetLabel(_("Run"));
		}
/**************** end of callbackEntry methods ********************/

    void startTimeout(){
        extern Console* pConsoleBeingTimed;
        m_timeout_check_counter = 0;
        if (m_time_to_allocate > 0) m_pcall_endtime =  wxGetUTCTimeMillis() + m_time_to_allocate;
        else m_pcall_endtime = 0;
        mStatus.reset(TIMEOUT);
        mJSrunning = true;
        pConsoleBeingTimed = this;
        }
    
    void clearTimeout() {   // to cancel the timeout
        extern Console* pConsoleBeingTimed;
        m_pcall_endtime = 0;
        mJSrunning = false;
        pConsoleBeingTimed = nullptr;
        }
    
    bool isWaiting() { // returns true if waiting for anything
        // for efficiencey, we may have a cached result in mWaiting
        if (mWaitingCached) {
            TRACE(66,mConsoleName + "->isWaiting() returning cached result " + (mWaiting?"true":"false"));
            return mWaiting;
            }
        TRACE(66,mConsoleName + "->isWaiting() doing full check");
        bool result = false;
        TRACE(66, wxString::Format("%s->isWaiting() found mCallbacks.size() is %d", mConsoleName, mCallbacks.size()));
        if (
        	(mCallbacks.size() > 0) ||
#ifdef SOCKETS
            (mSockets.GetCount() > 0) ||
#endif
            (mpMessageDialog != nullptr) ||
            (mAlert.palert != nullptr) ){
            	TRACE(75, mConsoleName + "->isWaiting() 1st if true");
                result = true;
                }
        mWaiting = result;
        mWaitingCached = true;
        TRACE(66,mConsoleName + "->isWaiting() returning new result " + (mWaiting?"true":"false"));
        return mWaiting;
        }
    
    bool isBusy(){  // tests if waiting or running main script
        return(mRunningMain || isWaiting());
        }  
        
	Completions afterwards(duk_int_t dukOutcome){	// called after execution of JavaScript
		// if certain flags set, it will have thown an error
		// we check for this before other types of error
		// For now we will mark not busy in case the following are true
		wxString result;
		Completions outcome;
#if TRACE_YES
        wxString statusesToString(status_t mStatus);
#endif
		
        TRACE(4, wxString::Format("%s->afterwards() with dukOutcome %d and status now %s",mConsoleName, dukOutcome, statusesToString(mStatus)));
        mWaiting = false;
		mWaitingCached = true;
		// here we determine the order of presidence of the various outcomes 
		if (mStatus.test(TIMEOUT)) return TIMEOUT;
        if (mStatus.test(CLOSING)) {
            duk_pop(mpCtx);	// pop off the error object
            return CLOSING;
            }
		if (mStatus.test(STOPPED)) return STOPPED;	// not a real error
		if (mStatus.test(TOCHAIN)) return TOCHAIN;	// not a real error
		if (dukOutcome != DUK_EXEC_SUCCESS){ // a real error
			mStatus.set(HAD_ERROR);
			return HAD_ERROR;	
			}
		
		mWaitingCached = false;   // now we are past those cases, get a re-check
	   result = duk_to_string(mpCtx, 0);
	   if (!m_explicitResult) m_result = result; // for when not explicit result
	   duk_pop(mpCtx);  // pop result
	   outcome = DONE; // starting assumption
   
	   if (isWaiting()){ // if script set up call-backs with nominated functions, check they exist
		  outcome = MORETODO;
		  TRACE(4, mConsoleName + "->afterwards()  completed with something to wait for");
		  }
		return(outcome);
		}

	void wrapUp(Completions reason) {    // clear down and maybe destroy ctx
		TRACE(66, wxString::Format("%s->wrapUp() called with reason %d", mConsoleName, reason));
		wrapUpWorks(reason);
/*
		if (!isBusy()){
			TRACE(66, wxString::Format("%s->wrapUp() ending with reason %d", mConsoleName, reason));
			if (mpCtx != nullptr) { // for safety - nasty consequences if no context
				if ((reason == DONE) || (reason == STOPPED) || (reason == TOCHAIN) || (reason == CLOSED)) {	// save the enduring variable
					TRACE(4, mConsoleName + "->wrapUp() will save _remember");
					duk_push_global_object(mpCtx);
					
					int OK = duk_get_prop_literal(mpCtx, -1, "_remember");
					if (OK)	m_remembered = duk_json_encode(mpCtx, -1);
					else m_remembered = "{}";	// play safe - if no _remember set as undefined
					duk_pop(mpCtx);
					
					OK = duk_get_prop_literal(mpCtx, -1, "_versionControl");
					if (OK)	m_versionControl = duk_json_encode(mpCtx, -1);
					else m_versionControl = "{}";	// play safe - if no _versionControl set as undefined
					duk_pop(mpCtx);
					}
				TRACE(66, mConsoleName + "->wrapUp() destroying ctx");
				duk_destroy_heap(mpCtx);
				mpCtx = nullptr;
				}
            TRACE(0, "------------------ Console " + mConsoleName + " wrapped up");
			}
*/
		}

    void wrapUpWorks(Completions reason) {    // consider clearing down and destroying context etc.
    	// this is guts of wrapUp as sub-function to allow multiple returns to wrapUp
       // wxString statusesToString(status_t mStatus);
        wxString formErrorMessage(duk_context *ctx);
        auto cleanup = [this](){	// this used in many places in this function, so define here
        	TRACE(4, wxString::Format("In cleanup have mpCtx  %s",(mpCtx != nullptr)?"true":"false"));
            clearCallbacks();
			if (mpCtx) duk_destroy_heap(mpCtx);
			mpCtx = nullptr;
        	};
        mStatus.reset(MORETODO);
        mStatus.set(reason);
#if TRACE_YES
        wxString statusesToString(status_t mStatus);
#endif
        TRACE(4, wxString::Format("%s->wrapUpWorks() with status %d and now %s\n%s",mConsoleName, reason, statusesToString(mStatus), dukDump()));
#if TRACE_YES
        {wxString dump {""};
            if (mBrief.hasBrief){
				dump += "mBrief\n";
				dump += "\ttheBrief:\t\t\t" + (mBrief.hasBrief?_("true"):_("false")) + "\n";
				dump += "\ttheBrief:\t\t\t" + mBrief.theBrief + "\n";
				}
            else dump += "No brief\n";
            TRACE(4, dump);
			}
#endif
        if (reason == TIMEOUT){
            message(STYLE_RED, m_result = "Script timed out");
            cleanup();
			replyToConsole(reason);	// only does so if due
            return;
			}
		if (reason == CLOSING){	// this differes from a real error in that no error message is generated
			reason = CLOSED;	// we now treat it further just like a simple Close
			cleanup();
			replyToConsole(reason);	// only does so if due
			return;
            }
        if (reason == HAD_ERROR){   // output error message
            message(STYLE_RED,  m_result = formErrorMessage(mpCtx));
            cleanup();
			replyToConsole(reason);	// only does so if due
            return;
            }
        if (reason == MORETODO){
        	TRACE(4, "wrapUpWorks says MORETODO");
        	return;
        	}
        if (reason & (STOPPED | TOCHAIN | DONE)) { // wrapping up normally
            if (m_exitHandler != nullptr){    // we have an onExit function to run
            	TRACE(4, wxString::Format("%s->wrapUpWorks() has exit handler about to be run", mConsoleName));
				mStatus.reset(DONE);
				mStatus.set(INEXIT);
				Completions onExitCompletion = executeCallableNargs(m_exitHandler,0);
				mStatus.reset(INEXIT);
				TRACE(4, wxString::Format("%s->wrapUpWorks() back from exit handler", mConsoleName));
				if (onExitCompletion == TIMEOUT){
					TRACE(4, wxString::Format("%s->wrapUpWorks() back from exit handler", mConsoleName));
					message(STYLE_RED, m_result = wxString::Format("%s->wrapUpWorks() onExit handler timed out", mConsoleName));
					cleanup();
					clearCallbacks();
					replyToConsole(onExitCompletion);
					return;
					}	
				if (onExitCompletion == HAD_ERROR){ // error during exit function				
					TRACE(4, wxString::Format("%s->wrapUpWorks() with exit handler error stack:\n", mConsoleName) + dukDump());
					message(STYLE_RED,  m_result = "onExit function " + formErrorMessage(mpCtx));
					cleanup();
					clearCallbacks();
					replyToConsole(onExitCompletion);
					return;
					}
				}
//		if (!isBusy()){
			TRACE(66, wxString::Format("%s->wrapUp() ending with reason %d", mConsoleName, reason));
			if (mpCtx != nullptr) { // for safety - nasty consequences if no context
				if ((reason == DONE) || (reason == STOPPED) || (reason == TOCHAIN) || (reason == CLOSED)) {	// save the enduring variable
					TRACE(4, mConsoleName + "->wrapUp() will save _remember");
					duk_push_global_object(mpCtx);
					
					int OK = duk_get_prop_literal(mpCtx, -1, "_remember");
					if (OK)	m_remembered = duk_json_encode(mpCtx, -1);
					else m_remembered = "{}";	// play safe - if no _remember set as undefined
					duk_pop(mpCtx);
					
					OK = duk_get_prop_literal(mpCtx, -1, "_versionControl");
					if (OK)	m_versionControl = duk_json_encode(mpCtx, -1);
					else m_versionControl = "{}";	// play safe - if no _versionControl set as undefined
					duk_pop(mpCtx);
					}
				TRACE(66, mConsoleName + "->wrapUp() destroying ctx");
				duk_destroy_heap(mpCtx);
				mpCtx = nullptr;
				}
            TRACE(0, "------------------ Console " + mConsoleName + " wrapped up");
			}
		cleanup();
		if (isBusy())	message(STYLE_RED, wxString::Format("%s->wrapUpWorks() logic error - isBusy() true", mConsoleName));
		// now for the result
		if (!this->m_explicitResult && this->m_result == wxEmptyString)this->m_result = "undefined";
		if (!this->m_explicitResult || this->m_result != wxEmptyString)
		this->message(STYLE_BLUE, _("result: ") + this->m_result);
        if (reason == TOCHAIN){  // a new script will have been loaded to chain to and any brief set
            TRACE(4, wxString::Format("%s->wrapUpWorks() chaining",mConsoleName));
            mStatus.reset(TOCHAIN);        
            CallAfter(&Console::doRunCommand, mBrief);
            return;	// return now so we leave brief intact for next in chain
            }
     	else {
     		replyToConsole(reason);	// only replies if so set up
			}
        if (reason != CLOSED) Show();   // make console visible
		}
        
    void replyToConsole(Completions reason){ // callback to console
        TRACE(4, wxString::Format("%s->replyToConsole() with reason %d",mConsoleName, reason));
        if (mBrief.callbackId == 0) return; // no reply to do
        TRACE(4, wxString::Format("%s->replyToConsole() callbackId: %d replying to console %s",mConsoleName, mBrief.callbackId, mBrief.ConsoleNameToCallback));
        Console* findConsoleByName(wxString name);
        Console* pOtherConsole = findConsoleByName(mBrief.ConsoleNameToCallback);
        if (!pOtherConsole) {
            // no console to report error - use first
            this->message(STYLE_RED, wxString::Format("%s->replyToConsole() console %s not found",mConsoleName,mBrief.ConsoleNameToCallback));
            }
        else if (!pOtherConsole->isWaiting()) this->message(STYLE_RED, wxString::Format("%s->replyToConsole() console %s not waiting for reply",mConsoleName,mBrief.ConsoleNameToCallback));
        else {
        	std::shared_ptr<callbackEntry> pCallersEntry = pOtherConsole->getCallbackEntry(mBrief.callbackId, true);
			if(!pCallersEntry){
			    this->message(STYLE_RED, wxString::Format("%s->replyToConsole() not found callbackEntry in console %s",mConsoleName,mBrief.ConsoleNameToCallback));
				return;
				}
        	ConsoleReplyResult result;
            result.result = m_result;
            result.type = reason;
            TRACE(4, wxString::Format("Replying to console %s with result %s & reason %d", mConsoleName, result.result, result.type));
            // construct reply in other console's duktape stack
            duk_context* otherCtx = pOtherConsole->mpCtx;;
            duk_push_object(otherCtx);
			duk_push_int(otherCtx, result.type);
			duk_put_prop_literal(otherCtx, -2, "type");
			duk_push_string(otherCtx , result.result.c_str());
			duk_put_prop_literal(otherCtx, -2, "value");
			void* othersHeapptr = pCallersEntry->func_heapptr;
			Completions outcome = pOtherConsole->executeCallableNargs(othersHeapptr, 1);
			mBrief.callbackId = 0;	// make sure we do not attempt this again
			if (!pOtherConsole->isBusy()) pOtherConsole->wrapUp(outcome);
            }
        }
    
    void clearAlert(){  // clears away any alert dialogue
        TRACE(4,this->mConsoleName + "->clearAlert()  " + ((this->mAlert.palert == nullptr)?"(None)":"Alert exists"));
        if (mAlert.palert != nullptr){ // only if have an alert displayed
            mAlert.position = ToDIP(mAlert.palert->GetPosition()); // remember alert position in DIP
            mAlert.palert->Close();
            mAlert.palert->Destroy();
            delete mAlert.palert;
            mAlert.palert = nullptr;
            mAlert.alertText = wxEmptyString;
            mWaitingCached = false;
            }
        }
    
#ifdef SOCKETS        
    void clearSockets()           {   // clear away any sockets for this console
        int count = mSockets.GetCount();
        for (int i = count-1; i >= 0; i--){   // do this backwards
	        if (mSockets[i].pSocket != nullptr) mSockets[i].pSocket->Close();        		
            mSockets.RemoveAt(i);
            }
        }
#endif
    /************************ window management ****************************/
    void message(int style, wxString message){
        void limitOutput(wxStyledTextCtrl* pText);
        message = JScleanOutput(message);	// internally, we are using DEGREE to represent degree - convert any back
        TRACE(5,mConsoleName + "->message() " + message);
        Show(); // make sure console is visible
    	makeBigEnough();
        wxStyledTextCtrl* output_window = m_Output;
        int long beforeLength = output_window->GetTextLength(); // where we are before adding text
        output_window->AppendText(message);
        output_window->AppendText("\n");
        int long afterLength = output_window->GetTextLength(); // where we are after adding text
#if (wxVERSION_NUMBER < 3100)
        output_window->StartStyling((int)beforeLength,255);   // 2nd parameter included Linux still using wxWidgets v3.0.2
#else
		 output_window->StartStyling((int)beforeLength);
#endif
        output_window->SetStyling((int)(afterLength-beforeLength-1), style);
		limitOutput(output_window);
        }
        
    void makeBigEnough(){	// ensure console at least minimum size
    	if (isParked()){
			unPark();
			this->message(STYLE_ORANGE,"Console unparked to show message");	// recursing!
			}
    	wxSize consoleSize = ToDIP(this->GetSize());	// using DIP size here
        if ((consoleSize.x < 200)|| (consoleSize.y < 400)){
        	consoleSize = FromDIP(wxSize(680,700));		// back to actual size
        	this->SetSize(consoleSize);
        	}
    	} 

    void setConsoleMinClientSize(){
    	// store parameters in DIP but minSize set in physical size here
    	double scale = SCALE(this);
    	wxSize minSize, size;
    	
    	wxSize textSize = this->GetTextExtent(mConsoleName);
    	minSize.x = pJavaScript_pi->m_parkingStub*scale + textSize.x + 3*scale;	// added 3 to be on safe side
    	TRACE(4, wxString::Format("setConsoleMinClientSize text %s, size X:%i Y: %i", mConsoleName, textSize.x, textSize.y));  	
    	minSize.y = 0;
    	SetMinClientSize(minSize);
    	}
    	
    location getLocation(){	// capture current console position and size
    	location now;
    	now.size = GetSize();
    	now.position = GetPosition();
    	now.set = true;
    	TRACE(25, wxString::Format("%s->getLocation()returning position\t%i\t%i\tsize %i\t%i", mConsoleName, now.position.x, now.position.y, now.size.x, now.size.y));
    	return now;
    	}
    	
    void setLocation (location loc){
    	if (!loc.set){
    	    message(STYLE_RED, _("setLocation called when not set - program error"));
    	    }
    	TRACE(25, wxString::Format("%s->setLocation() setting position\t%i\t%i\tsize %i\t%i", mConsoleName, loc.position.x, loc.position.y, loc.size.x, loc.size.y));
    	SetSize(loc.size);
    	SetPosition(loc.position);
    	}
    	
    void park(){	// park this console
    	if (isParked()) { // was already parked
    		setLocation(m_parkedLocation);	// might need re-minimizing
    		return;
    		}
    	m_notParkedLocation = getLocation();	//remember our unparked location
        int rightMost {0};    // most right hand edge of a parked console relative to frame
        bool foundParked {false};
        TRACE(25, wxString::Format("%s->park() parking called with minSize X:%i  Y:%i",
        	mConsoleName, GetMinSize().x, GetMinSize().y)); 
		wxSize size = GetMinClientSize();
		size.y = 0;	// we are zeroing the client size
#if defined(__LINUX__)
		size.y = 1;	// Linux does not work if y is 0
#endif
    	SetClientSize(size);
    	TRACE(25, wxString::Format("%s->park() readback of client size X:%i  Y:%i",
        	mConsoleName, GetClientSize().x, GetClientSize().y)); 
        TRACE(25, wxString::Format("%s->park() readback of size X:%i  Y:%i",
        	mConsoleName, GetSize().x, GetSize().y));
    	if (m_parkedLocation.set){	// have a parking slot
    	    TRACE(25, wxString::Format("%s->park() reparking", mConsoleName));
    		m_parkedLocation.size = GetSize();
    		setLocation(m_parkedLocation);	// repark
    		m_parked = true;
    		TRACE(25, wxString::Format("%s->park() readback of parked size X:%i  Y:%i",
        	mConsoleName, GetSize().x, GetSize().y));
    		TRACE(25, wxString::Format("%s->park() finished re-parking", mConsoleName));
    		return;
    		}
    	else {// first time parked - find horizontal place available avoiding other parked consoles
    	    TRACE(25, wxString::Format("%s->park() parking first time", mConsoleName));
    	    for (auto* pC : pJavaScript_pi->m_consoles){ // for each console
				int rhe {0};	// right hand edge within frame
				if (pC == this) continue;	// omit ourselves console from check            
				if (pC->isParked()){
					foundParked = true;
					wxPoint pCFramePos = pC->GetPosition();
					rhe = pCFramePos.x + pC->GetMinSize().x;
					}
				else if (pC->m_parkedLocation.set) {// not parked but has a parking place
					foundParked = true;
					rhe = pC->m_parkedLocation.position.x + pC->m_parkedLocation.size.x;
					}
				if (rhe > rightMost) rightMost = rhe;
				} 
			int newX = foundParked ? (rightMost + pJavaScript_pi->m_parkSep): pJavaScript_pi->m_parkFirstX;	// horizontal place for new parking in actual px
			m_parkedLocation.position = wxPoint(newX, pJavaScript_pi->m_parkingLevel);
			m_parkedLocation.size = GetSize();
			m_parkedLocation.set = true;
			setLocation(m_parkedLocation);
			TRACE(25, wxString::Format("%s->park() parking at X:%i  Y:%i frame", mConsoleName, m_parkedLocation.position.x, m_parkedLocation.position.y));
			m_parked = true;
			}
    	}
    	
    void unPark(){	//unpark this Console
    	void reviewParking();
    	TRACE(25, wxString::Format("%s->unpark() called with parking at X:%i  Y:%i frame", mConsoleName, m_parkedLocation.position.x, m_parkedLocation.position.y));

    	if (!isParked()) return;
    	m_parkedLocation = getLocation();	// remember where we parked
    	if (!m_notParkedLocation.set){	// have no unparked location - invent one
			m_notParkedLocation.position = FromDIP(NEW_CONSOLE_POSITION);
			m_notParkedLocation.size = FromDIP(NEW_CONSOLE_SIZE);
			m_notParkedLocation.set = true;
			}

    	setLocation(m_notParkedLocation);
    	
    	if (wxGetKeyState(WXK_SHIFT)){ // if shift key down, vacate parked location
    			m_parkedLocation.set = false;
    			reviewParking();
    			}
    	TRACE(25, wxString::Format("%s->unpark() completed to X:%i   Y:%i frame", mConsoleName, m_notParkedLocation.position.x, m_notParkedLocation.position.y));

    	m_parked = false;    	
    	}
    	
    bool isParked(){ //	returns true if console is parked      
    	if (!m_parked){
    		TRACE(25, wxString::Format("%s->isParked() already false", mConsoleName));
    		return false;
    		}
    	wxPoint posNow = GetPosition();
    	if ((abs(posNow.y - m_parkedLocation.position.y) < 4)){  // still parked (allowing small margin for error )
    		TRACE(25, wxString::Format("%s->isParked() found is parked", mConsoleName));
    		return true;
    		}
    	m_parked = false;	// has been moved
    	TRACE(25, wxString::Format("%s->isParked() found no longer parked", mConsoleName));
    	return false;
    	}
    	
    wxString deleteMe(){
    	// Delete this console if we can
    	void reviewParking();
    	if (pJavaScript_pi->m_consoles.size() < 2) return ("Cannot close the only console " + mConsoleName);
		if (isBusy()) return (mConsoleName + " is busy");
		if (mpCtx) return (mConsoleName + " still has dukTape context");
		Hide();
		for (auto it = pJavaScript_pi->m_consoles.begin(); it != pJavaScript_pi->m_consoles.end(); ++it){
			auto& pEntry = *it;
			if (pEntry == this){
				pJavaScript_pi->m_consoles.erase(it);			
				reviewParking();
				if (pJavaScript_pi->pTools != nullptr) pJavaScript_pi->pTools->setConsoleChoices();	// update if tools open
				wxTheApp->CallAfter([c = this] {delete c;});	// after we have returned, delete us
				return wxEmptyString;
				}
			}
		return ("program error " + mConsoleName + " not found in m_Consoles");
    	}
/***************************** end of window management **********************/
	    
	void prep_for_throw(duk_context *ctx, wxString message){
	// throw an error from within c++ code called from running script
	// either there is an error object on the stack or a message
	// Because of Windows, the actual throw has to be done after return!
	// ! do not call otherwise
	message = JScleanOutput(message);	// internally, we are using DEGREE to represent degree - convert any back
	TRACE(4, mConsoleName + "->prep_for_throw() " + message);
	m_result = wxEmptyString;    // supress result
	m_explicitResult = true;    // supress result
	if (!duk_is_error(ctx, -1)){
		// we do not have an error object on the stack
		TRACE(4, mConsoleName + "->prep_for_throw() pushing error object to stack");
		duk_push_error_object(ctx, DUK_ERR_ERROR, _("Console ") + mConsoleName + _(" - ") + message);  // convert message to error object
		}
	mRunningMain = false;
	}
	    
	void display_error(duk_context *ctx, wxString message){
	// display an error NOT from running script
	// either there is an error object on the stack or a message
		m_explicitResult = true;    // supress result
		m_result = duk_safe_to_string(ctx, -1);
		duk_pop(ctx);
		this->message(STYLE_RED, _("From display_error ") +  m_result);
		}   
    
    void floatOnParent(bool yes){	// set the wxFRAME_FLOAT_ON_PARENT style
    	long styles = GetWindowStyle();
		if (yes) styles |= wxFRAME_FLOAT_ON_PARENT;
		else styles ^= wxFRAME_FLOAT_ON_PARENT;
		SetWindowStyle(styles);
	    }
    
    wxString consoleDump(){    // returns string being dump of selected information from console structure
        wxString ptrToString(Console* address);
        wxString statusesToString(status_t mStatus);
        int i, count;
        wxString dump {""};
        dump += "Console name:\t\t" + mConsoleName + "\n";
        dump += "this:\t\t\t\t\t" +  ptrToString((Console *)this) + "\n";
        dump += "mStatus:\t\t\t\t" + statusesToString(mStatus) + "\n";
        dump += "has ctx:\t\t\t\t" + (mpCtx ? ("true"):_("false")) + "\n";
        dump += "mRunningMain:\t\t" + (this->mRunningMain?_("true"):_("false")) + "\n";
        dump += wxString::Format( "m_time_to_allocate:\t%ldms\n", this->m_time_to_allocate);
        dump += wxString::Format( "m_timeout_check_counter:\t%d\n", this->m_timeout_check_counter);
        dump += "mWaitingCached:\t\t" + (this->mWaitingCached?_("true"):_("false")) + "\n";
        dump += "mWaiting:\t\t\t" + (this->mWaiting?_("true"):_("false")) + "\n";
        dump += "mpMessageDialog:\t" +  ptrToString((Console *)mpMessageDialog) + "\n";
        dump += "isBusy():\t\t\t\t" + (this->isBusy()?_("true"):_("false")) + "\n";
        dump += "isWaiting():\t\t\t" + (this->isWaiting()?_("true"):_("false")) + "\n";
        dump += "auto_run:\t\t\t" + (this->auto_run ->GetValue()? _("true"):_("false")) + "\n";
        wxPoint screenPosition = GetPosition();
        wxPoint DIPposition = ToDIP(screenPosition);
        dump += wxString::Format("position:\t\t\t\tscreen x:%i y:%i\tDIP x:%i y:%i\n", screenPosition.x, screenPosition.y, DIPposition.x, DIPposition.y );
        wxSize size = GetSize();
        wxSize DIPsize = ToDIP(size);
        wxSize minSize = GetMinSize();
        dump += wxString::Format("Size():\t\t\t\tx:%i y:%i\tDIP\tx:%i y:%i\tMinSize() x:%i y:%i\n",
        	size.x, size.y,DIPsize.x, DIPsize.y, minSize.x, minSize.y );
        dump += wxString::Format("m_parked:\t\t\t%s\n", (m_parked ? _("true"):_("false")));
        dump += "isParked():\t\t\t" + (isParked() ?  _("true"):_("false")) + "\n";
        dump += !m_parkedLocation.set ? "No park location\n":
        	wxString::Format("parked Location\t\t%i\t%i\tsize %i\t%i\n", m_parkedLocation.position.x, m_parkedLocation.position.y, m_parkedLocation.size.x, m_parkedLocation.size.y);     
        dump += !m_notParkedLocation.set ? "No unparked location\n":
        	wxString::Format("notParked location\t%i\t%i\tsize %i\t%i\n", m_notParkedLocation.position.x, m_notParkedLocation.position.y, m_notParkedLocation.size.x, m_notParkedLocation.size.y);      
//        dump += "m_timerActionBusy:\t" + (this->mTimerActionBusy?_("true"):_("false")) + "\n";
        dump += _("Callbacks table\n");
        count = mCallbacks.size();
        if (count == 0) dump += _("\t(empty)\n");
        else {
            for (i = 0; i < count; i++){
            	std::shared_ptr<callbackEntry> pEntry = mCallbacks[i];
                dump += _("\t") + wxString::Format("id:%i\t\ttype:%i\tparameter:\"%s\"\tfunc_heapptr:%s\n\t\t\tstring1:\"%s\"\tstring2:\"%s\"\tinteger1:%i\tpersistant:%s\n",
                	pEntry->id, pEntry->type, pEntry->parameter, (pEntry->func_heapptr == nullptr)?"nullptr":"yes", pEntry->string1, pEntry->string2,
                	pEntry->integer1, pEntry->persistant?"true":"false");
                dump += "\tsubMenuIDs: [ ";
                for (auto it = pEntry->subMenuIDs.begin(); it != pEntry->subMenuIDs.end(); it++)
                	dump += wxString::Format("%d ", (*it));
                dump += "]\n";     
                }
            }

#ifdef SOCKETS
        dump += _("Sockets table\n");
        count = (int)this->mSockets.GetCount();
        if (count == 0) dump += _("\t(empty)\n");
        else {
            for (i = 0; i < count; i++){
                dump += wxString::Format("\tID %i\t%s\t%s\n", this->mSockets[i].socketID, this->mSockets[i].functionName,this->mSockets[i].argument);
                }
            }
#endif
        dump += wxString::Format("mDialogPosition:\tx=%d\ty=%d\n", mDialogPosition.x, mDialogPosition.y);
        dump += "m_exitHandler:\t" + ((m_exitHandler != nullptr)?_("set"):_("not set")) + "\n";
        dump += "m_explicitResult:\t\t" + (m_explicitResult?_("true"):_("false")) + "\n";
        dump += "m_result:\t\t\t" + m_result + "\n";
        dump += "mscriptToBeRun:\t" + (this->mscriptToBeRun?_("true"):_("false")) + "\n";
        dump += wxString::Format("Inter-console callback mBrief.callbackId: %d\tmBrief.ConsoleNameToCallback: %s\n", mBrief.callbackId, mBrief.ConsoleNameToCallback);
        if (mBrief.hasBrief){
            dump += "mBrief\n";
            dump += "\thasBrief:\t\t\t" + (mBrief.hasBrief?_("true"):_("false")) + "\n";
            dump += "\ttheBrief:\t\t\t" + mBrief.theBrief + "\n";
            }
        else dump += "No brief\n";
        dump += "m_remembered\t" + m_remembered + "\n";
        dump += "m_versionControl\t" + m_versionControl + "\n";
        int wxFileCount = m_wxFileFcbs.size();
        if (wxFileCount == 0) dump += "wxFile ids - none\n";
        else {
        	dump += "wxFile ids\n";
        	for (int f = 0; f < wxFileCount; f++) dump += wxString::Format("\t%d\n", m_wxFileFcbs[f].id);
        	}
        dump += dukDump();
        return dump;
        }
    
    wxString dukDump(){ // returns string being dump of duktape context
        if (!mpCtx) return "No Duktape context\n";
        else {
            wxString result;
            duk_push_context_dump(mpCtx);
            result = wxString(_("Duktape context dump:\n") + duk_to_string(mpCtx, -1) + _("\n"));
            duk_pop(mpCtx);
            return result;
        }
    }
        
#ifdef SOCKETS
	void onSocketEvent(wxSocketEvent& event){  // here when any socket event
		extern JavaScript_pi* pJavaScript_pi;
		void pushSocketDescriptor(duk_context *ctx, int ID, int type, int status, int lastEvent);
		wxSocketBase* pSocket;
		int s, count;
		int ID = 0;
		wxSocketNotify eventType;

		pSocket = event.GetSocket();
		eventType = event.GetSocketEvent();
		if (mSockets.IsEmpty()) return;  // ignore if nothing waiting
		// look for this socket in the array of sockets
		count = mSockets.GetCount();
		for (s = 0; s < count; s++){
			ID =mSockets[s].socketID;
			break;
			}
		SocketRecord record = mSockets[s];
		if (ID == 0) wxMessageBox("Program error: onSocketEvent for unknown socket");
		jsFunctionNameString_t thisFunction = record.functionName;
		pushSocketDescriptor(mpCtx, record.socketID, record.type, record.pSocket->IsConnected()?1:0, eventType);
		Completions outcome = executeFunction(thisFunction);
		if (outcome != MORE) wrapUp(outcome);
		}
#endif

	};


#endif  // JavaScript_impl_h
