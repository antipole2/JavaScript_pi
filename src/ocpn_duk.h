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
#ifndef OCPN_DUK_H
#define OCPN_DUK_H
#include "JavaScript_pi.h"
#include "JavaScriptgui.h"
#include "JavaScriptgui_impl.h"
#include "wx/tokenzr.h"
#include "ocpn_plugin.h"
// #include "wx/dynarray.h"
#include "wx/arrimpl.cpp"
#include <vector>

// declare wxStyledText styles
enum {
    STYLE_BLACK,
    STYLE_RED,
    STYLE_BLUE,
    STYLE_ORANGE,
    STYLE_GREEN
    };

#define DUK_DUMP true
#if DUK_DUMP
#define MAYBE_DUK_DUMP duk_push_context_dump(ctx);JS_control.message(STYLE_ORANGE, _(""), _("Duktape context dump:\n")+duk_to_string(ctx, -1)+_("\n"));duk_pop(ctx);
#define ERROR_DUMP {JS_control.message(STYLE_ORANGE, _(""), _("error dump\n")+(duk_is_error(ctx, -1)?(_("Error object with " )+duk_safe_to_string(ctx, -1)+_("\n")):_("No error object\n")));}\
MAYBE_DUK_DUMP
#endif  //  DUK_DUMP

typedef wxString jsFunctionNameString_t;
typedef wxString messageNameString_t;

#define NUMBER_OF_MESSAGE_TYPES 1

enum CallBackTypes
{
    // The call back types supported in the API
    NMEA_SENTENCES,
    PLUGIN_MESSAGING
};

class MessagePair  // holds OPCN messages seen, together with JS callback function name, if to be called back
{
public:
    messageNameString_t messageName;
    jsFunctionNameString_t functionName;
};
WX_DECLARE_OBJARRAY(MessagePair, MessagesArray);
#define MAX_SENTENCE_TYPES 50    // safety limit in case of coding error

class TimeActions  // holds times at which function is to be called
{
public:
    wxDateTime timeToCall;
    jsFunctionNameString_t functionName;
    wxString argument;  // optional argument to pass to function
};
#define MAX_TIMERS 20   // safety limit of timers

WX_DECLARE_OBJARRAY(TimeActions, TimesArray);

// dialogues storage
struct dialogElement{
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
    wxPoint     position = wxPoint(wxDefaultPosition);
    std::vector<dialogElement> dialogElementsArray;// will be an array of dialogue elements
};

class AlertDetails // holds details of open dialogue
{
public:
    wxDialog    *palert;    // points to open alert else nullptr
    wxPoint     position = wxPoint(wxDefaultPosition);
    wxString    alertText;  // the currently displayed text
};


class JS_control_class
{
public:
    duk_context             *m_pctx = nullptr;      // Pointer to the Duktape context
    Console                 *m_pJSconsole;  // our own pointer to the console
    bool                    m_JSactive = false;  // safety switch - when false, stops plugin attempting to run call-backs
    bool                    m_runCompleted; // true when main run script has completed
    bool                    m_exitScriptCalled = false; // true if existScript() has been executed

    // callback management
    bool                    m_timerActionBusy;  // true while handling timer event to stop them piling up
    MessagesArray           m_messages;   // The messages call-back table
    TimesArray              m_times;       // Timers call-back table
    DialogAction            m_dialog;      // Dialogue call-back table
    AlertDetails            m_alert;        // details of alert dialog
    jsFunctionNameString_t  m_NMEAmessageFunction;  // function to invoke on receipt of NMEA message, else ""
    PlugIn_Position_Fix_Ex  m_positionFix;  // latest position fix - if none yet, time is NULL
    
    // timeout handling
    wxLongLong              m_pcall_endtime;        // time we must end execution (msecs since 1970)
    long                    m_time_to_allocate;     // the time we allocate (msec)
    long                    m_timeout_check_counter;    // counter of number of timeout checks
    bool                    m_backingOut;            // true while backing out following timeout
    
