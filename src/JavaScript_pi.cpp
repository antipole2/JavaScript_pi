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
    void reviewParking();    
    if (TRACE_YES) wxLogMessage("JavaScript_pi->Init() entered");
    m_parent_window = GetOCPNCanvasWindow();
    pJavaScript_pi = this;  // Leave a way to find ourselves
    TRACE(1,"JavaScript_pi->Init() entered");
    AddLocaleCatalog( _T("opencpn-JavaScript_pi") );

    pTools = nullptr;
    mpFirstConsole = nullptr;
    mpBin = nullptr;

    //    Get a pointer to the opencpn configuration object
    m_pconfig = GetOCPNConfigObject();
    if (m_pconfig == nullptr){
    	wxLogMessage("JavaScript_pi->Init() unable to find ConfigObject");
    	return 0;
    	}

    //    And load the configuration items
    LoadConfig();
//    mShowingConsoles = false;   // consoles will hence be shown on toolbar callback

    //    This PlugIn needs a toolbar icon, so request its insertion
    if (m_bJavaScriptShowIcon){

#ifdef JavaScript_USE_SVG
        m_leftclick_tool_id = InsertPlugInToolSVG(_T("JavaScript"), _svg_JavaScript, _svg_JavaScript, _svg_JavaScript_toggled,
            wxITEM_CHECK, "JavaScript", _T(""), NULL, CONSOLE_POSITION, 0, this);
#else
    m_leftclick_tool_id = InsertPlugInTool(_T(""), _img_JavaScript, _img_JavaScript, wxITEM_CHECK,
                                           "JavaScript",_T(""), NULL,
                                           CONSOLE_POSITION, 0, this);
#endif // JavaScript_USE_SVG
    }
    
    // we have taken the present show/hide state from the config file
    if (mShowingConsoles){	// need to show Now    
    	mShowingConsoles = false;	// pretend not showing
    	OnToolbarToolCallback(0);	// and toggle the state
    	}
   
    mpPluginActive = true;
    mTimer.Bind(wxEVT_TIMER, &JavaScript_pi::OnTimer, this); //, this, mTimer.GetId());
    mTimer.Start(15000);	// 15s timer to check for deleted consoles
    
    reviewParking();
    
    // look out to see if we need to automatically run any console
    for (Console* pConsole = mpFirstConsole; pConsole!= nullptr; pConsole = pConsole->mpNextConsole) {	// walk consoles chain
        if (pConsole->mWaitingToRun){
            TRACE(3, "About to auto-run console " + pConsole->mConsoleName);
            pConsole->mWaitingToRun = false;
            pConsole->clearBrief();
            Completions outcome = pConsole->run(pConsole->m_Script->GetValue());
            if (!pConsole->isBusy()) pConsole->wrapUp(outcome);
            TRACE(3, "Finished auto-running console " + pConsole->mConsoleName);
            }
        }

    TRACE(1,"JavaScript_pi->Init() returning");

    return (WANTS_TOOLBAR_CALLBACK |
            WANTS_CURSOR_LATLON |
            WANTS_CONFIG             |
            WANTS_PLUGIN_MESSAGING |
            WANTS_NMEA_SENTENCES |
            WANTS_NMEA_EVENTS |
            WANTS_AIS_SENTENCES |
            WANTS_PREFERENCES |
            WANTS_ONPAINT_VIEWPORT
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
        try{
//        	pTools->Destroy();
        	delete pTools;

        	}
        catch (int i){;}
        pTools = nullptr;
        }

    while (mpFirstConsole) {    // close all remaining consoles
        TRACE(3,"JavaScript plugin DeInit closing console " + mpFirstConsole->mConsoleName);
        pConsole = mpFirstConsole;
        // purge stuff out of this one - we do not use wrapUp() because that might do all sorts of other things
        if (pConsole->mpCtx != nullptr) duk_destroy_heap(pConsole->mpCtx);
        pConsole->mMessages.Clear();
        pConsole->mpTimersVector.clear();
        pConsole->clearAlert();
        pConsole->clearDialog();
        pConsole->clearMenus();
#ifdef SOCKETS
		pConsole->clearSockets();
#endif
        mpFirstConsole = pConsole->mpNextConsole; // unhook first off chain
//        pConsole->Destroy();
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
//        pConsole->Destroy();
        delete pConsole;
        pConsole = NULL;
        }
    mpPluginActive = false;
    SetToolbarItemState(m_leftclick_tool_id, mpPluginActive);
    RequestRefresh(m_parent_window); // refresh main window
    wxLogMessage("JavaScript completed deinit");
    TRACE(1,"JavaScript_pi->DeInit() returning");
