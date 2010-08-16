/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / PageSetup.h                                            ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Interface for the CPageSetup class, which is          ## */
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
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef	PAGESETUP_DEFINED
#define PAGESETUP_DEFINED

/////////////////////////////////////////////////////////////////////////////
// CPageSetup dialog

#include "GalileoApp.h"
#include "IOTest.h"

enum {
	CyclingTargets = 1,
	CyclingWorkers,
	CyclingIncTargetsParallel,
	CyclingIncTargetsSerial,
	CyclingWorkersTargets,
	CyclingQueue,
	CyclingQueueTargets
};

// Used to determine cycling step incrementing.
enum {
	StepLinear = 0,
	StepExponential
};

// Information needed by the page to display cycling information.
struct Cycling_Info {
	BOOL selected;		// if selected test type uses this cycling

	int start;		// starting cycle value (e.g. 1)        1-100, step 10
	int end;		// ending cycle value (e.g. 100)
	int step;		// step of cycle value (e.g. 10)
	int step_type;		// indicates linear or exponential stepping

	int step_text;		// ID of text describing stepping option

	CEdit start_box;	// GUI edit boxes for above values
	CEdit end_box;
	CEdit step_box;

	CComboBox step_sel_box;	// drop down list of stepping options
};

class CPageSetup:public CPropertyPage {
	DECLARE_DYNCREATE(CPageSetup)
// Construction
      public:
	CPageSetup();
	~CPageSetup();

	void EnableWindow(BOOL enable);	// enable or disable the display

	// Functions that take data from the display and put it in a variable,
	// or take the data from a variable and display it.
	void UpdateData(BOOL save_from_window = TRUE);	// Updates to or from the display.
	void CheckData(UINT item_id, int *var, BOOL no_zero = FALSE);	// Updates a value given text, and 
	// makes sure it is not 0 if 
	// no_zero is TRUE.
	void Killfocus(int edit_ID, int *value);	// Reverts to the previous value if left blank.

	int GetRunTime();	// Converts the run time from hh:mm:ss to seconds.
	void SaveResults(ostream * file);	// Saves the setup data to the result file.     
	BOOL SaveConfig(ostream & outfile);	// Save test setup to a file.
	BOOL LoadConfig(const CString & infilename);	// Load test setup from a file.

	int minutes;		// Test run length.
	int seconds;		// Test run length.
	int hours;		// Test run length.
	int ramp_time;		// Time from when all the workers are running to when recording starts.
	CString test_name;	// Text field for entering a test description.
	int disk_worker_count;	// Number of disk workers to spawn for every new manager.
	int net_worker_count;	// Number of network workers to spawn for every new manager.
	int test_type;		// Identifies whether to cycle or not.
	int use_random_data;

	Cycling_Info worker_cycling;
	Cycling_Info target_cycling;
	Cycling_Info queue_cycling;

	int result_type;

// Dialog Data
	//{{AFX_DATA(CPageSetup)
	enum { IDD = IDD_SETUP };
	CComboBox m_CResultType;
	CEdit m_EDiskWorkerCount;
	CEdit m_ENetWorkerCount;
	CEdit m_ETestName;
	CEdit m_ERunTimeSeconds;
	CEdit m_ERunTimeMinutes;
	CEdit m_ERunTimeHours;
	CEdit m_ERampTime;
	CComboBox m_CTestType;
	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPageSetup)
      protected:
	 virtual void DoDataExchange(CDataExchange * pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
      protected:
	// Generated message map functions
	//{{AFX_MSG(CPageSetup)
	 virtual BOOL OnInitDialog();
	afx_msg void OnChangeEDiskWorkerCount();
	afx_msg void OnChangeENetWorkerCount();
	afx_msg void OnSelchangeCDStepType();
	afx_msg void OnSelchangeCWStepType();
	afx_msg void OnSelchangeCQStepType();
	afx_msg void OnKillfocusEDiskStart();
	afx_msg void OnKillfocusEDiskStep();
	afx_msg void OnKillfocusEQueueEnd();
	afx_msg void OnKillfocusEQueueStart();
	afx_msg void OnKillfocusEQueueStep();
	afx_msg void OnKillfocusERampTime();
	afx_msg void OnKillfocusERunTimeHours();
	afx_msg void OnKillfocusERunTimeMinutes();
	afx_msg void OnKillfocusERunTimeSeconds();
	afx_msg void OnKillfocusEDiskWorkerCount();
	afx_msg void OnKillfocusENetWorkerCount();
	afx_msg void OnKillfocusEWorkerStart();
	afx_msg void OnKillfocusEWorkerStep();
	afx_msg void OnSelchangeCResultType();
	afx_msg void OnSelchangeCTestType();
	afx_msg void OnRDiskSpawnCPUs();
	afx_msg void OnRDiskSpawnUser();
	afx_msg void OnRNetSpawnCPUs();
	afx_msg void OnRNetSpawnUser();
	//}}AFX_MSG
	 DECLARE_MESSAGE_MAP()
	void SetStepTypeLabel(int text_id, int step_type);

      private:

	// Update cycling values.
	void SetCyclingInfo(Cycling_Info * cycle_info, BOOL selected, int start, int end, int step, int cycle_type);

	// Enable/Disable cycling information.
	void EnableCyclingInfo(Cycling_Info * cycling_info, BOOL enable);
};

#endif
