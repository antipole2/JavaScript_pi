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

void onButton(wxCommandEvent & event){  // here when any dialogue button clicked ****************************
    duk_context *ctx;
    wxWindow *window;
    wxButton *button;
    wxString elementType;
    wxString theData, functionName;
    std::vector<dialogElement>::const_iterator it;
    int i;
    duk_bool_t JS_exec(duk_context *ctx);
    
#ifdef IN_HARNESS
    cout << "In onButton\n";
#endif  // IN_HARNESS
    ctx = JS_control.m_pctx;
    if (ctx == nullptr) JS_control.m_pJSconsole->m_Output->AppendText("onButton invoked with no ctx context\n");
    functionName = JS_control.m_dialog.functionName;
    if (!duk_get_global_string(ctx, functionName.c_str())){
        JS_control.display_error(ctx, _("JavaScript onDialogue function ") + functionName + " " + duk_safe_to_string(ctx, -1));
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
            duk_push_object(ctx);
            duk_push_string(ctx, elementType);
            duk_put_prop_string(ctx, -2, "type");
            if ((elementType == "caption") || (elementType == "text") || (elementType == "hLine")){
                duk_push_string(ctx, it->stringValue);
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
            else if (elementType == "slider"){
                wxSlider* slider;
                duk_push_string(ctx, it->label);
                duk_put_prop_string(ctx, -2, "label");
                slider = wxDynamicCast(window->FindWindowById(it->itemID), wxSlider);
                duk_push_number(ctx, slider->GetValue());
                duk_put_prop_string(ctx, -2, "value");
                }
            else if (elementType == "spinner"){
                wxSpinCtrl* spinner;
                duk_push_string(ctx, it->label);
                duk_put_prop_string(ctx, -2, "label");
                spinner = wxDynamicCast(window->FindWindowById(it->itemID), wxSpinCtrl);
                duk_push_number(ctx, spinner->GetValue());
                duk_put_prop_string(ctx, -2, "value");
                }
            else if (elementType == "button"){
                duk_push_string(ctx, button->GetLabel());
                duk_put_prop_string(ctx, -2, "label");
                }
            else JS_control.throw_error(ctx, "onDialog error: onButton found unexpected type " + elementType);
            duk_put_prop_index(ctx, -2, i);
            }
        // now to add extra element for clicked-on button
        duk_push_object(ctx);
        duk_push_string(ctx, button->GetLabel());
        duk_put_prop_string(ctx, -2, "label");
        duk_put_prop_index(ctx, -2, i); // i will have been left one greater than length of array so this extends it
        
        JS_control.m_dialog.dialogElementsArray.clear();
#ifdef IN_HARNESS
        cout << "Done onButton\n";
#endif  // IN_HARNESS
        window->Destroy();
        JS_control.m_dialog.pdialog = nullptr;
		duk_bool_t ret = JS_exec(ctx);  // calls the user's function
		if (!ret || JS_control.m_exitScriptCalled){
			JS_control.m_runCompleted = true;
			JS_control.clearAndDestroy();
			return;
			}
        }
    else JS_control.m_pJSconsole->m_Output->AppendText("JavaScript onDialogue data validation failed\n");
    duk_pop(ctx);
    if (!JS_control.waiting()) {
        JS_control.clearAndDestroy();
        }
    }

