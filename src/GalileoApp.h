/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Iometer / GalileoApp.h                                             ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: The header file for main Iometer application class.   ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Intel Open Source License                                          ## */
/* ##                                                                     ## */
/* ##  Copyright (c) 2001 Intel Corporation                               ## */
/* ##  All rights reserved.                                               ## */
/* ##  Redistribution and use in source and binary forms, with or         ## */
/* ##  without modification, are permitted provided that the following    ## */
/* ##  conditions are met:                                                ## */
/* ##                                                                     ## */
/* ##  Redistributions of source code must retain the above copyright     ## */
/* ##  notice, this list of conditions and the following disclaimer.      ## */
/* ##                                                                     ## */
/* ##  Redistributions in binary form must reproduce the above copyright  ## */
/* ##  notice, this list of conditions and the following disclaimer in    ## */
/* ##  the documentation and/or other materials provided with the         ## */
/* ##  distribution.                                                      ## */
/* ##                                                                     ## */
/* ##  Neither the name of the Intel Corporation nor the names of its     ## */
/* ##  contributors may be used to endorse or promote products derived    ## */
/* ##  from this software without specific prior written permission.      ## */
/* ##                                                                     ## */
/* ##  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND             ## */
/* ##  CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,      ## */
/* ##  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF           ## */
/* ##  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           ## */
/* ##  DISCLAIMED. IN NO EVENT SHALL THE INTEL OR ITS  CONTRIBUTORS BE    ## */
/* ##  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,   ## */
/* ##  OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,           ## */
/* ##  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,    ## */
/* ##  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY    ## */
/* ##  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR     ## */
/* ##  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT    ## */
/* ##  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY    ## */
/* ##  OF SUCH DAMAGE.                                                    ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Remarks ...: <none>                                                ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Changes ...: 2003-10-15 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-03-28 (joe@eiler.net)                            ## */
/* ##               - changes so VC++ 7 (.NET) will compile correctly.    ## */
/* ##               2003-03-28 (joe@eiler.net)                            ## */
/* ##               - added new header info.                              ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef GALILEO_DEFINED
#define GALILEO_DEFINED

#include "GalileoDefs.h"
#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "IOCommon.h"
#include "AccessSpecList.h"
#include "ManagerList.h"
#include "BigMeter.h"
#include "GalileoCmdLine.h"

class CGalileoView;		// forward declaration
class ManagerList;

/////////////////////////////////////////////////////////////////////////////
// CGalileoApp:
// See GalileoApp.cpp for the implementation of this class
//

class CGalileoApp:public CWinApp {
      public:
	CGalileoApp();
	~CGalileoApp();

	const char *GetVersionString(BOOL fWithDebugIndicator = FALSE);

	// Is Iometer running in batch mode?
	BOOL IsBatchMode();
	// Turns off batch mode.  (Does no harm if Iometer isn't in batch mode.)
	void OverrideBatchMode();
	// Determines whether the given address is a local to this machine.
	BOOL IsAddressLocal(const CString & addr);
	// Launch a local Dynamo with the given name.
	void LaunchDynamo(const CString & mgr_name = "");

	CGalileoView *pView;
	CToolBar m_wndToolBar;
	CStatusBar m_wndStatusBar;

	AccessSpecList access_spec_list;	// Global access spec list.
	ManagerList manager_list;	// Global manager list.

	Port *login_port;

	enum {
		closed,
		open,
		accepting,
		waiting,
		receiving,
		waiting_for_data,
		receiving_data,
		failed
	} login_state;

	TestState test_state;

	// Command line parser.  (Also retains parameters.)
	CGalileoCmdLine cmdline;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGalileoApp)
      public:
	 virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL OnIdle(LONG lCount);
	virtual CDocument *OpenDocumentFile(LPCTSTR lpszFileName);
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CGalileoApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	 DECLARE_MESSAGE_MAP()

      protected:
	void IdentifyLocalAddresses();

	CStringArray ip_addresses;
	CString netbios_hostname;

	// Formatting string for launching a new Dynamo.
	// This is not the actual command line string.
	// It is used by CGalileoApp::LaunchDynamo().
	CString new_manager_command_line_format;
      private:
	char *m_pVersionString;
	char *m_pVersionStringWithDebug;
};

extern CGalileoApp theApp;

/////////////////////////////////////////////////////////////////////////////

#endif
