/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / ICFSaveDialog.cpp                                      ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implementation of the CICFSaveDialog class, which     ## */
/* ##               extends the standard CFileDialog class with           ## */
/* ##               additional functionality for saving Iometer's .ICF    ## */
/* ##               (Iometer Configuration File) files.                   ## */
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
#include "GalileoApp.h"
#include "ICFSaveDialog.h"

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

IMPLEMENT_DYNAMIC(CICFSaveDialog, CFileDialog)

/////////////////////////////////////////////////////////////////////////////
// CICFSaveDialog dialog
    CICFSaveDialog::CICFSaveDialog()
:  CFileDialog(FALSE, "icf", "iometer", NULL,
	    "Iometer Configuration Files (*.icf)|*.icf|" "Text Files (*.txt)|*.txt|All Files (*.*)|*.*||")
{
	CString title;
	char *buf;

	VERIFY(title.LoadString(IDS_SAVE_DIALOG_TITLE));	// get frame title from resource file
	buf = new char[title.GetLength() + 1];

	strcpy(buf, title);
	m_ofn.lpstrTitle = buf;

	m_ofn.Flags |= OFN_ENABLETEMPLATE | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_FILESAVE_OPTS);
	//{{AFX_DATA_INIT(CICFSaveDialog)
	isCkTestSetup = TRUE;
	isCkResultsDisplay = TRUE;
	isCkGlobalAccessSpec = TRUE;
	isCkManagerWorker = TRUE;
	isCkAssignAccessSpec = TRUE;
	isCkAssignTargets = TRUE;
	//}}AFX_DATA_INIT

	wasCkAssignAccessSpec = TRUE;
	wasCkAssignTargets = TRUE;
}

CICFSaveDialog::~CICFSaveDialog()
{
	if (m_ofn.lpstrTitle)
		delete (void*)m_ofn.lpstrTitle;
}

void CICFSaveDialog::DoDataExchange(CDataExchange * pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CICFSaveDialog)
	DDX_Control(pDX, CkTestSetup, m_CkTestSetup);
	DDX_Control(pDX, CkResultsDisplay, m_CkResultsDisplay);
	DDX_Control(pDX, CkGlobalAccessSpec, m_CkGlobalAccessSpec);
	DDX_Control(pDX, CkManagerWorker, m_CkManagerWorker);
	DDX_Control(pDX, CkAssignAccessSpec, m_CkAssignAccessSpec);
	DDX_Control(pDX, CkAssignTargets, m_CkAssignTargets);
	DDX_Check(pDX, CkTestSetup, isCkTestSetup);
	DDX_Check(pDX, CkResultsDisplay, isCkResultsDisplay);
	DDX_Check(pDX, CkGlobalAccessSpec, isCkGlobalAccessSpec);
	DDX_Check(pDX, CkManagerWorker, isCkManagerWorker);
	DDX_Check(pDX, CkAssignAccessSpec, isCkAssignAccessSpec);
	DDX_Check(pDX, CkAssignTargets, isCkAssignTargets);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CICFSaveDialog, CDialog)
    //{{AFX_MSG_MAP(CICFSaveDialog)
    ON_BN_CLICKED(CkManagerWorker, OnCkManagerWorker)
    ON_BN_CLICKED(CkTestSetup, OnAnyCheck)
    ON_WM_HELPINFO()
    ON_BN_CLICKED(CkResultsDisplay, OnAnyCheck)
    ON_BN_CLICKED(CkGlobalAccessSpec, OnAnyCheck)
    ON_BN_CLICKED(CkAssignAccessSpec, OnAnyCheck)
    ON_BN_CLICKED(CkAssignTargets, OnAnyCheck)
