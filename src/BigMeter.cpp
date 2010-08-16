/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / BigMeter.cpp                                           ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implementation of the CBigMeter class, which is       ## */
/* ##               responsible for the "Presentation Meter" dialog.      ## */
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
/* ##  Changes ...:  2003-10-17 (daniel.scheibli@edelbyte.org)            ## */
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
#include "GalileoApp.h"
#include "GalileoView.h"
#include "BigMeter.h"
#include "MeterCtrl.h"
#include "TextDisplay.h"

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
// CBigMeter dialog
CBigMeter::CBigMeter(CWnd * pParent):CDialog(CBigMeter::IDD, pParent)
{
	is_displayed = FALSE;
	SetButtonState(FALSE, FALSE, FALSE);
	max_range = 100;
}

void CBigMeter::DoDataExchange(CDataExchange * pDX)
{
	CDialog::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CBigMeter)
	DDX_Control(pDX, CkWatermark, m_CkWatermark);
	DDX_Control(pDX, BResultType, m_BResultType);
	DDX_Control(pDX, BNext, m_BNext);
	DDX_Control(pDX, BStop, m_BStop);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBigMeter, CDialog)
    //{{AFX_MSG_MAP(CBigMeter)
    ON_BN_CLICKED(BResultType, OnBResultType)
    ON_BN_CLICKED(BNext, OnBNext)
    ON_BN_CLICKED(BStop, OnBStop)
    ON_EN_KILLFOCUS(EMaxRange, OnKillfocusEMaxRange)
    ON_EN_CHANGE(EMaxRange, OnChangeEMaxRange)
    ON_WM_PAINT()
ON_BN_CLICKED(CkWatermark, OnCkWatermark)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Implementation
//
// Displays the window.  (Non-blocking (modeless) window)
//
// The performance_bar value passed is the number of the performance bar the
// big meter should replicate.  (It will show the same statistic for the same
// worker as the bar whose number was passed in.)
//
BOOL CBigMeter::Create(int performance_bar, CString test_title, CString stat_title, CString worker_title)
{
	CString text;

	// If this dialog already exists, bring it to the foreground.
	if (is_displayed == TRUE) {
		// If the window is minimized, restore it, otherwise just bring it forward
		if (IsIconic())
			VERIFY(ShowWindow(SW_RESTORE));
		else
			VERIFY(SetForegroundWindow());
	} else {
		// Create the dialog box
		if (!CDialog::Create(IDD_BIGMETER, (CWnd *) theApp.pView)) {
			ErrorMessage("Could not create dialog box to display results.");
			return FALSE;
		}
		is_displayed = TRUE;
	}

	// Record information that meter should display.
	result_name = stat_title;
	worker_name = worker_title;
	// Point to the same worker/manager/AllManagers statistics
	// the selected performance bar is monitoring
	results = theApp.pView->m_pPageDisplay->barcharts[performance_bar].results;
	result_to_display = theApp.pView->m_pPageDisplay->barcharts[performance_bar].result_to_display;

	// Set the buttons to their appropriate states
	UpdateButtons();
	m_BResultType.SetWindowText(result_name);
	m_CkWatermark.SetCheck(0);

	// Set the text display boxes.
	((CTextDisplay *) GetDlgItem(TDTestInfo))->SetAll(&test_title, "Arial", 22,
							  AlignLeft, FALSE, TRUE,
							  FALSE,::GetSysColor(COLOR_ACTIVECAPTION));
	text = worker_name + " - " + result_name;
	((CTextDisplay *) GetDlgItem(TDWhichResult))->SetAll(&text, "Arial",
							     22, AlignCenter, TRUE, FALSE,
							     FALSE,::GetSysColor(COLOR_ACTIVECAPTION));
	text = "";
	((CTextDisplay *) GetDlgItem(TDResult))->SetAll(&text, "Arial",
							54, AlignCenter, TRUE, FALSE,
							FALSE,::GetSysColor(COLOR_ACTIVECAPTION));

	SetDlgItemInt(EMaxRange, max_range);
	((CMeterCtrl *) GetDlgItem(MMeter))->SetRange(0, max_range);

	UpdateDisplay();

	return TRUE;
}

//
// Closes and destroys the window.
//
void CBigMeter::OnOK()
{
	ASSERT(is_displayed == TRUE);

	// Hide the window from the display (CDialog does this for us)
	CDialog::OnCancel();
	// Delete the toolbar and dialog box from memory
	DestroyWindow();

	is_displayed = FALSE;
}

