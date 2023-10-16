/**************************************************************************
* Project:  OpenCPN
* Purpose:  JavaScript Plugin
* Author:   Tony Voss 25/04/2022
*
* Copyright Ⓒ 2023 by Tony Voss
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, under which
* you must including a copy of these terms in all copies
* https://www.gnu.org/licenses/gpl-3.0.en.html
***************************************************************************
*/
#include "JavaScript_pi.h"
#include "JavaScriptgui_impl.h"
#include <wx/wx.h>
#include <wx/arrimpl.cpp>
#include <wx/button.h>

extern JavaScript_pi* pJavaScript_pi;

wxSize getButtonSize(Console* pConsole) { // determine a size for the fileString buttons
    size_t maxLength = 0;
    wxString fileString, longestString;
    wxButton* fileButton;
    wxSize buttonSize;
    
    for (size_t i = 0; i < pJavaScript_pi->favouriteFiles.GetCount(); i++){
        fileString = pJavaScript_pi->favouriteFiles[i];
        if (fileString.Length() > maxLength){
            maxLength = fileString.Length();
            longestString = fileString;
            }
        }
    for (size_t i = 0; i < pJavaScript_pi->recentFiles.GetCount(); i++){
        fileString = pJavaScript_pi->recentFiles[i];
        if (fileString.Length() > maxLength){
            maxLength = fileString.Length();
            longestString = fileString;
            }
        }
    // make a test button
    fileButton = new wxButton(pConsole, wxID_ANY, longestString, wxDefaultPosition, wxDefaultSize, 0 );
    fileButton->Fit();
    buttonSize = fileButton->GetSize();
    delete fileButton;
    TRACE(5,  wxString::Format("Longest text is %s\nand text length is %d and button width is %d", longestString, maxLength, buttonSize.GetWidth()));
    return buttonSize;
    }

wxString chooseLoadFile(Console* console){
    // use dialogue to choose file for loading.
    // returns fiestring else  string "**cancel**" if cancelled
    wxString fileString;
    wxFileDialog openConsole( console, _( "File to load" ), _(""), _(""),
                             _( "*.js" ),
                             wxFD_OPEN);
    int response = openConsole.ShowModal();
    if (response == wxID_OK) fileString = openConsole.GetPath();
    else fileString = "**cancel**";
    return fileString;
    }

void onFilesButton(wxCommandEvent buttonEvent){
    // handles button click in recent files dialog
    // closes modal with return of:
    // if Other...   -1
    // if Cancel    -2
    // if Organise favourites -3
    // if file not located, -4
    // if Favourite the index in favouriteFiles
    // if Recent, the index in recentFiles + 1000000;
    wxButton* pbuttonPressed = wxDynamicCast(buttonEvent.GetEventObject(), wxButton);
    wxString label = pbuttonPressed->GetLabel();
    int index;
    TRACE(6, "Selected label " + label);
    if (label == _("Other...")) index = -1;
    else if (label == _("Cancel")) index = -2;
    else if (label == _("Organise Favourites")) index = -3;
    else {
        index = pJavaScript_pi->favouriteFiles.Index(label);
        if (index == wxNOT_FOUND){  // should be in recents
            index = pJavaScript_pi->recentFiles.Index(label);
            if (index == wxNOT_FOUND){
                index = -4;
                }
            else index += 1000000;
            }
        }
    wxDialog* pdialog = (wxDialog*)pbuttonPressed->GetParent();
    pdialog->EndModal(index);
    }

void onOrganiserButton(wxCommandEvent buttonEvent){
    // handles button click in orgniser dialog
    // closes modal with return of:
    // Done   -1
    // if file not located, -2
    // if Favourite the index in favouriteFiles
    // if Recent, the index in recentFiles + 1000000;
    wxButton* pbuttonPressed = wxDynamicCast(buttonEvent.GetEventObject(), wxButton);
    wxString label = pbuttonPressed->GetLabel();
    int index;
    TRACE(6, "Organiser selected label " + label);
    if (label == _("Done")) index = -1;
    else {
        index = pJavaScript_pi->favouriteFiles.Index(label);
        if (index == wxNOT_FOUND){  // should be in recents
            index = pJavaScript_pi->recentFiles.Index(label);
            if (index == wxNOT_FOUND){
                index = -2;
                }
            else index += 1000000;
            }
        }
    wxDialog* pdialog = (wxDialog*)pbuttonPressed->GetParent();
    pdialog->EndModal(index);
    }