// create the dialog  *********************************
duk_ret_t duk_dialog(duk_context *ctx) {  // provides wxWidgets dialogue
    int i;
    wxString elementType, value;
    dialogElement anElement;
    bool hadButton {false};
    wxArrayString strings;
    std::vector<dialogElement> dialogElementArray;  // will be array of the elements for this call

    if (duk_get_top(ctx) != 2) JS_control.throw_error(ctx, "onDialog error: requires two arguments");
    duk_require_function(ctx, -2);  // first arugment must be function
    if ( JS_control.m_dialog.functionName != wxEmptyString) JS_control.throw_error(ctx, "onDialog error: called with another dialogue active");
    
    wxDialog *dialog = new wxDialog(NULL,  wxID_ANY, _("JavaScript dialogue"), JS_control.m_dialog.position, wxDefaultSize,
            wxRESIZE_BORDER | wxCAPTION | wxSTAY_ON_TOP);
    JS_control.m_dialog.pdialog = dialog;    // save pointer to dialog
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);  // A top-level sizer
    dialog->SetSizer(topSizer);
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL); // A second box sizer to give more space around the controls
    topSizer->Add(boxSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    // work through the supplied structure
    int elements = (int) duk_get_length(ctx, -1);   // number of elements supplied for this dialog
    JS_control.m_dialog.dialogElementsArray.clear();    // clear out any previous incomplete stuff
    JS_control.m_dialog.dialogElementsArray.reserve(elements);    // reserve our space
    for (i = 0; i < elements; i++){
        anElement.label = wxEmptyString;
        anElement.stringValue = wxEmptyString;
        anElement.textValue = wxEmptyString;
        anElement.numberValue = 0;
        anElement.width = 100;  // default size
        anElement.height = 22;  // default height for field
        anElement.multiLine = false;
        anElement.suffix = wxEmptyString;
        anElement.itemID = 0;
        int fontSize = 12;    // default font size
        wxFont font = wxFontInfo(fontSize);
        duk_get_prop_index(ctx, -1, i);
        if (duk_get_prop_literal(ctx, -1, "type") == 0)
            JS_control.throw_error(ctx, wxString::Format(_("onDialog error: array index %i does not have type property"), i));

        elementType = duk_get_string(ctx, -1);
        anElement.type = elementType;
        duk_pop(ctx);
    
        // look for styling
        if (duk_get_prop_literal(ctx, -1, "style")){
            wxString fontName;
            if (duk_get_prop_literal(ctx, -1, "size")) {
                fontSize = duk_get_number(ctx, -1);
                font.SetPointSize(fontSize);
                } duk_pop(ctx);
            if (duk_get_prop_literal(ctx, -1, "font")) font.SetFaceName(duk_get_string(ctx, -1));
            duk_pop(ctx);
            if (duk_get_prop_literal(ctx, -1, "italic"))
                if (duk_get_boolean(ctx, -1)) font.MakeItalic();
            duk_pop(ctx);
            if (duk_get_prop_literal(ctx, -1, "underline"))
                if (duk_get_boolean(ctx, -1)) font.MakeUnderlined();
            duk_pop(ctx);
            if (duk_get_prop_literal(ctx, -1, "bold"))
                if (duk_get_boolean(ctx, -1)) font.MakeBold();
            duk_pop(ctx);
            } duk_pop(ctx);   // pop off the style
        if (elementType == _("caption")){
            duk_get_prop_literal(ctx, -1, "value");
            value = duk_get_string(ctx, -1);
            duk_pop_2(ctx);     // pop off the value string and the element
            dialog->SetTitle(value);
            anElement.stringValue = value;
            }
        else if (elementType == _("text")){
            duk_get_prop_literal(ctx, -1, "value");
            value = duk_get_string(ctx, -1);
            duk_pop_2(ctx);     // pop off the value string and the element
            anElement.stringValue = value;
            wxStaticText* staticText = new wxStaticText( dialog, wxID_STATIC, value, wxDefaultPosition, wxDefaultSize, 5 );
            staticText->SetFont(font);
            boxSizer->Add(staticText, 0, wxALIGN_LEFT|wxALL, 5);
            }
        else if (elementType == _("field")){
            wxString textValue, label, suffix;
            wxTextCtrl* textCtrl;
            if (duk_get_prop_literal(ctx, -1, "label")) label = duk_get_string(ctx, -1) + _(" ");
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
            if (duk_get_prop_literal(ctx, -1, "height")){
                anElement.height = duk_get_number(ctx, -1);
                }
            duk_pop(ctx);
            if (duk_get_prop_literal(ctx, -1, "multiLine")){
                anElement.multiLine = duk_get_boolean(ctx, -1)?wxTE_MULTILINE:0;
                }
            duk_pop(ctx);
            if (duk_get_prop_literal(ctx, -1, "suffix")){
                suffix = _(" ") + duk_get_string(ctx, -1);
                }
            else suffix = wxEmptyString;
            duk_pop(ctx);
            anElement.label = label;
            anElement.textValue = textValue;
            anElement.suffix = suffix;
            anElement.itemID = wxNewId();
            wxBoxSizer* fieldBox = new wxBoxSizer(wxHORIZONTAL);
            boxSizer->Add(fieldBox, 0, wxGROW|wxALL, 5);
            wxStaticText* staticText = new wxStaticText( dialog, wxID_STATIC, label, wxDefaultPosition, wxDefaultSize, 0 );
            staticText->SetFont(font);
            fieldBox->Add(staticText, 0, wxALIGN_LEFT|wxALIGN_CENTER_HORIZONTAL, 0);
            textCtrl = new wxTextCtrl ( dialog, anElement.itemID, wxT(""), wxDefaultPosition, wxSize(anElement.width,  anElement.height /* 6+fontSize */), anElement.multiLine);
            fieldBox->Add(textCtrl, 0, wxGROW|wxALL, 0);
            textCtrl->SetValidator(wxTextValidator(wxFILTER_NONE, &JS_control.m_dialog.dialogElementsArray[i].textValue));
            staticText = new wxStaticText( dialog, wxID_STATIC, suffix, wxDefaultPosition, wxDefaultSize, 0 );
            staticText->SetFont(font);
            fieldBox->Add(staticText, 0, wxALIGN_LEFT|wxALIGN_CENTER_HORIZONTAL, 0);
            
            // look for field styling which overrides the main styling
            if (duk_get_prop_literal(ctx, -1, "fieldStyle")){
                if (duk_get_prop_literal(ctx, -1, "font")) font.SetFaceName(duk_get_string(ctx, -1));
                duk_pop(ctx);
                if (duk_get_prop_literal(ctx, -1, "italic")){
                    if (duk_get_boolean(ctx, -1)) font.SetStyle(wxFONTSTYLE_ITALIC);
                    else font.SetStyle(wxFONTSTYLE_NORMAL);
                    }
                duk_pop(ctx);
                if (duk_get_prop_literal(ctx, -1, "underline"))
                    font.SetUnderlined(duk_get_boolean(ctx, -1));
                duk_pop(ctx);
                if (duk_get_prop_literal(ctx, -1, "bold")){
                    if (duk_get_boolean(ctx, -1)) font.SetWeight(wxFONTWEIGHT_BOLD);
                    else font.SetWeight(wxFONTWEIGHT_NORMAL);
                    }
                duk_pop(ctx);
                } duk_pop(ctx);   // pop off the style
            textCtrl->SetFont(font);    // overrid for the field
            duk_pop(ctx);
            }
        else if (elementType == _("tick")){
            if (!duk_get_prop_literal(ctx, -1, "value")){
                JS_control.throw_error(ctx, "onDialog error: tick requires value");
                }
            else {
                bool ticked = false;
                value = duk_get_string(ctx, -1);
                if (value.GetChar(0) == '*'){   // if first char is *, remove and pre-tick this
                    value.Remove(0, 1);
                    ticked = true;
                    }
                anElement.stringValue = value;
                anElement.itemID = wxNewId();
                wxCheckBox *checkBox = new wxCheckBox(dialog, anElement.itemID, value, wxDefaultPosition, wxDefaultSize, 0 );
                checkBox->SetValue(ticked);
                checkBox->SetFont(font);
                boxSizer->Add(checkBox, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
                }
            duk_pop_2(ctx);    // pop off the value string and the element
            }
        else if (elementType == _("tickList")){
            if (!duk_get_prop_literal(ctx, -1, "value")){
                JS_control.throw_error(ctx, "onDialog error: tickList requires value");
                }
            else {
                if (!duk_is_array(ctx, -1)) JS_control.throw_error(ctx, "onDialog error: tickLit requires value array");
                else {
                    int maxChars = 0;
                    strings.Clear();
                    int listLength = (int) duk_get_length(ctx, -1);
                    for (int j = 0; j < listLength; j++) {
                        duk_get_prop_index(ctx, -1, j);
                        value = duk_get_string(ctx,-1);
                        duk_pop(ctx);
                        strings.Add(value);
                        if (value.Length() > maxChars) maxChars = (int) value.Length();
                        }
                    duk_pop(ctx);
                    anElement.itemID = wxNewId();
                    wxBoxSizer* checkListBoxBox = new wxBoxSizer(wxVERTICAL);
                    boxSizer->Add(checkListBoxBox);

                    // add label if we have one
                    if (duk_get_prop_literal(ctx, -1, "label")){
                        anElement.label = duk_get_string(ctx, -1);

                        wxStaticText* label = new wxStaticText( dialog, wxID_STATIC, anElement.label, wxDefaultPosition, wxDefaultSize, 0 );
                        checkListBoxBox->Add(label, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
                        }
                    duk_pop(ctx);
                    
                    wxCheckListBox *checkListBox =  new wxCheckListBox(dialog, anElement.itemID, wxDefaultPosition, wxSize(maxChars*9+45, listLength*22), strings, wxLB_EXTENDED);
                    checkListBoxBox->Add(checkListBox, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
                    checkListBox->SetFont(font);
                    checkListBox->Fit();
                    
                    duk_pop(ctx);
                    }
                }
            }
        else if (elementType == _("slider")){
            int start, end;
            
            // range attribute
            if (!duk_get_prop_literal(ctx, -1, "range"))
                JS_control.throw_error(ctx, "onDialog error: slider requires range");
            if (!duk_is_array(ctx, -1) || (duk_get_length(ctx, -1) != 2)) JS_control.throw_error(ctx, "onDialog error: slider requires range with 2 values");
            duk_get_prop_index(ctx, -1, 0);
            start = duk_get_number(ctx, -1);
            duk_pop(ctx);
            duk_get_prop_index(ctx, -1, 1);
            end = duk_get_number(ctx, -1);
            duk_pop_2(ctx); // pop off end and range
            
            // value attribute
            if (duk_get_prop_literal(ctx, -1, "value")){
                anElement.numberValue = duk_get_number(ctx, -1);
                }
            else anElement.numberValue = (start+end)/2;    // default to mid-range
            duk_pop(ctx);
            
            // width attribute
            if (duk_get_prop_literal(ctx, -1, "width")){
                anElement.width = duk_get_number(ctx, -1);
                }
            else anElement.width = 300;
            duk_pop(ctx);
            
            anElement.itemID = wxNewId();
            wxBoxSizer* sliderBox = new wxBoxSizer(wxVERTICAL);
            wxSlider *slider = new wxSlider(dialog, anElement.itemID, anElement.numberValue, start, end, wxDefaultPosition, wxSize(anElement.width,-1), wxSL_HORIZONTAL|wxSL_AUTOTICKS|wxSL_LABELS);

            // add label if we have one
            if (duk_get_prop_literal(ctx, -1, "label")){
                anElement.label = duk_get_string(ctx, -1);

                wxStaticText* label = new wxStaticText( dialog, wxID_STATIC, anElement.label, wxDefaultPosition, wxDefaultSize, 0 );
                label->SetFont(font);
                sliderBox->Add(label, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
                }
            duk_pop(ctx);

            sliderBox->Add(slider, 0, wxGROW|wxALL, 0);
            boxSizer->Add(sliderBox,  0, wxGROW|wxALL, 5);
            duk_pop(ctx);
            }
        else if (elementType == _("spinner")){
            int start, end;
            
            // range attribute
            if (!duk_get_prop_literal(ctx, -1, "range"))
                JS_control.throw_error(ctx, "onDialog error: spinner requires range");
            if (!duk_is_array(ctx, -1) || (duk_get_length(ctx, -1) != 2)) JS_control.throw_error(ctx, "onDialog error: spinner requires range with 2 values");
            duk_get_prop_index(ctx, -1, 0);
            start = duk_get_number(ctx, -1);
            duk_pop(ctx);
            duk_get_prop_index(ctx, -1, 1);
            end = duk_get_number(ctx, -1);
            duk_pop_2(ctx); // pop off end and range
            
            // value attribute
            if (duk_get_prop_literal(ctx, -1, "value")){
                anElement.numberValue = duk_get_number(ctx, -1);
                }
            else anElement.numberValue = 0;
            duk_pop(ctx);
            
            // width attribute  // seems to have no effect - always expands to fill box
            if (duk_get_prop_literal(ctx, -1, "width")){
                anElement.width = duk_get_number(ctx, -1);
                }
            else anElement.width = 300;
            duk_pop(ctx);

            
            anElement.itemID = wxNewId();
            wxBoxSizer* spinnerBox = new wxBoxSizer(wxVERTICAL);
            wxSpinCtrl *spinner = new wxSpinCtrl(dialog, anElement.itemID, wxEmptyString, wxDefaultPosition, wxSize(anElement.width, 22), wxSP_ARROW_KEYS, start, end, anElement.numberValue);

            // add label if we have one
            if (duk_get_prop_literal(ctx, -1, "label")){
                anElement.label = duk_get_string(ctx, -1);

                wxStaticText* label = new wxStaticText( dialog, wxID_STATIC, anElement.label, wxDefaultPosition, wxDefaultSize, 0 );
                label->SetFont(font);
                spinnerBox->Add(label, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
                }
            duk_pop(ctx);

            spinnerBox->Add(spinner, 0, wxGROW|wxALL, 0);
            boxSizer->Add(spinnerBox,  0, wxGROW|wxALL, 5);
            duk_pop(ctx);
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
            if (!duk_get_prop_literal(ctx, -1, "value")) JS_control.throw_error(ctx, "onDialog error: radio requires value");
            else {
                if (!duk_is_array(ctx, -1)) JS_control.throw_error(ctx, "onDialog error: radio requires value array");
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
                    radioBox = new wxRadioBox(dialog, anElement.itemID,label, wxDefaultPosition, wxSize(maxChars*10+45, numberOfButtons*23+14), strings, 1, wxRA_SPECIFY_COLS);
                    boxSizer->Add(radioBox, 0, wxHORIZONTAL|wxALL, 2);
                    anElement.label = label;
                    }
                }
            }
        else if (elementType == _("hLine")){
            anElement.type = elementType;
            wxStaticLine* line = new wxStaticLine ( dialog, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
            boxSizer->Add(line, 0, wxGROW|wxALL, 5);
            duk_pop(ctx);
            }
        else if (elementType == _("button")){
            bool defaultButton;
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
                    button->SetFont(font);
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
            }
        else {
            JS_control.throw_error(ctx, "onDialogue - unsupported element type: " + elementType);
            }
        JS_control.m_dialog.dialogElementsArray.push_back(anElement);
        }
    if (!hadButton) {
        // caller has not upplied a button - create a default one
        wxBoxSizer* buttonBox = new wxBoxSizer(wxHORIZONTAL);
        boxSizer->Add(buttonBox, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
        wxButton* button = new wxButton ( dialog, wxNewId(), _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
        buttonBox->Add(button, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
        }
    dialog->Bind(wxEVT_BUTTON, &onButton, wxID_ANY);
    JS_control.m_dialog.functionName = JS_control.getFunctionName();
    JS_control.m_JSactive = true;
    duk_pop_2(ctx); // pop off both call arguments
    duk_push_boolean(ctx, true);
    dialog->Fit();
    dialog->Show(true);

/*
// for debug - dumps element array
 std::vector<dialogElement>::const_iterator it;
    int k;
    k = JS_control.m_dialog.dialogElementsArray.size();
    for (k = 0, it = JS_control.m_dialog.dialogElementsArray.begin(); it != JS_control.m_dialog.dialogElementsArray.end(); k++, it++ ){
        cout << k << " " << JS_control.m_dialog.dialogElementsArray.at(k).type << " " << JS_control.m_dialog.dialogElementsArray.at(k).itemID << "\n";
        }
 */

    return 1;
    }
