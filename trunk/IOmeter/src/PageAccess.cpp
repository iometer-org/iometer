/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / PageAccess.cpp                                         ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implementation of the CPageAccess class, which is     ## */
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
/* ##  Changes ...: 2004-05-13 (lamontcranston41@yahoo.com)               ## */
/* ##               - Make sure the current access spec is visible        ## */
/* ##               2003-10-17 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-04-25 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Updated the global debug flag (_DEBUG) handling     ## */
/* ##                 of the source file (check for platform etc.).       ## */
/* ##               2003-03-02 (joe@eiler.net)                            ## */
/* ##               - Changed LONG_PTR to ULONG_PTR, which is what it is  ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include "stdafx.h"
#include "PageAccess.h"
#include "GalileoView.h"
#include "AccessDialog.h"

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

/////////////////////////////////////////////////////////////////////////////
// CPageAccess property page

IMPLEMENT_DYNCREATE(CPageAccess, CPropertyPage)

CPageAccess::CPageAccess():CPropertyPage(CPageAccess::IDD)
{
	global_dragging = FALSE;
	assigned_dragging = FALSE;
	//{{AFX_DATA_INIT(CPageAccess)
	//}}AFX_DATA_INIT
}

void CPageAccess::DoDataExchange(CDataExchange * pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPageAccess)
	DDX_Control(pDX, GGlobalFrame, m_GGlobalFrame);
	DDX_Control(pDX, GAssignedFrame, m_GAssignedFrame);
	DDX_Control(pDX, BEditCopy, m_BEditCopy);
	DDX_Control(pDX, LAssignedAccess, m_LAssignedAccess);
	DDX_Control(pDX, BUp, m_BUp);
	DDX_Control(pDX, LGlobalAccess, m_LGlobalAccess);
	DDX_Control(pDX, BRemove, m_BRemove);
	DDX_Control(pDX, BNew, m_BNew);
	DDX_Control(pDX, BEdit, m_BEdit);
	DDX_Control(pDX, BDown, m_BDown);
	DDX_Control(pDX, BAdd, m_BAdd);
	DDX_Control(pDX, BDelete, m_BDelete);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPageAccess, CPropertyPage)
    //{{AFX_MSG_MAP(CPageAccess)
    ON_BN_CLICKED(BEdit, OnBEdit)
    ON_BN_CLICKED(BNew, OnBNew)
    ON_BN_CLICKED(BDelete, OnBDelete)
    ON_BN_CLICKED(BAdd, OnBAdd)
    ON_BN_CLICKED(BRemove, OnBRemove)
    ON_BN_CLICKED(BDown, OnBDown)
    ON_BN_CLICKED(BUp, OnBUp)
    ON_NOTIFY(NM_DBLCLK, LGlobalAccess, OnDblclkLGlobalAccess)
    ON_NOTIFY(LVN_BEGINDRAG, LGlobalAccess, OnBegindragLGlobalAccess)
    ON_WM_SETCURSOR()
    ON_NOTIFY(LVN_ITEMCHANGED, LAssignedAccess, OnItemchangedLAssignedAccess)
    ON_NOTIFY(LVN_BEGINDRAG, LAssignedAccess, OnBegindragLAssignedAccess)
    ON_NOTIFY(LVN_ITEMCHANGED, LGlobalAccess, OnItemchangedLGlobalAccess)
    ON_NOTIFY(NM_SETFOCUS, LGlobalAccess, OnSetfocusLGlobalAccess)
    ON_NOTIFY(NM_SETFOCUS, LAssignedAccess, OnSetfocusLAssignedAccess)
    ON_BN_CLICKED(BEditCopy, OnBEditCopy)
    ON_NOTIFY(NM_KILLFOCUS, LGlobalAccess, OnKillfocusLGlobalAccess)
    ON_NOTIFY(NM_KILLFOCUS, LAssignedAccess, OnKillfocusLAssignedAccess)
    ON_NOTIFY(NM_CLICK, LGlobalAccess, OnClickLGlobalAccess)
	ON_NOTIFY(NM_CLICK, LAssignedAccess, OnClickLAssignedAccess)
	ON_NOTIFY(LVN_KEYDOWN, LGlobalAccess, OnKeyDownGlobalAccess)
	ON_NOTIFY(LVN_KEYDOWN, LAssignedAccess, OnKeyDownAssignedAccess)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CPageAccess message handlers
BOOL CPageAccess::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	CRect rect;

	// Create and attach the LED image list to the assigned list control.
	m_AssignedImageList.Create(IDB_LED, ACCESS_ICON_SIZE, ACCESS_ICON_EXPAND, ASSIGNED_ACCESS_ICON_BGCOLOR);
	m_LAssignedAccess.SetImageList(&m_AssignedImageList, LVSIL_SMALL);

	// Create and attach the default assignment image list to the global list control.
	m_GlobalImageList.Create(IDB_ACCESS_ICONS, ACCESS_ICON_SIZE, ACCESS_ICON_EXPAND, GLOBAL_ACCESS_ICON_BGCOLOR);
	m_LGlobalAccess.SetImageList(&m_GlobalImageList, LVSIL_SMALL);

	// modify list controls to always show the selection, even when they don't have the focus.
	m_LAssignedAccess.ModifyStyle(NULL, LVS_SHOWSELALWAYS);
	m_LGlobalAccess.ModifyStyle(NULL, LVS_SHOWSELALWAYS);

	// Creates a single column the width of the list control.
	// for the assigned list.
	m_LAssignedAccess.GetClientRect(&rect);
	m_LAssignedAccess.InsertColumn(0, "", LVCFMT_LEFT, rect.Width());
	// for the global list.
	m_LGlobalAccess.GetClientRect(&rect);
	m_LGlobalAccess.InsertColumn(0, "", LVCFMT_LEFT, rect.Width());

	UpdateData(FALSE);

	// Display the global access list.
	ShowGlobalAccess();

	// Disable the assigned-specific items in the GUI.
	SetAssignedButtons(FALSE);
	SetAssignedAccess(FALSE);

	return TRUE;		// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPageAccess::ShowGlobalAccess()
{
	int spec_count;
	Test_Spec *spec;

	m_LGlobalAccess.DeleteAllItems();
	// Display the contents of the access spec list.
	spec_count = theApp.access_spec_list.Count();
	for (int i = 0; i < spec_count; i++) {
		spec = theApp.access_spec_list.Get(i);
		// Insert the Spec into the GUI.
		// Set the data portion to the index of the access spec in 
		// the access spec list.  Do not set the state to selected, since the
		// list box allows multiple selection.
		m_LGlobalAccess.InsertItem(LVIF_STATE | LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE,
					   i,
					   spec->name, NULL, LVIS_SELECTED, spec->default_assignment, (ULONG_PTR) spec);
	}
}

//
// Create a new access spec object and add it to the gui.
//
void CPageAccess::OnBNew()
{
	// Create a new access spec object and store the returned pointer.
	Test_Spec *spec;

	spec = theApp.access_spec_list.New();

	// Check for validity of returned index before adding anything to GUI.
	if (!spec)
		return;

	// Clear any selection in the global list.
	int i = m_LGlobalAccess.GetNextItem(-1, LVIS_SELECTED);

	while (i != NOTHING) {
		m_LGlobalAccess.SetItemState(i, NULL, LVIS_SELECTED);
		i = m_LGlobalAccess.GetNextItem(i, LVIS_SELECTED);
	}

	// Add the Access Spec to the global list view.
	// Set the data portion to the index of the access spec in the access spec
	// list. Select the newly added item.
	m_LGlobalAccess.InsertItem(LVIF_STATE | LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE,
				   theApp.access_spec_list.IndexByRef(spec), spec->name,
				   LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED,
				   AssignNone, (ULONG_PTR) spec);

	// Automatically call the edit function for the new spec.
	// Deletes the spec if editing was cancelled.
	if (!Edit())
		Delete();

	// Set the focus to the ListCtrl
	m_LGlobalAccess.SetFocus();
}

//
// Edit button handler.
//
void CPageAccess::OnBEdit()
{
	Edit();
	m_LGlobalAccess.SetFocus();
}

//
// Pops up an edit form for the currently selected item in the global list.
//
BOOL CPageAccess::Edit()
{
	// Get currently selected spec.
	int item_index = m_LGlobalAccess.GetNextItem(FIND_FIRST, LVNI_SELECTED);

	if (item_index == NOTHING || item_index == IDLE_SPEC)
		return FALSE;

	Test_Spec *spec = (Test_Spec *) m_LGlobalAccess.GetItemData(item_index);

	// Create access spec edit dialog and pass currently selected item's index.
	CAccessDialog EditDlg(spec);

	if (EditDlg.DoModal() == IDCANCEL)
		return FALSE;

	// Display the correct default assignment icon for the new access spec.
	m_LGlobalAccess.SetItem(item_index, 0, LVIF_IMAGE, NULL, spec->default_assignment, NULL, NULL, NULL);

	// Update a name change
	m_LGlobalAccess.SetItemText(item_index, 0, spec->name);
	ShowAssignedAccess();

	return TRUE;
}

//
// Deletes the selected access spec.
//
void CPageAccess::OnBDelete()
{
	Delete();
	m_LGlobalAccess.SetFocus();
}

void CPageAccess::Delete()
{
	int item_index = m_LGlobalAccess.GetNextItem(FIND_FIRST, LVNI_SELECTED);

	// Keep track of the first selected item so the highlight
	// will remain on the same index after item removal.
	int new_selected = item_index;

	LockWindowUpdate();
	// Loop through all the selected list items and delete them.
	while (item_index != NOTHING) {
		// Only delete the item if it isn't the idle spec.
		if (item_index != IDLE_SPEC) {
			theApp.access_spec_list.Delete((Test_Spec *) m_LGlobalAccess.GetItemData(item_index));
		}
		item_index = m_LGlobalAccess.GetNextItem(item_index, LVNI_SELECTED);
	}
	// Display the global access specs.
	ShowGlobalAccess();

	// Make sure the item selection won't be beyond the end of the list.
	if (new_selected >= m_LGlobalAccess.GetItemCount())
		new_selected = m_LGlobalAccess.GetItemCount() - 1;

	// Make a selection in the list.
	m_LGlobalAccess.SetItemState(new_selected, LVNI_SELECTED, LVNI_SELECTED);

	// Update the assigned access specs in case we just deleted one.
	ShowAssignedAccess();
	UnlockWindowUpdate();
}

//
// Adds the selected access spec from the global list to the selected assigned (all managers, manager, or worker).
//
void CPageAccess::OnBAdd()
{
	CPoint point;

	GetCursorPos(&point);
	// Add button has same functionality as dragging from the global list.
	global_dragging = TRUE;
	Insert(point);
	m_LAssignedAccess.SetFocus();
}

//
// Removes the selected items from the worker's assigned access specs.
//
void CPageAccess::OnBRemove()
{
	Remove();
	// Set the focus to the assigned access spec list so that the buttons do
	// not get disabled.
	m_LAssignedAccess.SetFocus();
}

//
// Removes the item at item_index from the assigned list.
//
void CPageAccess::Remove()
{
	Manager *manager;
	Worker *worker;
	int new_selection;
	int selected_type;
	int item_index, old_item_index;

	// Check to make sure something was selected.
	if (m_LAssignedAccess.GetSelectedCount() == 0)
		return;

	// Store the first selected index as the index to be selected after removal.
	new_selection = m_LAssignedAccess.GetNextItem(FIND_FIRST, LVNI_SELECTED);

	// Store the manager/worker selection type since it is used inside the while loop.
	selected_type = theApp.pView->m_pWorkerView->GetSelectedType();

	// Find out who is selected: All managers, a manager, or a worker.
	manager = theApp.pView->m_pWorkerView->GetSelectedManager();
	worker = theApp.pView->m_pWorkerView->GetSelectedWorker();

	LockWindowUpdate();

	// Items must be removed in reverse order (bottom --> up).
	// This isn't the case for the global access spec list.
	// See the comment within the removal loop below.

	// See if the last item in the list is selected.
	if (m_LAssignedAccess.GetItemState(m_LAssignedAccess.GetItemCount() - 1, LVNI_SELECTED)) {
		// It is.
		item_index = m_LAssignedAccess.GetItemCount() - 1;
	} else {
		// It isn't.  Find the next highest selected item.
		item_index = m_LAssignedAccess.GetNextItem(m_LAssignedAccess.GetItemCount() - 1,
							   LVNI_ABOVE | LVNI_SELECTED);
	}

	// Loop through all the selected list items and remove them.
	while (item_index != NOTHING) {
		// Remove the entry from the selected assigned (all managers,
		// manager, or worker).  There is no point in updating the
		// GUI since we will just refresh the contents when we are done
		// deleting.
		switch (selected_type) {
			// Nothing is actually removed from the GUI until the call to
			// ShowAssignedAccess(), below.  This is why we have to remove
			// the list items from the bottom up.
		case ALL_MANAGERS:
			theApp.manager_list.RemoveAccessSpecAt(item_index);
			break;
		case MANAGER:
			manager->RemoveAccessSpecAt(item_index);
			break;
		case WORKER:
			worker->RemoveAccessSpecAt(item_index);
			break;
		}

		// Keep track of the last item we removed.
		old_item_index = item_index;
		// Find the next selected item above it.
		item_index = m_LAssignedAccess.GetNextItem(old_item_index, LVNI_ABOVE | LVNI_SELECTED);
		// If the values are the same, we're at the top of the list.
		// (The CListCtrl should return -1, but doesn't.)
		if (item_index == old_item_index)
			break;
	}

	ShowAssignedAccess();

	// See if the selected item is beyond the end of the list now.
	if (new_selection >= m_LAssignedAccess.GetItemCount())
		new_selection = m_LAssignedAccess.GetItemCount() - 1;

	m_LAssignedAccess.SetItemState(new_selection, LVNI_SELECTED, LVNI_SELECTED);

	UnlockWindowUpdate();
}

//
// This function is called whenever the selection in the WorkerView changes.
// If the selection changes to a worker, it displays that worker's access
// spec assignment.
//
void CPageAccess::ShowAssignedAccess()
{
	int m, w, s, mgr_count, wkr_count = 0, spec_count;
	Manager *mgr = NULL;
	Worker *wkr = NULL;
	Test_Spec *spec;

	// Clear the list of assigned access specs.
	m_LAssignedAccess.DeleteAllItems();

	// find out who is selected: All managers, a manager, or a worker.
	switch (theApp.pView->m_pWorkerView->GetSelectedType()) {
	case ALL_MANAGERS:
		// Disable the Assigned Access Spec listbox if there are no workers.
		if (!theApp.manager_list.WorkerCount()) {
			SetAssignedAccess(FALSE);
			return;
		}
		// Enable the Assigned Access Spec listbox.
		SetAssignedAccess(TRUE);

		// Only display items in the listbox if all the descended workers
		// have the same access spec list.
		if (!theApp.manager_list.AreAccessSpecsIdentical())
			return;

		// All access specs are the same.  Show the specs stored by any worker.

		// Find a manager with any non-client workers.
		mgr_count = theApp.manager_list.ManagerCount();
		for (m = 0; m < mgr_count; m++) {
			mgr = theApp.manager_list.GetManager(m);
			wkr_count = mgr->WorkerCount();
			if (wkr_count > mgr->WorkerCount(GenericClientType))
				break;
		}

		// Find the manager's first non-client worker.
		for (w = 0; w < wkr_count; w++) {
			// Find the first non-client worker.
			wkr = mgr->GetWorker(w);
			if (!IsType(wkr->Type(), GenericClientType))
				break;
		}
		break;

	case MANAGER:
		mgr = theApp.pView->m_pWorkerView->GetSelectedManager();
		wkr_count = mgr->WorkerCount();

		// Disable the access spec list if there are no non-client workers.
		if (wkr_count == mgr->WorkerCount(GenericClientType)) {
			SetAssignedAccess(FALSE);
			return;
		}
		// Enable the Assigned Access Spec listbox.
		SetAssignedAccess(TRUE);

		// Only display items in the listbox if all the descended workers
		// have the same access spec list.
		if (!mgr->AreAccessSpecsIdentical())
			return;

		// All the access specs are the same.

		// Find the first non-client worker.
		for (w = 0; w < wkr_count; w++) {
			wkr = mgr->GetWorker(w);
			if (!IsType(wkr->Type(), GenericClientType))
				break;
		}
		break;

	case WORKER:

		wkr = theApp.pView->m_pWorkerView->GetSelectedWorker();

		if (IsType(wkr->Type(), GenericClientType)) {
			// Worker is a net client.  Display it's net server's spec 
			// assignment, but disable the view.
			wkr = wkr->net_partner;
			SetAssignedAccess(FALSE);
		} else {
			SetAssignedAccess(TRUE);
		}
		break;
	}
	if (!wkr) {
		ErrorMessage("No valid worker in CPageAccess::ShowAssignedAccess().");
		return;
	}
	// Add the Access Spec to the assigned access spec list view.
	spec_count = wkr->AccessSpecCount();
	for (s = 0; s < spec_count; s++) {
		spec = wkr->GetAccessSpec(s);
		// Set the data portion to pointer to the access spec in the
		// worker's access spec list.
		m_LAssignedAccess.InsertItem(LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE,
					     s, spec->name, NULL, LVIS_SELECTED, BLUE, (ULONG_PTR) spec);
	}
}

//
// Enables the assigned list and add button based on the input variable.
// Verifies that enabling is allowed before enabling.
//
void CPageAccess::SetAssignedAccess(BOOL enable)
{
	Manager *mgr;
	Worker *wkr;
	BOOL enabling_worker = FALSE;

	// Get the currently selected manager and worker.
	switch (theApp.pView->m_pWorkerView->GetSelectedType()) {
	case ALL_MANAGERS:
		// Allow enabling if there are any workers,
		enabling_worker = theApp.manager_list.WorkerCount();
		break;

	case MANAGER:
		mgr = theApp.pView->m_pWorkerView->GetSelectedManager();

		// Allow enabling if there are non-client workers.
		enabling_worker = (mgr->WorkerCount() > mgr->WorkerCount(GenericClientType));
		break;

	case WORKER:
		wkr = theApp.pView->m_pWorkerView->GetSelectedWorker();

		// Allow enabling if it's not a client.
		enabling_worker = !IsType(wkr->Type(), GenericClientType);
		break;
	}

	m_LAssignedAccess.EnableWindow(enable && enabling_worker && theApp.test_state == TestIdle);

	m_BAdd.EnableWindow(enable && enabling_worker && theApp.test_state == TestIdle &&
			    (m_LGlobalAccess.GetNextItem(FIND_FIRST, LVNI_SELECTED) != -1));

	m_LAssignedAccess.RedrawWindow();
	m_BAdd.RedrawWindow();
}

//
// Called by both the up and down button handlers.  This function actually moves the things around
// in both the GUI and the selected assigned's access spec list.
//
void CPageAccess::Move(int desired_index)
{
	Manager *manager;
	Worker *worker;
	int item_index = m_LAssignedAccess.GetNextItem(FIND_FIRST, LVNI_SELECTED);

	// Check to make sure something was selected.
	if (m_LAssignedAccess.GetSelectedCount() == 0)
		return;

	LockWindowUpdate();
	// Move the access spec referred to by the selected item.
	switch (theApp.pView->m_pWorkerView->GetSelectedType()) {
	case ALL_MANAGERS:
		theApp.manager_list.MoveAccessSpec(item_index, desired_index);
		break;
	case MANAGER:
		manager = theApp.pView->m_pWorkerView->GetSelectedManager();
		manager->MoveAccessSpec(item_index, desired_index);
		break;
	case WORKER:
		worker = theApp.pView->m_pWorkerView->GetSelectedWorker();
		worker->MoveAccessSpec(item_index, desired_index);
		break;
	}
	ShowAssignedAccess();
	// Select the moved item so that the move buttons are still enabled.
	m_LAssignedAccess.SetItemState(desired_index, LVNI_SELECTED, LVNI_SELECTED);
	UnlockWindowUpdate();
}

//
// Moves the selected item up/down in the assigned's list of access specs.
//
void CPageAccess::OnBDown()
{
	// Get the currently selected item from the workers ListCtrl.
	int item_index = m_LAssignedAccess.GetNextItem(FIND_FIRST, LVNI_SELECTED);

	Move(item_index + 1);
	m_LAssignedAccess.SetFocus();
}

void CPageAccess::OnBUp()
{
	// Get the currently selected item from the workers ListCtrl.
	int item_index = m_LAssignedAccess.GetNextItem(FIND_FIRST, LVNI_SELECTED);

	Move(item_index - 1);
	m_LAssignedAccess.SetFocus();
}

//
// Enables or disable the Up, Down, and Remove buttons.
//
void CPageAccess::SetAssignedButtons(BOOL enable)
{
	int item_index = m_LAssignedAccess.GetNextItem(FIND_FIRST, LVNI_SELECTED);
	BOOL net_client_selected = (theApp.pView->m_pWorkerView->GetSelectedWorkerType()
				    == GenericClientType);

	// Enables the Up Button when the selected item is at least the second entry
	m_BUp.EnableWindow(enable &&
			   item_index > 0 &&
			   !net_client_selected &&
			   m_LAssignedAccess.GetSelectedCount() == 1 &&
			   theApp.test_state == TestIdle && GetFocus() == &m_LAssignedAccess);
	// Enables the Down Button when the selected item is not the last entry.
	m_BDown.EnableWindow(enable &&
			     item_index < m_LAssignedAccess.GetItemCount() - 1 &&
			     item_index >= 0 &&
			     !net_client_selected &&
			     m_LAssignedAccess.GetSelectedCount() == 1 &&
			     theApp.test_state == TestIdle && GetFocus() == &m_LAssignedAccess);
	// Remove Button when any item is selected.
	m_BRemove.EnableWindow(enable &&
			       item_index != NOTHING &&
			       !net_client_selected &&
			       theApp.test_state == TestIdle && GetFocus() == &m_LAssignedAccess);

	m_BUp.RedrawWindow();
	m_BDown.RedrawWindow();
	m_BRemove.RedrawWindow();
}

//
// Edits an access spec when the user double clicks on it in the global list.
//
void CPageAccess::OnDblclkLGlobalAccess(NMHDR * pNMHDR, LRESULT * pResult)
{
	Edit();
	m_LGlobalAccess.SetFocus();

	*pResult = 0;
}

//
// The user is beginning a drag operation from the global list.
//
void CPageAccess::OnBegindragLGlobalAccess(NMHDR * pNMHDR, LRESULT * pResult)
{
	// check that input parameter is valid and that we are allowed to drag.
	if (!pNMHDR || theApp.test_state != TestIdle || theApp.pView->m_pWorkerView->GetSelectedWorkerType()
	    == GenericClientType || !m_LAssignedAccess.IsWindowEnabled()) {
		return;
	}
	// Select the item to drag.
	NM_LISTVIEW *pNMListView = (NM_LISTVIEW *) pNMHDR;

	// Set flag indicating that a drag operation is underway.
	global_dragging = TRUE;
	CPoint point;

	p_DragImage = m_LGlobalAccess.CreateDragImage(pNMListView->iItem, &point);

	// set hot spot to middle of 16x16 icon
	point.x = 8;
	point.y = 8;

	// set drag icon to icon of current selection
	p_DragImage->BeginDrag(0, point);

	if (m_LGlobalAccess.GetSelectedCount() > 1) 	
	theApp.m_wndStatusBar.GetStatusBarCtrl().SetText("", 0, 0);

	*pResult = 0;
}

//
// The user is beginning a drag operation from the assigned list.
//
void CPageAccess::OnBegindragLAssignedAccess(NMHDR * pNMHDR, LRESULT * pResult)
{
	// check that input parameter is valid
	if (!pNMHDR || theApp.test_state != TestIdle || theApp.pView->m_pWorkerView->GetSelectedWorkerType()
	    == GenericClientType || !m_LAssignedAccess.IsWindowEnabled()) {
		return;
	}
	// Set focus to assigned access list.
	m_LAssignedAccess.SetFocus();

	// Select the item to drag.
	NM_LISTVIEW *pNMListView = (NM_LISTVIEW *) pNMHDR;

	// Set flag indicating that a drag operation is underway.
	assigned_dragging = TRUE;
	CPoint point;

	p_DragImage = m_LAssignedAccess.CreateDragImage(pNMListView->iItem, &point);

	// set hot spot to middle of 16x16 icon
	point.x = 24;
	point.y = 8;

	// set drag icon to icon of current selection
	p_DragImage->BeginDrag(0, point);

	if (m_LAssignedAccess.GetSelectedCount() > 1) 	
		theApp.m_wndStatusBar.GetStatusBarCtrl().SetText("", 0, 0);
	
	*pResult = 0;
}

//
// Updates the cursor during a drag operation.
//
BOOL CPageAccess::OnSetCursor(CWnd * pWnd, UINT nHitTest, UINT message)
{
	CPoint point;
	CRect global_rect, assigned_rect;

	// Get the areas of the two list controls.
	m_LAssignedAccess.GetWindowRect(&assigned_rect);
	m_LGlobalAccess.GetWindowRect(&global_rect);

	// Normalize these areas so that we have no negative widths or heights.
	assigned_rect.NormalizeRect();
	global_rect.NormalizeRect();

	// Check to see if the mouse is moved during a drag event.
	if (message == WM_MOUSEMOVE && (global_dragging || assigned_dragging)) {
		// Get current curser position.
		GetCursorPos(&point);

		// Set the curser to either the arrow or the crossed circle.
		if (global_dragging && !assigned_rect.PtInRect(point) && !global_rect.PtInRect(point))
			SetCursor(LoadCursor(NULL, IDC_NO));
		else
			SetCursor(LoadCursor(NULL, IDC_ARROW));

		// Show the drag image in addition to the cursor.
		p_DragImage->DragShowNolock(TRUE);
		p_DragImage->DragMove(point);

		return TRUE;
	}
	// Check to see if the button up mouse event happened after a drag event.
	if ((message == WM_LBUTTONUP || message == WM_LBUTTONDOWN) && (global_dragging || assigned_dragging)) {
		p_DragImage->EndDrag();

		delete p_DragImage;

		// Return the cursor to the Arrow.
		SetCursor(LoadCursor(NULL, IDC_ARROW));

		GetCursorPos(&point);

		// dragging within the assigned list?
		if (assigned_dragging) {
			if (assigned_rect.PtInRect(point)) {	// dropped within the assigned list.
				if (m_LAssignedAccess.GetSelectedCount() == 1)
					Insert(point);
			} else {	// dropped outside the assigned list.
				Remove();
			}
		}
		// dragging from the global list to the assigned list?
		else if (global_dragging && assigned_rect.PtInRect(point)) {
			Insert(point);
			m_LAssignedAccess.SetFocus();
		}

		assigned_dragging = FALSE;
		global_dragging = FALSE;
		return TRUE;
	}

	return CPropertyPage::OnSetCursor(pWnd, nHitTest, message);
}

//
// Handle the item selection changing in the assigned list.
//
void CPageAccess::OnItemchangedLAssignedAccess(NMHDR * pNMHDR, LRESULT * pResult)
{
	NM_LISTVIEW *pNMListView = (NM_LISTVIEW *) pNMHDR;

	// Set buttons correctly.
	SetAssignedButtons(TRUE);

	m_LAssignedAccess.RedrawWindow();
	*pResult = 0;
}

//
// Inserts an item into the assigned list before the currently selected item.
//
void CPageAccess::Insert(CPoint point)
{
	int move_to_index, global_index;
	Manager *manager;
	Worker *worker;

	global_index = m_LGlobalAccess.GetNextItem(FIND_FIRST, LVNI_SELECTED);

	if (global_dragging && global_index == NOTHING)
		return;

	// find out who is selected: All managers, a manager, or a worker.
	manager = theApp.pView->m_pWorkerView->GetSelectedManager();
	worker = theApp.pView->m_pWorkerView->GetSelectedWorker();

	// Set all spec LED's to their initial state.
	Reset();

	// If this is a new selection, any additions to all managers or to a manager
	// will clear all their children workers' access spec settings.
	if (!worker && m_LAssignedAccess.GetItemCount() == 0) {
		// There is nothing in the GUI.  The Add will replace
		// all the children workers' selection.
		if (!manager)
			theApp.manager_list.RemoveAllAccessSpecs();
		else
			manager->RemoveAllAccessSpecs();
	}
	// Find the item being hovered over.
	m_LAssignedAccess.ScreenToClient(&point);
	move_to_index = m_LAssignedAccess.HitTest(point);
	if (move_to_index == -1)
		move_to_index = m_LAssignedAccess.GetItemCount();

	if (global_dragging) {
		InsertAt(move_to_index);
	} else if (assigned_dragging) {
		Move(move_to_index);

		assigned_dragging = FALSE;	// Reset assigned dragging flag.
	}
}

void CPageAccess::InsertAt(int insert_index)
{
	Test_Spec *spec;
	int inserted_count = 0;
	int global_index = m_LGlobalAccess.GetNextItem(FIND_FIRST, LVNI_SELECTED);

	// find out who is selected: All managers, a manager, or a worker.
	Manager *manager = theApp.pView->m_pWorkerView->GetSelectedManager();
	Worker *worker = theApp.pView->m_pWorkerView->GetSelectedWorker();

	LockWindowUpdate();
	// Loop through all the selected list items from the global list
	// and add them to the assigned list.
	while (global_index != NOTHING) {
		spec = (Test_Spec *) m_LGlobalAccess.GetItemData(global_index);
		switch (theApp.pView->m_pWorkerView->GetSelectedType()) {
		case ALL_MANAGERS:
			// Insert access spec to all managers
			if (!theApp.manager_list.InsertAccessSpec(spec, insert_index + inserted_count++)) {
				UnlockWindowUpdate();
				ErrorMessage
				    ("Failed to add the selected access spec to all managers in CPageAccess::Insert().");
				return;
			}
			break;
		case MANAGER:
			// Insert access spec to a manager
			if (!manager->InsertAccessSpec(spec, insert_index + inserted_count++)) {
				UnlockWindowUpdate();
				ErrorMessage
				    ("Failed to add the selected access spec to the selected managers in CPageAccess::Insert().");
				return;
			}
			break;
		case WORKER:
			// Insert acces spec to a worker
			if (!worker->InsertAccessSpec(spec, insert_index + inserted_count++)) {
				UnlockWindowUpdate();
				ErrorMessage
				    ("Failed to add the selected access spec to the selected worker in CPageAccess::Insert().");
				return;
			}
			break;
		}
		global_index = m_LGlobalAccess.GetNextItem(global_index, LVNI_SELECTED);
	}
	ShowAssignedAccess();
	UnlockWindowUpdate();
	global_dragging = FALSE;	// Reset global dragging flag.
}

void CPageAccess::OnItemchangedLGlobalAccess(NMHDR * pNMHDR, LRESULT * pResult)
{
	NM_LISTVIEW *pNMListView = (NM_LISTVIEW *) pNMHDR;

	SetAssignedAccess(TRUE);

	// Disable delete and edit buttons for the Idle Spec
	SetGlobalButtons(pNMListView->iItem != IDLE_SPEC && (pNMListView->uNewState && LVIS_SELECTED));

	*pResult = 0;
}

//
// Enables the apropriate window objects after test run.
//
void CPageAccess::EnableWindow(BOOL enable)
{
	// Enables the assigned list and the add button unless the selected worker is a TCPClient.
	SetAssignedAccess(enable);

	m_BNew.EnableWindow(enable);
	m_BNew.RedrawWindow();

	m_LGlobalAccess.EnableWindow(enable);
	m_LGlobalAccess.RedrawWindow();
}

//
// Resets all the LED's to BLUE
//
void CPageAccess::Reset()
{
	for (int i = 0; i < m_LAssignedAccess.GetItemCount(); i++)
		MarkAccess(i, BLUE);
}

//
// Mark given spec given color.
//
void CPageAccess::MarkAccess(int access_index, int color)
{
	m_LAssignedAccess.SetItem(access_index,	// item index
				  NULL,	// subitem index
				  LVIF_IMAGE,	// mask
				  NULL,	// text
				  color,	// image
				  NULL,	// state
				  NULL,	// state mask
				  NULL);	// data

}

//
// Set the LEDs for the currently displayed access specs based on the 
// currently running spec.
//
void CPageAccess::MarkAccesses(int current_access)
{
	for (int i = 0; i < m_LAssignedAccess.GetItemCount(); i++) {
		if ((theApp.test_state == TestIdle) || (theApp.test_state == TestPreparing) || i > current_access) {
			MarkAccess(i, BLUE);
		} else {
			if (i == current_access) {
				MarkAccess(i, GREEN);
				m_LAssignedAccess.EnsureVisible(i, FALSE);
			} else {
				MarkAccess(i, RED);
			}
		}
	}
}

//
// Enables the edit and delete buttons for the global access spec list.
//
void CPageAccess::SetGlobalButtons(BOOL enable)
{
	int global_index = m_LGlobalAccess.GetNextItem(FIND_FIRST, LVNI_SELECTED);

	m_BEdit.EnableWindow(enable &&
			     global_index != IDLE_SPEC &&
			     GetFocus() == &m_LGlobalAccess && m_LGlobalAccess.GetSelectedCount() == 1);
	m_BEditCopy.EnableWindow(enable &&
				 global_index != IDLE_SPEC &&
				 GetFocus() == &m_LGlobalAccess && m_LGlobalAccess.GetSelectedCount() == 1);
	m_BDelete.EnableWindow(enable && global_index != IDLE_SPEC
			       && GetFocus() == &m_LGlobalAccess && global_index != NOTHING);

	m_BEdit.RedrawWindow();
	m_BEditCopy.RedrawWindow();
	m_BDelete.RedrawWindow();
}

/////////////////////////////////////////////////////////////////////////
//
// The following functions enable and disable GUI buttons
// based on the input focus.
//
/////////////////////////////////////////////////////////////////////////
void CPageAccess::OnKillfocusLGlobalAccess(NMHDR * pNMHDR, LRESULT * pResult)
{
	CPoint point;
	CRect frame_rect, button_rect;

	GetCursorPos(&point);

	// Get the area of the global list controls.
	m_GGlobalFrame.GetWindowRect(&frame_rect);
	m_BAdd.GetWindowRect(&button_rect);

	// Normalize the area so that we have no negative widths or heights.
	frame_rect.NormalizeRect();
	button_rect.NormalizeRect();

	frame_rect.UnionRect(frame_rect, button_rect);

	if (!frame_rect.PtInRect(point))
		SetGlobalButtons(FALSE);

	// Clear the status bar when the focus is moved away from the access list
	if (theApp.m_wndStatusBar.m_hWnd) theApp.m_wndStatusBar.GetStatusBarCtrl().SetText("", 0, 0);
}

void CPageAccess::OnKillfocusLAssignedAccess(NMHDR * pNMHDR, LRESULT * pResult)
{
	CPoint point;
	CRect frame_rect, button_rect;

	GetCursorPos(&point);

	// Get the area of the global list controls.
	m_GAssignedFrame.GetWindowRect(&frame_rect);
	m_BRemove.GetWindowRect(&button_rect);

	// Normalize the area so that we have no negative widths or heights.
	frame_rect.NormalizeRect();
	button_rect.NormalizeRect();

	frame_rect.UnionRect(frame_rect, button_rect);

	if (!frame_rect.PtInRect(point))
		SetAssignedButtons(FALSE);

	// Clear the status bar when the focus is moved away from the access list
	theApp.m_wndStatusBar.GetStatusBarCtrl().SetText("", 0, 0);
}

void CPageAccess::OnSetfocusLGlobalAccess(NMHDR * pNMHDR, LRESULT * pResult)
{
	SetGlobalButtons(TRUE);
	*pResult = 0;
}

void CPageAccess::OnSetfocusLAssignedAccess(NMHDR * pNMHDR, LRESULT * pResult)
{
	SetAssignedButtons(TRUE);
	*pResult = 0;
}

void CPageAccess::OnBEditCopy()
{
	Test_Spec *spec;

	// Get the selected access spec's index into the global list.
	spec = (Test_Spec *) m_LGlobalAccess.GetItemData(m_LGlobalAccess.GetNextItem(FIND_FIRST, LVNI_SELECTED));
	// Create the copy of the spec object.
	spec = theApp.access_spec_list.Copy(spec);

	// Check for validity of returned index before adding anything to GUI.
	if (!spec)
		return;

	// Add the Access Spec to the global list view.
	// Set the data portion to the index of the access spec in the access spec list.
	// Select the newly added item and remove the selection from the previous
	// selected item.
	m_LGlobalAccess.SetItemState(m_LGlobalAccess.GetNextItem(FIND_FIRST, LVNI_SELECTED), NULL, LVIS_SELECTED);
	m_LGlobalAccess.InsertItem(LVIF_STATE | LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE,
				   m_LGlobalAccess.GetItemCount(), spec->name,
				   LVIS_SELECTED, LVIS_SELECTED, AssignNone, (ULONG_PTR) spec);

	// Automatically call the edit function for the new spec.
	// Deletes the spec if editing was cancelled.
	if (!Edit())
		Delete();

	// Set the focus to the ListCtrl
	m_LGlobalAccess.SetFocus();
}

void CPageAccess::OnClickLGlobalAccess(NMHDR * pNMHDR, LRESULT * pResult)
{
	SetGlobalButtons(TRUE);
	*pResult = 0;

	// Display the access spec name in the left most field of the status bar
#if 0 // null the status bar if more than one item selected
	if (m_LGlobalAccess.GetSelectedCount() > 1) 
	{
		theApp.m_wndStatusBar.GetStatusBarCtrl().SetText("", 0, 0);
		return;
	}
#endif

	int item_index = m_LGlobalAccess.GetSelectionMark();
	CString str = m_LGlobalAccess.GetItemText(item_index, 0);
	theApp.m_wndStatusBar.GetStatusBarCtrl().SetText(str, 0, 0);
}

void CPageAccess::OnClickLAssignedAccess(NMHDR * pNMHDR, LRESULT * pResult)
{
	SetAssignedButtons(TRUE);
	*pResult = 0;

	// Display the access spec name in the left most field of the status bar
#if 0 // null the status bar if more than one item selected
	if (m_LAssignedAccess.GetSelectedCount() > 1) 
	{
		theApp.m_wndStatusBar.GetStatusBarCtrl().SetText("", 0, 0);
		return;
	}
#endif

	int item_index = m_LAssignedAccess.GetSelectionMark(); 
	CString str = m_LAssignedAccess.GetItemText(item_index, 0);
	theApp.m_wndStatusBar.GetStatusBarCtrl().SetText(str, 0, 0);
}

// Ved: Added key up/down handlers (OnKeyDownGlobalAccess, OnKeyDownAssignedAccess) to deal with 
// status bar updates when the GUI does not display the full access spec string.

void CPageAccess::OnKeyDownGlobalAccess(NMHDR * pNMHDR, LRESULT * pResult)
{
	LV_KEYDOWN* pLVKeyDown = (LV_KEYDOWN*)pNMHDR;
	*pResult = 0;

#if 0 // null the status bar if more than one item selected
	if (m_LGlobalAccess.GetSelectedCount() > 1) 
	{
			theApp.m_wndStatusBar.GetStatusBarCtrl().SetText("", 0, 0);
			return;
	}
#endif

	int item_index = m_LGlobalAccess.GetSelectionMark(); //GetNextItem(FIND_FIRST, LVNI_SELECTED);

	if(pLVKeyDown->wVKey == VK_UP ) 
	{
		if (item_index)
		{
			m_LGlobalAccess.SetSelectionMark(--item_index);
		}
	}
	else if (pLVKeyDown->wVKey == VK_DOWN ) 
	{
		if (item_index < m_LGlobalAccess.GetItemCount() - 1)
		{
			m_LGlobalAccess.SetSelectionMark(++item_index);
		}
	}

	CString str = m_LGlobalAccess.GetItemText(item_index, 0);
	theApp.m_wndStatusBar.GetStatusBarCtrl().SetText(str, 0, 0);
}

void CPageAccess::OnKeyDownAssignedAccess(NMHDR * pNMHDR, LRESULT * pResult)
{
	LV_KEYDOWN* pLVKeyDown = (LV_KEYDOWN*)pNMHDR;
	*pResult = 0;

#if 0 // null the status bar if more than one item selected
	if (m_LAssignedAccess.GetSelectedCount() > 1) 
	{
			theApp.m_wndStatusBar.GetStatusBarCtrl().SetText("", 0, 0);
			return;
	}
#endif

	int item_index = m_LAssignedAccess.GetSelectionMark(); //GetNextItem(FIND_FIRST, LVNI_SELECTED);

	if(pLVKeyDown->wVKey == VK_UP ) 
	{
		if (item_index)
		{
			m_LAssignedAccess.SetSelectionMark(--item_index);
		}
	}
	else if (pLVKeyDown->wVKey == VK_DOWN ) 
	{
		if (item_index < m_LAssignedAccess.GetItemCount() - 1)
		{
			m_LAssignedAccess.SetSelectionMark(++item_index);
		}
	}

	CString str = m_LAssignedAccess.GetItemText(item_index, 0);
	theApp.m_wndStatusBar.GetStatusBarCtrl().SetText(str, 0, 0);
}