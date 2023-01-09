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
*/

#include <iostream>
#include "JavaScript_pi.h"
#include "JavaScriptgui_impl.h"
#include <wx/wx.h>
#include <wx/arrimpl.cpp>
#include <wx/button.h>
#include "ocpn_plugin.h"
#include "trace.h"
#include <wx/event.h>
#include "wx/window.h"

#define FAIL(X) do { error = X; goto failed; } while(0)

extern JavaScript_pi *pJavaScript_pi;

void updateRecentFiles(wxString fileString){
    //  Updates the array of recent file names
    const int maxFiles {10}; // maximum number of recents to save
    if (pJavaScript_pi->favouriteFiles.Index(fileString) == wxNOT_FOUND){  // not in favourites, so revise recents
        pJavaScript_pi->recentFiles.Remove(fileString); // remove any existing matching entry
        pJavaScript_pi->recentFiles.Insert(fileString, 0);  // insert this one as the first
        int newCount = pJavaScript_pi->recentFiles.GetCount();
        // trim list if over maximum
        if (newCount > maxFiles) pJavaScript_pi->recentFiles.RemoveAt(maxFiles, newCount-maxFiles);
        }
    }

int messageComp(MessagePair** arg1, MessagePair** arg2) {   // used when sorting messages
    return (strcmp((*arg1)->messageName, (*arg2)->messageName));}

WX_DEFINE_OBJARRAY(MessagesArray);
WX_DEFINE_OBJARRAY(TimesArray);
WX_DEFINE_OBJARRAY(MenusArray);
#ifdef SOCKETS
WX_DEFINE_OBJARRAY(SocketRecordsArray);
#endif
WX_DEFINE_OBJARRAY(ConsoleRepliesArray);

void Console::OnActivate(wxActivateEvent& event){
    wxDialog* pConsole = wxDynamicCast(event.GetEventObject(), wxDialog);
    long int style = pConsole->GetWindowStyle();
    if (event.GetActive()) pConsole->SetWindowStyle(style | wxSTAY_ON_TOP); // bring console on top
    pConsole->SetWindowStyle(style ^ wxSTAY_ON_TOP);    // but do not force to stay there
    };

void Console::OnLoad( wxCommandEvent& event ) { // we are to load a script
    wxString fileString;
    wxTextFile ourFile;
    wxString lineOfData, script;
    wxString JScleanString(wxString line);
    wxString chooseFileString(Console*);
    wxString chooseLoadFile(Console*);
    
    fileString = chooseFileString(this);
    if (fileString == "**cancel**"){
        TRACE(3, "Load cancelled");
        return;
        }
    if (fileString == wxEmptyString) fileString = chooseLoadFile(this);  // if no favourite or recent ask for new
    if (fileString != wxEmptyString){   // only if something now chosen
        bool OK = ourFile.Open(fileString);
        if (!OK){ // can't read file - it may have gone away
            message(STYLE_RED, "File '" + fileString + "' not found or unreadable");
            pJavaScript_pi->recentFiles.Remove(fileString); // remove from recents
            return;
            }
        m_Script->ClearAll();   // clear old content
        for (lineOfData = ourFile.GetFirstLine(); !ourFile.Eof(); lineOfData = ourFile.GetNextLine()){
            script += lineOfData + "\n";
            }
        script = JScleanString(script);
        m_Script->AppendText(script);
        m_fileStringBox->SetValue(wxString(fileString));
        auto_run->Show();
        auto_run->SetValue(false);
        updateRecentFiles(fileString);
        }
    }

void Console::OnSaveAs( wxCommandEvent& event ) {
    int response = wxID_CANCEL;
    wxArrayString file_array;
    wxString filename, filePath;
    wxTextFile ourFile;
    wxString lineOfData;
    wxDialog query;
    
    wxFileDialog SaveAsConsole( this, _( "Saving your script" ), wxEmptyString, wxEmptyString,
                               _T("js files (*.js)|*.js|All files (*.*)|*.*"),
                               wxFD_SAVE|wxFD_OVERWRITE_PROMPT|wxDD_NEW_DIR_BUTTON);
    response = SaveAsConsole.ShowModal();
    if( response == wxID_OK ) {
        filePath = SaveAsConsole.GetPath();
        if (!filePath.EndsWith(".js")) filePath += ".js";
        m_Script->SaveFile(filePath, wxTEXT_TYPE_ANY);
        m_fileStringBox->SetValue(wxString(filePath));
        auto_run->Show();
        updateRecentFiles(filePath);
        return;
    }
    else if(response == wxID_CANCEL){
        return;
        }
    }

