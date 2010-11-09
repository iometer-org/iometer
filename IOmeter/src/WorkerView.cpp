/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / WorkerView.cpp                                         ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implementation of the CWorkerView class, which is     ## */
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
/* ##               2003-04-25 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Updated the global debug flag (_DEBUG) handling     ## */
/* ##                 of the source file (check for platform etc.).       ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include "stdafx.h"
#include "ManagerList.h"
#include "WorkerView.h"
#include "GalileoView.h"
#include "resource.h"

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
// CWorkerView dialog

CWorkerView::CWorkerView(CWnd * pParent /*=NULL*/ )
:  CDialog(CWorkerView::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWorkerView)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	selected_type = IOERROR;
	editing_label = FALSE;
	old_label_name = NULL;
}

void CWorkerView::DoDataExchange(CDataExchange * pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWorkerView)
	DDX_Control(pDX, TWorkers, m_TWorkers);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CWorkerView, CDialog)
ON_WM_CONTEXTMENU()
    //{{AFX_MSG_MAP(CWorkerView)
    ON_NOTIFY(TVN_SELCHANGED, TWorkers, OnSelchangedTWorkers)
    ON_NOTIFY(TVN_SELCHANGING, TWorkers, OnSelchangingTWorkers)
    ON_NOTIFY(TVN_ENDLABELEDIT, TWorkers, OnEndlabeleditTWorkers)
    ON_NOTIFY(TVN_BEGINDRAG, TWorkers, OnBegindragTWorkers)
    ON_NOTIFY(NM_RCLICK, TWorkers, OnRclickTWorkers)
ON_NOTIFY(TVN_BEGINLABELEDIT, TWorkers, OnBeginlabeleditTWorkers)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CWorkerView message handlers
//
// Retrieving a pointer to a specified manager item in the view.
//
HTREEITEM CWorkerView::GetManager(Manager * manager)
{
	HTREEITEM hmanager;

	// Get the first manager
	hmanager = m_TWorkers.GetChildItem(hall_managers);

	// Find the referenced manager.
	while (m_TWorkers.GetItemData(hmanager) != (DWORD_PTR) manager)
		hmanager = m_TWorkers.GetNextItem(hmanager, TVGN_NEXT);

	return hmanager;
}

//
// Retrieving a pointer to a specified worker item in the view.
//
HTREEITEM CWorkerView::GetWorker(Worker * worker)
{
	HTREEITEM hworker;

	// Get the first worker.
	hworker = m_TWorkers.GetChildItem(GetManager(worker->manager));

	// Loop through manager's workers until we find the correct one.
	while (m_TWorkers.GetItemData(hworker) != (DWORD_PTR) worker)
		hworker = m_TWorkers.GetNextSiblingItem(hworker);

	return hworker;
}

//
// Return the type of the selected worker.
//
TargetType CWorkerView::GetSelectedWorkerType()
{
	// Getting the type of the currently selected worker, if any.
	if (selected_type != WORKER)
		return InvalidType;

	return GetSelectedWorker()->Type();
}

//
// Select the specified item in the worker tree view and scroll it into view.
// If both mgr and wkr params are NULL, "All Managers" will be selected.
// If mgr points to a valid manager and wkr is NULL, the manager will be selected.
// If mgr and wkr are both defined, the appropriate worker on the specified
// manager will be selected.
//
void CWorkerView::SelectItem(Manager * mgr, Worker * wkr)
{
	HTREEITEM item;

	if (mgr) {
		if (wkr) {
			// The bar is monitoring a worker on the specified manager.
			// Find the HTREEITEM corresponding to the worker.
			item = GetWorker(wkr);
		} else {
			// The bar is monitoring a manager.
			// Find the HTREEITEM corresponding to the manager.
			item = GetManager(mgr);
		}
	} else {
		// The bar is monitoring "All Managers".
		// Find the HTREEITEM corresponding to "All Managers".
		item = m_TWorkers.GetRootItem();
	}

	// Select the item and scroll it into view.
	m_TWorkers.Select(item, TVGN_CARET);
	m_TWorkers.EnsureVisible(item);
}

