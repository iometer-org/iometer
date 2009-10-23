/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / ICFSaveDialog.h                                        ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Interface for the CICFSaveDialog class, which         ## */
/* ##               extends the standard CFileDialog class with           ## */
/* ##               additional functionality for saving Iometer's .ICF    ## */
/* ##               (Iometer Configuration File) files.                   ## */
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
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef ICFSAVEDIALOG_DEFINED
#define ICFSAVEDIALOG_DEFINED

/////////////////////////////////////////////////////////////////////////////
// CICFSaveDialog dialog

class CICFSaveDialog:public CFileDialog {
	DECLARE_DYNAMIC(CICFSaveDialog)
// Construction
      public:
	CICFSaveDialog();
	~CICFSaveDialog();

// Dialog Data
	//{{AFX_DATA(CICFSaveDialog)
	enum { IDD = IDD_FILESAVE_OPTS };
	CButton m_CkTestSetup;
	CButton m_CkResultsDisplay;
	CButton m_CkGlobalAccessSpec;
	CButton m_CkManagerWorker;
	CButton m_CkAssignAccessSpec;
	CButton m_CkAssignTargets;
	BOOL isCkTestSetup;
	BOOL isCkResultsDisplay;
	BOOL isCkGlobalAccessSpec;
	BOOL isCkManagerWorker;
	BOOL isCkAssignAccessSpec;
	BOOL isCkAssignTargets;
	//}}AFX_DATA

	// Once checkboxes are cleared and disabled in response to the
	// deselection of other checkboxes, their old states are maintained here.
	BOOL wasCkAssignAccessSpec;
	BOOL wasCkAssignTargets;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CICFSaveDialog)
      protected:
	 virtual void DoDataExchange(CDataExchange * pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
      protected:

	// Generated message map functions
	//{{AFX_MSG(CICFSaveDialog)
	 afx_msg void OnCkManagerWorker();
	virtual BOOL OnInitDialog();
	afx_msg void OnAnyCheck();
	afx_msg BOOL OnHelpInfo(HELPINFO * pHelpInfo);
	afx_msg void OnPaint();
	//}}AFX_MSG
	 DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif
