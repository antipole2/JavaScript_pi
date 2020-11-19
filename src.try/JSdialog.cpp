/*************************************************************************
 * Project:  OpenCPN
 * Purpose:  JavaScript Plugin
 * Author:   Tony Voss
 *
 * Copyright Ⓒ 2020 by Tony Voss
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, under which
 * you must including a copy of these terms in all copies
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 ***************************************************************************
*/

#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/progdlg.h>
#include <wx/button.h>
#include "wx/spinctrl.h"
#include "wx/valgen.h"
#include "wx/statline.h"
#include "wx/textctrl.h"
#include "JavaScriptgui.h"
#include "duktape.h"
#include "ocpn_duk.h"
// #include "wx/valtext.h"
#include "wx/radiobox.h"
#include "wx/listbox.h"

extern JS_control_class JS_control;
void JS_dk_error(duk_context *ctx, wxString message);

void onButton(wxCommandEvent & event){
    // here when any dialogue button clicked
    duk_context *ctx;
    wxWindow *window;
    wxButton *button;
    wxString elementType;
    wxString theData, functionName;
    DialogAction dialogAction;
    std::vector<dialogElement>::const_iterator it;
    int i;
    duk_bool_t ret;
    void JS_exec(duk_context *ctx);
//    void JSduk_start_exec_timeout(void);
 //   void  JSduk_clear_exec_timeout(void);
    
    ctx = JS_control.m_pctx;
#ifdef IN_HARNESS
    cout << "In onButton\n";
    MAYBE_DUK_DUMP
#endif  // IN_HARNESS

    if (ctx == nullptr) JS_control.m_pJSconsole->m_Output->AppendText("onButton invoked with no ctx context\n");
    // locate our dialogeAction
#if 0
    functionName = JS_control.m_dialog.functionName;
    if (!duk_get_global_string(ctx, functionName.c_str())){
        jsMessage(ctx, STYLE_RED, _("JavaScript onDialogue function ") + functionName + " ", duk_safe_to_string(ctx, -1));
        }
    JS_control.m_dialog.functionName = wxEmptyString;   // clear out so we do not use again
    button = wxDynamicCast(event.GetEventObject(), wxButton);
    window = button->GetParent();
    JS_control.m_dialog.position = window->GetPosition();   // remember where it is
    if ( window->Validate() && window->TransferDataFromWindow() ){
        window->Show(false);
        // now to build the returned object
        duk_push_array(ctx);
        for (i = 0, it = JS_control.m_dialog.dialogElementsArray.cbegin(); it !=  JS_control.m_dialog.dialogElementsArray.cend(); i++, it++){
            elementType = it->type;
//          jsMessage(ctx, STYLE_BLACK, _("Processing "), wxString::Format(_("i:%d %d %s"), i, it->itemID, elementType));
            duk_push_object(ctx);
            duk_push_string(ctx, elementType);
            duk_put_prop_string(ctx, -2, "type");
            if ((elementType == "caption") || (elementType == "text") || (elementType == "hLine")){
                duk_push_string(ctx, it->value);
                duk_put_prop_string(ctx, -2, "value");
                }
            else if (elementType == "field"){
                duk_push_string(ctx, it->label);
                duk_put_prop_string(ctx, -2, "label");
                duk_push_string(ctx, it->textValue);
                duk_put_prop_string(ctx, -2, "value");
                if (it->suffix != wxEmptyString){
                    duk_push_string(ctx, it->suffix);
                    duk_put_prop_string(ctx, -2, "suffix");
                    }
                }
            else if (elementType == "tick"){
                wxCheckBox *tickedBox;
                tickedBox = wxDynamicCast(window->FindWindowById(it->itemID), wxCheckBox);
                duk_push_boolean(ctx, tickedBox->GetValue());
                duk_put_prop_string(ctx, -2, "value");
                }
            else if (elementType == "tickList"){
                wxCheckListBox *tickListBox = wxDynamicCast(window->FindWindowById(it->itemID), wxCheckListBox);
                duk_push_array(ctx);
                for (int j = 0, k = 0; j < tickListBox->GetCount(); j++){
                    if (tickListBox->IsChecked(j)){
                        duk_push_string(ctx, tickListBox->GetString(j));
                        duk_put_prop_index(ctx, -2, k++);
                        }
                    }
                duk_put_prop_string(ctx, -2, "value");
                }
            else if (elementType == "radio"){
                wxRadioBox* radioBox;
                duk_push_string(ctx, it->label);
                duk_put_prop_string(ctx, -2, "label");
                radioBox = wxDynamicCast(window->FindWindowById(it->itemID), wxRadioBox);
                theData = radioBox->GetString(radioBox->GetSelection());
                duk_push_string(ctx, theData);
                duk_put_prop_string(ctx, -2, "value");
                }
            else if (elementType == "button"){
                duk_push_string(ctx, button->GetLabel());
                duk_put_prop_string(ctx, -2, "label");
                }
            else JS_dk_error(ctx, "OCPNonDialog error: onButton found unexpected type " + elementType);
            duk_put_prop_index(ctx, -2, i);
            }
        JS_control.m_dialog.dialogElementsArray.clear();
#endif
#ifdef IN_HARNESS
        cout << "Done onButton\n";
//        MAYBE_DUK_DUMP
#endif  // IN_HARNESS
#if 0
        window->Destroy();
        JS_control.m_dialog.pdialog = nullptr;
        JS_exec(ctx);
        }
    else JS_control.m_pJSconsole->m_Output->AppendText("JavaScript onDialogue data validation failed\n");
    duk_pop(ctx);
    if (!JS_control.waiting()) {
        JS_control.clear();
        }
#endif
    }

