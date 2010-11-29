/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Iometer / WaitingForManagers.h                                     ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: WaitingForManagers.h: Interface for the               ## */
/* ##               CWaitingForManagers class, which is responsible for   ## */
/* ##               the "Waiting for Managers" dialog (called when an     ## */
/* ##               .ICF file specified on the command line calls for     ## */
/* ##               managers that are not yet logged in). The managers    ## */
/* ##               being waited on are displayed in a List Control.      ## */
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
/* ##  Changes ...: 2003-10-17 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-03-28 (joe@eiler.net)                            ## */
/* ##               - changes so VC++ 7 (.NET) will compile correctly.    ## */
/* ##               2003-03-28 (joe@eiler.net)                            ## */
/* ##               - added new header info.                              ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef WAITING_FOR_MANAGERS_DEFINED
#define WAITING_FOR_MANAGERS_DEFINED

#include <afxtempl.h>
#include "resource.h"
#include "GalileoDefs.h"

/////////////////////////////////////////////////////////////////////////////
// CWaitingForManagers dialog

class CWaitingForManagers:public CDialog {
// Construction
      public:
	CWaitingForManagers();	// standard constructor

	// Create CWaitingForManagers as a modeless dialog
	BOOL Create(const CString & infilename, BOOL * flags, BOOL replace);

	// Add or remove managers from the waiting list
	void AddWaitingManager(const CString & name, const CString & address);
	BOOL RemoveWaitingManager(const CString & name, const CString & address);

	// Prepares the waiting list dialog for reuse.
	void Reset();

// Dialog Data
	//{{AFX_DATA(CWaitingForManagers)
	enum { IDD = IDD_WAITING_FOR_MANAGERS };
	CListCtrl m_LManagers;
	CString m_TCountdown;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWaitingForManagers)
      protected:
	 virtual void DoDataExchange(CDataExchange * pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
      protected:

	// Update the GUI waiting list to reflect the state of the names and addresses arrays
	void DisplayManagers();

	// Displays the countdown notice
	void UpdateCountdown();

	// Called to clean up the OS resources and delete this object
	BOOL Close();

	// Generated message map functions
	//{{AFX_MSG(CWaitingForManagers)
	virtual void OnCancel();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	 DECLARE_MESSAGE_MAP()

	int remaining_seconds;	// The remaining seconds before automatic abort.
	// Reinitialized in OnInitDialog from theApp.cmdline.m_iTimeout.

	// The parameters for the callback to the function which will
	// receive notification when this dialog is finished
	CString callback_infilename;
	BOOL callback_flags[NumICFFlags];
	BOOL callback_replace;

	// How many times a second to see whether the waiting list is empty
	static const int polling_frequency;

	CStringArray names;
	CStringArray addresses;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif				// WAITING_FOR_MANAGERS_DEFINED
