/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Iometer / PageDisplay.h                                            ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Interface for the CPageDisplay  class, which is       ## */
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
/* ##               - added new header info.                              ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef PAGEDISPLAY_DEFINED
#define PAGEDISPLAY_DEFINED

/////////////////////////////////////////////////////////////////////////////
// CPageDisplay dialog

#include "GalileoApp.h"

#define	DISPLAY_TIMER				IDD_DISPLAY
#define NUM_STATUS_BARS				6
#define NUM_UPDATE_TIMES			11

// Types of results which may be displayed.
#define ERROR_RESULT				0
#define READ_ERROR_RESULT			1
#define WRITE_ERROR_RESULT			2

#define IOPS_RESULT				3
#define	READ_IOPS_RESULT			4
#define WRITE_IOPS_RESULT			5

#define MBPS_RESULT				6
#define READ_MBPS_RESULT			7
#define WRITE_MBPS_RESULT			8

#define CONNECTION_PER_SEC_RESULT		9
#define TRANSACTION_PER_SEC_RESULT		10

#define MAX_LATENCY_RESULT			11
#define MAX_READ_LATENCY_RESULT			12
#define MAX_WRITE_LATENCY_RESULT		13
#define MAX_TRANSACTION_LATENCY_RESULT		14
#define MAX_CONNECTION_LATENCY_RESULT		15

#define AVG_LATENCY_RESULT			16
#define AVG_READ_LATENCY_RESULT			17
#define AVG_WRITE_LATENCY_RESULT		18
#define AVG_TRANSACTION_LATENCY_RESULT		19
#define AVG_CONNECTION_LATENCY_RESULT		20

#define CPU_UTILIZATION_RESULT			21
#define USER_UTILIZATION_RESULT			22
#define PRIVILEGED_UTILIZATION_RESULT		23
#define DPC_UTILIZATION_RESULT			24
#define IRQ_UTILIZATION_RESULT			25

#define CPU_INTERRUPT_RESULT			26

#define CPU_EFFECTIVENESS_RESULT		27

#define NI_PACKET_RESULT			28
#define NI_ERROR_RESULT				29
#define TCP_SEGMENT_RESULT			30

#define NUM_RESULTS				31

// Submenu codes for the IDR_POPUP_DISPLAY_LIST pop-up menu
enum {
	MDisplayOpsSubmenuID = 0,
	MDisplayMBsSubmenuID,
	MDisplayAvgSubmenuID,
	MDisplayMaxSubmenuID,
	MDisplayCPUSubmenuID,
	MDisplayNetSubmenuID,
	MDisplayErrSubmenuID
};

const int MDisplayNumSubmenus = 7;

class CPageDisplay:public CPropertyPage {
	DECLARE_DYNCREATE(CPageDisplay)
	    // CBigMeter needs access to many of CPageDisplay's private members
	friend class CBigMeter;

// Construction
      public:
	 CPageDisplay();
	~CPageDisplay();
	void Update();		// Updates the display using with the current results.
	UINT GetUpdateDelay();	// Reports the update delay in milliseconds based on the slider position.
	void SetUpdateDelay(UINT new_delay);	// Sets the slider based on the update delay in milliseconds.
	int GetWhichPerf();	// Reports the current setting of the WholeTest / StartOfTest control.
	void SetWhichPerf(int which_perf);	// Sets the WholeTest/StartOfTest control.
	void Initialize();	// Set the display to an initial state.
	void Reset();		// Resets the display.
	void ClearCheckButton();
	void SetResultSource(int manager_index, int worker_index, CPoint point);	// to any worker or manager.
	void SetResultSource(int manager_index, int worker_index, int selected_progress_bar);
	void OnMDisplay(int submenu_id, int menu_item, int result_code);	// Handle a menu item hit
	void ShowBigMeter(int performance_bar);	// Show the big performance meter.
	BOOL SaveConfig(ostream & outfile);	// Save current settings to a file.
	BOOL LoadConfig(const CString & infilename);	// Load current settings from a file.
	void SelectStatisticByName(int button, const char *statistic_name);
	void SelectManagerByName(int button, const char *mgr_name, const int mgr_id);
	void SelectWorkerByName(int button, const char *wkr_name, const int wkr_id);
	void SelectWorkerViewItem(const int bar_number);
// Dialog Data
	//{{AFX_DATA(CPageDisplay)
	enum { IDD = IDD_DISPLAY };
	CSliderCtrl m_SUpdateDelay;
	CProgressCtrl m_PRate1;
	CProgressCtrl m_PRate2;
	CProgressCtrl m_PRate3;
	CProgressCtrl m_PRate4;
	CProgressCtrl m_PRate5;
	CProgressCtrl m_PRate6;
	//}}AFX_DATA

	// Big Meter dialog box
	CBigMeter m_dlgBigMeter;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPageDisplay)
      protected:
	 virtual void DoDataExchange(CDataExchange * pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
      protected:
	// Generated message map functions
	//{{AFX_MSG(CPageDisplay)
	 virtual BOOL OnInitDialog();	// Inits everything.
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar * pScrollBar);	// Update the display when
	afx_msg void OnRAvgLastUpdate();
	afx_msg void OnRAvgWholeTest();
	//}}AFX_MSG
	 DECLARE_MESSAGE_MAP()
	    // The OnBResultType[1..6]() functions are called in response to a click on one of the result
	    // type selection buttons.  We need 6 separate functions since a button press handler does
	    // not receive any parameters indicating which button was pressed.  Each one just calls 
	    // OnBResultType() to pop up the result type menu, with a numeric parameter indicating 
	    // which button was pressed.
	afx_msg void OnBResultType1() {
		OnBResultType(0);
	} afx_msg void OnBResultType2() {
		OnBResultType(1);
	}
	afx_msg void OnBResultType3() {
		OnBResultType(2);
	}
	afx_msg void OnBResultType4() {
		OnBResultType(3);
	}
	afx_msg void OnBResultType5() {
		OnBResultType(4);
	}
	afx_msg void OnBResultType6() {
		OnBResultType(5);
	}

	// Inline BBigMeterX button click handlers
	afx_msg void OnBBigMeter1() {
		ShowBigMeter(0);
	}
	afx_msg void OnBBigMeter2() {
		ShowBigMeter(1);
	}
	afx_msg void OnBBigMeter3() {
		ShowBigMeter(2);
	}
	afx_msg void OnBBigMeter4() {
		ShowBigMeter(3);
	}
	afx_msg void OnBBigMeter5() {
		ShowBigMeter(4);
	}
	afx_msg void OnBBigMeter6() {
		ShowBigMeter(5);
	}

	// Called when user clicks on worker/manager name above each result bar.
	afx_msg void OnTWorker1() {
		SelectWorkerViewItem(0);
	}
	afx_msg void OnTWorker2() {
		SelectWorkerViewItem(1);
	}
	afx_msg void OnTWorker3() {
		SelectWorkerViewItem(2);
	}
	afx_msg void OnTWorker4() {
		SelectWorkerViewItem(3);
	}
	afx_msg void OnTWorker5() {
		SelectWorkerViewItem(4);
	}
	afx_msg void OnTWorker6() {
		SelectWorkerViewItem(5);
	}

	void OnBResultType(int which_button);
	unsigned int GetMaxRange(Results * results, int result_type);
	unsigned int ReportMaxRange(double max_range_needed);

	// Returns the appropriate result value in a double and a CString
	BOOL GetDisplayData(Results * results, int result_type, double *result_value, CString * result_text);

      private:

	struct {
		int result_to_display;
		char *name;
		Results *results;	// Pointer to base of results[MAX_UPDATE] array
		Manager *manager;
		Worker *worker;
	} barcharts[NUM_STATUS_BARS];

	int selected_button;

	UINT delay_table[NUM_UPDATE_TIMES];
};

#endif