#if TRACE_YES
	if (mpTraceWindow != nullptr) delete mpTraceWindow;
#endif
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
    return "Run JavaScripts";
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

    TRACE(34,wxString::Format("JavaScript_pi->OnToolbarToolCallback() entered with mShowingConsoles  %s", (mShowingConsoles ? "true":"false")));
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
    wxString welcome {wxEmptyString};
    if (TRACE_YES) wxLogMessage("Entered LoadConfig");
	TRACE(2, wxString::Format("Screen size width:%d height:%d", m_display_width, m_display_height));
	m_showHelp = false;
    if(pConf != nullptr){
        pConf->SetPath (configSection);
        bool configFound = pConf->Read ( _T( "ShowJavaScriptIcon" ), &m_bJavaScriptShowIcon, true );
        if (!configFound){
        	welcome =  PLUGIN_FIRST_TIME_WELCOME;
        	}
        else {
			int versionMajor = pConf->Read ( _T ( "VersionMajor" ), 0L );
			int versionMinor = pConf->Read ( _T ( "VersionMinor" ), 0L );
			if ((versionMajor < PLUGIN_VERSION_MAJOR ) || ((versionMajor <= PLUGIN_VERSION_MAJOR ) && (versionMinor < PLUGIN_VERSION_MINOR))){ // updated
				welcome = wxString(PLUGIN_UPDATE_WELCOME);
				m_showHelp = true;
				}
			}
		TRACE(2, "Loading console configurations");
		if (TRACE_YES) wxLogMessage("LoadConfig loading console configurations");
		mCurrentDirectory = pConf->Read(_T("CurrentDirectory"), _T("") );
		TRACE(2, "Current Directory set to " + mCurrentDirectory);
		mRememberToggleStatus = (pConf->Read ( _T ( "RememberToggle" ), "0" ) == "0")?false:true;
		if (mRememberToggleStatus) mShowingConsoles = (pConf->Read ( _T ( "ShowingConsoles" ), "0" ) == "0")?false:true;
		else mShowingConsoles = false;
		TRACE(2,wxString::Format("JavaScript_pi->LoadConfig() setting mShowingConsoles  %s", (mShowingConsoles ? "true":"false")));
#ifdef __DARWIN__
		m_floatOnParent = (pConf->Read ( _T ( "FloatOnParent" ), "0" ) == "0")?false:true;
#else
		m_floatOnParent = false;
#endif
		// load parking config - platform defaults if none
		// saved and default values are in DIP
		m_parkingBespoke = ((pConf->Read( "ParkingBespoke" , 0L) == 1)) ? true : false;	// if none, set false
		if (m_parkingBespoke){
			m_parkingStub = pConf->Read("ParkingStub", CONSOLE_STUB);
			m_parkingLevel = pConf->Read("ParkingLevel", PARK_LEVEL);
			m_parkFirstX = pConf->Read("ParkingFirstX", PARK_FIRST_X);
			m_parkSep = pConf->Read("ParkingSep", PARK_SEP);
			}
		TRACE(2, wxString::Format("Loaded parking config ParkingBespoke:%s ParkingStub:%i ",
			(m_parkingBespoke?"true":"false"), m_parkingStub ));
		
		// create consoles as in config file
		mpFirstConsole = nullptr;	//start with no consoles
		wxString consoles = pConf->Read ( _T ( "Consoles" ), _T("") );
		if (consoles == wxEmptyString){ // no consoles configured
			Console* newConsole = new Console(m_parent_window, "JavaScript", NEW_CONSOLE_POSITION,
				NEW_CONSOLE_SIZE, wxPoint(150, 100), wxPoint(90, 20), wxEmptyString, false, welcome);
			newConsole->setup(welcome);
			newConsole->setConsoleMinClientSize();
			// position and size likely wrong for hi res displays, and could not fix until after construction, so...
			wxPoint position = newConsole->FromDIP(NEW_CONSOLE_POSITION);
			newConsole->Move(position);
			wxSize  size = newConsole->FromDIP(NEW_CONSOLE_SIZE);
			newConsole->SetSize(size);
			}
		else {
			wxStringTokenizer tkz(consoles, ":");
//			wxString welcome = wxEmptyString;
//			if ((versionMajor < PLUGIN_VERSION_MAJOR ) || ((versionMajor <= PLUGIN_VERSION_MAJOR ) && (versionMinor < PLUGIN_VERSION_MINOR))){
//				welcome = wxString(PLUGIN_UPDATE_WELCOME);
//				}
			while ( tkz.HasMoreTokens() ){
				wxPoint consolePosition, dialogPosition, alertPosition;
				wxSize  consoleSize;
				wxString fileString;
				bool autoRun, parked;

				wxString name = tkz.GetNextToken();
				consolePosition.x =  pConf->Read ( name + _T ( ":ConsolePosX" ), 20L );
				consolePosition.y =  pConf->Read ( name + _T ( ":ConsolePosY" ), 20L );
				consoleSize.x =  pConf->Read ( name + _T ( ":ConsoleSizeX" ), 10L );
				consoleSize.y =  pConf->Read ( name + _T ( ":ConsoleSizeY" ), 5L );
				if ((consoleSize.x < 40) || (consoleSize.y < 15)) consoleSize = NEW_CONSOLE_SIZE;	// in case size is uselessly small
				dialogPosition.x =  pConf->Read ( name + _T ( ":DialogPosX" ), 20L );
				dialogPosition.y =  pConf->Read ( name + _T ( ":DialogPosY" ), 20L );
				alertPosition.x =  pConf->Read ( name + _T ( ":AlertPosX" ), 20L );
				alertPosition.y =  pConf->Read ( name + _T ( ":AlertPosY" ), 20L );
				fileString = pConf->Read ( name + _T ( ":LoadFile" ), _T(""));
				autoRun = (pConf->Read ( name + _T ( ":AutoRun" ), "0" ) == "0")?false:true;
				parked = (pConf->Read ( name + _T ( ":Parked" ), "0" ) == "0")?false:true;
				TRACE(2, wxString::Format("Loaded config for %s position x:%d y:%d  size x:%d y:%d", name, consolePosition.x, consolePosition.y, consoleSize.x, consoleSize.y));
				// from V2 positions have been saved relative to frame
				Console* newConsole = new Console(m_parent_window , name, consolePosition, consoleSize, dialogPosition, alertPosition, fileString, autoRun,  welcome, parked);
				newConsole->setup(welcome, fileString, autoRun);
				// constructor should have position console but does not seem to work on Hi Res display so force it
				newConsole->Move(newConsole->FromDIP(consolePosition));
				newConsole->SetSize(newConsole->FromDIP(consoleSize));
				TRACE(2, wxString::Format("Post-construction  %s->Move x:%d y:%d", name, consolePosition.x, consolePosition.y));
				newConsole->m_remembered = pConf->Read ( name + _T ( ":_remember" ), _T(""));
				// set up the locations
				bool set = (pConf->Read ( name + _T ( ":UnparkedLocationSet" ), "0" ) == "1")? true:false;
				if (set){  // set up unparked location               
					wxPoint position; wxSize size;
					position.x =  pConf->Read ( name + _T ( ":UnparkedLocationPosX" ), 20L );
					position.y =  pConf->Read ( name + _T ( ":UnparkedLocationPosY" ), 20L );
					size.x =  pConf->Read ( name + _T ( ":UnparkedLocationSizeX" ), 20L );
					size.y =  pConf->Read ( name + _T ( ":UnparkedLocationSizeY" ), 20L );
					position = newConsole->FromDIP(position);
					size = newConsole->FromDIP(size);
					newConsole->m_notParkedLocation.position = position;
					newConsole->m_notParkedLocation.size = size;
					newConsole->m_notParkedLocation.set = set;
					}
				set = (pConf->Read ( name + _T ( ":ParkedLocationSet" ), "0" ) == "1")? true:false;	
				if (set){	// set up parked location
					wxPoint position; wxSize size;
					position.x =  pConf->Read ( name + _T ( ":ParkedLocationPosX" ), 20L );
					position.y =  pConf->Read ( name + _T ( ":ParkedLocationPosY" ), 20L );
					size.x =  pConf->Read ( name + _T ( ":ParkedLocationSizeX" ), 20L );
					size.y =  pConf->Read ( name + _T ( ":ParkedLocationSizeY" ), 20L );
					position = newConsole->FromDIP(position);
					size = newConsole->FromDIP(size);
					newConsole->m_parkedLocation.position = position;
					newConsole->m_parkedLocation.size = size;
					newConsole->m_parkedLocation.set = set;                    	
					}
				newConsole->m_parked = parked;
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
/*
        if (!configUptoDate){	// read config was from old place
        	pConf->DeleteGroup ( _T ( "/Settings/JavaScript_pi" ) );	// delete the old one
        	pConf->SetPath (configSection);	// the new grouping
        	pConf->Flush();	// make sure it gets into new location
        	} 
*/   
        return true;
    }
    else {
    	wxLogMessage("JavaScript_pi->LoadConfig unable to create pConf*");
        return false;
        }
    return true;
}

bool JavaScript_pi::SaveConfig(void)
{
    wxFileConfig *pConf = (wxFileConfig *)m_pconfig;
    Console *pConsole;
    wxPoint screenToFrame(wxPoint pos);

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
        pConf->Write ( _T ( "RememberToggle" ), mRememberToggleStatus?"1":"0" );
        if (mRememberToggleStatus) pConf->Write ( _T ( "ShowingConsoles" ), mShowingConsoles?"1":"0" );
        pConf->Write ( _T ( "FloatOnParent" ), m_floatOnParent?"1":"0" );
        
        // now to save the recent files list - if any
        if (recentFiles.GetCount() > 0){
			wxString recents;
			for (unsigned int i = 0; i < recentFiles.GetCount(); i++){
				recents += recentFiles.Item(i) + ";";        	
				}
			recents = recents.BeforeLast(wxString(";").Last());	// drop last :
			pConf->Write (_T ("Recents"),  recents);
			}
        
        // and the favourite files list - if any
        if (favouriteFiles.GetCount() > 0){
            wxString favourites;
            for (unsigned int i = 0; i < favouriteFiles.GetCount(); i++){
                favourites += favouriteFiles.Item(i) + FS;
                }
            favourites = favourites.BeforeLast(wxString(FS).Last());    // drop last :
            pConf->Write (_T ("Favourites"),  favourites);
            }
            
        //save custom parking config, if any
        if (m_parkingBespoke){
        	pConf->Write (nameColon + _T ( "ParkingBespoke" ),   1 );
        	pConf->Write (nameColon + _T ( "ParkingStub" ),   m_parkingStub);
         	pConf->Write (nameColon + _T ( "ParkingLevel" ),   m_parkingLevel);
         	pConf->Write (nameColon + _T ( "ParkingFirstX" ),   m_parkFirstX);
         	pConf->Write (nameColon + _T ( "ParkingSep" ),   m_parkSep);  	
        	}

        for (pConsole = pJavaScript_pi->mpFirstConsole; pConsole != nullptr; pConsole = pConsole->mpNextConsole){
        	extern Console* pTestConsole1;
        	extern Console* pTestConsole2;
        	
        	if ((pConsole == pTestConsole1) || (pConsole == pTestConsole2)) continue;	// do not save any test consoles
        	// v2 positions now saved relative to frame
            name = pConsole->mConsoleName;
            nameColon = name + ":";
            // will save in DIP
            consoleNames += ((pConsole == pJavaScript_pi->mpFirstConsole)? "":":") + name;
// #if SCREEN_RESOLUTION_AVAILABLE
            wxPoint consolePosition = pConsole->ToDIP(pConsole->GetPosition());
            wxSize  consoleSize = pConsole->ToDIP(pConsole->GetSize());
/*
#else
			wxPoint consolePosition = pConsole->GetPosition();
			wxSize  consoleSize = pConsole->GetSize();
#endif
*/
            if (pConsole->mDialog.pdialog != nullptr) pConsole->clearDialog();
            wxPoint dialogPosition = pConsole->mDialog.position;	// already DIP
            if (pConsole->mAlert.palert != nullptr) pConsole->clearAlert();
            wxPoint alertPosition = pConsole->mAlert.position;	// already DIP
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
            pConf->Write (nameColon + _T ( "_remember" ),  pConsole->m_remembered);
            // now the locations
            bool set = pConsole->m_notParkedLocation.set;
            wxPoint position = pConsole->m_notParkedLocation.position;
            wxSize size = pConsole->m_notParkedLocation.size;
//#if SCREEN_RESOLUTION_AVAILABLE
			position =  pConsole->ToDIP(position);
			size =  pConsole->ToDIP(size);
//#endif           
            pConf->Write (nameColon + _T ( "UnparkedLocationSet" ),  set?"1":"0");
            pConf->Write (nameColon + _T ( "UnparkedLocationPosX" ),  position.x);
            pConf->Write (nameColon + _T ( "UnparkedLocationPosY" ),  position.y);
            pConf->Write (nameColon + _T ( "UnparkedLocationSizeX" ),  size.x);
            pConf->Write (nameColon + _T ( "UnparkedLocationSizeY" ),  size.y);
            set = pConsole->m_parkedLocation.set;
            position = pConsole->m_parkedLocation.position;
            size = pConsole->m_parkedLocation.size;
//#if SCREEN_RESOLUTION_AVAILABLE
			position =  pConsole->ToDIP(position);
			size =  pConsole->ToDIP(size);
//#endif 
            pConf->Write (nameColon + _T ( "ParkedLocationSet" ),  set?"1":"0");
            pConf->Write (nameColon + _T ( "ParkedLocationPosX" ),  position.x);
            pConf->Write (nameColon + _T ( "ParkedLocationPosY" ),  position.y);
            pConf->Write (nameColon + _T ( "ParkedLocationSizeX" ),  size.x);
            pConf->Write (nameColon + _T ( "ParkedLocationSizeY" ),  size.y);            
            }
        pConf->Write (_T ("Consoles"),  consoleNames);
        return true;
    }
    else
        return false;
}

