/**************************************************************************
* Project:  OpenCPN
* Purpose:  JavaScript Plugin
* Author:   Tony Voss 16/05/2020
*
* Copyright Ⓒ 2021 by Tony Voss
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, under which
* you must including a copy of these terms in all copies
* https://www.gnu.org/licenses/gpl-3.0.en.html
***************************************************************************
 * This module draws on  DR_pi by Mike Rossiter
 */

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include "JavaScript_pi.h"
#include "JavaScriptgui_impl.h"
#include "icons.h"
#include "trace.h"
#include "wx/tokenzr.h"
#include "wx/stdpaths.h"

int messageIndex(MessagesArray&, messageNameString_t);

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr)
{
    return new JavaScript_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p)
{
    delete p;
}

//---------------------------------------------------------------------------------------------------------
//
//    JavaScript PlugIn Implementation
//
//---------------------------------------------------------------------------------------------------------

#include "icons.h"

//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

JavaScript_pi *pJavaScript_pi;     // we will keep a pointer to ourself here

JavaScript_pi::JavaScript_pi(void *ppimgr)
:opencpn_plugin_117	 (ppimgr)  // was 16
{
#ifndef IN_HARNESS
    // Create the PlugIn icons
    initialize_images();

	wxFileName fn;
    auto path = GetPluginDataDir("javascript_pi");
    fn.SetPath(path);
    fn.AppendDir("data");
    fn.SetFullName("JavaScript_pi_panel_icon.png");

    path = fn.GetFullPath();
    wxString forDebug = fn.GetFullPath();
    wxInitAllImageHandlers();

    wxLogDebug(wxString("Using icon path: ") + path);
    if (!wxImage::CanRead(path)) {
        wxLogDebug("Initiating image handlers.");
        wxInitAllImageHandlers();
    }
    wxImage panelIcon(path);
    if (panelIcon.IsOk())
        m_panelBitmap = wxBitmap(panelIcon);
    else
        wxLogWarning("JavaScript panel icon has NOT been loaded");
    m_bShowJavaScript = false;
#endif
}

JavaScript_pi::~JavaScript_pi(void)
{
    // wxLogMessage("JavaScript_pi deconstructing");
    delete _img_JavaScript_pi;
    delete _img_JavaScript;
    
}

int JavaScript_pi::Init(void)
{
    ::wxDisplaySize(&m_display_width, &m_display_height);
    m_parent_window = GetOCPNCanvasWindow();
    pJavaScript_pi = this;  // Leave a way to find ourselves
    TRACE(1,"JavaScript_pi->Init() entered");
    AddLocaleCatalog( _T("opencpn-JavaScript_pi") );
    
    mpFirstConsole = nullptr;
    mpBin = nullptr;

    //    Get a pointer to the opencpn configuration object
    m_pconfig = GetOCPNConfigObject();
    
    //    And load the configuration items
    LoadConfig();
    mShowingConsoles = false;   // consoles will hence be shown on toolbar callback
    
    //    This PlugIn needs a toolbar icon, so request its insertion
    if(m_bJavaScriptShowIcon){

#ifndef IN_HARNESS
                
#ifdef JavaScript_USE_SVG
        m_leftclick_tool_id = InsertPlugInToolSVG(_T("JavaScript"), _svg_JavaScript, _svg_JavaScript, _svg_JavaScript_toggled,
            wxITEM_CHECK, "JavaScript", _T(""), NULL, CONSOLE_POSITION, 0, this);
#else
    m_leftclick_tool_id = InsertPlugInTool(_T(""), _img_JavaScript, _img_JavaScript, wxITEM_CHECK,
                                           "JavaScript",_T(""), NULL,
                                           CONSOLE_POSITION, 0, this);
#endif // JavaScript_USE_SVG
#endif // IN_HARNESS
    }
    mpPluginActive = true;
    mTimer.Bind(wxEVT_TIMER, &JavaScript_pi::OnTimer, this, mTimer.GetId());
    mTimer.Start(1000);

    TRACE(1,"JavaScript_pi->Init() returning");
    
    return (WANTS_TOOLBAR_CALLBACK |
            WANTS_CONFIG             |
            WANTS_PLUGIN_MESSAGING |
            WANTS_NMEA_SENTENCES |
            WANTS_NMEA_EVENTS |
            WANTS_AIS_SENTENCES |
            WANTS_PREFERENCES
            );
}