//
// Adding a the specified manager to the display.
//
void CWorkerView::AddManager(Manager * manager)
{
	HTREEITEM hmanager;

	// Insert a new item into the view and sort the view by name.
	hmanager = m_TWorkers.InsertItem(manager->name, WORKER_ICON_MANAGER, WORKER_ICON_MANAGER,
					 hall_managers, TVI_SORT);
	// Store the pointer to the manager in the newly inserted item's data member.
	m_TWorkers.SetItemData(hmanager, (DWORD_PTR) manager);
	m_TWorkers.RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASE);

	// Update the network target list.
	theApp.pView->m_pPageNetwork->ShowData();

	// Update the Assigned Access Specs listbox.
	theApp.pView->m_pPageAccess->ShowAssignedAccess();
}

//
// Adding the specified worker to the display.
//
void CWorkerView::AddWorker(Worker * worker)
{
	HTREEITEM hmanager, hworker;
	int icon;

	// Get the handle to the tree view entry of the worker's manager.
	hmanager = GetManager(worker->manager);

	if (IsType(worker->Type(), GenericDiskType))
		icon = WORKER_ICON_DISKWORKER;
	else if (IsType(worker->Type(), GenericServerType))
		icon = WORKER_ICON_NETSERVER;
	else if (IsType(worker->Type(), GenericClientType))
		icon = WORKER_ICON_NETCLIENT;
	else
		icon = WORKER_ICON_MANAGER;

	// Add the worker to the manager in the tree view.
	hworker = m_TWorkers.InsertItem(worker->name, icon, icon, hmanager, TVI_LAST);
	m_TWorkers.SetItemData(hworker, (DWORD_PTR) worker);
	m_TWorkers.RedrawWindow();

	// Update the Assigned Access Specs listbox.
	theApp.pView->m_pPageAccess->ShowAssignedAccess();
}

//
// Returning the pointer to a selected manager or selected
// worker's manager.
//
Manager *CWorkerView::GetSelectedManager()
{
	switch (selected_type) {
	case MANAGER:
		return (Manager *) m_TWorkers.GetItemData(m_TWorkers.GetSelectedItem());
	case WORKER:
		return GetSelectedWorker()->manager;
	default:
		// No manager or worker was selected.
		return NULL;
	}
}

//
// Returning the index of a selected manager or selected
// worker's manager.
//
int CWorkerView::GetSelectedManagerIndex()
{
	switch (selected_type) {
	case ALL_MANAGERS:
		return ALL_MANAGERS;
	case MANAGER:
	case WORKER:
		// Return the index of the currently selected manager or selected
		// worker's manager.
		return GetSelectedManager()->GetIndex();
	default:
		// No item was selected.  Returning an error.
		return IOERROR;
	}
}

//
// Returning the index of a selected worker.
//
Worker *CWorkerView::GetSelectedWorker()
{
	if (selected_type != WORKER)
		return NULL;

	return (Worker *) m_TWorkers.GetItemData(m_TWorkers.GetSelectedItem());
}

//
// Returning the index of a selected worker.
//
int CWorkerView::GetSelectedWorkerIndex()
{
	if (selected_type != WORKER)
		return IOERROR;

	return GetSelectedWorker()->GetIndex();
}

//
// Clearing display contents.
//
void CWorkerView::Reset()
{
	HTREEITEM hmanager;

	// Lock the window.

	m_TWorkers.SelectItem(hall_managers);
	while (hmanager = m_TWorkers.GetChildItem(hall_managers)) {
		m_TWorkers.DeleteItem(hmanager);
	}
	m_TWorkers.RedrawWindow();
}

//
// Removing the selected item from the GUI.
//
void CWorkerView::RemoveSelectedItem()
{
	// Removing selected manager or worker.
	// First make sure that a manager or worker was selected.
	switch (selected_type) {
	case IOERROR:
		break;
	case ALL_MANAGERS:
		theApp.manager_list.RemoveAllManagers();
		break;
	case MANAGER:
		RemoveManager(GetSelectedManager());
		break;
	case WORKER:
		RemoveWorker(GetSelectedWorker());
		break;
	}
}

