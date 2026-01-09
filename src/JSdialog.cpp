/*************************************************************************
 * Project:  OpenCPN
 * Purpose:  JavaScript Plugin
 * Author:   Tony Voss
 *
 * Copyright Ⓒ 2025 by Tony Voss
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
#include "JavaScriptgui_impl.h"
#include "wx/radiobox.h"
#include "wx/listbox.h"
#include "wx/choice.h"
#include "wx/gdicmn.h"
#include "jsDialog.h"

 wxString getStringFromDuk(duk_context *ctx){
     // gets a string safely from top of duk stack and fixes º-symbol for Windose
    wxString JScleanOutput(wxString given);
     wxString string = wxString(duk_to_string(ctx, -1));
     string = JScleanOutput(string);	// internally, we are using DEGREE to represent degree - convert any back
     return string;
     }
     
 void buildDialogueResponse(duk_context *ctx, Console *pConsole, wxWindow *window, jsButton *button, std::shared_ptr<callbackEntry> pEntry){
	//Given a callback entry, builds the dialogue to be returned and leaves it on the duktape stack
	wxString JScleanString(wxString given);
    wxString elementType, theData;
//duk_push_string(ctx, "Dummy response");
//return;
	duk_push_array(ctx);
	for (int i = 0; i < pEntry->dialogue->dialogElementsArray.size(); i++){
		TRACE(1011, wxString::Format("Starting on handling element iteration %d  dukstack: %s", i, pConsole->dukDump())); 
		dialogElement element = pEntry->dialogue->dialogElementsArray[i];
		elementType = element.type;
		duk_push_object(ctx);
		duk_push_string(ctx, elementType);
		duk_put_prop_string(ctx, -2, "type");
		if ((elementType == "caption") || (elementType == "text") || (elementType == "hLine")){          
			duk_push_string(ctx, JScleanString(element.stringValue));
			duk_put_prop_string(ctx, -2, "value");             
			}
		else if (elementType == "field"){
			duk_push_string(ctx, JScleanString(element.label));
			duk_put_prop_string(ctx, -2, "label");
			wxTextCtrl* textCtrl = wxDynamicCast(window->FindWindow(element.itemID), wxTextCtrl);
			if (textCtrl == nullptr){
				pConsole->message(STYLE_RED, "Plugin logic error: textCtrl is nullptr\n");
				return;
				}
			wxString textValue = textCtrl->GetValue();
			TRACE(1011,  wxString::Format("textValue: %s", textValue));
			duk_push_string(ctx, JScleanString(textValue));
			duk_put_prop_string(ctx, -2, "value");
			if (element.suffix != wxEmptyString){
				duk_push_string(ctx, JScleanString(element.suffix));
				duk_put_prop_string(ctx, -2, "suffix");
				}
			}
		else if (elementType == "tick"){
			wxCheckBox *tickedBox;
			tickedBox = wxDynamicCast(window->FindWindowById(element.itemID), wxCheckBox);
			duk_push_boolean(ctx, tickedBox->GetValue());
			duk_put_prop_string(ctx, -2, "value");
			}
		else if (elementType == "tickList"){
			wxCheckListBox *tickListBox = wxDynamicCast(window->FindWindowById(element.itemID), wxCheckListBox);
			duk_push_array(ctx);
			for (unsigned int j = 0, k = 0; j < tickListBox->GetCount(); j++){
				if (tickListBox->IsChecked(j)){
					duk_push_string(ctx, JScleanString(tickListBox->GetString(j)));
					duk_put_prop_index(ctx, -2, k++);
					}
				}
			duk_put_prop_string(ctx, -2, "value");
			}
		else if (elementType == "radio"){
			wxRadioBox* radioBox;
			duk_push_string(ctx, element.label);
			duk_put_prop_string(ctx, -2, "label");
			radioBox = wxDynamicCast(window->FindWindowById(element.itemID), wxRadioBox);
			theData =JScleanString(radioBox->GetString(radioBox->GetSelection()));
			duk_push_string(ctx, theData);
			duk_put_prop_string(ctx, -2, "value");
			}
		else if (elementType == "choice"){
			wxChoice* choice;
			choice = wxDynamicCast(window->FindWindowById(element.itemID), wxChoice);
			theData = JScleanString(choice->GetString(choice->GetSelection()));
			duk_push_string(ctx, theData);
			duk_put_prop_string(ctx, -2, "value");
			}
		else if (elementType == "slider"){
			wxSlider* slider;
			duk_push_string(ctx, JScleanString(element.label));
			duk_put_prop_string(ctx, -2, "label");
			slider = wxDynamicCast(window->FindWindowById(element.itemID), wxSlider);
			duk_push_number(ctx, slider->GetValue());
			duk_put_prop_string(ctx, -2, "value");
			}
		else if (elementType == "spinner"){
			wxSpinCtrl* spinner;
			duk_push_string(ctx, JScleanString(element.label));
			duk_put_prop_string(ctx, -2, "label");
			spinner = wxDynamicCast(window->FindWindowById(element.itemID), wxSpinCtrl);
			duk_push_number(ctx, spinner->GetValue());
			duk_put_prop_string(ctx, -2, "value");
			}
		else if (elementType == "button"){
			duk_push_string(ctx, JScleanString(button->GetLabel()));
			duk_put_prop_string(ctx, -2, "label");
			}
		else if ((elementType == "position")|| (elementType == "colours")){
			// omit details
			}
		else {
			pConsole->prep_for_throw(ctx, "onDialog error: onButton found unexpected type " + elementType);
			duk_throw(ctx);
			}
		duk_put_prop_index(ctx, -2, i);
		}
	}

void onButton(wxCommandEvent & event){  // here when any dialogue button clicked ****************************    
    jsButton* button = wxDynamicCast(event.GetEventObject(), jsButton);
    Console* pConsole = button->pConsole;
    wxWindow* window = button->GetParent();
    duk_context* ctx = pConsole->mpCtx;
    if (ctx == nullptr){
        pConsole->message(STYLE_RED, "Plugin logic error: onButton invoked with no ctx context\n");
        JsDialog* pTheDialog = wxDynamicCast(window, JsDialog);
        pTheDialog->Destroy();
        return;
        }
    std::shared_ptr<callbackEntry> pEntry = pConsole->getCallbackEntry(button->entryId, false);	// find the callback entry
    TRACE(68, wxString::Format("Handle dialogue button - got entry pEntry->persistant is %s length of mCallbacks:%d", (pEntry->persistant?"true":"false"), pConsole->mCallbacks.size()));
    if (!pEntry->dialogue->customPosition) pConsole->mDialogPosition = pConsole->ToDIP(window->GetPosition());
    if ( window->Validate()){
    	buildDialogueResponse(ctx, pConsole, window, button, pEntry);  
        // now to add extra element for clicked-on button
        duk_push_object(ctx);
        duk_push_string(ctx, button->GetLabel());
        duk_put_prop_string(ctx, -2, "label");
        duk_put_prop_index(ctx, -2, pEntry->dialogue->dialogElementsArray.size()/* + 1*/); // add extra element in built array
        JsDialog* dialogue = wxDynamicCast(window, JsDialog);
        if (dialogue->IsModal()){
        	// The response is on the duktape stack
        	dialogue->EndModal(0);	// Over to createDialogue()
        	return;
        	}
        duk_push_string(ctx, button->GetLabel());	// add the button label as a second argument