bool JavaScript_pi::DeInit(void) {
    // clean up and remember stuff for next time
    Console *pConsole;

    wxLogMessage("JavaScript_pi->DeInit() entered");
     mTimer.Stop();
     mTimer.Unbind(wxEVT_TIMER, &JavaScript_pi::OnTimer, this, mTimer.GetId());
    
    SaveConfig();

    if (pTools) {
        TRACE(3,"JavaScript plugin DeInit destroying tools pane");
        pTools->Hide();
        pTools->Close();
        pTools = nullptr;
        }

    while (mpFirstConsole) {    // close all remaining consoles
        TRACE(3,"JavaScript plugin DeInit closing console " + mpFirstConsole->mConsoleName);
        pConsole = mpFirstConsole;
        if (pConsole->m_exitFunction != wxEmptyString){
            // there is an onExit function to run
            duk_int_t outcome;
            TRACE(4, "Deinit running onExit function for console " + pConsole->mConsoleName);
            outcome = duk_get_global_string(pConsole->mpCtx, pConsole->m_exitFunction.c_str()); // make sure function exists
            if (outcome) duk_pcall(pConsole->mpCtx, 0); // only call it if it exists and no arguments
            // don't bother checking for errors - we will not be around to do anything with them
            }
        // purge stuff out of this one - we do not use wrapUp() because that might do all sorts of other things
        if (pConsole->mpCtx != nullptr) duk_destroy_heap(pConsole->mpCtx);
        pConsole->mMessages.Clear();
        pConsole->mTimes.Clear();
        pConsole->clearAlert();
        pConsole->clearDialog();
        mpFirstConsole = pConsole->mpNextConsole; // unhook first off chain
        
        }

    while (mpBin) {    // also any in the bin
        TRACE(3,"JavaScript plugin DeInit deleting console " + mpBin->mConsoleName + " from bin");
        pConsole = mpBin;
        mpBin = pConsole->mpNextConsole; // take first off chain        
        }
    mpPluginActive = false;
    SetToolbarItemState(m_leftclick_tool_id, mpPluginActive);
    RequestRefresh(m_parent_window); // refresh main window
    wxLogMessage("JavaScript completed deinit");
    TRACE(1,"JavaScript_pi->DeInit() returning");

    delete pConsole;
    pConsole = NULL;

    delete mpFirstConsole;
    mpFirstConsole = NULL;
    
    return true;
}

int JavaScript_pi::GetAPIVersionMajor()
{
    return atoi(API_VERSION);
}

int JavaScript_pi::GetAPIVersionMinor()
{
    std::string v(API_VERSION);
    size_t dotpos = v.find('.');
    return atoi(v.substr(dotpos + 1).c_str());
}

int JavaScript_pi::GetPlugInVersionMajor()
{
    return PLUGIN_VERSION_MAJOR;
}

int JavaScript_pi::GetPlugInVersionMinor()
{
    return PLUGIN_VERSION_MINOR;
}

wxBitmap *JavaScript_pi::GetPlugInBitmap()
{
    return &m_panelBitmap;
}

wxString JavaScript_pi::GetCommonName()
{
    return "JavaScript";
}


wxString JavaScript_pi::GetShortDescription()
{
    return "Providing JavaScript support";
}

wxString JavaScript_pi::GetLongDescription()
{
    return "Allows running of JavaScript\n\
             provided in a file";
}

int JavaScript_pi::GetToolbarToolCount(void)
{
    return 1;
}

void JavaScript_pi::SetColorScheme(PI_ColorScheme cs)
{
/*
 if (NULL == m_pConsole)
        return;
    
    DimeWindow(m_pConsole);
 */
}

void JavaScript_pi::OnToolbarToolCallback(int id)
{
    void JSlexit(wxStyledTextCtrl* pane);
    void fatal_error_handler(void *udata, const char *msg);
    TRACE(2,"JavaScript_pi->OnToolbarToolCallback() entered");
    
    mShowingConsoles = !mShowingConsoles;   // toggle console display state
    
    Console *m_pConsole = mpFirstConsole;
    for (m_pConsole = mpFirstConsole; m_pConsole != nullptr; m_pConsole = m_pConsole->mpNextConsole){
        if (mShowingConsoles)   m_pConsole->Show();
        else m_pConsole->Hide();
        if (m_pConsole->mWaitingToRun){ // we have a script to run
            wxString script = m_pConsole->m_Script->GetValue();
            if (script == wxEmptyString) continue;  // should not be but just in case
            m_pConsole->run(script);  // auto-run the script
            }
        }
    // Toggle is handled by the toolbar but we must keep plugin manager b_toggle updated
    // to actual status to ensure correct status upon toolbar rebuild
    SetToolbarItemState( m_leftclick_tool_id, mShowingConsoles );
    RequestRefresh(m_parent_window); // refresh main window
    }

