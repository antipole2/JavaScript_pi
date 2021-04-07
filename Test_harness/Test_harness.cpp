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
/*
Instructions for running under Xcode

(1) Fix up the plugin name whereever it occurs in this plugin to match what you are testing
(2) create a new Target for MacOS of type Command Line Tool and confirming C++ as the language
(3) In the Build phase settings, remove the main.cpp file that was put there when the traget was created and include this code and all plugin sources.
(4) set the header search path to include wxWidgets/include, which contains the wx folder of include files.  In my case this is 
    /Users/Tony/OpenCPN_project/wxWidgets-3.1.2/include
(5) Add the required library search path for wxWidgets, which in my case is case is
    /Users/Tony/OpenCPN_project/wxWidgets-3.1.2/build-release/lib
(6) In Apple Clang - Preprocessing macros, set
    define TIXML_USE_STL
                If this is not set, the header processing attempts to incorrectly include tinystr.h
    define IN_HARNESS
                This can be used to omit compiling any code that will not run under the harness, e.g. the creation of the plugin icons
(7) Extract the compiler build flags with wx-config —cppflags and insert these into the Xcode Other C++ Flags setting.
    This is added to ensure that compiled code is compatible
(8) Extract the library linking flags with wx-config — libs and add the specified frameworks and libraries into the Build Phase settings.
    Note that under Catalina at least, there is no System framework to be added
 
 When the harness runs it  opens the dialogue panel automatically or it can be opened with the Test option in the File menu of the main frame.
 Other invocations could be added to the test frame menus.
 NB To get the test frame menus to be active, Xcode must be foregrounded before then activating the test frame.
*/

// For compilers that support precompilation, includes "wx/wx.h".

#include <wx/wxprec.h>
#include <wx/wx.h>
#include "ocpn_plugin.h"
#include "JavaScriptgui_impl.h"
#include "JavaScript_pi.h"
#include "JavaScriptgui.h"
#include <wx/listimpl.cpp>
#include "wx/config.h"

// #include "trace.h"
// Configuration        ************
bool activeImmediately {true};
bool loadFromFile = false;
wxString configFileName = _("OpenCPN_project/JavaScript-project/testConfig.txt");    // set to existing file.  Can be OCPN config file

// Manual configuration if loadFromFile == false
 wxString consoleNames[] {_("One"), _("Two"), _("Three"), _("Four")};

// end of configuration ************

class TestHarness: public wxApp
{
public:
    virtual bool OnInit();
};
class TestFrame: public wxFrame
{
public:
    TestFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
private:
    void OnTest(wxCommandEvent& event);
    void OnActivate(wxCommandEvent& event);
    void OnDeactivate(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    wxDECLARE_EVENT_TABLE();
};

extern JavaScript_pi *pJavaScript_pi;


enum
{
    ID_Test = 1,
    ID_Activate,
    ID_Deactivate
};

TestFrame *frame{};
wxString runJS(void);   // test

TestFrame::TestFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxFrame(NULL, wxID_ANY, title, pos, size)
{
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(ID_Test, "&Test...\tCtrl-H",
                     "Harness says Help string shown in status bar for this menu item");
    menuFile->Append(ID_Activate, "&Activate\tCtrl-A",
                     "Simulates Activating plaugin");
    menuFile->Append(ID_Deactivate, "&Dactivate\tCtrl-D",
                     "Simulates Deactivating plaugin");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append( menuFile, "&File" );
    menuBar->Append( menuHelp, "&Help" );
    SetMenuBar( menuBar );
    CreateStatusBar();
    SetStatusText( "Harness says Getting JavaScript into OpenCPN" );
}

void TestFrame::OnQuit(wxCommandEvent& event)
{
    extern JavaScript_pi *pJavaScript_pi;
    OnDeactivate(event);
    delete pJavaScript_pi;
    Close( true );
}
void TestFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox( "This is a test frame to explore wxForms",
                  "About this test frame", wxOK | wxICON_INFORMATION );
}
void TestFrame::OnTest(wxCommandEvent& event)
{
    extern JavaScript_pi *pJavaScript_pi;
    cout << "Entered Harness OnTest\n";
    // plugin has not been initialised, so we need to fix it up here:
    pJavaScript_pi->m_parent_window = frame;
     
    // Invoke the plugin callback as if the toolbar button had been clicked
    pJavaScript_pi->OnToolbarToolCallback(1);
}

