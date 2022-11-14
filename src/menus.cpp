/**************************************************************************
* Project:  OpenCPN
* Purpose:  JavaScript Plugin
* Author:   Tony Voss 25/02/2021
*
* Copyright Ⓒ 2022 by Tony Voss
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, under which
* you must including a copy of these terms in all copies
* https://www.gnu.org/licenses/gpl-3.0.en.html
***************************************************************************
*/

// This file is the start an exploration of adding a general menus capability
// It is in abeyance because of the difficulty of accessing the menubar (gFrame->m_pMenuBar) without a plugin API
#include <iostream>
#include "wx/wx.h"
#include "wx/fileconf.h"
#include "duktape.h"
#include <wx/menuitem.h>
#include <iostream>
#include "JavaScriptgui_impl.h"

// #include "canvasMenu.h"
//#include "wx/arrimpl.cpp"

extern JavaScript_pi* pJavaScript_pi;
extern Console* pConsoleBeingTimed;
//extern myFrame* gFrame;
void throwErrorByCtx(duk_context *ctx, wxString message);
Console* findConsoleByName(wxString name);

//wxMenuBar menuBar = gFrame->GetMainMenuBar();

//wxMenuItem* menuItem;
//wxWindow* canvas = GetOCPNCanvasWindow();

//wxAuiManager* auiManager = GetFrameAuiManager();
//wxMenuBar* menuBar = auiManager->GetMenuBar();


wxMenuItem* menuItem = new wxMenuItem(nullptr,wxID_ANY, "Test2 item label");
wxMenuItem* subMenu = new wxMenuItem(nullptr, wxID_ANY, "Details");

duk_ret_t APImenu(duk_context *ctx){
//    Console* pConsole = findConsoleByName(wxString name);
    duk_idx_t nargs = duk_get_top(ctx);  // number of args 
    duk_require_number(ctx, 0);	// first should be action
	int action = duk_get_number(ctx, 0);
	int id;
	wxString name = wxString("Test name");
	switch (action){
	case 0:	// delete menu item
		RemoveCanvasMenuItem(id, name); 
		break;
	case 1: // create menuItem
/*	
		duk_require_object(ctx, 1);	// 2nd arg should be menuItem
		if (!duk_get_prop_string(ctx, 1, "kind")) throwErrorByCtx(ctx, "menu has no kind");
		if (!duk_is_symbol(ctx, -1)) throwErrorByCtx(ctx, "menu kind is not symbol");
		string kind = duk_to_string(ctx, -1);
		duk_pop(ctx);
*/
		id = AddCanvasMenuItem(menuItem, pJavaScript_pi, name);
        break;
        }
	duk_pop_2(ctx);	// temp stack adjustment
	return 0;
    }


