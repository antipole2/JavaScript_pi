/**************************************************************************
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
Provides the functions to handle timeout checks
 */

#include "duktape.h"
#include "ocpn_duk.h"

extern JS_control_class JS_control;

#define  JSDEFAULTTIMEOUT  1  /* seconds */

void JSduk_start_exec_timeout() {   // to be called before we start execution
    JS_control.m_timeout_check_counter = 0;
    JS_control.m_pcall_endtime =  wxGetUTCTimeMillis() + JS_control.m_time_to_allocate;
	}

void JSduk_clear_exec_timeout(void) {   // to cancel the timeout
    JS_control.m_pcall_endtime = 0;
}

duk_bool_t JSduk_timeout_check(void *udata) {

	(void) udata;  /* not needed */
    
#ifdef TIMEOUT_DUMP    // for diagnostics
        wxString buffer;
#if IN_HARNESS
        fprintf(stderr, "exec timeout check %ld: now=%lld, allocate=%ld, end=%lld, left=%lld\n",
                (long) JS_control.m_timeout_check_counter, wxGetUTCTimeMillis(), JS_control.m_time_to_allocate, JS_control.m_pcall_endtime, JS_control.m_pcall_endtime-wxGetUTCTimeMillis());
        fflush(stderr);
#else
        buffer.Printf("exec timeout check %ld: backingOut=%s, now=%lld, allocate=%ld, end=%lld, left=%lld\n",
                      (long) JS_control.m_timeout_check_counter, JS_control.m_backingOut?"true":"false", wxGetUTCTimeMillis(), JS_control.m_time_to_allocate, JS_control.m_pcall_endtime, JS_control.m_pcall_endtime-wxGetUTCTimeMillis());
        JS_control.m_pJSconsole->m_Output->AppendText(buffer);
#endif // IN_HARNESS
        }
#endif   // TIMEOUT_DUMP

    if (JS_control.m_pcall_endtime == 0) return(0); // we are not timing at this moment
    JS_control.m_timeout_check_counter++;
    if (wxGetUTCTimeMillis() > JS_control.m_pcall_endtime) {
        if (!JS_control.m_backingOut){ // just starting to back out
            JS_control.m_backingOut = true; // so we do not clear more than once
            JS_control.m_result = wxEmptyString;    // supress result
            JS_control.clear();
            }
		return 1;
	}
	return 0;
}
