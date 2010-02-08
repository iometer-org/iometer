/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Iometer / WaitingForManagers.cpp                                   ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: This is the Waiting for Managers dialog in Iometer.   ## */
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
/* ##  Changes ...: 2004-03-18 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Changed the "truly batch oriented" patch behaviour. ## */
/* ##               2003-10-17 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               2003-08-03 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Integrated the patch submitted by Sander Pool to    ## */
/* ##                 get Iometer "truly batch oriented".                 ## */
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
#include "IOCommon.h"
#include "WaitingForManagers.h"
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

enum { TIMER_COUNTDOWN = 1 };

// Check for an empty waiting list 2 times a second.
const int CWaitingForManagers::polling_frequency = 2;

/////////////////////////////////////////////////////////////////////////////
// CWaitingForManagers dialog

CWaitingForManagers::CWaitingForManagers()
:  CDialog(CWaitingForManagers::IDD)
{
	//{{AFX_DATA_INIT(CWaitingForManagers)
	m_TCountdown = _T("");
	//}}AFX_DATA_INIT

	Reset();
}

void CWaitingForManagers::DoDataExchange(CDataExchange * pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWaitingForManagers)
	DDX_Control(pDX, LManagers, m_LManagers);
	DDX_Text(pDX, TCountdown, m_TCountdown);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CWaitingForManagers, CDialog)
    //{{AFX_MSG_MAP(CWaitingForManagers)
ON_WM_TIMER()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
// Prepares the waiting list dialog for reuse.
//
void CWaitingForManagers::Reset()
{
	// Set the arrays to size 0.
	names.RemoveAll();
	addresses.RemoveAll();
}

//
// Overrides the base class Create function for modeless dialog boxes.
//
// If all managers on the waiting list log in successfully, the dialog will
// call back the CGalileoView::OpenConfigFile() with parameters indicating
// the file to restore (CString infilename), which options to restore
// (BOOL flags[]), and how to restore the settings (BOOL replace).  These
// options are passed in here and stored in the callback_* member
// variables for later use (when the callback occurs).
//
// If the dialog is aborted or not all managers log in, no callback will occur.
//
BOOL CWaitingForManagers::Create(const CString & infilename, BOOL * flags, BOOL replace)
{
	// Store the parameters to the callback function (CGalileoView::OpenConfigFile())
	callback_infilename = infilename;
	callback_replace = replace;
	for (int counter = 0; counter < NumICFFlags; counter++)
		callback_flags[counter] = flags[counter];

	// Create the dialog (modeless)
	if (!CDialog::Create(IDD_WAITING_FOR_MANAGERS)) {
		ErrorMessage("Couldn't create CWaitingForManagers dialog.  " "Please report this as an Iometer bug.");
		return FALSE;
	}
	// Disable the Iometer main frame
	GetParent()->EnableWindow(FALSE);
	// Enable this dialog, specifically (it was disabled by the main frame disable call)
	EnableWindow(TRUE);
	// *** Make sure the main frame is always re-enabled!

	return TRUE;
}

//
// Called to remove the OS resources (timer and window).
//
BOOL CWaitingForManagers::Close()
{
	if (!KillTimer(TIMER_COUNTDOWN)) {
		ErrorMessage("Couldn't kill the CWaitingForManagers TIMER_COUNTDOWN "
			     "timer.  Please report this as an Iometer bug.");
		return FALSE;
	}
	// Re-enable the Iometer main frame.
	GetParent()->EnableWindow(TRUE);

	if (!DestroyWindow()) {
		ErrorMessage("Couldn't close the CWaitingForManagers window.  "
			     "Please report this as an Iometer bug.");
		return FALSE;
	}

	return TRUE;
}

//
// Abort button handler (there is no Cancel button)
// Triggered when the user clicks Abort, presses Alt-F4, or closes the window.
// NOT triggered when countdown reaches zero.
//
void CWaitingForManagers::OnCancel()
{
	// The user performed some mouse or keyboard action to abort
	// the dialog, so make sure Iometer goes to interactive mode.
	theApp.OverrideBatchMode();

	(void)Close();

	// Clear the manager map and the CWaitingForManagers waiting list.
	theApp.manager_list.loadmap.Reset();
}

