/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / GalileoView.h                                          ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Interface for the CGalileoView class, which manages   ## */
/* ##               Iometer's overall GUI (e.g. toolbar).                 ## */
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
#ifndef	VIEW_DEFINED
#define VIEW_DEFINED

#define	TEST_TIMER		1
#define PREPARE_TIMER		2
#define RAMP_TIMER		3
#define IDLE_TIMER		4
#define NOTE_TIMER		5

#define DISK_PAGE		0
#define NETWORK_PAGE		1
#define ACCESS_PAGE		2
#define DISPLAY_PAGE		3
#define SETUP_PAGE		4

#define NUM_TOOLBAR_BUTTONS	13

#include "GalileoDoc.h"
#include "WorkerView.h"
#include "ManagerList.h"
 // include .h files for each page of the prop sheet
#include "PageDisk.h"
#include "PageNetwork.h"
#include "PageAccess.h"
#include "PageDisplay.h"
#include "PageSetup.h"
#include "ICFOpenDialog.h"
#include "ICFSaveDialog.h"

class CGalileoView:public CView {
      protected:		// create from serialization only
	CGalileoView();
	DECLARE_DYNCREATE(CGalileoView)
// Attributes
      public:
	CGalileoDoc * GetDocument();

// Operations
      public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGalileoView)
      public:
	virtual void OnDraw(CDC * pDC);	// overridden to draw this view
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT & rect,
			    CWnd * pParentWnd, UINT nID, CCreateContext * pContext = NULL);
      protected:
	 virtual void DoDataExchange(CDataExchange * pDX);
	//}}AFX_VIRTUAL

// Implementation
      public:
	 virtual ~ CGalileoView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext & dc) const;
#endif
	void AddManager(Manager * manager);
	Worker *AddWorker(TargetType worker_type, Manager * manager, const CString & name = "");
	void OnNewWorker(TargetType worker_type);
	void AddDefaultWorkers(Manager * manager);
	BOOL DisksNotPrepared();
	void ChangedSelection();
	void ChangingSelection();
	TargetType GetSelectedTargetType();

	// Functions to set button states.
	void ButtonReset();

	// Called by the button handlers for the toolbar and for the big meter dialog
	void Go();
	void StopAll();

	//////////////////////////////////////////////////////
	// Saves/loads an Iometer Configuration File (.icf) //
	//////////////////////////////////////////////////////
	// Preprocesses the config file and makes sure all managers specified in
	// the file are available (or waits on them), then calls OpenConfigFile.
	BOOL PrepareToOpenConfigFile(const CString & infilename, BOOL * flags, BOOL replace);
	// Restores configuration from specified file.
	BOOL OpenConfigFile(const CString & infilename,	// DO NOT CALL DIRECTLY!
			    BOOL * flags,	// Use OpenConfigFileWrapper!
			    BOOL replace);
	// Saves configuration to specified file.
	BOOL SaveConfigFile(const CString & outfilename, BOOL * flags);

	// Resets display as needed.
	void ResetDisplayforNewTest();

	// Function to set text in the status bar.
	void SetStatusBarText(CString text1 = "", CString text2 = "", CString text3 = "");
	void SetStatusBarPaneText(int paneIndex = 0, CString text = "");
	void ClearStatusBar();
	void UpdateTestStatus();

	// Returns the currently running access spec's index.
	int GetCurrentAccessIndex();

	void CalcRunCount();

	// Tells the results display dialogs which result was selected to be monitored.
	void OnMDisplay(int submenu_id, int menu_item);

	void Reset();

	///////////////////////////////////////////////////////////////////////////
	// Tab sheets.
	//
	CWorkerView *m_pWorkerView;
	CPageDisk *m_pPageDisk;
	CPageNetwork *m_pPageNetwork;
	CPageDisplay *m_pPageDisplay;
	CPageSetup *m_pPageSetup;
	CPageAccess *m_pPageAccess;
	//
	///////////////////////////////////////////////////////////////////////////

	HTREEITEM right_clicked_item;
	CImageList *p_DragImage;
	BOOL dragging;

	// property sheet is wired to MDI child frame and is not displayed
	CPropertySheet *m_pPropSheet;

      protected:
	CICFOpenDialog file_open_box;	// open config file dialog box
	CICFSaveDialog file_save_box;	// save config file dialog box

	// tracks whether parent frame has already been sized.
	BOOL m_bSizedBefore;

	int manager_to_prepare;
	int worker_to_prepare;
	CString result_file;
	int noteTime;

	struct {
		UINT nID;
		BOOL enabled;
	} toolbar_buttons[NUM_TOOLBAR_BUTTONS];

	///////////////////////////////////////////////////////////////////////////
	// Information needed to save configuration settings before testing and
	// restore them afterwards.
	//
	void SaveSettings();
	void RestoreSettings();
	//
	int *save_queue_depth;
	//
	///////////////////////////////////////////////////////////////////////////

	void InitAccessSpecRun();
	BOOL SetAccess();
	void SaveAccessSpecs();
	void StartTest();
	void StartRecording();
	void StartRemainNotification( int timeRem);
	void UpdateRemainNotification();

      public:
	void StopTest(ReturnVal test_successful);
      protected:
	void TestDone(ReturnVal test_successful);
	void SaveResults();
	BOOL SetActiveTargets();
	BOOL SetTargets();

	void EnableWindow(BOOL enable = TRUE);

	///////////////////////////////////////////////////////////////////////////
	// Functions to set button states.
	//
	void SetButton(UINT nID, BOOL state);
	void ButtonReady();
	void ButtonTest();
	void ButtonPrepare();
	void ButtonOff();
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Information used to manage cycling through targets, workers, queue
	// depth, access specs, etc.
	//
	int run_count;
	int run_index;
	int access_count;
	int access_index;
	//
	BOOL CycleTargets();
	BOOL CycleWorkers();
	BOOL IncrementTargets();
	BOOL IncrementTargetsSerial();
	BOOL Normal();
	int CycleWorkersTargets();
	BOOL CycleQueue();
	BOOL CycleTargetsQueue();
	//
	int workers_to_run;
	int targets_to_run;
	int queue_depth_to_run;
	int worker_exponent;
	int target_exponent;
	int queue_exponent;
	//
	///////////////////////////////////////////////////////////////////////////