//
// Removing manager indicated by index from the display.
// Update managers with indexes greater than the one removed.
// This function must be called before removing the manager from 
// the manager list.
//
void CWorkerView::RemoveManager(Manager * manager)
{
	HTREEITEM hworker, hmanager;

	hmanager = GetManager(manager);
	// Remove the manager's workers and associated net clients.
	while (hworker = m_TWorkers.GetChildItem(hmanager)) {
		RemoveWorker((Worker *) m_TWorkers.GetItemData(hworker));
	}

	// Remove the indicated manager from the display.
	m_TWorkers.DeleteItem(hmanager);
	m_TWorkers.RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASE);
	theApp.pView->m_pPageNetwork->RemoveManager(manager);

	// Remove the indicated manager from memory.
	theApp.manager_list.RemoveManager(manager->GetIndex());
}

//
// Remove worker from indicated manager.  Update indexes for other
// workers.  This function must be called before removing the worker
// from the list of workers.
//
void CWorkerView::RemoveWorker(Worker * worker)
{
	// Get the worker tree item.
	HTREEITEM hworker = GetWorker(worker);

	// If removing a server with an active partner, remove partner as well.
	if (IsType(worker->Type(), GenericServerType) && worker->net_partner) {
		// Call RemoveWorker recursively to remove the NetClient from the GUI.
		RemoveWorker(worker->net_partner);
	}
	// Remove the indicated worker from memory.
	worker->manager->RemoveWorker(worker->GetIndex());

	// Remove the indicated worker from the display.
	m_TWorkers.DeleteItem(hworker);
	m_TWorkers.RedrawWindow();

	// Reset the results display tab to its initial state.
	theApp.pView->m_pPageDisplay->Reset();
}

//
// A new worker or manager is selected.  Update the display.
//
void CWorkerView::OnSelchangedTWorkers(NMHDR * pNMHDR, LRESULT * pResult)
{
	NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW *) pNMHDR;
	HTREEITEM item;

	// If an item is selected, finding out what it is.
	// Peforming any needed updates after changing selection.
	item = m_TWorkers.GetSelectedItem();
	// Is the item selected is all managers?
	if (item == hall_managers) {
		selected_type = ALL_MANAGERS;
		theApp.pView->ChangedSelection();
	}
	// If the item's parent is all managers, the item is a manager.
	else if (m_TWorkers.GetParentItem(item) == hall_managers) {
		selected_type = MANAGER;
		theApp.pView->ChangedSelection();
	} else {
		// Item is a worker.
		selected_type = WORKER;
		theApp.pView->ChangedSelection();
	}
	*pResult = 0;
}

void CWorkerView::OnSelchangingTWorkers(NMHDR * pNMHDR, LRESULT * pResult)
{
	// Peforming any needed updates before changing selection.
	theApp.pView->ChangingSelection();

	*pResult = 0;
}

//
// Mark that the user has started editing a label.
//
void CWorkerView::OnBeginlabeleditTWorkers(NMHDR * pNMHDR, LRESULT * pResult)
{

	TV_DISPINFO *pTVDispInfo = (TV_DISPINFO *) pNMHDR;

	editing_label = TRUE;

	// Record the current label name and mark that we're editing the name.
	if (!old_label_name) {
		if (!(old_label_name = new CString)) {
			ErrorMessage("Unable to allocate memory to store current label name."
				     "  Modifications cannot be undone.");
			return;
		}
	}
	*old_label_name = m_TWorkers.GetItemText(m_TWorkers.GetSelectedItem());

	*pResult = 0;
}