//
// Called in response to an Alt-F4 or a click on the X button.
//
void CBigMeter::OnCancel()
{
	OnOK();
}

//
// Displays the current results data.
//
void CBigMeter::UpdateDisplay()
{
	// Retrieve the formatted statistics from the CPageDisplay tab
	theApp.pView->m_pPageDisplay->GetDisplayData(&(results[theApp.pView->m_pPageDisplay->GetWhichPerf()]),
						     result_to_display, &result_value, &result_text);

	// Set the meter's value
	((CMeterCtrl *) GetDlgItem(MMeter))->SetValue(result_value);

	// Display the result in the text display box.
	((CTextDisplay *) GetDlgItem(TDResult))->SetText(&result_text);
}

//
// Enables or disables the test control buttons as appropriate.
//
void CBigMeter::UpdateButtons()
{
	if (!is_displayed)
		return;

	if (button_state & BUTTON_START) {
		m_BNext.SetWindowText("Start");
		m_BNext.EnableWindow(TRUE);
		m_BStop.EnableWindow(FALSE);
	} else {
		if (button_state & (BUTTON_STOP | BUTTON_STOPALL)) {
			m_BNext.SetWindowText("Next >>");
			m_BNext.EnableWindow(button_state & BUTTON_STOPALL);
			m_BStop.EnableWindow(TRUE);
		} else {
			m_BNext.SetWindowText("Start");
			m_BNext.EnableWindow(FALSE);
			m_BStop.EnableWindow(FALSE);
		}
	}
}

//
// Handler which pops up a menu of results to monitor.
//
void CBigMeter::OnBResultType()
{
	CMenu menu;
	CRect button_rect;
	RECT screen_rect;
	int menu_height, menu_x, menu_y;

	// This prevents the menu from popping up if the user is "unchecking" the button.
	if (m_BResultType.GetCheck()) {
		m_BResultType.SetCheck(FALSE);
		return;
	}
	// Create the popup menu from the resource.
	VERIFY(menu.LoadMenu(IDR_POPUP_DISPLAY_LIST));
	CMenu *pPopup = menu.GetSubMenu(0);

	ASSERT(pPopup != NULL);

	// Get the screen coordinates of the button that was pressed.
	m_BResultType.GetWindowRect(&button_rect);

	// Position the menu with its upper left corner at the lower left corner of the button.
	menu_x = button_rect.TopLeft().x;
	menu_y = button_rect.BottomRight().y;

	// If the menu would go off the bottom of the screen, make it go *up* from the button instead.
	menu_height = GetSystemMetrics(SM_CYMENUSIZE) * MDisplayNumSubmenus;
	if (SystemParametersInfo(SPI_GETWORKAREA, 0, &screen_rect, 0) && (menu_y + menu_height > screen_rect.bottom)) {
		menu_y = button_rect.TopLeft().y - menu_height + 1;
	}
	// Set the button's visual state to "pressed" as long as the menu is popped up.
	m_BResultType.SetCheck(TRUE);

	// Pop up the menu, giving the main frame (CGalileoView's parent) as the parent
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, menu_x, menu_y, theApp.pView->GetParent());

	// The user's selection (if any) from the pop-up menu will result in a call to one of the 
	// CGalileoView::OnMDisplay...() functions, which will in turn call CPageDisplay::OnMDisplay().

	// Set the button's visual state to "not pressed" after the menu is dismissed.
	m_BResultType.SetState(FALSE);
}

//
// Monitors a new result, as chosen from the popup menu in OnBResultType.
// This is called by CGallileoView when the user chooses a value from that popup menu.
//
void CBigMeter::OnMDisplay(int submenu_id, int menu_item, int result_code)
{
	CString which_result;
	CMenu menu, *popup, *sub_popup;

	if ((submenu_id < 0) || (submenu_id >= MDisplayNumSubmenus)) {
		ErrorMessage("Invalid submenu_id value in CBigMeter::OnMDisplay()");
		return;
	}
	if ((result_code < 0) || (result_code >= NUM_RESULTS)) {
		ErrorMessage("Invalid result_code value in CBigMeter::OnMDisplay()");
		return;
	}
	// If the user selected the same result, keep the same information.
	if (result_to_display == result_code)
		return;

	// store the selected result for later use
	result_to_display = result_code;

	// get the selected menu item's title and store it in the selected button
	VERIFY(menu.LoadMenu(IDR_POPUP_DISPLAY_LIST));

	// we can use ASSERT here because this should work every time if 
	// it works at all
	popup = menu.GetSubMenu(0);
	ASSERT(popup != NULL);

	sub_popup = popup->GetSubMenu(submenu_id);
	if (sub_popup == NULL) {
		ErrorMessage("Could not get submenu in CBigMeter::OnMDisplay()");
		return;
	}

	sub_popup->GetMenuString(menu_item, result_name, MF_BYCOMMAND);

	// Display the new monitored value on the button
	m_BResultType.SetWindowText(result_name);
	m_BResultType.SetCheck(FALSE);

	// Update the edit box displaying which result is being shown.
	which_result = worker_name + " - " + result_name;
	((CTextDisplay *) GetDlgItem(TDWhichResult))->SetText(&which_result);

	((CMeterCtrl *) GetDlgItem(MMeter))->ResetWatermark();
	UpdateDisplay();
}

