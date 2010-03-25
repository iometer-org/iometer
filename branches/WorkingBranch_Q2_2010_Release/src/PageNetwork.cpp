/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Iometer / PageNetwork.cpp                                          ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implementation of the CPageNetwork class, which is    ## */
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
/* ##               2003-04-25 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Updated the global debug flag (_DEBUG) handling     ## */
/* ##                 of the source file (check for platform etc.).       ## */
/* ##               2003-03-28 (joe@eiler.net)                            ## */
/* ##               - changes so VC++ 7 (.NET) will compile correctly.    ## */
/* ##               2003-03-28 (joe@eiler.net)                            ## */
/* ##               - added new header info.                              ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include "stdafx.h"
#include "PageNetwork.h"
#include "GalileoView.h"

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
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// CPageNetwork property page

IMPLEMENT_DYNCREATE(CPageNetwork, CPropertyPage)

CPageNetwork::CPageNetwork():CPropertyPage(CPageNetwork::IDD)
{
	//{{AFX_DATA_INIT(CPageNetwork)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPageNetwork::~CPageNetwork()
{
}

void CPageNetwork::DoDataExchange(CDataExchange * pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPageNetwork)
	DDX_Control(pDX, EMaxSends, m_EMaxSends);
	DDX_Control(pDX, CConnectionRate, m_CConnectionRate);
	DDX_Control(pDX, DInterface, m_DInterface);
	DDX_Control(pDX, EConnectionRate, m_EConnectionRate);
	DDX_Control(pDX, SConnectionRate, m_SConnectionRate);
	DDX_Control(pDX, TTargets, m_TTargets);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPageNetwork, CPropertyPage)
    //{{AFX_MSG_MAP(CPageNetwork)
    ON_NOTIFY(NM_CLICK, TTargets, OnClickTTargets)
    ON_BN_CLICKED(CConnectionRate, OnCConnectionRate)
    ON_NOTIFY(UDN_DELTAPOS, SConnectionRate, OnDeltaposSConnectionRate)
    ON_EN_SETFOCUS(EConnectionRate, OnSetfocusEConnectionRate)
    ON_NOTIFY(TVN_SELCHANGING, TTargets, OnSelchangingTTargets)
    ON_CBN_SELCHANGE(DInterface, OnSelchangeDInterface)
    ON_EN_KILLFOCUS(EConnectionRate, OnKillfocusEConnectionRate)
    ON_NOTIFY(TVN_KEYDOWN, TTargets, OnKeydownTTargets)
    ON_NOTIFY(NM_DBLCLK, TTargets, OnDblclkTTargets)
    ON_NOTIFY(NM_SETFOCUS, TTargets, OnSetfocusTTargets)
    ON_EN_KILLFOCUS(EMaxSends, OnKillfocusEMaxSends)
ON_EN_SETFOCUS(EMaxSends, OnSetfocusEMaxSends)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CPageNetwork message handlers
BOOL CPageNetwork::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// Associate the image list with the TTargets tree control
	m_ImageList.Create(IDB_TARGET_ICONS, TARGET_ICON_SIZE, TARGET_ICON_EXPAND, TARGET_ICON_BGCOLOR);
	m_TTargets.SetImageList(&m_ImageList, TVSIL_NORMAL);
	m_TTargets.SetImageList(&m_ImageList, TVSIL_STATE);	// Use the same image list for state images

	// Set the spin control to translate an up click into a positive movement.
	m_SConnectionRate.SetRange(0, UD_MAXVAL);

	Reset();
	return TRUE;
}

//
// Grays out the check boxes in the target list, clears the interface
// selection, sets the conneciton rate check box to it's 3rd state,
// and hides the content of the connection rate edit box.
//
void CPageNetwork::Reset()
{
	// Clear the target list.
	m_TTargets.DeleteAllItems();

	// Clear the interface selection.
	ShowInterface();

	// Set the connection rate check box to it's ambiguous state.
	ShowConnectionRate();
	ShowSettings();

	selected = NULL;
	highlighted = NULL;

	EnableWindow();
}

//
// Enable or disable the tab's windows, if apropriate.
//
void CPageNetwork::EnableWindow(BOOL enable)
{
	Manager *manager;
	Worker *worker;
	int selected_type;
	CWnd *last_focused;
	BOOL enable_max_sends = false;

	// Only enable thingies if a test is not running.
	if (theApp.test_state != TestIdle)
		return;

	selected_type = theApp.pView->m_pWorkerView->GetSelectedType();
	switch (selected_type) {
	case WORKER:
		// Get the current worker selection.
		worker = theApp.pView->m_pWorkerView->GetSelectedWorker();

		enable = enable && IsType(worker->Type(), GenericServerType);
		enable_max_sends = enable && IsType(worker->Type(), VIServerType);

		// If the selected worker is a disk worker,
		// switch to the disk targets page.
		if (IsType(worker->Type(), GenericDiskType) &&
		    theApp.pView->m_pPropSheet->GetActiveIndex() == NETWORK_PAGE) {
			last_focused = GetFocus();
			theApp.pView->m_pPropSheet->SetActivePage(DISK_PAGE);
			if (last_focused)
				last_focused->SetFocus();
		}
		break;
	case MANAGER:
		// Get the current manager selection.
		manager = theApp.pView->m_pWorkerView->GetSelectedManager();

		enable = enable && manager->WorkerCount(GenericServerType);
		enable_max_sends = enable && manager->WorkerCount(VIServerType);

		// If the selected manager has no network servers but has disk workers,
		// switch to the disk targets page.
		if (!manager->WorkerCount(GenericServerType) &&
		    manager->WorkerCount(GenericDiskType) &&
		    theApp.pView->m_pPropSheet->GetActiveIndex() == NETWORK_PAGE) {
			enable = FALSE;
			last_focused = GetFocus();
			theApp.pView->m_pPropSheet->SetActivePage(DISK_PAGE);
			if (last_focused)
				last_focused->SetFocus();
		}
		break;
	case ALL_MANAGERS:
		// Only enable the connection rate controls if there exists a
		// server worker.
		enable = enable && theApp.manager_list.WorkerCount(GenericServerType);
		enable_max_sends = enable && (theApp.manager_list.WorkerCount(VIServerType));
	}

	// Do not enable the target list for all managers.
	m_TTargets.EnableWindow(enable && selected_type != ALL_MANAGERS);
	m_DInterface.EnableWindow(enable && selected_type != ALL_MANAGERS);

	m_EMaxSends.EnableWindow(enable_max_sends);

	// Enable the connection rate check box.
	m_CConnectionRate.EnableWindow(enable);

	// Enable the connection rate edit box and spin control if the connection 
	// rate check box is checked.
	m_EConnectionRate.EnableWindow(enable && m_CConnectionRate.GetCheck() == 1);
	m_SConnectionRate.EnableWindow(enable && m_CConnectionRate.GetCheck() == 1);

	// The spin button sometimes needs to be redrawn even if it didn't change
	// when the edit box was partially redrawn.
	m_SConnectionRate.Invalidate(TRUE);

	// Redraw the page.  Note that all the child windows will be redrawn too.
	// Also note that only windows that have already been invalidated will get
	// redrawn, avoiding much flicker.
	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASE);
}

//
// Adds a manager and it's interfaces to the tree view.
//
void CPageNetwork::AddManager(Manager * manager)
{
	TV_INSERTSTRUCT tvstruct;
	HTREEITEM hmgr, hifc;
	Manager *sel_mgr;
	int n;

	// Do not add the manager if it doesn't have any interfaces that are
	// accessible from the selected manager.
	if (!(sel_mgr = theApp.pView->m_pWorkerView->GetSelectedManager()))
		return;

	// First see if it has any interfaces, if not, don't add it.
	if ((!sel_mgr->InterfaceCount(GenericNetType)) ||
	    // Next see if the selected manager only has TCP interfaces, but the
	    // manager to add has none.
	    ((sel_mgr->InterfaceCount(GenericTCPType) ==
	      sel_mgr->InterfaceCount(GenericNetType)) && !manager->InterfaceCount(GenericTCPType)) ||
	    // Next see if the selected manager only has VI interfaces, but the
	    // manager to add has none.
	    ((sel_mgr->InterfaceCount(GenericVIType) ==
	      sel_mgr->InterfaceCount(GenericNetType)) && !manager->InterfaceCount(GenericVIType))) {
		return;
	}
	// At least one of the manager's interfaces are accessible.

	// Set the flags to indicate that the image, selected image, text and 
	// parameter are to be set.
	tvstruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	// Position the manager in the tree.
	tvstruct.hParent = TVI_ROOT;
	tvstruct.hInsertAfter = TVI_SORT;
	// Set the item's text to the manager's name.
	tvstruct.item.pszText = manager->name;
	// Set the icon associated with the manager.
	tvstruct.item.iImage = TARGET_ICON_MANAGER;
	tvstruct.item.iSelectedImage = TARGET_ICON_MANAGER;
	// Set the item data associated with the manager (pointer to Manager object).
	tvstruct.item.lParam = (LPARAM) manager;
	// note: manager has no state (check box) icon!

	// Insert the manager into the tree control.
	hmgr = m_TTargets.InsertItem(&tvstruct);
	if (hmgr == NULL) {
		ErrorMessage("InsertItem() for target manager failed in CPageNetwork::AddManager()");
		return;
	}
	// Set the interface item properties here.  The only property that changes
	// is the name.
	// Set the mask to indicate that the image, selected image, text, and state
	// are valid and should be set.
	tvstruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_STATE;
	// Set the state mask to indicate that the state parameter refers to the
	// index of the state image in the state image list.
	tvstruct.item.stateMask = TVIS_STATEIMAGEMASK;
	// Set the parent of this item to be the manager item we just inserted.
	tvstruct.hParent = hmgr;
	// Insert the interface after all its siblings.
	tvstruct.hInsertAfter = TVI_LAST;
	// Set the normal and selected icons associated with the interface.
	tvstruct.item.iImage = TARGET_ICON_NETWORK;
	tvstruct.item.iSelectedImage = TARGET_ICON_NETWORK;

	// Set the interface's state (check box) depending on whether the
	// existing interfaces are grayed (it should also be grayed) or
	// either checked or cleared (it should be cleared).  Default to cleared
	// if this is the first manager.  This needs to be done in case a manager
	// logs in while we are already displaying a selection.
	tvstruct.item.state = INDEXTOSTATEIMAGEMASK(TARGET_STATE_UNCHECKED);

	// insert the manager's network interfaces as its children in the tree
	for (n = 0; n < manager->InterfaceCount(GenericNetType); n++) {
		// Only add interfaces that are accessible.
		if (!sel_mgr->InterfaceCount(manager->GetInterface(n, GenericNetType)->type)) {
			continue;
		}
		// name the interface
		tvstruct.item.pszText = manager->GetInterface(n, GenericNetType)->name;

		// insert the interface into the tree control
		hifc = m_TTargets.InsertItem(&tvstruct);
		if (hifc == NULL) {
			ErrorMessage("InsertItem() for interface failed in CPageNetwork::AddManager()");
			return;
		}
	}

	// make the manager's child icons visible
	if (n && !m_TTargets.Expand(tvstruct.hParent, TVE_EXPAND)) {
		ErrorMessage("Expand() for target manager failed in CPageNetwork::AddManager()");
	}
}

//
// Takes a given manager and its interfaces out of the tree.
//
void CPageNetwork::RemoveManager(Manager * manager)
{
	HTREEITEM hmgr;

	// Walk the tree until we find the given manager.
	for (hmgr = m_TTargets.GetRootItem(); hmgr; hmgr = m_TTargets.GetNextSiblingItem(hmgr)) {
		if (m_TTargets.GetItemData(hmgr) == (DWORD_PTR) manager) {
			m_TTargets.DeleteItem(hmgr);
			break;
		}
	}
}

//
// Fill in the target list.
//
void CPageNetwork::ShowTargets()
{
	Manager *manager;
	int mgr_no = 0;

	m_TTargets.DeleteAllItems();

	// Determine what interfaces are visible to the selected item.
	if (theApp.pView->m_pWorkerView->GetSelectedType() == ALL_MANAGERS)
		return;

	// Loop through the managers and add them to the tree view.
	while (manager = theApp.manager_list.GetManager(mgr_no++)) {
		// Add all accessible interfaces to the target list.
		AddManager(manager);
	}
}

void CPageNetwork::ShowTargetSelection()
{
	Manager *manager, *partner_manager;
	Worker *worker, *partner_worker;
	char *remote_address;
	HTREEITEM hmgr;
	HTREEITEM hifc;
	int i, expected_worker;
	TargetSelType state;

	switch (theApp.pView->m_pWorkerView->GetSelectedType()) {
	case WORKER:
		worker = theApp.pView->m_pWorkerView->GetSelectedWorker();

		// Get the interface to check.
		if (IsType(worker->Type(), GenericServerType)) {
			if (!worker->net_partner) {
				partner_manager = NULL;
			} else {
				partner_manager = worker->net_partner->manager;
				if (IsType(worker->GetTarget(0)->spec.type, TCPClientType))
					remote_address = worker->GetTarget(0)->spec.tcp_info.remote_address;
				else if (IsType(worker->GetTarget(0)->spec.type, VIClientType))
					remote_address = worker->GetTarget(0)->spec.vi_info.remote_nic_name;
				else {
					ErrorMessage("Invalid client target type in CPageNetwork::"
						     "ShowTargetSelection().");
					return;
				}
			}
		} else if (IsType(worker->Type(), GenericClientType)) {
			partner_manager = worker->net_partner->manager;
			remote_address = worker->net_partner->spec.name;
		} else {
			return;
		}

		// Walk the target tree to find the correct partner manager.
		for (hmgr = m_TTargets.GetRootItem(); hmgr; hmgr = m_TTargets.GetNextSiblingItem(hmgr)) {
			if (m_TTargets.GetItemData(hmgr) == (DWORD_PTR) partner_manager) {
				// Check the appropriate interface on the partner manager.
				CheckInterface(hmgr, remote_address);
			} else {
				// Clear all check marks for this manager's interfaces.
				CheckInterface(hmgr, NULL);
			}
		}
		break;
	case MANAGER:
		// Get the selected manager.
		manager = theApp.pView->m_pWorkerView->GetSelectedManager();
		expected_worker = 0;

		// Loop through all the target tree items.
		for (hmgr = m_TTargets.GetRootItem(); hmgr; hmgr = m_TTargets.GetNextSiblingItem(hmgr)) {
			// Get the possible partner manager from the tree.
			partner_manager = (Manager *) m_TTargets.GetItemData(hmgr);

			// Loop through that manager item's interfaces.
			for (hifc = m_TTargets.GetChildItem(hmgr); hifc; hifc = m_TTargets.GetNextSiblingItem(hifc)) {
				// Initialize the state.  It may or may not be changed, 
				// depending on whether we find it selected by a worker.
				state = TargetUnChecked;

				// Check the interface for selection by any of the manager's
				// workers
				for (i = 0; i < manager->WorkerCount(GenericServerType); i++) {
					// Get the net partner of the expected worker.
					partner_worker = manager->GetWorker(i, GenericServerType)->net_partner;

					// If the partner worker exists and it's manager is the
					// same as the manager in the target tree that we are
					// currently looking at and..
					if (partner_worker && partner_worker->manager == partner_manager) {
						// if the net addresses match but..
						if (m_TTargets.GetItemText(hifc) ==
						    partner_worker->GetLocalNetworkInterface()) {
							// the worker is not the expected worker..
							if (i != expected_worker || state != TargetUnChecked) {
								// gray the selection
								SetAllCheck(TargetGrayed);
								return;
							}
							state = TargetChecked;
							expected_worker++;
						}
					}
				}
				SetSelectionCheck(hifc, state);
			}
		}
		break;
	default:
		SetAllCheck(TargetGrayed);
		break;
	}
}

//
// Used to select one or more targets.  May or may not clear any previous
// selection depending on the replace parameter.
//
void CPageNetwork::SelectRange(HTREEITEM hstart, HTREEITEM hend, BOOL replace, TargetSelType state)
{
	HTREEITEM hmgr, hifc;
	BOOL in_range;

	if (!hstart || !hend) {
		ErrorMessage("Unexpectedly found start or end point of selection"
			     " range equal NULL in CPageNetwork::SelectRange().");
		return;
	}
	// Clear all the check boxes if forced to or if a multiple selection occurs.
	if (replace)
		SetAllCheck(TargetUnChecked);

	// The last selection is the start point.
	selected = hstart;
	highlighted = hend;

	// Note that if both endpoints are the same, other selections will be 
	// unchecked depending on the value of the 'replace' variable.
	if (hstart == hend) {
		SetSelectionCheck(hstart, state);
		return;
	}
	// Since we don't know the relationship between the clicked item
	// and the last selection (up or down), we do not know what
	// direction to select targets in.  Do a linear seach of the target tree
	// until we find either the start or end point.  From there, mark
	// all the targets the specified selection type until we reach the end or
	// start (whichever we didn't get before).
	in_range = FALSE;
	for (hmgr = m_TTargets.GetRootItem(); hmgr; hmgr = m_TTargets.GetNextSiblingItem(hmgr)) {
		for (hifc = m_TTargets.GetChildItem(hmgr); hifc; hifc = m_TTargets.GetNextSiblingItem(hifc)) {
			// Do we have either the previously selected item or the
			// clicked item?                        
			if (hifc == hstart || hifc == hend)
				in_range = !in_range;

			// If we are between the start and end point, set the state.
			if (in_range || hifc == hstart || hifc == hend)
				SetSelectionCheck(hifc, state);
		}
	}
}

//
// Sets the specified tree item's check box to the specified state.
//
void CPageNetwork::SetSelectionCheck(HTREEITEM hitem, TargetSelType selection)
{
	// set the interface icon's associated state icon
	if (!m_TTargets.SetItemState(hitem, INDEXTOSTATEIMAGEMASK(selection), TVIS_STATEIMAGEMASK)) {
		ErrorMessage("SetItemState() failed in CPageNetwork::" "SetSelectionCheck()");
	}
}

//
// Retrieves the target's check box state.
//
TargetSelType CPageNetwork::GetSelectionCheck(HTREEITEM hitem)
{
	return (TargetSelType) STATEIMAGEMASKTOINDEX(m_TTargets.GetItemState(hitem, TVIS_STATEIMAGEMASK));
}

//
// Uniquely selects an interface given a manager and a string representing
// the desired interface.  
//
void CPageNetwork::CheckInterface(HTREEITEM hmgr, char *net_address)
{
	HTREEITEM hifc;

	if (!hmgr) {
		ErrorMessage("invalid tree item in CPageNetwork::CheckInterface()");
		return;
	}
	// walk through its list of interfaces
	for (hifc = m_TTargets.GetChildItem(hmgr); hifc; hifc = m_TTargets.GetNextSiblingItem(hifc)) {
		// check to see if the current interface should have a check mark
		if (net_address && !strcmp(m_TTargets.GetItemText(hifc), net_address)) {
			// found it!  set its check mark
			SetSelectionCheck(hifc, TargetChecked);
		} else {
			// not the one.  clear its check mark
			SetSelectionCheck(hifc, TargetUnChecked);
		}
	}
}

//
// Sets all the interface's check box to the specified state.
//
void CPageNetwork::SetAllCheck(TargetSelType selection)
{
	HTREEITEM hmgr;
	HTREEITEM hifc;

	// Get the first manager item of the target tree
	for (hmgr = m_TTargets.GetRootItem(); hmgr; hmgr = m_TTargets.GetNextSiblingItem(hmgr)) {
		// walk through its list of interfaces
		for (hifc = m_TTargets.GetChildItem(hmgr); hifc; hifc = m_TTargets.GetNextSiblingItem(hifc)) {
			SetSelectionCheck(hifc, selection);
		}
	}
}

//
// Fills in the list of interfaces and displays a selection
// if all children workers have the same interface.
//
void CPageNetwork::ShowInterface()
{
	Manager *manager;
	Worker *worker;

	m_DInterface.ResetContent();

	// Check that either a manager or a worker is selected.
	if (!(manager = theApp.pView->m_pWorkerView->GetSelectedManager())) {
		return;
	}
	// Put the manager's network interfaces into the DInterface combo box
	for (int i = 0; i < manager->InterfaceCount(GenericNetType); i++) {
		if (m_DInterface.AddString(manager->GetInterface(i, GenericNetType)->name) != i) {
			ErrorMessage("AddString() for interface failed in CPageNetwork::SetTargetSelection()");
			return;
		}
	}

	// Select (in the combo box) the worker's or manager's selected interface.
	if (worker = theApp.pView->m_pWorkerView->GetSelectedWorker()) {
		// select the current interface (if any) in the combo box
		m_DInterface.SelectString(-1, worker->GetLocalNetworkInterface());
	} else {
		// select the current interface (if any) in the combo box
		m_DInterface.SelectString(-1, manager->GetLocalNetworkInterface());
	}
}

//
// Displays the connection rate settings for the current selection in the
// worker view.  If the selection is a manager or all managers, displays
// a value if all the children's values are the same.
//
void CPageNetwork::ShowConnectionRate()
{
	Manager *manager;
	Worker *worker;
	int trans_per_conn = 0;
	int test_connection_rate = 0;

	switch (theApp.pView->m_pWorkerView->GetSelectedType()) {
	case WORKER:
		worker = theApp.pView->m_pWorkerView->GetSelectedWorker();
		// update controls with worker's data
		if (IsType(worker->Type(), GenericNetType)) {
			trans_per_conn = worker->GetTransPerConn(GenericNetType);
			test_connection_rate = worker->GetConnectionRate(GenericNetType);
		}
		break;
	case MANAGER:
		manager = theApp.pView->m_pWorkerView->GetSelectedManager();
		trans_per_conn = manager->GetTransPerConn(GenericServerType);
		test_connection_rate = manager->GetConnectionRate(GenericServerType);
		break;
	default:
		trans_per_conn = theApp.manager_list.GetTransPerConn(GenericServerType);
		test_connection_rate = theApp.manager_list.GetConnectionRate(GenericServerType);
		break;
	}
	// If the test connection rate settings are different between a manager's
	// workers, set the state of the check box to AUTO3STATE and disable the
	// edit box and spin control.
	SetDlgItemInt(EConnectionRate, trans_per_conn);
	if (test_connection_rate == AMBIGUOUS_VALUE) {
		m_CConnectionRate.SetButtonStyle(BS_AUTO3STATE);
		m_EConnectionRate.SetPasswordChar(32);
		m_EConnectionRate.Invalidate(TRUE);

		// Set check box to undetermined state.
		CheckDlgButton(CConnectionRate, 2);
	} else {
		m_CConnectionRate.SetButtonStyle(BS_AUTOCHECKBOX);
		CheckDlgButton(CConnectionRate, test_connection_rate);

		if (test_connection_rate == ENABLED_VALUE && trans_per_conn != AMBIGUOUS_VALUE) {
			m_EConnectionRate.SetPasswordChar(0);
		} else {
			m_EConnectionRate.SetPasswordChar(32);
			m_EConnectionRate.Invalidate();
		}
	}
}

//
// Sets the focus to the list item specified by cursor movement or mouse event.
//
void CPageNetwork::ShowFocus()
{
	for (HTREEITEM hmgr = m_TTargets.GetRootItem(); hmgr; hmgr = m_TTargets.GetNextSiblingItem(hmgr)) {
		for (HTREEITEM hifc = m_TTargets.GetChildItem(hmgr); hifc; hifc = m_TTargets.GetNextSiblingItem(hifc)) {
			if (hifc == highlighted) {
				m_TTargets.SetItemState(hifc, TVIS_SELECTED, TVIS_SELECTED);
			} else {
				m_TTargets.SetItemState(hifc, NULL, TVIS_SELECTED);
			}
		}
	}
}

//
// Displays the values from memory in the GUI.
//
void CPageNetwork::ShowData()
{
	Manager *manager;
	Worker *worker;

	// Only enable the display if a network worker or a manager with a 
	// network worker is selected.
	manager = theApp.pView->m_pWorkerView->GetSelectedManager();
	worker = theApp.pView->m_pWorkerView->GetSelectedWorker();

	if ((theApp.pView->m_pWorkerView->GetSelectedType() == ALL_MANAGERS) ||
	    (manager && !manager->InterfaceCount(GenericNetType)) ||
	    (worker && !IsType(worker->Type(), GenericNetType))) {
		Reset();
		return;
	}
	// This is a new view of the target assignment.  
	// It has not been modified.
	selected = NULL;
	highlighted = NULL;
	// Show the target list.
	ShowTargets();
	// Set the target selection check boxes.
	ShowTargetSelection();
	// Bold the last selected item.
	ShowFocus();
	// Show the connection rate settings.
	ShowConnectionRate();
	// Show the selected interface.
	ShowInterface();
	ShowSettings();
	// Enable the apropriate windows and redraw the page.

	EnableWindow();
}

//
// Process mouse click in the target list.  For a worker, assign the selected
// target.  For a manager, distribute the assigned targets among its net servers.
//
void CPageNetwork::OnClickTTargets(NMHDR * pNMHDR, LRESULT * pResult)
{
	CPoint point;
	TV_HITTESTINFO test_info;
	TargetSelType state;

	// Get the cursor position.
	GetCursorPos(&point);
	test_info.pt = point;
	m_TTargets.ScreenToClient(&(test_info.pt));

	// Check to see whether the cursor is on an item.
	m_TTargets.HitTest(&test_info);

	// Check that we have an interface item.
	if (!test_info.hItem || !m_TTargets.GetParentItem(test_info.hItem))
		return;

	// A new target assignment is being made.  Clear the results since they are
	// for a configuration we no longer have.
	theApp.pView->ResetDisplayforNewTest();

	// Toggle the selection if the control key is pressed.
	if (GetKeyState(VK_CONTROL) & 0x8000 && GetSelectionCheck(test_info.hItem) == TargetChecked) {
		state = TargetUnChecked;
	} else {
		state = TargetChecked;
	}

	switch (theApp.pView->m_pWorkerView->GetSelectedType()) {
	case WORKER:
		// All clicks work the same for the worker.
		SelectRange(test_info.hItem, test_info.hItem, TRUE, state);
		break;
	case MANAGER:
		// A shift click extends the selection from the last selected item
		// to the currently focused item.  When the control key is also
		// pressed, any previous selection is not cleared.
		if ((GetKeyState(VK_SHIFT) & 0x8000) && selected) {
			// We have a previous item (not the first click) and the shift
			// key is down.
			SelectRange(selected, test_info.hItem, !(GetKeyState(VK_CONTROL) & 0x8000));
		} else if (GetKeyState(VK_CONTROL) & 0x8000) {
			// The control key is down.
			SelectRange(test_info.hItem, test_info.hItem, FALSE, state);
		} else {
			SelectRange(test_info.hItem, test_info.hItem, TRUE, state);
		}
		break;
	default:
		ErrorMessage("Unexpected selection type in CPageNetwork::" "OnClickTTargets().");
		return;
	}

	// immediately refresh the display (create/delete NetClients as needed)
	StoreTargetSelection();
	ShowSettings();
	EnableWindow();

	*pResult = 0;
}

void CPageNetwork::StoreTargetSelection()
{
	HTREEITEM hmgr, hifc;
	Manager *manager;
	Worker *worker;
	Target_Spec new_target;
	int local_interface_no;
	int interface_no;
	int target_count = 0;
	int expected_worker = 0;

	// Make sure we have a selected manager (or worker).
	manager = theApp.pView->m_pWorkerView->GetSelectedManager();
	if (!manager) {
		ErrorMessage("Unexpectedly found no selected manager in " "CPageNetwork::StoreTargetSelection.");
		return;
	}
	// Count the number of assigned targets.
	for (hmgr = m_TTargets.GetRootItem(); hmgr; hmgr = m_TTargets.GetNextSiblingItem(hmgr)) {
		for (hifc = m_TTargets.GetChildItem(hmgr); hifc; hifc = m_TTargets.GetNextSiblingItem(hifc)) {
			if (GetSelectionCheck(hifc) == TargetChecked)
				target_count++;
		}
	}

	// Make sure we are not assigning more targets than we have workers.
	if (target_count > manager->WorkerCount(GenericServerType)) {
		ErrorMessage("You do not have enough network workers to assign all " "the selected targets.");
		// Restore the last selection.
		ShowTargetSelection();
		// Set the focus to the target list.
		::SetFocus(m_TTargets);
		return;
	}
	// The selection will succeed.  Remove current network clients.
	if (worker = theApp.pView->m_pWorkerView->GetSelectedWorker())
		worker->RemoveTargets(GenericNetType);
	else
		manager->RemoveTargets(GenericNetType);

	// Get the assigned local interface to use for the connection.
	local_interface_no = m_DInterface.GetCurSel();

	// Assign the targets.
	// Loop through all managers.
	for (hmgr = m_TTargets.GetRootItem(); hmgr; hmgr = m_TTargets.GetNextSiblingItem(hmgr)) {
		// Loop through all interfaces of a manager.
		interface_no = 0;
		for (hifc = m_TTargets.GetChildItem(hmgr); hifc; hifc = m_TTargets.GetNextSiblingItem(hifc)) {
			if (GetSelectionCheck(hifc) == TargetChecked) {
				// Are we dealing with a worker or a manager?
				if (!worker) {
					// A manager is selected.  Get the next available 
					// network server worker.
					worker = manager->GetWorker(expected_worker++, GenericServerType);

				}
				// Get the interface of the selected target from the manager
				// whose interface is selected.
				memcpy(&new_target,
				       ((Manager *) m_TTargets.GetItemData(hmgr))->GetInterface(interface_no,
												GenericNetType),
				       sizeof(Target_Spec));

				// Set the local and remote addresses of the connection.
				// The remote address used by the server is stored as the local
				// address for some manager's interface.
				if (IsType(new_target.type, TCPClientType)) {
					strcpy(new_target.tcp_info.remote_address, new_target.name);

					// Verify that the locally assigned interface matches the
					// selected target's interface.
					if (local_interface_no >= manager->InterfaceCount(GenericTCPType)) {
						// Use the first TCP interface.
						local_interface_no = 0;
						m_DInterface.SetCurSel(local_interface_no);
					}
				} else if (IsType(new_target.type, VIClientType)) {
					strcpy(new_target.vi_info.remote_nic_name, new_target.name);
					memcpy(&new_target.vi_info.remote_address,
					       &new_target.vi_info.local_address, VI_ADDRESS_SIZE);

					// Verify that the locally assigned interface matches the
					// selected target's interface.
					if (local_interface_no <= manager->InterfaceCount(GenericTCPType)) {
						// Use the first VI interface.
						local_interface_no = manager->InterfaceCount(GenericTCPType);
						m_DInterface.SetCurSel(local_interface_no);
					}
				} else {
					ErrorMessage("Invalid target type for new target in "
						     "CPageNetwork::StoreTargetSelection().");
				}
				worker->AddTarget(&new_target);

				// Record information about what local interface the server
				// should use.
				worker->SetLocalNetworkInterface(local_interface_no);

				// Create the corresponding network client.
				worker->CreateNetClient((Manager *)
							m_TTargets.GetItemData(hmgr), new_target.type);

				// Clear the worker pointer so that the next iteration
				// through the loop (for a manager with multiple selected
				// interfaces) will get the next available network server.
				worker = NULL;
			}
			interface_no++;
		}
	}
}

void CPageNetwork::OnCConnectionRate()
{
	Manager *manager;
	Worker *worker;

	// change the check box to true/false only (having clicked, cannot 
	// return to intermediate state)
	m_CConnectionRate.SetButtonStyle(BS_AUTOCHECKBOX);
	if (IsDlgButtonChecked(CConnectionRate) == 1) {
		m_EConnectionRate.SetPasswordChar(0);
		if (GetDlgItemInt(EConnectionRate) == AMBIGUOUS_VALUE)
			SetDlgItemInt(EConnectionRate, 0);
	} else {
		m_EConnectionRate.SetPasswordChar(32);
	}
	switch (theApp.pView->m_pWorkerView->GetSelectedType()) {
	case WORKER:
		worker = theApp.pView->m_pWorkerView->GetSelectedWorker();
		worker->SetConnectionRate(m_CConnectionRate.GetCheck());
		break;
	case MANAGER:
		manager = theApp.pView->m_pWorkerView->GetSelectedManager();
		manager->SetConnectionRate(m_CConnectionRate.GetCheck(), GenericServerType);
		break;
	case ALL_MANAGERS:
		theApp.manager_list.SetConnectionRate(m_CConnectionRate.GetCheck(), GenericServerType);
		break;
	}
	EnableWindow();
}

void CPageNetwork::OnDeltaposSConnectionRate(NMHDR * pNMHDR, LRESULT * pResult)
{
	int trans_per_conn = GetDlgItemInt(EConnectionRate);
	NM_UPDOWN *pNMUpDown = (NM_UPDOWN *) pNMHDR;

	trans_per_conn += pNMUpDown->iDelta;
	if (trans_per_conn < 0) {
		trans_per_conn = 0;
	} else {
		SetDlgItemInt(EConnectionRate, trans_per_conn);
	}
	*pResult = 0;
}

void CPageNetwork::EditSetfocus(CEdit * edit)
{
	if (!edit->GetPasswordChar())
		return;

	// Select everything in the edit box.
	edit->SetSel(0, -1);
	// Clear selection.
	edit->Clear();
	// Make the characters visible.
	edit->SetPasswordChar(0);
}

void CPageNetwork::OnSetfocusEConnectionRate()
{
	EditSetfocus(&m_EConnectionRate);
}

void CPageNetwork::OnSetfocusEMaxSends()
{
	EditSetfocus(&m_EMaxSends);
}

void CPageNetwork::OnSelchangingTTargets(NMHDR * pNMHDR, LRESULT * pResult)
{
	NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW *) pNMHDR;

	theApp.pView->ResetDisplayforNewTest();
	ShowFocus();

	// Return 1 so that the control doesn't update the selection since
	// we are managing it.
	*pResult = 1;
}

void CPageNetwork::OnSelchangeDInterface()
{
	Manager *manager;
	Worker *worker;

	switch (theApp.pView->m_pWorkerView->GetSelectedType()) {
	case WORKER:
		worker = theApp.pView->m_pWorkerView->GetSelectedWorker();
		worker->SetLocalNetworkInterface(m_DInterface.GetCurSel());
		break;
	case MANAGER:
		manager = theApp.pView->m_pWorkerView->GetSelectedManager();
		manager->SetLocalNetworkInterface(m_DInterface.GetCurSel());
		break;
	}

	ShowTargets();
	ShowTargetSelection();
	ShowSettings();
	EnableWindow();
}

void CPageNetwork::OnKillfocusEConnectionRate()
{
	Manager *manager;
	Worker *worker;

	// Restores the previous value if the edit box was left blank.
	if (!m_EConnectionRate.LineLength())
		ShowConnectionRate();
	else {
		switch (theApp.pView->m_pWorkerView->GetSelectedType()) {
		case WORKER:
			worker = theApp.pView->m_pWorkerView->GetSelectedWorker();
			worker->SetTransPerConn(GetDlgItemInt(EConnectionRate));
			break;
		case MANAGER:
			manager = theApp.pView->m_pWorkerView->GetSelectedManager();
			manager->SetTransPerConn(GetDlgItemInt(EConnectionRate), GenericServerType);
			break;
		case ALL_MANAGERS:
			theApp.manager_list.SetTransPerConn(GetDlgItemInt(EConnectionRate), GenericServerType);
			break;
		}
	}
	EnableWindow();
}

//
// Keyboard handler for the target tree control:
// 
// Behavior:
// The up and down arrows move the highlighted item up and down 
// respectively.  When modified with the shift key, the items selection is
// extended up and down, respectively.  
//
// Page up/page down are not handled and do nothing.
//
// The home and end keys move the highlight to the first/last item in the tree,
// scrolling it into view.  The shift key extends the selection to the 
// first/last item in the tree.
// 
// Unlike windows explorer, the non-modified keys do not select (set the check
// box) the item moved to.  There are no Ctrl-modified movement handlers, as
// Ctrl-PgUp and Ctrl-PgDn are already bound to the property sheet to change
// tabs.
//
// Ctrl-A selects all items.
// 
// The space bar selects the highlighted item and clears all other selections.
// When modified with the shift key, it extends the selection to the
// highlighted item.  When modified by the control key, the highlighted item's
// check box is toggled.
//
// Note that the keyboard handler function treats managers differently from
// workers to allow selecting multiple interfaces from the manager, whereas
// workers are currently restricted to a single selection for network targets.
// 
void CPageNetwork::OnKeydownTTargets(NMHDR * pNMHDR, LRESULT * pResult)
{
	TV_KEYDOWN *pTVKeyDown = (TV_KEYDOWN *) pNMHDR;

	// A new target assignment is being made.  Clear the results since they are
	// for a configuration we no longer have.
	theApp.pView->ResetDisplayforNewTest();

	if (theApp.pView->m_pWorkerView->GetSelectedType() == MANAGER)
		KeyMultiSel(pTVKeyDown->wVKey);
	else
		KeySingleSel(pTVKeyDown->wVKey);
	*pResult = 0;
}

//
// Single selection keyboard handler for the tree control.
//
void CPageNetwork::KeySingleSel(WORD wVKey)
{
	BOOL shift = GetKeyState(VK_SHIFT) & 0x8000;
	BOOL control = GetKeyState(VK_CONTROL) & 0x8000;

	// Make sure there are interfaces.
	if (!highlighted)
		return;

	switch (wVKey) {
	case VK_UP:
		SetFocusUp();
		// Only select items if the shift key is pressed.
		if (shift) {
			SelectRange(highlighted, highlighted, TRUE, TargetChecked);
			StoreTargetSelection();
		}
		break;
	case VK_DOWN:
		SetFocusDown();
		// Only select items if the shift key is pressed.
		if (shift) {
			SelectRange(highlighted, highlighted, TRUE, TargetChecked);
			StoreTargetSelection();
		}
		break;
	case VK_HOME:
		SetFocusHome();
		// Only select items if the shift key is pressed.
		if (shift) {
			SelectRange(highlighted, highlighted, TRUE, TargetChecked);
			StoreTargetSelection();
		}
		break;
	case VK_END:
		SetFocusEnd();
		// Only select items if the shift key is pressed.
		if (shift) {
			SelectRange(highlighted, highlighted, TRUE, TargetChecked);
			StoreTargetSelection();
		}
		break;
	case VK_SPACE:
		if (control)	// toggle.
		{
			// Toggle the selection, but do not clear any other items.
			if (GetSelectionCheck(highlighted) == TargetChecked) {
				SelectRange(highlighted, highlighted, FALSE, TargetUnChecked);
			} else {
				SelectRange(highlighted, highlighted, FALSE, TargetChecked);
			}
		} else		// normal selection.
		{
			// Extend the selection and clear any other items.
			SelectRange(highlighted, highlighted, TRUE, TargetChecked);
		}

		ShowFocus();
		StoreTargetSelection();
		break;
	}
	ShowSettings();
	EnableWindow();
}

//
// Multi selection keyboard handler for the tree control.
//
void CPageNetwork::KeyMultiSel(WORD wVKey)
{
	BOOL shift = GetKeyState(VK_SHIFT) & 0x8000;
	BOOL control = GetKeyState(VK_CONTROL) & 0x8000;

	// Make sure there are interfaces.
	if (!highlighted)
		return;

	switch (wVKey) {
	case VK_UP:
		if (!selected && shift)
			selected = highlighted;
		SetFocusUp();
		// Only select items if the shift key is pressed.
		if (shift) {
			SelectRange(selected, highlighted, !control, TargetChecked);
			StoreTargetSelection();
		}
		break;
	case VK_DOWN:
		if (!selected && shift)
			selected = highlighted;

		SetFocusDown();
		// Only select items if the shift key is pressed.
		if (shift) {
			SelectRange(selected, highlighted, !control, TargetChecked);
			StoreTargetSelection();
		}
		break;
	case VK_HOME:
		if (!selected && shift)
			selected = highlighted;

		SetFocusHome();
		// Only select items if the shift key is pressed.
		if (shift) {
			SelectRange(selected, highlighted, !control, TargetChecked);
			StoreTargetSelection();
		}
		break;
	case VK_END:
		if (!selected && shift)
			selected = highlighted;

		SetFocusEnd();
		// Only select items if the shift key is pressed.
		if (shift) {
			SelectRange(selected, highlighted, !control, TargetChecked);
			StoreTargetSelection();
		}
		break;
	case VK_SPACE:
		if (shift) {
			// Extend the selection. Clear any other items if the control
			// key is not pressed.
			SelectRange(selected, highlighted, !control, TargetChecked);
		} else if (control) {	// toggle.
			// Toggle the selection, but do not clear any other items.
			if (GetSelectionCheck(highlighted) == TargetChecked) {
				SelectRange(highlighted, highlighted, FALSE, TargetUnChecked);
			} else {
				SelectRange(highlighted, highlighted, FALSE, TargetChecked);
			}
		} else {	// normal
			// Uniquely select the item with the focus.
			SelectRange(highlighted, highlighted, TRUE, TargetChecked);
		}

		ShowFocus();
		StoreTargetSelection();
		break;
	case 'A':
		if (control) {
			SetAllCheck(TargetChecked);
			StoreTargetSelection();
		}
	}
	ShowSettings();
	EnableWindow();
}

// 
// The SetFocusUp, Down, Home, and End functions handle keyboard input for
// the target tree.
//
void CPageNetwork::SetFocusUp()
{
	HTREEITEM hmgr, hifc;

	hifc = m_TTargets.GetPrevSiblingItem(highlighted);
	// Does the currently selected interface have a previous sibling?
	if (hifc) {
		// Yes. Set the focus to the sibling.
		highlighted = hifc;
	} else {
		// No.  Set the focus to the last interface on the first manager with 
		// interfaces in the tree before the currently focused interface's 
		// manager.
		hmgr = m_TTargets.GetParentItem(highlighted);
		while (hmgr = m_TTargets.GetPrevSiblingItem(hmgr)) {
			if (hifc = m_TTargets.GetChildItem(hmgr))
				break;
		}
		while (hifc) {
			highlighted = hifc;
			hifc = m_TTargets.GetNextSiblingItem(hifc);
		}
	}

	// Make sure the focused interface is visible, and if possible,
	// also its manager.
	m_TTargets.EnsureVisible(m_TTargets.GetParentItem(highlighted));
	m_TTargets.EnsureVisible(highlighted);
	ShowFocus();
}

void CPageNetwork::SetFocusDown()
{
	HTREEITEM hmgr, hifc;

	hifc = m_TTargets.GetNextSiblingItem(highlighted);
	// Does the currently focused interface have a next sibling?
	if (hifc) {
		// Yes. Set the focus to the sibling.
		highlighted = hifc;
	} else {
		// No.  Set the focus to the first interface on the first manager with 
		// interfaces in the tree after the currently focused interface's 
		// manager.
		hmgr = m_TTargets.GetParentItem(highlighted);
		while (hmgr = m_TTargets.GetNextSiblingItem(hmgr)) {
			if (hifc = m_TTargets.GetChildItem(hmgr)) {
				highlighted = hifc;
				break;
			}
		}
	}

	// Make the focused interface visible, and if possible,
	// also its manager.
	m_TTargets.EnsureVisible(m_TTargets.GetParentItem(highlighted));
	m_TTargets.EnsureVisible(highlighted);
	ShowFocus();
}

//
// Highlight the first interface of the first manager;
//
void CPageNetwork::SetFocusHome()
{
	highlighted = m_TTargets.GetChildItem(m_TTargets.GetRootItem());

	// Make sure the first manager item is visible.
	m_TTargets.EnsureVisible(m_TTargets.GetRootItem());
	m_TTargets.EnsureVisible(highlighted);
	ShowFocus();
}

//
// Highlight the last interface of the last manager with an interface.
//
void CPageNetwork::SetFocusEnd()
{
	HTREEITEM hmgr, hifc;

	// Get the last manager.
	hmgr = m_TTargets.GetRootItem();
	while (m_TTargets.GetNextSiblingItem(hmgr))
		hmgr = m_TTargets.GetNextSiblingItem(hmgr);

	// Loop through it's interfaces.
	for (hifc = m_TTargets.GetChildItem(hmgr); hifc; hifc = m_TTargets.GetNextSiblingItem(hifc)) {
		// Set the highlighted item to the last non-null interface.
		highlighted = hifc;
	}

	// Make sure the last item is visible.
	m_TTargets.EnsureVisible(highlighted);
	ShowFocus();
}

// 
// Disable collapsing the tree item when double clicking.
//
void CPageNetwork::OnDblclkTTargets(NMHDR * pNMHDR, LRESULT * pResult)
{
	*pResult = 1;
}

//
// Sets the highlighted item to the first interface.
//
void CPageNetwork::OnSetfocusTTargets(NMHDR * pNMHDR, LRESULT * pResult)
{
	if (!highlighted) {
		//Find the first interface.
		for (HTREEITEM hmgr = m_TTargets.GetRootItem(); hmgr; hmgr = m_TTargets.GetNextSiblingItem(hmgr)) {
			highlighted = m_TTargets.GetChildItem(hmgr);
			if (highlighted) {
				ShowFocus();
				break;
			}
		}
	}
	*pResult = 0;
}

//
// Shows the selected item's network settings.
//
void CPageNetwork::ShowSettings()
{
	Manager *manager;
	Worker *worker;
	int max_sends;

	// Get the values for the network settings from memory.
	switch (theApp.pView->m_pWorkerView->GetSelectedType()) {
	case WORKER:
		worker = theApp.pView->m_pWorkerView->GetSelectedWorker();
		max_sends = worker->GetMaxSends(VIServerType);
		break;
	case MANAGER:
		manager = theApp.pView->m_pWorkerView->GetSelectedManager();
		max_sends = manager->GetMaxSends(VIServerType);
		break;
	default:
		max_sends = theApp.manager_list.GetMaxSends(VIServerType);
	}

	// If there is a displayable value, display it.  Otherwise, hide the content
	// of the edit box.
	if (max_sends == AMBIGUOUS_VALUE) {
		m_EMaxSends.SetPasswordChar(32);
		m_EMaxSends.Invalidate();
	} else {
		m_EMaxSends.SetPasswordChar(0);
		SetDlgItemInt(EMaxSends, max_sends);
	}
}

//
// Updates the maximum number of sends if necessary that can be outstanding.
// Currently this only applies to VI network servers.
//
void CPageNetwork::OnKillfocusEMaxSends()
{
	Manager *manager;
	Worker *worker;

	// If max sends was left blank, use the previous value
	if (!m_EMaxSends.LineLength()) {
		ShowSettings();
	}
	// Verify that the max sends is > 0.
	else if (GetDlgItemInt(EMaxSends) <= 0) {
		ErrorMessage("Maximum # of Outstanding Sends cannot be 0.");
		ShowSettings();
		return;
	} else {
		// Seeing what kind of item is selected.
		switch (theApp.pView->m_pWorkerView->GetSelectedType()) {
		case WORKER:
			worker = theApp.pView->m_pWorkerView->GetSelectedWorker();
			worker->SetMaxSends(GetDlgItemInt(EMaxSends));
			break;
		case MANAGER:
			manager = theApp.pView->m_pWorkerView->GetSelectedManager();
			manager->SetMaxSends(GetDlgItemInt(EMaxSends));
			break;
		case ALL_MANAGERS:
			theApp.manager_list.SetMaxSends(GetDlgItemInt(EMaxSends));
			break;
		}
	}

	EnableWindow();
}
