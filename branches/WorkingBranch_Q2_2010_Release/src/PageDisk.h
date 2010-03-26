/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Iometer / PageDisk.h                                               ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Interface for the CPageDisk class, which is           ## */
/* ##               responsible for the "Disk Targets" tab in Iometer's   ## */
/* ##               main window.                                          ## */
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
#ifndef PAGEDISK_DEFINED
#define PAGEDISK_DEFINED

/////////////////////////////////////////////////////////////////////////////
// CPageDisk dialog

#include "GalileoApp.h"
#include "PageTarget.h"

class CPageDisk:public CPropertyPage {
	DECLARE_DYNCREATE(CPageDisk)
// Construction
      public:
	CPageDisk();
	~CPageDisk();

	void Reset();
	void EnableWindow(BOOL enable = TRUE);
	void ShowData();
	void ShowTargets();
// Dialog Data
	//{{AFX_DATA(CPageDisk)
	enum { IDD = IDD_DISKS };
	CEdit m_EQueueDepth;
	CSpinButtonCtrl m_SConnectionRate;
	CButton m_CConnectionRate;
	CButton m_CUseRandomData;
	CTreeCtrl m_TTargets;
	CEdit m_EConnectionRate;
	CEdit m_EDiskStart;
	CEdit m_EDiskSize;
	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPageDisk)
      protected:
	 virtual void DoDataExchange(CDataExchange * pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
      protected:
	void SetFocusUp();
	void SetFocusDown();
	void SetFocusHome();
	void SetFocusEnd();

	void ShowRandomData();
	void ShowConnectionRate();
	void ShowTargetSelection();
	void ShowSettings();
	void ShowFocus();
	void StoreTargetSelection();

	void SelectRange(HTREEITEM hstart, HTREEITEM hend, BOOL replace = TRUE, TargetSelType state = TargetChecked);
	void SetSelectionCheck(HTREEITEM hitem, TargetSelType selection);
	TargetSelType GetSelectionCheck(HTREEITEM hitem);
	void SetAllCheck(TargetSelType selection);
	void KeyMultiSel(WORD wVKey);

	void EditSetfocus(CEdit * edit);

	HTREEITEM GetNextTreeObject(HTREEITEM hnode);

	// These do not exist in the mfc...
	DWORDLONG GetDlgItemInt64(int nID, BOOL* lpTrans = NULL, BOOL bSigned = TRUE, int* lpRadix = NULL);
	void SetDlgItemInt64(int nID, __int64 nValue, BOOL bSigned = TRUE, int nRadix = 10);

	CImageList m_ImageList;
	HTREEITEM selected;	// The last selected item, or NULL if the 
	// selection has not changed.
	HTREEITEM highlighted;	// The current selection when using the keyboard.

	// Generated message map functions
	//{{AFX_MSG(CPageDisk)
	virtual BOOL OnInitDialog();
	afx_msg void OnKillfocusEQueueDepth();
	afx_msg void OnKillfocusEDiskSize();
	afx_msg void OnKillfocusEDiskStart();
	afx_msg void OnSetfocusEQueueDepth();
	afx_msg void OnSetfocusEDiskSize();
	afx_msg void OnSetfocusEDiskStart();
	afx_msg void OnKeydownTTargets(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnClickTTargets(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnKillfocusEConnectionRate();
	afx_msg void OnSetfocusEConnectionRate();
	afx_msg void OnDeltaposSConnectionRate(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnCConnectionRate();
	afx_msg void OnCUseRandomData();
	afx_msg void OnSetfocusTTargets(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnSelchangingTTargets(NMHDR * pNMHDR, LRESULT * pResult);
	//}}AFX_MSG
	 DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnEnChangeEdisksize();
	afx_msg void OnEnChangeEdiskstart();
};

#endif