bool JavaScript_pi::LoadConfig(void)
{
    void JSlexit(wxStyledTextCtrl* pane);
    wxFileConfig *pConf = (wxFileConfig *)m_pconfig;
#ifndef IN_HARNESS
    if(pConf)
    {
        pConf->SetPath ( _T( "/Settings/JavaScript_pi" ) );
        pConf->Read ( _T( "ShowJavaScriptIcon" ), &m_bJavaScriptShowIcon, 1 );
        int versionMajor = pConf->Read ( _T ( "VersionMajor" ), 20L );
        int versionMinor = pConf->Read ( _T ( "VersionMinor" ), 20L );
        if ((versionMajor == 0) && (versionMinor < 4)){
            TRACE(2, "Pre v0.4 - creating default consoles");
            // must be old version - forget previous settings
            pConf->DeleteGroup ( _T ( "/Settings/JavaScript_pi" ) );
            pConf->SetPath ( _T ( "/Settings/JavaScript_pi" ) );
            mCurrentDirectory = wxStandardPaths::Get().GetDocumentsDir();
            // create one default console
            mpFirstConsole = new Console(m_parent_window, "JavaScript");
            mpFirstConsole->Show();
            }
        else {
            TRACE(2, "Loading console configurations");
            mCurrentDirectory = pConf->Read(_T("CurrentDirectory"), _T("") );
            TRACE(2, "Current Directory set to " + mCurrentDirectory);
            // create consoles as in config file
            wxString consoles = pConf->Read ( _T ( "Consoles" ), _T("") );
            if (consoles == wxEmptyString){ // no consoles configued
                new Console(m_parent_window, "JavaScript");
                }
            else {
                wxStringTokenizer tkz(consoles, ":");
                mpFirstConsole = nullptr;
                while ( tkz.HasMoreTokens() ){
                    wxPoint consolePosition, dialogPosition, alertPosition;
                    wxString fileString;
                    bool autoRun;
                    
                    wxString name = tkz.GetNextToken();
                    consolePosition.x =  pConf->Read ( name + _T ( ":ConsolePosX" ), 20L );
                    consolePosition.y =  pConf->Read ( name + _T ( ":ConsolePosY" ), 20L );
                    dialogPosition.x =  pConf->Read ( name + _T ( ":DialogPosX" ), 20L );
                    dialogPosition.y =  pConf->Read ( name + _T ( ":DialogPosY" ), 20L );
                    alertPosition.x =  pConf->Read ( name + _T ( ":AlertPosX" ), 20L );
                    alertPosition.y =  pConf->Read ( name + _T ( ":AlertPosY" ), 20L );
                    fileString = pConf->Read ( name + _T ( ":LoadFile" ), _T(""));
                    autoRun = (pConf->Read ( name + _T ( ":AutoRun" ), "0" ) == "0")?false:true;
                    new Console(m_parent_window, name, consolePosition, dialogPosition, alertPosition, fileString, autoRun);
                    }
                }
            }
        return true;
    }
    else
#endif
        return false;
}

bool JavaScript_pi::SaveConfig(void)
{
    wxFileConfig *pConf = (wxFileConfig *)m_pconfig;
    Console *pConsole;
    
    TRACE(3,"JavaScript_pi->SaveConfig() entered");
    
    if(pConf)
    {
        wxString consoleNames {wxEmptyString};
        wxString name, nameColon;
        pConf->DeleteGroup ( _T ( "/Settings/JavaScript_pi" ) );
        pConf->SetPath ( _T ( "/Settings/JavaScript_pi" ) );
        pConf->Write ( _T ( "VersionMajor" ), PLUGIN_VERSION_MAJOR );
        pConf->Write ( _T ( "VersionMinor" ), PLUGIN_VERSION_MINOR );
        pConf->Write ( _T ( "ShowJavaScriptIcon" ), m_bJavaScriptShowIcon );
        pConf->Write ( _T ( "CurrentDirectory" ), mCurrentDirectory );
        for (pConsole = pJavaScript_pi->mpFirstConsole; pConsole != nullptr; pConsole = pConsole->mpNextConsole){
            name = pConsole->mConsoleName;
            nameColon = name + ":";
            consoleNames += ((pConsole == pJavaScript_pi->mpFirstConsole)? "":":") + name;
            wxPoint p = pConsole->GetPosition();
            pConf->Write (nameColon + _T ( "ConsolePosX" ),   p.x );
            pConf->Write (nameColon + _T ( "ConsolePosY" ),   p.y );
            pConf->Write (nameColon + _T ( "DialogPosX" ),   pConsole->mDialog.position.x );
            pConf->Write (nameColon + _T ( "DialogPosY" ),   pConsole->mDialog.position.y);
            pConf->Write (nameColon + _T ( "AlertPosX" ),   pConsole->mAlert.position.x );
            pConf->Write (nameColon + _T ( "AlertPosY" ),   pConsole->mAlert.position.y);
            pConf->Write (nameColon + _T ( "AutoRun" ),   (pConsole->auto_run->GetValue())?"1":"0");
            pConf->Write (nameColon + _T ("LoadFile"),  pConsole->m_fileStringBox->GetValue());
            }
        pConf->Write (_T ("Consoles"),  consoleNames);
        return true;
    }
    else
        return false;
}

