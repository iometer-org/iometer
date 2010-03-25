/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Iometer / PageNetwork.h                                            ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Interface for the CPageNetwork class, which is        ## */
/* ##               responsible for the "Network Targets" tab in          ## */
/* ##               Iometer's main window.                                ## */
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
/* ##               - added new header info.                              ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef PAGENETWORK_DEFINED
#define PAGENETWORK_DEFINED

#if _MSC_VER >= 1000
#pragma once
#endif				// _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CPageNetwork dialog

#include "GalileoApp.h"
#include "PageTarget.h"

class CPageNetwork:public CPropertyPage {
	DECLARE_DYNCREATE(CPageNetwork)
// Construction
      public:
	CPageNetwork();
	~CPageNetwork();

	void EnableWindow(BOOL enable = TRUE);
	void AddManager(Manager * manager);
	void RemoveManager(Manager * manager);
	void ShowData();
	void ShowTargets();
	void Reset();

// Dialog Data
	//{{AFX_DATA(CPageNetwork)
	enum { IDD = IDD_NETWORKS };
	CEdit m_EMaxSends;
	CButton m_CConnectionRate;
	CComboBox m_DInterface;
	CEdit m_EConnectionRate;
	CSpinButtonCtrl m_SConnectionRate;
	CTreeCtrl m_TTargets;
	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPageNetwork)
      protected:
	 virtual void DoDataExchange(CDataExchange * pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
      protected:
	void StoreTargetSelection();

	void ShowTargetSelection();
	void ShowFocus();
	void ShowConnectionRate();
	void ShowInterface();
	void ShowSettings();

	// Functions that move the highlight.
	void SetFocusUp();
	void SetFocusDown();
	void SetFocusHome();
	void SetFocusEnd();

	void SelectRange(HTREEITEM hstart, HTREEITEM hend, BOOL replace = TRUE, TargetSelType state = TargetChecked);
	void SetSelectionCheck(HTREEITEM hitem, TargetSelType selection);
	TargetSelType GetSelectionCheck(HTREEITEM hitem);
	void SetAllCheck(TargetSelType selection);
	void CheckInterface(HTREEITEM hmanager, char *net_address = NULL);
	void KeySingleSel(WORD wVKey);	// Single selection keyboard handler
	void KeyMultiSel(WORD wVKey);	// Multi selection keyboard handler

	void EditSetfocus(CEdit * edit);

	CImageList m_ImageList;
	HTREEITEM selected;	// The last selected item, or NULL if the 
	// selection has not changed.
	HTREEITEM highlighted;	// The current selection when using the keyboard.

	// Generated message map functions
	//{{AFX_MSG(CPageNetwork)
	afx_msg void OnClickTTargets(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnCConnectionRate();
	afx_msg void OnDeltaposSConnectionRate(NMHDR * pNMHDR, LRESULT * pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnSetfocusEConnectionRate();
	afx_msg void OnSelchangingTTargets(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnSelchangeDInterface();
	afx_msg void OnKillfocusEConnectionRate();
	afx_msg void OnKeydownTTargets(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnDblclkTTargets(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnSetfocusTTargets(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnKillfocusEMaxSends();
	afx_msg void OnSetfocusEMaxSends();
	//}}AFX_MSG
	 DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif				// !defined(PAGENETWORK_DEFINED)
