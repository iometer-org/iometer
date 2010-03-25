/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / WorkerView.h                                           ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Interface for the CWorkerView class, which is         ## */
/* ##               responsible for the "Topology" pane in Iometer's      ## */
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
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef	WORKER_VIEW_DEFINED
#define WORKER_VIEW_DEFINED

#if _MSC_VER >= 1000
#pragma once
#endif				// _MSC_VER >= 1000
// WorkerView.h : header file
//

#include "GalileoApp.h"

// Info for Workers image list
#define WORKER_ICON_SIZE					16
#define WORKER_ICON_EXPAND					10	// number of icons to add to ImageList when expanding
#define WORKER_ICON_BGCOLOR					0x00ff00ff

// Icons for Worker pane (position of icon in bitmap)
#define WORKER_ICON_ALLMGRS		0
#define WORKER_ICON_MANAGER		1
#define WORKER_ICON_DISKWORKER	2
#define WORKER_ICON_NETSERVER	3
#define WORKER_ICON_NETCLIENT	4

/////////////////////////////////////////////////////////////////////////////
// CWorkerView dialog

class CWorkerView:public CDialog {
// Construction
      public:
	CWorkerView(CWnd * pParent = NULL);	// standard constructor

	TargetType GetSelectedWorkerType();

	void AddManager(Manager * manager);
	void AddWorker(Worker * worker);

	void Initialize();
	void Reset();

	void RemoveSelectedItem();
	void RemoveManager(Manager * manager);
	void RemoveWorker(Worker * worker);

	Manager *GetSelectedManager();
	Worker *GetSelectedWorker();
	int GetSelectedManagerIndex();
	int GetSelectedWorkerIndex();
	int GetSelectedType();

	void SelectItem(Manager * mgr, Worker * wkr);

// Dialog Data
	//{{AFX_DATA(CWorkerView)
	enum { IDD = IDD_MANAGER_VIEW };
	CTreeCtrl m_TWorkers;
	//}}AFX_DATA
	CImageList m_ImageList;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWorkerView)
      protected:
	 virtual void DoDataExchange(CDataExchange * pDX);	// DDX/DDV support
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
      protected:
	 virtual BOOL PreTranslateMessage(MSG * pMsg);

	// The OK and Cancel buttons do not exist, but are called when the enter
	// and escape key are pressed.  Use these functions to trap on these keys.
	virtual void OnOK();
	virtual void OnCancel();

	HTREEITEM GetManager(Manager * manager);
	HTREEITEM GetWorker(Worker * worker);

	HTREEITEM hall_managers;
	int selected_type;

	BOOL editing_label;
	CString *old_label_name;

	// Generated message map functions
	//{{AFX_MSG(CWorkerView)
	afx_msg void OnSelchangedTWorkers(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnSelchangingTWorkers(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnEndlabeleditTWorkers(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnBegindragTWorkers(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnRclickTWorkers(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnBeginlabeleditTWorkers(NMHDR * pNMHDR, LRESULT * pResult);
	//}}AFX_MSG
	 DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif
