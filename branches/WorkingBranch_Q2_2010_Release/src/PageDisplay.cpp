/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Iometer / PageDisplay.cpp                                          ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implementation of the CPageDisplay class, which is    ## */
/* ##               responsible for the "Results Display" tab in          ## */
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
#include "PageDisplay.h"
#include "ManagerList.h"
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
// CPageDisplay property page

IMPLEMENT_DYNCREATE(CPageDisplay, CPropertyPage)
//
// Default constructor.
CPageDisplay::CPageDisplay():CPropertyPage(CPageDisplay::IDD)
{
	//{{AFX_DATA_INIT(CPageDisplay)
	//}}AFX_DATA_INIT

	// Fill in conversion table of delay (ms) value for each setting of m_SUpdateDelay control
	delay_table[0] = 1000;
	delay_table[1] = 2000;
	delay_table[2] = 3000;
	delay_table[3] = 4000;
	delay_table[4] = 5000;
	delay_table[5] = 10000;
	delay_table[6] = 15000;
	delay_table[7] = 30000;
	delay_table[8] = 45000;
	delay_table[9] = 60000;
	delay_table[10] = 0;
}

CPageDisplay::~CPageDisplay()
{
}

void CPageDisplay::DoDataExchange(CDataExchange * pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPageDisplay)
	DDX_Control(pDX, PRate1, m_PRate1);
	DDX_Control(pDX, PRate2, m_PRate2);
	DDX_Control(pDX, PRate3, m_PRate3);
	DDX_Control(pDX, PRate4, m_PRate4);
	DDX_Control(pDX, PRate5, m_PRate5);
	DDX_Control(pDX, PRate6, m_PRate6);
	DDX_Control(pDX, SUpdateDelay, m_SUpdateDelay);
	//}}AFX_DATA_MAP
}

BOOL CPageDisplay::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// Init all display thingies.
	m_SUpdateDelay.SetRange(0, (NUM_UPDATE_TIMES - 1));
	m_SUpdateDelay.SetPageSize(1);
	Initialize();		// Initialize most everything.

	return TRUE;		// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CPageDisplay, CPropertyPage)
    //{{AFX_MSG_MAP(CPageDisplay)
    ON_WM_HSCROLL()
    ON_BN_CLICKED(RAvgLastUpdate, OnRAvgLastUpdate)
    ON_BN_CLICKED(RAvgWholeTest, OnRAvgWholeTest)
    ON_BN_CLICKED(TWorker1, OnTWorker1)
    ON_BN_CLICKED(TWorker2, OnTWorker2)
    ON_BN_CLICKED(TWorker3, OnTWorker3)
    ON_BN_CLICKED(TWorker4, OnTWorker4)
    ON_BN_CLICKED(TWorker5, OnTWorker5)
    ON_BN_CLICKED(TWorker6, OnTWorker6)
    ON_BN_CLICKED(BResultType1, OnBResultType1)
    ON_BN_CLICKED(BResultType2, OnBResultType2)
    ON_BN_CLICKED(BResultType3, OnBResultType3)
    ON_BN_CLICKED(BResultType4, OnBResultType4)
    ON_BN_CLICKED(BResultType5, OnBResultType5)
    ON_BN_CLICKED(BResultType6, OnBResultType6)
    ON_BN_CLICKED(BBigMeter1, OnBBigMeter1)
    ON_BN_CLICKED(BBigMeter2, OnBBigMeter2)
    ON_BN_CLICKED(BBigMeter3, OnBBigMeter3)
    ON_BN_CLICKED(BBigMeter4, OnBBigMeter4)
    ON_BN_CLICKED(BBigMeter5, OnBBigMeter5)
ON_BN_CLICKED(BBigMeter6, OnBBigMeter6)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPageDisplay message handlers
void CPageDisplay::Initialize()
{
	// Set all of the status bars to all managers.
	for (int i = 0; i < NUM_STATUS_BARS; i++) {
		barcharts[i].results = theApp.manager_list.results;
		barcharts[i].name = theApp.manager_list.name;
		barcharts[i].manager = NULL;
		barcharts[i].worker = NULL;
	}

	// Init all display thingies.
	selected_button = 0;
	theApp.pView->OnMDisplay(MDisplayOpsSubmenuID, MDisplayOpsIOPS);

	selected_button = 1;
	theApp.pView->OnMDisplay(MDisplayMBsSubmenuID, MDisplayMBsMBPSDec);

	selected_button = 2;
	theApp.pView->OnMDisplay(MDisplayAvgSubmenuID, MDisplayAvgIO);

	selected_button = 3;
	theApp.pView->OnMDisplay(MDisplayMaxSubmenuID, MDisplayMaxIO);

	selected_button = 4;
	theApp.pView->OnMDisplay(MDisplayCPUSubmenuID, MDisplayCPUUtilization);

	selected_button = 5;
	theApp.pView->OnMDisplay(MDisplayErrSubmenuID, MDisplayErrIO);

	SetUpdateDelay(0);

	SetWhichPerf(WHOLE_TEST_PERF);

	Reset();
}