// Generated message map functions
      protected:
	//{{AFX_MSG(CGalileoView)
	 afx_msg BOOL OnEraseBkgnd(CDC * pDC);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBReset();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	afx_msg void OnBExitOne();
	afx_msg void OnBNewDynamo();
	afx_msg void OnBNewDiskWorker();
	afx_msg void OnBNewNetWorker();
	afx_msg void OnBCopyWorker();
	afx_msg void OnMRefresh();
	afx_msg void OnBStart();
	afx_msg void OnBStop();
	afx_msg void OnBStopAll();
	afx_msg BOOL OnSetCursor(CWnd * pWnd, UINT nHitTest, UINT message);
	afx_msg void OnUpdateToolbarButton(CCmdUI * pCmdUI);
	afx_msg void OnDestroy();
	afx_msg void OnMDisplayCPUDPC();
	afx_msg void OnMDisplayCPUEffectiveness();
	afx_msg void OnMDisplayCPUInterruptsPS();
	afx_msg void OnMDisplayCPUInterruptTime();
	afx_msg void OnMDisplayCPUPrivileged();
	afx_msg void OnMDisplayCPUUser();
	afx_msg void OnMDisplayCPUUtilization();
	afx_msg void OnMDisplayErrIO();
	afx_msg void OnMDisplayErrRIO();
	afx_msg void OnMDisplayErrWIO();
	afx_msg void OnMDisplayAvgCon();
	afx_msg void OnMDisplayAvgIO();
	afx_msg void OnMDisplayAvgRIO();
	afx_msg void OnMDisplayAvgTrans();
	afx_msg void OnMDisplayAvgWIO();
	afx_msg void OnMDisplayMaxCon();
	afx_msg void OnMDisplayMaxIO();
	afx_msg void OnMDisplayMaxRIO();
	afx_msg void OnMDisplayMaxTrans();
	afx_msg void OnMDisplayMaxWIO();
	afx_msg void OnMDisplayMBsMBPSBin();
	afx_msg void OnMDisplayMBsRMBPSBin();
	afx_msg void OnMDisplayMBsWMBPSBin();
	afx_msg void OnMDisplayMBsMBPSDec();
	afx_msg void OnMDisplayMBsRMBPSDec();
	afx_msg void OnMDisplayMBsWMBPSDec();
	afx_msg void OnMDisplayNetPacketErrors();
	afx_msg void OnMDisplayNetPacketsPS();
	afx_msg void OnMDisplayNetRetransPS();
	afx_msg void OnMDisplayOpsConPS();
	afx_msg void OnMDisplayOpsIOPS();
	afx_msg void OnMDisplayOpsRIOPS();
	afx_msg void OnMDisplayOpsTransPS();
	afx_msg void OnMDisplayOpsWIOPS();
	//}}AFX_MSG
	 DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG			// debug version in GalileoView.cpp
inline CGalileoDoc *CGalileoView::GetDocument()
{
	return (CGalileoDoc *) m_pDocument;
}
#endif

/////////////////////////////////////////////////////////////////////////////

#endif