void Console::OnSave( wxCommandEvent& event ) {
    wxArrayString file_array;
    wxString filename;
    wxTextFile ourFile;
    wxString lineOfData;
    wxDialog query;
    
    mFileString = m_fileStringBox->GetValue();
    if ((   mFileString != "") & wxFileExists(   mFileString)) {
        // Have a 'current' file, so can just save to it
        m_Script->SaveFile(mFileString);
        TRACE(3, wxString::Format("Saved to  %s",mFileString));
        auto_run->Show();
        updateRecentFiles(mFileString);
        return;
        }
    else OnSaveAs(event);  // No previous save, so do Save As
    return;
    }

void Console::OnCopyAll(wxCommandEvent& event) {
    int currentPosition = m_Script->GetCurrentPos();
    m_Script->SelectAll();
    m_Script->Copy();
    m_Script->GotoPos(currentPosition);
    }

void Console::OnClearScript( wxCommandEvent& event ){
    m_Script->ClearAll();
    m_Script->SetFocus();
    m_fileStringBox->SetValue(wxEmptyString);
    mFileString = wxEmptyString;
    auto_run->SetValue(false);
    auto_run->Hide();
    }

void Console::OnClearOutput( wxCommandEvent& event ){
    m_Output->ClearAll();
    }

void Console::OnRun( wxCommandEvent& event ) {
#ifdef TRACE_YES
    extern JavaScript_pi *pJavaScript_pi;
    if (!pJavaScript_pi->mTraceLevelStated)
        message(STYLE_ORANGE, wxString::Format("Tracing levels %d - %d",TRACE_MIN, TRACE_MAX));
    pJavaScript_pi->mTraceLevelStated = true;
#endif
    clearBrief();
    mConsoleRepliesAwaited = 0;
    TRACE(0, "------------------ Run console " + mConsoleName);
    doRunCommand(mBrief);
    }

void Console::OnAutoRun(wxCommandEvent& event){   // Auto run tick box
    if (this->auto_run->GetValue()){
        // box has been ticked
        mFileString  = m_fileStringBox->GetValue();
        if ((   mFileString != "") & wxFileExists(   mFileString /*FilePath*/)){
            // Have a 'current' file we can auto-run
            this->m_autoRun = true;
            }
        else {
            this->message(STYLE_RED, wxString("No script loaded to auto-run"));
            this->auto_run->SetValue(false);
            this->m_autoRun = false;
            }
        }
    else this->m_autoRun = false;
    }

void Console::OnClose(wxCloseEvent& event) {
    extern JavaScript_pi *pJavaScript_pi;
    TRACE(1, "Closing console " + this->mConsoleName + " Can veto is " + (event.CanVeto()?"true":"false"));

    if (event.CanVeto()){
        if ((this == pJavaScript_pi->mpFirstConsole) && (this->mpNextConsole == nullptr)) {
            // This is only console - decline
            this->message(STYLE_RED, "Console close: You cannot close the only console");
            event.Veto(true);
            return;
            }
        if (!this->m_Script->IsEmpty()) {
            // We will not delete a console with a script
            this->message(STYLE_RED, "Console close: clear the script first");
            event.Veto(true);
            return;
            }
        }
	this->bin();
	// take care to remove from tools, if we have them open
    ToolsClass *pTools = pJavaScript_pi->pTools;
    if (pTools != nullptr) pTools->setConsoleChoices();
	TRACE(3, "Binning console " + this->mConsoleName);
	return;
    }

static wxString dummyMessage, message_id;
wxDialog* dialog;

void Console::OnTools( wxCommandEvent& event){
    extern JavaScript_pi *pJavaScript_pi;
    
    pJavaScript_pi->ShowPreferencesDialog(pJavaScript_pi->m_parent_window);
    return;
    }