int * ppimgr;

wxBEGIN_EVENT_TABLE(TestFrame, wxFrame)
    EVT_MENU(ID_Test,   TestFrame::OnTest)
    EVT_MENU(ID_Activate,   TestFrame::OnActivate)
    EVT_MENU(ID_Deactivate,   TestFrame::OnDeactivate)
    EVT_MENU(wxID_EXIT,  TestFrame::OnQuit)
    EVT_MENU(wxID_ABOUT, TestFrame::OnAbout)
wxEND_EVENT_TABLE()
wxIMPLEMENT_APP(TestHarness);

wxFileConfig *settingsFile = new wxFileConfig(_("JavaScript_pi"), wxEmptyString, wxEmptyString, loadFromFile?configFileName:_(""), wxCONFIG_USE_GLOBAL_FILE);

void activate(){
    extern JavaScript_pi *pJavaScript_pi;
    Console *pConsole;
    int numberOfConsoles {2};
    if (pJavaScript_pi->mpFirstConsole){
        cout << "Entered Harness activate - already active\n";
        return;
        }
    cout << "Entered Harness activate - activating\n";
    wxPoint consolePosition = wxPoint(80, 50);
    for (int i = 0; i < numberOfConsoles; i++){
        pConsole = new Console(pJavaScript_pi->m_parent_window, consoleNames[i], consolePosition);
        consolePosition.x += 200; consolePosition.y += 100;
        pConsole->Show();
        }
    pJavaScript_pi->mCurrentDirectory = _("/Users/Tony/OpenCPN_project/JavaScript-project/JavaScript_pi/Tests_scripts");

    // Invoke the plugin callback as if the toolbar button had been clicked
    pJavaScript_pi->mShowingConsoles = false;   // next will toggle to show
    pJavaScript_pi->OnToolbarToolCallback(1);
    }

void TestFrame::OnActivate(wxCommandEvent& event)
{
    activate();
    }

bool TestHarness::OnInit()
{
    extern JavaScript_pi *pJavaScript_pi;
//    void JSlexit(wxStyledTextCtrl* pane);
    cout << "Entered harness On_Init\n";
 
    frame = new TestFrame( "Test frame", wxPoint(50, 50), wxSize(450, 340) );
    frame->Show( true );
    // create the plugin
    pJavaScript_pi = new JavaScript_pi(ppimgr);
    pJavaScript_pi->m_parent_window = frame;
    pJavaScript_pi->Init();
    if (activeImmediately) activate();
    return true;
    }

void TestFrame::OnDeactivate(wxCommandEvent& event)
{
    extern JavaScript_pi *pJavaScript_pi;
    pJavaScript_pi->DeInit();
    }



// Replace neccessary OPCN functions with make-do's —————————————————————————————————————————————————
wxWindow *GetOCPNCanvasWindow(){
    return(frame);    // point to test frame instead of OPCN canvas
}
// void initialize_images(){return;}       // not bothering with images
bool AddLocaleCatalog( wxString catalog ){return(false);};  // not doing locales
wxFileConfig *GetOCPNConfigObject(){return(settingsFile);}
void loadconfig(){return;}      // don't load config

