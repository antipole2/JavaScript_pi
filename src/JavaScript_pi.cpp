/**************************************************************************
* Project:  OpenCPN
* Purpose:  JavaScript Plugin
* Author:   Tony Voss 16/05/2020
*
* Copyright Ⓒ 2023 by Tony Voss
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
wxString configSection = ("/PlugIns/JavaScript_pi");

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr)
{
    return new JavaScript_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p)
{
    delete p;
}

#define FS ";"	// character to use as file separator in recents and favourites in .in file

//----------------------------------------------------------------------------------------
//
//    JavaScript PlugIn Implementation
//
//----------------------------------------------------------------------------------------

JavaScript_pi *pJavaScript_pi;     // we will keep a pointer to ourself here

JavaScript_pi::JavaScript_pi(void *ppimgr)
:opencpn_plugin_118	 (ppimgr)  // was 17
{
#ifndef IN_HARNESS
    // Create the PlugIn icons
    initialize_images();

	wxFileName fn;
    auto path = GetPluginDataDir("JavaScript_pi");
    fn.SetPath(path);
    fn.AppendDir("data");
    fn.SetFullName("JavaScript_pi_panel_icon.png");

    path = fn.GetFullPath();
//    wxString forDebug = fn.GetFullPath();
//    wxInitAllImageHandlers();

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
#endif // not IN_HARNESS
}

JavaScript_pi::~JavaScript_pi(void)
{
    delete _img_JavaScript_pi;
    delete _img_JavaScript;

}

int JavaScript_pi::Init(void)
{
    ::wxDisplaySize(&m_display_width, &m_display_height);
    
    wxLogMessage("JavaScript_pi->Init() entered");
    m_parent_window = GetOCPNCanvasWindow();
    pJavaScript_pi = this;  // Leave a way to find ourselves
    TRACE(1,"JavaScript_pi->Init() entered");
    AddLocaleCatalog( _T("opencpn-JavaScript_pi") );

    pTools = nullptr;
    mpFirstConsole = nullptr;
    mpBin = nullptr;

    //    Get a pointer to the opencpn configuration object
    m_pconfig = GetOCPNConfigObject();

    //    And load the configuration items
    LoadConfig();
    mShowingConsoles = false;   // consoles will hence be shown on toolbar callback

    //    This PlugIn needs a toolbar icon, so request its insertion
    if(m_bJavaScriptShowIcon){

#ifdef JavaScript_USE_SVG
        m_leftclick_tool_id = InsertPlugInToolSVG(_T("JavaScript"), _svg_JavaScript, _svg_JavaScript, _svg_JavaScript_toggled,
            wxITEM_CHECK, "JavaScript", _T(""), NULL, CONSOLE_POSITION, 0, this);
#else
    m_leftclick_tool_id = InsertPlugInTool(_T(""), _img_JavaScript, _img_JavaScript, wxITEM_CHECK,
                                           "JavaScript",_T(""), NULL,
                                           CONSOLE_POSITION, 0, this);
#endif // JavaScript_USE_SVG
    }
    mpPluginActive = true;
    mTimer.Bind(wxEVT_TIMER, &JavaScript_pi::OnTimer, this, mTimer.GetId());
    mTimer.Start(1000);

    TRACE(1,"JavaScript_pi->Init() returning");

    return (WANTS_TOOLBAR_CALLBACK |
            WANTS_CURSOR_LATLON |
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

    if (pTools != nullptr) {
        TRACE(3,"JavaScript plugin DeInit destroying tools pane");
 //       pTools->Close();
 //       pTools->Destroy();
        delete pTools;
        pTools = nullptr;
        }

    while (mpFirstConsole) {    // close all remaining consoles
        TRACE(3,"JavaScript plugin DeInit closing console " + mpFirstConsole->mConsoleName);
        pConsole = mpFirstConsole;
        // purge stuff out of this one - we do not use wrapUp() because that might do all sorts of other things
        if (pConsole->mpCtx != nullptr) duk_destroy_heap(pConsole->mpCtx);
        pConsole->mMessages.Clear();
        pConsole->mTimes.Clear();
        pConsole->clearAlert();
        pConsole->clearDialog();
#ifdef SOCKETS
		pConsole->clearSockets();
#endif
        mpFirstConsole = pConsole->mpNextConsole; // unhook first off chain
        delete pConsole;
        pConsole = nullptr;
        }
    mpFirstConsole = nullptr;	// guard against doing deinit again

    while (mpBin) {    // also any in the bin
        TRACE(3,"JavaScript plugin DeInit deleting console " + mpBin->mConsoleName + " from bin");
        pConsole = mpBin;
        if (pConsole->mpCtx != nullptr) { // deferred heap distruction
            duk_destroy_heap(pConsole->mpCtx);
            pConsole->mpCtx = nullptr;  // don't need this but...
            TRACE(3,"JavaScript plugin deinit destroying console" + pConsole->mConsoleName + " ctx while emtying bin");
            }
        mpBin = pConsole->mpNextConsole; // take first off chain
        delete pConsole;
        pConsole = NULL;
        }
    mpPluginActive = false;
    SetToolbarItemState(m_leftclick_tool_id, mpPluginActive);
    RequestRefresh(m_parent_window); // refresh main window
    wxLogMessage("JavaScript completed deinit");
    TRACE(1,"JavaScript_pi->DeInit() returning");    
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
    return "javascript";
}


wxString JavaScript_pi::GetShortDescription()
{
    return "Run JavaScripts and interact with OpenCPN";
}

wxString JavaScript_pi::GetLongDescription()
{
    return "Run JavaScripts and interact with OpenCPN\n\
See User Guide at https://opencpn-manuals.github.io/main/javascript/index.html\n\
and fuller details in the plugin Help ";
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
	if (mShowingConsoles & m_showHelp){
			ShowTools(m_parent_window, -1);	// show them the help page
			m_showHelp = false;
			}
    Console *m_pConsole = mpFirstConsole;
    for (m_pConsole = mpFirstConsole; m_pConsole != nullptr; m_pConsole = m_pConsole->mpNextConsole){
        if (mShowingConsoles)   m_pConsole->Show();
        else m_pConsole->Hide();
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
    wxString fileNames;
#ifndef IN_HARNESS
	m_showHelp = false;
    if(pConf){
    	wxString welcome = wxString(PLUGIN_FIRST_TIME_WELCOME);
        pConf->SetPath (configSection);
        bool configUptoDate = pConf->Read ( _T( "ShowJavaScriptIcon" ), &m_bJavaScriptShowIcon, 1 );
        if (!configUptoDate){	// configuration may be pre-v0.5
        	pConf->SetPath("/Settings/JavaScript_pi");	// try old location
        	pConf->Read ( _T( "ShowJavaScriptIcon" ), &m_bJavaScriptShowIcon, 1 );
        	}
        int versionMajor = pConf->Read ( _T ( "VersionMajor" ), 0L );
        int versionMinor = pConf->Read ( _T ( "VersionMinor" ), 0L );
        if ((versionMajor == 0) && (versionMinor < 4)){
            TRACE(2, "Pre v0.4 or first time ever - creating default consoles");
            // must be old version - forget previous settings
            pConf->DeleteGroup ( _T ( "/Settings/JavaScript_pi" ) );
            pConf->SetPath ( _T ( "/Settings/JavaScript_pi" ) );
            mCurrentDirectory = wxStandardPaths::Get().GetDocumentsDir();
            // create one default console
            mpFirstConsole = new Console(m_parent_window, "JavaScript");
            mpFirstConsole->setConsoleMinSize();
            mpFirstConsole->m_Output->AppendText(welcome);
            m_showHelp = true;
            }
        else {
        	if ((versionMajor < PLUGIN_VERSION_MAJOR ) || ((versionMajor <= PLUGIN_VERSION_MAJOR ) && (versionMinor < PLUGIN_VERSION_MINOR))){ // updated
				welcome = wxString(PLUGIN_UPDATE_WELCOME);
				m_showHelp = true;
				}
            TRACE(2, "Loading console configurations");
            mCurrentDirectory = pConf->Read(_T("CurrentDirectory"), _T("") );
            TRACE(2, "Current Directory set to " + mCurrentDirectory);
            
            // load parking config - platform defaults if none
            // saved and default values are in DIP
            m_parkingBespoke = ((pConf->Read( "ParkingBespoke" , 0L) == 1)) ? true : false;	// if none, set false
            m_parkingMinHeight = pConf->Read("ParkingMinHeight", CONSOLE_MIN_HEIGHT);
            m_parkingStub = pConf->Read("ParkingStub", CONSOLE_STUB);
            m_parkingLevel = pConf->Read("ParkingLevel", PARK_LEVEL);
            m_parkFirstX = pConf->Read("ParkingFirstX", PARK_FIRST_X);
            m_parkSep = pConf->Read("ParkingSep", PARK_SEP);
            TRACE(4, wxString::Format("Loaded parking config ParkingBespoke:%s ParkingMinHeight:%i, ParkingStub:%i ",
            	(m_parkingBespoke?"true":"false"), m_parkingMinHeight, m_parkingStub ));
            
            // create consoles as in config file
            mpFirstConsole = nullptr;	//start with no consoles
            wxString consoles = pConf->Read ( _T ( "Consoles" ), _T("") );
            if (consoles == wxEmptyString){ // no consoles configured
                Console* newConsole = new Console(m_parent_window, "JavaScript", wxPoint(300,20),
                	wxSize(738,800), wxPoint(150, 100), wxPoint(90, 20), wxEmptyString, false, welcome);
                newConsole->setConsoleMinSize();
                }
            else {
                wxStringTokenizer tkz(consoles, ":");
                wxString welcome = wxEmptyString;
                if ((versionMajor < PLUGIN_VERSION_MAJOR ) || ((versionMajor <= PLUGIN_VERSION_MAJOR ) && (versionMinor < PLUGIN_VERSION_MINOR))){
                    welcome = wxString(PLUGIN_UPDATE_WELCOME);
                    }
                while ( tkz.HasMoreTokens() ){
                    wxPoint consolePosition, dialogPosition, alertPosition;
                    wxSize  consoleSize;
                    wxString fileString;
                    bool autoRun, parked;
                    wxPoint frameToScreen(wxPoint);

                    wxString name = tkz.GetNextToken();
                    consolePosition.x =  pConf->Read ( name + _T ( ":ConsolePosX" ), 20L );
                    consolePosition.y =  pConf->Read ( name + _T ( ":ConsolePosY" ), 20L );
                    consoleSize.x =  pConf->Read ( name + _T ( ":ConsoleSizeX" ), 20L );
                    consoleSize.y =  pConf->Read ( name + _T ( ":ConsoleSizeY" ), 20L );
                    dialogPosition.x =  pConf->Read ( name + _T ( ":DialogPosX" ), 20L );
                    dialogPosition.y =  pConf->Read ( name + _T ( ":DialogPosY" ), 20L );
                    alertPosition.x =  pConf->Read ( name + _T ( ":AlertPosX" ), 20L );
                    alertPosition.y =  pConf->Read ( name + _T ( ":AlertPosY" ), 20L );
                    fileString = pConf->Read ( name + _T ( ":LoadFile" ), _T(""));
                    autoRun = (pConf->Read ( name + _T ( ":AutoRun" ), "0" ) == "0")?false:true;
                    parked = (pConf->Read ( name + _T ( ":Parked" ), "0" ) == "0")?false:true;
                    // take care of no console size
                    if ((consoleSize.x < 80)|| (consoleSize.y < 9)) consoleSize = wxSize(738,800);
                    // from V2 positions have been saved relative to frame
                    Console* newConsole = new Console(m_parent_window , name, consolePosition, consoleSize, dialogPosition, alertPosition, fileString, autoRun,  welcome, parked);
                    newConsole->setConsoleMinSize();
                    if (parked) newConsole->park();
                    }
                }
            }
        fileNames = pConf->Read ( _T ( "Recents" ), _T("") );
        if (fileNames != wxEmptyString){	// we have some recent file names
        	wxStringTokenizer tkz(fileNames, FS);
        	while ( tkz.HasMoreTokens() ){
        		wxString name = tkz.GetNextToken();
        		recentFiles.Add(name);
        		}
        	}
        fileNames = pConf->Read ( _T ( "Favourites" ), _T("") );
        if (fileNames != wxEmptyString){    // we have some favourite file names
            wxStringTokenizer tkz(fileNames, FS);
            while ( tkz.HasMoreTokens() ){
                wxString name = tkz.GetNextToken();
                favouriteFiles.Add(name);
                }
            }
        if (!configUptoDate){	// read config was from old place
        	pConf->DeleteGroup ( _T ( "/Settings/JavaScript_pi" ) );	// delete the old one
        	pConf->SetPath (configSection);	// the new grouping
        	pConf->Flush();	// make sure it gets into new location
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
        pConf->DeleteGroup (configSection);
        pConf->SetPath (configSection);
        pConf->Write ( _T ( "VersionMajor" ), PLUGIN_VERSION_MAJOR );
        pConf->Write ( _T ( "VersionMinor" ), PLUGIN_VERSION_MINOR );
        pConf->Write ( _T ( "ShowJavaScriptIcon" ), m_bJavaScriptShowIcon );
        pConf->Write ( _T ( "CurrentDirectory" ), mCurrentDirectory );
        
        // now to save the recent files list - if any
        if (recentFiles.GetCount() > 0){
			wxString recents;
			for (int i = 0; i < recentFiles.GetCount(); i++){
				recents += recentFiles.Item(i) + ";";        	
				}
			recents = recents.BeforeLast(wxString(";").Last());	// drop last :
			pConf->Write (_T ("Recents"),  recents);
			}
        
        // and the favourite files list - if any
        if (favouriteFiles.GetCount() > 0){
            wxString favourites;
            for (int i = 0; i < favouriteFiles.GetCount(); i++){
                favourites += favouriteFiles.Item(i) + FS;
                }
            favourites = favourites.BeforeLast(wxString(FS).Last());    // drop last :
            pConf->Write (_T ("Favourites"),  favourites);
            }
 //        double scale = m_parent_window->GetDPIScaleFactor();	// we will save config in DIP
            
        //save custom parking config, if any
        if (m_parkingBespoke){
        	pConf->Write (nameColon + _T ( "ParkingBespoke" ),   1 );
        	pConf->Write (nameColon + _T ( "ParkingMinHeight" ),   m_parkingMinHeight);
        	pConf->Write (nameColon + _T ( "ParkingStub" ),   m_parkingStub);
         	pConf->Write (nameColon + _T ( "ParkingLevel" ),   m_parkingLevel);
         	pConf->Write (nameColon + _T ( "ParkingFirstX" ),   m_parkFirstX);
         	pConf->Write (nameColon + _T ( "ParkingSep" ),   m_parkSep);  	
        	}

        for (pConsole = pJavaScript_pi->mpFirstConsole; pConsole != nullptr; pConsole = pConsole->mpNextConsole){
        	wxPoint screenToFrame(wxPoint);
        	// v2 positions now saved relative to frame
            name = pConsole->mConsoleName;
            nameColon = name + ":";
            // will save in DIP
            consoleNames += ((pConsole == pJavaScript_pi->mpFirstConsole)? "":":") + name;
            wxPoint consolePosition = m_parent_window->ToDIP(screenToFrame(pConsole->GetPosition()));
            wxPoint dialogPosition = m_parent_window->ToDIP(screenToFrame(pConsole->mDialog.position));
            wxPoint alertPosition = m_parent_window->ToDIP(screenToFrame(pConsole->mAlert.position));
            wxSize  consoleSize = m_parent_window->ToDIP(pConsole->GetSize());            
            pConf->Write (nameColon + _T ( "Parked" ),   (pConsole->isParked())?"1":"0");	// first in case it has been moved
            pConf->Write (nameColon + _T ( "ConsolePosX" ),   consolePosition.x );
            pConf->Write (nameColon + _T ( "ConsolePosY" ),   consolePosition.y );
            pConf->Write (nameColon + _T ( "ConsoleSizeX" ),   consoleSize.x );
            pConf->Write (nameColon + _T ( "ConsoleSizeY" ),   consoleSize.y );
            pConf->Write (nameColon + _T ( "DialogPosX" ),   dialogPosition.x );
            pConf->Write (nameColon + _T ( "DialogPosY" ),   dialogPosition.y);
            pConf->Write (nameColon + _T ( "AlertPosX" ),   alertPosition.x );
            pConf->Write (nameColon + _T ( "AlertPosY" ),  alertPosition.y);
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
    Completions outcome;
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
    for (m_pConsole = pJavaScript_pi->mpFirstConsole; m_pConsole != nullptr; m_pConsole = m_pConsole->mpNextConsole){    // work through all consoles
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
        if (m_pConsole->mJSrunning){
            m_pConsole->message(STYLE_RED, "NMEA callback while JS active - ignored\n");
            return;
            }

        duk_push_object(ctx);
        duk_push_string(ctx, sentence.c_str());
        duk_put_prop_literal(ctx, -2, "value");
        duk_push_boolean(ctx, OK);
        duk_put_prop_literal(ctx, -2, "OK");
        outcome = m_pConsole->executeFunction(thisFunction);
        m_pConsole->wrapUp(outcome);
        }   // end for this console
    }

void JavaScript_pi::SetPositionFixEx(PlugIn_Position_Fix_Ex &pfix)
{   // note the pfix for others use
    m_positionFix = pfix;
}

void JavaScript_pi::SetCursorLatLon(double lat, double lon){
    mCursorPosition.lat = lat;
    mCursorPosition.lon = lon;
    }

void JavaScript_pi::SetActiveLegInfo( Plugin_Active_Leg_Info &leg_info)
{
    wxString thisFunction;
    duk_context *ctx;
    Console *m_pConsole;
    void JSduk_start_exec_timeout(Console);
    void  JSduk_clear_exec_timeout(Console);
    for (m_pConsole = pJavaScript_pi->mpFirstConsole; m_pConsole != nullptr; m_pConsole = m_pConsole->mpNextConsole){    // work through all consoles
        if (m_pConsole == nullptr) continue;  // ignore if not ready
        if (!m_pConsole->isWaiting()) continue;
        thisFunction = m_pConsole->m_activeLegFunction;  // function to be called - if any
        if (thisFunction == wxEmptyString) continue;  // not waiting for active leg_info
        m_pConsole->m_activeLegFunction = wxEmptyString; // call only once
        ctx = m_pConsole->mpCtx;
        duk_push_object(ctx);
        duk_push_string(ctx, leg_info.wp_name);
        duk_put_prop_literal(ctx, -2, "markName");
        duk_push_number(ctx, leg_info.Btw);
        duk_put_prop_literal(ctx, -2, "bearing");
        duk_push_number(ctx, leg_info.Dtw);
        duk_put_prop_literal(ctx, -2, "distance");
        duk_push_number(ctx, leg_info.Xte);
        duk_put_prop_literal(ctx, -2, "xte");
        duk_push_boolean(ctx, leg_info.arrival);
        duk_put_prop_literal(ctx, -2, "arrived");
        Completions outcome = m_pConsole->executeFunction(thisFunction);
        if (!m_pConsole->isBusy()) m_pConsole->wrapUp(outcome);
        }   // end for this console
}

void JavaScript_pi::OnTimer(wxTimerEvent& ){
    // This is the regular timer tick through which all timed functions are implemented
    Console* pConsole;
    for (pConsole = pJavaScript_pi->mpFirstConsole; pConsole != nullptr; pConsole = pConsole->mpNextConsole){	// for each console
        // look out to see if we need to automatically run this console
        if (pConsole->mWaitingToRun){
            TRACE(3, "About to auto-run console " + pConsole->mConsoleName);
            pConsole->mWaitingToRun = false;
            pConsole->clearBrief();
            Completions outcome = pConsole->run(pConsole->m_Script->GetValue());
            if (!pConsole->isBusy()) pConsole->wrapUp(outcome);
            TRACE(3, "Finished auto-running console " + pConsole->mConsoleName);
            }

        if (!pConsole->isWaiting()) continue;  // ignore if we are not waiting on something
        if (!pConsole->mTimerActionBusy){  // only look at timers if not already working on a timer - to stop recursion
            if (!pConsole->mTimes.IsEmpty()){
                int count;
                TRACE(8, "Looking at timers for console " + pConsole->mConsoleName);
                pConsole->mTimerActionBusy = true;  // There is at least one timer running - stop being called again until we are done
                count = (int)pConsole->mTimes.GetCount();
                 for (int i = 0; i < count; i++){
                    if (pConsole->mTimes[i].timeToCall <= wxDateTime::Now()){  // this one due
                        wxString argument;
                        jsFunctionNameString_t thisFunction;
                        duk_context *ctx = pConsole->mpCtx;
                        thisFunction = pConsole->mTimes[i].functionName;
                        argument = pConsole->mTimes[i].argument;
                        pConsole->mTimes.RemoveAt(i);	// remove timer to stop it firing again
                        count--; i--;	// adjust because have removed item
                        if (thisFunction != wxEmptyString){  //only if have valid function
                            Completions  outcome;
                            if (pConsole->mJSrunning){
                                pConsole->message(STYLE_RED, "Timer callback while JS active - ignored\n");
                                return;
                                }

                            duk_push_string(ctx, argument.c_str());
                            outcome = pConsole->executeFunction(thisFunction);
                            if (outcome == MORETODO) continue;
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

    Console* pLater = nullptr;  // the linked list of consoles whose deletion is to be deferred
    while (mpBin) {    // empty the bin if anything in it that can go
        Console* pThisConsole = mpBin;
        mpBin = pThisConsole->mpNextConsole; // take first off chain
        if (pThisConsole->isBusy()){  // this console's messageBox not yet dismissed
            TRACE(8,"JavaScript plugin console " + pThisConsole->mConsoleName + " deletion from bin deferred");
            // push it onto the do later chain
            pThisConsole->mpNextConsole = (pLater == nullptr) ? nullptr : pLater;
            pLater = pThisConsole;
            }
        else {
            if (pThisConsole->mpCtx != nullptr) { // deferred heap distruction
                duk_destroy_heap(pThisConsole->mpCtx);
                pThisConsole->mpCtx = nullptr;  // don't need this but...
                TRACE(3,"JavaScript plugin console " + pThisConsole->mConsoleName + " destroying ctx while emtying bin");
                }
            TRACE(3,"JavaScript plugin deleting console " + pThisConsole->mConsoleName + " from bin");
            pThisConsole->Destroy();
            }
        }
    mpBin = pLater; // try later
    }

void JavaScript_pi::OnContextMenuItemCallback(int menuID){
    Console* pConsole;
    for (pConsole = pJavaScript_pi->mpFirstConsole; pConsole != nullptr; pConsole = pConsole->mpNextConsole){
        if (!pConsole->isWaiting()) continue;  // ignore if we are not waiting on something
        if (!pConsole->mMenus.IsEmpty()){
            int count;
            TRACE(3, "Looking at menus for console " + pConsole->mConsoleName);
            count = (int)pConsole->mMenus.GetCount();
             for (int i = 0; i < count; i++){
                if (pConsole->mMenus[i].menuID == menuID){
                    // it's for this one
                    wxString argument;
                    jsFunctionNameString_t thisFunction;
                    duk_context *ctx = pConsole->mpCtx;
                    thisFunction = pConsole->mMenus[i].functionName;
                    argument = pConsole->mMenus[i].argument;
                    RemoveCanvasContextMenuItem(pConsole->mMenus[i].menuID);
                    pConsole->mMenus.RemoveAt(i);
                    count--; i--;
                    if (thisFunction != wxEmptyString){  //only if have valid function
                        if (pConsole->mJSrunning){
                            pConsole->message(STYLE_RED, "Menu callback while JS active - ignored\n");
                            return;
                            }
                        // we return a position, supplemented with the info from when it was set up.
						duk_push_object(ctx);
						duk_push_number(ctx, mCursorPosition.lat);
						duk_put_prop_literal(ctx, -2, "latitude");
						duk_push_number(ctx, mCursorPosition.lon);
						duk_put_prop_literal(ctx, -2, "longitude");
						duk_push_string(ctx, argument.c_str());
						duk_put_prop_literal(ctx, -2, "info");                       	
                        Completions outcome = pConsole->executeFunction(thisFunction);
                        pConsole->wrapUp(outcome);
                        return;
                        }
                    }
                }
            }
        }
    }

void JavaScript_pi::SetPluginMessage(wxString &message_id, wxString &message_body) {
    // message received but we use this regular event to also do many things
    int index;
    Completions outcome;
    Console *m_pConsole;
    extern JavaScript_pi *pJavaScript_pi;
    wxString statusesToString(status_t mStatus);
    TRACE(5,"Entered SetPlugin Message message_id: " + message_id + " message:" + message_body);
    if (message_id == "OpenCPN Config"){
        // capture this while we can
        TRACE(4, "Captured openCPNConfig");
        openCPNConfig = message_body;
        };
    for (m_pConsole = pJavaScript_pi->mpFirstConsole; m_pConsole != nullptr; m_pConsole = m_pConsole->mpNextConsole){    // work through all consoles
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
            if (m_pConsole->mJSrunning){
                m_pConsole->message(STYLE_RED, "Message callback while JS active - ignored - message is:\n" + message_body + "\n");
                return;
                }
            TRACE(3, "Will execute function " /* + m_pConsole->dukDump() */);
            outcome = m_pConsole->executeFunction(thisFunction);
            TRACE(3, "Have processed message for console " + m_pConsole->mConsoleName + " and result was " +  statusesToString(m_pConsole->mStatus.set(outcome)));
            m_pConsole->wrapUp(outcome);
            }
        }
    }