//
// Resetting the result display.  This is usually called because a manager or worker has
// been deleted.  Any references to the deleted item need to be removed.
//
void CPageDisplay::Reset()
{
	// Set all of the status bars to all managers.
	for (int i = 0; i < NUM_STATUS_BARS; i++) {
		barcharts[i].results = theApp.manager_list.results;
		barcharts[i].name = theApp.manager_list.name;
		barcharts[i].manager = NULL;
		barcharts[i].worker = NULL;
	}

	// Update the display information.
	m_PRate1.SetPos(0);
	m_PRate2.SetPos(0);
	m_PRate3.SetPos(0);
	m_PRate4.SetPos(0);
	m_PRate5.SetPos(0);
	m_PRate6.SetPos(0);

	// Do one update to set everything properly.
	Update();
}

//
// Sets the source for a progress bar given for a drag-drop selection.
//
void CPageDisplay::SetResultSource(int manager_index, int worker_index, CPoint point)
{
	void *drop_target;

	// Get the pointer of the window dropped on.
	drop_target = (void *)WindowFromPoint(point);

	int selected_progress_bar = IOERROR;	// init index.

	for (int i = 0; i < NUM_STATUS_BARS; i++) {
		// Check to see if the drop target is one of the progress bars or Result Type buttons.
		if ((drop_target == (void *)GetDlgItem(PRate1 + i)) ||
		    (drop_target == (void *)GetDlgItem(BResultType1 + i))) {
			// If so, set the index for future use.
			selected_progress_bar = i;
			break;
		}
	}

	// The drop target was not a progress bar or Result Type button, so exit this function.
	if (selected_progress_bar == IOERROR)
		return;

	// Call SetResultSource( int, int, int ) to do the rest of the job
	SetResultSource(manager_index, worker_index, selected_progress_bar);
}

//
// Sets the source for a progress bar given for a specific progress bar.
//
void CPageDisplay::SetResultSource(int manager_index, int worker_index, int selected_progress_bar)
{
	if ((selected_progress_bar < 0) || (selected_progress_bar >= NUM_STATUS_BARS)) {
		ErrorMessage("Invalid selected_progress_bar value in CPageDisplay::SetResultSource()");
		return;
	}

	if (manager_index == ALL_MANAGERS)	// looking for all manager results
	{
		barcharts[selected_progress_bar].results = theApp.manager_list.results;
		barcharts[selected_progress_bar].name = theApp.manager_list.name;
		barcharts[selected_progress_bar].manager = NULL;
		barcharts[selected_progress_bar].worker = NULL;
	} else if (worker_index == IOERROR)	// looking for a manager's results
	{
		barcharts[selected_progress_bar].results = theApp.manager_list.GetManager(manager_index)->results;
		barcharts[selected_progress_bar].name = theApp.manager_list.GetManager(manager_index)->name;
		barcharts[selected_progress_bar].manager = theApp.manager_list.GetManager(manager_index);
		barcharts[selected_progress_bar].worker = NULL;
	} else			// looking for a worker's results
	{
		barcharts[selected_progress_bar].results =
		    theApp.manager_list.GetManager(manager_index)->GetWorker(worker_index)->results;
		barcharts[selected_progress_bar].name =
		    theApp.manager_list.GetManager(manager_index)->GetWorker(worker_index)->name;
		barcharts[selected_progress_bar].manager = theApp.manager_list.GetManager(manager_index);
		barcharts[selected_progress_bar].worker =
		    theApp.manager_list.GetManager(manager_index)->GetWorker(worker_index);
	}

	// Updating the rates.
	Update();
}

void CPageDisplay::Update()
{
	int max_rate[NUM_STATUS_BARS];
	int i, j;
	CString max_range_text;
	double stat_double;
	CString stat_string;

	// Setting ranges on status bar display based on selections.
	for (i = 0; i < NUM_STATUS_BARS; i++) {
		max_rate[i] = GetMaxRange(&(barcharts[i].results[GetWhichPerf()]), barcharts[i].result_to_display);
	}

	// If the results to display are the same, giving them the same ranges for easier comparisons.
	for (i = 0; i < NUM_STATUS_BARS - 1; i++) {
		for (j = i; j < NUM_STATUS_BARS; j++) {
			if (barcharts[i].result_to_display == barcharts[j].result_to_display) {
				if (max_rate[i] > max_rate[j])
					max_rate[j] = max_rate[i];
				else
					max_rate[i] = max_rate[j];
			}
		}
	}

	// Setting ranges on status bar and displaying range value.
	for (i = 0; i < NUM_STATUS_BARS; i++) {
		((CProgressCtrl *) GetDlgItem(PRate1 + i))->SetRange(0, max_rate[i]);

		// See if results are displaying a percentage.
		if ((barcharts[i].result_to_display >= CPU_UTILIZATION_RESULT) &&
		    (barcharts[i].result_to_display <= IRQ_UTILIZATION_RESULT)) {
			max_range_text.Format("%d %%", max_rate[i]);	// display % sign
			((CStatic *) GetDlgItem(TRate1MAX + i))->SetWindowText(max_range_text);
		} else {
			max_range_text.Format("%d", max_rate[i]);	// displaying a rate
			((CStatic *) GetDlgItem(TRate1MAX + i))->SetWindowText(max_range_text);
		}

		// Update the name.
		SetDlgItemText(TWorker1 + i, barcharts[i].name);

		// Get the performance data to display (a number and a string)
		GetDisplayData(&(barcharts[i].results[GetWhichPerf()]),
			       barcharts[i].result_to_display, &stat_double, &stat_string);

		// Update the appropriate progress bar
		((CProgressCtrl *) GetDlgItem(PRate1 + i))->SetPos((int)stat_double);

		// Display the new value to the user
		((CStatic *) GetDlgItem(TRate1 + i))->SetWindowText(stat_string);
	}

	// Update the data.
	UpdateWindow();

	// Update the big meter dialog, if it exists.
	if (m_dlgBigMeter.is_displayed)
		m_dlgBigMeter.UpdateDisplay();
}