// a GPX sentence to return
char const* gpxSample = "<?xml version=\"1.0\"?>\n<gpx version=\"1.1\" creator=\"OpenCPN\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://www.topografix.com/GPX/1/1\" xmlns:gpxx=\"http://www.garmin.com/xmlschemas/GpxExtensions/v3\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\" xmlns:opencpn=\"http://www.opencpn.org\">\n  <wpt lat=\"50.689863989\" lon=\"-1.719423187\">\n    <time>2020-06-09T10:19:03Z</time>\n    <name>002</name>\n    <sym>circle</sym>\n    <type>WPT</type>\n    <extensions>\n      <opencpn:guid>1cb47347-15f8-423e-9f9e-1bd915178835</opencpn:guid>\n      <opencpn:viz_name>1</opencpn:viz_name>\n      <opencpn:auto_name>1</opencpn:auto_name>\n      <opencpn:arrival_radius>0.050</opencpn:arrival_radius>\n      <opencpn:waypoint_range_rings visible=\"false\" number=\"0\" step=\"1\" units=\"0\" colour=\"#FF0000\" />\n      <opencpn:scale_min_max UseScale=\"false\" ScaleMin=\"2147483646\" ScaleMax=\"0\" />\n    </extensions>\n  </wpt>\n</gpx>";
bool GetActiveRoutepointGPX( char *gpx, unsigned int length ) {
    int i;
    for (i = 0; gpxSample[i] != '\0'; i++) gpx[i] = gpxSample[i];
    gpx[i] = '\0';
    return true;};

// We do not need to insert the plugin tools
int InsertPlugInTool(wxString label, wxBitmap *bitmap, wxBitmap *bmpRollover, wxItemKind kind,
wxString shortHelp, wxString longHelp, wxObject *clientData, int position,
int tool_sel, opencpn_plugin *pplugin){return(1);}

int InsertPlugInToolSVG(wxString label, wxBitmap *bitmap, wxBitmap *bmpRollover, wxItemKind kind,
wxString shortHelp, wxString longHelp, wxObject *clientData, int position,
int tool_sel, opencpn_plugin *pplugin){return(1);}


// dummy out OPCN functions not available to test harness

void DimeWindow(wxWindow *){std::cout << "DimeWindow called\n";}
wxWindow canvasA;
wxString sharedLocation = _("/Applications/OpenCPN.app/Contents/SharedSupport/");
wxString *GetpSharedDataLocation(){ return(& sharedLocation);}
void RequestRefresh(wxWindow *){return;}  // ignore this until we find we need it
void SetToolbarItemState(int item, bool toggle){return;}  // and ignore this  until we need it


// API simulators
void PushNMEABuffer(wxString NMEAsentence){
    cout << "Harness says Sent NMEA: " << NMEAsentence;
}

void SendPluginMessage(wxString message_id, wxString message_body){
    cout << "Harness says Sent message_ID: " << message_id << " message_body:" << message_body << "\n";
}

// The following needs to be here to keep the compiler and linker happy - this cloned from OpenCPN itself with a fix or two

//-----------------------------------------------------------------------------------------
//    The opencpn_plugin base class implementation
//-----------------------------------------------------------------------------------------

opencpn_plugin::~opencpn_plugin()
{}

int opencpn_plugin::Init(void)
{
    return 0;
}


bool opencpn_plugin::DeInit(void)
{
    return true;
}

int opencpn_plugin::GetAPIVersionMajor()
{
    return 1;
}

int opencpn_plugin::GetAPIVersionMinor()
{
    return 14;
}

int opencpn_plugin::GetPlugInVersionMajor()
{
    return 1;
}

int opencpn_plugin::GetPlugInVersionMinor()
{
    return 0;
}

wxBitmap *opencpn_plugin::GetPlugInBitmap()
{
//    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();  Replaced with dummy by Tony
//  return new wxBitmap(style->GetIcon( _T("default_pi") ) );
    return new wxBitmap();                                          // nonesense to keep compiler happy
}

wxString opencpn_plugin::GetCommonName()
{
    return _T("BaseClassCommonName");
}

wxString opencpn_plugin::GetShortDescription()
{
    return _T("OpenCPN PlugIn Base Class");
}

wxString opencpn_plugin::GetLongDescription()
{
    return _T("OpenCPN PlugIn Base Class\n\
PlugInManager created this base class");
}


void opencpn_plugin::SetPositionFix(PlugIn_Position_Fix &pfix)
{}

void opencpn_plugin::SetNMEASentence(wxString &sentence)
{}

void opencpn_plugin::SetAISSentence(wxString &sentence)
{}

int opencpn_plugin::GetToolbarToolCount(void)
{
    return 0;
}

int opencpn_plugin::GetToolboxPanelCount(void)
{
    return 0;
}