//
// User renamed one of the workers.  Updating the information in
// the worker list.
//
void CWorkerView::OnEndlabeleditTWorkers(NMHDR * pNMHDR, LRESULT * pResult)
{
	TV_DISPINFO *pTVDispInfo = (TV_DISPINFO *) pNMHDR;
	CString new_name = pTVDispInfo->item.pszText;

	// Don't allow renaming of "All Managers" or network clients.
	if (pTVDispInfo->item.mask & TVIF_TEXT && (selected_type == MANAGER || selected_type == WORKER)
	    && !IsType(GetSelectedWorkerType(), GenericClientType)) {
		// Don't allow any leading or trailing spaces.
		new_name.TrimLeft();
		new_name.TrimRight();

		// It's okay to put the string back into the array of unknown
		// size because we know we have not lengthened the string.
		strcpy(pTVDispInfo->item.pszText, (LPCTSTR) new_name);

		if (!new_name.IsEmpty()) {
			// New name is non-empty; change it.  (If new name is an empty string, 
			// do nothing -- old name remains unchanged.)

			if (selected_type == WORKER) {
				// Modify worker's name.
				strcpy(GetSelectedWorker()->name, pTVDispInfo->item.pszText);

				// Assign disambiguating integer identifiers to same-named managers.
				GetSelectedManager()->IndexWorkers();
			} else {
				// Modify manager's name.
				strcpy(GetSelectedManager()->name, pTVDispInfo->item.pszText);
				theApp.pView->m_pPageNetwork->ShowData();

				// Assign disambiguating integer identifiers to same-named managers.
				theApp.manager_list.IndexManagers();
			}

			// Changing name of selected item.
			m_TWorkers.SetItemText(m_TWorkers.GetSelectedItem(), pTVDispInfo->item.pszText);

			// See if a network server worker is selected.
			if (IsType(GetSelectedWorkerType(), GenericServerType)) {
				Worker *server = GetSelectedWorker();

				// See if this server has a client worker assigned to it.
				if (server->net_partner) {
					// Change the worker's name.
					strcpy(server->net_partner->name,
					       "[" + (CString) pTVDispInfo->item.pszText + "]");

					// Update the tree control to reflect the new name.
					m_TWorkers.SetItemText(GetWorker(server->net_partner),
							       server->net_partner->name);
				}
			}
			// Re-sort the manager list.
			m_TWorkers.SortChildren(m_TWorkers.GetRootItem());

			// Update the Results Display page.
			theApp.pView->m_pPageDisplay->Update();
		}
	}

	editing_label = FALSE;
	if (old_label_name) {
		delete old_label_name;

		old_label_name = NULL;
	}
	*pResult = 0;
}

LRESULT CWorkerView::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_WINDOWPOSCHANGING)
		((WINDOWPOS *) lParam)->flags =
		    ((WINDOWPOS *) lParam)->flags & (0xffffffff ^ SWP_NOACTIVATE) & (0xffffffff ^ SWP_HIDEWINDOW);
	return CDialog::DefWindowProc(message, wParam, lParam);
}

