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

#ifndef JavaScript_impl_h
#define JavaScript_impl_h

#include "wx/wx.h"
#include "wx/window.h"
#include "wx/string.h"
#include "wx/datetime.h"
#include "wx/arrimpl.cpp"
#include <wx/stc/stc.h>
#include "duktape.h"
#include "ocpn_plugin.h"
#include "JavaScript_pi.h"
#include "JavaScriptgui.h"
#include "trace.h"
#include <vector>
#include <bitset>
#include "buildConfig.h"
#include "consolePositioning.h"
#include <wx/event.h>

#define DUK_DUMP true
#if DUK_DUMP
#define MAYBE_DUK_DUMP duk_push_context_dump(ctx);cout<<"Duktape context dump:\n"<<duk_to_string(ctx, -1)+_("\n");duk_pop(ctx);
//#define MAYBE_DUK_DUMP duk_push_context_dump(ctx);message(STYLE_ORANGE, _(""), _("Duktape context dump:\n")+duk_to_string(ctx, -1)+_("\n"));duk_pop(ctx);
#define ERROR_DUMP {message(STYLE_ORANGE, _(""), _("error dump\n")+(duk_is_error(ctx, -1)?(_("Error object with " )+duk_safe_to_string(ctx, -1)+_("\n")):_("No error object\n")));}\
MAYBE_DUK_DUMP
#endif  //  DUK_DUMP

using namespace std;
typedef wxString jsFunctionNameString_t;
typedef wxString messageNameString_t;

extern JavaScript_pi *pJavaScript_pi;

