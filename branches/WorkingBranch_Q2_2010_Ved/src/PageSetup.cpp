/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / PageSetup.cpp                                          ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implementation of the CPageSetup class, which is      ## */
/* ##               responsible for the "Test Setup" tab in Iometer's     ## */
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
/* ##               2003-05-07 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Modified output of the current timestamp to         ## */
/* ##                 contain milliseconds as well.                       ## */
/* ##               2003-04-25 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Updated the global debug flag (_DEBUG) handling     ## */
/* ##                 of the source file (check for platform etc.).       ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include "stdafx.h"
#include "PageSetup.h"
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
#ifdef IOMTR_SETTING_MFC_MEMALLOC_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// CPageSetup property page

IMPLEMENT_DYNCREATE(CPageSetup, CPropertyPage)

CPageSetup::CPageSetup():CPropertyPage(CPageSetup::IDD)
{
	//{{AFX_DATA_INIT(CPageSetup)
	//}}AFX_DATA_INIT
}

CPageSetup::~CPageSetup()
{
}

void CPageSetup::DoDataExchange(CDataExchange * pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, EWorkerStart, worker_cycling.start_box);
	DDX_Control(pDX, EWorkerStep, worker_cycling.step_box);
	DDX_Control(pDX, CWStepType, worker_cycling.step_sel_box);

	DDX_Control(pDX, EDiskStart, target_cycling.start_box);
	DDX_Control(pDX, EDiskStep, target_cycling.step_box);
	DDX_Control(pDX, CDStepType, target_cycling.step_sel_box);

	DDX_Control(pDX, EQueueStart, queue_cycling.start_box);
	DDX_Control(pDX, EQueueEnd, queue_cycling.end_box);
	DDX_Control(pDX, EQueueStep, queue_cycling.step_box);
	DDX_Control(pDX, CQStepType, queue_cycling.step_sel_box);
	//{{AFX_DATA_MAP(CPageSetup)
	DDX_Control(pDX, CResultType, m_CResultType);
	DDX_Control(pDX, EDiskWorkerCount, m_EDiskWorkerCount);
	DDX_Control(pDX, ENetWorkerCount, m_ENetWorkerCount);
	DDX_Control(pDX, ETestName, m_ETestName);
	DDX_Control(pDX, ERunTimeSeconds, m_ERunTimeSeconds);
	DDX_Control(pDX, ERunTimeMinutes, m_ERunTimeMinutes);
	DDX_Control(pDX, ERunTimeHours, m_ERunTimeHours);
	DDX_Control(pDX, ERampTime, m_ERampTime);
	DDX_Control(pDX, CTestType, m_CTestType);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPageSetup, CPropertyPage)
    //{{AFX_MSG_MAP(CPageSetup)
    ON_EN_CHANGE(EDiskWorkerCount, OnChangeEDiskWorkerCount)
    ON_EN_CHANGE(ENetWorkerCount, OnChangeENetWorkerCount)
    ON_CBN_SELCHANGE(CDStepType, OnSelchangeCDStepType)
    ON_CBN_SELCHANGE(CWStepType, OnSelchangeCWStepType)
    ON_CBN_SELCHANGE(CQStepType, OnSelchangeCQStepType)
    ON_EN_KILLFOCUS(EDiskStart, OnKillfocusEDiskStart)
    ON_EN_KILLFOCUS(EDiskStep, OnKillfocusEDiskStep)
    ON_EN_KILLFOCUS(EQueueEnd, OnKillfocusEQueueEnd)
    ON_EN_KILLFOCUS(EQueueStart, OnKillfocusEQueueStart)
    ON_EN_KILLFOCUS(EQueueStep, OnKillfocusEQueueStep)
    ON_EN_KILLFOCUS(ERampTime, OnKillfocusERampTime)
    ON_EN_KILLFOCUS(ERunTimeHours, OnKillfocusERunTimeHours)
    ON_EN_KILLFOCUS(ERunTimeMinutes, OnKillfocusERunTimeMinutes)
    ON_EN_KILLFOCUS(ERunTimeSeconds, OnKillfocusERunTimeSeconds)
    ON_EN_KILLFOCUS(EDiskWorkerCount, OnKillfocusEDiskWorkerCount)
    ON_EN_KILLFOCUS(ENetWorkerCount, OnKillfocusENetWorkerCount)
    ON_EN_KILLFOCUS(EWorkerStart, OnKillfocusEWorkerStart)
    ON_EN_KILLFOCUS(EWorkerStep, OnKillfocusEWorkerStep)
    ON_CBN_SELCHANGE(CResultType, OnSelchangeCResultType)
    ON_CBN_SELCHANGE(CTestType, OnSelchangeCTestType)
    ON_BN_CLICKED(RDiskSpawnCPUs, OnRDiskSpawnCPUs)
    ON_BN_CLICKED(RDiskSpawnUser, OnRDiskSpawnUser)
    ON_BN_CLICKED(RNetSpawnCPUs, OnRNetSpawnCPUs)
ON_BN_CLICKED(RNetSpawnUser, OnRNetSpawnUser)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CPageSetup message handlers
BOOL CPageSetup::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// Set run time.
	minutes = 0;
	seconds = 0;
	hours = 0;
	ramp_time = 0;

	test_name = _T("");	// blank test description

	SetDlgItemInt(EDiskWorkerCount, 0);	// Display a number.
	SetDlgItemInt(ENetWorkerCount, 0);	// Display a number.
	disk_worker_count = -1;	// default number of disk workers               
#ifdef _DEBUG
	net_worker_count = -1;	// create network workers for debug
#else
	net_worker_count = 0;	// default number of network workers
#endif
	test_type = 0;		// normal test by default, disable cycling info below

	// Set worker cycling stepping.
	SetCyclingInfo(&worker_cycling, FALSE, 1, IOERROR, 1, StepLinear);
	OnSelchangeCWStepType();
	SetCyclingInfo(&target_cycling, FALSE, 1, IOERROR, 1, StepLinear);
	OnSelchangeCDStepType();
	SetCyclingInfo(&queue_cycling, FALSE, 1, 32, 2, StepExponential);
	OnSelchangeCQStepType();

#ifdef _DEBUG
	result_type = RecordNone;	// don't save results for debug code
#else
	result_type = RecordAll;	// save all results for release code
#endif

	UpdateData(FALSE);

	return TRUE;		// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//
// Enable or disable the display for user access.
//
void CPageSetup::EnableWindow(BOOL enable)
{
	// All the windows are enabled or disabled at the same time.
	// Don't bother enabling the windows if they allready are.
	if ((theApp.test_state != TestIdle) || (m_CResultType.IsWindowEnabled() == enable))
		return;

	m_CResultType.EnableWindow(enable);
	m_CResultType.RedrawWindow();
	m_CTestType.EnableWindow(enable);
	m_CTestType.RedrawWindow();

	EnableCyclingInfo(&worker_cycling, enable);
	EnableCyclingInfo(&target_cycling, enable);
	EnableCyclingInfo(&queue_cycling, enable);

	m_ERampTime.EnableWindow(enable);
	m_ERunTimeHours.EnableWindow(enable);
	m_ERunTimeMinutes.EnableWindow(enable);
	m_ERunTimeSeconds.EnableWindow(enable);
	m_EDiskWorkerCount.EnableWindow(enable);
	m_ENetWorkerCount.EnableWindow(enable);
	m_ETestName.EnableWindow(enable);

	GetDlgItem(RDiskSpawnCPUs)->EnableWindow(enable);
	GetDlgItem(RDiskSpawnUser)->EnableWindow(enable);
	GetDlgItem(RNetSpawnCPUs)->EnableWindow(enable);
	GetDlgItem(RNetSpawnUser)->EnableWindow(enable);

	UpdateWindow();
}

//
// Enable or disable cycling information on the GUI.  This is usually done as a result 
// of the selected test type. 
//
void CPageSetup::EnableCyclingInfo(Cycling_Info * cycling_info, BOOL enable)
{
	// Do not enable the GUI unless its cycling type is selected.
	if (enable && !cycling_info->selected)
		return;

	cycling_info->start_box.EnableWindow(enable);
	cycling_info->start_box.RedrawWindow();

	// Make sure that an end cycling number is supported.
	if (cycling_info->end != IOERROR) {
		// End edit box exists, so update it.
		cycling_info->end_box.EnableWindow(enable);
		cycling_info->end_box.RedrawWindow();
	}

	cycling_info->step_box.EnableWindow(enable);
	cycling_info->step_box.RedrawWindow();

	cycling_info->step_sel_box.EnableWindow(enable);
	cycling_info->step_sel_box.RedrawWindow();
}

//
// Set cycling information.
//
void CPageSetup::SetCyclingInfo(Cycling_Info * cycle_info, BOOL selected, int start, int end, int step, int cycle_type)
{
	cycle_info->selected = selected;
	cycle_info->start = start;
	cycle_info->end = end;
	cycle_info->step = step;
	cycle_info->step_type = cycle_type;
	cycle_info->step_sel_box.SetCurSel(cycle_type);

	EnableCyclingInfo(cycle_info, selected);

	UpdateWindow();
}

//
// Saving test setup and its starting time to a file.
//
void CPageSetup::SaveResults(ostream * file)
{
	struct _timeb tb;
	struct tm *ptm;
	char acDummy[64];

	(*file) << "'Test Type,Test Description" << endl
	    << m_CTestType.GetCurSel() << "," << (LPCTSTR) test_name << endl;

	(*file) << "'Version" << endl << theApp.GetVersionString(TRUE) << endl;

	// Write current timestamp into the result file
	(*file) << "'Time Stamp" << endl;
	_ftime(&tb);
	ptm = localtime(&tb.time);
	snprintf(acDummy, 64, "%04d-%02d-%02d %02d:%02d:%02d:%003d", ptm->tm_year + 1900,
		ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, tb.millitm);
	(*file) << acDummy << endl;
}

//
// Saving test setup information to a file.
//
BOOL CPageSetup::SaveConfig(ostream & outfile)
{
	// Update memory variables with data located on GUI.
	UpdateData(TRUE);

	// Save test setup information.
	outfile << "'TEST SETUP ============================" "========================================" << endl;

	outfile << "'Test Description" << endl << "\t" << (LPCTSTR) test_name << endl;

	// Save run time information.

	outfile << setiosflags(ios::left);

	outfile << "'Run Time" << endl
	    << "'\t" << setw(11) << "hours"
	    << setw(11) << "minutes"
	    << "seconds" << endl << "\t" << setw(11) << hours << setw(11) << minutes << seconds << endl;

	// Save miscellaneous information.
	outfile << "'Ramp Up Time (s)" << endl << "\t" << ramp_time << endl;

	outfile << "'Default Disk Workers to Spawn" << endl << "\t";

	if (disk_worker_count == -1)
		outfile << "NUMBER_OF_CPUS" << endl;
	else
		outfile << disk_worker_count << endl;

	outfile << "'Default Network Workers to Spawn" << endl << "\t";

	if (net_worker_count == -1)
		outfile << "NUMBER_OF_CPUS" << endl;
	else
		outfile << net_worker_count << endl;

	outfile << "'Record Results" << endl << "\t";

	switch (result_type) {
	case RecordAll:
		outfile << "ALL" << endl;
		break;
	case RecordNoTargets:
		outfile << "NO_TARGETS" << endl;
		break;
	case RecordNoWorkers:
		outfile << "NO_WORKERS" << endl;
		break;
	case RecordNoManagers:
		outfile << "NO_MANAGERS" << endl;
		break;
	case RecordNone:
		outfile << "NONE" << endl;
		break;
	default:
		ErrorMessage("Error while saving test setup information.  "
			     "Record Results setting in Test Setup tab is invalid.  "
			     "Please report this as an Iometer bug.");
		return FALSE;
	}

	// Save worker cycling information.
	outfile << "'Worker Cycling" << endl
	    << "'\t" << setw(11) << "start"
	    << setw(11) << "step"
	    << "step type" << endl << "\t" << setw(11) << worker_cycling.start << setw(11) << worker_cycling.step;

	switch (worker_cycling.step_type) {
	case StepLinear:
		outfile << "LINEAR" << endl;
		break;
	case StepExponential:
		outfile << "EXPONENTIAL" << endl;
		break;
	default:
		ErrorMessage("Error saving test setup data.  Worker cycling step type "
			     "is neither StepLinear nor StepExponential.");
		return FALSE;
	}

	// Save disk cycling information.
	outfile << "'Disk Cycling" << endl
	    << "'\t" << setw(11) << "start"
	    << setw(11) << "step"
	    << "step type" << endl << "\t" << setw(11) << target_cycling.start << setw(11) << target_cycling.step;

	switch (target_cycling.step_type) {
	case StepLinear:
		outfile << "LINEAR" << endl;
		break;
	case StepExponential:
		outfile << "EXPONENTIAL" << endl;
		break;
	default:
		ErrorMessage("Error saving test setup data.  Disk cycling step type "
			     "is neither StepLinear nor StepExponential.");
		return FALSE;
	}

	// Save queue cycling information.
	outfile << "'Queue Depth Cycling" << endl
	    << "'\t" << setw(11) << "start"
	    << setw(11) << "end"
	    << setw(11) << "step"
	    << "step type" << endl
	    << "\t" << setw(11) << queue_cycling.start
	    << setw(11) << queue_cycling.end << setw(11) << queue_cycling.step;

	switch (queue_cycling.step_type) {
	case StepLinear:
		outfile << "LINEAR" << endl;
		break;
	case StepExponential:
		outfile << "EXPONENTIAL" << endl;
		break;
	default:
		ErrorMessage("Error saving test setup data.  Queue depth cycling step type "
			     "is neither LINEAR nor EXPONENTIAL.");
		return FALSE;
	}

	// Save test type.
	outfile << "'Test Type" << endl << "\t";

	switch (test_type) {
	case CyclingTargets:
		outfile << "CYCLE_TARGETS" << endl;
		break;
	case CyclingWorkers:
		outfile << "CYCLE_WORKERS" << endl;
		break;
	case CyclingIncTargetsParallel:
		outfile << "INCREMENT_TARGETS_PARALLEL" << endl;
		break;
	case CyclingIncTargetsSerial:
		outfile << "INCREMENT_TARGETS_SERIAL" << endl;
		break;
	case CyclingWorkersTargets:
		outfile << "CYCLE_WORKERS_AND_TARGETS" << endl;
		break;
	case CyclingQueue:
		outfile << "CYCLE_OUTSTANDING_IOS" << endl;
		break;
	case CyclingQueueTargets:
		outfile << "CYCLE_OUTSTANDING_IOS_AND_TARGETS" << endl;
		break;
	default:
		outfile << "NORMAL" << endl;
		break;
	}

	// Mark end of test setup information.
	outfile << "'END test setup" << endl;

	return TRUE;		// no errors (FALSE indicates an error)
}

//
// Loading test setup information.
//
BOOL CPageSetup::LoadConfig(const CString & infilename)
{
	long version;
	CString key, value;
	CString token;
	ICF_ifstream infile(infilename);

	// Reading in the saved setup file.
	version = infile.GetVersion();
	if (version == -1)
		return FALSE;
	if (!infile.SkipTo("'TEST SETUP"))
		return TRUE;	// no test setup to restore (this is OK)

	while (1) {
		if (!infile.GetPair(key, value)) {
			ErrorMessage("File is improperly formatted.  Expected "
				     "test setup data or \"END test setup\".");
			return FALSE;
		}

		if (key.CompareNoCase("'END test setup") == 0) {
			break;
		} else if (key.CompareNoCase("'Test Description") == 0) {
			test_name = value;
		} else if (key.CompareNoCase("'Run Time") == 0) {
			if (!ICF_ifstream::ExtractFirstInt(value, hours)
			    || !ICF_ifstream::ExtractFirstInt(value, minutes)
			    || !ICF_ifstream::ExtractFirstInt(value, seconds)) {
				ErrorMessage("Error while reading file.  "
					     "\"Run time\" should be specified as three integer values "
					     "(hours, minutes, seconds).");
				return FALSE;
			}
		} else if (key.Left((int)(strlen("'Ramp Up Time"))).CompareNoCase("'Ramp Up Time") == 0) {
			if (!ICF_ifstream::ExtractFirstInt(value, ramp_time)) {
				ErrorMessage("Error while reading file.  "
					     "\"Ramp up time\" should be specified as an integer value.");
				return FALSE;
			}
		}
		// For backward compatibility...
		else if (key.CompareNoCase("'Default Workers to Spawn") == 0) {
			if (!ICF_ifstream::ExtractFirstInt(value, disk_worker_count)) {
				ErrorMessage("Error while reading file.  "
					     "\"Default workers to spawn\" should be "
					     "specified as an integer value.");
				return FALSE;
			}

			if (disk_worker_count == 0)	// "# of CPUs" used to be rep.
				disk_worker_count = -1;	// as 0 internally, now it is -1.
		} else if (key.CompareNoCase("'Default Disk Workers to Spawn") == 0) {
			token = ICF_ifstream::ExtractFirstToken(value);

			if (token.CompareNoCase("NUMBER_OF_CPUS") == 0)
				disk_worker_count = -1;
			else	// all other values are stored as an integer
				disk_worker_count = atoi((LPCTSTR) token);
		} else if (key.CompareNoCase("'Default Network Workers to Spawn") == 0) {
			token = ICF_ifstream::ExtractFirstToken(value);

			if (token.CompareNoCase("NUMBER_OF_CPUS") == 0)
				net_worker_count = -1;
			else	// all other values are stored as an integer
				net_worker_count = atoi((LPCTSTR) token);
		} else if (key.CompareNoCase("'Record Results") == 0) {
			value.TrimLeft();
			value.TrimRight();

			if (value.CompareNoCase("ALL") == 0)
				result_type = RecordAll;
			else if (value.CompareNoCase("NO_TARGETS") == 0)
				result_type = RecordNoTargets;
			else if (value.CompareNoCase("NO_WORKERS") == 0)
				result_type = RecordNoWorkers;
			else if (value.CompareNoCase("NO_MANAGERS") == 0)
				result_type = RecordNoManagers;
			else if (value.CompareNoCase("NONE") == 0)
				result_type = RecordNone;
			// This value used to be stored as an integer.
			// If no string match occured, restore the value as an integer.
			else if (!ICF_ifstream::ExtractFirstInt(value, result_type)) {
				ErrorMessage("Error while reading file.  "
					     "For \"Record results\", expected a legal identifier or an "
					     "integer value.  See the documentation for details.");
				return FALSE;
			}
		} else if (key.CompareNoCase("'Worker Cycling") == 0) {
			if (!ICF_ifstream::ExtractFirstInt(value, worker_cycling.start)
			    || !ICF_ifstream::ExtractFirstInt(value, worker_cycling.step)) {
				ErrorMessage("Error while reading file.  "
					     "\"Worker cycling\" start and step should be specified as "
					     "integer values.");
				return FALSE;
			}

			token = ICF_ifstream::ExtractFirstToken(value);

			if (token.CompareNoCase("LINEAR") == 0) {
				worker_cycling.step_type = StepLinear;
			} else if (token.CompareNoCase("EXPONENTIAL") == 0) {
				worker_cycling.step_type = StepExponential;
			} else	// value used to be stored as an integer
			{
				worker_cycling.step_type = atoi((LPCTSTR) token);
			}
		} else if (key.CompareNoCase("'Disk Cycling") == 0) {
			if (!ICF_ifstream::ExtractFirstInt(value, target_cycling.start)
			    || !ICF_ifstream::ExtractFirstInt(value, target_cycling.step)) {
				ErrorMessage("Error while reading file.  "
					     "\"Disk cycling\" start and step should be specified as "
					     "integer values.");
				return FALSE;
			}

			token = ICF_ifstream::ExtractFirstToken(value);

			if (token.CompareNoCase("LINEAR") == 0) {
				target_cycling.step_type = StepLinear;
			} else if (token.CompareNoCase("EXPONENTIAL") == 0) {
				target_cycling.step_type = StepExponential;
			} else	// value used to be stored as an integer
			{
				target_cycling.step_type = atoi((LPCTSTR) token);
			}
		} else if (key.CompareNoCase("'Queue Depth Cycling") == 0) {
			if (!ICF_ifstream::ExtractFirstInt(value, queue_cycling.start)
			    || !ICF_ifstream::ExtractFirstInt(value, queue_cycling.end)
			    || !ICF_ifstream::ExtractFirstInt(value, queue_cycling.step)) {
				ErrorMessage("Error while reading file.  "
					     "\"Queue depth cycling\" start, end, and step should be "
					     "specified as integer values.");
				return FALSE;
			}

			token = ICF_ifstream::ExtractFirstToken(value);

			if (token.CompareNoCase("LINEAR") == 0) {
				queue_cycling.step_type = StepLinear;
			} else if (token.CompareNoCase("EXPONENTIAL") == 0) {
				queue_cycling.step_type = StepExponential;
			} else	// value used to be stored as an integer
			{
				queue_cycling.step_type = atoi((LPCTSTR) token);
			}
		} else if (key.CompareNoCase("'Test Type") == 0) {
			value.TrimLeft();
			value.TrimRight();

			if (value.CompareNoCase("NORMAL") == 0)
				test_type = 0;
			else if (value.CompareNoCase("CYCLE_TARGETS") == 0)
				test_type = CyclingTargets;
			else if (value.CompareNoCase("CYCLE_WORKERS") == 0)
				test_type = CyclingWorkers;
			else if (value.CompareNoCase("INCREMENT_TARGETS_PARALLEL") == 0)
				test_type = CyclingIncTargetsParallel;
			else if (value.CompareNoCase("INCREMENT_TARGETS_SERIAL") == 0)
				test_type = CyclingIncTargetsSerial;
			else if (value.CompareNoCase("CYCLE_WORKERS_AND_TARGETS") == 0)
				test_type = CyclingWorkersTargets;
			else if (value.CompareNoCase("CYCLE_OUTSTANDING_IOS") == 0)
				test_type = CyclingQueue;
			else if (value.CompareNoCase("CYCLE_OUTSTANDING_IOS_AND_TARGETS") == 0)
				test_type = CyclingQueueTargets;
			// This value used to be stored as an integer.
			// If no string match occured, restore the value as an integer.
			else if (!ICF_ifstream::ExtractFirstInt(value, test_type)) {
				ErrorMessage("Error while reading file.  "
					     "For \"Test type\", expected a legal identifier or an "
					     "integer value.  See the documentation for details.");
				return FALSE;
			}
		} else		// Unrecognized comment 
		{
			if (version >= 19990217) {
				ErrorMessage("File is improperly formatted.  TEST SETUP "
					     "section contained an unrecognized \"" + key + "\" comment.");
				return FALSE;
			}
			// For backward compatibility reasons, we can't count on
			// "'Test Setup" having a corresponding "'End test setup"
			// because versions as recent as 1998.10.08 didn't follow
			// this convention.  All other configuration text blocks
			// should be able to follow the rule that all text blocks
			// are terminated by an "'End..." statement.
			break;
		}
	}

	infile.close();

	// Update the GUI with the values read in from the file.
	UpdateData(FALSE);

	return TRUE;
}

//
// Retrieve the specified run time from the GUI and return it (in ms).
//
int CPageSetup::GetRunTime()
{
	return (1000 *		// ms per second x 
		(hours * 3600	// ( seconds per hour
		 + minutes * 60	// + seconds per minute
		 + seconds));	// + seconds )
}

//
//      Updating either the GUI with data in memory or vice versa.
//
void CPageSetup::UpdateData(BOOL save_from_window)
{
	if (save_from_window) {
		// Update all the combo box selections.
		result_type = m_CResultType.GetCurSel();
		test_type = m_CTestType.GetCurSel();
		worker_cycling.step_type = worker_cycling.step_sel_box.GetCurSel();
		target_cycling.step_type = target_cycling.step_sel_box.GetCurSel();
		queue_cycling.step_type = queue_cycling.step_sel_box.GetCurSel();

		CheckData(ERunTimeHours, &hours);
		CheckData(ERunTimeMinutes, &minutes);
		CheckData(ERunTimeSeconds, &seconds);
		CheckData(ERampTime, &ramp_time);
		CheckData(EWorkerStep, &(worker_cycling.step), TRUE);
		CheckData(EDiskStep, &(target_cycling.step), TRUE);
		CheckData(EQueueStep, &(queue_cycling.step), TRUE);
		CheckData(EWorkerStart, &(worker_cycling.start), TRUE);
		CheckData(EDiskStart, &(target_cycling.start), TRUE);
		CheckData(EQueueStart, &(queue_cycling.start), TRUE);
		CheckData(EQueueEnd, &(queue_cycling.end), TRUE);

		if (GetCheckedRadioButton(RDiskSpawnCPUs, RDiskSpawnUser) == RDiskSpawnCPUs)
			disk_worker_count = -1;
		else
			CheckData(EDiskWorkerCount, &disk_worker_count, FALSE);

		if (GetCheckedRadioButton(RNetSpawnCPUs, RNetSpawnUser) == RNetSpawnCPUs)
			net_worker_count = -1;
		else
			CheckData(ENetWorkerCount, &net_worker_count, FALSE);

		GetDlgItemText(ETestName, test_name);
		// Find all commas and replace with '-'
		test_name.Replace(',', '-');
	} else {
		// Update all the combo box selections.
		m_CResultType.SetCurSel(result_type);
		m_CTestType.SetCurSel(test_type);
		worker_cycling.step_sel_box.SetCurSel(worker_cycling.step_type);
		target_cycling.step_sel_box.SetCurSel(target_cycling.step_type);
		queue_cycling.step_sel_box.SetCurSel(queue_cycling.step_type);

		// Enable/disable and label cycling controls.
		OnSelchangeCTestType();
		SetStepTypeLabel(TWorkerStep, worker_cycling.step_type);
		SetStepTypeLabel(TDiskStep, target_cycling.step_type);
		SetStepTypeLabel(TQueueStep, queue_cycling.step_type);

		// Update the run time and other default information.
		SetDlgItemInt(ERunTimeHours, hours, FALSE);
		SetDlgItemInt(ERunTimeMinutes, minutes, FALSE);
		SetDlgItemInt(ERunTimeSeconds, seconds, FALSE);

		if (disk_worker_count == -1) {
			CheckRadioButton(RDiskSpawnCPUs, RDiskSpawnUser, RDiskSpawnCPUs);
			m_EDiskWorkerCount.EnableWindow(FALSE);
		} else {
			CheckRadioButton(RDiskSpawnCPUs, RDiskSpawnUser, RDiskSpawnUser);
			m_EDiskWorkerCount.EnableWindow(TRUE);
			SetDlgItemInt(EDiskWorkerCount, disk_worker_count, FALSE);
		}

		if (net_worker_count == -1) {
			CheckRadioButton(RNetSpawnCPUs, RNetSpawnUser, RNetSpawnCPUs);
			m_ENetWorkerCount.EnableWindow(FALSE);
		} else {
			CheckRadioButton(RNetSpawnCPUs, RNetSpawnUser, RNetSpawnUser);
			m_ENetWorkerCount.EnableWindow(TRUE);
			SetDlgItemInt(ENetWorkerCount, net_worker_count, FALSE);
		}

		SetDlgItemInt(ERampTime, ramp_time, FALSE);

		// Set the displayed value for the step size and start values.
		SetDlgItemInt(EWorkerStep, worker_cycling.step, FALSE);
		SetDlgItemInt(EDiskStep, target_cycling.step, FALSE);
		SetDlgItemInt(EQueueStep, queue_cycling.step, FALSE);
		SetDlgItemInt(EWorkerStart, worker_cycling.start, FALSE);
		SetDlgItemInt(EDiskStart, target_cycling.start, FALSE);
		SetDlgItemInt(EQueueStart, queue_cycling.start, FALSE);
		SetDlgItemInt(EQueueEnd, queue_cycling.end, FALSE);

		// Set the test name.
		SetDlgItemText(ETestName, test_name);

		UpdateWindow();
	}
}

void CPageSetup::CheckData(UINT item_id, int *var, BOOL no_zero)
{
	if ((*var == 0) && no_zero) {
		*var = 1;
		SetDlgItemInt(item_id, *var, FALSE);
	}
}

void CPageSetup::OnChangeEDiskWorkerCount()
{
	CString buffer;

	m_EDiskWorkerCount.GetWindowText(buffer);
	if (!buffer.IsEmpty())
		disk_worker_count = GetDlgItemInt(EDiskWorkerCount, NULL, FALSE);
}

void CPageSetup::OnChangeENetWorkerCount()
{
	CString buffer;

	m_ENetWorkerCount.GetWindowText(buffer);
	if (!buffer.IsEmpty())
		net_worker_count = GetDlgItemInt(ENetWorkerCount, NULL, FALSE);
}

//
// Set the label for the step edit box based on the step type.
//
void CPageSetup::SetStepTypeLabel(int text_id, int step_type)
{
	if (step_type == StepLinear)
		SetDlgItemText(text_id, "Step");
	else
		SetDlgItemText(text_id, "Power of");
	UpdateWindow();
}

// Worker step type has changed.
void CPageSetup::OnSelchangeCWStepType()
{
	worker_cycling.step_type = worker_cycling.step_sel_box.GetCurSel();
	SetStepTypeLabel(TWorkerStep, worker_cycling.step_type);
}

// Disk step type has changed.
void CPageSetup::OnSelchangeCDStepType()
{
	target_cycling.step_type = target_cycling.step_sel_box.GetCurSel();
	SetStepTypeLabel(TDiskStep, target_cycling.step_type);
}

// Queue depth step type has changed.
void CPageSetup::OnSelchangeCQStepType()
{
	queue_cycling.step_type = queue_cycling.step_sel_box.GetCurSel();
	SetStepTypeLabel(TQueueStep, queue_cycling.step_type);
}

// 
// When an edit box loses the input focus, reverts to the previous
// value if left blank, otherwise saves the value.
//
void CPageSetup::Killfocus(int edit_ID, int *value)
{
	CString buffer;

	// Get the window's text to check content.
	GetDlgItem(edit_ID)->GetWindowText(buffer);
	if (buffer.IsEmpty())
		// If blank revert to previous value.
		SetDlgItemInt(edit_ID, *value, FALSE);
	else
		*value = GetDlgItemInt(edit_ID);
}

/////////////////////////////////////////////////////////////////////////////
//
// The following group of functions are called by the windows framework when
// the coresponding edit boxes lose the input focus.
//
/////////////////////////////////////////////////////////////////////////////

void CPageSetup::OnKillfocusEDiskStart()
{
	Killfocus(EDiskStart, &(target_cycling.start));
}

void CPageSetup::OnKillfocusEDiskStep()
{
	Killfocus(EDiskStep, &(target_cycling.step));
}

void CPageSetup::OnKillfocusEQueueEnd()
{
	Killfocus(EQueueEnd, &(queue_cycling.end));
}

void CPageSetup::OnKillfocusEQueueStart()
{
	Killfocus(EQueueStart, &(queue_cycling.start));
}

void CPageSetup::OnKillfocusEQueueStep()
{
	Killfocus(EQueueStep, &(queue_cycling.step));
}

void CPageSetup::OnKillfocusERampTime()
{
	Killfocus(ERampTime, &ramp_time);
}

void CPageSetup::OnKillfocusERunTimeHours()
{
	Killfocus(ERunTimeHours, &hours);
}

void CPageSetup::OnKillfocusERunTimeMinutes()
{
	Killfocus(ERunTimeMinutes, &minutes);
}

void CPageSetup::OnKillfocusERunTimeSeconds()
{
	Killfocus(ERunTimeSeconds, &seconds);
}

void CPageSetup::OnKillfocusEDiskWorkerCount()
{
	Killfocus(EDiskWorkerCount, &disk_worker_count);
}

void CPageSetup::OnKillfocusENetWorkerCount()
{
	Killfocus(ENetWorkerCount, &net_worker_count);
}

void CPageSetup::OnKillfocusEWorkerStart()
{
	Killfocus(EWorkerStart, &(worker_cycling.start));
}

void CPageSetup::OnKillfocusEWorkerStep()
{
	Killfocus(EWorkerStep, &(worker_cycling.step));
}

void CPageSetup::OnSelchangeCResultType()
{
	result_type = m_CResultType.GetCurSel();
}

//
// The desired test cycling type has changed.  Enable/disable the
// display appropriately.
//
void CPageSetup::OnSelchangeCTestType()
{
	test_type = m_CTestType.GetCurSel();

	// Determine if a particular cycling type is selected.
	worker_cycling.selected = (test_type == CyclingWorkers || test_type == CyclingWorkersTargets);
	target_cycling.selected = (test_type == CyclingTargets || test_type == CyclingWorkersTargets ||
				   test_type == CyclingIncTargetsParallel || test_type == CyclingIncTargetsSerial ||
				   test_type == CyclingQueueTargets);
	queue_cycling.selected = (test_type == CyclingQueue || test_type == CyclingQueueTargets);

	// Enable/disable the display.
	EnableCyclingInfo(&worker_cycling, worker_cycling.selected);
	EnableCyclingInfo(&target_cycling, target_cycling.selected);
	EnableCyclingInfo(&queue_cycling, queue_cycling.selected);

	// If queue depth cycling is enabled, disable the target's queue depth and
	// vice-versa.  Do not enable the queue depth if there are no managers logged
	// in.
	theApp.pView->m_pPageDisk->m_EQueueDepth.EnableWindow(!queue_cycling.selected &&
							      theApp.manager_list.ManagerCount());
	theApp.pView->m_pPageDisk->m_EQueueDepth.RedrawWindow();
}

void CPageSetup::OnRDiskSpawnCPUs()
{
	m_EDiskWorkerCount.EnableWindow(FALSE);
	disk_worker_count = -1;
}

void CPageSetup::OnRDiskSpawnUser()
{
	m_EDiskWorkerCount.EnableWindow(TRUE);
	disk_worker_count = GetDlgItemInt(EDiskWorkerCount, NULL, FALSE);
}

void CPageSetup::OnRNetSpawnCPUs()
{
	m_ENetWorkerCount.EnableWindow(FALSE);
	net_worker_count = -1;
}

void CPageSetup::OnRNetSpawnUser()
{
	m_ENetWorkerCount.EnableWindow(TRUE);
	net_worker_count = GetDlgItemInt(ENetWorkerCount, NULL, FALSE);
}