void JavaScript_pi::SetNMEASentence(wxString &sentence)
{    // NMEA sentence received
    wxString thisFunction, checksum, correctChecksum;
    size_t starPos;
    bool OK {false};
    status_t outcome;;
    duk_context *ctx;
    wxUniChar star = '*';
    Console *m_pConsole;
    void JSduk_start_exec_timeout(Console);
    void  JSduk_clear_exec_timeout(Console);
    auto ComputeChecksum{       // Using Lambda function here to keep it private to this function
        [](wxString sentence) {
            unsigned char calculated_checksum = 0;
            for(wxString::const_iterator i = sentence.begin()+1; i != sentence.end() && *i != '*'; ++i)
                calculated_checksum ^= static_cast<unsigned char> (*i);
            return( wxString::Format("%02X", calculated_checksum) );
            }
        };
    bool haveDoneChecksum = false;
    // work through all consoles
    for (m_pConsole = pJavaScript_pi->mpFirstConsole; m_pConsole != nullptr; m_pConsole = m_pConsole->mpNextConsole){
        if (m_pConsole == nullptr) continue;  // ignore if not ready
        if (!m_pConsole->isWaiting()) continue;
        if (!haveDoneChecksum){ // only do this once, avoiding repeat for each console
            thisFunction = m_pConsole->m_NMEAmessageFunction;  // function to be called - if any
            if (thisFunction == wxEmptyString) continue;  // not waiting for NMEA
            m_pConsole->m_NMEAmessageFunction = wxEmptyString; // call only once
            sentence.Trim();
            // check the checksum
            starPos = sentence.find(star); // position of *
            if (starPos != wxNOT_FOUND){ // yes there is one
                checksum = sentence.Mid(starPos + 1, 2);
                sentence = sentence.SubString(0, starPos-1); // truncate at * onwards
                }
            correctChecksum = ComputeChecksum(sentence);
            OK = (checksum == correctChecksum) ? true : false;
            sentence = sentence.BeforeFirst('*');   // drop * onwards
            }
        ctx = m_pConsole->mpCtx;
        duk_push_object(ctx);
        duk_push_string(ctx, sentence.c_str());
        duk_put_prop_literal(ctx, -2, "value");
        duk_push_boolean(ctx, OK);
        duk_put_prop_literal(ctx, -2, "OK");
        outcome = m_pConsole->executeFunction(thisFunction);
        if (outcome == HAD_ERROR) {
            m_pConsole->wrapUp(HAD_ERROR);
            }
        else if ((outcome == DONE)||(outcome == STOPPED)) {
            m_pConsole->wrapUp(DONE);
            }
        }   // end for this console
    }

void JavaScript_pi::SetPositionFixEx(PlugIn_Position_Fix_Ex &pfix)
{   // note the pfix for others use
    m_positionFix = pfix;
}