//
// Get Maximum Range:
//
// Finds and returns the maximum range that a status bar should be set to based on
// what data is to be displayed.
//
unsigned int CPageDisplay::GetMaxRange(Results * results, int result_type)
{
	switch (result_type) {
	case MBPS_BIN_RESULT:
		return ReportMaxRange((double)results->MBps_Bin);
		break;
	case READ_MBPS_BIN_RESULT:
		return ReportMaxRange((double)results->read_MBps_Bin);
		break;
	case WRITE_MBPS_BIN_RESULT:
		return ReportMaxRange((double)results->write_MBps_Bin);
		break;
	case MBPS_DEC_RESULT:
		return ReportMaxRange((double)results->MBps_Dec);
		break;
	case READ_MBPS_DEC_RESULT:
		return ReportMaxRange((double)results->read_MBps_Dec);
		break;
	case WRITE_MBPS_DEC_RESULT:
		return ReportMaxRange((double)results->write_MBps_Dec);
		break;

	case IOPS_RESULT:
		return ReportMaxRange((double)results->IOps);
		break;
	case READ_IOPS_RESULT:
		return ReportMaxRange((double)results->read_IOps);
		break;
	case WRITE_IOPS_RESULT:
		return ReportMaxRange((double)results->write_IOps);
		break;

	case CONNECTION_PER_SEC_RESULT:
		return ReportMaxRange((double)results->connections_per_second);
		break;
	case AVG_CONNECTION_LATENCY_RESULT:
		return ReportMaxRange((double)results->ave_connection_latency);
		break;
	case MAX_CONNECTION_LATENCY_RESULT:
		return ReportMaxRange((double)results->max_connection_latency);
		break;

	case MAX_LATENCY_RESULT:
		return ReportMaxRange((double)results->max_latency);
		break;
	case MAX_READ_LATENCY_RESULT:
		return ReportMaxRange((double)results->max_read_latency);
		break;
	case MAX_WRITE_LATENCY_RESULT:
		return ReportMaxRange((double)results->max_write_latency);
		break;

	case AVG_LATENCY_RESULT:
		return ReportMaxRange((double)results->ave_latency);
		break;
	case AVG_READ_LATENCY_RESULT:
		return ReportMaxRange((double)results->ave_read_latency);
		break;
	case AVG_WRITE_LATENCY_RESULT:
		return ReportMaxRange((double)results->ave_write_latency);
		break;

	case TRANSACTION_PER_SEC_RESULT:
		return ReportMaxRange((double)results->transactions_per_second);
		break;
	case MAX_TRANSACTION_LATENCY_RESULT:
		return ReportMaxRange((double)results->max_transaction_latency);
		break;
	case AVG_TRANSACTION_LATENCY_RESULT:
		return ReportMaxRange((double)results->ave_transaction_latency);
		break;

	case ERROR_RESULT:
		return ReportMaxRange((double)results->total_errors);
		break;
	case READ_ERROR_RESULT:
		return ReportMaxRange((double)results->raw.read_errors);
		break;
	case WRITE_ERROR_RESULT:
		return ReportMaxRange((double)results->raw.write_errors);
		break;

	case CPU_UTILIZATION_RESULT:
		return ReportMaxRange((double)results->CPU_utilization[CPU_TOTAL_UTILIZATION]);
		break;
	case USER_UTILIZATION_RESULT:
		return ReportMaxRange((double)results->CPU_utilization[CPU_USER_UTILIZATION]);
		break;
	case PRIVILEGED_UTILIZATION_RESULT:
		return ReportMaxRange((double)results->CPU_utilization[CPU_PRIVILEGED_UTILIZATION]);
		break;
	case DPC_UTILIZATION_RESULT:
		return ReportMaxRange((double)results->CPU_utilization[CPU_DPC_UTILIZATION]);
		break;
	case IRQ_UTILIZATION_RESULT:
		return ReportMaxRange((double)results->CPU_utilization[CPU_IRQ_UTILIZATION]);
		break;

	case CPU_INTERRUPT_RESULT:
		return ReportMaxRange((double)results->CPU_utilization[CPU_IRQ]);
		break;

	case CPU_EFFECTIVENESS_RESULT:
		return ReportMaxRange((double)results->CPU_effectiveness);
		break;

	case NI_PACKET_RESULT:
		return ReportMaxRange((double)results->ni_statistics[NI_PACKETS]);
		break;
	case NI_ERROR_RESULT:
		return ReportMaxRange((double)results->ni_statistics[NI_ERRORS]);
		break;
	case TCP_SEGMENT_RESULT:
		return ReportMaxRange((double)results->tcp_statistics[TCP_SEGMENTS_RESENT]);
		break;

	default:
		return 0;
	}
}

