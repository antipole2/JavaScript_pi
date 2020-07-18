//
//  main.cpp
//  Plugin development
//
//  Created by Tony Voss on 25/04/2020.
//  Copyright © 2020 Tony Voss. All rights reserved.
//
// provides a harness for testing OpenCPN plugins outside OpenCPN

// #define TIXML_USE_STL
#include <iostream>
// include "harness.h"
#include "JavaScript_pi.h"

// opencpn_plugin* JavaScript_pi;

int main(int argc, const char * argv[]) {
    
    std::cout << "Harness started\n";
 //   std::cout << wxString(JavaScript_pi->GetCommonName());
    return 0;
}

// dummy routines to replace OpenCPN ones
/*
void DimeWindow(wxWindow *){std::cout << "DimeWindow called\n";}
wxWindow canvasA;
wxWindow *GetOCPNCanvasWindow(){std::cout << "*GetOCPNCanvasWindow called\n"; return(& canvasA);}
bool AddLocaleCatalog( wxString catalog ){std::cout << "AddLocaleCatalog called\n"; return(true);}
wxFileConfig *p_configA;
wxFileConfig *GetOCPNConfigObject(){std::cout << "*GetOCPNConfigObject called\n"; return((wxFileConfig *) p_configA);}
wxString sharedA;
wxString *GetpSharedDataLocation(){std::cout << "*GetpSharedDataLocation called\n"; return(& sharedA);}
int InsertPlugInTool(wxString label, wxBitmap *bitmap, wxBitmap *bmpRollover, wxItemKind kind,
                 wxString shortHelp, wxString longHelp, wxObject *clientData, int position,
                 int tool_sel, opencpn_plugin *pplugin){std::cout << "InsertPlugInTool called\n"; return(1);}
void RequestRefresh(wxWindow *){std::cout << "RequestRefresh called\n"; return;}
void SetToolbarItemState(int item, bool toggle){std::cout << "SetToolbarItemState called\n"; return;}
bool destLoxodrome(double lat1, double lon1, double brng, double dist, double* lat2, double* lon2){std::cout << "destLoxodrome called\n"; return(false);}
void DistanceBearingMercator(double lat0, double lon0, double lat1, double lon1, double *dist, double *brg){std::cout << "DistanceBearingMercator called\n"; return;}
void opencpn_plugin::SetDefaults(void){std::cout << "SetDefaults called\n";}
bool opencpn_plugin::RenderOverlay(wxMemoryDC *pmdc, PlugIn_ViewPort *vp){std::cout << "RenderOverlay called\n"; return(false);}
void opencpn_plugin::SetAISSentence(wxString &sentence){std::cout << "SetAISSentence called\n";}
void opencpn_plugin::SetPositionFix(PlugIn_Position_Fix &pfix){std::cout << "SetPositionFix called\n";}
void opencpn_plugin::SetCursorLatLon(double lat, double lon){std::cout << "SetCursorLatLon called\n";}
void opencpn_plugin::UpdateAuiStatus(void){std::cout << "UpdateAuiStatus called\n";}
void opencpn_plugin::SetNMEASentence(wxString &sentence){std::cout << "SetNMEASentence called\n";}
void opencpn_plugin::OnCloseToolboxPanel(int page_sel, int ok_apply_cancel){std::cout << "OnCloseToolboxPanel called\n";}
void opencpn_plugin::SetCurrentViewPort(PlugIn_ViewPort &vp){std::cout << "SetCurrentViewPort called\n";}
void opencpn_plugin::ProcessParentResize(int x, int y){std::cout << "ProcessParentResize called\n";}
void opencpn_plugin::SetupToolboxPanel(int page_sel, wxNotebook* pnotebook){std::cout << "SetupToolboxPanel called\n";}
int opencpn_plugin::GetToolboxPanelCount(void){std::cout << "GetToolboxPanelCount called\n"; return(0);}
void opencpn_plugin::ShowPreferencesDialog( wxWindow* parent ){std::cout << "ShowPreferencesDialog called\n";}
void opencpn_plugin::OnContextMenuItemCallback(int id){std::cout << "OnContextMenuItemCallback called\n";}
wxArrayString opencpn_plugin::GetDynamicChartClassNameArray(void){std::cout << "GetDynamicChartClassNameArray called\n"; return(wxArrayString());}
bool opencpn_plugin_17::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp){std::cout << "RenderOverlay_17 called\n"; return(false);}
bool opencpn_plugin_17::RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp){std::cout << "RenderGLOverlay_17 called\n";return(false);}
void opencpn_plugin_17::SetPluginMessage(wxString &message_id, wxString &message_body){std::cout << "SetPluginMessage_17 called\n";}

*/


/*
void opencpn_plugin(void *pmgr) {
   std::cout << "Opencpn called\n";
}
 



wxString opencpn_plugin::GetCommonName(){std::cout << "GetCommonName called\n"; return(wxString());}
void opencpn_plugin::SetColorScheme(PI_ColorScheme cs){std::cout << "SetColorScheme called\n";}

    opencpn_plugin_17::~opencpn_plugin_17(){std::cout << "~opencpn_plugin_17 called\n";}



wxBitmap *p_bitmap;
wxBitmap *opencpn_plugin::GetPlugInBitmap(){std::cout << "*GetPlugInBitmap called\n"; return((wxBitmap *)p_bitmap);}
bool AddLocaleCatalog( wxString * ){
    std::cout << "AddLocaleCatalog called\n";
    return (true);
}
int opencpn_plugin::GetAPIVersionMajor(){return(1);}
int opencpn_plugin::GetAPIVersionMinor(){return(2);}
int opencpn_plugin::GetPlugInVersionMajor(){return(3);}
int opencpn_plugin::GetPlugInVersionMinor(){return(4);}
wxString opencpn_plugin::GetShortDescription(){return(wxString("Short Description"));}
wxString opencpn_plugin::GetLongDescription(){return(wxString("Long Description"));}
int opencpn_plugin::GetToolbarToolCount(void){return(10);}
void opencpn_plugin::OnToolbarToolCallback(int id){std::cout << "OnToolbarToolCallback called\n"; return;}
int opencpn_plugin::Init(void){std::cout << "Init called\n"; return(1);}
bool opencpn_plugin::DeInit(void){std::cout << "Init called\n"; return(0);}

 

 }

 

void initialize_images(){
    std::cout << "initialize_images called\n";
    return;
}
*/