    // result handling
    bool                    m_explicitResult;    // true if using explicity result
    wxString                m_result;   // the result if explicitly created by call to result()
    


    
private:
    static int messageComp(MessagePair** arg1, MessagePair** arg2) {
        return (strcmp((*arg1)->messageName, (*arg2)->messageName));}
    
public:
    void init(duk_context *ctx) { // initialise
        m_pctx = ctx;
        m_JSactive = false;
        m_explicitResult = false;
        m_result = wxEmptyString;
        m_runCompleted = false;
        m_time_to_allocate = 1000;   //default time allocation (msecs)
        m_backingOut = false;
        m_exitScriptCalled = false;
        m_dialog.functionName = wxEmptyString;
        m_dialog.pdialog = nullptr;
        m_alert.palert = nullptr;
        m_alert.alertText = wxEmptyString;
        if (m_alert.position.y == -1){ // shift initial position of alert  away from fefault used by dialogue
            m_alert.position.y = 150;
            }
        return;
        }
    
    
    bool waiting()  // returns true if waiting for anything
    {
        if (m_times.GetCount() > 0) return true;    // timer active
        if (m_NMEAmessageFunction != wxEmptyString) return true;  // waiting on NMEA
        
        size_t messageCount = m_messages.GetCount();    // look at messages
        for(unsigned int index = 0; index < messageCount; index++){
            if (m_messages[index].functionName != wxEmptyString) return true;
            }
        if (m_dialog.pdialog != nullptr) return true;
        if (m_alert.palert != nullptr) return true;
        return false;
    }
    
    void clear()  // clear up and clear out any waiting callbacks
    {
        m_JSactive = false;
        wxString runLabel="Run";
        m_pJSconsole->Show();
        if (!this->m_backingOut){   // display result unless we are backing ot after time-out
            if (!this->m_explicitResult && this->m_result == wxEmptyString)this->m_result = "undefined";
            if (!this->m_explicitResult || this->m_result != wxEmptyString)
                this->message(STYLE_BLUE, _("result: "),this->m_result);
            }
        m_pJSconsole->setRunButton(runLabel);
        m_NMEAmessageFunction = wxEmptyString;
        size_t messageCount = m_messages.GetCount();
        if (messageCount > 0){
            for(unsigned int index = 0; index < messageCount; index++){
                m_messages[index].functionName = wxEmptyString;
                }
            }
        m_times.Clear();  // clear out the timers - does not seem to work
        m_timerActionBusy = false;
        clearDialog();
        clearAlert();
        return;
    }
    
    void clearAndDestroy() {    // clears down and destroys context etc.
        this->clear();
        if (m_pctx != nullptr) {
            duk_destroy_heap(m_pctx);
            m_pctx = nullptr;
            }
        return;
        }
    
    void clearAlert(){  // clears away any alert dialogue
        if (m_alert.palert != nullptr){ // only if have an alert displayed
            m_alert.position = m_alert.palert->GetPosition(); // remember alert position
            m_alert.palert->Close();
            m_alert.palert->Destroy();
            m_alert.palert = nullptr;
            m_alert.alertText = wxEmptyString;
            }
        }
    
    void clearDialog(){ // clear away any open dialogue
        if (m_dialog.pdialog != nullptr){
            m_dialog.position = m_dialog.pdialog->GetPosition();   // remember where it is
            m_dialog.dialogElementsArray.clear();
            m_dialog.pdialog->Close();
            m_dialog.pdialog->Destroy();
            m_dialog.pdialog = nullptr;
            m_dialog.functionName = wxEmptyString;
            }
        }
    
    void throw_error(duk_context *ctx, wxString message){
        // throw an error from within c++ code called from running script
        // either there is an error object on the stack or a message
        // ! do not call otherwise
        m_pJSconsole->Show(); // make sure console is visible
        m_result = wxEmptyString;    // supress result
        m_explicitResult = true;    // supress result
        if (!duk_is_error(ctx, -1)){
            // we do not have an error object on the stack
            duk_push_error_object(ctx, DUK_ERR_ERROR, message);  // convert message to error object
            }
        duk_throw(ctx);
        }
    
    void display_error(duk_context *ctx, wxString message){
        // display an error NOT from running script
        // either there is an error object on the stack or a message
        m_result = wxEmptyString;    // supress result
        m_explicitResult = true;    // supress result
        if (duk_is_error(ctx, -1)){
            // we have an error object on the stack
            message = duk_safe_to_string(ctx, -1);
            }
        this->message(STYLE_RED, _(""), message);
        }
    
    void message(int style, wxString messageAttribute, wxString message){
        m_pJSconsole->Show(); // make sure console is visible
        wxStyledTextCtrl* output_window = m_pJSconsole->m_Output;
        int long beforeLength = output_window->GetTextLength(); // where we are before adding text
        output_window->AppendText("JavaScript ");
        output_window->AppendText(messageAttribute);
        output_window->AppendText(message);
        output_window->AppendText("\n");
        int long afterLength = output_window->GetTextLength(); // where we are after adding text
        output_window->StartStyling((int)beforeLength,0);   // 2nd parameter included Linux still using wxWidgets v3.0.2
        output_window->SetStyling((int)(afterLength-beforeLength-1), style);
        }
    