BOOL CWorkerView::PreTranslateMessage(MSG * pMsg)
{
	// CG: This block was added by the Pop-up Menu component
	{
		// Shift+F10: show pop-up menu.
		if ((((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN) &&	// If we hit a key and
		      (pMsg->wParam == VK_F10) && (GetKeyState(VK_SHIFT) & ~1)) != 0) ||	// it's Shift+F10 OR
		    (pMsg->message == WM_CONTEXTMENU))	// Natural keyboard key
		{
			CRect rect;

			GetClientRect(rect);
			ClientToScreen(rect);

			CPoint point = rect.TopLeft();

			point.Offset(5, 5);
			OnContextMenu(NULL, point);

			return TRUE;
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CWorkerView::OnBegindragTWorkers(NMHDR * pNMHDR, LRESULT * pResult)
{
	// check that input parameter is valid
	if (!pNMHDR) {
		return;
	}
	// Select the item to drag.
	NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW *) pNMHDR;

	m_TWorkers.SelectItem(pNMTreeView->itemNew.hItem);

	// Check to see if dragging is apropriate.
	if (theApp.pView->m_pPropSheet->GetActiveIndex() == DISPLAY_PAGE) {
		CPoint point;

		theApp.pView->dragging = TRUE;	// Set flag indicating that a drag operation is underway.

		// set hot spot to middle of 16x16 icon
		point.x = 8;
		point.y = 8;

		// set drag icon to icon of current selection
		theApp.pView->p_DragImage = m_TWorkers.CreateDragImage(pNMTreeView->itemNew.hItem);
		theApp.pView->p_DragImage->BeginDrag(0, point);
	}
	*pResult = 0;
}

int CWorkerView::GetSelectedType()
{
	return selected_type;
}

//
// This function selects the manager that the user just right clicked on.
// It removes anoying flickering when right clicking on a manager.
// 
void CWorkerView::OnRclickTWorkers(NMHDR * pNMHDR, LRESULT * pResult)
{
	CPoint point;
	TV_HITTESTINFO test_info;

	// Get the cursor position.
	GetCursorPos(&point);
	test_info.pt = point;
	m_TWorkers.ScreenToClient(&(test_info.pt));
	// Check to see whether the cursor is on an item, and if the item a manager.
	m_TWorkers.HitTest(&test_info);
	if (test_info.hItem) {
		if (m_TWorkers.GetParentItem(test_info.hItem) &&
		    !m_TWorkers.GetParentItem(m_TWorkers.GetParentItem(test_info.hItem))) {
			// Item is a manager, select it.
			m_TWorkers.SelectItem(test_info.hItem);
			// CG: This function was added by the Pop-up Menu component
			CMenu menu;

			VERIFY(menu.LoadMenu(CG_IDR_POPUP_WORKER_VIEW));

			CMenu *pPopup = menu.GetSubMenu(0);

			ASSERT(pPopup != NULL);

			// Get the first non-child window.
			CWnd *pWndPopupOwner = this;

			while (pWndPopupOwner->GetStyle() & WS_CHILD)
				pWndPopupOwner = pWndPopupOwner->GetParent();

			// Save the item so that the user can refresh.
			theApp.pView->right_clicked_item = test_info.hItem;

			pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, pWndPopupOwner);
		}
	}
	*pResult = 0;
}

void CWorkerView::Initialize()
{
	// Associate the image list with the worker tree control.
	m_ImageList.Create(IDB_WORKER_ICONS, WORKER_ICON_SIZE, WORKER_ICON_EXPAND, WORKER_ICON_BGCOLOR);
	m_TWorkers.SetImageList(&m_ImageList, TVSIL_NORMAL);

	// Insert all managers item at the root of the tree view.
	hall_managers = m_TWorkers.InsertItem("All Managers",
					      WORKER_ICON_ALLMGRS, WORKER_ICON_ALLMGRS, TVI_ROOT, TVI_SORT);

	// Expand the all managers item so that managers will be visible.
	m_TWorkers.SetItemState(hall_managers, TVIS_EXPANDED, TVIS_EXPANDED);

	// Set the All Managers item's data.
	m_TWorkers.SetItemData(hall_managers, (DWORD) NULL);

	// Select all managers by default.
	m_TWorkers.SelectItem(hall_managers);
}

//
// Since CWorkerView is based on the Dialog class, the OnOK handler is called
// whenever the user presses the <Enter> key.  The <Enter> key being pressed
// will not cause any of the keyboard event handlers to be called.
//
void CWorkerView::OnOK()
{
	// Only trap the enter key if we're editing a label.
	if (!editing_label)
		return;

	// Set the focus to the tree control to end the editing session.
	// This will remove it from the temporary edit control.
	m_TWorkers.SetFocus();
}

//
// Since CWorkerView is based on the Dialog class, the OnCancel handler is 
// called whenever the user presses the <Esc> key.  The <Esc> key being pressed
// will not cause any of the keyboard event handlers to be called.
//
void CWorkerView::OnCancel()
{
	CEdit *label_edit;

	// Only trap the escape key if we're editing a label and we know its
	// original value.
	if (!editing_label || !old_label_name)
		return;

	// Set the label to the original text.
	if (!(label_edit = m_TWorkers.GetEditControl())) {
		ErrorMessage("Unable to retrieve edit box used to modify label in " "CWorkerView::OnCancel().");
		return;
	}
	label_edit->SetWindowText(*old_label_name);

	// Set the focus to the tree control to end the editing session.
	// This will remove it from the temporary edit control.
	m_TWorkers.SetFocus();
}