wxString GetNewGUID()
{
    return _("dummy GUID");
}

wxArrayString GetWaypointGUIDArray(){
    return(wxArrayString());
}

PlugIn_Waypoint:: PlugIn_Waypoint(){
    this->m_GUID = "";
}
PlugIn_Waypoint:: ~PlugIn_Waypoint(){
    this->m_GUID = "";
}

PlugIn_Route:: PlugIn_Route(){
    this->m_GUID = "";
}
PlugIn_Route:: ~PlugIn_Route(){
    this->m_GUID = "";
}

bool AddSingleWaypoint( PlugIn_Waypoint *pwaypoint, bool b_permanent)
{
    return (true);
}

bool GetSingleWaypoint(wxString GUID,  PlugIn_Waypoint *pwaypoint){
    pwaypoint->m_lat = 60.5;
    pwaypoint->m_lon = -1.5;
    pwaypoint->m_MarkName = _("My synthetic mark");
    pwaypoint->m_GUID = _("the GUID string");
    pwaypoint->m_MarkDescription = _("This is the mark description");
    pwaypoint->m_IconName = _("TheIconName");
    pwaypoint->m_IsVisible = true;
    pwaypoint->m_CreateTime = wxDateTime::Now();
    return true;
}

bool UpdateSingleWaypoint(PlugIn_Waypoint *pwaypoint) {return true;}

bool DeleteSingleWaypoint( wxString &GUID )
{
    return (true);
}

ArrayOfPlugIn_AIS_Targets AIStargetArray;
ArrayOfPlugIn_AIS_Targets *GetAISTargetArray(void){
    PlugIn_AIS_Target target;
    wxString::iterator i;
    wxString name;
    
 //   if (AIStargetArray.GetCount() > 0) WX_CLEAR_ARRAY(AIStargetArray);
    
    target.MMSI = 123456789;
    target.Lon = 60;
    target.Lat = 1;
    AIStargetArray.Add(&target);
    
    target.MMSI = 987654321;
    target.Lon = 50;
    target.Lat = -1;
    AIStargetArray.Add(&target);
    
    return(&AIStargetArray);
    }

bool AddPlugInRoute( PlugIn_Route *proute, bool b_permanent) {return(true);}

bool UpdatePlugInRoute( PlugIn_Route *proute) {return(true);}

bool DeletePlugInRoute(wxString &GUID){return(true);}

PlugIn_Route *dummyRoute = new PlugIn_Route;

std::unique_ptr<PlugIn_Route> GetRoute_Plugin( const wxString& ){return(std::unique_ptr<PlugIn_Route>(dummyRoute));}

wxString GetSelectedWaypointGUID_plugin(){return( _("This is a pretend GUID"));}

wxString GetSelectedRouteGUID_plugin(){return( _("This is a pretend GUID"));}

void opencpn_plugin::SetupToolboxPanel(int page_sel, wxNotebook* pnotebook)
{}

void opencpn_plugin::OnCloseToolboxPanel(int page_sel, int ok_apply_cancel)
{}

void opencpn_plugin::ShowPreferencesDialog( wxWindow* parent )
{}

void opencpn_plugin::OnToolbarToolCallback(int id)
{}

void opencpn_plugin::OnContextMenuItemCallback(int id)
{}

bool opencpn_plugin::RenderOverlay(wxMemoryDC *dc, PlugIn_ViewPort *vp)
{
    return false;
}

void opencpn_plugin::SetCursorLatLon(double lat, double lon)
{}

void opencpn_plugin::SetCurrentViewPort(PlugIn_ViewPort &vp)
{}

void opencpn_plugin::SetDefaults(void)
{}

void opencpn_plugin::ProcessParentResize(int x, int y)
{}

void opencpn_plugin::SetColorScheme(PI_ColorScheme cs)
{}

void opencpn_plugin::UpdateAuiStatus(void)
{}

void DistanceBearingMercator_Plugin(double lat, double lon, double lat0, double lon0, double *bearing, double *distance){
    *bearing = 270; *distance = 20;
    return;
    }