#include "toolsDialogImp.h"
#include "wx/display.h"
	
void JavaScript_pi::ShowPreferencesDialog(wxWindow *m_parent_window){
	TRACE(60, "ShowPreferencesDialog invoked ");
	ShowTools(m_parent_window, -1);
	}


void JavaScript_pi::ShowTools(wxWindow *m_parent_window, int page){
	// if page == -1, show Help
	TRACE(60, wxString::Format("entered ShowTools page: %i", page));

    if (pTools == nullptr) {  // do not yet have a tools dialogue
    	int x, y;
        pTools = new ToolsClass(m_parent_window, wxID_ANY /*, "JavaScript Tools" */);
        // position it top right in display
        wxWindow* frame = GetOCPNCanvasWindow()->GetParent();
        wxDisplay display(wxDisplay::GetFromWindow(frame));
		wxRect screen = display.GetClientArea();
        wxSize toolsSize = pTools->GetSize();      	
        x = screen.x + screen.width - toolsSize.x;
        y = screen.y;
        pTools->SetPosition(wxPoint(x,y));
        }
    pTools->Show();
    if (page == -1){
    	// need to determin which page is Help
    	int i;
		int count = pJavaScript_pi->pTools->m_notebook->GetPageCount();
		for (i = 0; i < count; i++){
			TRACE(60, wxString::Format("Show Tools count: %i, i: %i, GetPageText: %s", count, i, pJavaScript_pi->pTools->m_notebook->GetPageText(i)));
			if (pJavaScript_pi->pTools->m_notebook->GetPageText(i) == "Help") break;
			}
		page = i;
		}   
    pTools->setupPage(page);
    return;
    };

