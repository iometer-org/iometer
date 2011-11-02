/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / PageAccess.h                                           ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Interface for the CPageAccess class, which is         ## */
/* ##               responsible for the "Access Specifications" tab in    ## */
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
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef PAGEACCESS_DEFINED
#define PAGEACCESS_DEFINED

#include "GalileoApp.h"
#include "IOTest.h"

// Info for access spec image list (LED's and default assignment)
#define ACCESS_ICON_SIZE			15
#define ACCESS_ICON_EXPAND			10	// number of icons to add to ImageList when expanding
#define GLOBAL_ACCESS_ICON_BGCOLOR		0x00ff00ff
#define ASSIGNED_ACCESS_ICON_BGCOLOR		0x00ffffff

// Color of LED's.  (Position of icon in bitmap.)
#define	RED		0
#define	BLUE		1
#define GREEN		2

// Info for searching in CListCtrls
#define NOTHING		-1	// Used to compare return values.  Indicates nothing is selected.
#define FIND_FIRST	-1	// Used when searching through a list control.  Indicates that the first item
						// that matches the criteria should be returned.

/////////////////////////////////////////////////////////////////////////////
// CPageAccess dialog

class CPageAccess:public CPropertyPage {
	DECLARE_DYNCREATE(CPageAccess)
// Construction
      public:
	CPageAccess();
	~CPageAccess() {
	};

      protected:
// Dialog Data
	//{{AFX_DATA(CPageAccess)
	enum { IDD = IDD_ACCESS };
	CButton m_GGlobalFrame;
	CButton m_GAssignedFrame;
	CButton m_BEditCopy;
	CListCtrl m_LAssignedAccess;
	CButton m_BUp;
	CListCtrl m_LGlobalAccess;
	CButton m_BRemove;
	CButton m_BNew;
	CButton m_BEdit;
	CButton m_BDown;
	CButton m_BAdd;
	CButton m_BDelete;

	//}}AFX_DATA

      public:

	void ShowAssignedAccess();
	void ShowGlobalAccess();
	void EnableWindow(BOOL enable = TRUE);
	void Reset();

	void MarkAccess(int access_index, int color);
	void MarkAccesses(int current_access);

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPageAccess)
      protected:
	virtual void DoDataExchange(CDataExchange * pDX);	// DDX/DDV support

	//}}AFX_VIRTUAL

// Implementation
      protected:

	void SetGlobalButtons(BOOL enable = TRUE);
	void SetAssignedButtons(BOOL enable = TRUE);
	void SetAssignedAccess(BOOL enable = TRUE);
	void Move(int desired_index);

	void Insert(CPoint point);	// Inserts an item into the assigned list before the currently selected item.
	void InsertAt(int insert_index);
	void Remove();		// Removes the item in the assigned list specified by item_index.
	BOOL Edit();
	void Delete();

	CImageList m_AssignedImageList;	// LEDs indicate which access spec has run, is running, and will run.
	CImageList m_GlobalImageList;	// Icons indicate which access specs get assigned by default to which type of worker.
	CImageList *p_DragImage;
	BOOL global_dragging;	// indicates that an item is being dragged 

	// from the global list to the assigned list.
	BOOL assigned_dragging;	// indicates that an item is being dragged 

	// within the assigned list.

	// Generated message map functions
	//{{AFX_MSG(CPageAccess)
	virtual BOOL OnInitDialog();
	afx_msg void OnBEdit();
	afx_msg void OnBNew();
	afx_msg void OnBDelete();
	afx_msg void OnBAdd();
	afx_msg void OnBRemove();
	afx_msg void OnBDown();
	afx_msg void OnBUp();
	afx_msg void OnDblclkLGlobalAccess(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnBegindragLGlobalAccess(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg BOOL OnSetCursor(CWnd * pWnd, UINT nHitTest, UINT message);
	afx_msg void OnItemchangedLAssignedAccess(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnBegindragLAssignedAccess(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnItemchangedLGlobalAccess(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnSetfocusLGlobalAccess(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnSetfocusLAssignedAccess(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnBEditCopy();
	afx_msg void OnKillfocusLGlobalAccess(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnKillfocusLAssignedAccess(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnClickLGlobalAccess(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnClickLAssignedAccess(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnKeyDownGlobalAccess(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnKeyDownAssignedAccess(NMHDR * pNMHDR, LRESULT * pResult);

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