duk_ret_t duk_dialog(duk_context *ctx) {  // provides wxWidgets dialogue
    int i;
    wxString elementType, value;
    dialogElement anElement;
    DialogAction dialogAction;
    DialogAction* p_dialogAction;
    bool hadButton {false};
    wxArrayString strings;
    std::vector<dialogElement> dialogElementArray;  // will be array of the elements for this call
    
#ifdef IN_HARNESS
    cout << "In duk_dialog\n";
    MAYBE_DUK_DUMP
#endif  // IN_HARNESS

    if (duk_get_top(ctx) != 2) JS_dk_error(ctx, "OCPNonDialog error: requires two arguments");
    duk_require_function(ctx, -2);  // first arugment must be function
 //   if ( JS_control.m_dialog.functionName != wxEmptyString) JS_dk_error(ctx, "OCPNonDialog error: called with another dialogue active");
    
    // create the dialog
    wxDialog *dialog = new wxDialog(NULL,  wxID_ANY, _("JavaScript dialogue"), wxDefaultPosition /*JS_control.m_dialog.position*/, wxDefaultSize,
            wxRESIZE_BORDER | wxCAPTION | wxSTAY_ON_TOP);
    p_dialogAction = new DialogAction;
    p_dialogAction->pdialog = dialog;    // save pointer to dialog in this dialog action
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);  // A top-level sizer
    dialog->SetSizer(topSizer);
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL); // A second box sizer to give more space around the controls
    topSizer->Add(boxSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    // work through the supplied structure
    int elements = (int) duk_get_length(ctx, -1);   // number of elements supplied for this dialog
//    JS_control.m_dialog.dialogElementsArray.clear();    // clear out any previous incomplete stuff
    p_dialogAction->dialogElementsArray.reserve(elements);    // reserve our space
    for (i = 0; i < elements; i++){
        anElement.label = wxEmptyString;
        anElement.value = wxEmptyString;
        anElement.textValue = wxEmptyString;
        anElement.width = 100;  // default size
        anElement.suffix = wxEmptyString;
        anElement.itemID = 0;
        duk_get_prop_index(ctx, -1, i);
        if (duk_get_prop_literal(ctx, -1, "type") == 0){
                JS_dk_error(ctx, wxString::Format(_("OCPNonDialog error: array index %i does not have type property"), i));
                continue;
                }
                elementType = duk_get_string(ctx, -1);
                anElement.type = elementType;
                duk_pop(ctx);
            if (elementType == _("caption")){
                duk_get_prop_literal(ctx, -1, "value");
                value = duk_get_string(ctx, -1);
                duk_pop_2(ctx);     // pop off the value string and the element
                dialog->SetTitle(value);
                anElement.value = value;
                p_dialogAction->dialogElementsArray.push_back(anElement);
                }
            else if (elementType == _("text")){
                duk_get_prop_literal(ctx, -1, "value");
                value = duk_get_string(ctx, -1);
                duk_pop_2(ctx);     // pop off the value string and the element
                anElement.value = value;
                p_dialogAction->dialogElementsArray.push_back(anElement);
                wxStaticText* staticText = new wxStaticText( dialog, wxID_STATIC, value, wxDefaultPosition, wxDefaultSize, 0 );
                boxSizer->Add(staticText, 0, wxALIGN_LEFT|wxALL, 5);
                }
            else if (elementType == _("field")){
                wxString textValue, label, suffix;
                wxTextCtrl* textCtrl;
                if (duk_get_prop_literal(ctx, -1, "label"))label = duk_get_string(ctx, -1);
                else label = wxEmptyString;
                duk_pop(ctx);
                if (duk_get_prop_literal(ctx, -1, "value")){
                    textValue = duk_get_string(ctx, -1);
                    }
                else textValue = wxEmptyString;
                duk_pop(ctx);
                if (duk_get_prop_literal(ctx, -1, "width")){
                    anElement.width = duk_get_number(ctx, -1);
                    }
                duk_pop(ctx);

                if (duk_get_prop_literal(ctx, -1, "suffix")){
                    suffix = duk_get_string(ctx, -1);
                    }
                else suffix = wxEmptyString;
                duk_pop_2(ctx);
                anElement.label = label;
                anElement.textValue = textValue;
                anElement.suffix = suffix;
                anElement.itemID = wxNewId();
                p_dialogAction->dialogElementsArray.push_back(anElement);
                wxBoxSizer* fieldBox = new wxBoxSizer(wxHORIZONTAL);
                boxSizer->Add(fieldBox, 0, wxGROW|wxALL, 5);
                boxSizer->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2); // Label for the text control
                wxStaticText* staticText = new wxStaticText( dialog, wxID_STATIC, label, wxDefaultPosition, wxDefaultSize, 0 );
                fieldBox->Add(staticText, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
                textCtrl = new wxTextCtrl ( dialog, anElement.itemID, wxT(""), wxDefaultPosition, wxSize(anElement.width, 22), 0);
                fieldBox->Add(textCtrl, 0, wxGROW|wxALL, 0);
//                textCtrl->SetValidator(wxTextValidator(wxFILTER_NONE, p_dialogAction->dialogElementsArray[i].textValue));
                
                staticText = new wxStaticText( dialog, wxID_STATIC, suffix, wxDefaultPosition, wxDefaultSize, 0 );
                fieldBox->Add(staticText, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
                
                }
            else if (elementType == _("tick")){
                if (!duk_get_prop_literal(ctx, -1, "value")){
                    JS_dk_error(ctx, "OCPNonDialog error: tick requires value");
                    }
                else {
                    bool ticked = false;
                    value = duk_get_string(ctx, -1);
                    if (value.GetChar(0) == '*'){   // if first char is *, remove and pre-tick this
                        value.Remove(0, 1);
                        ticked = true;
                        }
                    anElement.value = value;
                    anElement.itemID = wxNewId();
                    p_dialogAction->dialogElementsArray.push_back(anElement);
                    wxCheckBox *checkBox = new wxCheckBox(dialog, anElement.itemID, value, wxDefaultPosition, wxDefaultSize, 0 );
                    checkBox->SetValue(ticked);
                    boxSizer->Add(checkBox, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
                    }
                duk_pop_2(ctx);    // pop off the value string and the element
                }
            else if (elementType == _("tickList")){
                if (!duk_get_prop_literal(ctx, -1, "value")){
                    JS_dk_error(ctx, "OCPNonDialog error: tickList requires value");
                    }
                else {
                    if (!duk_is_array(ctx, -1)) JS_dk_error(ctx, "OCPNonDialog error: tickLit requires value array");
                    else {
                       strings.Clear();
                        int listLength = (int) duk_get_length(ctx, -1);
                        for (int j = 0; j < listLength; j++) {
                            duk_get_prop_index(ctx, -1, j);
                            value = duk_get_string(ctx,-1);
                            duk_pop(ctx);
                            strings.Add(value);
                            }
                        duk_pop_2(ctx);
                        anElement.itemID = wxNewId();
                        wxCheckListBox *tickListBox =  new wxCheckListBox(dialog, anElement.itemID, wxDefaultPosition, wxSize(180, listLength*22),
                                    strings, wxLB_EXTENDED);
                        p_dialogAction->dialogElementsArray.push_back(anElement);
                        boxSizer->Add(tickListBox, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
                        tickListBox->Fit();
                        cout << "Done tickList\n";
                        }
                    }
                }
            else if (elementType == _("radio")){
                wxString label;
                wxRadioBox *radioBox;
                int numberOfButtons = (int) duk_get_length(ctx, -1);
                strings.Clear();
                if (duk_get_prop_literal(ctx, -1, "label")){
                    label = duk_get_string(ctx, -1);
                    }
                else label = wxEmptyString;
                duk_pop(ctx);
                if (!duk_get_prop_literal(ctx, -1, "value")) JS_dk_error(ctx, "OCPNonDialog error: radio requires value");
                else {
                    if (!duk_is_array(ctx, -1)) JS_dk_error(ctx, "OCPNonDialog error: radio requires value array");
                    else {
                        numberOfButtons = (int) duk_get_length(ctx, -1);
                        numberOfButtons = numberOfButtons>50 ? 50: numberOfButtons; // place an upper limit
                        int maxChars = 0;
                        for (int j = 0; j < numberOfButtons; j++) {
                            duk_get_prop_index(ctx, -1, j);
                            value = duk_get_string(ctx,-1);
                            duk_pop(ctx);
                            strings.Add(value);
                            if (value.Length() > maxChars) maxChars = (int) value.Length();
                            }
                        duk_pop_2(ctx);
                        anElement.itemID = wxNewId();
                        radioBox = new wxRadioBox(dialog, anElement.itemID,label, wxDefaultPosition, wxSize(maxChars*9+5, numberOfButtons*23+10), strings, 1, wxRA_SPECIFY_COLS);
                        boxSizer->Add(radioBox, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);
                        anElement.label = label;
                        p_dialogAction->dialogElementsArray.push_back(anElement);
                        }
                    }
                }
            else if (elementType == _("hLine")){
                anElement.type = elementType;
                p_dialogAction->dialogElementsArray.push_back(anElement);
                wxStaticLine* line = new wxStaticLine ( dialog, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
                boxSizer->Add(line, 0, wxGROW|wxALL, 5);
                duk_pop(ctx);
                }
            else if (elementType == _("button")){
                bool defaultButton;
                if (hadButton) JS_dk_error(ctx, "OCPNonDialog error: multiple button elements not allowed");
                else {
                    wxString label;
                    wxButton *button;
                    wxBoxSizer* buttonBox;
                    hadButton = true;
                    buttonBox = new wxBoxSizer(wxHORIZONTAL);
                    boxSizer->Add(buttonBox, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);
                    duk_get_prop_literal(ctx, -1, "label");
                    if (duk_is_array(ctx, -1)){
                        // have array of buttons
                        int numberOfButtons = (int) duk_get_length(ctx, -1);
                        for (int j = 0; j < numberOfButtons; j++) {
                            defaultButton = false;
                            duk_get_prop_index(ctx, -1, j);
                            label = duk_get_string(ctx,-1);
                            duk_pop(ctx);
                            if (label.GetChar(0) == '*'){   // if first char is *, remove and make this default button
                                label.Remove(0, 1);
                                defaultButton = true;
                                }
                            anElement.itemID = wxNewId();
                            button = new wxButton ( dialog, anElement.itemID, label, wxDefaultPosition, wxDefaultSize, 0 );
                            if (defaultButton) button->SetDefault();
                            buttonBox->Add(button, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);
                            }
                        duk_pop_2(ctx);
                        }
                    else {
                        label = duk_get_string(ctx, -1);
                        if (label.GetChar(0) == '*'){   // if first char is *, remove and make this default button
                            label.Remove(0, 1);
                            defaultButton = true;
                            }
                        else defaultButton = false;
                        anElement.itemID = wxNewId();
                        button = new wxButton ( dialog, anElement.itemID, label, wxDefaultPosition, wxDefaultSize, 0 );
                        buttonBox->Add(button, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
                        if (defaultButton) button->SetDefault();
                        duk_pop_2(ctx);     // pop off the text string and the element
                         }
                    anElement.label = label;
                    p_dialogAction->dialogElementsArray.push_back(anElement);
                    dialog->Bind(wxEVT_BUTTON, &onButton, wxID_ANY);
                    }
                }
            else {
                JS_dk_error(ctx, "dialogue - unsupported type: " + elementType);
                }
            }
    if (!hadButton) {
        // caller has not upplied a button - create a default one
        wxBoxSizer* buttonBox = new wxBoxSizer(wxHORIZONTAL);
        boxSizer->Add(buttonBox, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
        wxButton* button = new wxButton ( dialog, wxNewId(), _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
        buttonBox->Add(button, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
        dialog->Bind(wxEVT_BUTTON, &onButton, wxID_ANY);
        }
    p_dialogAction->functionName = JS_control.getFunctionName();
    duk_pop_2(ctx); // pop off both call arguments
    duk_push_boolean(ctx, true);
    JS_control.m_JSactive = true;
    dialog->Fit();
    JS_control.m_dialogsActionsArray.Add(p_dialogAction); // add this action to the array of actions
    dialog->Show(true);

//  dump of array of dialogActions
    for (unsigned int i = 0; i <  JS_control.m_dialogsActionsArray.GetCount(); i++){
        cout << "dialoActionArray[" << i << "]\n";
        dialogAction = JS_control.m_dialogsActionsArray[i];
        std::vector<dialogElement>::const_iterator it;
        int k;
//        k = dialogAction.dialogElementsArray.size();
        for (k = 0, it = dialogAction.dialogElementsArray.begin(); it != dialogAction.dialogElementsArray.end(); k++, it++ ){
            cout << k << " " << dialogAction.dialogElementsArray.at(k).type << " " << dialogAction.dialogElementsArray.at(k).itemID << "\n";
            }
        }


    return 1;
    }
