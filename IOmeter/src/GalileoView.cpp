/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / GalileoView.cpp                                        ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implementation of the CGalileoView class, which       ## */
/* ##               manages Iometer's overall GUI (e.g. toolbar).         ## */
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
/* ##  Changes ...: 2004-06-11 (lamontcranston41@yahoo.com)               ## */
/* ##               - Add code to allow potentially invalid access specs  ## */
/* ##                 but warn the user.                                  ## */
/* ##               2003-10-17 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-04-25 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Updated the global debug flag (_DEBUG) handling     ## */
/* ##                 of the source file (check for platform etc.).       ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include "stdafx.h"
#include "GalileoView.h"
#include "Mainfrm.h"
#include "ManagerList.h"

// Needed for MFC Library support for assisting in finding memory leaks
//
// NOTE: Based on the documentation[1] I found, it should be enough to have
//       a "#define new DEBUG_NEW" statement for the case, that we are
//       running Windows. There should be no need for checking the _DEBUG
//       flag and no need for redefiniting the THIS_FILE string. Maybe there
//       will be a MFC hacker who could advice here.
//       [1] = http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclib/html/_mfc_debug_new.asp
//
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
#ifdef IOMTR_SETTING_MFC_MEMALLOC_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

void CGalileoView::SetButton(UINT nID, BOOL state)
{
	int i;

	// find the given button in toolbar_buttons array
	for (i = 0; i < NUM_TOOLBAR_BUTTONS; i++) {
		if (toolbar_buttons[i].nID == nID)
			break;
	}

	// did we find it?
	if (i == NUM_TOOLBAR_BUTTONS) {
		// nID not found, do nothing
		return;
	}
	// record new state
	toolbar_buttons[i].enabled = state;

	// set new state
	theApp.m_wndToolBar.GetToolBarCtrl().EnableButton(nID, state);
}

//
// Message handler for ON_UPDATE_COMMAND_UI for all toolbar buttons
//
void CGalileoView::OnUpdateToolbarButton(CCmdUI * pCmdUI)
{
	int i;

	// find the given button in toolbar_buttons array
	for (i = 0; i < NUM_TOOLBAR_BUTTONS; i++) {
		if (toolbar_buttons[i].nID == pCmdUI->m_nID) {
			break;
		}
	}

	// did we find it?
	if (i == NUM_TOOLBAR_BUTTONS) {
		// nID not found, do nothing
		return;
	}
	// set its state to match recorded state
	pCmdUI->Enable(toolbar_buttons[i].enabled);
}

/////////////////////////////////////////////////////////////////////////////
// CGalileoView

IMPLEMENT_DYNCREATE(CGalileoView, CView)

BEGIN_MESSAGE_MAP(CGalileoView, CView)
    //{{AFX_MSG_MAP(CGalileoView)
    ON_WM_ERASEBKGND()
    ON_COMMAND(BStart, OnBStart)
    ON_COMMAND(BStop, OnBStop)
    ON_WM_TIMER()
    ON_COMMAND(BStopAll, OnBStopAll)
    ON_COMMAND(BReset, OnBReset)
    ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
    ON_COMMAND(ID_FILE_SAVE, OnFileSave)
    ON_COMMAND(BExitOne, OnBExitOne)
    ON_COMMAND(BNewDynamo, OnBNewDynamo)
    ON_COMMAND(BNewDiskWorker, OnBNewDiskWorker)
    ON_COMMAND(BNewNetWorker, OnBNewNetWorker)
    ON_COMMAND(BCopyWorker, OnBCopyWorker)
    ON_COMMAND(MRefresh, OnMRefresh)
    ON_WM_SETCURSOR()
    ON_UPDATE_COMMAND_UI(BExitOne, OnUpdateToolbarButton)
    ON_WM_DESTROY()
    ON_COMMAND(MDisplayCPUDPC, OnMDisplayCPUDPC)
    ON_COMMAND(MDisplayCPUEffectiveness, OnMDisplayCPUEffectiveness)
    ON_COMMAND(MDisplayCPUInterruptsPS, OnMDisplayCPUInterruptsPS)
    ON_COMMAND(MDisplayCPUInterruptTime, OnMDisplayCPUInterruptTime)
    ON_COMMAND(MDisplayCPUPrivileged, OnMDisplayCPUPrivileged)
    ON_COMMAND(MDisplayCPUUser, OnMDisplayCPUUser)
    ON_COMMAND(MDisplayCPUUtilization, OnMDisplayCPUUtilization)
    ON_COMMAND(MDisplayErrIO, OnMDisplayErrIO)
    ON_COMMAND(MDisplayErrRIO, OnMDisplayErrRIO)
    ON_COMMAND(MDisplayErrWIO, OnMDisplayErrWIO)
    ON_COMMAND(MDisplayAvgCon, OnMDisplayAvgCon)
    ON_COMMAND(MDisplayAvgIO, OnMDisplayAvgIO)
    ON_COMMAND(MDisplayAvgRIO, OnMDisplayAvgRIO)
    ON_COMMAND(MDisplayAvgTrans, OnMDisplayAvgTrans)
    ON_COMMAND(MDisplayAvgWIO, OnMDisplayAvgWIO)
    ON_COMMAND(MDisplayMaxCon, OnMDisplayMaxCon)
    ON_COMMAND(MDisplayMaxIO, OnMDisplayMaxIO)
    ON_COMMAND(MDisplayMaxRIO, OnMDisplayMaxRIO)
    ON_COMMAND(MDisplayMaxTrans, OnMDisplayMaxTrans)
    ON_COMMAND(MDisplayMaxWIO, OnMDisplayMaxWIO)
    ON_COMMAND(MDisplayMBsMBPSBin, OnMDisplayMBsMBPSBin)
    ON_COMMAND(MDisplayMBsRMBPSBin, OnMDisplayMBsRMBPSBin)
    ON_COMMAND(MDisplayMBsWMBPSBin, OnMDisplayMBsWMBPSBin)
    ON_COMMAND(MDisplayMBsMBPSDec, OnMDisplayMBsMBPSDec)
    ON_COMMAND(MDisplayMBsRMBPSDec, OnMDisplayMBsRMBPSDec)
    ON_COMMAND(MDisplayMBsWMBPSDec, OnMDisplayMBsWMBPSDec)
    ON_COMMAND(MDisplayNetPacketErrors, OnMDisplayNetPacketErrors)
    ON_COMMAND(MDisplayNetPacketsPS, OnMDisplayNetPacketsPS)
    ON_COMMAND(MDisplayNetRetransPS, OnMDisplayNetRetransPS)
    ON_COMMAND(MDisplayOpsConPS, OnMDisplayOpsConPS)
    ON_COMMAND(MDisplayOpsIOPS, OnMDisplayOpsIOPS)
    ON_COMMAND(MDisplayOpsRIOPS, OnMDisplayOpsRIOPS)
    ON_COMMAND(MDisplayOpsTransPS, OnMDisplayOpsTransPS)
    ON_COMMAND(MDisplayOpsWIOPS, OnMDisplayOpsWIOPS)
    ON_UPDATE_COMMAND_UI(BNewDynamo, OnUpdateToolbarButton)
    ON_UPDATE_COMMAND_UI(BNewDiskWorker, OnUpdateToolbarButton)
    ON_UPDATE_COMMAND_UI(BNewNetWorker, OnUpdateToolbarButton)
    ON_UPDATE_COMMAND_UI(BCopyWorker, OnUpdateToolbarButton)
    ON_UPDATE_COMMAND_UI(BReset, OnUpdateToolbarButton)
    ON_UPDATE_COMMAND_UI(BStart, OnUpdateToolbarButton)
    ON_UPDATE_COMMAND_UI(BStop, OnUpdateToolbarButton)
    ON_UPDATE_COMMAND_UI(BStopAll, OnUpdateToolbarButton)
    ON_UPDATE_COMMAND_UI(ID_APP_ABOUT, OnUpdateToolbarButton)
    ON_UPDATE_COMMAND_UI(ID_APP_EXIT, OnUpdateToolbarButton)
    ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, OnUpdateToolbarButton)
ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateToolbarButton)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CGalileoView construction/destruction
    CGalileoView::CGalileoView()
{
	// set pointers to pages and sheets to NULL
	m_bSizedBefore = FALSE;
	m_pPropSheet = NULL;
	m_pPageDisk = NULL;
	m_pPageAccess = NULL;
	m_pPageDisplay = NULL;
	m_pPageSetup = NULL;
	m_pWorkerView = NULL;
	p_DragImage = NULL;
	dragging = FALSE;

	run_count = 0;
	run_index = 0;
	access_count = 0;
	access_index = 0;
	manager_to_prepare = 0;
	worker_to_prepare = 0;
	workers_to_run = 0;
	targets_to_run = 0;

	save_queue_depth = NULL;

	int i = 0;

	toolbar_buttons[i].nID = ID_FILE_OPEN;
	toolbar_buttons[i].enabled = FALSE;
	i++;
	toolbar_buttons[i].nID = ID_FILE_SAVE;
	toolbar_buttons[i].enabled = FALSE;
	i++;
	toolbar_buttons[i].nID = BNewDynamo;
	toolbar_buttons[i].enabled = FALSE;
	i++;
	toolbar_buttons[i].nID = BNewDiskWorker;
	toolbar_buttons[i].enabled = FALSE;
	i++;
	toolbar_buttons[i].nID = BNewNetWorker;
	toolbar_buttons[i].enabled = FALSE;
	i++;
	toolbar_buttons[i].nID = BCopyWorker;
	toolbar_buttons[i].enabled = FALSE;
	i++;
	toolbar_buttons[i].nID = BStart;
	toolbar_buttons[i].enabled = FALSE;
	i++;
	toolbar_buttons[i].nID = BStop;
	toolbar_buttons[i].enabled = FALSE;
	i++;
	toolbar_buttons[i].nID = BStopAll;
	toolbar_buttons[i].enabled = FALSE;
	i++;
	toolbar_buttons[i].nID = BReset;
	toolbar_buttons[i].enabled = FALSE;
	i++;
	toolbar_buttons[i].nID = BExitOne;
	toolbar_buttons[i].enabled = FALSE;
	i++;
	toolbar_buttons[i].nID = ID_APP_EXIT;
	toolbar_buttons[i].enabled = TRUE;
	i++;
	toolbar_buttons[i].nID = ID_APP_ABOUT;
	toolbar_buttons[i].enabled = TRUE;
	i++;
	ASSERT(i == NUM_TOOLBAR_BUTTONS);
}

CGalileoView::~CGalileoView()
{
	// explicitly delete sheet and pages
	delete m_pPropSheet;
	delete m_pPageDisk;
	delete m_pPageNetwork;
	delete m_pPageAccess;
	delete m_pPageDisplay;
	delete m_pPageSetup;
	delete m_pWorkerView;
}

/////////////////////////////////////////////////////////////////////////////
// CGalileoView drawing

void CGalileoView::OnDraw(CDC * pDC)
{
// we don't draw because the child window will do it all
}

/////////////////////////////////////////////////////////////////////////////
// CGalileoView diagnostics

#ifdef _DEBUG
void CGalileoView::AssertValid() const
{
	CView::AssertValid();
}

void CGalileoView::Dump(CDumpContext & dc) const
{
	CView::Dump(dc);
}

CGalileoDoc *CGalileoView::GetDocument()	// non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGalileoDoc)));
	return (CGalileoDoc *) m_pDocument;
}
#endif				//_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGalileoView message handlers

BOOL CGalileoView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
			  DWORD dwStyle, const RECT & rect, CWnd * pParentWnd, UINT nID, CCreateContext * pContext)
{
	ASSERT(pParentWnd != NULL);
	ASSERT_KINDOF(CFrameWnd, pParentWnd);

	theApp.pView = this;

	if (!CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext)) {
		return FALSE;
	}
	m_pWorkerView = new CWorkerView;
	if (!m_pWorkerView->Create(IDD_MANAGER_VIEW, this)) {
		DestroyWindow();
		return FALSE;
	}
	// Adding tab sheet pages to the display.
	m_pPageDisk = new CPageDisk;
	m_pPageNetwork = new CPageNetwork;
	m_pPageAccess = new CPageAccess;
	m_pPageDisplay = new CPageDisplay;
	m_pPageSetup = new CPageSetup;

	// Note: Add the pages in the order they should appear in the GUI.
	m_pPropSheet = new CPropertySheet(" ");
	// Add the first page, so that the call to create will succeed.
	m_pPropSheet->AddPage(m_pPageDisk);
	// create a modeless property page
	if (!m_pPropSheet->Create(this, DS_CONTEXTHELP | DS_SETFONT | WS_CHILD | WS_VISIBLE)) {
		DestroyWindow();
		return FALSE;
	}
	// Change the style of the underlying tab control so that 
	// all tabs are on the same line.
	m_pPropSheet->GetTabControl()->ModifyStyle(TCS_MULTILINE, NULL);

	// Add the remaining pages.
	m_pPropSheet->AddPage(m_pPageNetwork);
	m_pPropSheet->AddPage(m_pPageAccess);
	m_pPropSheet->AddPage(m_pPageDisplay);
	m_pPropSheet->AddPage(m_pPageSetup);

	// Activate all pages to enable dialog item access 
	// from last to first, so that the first one remains visible.
	for (int i = SETUP_PAGE; i >= 0; i--) {
		m_pPropSheet->SetActivePage(i);
	}

	///////////////////////////////////////////////////////////////////
	//
	// Arrange the topology window and property sheets in the frame.
	//
	///////////////////////////////////////////////////////////////////
	CFrameWnd *pParentFrame = GetParentFrame();
	CRect rect_size, tab_sheet_size;

	// set the property sheet in the upper left corner of the view window
	m_pWorkerView->GetWindowRect(&rect_size);

	// Moving the worker view window for a pleasing look.
	m_pWorkerView->SetWindowPos(NULL, 5, 8, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	// Move the property sheet next to the topology window.
	m_pPropSheet->SetWindowPos(NULL, rect_size.Width() + 5, 1, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

	// Size the frame window to fit the topology window and the property sheets.
	m_pPropSheet->GetWindowRect(&tab_sheet_size);

	rect_size.InflateRect(0, 0, tab_sheet_size.Width() + 18, 112);

	CFrameWnd *pFrame = GetParentFrame();

	pFrame->MoveWindow(&rect_size);

	//////////////////////////////////////////////////////////
	//
	// Center the window.
	//
	//////////////////////////////////////////////////////////
	pFrame->GetWindowRect(&rect_size);

	// Calculate the desired upper left point of the window.
	int y =::GetSystemMetrics(SM_CYSCREEN) / 2;
	int x =::GetSystemMetrics(SM_CXSCREEN) / 2;

	// Get the current center point of the window.
	CPoint app_center;

	app_center = rect_size.CenterPoint();

	// Move the window by the differnce of the desired and actual.
	rect_size.OffsetRect(x - app_center.x, y - app_center.y);
	pFrame->MoveWindow(&rect_size);

	// Insert the all managers item in the GUI.
	m_pWorkerView->Initialize();

	theApp.manager_list.ResetResults(WHOLE_TEST_PERF);
	theApp.manager_list.ResetResults(LAST_UPDATE_PERF);

	return TRUE;
}

void CGalileoView::DoDataExchange(CDataExchange * pDX)
{
	CView::DoDataExchange(pDX);
}

BOOL CGalileoView::OnEraseBkgnd(CDC * pDC)
{
	CBrush *pOldBrush;

	CBrush backBrush;

	backBrush.CreateSolidBrush(::GetSysColor(COLOR_3DFACE));	//COLOR_3DLIGHT
	backBrush.UnrealizeObject();

	CRect rectClient;

	GetClientRect(rectClient);

	pOldBrush = pDC->SelectObject(&backBrush);
	pDC->Rectangle(rectClient);

	pDC->SelectObject(pOldBrush);
	return TRUE;
}

//
// Enabling the display parts for user access.
//
void CGalileoView::EnableWindow(BOOL enable)
{
	// Do not enable the display when we are testing.
	if (enable && theApp.test_state != TestIdle)
		return;

	m_pPageDisk->EnableWindow(enable);
	m_pPageNetwork->EnableWindow(enable);
	m_pPageAccess->EnableWindow(enable);
	m_pPageSetup->EnableWindow(enable);
}

//
// Resetting display to initial start-up state.
//
void CGalileoView::Reset()
{
	// Lock the GUI while we change it's contents.
	LockWindowUpdate();
	// Reseting data for status bar.
	workers_to_run = 0;
	targets_to_run = 0;

	// Reset the managers and workers first.  This is necessary because they
	// call back to the GUI worker view to remove all workers.  (This will be
	// fixed by making the worker view a true *view* of memory only.)
	theApp.manager_list.Reset();

	// Resetting display information.
	ResetDisplayforNewTest();
	// WorkerView reset must be called before PageDisk or PageNetwork.
	m_pWorkerView->Reset();
	m_pPageDisk->Reset();
	m_pPageNetwork->Reset();
	m_pPageDisplay->Reset();

	m_pPageSetup->UpdateData(TRUE);

	ButtonReset();

	// Unlock the GUI, we are done changing the contents.
	UnlockWindowUpdate();
}

//
// Resetting display as needed.  Not all items are reset.
//
void CGalileoView::ResetDisplayforNewTest()
{
	ClearStatusBar();
	m_pPageAccess->Reset();
}

//
// Adds a manager to the display.
//
void CGalileoView::AddManager(Manager * manager)
{
	// Lock the GUI while we change it's contents.
	LockWindowUpdate();
	m_pWorkerView->AddManager(manager);
	if (theApp.manager_list.ManagerCount() == 1)
		ButtonReady();
	m_pPageDisk->ShowData();
	m_pPageNetwork->ShowData();
	EnableWindow(TRUE);
	// Unlock the GUI, we are done changing the contents.
	UnlockWindowUpdate();
}

//
// Adds a worker to the display.
//
Worker *CGalileoView::AddWorker(TargetType worker_type, Manager * manager, const CString & name)
{
	Worker *worker;

	// Lock the GUI while we change it's contents.
	LockWindowUpdate();

	// Add a worker to the manager.
	if (name.IsEmpty())
		worker = manager->AddWorker(worker_type);
	else
		worker = manager->AddWorker(worker_type, NULL, name);

	// Pass the new worker into the worker view
	if (worker) {
		m_pWorkerView->AddWorker(worker);
		m_pPageDisk->ShowData();
		m_pPageNetwork->ShowData();
	}

	EnableWindow(TRUE);
	// Unlock the GUI, we are done changing the contents.
	UnlockWindowUpdate();

	return worker;
}

void CGalileoView::OnBStart()
{
	Go();
}

void CGalileoView::Go()
{
	BOOL invalidSpecOK;

	// Show hour glass cursor.
	CWaitCursor wait;

	// Remove the focus from whoever has it.  This sends the KILLFOCUS message
	// to whoever has the focus, possibly causing an update of the GUI data.
	::SetFocus(NULL);

	// If in batch mode, assume the user knows what specs are
	// running and that they are setup properly.
	if (theApp.IsBatchMode()) {
		invalidSpecOK = TRUE;
	} else {
		invalidSpecOK = FALSE;
	}

	// Verify that all tests were defined correctly.
	if (theApp.manager_list.InvalidSetup(invalidSpecOK)) {
		return;
	}
	// Disable displays to prevent user access.
	EnableWindow(FALSE);
	ButtonOff();

	// Updating the data from the setup page
	m_pPageSetup->UpdateData(TRUE);

	// Reset access specification display.
	m_pPageAccess->Reset();

	// Figure out the name of the result file.
	if (!theApp.cmdline.GetResultFile().IsEmpty()) {
		// The result file was specified on the command line.
		result_file = theApp.cmdline.GetResultFile();

		// If the test setup indicates that results are not to be recorded,
		// record ALL results anyway.  (It makes no sense to specify a result
		// file and not have the test record results.)
		if (m_pPageSetup->result_type == RecordNone)
			m_pPageSetup->result_type = RecordAll;
	} else if (m_pPageSetup->result_type != RecordNone) {
		// The user wants to save the results to a file,
		// but no filename was specified on the command line.
		CFileDialog file_box(FALSE, "csv", "results",
				     OFN_HIDEREADONLY | OFN_NOTESTFILECREATE,
				     "CSV Files (*.csv)|*.csv|Text Files (*.txt)|*.txt|All Files (*.*)|*.*||");
		file_box.m_ofn.lpstrTitle = "Save Results";

		if (file_box.DoModal() == IDOK)
			result_file = file_box.GetPathName();
		else
			result_file.Empty();
	}
	// If a result file is being used, 
	if (!result_file.IsEmpty()) {
		// Opening result file.
		ofstream file(result_file, ios::app);

		file << setiosflags(ios::fixed) << setprecision(2);

		// Saving test configuration
		m_pPageSetup->SaveResults(&file);

		file.close();
	}
	// Save any settings that may change during the test, so they can be reset
	// after the test runs.
	SaveSettings();

	// Determine how many total tests runs there will be.
	CalcRunCount();

	// Initialize information for first test run.
	access_index = 0;
	run_index = 0;
	InitAccessSpecRun();

	// Make sure that all drives are prepared before running the test.
	// If any are not, they will be prepared, and the test will start after
	// they complete.
	worker_to_prepare = 0;
	manager_to_prepare = 0;
	if (DisksNotPrepared())
		return;

	// Start running the tests.
	SetActiveTargets();
	SaveAccessSpecs();
	StartTest();
}

//
// Mark assigned targets as active based on selected cycling option.
//
BOOL CGalileoView::SetActiveTargets()
{
	// Check for test cycling.
	switch (m_pPageSetup->test_type) {
		// Add disk_step drives for all workers to all managers at a time.
	case CyclingTargets:
		return CycleTargets();
		// Add worker_step workers, with all selected drives, to all managers.
	case CyclingWorkers:
		return CycleWorkers();
		// Add disk_step drives to worker_step workers.
	case CyclingWorkersTargets:
		return CycleWorkersTargets();
		// Add disk_step drives, down each worker, at a time for all managers.
	case CyclingIncTargetsParallel:
		return IncrementTargets();
		// Add disk_step drives, down each worker, one manager at a time.
	case CyclingIncTargetsSerial:
		return IncrementTargetsSerial();
		// Add disk_step drives, down each worker, one manager at a time.
	case CyclingQueue:
		return CycleQueue();
		// Add disk_step drives, down each worker, one manager at a time.
	case CyclingQueueTargets:
		return CycleTargetsQueue();
		// Normal test run.  Run selected targets for all workers.
	default:
		return Normal();
	}
}

//
// Set targets for all workers according to the current test and cycling mode.
// Returns TRUE for success, or FALSE if any error occurs.
//
BOOL CGalileoView::SetTargets()
{
	// Resetting targets that a worker will run.  All workers are initially given 0
	// targets to access, then the correct number is set.  Setting their count to 0
	// prevents having to keep track of which workers did not have drives set for a
	// given test.
	theApp.test_state = TestOpening;
	return theApp.manager_list.SetTargets();
}

//
// Set the access specifications for all managers and workers.
//
BOOL CGalileoView::SetAccess()
{
	return theApp.manager_list.SetAccess(access_index);
}

//
// Returns the currently running access spec's index.
//
int CGalileoView::GetCurrentAccessIndex()
{
	return access_index;
}

//
// Initialize all test cycling control variables for the next set of tests
// on the current access spec.
//
void CGalileoView::InitAccessSpecRun()
{
	worker_exponent = 1;
	target_exponent = 1;
	queue_exponent = 1;
	targets_to_run = m_pPageSetup->target_cycling.start;
	workers_to_run = m_pPageSetup->worker_cycling.start;
	queue_depth_to_run = m_pPageSetup->queue_cycling.start;

	// Start with all targets inactive to ensure proper cycling.
	theApp.manager_list.ClearActiveTargets();
}

//
// Saves and configuration settings that may be changed during testing, so
// they may be restored after testing completes.
//
void CGalileoView::SaveSettings()
{
	int i = 0, m, w, mgr_count, wkr_count;
	Manager *mgr;

	// See if queue depth will change during testing.
	if (m_pPageSetup->test_type != CyclingQueue && m_pPageSetup->test_type != CyclingQueueTargets) {
		return;
	}
	// Save queue depth settings.  Allocate an array large enough.
	save_queue_depth = (int *)malloc(theApp.manager_list.WorkerCount()
					 * sizeof(int));
	if (!save_queue_depth) {
		ErrorMessage("Could not allocate memory to save current worker "
			     "settings.  Current values will be lost.");
		return;
	}

	mgr_count = theApp.manager_list.ManagerCount();
	for (m = 0; m < mgr_count; m++) {
		mgr = theApp.manager_list.GetManager(m);
		wkr_count = mgr->WorkerCount();
		for (w = 0; w < wkr_count; w++) {
			// Assumes all targets for a worker have the same queue depth.
			save_queue_depth[i++] = mgr->GetWorker(w)->GetQueueDepth(GenericType);
		}
	}
}

//
// Restores all previously saved test configuration settings.
//
void CGalileoView::RestoreSettings()
{
	int i = 0, m, w, mgr_count, wkr_count;
	Manager *mgr;

	if (!save_queue_depth)
		return;

	// Restore saved queue depth settings.
	mgr_count = theApp.manager_list.ManagerCount();
	for (m = 0; m < mgr_count; m++) {
		mgr = theApp.manager_list.GetManager(m);
		wkr_count = mgr->WorkerCount();
		for (w = 0; w < wkr_count; w++)
			mgr->GetWorker(w)->SetQueueDepth(save_queue_depth[i++]);
	}

	// Free memory used to save configuration.
	free(save_queue_depth);
	save_queue_depth = NULL;
}

//
// Starting all testing.  This signals all Dynamos to begin performing I/Os.
//
void CGalileoView::StartTest()
{
	if (!SetAccess() || !SetTargets()) {
		TestDone(ReturnError);
		return;
	}

	theApp.test_state = TestRampingUp;
	run_index++;

	// If IOmeter was invoked from the commandline with a config file, activate the display page
	if (theApp.cmdline.GetConfigFile())
	{
		m_pPropSheet->SetActivePage(DISPLAY_PAGE);
	}

	m_pPageDisplay->Update();

	// display BigMeter if need
	if (theApp.cmdline.GetShowBigmeter())
	{
		m_pPageDisplay->ShowBigMeter(0);
	}
	
	// Display the current test status.
	UpdateTestStatus();

	// Signal Dynamo to start the next test.
	if (!theApp.manager_list.SendActiveManagers(START) || !theApp.manager_list.SendActiveManagers(BEGIN_IO)) {
		// Testing failed to successfully start.
		StopTest(ReturnError);
		return;
	}
	// Waiting ramping up period or signaling to start recording results.
	if (m_pPageSetup->ramp_time) {
		SetTimer(RAMP_TIMER, m_pPageSetup->ramp_time * 1000, NULL);
		StartRemainNotification(m_pPageSetup->ramp_time);
	} else
		StartRecording();

	// enable Stop and Stop All buttons
	ButtonTest();
	if (run_index < run_count) {
		m_pPageDisplay->m_dlgBigMeter.SetButtonState(FALSE, TRUE, TRUE);
		SetButton(BStopAll, TRUE);
	}

	theApp.m_wndToolBar.RedrawWindow();
}

//
// Starting to record results.  We are through ramping up.
//
void CGalileoView::StartRecording()
{
	int timer_delay;

	theApp.test_state = TestRecording;

	// Get amount of time to run test.
	if (timer_delay = m_pPageSetup->GetRunTime())
		SetTimer(TEST_TIMER, timer_delay, NULL);

	// Signal Dynamo to start recording.
	theApp.manager_list.SendActiveManagers(RECORD_ON);

	// Get update frequency of results display.
	if (timer_delay = m_pPageDisplay->GetUpdateDelay())
		SetTimer(DISPLAY_TIMER, timer_delay, NULL);

	// Start "remaining" notification.
	StartRemainNotification(m_pPageSetup->GetRunTime() / 1000);
}

//
// Start the 'time remaining' notification in the status bar.
//
void CGalileoView::StartRemainNotification(int timeRem)
{
	SetTimer(NOTE_TIMER, 1000, NULL);
	noteTime = timeRem;
	UpdateRemainNotification();
}

//
// Update the 'time remaining' notification in the status bar.
//
void CGalileoView::UpdateRemainNotification(void)
{
	CString note;

	if (!noteTime) {
		KillTimer(NOTE_TIMER);
		note = "";
 	} else if (theApp.test_state == TestRecording)
		note.Format("Run remaining: %d sec", noteTime);
	else
		note.Format("Ramp remaining: %d sec", noteTime);
	SetStatusBarPaneText(1, note);
	noteTime--;
}

//
// Prepare drives that need to be prepared.
//
BOOL CGalileoView::DisksNotPrepared()
{
	Manager *manager;
	int wkr_count;

	// Loop through managers.
	while (manager_to_prepare < theApp.manager_list.ManagerCount()) {
		manager = theApp.manager_list.GetManager(manager_to_prepare);

		// Loop through each manager's workers.
		wkr_count = manager->WorkerCount();
		for (worker_to_prepare = 0; worker_to_prepare < wkr_count; worker_to_prepare++) {
			// Does the worker need to prepare its targets?
			if (!manager->GetWorker(worker_to_prepare)->ReadyToRunTests()) {
				// enable StopAll button
				manager->SetTargetsToPrepare(worker_to_prepare);
				ButtonPrepare();
				SetTimer(PREPARE_TIMER, LONG_DELAY, NULL);
				SetStatusBarText(m_pPageSetup->test_name, "Preparing Drives");
				return TRUE;	// preparing
			}
		}
		manager_to_prepare++;
	}
	return FALSE;		// all drives prepared
}

void CGalileoView::OnTimer(UINT_PTR nIDEvent)
{
	int timer_delay;

	switch (nIDEvent) {
		// Display timer has expired.  Get result update.
	case DISPLAY_TIMER:
		KillTimer(nIDEvent);

		// Get a results update from Dynamo and update the display.

		// Always get results for the whole test so far.
		theApp.manager_list.UpdateResults(WHOLE_TEST_PERF);
		// Also get results since the last update, if requested.
		if (m_pPageDisplay->GetWhichPerf() == LAST_UPDATE_PERF)
			theApp.manager_list.UpdateResults(LAST_UPDATE_PERF);
		// Update the display, using whole test or last update results as appropriate.
		m_pPageDisplay->Update();

		// Reset the result update timer
		if (timer_delay = m_pPageDisplay->GetUpdateDelay())
			SetTimer(DISPLAY_TIMER, timer_delay, NULL);
		return;

		// Ramp-up timer has expired.  Beginning testing.
	case RAMP_TIMER:
		KillTimer(nIDEvent);
		StartRecording();
		return;

	case NOTE_TIMER:
		UpdateRemainNotification();
		return;

	case PREPARE_TIMER:
		// See if Dynamo has responded that the drive is prepared.
		if (!theApp.manager_list.GetManager(manager_to_prepare)->Peek(worker_to_prepare))
			return;	// not done yet - keep waiting

		// Completion message has arrived.  Stop waiting and get response.
		KillTimer(nIDEvent);
		if (!theApp.manager_list.GetManager(manager_to_prepare)->PreparedAnswer(worker_to_prepare)) {
			ErrorMessage("Failed to prepare targets for testing.");
			TestDone(ReturnError);
		}
		// See if anymore drives need to be prepared on other systems.
		if (DisksNotPrepared())
			return;

		ButtonOff();

		// Done preparing, start the test.  Clearing all active targets to
		// prevent cycling options from failing.
		theApp.manager_list.ClearActiveTargets();
		SetActiveTargets();
		SaveAccessSpecs();
		StartTest();
		return;

	case TEST_TIMER:
		StopTest(ReturnSuccess);
		return;

	case IDLE_TIMER:
		// guarantee that we call the OnIdle handler at least once in a while,
		// even if nothing else is happening
		theApp.OnIdle(0);
		return;
	}
}

void CGalileoView::OnBStop()
{
	theApp.cmdline.OverrideBatchMode();	// abort batch mode
	StopTest(ReturnSuccess);
}

void CGalileoView::StopTest(ReturnVal test_successful)
{
	bool recording;
	CWaitCursor wait;

	ButtonOff();

	// Signal all workers to stop recording if they currently are.
	if (recording = (theApp.test_state == TestRecording)) {
		theApp.test_state = TestRampingDown;
		theApp.manager_list.SendActiveManagers(RECORD_OFF);
	}
	// Stop all timers and tell all workers to stop test
	KillTimer(RAMP_TIMER);
	KillTimer(TEST_TIMER);
	KillTimer(DISPLAY_TIMER);
	KillTimer(NOTE_TIMER);
	theApp.test_state = TestIdle;
	theApp.manager_list.SendActiveManagers(STOP);

	// If we were recording results when we stopped.  Update the results.
	if (recording) {
		// Always get results for the whole test and since the last update.
		theApp.manager_list.UpdateResults(WHOLE_TEST_PERF);
		theApp.manager_list.UpdateResults(LAST_UPDATE_PERF);
		// Update the display, using whole test or last update results as appropriate.
		m_pPageDisplay->Update();
		// Save them to a file if user requested it.
		SaveResults();
	}
	// See if the testing ended abnormally or was aborted.
	if (test_successful != ReturnSuccess) {
		// If saving results, write an abort message into the result file.
		if ((m_pPageSetup->result_type != RecordNone) && !result_file.IsEmpty()) {
			ofstream file(result_file, ios::app);

			file << "Aborted test" << endl;
			file.close();
		}
		TestDone(test_successful);
		return;
	}
	// See if we need to run another test.
	if (run_index >= run_count) {
		TestDone(ReturnSuccess);
		return;
	}
	// See if there are more tests to run for the current access spec.
	if (!SetActiveTargets()) {
		// Continue running on the next access spec.  First find a spec
		// that will run.  This is necessary in case an Idle spec has been set
		// after all real access specs.
		do {
			// Move to the next access spec.
			access_index++;

			// Call Normal() to set all targets as active, then see if
			// anyone will run for the current access spec.
			Normal();
		} while (!theApp.manager_list.ActiveInCurrentTest());
		InitAccessSpecRun();
		SetActiveTargets();
		SaveAccessSpecs();
	}
	// Start the next test.
	StartTest();
}

void CGalileoView::OnBStopAll()
{
	theApp.cmdline.OverrideBatchMode();	// abort batch mode
	StopAll();
}

//
// Stopping testing or disk preparation.
//
void CGalileoView::StopAll()
{
	switch (theApp.test_state) {
	case TestPreparing:
		// We are aborting disk preparation.  Signal workers to stop.
		theApp.test_state = TestIdle;
		KillTimer(PREPARE_TIMER);
		theApp.manager_list.Send(manager_to_prepare, STOP_PREPARE, worker_to_prepare);
		theApp.manager_list.GetManager(manager_to_prepare)->Receive();

		// Notify user of the abort and re-enable the GUI.
		SetStatusBarText(m_pPageSetup->test_name, "Preparation Aborted");
		EnableWindow(TRUE);
		ButtonReady();
		break;
	case TestRampingUp:
	case TestRecording:
		// Abort testing.
		StopTest(ReturnAbort);
		break;
	default:
		ErrorMessage("Unexpected test state in CGalileoView::OnBStopAll().");
	}
}

//
// All testing has completed.
//
void CGalileoView::TestDone(ReturnVal test_successful)
{
	// If Iometer is in batch mode and the test finishes, close Iometer.
	if (theApp.IsBatchMode())
		::PostQuitMessage(0);

	// Re-enable the GUI.
	EnableWindow(TRUE);
	ButtonReady();

	switch (test_successful) {
	case ReturnSuccess:
		//If saving results, write a message into the result file to indicate all runs are complete
		//This is to allow Automated tools to know when all testing is complete.
		if ((m_pPageSetup->result_type != RecordNone) && !result_file.IsEmpty()) {
			ofstream file(result_file, ios::app);

			file << "'End Test" << endl;
			file.close();
		}
		m_pPageAccess->MarkAccesses(access_index);
		SetStatusBarText("Test Completed Successfully");
		break;
	case ReturnError:
		ErrorMessage("Testing encountered an error and is unable to continue."
			     "  See Dynamo output for additional error messages.");
	case ReturnAbort:
		theApp.test_state = TestIdle;
		SetStatusBarText("Test Aborted");
	}

	// Restore any previously saved test values.
	RestoreSettings();
}

//
// User wishes to reset the program.
//
void CGalileoView::OnBReset()
{
	::SetFocus(NULL);

	Reset();
}

//
// Saving results from test into a file.  Open the file here and have each manager, worker, etc. 
// save their own results.
//
void CGalileoView::SaveResults()
{
	// Don't do anything unless the user has specified some results to be saved
	// and a file to save them to
	if ((m_pPageSetup->result_type == RecordNone) || result_file.IsEmpty())
		return;

	ofstream file(result_file, ios::app);

	// Save results for workers and their managers.
	file << setiosflags(ios::fixed) << setprecision(6);
	theApp.manager_list.SaveResults(&file, access_index, m_pPageSetup->result_type);
	file.close();
}

//
// Saving all active access specs for the current access_index to the result_file.
//
void CGalileoView::SaveAccessSpecs()
{
	// Don't do anything unless the user has specified some results to be saved
	// and a file to save them to
	if ((m_pPageSetup->result_type == RecordNone) || result_file.IsEmpty())
		return;

	ofstream file(result_file, ios::app);

	theApp.access_spec_list.SaveResults(file);
	file.close();
}

//
// Open a file to load test specifications.
//
void CGalileoView::OnFileOpen()
{
	BOOL flags[NumICFFlags];

	// Show the custom file open dialog.
	if (file_open_box.DoModal() == IDCANCEL)
		return;

	flags[ICFTestSetupFlag] = file_open_box.isCkTestSetup;
	flags[ICFResultsDisplayFlag] = file_open_box.isCkResultsDisplay;
	flags[ICFGlobalAspecFlag] = file_open_box.isCkGlobalAccessSpec;
	flags[ICFManagerWorkerFlag] = file_open_box.isCkManagerWorker;
	flags[ICFAssignedAspecFlag] = file_open_box.isCkAssignAccessSpec;
	flags[ICFAssignedTargetFlag] = file_open_box.isCkAssignTargets;

	// Could optionally test return value to see if file was restored without errors.
	(void)PrepareToOpenConfigFile(file_open_box.GetPathName(), flags, file_open_box.isROverwrite);
}

//
// Saving current test setup information.
//
void CGalileoView::OnFileSave()
{
	BOOL flags[NumICFFlags];

	// Show the custom file save dialog.
	if (file_save_box.DoModal() == IDCANCEL)
		return;

	flags[ICFTestSetupFlag] = file_save_box.isCkTestSetup;
	flags[ICFResultsDisplayFlag] = file_save_box.isCkResultsDisplay;
	flags[ICFGlobalAspecFlag] = file_save_box.isCkGlobalAccessSpec;
	flags[ICFManagerWorkerFlag] = file_save_box.isCkManagerWorker;
	flags[ICFAssignedAspecFlag] = file_save_box.isCkAssignAccessSpec;
	flags[ICFAssignedTargetFlag] = file_save_box.isCkAssignTargets;

	// Could optionally test return value to see if file was saved without errors.
	(void)SaveConfigFile(file_save_box.GetPathName(), flags);
}

//
// Preprocesses the config file and makes sure all managers specified in
// the file are available (or waits on them), then calls OpenConfigFile.
//
// Return value of FALSE indicates failure of either this function or OpenConfigFile.
// Return value of TRUE indicates either that the file restore completed successfully
// or that managers are being waited on.  (In the latter case, no indication will be
// given about the success of the eventual call to OpenConfigFile.)
//
BOOL CGalileoView::PrepareToOpenConfigFile(const CString & infilename, BOOL * flags, BOOL replace)
{
	// Do some preliminary checking on the file before we toss
	// the filename off to the other LoadConfig routines.

	ICF_ifstream infile(infilename);

	// Was the file opened successfully?
	if (!infile.is_open()) {
		ErrorMessage("Could not open \"" + infilename + "\"");
		return FALSE;
	}
	// Verify that the file can be read.
	if (infile.rdstate()) {
		ErrorMessage("Error reading from \"" + infilename + "\"");
		infile.close();
		return FALSE;
	}

	infile.close();

	if (flags[ICFManagerWorkerFlag]) {
		// Manager configuration is being restored,

		// Prepare a map of which saved managers should
		// be restored to which managers logged into Iometer.
		theApp.manager_list.loadmap.Reset();

		if (!theApp.manager_list.LoadConfigPreprocess(infilename, flags, replace)) {
			return FALSE;
		}

		if (theApp.manager_list.loadmap.IsWaitingList()) {
			// The ManagerMap is in a waiting state.

			// Display the waiting list dialog.
			theApp.manager_list.loadmap.ShowWaitingList(infilename, flags, replace);

			// Control will immediately return to Iometer so new Dynamos can log in.
			// The user will not be allowed to interfere with Iometer while
			// the waiting list dialog is in place.

			return TRUE;
		}
	}
	// Either the loadmap has been set up or it is not needed.
	// Go ahead and start loading the config file.
	return OpenConfigFile(infilename, flags, replace);
}

//
// Perform load of Iometer Configuration File (.icf).
// This function assumes that the ManagerMap has been created and all
// "waiting list" managers necessary to restore the file have already logged in.
//
// This should only be called by OpenConfigFileWrapper or as the callback
// function for the CWaitingForManagers dialog box.
//
BOOL CGalileoView::OpenConfigFile(const CString & infilename, BOOL * flags, BOOL replace)
{
	CWaitCursor wait;
	BOOL retval;

	if (flags[ICFTestSetupFlag]) {
		// Restore test setup tab settings.

		retval = m_pPageSetup->LoadConfig(infilename);

		m_pPageSetup->UpdateData(FALSE);	// copy variables to GUI

		if (!retval)
			return FALSE;
	}

	if (flags[ICFGlobalAspecFlag]) {
		// Restore global access specification list.
		retval = theApp.access_spec_list.LoadConfig(infilename, replace);

		// Update the Access Spec page.
		m_pPageAccess->ShowGlobalAccess();
		m_pPageAccess->ShowAssignedAccess();

		// Did we fail?  (Have to check for this AFTER the update
		// in case some modifications were made before the error.)
		if (!retval)
			return FALSE;
	}

	if (flags[ICFManagerWorkerFlag]) {
		// Restore manager/worker configuration.

		// Prevent the worker view from updating until
		// all updates have taken place.
		LockWindowUpdate();

		retval = theApp.manager_list.LoadConfig(infilename,
							flags[ICFAssignedAspecFlag],
							flags[ICFAssignedTargetFlag], replace);

		UnlockWindowUpdate();

		// Update the various GUI tabs.
		m_pPageAccess->ShowAssignedAccess();
		m_pPageDisk->ShowData();
		m_pPageNetwork->ShowData();

		if (!retval)
			return FALSE;
	}

	if (flags[ICFResultsDisplayFlag]) {
		// Restore results display tab settings.
		// This must be done AFTER restoring workers/managers!

		retval = m_pPageDisplay->LoadConfig(infilename);

		m_pPageDisplay->UpdateData(FALSE);	// copy variables to GUI

		if (!retval)
			return FALSE;
	} else {
		// If results display settings is not being restored
		// from the file, reset the results display tab.
		theApp.pView->m_pPageDisplay->Reset();
	}

	return TRUE;
}

//
// Saving current test setup information.
//
BOOL CGalileoView::SaveConfigFile(const CString & outfilename, BOOL * flags)
{
	CWaitCursor wait;

	CString cstrVersion;

	cstrVersion.Format(IDS_VERSION_OUTPUT, (LPCTSTR) theApp.GetVersionString(TRUE));
	char *pVersion = cstrVersion.GetBuffer(cstrVersion.GetLength());

	// Open the specified file for output and save the test setup.
	ofstream outfile(outfilename);

	// Was the file opened successfully?
	if (!outfile.is_open()) {
		ErrorMessage("Could not open \"" + outfilename + "\" for writing.");
		return FALSE;
	}

	outfile << pVersion << endl;

	if (flags[ICFTestSetupFlag]) {
		// Save test setup tab settings.

		m_pPageSetup->UpdateData(TRUE);	// copy GUI data to variables

		if (!m_pPageSetup->SaveConfig(outfile)) {
			outfile << pVersion << endl;
			outfile << "Error while writing test setup" << endl;
			outfile.close();
			return FALSE;
		}
	}

	if (flags[ICFResultsDisplayFlag]) {
		// Save results display tab settings.

		m_pPageDisplay->UpdateData(TRUE);	// copy GUI data to variables

		if (!m_pPageDisplay->SaveConfig(outfile)) {
			outfile << pVersion << endl;
			outfile << "Error while writing results display" << endl;
			outfile.close();
			return FALSE;
		}
	}

	if (flags[ICFGlobalAspecFlag]) {
		// Save global access specification list.

		if (!theApp.access_spec_list.SaveConfig(outfile)) {
			outfile << pVersion << endl;
			outfile << "Error while writing global access spec list" << endl;
			outfile.close();
			return FALSE;
		}
	}

	if (flags[ICFManagerWorkerFlag]) {
		// Save manager/worker configuration.

		if (!theApp.manager_list.SaveConfig(outfile, flags[ICFAssignedAspecFlag], flags[ICFAssignedTargetFlag])) {
			outfile << pVersion << endl;
			outfile << "Error while writing manager/worker configuration" << endl;
			outfile.close();
			return FALSE;
		}
	}

	outfile << pVersion << endl;
	outfile.close();

	return TRUE;
}

void CGalileoView::OnBExitOne()
{
	// Forcing update to target selection information in case one is needed.
	::SetFocus(NULL);

	// Lock the GUI while we change it's contents.
	LockWindowUpdate();
	m_pWorkerView->RemoveSelectedItem();

	// Managers or workers have been removed.  We need to reset the results display
	// to make certain that it is not displaying information for the removed item.
	theApp.manager_list.ResetAllResults();
	m_pPageDisplay->Reset();
	ResetDisplayforNewTest();
	// Unlock the GUI, we are done changing the contents.
	UnlockWindowUpdate();
}

//
// Processing request to spawn a new manager.
//
void CGalileoView::OnBNewDynamo()
{
	// Spawn a Dynamo with no command line options.
	theApp.LaunchDynamo();
}

//
// The worker selection has changed.  Processing the update.
//
void CGalileoView::ChangedSelection()
{
	CWaitCursor wait;

	Manager *manager;
	Worker *worker;

	// Lock the GUI while we change it's contents.
	LockWindowUpdate();
	// Set the new worker/manager buttons if the test is idle and the
	// toolbar window exists.
	if (theApp.test_state == TestIdle &&::IsWindow(theApp.m_wndToolBar.GetToolBarCtrl().m_hWnd)) {
		manager = m_pWorkerView->GetSelectedManager();

		// Enable the new worker buttons
		SetButton(BNewDiskWorker, manager && manager->InterfaceCount(GenericDiskType));
		SetButton(BNewNetWorker, manager && manager->InterfaceCount(GenericNetType));

		// Don't allow copies of network clients.  Will be very confusing when
		// we allow multiple clients per server.
		worker = m_pWorkerView->GetSelectedWorker();
		SetButton(BCopyWorker, worker && !IsType(worker->Type(), GenericClientType));
	}
	// Display and enable the target pages, if appropriate.
	m_pPageNetwork->ShowData();
	m_pPageDisk->ShowData();

	// Update the access spec page to reflect the change in selection.
	m_pPageAccess->ShowAssignedAccess();
	m_pPageAccess->MarkAccesses(access_index);

	// Unlock the GUI, we are done changing the contents.
	UnlockWindowUpdate();
}

//
// The worker selection is changing.  Processing the update.
void CGalileoView::ChangingSelection()
{
	// Update the target selections before changing.
	::SetFocus(m_pWorkerView->m_TWorkers);
}

//
// Setting specified number of targets active for all workers on all managers 
// at one time.
//
// Returns TRUE if additional cycling occurred, signalling that another test
// should be run.  Returns FALSE if all cycling has completed for the current
// access spec.
//
BOOL CGalileoView::CycleTargets()
{
	Manager *mgr;
	Worker *wkr;
	int m, w;
	int old_count;		// targets currently active
	int new_count = 0;	// targets set active

	// Record the number of targets currently active.
	old_count = theApp.manager_list.TargetCount(ActiveType);

	// Count total # of targets that will run (sum of targets on all workers & mgrs).
	// (TBD: Make this code a separate method, and call from CycleTargetsQueue(), if
	// possible.  It would require some way of determining when cycling is complete,
	// since that's the other reason CycleTargetsQueue() is calling this method.
	// If it can be done, it would help to de-couple the 2 methods.)
	for (m = 0; m < theApp.manager_list.ManagerCount(); m++) {
		mgr = theApp.manager_list.GetManager(m);

		for (w = 0; w < mgr->WorkerCount(); w++) {
			wkr = mgr->GetWorker(w);
			if (IsType(wkr->Type(), GenericClientType))
				continue;

			// Only include workers who could be active for the current test.
			if (wkr->HasActiveCurrentSpec() && wkr->TargetCount())
				new_count += wkr->SetActiveTargets(targets_to_run);
		}
	}

	// Determine how many targets should run with the next iteration based on
	// the cycling type.
	if (m_pPageSetup->target_cycling.step_type == StepLinear || m_pPageSetup->target_cycling.step == 1) {
		targets_to_run += m_pPageSetup->target_cycling.step;
	} else {
		targets_to_run = m_pPageSetup->target_cycling.start *
		    (int)powf((float)m_pPageSetup->target_cycling.step, (float)target_exponent++);
	}

	// Return TRUE if new targets are now active.
	return (new_count > old_count);
}

//
// Add worker_step workers, with all selected targets, to all managers at a time.
//
// Returns TRUE if additional cycling occurred, signalling that another test
// should be run.  Returns FALSE if all cycling has completed for the current
// access spec.
//
BOOL CGalileoView::CycleWorkers()
{
	Manager *mgr;
	Worker *wkr;
	int m, w;
	int workers_remaining;
	int old_count;		// targets currently active
	int new_count = 0;	// targets set active

	// Record the number of targets currently active.
	old_count = theApp.manager_list.TargetCount(ActiveType);

	for (m = 0; m < theApp.manager_list.ManagerCount(); m++) {
		mgr = theApp.manager_list.GetManager(m);

		// Each manager will run with workers_to_run workers, (if the
		// manager has that many workers).
		workers_remaining = workers_to_run;

		for (w = 0; w < mgr->WorkerCount(); w++) {
			wkr = mgr->GetWorker(w);
			if (IsType(wkr->Type(), GenericClientType))
				continue;

			// Only include workers who could be active for the current test.
			if (wkr->HasActiveCurrentSpec() && wkr->TargetCount()) {
				new_count += wkr->SetActiveTargets(wkr->TargetCount());

				if (!--workers_remaining)
					break;
			}
		}
	}

	// Determine how many workers should run with the next iteration based on
	// the cycling type.
	if (m_pPageSetup->worker_cycling.step_type == StepLinear || m_pPageSetup->worker_cycling.step == 1) {
		workers_to_run += m_pPageSetup->worker_cycling.step;
	} else {
		workers_to_run = m_pPageSetup->worker_cycling.start *
		    (int)powf((float)m_pPageSetup->worker_cycling.step, (float)worker_exponent++);
	}

	// Return TRUE if new targets are now active.
	return (new_count > old_count);
}

//
// Add target_step targets to worker_step workers.
//
// Returns >0 if additional cycling occurred, signalling that another test
// should be run.  Returns 0 if all cycling has completed for the current
// access spec.
//
int CGalileoView::CycleWorkersTargets()
{
	Manager *mgr;
	Worker *wkr;
	int m, w;
	int workers_remaining;
	int max_targets = 0, max_workers = 0;
	int old_target_count;
	int new_target_count = 0, worker_count = 0;

	// Record the number of targets currently active.
	old_target_count = theApp.manager_list.TargetCount(ActiveType);

	for (m = 0; m < theApp.manager_list.ManagerCount(); m++) {
		mgr = theApp.manager_list.GetManager(m);

		// Each manager will run with workers_to_run workers, (if the
		// manager has that many workers).
		workers_remaining = workers_to_run;

		for (w = 0; w < mgr->WorkerCount(); w++) {
			wkr = mgr->GetWorker(w);
			if (IsType(wkr->Type(), GenericClientType))
				continue;

			// Only include workers who could be active for the current test.
			if (wkr->HasActiveCurrentSpec() && wkr->TargetCount()) {
				new_target_count += wkr->SetActiveTargets(targets_to_run);
				worker_count++;

				if (!--workers_remaining)
					break;
			}
		}
	}

	// Determine how many workers and targets should run with the next 
	// iteration based on the cycling type.
	if (!new_target_count) {
		// No targets were set, everyone's idle.
		return 0;
	} else if (new_target_count > old_target_count) {
		// Add additional targets to the same workers
		if (m_pPageSetup->target_cycling.step_type == StepLinear || m_pPageSetup->worker_cycling.step == 1)
			targets_to_run += m_pPageSetup->target_cycling.step;
		else
			targets_to_run = m_pPageSetup->target_cycling.start *
			    (int)powf((float)m_pPageSetup->target_cycling.step, (float)target_exponent++);
		return worker_count;
	}
	// Reset the number of targets to run and add additional workers.
	theApp.manager_list.ClearActiveTargets();
	targets_to_run = m_pPageSetup->target_cycling.start;
	target_exponent = 1;
	if (m_pPageSetup->worker_cycling.step_type == StepLinear || m_pPageSetup->worker_cycling.step == 1)
		workers_to_run += m_pPageSetup->worker_cycling.step;
	else
		workers_to_run = m_pPageSetup->worker_cycling.start *
		    (int)powf((float)m_pPageSetup->worker_cycling.step, (float)worker_exponent++);

	// Try running with the next set of workers.  If no new workers were set
	// running, we're done.
	return (CycleWorkersTargets() > worker_count);
}

//
// Add a single drive, down each worker, at a time for all managers.
//
// Returns TRUE if additional cycling occurred, signalling that another test
// should be run.  Returns FALSE if all cycling has completed for the current
// access spec.
//
BOOL CGalileoView::IncrementTargets()
{
	Manager *mgr;
	Worker *wkr;
	int m, w;
	int targets_remaining;
	int old_count;		// targets currently active
	int set_count, new_count = 0;	// targets set active

	// Record the number of targets currently active.
	old_count = theApp.manager_list.TargetCount(ActiveType);

	for (m = 0; m < theApp.manager_list.ManagerCount(); m++) {
		mgr = theApp.manager_list.GetManager(m);
		targets_remaining = targets_to_run;

		for (w = 0; w < mgr->WorkerCount(); w++) {
			wkr = mgr->GetWorker(w);
			if (IsType(wkr->Type(), GenericClientType))
				continue;

			// Only include workers who could be active for the current test.
			if (wkr->HasActiveCurrentSpec() && wkr->TargetCount()) {
				set_count = wkr->SetActiveTargets(targets_remaining);
				new_count += set_count;
				targets_remaining -= set_count;
			}
		}
	}

	// Determine how many targets should run with the next iteration based on
	// the cycling type.
	if (m_pPageSetup->target_cycling.step_type == StepLinear || m_pPageSetup->target_cycling.step == 1) {
		targets_to_run += m_pPageSetup->target_cycling.step;
	} else {
		targets_to_run = m_pPageSetup->target_cycling.start *
		    (int)powf((float)m_pPageSetup->target_cycling.step, (float)target_exponent++);
	}

	// Return TRUE if new targets are now active.
	return (new_count > old_count);
}

//
// Add a single drive, down each worker, for one manager at a time.
//
// Returns TRUE if additional cycling occurred, signalling that another test
// should be run.  Returns FALSE if all cycling has completed for the current
// access spec.
//
BOOL CGalileoView::IncrementTargetsSerial()
{
	Manager *mgr;
	Worker *wkr;
	int m, w;
	int targets_remaining;
	int old_count;		// targets currently active
	int set_count, new_count = 0;	// targets set active

	// Record the number of targets currently active.
	old_count = theApp.manager_list.TargetCount(ActiveType);
	targets_remaining = targets_to_run;

	for (m = 0; m < theApp.manager_list.ManagerCount(); m++) {
		mgr = theApp.manager_list.GetManager(m);

		for (w = 0; w < mgr->WorkerCount(); w++) {
			wkr = mgr->GetWorker(w);
			if (IsType(wkr->Type(), GenericClientType))
				continue;

			// Only include workers who could be active for the current test.
			if (wkr->HasActiveCurrentSpec() && wkr->TargetCount()) {
				set_count = wkr->SetActiveTargets(targets_remaining);
				new_count += set_count;
				targets_remaining -= set_count;
			}
		}
	}

	// Determine how many targets should run with the next iteration based on
	// the cycling type.
	if (m_pPageSetup->target_cycling.step_type == StepLinear || m_pPageSetup->target_cycling.step == 1) {
		targets_to_run += m_pPageSetup->target_cycling.step;
	} else {
		targets_to_run = m_pPageSetup->target_cycling.start *
		    (int)powf((float)m_pPageSetup->target_cycling.step, (float)target_exponent++);
	}

	// Return TRUE if new targets are now active.
	return (new_count > old_count);
}

//
// Run selected drives cycling the queue depth for all workers.
//
// Returns TRUE if additional cycling occurred, signalling that another test
// should be run.  Returns FALSE if all cycling has completed for the current
// access spec.
//
BOOL CGalileoView::CycleQueue()
{
	Manager *mgr;
	Worker *wkr;
	int m, w;
	BOOL workers_running = FALSE;
	BOOL disk_workers = FALSE;

	// Stop cycling when our queue depth to run is greater than the last
	// queue depth to run.
	if (queue_depth_to_run > m_pPageSetup->queue_cycling.end)
		return FALSE;

	for (m = 0; m < theApp.manager_list.ManagerCount(); m++) {
		mgr = theApp.manager_list.GetManager(m);

		for (w = 0; w < mgr->WorkerCount(); w++) {
			wkr = mgr->GetWorker(w);
			if (IsType(wkr->Type(), GenericClientType))
				continue;

			// Only include workers who could be active for the current test.
			if (wkr->HasActiveCurrentSpec() && wkr->TargetCount()) {
				wkr->SetQueueDepth(queue_depth_to_run);
				wkr->SetActiveTargets(wkr->TargetCount());
				workers_running = TRUE;

				// Record if any disk workers are running.
				if (IsType(wkr->Type(), GenericDiskType))
					disk_workers = TRUE;
			}
		}
	}

	// Continue cycling queue depth only if there are disk workers running.
	if (!disk_workers) {
		// No disk workers running, but we might have idle network workers.
		// Let them run, but set the queue depth past the last value to stop
		// cycling queue depth for these workers.
		queue_depth_to_run = m_pPageSetup->queue_cycling.end + 1;
	} else if (m_pPageSetup->queue_cycling.step_type == StepLinear || m_pPageSetup->queue_cycling.step == 1)
		queue_depth_to_run += m_pPageSetup->queue_cycling.step;
	else
		queue_depth_to_run = m_pPageSetup->queue_cycling.start *
		    (int)powf((float)m_pPageSetup->queue_cycling.step, (float)queue_exponent++);

	// Continue to cycle queue depth as long as all workers are not idle.
	return workers_running;
}

//
// Cycle through queue depth while cycling through targets.
//
// Returns TRUE if additional cycling occurred, signalling that another test
// should be run.  Returns FALSE if all cycling has completed for the current
// access spec.
//
BOOL CGalileoView::CycleTargetsQueue()
{
	int save_targets_to_run;
	int save_target_exponent;

	// Cycle through all queue depths for the selected number of targets.
	if (CycleQueue()) {
		// Set the appropriate number of targets.
		theApp.manager_list.ClearActiveTargets();
		save_targets_to_run = targets_to_run;
		save_target_exponent = target_exponent;
		CycleTargets();

		// Revert to the saved values.  We don't want to run with more targets
		// until we're done cycling queue depth.
		targets_to_run = save_targets_to_run;
		target_exponent = save_target_exponent;
	} else {
		// See if we're through cycling targets as well.  Call CycleTargets()
		// to update the number of targets that we need to run.
		// targets_to_run was set back to a saved value above, so we know that
		// this call will succeed.
		CycleTargets();
		save_targets_to_run = targets_to_run;
		save_target_exponent = target_exponent;

		// Now try to cycle with more targets.  This check determines if we're
		// through cycling targets.
		if (!CycleTargets())
			return FALSE;
		targets_to_run = save_targets_to_run;
		target_exponent = save_target_exponent;

		// Cycling additional targets, reset the queue depth.
		queue_exponent = 1;
		queue_depth_to_run = m_pPageSetup->queue_cycling.start;
		CycleQueue();

		// Reset the targets that need to run, CycleQueue() will have cleared
		// the settings.
		theApp.manager_list.ClearActiveTargets();
		CycleTargets();
		targets_to_run = save_targets_to_run;
		target_exponent = save_target_exponent;
	}

	return TRUE;
}

//
// Prepares all worker targets for a normal test run.  All assigned targets
// are listed as active.
//
// Returns TRUE if additional cycling occurred, signalling that another test
// should be run.  Returns FALSE if all cycling has completed for the current
// access spec.
//
BOOL CGalileoView::Normal()
{
	Manager *mgr;
	Worker *wkr;
	int m, w;
	int old_count;		// targets currently active
	int new_count = 0;	// targets set active

	// Record the number of targets currently active.
	old_count = theApp.manager_list.TargetCount(ActiveType);

	// Mark all assigned targets as active.
	for (m = 0; m < theApp.manager_list.ManagerCount(); m++) {
		mgr = theApp.manager_list.GetManager(m);

		for (w = 0; w < mgr->WorkerCount(); w++) {
			wkr = mgr->GetWorker(w);
			if (IsType(wkr->Type(), GenericClientType))
				continue;

			// Only include workers who could be active for the current test.
			if (wkr->HasActiveCurrentSpec() && wkr->TargetCount())
				new_count += mgr->SetActiveTargets(w, wkr->TargetCount());
		}
	}

	// Return TRUE if new targets are now active.
	return (new_count > old_count);
}

void CGalileoView::OnMRefresh()
{
	Manager *manager = (Manager *) m_pWorkerView->m_TWorkers.GetItemData(right_clicked_item);

	// Updates the target lists when the user right clicks on a manager.
	manager->UpdateTargetLists();
	m_pPageDisk->ShowData();
	m_pPageNetwork->ShowData();
}

void CGalileoView::ButtonReset()
{
	if (theApp.m_wndToolBar) {
		SetButton(ID_FILE_OPEN, TRUE);
		SetButton(ID_FILE_SAVE, TRUE);
		SetButton(BNewDynamo, TRUE);
		SetButton(BNewDiskWorker, FALSE);
		SetButton(BNewNetWorker, FALSE);
		SetButton(BCopyWorker, FALSE);
		SetButton(BReset, FALSE);
		SetButton(BExitOne, FALSE);
		SetButton(ID_APP_EXIT, TRUE);

		m_pPageDisplay->m_dlgBigMeter.SetButtonState(FALSE, FALSE, FALSE);
		SetButton(BStart, FALSE);
		SetButton(BStop, FALSE);
		SetButton(BStopAll, FALSE);

		theApp.m_wndToolBar.RedrawWindow();
	}
}

//
// Set buttons to indicate that we're ready to run the next test.
//
void CGalileoView::ButtonReady()
{
	if (theApp.m_wndToolBar) {
		Manager *manager = m_pWorkerView->GetSelectedManager();
		BOOL new_disk_worker_ok;
		BOOL new_net_worker_ok;

		SetButton(ID_FILE_OPEN, TRUE);
		SetButton(ID_FILE_SAVE, TRUE);
		SetButton(BNewDynamo, TRUE);

		// Only enable the new worker buttons if creating a worker of that kind
		// is allowed on the given manager or worker's manager.
		if (manager) {
			new_disk_worker_ok = manager->InterfaceCount(GenericDiskType);
			new_net_worker_ok = manager->InterfaceCount(GenericNetType);
		} else {
			new_disk_worker_ok = FALSE;
			new_net_worker_ok = FALSE;
		}
		SetButton(BNewDiskWorker, new_disk_worker_ok);
		SetButton(BNewNetWorker, new_net_worker_ok);

		// Enable worker copy button if a worker is selected.
		SetButton(BCopyWorker, (BOOL) (PtrToLong(m_pWorkerView->GetSelectedWorker())));
		SetButton(BReset, TRUE);
		SetButton(BExitOne, TRUE);
		SetButton(ID_APP_EXIT, TRUE);

		m_pPageDisplay->m_dlgBigMeter.SetButtonState(TRUE, FALSE, FALSE);
		SetButton(BStart, TRUE);
		SetButton(BStop, FALSE);
		SetButton(BStopAll, FALSE);

		theApp.m_wndToolBar.RedrawWindow();
	}
}

//
// Set buttons to indicate that we're preparing a drive.
//
void CGalileoView::ButtonPrepare()
{
	if (theApp.m_wndToolBar) {
		SetButton(ID_FILE_OPEN, FALSE);
		SetButton(ID_FILE_SAVE, FALSE);
		SetButton(BNewDynamo, FALSE);
		SetButton(BNewDiskWorker, FALSE);
		SetButton(BNewNetWorker, FALSE);
		SetButton(BCopyWorker, FALSE);
		SetButton(BReset, FALSE);
		SetButton(BExitOne, FALSE);
		SetButton(ID_APP_EXIT, TRUE);

		m_pPageDisplay->m_dlgBigMeter.SetButtonState(FALSE, FALSE, TRUE);
		SetButton(BStart, FALSE);
		SetButton(BStop, FALSE);
		SetButton(BStopAll, TRUE);

		theApp.m_wndToolBar.RedrawWindow();
	}
}

//
// Set buttons to indicate that we're running a test.
//
void CGalileoView::ButtonTest()
{
	if (theApp.m_wndToolBar) {
		SetButton(ID_FILE_OPEN, FALSE);
		SetButton(ID_FILE_SAVE, FALSE);
		SetButton(BNewDynamo, FALSE);
		SetButton(BNewDiskWorker, FALSE);
		SetButton(BNewNetWorker, FALSE);
		SetButton(BCopyWorker, FALSE);
		SetButton(BReset, FALSE);
		SetButton(BExitOne, FALSE);
		SetButton(ID_APP_EXIT, TRUE);

		m_pPageDisplay->m_dlgBigMeter.SetButtonState(FALSE, TRUE, FALSE);
		SetButton(BStart, FALSE);
		SetButton(BStop, TRUE);
		SetButton(BStopAll, FALSE);
	}
}

//
// Disable most buttons on the GUI to prevent user access.
//
void CGalileoView::ButtonOff()
{
	if (theApp.m_wndToolBar) {
		SetButton(ID_FILE_OPEN, FALSE);
		SetButton(ID_FILE_SAVE, FALSE);
		SetButton(BNewDynamo, FALSE);
		SetButton(BNewDiskWorker, FALSE);
		SetButton(BNewNetWorker, FALSE);
		SetButton(BCopyWorker, FALSE);
		SetButton(BReset, FALSE);
		SetButton(BExitOne, FALSE);
		SetButton(ID_APP_EXIT, TRUE);

		m_pPageDisplay->m_dlgBigMeter.SetButtonState(FALSE, FALSE, FALSE);
		SetButton(BStart, FALSE);
		SetButton(BStop, FALSE);
		SetButton(BStopAll, FALSE);

		theApp.m_wndToolBar.RedrawWindow();
	}
}

//
// Setting the text in the status bar to inform the user to the status of which test
// is running and how many are runs are left.
//
void CGalileoView::UpdateTestStatus()
{
	CString blank, run;

	// Mark the currently running access spec.
	m_pPageAccess->MarkAccesses(access_index);

	// Display access specification and run numbers.
	blank = "";
	run.Format("Run %d of %d", run_index, run_count);

	SetStatusBarText(m_pPageSetup->test_name, blank, run);
}

//
// Updating the status bar with a single notification message.
//
void CGalileoView::SetStatusBarText(CString text1, CString text2, CString text3)
{
	if (theApp.m_wndStatusBar) {
		theApp.m_wndStatusBar.GetStatusBarCtrl().SetText(text1, 0, 0);
		theApp.m_wndStatusBar.GetStatusBarCtrl().SetText(text2, 1, 0);
		theApp.m_wndStatusBar.GetStatusBarCtrl().SetText(text3, 2, 0);
	}
}

//
// Updating a pane in the status bar with a single notification message.
//
void CGalileoView::SetStatusBarPaneText(int paneIndex, CString text)
{
	if (theApp.m_wndStatusBar)
		theApp.m_wndStatusBar.GetStatusBarCtrl().SetText(text, paneIndex, 0);
}

//
// Clearing all information displayed in the status bar.
//
void CGalileoView::ClearStatusBar()
{
	SetStatusBarText();
}

//
// Calculates the total number of runs for all testing.
//
void CGalileoView::CalcRunCount()
{
	int old_count, max_spec_count;

	run_count = 0;

	// Determine the total number of runs for each access spec.
	max_spec_count = theApp.manager_list.GetMaxAccessSpecCount();
	for (access_index = 0; access_index < max_spec_count; access_index++) {
		InitAccessSpecRun();

		// Ensure that we do at least one run per access spec.  This is
		// necessary in case all workers have been set idle.
		old_count = run_count;

		switch (m_pPageSetup->test_type) {
		case CyclingTargets:
			while (CycleTargets())
				run_count++;
			break;
		case CyclingWorkers:
			while (CycleWorkers())
				run_count++;
			break;
		case CyclingIncTargetsParallel:
			while (IncrementTargets())
				run_count++;
			break;
		case CyclingIncTargetsSerial:
			while (IncrementTargetsSerial())
				run_count++;
			break;
		case CyclingWorkersTargets:
			while (CycleWorkersTargets())
				run_count++;
			break;
		case CyclingQueue:
			while (CycleQueue())
				run_count++;
			break;
		case CyclingQueueTargets:
			while (CycleTargetsQueue())
				run_count++;
			break;
		default:
			if (Normal())
				run_count++;
		}

		// If old_count = run_count, then all workers were marked as idle and
		// no new targets were set.  If the user specified that someone should 
		// actually be idle, increment the run count.
		if (old_count == run_count && theApp.manager_list.HasIdleCurrentSpec()) {
			run_count++;
		}
	}
}

BOOL CGalileoView::OnSetCursor(CWnd * pWnd, UINT nHitTest, UINT message)
{
	// Check to see if the mouse is moved during a drag event.
	if (message == WM_MOUSEMOVE && dragging) {
		CPoint point;

		GetCursorPos(&point);
		p_DragImage->DragShowNolock(TRUE);
		p_DragImage->DragMove(point);
		return TRUE;
	}
	// Check to see if the button up event happened after a drag event.
	if (message == WM_LBUTTONUP && dragging) {
		p_DragImage->EndDrag();
		delete p_DragImage;

		CPoint point;

		GetCursorPos(&point);

		m_pPageDisplay->SetResultSource(m_pWorkerView->GetSelectedManagerIndex(),
						m_pWorkerView->GetSelectedWorkerIndex(), point);
		dragging = FALSE;	// Reset dragging flag.
		return TRUE;
	}

	return CView::OnSetCursor(pWnd, nHitTest, message);
}

TargetType CGalileoView::GetSelectedTargetType()
{
	Worker *worker;

	// See if a worker was selected.
	if (worker = m_pWorkerView->GetSelectedWorker())
		// Return selected worker's type
		return worker->Type();

	return InvalidType;
}

//
// called as the CGalileoView window is being destroyed (i.e. on application exit)
//
void CGalileoView::OnDestroy()
{
	// If a test is running, stop it!
	if (theApp.test_state != TestIdle) {
		// Test was running or drives were being prepared, abort.
		StopAll();
	}
	// Remove all manager and worker information.
	theApp.manager_list.RemoveAllManagers();

	CView::OnDestroy();
}

//
// Processing request to spawn a new worker thread.
// This is called by the OnBNewDiskWorker() and OnBNewNetWorker() handlers.
// It is BYPASSED by AddDefaultWorkers().
//
void CGalileoView::OnNewWorker(TargetType worker_type)
{
	Manager *manager;

	// Forcing update to target selection information in case one is needed.
	::SetFocus(NULL);

	// Updating the data from the setup page
	m_pPageSetup->UpdateData(TRUE);
	ResetDisplayforNewTest();

	// Seeing if there is a manager who's selected to receive the new worker.
	manager = m_pWorkerView->GetSelectedManager();

	if (manager) {
		AddWorker(worker_type, manager);
	} else {
		// Shouldn't ever get to this code.  (Button should be disabled.)
		ErrorMessage("You must first highlight the manager you wish to start a worker on."
			     "  This should not have been allowed by Iometer.");
	}
}

void CGalileoView::OnBNewDiskWorker()
{
	OnNewWorker(GenericDiskType);
}

void CGalileoView::OnBNewNetWorker()
{
	OnNewWorker(GenericServerType);
}

//
// Adds the number of disk and network workers specified in Test Setup to a manager.
//
void CGalileoView::AddDefaultWorkers(Manager * manager)
{
	int i;
	int number_of_workers;

	if (manager->InterfaceCount(GenericDiskType)) {
		// Get default number of disk workers.
		number_of_workers = m_pPageSetup->disk_worker_count;

		if (number_of_workers == -1) {
			number_of_workers = manager->processors;
		}
		// Add default number of disk workers.
		for (i = 0; i < number_of_workers; i++)
			AddWorker(GenericDiskType, manager);
	}

	if (manager->InterfaceCount(GenericNetType)) {
		// Get default number of net workers.
		number_of_workers = m_pPageSetup->net_worker_count;

		if (number_of_workers == -1) {
			number_of_workers = manager->processors;
		}
		// Add default number of net workers.
		for (i = 0; i < number_of_workers; i++)
			AddWorker(GenericServerType, manager);
	}
}

//
// Adds a worker to the display.
//
void CGalileoView::OnBCopyWorker()
{
	Manager *src_manager;
	Worker *src_worker, *new_worker;

	// Forcing update to target selection information in case one is needed.
	::SetFocus(NULL);

	// Updating the data from the setup page.
	m_pPageSetup->UpdateData(TRUE);
	ResetDisplayforNewTest();

	src_manager = m_pWorkerView->GetSelectedManager();
	src_worker = m_pWorkerView->GetSelectedWorker();

	if (!src_worker) {
		ErrorMessage("No worker was selected.  " "This should not have been allowed by Iometer.");
		return;
	}
	// Add the new worker to the manager and retrieve its pointer.
	new_worker = src_manager->AddWorker(src_worker->Type(), src_worker);

	if (new_worker)
		m_pWorkerView->AddWorker(new_worker);

	EnableWindow(TRUE);
}

//
// The OnMDisplay...() functions are called in response to a selection from the 
// IDR_POPUP_DISPLAY_LIST menu, which is popped up by the function CPageDisplay::OnBResultType(). 
// The main purpose of this group of functions is to map menu item IDs to *_RESULT constants.
//
// IDR_POPUP_DISPLAY_LIST menu item ID's have the following format:
//
//     MDisplay<sss><xxxx>
//
// where <sss> is a 3-letter code indicating the submenu ("CPU" for CPU, "Avg" for Average 
// Latency, "Max" for Maximum Latency, "Ops" for Operations per Second, etc.) 
// and <xxxx> is a word indicating the specific selection from that submenu.  
// Within <xxxx>, "R" means Read, "W" means Write, "PS" means Per Second, "IO" means I/Os,
// "Con" means Connections, and "Trans" means Transactions.
//
// Each IDR_POPUP_DISPLAY_LIST submenu also has an ID, with the following format:
//
//    MDisplay<sss>SubmenuID
//
// where <sss> is the same 3-letter code used in menu item ID's.
//
// We need many separate functions since a menu selection handler does not receive any
// parameters indicating which menu item was selected.  Each one just calls 
// CGalileoView::OnMDisplay() to handle the selection, with parameters indicating
// which menu item was selected and which submenu it is found on.  
//
// CGalileoView::OnMDisplay() determines the *_RESULT constant (defined in PageDisplay.h)
// describing the corresponding result, then calls either CBigMeter::OnMDisplay() (if the
// big meter dialog is open) or CPageDisplay::OnMDisplay().  CPageDisplay figures out
// which of its buttons was changed from data it maintains.
//
// These functions must be changed whenever a menu item in IDR_POPUP_DISPLAY_LIST
// changes its resource ID, moves to a different submenu, or changes its functionality
// (different *_RESULT constant).
//
void CGalileoView::OnMDisplayCPUDPC()
{
	OnMDisplay(MDisplayCPUSubmenuID, MDisplayCPUDPC);
}

void CGalileoView::OnMDisplayCPUEffectiveness()
{
	OnMDisplay(MDisplayCPUSubmenuID, MDisplayCPUEffectiveness);
}

void CGalileoView::OnMDisplayCPUInterruptsPS()
{
	OnMDisplay(MDisplayCPUSubmenuID, MDisplayCPUInterruptsPS);
}

void CGalileoView::OnMDisplayCPUInterruptTime()
{
	OnMDisplay(MDisplayCPUSubmenuID, MDisplayCPUInterruptTime);
}

void CGalileoView::OnMDisplayCPUPrivileged()
{
	OnMDisplay(MDisplayCPUSubmenuID, MDisplayCPUPrivileged);
}

void CGalileoView::OnMDisplayCPUUser()
{
	OnMDisplay(MDisplayCPUSubmenuID, MDisplayCPUUser);
}

void CGalileoView::OnMDisplayCPUUtilization()
{
	OnMDisplay(MDisplayCPUSubmenuID, MDisplayCPUUtilization);
}

void CGalileoView::OnMDisplayErrIO()
{
	OnMDisplay(MDisplayErrSubmenuID, MDisplayErrIO);
}

void CGalileoView::OnMDisplayErrRIO()
{
	OnMDisplay(MDisplayErrSubmenuID, MDisplayErrRIO);
}

void CGalileoView::OnMDisplayErrWIO()
{
	OnMDisplay(MDisplayErrSubmenuID, MDisplayErrWIO);
}

void CGalileoView::OnMDisplayAvgCon()
{
	OnMDisplay(MDisplayAvgSubmenuID, MDisplayAvgCon);
}

void CGalileoView::OnMDisplayAvgIO()
{
	OnMDisplay(MDisplayAvgSubmenuID, MDisplayAvgIO);
}

void CGalileoView::OnMDisplayAvgRIO()
{
	OnMDisplay(MDisplayAvgSubmenuID, MDisplayAvgRIO);
}

void CGalileoView::OnMDisplayAvgTrans()
{
	OnMDisplay(MDisplayAvgSubmenuID, MDisplayAvgTrans);
}

void CGalileoView::OnMDisplayAvgWIO()
{
	OnMDisplay(MDisplayAvgSubmenuID, MDisplayAvgWIO);
}

void CGalileoView::OnMDisplayMaxCon()
{
	OnMDisplay(MDisplayMaxSubmenuID, MDisplayMaxCon);
}

void CGalileoView::OnMDisplayMaxIO()
{
	OnMDisplay(MDisplayMaxSubmenuID, MDisplayMaxIO);
}

void CGalileoView::OnMDisplayMaxRIO()
{
	OnMDisplay(MDisplayMaxSubmenuID, MDisplayMaxRIO);
}

void CGalileoView::OnMDisplayMaxTrans()
{
	OnMDisplay(MDisplayMaxSubmenuID, MDisplayMaxTrans);
}

void CGalileoView::OnMDisplayMaxWIO()
{
	OnMDisplay(MDisplayMaxSubmenuID, MDisplayMaxWIO);
}

void CGalileoView::OnMDisplayMBsMBPSBin()
{
	OnMDisplay(MDisplayMBsSubmenuID, MDisplayMBsMBPSBin);
}

void CGalileoView::OnMDisplayMBsRMBPSBin()
{
	OnMDisplay(MDisplayMBsSubmenuID, MDisplayMBsRMBPSBin);
}

void CGalileoView::OnMDisplayMBsWMBPSBin()
{
	OnMDisplay(MDisplayMBsSubmenuID, MDisplayMBsWMBPSBin);
}

void CGalileoView::OnMDisplayMBsMBPSDec()
{
	OnMDisplay(MDisplayMBsSubmenuID, MDisplayMBsMBPSDec);
}

void CGalileoView::OnMDisplayMBsRMBPSDec()
{
	OnMDisplay(MDisplayMBsSubmenuID, MDisplayMBsRMBPSDec);
}

void CGalileoView::OnMDisplayMBsWMBPSDec()
{
	OnMDisplay(MDisplayMBsSubmenuID, MDisplayMBsWMBPSDec);
}

void CGalileoView::OnMDisplayNetPacketErrors()
{
	OnMDisplay(MDisplayNetSubmenuID, MDisplayNetPacketErrors);
}

void CGalileoView::OnMDisplayNetPacketsPS()
{
	OnMDisplay(MDisplayNetSubmenuID, MDisplayNetPacketsPS);
}

void CGalileoView::OnMDisplayNetRetransPS()
{
	OnMDisplay(MDisplayNetSubmenuID, MDisplayNetRetransPS);
}

void CGalileoView::OnMDisplayOpsConPS()
{
	OnMDisplay(MDisplayOpsSubmenuID, MDisplayOpsConPS);
}

void CGalileoView::OnMDisplayOpsIOPS()
{
	OnMDisplay(MDisplayOpsSubmenuID, MDisplayOpsIOPS);
}

void CGalileoView::OnMDisplayOpsRIOPS()
{
	OnMDisplay(MDisplayOpsSubmenuID, MDisplayOpsRIOPS);
}

void CGalileoView::OnMDisplayOpsTransPS()
{
	OnMDisplay(MDisplayOpsSubmenuID, MDisplayOpsTransPS);
}

void CGalileoView::OnMDisplayOpsWIOPS()
{
	OnMDisplay(MDisplayOpsSubmenuID, MDisplayOpsWIOPS);
}

//
// This function determines the _RESULT constant for the selected menu item, then 
// passes the word along to the appropriate display dialog.  (See the explanation
// just above the OnMDisplay...() functions.)
//
void CGalileoView::OnMDisplay(int submenu_id, int menu_item)
{
	int result_code;

	// Determine result code associated with selected menu item.
	switch (menu_item) {
	case MDisplayCPUDPC:
		result_code = DPC_UTILIZATION_RESULT;
		break;
	case MDisplayCPUEffectiveness:
		result_code = CPU_EFFECTIVENESS_RESULT;
		break;
	case MDisplayCPUInterruptsPS:
		result_code = CPU_INTERRUPT_RESULT;
		break;
	case MDisplayCPUInterruptTime:
		result_code = IRQ_UTILIZATION_RESULT;
		break;
	case MDisplayCPUPrivileged:
		result_code = PRIVILEGED_UTILIZATION_RESULT;
		break;
	case MDisplayCPUUser:
		result_code = USER_UTILIZATION_RESULT;
		break;
	case MDisplayCPUUtilization:
		result_code = CPU_UTILIZATION_RESULT;
		break;
	case MDisplayErrIO:
		result_code = ERROR_RESULT;
		break;
	case MDisplayErrRIO:
		result_code = READ_ERROR_RESULT;
		break;
	case MDisplayErrWIO:
		result_code = WRITE_ERROR_RESULT;
		break;
	case MDisplayAvgCon:
		result_code = AVG_CONNECTION_LATENCY_RESULT;
		break;
	case MDisplayAvgIO:
		result_code = AVG_LATENCY_RESULT;
		break;
	case MDisplayAvgRIO:
		result_code = AVG_READ_LATENCY_RESULT;
		break;
	case MDisplayAvgTrans:
		result_code = AVG_TRANSACTION_LATENCY_RESULT;
		break;
	case MDisplayAvgWIO:
		result_code = AVG_WRITE_LATENCY_RESULT;
		break;
	case MDisplayMaxCon:
		result_code = MAX_CONNECTION_LATENCY_RESULT;
		break;
	case MDisplayMaxIO:
		result_code = MAX_LATENCY_RESULT;
		break;
	case MDisplayMaxRIO:
		result_code = MAX_READ_LATENCY_RESULT;
		break;
	case MDisplayMaxTrans:
		result_code = MAX_TRANSACTION_LATENCY_RESULT;
		break;
	case MDisplayMaxWIO:
		result_code = MAX_WRITE_LATENCY_RESULT;
		break;
	case MDisplayMBsMBPSBin:
		result_code = MBPS_BIN_RESULT;
		break;
	case MDisplayMBsRMBPSBin:
		result_code = READ_MBPS_BIN_RESULT;
		break;
	case MDisplayMBsWMBPSBin:
		result_code = WRITE_MBPS_BIN_RESULT;
		break;
	case MDisplayMBsMBPSDec:
		result_code = MBPS_DEC_RESULT;
		break;
	case MDisplayMBsRMBPSDec:
		result_code = READ_MBPS_DEC_RESULT;
		break;
	case MDisplayMBsWMBPSDec:
		result_code = WRITE_MBPS_DEC_RESULT;
		break;
	case MDisplayNetPacketErrors:
		result_code = NI_ERROR_RESULT;
		break;
	case MDisplayNetPacketsPS:
		result_code = NI_PACKET_RESULT;
		break;
	case MDisplayNetRetransPS:
		result_code = TCP_SEGMENT_RESULT;
		break;
	case MDisplayOpsConPS:
		result_code = CONNECTION_PER_SEC_RESULT;
		break;
	case MDisplayOpsIOPS:
		result_code = IOPS_RESULT;
		break;
	case MDisplayOpsRIOPS:
		result_code = READ_IOPS_RESULT;
		break;
	case MDisplayOpsTransPS:
		result_code = TRANSACTION_PER_SEC_RESULT;
		break;
	case MDisplayOpsWIOPS:
		result_code = WRITE_IOPS_RESULT;
		break;
	default:
		ErrorMessage("Invalid menu_item value in CGalileoView::OnMDisplay()");
		return;
	}

	// If the big meter dialog is open, dispatch this message to it.
	// Otherwise, send it to CPageDisplay to figure out which button will be updated.
	if (m_pPageDisplay->m_dlgBigMeter.is_displayed)
		m_pPageDisplay->m_dlgBigMeter.OnMDisplay(submenu_id, menu_item, result_code);
	else
		m_pPageDisplay->OnMDisplay(submenu_id, menu_item, result_code);
}