ON_WM_PAINT()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CICFSaveDialog message handlers
void CICFSaveDialog::OnCkManagerWorker()
{
	UpdateData(TRUE);	// copy the GUI state to the variables

	if (isCkManagerWorker) {
		// The box was checked
		m_CkAssignAccessSpec.EnableWindow(TRUE);
		isCkAssignAccessSpec = wasCkAssignAccessSpec;

		m_CkAssignTargets.EnableWindow(TRUE);
		isCkAssignTargets = wasCkAssignTargets;
	} else {
		// The box was unchecked
		if (m_CkAssignAccessSpec.IsWindowEnabled()) {
			m_CkAssignAccessSpec.EnableWindow(FALSE);
			wasCkAssignAccessSpec = isCkAssignAccessSpec;
			isCkAssignAccessSpec = FALSE;
		}

		if (m_CkAssignTargets.IsWindowEnabled()) {
			m_CkAssignTargets.EnableWindow(FALSE);
			wasCkAssignTargets = isCkAssignTargets;
			isCkAssignTargets = FALSE;
		}
	}

	UpdateData(FALSE);	// copy the variable states to the GUI

	OnAnyCheck();
}

void CICFSaveDialog::OnAnyCheck()
{
	UpdateData(TRUE);	// copy the GUI state to the variables

	// Disable OK button if no boxes are checked
	if (isCkTestSetup || isCkResultsDisplay
	    || isCkGlobalAccessSpec || isCkManagerWorker || isCkAssignAccessSpec || isCkAssignTargets)
		CWnd::GetOwner()->GetDlgItem(IDOK)->EnableWindow(TRUE);
	else
		CWnd::GetOwner()->GetDlgItem(IDOK)->EnableWindow(FALSE);
}

BOOL CICFSaveDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	UpdateData(FALSE);	// copy the variable states to the GUI

	if (!isCkManagerWorker) {
		m_CkAssignAccessSpec.EnableWindow(FALSE);
		m_CkAssignTargets.EnableWindow(FALSE);
	}

	return TRUE;		// return TRUE unless you set the focus to a control
}

BOOL CICFSaveDialog::OnHelpInfo(HELPINFO * pHelpInfo)
{
	switch (pHelpInfo->iCtrlId) {
		// When we figure out the help, put it here...
	case CkTestSetup:
		break;
	case CkResultsDisplay:
		break;
	case CkGlobalAccessSpec:
		break;
	case CkManagerWorker:
		break;
	case CkAssignAccessSpec:
		break;
	case CkAssignTargets:
		break;
	}

	return CDialog::OnHelpInfo(pHelpInfo);
}

//
// Draw the lines that show the relation between the Manager/Worker
// checkbox and the access spec and target assignment checkboxes.
// There is no need to call CFileDialog::OnPaint.  That is a separate
// message, since this is an extension to the file open/save common
// dialog.
//
void CICFSaveDialog::OnPaint()
{
	CPaintDC dc(this);	// device context for painting

	CRect MgrWkr;
	CRect AsgnAspec;
	CRect AsgnTargets;

	int centerer;
	int x1, x2, y1, y2, y3;

	// Get the screen coordinates of the controls.
	m_CkManagerWorker.GetWindowRect(MgrWkr);
	m_CkAssignAccessSpec.GetWindowRect(AsgnAspec);
	m_CkAssignTargets.GetWindowRect(AsgnTargets);

	// Convert screen coords to window (client-area) coords.
	ScreenToClient(MgrWkr);
	ScreenToClient(AsgnAspec);
	ScreenToClient(AsgnTargets);

	// The distance from the corner of a checkbox square to the center of one of its sides.
	centerer = (MgrWkr.bottom - MgrWkr.top) / 2 - 1;

	x1 = MgrWkr.left + centerer - 1;
	x2 = AsgnAspec.left;

	y1 = MgrWkr.bottom;
	y2 = AsgnAspec.top + centerer;
	y3 = AsgnTargets.top + centerer;

	dc.MoveTo(x1, y1);
	dc.LineTo(x1, y3);	// Vertical line (from MgrWkr checkbox, straight down)
	dc.MoveTo(x1, y2);
	dc.LineTo(x2, y2);	// Horiz line from the first line to the AsgnAspec checkbox
	dc.MoveTo(x1, y3);
	dc.LineTo(x2, y3);	// Horiz line from the first line to the AsgnTargets checkbox
}
