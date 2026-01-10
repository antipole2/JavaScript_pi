/**************************************************************************
* Project:  OpenCPN
* Purpose:  JavaScript Plugin
* Author:   Tony Voss 16/05/2020
*
* Copyright Ⓒ 2025 by Tony Voss
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

extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr){
    return new JavaScript_pi(ppimgr);
	}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p){
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
:opencpn_plugin_120 (ppimgr)  // was 18
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

    //    Get a pointer to the opencpn configuration object
    m_pconfig = GetOCPNConfigObject();
    if (m_pconfig == nullptr){
    	wxLogMessage("JavaScript_pi->Init() unable to find ConfigObject");
    	return 0;
    	}

    //    And load the configuration items
    LoadConfig();

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
    
    reviewParking();
    
    // look out to see if we need to automatically run any console
    for (auto* pConsole : pJavaScript_pi->m_consoles){
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
    wxLogMessage("JavaScript_pi->DeInit() entered");
    if (pTools != nullptr) {
        TRACE(3,"JavaScript plugin DeInit destroying tools pane");
		pTools->cleanupParking();
        try{
        	delete pTools;
        	}
        catch (int i){i = i++;}	// do nothing - just avoid throwing error and suppress warning
        pTools = nullptr;
        }
    SaveConfig();
  
	// ensure all console stuff cleared away and wait for wxWidgets to complete
    for (auto* c : m_consoles) if (c) delete c; //c->Destroy();
    m_consoles.clear();
	wxTheApp->ProcessPendingEvents();

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

int JavaScript_pi::GetAPIVersionMajor(){
	return atoi(API_VERSION);
	}

int JavaScript_pi::GetAPIVersionMinor(){
    std::string v(API_VERSION);
    size_t dotpos = v.find('.');
    return atoi(v.substr(dotpos + 1).c_str());
	}

int JavaScript_pi::GetPlugInVersionMajor(){
    return PLUGIN_VERSION_MAJOR;
	}

int JavaScript_pi::GetPlugInVersionMinor(){
    return PLUGIN_VERSION_MINOR;
	}

int JavaScript_pi::GetPlugInVersionPatch(){
    return PLUGIN_VERSION_PATCH;
	}

wxBitmap *JavaScript_pi::GetPlugInBitmap(){
    return &m_panelBitmap;
	}

wxString JavaScript_pi::GetCommonName(){
    return "javascript";
	}

wxString JavaScript_pi::GetShortDescription(){
    return "Run JavaScripts";
	}

wxString JavaScript_pi::GetLongDescription(){
    return "Run JavaScripts and interact with OpenCPN\n\
See User Guide at https://opencpn-manuals.github.io/main/javascript/index.html\n\
and fuller details in the plugin Help ";
	}

int JavaScript_pi::GetToolbarToolCount(void){
    return 1;
	}

void JavaScript_pi::SetColorScheme(PI_ColorScheme cs){
	}

void JavaScript_pi::OnToolbarToolCallback(int id){
    void JSlexit(wxStyledTextCtrl* pane);
    void fatal_error_handler(void *udata, const char *msg);

    TRACE(34,wxString::Format("JavaScript_pi->OnToolbarToolCallback() entered with mShowingConsoles  %s", (mShowingConsoles ? "true":"false")));
    mShowingConsoles = !mShowingConsoles;   // toggle console display state
	if (mShowingConsoles & m_showHelp){
			ShowTools(m_parent_window, -1);	// show them the help page
			m_showHelp = false;
			}
    for (auto* pConsole : pJavaScript_pi->m_consoles){
        if (mShowingConsoles)   pConsole->Show();
        else pConsole->Hide();
        }
    // Toggle is handled by the toolbar but we must keep plugin manager b_toggle updated
    // to actual status to ensure correct status upon toolbar rebuild
    SetToolbarItemState( m_leftclick_tool_id, mShowingConsoles );
    RequestRefresh(m_parent_window); // refresh main window
    }

bool JavaScript_pi::LoadConfig(void){
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
			if ((versionMajor < PLUGIN_VERSION_MAJOR ) || ((versionMajor <= PLUGIN_VERSION_MAJOR ) && (versionMinor < PLUGIN_VERSION_MINOR))){
				// plugin has ben updated
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
		else mShowingConsoles = true;
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
 		wxString consoles = pConf->Read ( _T ( "Consoles" ), _T("") );
		if (consoles == wxEmptyString){ // no consoles configured
			Console* newConsole = new Console(m_parent_window, "JavaScript");
			newConsole->setup(welcome);
			m_consoles.push_back(newConsole);
			}
		else {
			wxStringTokenizer tkz(consoles, ":");
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
				Console* newConsole = new Console(m_parent_window , name);
				newConsole->setup(welcome, fileString, autoRun);
				newConsole->SetPosition(newConsole->FromDIP(consolePosition));
				newConsole->SetSize(newConsole->FromDIP(consoleSize));
				newConsole->m_remembered = pConf->Read ( name + _T ( ":_remember" ), _T("{}"));
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
				m_consoles.push_back(newConsole);
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
        return true;
	    }
    else {
    	wxLogMessage("JavaScript_pi->LoadConfig unable to create pConf*");
        return false;
        }
    return true;
	}

bool JavaScript_pi::SaveConfig(void){
    wxFileConfig *pConf = (wxFileConfig *)m_pconfig;
    TRACE(3,"JavaScript_pi->SaveConfig() entered");
    if(pConf){
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
		size_t index = 0;
	    for (auto* pConsole : pJavaScript_pi->m_consoles){
            name = pConsole->mConsoleName;
            nameColon = name + ":";
            // will save in DIP
            consoleNames += ((index++ == 0)? "":":") + name;
            wxPoint consolePosition = pConsole->ToDIP(pConsole->GetPosition());
            wxSize  consoleSize = pConsole->ToDIP(pConsole->GetSize());
            wxPoint dialogPosition = pConsole->mDialogPosition;	// already DIP
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
			position =  pConsole->ToDIP(position);
			size =  pConsole->ToDIP(size);        
            pConf->Write (nameColon + _T ( "UnparkedLocationSet" ),  set?"1":"0");
            pConf->Write (nameColon + _T ( "UnparkedLocationPosX" ),  position.x);
            pConf->Write (nameColon + _T ( "UnparkedLocationPosY" ),  position.y);
            pConf->Write (nameColon + _T ( "UnparkedLocationSizeX" ),  size.x);
            pConf->Write (nameColon + _T ( "UnparkedLocationSizeY" ),  size.y);
            set = pConsole->m_parkedLocation.set;
            position = pConsole->m_parkedLocation.position;
            size = pConsole->m_parkedLocation.size;
			position =  pConsole->ToDIP(position);
			size =  pConsole->ToDIP(size);
            pConf->Write (nameColon + _T ( "ParkedLocationSet" ),  set?"1":"0");
            pConf->Write (nameColon + _T ( "ParkedLocationPosX" ),  position.x);
            pConf->Write (nameColon + _T ( "ParkedLocationPosY" ),  position.y);
            pConf->Write (nameColon + _T ( "ParkedLocationSizeX" ),  size.x);
            pConf->Write (nameColon + _T ( "ParkedLocationSizeY" ),  size.y);            
            }
        pConf->Write (_T ("Consoles"),  consoleNames);
        return true;
    	}
    else return false;
	}

void JavaScript_pi::SetNMEASentence(wxString &sentence){    // NMEA sentence received
	wxString NMEAchecksum(wxString sentence);
	bool checkNMEAsum(wxString sentence);
    wxString thisFunction, checksum, correctChecksum;
    bool OK {false};
    void JSduk_start_exec_timeout(Console);
    void  JSduk_clear_exec_timeout(Console);
    if (!m_SetActive.test(CB_N0183)) return;	// nothing to do so do not hang around
    m_SetActive.set(CB_N0183, false);	// this will be set true if anything waiting
    bool haveDoneChecksum = false;
    for (auto* pConsole : pJavaScript_pi->m_consoles){
        if (pConsole == nullptr) continue;  // ignore if not ready
        if (!pConsole->isWaiting()) continue;
        for (unsigned int i = 0; i < pConsole->mCallbacks.size(); i++){ // work through all callback entries
        	std::shared_ptr<callbackEntry> pEntry = pConsole->mCallbacks[i];
        	if ((pEntry->type == CB_N0183) && (pEntry->_IDENT == wxEmptyString)){ // this one is for us
        		m_SetActive.set(CB_N0183, true);
        		if (!haveDoneChecksum) { // first time with this messageIndex
        			OK = checkNMEAsum(sentence);
					size_t starPos = sentence.find("*");
					sentence = sentence.SubString(0, starPos-1); // truncate at * onwards
					haveDoneChecksum = true;
        			}
        		duk_context *ctx = pConsole->mpCtx;
				duk_push_object(ctx);
				duk_push_string(ctx, sentence.c_str());
					duk_put_prop_literal(ctx, -2, "value");
				duk_push_boolean(ctx, OK);
					duk_put_prop_literal(ctx, -2, "OK");
				if (!pEntry->persistant) pConsole->mCallbacks.erase(pConsole->mCallbacks.begin() + i);
				// the above simple remove is OK as this type of entry does not have dependents	
				Completions outcome = pConsole->executeCallableNargs(pEntry->func_heapptr, 1);	// only one object on stack
				if (!pConsole->isBusy()) pConsole->wrapUp(outcome);
        		}
        	}
        }   // end for this console
    }
    
void JavaScript_pi::SetAISSentence(wxString &sentence) {    // AIS sentence received
	wxString NMEAchecksum(wxString sentence);
	bool checkNMEAsum(wxString sentence);
	TRACE(1001, "SetAISSentence ");
    if (!m_SetActive.test(CB_AIS)) return;	// nothing to do so do not hang around
    TRACE(1001, "SetAISSentence Will search for customers");
    m_SetActive.set(CB_AIS, false);	// this will be set true if anything waiting
    bool haveDoneChecksum = false;
    bool OK = false;
    for (auto* pConsole : pJavaScript_pi->m_consoles){
		if (pConsole == nullptr) continue;  // ignore if not ready
		if (!pConsole->isWaiting()) continue;
		for (size_t i = 0; i < pConsole->mCallbacks.size(); ++i) {
			std::shared_ptr<callbackEntry> pEntry	=  pConsole->mCallbacks[i];
			if (pEntry->type != CB_AIS) continue;
			if (!pEntry->persistant)  pConsole->mCallbacks.erase( pConsole->mCallbacks.begin() + i);
			// the above simple erase is OK as this type of entry does not have dependents	
			TRACE(1001, "SetAISSentence have customer for " + sentence);
			m_SetActive.set(CB_AIS, true);
			if (!haveDoneChecksum) { // first time with this messageIndex
				OK = checkNMEAsum(sentence);
				size_t starPos = sentence.find("*");
				sentence = sentence.SubString(0, starPos-1); // truncate at * onwards
				haveDoneChecksum = true;
				TRACE(1001, "Set checksum OK");
				}
       		duk_context *ctx = pConsole->mpCtx;
			duk_push_object(ctx);
			duk_push_string(ctx, sentence.c_str());
				duk_put_prop_literal(ctx, -2, "value");
			duk_push_boolean(ctx, OK);
				duk_put_prop_literal(ctx, -2, "OK");
			Completions outcome = pConsole->executeCallableNargs(pEntry->func_heapptr, 1);	// only one object on stack
			if (!pConsole->isBusy()) pConsole->wrapUp(outcome);
			}
		}       	
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
        
void JavaScript_pi::OnContextMenuItemCallbackExt(int menuID, std::string identifier, std::string objectType, double lat, double lon){
//    Console* pConsole;
    std::shared_ptr<callbackEntry> pEntry;
    Completions outcome;
    TRACE(655, wxString::Format("OnContextMenuItemCallbackExt with menuID:%i, identifier:%s, objectType:%s, lat:%f, lon:%f",
    	menuID, identifier, objectType, lat,lon));
	for (auto* pConsole : pJavaScript_pi->m_consoles){
        if (!pConsole->isWaiting()) continue;  // ignore if we are not waiting on something
        TRACE(655, wxString::Format("In OnContextMenuCallback loop and console %s and MenuID %d", pConsole->mConsoleName, menuID));
		pEntry = pConsole->getCallbackEntry(menuID, false);
		if (!pEntry) continue;	// not found in this console
		if (pEntry->type == CB_CONTEXT_SUBMENU && !pEntry->persistant){
			// Have just cleared out a sub-menu.  We need to clear down the whole structure
			std::shared_ptr<callbackEntry> waste = pConsole->getCallbackEntry(pEntry->_SUBMENU_PARENT_ID, true); // force-clear the owning entry
			}
		// we return a position, supplemented with the info from when it was set up.
		duk_context *ctx = pConsole->mpCtx;
		duk_push_object(ctx);
		duk_push_number(ctx, lat);
		duk_put_prop_literal(ctx, -2, "latitude");
		duk_push_number(ctx,lon);
		duk_put_prop_literal(ctx, -2, "longitude");
		duk_push_string(ctx, (wxString)objectType);
		duk_put_prop_literal(ctx, -2, "objecttype");
		if ((objectType == "Waypoint") || (objectType == "Route")|| (objectType == "Track")){
			duk_push_string(ctx, (wxString)identifier);
			duk_put_prop_literal(ctx, -2, "GUID");
			}
		else if (objectType == "AIS") {
			duk_push_string(ctx, (wxString)identifier);
			duk_put_prop_literal(ctx, -2, "MMSI");
			}
		duk_push_string(ctx, pEntry->_MENU_NAME);
		duk_put_prop_literal(ctx, -2, "menuName");
		duk_push_string(ctx, pEntry->parameter);
		duk_put_prop_literal(ctx, -2, "info");
		wxString dukdump_to_string(duk_context* ctx);
		TRACE(655, wxString::Format("OnContextMenuItemCallbackExt stack\n%s", dukdump_to_string(ctx)));
		void* heapptr = pEntry->func_heapptr;
		outcome = pConsole->executeCallableNargs(heapptr, 1);
		pConsole->mWaitingCached = false;
		if (!pConsole->isBusy()) pConsole->wrapUp(outcome);
        }
    }

void JavaScript_pi::SetPluginMessage(wxString &message_id, wxString &message_body) {
	// do we have it already?
	auto it = std::find(m_messages.begin(), m_messages.end(), message_id);
	if (it == m_messages.end()) {	// not matched, so add it in correct place
		auto it = std::lower_bound(m_messages.begin(), m_messages.end(), message_id);
		m_messages.insert(it, message_id);
		}
	m_lastMessage = message_body;	// remember this message
    if (message_id == "OpenCPN Config"){
        // capture this while we can
        TRACE(15, "Captured openCPNConfig");
        openCPNConfig = message_body;
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
    	TRACE(60, wxString::Format("About to create Tools with page: %i", page));
    	int x, y;
        pTools = new ToolsClass(m_parent_window, wxID_ANY /*, "JavaScript Tools" */);
        TRACE(60, wxString::Format("Have created Tools with page: %i", page));
        pTools->fixForScreenRes();
        // position it top right in display
        wxWindow* frame = GetOCPNCanvasWindow()->GetParent();
        wxDisplay display(wxDisplay::GetFromWindow(frame));
		wxRect screen = display.GetClientArea();
        wxSize toolsSize = pTools->FromDIP(pTools->GetSize());
        TRACE(60, wxString::Format("Screen x: %i y: %i width: %i, toolsSize x: %i y: %i", screen.x, screen.y, screen.width, toolsSize.x, toolsSize.y)) ;    	
        x = screen.x + screen.width - toolsSize.x;
        y = screen.y;
        pTools->SetPosition(wxPoint(x,y));
        }
return;
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


