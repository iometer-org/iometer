/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / AccessDialog.h                                         ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Interface for the CAccessDialog class, which is       ## */
/* ##               responsible for the "Edit Access Specification"       ## */
/* ##               dialog.                                               ## */
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
#ifndef ACCESS_DIALOG_DEFINED
#define ACCESS_DIALOG_DEFINED

#if _MSC_VER >= 1000
#pragma once
#endif				// _MSC_VER >= 1000
// AccessDialog.h : header file
//

#include "GalileoApp.h"
#include "GalileoView.h"
#include "IOTest.h"

class CAccessDialog;		// Forward declaration

//
// Structure that defines a group of controls for Megabytes, Kilobytes, and Bytes
//
struct MKBControls {
	CEdit *EBytesCtrl;
	CEdit *EKilobytesCtrl;
	CEdit *EMegabytesCtrl;
	UINT EBytesID;
	UINT EKilobytesID;
	UINT EMegabytesID;
	CSpinButtonCtrl *SBytesCtrl;
	CSpinButtonCtrl *SKilobytesCtrl;
	CSpinButtonCtrl *SMegabytesCtrl;
	UINT SBytesID;
	UINT SKilobytesID;
	UINT SMegabytesID;
	UINT RSelectedID;	// radio button indicating MKBControls are selected
	UINT RNotSelectedID;	// radio button indicating MKBControls are not selected
	// RSelectedID MUST be greater than RNotSelectedID, for the sake of GetCheckedRadioButton().
	// If there are no controlling radio buttons, set both to zero.
	 DWORD(CAccessDialog::*GetFunc) ();	// pointer to getter function
	void (CAccessDialog::*SetFunc) (DWORD);	// pointer to setter function
	// GetFunc and SetFunc are POINTERS TO MEMBER FUNCTIONS, which have a special syntax.
	// Given:                                                       MKBControls *which;
	// To assign FUNCNAME to SetFunc:       which->SetFunc = this->FUNCNAME;
	// To call FUNCNAME with VALUE:         (this->*(which->SetFunc))( VALUE );
};

#define MAX_SIZE_RANGE	KILOBYTE - 1

/////////////////////////////////////////////////////////////////////////////
// CAccessDialog dialog

class CAccessDialog:public CDialog {
	// LAccess subitems (integer values should remain
	// unchanged for backward file format compatibility)
	enum {
		LAccessSize = 0,
		LAccessOfSize = 1,
		LAccessReads = 2,
		LAccessRandom = 3,
		LAccessDelay = 4,
		LAccessBurst = 5,
		LAccessAlignment = 6,
		LAccessReply = 7
	};

// Construction
      public:
	 CAccessDialog(Test_Spec * edit_spec, CWnd * pParent = NULL);	// standard constructor

      protected:
// Dialog Data
	//{{AFX_DATA(CAccessDialog)
	enum { IDD = IDD_ACCESSDLG };
	CButton m_RReplySize;
	CButton m_RNoReply;
	CButton m_RAlignSector;
	CButton m_RAlignBytes;
	CSpinButtonCtrl m_SReplyMegabytes;
	CSpinButtonCtrl m_SReplyKilobytes;
	CSpinButtonCtrl m_SReplyBytes;
	CEdit m_EReplyMegabytes;
	CEdit m_EReplyKilobytes;
	CEdit m_EReplyBytes;
	CSpinButtonCtrl m_SAlignMegabytes;
	CSpinButtonCtrl m_SAlignKilobytes;
	CSpinButtonCtrl m_SAlignBytes;
	CEdit m_EAlignMegabytes;
	CEdit m_EAlignKilobytes;
	CEdit m_EAlignBytes;
	CEdit m_EBytes;
	CEdit m_EMegabytes;
	CEdit m_EKilobytes;
	CComboBox m_CDefaultAssignment;
	CSpinButtonCtrl m_SMegabytes;
	CSpinButtonCtrl m_SKilobytes;
	CSpinButtonCtrl m_SBytes;
	CButton m_BOk;
	CEdit m_EName;
	CSliderCtrl m_SRead;
	CSliderCtrl m_SRandom;
	CSliderCtrl m_SAccess;
	CListCtrl m_LAccess;
	CEdit m_EDelayTime;
	CEdit m_EBurstLength;
	CButton m_BInsertBefore;
	CButton m_BInsertAfter;
	CButton m_BDelete;
	CString access_txt;
	CString random_txt;
	CString read_txt;
	CString sequential_txt;
	CString transfer_txt;
	CString write_txt;
	//}}AFX_DATA

	Test_Spec *spec;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAccessDialog)
      protected:
	 virtual void DoDataExchange(CDataExchange * pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
      protected:

	 MKBControls size_controls;
	MKBControls align_controls;
	MKBControls reply_controls;

	// Verifies that everything is okay when the user clicks OK.
	BOOL CheckAccess();

	// These are the functions which deal with the access spec listbox.
	void InitializeList();
	void LoadList();
	BOOL SaveList();
	void InsertWrapper(BOOL after);
	BOOL InsertLine(Access_Spec * access_spec, int line_no);
	BOOL DeleteLine();

	// Each function gets a value from the working access_spec.
	// (GetAll calls each of the other functions)
	void GetAll(Access_Spec * spec);
	int GetAccess();
	int GetReads();
	int GetRandom();
	int GetDelay();
	int GetBurst();
	DWORD GetAlign();
	DWORD GetSize();
	DWORD GetReply();

	// Each function sets a value in the working access_spec,
	// updates that value in the access spec listbox,
	// and changes the controls to reflect the change.
	// (SetAll calls each of the other functions)
	void SetAll(Access_Spec * spec);
	void SetAccess(int of_size);
	void SetReads(int reads);
	void SetRandom(int random);
	void SetDelay(int delay);
	void SetBurst(int burst);
	void SetAlign(DWORD align);
	void SetSize(DWORD size);
	void SetReply(DWORD reply);

	void SizeToText(DWORD size, CString * size_text);
	// Set the spinners, which in turn set the edit boxes.
	void SetMKBSpinners(MKBControls * which, DWORD new_value);
	// Get the size reported by the spinners.
	DWORD GetMKBSpinners(MKBControls * which);
	// Get the size reported by the edit boxes.
	DWORD GetMKBEditbox(MKBControls * which);
	// Handle a change in one of the spinners.
	void OnDeltaposMKB(MKBControls * which, NMHDR * pNMHDR);
	// Enable or disable the controls.
	void EnableMKBControls(MKBControls * which, BOOL enabled);

	int item_being_changed;

	// Generated message map functions
	//{{AFX_MSG(CAccessDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnBDelete();
	afx_msg void OnBInsertAfter();
	afx_msg void OnBInsertBefore();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar * pScrollBar);
	afx_msg void OnKillfocusESizes();
	afx_msg void OnKillfocusEAligns();
	virtual void OnOK();
	afx_msg void OnDeltaposSSizes(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnDeltaposSAligns(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnRAlignSector();
	afx_msg void OnRAlignBytes();
	afx_msg void OnChangeBurst();
	afx_msg void OnChangeDelay();
	virtual void OnCancel();
	afx_msg void OnDeltaposSReply(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnKillfocusEReply();
	afx_msg void OnRNoReply();
	afx_msg void OnRReplySize();
	afx_msg void OnChangeLAccess(NMHDR * pNMHDR, LRESULT * pResult);
	//}}AFX_MSG
	 DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif				// !defined(ACCESS_DIALOG_DEFINED)