//		if (!pEntry->persistant) pConsole->cleanOutEntry(pEntry);	// only clean out if we want to get rid of the dialogue
        Completions result = pConsole->executeCallableNargs(pEntry->func_heapptr, 2);
        TRACE(1011,pConsole->mConsoleName + " Button processing - back from handler");
        if (!pConsole->isBusy()) pConsole->wrapUp(result);
//        if (result != MORETODO) pConsole->wrapUp(result);
        }
    else {
//    	pConsole->cleanOutEntry(pEntry);
    	pConsole->prep_for_throw(ctx, "JavaScript onDialogue data validation failed");
    	duk_throw(ctx);
    	}
    }

// create the dialog  *********************************
void createDialogue(duk_context *ctx, int type){
	// type of call: 0 single shot callback, +1 persistant callback, -1 modal
	Console* findConsoleByCtx(duk_context *ctx);
	void throwErrorByCtx(duk_context *ctx, wxString message);
	void cancelCallbackPerCtx(duk_context *ctx, Console* pConsole, CallbackType type, wxString callbackName);
	
	duk_idx_t nargs = duk_get_top(ctx);   // number of arguments in call
	Console* pConsole = findConsoleByCtx(ctx);
	if (type == -1){	// is modal set up
		if ((nargs != 1) || !duk_is_array(ctx, 0)) throwErrorByCtx(ctx, "modalDialogue invalid call");
		}
	else {	// not modal
		if (pConsole->mStatus.test(INEXIT)) throwErrorByCtx(ctx, "onDialogue within onExit");
		if (!duk_is_callable(ctx, 0)){	// this is a cancel
			cancelCallbackPerCtx(ctx, pConsole, CB_DIALOGUE, "onDialogue");
			return;
			}
		else if (nargs != 2) {
			pConsole->prep_for_throw(ctx, "onDialog error: creating dialogue requires two arguments");
			duk_throw(ctx);
			}  
		}
	extern JavaScript_pi* pJavaScript_pi;
	// create the dialogue
	std::shared_ptr<callbackEntry> pEntry = pConsole->newCallbackEntry(CB_DIALOGUE);
	pEntry->func_heapptr = duk_get_heapptr(ctx, 0);
	pEntry->persistant = (type == 1)? true:false;
    int i;
    wxString elementType, value;
    bool hadButton {false};
    wxArrayString strings;
    wxColour textColour = *wxRED;
 
    // ready to create new dialogue
//-----------------------
    JsDialog* dialog = new JsDialog(pJavaScript_pi->m_parent_window,  wxID_ANY, "JavaScript dialogue",
    	 pConsole->FromDIP(pConsole->mDialogPosition), wxDefaultSize,
    	wxRESIZE_BORDER | wxCAPTION | wxSTAY_ON_TOP);
	pEntry->dialogue = std::unique_ptr<JsDialog>(dialog);
    double scale = SCALE(dialog);
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);  // A top-level sizer
    dialog->SetSizer(topSizer);
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL); // A second box sizer to give more space around the controls
    topSizer->Add(boxSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    // work through the supplied structure
    int elements = (int) duk_get_length(ctx, -1);   // number of elements supplied for this dialog      
    for (i = 0; i < elements; i++){
		TRACE(1011, wxString::Format("Starting on element iteration %d  dukstack: %s", i, pConsole->dukDump()));  
		dialogElement anElement;
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
        if (duk_get_prop_literal(ctx, -1, "type") == 0){
            pConsole->prep_for_throw(ctx, wxString::Format("onDialog error: array index %i does not have type property", i));
            duk_throw(ctx);
            }
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
            if (duk_get_prop_literal(ctx, -1, "font")) {
            	wxString fontName;
            	fontName = duk_get_string(ctx, -1);
            	if (fontName == "monospace") font.SetFamily(wxFONTFAMILY_TELETYPE);
            	else font.SetFaceName(fontName);
            	}
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
        if (elementType == "caption"){
            duk_get_prop_literal(ctx, -1, "value");
            value = getStringFromDuk(ctx);
            duk_pop_2(ctx);     // pop off the value string and the element
            dialog->SetTitle(value);
            anElement.stringValue = value;
            }
        else if (elementType == "text"){
            duk_get_prop_literal(ctx, -1, "value");
            value = getStringFromDuk(ctx);
            duk_pop_2(ctx);     // pop off the value string and the element
            TRACE(1011, wxString::Format("text value: %s", value));
            anElement.stringValue = value;
            wxStaticText* staticText = new wxStaticText( dialog, wxID_STATIC, value, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
            staticText->SetFont(font);
//            staticText->SetForegroundColour(textColour);
            boxSizer->Add(staticText, 0, wxALIGN_LEFT|wxALL, 5);
            }
        else if (elementType == "field"){
            wxString textValue, label, suffix;
            wxTextCtrl* textCtrl;
            if (duk_get_prop_literal(ctx, -1, "label")){
                label = getStringFromDuk(ctx) + " ";
                }
            else label = wxEmptyString;
            duk_pop(ctx);
            if (duk_get_prop_literal(ctx, -1, "value")){
                textValue = getStringFromDuk(ctx);
                }
            else textValue = wxEmptyString;
            duk_pop(ctx);
            TRACE(1011, wxString::Format("field value: %s", textValue));
            if (duk_get_prop_literal(ctx, -1, "width")){
                anElement.width = duk_get_number(ctx, -1);
                }
            duk_pop(ctx);
            if (duk_get_prop_literal(ctx, -1, "height")){
                anElement.height = duk_get_number(ctx, -1);
                }
            duk_pop(ctx);
            if (duk_get_prop_literal(ctx, -1, "multiLine")){
                anElement.multiLine = duk_get_boolean(ctx, -1)?(wxTE_MULTILINE | wxTE_BESTWRAP):0;
                }
            duk_pop(ctx);
            if (duk_get_prop_literal(ctx, -1, "suffix")){
                suffix = " " + getStringFromDuk(ctx);
                }
            else suffix = wxEmptyString;
            duk_pop(ctx);
            anElement.label = label;
            anElement.textValue = textValue;
            anElement.suffix = suffix;
            anElement.itemID = wxNewId();
            wxBoxSizer* fieldBox = new wxBoxSizer(wxHORIZONTAL);
            boxSizer->Add(fieldBox, 0, wxGROW|wxALL, 5*scale);
            wxStaticText* staticText = new wxStaticText( dialog, wxID_STATIC, label, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
            staticText->SetFont(font);
            fieldBox->Add(staticText, 0, wxALIGN_LEFT|wxALIGN_CENTER_HORIZONTAL, 0);
            TRACE(1011, wxString::Format("anElement.textValue: %s", anElement.textValue));
            textCtrl = new wxTextCtrl ( dialog, anElement.itemID, anElement.textValue, wxDefaultPosition, pConsole->FromDIP(wxSize(anElement.width,
            	anElement.height)), anElement.multiLine);
            fieldBox->Add(textCtrl, 0, wxGROW|wxALL, 0);
            staticText = new wxStaticText( dialog, wxID_STATIC, suffix, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
            staticText->SetFont(font);
            fieldBox->Add(staticText, 0, wxALIGN_LEFT|wxALIGN_CENTER_HORIZONTAL, 0);
            
            // look for field styling which overrides the main styling
            if (duk_get_prop_literal(ctx, -1, "fieldStyle")){
                if (duk_get_prop_literal(ctx, -1, "italic")){
                    if (duk_get_boolean(ctx, -1)) font.SetStyle(wxFONTSTYLE_ITALIC);
                    else font.SetStyle(wxFONTSTYLE_NORMAL);
                    }  duk_pop(ctx);
                if (duk_get_prop_literal(ctx, -1, "underline"))
                    font.SetUnderlined(duk_get_boolean(ctx, -1));
                	duk_pop(ctx);
                if (duk_get_prop_literal(ctx, -1, "bold")){
                    if (duk_get_boolean(ctx, -1)) font.SetWeight(wxFONTWEIGHT_BOLD);
                    else font.SetWeight(wxFONTWEIGHT_NORMAL);
                    } duk_pop(ctx);
                } duk_pop(ctx);   // pop off the style
            textCtrl->SetFont(font);    // override for the field
            duk_pop(ctx);
            }
        else if (elementType == "tick"){
            if (!duk_get_prop_literal(ctx, -1, "value")){
/*            
                pConsole->prep_for_throw(ctx, "onDialog error: tick requires value");
                duk_throw(ctx);
*/
				THROWCONSOLE("onDialog error: tick requires value");
                }
            else {
                bool ticked = false;
                value = getStringFromDuk(ctx);
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
        else if (elementType == "tickList"){
            if (!duk_get_prop_literal(ctx, -1, "value")){
                pConsole->prep_for_throw(ctx, "onDialog error: tickList requires value");
                duk_throw(ctx);
                }
            else {
                if (!duk_is_array(ctx, -1)) {
                	pConsole->prep_for_throw(ctx, "onDialog error: tickList requires value array");
                	duk_throw(ctx);                	
                	}
                else {
                    unsigned int maxChars = 0;
                    strings.Clear();
                    unsigned int listLength = duk_get_length(ctx, -1);
                    if (listLength < 1) {
                    	pConsole->prep_for_throw(ctx, "onDialog error: tickList has empty value array");
                    	duk_throw(ctx);
                    	}
                    for (unsigned int j = 0; j < listLength; j++) {
                        duk_get_prop_index(ctx, -1, j);
                        value = getStringFromDuk(ctx);
                        duk_pop(ctx);
                        strings.Add(value);
                        if (value.Length() > maxChars) maxChars = value.Length();
                        }
                    duk_pop(ctx);
                    anElement.itemID = wxNewId();
                    wxBoxSizer* checkListBoxBox = new wxBoxSizer(wxVERTICAL);
                    boxSizer->Add(checkListBoxBox);

                    // add label if we have one
                    if (duk_get_prop_literal(ctx, -1, "label")){
                        anElement.label = getStringFromDuk(ctx);
                        wxStaticText* label = new wxStaticText( dialog, wxID_STATIC, anElement.label, wxDefaultPosition,
                        	wxDefaultSize, wxALIGN_LEFT );
                        checkListBoxBox->Add(label, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
                        }
                    duk_pop(ctx);
                    
                    wxCheckListBox *checkListBox =  new wxCheckListBox(dialog, anElement.itemID, wxDefaultPosition,
                    	pConsole->FromDIP(wxSize((maxChars*10+75), listLength*22)), strings, wxLB_EXTENDED);
                    checkListBoxBox->Add(checkListBox, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
                    checkListBox->SetFont(font);
                    checkListBox->Fit();
                    duk_pop(ctx);
                    }
                }
            }
        else if (elementType == "slider"){
            int start, end;
            
            // range attribute
            if (!duk_get_prop_literal(ctx, -1, "range")){
                pConsole->prep_for_throw(ctx, "onDialog error: slider requires range");
                duk_throw(ctx);
                }
            if (!duk_is_array(ctx, -1) || (duk_get_length(ctx, -1) != 2)){
            	pConsole->prep_for_throw(ctx, "onDialog error: slider requires range with 2 values");
            	duk_throw(ctx);
            	}
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
            wxSlider *slider = new wxSlider(dialog, anElement.itemID, anElement.numberValue, start, end, wxDefaultPosition,
            	pConsole->FromDIP(wxSize(anElement.width,-1)), wxSL_HORIZONTAL|wxSL_AUTOTICKS|wxSL_LABELS);

            // add label if we have one
            if (duk_get_prop_literal(ctx, -1, "label")){
                anElement.label = getStringFromDuk(ctx);
                wxStaticText* label = new wxStaticText( dialog, wxID_STATIC, anElement.label, wxDefaultPosition, wxDefaultSize,
                	wxALIGN_LEFT );
                label->SetFont(font);
                sliderBox->Add(label, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
                }
            duk_pop(ctx);

            sliderBox->Add(slider, 0, wxGROW|wxALL, 0);
            boxSizer->Add(sliderBox,  0, wxGROW|wxALL, 5*scale);
            duk_pop(ctx);
            }
        else if (elementType == "spinner"){
            int start, end;
            
            // range attribute
            if (!duk_get_prop_literal(ctx, -1, "range")){
                pConsole->prep_for_throw(ctx, "onDialog error: spinner requires range");
                duk_throw(ctx);
                }
            if (!duk_is_array(ctx, -1) || (duk_get_length(ctx, -1) != 2)){
            	pConsole->prep_for_throw(ctx, "onDialog error: spinner requires range with 2 values");
            	duk_throw(ctx);
            	}
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
            wxSpinCtrl *spinner = new wxSpinCtrl(dialog, anElement.itemID, wxEmptyString, wxDefaultPosition,
            	pConsole->FromDIP(wxSize(anElement.width, 22)), wxSP_ARROW_KEYS, start, end, anElement.numberValue);

            // add label if we have one
            if (duk_get_prop_literal(ctx, -1, "label")){
                anElement.label = getStringFromDuk(ctx);
                wxStaticText* label = new wxStaticText( dialog, wxID_STATIC, anElement.label, wxDefaultPosition,
                	wxDefaultSize, wxALIGN_LEFT );
                label->SetFont(font);
                spinnerBox->Add(label, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
                }
            duk_pop(ctx);

            spinnerBox->Add(spinner, 0, wxGROW|wxALL, 0);
            boxSizer->Add(spinnerBox,  0, wxGROW|wxALL, 5*scale);
            duk_pop(ctx);
            }
        else if (elementType == "choice"){
            if (!duk_get_prop_literal(ctx, -1, "value")){
                pConsole->prep_for_throw(ctx, "onDialog error: choice requires value");
                duk_throw(ctx);
                }
            else {
                if (!duk_is_array(ctx, -1)){
                	 pConsole->prep_for_throw(ctx, "onDialog error: choice requires value array");
                	 duk_throw(ctx);
                	 }
                else {
                    duk_size_t maxChars = 0;
                    strings.Clear();
                    duk_size_t listLength = duk_get_length(ctx, -1);
                    if (listLength < 1) {
                    	pConsole->prep_for_throw(ctx, "onDialog error: choice has empty value array");
                    	duk_throw(ctx);
                    	}
                    for (duk_size_t j = 0; j < listLength; j++) {
                        duk_get_prop_index(ctx, -1, j);
                        value = getStringFromDuk(ctx);
                        duk_pop(ctx);
                        strings.Add(value);
                        if (value.Length() > maxChars) maxChars = (int) value.Length();
                        }
                    duk_pop(ctx);
                    anElement.itemID = wxNewId();
                    wxBoxSizer* choiceBox = new wxBoxSizer(wxVERTICAL);
                    boxSizer->Add(choiceBox);
                    wxChoice *choice =  new wxChoice(dialog, anElement.itemID, wxDefaultPosition, pConsole->FromDIP(wxSize((maxChars*9+45), listLength*22)), strings, wxCB_DROPDOWN);
//                    wxChoice *choice =  new wxChoice(dialog, anEle)ment.itemID, wxDefaultPosition,
//                    wxDefaultSize, strings, wxCB_DROPDOWN);
                    choiceBox->Add(choice, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5*scale);
                    duk_pop(ctx);
                    }
                }
            }
        else if (elementType == "radio"){
            wxString label;
            wxRadioBox *radioBox;
            duk_size_t numberOfButtons = duk_get_length(ctx, -1);
            strings.Clear();
            if (duk_get_prop_literal(ctx, -1, "label")){
                label = getStringFromDuk(ctx);
                }
            else label = wxEmptyString;
            duk_pop(ctx);
            if (!duk_get_prop_literal(ctx, -1, "value")){
            	pConsole->prep_for_throw(ctx, "onDialog error: radio requires value");
            	duk_throw(ctx);
            	}
            else {
                if (!duk_is_array(ctx, -1)){
                	pConsole->prep_for_throw(ctx, "onDialog error: radio requires value array");
                	duk_throw(ctx);
                	}
                else {
                    numberOfButtons = duk_get_length(ctx, -1);
                    if (numberOfButtons < 1) {
                    	pConsole->prep_for_throw(ctx, "onDialog error: radioButtons has empty value array");
                    	duk_throw(ctx);
                    	}
                    numberOfButtons = numberOfButtons>50 ? 50: numberOfButtons; // place an upper limit
                    unsigned int maxChars = 0;
                    duk_size_t defaultIndex = 0;
                    for (duk_size_t j = 0; j < numberOfButtons; j++) {
                        duk_get_prop_index(ctx, -1, j);
                        value = getStringFromDuk(ctx);
                        duk_pop(ctx);
                        if (value.GetChar(0) == '*'){   // if first char is *, remove and make this default button
							value.Remove(0, 1);
							defaultIndex = j;
							}
                        strings.Add(value);
                        if (value.Length() > maxChars) maxChars = (int) value.Length();
                        }
                    duk_pop_2(ctx);
                    anElement.itemID = wxNewId();
                    radioBox = new wxRadioBox(dialog, anElement.itemID,label, wxDefaultPosition, wxDefaultSize , strings, 1, wxRA_SPECIFY_COLS);
                    radioBox->SetSelection(defaultIndex);
                    boxSizer->Add(radioBox, 0, wxHORIZONTAL|wxALL, 2);
                    anElement.label = label;
                    }
                }
            }
        else if (elementType == "hLine"){
            anElement.type = elementType;
//			wxStaticLine gives a line so thin it is not visible.  Instead we create a narrow box using wxPanel
//          wxStaticLine* line = new wxStaticLine ( dialog, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
            wxPanel* line = new wxPanel(dialog, wxID_ANY, wxDefaultPosition, wxSize(-1, 2));
			line->SetBackgroundColour(*wxBLACK);
            boxSizer->Add(line, 0, wxGROW|wxALL, 5*scale);
            duk_pop(ctx);
            }
        else if (elementType == "button"){
            bool defaultButton;
            wxString label;
            jsButton *button;
            wxBoxSizer* buttonBox;
            hadButton = true;
            buttonBox = new wxBoxSizer(wxHORIZONTAL);
            boxSizer->Add(buttonBox, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);
            duk_get_prop_literal(ctx, -1, "label");
            if (duk_is_array(ctx, -1)){
                // have array of buttons
                int numberOfButtons = (int) duk_get_length(ctx, -1);
                if (numberOfButtons < 1){
                	pConsole->prep_for_throw(ctx, "onDialog error: buttons has empty labels array");
                	duk_throw(ctx);
                	}
                for (int j = 0; j < numberOfButtons; j++) {
                    defaultButton = false;
                    duk_get_prop_index(ctx, -1, j);
                    label = getStringFromDuk(ctx);
                    duk_pop(ctx);
                    if (label.GetChar(0) == '*'){   // if first char is *, remove and make this default button
                        label.Remove(0, 1);
                        defaultButton = true;
                        }
                    anElement.itemID = wxNewId();
                    button = new jsButton ( pConsole, dialog, anElement.itemID, label, wxDefaultPosition, wxDefaultSize, 0 );
                    button->entryId = pEntry->id;
                    button->SetFont(font);
                    if (defaultButton) button->SetDefault();
                    buttonBox->Add(button, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);
                    }
                duk_pop_2(ctx);
                }
            else {
                label = getStringFromDuk(ctx);
                if (label.GetChar(0) == '*'){   // if first char is *, remove and make this default button
                    label.Remove(0, 1);
                    defaultButton = true;
                    }
                else defaultButton = false;
                anElement.itemID = wxNewId();
                button = new jsButton ( pConsole, dialog, anElement.itemID, label, wxDefaultPosition, wxDefaultSize, 0 );
                button->entryId = pEntry->id;
                buttonBox->Add(button, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5*scale);
                if (defaultButton) button->SetDefault();
                duk_pop_2(ctx);     // pop off the text string and the element
                 }
            anElement.label = label;
            }
        else if (elementType == "position"){	// dialogue position specified      
        	duk_get_prop_literal(ctx, -1, "x");
        	int x = duk_get_number(ctx, -1);     	
        	duk_pop(ctx);
        	duk_get_prop_literal(ctx, -1, "y");
        	int y = duk_get_number(ctx, -1);  
        	duk_pop(ctx);
        	wxPoint position = wxPoint(x, y);
        	wxPoint checkPointOnScreen(wxPoint point);
        	position = checkPointOnScreen(position);
        	dialog->SetPosition(position);
        	pEntry->dialogue->customPosition = true;  
        	duk_pop(ctx);	// pop off the element     	
        	}
        else if (elementType == "colours"){
        	wxString colourName, errorMessage;        	
        	wxColour colour;
            wxColourDatabase cdb;
        	duk_get_prop_literal(ctx, -1, "text");
        	colourName = duk_get_string(ctx, -1);
        	duk_pop(ctx);       	
			colour = cdb.Find(colourName);
			// NB Because prep_for_throw unwinds the call stack, the error message using wxString::Format has to be prepared before the call 
			if (!colour.IsOk()){
				errorMessage =  wxString::Format("onDialogue - text colour: %s is invalid", colourName);
				pConsole->prep_for_throw(ctx, errorMessage);
				duk_throw(ctx);
				}
			dialog->SetForegroundColour(colour);
			duk_get_prop_literal(ctx, -1, "background");
        	colourName = duk_get_string(ctx, -1);
        	duk_pop(ctx);       	
			colour = cdb.Find(colourName);
			if (!colour.IsOk()){
				errorMessage =  wxString::Format("onDialogue - background colour: %s is invalid", colourName);
				pConsole->prep_for_throw(ctx, errorMessage);
				duk_throw(ctx);
				}
			dialog->SetBackgroundColour(colour);
			duk_pop(ctx);
        	}
        else {
            pConsole->prep_for_throw(ctx, "onDialogue - unsupported element type: " + elementType);
            duk_throw(ctx);
            }
        pEntry->dialogue->dialogElementsArray.push_back(anElement);
/*
duk_pop_n(ctx, nargs);
duk_push_string(ctx, "Run Rabbit run z");
return;
*/
        }
 
    if (!hadButton) {
        // caller has not upplied a button - create a default one
        wxBoxSizer* buttonBox = new wxBoxSizer(wxHORIZONTAL);
        boxSizer->Add(buttonBox, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5*scale);
        jsButton* button = new jsButton ( pConsole, dialog, wxNewId(), "OK", wxDefaultPosition, wxDefaultSize, 0 );
        button->entryId = pEntry->id;
        buttonBox->Add(button, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5*scale);
        }
/*
duk_pop_n(ctx, nargs);
duk_push_string(ctx, "Run Rabbit run 3");
return;
*/
//    pConsole->mCallbacks.push_back(pEntry);
    dialog->Bind(wxEVT_BUTTON, &onButton, wxID_ANY);    
    dialog->Fit();
    duk_pop_n(ctx, nargs);
    if (type >= 0){
		dialog->Show(true);
		pConsole->mWaitingCached = false;
		duk_push_number(ctx, pEntry->id);	// return the entry id
		}
	else { // modal
		pConsole->mModalCount++;
		dialog->ShowModal();
		// here when modal window dismissed - readback is on duktape stack which we return
		pConsole->mModalCount--;
		}
    return;
    }
    
    duk_ret_t duk_dialog(duk_context *ctx){
    	createDialogue(ctx, 0);
    	return 1;
    	}
    	
    duk_ret_t duk_dialog_persist(duk_context *ctx){
    	createDialogue(ctx, 1);
    	return 1;
    	}
    	
    duk_ret_t duk_dialog_modal(duk_context *ctx){
    	createDialogue(ctx, -1);
    	return 1;
    	}
    	
    duk_ret_t duk_read_dialog(duk_context *ctx){
    	void throwErrorByCtx(duk_context *ctx, wxString message);
    	Console* findConsoleByCtx(duk_context *ctx);
    	Console* pConsole = findConsoleByCtx(ctx);
    	std::shared_ptr<callbackEntry> pEntry = pConsole->getCallbackEntry(duk_get_number(ctx, 0), false);	// find the callback entry
    	duk_pop(ctx);
    	if (!pEntry){
    		pConsole->clearCallbacks();
    		throwErrorByCtx(ctx, "readDialogue with invalid callback id");
    		}
    	if (!pEntry->persistant) {
    		// we have removed the entry but it needs to endure, so put it back
    		pConsole->mCallbacks.push_back(pEntry);
    		}
    	JsDialog* dlg = pEntry->dialogue.get();        // from shared_ptr<JsDialog>
    	jsButton* button = new jsButton(pConsole, dlg, 0);	// create a dummy button
		buildDialogueResponse(ctx, pConsole, dlg, button, pEntry); // get dialogue as it is now onto duktape stack
		button->Destroy();
//		delete button;
		return 1;
		}
/*    
    	wxWindow* window = entry.dialogue->GetContentWindow();
    	jsButton* button = new jsButton(pConsole, window);
    	buildDialogueResponse(ctx, pConsole, window, button, entry);	// get dialogue as it is now onto duktape stack
    	delete button;
    	return 1;
    	}
*/