void JavaScript_pi::SetNMEASentence(wxString &sentence)
{    // NMEA sentence received
	wxString NMEAchecksum(wxString sentence);
    wxString thisFunction, checksum, correctChecksum;
    size_t starPos;
    bool OK {false};
    Completions outcome;
    duk_context *ctx;
    wxUniChar star = '*';
    Console *m_pConsole;
    void JSduk_start_exec_timeout(Console);
    void  JSduk_clear_exec_timeout(Console);
    bool haveDoneChecksum = false;

    for (m_pConsole = pJavaScript_pi->mpFirstConsole; m_pConsole != nullptr; m_pConsole = m_pConsole->mpNextConsole){    // work through all consoles
        if (m_pConsole == nullptr) continue;  // ignore if not ready
        if (!m_pConsole->isWaiting()) continue;
        thisFunction = m_pConsole->m_NMEAmessageFunction;
        if (thisFunction == wxEmptyString) continue;	// this one not waiting for us
        if (m_pConsole->mJSrunning){
            m_pConsole->message(STYLE_RED, "NMEA callback while JS active - ignored\n");
            return;
            }
        if (!haveDoneChecksum){
			// check checksum and set OK accordingly
			// we do this in the console loop to avoid when none waiting for us
			sentence.Trim();
			starPos = sentence.find(star); // position of *
			if (starPos != wxNOT_FOUND){ // yes there is one
				checksum = sentence.Mid(starPos + 1, 2);
				sentence = sentence.SubString(0, starPos-1); // truncate at * onwards
				}
			correctChecksum = NMEAchecksum(sentence);
			OK = (checksum == correctChecksum) ? true : false;
			sentence = sentence.BeforeFirst('*');   // drop * onwards
			haveDoneChecksum = true;
        	}
        ctx = m_pConsole->mpCtx;
		duk_push_object(ctx);
		duk_push_string(ctx, sentence.c_str());
		duk_put_prop_literal(ctx, -2, "value");
		duk_push_boolean(ctx, OK);
		duk_put_prop_literal(ctx, -2, "OK");
		if (!m_pConsole->m_NMEApersistance) m_pConsole->m_NMEAmessageFunction = wxEmptyString;	// only once
		outcome = m_pConsole->executeFunction(thisFunction);		
		if (!m_pConsole->isBusy()) m_pConsole->wrapUp(outcome);
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

void JavaScript_pi::SetCurrentViewPort(PlugIn_ViewPort &vp){
	m_currentViewPort = vp;
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
                TRACE(3,"JavaScript plugin console " + pThisConsole->mConsoleName + " destroying ctx while emptying bin");
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
                        if (!pConsole->isBusy()) pConsole->wrapUp(outcome);
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
    TRACE(15,"Entered SetPlugin Message message_id: " + message_id + " message:" + message_body);
    if (message_id == "OpenCPN Config"){
        // capture this while we can
        TRACE(15, "Captured openCPNConfig");
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
            if (!m_pConsole->mMessages[index].persist)
            	m_pConsole->mMessages[index].functionName = wxEmptyString;  // do not call again
            TRACE(15, "About to process message for console " + m_pConsole->mConsoleName + " " + m_pConsole->consoleDump());
            if (!ctx) continue; // just in case
            duk_push_string(ctx, message_body.c_str());
            if (m_pConsole->mJSrunning){
                m_pConsole->message(STYLE_RED, "Message callback while JS active - ignored - message is:\n" + message_body + "\n");
                return;
                }
            TRACE(15, "Will execute function " /* + m_pConsole->dukDump() */);
            outcome = m_pConsole->executeFunction(thisFunction);
            TRACE(15, "Have processed message for console " + m_pConsole->mConsoleName + " and result was " +  statusesToString(m_pConsole->mStatus.set(outcome)));
            if (!m_pConsole->isBusy()) m_pConsole->wrapUp(outcome);
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
        pTools->fixForScreenRes();
        // position it top right in display
        wxWindow* frame = GetOCPNCanvasWindow()->GetParent();
        wxDisplay display(wxDisplay::GetFromWindow(frame));
		wxRect screen = display.GetClientArea();
        wxSize toolsSize = pTools->FromDIP(pTools->GetSize());
        TRACE(99, wxString::Format("Screen x: %i y: %i width: %i, toolsSize x: %i y: %i", screen.x, screen.y, screen.width, toolsSize.x, toolsSize.y)) ;    	
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