//
// This is called by CGalileoView whenever the main frame's toolbar button states
// are set.  The three parameters here are the states of the "Start", "Stop",
// and "Stop All" toolbar buttons.  These button states are used to determine the
// states of CBigMeter's test control buttons.
//
void CBigMeter::SetButtonState(BOOL start, BOOL stop, BOOL stopall)
{
	button_state = 0;

	if (start)
		button_state |= BUTTON_START;
	if (stop)
		button_state |= BUTTON_STOP;
	if (stopall)
		button_state |= BUTTON_STOPALL;

	UpdateButtons();
}

//
// Called when user clicks the "Next>>" (sometimes labeled "Start") button.
//
// If the tests have not been started, this button is a START button.
// If the tests have started, this button uses the main toolbar's STOP
// button to continue to the next test, and it is therefore labeled "Next>>".
//
void CBigMeter::OnBNext()
{
	if (button_state & BUTTON_START) {
		((CMeterCtrl *) GetDlgItem(MMeter))->ResetWatermark();
		theApp.pView->Go();
	} else {
		theApp.pView->StopTest(ReturnSuccess);
	}
}

//
// Stops all tests.
//
// If tests remain after the current one, this is the same as clicking STOP ALL.
// If only running one test, this is the same as clicking STOP.
//
void CBigMeter::OnBStop()
{
	if (button_state & BUTTON_STOPALL)
		theApp.pView->StopAll();
	else
		theApp.pView->StopTest(ReturnSuccess);
}

//
// Prevents the user from leaving the max range edit box blank.
//
void CBigMeter::OnKillfocusEMaxRange()
{
	int new_range;

	// Update the maximum range, but disallow a blank or zero range.
	if (!(new_range = GetDlgItemInt(EMaxRange))) {
		SetDlgItemInt(EMaxRange, max_range);
		return;
	}

	SetMaxRange(new_range);
}

//
// Updates the meter's range as the user changes the max range edit box value.
//
void CBigMeter::OnChangeEMaxRange()
{
	int new_range;

	// Only set the range if the displayed value is not zero or blank.
	if (new_range = GetDlgItemInt(EMaxRange))
		SetMaxRange(new_range);
}

//
// Sets a new maximum range for the display.
//
void CBigMeter::SetMaxRange(int new_range)
{
	// If the range is the same as the old one, we don't need to update it.
	if (new_range == max_range)
		return;

	max_range = new_range;
	((CMeterCtrl *) GetDlgItem(MMeter))->SetRange(0, max_range);
}

//
// Called to uncheck the result selection button when the popup menu is dismissed.
//
void CBigMeter::ClearCheckButton()
{
	CRect button_rect;
	CPoint point;

	// When displaying the popup menu, if the user clicks outside the popup and
	// not on the button that poped the popup up, Windows handles removing the
	// menu.  We have to handle unchecking the button.

	m_BResultType.GetWindowRect(&button_rect);
	GetCursorPos(&point);

	// Note that we only reset the state of the check button if the cursor is
	// not within the button's bounding rectangle.  If the cursor is within
	// the bounding rectangle, the user clicked the button a second time.  The
	// Button handler will take care of unchecking the button, and will not
	// display the menu.  This allows the button to toggle the menu.

	if (m_BResultType.GetCheck() && !button_rect.PtInRect(point))
		m_BResultType.SetCheck(FALSE);
}

//
// Called when the user clicks on the watermark check box.  This turns the
// watermark off and on.
//
void CBigMeter::OnCkWatermark()
{
	((CMeterCtrl *) GetDlgItem(MMeter))->show_watermark = m_CkWatermark.GetCheck();
	((CMeterCtrl *) GetDlgItem(MMeter))->ResetWatermark();
}