void JavaScript_pi::OnTimer(wxTimerEvent& tick){
    Console* pConsole;
    for (pConsole = pJavaScript_pi->mpFirstConsole; pConsole != nullptr; pConsole = pConsole->mpNextConsole){

        // look out to see if we need to automatically run this console
        if (pConsole->mWaitingToRun){
            TRACE(3, "About to auto-run console " + pConsole->mConsoleName);
            pConsole->mWaitingToRun = false;
            pConsole->run(pConsole->m_Script->GetValue());
            TRACE(3, "Finished auto-running console " + pConsole->mConsoleName);
            }
        
        if (!pConsole->isWaiting()) continue;  // ignore if we are not waiting on something
        if (!pConsole->mTimerActionBusy){  // only look at timers if not already working on a timer - to stop recursion
            if (!pConsole->mTimes.IsEmpty()){
                int count;
                TRACE(3, "Looking at timers for console " + pConsole->mConsoleName);
                pConsole->mTimerActionBusy = true;  // There is at least one timer running - stop being called again until we are done
                count = (int)pConsole->mTimes.GetCount();
                 for (int i = 0; i < count; i++){
                    if (pConsole->mTimes[i].timeToCall <= wxDateTime::Now()){
                        // this one due
                        wxString argument;
                        jsFunctionNameString_t thisFunction;
                        duk_context *ctx = pConsole->mpCtx;
                        thisFunction = pConsole->mTimes[i].functionName;
                        argument = pConsole->mTimes[i].argument;
                        pConsole->mTimes.RemoveAt(i);
                        count--; i--;
                        if (thisFunction != wxEmptyString){  //only if have valid function
                            Completions  outcome;
                            duk_push_string(ctx, argument.c_str());
                            outcome = pConsole->executeFunction(thisFunction);
                            if (outcome == MORE) continue;
                            else {
                                i = 99999;  // this will stop us looking for further timers on this console
                                pConsole->wrapUp(outcome);
                                }
                            }
                        }
                    }
                }
            }
        pConsole->mTimerActionBusy = false;
        }
    while (mpBin) {    // empty the bin if anything in it
        Console* pConsole;
        TRACE(3,"JavaScript plugin deleting console " + mpBin->mConsoleName + " from bin");
        pConsole = mpBin;
        mpBin = pConsole->mpNextConsole; // take first off chain
        pConsole->Destroy();
        }
    }

void JavaScript_pi::SetPluginMessage(wxString &message_id, wxString &message_body) {
    // message received but we use this regular event to also do many things
    int index;
    int outcome;
    Console *m_pConsole;
    extern JavaScript_pi *pJavaScript_pi;
    wxString statusesToString(status_t mStatus);
    TRACE(15,"Entered SetPluginMessage");
    if (message_id == "OpenCPN Config"){
//    if (message_id.Cmp("OpenCPN Config")){
        // capture this while we can
        TRACE(4, "Captured openCPNConfig");
        openCPNConfig = message_body;
        };
    // work through all consoles
    for (m_pConsole = pJavaScript_pi->mpFirstConsole; m_pConsole != nullptr; m_pConsole = m_pConsole->mpNextConsole){
        jsFunctionNameString_t thisFunction;
        if (m_pConsole == nullptr) continue;  // ignore if not ready
        duk_context *ctx = m_pConsole->mpCtx;
        // remember the message
        index = m_pConsole->OCPNmessageIndex(message_id);  // look up and remember if new
        //now to deal with the message unless no longer waiting
        if (!m_pConsole->isWaiting()) continue;  // ignore if we are not waiting on something
        thisFunction = m_pConsole->mMessages[index].functionName;
        if (thisFunction != wxEmptyString){
            // have function to be invoked
            m_pConsole->mMessages[index].functionName = wxEmptyString;  // do not call again
            TRACE(3, "About to process message for console " + m_pConsole->mConsoleName + " " + m_pConsole->consoleDump());
            m_pConsole->mMessages[index].functionName = wxEmptyString;  // only use once
            if (!ctx) continue; // just in case
            duk_push_string(ctx, message_body.c_str());
            TRACE(3, "Will execute function " /* + m_pConsole->dukDump() */);
            outcome = m_pConsole->executeFunction(thisFunction);
            TRACE(3, "Have processed message for console " + m_pConsole->mConsoleName + " and result was " +  statusesToString(m_pConsole->mStatus.set(outcome)));
            if (outcome == HAD_ERROR){
                m_pConsole->wrapUp(HAD_ERROR);
                }
            else if ((outcome == DONE)||(outcome == STOPPED)) {
                m_pConsole->wrapUp(DONE);
                }
            }
        }
    }

#include "toolsDialogImp.h"
//void JavaScript_pi::ShowToolsDialog(wxWindow *m_parent_window ){
void JavaScript_pi::ShowPreferencesDialog(wxWindow *m_parent_window ){
    if (pTools != nullptr) return;    // ignore if already open
    pTools = new ToolsClass(m_parent_window, wxID_ANY, "JavaScript Tools");
    // next a horrible kludge.  Cannot get a reference to this plugin to compile in the
    // tools distructor, so will store its address in the preference for later access.
    pTools->pPointerToThisInJavaScript_pi = &this->pTools;  // yuck!
    pTools->Show();
    };
