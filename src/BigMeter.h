/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / BigMeter.h                                             ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Interface for the CBigMeter class, which is           ## */
/* ##               responsible for the "Presentation Meter" dialog.      ## */
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
/* ##  Changes ...:  2003-10-17 (daniel.scheibli@edelbyte.org)            ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef BIGMETER_DEFINED
#define BIGMETER_DEFINED

#define BUTTON_START		0x01
#define BUTTON_STOP		0x02
#define BUTTON_STOPALL		0x04

/////////////////////////////////////////////////////////////////////////////
// CBigMeter dialog

class CBigMeter:public CDialog {
// Construction
      public:
	CBigMeter(CWnd * pParent = NULL);	// standard constructor

	// Create the dialog
	virtual BOOL Create(int performance_bar, CString test_title, CString result_title, CString worker_title);

	// Tell the dialog which main frame toolbar
	// buttons are enabled (of START, STOP, STOP ALL)
	void SetButtonState(BOOL start, BOOL stop, BOOL stopall);

	// Called from CGalileoView when user selects a new result to display
	void OnMDisplay(int submenu_id, int menu_item, int result_code);
	void ClearCheckButton();

	void UpdateDisplay();	// Call this when value has changed

	// Is the dialog visible?
	BOOL is_displayed;

      protected:

	// Update the "Next>>" and "Stop buttons.
	void UpdateButtons();
	int button_state;	// The state of CGalileoView's toolbar buttons

	// Controlling the maximum displayed value for the meter.
	void SetMaxRange(int new_range);
	int max_range;

	virtual void OnOK();
	virtual void OnCancel();

	// Information needed to extract the result value being displayed.
	Results *results;	// Pointer to base of results[MAX_UPDATE] array
	int result_to_display;
	double result_value;
	CString result_text;

	CString result_name;
	CString worker_name;

// Dialog Data
	//{{AFX_DATA(CBigMeter)
	enum { IDD = IDD_BIGMETER };
	CButton m_CkWatermark;
	CButton m_BResultType;
	CButton m_BNext;
	CButton m_BStop;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBigMeter)
      protected:
	 virtual void DoDataExchange(CDataExchange * pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
      protected:
	// Generated message map functions
	//{{AFX_MSG(CBigMeter)
	 afx_msg void OnBResultType();
	afx_msg void OnBNext();
	afx_msg void OnBStop();
	afx_msg void OnKillfocusEMaxRange();
	afx_msg void OnChangeEMaxRange();
	afx_msg void OnCkWatermark();
	//}}AFX_MSG
	 DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif				// !BIGMETER_DEFINED
