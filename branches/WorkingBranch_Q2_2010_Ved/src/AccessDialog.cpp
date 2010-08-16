/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / AccessDialog.cpp                                       ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: This file is responsible for allowing the access      ## */
/* ##               specifications to be edited (the little dialog)       ## */
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
/* ##               2003-03-02 (joe@eiler.net)                            ## */
/* ##               - Changed LONG_PTR to ULONG_PTR, which is what it is  ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include "StdAfx.h"
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
// CAccessDialog dialog

CAccessDialog::CAccessDialog(Test_Spec * edit_spec, CWnd * pParent)
:  CDialog(CAccessDialog::IDD, pParent)
{
	spec = edit_spec;
	//{{AFX_DATA_INIT(CAccessDialog)
	access_txt = _T("");
	random_txt = _T("");
	read_txt = _T("");
	sequential_txt = _T("");
	transfer_txt = _T("");
	write_txt = _T("");
	//}}AFX_DATA_INIT
}

void CAccessDialog::DoDataExchange(CDataExchange * pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAccessDialog)
	DDX_Control(pDX, RReplySize, m_RReplySize);
	DDX_Control(pDX, RNoReply, m_RNoReply);
	DDX_Control(pDX, RAlignSector, m_RAlignSector);
	DDX_Control(pDX, RAlignBytes, m_RAlignBytes);
	DDX_Control(pDX, SReplyMegabytes, m_SReplyMegabytes);
	DDX_Control(pDX, SReplyKilobytes, m_SReplyKilobytes);
	DDX_Control(pDX, SReplyBytes, m_SReplyBytes);
	DDX_Control(pDX, EReplyMegabytes, m_EReplyMegabytes);
	DDX_Control(pDX, EReplyKilobytes, m_EReplyKilobytes);
	DDX_Control(pDX, EReplyBytes, m_EReplyBytes);
	DDX_Control(pDX, SAlignMegabytes, m_SAlignMegabytes);
	DDX_Control(pDX, SAlignKilobytes, m_SAlignKilobytes);
	DDX_Control(pDX, SAlignBytes, m_SAlignBytes);
	DDX_Control(pDX, EAlignMegabytes, m_EAlignMegabytes);
	DDX_Control(pDX, EAlignKilobytes, m_EAlignKilobytes);
	DDX_Control(pDX, EAlignBytes, m_EAlignBytes);
	DDX_Control(pDX, EBytes, m_EBytes);
	DDX_Control(pDX, EMegabytes, m_EMegabytes);
	DDX_Control(pDX, EKilobytes, m_EKilobytes);
	DDX_Control(pDX, SMegabytes, m_SMegabytes);
	DDX_Control(pDX, SKilobytes, m_SKilobytes);
	DDX_Control(pDX, SBytes, m_SBytes);
	DDX_Control(pDX, CDefaultAssignment, m_CDefaultAssignment);
	DDX_Control(pDX, IDOK, m_BOk);
	DDX_Control(pDX, EName, m_EName);
	DDX_Control(pDX, SRead, m_SRead);
	DDX_Control(pDX, SRandom, m_SRandom);
	DDX_Control(pDX, SAccess, m_SAccess);
	DDX_Control(pDX, LAccess, m_LAccess);
	DDX_Control(pDX, EDelayTime, m_EDelayTime);
	DDX_Control(pDX, EBurstLength, m_EBurstLength);
	DDX_Control(pDX, BInsertBefore, m_BInsertBefore);
	DDX_Control(pDX, BInsertAfter, m_BInsertAfter);
	DDX_Control(pDX, BDelete, m_BDelete);
	DDX_Text(pDX, TAccess, access_txt);
	DDX_Text(pDX, TRandom, random_txt);
	DDX_Text(pDX, TRead, read_txt);
	DDX_Text(pDX, TSequential, sequential_txt);
	DDX_Text(pDX, TWrite, write_txt);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAccessDialog, CDialog)
    //{{AFX_MSG_MAP(CAccessDialog)
    ON_BN_CLICKED(BDelete, OnBDelete)
    ON_BN_CLICKED(BInsertAfter, OnBInsertAfter)
    ON_BN_CLICKED(BInsertBefore, OnBInsertBefore)
    ON_WM_HSCROLL()
    ON_EN_KILLFOCUS(EBytes, OnKillfocusESizes)
    ON_EN_KILLFOCUS(EAlignBytes, OnKillfocusEAligns)
    ON_NOTIFY(UDN_DELTAPOS, SBytes, OnDeltaposSSizes)
    ON_NOTIFY(UDN_DELTAPOS, SAlignBytes, OnDeltaposSAligns)
    ON_BN_CLICKED(RAlignSector, OnRAlignSector)
    ON_BN_CLICKED(RAlignBytes, OnRAlignBytes)
    ON_EN_KILLFOCUS(EBurstLength, OnChangeBurst)
    ON_EN_KILLFOCUS(EDelayTime, OnChangeDelay)
    ON_NOTIFY(UDN_DELTAPOS, SReplyBytes, OnDeltaposSReply)
    ON_EN_KILLFOCUS(EReplyBytes, OnKillfocusEReply)
    ON_BN_CLICKED(RNoReply, OnRNoReply)
    ON_BN_CLICKED(RReplySize, OnRReplySize)
    ON_NOTIFY(NM_CLICK, LAccess, OnChangeLAccess)
    ON_EN_KILLFOCUS(EKilobytes, OnKillfocusESizes)
    ON_EN_KILLFOCUS(EMegabytes, OnKillfocusESizes)
    ON_NOTIFY(UDN_DELTAPOS, SKilobytes, OnDeltaposSSizes)
    ON_NOTIFY(UDN_DELTAPOS, SMegabytes, OnDeltaposSSizes)
    ON_EN_KILLFOCUS(EAlignKilobytes, OnKillfocusEAligns)
    ON_EN_KILLFOCUS(EAlignMegabytes, OnKillfocusEAligns)
    ON_NOTIFY(UDN_DELTAPOS, SAlignKilobytes, OnDeltaposSAligns)
    ON_NOTIFY(UDN_DELTAPOS, SAlignMegabytes, OnDeltaposSAligns)
    ON_NOTIFY(UDN_DELTAPOS, SReplyKilobytes, OnDeltaposSReply)
    ON_NOTIFY(UDN_DELTAPOS, SReplyMegabytes, OnDeltaposSReply)
    ON_EN_KILLFOCUS(EReplyKilobytes, OnKillfocusEReply)
    ON_EN_KILLFOCUS(EReplyMegabytes, OnKillfocusEReply)
    ON_NOTIFY(NM_DBLCLK, LAccess, OnChangeLAccess)
ON_NOTIFY(LVN_ITEMCHANGED, LAccess, OnChangeLAccess)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CAccessDialog message handlers
BOOL CAccessDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_SMegabytes.SetRange(0, MAX_SIZE_RANGE);
	m_SKilobytes.SetRange(0, MAX_SIZE_RANGE);
	m_SBytes.SetRange(0, MAX_SIZE_RANGE);
	m_SAlignMegabytes.SetRange(0, MAX_SIZE_RANGE);
	m_SAlignKilobytes.SetRange(0, MAX_SIZE_RANGE);
	m_SAlignBytes.SetRange(0, MAX_SIZE_RANGE);
	m_SAlignBytes.SetPos(512);
	m_SReplyMegabytes.SetRange(0, MAX_SIZE_RANGE);
	m_SReplyKilobytes.SetRange(0, MAX_SIZE_RANGE);
	m_SReplyBytes.SetRange(0, MAX_SIZE_RANGE);

	m_SAccess.SetRange(1, 100);
	m_SAccess.SetPageSize(10);
	m_SRead.SetRange(0, 100);
	m_SRead.SetPageSize(10);
	m_SRandom.SetRange(0, 100);
	m_SRandom.SetPageSize(10);

	size_controls.EBytesCtrl = &m_EBytes;
	size_controls.EKilobytesCtrl = &m_EKilobytes;
	size_controls.EMegabytesCtrl = &m_EMegabytes;
	size_controls.EBytesID = EBytes;
	size_controls.EKilobytesID = EKilobytes;
	size_controls.EMegabytesID = EMegabytes;
	size_controls.SBytesCtrl = &m_SBytes;
	size_controls.SKilobytesCtrl = &m_SKilobytes;
	size_controls.SMegabytesCtrl = &m_SMegabytes;
	size_controls.SBytesID = SBytes;
	size_controls.SKilobytesID = SKilobytes;
	size_controls.SMegabytesID = SMegabytes;
	size_controls.RSelectedID = 0;
	size_controls.RNotSelectedID = 0;

	// 2462 SDK compiler didn't like the "this->" for Win64 so we'll just remove it
	//  since it really isn't needed anyway...
	//size_controls.GetFunc                 = this->GetSize;
	//size_controls.SetFunc                 = this->SetSize;
	size_controls.GetFunc = &CAccessDialog::GetSize;
	size_controls.SetFunc = &CAccessDialog::SetSize;

	align_controls.EBytesCtrl = &m_EAlignBytes;
	align_controls.EKilobytesCtrl = &m_EAlignKilobytes;
	align_controls.EMegabytesCtrl = &m_EAlignMegabytes;
	align_controls.EBytesID = EAlignBytes;
	align_controls.EKilobytesID = EAlignKilobytes;
	align_controls.EMegabytesID = EAlignMegabytes;
	align_controls.SBytesCtrl = &m_SAlignBytes;
	align_controls.SKilobytesCtrl = &m_SAlignKilobytes;
	align_controls.SMegabytesCtrl = &m_SAlignMegabytes;
	align_controls.SBytesID = SAlignBytes;
	align_controls.SKilobytesID = SAlignKilobytes;
	align_controls.SMegabytesID = SAlignMegabytes;
	align_controls.RSelectedID = RAlignBytes;
	align_controls.RNotSelectedID = RAlignSector;

	// 2462 SDK compiler didn't like the "this->" for Win64 so we'll just remove it
	//  since it really isn't needed anyway...
	//align_controls.GetFunc                        = this->GetAlign;
	//align_controls.SetFunc                        = this->SetAlign;
	align_controls.GetFunc = &CAccessDialog::GetAlign;
	align_controls.SetFunc = &CAccessDialog::SetAlign;

// RSelectedID MUST be greater than RNotSelectedID, for the sake of GetCheckedRadioButton().
#if !(RAlignBytes > RAlignSector)
#error RAlignBytes is not greater than RAlignSector in resource.h!
#endif

	reply_controls.EBytesCtrl = &m_EReplyBytes;
	reply_controls.EKilobytesCtrl = &m_EReplyKilobytes;
	reply_controls.EMegabytesCtrl = &m_EReplyMegabytes;
	reply_controls.EBytesID = EReplyBytes;
	reply_controls.EKilobytesID = EReplyKilobytes;
	reply_controls.EMegabytesID = EReplyMegabytes;
	reply_controls.SBytesCtrl = &m_SReplyBytes;
	reply_controls.SKilobytesCtrl = &m_SReplyKilobytes;
	reply_controls.SMegabytesCtrl = &m_SReplyMegabytes;
	reply_controls.SBytesID = SReplyBytes;
	reply_controls.SKilobytesID = SReplyKilobytes;
	reply_controls.SMegabytesID = SReplyMegabytes;
	reply_controls.RSelectedID = RReplySize;
	reply_controls.RNotSelectedID = RNoReply;

	// 2462 SDK compiler didn't like the "this->" for Win64 so we'll just remove it
	//  since it really isn't needed anyway...
	//reply_controls.GetFunc                        = this->GetReply;
	//reply_controls.SetFunc                        = this->SetReply;
	reply_controls.GetFunc = &CAccessDialog::GetReply;
	reply_controls.SetFunc = &CAccessDialog::SetReply;

// RSelectedID MUST be greater than RNotSelectedID, for the sake of GetCheckedRadioButton().
#if !(RReplySize > RNoReply)
#error RReplySize is not greater than RNoReply in resource.h!
#endif

	UpdateData(FALSE);

	// Initialize the listbox object.
	InitializeList();

	// Get listbox display data from AccessSpecList.
	LoadList();

	// If there are any entries in the list, highlight the first one.
	if (m_LAccess.GetItemCount()) {
		item_being_changed = 0;
		m_LAccess.SetItemState(m_LAccess.GetItemCount() - 1, !LVIS_SELECTED, LVIS_SELECTED);
		m_LAccess.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
		SetAll((Access_Spec *) m_LAccess.GetItemData(0));
	}

	UpdateData(TRUE);

	return TRUE;		// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//
// Converting a given size (I/O size or byte alignment size) in bytes to a string.
//
void CAccessDialog::SizeToText(DWORD size, CString * size_text)
{
	// Breaking the size down into MBs, KBs, and bytes.
	size_text->Format("%4dMB %4dKB %4dB", size / MEGABYTE_BIN, (size % MEGABYTE_BIN) / KILOBYTE_BIN, size % KILOBYTE_BIN);
}

//
// Get the value of a specified Megabytes/Kilobytes/Bytes control for the currently selected 
// access spec, as shown by the edit boxes (MB, KB, bytes).
//
DWORD CAccessDialog::GetMKBEditbox(MKBControls * which)
{
	// If any edit boxes are blank, set their sliders to the appropriate current value 
	// (which, in turn, sets the edit box).
	if (!which->EBytesCtrl->LineLength())
		which->SBytesCtrl->SetPos((this->*(which->GetFunc)) () % KILOBYTE_BIN);

	if (!which->EKilobytesCtrl->LineLength())
		which->SKilobytesCtrl->SetPos(((this->*(which->GetFunc)) () % MEGABYTE_BIN) / KILOBYTE_BIN);

	if (!which->EMegabytesCtrl->LineLength())
		which->SMegabytesCtrl->SetPos((this->*(which->GetFunc)) () / MEGABYTE_BIN);

	// Verify that the megabyte value has not overflowed.
	if (GetDlgItemInt(which->EMegabytesID) > MAX_SIZE_RANGE) {
		which->SMegabytesCtrl->SetPos(MAX_SIZE_RANGE);
		ErrorMessage("The maximum value supported by Iometer for this control "
			     "is 1023 MB + 1023 KB + 1023 bytes.");
	}

	return GetDlgItemInt(which->EBytesID)
	    + GetDlgItemInt(which->EKilobytesID) * KILOBYTE_BIN + GetDlgItemInt(which->EMegabytesID) * MEGABYTE_BIN;
}

//
// Get the value of a specified Megabytes/Kilobytes/Bytes control for the currently selected 
// access spec, as shown by the spinner controls.
//
DWORD CAccessDialog::GetMKBSpinners(MKBControls * which)
{
	return (which->SBytesCtrl->GetPos() +
		which->SKilobytesCtrl->GetPos() * KILOBYTE_BIN + which->SMegabytesCtrl->GetPos() * MEGABYTE_BIN);
}

//
// Enable or disable a specified Megabytes/Kilobytes/Bytes control.
//
void CAccessDialog::EnableMKBControls(MKBControls * which, BOOL enabled)
{
	if (which->RSelectedID && which->RNotSelectedID) {
		CheckRadioButton(which->RNotSelectedID, which->RSelectedID,
				 (enabled ? which->RSelectedID : which->RNotSelectedID));
	}

	which->EMegabytesCtrl->EnableWindow(enabled);
	which->EKilobytesCtrl->EnableWindow(enabled);
	which->EBytesCtrl->EnableWindow(enabled);
	which->SMegabytesCtrl->EnableWindow(enabled);
	which->SKilobytesCtrl->EnableWindow(enabled);
	which->SBytesCtrl->EnableWindow(enabled);
}

//
// Update the size of the current access specification line when any of
// the size edit boxes have lost the focus.
//
void CAccessDialog::OnKillfocusESizes()
{
	SetSize(GetMKBEditbox(&size_controls));
}

//
// Update the byte alignment of the current access specification line when any of
// the alignment edit boxes have lost the focus.
//
void CAccessDialog::OnKillfocusEAligns()
{
	if (GetCheckedRadioButton(RAlignSector, RAlignBytes) == RAlignBytes)
		SetAlign(GetMKBEditbox(&align_controls));
}

//
// Update the reply size of the current access specification line when any of
// the reply edit boxes have lost the focus.
//
void CAccessDialog::OnKillfocusEReply()
{
	if (GetCheckedRadioButton(RNoReply, RReplySize) == RReplySize)
		SetReply(GetMKBEditbox(&reply_controls));
}

//
// The user has incremented/decremented the access spec size by using
// one of the spinner controls.
//
void CAccessDialog::OnDeltaposSSizes(NMHDR * pNMHDR, LRESULT * pResult)
{
	OnDeltaposMKB(&size_controls, pNMHDR);
	*pResult = TRUE;	// Manually process the spinner movements.
}

//
// The user has incremented/decremented the access spec byte alignment by using
// one of the spinner controls.
//
void CAccessDialog::OnDeltaposSAligns(NMHDR * pNMHDR, LRESULT * pResult)
{
	OnDeltaposMKB(&align_controls, pNMHDR);
	*pResult = TRUE;	// Manually process the spinner movements.
}

//
// The user has incremented/decremented the access reply size by using
// one of the spinner controls.
//
void CAccessDialog::OnDeltaposSReply(NMHDR * pNMHDR, LRESULT * pResult)
{
	OnDeltaposMKB(&reply_controls, pNMHDR);
	*pResult = TRUE;	// Manually process the spinner movements.
}

//
// Handle event when the user has incremented/decremented a Megabytes/Kilobytes/Bytes 
// control by using one of the spinner controls.
//
void CAccessDialog::OnDeltaposMKB(MKBControls * which, NMHDR * pNMHDR)
{
	NM_UPDOWN *pNMUpDown = (NM_UPDOWN *) pNMHDR;
	DWORD edited_size;

	// Do not process spinner change if the RSelectedID radio button is specified and is not selected.
	if (which->RSelectedID && which->RNotSelectedID &&
	    (UINT) GetCheckedRadioButton(which->RNotSelectedID, which->RSelectedID)
	    != which->RSelectedID) {
		return;
	}
	// See if the user edited the value directly, then pressed the spin
	// control.  If so, force an update of the edited value before processing
	// the up/down spin.
	edited_size = GetMKBEditbox(which);
	if (GetMKBSpinners(which) != edited_size) {
		SetMKBSpinners(which, edited_size);
	}
	// Adjust the spinner positions based on their movement.
	if (pNMHDR->idFrom == which->SBytesID) {
		which->SBytesCtrl->SetPos(which->SBytesCtrl->GetPos() + pNMUpDown->iDelta);
	} else if (pNMHDR->idFrom == which->SKilobytesID) {
		which->SKilobytesCtrl->SetPos(which->SKilobytesCtrl->GetPos() + pNMUpDown->iDelta);
	} else if (pNMHDR->idFrom == which->SMegabytesID) {
		which->SMegabytesCtrl->SetPos(which->SMegabytesCtrl->GetPos() + pNMUpDown->iDelta);
	} else {
		ErrorMessage("Unexpected value for idFrom in CAccessDialog::OnDeltaposMKB");
	}

	// Save the new size.
	(this->*(which->SetFunc)) (GetMKBSpinners(which));
}

//
// Insert a new line into the access spec after or before the current one.
//
void CAccessDialog::InsertWrapper(BOOL after)
{
	int current_line;
	Access_Spec *spec = NULL;

	if (!(spec = new Access_Spec)) {
		ErrorMessage("Unable to allocate memory to insert new access " "spec line.");
		return;
	}
	// Get the first selected line.
	if (item_being_changed == -1) {
		// Nothing is selected. Create a default entry.
		item_being_changed = 0;
		spec->of_size = 100;
		spec->reads = 67;
		spec->random = 100;
		spec->delay = 0;
		spec->burst = 1;
		spec->align = 0;
		spec->reply = 0;
		spec->size = 2048;
	} else {
		// If we're inserting after the current selection, we need to find the
		// last selected line.
		current_line = m_LAccess.GetNextItem(-1, LVNI_SELECTED);
		if (after) {
			// Find the last selected line number.
			while (current_line != -1) {
				item_being_changed = current_line;
				current_line = m_LAccess.GetNextItem(current_line, LVNI_SELECTED);
			}

			// Get the last selected line's information.
			GetAll(spec);
			item_being_changed++;
		} else {
			// We're inserting before the first selected line.
			if (current_line != -1)
				item_being_changed = current_line;
			GetAll(spec);
		}
	}

	// Insert the new line.
	InsertLine(spec, item_being_changed);
}

//
// Inserting a new line into the access spec.
//
void CAccessDialog::OnBInsertAfter()
{
	InsertWrapper(TRUE);
}

void CAccessDialog::OnBInsertBefore()
{
	InsertWrapper(FALSE);
}

//
// A change has occurred in the access spec list.  Update the GUI
// to reflect the current selection, if necessary.
//
void CAccessDialog::OnChangeLAccess(NMHDR * pNMHDR, LRESULT * pResult)
{
	// If no item is selected or the same item was reselected, ignore the action.
	int new_selection = m_LAccess.GetNextItem(-1, LVNI_SELECTED);

	if ((new_selection == -1) || (new_selection == item_being_changed))
		return;

	// Updating the last selected access spec line before possibly selecting
	// a new line.  This is needed or the newly selected item will receive the
	// same update as the previously selected item.
	if (item_being_changed != -1) {
		// Force all the other controls' OnKillfocus handlers to be called
		m_LAccess.SetFocus();
	}
	// Determine if a new item has been selected.  If not, keep the
	// currently selected item with the focus.
	if (m_LAccess.GetNextItem(-1, LVNI_SELECTED) != -1)
		item_being_changed = m_LAccess.GetNextItem(-1, LVNI_SELECTED);

	m_LAccess.SetItemState(item_being_changed, LVIS_SELECTED, LVIS_SELECTED);
	SetAll((Access_Spec *) m_LAccess.GetItemData(item_being_changed));

	*pResult = 0;
}

//
// Checks for the validity of the entered access spec.
//
BOOL CAccessDialog::CheckAccess()
{
	int line_index;
	int total_access = 0;
	CString spec_name;
	Test_Spec *name_spec;

	// There must be no more than 100 entries, the sum of the "% of Access" 
	// fields for all the entries must equal 100, and all sizes must be > 0.
	for (line_index = 0; line_index < m_LAccess.GetItemCount(); line_index++) {
		item_being_changed = line_index;
		if (GetSize() <= 0) {
			ErrorMessage("A line in the access specification is for 0 bytes.  "
				     "All sizes must be greater than 0.");
			return FALSE;
		}
		total_access += GetAccess();
	}
	if (total_access != 100) {
		ErrorMessage("Percent of Access Specification values must sum to exactly 100.");
		return FALSE;
	}
	// Update the spec's name with the name in the edit box.
	m_EName.GetWindowText(spec_name.GetBuffer(MAX_WORKER_NAME), MAX_WORKER_NAME);
	spec_name.ReleaseBuffer();
	spec_name.TrimLeft();	//remove leading & trailing whitespace (bug #363)
	spec_name.TrimRight();
	m_EName.SetWindowText(spec_name);	//update edit control

	// Check for a blank name
	if (spec_name.IsEmpty()) {
		ErrorMessage("You must assign a name to this access specification.");
		return FALSE;
	}
	// Check for commas in the name.  Commas invalidate the results file, which is comma separated.
	if (spec_name.Find(',') != -1) {
		ErrorMessage("Commas are not allowed in access specification names.");
		return FALSE;
	}
	// Check for duplicate names.
	name_spec = theApp.access_spec_list.RefByName((LPCTSTR) spec_name);
	if (name_spec && name_spec != spec) {
		ErrorMessage("An access specification named \"" + spec_name + "\" already exists.  " +
			     "Access specification names must be unique.");
		return FALSE;
	}

	return TRUE;
}

//
// The user has moved a scroll bar on the GUI.  Process the update.
// Typically, this means that the user has changed one of the following
// values: % reads, % random, % of size, etc.
//
void CAccessDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar * pScrollBar)
{
	int position = ((CSliderCtrl *) pScrollBar)->GetPos();

	// Modifying access specification based on slider controls.
	if (nSBCode == SB_THUMBTRACK || nSBCode == SB_ENDSCROLL) {
		if (pScrollBar == (CScrollBar *) & m_SAccess) {
			SetAccess(position);
		} else if (pScrollBar == (CScrollBar *) & m_SRead) {
			SetReads(position);
		} else if (pScrollBar == (CScrollBar *) & m_SRandom) {
			SetRandom(position);
		}
	}
}

//
// The user clicked OK.
// Save the user's changes to the access spec.
//
void CAccessDialog::OnOK()
{
	int line_index;

	// Set the focus to the OK button, to force any appropriate Killfocus handler to run
	// (handles the case that the user modifies a control and then hits ENTER).
	m_BOk.SetFocus();

	if (SaveList()) {
		// Delete all working copies of Access_Spec list items.
		for (line_index = 0; line_index < m_LAccess.GetItemCount(); line_index++)
			delete(Access_Spec *) m_LAccess.GetItemData(line_index);

		CDialog::OnOK();
	}
}

//
// User selected sector alignment radio button - disable byte align edit box, other stuff
//
void CAccessDialog::OnRAlignSector()
{
	// Get the current alignment setting.
	DWORD old_align = GetAlign();

	// Disable the alignment controls.
	EnableMKBControls(&align_controls, FALSE);
	// Set the alignment in the access spec to "sector".
	SetAlign(0);
	// Set the (disabled) alignment controls to the last value shown.
	SetMKBSpinners(&align_controls, old_align);
}

//
// User selected byte alignment radio button - enable byte align edit box, other stuff
//
void CAccessDialog::OnRAlignBytes()
{
	// Enable the alignment controls.
	EnableMKBControls(&align_controls, TRUE);
	// Set the alignment in the access spec to the value shown in the alignment controls.
	SetAlign(GetMKBSpinners(&align_controls));
}

//
// User selected "No Reply" radio button - disable reply size edit boxes, other stuff
//
void CAccessDialog::OnRNoReply()
{
	// Get the current reply size.
	DWORD old_reply = GetReply();

	// Disable the reply controls.
	EnableMKBControls(&reply_controls, FALSE);
	// Set the reply size in the access spec to "none".
	SetReply(0);
	// Set the (disabled) reply size controls to the last value shown.
	SetMKBSpinners(&reply_controls, old_reply);
}

//
// User selected reply size radio button - enable reply size edit boxes, other stuff
//
void CAccessDialog::OnRReplySize()
{
	// Enable the reply size controls.
	EnableMKBControls(&reply_controls, TRUE);
	// Set the reply size in the access spec to the value shown in the reply size controls.
	SetReply(GetMKBSpinners(&reply_controls));
}

//
// Setting the controls that display the size of the access spec.
//
void CAccessDialog::SetMKBSpinners(MKBControls * which, DWORD new_value)
{
	// The byte alignment and reply size spinners should never show zero.
	if (((which == &align_controls) || (which == &reply_controls)) && new_value == 0) {
		return;
	}
	// Updating the display.  Since the spin controls are linked to the
	// edit boxes, we only need to update the spinners.
	if (GetFocus() != which->EBytesCtrl)
		which->SBytesCtrl->SetPos(new_value % KILOBYTE_BIN);
	if (GetFocus() != which->EKilobytesCtrl)
		which->SKilobytesCtrl->SetPos((new_value % MEGABYTE_BIN) / KILOBYTE_BIN);
	if (GetFocus() != which->EMegabytesCtrl)
		which->SMegabytesCtrl->SetPos(new_value / MEGABYTE_BIN);
}

//
// Initialize the listbox.
//
void CAccessDialog::InitializeList()
{
	// Set the list control column names and widths
	m_LAccess.ModifyStyle(NULL, LVS_SHOWSELALWAYS);

	m_LAccess.InsertColumn(LAccessSize, "Size", LVCFMT_CENTER, 121);
	m_LAccess.InsertColumn(LAccessOfSize, "% Access", LVCFMT_CENTER, 58, LAccessOfSize);
	m_LAccess.InsertColumn(LAccessReads, "% Read", LVCFMT_CENTER, 49, LAccessReads);
	m_LAccess.InsertColumn(LAccessRandom, "% Random", LVCFMT_CENTER, 63, LAccessRandom);
	m_LAccess.InsertColumn(LAccessDelay, "Delay", LVCFMT_CENTER, 43, LAccessDelay);
	m_LAccess.InsertColumn(LAccessBurst, "Burst", LVCFMT_CENTER, 36, LAccessBurst);
	m_LAccess.InsertColumn(LAccessAlignment, "Alignment", LVCFMT_CENTER, 114, LAccessAlignment);
	m_LAccess.InsertColumn(LAccessReply, "Reply", LVCFMT_CENTER, 114, LAccessReply);
}

//
// Loads the access spec lines from the application's test spec structure
// into an MFC list control and into working space in memory.  By using working
// space, we allow the user to abort all changes by hitting cancel.
//
void CAccessDialog::LoadList()
{
	int spec_line_no = 0;
	Access_Spec *new_spec;

	// Abort if we encounter an error.
	if (spec == NULL) {
		ErrorMessage("Unexpectedly found spec == NULL in CAccessDialog::LoadList()");
		CDialog::OnOK();
		return;
	}
	// Clear display before loading accesses.
	m_LAccess.DeleteAllItems();

	// Update the name edit box with the spec's name.
	m_EName.SetWindowText(spec->name);

	// Update the default assignment settings.
	m_CDefaultAssignment.SetCurSel(spec->default_assignment);

	// Fill in the access spec list.
	// An "of_size" of IOERROR indicates the end of the list if the list isn't full.
	item_being_changed = 0;
	while (spec->access[spec_line_no].of_size != IOERROR && item_being_changed < MAX_ACCESS_SPECS) {
		// Allocate memory for a new access spec line.
		if (!(new_spec = new Access_Spec)) {
			ErrorMessage("Unable to allocate memory to insert new access " "spec line.");
			return;
		}
		// Add the new access spec line to the display.
		memcpy((void *)new_spec, (void *)&(spec->access[spec_line_no++]), sizeof(Access_Spec));
		if (InsertLine(new_spec, item_being_changed)) {
			item_being_changed++;
			new_spec = NULL;
		} else {
			// Remove the memory for the new spec.  It was not used.
			// (This should rarely happen.)
			delete new_spec;
		}
	}

	// The last line successfully inserted will be the one available for 
	// editing.  This line number is one less than where we tried to insert at.
	--item_being_changed;

	// If there are no entries in the access spec, disable the OK button.  This 
	// avoids the error message about incomplete access specs for this special 
	// case.
	if (item_being_changed < 0)
		m_BOk.EnableWindow(FALSE);

	m_LAccess.SetFocus();
}

//
// Copies the working access spec to a spot in the application's test spec structure.
// (Saves the changes the user has made to the access spec.)
//
BOOL CAccessDialog::SaveList()
{
	int line_index;

	// Abort if we encounter an error.
	if (spec == NULL) {
		ErrorMessage("Unexpectedly found spec == NULL in CAccessDialog::SaveList()");
		CDialog::OnOK();
	}
	// Check to make sure the spec is filled in correctly.
	if (!CheckAccess())
		return FALSE;

	// Update the spec's name with the name edit box's contents.
	m_EName.GetWindowText(spec->name, MAX_WORKER_NAME);

	// Store the default assignment setting.
	spec->default_assignment = m_CDefaultAssignment.GetCurSel();

	// Fill in the access spec from the entries in the GUI list.
	for (line_index = 0; line_index < m_LAccess.GetItemCount(); line_index++) {
		item_being_changed = line_index;
		GetAll(&(spec->access[line_index]));
	}

	// If all the spec's lines have not been filled, mark
	// the last one as the end.
	if (line_index < MAX_ACCESS_SPECS)
		spec->access[line_index].of_size = IOERROR;

	// Indicate that the access was saved properly.
	return TRUE;
}

//
// Inserting a line into the access specification list.
//
BOOL CAccessDialog::InsertLine(Access_Spec * access_spec, int line_no)
{
	int current_line;

	// Verify that we can handle a new line into the access spec.
	if (m_LAccess.GetItemCount() == MAX_ACCESS_SPECS) {
		ErrorMessage("Can not add another entry to the access spec: the access spec is full");
		return FALSE;
	}
	// Unselect all selected lines to select only the new line.
	current_line = m_LAccess.GetNextItem(-1, LVNI_SELECTED);
	while (current_line != -1) {
		m_LAccess.SetItemState(current_line, !LVIS_SELECTED, LVIS_SELECTED);
		current_line = m_LAccess.GetNextItem(current_line, LVNI_SELECTED);
	}

	// Insert the new spec line and mark it as selected.
	item_being_changed = line_no;
	m_LAccess.InsertItem(LVIF_IMAGE | LVIF_STATE | LVIF_PARAM,
			     item_being_changed, NULL, LVIS_SELECTED | LVIS_FOCUSED,
			     LVIS_SELECTED | LVIS_FOCUSED, 1, (ULONG_PTR) access_spec);

	// Update the controls to reflect the newly inserted line.
	SetAll(access_spec);

	m_LAccess.EnsureVisible(item_being_changed, FALSE);

	// Re-enable all of the controls if this is the first item added to the list.
	if (m_LAccess.GetItemCount() == 1) {
		m_EMegabytes.EnableWindow(TRUE);
		m_EKilobytes.EnableWindow(TRUE);
		m_EBytes.EnableWindow(TRUE);
		m_SMegabytes.EnableWindow(TRUE);
		m_SKilobytes.EnableWindow(TRUE);
		m_SBytes.EnableWindow(TRUE);
		m_SAccess.EnableWindow(TRUE);
		m_SRead.EnableWindow(TRUE);
		m_SRandom.EnableWindow(TRUE);
		m_EDelayTime.EnableWindow(TRUE);
		m_EBurstLength.EnableWindow(TRUE);
		m_RAlignBytes.EnableWindow(TRUE);
		m_RAlignSector.EnableWindow(TRUE);
		m_RReplySize.EnableWindow(TRUE);
		m_RNoReply.EnableWindow(TRUE);
		m_BInsertBefore.EnableWindow(TRUE);
		m_BDelete.EnableWindow(TRUE);
		m_BOk.EnableWindow(TRUE);

		if (GetCheckedRadioButton(RAlignSector, RAlignBytes) == RAlignBytes) {
			m_EAlignMegabytes.EnableWindow(TRUE);
			m_EAlignKilobytes.EnableWindow(TRUE);
			m_EAlignBytes.EnableWindow(TRUE);
			m_SAlignMegabytes.EnableWindow(TRUE);
			m_SAlignKilobytes.EnableWindow(TRUE);
			m_SAlignBytes.EnableWindow(TRUE);
		}

		if (GetCheckedRadioButton(RNoReply, RReplySize) == RReplySize) {
			m_EReplyMegabytes.EnableWindow(TRUE);
			m_EReplyKilobytes.EnableWindow(TRUE);
			m_EReplyBytes.EnableWindow(TRUE);
			m_SReplyMegabytes.EnableWindow(TRUE);
			m_SReplyKilobytes.EnableWindow(TRUE);
			m_SReplyBytes.EnableWindow(TRUE);
		}
	}

	return TRUE;
}

//
// Deletes a line in the access spec.
//
BOOL CAccessDialog::DeleteLine()
{
	// Retrieve the access spec pointer from the list and deallocate the object.
	delete(Access_Spec *) m_LAccess.GetItemData(item_being_changed);

	// Remove the line from the listbox.
	m_LAccess.DeleteItem(item_being_changed);

	// All the entries have been removed.  Disable all related controls.
	if (!m_LAccess.GetItemCount()) {
		m_EMegabytes.EnableWindow(FALSE);
		m_EKilobytes.EnableWindow(FALSE);
		m_EBytes.EnableWindow(FALSE);
		m_SMegabytes.EnableWindow(FALSE);
		m_SKilobytes.EnableWindow(FALSE);
		m_SBytes.EnableWindow(FALSE);
		m_SAccess.EnableWindow(FALSE);
		m_SRead.EnableWindow(FALSE);
		m_SRandom.EnableWindow(FALSE);
		m_EDelayTime.EnableWindow(FALSE);
		m_EBurstLength.EnableWindow(FALSE);
		m_RAlignBytes.EnableWindow(FALSE);
		m_RAlignSector.EnableWindow(FALSE);
		m_EAlignMegabytes.EnableWindow(FALSE);
		m_EAlignKilobytes.EnableWindow(FALSE);
		m_EAlignBytes.EnableWindow(FALSE);
		m_SAlignMegabytes.EnableWindow(FALSE);
		m_SAlignKilobytes.EnableWindow(FALSE);
		m_SAlignBytes.EnableWindow(FALSE);
		m_RReplySize.EnableWindow(FALSE);
		m_RNoReply.EnableWindow(FALSE);
		m_SReplyMegabytes.EnableWindow(FALSE);
		m_SReplyKilobytes.EnableWindow(FALSE);
		m_SReplyBytes.EnableWindow(FALSE);
		m_EReplyMegabytes.EnableWindow(FALSE);
		m_EReplyKilobytes.EnableWindow(FALSE);
		m_EReplyBytes.EnableWindow(FALSE);
		m_BInsertBefore.EnableWindow(FALSE);
		m_BDelete.EnableWindow(FALSE);
		m_BOk.EnableWindow(FALSE);
	}

	return TRUE;
}

//
// Calls each of the Set functions.
//
void CAccessDialog::SetAll(Access_Spec * spec)
{
	SetSize(spec->size);
	SetAccess(spec->of_size);
	SetReads(spec->reads);
	SetRandom(spec->random);
	SetDelay(spec->delay);
	SetBurst(spec->burst);

	SetAlign(spec->align);
	EnableMKBControls(&align_controls, spec->align);
	if (!spec->align) {
		// Set the (disabled) alignment controls to 512 bytes (standard sector size).
		SetMKBSpinners(&align_controls, 512);
	}

	SetReply(spec->reply);
	EnableMKBControls(&reply_controls, spec->reply);
	if (!spec->reply) {
		// Set the (disabled) reply size controls to the current transfer size.
		SetMKBSpinners(&reply_controls, spec->size);
	}
}

//
// Calls each of the Get functions.
//
void CAccessDialog::GetAll(Access_Spec * spec)
{
	spec->size = GetSize();
	spec->of_size = GetAccess();
	spec->reads = GetReads();
	spec->random = GetRandom();
	spec->delay = GetDelay();
	spec->burst = GetBurst();
	spec->align = GetAlign();
	spec->reply = GetReply();
}

void CAccessDialog::SetAccess(int of_size)
{
	CString buffer;

	// Change the value in the access spec.
	((Access_Spec *) m_LAccess.GetItemData(item_being_changed))->of_size = of_size;

	// Display the change.
	buffer.Format("%d", of_size);
	m_LAccess.SetItemText(item_being_changed, LAccessOfSize, buffer);

	// Update the control.
	m_SAccess.SetPos(of_size);
	access_txt.Format("%d Percent", of_size);

	UpdateData(FALSE);
}

int CAccessDialog::GetAccess()
{
	// Retrieve a pointer to the access spec for the requested line
	// and return the requested member of the spec.
	return ((Access_Spec *) m_LAccess.GetItemData(item_being_changed))->of_size;
}

void CAccessDialog::SetReads(int reads)
{
	CString buffer;

	// Change the value in the access spec.
	((Access_Spec *) m_LAccess.GetItemData(item_being_changed))->reads = reads;

	// Display the change.
	buffer.Format("%d", reads);
	m_LAccess.SetItemText(item_being_changed, LAccessReads, buffer);

	// Update the control.
	m_SRead.SetPos(reads);
	read_txt.Format("%d%%\nRead", reads);
	write_txt.Format("%d%%\nWrite", 100 - reads);

	UpdateData(FALSE);
}

int CAccessDialog::GetReads()
{
	// Retrieve a pointer to the access spec for the requested line
	// and return the requested member of the spec.
	return ((Access_Spec *) m_LAccess.GetItemData(item_being_changed))->reads;
}

void CAccessDialog::SetRandom(int random)
{
	CString buffer;

	// Change the value in the access spec.
	((Access_Spec *) m_LAccess.GetItemData(item_being_changed))->random = random;

	// Display the change.
	buffer.Format("%d", random);
	m_LAccess.SetItemText(item_being_changed, LAccessRandom, buffer);

	// Update the control.
	m_SRandom.SetPos(random);
	random_txt.Format("%d%%\nRandom", random);
	sequential_txt.Format("%d%%\nSequential", 100 - random);

	UpdateData(FALSE);
}

int CAccessDialog::GetRandom()
{
	// Retrieve a pointer to the access spec for the requested line
	// and return the requested member of the spec.
	return ((Access_Spec *) m_LAccess.GetItemData(item_being_changed))->random;
}

void CAccessDialog::SetDelay(int delay)
{
	CString buffer;

	// Change the value in the access spec.
	((Access_Spec *) m_LAccess.GetItemData(item_being_changed))->delay = delay;

	// Display the change.
	buffer.Format("%d", delay);
	m_LAccess.SetItemText(item_being_changed, LAccessDelay, buffer);

	// Update the control.
	SetDlgItemInt(EDelayTime, delay, FALSE);

	UpdateData(FALSE);
}

int CAccessDialog::GetDelay()
{
	// Retrieve a pointer to the access spec for the requested line
	// and return the requested member of the spec.
	return ((Access_Spec *) m_LAccess.GetItemData(item_being_changed))->delay;
}

void CAccessDialog::SetBurst(int burst)
{
	CString buffer;

	// Change the value in the access spec.
	((Access_Spec *) m_LAccess.GetItemData(item_being_changed))->burst = burst;

	// Display the change.
	buffer.Format("%d", burst);
	m_LAccess.SetItemText(item_being_changed, LAccessBurst, buffer);

	// Update the control.
	SetDlgItemInt(EBurstLength, burst, FALSE);

	UpdateData(FALSE);
}

int CAccessDialog::GetBurst()
{
	// Retrieve a pointer to the access spec for the requested line
	// and return the requested member of the spec.
	return ((Access_Spec *) m_LAccess.GetItemData(item_being_changed))->burst;
}

void CAccessDialog::SetAlign(DWORD align)
{
	CString buffer;

	// Change the value in the access spec.
	((Access_Spec *) m_LAccess.GetItemData(item_being_changed))->align = align;

	// Display the change.
	if (align)
		SizeToText(align, &buffer);
	else
		buffer.Format("sector");

	m_LAccess.SetItemText(item_being_changed, LAccessAlignment, buffer);

	// Update the controls.
	SetMKBSpinners(&align_controls, align);

	UpdateData(FALSE);
}

DWORD CAccessDialog::GetAlign()
{
	// Retrieve a pointer to the access spec for the requested line
	// and return the requested member of the spec.
	return ((Access_Spec *) m_LAccess.GetItemData(item_being_changed))->align;
}

void CAccessDialog::SetReply(DWORD reply)
{
	CString buffer;

	// Change the value in the access spec.
	((Access_Spec *) m_LAccess.GetItemData(item_being_changed))->reply = reply;

	// Display the change.
	if (reply)
		SizeToText(reply, &buffer);
	else
		buffer.Format("none");

	m_LAccess.SetItemText(item_being_changed, LAccessReply, buffer);

	// Update the controls.
	SetMKBSpinners(&reply_controls, reply);

	UpdateData(FALSE);
}

DWORD CAccessDialog::GetReply()
{
	// Retrieve a pointer to the access spec for the requested line
	// and return the requested member of the spec.
	return ((Access_Spec *) m_LAccess.GetItemData(item_being_changed))->reply;
}

void CAccessDialog::SetSize(DWORD size)
{
	CString buffer;

	// Change the value in the access spec.
	((Access_Spec *) m_LAccess.GetItemData(item_being_changed))->size = size;

	// Display the change.
	SizeToText(size, &buffer);
	m_LAccess.SetItemText(item_being_changed, LAccessSize, buffer);

	// Update the controls.
	SetMKBSpinners(&size_controls, size);

	// If reply size is not currently selected, also set the (disabled) reply size controls
	// to the new transfer size.
	if ((UINT) GetCheckedRadioButton(RNoReply, RReplySize) != RReplySize)
		SetMKBSpinners(&reply_controls, size);

	UpdateData(FALSE);
}

DWORD CAccessDialog::GetSize()
{
	// Retrieve a pointer to the access spec for the requested line
	// and return the requested member of the spec.
	return ((Access_Spec *) m_LAccess.GetItemData(item_being_changed))->size;
}

//
// Called when the user clicks the Delete button.
// Removes all selected lines from the access spec.
//
void CAccessDialog::OnBDelete()
{
	int first_highlighted_item;

	// Return immediately if nothing is selected.
	if ((first_highlighted_item = m_LAccess.GetNextItem(-1, LVNI_SELECTED)) == -1)
		return;

	// Loop through all selected items and delete them.
	while ((item_being_changed = m_LAccess.GetNextItem(-1, LVNI_SELECTED)) != -1)
		DeleteLine();

	item_being_changed = first_highlighted_item;

	if (item_being_changed != -1 && item_being_changed < m_LAccess.GetItemCount()) {
		m_LAccess.SetItemState(item_being_changed, LVIS_SELECTED, LVIS_SELECTED);
		SetAll((Access_Spec *) m_LAccess.GetItemData(item_being_changed));
	} else if (m_LAccess.GetItemCount()) {
		item_being_changed = m_LAccess.GetItemCount() - 1;
		m_LAccess.SetItemState(item_being_changed, LVIS_SELECTED, LVIS_SELECTED);
		SetAll((Access_Spec *) m_LAccess.GetItemData(item_being_changed));
	} else {
		// No more items left in the list.
		item_being_changed = -1;
	}
}

//
// Called when the contents of the delay editbox change
//
void CAccessDialog::OnChangeDelay()
{
	// Update the delay time if it's not blank.
	if (m_EDelayTime.LineLength()) {
		SetDelay(GetDlgItemInt(EDelayTime));
	} else if (GetFocus() != &m_EDelayTime) {
		// Entry box is empty and it no longer has focus.
		// Revert to the value in the access spec.
		SetDelay(GetDelay());
	}
}

//
// Called when the contents of the burst rate editbox change
//
void CAccessDialog::OnChangeBurst()
{
	// Update the burst rate if it's not blank.
	if (m_EBurstLength.LineLength()) {
		// Prevent a burst length of 0.
		if (!GetDlgItemInt(EBurstLength)) {
			ErrorMessage("Burst length cannot be set to 0.");

			// Revert to the old value.
			if (GetBurst())
				SetBurst(GetBurst());
			else
				SetBurst(1);
		} else {
			SetBurst(GetDlgItemInt(EBurstLength));
		}
	} else if (GetFocus() != &m_EBurstLength) {
		// Entry box is empty and it no longer has focus.
		// Revert to the old value.
		SetBurst(GetBurst());
	}
}

//
// Called when the user clicks cancel or otherwise closes the window
// (Alt-F4, clicking the top right X)
// This throws away any changes by deleting the working access spec
// without copying them to the main access spec structure.
//
void CAccessDialog::OnCancel()
{
	int line_index;

	// Delete all working copies of Access_Spec list items.
	for (line_index = 0; line_index < m_LAccess.GetItemCount(); line_index++)
		delete(Access_Spec *) m_LAccess.GetItemData(line_index);

	CDialog::OnCancel();
}