    int messageIndex(messageNameString_t thisMessage)
    {   // Returns index number for given sentence
        // If unknown sentence, adds it to array, sorts it into place and returns its index
        MessagePair newPair;
        
        size_t messageCount = m_messages.GetCount();
        int index = 0;
        if (messageCount == 0){
            // it's empty
            newPair.messageName = thisMessage;
            newPair.functionName = wxEmptyString;
            m_messages.Add(newPair);
            return(0);  // this is first and only entry, so index is 0
            }
        else { // have existing entries - search for match in sorted list
            for (index = 0; index < messageCount; index++)
                {
                    int cmp = strcmp(thisMessage, m_messages[index].messageName);
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
        m_messages.Add(newPair);
        m_messages.Sort(messageComp);  // This likely to move new entry so look up index again
        messageCount = m_messages.GetCount();  // update
        for (index = 0; index < messageCount; index++) // NB condition == here to allow for new entry
            {
            if ((m_messages[index].messageName == thisMessage)) return(index);
            }
        // should never get here
        wxMessageBox(_("Failed to add new sentence - programming error"), _("JavaScript plugin - messageIndex fuction"));
        return(-1);
        }
            
    wxString getMessagesString()        // returns string containing list of all messages
    {
        wxString result = wxEmptyString;
        wxString functionPart;
        
        size_t count = this->m_messages.GetCount();
        if (count > 0){
            for (unsigned int i = 0; i < count; i++){
                result += m_messages[i].messageName;
                functionPart = m_messages[i].functionName;
                if (functionPart != wxEmptyString)
                    result += _(":") + functionPart;
                result += _("\n");
            }
        }
        else result = _("<nil>\n");
        return (result);
    }
    
    wxString getFunctionName() // extracts function name from call string
    {
        JS_control_class JS_control;
        
        wxStringTokenizer tokens( wxString(duk_to_string(m_pctx, 0)), _(" ("));
        if (tokens.GetNextToken() != "function") this->throw_error(m_pctx, "OCPNon.. error: must supply function name");
        return (tokens.GetNextToken());
        }

    void setTimedCallback(jsFunctionNameString_t functionName, wxString argument, wxDateTime timeToCall){
    // create a callback entry to call fuction with argument at timeToCall
    size_t timersCount = this->m_times.GetCount();
    if (timersCount >= MAX_TIMERS){
        // should not get here unless something wrong
        wxMessageBox(_("Number of timers exceeding safety limit"), _("JavaScript plugin - setTimedCallback"));
        return;
        }
    TimeActions newAction;
    newAction.functionName = functionName;
    newAction.argument = argument;
    newAction.timeToCall = timeToCall;
    m_times.Add(newAction);
    }
    
    wxString dump(){    // returns string being dump of selected information from JS_control structure
        int i, count;
        wxString dump = _("JS_control dump\n");
        dump += "m_runCompleted:\t" + (this->m_runCompleted?_("true"):_("false")) + "\n";
        dump += "m_backingOut:\t" + (this->m_backingOut?_("true"):_("false")) + "\n";
        dump += "m_exitScriptCalled:\t" + (this->m_exitScriptCalled?_("true"):_("false")) + "\n";
        dump += "m_JSactive:\t" + (this->m_JSactive?_("true"):_("false")) + "\n";
        dump += _("Messages callback table\n");
        count =(int)this->m_messages.GetCount();
        if (count == 0) dump += _("\t(empty)\n");
        else {
            for (i = 0; i < count; i++) dump += _("\t") + this->m_messages[i].messageName +_("\t") +
            this->m_messages[i].functionName + _("\n");
            }
        dump += "m_timerActionBusy:\t" + (this->m_timerActionBusy?_("true"):_("false")) + "\n";
        dump += _("Timers callback table\n");
        count = (int)this->m_times.GetCount();
        if (count == 0) dump += _("\t(empty)\n");
        else {
            for (i = 0; i < count; i++) dump += _("\t") + this->m_times[i].functionName +_("\t") + this->m_times[i].argument + _("\n");
            }
        dump += "m_dialog:\t" + ((this->m_dialog.pdialog == nullptr)?_("None"):wxString::Format(wxT("Active with %d elements"),  this->m_dialog.dialogElementsArray.size()) ) + "\n";
        dump += "m_alert:\t" + ((this->m_alert.palert == nullptr)?_("None"):_("Active")) + "\n";
        dump += "m_NMEAmessageFunction:\t" + this->m_NMEAmessageFunction + "\n";
        dump += "m_explicitResult:\t" + (this->m_explicitResult?_("true"):_("false")) + "\n";
        dump += "m_result:\t" + this->m_result + "\n";
        return dump;
    }
};


#endif /* OCPN_DUK_H */


    
