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
*/
#include "wx/stc/stc.h"
#include "wx/settings.h"

void JSlexit(wxStyledTextCtrl* pane){  // lex the script window
        // markers
        pane->MarkerDefine (wxSTC_MARKNUM_FOLDER, wxSTC_MARK_BOXPLUS);
        pane->MarkerSetBackground (wxSTC_MARKNUM_FOLDER, wxColour ("BLACK"));
        pane->MarkerSetForeground (wxSTC_MARKNUM_FOLDER, wxColour ("WHITE"));
        pane->MarkerDefine (wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_BOXMINUS);
        pane->MarkerSetBackground (wxSTC_MARKNUM_FOLDEROPEN, wxColour ("BLACK"));
        pane->MarkerSetForeground (wxSTC_MARKNUM_FOLDEROPEN, wxColour ("WHITE"));
        pane->MarkerDefine (wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_EMPTY);
        pane->MarkerDefine (wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_BOXPLUS);
        pane->MarkerSetBackground (wxSTC_MARKNUM_FOLDEREND, wxColour ("BLACK"));
        pane->MarkerSetForeground (wxSTC_MARKNUM_FOLDEREND, wxColour ("WHITE"));
        pane->MarkerDefine (wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_BOXMINUS);
        pane->MarkerSetBackground (wxSTC_MARKNUM_FOLDEROPENMID, wxColour ("BLACK"));
        pane->MarkerSetForeground (wxSTC_MARKNUM_FOLDEROPENMID, wxColour ("WHITE"));
        pane->MarkerDefine (wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY);
        pane->MarkerDefine (wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_EMPTY);
        pane->SetMarginType (1, wxSTC_MARGIN_SYMBOL);
        pane->SetMarginMask (1, wxSTC_MASK_FOLDERS);
        pane->SetMarginWidth (1, 16);
        pane->SetMarginSensitive (1, true);
        pane->SetProperty( "fold", "1" );
        pane->SetFoldFlags( wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED );
        pane->SetMarginWidth( 1, 0 );
        pane->SetMarginWidth( 2, 0 );
        pane->SetLexer(wxSTC_LEX_CPP);
        // supported keywords
        wxString keywordsSupported = "arguments\
            boolean break\
            case catch const continue\
            default delete do\
            else eval\
            false finally for function\
            if in\
            new null NaN\
            package private protected public\
            return\
            switch\
            this throw true try typeof\
            undefined\
            var void\
            while";
        // The following are unsupported or reserved for developer use and lexed orange
        wxString keywordsUnsupported = "abstract\
            byte\
            char class\
            debugger double\
            enum\
            export extends\
            final float\
            goto\
            implements import instanceof int interface\
            of\
            let long\
            native\
            short static super synchronized\
            throws transient\
            volatile \
            with\
            yield\
            JS_duk_dump\
            consoleDump\
            consoleDetails\
            JS_throw_test\
            JS_mainThread\
            =>\
            ";
    wxString extensionNames = "print printRed printOrange printGreen printBlue printUnderlined printLog\
            alert\
            readTextFile\
            writeTextFile\
            require\
            timeAlloc\
            keyboardState\
            scriptResult\
            consoleHide\
            consoleShow\
            consoleName\
            consolePark\
            onSeconds\
            onAllSeconds\
            onDialogue\
            onExit\
            onCloseButton\
            stopScript\
            messageBox\
            toClipboard\
            fromClipboard\
            cleanString\
            NMEA0183checksum\
            OCPNpushNMEA0183\
            OCPNpushNMEA2kSentence\
            OCPNgetMessageNames\
            OCPNsendMessage\
            OCPNonNMEA0183\
            OCPNonAllNMEA0183\
            OCPNonNMEA2000\
            OCPNonAllNMEA2000\
            OCPNonMessageName\
            OCPNonAllMessageName\
            OCPNgetNavigation\
            OCPNonNavigation\
            OCPNonAllNavigation\
            OCPNgetARPgpx\
            OCPNonActiveLeg\
            OCPNonContextMenu\
            OCPNgetNewGUID\
            OCPNgetWaypointGUIDs\
            OCPNgetPluginConfig\
            OCPNgetSingleWaypoint\
            OCPNdeleteSingleWaypoint\
            OCPNaddSingleWaypoint\
            OCPNupdateSingleWaypoint\
            OCPNgetActiveWaypointGUID\
            OCPNgetCursorPosition\
            OCPNgetRouteGUIDs\
            OCPNgetActiveRouteGUID\
            OCPNgetRoute\
            OCPNdeleteRoute\
            OCPNaddRoute\
            OCPNupdateRoute\
            OCPNgetTrackGUIDs\
            OCPNaddTrack\
            OCPNgetTrack\
            OCPNupdateTrack\
            OCPNdeleteTrack\
            OCPNrefreshCanvas\
            OCPNgetCanvasView\
            OCPNcentreCanvas\
            OCPNgetAISTargets\
            OCPNgetVectorPP\
            OCPNgetPositionPV\
            OCPNgetGCdistance\
            OCPNsoundAlarm\
            OCPNisOnline\
            consoleExists\
            consoleBusy\
            consoleClearOutput\
            consoleGetOutput\
            consoleAdd\
            consoleClose\
            consoleLoad\
            consoleRun\
            onConsoleResult\
            _remember\
            ";
 		wxString keywordsDeprecated = "OCPNpushNMEA OCPNonNMEAsentence";

        pane->SetKeyWords(0, keywordsSupported);
        pane->SetKeyWords(1, keywordsUnsupported + keywordsDeprecated);
        pane->SetKeyWords(3, extensionNames);

 //     wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
        pane->StyleClearAll();
        pane->StyleSetBold(wxSTC_C_WORD, true);
        pane->StyleSetForeground(wxSTC_C_WORD, wxColour(28,120,255));// blue for supported keywords
        pane->StyleSetForeground(wxSTC_C_UUID, wxColour(255,64,255));  // magenta for deprecated keywords
        pane->StyleSetForeground(wxSTC_C_WORD2, wxColour(255,118,0));  // orange for unsupported keywords
        pane->StyleSetForeground(wxSTC_C_GLOBALCLASS, wxColour(132, 0, 255));  // purple for extensions
        pane->StyleSetForeground(wxSTC_C_STRING, *wxRED);
        pane->StyleSetForeground(wxSTC_C_STRINGEOL, *wxRED);
 //     pane->StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour(49, 106, 197));
        pane->StyleSetForeground(wxSTC_C_COMMENT, wxColour(0, 128, 0));
        pane->StyleSetForeground(wxSTC_C_COMMENTLINE, wxColour(0, 128, 0));
 //     pane->StyleSetForeground(wxSTC_C_COMMENTDOC, wxColour(0, 128, 0));
        pane->StyleSetForeground(wxSTC_C_COMMENTLINEDOC, wxColour(0, 128, 0));
        pane->StyleSetForeground(wxSTC_C_NUMBER, *wxBLUE );
        pane->SetSelBackground(true, wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
        pane->SetSelForeground(true, wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
        pane->SetCaretWidth(2);
    }