void PositionBearingDistanceMercator_Plugin(double lat, double lon, double brg, double dist, double *dlat, double *dlon){
    *dlat = 55; *dlon = -1.5;
    return;
    }

double DistGreatCircle_Plugin(double slat, double slon, double dlat, double dlon){ return 1234;}

wxArrayString opencpn_plugin::GetDynamicChartClassNameArray()
{
    wxArrayString array;
    return array;
}

//    Opencpn_Plugin_17 Implementation
opencpn_plugin_17::opencpn_plugin_17(void *pmgr)
    : opencpn_plugin(pmgr)
{
}

opencpn_plugin_17::~opencpn_plugin_17(void)
{}


bool opencpn_plugin_18::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp)
{
    return false;
}

bool opencpn_plugin_18::RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp)
{
    return false;
}

void opencpn_plugin_17::SetPluginMessage(wxString &message_id, wxString &message_body)
{}

// added 14/6/2020 when upgrading to API 116
opencpn_plugin_18::opencpn_plugin_18(void *pmgr):opencpn_plugin(pmgr){}
opencpn_plugin_18::~opencpn_plugin_18(void){}
opencpn_plugin_19::opencpn_plugin_19(void *pmgr):opencpn_plugin_18(pmgr){}
opencpn_plugin_19::~opencpn_plugin_19(void){}
opencpn_plugin_110::opencpn_plugin_110(void *pmgr):opencpn_plugin_19(pmgr){}
opencpn_plugin_110::~opencpn_plugin_110(void){}
opencpn_plugin_111::opencpn_plugin_111(void *pmgr):opencpn_plugin_110(pmgr){}
opencpn_plugin_111::~opencpn_plugin_111(void){}
opencpn_plugin_112::opencpn_plugin_112(void *pmgr):opencpn_plugin_111(pmgr){}
opencpn_plugin_112::~opencpn_plugin_112(void){}
opencpn_plugin_113::opencpn_plugin_113(void *pmgr):opencpn_plugin_112(pmgr){}
opencpn_plugin_113::~opencpn_plugin_113(void){}
opencpn_plugin_114::opencpn_plugin_114(void *pmgr): opencpn_plugin_113(pmgr){}
opencpn_plugin_114::~opencpn_plugin_114(void){}
opencpn_plugin_115::opencpn_plugin_115(void *pmgr): opencpn_plugin_114(pmgr){}
opencpn_plugin_115::~opencpn_plugin_115(void){}
opencpn_plugin_116::opencpn_plugin_116(void *pmgr): opencpn_plugin_115(pmgr){}
opencpn_plugin_116::~opencpn_plugin_116(void){}
bool opencpn_plugin_116::RenderOverlayMultiCanvas(wxDC &, PlugIn_ViewPort *vp, int canvasIndex) {return false;}
bool opencpn_plugin_116::RenderGLOverlayMultiCanvas(wxGLContext *pcontext, PlugIn_ViewPort *vp, int canvasIndex){return false;}
bool opencpn_plugin_17::RenderOverlay(wxDC &, PlugIn_ViewPort *vp) {return false;}
bool opencpn_plugin_17::RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp)  {return false;}
void opencpn_plugin_19::OnSetupOptions(void) {return;}
void opencpn_plugin_18::SetPluginMessage(wxString &message_id, wxString &message_body){}
void opencpn_plugin_18::SetPositionFixEx(PlugIn_Position_Fix_Ex &pfix){}
void opencpn_plugin_110::LateInit(void){return;}
bool opencpn_plugin_112::MouseEventHook( wxMouseEvent &event ){return false;}
void opencpn_plugin_112::SendVectorChartObjectInfo(wxString &chart, wxString &feature, wxString &objname, double lat, double lon, double scale, int nativescale){return;}
bool opencpn_plugin_113::KeyboardEventHook( wxKeyEvent &event ){return false;}
void opencpn_plugin_113::OnToolbarToolDownCallback(int id){return;}
void opencpn_plugin_113::OnToolbarToolUpCallback(int id){return;}
void opencpn_plugin_116::PrepareContextMenu( int canvasIndex) {return;}