//
// Called when Dialog first appears.
//
BOOL CWaitingForManagers::OnInitDialog()
{
	RECT rect;
	int width;

	CDialog::OnInitDialog();

	// Initialize the countdown.
	remaining_seconds = theApp.cmdline.GetTimeout();
	UpdateCountdown();

	// Tell Windows to call OnTimer every xxxx ms to check
	// for an empty list and to decrement the countdown.
	SetTimer(TIMER_COUNTDOWN, 1000 / polling_frequency, NULL);

	// Figure out the width of the waiting manager list control
	m_LManagers.GetWindowRect(&rect);
	width = rect.right - rect.left + 1;
	width -= 5;		// necessary to avoid seeing a scrollbar (for some reason?)

	// Initialize the manager waiting list CListCtrl
	m_LManagers.InsertColumn(0, "Name", LVCFMT_LEFT, width / 2);
	m_LManagers.InsertColumn(1, "Address", LVCFMT_LEFT, width / 2, 1);

	// Update the waiting list
	DisplayManagers();

	return TRUE;		// return TRUE unless you set the focus to a control
}

//
// Timer handler -- decrements countdown every second.
//
void CWaitingForManagers::OnTimer(UINT_PTR nIDEvent)
{
	static int accumulator = 0;

	switch (nIDEvent) {
	case TIMER_COUNTDOWN:
		accumulator += 1000 / polling_frequency;

		// Has a second gone by?
		if (accumulator >= 1000) {
			accumulator -= 1000;
			remaining_seconds--;
			UpdateCountdown();
		}

		if (names.GetSize() == 0) {
			theApp.pView->OpenConfigFile(callback_infilename, callback_flags, callback_replace);
			(void)Close();

			// If we are in batch mode, start the test.
			// (In batch mode, this dialog will always appear.)
			if (theApp.IsBatchMode())
				theApp.pView->Go();
		} else if (remaining_seconds <= 0) {
			// theApp.OverrideBatchMode();

			if (theApp.IsBatchMode()) {
				exit(1);
			}

			(void)Close();

			ErrorMessage("Time expired.  Not all managers specified in the config file "
				     "were available in Iometer, so loading was aborted.");
		}
		break;
	default:
		CDialog::OnTimer(nIDEvent);
	}
}

//
// Displays the countdown notice.
//
void CWaitingForManagers::UpdateCountdown()
{
	char string[10];	// assuming no delay will be longer than 10 digits long

	_itoa(remaining_seconds, string, 10);

	m_TCountdown = "Automatically aborting in " + (CString) string + " seconds...";
	UpdateData(FALSE);	// update the GUI
}

//
// Add a manager's entry to the waiting list.
//
void CWaitingForManagers::AddWaitingManager(const CString & name, const CString & address)
{
	names.SetSize(names.GetSize() + 1);
	addresses.SetSize(addresses.GetSize() + 1);
	names[names.GetSize() - 1] = name;
	addresses[addresses.GetSize() - 1] = address;
}

//
// Remove a manager's entry from the waiting list.
//
BOOL CWaitingForManagers::RemoveWaitingManager(const CString & name, const CString & address)
{
	for (int counter = 0; counter < names.GetSize(); counter++) {
		// Do these managers match?
		if ((name.CompareNoCase(names[counter]) == 0 && address.CompareNoCase(addresses[counter]) == 0)
		    || ((addresses[counter] == "")	// "special local host" case
			&& (names[counter].Compare(HOSTNAME_LOCAL) == 0))) {
			// Remove this entry from the waiting list
			names.RemoveAt(counter);
			addresses.RemoveAt(counter);

			// It wouldn't be a good idea to put code here to test
			// for an empty waiting list and call the appropriate
			// handler, because the calling function has to do some
			// more preparation before the file can be restored.

			// Update the waiting list
			DisplayManagers();

			return TRUE;
		}
	}

	return FALSE;
}

//
// Fills in the m_LManagers CListCtrl with the contents of the name and address CArrays
//
void CWaitingForManagers::DisplayManagers()
{
	m_LManagers.LockWindowUpdate();

	// Clear the manager waiting list CListCtrl.
	m_LManagers.DeleteAllItems();

	for (int counter = 0; counter < names.GetSize(); counter++) {
		// Insert a row into the list
		m_LManagers.InsertItem(counter, NULL);

		// Fill in the manager name
		if (!m_LManagers.SetItemText(counter, 0, names[counter])) {
			ErrorMessage("Couldn't SetItemText for the name column in the CListCtrl.  "
				     "Please report this as an Iometer bug.");
			return;
		}
		// Fill in the address
		if (!m_LManagers.SetItemText(counter, 1, addresses[counter])) {
			ErrorMessage("Couldn't SetItemText for the address column in the CListCtrl.  "
				     "Please report this as an Iometer bug.");
			return;
		}
	}

	m_LManagers.UnlockWindowUpdate();
}