//
// Report Maximum Range:
// 
// Returns the value that the maximum range for a status bar should be set to based 
// on what's needed.
//
unsigned int CPageDisplay::ReportMaxRange(double max_range_needed)
{
	double max_rate = (double)10;	// Start the max range at 1.0,

	while (max_rate < max_range_needed)	// and continue to grow it in increments
	{			// of 10x until the need is met.
		if (max_rate * (double)10 < max_rate)	// Trap overflow.
			break;
		max_rate *= (double)10;
	}

	// The maximum range for the progress bar is 64K.
	if (max_rate > 60000)
		max_rate = 60000;

	return (unsigned int)max_rate;
}

//
// Pass in a results structure and the ID# of the statistic you want.
//
// GetDisplayData will fill in these values:
//              result_value - the performance number (64.566 IOps) requested
//              result_text - the user-friendly text version of that result
//
BOOL CPageDisplay::GetDisplayData(Results * results, int result_type, double *result_value, CString * result_text)
{
	switch (result_type) {
	case MBPS_BIN_RESULT:
		*result_value = results->MBps_Bin;
		result_text->Format("%.2f", results->MBps_Bin);
		break;
	case READ_MBPS_BIN_RESULT:
		*result_value = results->read_MBps_Bin;
		result_text->Format("%.2f", results->read_MBps_Bin);
		break;
	case WRITE_MBPS_BIN_RESULT:
		*result_value = results->write_MBps_Bin;
		result_text->Format("%.2f", results->write_MBps_Bin);
		break;
	case MBPS_DEC_RESULT:
		*result_value = results->MBps_Dec;
		result_text->Format("%.2f", results->MBps_Dec);
		break;
	case READ_MBPS_DEC_RESULT:
		*result_value = results->read_MBps_Dec;
		result_text->Format("%.2f", results->read_MBps_Dec);
		break;
	case WRITE_MBPS_DEC_RESULT:
		*result_value = results->write_MBps_Dec;
		result_text->Format("%.2f", results->write_MBps_Dec);
		break;

	case IOPS_RESULT:
		*result_value = results->IOps;
		result_text->Format("%.2f", results->IOps);
		break;
	case READ_IOPS_RESULT:
		*result_value = results->read_IOps;
		result_text->Format("%.2f", results->read_IOps);
		break;
	case WRITE_IOPS_RESULT:
		*result_value = results->write_IOps;
		result_text->Format("%.2f", results->write_IOps);
		break;

	case CONNECTION_PER_SEC_RESULT:
		*result_value = results->connections_per_second;
		result_text->Format("%.2f", results->connections_per_second);
		break;
	case AVG_CONNECTION_LATENCY_RESULT:
		*result_value = results->ave_connection_latency;
		result_text->Format("%.4f", results->ave_connection_latency);
		break;
	case MAX_CONNECTION_LATENCY_RESULT:
		*result_value = results->max_connection_latency;
		result_text->Format("%.4f", results->max_connection_latency);
		break;

	case MAX_LATENCY_RESULT:
		*result_value = results->max_latency;
		result_text->Format("%.4f", results->max_latency);
		break;
	case MAX_READ_LATENCY_RESULT:
		*result_value = results->max_read_latency;
		result_text->Format("%.4f", results->max_read_latency);
		break;
	case MAX_WRITE_LATENCY_RESULT:
		*result_value = results->max_write_latency;
		result_text->Format("%.4f", results->max_write_latency);
		break;

	case AVG_LATENCY_RESULT:
		*result_value = results->ave_latency;
		result_text->Format("%.4f", results->ave_latency);
		break;
	case AVG_READ_LATENCY_RESULT:
		*result_value = results->ave_read_latency;
		result_text->Format("%.4f", results->ave_read_latency);
		break;
	case AVG_WRITE_LATENCY_RESULT:
		*result_value = results->ave_write_latency;
		result_text->Format("%.4f", results->ave_write_latency);
		break;

	case TRANSACTION_PER_SEC_RESULT:
		*result_value = results->transactions_per_second;
		result_text->Format("%.2f", results->transactions_per_second);
		break;
	case MAX_TRANSACTION_LATENCY_RESULT:
		*result_value = results->max_transaction_latency;
		result_text->Format("%.4f", results->max_transaction_latency);
		break;
	case AVG_TRANSACTION_LATENCY_RESULT:
		*result_value = results->ave_transaction_latency;
		result_text->Format("%.4f", results->ave_transaction_latency);
		break;

	case ERROR_RESULT:
		*result_value = results->total_errors;
		result_text->Format("%d", results->total_errors);
		break;
	case READ_ERROR_RESULT:
		*result_value = results->raw.read_errors;
		result_text->Format("%d", results->raw.read_errors);
		break;
	case WRITE_ERROR_RESULT:
		*result_value = results->raw.write_errors;
		result_text->Format("%d", results->raw.write_errors);
		break;

	case CPU_UTILIZATION_RESULT:
		*result_value = results->CPU_utilization[CPU_TOTAL_UTILIZATION];
		result_text->Format("%.2f %%", results->CPU_utilization[CPU_TOTAL_UTILIZATION]);
		break;
	case USER_UTILIZATION_RESULT:
		*result_value = results->CPU_utilization[CPU_USER_UTILIZATION];
		result_text->Format("%.2f %%", results->CPU_utilization[CPU_USER_UTILIZATION]);
		break;
	case PRIVILEGED_UTILIZATION_RESULT:
		*result_value = results->CPU_utilization[CPU_PRIVILEGED_UTILIZATION];
		result_text->Format("%.2f %%", results->CPU_utilization[CPU_PRIVILEGED_UTILIZATION]);
		break;
	case DPC_UTILIZATION_RESULT:
		*result_value = results->CPU_utilization[CPU_DPC_UTILIZATION];
		result_text->Format("%.2f %%", results->CPU_utilization[CPU_DPC_UTILIZATION]);
		break;
	case IRQ_UTILIZATION_RESULT:
		*result_value = results->CPU_utilization[CPU_IRQ_UTILIZATION];
		result_text->Format("%.2f %%", results->CPU_utilization[CPU_IRQ_UTILIZATION]);
		break;

	case CPU_INTERRUPT_RESULT:
		*result_value = results->CPU_utilization[CPU_IRQ];
		result_text->Format("%.2f", results->CPU_utilization[CPU_IRQ]);
		break;

	case CPU_EFFECTIVENESS_RESULT:
		*result_value = results->CPU_effectiveness;
		result_text->Format("%.2f", results->CPU_effectiveness);
		break;

	case NI_PACKET_RESULT:
		*result_value = results->ni_statistics[NI_PACKETS];
		result_text->Format("%.2f", results->ni_statistics[NI_PACKETS]);
		break;
	case NI_ERROR_RESULT:
		*result_value = results->ni_statistics[NI_ERRORS];
		result_text->Format("%.2f", results->ni_statistics[NI_ERRORS]);
		break;
	case TCP_SEGMENT_RESULT:
		*result_value = results->tcp_statistics[TCP_SEGMENTS_RESENT];
		result_text->Format("%.2f", results->tcp_statistics[TCP_SEGMENTS_RESENT]);
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

UINT CPageDisplay::GetUpdateDelay()
{
	// We know that the slider value will be in the range 0..(NUM_UPDATE_TIMES - 1)
	return (delay_table[m_SUpdateDelay.GetPos()]);
}

void CPageDisplay::SetUpdateDelay(UINT new_delay)
{
	int i;
	int new_setting = (NUM_UPDATE_TIMES - 1);	// Default if new_delay is not found in delay_table

	for (i = 0; i < NUM_UPDATE_TIMES; i++) {
		if ((new_delay * 1000) == delay_table[i]) {
			new_setting = i;
			break;
		}
	}

	m_SUpdateDelay.SetPos(new_setting);
}

int CPageDisplay::GetWhichPerf()
{
	if (GetCheckedRadioButton(RAvgWholeTest, RAvgLastUpdate) == RAvgLastUpdate) {
		return LAST_UPDATE_PERF;
	} else {
		return WHOLE_TEST_PERF;
	}
}

void CPageDisplay::SetWhichPerf(int which_perf)
{
	if (which_perf == LAST_UPDATE_PERF) {
		CheckRadioButton(RAvgWholeTest, RAvgLastUpdate, RAvgLastUpdate);
	} else {
		CheckRadioButton(RAvgWholeTest, RAvgLastUpdate, RAvgWholeTest);
	}
}

void CPageDisplay::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar * pScrollBar)
{
	// Treat moving the update frequency scroll bar as if the timer had expired.
	if (pScrollBar == (CScrollBar *) & m_SUpdateDelay && nSBCode == SB_ENDSCROLL &&
	    (theApp.test_state == TestRecording)) {
		theApp.pView->SetTimer(DISPLAY_TIMER, 0, NULL);
	}
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

//
// Generic handler for press on any BResultType button.  
// Records which button was pressed, then pops up the result type menu.
// 
void CPageDisplay::OnBResultType(int which_button)
{
	CMenu menu;
	CRect button_rect;
	RECT screen_rect;
	int menu_height, menu_x, menu_y;

	// Record which button was pressed, for later use by OnMDisplay().
	selected_button = which_button;

	// This prevents the menu from popping up if the user is "unchecking" the button.
	if (((CButton *) GetDlgItem(BResultType1 + selected_button))->GetCheck()) {
		((CButton *) GetDlgItem(BResultType1 + selected_button))->SetCheck(FALSE);
		return;
	}
	// Create the popup menu from the resource.
	VERIFY(menu.LoadMenu(IDR_POPUP_DISPLAY_LIST));
	CMenu *pPopup = menu.GetSubMenu(0);

	ASSERT(pPopup != NULL);

	// Find the first non-child window to be the popup's parent.
	CWnd *pWndPopupOwner = this;

	while (pWndPopupOwner->GetStyle() & WS_CHILD)
		pWndPopupOwner = pWndPopupOwner->GetParent();

	// Get the screen coordinates of the button that was pressed.
	GetDlgItem(BResultType1 + selected_button)->GetWindowRect(&button_rect);

	// Position the menu with its upper left corner at the lower left corner of the button.
	menu_x = button_rect.TopLeft().x;
	menu_y = button_rect.BottomRight().y;

	// If the menu would go off the bottom of the screen, make it go *up* from the button instead.
	menu_height = GetSystemMetrics(SM_CYMENUSIZE) * MDisplayNumSubmenus;
	if (SystemParametersInfo(SPI_GETWORKAREA, 0, &screen_rect, 0) && (menu_y + menu_height > screen_rect.bottom)) {
		menu_y = button_rect.TopLeft().y - menu_height + 1;
	}
	// Set the button's visual state to "pressed" as long as the menu is popped up.
	((CButton *) GetDlgItem(BResultType1 + selected_button))->SetCheck(TRUE);

	// Pop up the menu.
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, menu_x, menu_y, pWndPopupOwner);

	// The user's selection (if any) from the pop-up menu will result in a call to one of the 
	// CGalileoView::OnMDisplay...() functions, which will in turn call CPageDisplay::OnMDisplay().

	// Set the button's visual state to "not pressed" after the menu is dismissed.
	((CButton *) GetDlgItem(BResultType1 + selected_button))->SetState(FALSE);
}

//
// Generic handler for any selection from the IDR_POPUP_DISPLAY_LIST menu.  
// (The specific handlers for each selection are members of the CGalileoView class;
// they each call this function.)  Its main purpose is to set the result_to_display
// member variable.
//
// This function assumes that the member variable selected_button is set to 
// indicate the button that was pressed to pop up the menu.  This is typically done
// by the function OnBResultType(), which popped up the menu.
//
// The parameters submenu_id and menu_item are resource ID's, used to get the
// menu item string for the button label.  The parameter result_code must be one 
// of the *_RESULT constants from PageDisplay.h, and determines which result will
// be displayed by the status bar associated with the selected_button.
// 
void CPageDisplay::OnMDisplay(int submenu_id, int menu_item, int result_code)
{
	if ((selected_button < 0) || (selected_button >= NUM_STATUS_BARS)) {
		ErrorMessage("Invalid selected_button value in CPageDisplay::OnMDisplay()");
		return;
	}
	if ((submenu_id < 0) || (submenu_id >= MDisplayNumSubmenus)) {
		ErrorMessage("Invalid submenu_id value in CPageDisplay::OnMDisplay()");
		return;
	}
	if ((result_code < 0) || (result_code >= NUM_RESULTS)) {
		ErrorMessage("Invalid result_code value in CPageDisplay::OnMDisplay()");
		return;
	}
	// store the selected result for later use
	barcharts[selected_button].result_to_display = result_code;

	// get the selected menu item's title and store it in the selected button
	CMenu menu;

	VERIFY(menu.LoadMenu(IDR_POPUP_DISPLAY_LIST));

	CMenu *pPopup = menu.GetSubMenu(0);

	ASSERT(pPopup != NULL);	// we can use ASSERT here because this should work every time if it works at all

	CMenu *pSubPopup = pPopup->GetSubMenu(submenu_id);

	if (pSubPopup == NULL) {
		ErrorMessage("Could not get submenu in CPageDisplay::OnMDisplay()");
		return;
	}

	CString title;

	pSubPopup->GetMenuString(menu_item, title, MF_BYCOMMAND);

	GetDlgItem(BResultType1 + selected_button)->SetWindowText(title);
	((CButton *) GetDlgItem(BResultType1 + selected_button))->SetCheck(FALSE);

	Update();
}

//
// Find a statistic by its name (as stored in result file) and select it
// as though it had been selected from the menu.
// 
void CPageDisplay::SelectStatisticByName(int button, const char *statistic_name)
{
	unsigned int submenu_id, menu_item;

	if ((button < 0) || (button >= NUM_STATUS_BARS)) {
		ErrorMessage("Invalid button value in CPageDisplay::SelectStatisticByName()");
		return;
	}
	if (!statistic_name) {
		ErrorMessage("Invalid string in CPageDisplay::SelectStatisticByName()");
		return;
	}

	CMenu menu;

	VERIFY(menu.LoadMenu(IDR_POPUP_DISPLAY_LIST));

	CMenu *pPopup = menu.GetSubMenu(0);

	ASSERT(pPopup != NULL);	// we can use ASSERT here because this should work every time if it works at all

	for (submenu_id = 0; submenu_id < MDisplayNumSubmenus; submenu_id++) {
		CMenu *pSubPopup = pPopup->GetSubMenu(submenu_id);

		if (pSubPopup == NULL) {
			ErrorMessage("Could not get submenu in CPageDisplay::SelectStatisticByName()");
			return;
		}

		for (menu_item = 0; menu_item < pSubPopup->GetMenuItemCount(); menu_item++) {
			CString title;

			pSubPopup->GetMenuString(menu_item, title, MF_BYPOSITION);

			if (title.CompareNoCase(statistic_name) == 0) {
				selected_button = button;
				theApp.pView->OnMDisplay(submenu_id, pSubPopup->GetMenuItemID(menu_item));
				return;
			}
		}
	}
	// No match found, just leave the current selection unchanged
	return;
}

//
// Find a manager by its name (as stored in result file) and select it
// as though it had been dragged.
// 
void CPageDisplay::SelectManagerByName(int button, const char *mgr_name, const int mgr_id)
{
	Manager *mgr;

	if (!mgr_name) {
		ErrorMessage("Invalid string in CPageDisplay::SelectManagerByName()");
		return;
	}

	if (strlen(mgr_name)) {
		// The manager name is non-empty; get a pointer to the
		// first manager with the specified name
		mgr = theApp.manager_list.GetManagerByName(mgr_name, mgr_id);

		if (mgr) {
			// A manager with the specified name was found in the ManagerList; use it
			SetResultSource(mgr->GetIndex(), IOERROR, button);
		} else {
			// No manager with the specified name was found
			if (theApp.manager_list.ManagerCount() == 1) {
				// There is only one manager; use it (ignoring its name)
				SetResultSource(0, IOERROR, button);
			} else {
				// More than one manager; default to ALL_MANAGERS
				SetResultSource(ALL_MANAGERS, IOERROR, button);
			}
		}
	} else {
		// The manager name is an empty string; use ALL_MANAGERS
		SetResultSource(ALL_MANAGERS, IOERROR, button);
	}
}

//
// Find a worker by its name (as stored in result file) and select it
// as though it had been dragged.
// 
void CPageDisplay::SelectWorkerByName(int button, const char *wkr_name, const int wkr_id)
{
	Manager *mgr;
	Worker *wkr;

	if (!wkr_name) {
		ErrorMessage("Invalid string in CPageDisplay::SelectWorkerByName()");
		return;
	}
	// Get a pointer to the button's currently assigned manager
	mgr = barcharts[button].manager;
	if (mgr) {
		// The specified button already has a manager (probably was just assigned by 
		// the previous line in the config file, but even if not we'll use it anyway).

		if (strlen(wkr_name)) {
			// The worker name is non-empty; get a pointer to the
			// first worker with the specified name within the current manager
			wkr = mgr->GetWorkerByName(wkr_name, wkr_id);

			if (wkr) {
				// A worker with the specified name was found; use it
				SetResultSource(mgr->GetIndex(), wkr->GetIndex(), button);
			} else {
				// No worker with the specified name was found
				if (mgr->WorkerCount() == 1) {
					// There is only one worker; use it (ignoring its name)
					SetResultSource(mgr->GetIndex(), 0, button);
				} else {
					// More than one worker; default to all workers
					SetResultSource(mgr->GetIndex(), IOERROR, button);
				}
			}
		} else {
			// The worker name is an empty string; use all workers
			SetResultSource(mgr->GetIndex(), IOERROR, button);
		}
	}
	// Else: the specified button does not have a manager, ignore the specified worker name
}

//
// Called by OnTWorker1 - OnTWorker6 in response to left mouse button
// clicks on the worker/manager name above the result display bars.
//
void CPageDisplay::SelectWorkerViewItem(const int bar_number)
{
	// Highlight the manager/worker/whatever being monitored by the chosen bar.
	theApp.pView->m_pWorkerView->SelectItem(barcharts[bar_number].manager, barcharts[bar_number].worker);
}

void CPageDisplay::ClearCheckButton()
{
	CButton *button;
	CRect button_rect;
	CPoint point;

	// When displaying the popup menu, if the user clicks outside the popup and
	// not on the button that poped the popup up, Windows handles removing the
	// menu.  We have to handle unchecking the button.

	button = (CButton *) GetDlgItem(BResultType1 + selected_button);
	button->GetWindowRect(&button_rect);
	GetCursorPos(&point);

	// Note that we only reset the state of the check button if the cursor is
	// not within the button's bounding rectangle.  If the cursor is within
	// the bounding rectangle, the user clicked the button a second time.  The
	// Button handler will take care of unchecking the button, and will not
	// display the menu.  This allows the button to toggle the menu.

	if (button->GetCheck() && !button_rect.PtInRect(point))
		button->SetCheck(FALSE);

	// If the Big Meter display is open, the WM_EXITMENULOOP message may have
	// been intended for that dialog box.  Uncheck its ResultType button.

	if (m_dlgBigMeter.is_displayed)
		m_dlgBigMeter.ClearCheckButton();
}

//
// Show the big performance meter for a given performance bar.
//
void CPageDisplay::ShowBigMeter(int performance_bar)
{
	CString stat_title, worker_title;

	// Get the name of the result we are displaying from the appropriate performance bar.
	GetDlgItem(BResultType1 + performance_bar)->GetWindowText(stat_title);
	// Get the name of the worker (or manager or All Managers) being watched.
	GetDlgItem(TWorker1 + performance_bar)->GetWindowText(worker_title);

	// Create the big performance meter dialog for the given performance bar.
	m_dlgBigMeter.Create(performance_bar, (CString) "Iometer " + theApp.GetVersionString()
			     + " " + theApp.pView->m_pPageSetup->test_name, stat_title, worker_title);
}

//
// Save the current settings of the page to a file.
//
BOOL CPageDisplay::SaveConfig(ostream & outfile)
{
	int i;
	CString statistic;

	// Update memory variables with data located on GUI.
	UpdateData(TRUE);

	// Save settings for page as a whole.
	outfile << "'RESULTS DISPLAY =======================" "========================================" << endl;

	outfile << "'Update Frequency,Update Type" << endl << "\t" << GetUpdateDelay() / 1000;

	if (GetWhichPerf() == LAST_UPDATE_PERF)
		outfile << "," << "LAST_UPDATE" << endl;
	else
		outfile << "," << "WHOLE_TEST" << endl;

	// Save bar chart settings.
	for (i = 0; i < NUM_STATUS_BARS; i++) {
		// Get the name of the result we are displaying.
		GetDlgItem(BResultType1 + i)->GetWindowText(statistic);

		// Output settings for this bar chart (each setting gets a line to itself
		// because these names can contain spaces and commas)

		// Record which statistic is being monitored.
		outfile << "'Bar chart " << (i + 1) << " statistic" << endl << "\t" << (LPCTSTR) statistic << endl;

		// Is a specific manager being monitored?
		if (barcharts[i].manager) {
			outfile << "'Bar chart " << (i + 1) << " manager ID, manager name" << endl
			    << "\t" << barcharts[i].manager->id << "," << barcharts[i].manager->name << endl;

			// Is a specific worker on that manager being monitored?
			if (barcharts[i].worker) {
				outfile << "'Bar chart " << (i + 1) << " worker ID, worker name" << endl
				    << "\t" << barcharts[i].worker->id << "," << barcharts[i].worker->name << endl;
			}
		}
	}

	// Mark end of this page's information.
	outfile << "'END results display" << endl;

	return TRUE;
}

//
// Load the settings of the page from a file.
//
BOOL CPageDisplay::LoadConfig(const CString & infilename)
{
	long version;
	CString key, value;
	ICF_ifstream infile(infilename);

	version = infile.GetVersion();
	if (version == -1)
		return FALSE;
	if (!infile.SkipTo("'RESULTS DISPLAY"))
		return TRUE;	// no results display to restore (this is OK)

	while (1) {
		if (!infile.GetPair(key, value)) {
			ErrorMessage("File is improperly formatted.  Expected results "
				     "display data or \"END results display\".");
			return FALSE;
		}

		if (key.CompareNoCase("'END results display") == 0) {
			break;
		} else if (key.CompareNoCase("'Update Frequency,Update Type") == 0) {
			int update_frequency;
			CString update_type;

			if (!ICF_ifstream::ExtractFirstInt(value, update_frequency)) {
				ErrorMessage("Error while reading file.  "
					     "\"Update frequency\" should be specified as an integer value.");
				return FALSE;
			}

			update_type = value;
			update_type.TrimRight();

			SetUpdateDelay(update_frequency);

			if (update_type.CompareNoCase("LAST_UPDATE") == 0)
				SetWhichPerf(LAST_UPDATE_PERF);
			else if (update_type.CompareNoCase("WHOLE_TEST") == 0)
				SetWhichPerf(WHOLE_TEST_PERF);
			else {
				ErrorMessage("File is improperly formatted.  In RESULTS "
					     "DISPLAY section, Update Frequency was not followed by "
					     "an appropriate Update Type string.");
				return FALSE;
			}
		} else if (key.Left((int)(strlen("'Bar chart"))).CompareNoCase("'Bar chart") == 0) {
			int bar_number;
			CString bar_item;

			if (!ICF_ifstream::ExtractFirstInt(key, bar_number)) {
				ErrorMessage("Error while reading file.  "
					     "\"Bar chart\" should be followed by an integer value.");
				return FALSE;
			}

			bar_item = key;
			bar_item.TrimLeft();
			bar_item.TrimRight();

			if (bar_number < 1 || bar_number > NUM_STATUS_BARS) {
				ErrorMessage("Invalid bar chart number in RESULTS DISPLAY "
					     "section.  Ignoring this bar setting.");
				continue;
			}

			bar_number--;	// convert from one-based to zero-based index

			if (bar_item.CompareNoCase("statistic") == 0) {
				SelectStatisticByName(bar_number, value);
			} else if (bar_item.CompareNoCase("manager ID, manager name") == 0) {
				CString mgr_name;
				int mgr_id;

				if (!ICF_ifstream::ExtractFirstInt(value, mgr_id)) {
					ErrorMessage("Error while reading file.  "
						     "Expected a manager ID integer after \"manager ID, manager name\""
						     "comment in RESULTS DISPLAY section.");
					return FALSE;
				}

				mgr_name = value;

				SelectManagerByName(bar_number, mgr_name, mgr_id);
			} else if (bar_item.CompareNoCase("worker ID, worker name") == 0) {
				CString wkr_name;
				int wkr_id;

				if (!ICF_ifstream::ExtractFirstInt(value, wkr_id)) {
					ErrorMessage("Error while reading file.  "
						     "Expected a worker ID integer after \"worker ID, worker name\""
						     "comment in RESULTS DISPLAY section.");
					return FALSE;
				}

				wkr_name = value;

				SelectWorkerByName(bar_number, wkr_name, wkr_id);
			} else {
				CString str;

				str.Format("%d", bar_number);
				ErrorMessage("Invalid bar chart item name \""
					     + bar_item + "\" for bar #" + str
					     + " in RESULTS DISPLAY section.  Ignoring this bar " + "setting.");
				continue;
			}
		} else {
			ErrorMessage("File is improperly formatted.  RESULTS DISPLAY "
				     "section contained an unrecognized \"" + key + "\" comment.");
			return FALSE;
		}
	}

	infile.close();

	// Update the GUI with the values read in from the file.
	UpdateData(FALSE);

	return TRUE;
}

void CPageDisplay::OnRAvgLastUpdate()
{
	if (theApp.test_state == TestRecording)
		theApp.manager_list.UpdateResults(LAST_UPDATE_PERF);
	Update();
}

void CPageDisplay::OnRAvgWholeTest()
{
	if (theApp.test_state == TestRecording)
		theApp.manager_list.UpdateResults(WHOLE_TEST_PERF);
	Update();
}