class MessagePair  // holds OPCN messages seen, together with JS callback function name, if to be called back
    {
public:
    messageNameString_t messageName;
    jsFunctionNameString_t functionName;
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

#define MAX_SENTENCE_TYPES 50    // safety limit in case of coding error

// timers
#include "wx/datetime.h"
class TimeActions  // holds times at which function is to be called
    {
public:
    wxDateTime timeToCall;
    jsFunctionNameString_t functionName;
    wxString argument;  // optional argument to pass to function
    };
#define MAX_TIMERS 10   // safety limit of timers
WX_DECLARE_OBJARRAY(TimeActions, TimesArray);

// menus
//#include "wx/datetime.h"
class MenuActions
    {
public:
    int menuID;
    wxMenuItem* pmenuItem;
    wxString menuName;
    jsFunctionNameString_t functionName;
    wxString argument;  // optional argument to pass to function
    };
#define MAX_MENUS 10   // safety limit of number of menus
WX_DECLARE_OBJARRAY(MenuActions, MenusArray);

#ifdef SOCKETS
#include "wx/socket.h"
class SocketRecord
    {
public:
    int socketID;
    int type;	// 0 not in use; 1 server; 2 client
    wxSocketBase * pSocket;
    jsFunctionNameString_t functionName;
    wxString argument;  // optional argument to pass to function
}   ;
#define MAX_SOCKETS 10   // safety limit of number of sockets
WX_DECLARE_OBJARRAY(SocketRecord, SocketRecordsArray);
#endif

#ifdef IPC
#include "wx/sckipc.h"
#endif	// IPC

class ConsoleReplyAwaited
    {
public:
    Console*    pWaitedfrom;   // The console the console reply is awaited from
    wxString    consoleName;    // name of console
    wxString    functionToCall; // the function to call when console reply arrives
    };

WX_DECLARE_OBJARRAY(ConsoleReplyAwaited, ConsoleRepliesArray);

class ConsoleReplyResult{
public:
    wxString        result;
    Completions     resultType;
    wxString        functionName;   // function to be called
    };

// dialogues storage
class dialogElement{
public:
    wxString    type;
    wxString    stringValue;
    wxString    label;
    wxString    textValue;
    int         numberValue;
    int         width;
    int         height;
    int         multiLine;
    wxString    suffix;
    int         itemID; // The ID of the window element if anyitemID
    };

class DialogAction // hold details of open dialogue
    {
public:
    jsFunctionNameString_t functionName;    // function to be called when dialogue acted on
    wxDialog    *pdialog;    // points to open dialog else nullptr
    wxPoint     position = wxPoint(wxDefaultPosition);	// in DIP
    std::vector<dialogElement> dialogElementsArray;// will be an array of dialogue elements
    };

class AlertDetails // holds details of open dialogue
    {
public:
    wxDialog    *palert;    // points to open alert else nullptr
    wxPoint     position = wxPoint(wxDefaultPosition);	// in DIP
    wxString    alertText;  // the currently displayed text
    };

class Brief   // holds details of briefs and call-backs for this console
    {
public:
    bool        fresh; // this brief just posted - keep for collection
    bool        hasBrief;  // there is a brief in theBrief
    wxString    theBrief;
    bool        reply;   // true if console reply to be made
    wxString    briefingConsoleName;
    wxString    function;   // function to be called back
    };

class Console : public m_Console {
public:
    Console     *mpNextConsole {nullptr}; // -> next console in chained list of consoles
    wxString    mConsoleName;
    duk_context *mpCtx {nullptr};      // Pointer to the Duktape context
    bool        mWaitingCached = true; // true if mWaiting valid cached state
    bool        mWaiting {false};       // true if waiting for something
    bool        mRunningMain {false}; // true while main is running
    bool        mJSrunning {false}; // true while any JS code is running
    bool        mscriptToBeRun {false};   // true if script to be run because it was chained
    status_t    mStatus;     // the status of this process
    

    // callback management
    bool        mTimerActionBusy;  // true while handling timer event to stop them piling up
    MessagesArray   mMessages;   // The messages call-back table
    TimesArray      mTimes;       // Timers call-back table
    MenusArray      mMenus;         // Menus callback table
#ifdef SOCKETS
	SocketRecordsArray	mSockets;		// array of sockets
#endif
    DialogAction    mDialog;      // Dialogue call-back table
    AlertDetails    mAlert;        // details of alert dialog
    wxDialog* 		mpMessageDialog;	// the wxWidgets message dialogue if there is one
    jsFunctionNameString_t  m_NMEAmessageFunction;  // function to invoke on receipt of NMEA message, else ""
    jsFunctionNameString_t  m_activeLegFunction;  // function to invoke on receipt of active leg, else ""
    jsFunctionNameString_t m_exitFunction;  // function to call on exit, else ""
    int         mConsoleRepliesAwaited {0};   // number of console replies awaited

    // duktape timeout handling
    wxLongLong m_pcall_endtime;         // time we must end execution (msecs since 1970)
    long       m_time_to_allocate;      // the time we allocate (msec)
    long       m_timeout_check_counter; // counter of number of timeout checks
    
    // result handling - can be made explicit by scriptResult or stopScript
    bool        m_explicitResult;    // true if using explicit result
    wxString    m_result {wxEmptyString};   // the result if explicit
    bool        m_hadError;     // true if unwinding from dukpate after throing error
    Brief       mBrief;         // brief for this console
    
    // file handling
    wxString    mFileString;   // file loaded/saved, if any
    wxString    mFolderPath;    //  of current folder
    wxFileName  mCurrentFile;
    bool        m_autoRun;      // true if script to be auto run after activation of plugin
    bool        mWaitingToRun {false};  // true if an auto-run script has been loaded but not yet run

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
    void onActivate( wxActivateEvent& event );
    void OnMouse(wxMouseEvent& event);
    void OnActivate(wxActivateEvent& event);

#ifdef SOCKETS
	DECLARE_EVENT_TABLE()
#endif	// SOCKETS

#ifdef IPC
	wxTCPServer*	m_IPCserver;
#endif	// IPC

private:
    bool		m_parked;		// true if this console parked
    wxPoint		m_parkedPosition;	// if parked, parked position
    void OnClose( wxCloseEvent& event );
    
public:
	// Console constructor is given positions and sizes in DIP.  Constructor makes necessary adjustments.
    Console(wxWindow *parent, wxString consoleName,
    		wxPoint consolePosition = pJavaScript_pi->m_parent_window->ToDIP(wxPoint(300,100)),
    		wxSize consoleSize = pJavaScript_pi->m_parent_window->ToDIP(wxSize(738,800)),
    		wxPoint dialogPosition = wxPoint(150, 100), wxPoint alertPosition = wxPoint(90, 20), wxString fileString = wxEmptyString,
    		bool autoRun = false, wxString welcome = wxEmptyString, bool parked = false)
    		: m_Console(parent, wxID_ANY, consoleName, FromDIP(consolePosition), FromDIP(consoleSize),
    		wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX|wxMINIMIZE|wxSYSTEM_MENU)
        {
        void JSlexit(wxStyledTextCtrl* pane);
        Console *pConsole;
        wxPoint checkPointOnScreen(wxPoint point);
        wxPoint screenToFrame(wxPoint);
        TRACE(3, "Constructing console " + consoleName);
   
        // hook this new console onto end of chain of consoles
        pConsole = pJavaScript_pi->mpFirstConsole;
        if (pConsole == nullptr) pJavaScript_pi->mpFirstConsole = this;   // is first and only
        else {
            while (pConsole->mpNextConsole) pConsole = pConsole->mpNextConsole; // point to last
            pConsole->mpNextConsole = this; // add us
            }
        mConsoleName = consoleName;
        consolePosition = ToDIP(checkPointOnScreen(FromDIP(consolePosition))); // check position on screen
        m_parked = parked;
        if (parked) m_parkedPosition = consolePosition;	// if parked, restore position relative to frame
        consoleInit();        
        Move(FromDIP(consolePosition));
        mDialog.position = ToDIP(checkPointOnScreen(FromDIP(dialogPosition)));
        mDialog.pdialog = nullptr;
        mAlert.position = ToDIP(checkPointOnScreen(FromDIP(alertPosition)));
        mAlert.palert = nullptr;
        m_fileStringBox->SetValue(fileString);
        auto_run->SetValue(autoRun);
        mBrief.hasBrief = false;
        mBrief.theBrief = wxEmptyString;
        mBrief.reply = false;
        mBrief.briefingConsoleName = wxEmptyString;

        // output pane set up
        m_Output->StyleSetSpec(STYLE_RED, _("fore:#FF0000"));
        m_Output->StyleSetSpec(STYLE_BLUE, _("fore:#2020FF"));
        m_Output->StyleSetSpec(STYLE_ORANGE, _("fore:#FF7F00"));
        m_Output->StyleSetSpec(STYLE_GREEN, _("fore:#00CE00"));
        m_Output->StyleSetSpec(STYLE_BOLD, _("bold"));
        m_Output->StyleSetSpec(STYLE_UNDERLINE, _("underline"));
        // wrap text in output pane
        m_Output->SetWrapMode(wxSTC_WRAP_WORD);
        m_Output->SetWrapIndentMode(wxSTC_WRAPINDENT_INDENT);
        m_Output->SetWrapVisualFlags(wxSTC_WRAPVISUALFLAG_START);
        
        // script pane set up
        JSlexit(this->m_Script);  // set up lexing
        // wrap text in script pane
        m_Script->SetWrapMode(wxSTC_WRAP_WORD);
        m_Script->SetWrapIndentMode(wxSTC_WRAPINDENT_INDENT);
        m_Script->SetWrapVisualFlags(wxSTC_WRAPVISUALFLAG_START);
        

        auto_run->Hide();
        if (m_fileStringBox->GetValue() != wxEmptyString) {    // we have a script to load
            wxString    script;
            wxString    fileString = this->m_fileStringBox->GetValue();
            if (!wxFileExists(fileString)){
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
        			<<  patchString  << " " << PLUGIN_VERSION_DATE << " " << PLUGIN_VERSION_COMMENT << " JS v" << DUK_VERSION << ("\\n\");\n\"All OK\";");
            m_Script->AddText(welcome); // some initial script
            }
        m_Output->AppendText(welcome);
        this->SetSize(consoleSize);
        TRACE(4, "Constructed console " + consoleName + wxString::Format(" size x %d y %d  minSize x %d y %d", consoleSize.x, consoleSize.y, this->GetMinSize().x, this->GetMinSize().y ));        
        }
    
    Console *clearAndUnhook(){  //  Clear down and unhook console prior to deleting
        Console *pConsole, *pPrevConsole;
        TRACE(3,"Unhooking console " + this->mConsoleName);

        // unhook ourself from the chained list of consoles
        if (!pJavaScript_pi->mpFirstConsole)
            message(STYLE_RED, "Logic error ClearAndUnhook called with no first console");   // should never be
        if (this == pJavaScript_pi->mpFirstConsole){    // we are the first on the chain
            pJavaScript_pi->mpFirstConsole = this->mpNextConsole;   // unhook ourself
            }
        else {  // we will start at the second on the chain
            pPrevConsole = pJavaScript_pi->mpFirstConsole;
            for (pConsole = pPrevConsole->mpNextConsole; pConsole != nullptr;
                        pPrevConsole = pConsole, pConsole = pConsole->mpNextConsole){
                if (this == pConsole){
                    // this is us
                    pPrevConsole->mpNextConsole = this->mpNextConsole;
                    break;
                }
            }
        }
        this->mpNextConsole = nullptr;
        TRACE(3,"Done unhooking console " + this->mConsoleName);
        return this;
    }
    
	void clearBrief(){  // initialise brief by clearing it down
		mBrief.hasBrief = mBrief.fresh = mBrief.reply = false;
		mConsoleRepliesAwaited = 0;
		}
	
 
	 void consoleInit(){	// Initialises console, clearing out settings
		mpCtx = nullptr;
		mRunningMain = false;
		mStatus = 0;
		mWaitingCached = false;
		m_explicitResult = false;
		m_result = wxEmptyString;
		m_hadError = false;
		m_time_to_allocate = 1000;   //default time allocation (msecs)
		mTimerActionBusy = false;
		mDialog.pdialog = nullptr;
		mDialog.functionName = wxEmptyString;
		mAlert.palert = nullptr;
		mAlert.alertText = wxEmptyString;
		mpMessageDialog = nullptr;
		mConsoleRepliesAwaited = 0;
		m_exitFunction = wxEmptyString;
		#ifdef SOCKETS
			clearSockets();
		#endif
		mscriptToBeRun = false;
		run_button->SetLabel("Run");
		if (mAlert.position.y == -1){ // shift initial position of alert  away from default used by dialogue
			mAlert.position.y = 150;
			}
		return;
		}
 
  Completions run(wxString script) { // compile and run the script
	   Completions outcome;       // result of JS run
	   extern bool runLable, stopLabel;
	   wxString result;
	   void fatal_error_handler(void *udata, const char *msg);
	   void duk_extensions_init(duk_context *ctx);
	   void ocpn_apis_init(duk_context *ctx);
	   void ConsoleHandle_init(duk_context *ctx);
   
	   TRACE(3, this->mConsoleName + "->run() entered");
	   if (script == wxEmptyString) return HAD_ERROR;  // ignore
	   consoleInit();
	   mpCtx = duk_create_heap(NULL, NULL, NULL, NULL, fatal_error_handler);  // create the Duktape context
	   if (!mpCtx) {
		message(STYLE_RED, _("Plugin logic error: Duktape failed to create heap"));
		  return HAD_ERROR;
		  }
	   duk_extensions_init(mpCtx);  // register our own extensions
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
            Completions outcome;
            mBrief = brief;
            outcome = run(m_Script->GetText());
       		if (!isBusy()) wrapUp(outcome);
            }
        else { // Stop button clicked - we have a script running - kill it off
            TRACE(4, mConsoleName + " script stopped");
            m_explicitResult = true;
            m_result = _("script was stopped");
            mStatus = 0;
            wrapUp(STOPPED);
            }
    }
    
    Completions executeFunction(wxString function){
        // execute function wih single argument already on duktape stack
        // returns result with nothing on stack
        duk_int_t duk_outcome;
        duk_context *ctx = mpCtx;
       
        duk_outcome = duk_get_global_string(ctx, function.c_str());
        if (!duk_outcome){ // failed to find the function
            TRACE(14, "executeFunction failed to find function");
            m_result = _("Logic error: ") + mConsoleName + _("->executeFunction - no function ") + function;
            duk_pop(ctx);
            return HAD_ERROR;
            }
        duk_swap_top(ctx, -2); // the argument needs to be above the function
        Console *findConsoleByCtx(duk_context *ctx); // ******
        TRACE(8, "Before call " + findConsoleByCtx(ctx)->dukDump());
        startTimeout();
        duk_outcome = duk_pcall(ctx, 1);   // single argument
        clearTimeout();
        TRACE(8, "After call " + findConsoleByCtx(ctx)->dukDump() + _("Result is ") + ((duk_outcome == DUK_EXEC_SUCCESS)?"success":"error"));
		return(afterwards(duk_outcome));
        }
    
    void doExecuteFunction(Console* pFromConsole, ConsoleReplyResult result){
        // this is implemented as a method so we can lazy-call a fuction with CallAfter
        wxString functionName;
        TRACE(4, mConsoleName + "->doExecuteFunction " + result.functionName + " with " + result.result + " reason " +
        	((result.resultType == HAD_ERROR)?"HAD_ERROR":((result.resultType == MORETODO)?"MORETODO":((result.resultType == STOPPED)?"STOPPED":((result.resultType == DONE)?"DONE":"unknown")))));
        mConsoleRepliesAwaited--;
        // ready to invoke function and give it an object we will construct here
        functionName = result.functionName;
        duk_push_object(mpCtx);
        duk_push_int(mpCtx, result.resultType);
        duk_put_prop_literal(mpCtx, -2, "type");
        duk_push_string(mpCtx , result.result.c_str());
        duk_put_prop_literal(mpCtx, -2, "value");
        Completions outcome = executeFunction(functionName);
        if (!isBusy()) wrapUp(outcome);
        }
    
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
    
    void setWaiting(){
        // mark that we have set something that waits on a response
        TRACE(4,mConsoleName + "->setWaiting()");
        mWaitingCached = mWaiting = true;
        }
    
    bool isWaiting() { // returns true if waiting for anything
        // for efficiencey, we may have a cached result in mWaiting
        if (mWaitingCached) {
            TRACE(15,mConsoleName + "->isWaiting() returning cached result " + (mWaiting?"true":"false"));
            return mWaiting;
            }
        TRACE(4,mConsoleName + "->isWaiting() doing full check");
        bool result = false;
        int count;
        if (
            (mTimes.GetCount() > 0) ||
            (mMenus.GetCount() > 0) ||
#ifdef SOCKETS
            (mSockets.GetCount() > 0) ||
#endif
            (m_NMEAmessageFunction != wxEmptyString) ||
            (m_activeLegFunction != wxEmptyString) ||
            (mDialog.pdialog != nullptr) ||
            (mpMessageDialog != nullptr) ||
            (mAlert.palert != nullptr) ){
            	TRACE(5, mConsoleName + "->isWaiting() 1st if true");
                result = true;
                }
        else if ((count = (int)mMessages.GetCount()) > 0){ // look at messages
             for(unsigned int index = 0; index < count; index++){
                if (mMessages[index].functionName != wxEmptyString) {
                    TRACE(5, mConsoleName + "->isWaiting() else if true");
                    result = true;
                    break;
                    }
                }
            }
        if (mConsoleRepliesAwaited > 0) result = true;
        mWaiting = result;
        mWaitingCached = true;
        TRACE(4,mConsoleName + "->isWaiting() returning new result " + (mWaiting?"true":"false"));
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
		wxString getStringFromDuk(duk_context *ctx);
		Completions outcome;
        wxString statusesToString(status_t mStatus);
		
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
		if (dukOutcome != DUK_EXEC_SUCCESS) return HAD_ERROR;	// a real error
		
		mWaitingCached = false;   // now we are past those cases, get a re-check
	   result = getStringFromDuk(mpCtx);
	   if (!m_explicitResult) m_result = result; // for when not explicit result
	   duk_pop(mpCtx);  // pop result
	   outcome = DONE; // starting assumption
   
	   if (isWaiting()){ // if script set up call-backs with nominated functions, check they exist
		  outcome = MORETODO;
		  TRACE(4, mConsoleName + "->afterwards()  completed with something to wait for");
		  duk_push_global_object(mpCtx);  // get our compiled script
		  if (
			 functionMissing(m_NMEAmessageFunction) ||
			 functionMissing(mDialog.functionName) ||
			 functionMissing(m_activeLegFunction) )
			 outcome = HAD_ERROR;;
		  if (!mTimes.IsEmpty()){
			 // has set up one or more timers - check them out
			 int count = (int)mTimes.GetCount();
			 for (int i = 0; i < count; i++)
				if (functionMissing(mTimes[i].functionName)) outcome = HAD_ERROR;
			 }
		  if (!mMenus.IsEmpty()){
			 // has set up one or more context menus - check them out
			 int count = (int)mMenus.GetCount();
			 for (int i = 0; i < count; i++)
				if (functionMissing(mMenus[i].functionName)) outcome = HAD_ERROR;
			 }
		  duk_pop(mpCtx);   // the global object
			}
		return(outcome);
		}
    
	void wrapUp(Completions reason) {    // clean down and maybe destroy ctx
		wrapUpWorks(reason);
		if (!isBusy()){
			if (mpCtx != nullptr) { // for safety - nasty consequences if no context
				TRACE(4, mConsoleName + "->wrapUp() destroying ctx");
				duk_destroy_heap(mpCtx);
				mpCtx = nullptr;
				}
			}
		}

    void wrapUpWorks(Completions reason) {    // consider clearing down and destroying context etc.
    	// this is guts of wrapUp as sub-function to allow multiple returns to wrapUp
        wxString statusesToString(status_t mStatus);
        wxString formErrorMessage(duk_context *ctx);
        
        mStatus.set(reason);
        TRACE(4, wxString::Format("%s->wrapUpWorks() with status %d and now %s\n%s",mConsoleName, reason, statusesToString(mStatus), dukDump()));
#if TRACE_YES
        {wxString dump {""};
            if (mBrief.hasBrief || mBrief.reply){
                dump += "mBrief\n";
                dump += "\tfresh:\t\t\t" + (mBrief.fresh?_("true"):_("false")) + "\n";
                dump += "\thasBrief:\t\t\t" + (mBrief.hasBrief?_("true"):_("false")) + "\n";
                dump += "\ttheBrief:\t\t\t" + mBrief.theBrief + "\n";
                dump += "\treply:\t\t\t" + (mBrief.reply?_("true"):_("false")) + "\n";
                dump += "\tconsole:\t\t\t" + mBrief.briefingConsoleName + "\n";
                dump += "\tfunction:\t\t\t" + mBrief.function + "\n";
            }
            else dump += "No brief\n";
            TRACE(4, dump);
        }
#endif
        if (reason == TIMEOUT){
            message(STYLE_RED, m_result = "Script timed out");
            clearCallbacks();
            replyToConsole(reason);
            return;
			}
		if (reason == CLOSING){	// this differes from a real error in that no error message is generated
			clearCallbacks();
			reason = CLOSED;	// we now treat it further just like a simple Close
            }
        if (reason == HAD_ERROR){   // output error message
            TRACE(4, "wrapUpWorks with error stack:" + dukDump());
            message(STYLE_RED,  m_result = formErrorMessage(mpCtx));
            clearCallbacks();
            replyToConsole(reason);
            return;
            }
        if (reason == MORETODO){
        	TRACE(4, "wrapUpWorks says MORETODO");
        	return;
        	}
        if (reason & (STOPPED | TOCHAIN | DONE)) { // exit function if any of these
            if (m_exitFunction != wxEmptyString){    // we have an onExit function to run
				TRACE(2, "wrapUpWorks has exit function about to be run: " + m_exitFunction);
				mStatus.set(INEXIT);
				duk_push_string(mpCtx, ""); // next bit expects an argument - put anything there
				Completions onExitCompletion = executeFunction(m_exitFunction);
				mStatus.reset(INEXIT);
				TRACE(4, "wrapUpWorks back from exit function");
				if (onExitCompletion == TIMEOUT){
					message(STYLE_RED, m_result = "onExit function timed out");
					clearCallbacks();
					replyToConsole(onExitCompletion);
					return;
					}	
				if (onExitCompletion == HAD_ERROR){ // error during exit function
					TRACE(4, "wrapUpWorks with exit function error stack:" + dukDump());
					message(STYLE_RED,  m_result = "onExit function " + formErrorMessage(mpCtx));
					clearCallbacks();
					replyToConsole(onExitCompletion);
					return;
					}
				}
			}
		clearCallbacks();
		if (isBusy())	message(STYLE_RED, "wrapUpWorks logic error - isBusy() true");
		// now for the result
		if (!this->m_explicitResult && this->m_result == wxEmptyString)this->m_result = "undefined";
		if (!this->m_explicitResult || this->m_result != wxEmptyString)
		this->message(STYLE_BLUE, _("result: ") + this->m_result);
        
        if (!mBrief.fresh) mBrief.hasBrief = false; // was only available once

        if (reason == TOCHAIN){  // a new script will have been loaded to chain to and any brief set
            TRACE(4, wxString::Format("%s->wrapUpWorks() chaining",mConsoleName));
            mStatus.reset(TOCHAIN);
            CallAfter(&Console::doRunCommand, mBrief);
            }
     	else {
     		replyToConsole(reason);	// only replies if so set up
			}
        if (reason != CLOSED) Show();   // make console visiibleunless was binned
		}
        
    void replyToConsole(Completions reason){ // reply according to brief
        if (!mBrief.reply) return; // no reply to do
        TRACE(4, wxString::Format("%s->reply() replying to console %s",mConsoleName, mBrief.briefingConsoleName));
        Console* findConsoleByName(wxString name);
        Console* pConsole = findConsoleByName(mBrief.briefingConsoleName);
        ConsoleReplyResult resultStruct;
        
        mBrief.reply = false;    // we only do this once
        if (!pConsole) {
            // no console to report error - use first
            wxMessageBox("Console "+ mBrief.briefingConsoleName + " not found to call back");
            }
        else if (!pConsole->isWaiting()) this->message(STYLE_RED, "Console to call back not waiting for us");
        else {
            resultStruct.result = m_result;
            resultStruct.resultType = reason;
            resultStruct.functionName = mBrief.function;
            TRACE(4, wxString::Format("Replying to console %s function %s with result %s & reason %d", mBrief.briefingConsoleName, resultStruct.functionName, resultStruct.result, resultStruct.resultType));
            pConsole->CallAfter(&Console::doExecuteFunction , this, resultStruct);
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
    
    void clearDialog(){ // clear away any open dialogue
        TRACE(4,this->mConsoleName + "->clearDialog()  " + ((this->mDialog.pdialog == nullptr)?"(None)":"Dialog exists"));
        if (mDialog.pdialog != nullptr){
            mDialog.position = ToDIP(mDialog.pdialog->GetPosition());   // remember where it is in DIP
            mDialog.dialogElementsArray.clear();
            mDialog.pdialog->Close();
            mDialog.pdialog->Destroy();
            delete mDialog.pdialog;
            mDialog.pdialog = nullptr;
            mDialog.functionName = wxEmptyString;
            mWaitingCached = false;
            }
        }
    
    void clearMenus()           {   // clear away any menus for this console
        int count = mMenus.GetCount();
        for (int i = count-1; i >= 0; i--){   // do this backwards
            delete mMenus[i].pmenuItem;
            RemoveCanvasContextMenuItem(mMenus[i].menuID);
            mMenus.RemoveAt(i);
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
    
	void clearCallbacks(){// clears all reasons a console might be called back
		mTimes.Clear();	//timers
		clearDialog();
		clearAlert();
		clearMenus();
		m_NMEAmessageFunction = wxEmptyString;  // function to invoke on receipt of NMEA message, else ""
    	m_activeLegFunction = wxEmptyString;  // function to invoke on receipt of active leg, else ""
		m_exitFunction = wxEmptyString;  // function to call on exit, else ""
		size_t messageCount = mMessages.GetCount();
		if (messageCount > 0){
			for(unsigned int index = 0; index < messageCount; index++){
				mMessages[index].functionName = wxEmptyString;
				}
            }
        mConsoleRepliesAwaited = 0;
        mTimerActionBusy = false;
        mRunningMain = false;
        if (mpMessageDialog != nullptr){
        	TRACE(4,mConsoleName + "->clearCallbacks() found mpMessageDialog - clearing");
        	mpMessageDialog->Destroy();
        	mpMessageDialog = nullptr;
        	}
#ifdef SOCKETS
		clearSockets();wrapUp
#endif
		mWaitingCached = false;
		run_button->SetLabel(_("Run"));
		}
		
    void throw_error(duk_context *ctx, wxString message){
        // throw an error from within c++ code called from running script
        // either there is an error object on the stack or a message
        // ! do not call otherwise
        TRACE(4, mConsoleName + "->throw_error() " + message);        m_result = wxEmptyString;    // supress result
        m_explicitResult = true;    // supress result
        if (!duk_is_error(ctx, -1)){
            // we do not have an error object on the stack
            duk_push_error_object(ctx, DUK_ERR_ERROR, _("Console ") + mConsoleName + _(" - ") + message);  // convert message to error object
            }
        m_hadError = true;
        duk_throw(ctx);
        mRunningMain = false;
        }
    
    void message(int style, wxString message){
        void limitOutput(wxStyledTextCtrl* pText);
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
    	wxSize consoleSize = ToDIP(this->GetSize());	// using DIP size here
        if ((consoleSize.x < 200)|| (consoleSize.y < 400)){
        	consoleSize = FromDIP(wxSize(680,700));		// back to actual size
        	this->SetSize(consoleSize);
        	}
    	}
    
    void display_error(duk_context *ctx, wxString message){
        // display an error NOT from running script
        // either there is an error object on the stack or a message
        m_explicitResult = true;    // supress result
        m_result = duk_safe_to_string(ctx, -1);
        m_hadError = true;
        duk_pop(ctx);
        this->message(STYLE_RED, _("From display_error ") +  m_result);
        }

    int OCPNmessageIndex(messageNameString_t thisMessage){
        // Returns index number for given sentence
        // If unknown sentence, adds it to array, sorts it into place and returns its index
        MessagePair newPair;
        int messageComp(MessagePair** arg1, MessagePair** arg2);
        
        size_t messageCount = mMessages.GetCount();
        unsigned int index = 0;
        if (messageCount == 0){
            // it's empty
            newPair.messageName = thisMessage;
            newPair.functionName = wxEmptyString;
            mMessages.Add(newPair);
            return(0);  // this is first and only entry, so index is 0
            }
        else { // have existing entries - search for match in sorted list
            for (index = 0; index < messageCount; index++)
                {
                    int cmp = strcmp(thisMessage, mMessages[index].messageName);
                    if (cmp > 0) continue;  // look further
                    else if (cmp == 0) return(index);
                    else break;             // no need to look further as list is sorted
                };
            }
        if (index == messageCount){  // no match
            if (index >= MAX_SENTENCE_TYPES){
                // should not get here unless something wrong
                wxMessageBox(_("Number of message types exceeding safety limit"), _("JavaScript plugin - lookupfunction"));
                return(-1);
                }
        }
        // did not find a match, so add a new entry
        newPair.messageName = thisMessage;
        newPair.functionName = wxEmptyString;
        mMessages.Add(newPair);
        mMessages.Sort(messageComp);  // This likely to move new entry so look up index again
        messageCount = mMessages.GetCount();  // update
        for (index = 0; index < messageCount; index++) // NB condition == here to allow for new entry
            {
            if ((mMessages[index].messageName == thisMessage)) return(index);
            }
        // should never get here
        message(STYLE_RED, _("Failed to add new sentence - programming error") + ("JavaScript plugin - messageIndex fuction"));
        return(-1);
        }
            
    wxString getMessagesString()        // returns string containing list of all messages
    {
        wxString result = wxEmptyString;
        wxString functionPart;
        
        size_t count = this->mMessages.GetCount();
        if (count > 0){
            for (unsigned int i = 0; i < count; i++){
                result += mMessages[i].messageName;
                functionPart = mMessages[i].functionName;
                if (functionPart != wxEmptyString)
                    result += _(":") + functionPart;
                result += _("\n");
            }
        }
        else result = _("<nil>\n");
        return (result);
    }

    void setTimedCallback(jsFunctionNameString_t functionName, wxString argument, wxDateTime timeToCall){
        // create a callback entry to call fuction with argument at timeToCall
        size_t timersCount = this->mTimes.GetCount();
        if (timersCount >= MAX_TIMERS){
            // should not get here unless something wrong
            wxMessageBox(_("Number of timers exceeding safety limit"), _("JavaScript plugin - setTimedCallback"));
            return;
            }
        TimeActions newAction;
        newAction.functionName = functionName;
        newAction.argument = argument;
        newAction.timeToCall = timeToCall;
        mTimes.Add(newAction);
        }
        
    void setConsoleMinSize(){
    	// all in physical size here
    	double scale = this->GetDPIScaleFactor();
    	wxSize minSize, size;
    	
    	wxSize textSize = this->GetTextExtent(mConsoleName);
    	minSize.x = pJavaScript_pi->m_parkingStub*scale + textSize.x + 3*scale;	// added 3 to be on safe side
    	minSize.y = pJavaScript_pi->m_parkingMinHeight*scale;
    	TRACE(4, wxString::Format("setConsoleMinSize text %s, size X:%i Y: %i", mConsoleName, textSize.x, textSize.y));  	
    	SetMinSize(minSize);
    	// make sure is no smaller than new min size
    	size = GetSize();
    	if (size.x < minSize.x) size.x = minSize.x;
    	if (size.y < minSize.y) size.y = minSize.y;
    	SetSize(size); 	
    	}
    	
    void park(){	// park this console
    	wxPoint screenToFrame(wxPoint pos);
    	wxPoint frameToScreen(wxPoint pos);

        int rightMost {0};    // most right hand edge of a parked console relative to frame
        bool foundParked {false};
        TRACE(25, wxString::Format("%s->park() parking called with minSize X:%i  Y:%i",
        	mConsoleName, GetMinSize().x, GetMinSize().y)); 
		wxSize size = GetMinSize();
    	SetSize(size);	
    	if (isParked()) return;	// was already parked
    	// find horizontal place available avoiding other parked consoles
    	for (Console* pC = pJavaScript_pi->mpFirstConsole; pC != nullptr; pC = pC->mpNextConsole){
    		// for each console
    		int rhe;	// right hand edge within frame
            if (pC == this) continue;	// omit ourselves console from check
            if (!pC->isParked()) continue;	// ignore non-parked consoles
            // this one is parked
            foundParked = true;
            wxPoint pCFramePos = screenToFrame(pC->GetPosition());	// position of this console in frame
            rhe = pCFramePos.x + pC->GetSize().x;
            if (rhe > rightMost) rightMost = rhe;
            }   
    	int newX = foundParked ? (rightMost + pJavaScript_pi->m_parkSep): pJavaScript_pi->m_parkFirstX;	// horizontal place for new parking in actual px
		m_parkedPosition = wxPoint(newX, pJavaScript_pi->m_parkingLevel);	// temp in actual px
		Move(frameToScreen(m_parkedPosition));
        TRACE(25, wxString::Format("%s->park() parking at X:%i  Y:%i frame", mConsoleName, m_parkedPosition.x, m_parkedPosition.y));
		m_parkedPosition = ToDIP(m_parkedPosition);	// save in DIP
        m_parked = true;
    	}
    	
    bool isParked(){ //	returns true if console is parked
        wxPoint screenToFrame(wxPoint pos);
        
    	if (!m_parked) return false;
    	wxPoint posNow = screenToFrame(GetPosition());	// pos rel to frame
    	posNow = ToDIP(posNow);	// stored position in DIP
    	if ((abs(posNow.y - m_parkedPosition.y) < 4)) return true; // still parked (allowing small margin for error )
    	m_parked = false;	// has been moved
    	return false;
    	}
    
    void destroyConsole(){
        Destroy();
    };
    
    void bin(){  // move console to bin
        Console *pConsole, *pFreedConsole;
        TRACE(4, "bin() " + this->mConsoleName + " being binned");
        pFreedConsole = this->clearAndUnhook();
        pConsole = pJavaScript_pi->mpBin;
        if (pConsole == nullptr){	// bin was empty
        	pJavaScript_pi->mpBin = pFreedConsole;
        	pFreedConsole->mpNextConsole = nullptr;
        	}
        else {  // there is one already in the bin - push freed one onto the front
            pFreedConsole->mpNextConsole = pJavaScript_pi->mpBin;
            pJavaScript_pi->mpBin = pFreedConsole;
            }
        pFreedConsole->Hide();
    }
    
    wxString consoleDump(){    // returns string being dump of selected information from console structure
        wxString ptrToString(Console* address);
        wxString statusesToString(status_t mStatus);
        wxPoint screenToFrame(wxPoint);
        int i, count;
        wxString dump {""};
        dump += "Console name:\t\t" + mConsoleName + "\n";
        dump += "this:\t\t\t\t\t" +  ptrToString((Console *)this) + "\n";
        dump += "mpNextConsole:\t\t" + ptrToString((Console *)mpNextConsole) + "\n";
        dump += "mStatus:\t\t\t\t" + statusesToString(mStatus) + "\n";
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
        wxPoint framePosition = screenToFrame(screenPosition);
        dump += wxString::Format("position:\t\t\t\tscreen x:%i y:%i\tframe x:%i y:%i\n", screenPosition.x, screenPosition.y, framePosition.x, framePosition.y );
        wxSize size = GetSize();
        wxSize minSize = GetMinSize();
        dump += wxString::Format("Size():\t\t\t\tx:%i y:%i\tMinSize() x:%i y:%i\n", size.x, size.y, minSize.x, minSize.y );
        dump += wxString::Format("m_parked:\t\t\t%s position x:%i y:%i\n", (m_parked ? _("true"):_("false")), m_parkedPosition.x, m_parkedPosition.y);
        dump += "isParked():\t\t\t" + (isParked() ?  _("true"):_("false")) + "\n";       
        dump += _("Messages callback table\n");
        count =(int)this->mMessages.GetCount();
        if (count == 0) dump += _("\t(empty)\n");
        else {
            for (i = 0; i < count; i++) dump += _("\t") + this->mMessages[i].messageName +_("\t\t") +
            this->mMessages[i].functionName + _("\n");
            }
        dump += "m_timerActionBusy:\t" + (this->mTimerActionBusy?_("true"):_("false")) + "\n";
        dump += _("Timers callback table\n");
        count = (int)this->mTimes.GetCount();
        if (count == 0) dump += _("\t(empty)\n");
        else {
            for (i = 0; i < count; i++){
                dump += _("\t") + mTimes[i].timeToCall.FormatTime() + "\t" + this->mTimes[i].functionName +_("\t") + this->mTimes[i].argument + _("\n");
                }
            }
        dump += _("Menus callback table\n");
        count = (int)this->mMenus.GetCount();
        if (count == 0) dump += _("\t(empty)\n");
        else {
            for (i = 0; i < count; i++){
                dump += _("\t") + this->mMenus[i].menuName +_("\t") + this->mMenus[i].functionName +_("\t") + this->mMenus[i].argument + _("\n");
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
        dump += "m_dialog:\t\t\t" + ((this->mDialog.pdialog == nullptr)?_("None"):wxString::Format("Active with %d elements",  this->mDialog.dialogElementsArray.size()) ) + "\n";
        dump += "m_alert:\t\t\t\t" + ((this->mAlert.palert == nullptr)?_("None"):_("Active")) + "\n";
        dump += "m_NMEAmessageFunction:\t\t" + m_NMEAmessageFunction + "\n";
        dump += "m_activeLegFunction:\t\t" + m_activeLegFunction + "\n";
        dump += "m_exitFunction:\t" + m_exitFunction + "\n";
        dump += "m_explicitResult:\t\t" + (m_explicitResult?_("true"):_("false")) + "\n";
        dump += "m_result:\t\t\t\t" + m_result + "\n";
        dump += "mscriptToBeRun:\t" + (this->mscriptToBeRun?_("true"):_("false")) + "\n";
        if (mBrief.hasBrief || mBrief.reply){
            dump += "mBrief\n";
            dump += "\tfresh:\t\t\t" + (mBrief.fresh?_("true"):_("false")) + "\n";
            dump += "\thasBrief:\t\t\t" + (mBrief.hasBrief?_("true"):_("false")) + "\n";
            dump += "\ttheBrief:\t\t\t" + mBrief.theBrief + "\n";
            dump += "\treply:\t\t\t" + (mBrief.reply?_("true"):_("false")) + "\n";
            dump += "\tconsole:\t\t\t" + mBrief.briefingConsoleName + "\n";
            dump += "\tfunction:\t\t\t" + mBrief.function + "\n";
            }
        else dump += "No brief\n";
        dump += wxString::Format("mConsoleRepliesAwaited\t%d\n", mConsoleRepliesAwaited);
        dump+= dukDump();
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
    
private:
    bool functionMissing(wxString function){ // checks if function missing in global object
        // global object must be on stack when this is called
        if (function == wxEmptyString) return false;
        if (!duk_get_prop_string(mpCtx, -1, function.c_str())){  // Does function exist in there?
            duk_pop(mpCtx);   // pop off function or undefined
            m_result = "call-back function " + function + " not provided";
//                display_error(mpCtx, "call-back function " + function + " not provided");
            return true;
            }
        duk_pop(mpCtx);   // pop off function or undefined
        return false;
        }
    };

class jsButton : public wxButton {
    // adds pointer to console to button so we can find it from button event
public:
    Console *pConsole;   // pointer to the console associated with this button
    jsButton (Console *buttonConsole, wxWindow *parent, wxWindowID id, const wxString &label = wxEmptyString, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = 0, const wxValidator &validator = wxDefaultValidator,const wxString &name = wxButtonNameStr) :
            wxButton(parent, id, label, pos, size, style, validator, name ),
            pConsole(buttonConsole)
    {};
    
};

#endif  // JavaScript_impl_h