wxBoxSizer* constructButtons(wxDialog* pdialog,  wxArrayString fileStrings, wxSize size, wxString caption){
    // constructs a sizer comprsing a caption followed by buttons for each string in filestrings
    // preceeded with a caption
    TRACE(5, "Entered constructButtons with caption " + caption);
    wxButton* fileButton;
    wxBoxSizer* box = new wxBoxSizer(wxVERTICAL);
    wxStaticText* subhead = new wxStaticText( pdialog, wxID_STATIC, caption, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
    wxFont font = subhead->GetFont().MakeLarger();
    subhead->SetFont(font);
    box->Add(subhead);
    for (size_t i = 0; i < fileStrings.GetCount(); i++){
        fileButton = new wxButton(pdialog, wxID_ANY, fileStrings[i], wxDefaultPosition, size, wxBU_LEFT );
        box->Add(fileButton, 0, wxALIGN_LEFT|wxALL, 5);
        }
    wxStaticLine* line = new wxStaticLine ( pdialog, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    box->Add(line, 0, wxGROW|wxALL, 5);
    TRACE(5, "Returning from constructButtons");
    return box;
    }

bool organiseFavourites(Console* pConsole){ // returns true while more to organise
    wxDialog* pdialog = new wxDialog(pConsole,  wxID_ANY, "Favourites organiser", wxDefaultPosition, wxDefaultSize,
                                   wxRESIZE_BORDER | wxCAPTION | wxSTAY_ON_TOP);
    TRACE(5, "Entered organiseFavourites");
    int favouritesCount = pJavaScript_pi->favouriteFiles.GetCount();
    int recentsCount = pJavaScript_pi->recentFiles.GetCount();
    if ((favouritesCount + recentsCount) == 0) return false;
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);  // A top-level sizer
    pdialog->SetSizer(topSizer);
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL); // A second box sizer to give more space around the controls
    topSizer->Add(boxSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxBoxSizer* buttonBox;
    wxSize buttonSize;
    buttonSize = getButtonSize(pConsole);
    if (favouritesCount > 0){
        buttonBox = constructButtons(pdialog,  pJavaScript_pi->favouriteFiles, buttonSize, "Favourites: select to remove from favourites");
        boxSizer->Add(buttonBox);
        }
    if (recentsCount > 0){
        buttonBox = constructButtons(pdialog,  pJavaScript_pi->recentFiles, buttonSize, "Recents: select to add to favourites");
        boxSizer->Add(buttonBox);
        }
    wxBoxSizer* finalButtonBox = new wxBoxSizer(wxHORIZONTAL);
    boxSizer->Add(finalButtonBox, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxButton* doneButton = new wxButton(pdialog, wxID_ANY, "Done");
    finalButtonBox->Add(doneButton, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    pdialog->Bind(wxEVT_BUTTON, &onOrganiserButton, wxID_ANY);
    pdialog->Fit();
    int index = pdialog->ShowModal();
    if (index == -2){
        pConsole->message(STYLE_RED, wxString("Failed to find fileString - logic error in organiseFavourites"));
        return false;
        }
    else if (index == -1) return false; // done
    else if (index < 1000000){  // Favourite selected - remove it
        pJavaScript_pi->favouriteFiles.RemoveAt(index);
        }
    else {  // recent selected - promote to favourite
        index -= 1000000;
        wxString fileString = pJavaScript_pi->recentFiles.Item(index);
        pJavaScript_pi->recentFiles.RemoveAt(index);
        pJavaScript_pi->favouriteFiles.Add(fileString);
        }
    return true;
    }

wxString chooseFileString(Console* pConsole){
    // choose fileString from favourites or recents
    wxString fileString;
    int favouritesCount = pJavaScript_pi->favouriteFiles.GetCount();
    int recentsCount = pJavaScript_pi->recentFiles.GetCount();
    if ((favouritesCount + recentsCount) == 0) return wxEmptyString;    // nothing to choose from
    wxDialog* pdialog = new wxDialog(pConsole,  wxID_ANY, "Load a script", wxDefaultPosition, wxDefaultSize,
                                   wxRESIZE_BORDER | wxCAPTION | wxSTAY_ON_TOP);
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);  // A top-level sizer
    pdialog->SetSizer(topSizer);
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL); // A second box sizer to give more space around the controls
    topSizer->Add(boxSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxBoxSizer* buttonBox;
    wxSize buttonSize;
    buttonSize = getButtonSize(pConsole);
    if (favouritesCount > 0){
        buttonBox = constructButtons(pdialog,  pJavaScript_pi->favouriteFiles, buttonSize, "Favourites");
        boxSizer->Add(buttonBox);
        }
    if (recentsCount > 0){
        buttonBox = constructButtons(pdialog,  pJavaScript_pi->recentFiles, buttonSize, "Recents");
        boxSizer->Add(buttonBox);
        }
    wxBoxSizer* finalButtonBox = new wxBoxSizer(wxHORIZONTAL);
    boxSizer->Add(finalButtonBox, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxButton* cancelButton = new wxButton(pdialog, wxID_ANY, "Cancel");
    wxButton* otherButton = new wxButton(pdialog, wxID_ANY, "Other...");
    otherButton->SetDefault();
    finalButtonBox->Add(cancelButton, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    finalButtonBox->Add(otherButton, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    if ((favouritesCount + recentsCount)  > 0){
        // add organiser button, which needs to stand apart
        wxButton* organiserButton = new wxButton(pdialog, wxID_ANY, "Organise Favourites", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
        wxFont font = organiserButton->GetFont().MakeSmaller();
        organiserButton->SetFont(font);
        boxSizer->Add(organiserButton, 0, wxALIGN_LEFT|wxALL, 5);
        TRACE(5, "Added organiser button to chooseFileString dialog");
        }
    pdialog->Bind(wxEVT_BUTTON, &onFilesButton, wxID_ANY);
    pdialog->Fit();
    TRACE(5, "About to display chooseFileString dialog");
    int index = pdialog->ShowModal();
    TRACE(6, wxString::Format("back from chooseFileString with index %d", index));
    if (index == -1){   // to load Other
        fileString = chooseLoadFile(pConsole);
        }
    else if (index == -2){  // cancelled
        fileString = "**cancel**";
        }
    else if (index == -3){  // organise favourites
        while (organiseFavourites(pConsole)) {};
        fileString = chooseFileString(pConsole);    // recursing!
        }
    else if (index < -3){
        pConsole->message(STYLE_RED, wxString("Failed to find fileString - logic error in chooseFileString"));
        fileString = wxEmptyString;
        }
    else if (index < 1000000){  // It is a favourite
        fileString = pJavaScript_pi->favouriteFiles[index];
        }
    else {  // it must be a recent
        fileString = pJavaScript_pi->recentFiles[index-1000000];
        }
    return fileString;
    }
