/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Iometer / GalileoApp.cpp                                           ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: The central class of the Iometer application.         ## */
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
/* ##  Changes ...: 2004-03-20 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Changed the registry key from "Intel" to            ## */
/* ##                 "iometer.org"                                       ## */
/* ##               2003-10-15 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-04-25 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Updated the global debug flag (_DEBUG) handling     ## */
/* ##                 of the source file (check for platform etc.).       ## */
/* ##               2003-03-28 (joe@eiler.net)                            ## */
/* ##               - changes so VC++ 7 (.NET) will compile correctly.    ## */
/* ##               2003-03-05 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Changed the NDEBUG check to the generic _DEBUG.     ## */
/* ##               2003-03-01 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Cut out the Windows Pipes support for               ## */
/* ##                 communication efforts.                              ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include "stdafx.h"
#include "GalileoDefs.h"
#include "GalileoApp.h"
#include "MainFrm.h"
#include "GalileoDoc.h"
#include "GalileoView.h"
#include "ManagerList.h"
#include "Legalbox.h"
#include "IOPortTCP.h"

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
// CGalileoApp

BEGIN_MESSAGE_MAP(CGalileoApp, CWinApp)
    //{{AFX_MSG_MAP(CGalileoApp)
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    //}}AFX_MSG_MAP
    // Standard file based document commands
//      ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
//      ON_COMMAND(ID_FILE_OPEN, CPageAccess::OnFileOpen)
    END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CGalileoApp construction/destruction
    CGalileoApp::CGalileoApp()
:  m_pVersionString(NULL), m_pVersionStringWithDebug(NULL)
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	pView = NULL;
	login_port = NULL;
}

CGalileoApp::~CGalileoApp()
{
	delete login_port;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CGalileoApp object

CGalileoApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CGalileoApp initialization

BOOL CGalileoApp::InitInstance()
{
	char iometer_path[MAX_PATH];
	char *p;

	test_state = TestIdle;
	login_state = closed;

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.
#if _MSC_VER < 1300
#ifdef _AFXDLL
	Enable3dControls();	// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
#endif

	//init file version strings
	GetAppFileVersionString(&m_pVersionString, &m_pVersionStringWithDebug);

	SetRegistryKey("iometer.org");	// Stores information in the registry under
	// HKEY_CURRENT_USER\Software\iometer.org
	// \Iometer\Settings\Version

	LoadStdProfileSettings();	// Load standard INI file options (including MRU)

	//
	// Check to see if the end user license agreement should be displayed.
	// If the current version is not stored in the registry, disply the EULA.
	//
#ifndef	_DEBUG
	if (GetProfileString("Settings", "Version") != m_pVersionString) {
		CLegalBox legalDlg;	// Display EULA.

		if (legalDlg.DoModal() == IDOK)
			// User agrees to terms, so store the version in the registry.
			WriteProfileString("Settings", "Version", m_pVersionString);
		else
			return FALSE;	// User does not agree to terms so exit.
	}
#endif

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate *pDocTemplate;

	pDocTemplate = new CSingleDocTemplate(IDR_MAINFRAME, RUNTIME_CLASS(CGalileoDoc), RUNTIME_CLASS(CMainFrame),	// main SDI frame window
					      RUNTIME_CLASS(CGalileoView));
	AddDocTemplate(pDocTemplate);

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	//
	// Pass the command line info to cmdline.ParseParam().
	// Here is the expected syntax:
	//
	// iometer ?
	// iometer [config_file [result_file [timeout_value]]]
	// iometer [/c config_file] [/r results_file] [/t timeout_value] [/p port_number] [/m 1]
	//
	//    ? - show Iometer version number and command line syntax
	//
	//    config_file - the name of the Iometer configuration file to restore
	//       when Iometer starts (must be a valid .icf file)
	//
	//    results_file - the name of a writable text file to which the test
	//       results should be appended.  If the file doesn't exist, it will be
	//       created.
	//
	//    timeout_value - the number of seconds to wait for managers to log in
	//       when restoring a config file.
	//
	//    port_number - allow iometer listen on this port instead of default port.
	//
	//    specify /m option will have BigMeter shown automatically. The 
	//       parameter must be 1.
	//
	//    If both config_file and results_file are specified, Iometer will
	//    attempt to run the test in batch mode, requiring no user intervention
	//    to load the config file, start the tests, stop the tests (if a run time
	//    was specified in the Test Setup tab), and close Iometer.  Batch mode
	//    can be canceled by aborting the manager waiting list dialog or by
	//    manually stopping a test while it is running.
	//
	CWinApp::ParseCommandLine(cmdline);

	// If the command line parser had any trouble, kill the program.
	if (cmdline.m_bFail)
		return FALSE;

	if (!OnCmdMsg(ID_FILE_NEW, 0, NULL, NULL)) {
		OnFileNew();
	}
	if (m_pMainWnd == NULL) {
		ErrorMessage("Error creating application's main frame.  " "Please report this as an Iometer bug.");
		return FALSE;
	}
	// Fill in the local address strings.
	IdentifyLocalAddresses();

	// Set the status bar.
	CRect rect;

	m_wndStatusBar.GetWindowRect(&rect);
	int *widths;
	widths = (int *)malloc(sizeof(int) * 3);
	widths[0] = rect.Width() - 300;
	widths[1] = rect.Width() - 150;
	widths[2] = -1;
	m_wndStatusBar.GetStatusBarCtrl().SetParts(3, widths);
	free(widths);

	// Set status bar text.
	pView->ClearStatusBar();

	// Disable most buttons.
	pView->ButtonReset();

	// make sure CGalileoApp::OnIdle() gets called at least once in a while
	pView->SetTimer(IDLE_TIMER, IDLE_DELAY, NULL);

	// Get the full pathname for Iometer.exe.
	if (!GetModuleFileName(NULL, iometer_path, sizeof(iometer_path))) {
		ErrorMessage("Could not get Iometer.exe pathname!");

		// Set iometer_path to a null string; this will make NEW_WORKER_COMMAND
		// search for Dynamo in the current directory and $PATH
		iometer_path[0] = '\0';
	}
	// Find the last backslash in the pathname, if any
	if (p = strrchr(iometer_path, '\\')) {
		// Terminate the string right after the last backslash, leaving
		// the full pathname of the directory containing Iometer.exe.
		p++;
		*p = '\0';
	}
	// Build the command line to launch Dynamo.  We must quote the pathname in case it 
	// contains any spaces, but the "start" command (NEW_WORKER_COMMAND) assumes the
	// first quoted argument is the window title, so we must provide it twice.
	new_manager_command_line_format = (CString) NEW_WORKER_COMMAND
	    + "\"" + iometer_path + NEW_WORKER_EXECUTABLE + "%s%s\"" + " "
	    + "\"" + iometer_path + NEW_WORKER_EXECUTABLE + "\"%s%s";

	if (cmdline.GetConfigFile().IsEmpty()) {
#ifndef	_DEBUG
		// If the default config file exists, load it.
		if (::GetFileAttributes(DEFAULT_CONFIG_FILE) != 0xFFFFFFFF) {
			OpenDocumentFile(DEFAULT_CONFIG_FILE);
		} else {
			// If no config file was specified on the command line
			// and the default config file doesn't exist, start a
			// local manager.with no command line options.
			LaunchDynamo();
		}
#endif
	} else {
		// If a config file was specified on the command line, open it
		OpenDocumentFile(cmdline.GetConfigFile());
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//      Function Name:
//              CGalileoApp::ExitInstance
//
//      Syntax:
//              int ExitInstance()
//
//      Processing:
//              Clean up instance data
///////////////////////////////////////////////////////////////////////////////
int CGalileoApp::ExitInstance()
{
	delete[]m_pVersionString;
	delete[]m_pVersionStringWithDebug;

	return CWinApp::ExitInstance();
}

///////////////////////////////////////////////////////////////////////////////
//      Function Name:
//              CGalileoApp::GetVersionString
//
//      Syntax:
//              const char*             GetVersionString(BOOL fWithDebugIndicator = FALSE);
//
//      Processing:
//              Return version string ptr (m_pVersionString or m_pVersionStringWithDebug).
///////////////////////////////////////////////////////////////////////////////
const char *CGalileoApp::GetVersionString(BOOL fWithDebugIndicator)
{
	if (!fWithDebugIndicator)
		return m_pVersionString;
	else
		return m_pVersionStringWithDebug;
}

//
// Keep track of which addresses are local to this host.
// This includes the NetBIOS name of the computer and the IP addresses of
// each NIC attached to the computer.  When managers are being restored
// from a config file, if no matching manager is currently logged in and
// the manager's address is local to Iometer, Iometer will spawn a local
// Dynamos with the appropriate name.
//
void CGalileoApp::IdentifyLocalAddresses()
{
	DWORD namelength = MAX_NETWORK_NAME;
	WSADATA wd;
	char hostname[128];
	hostent *hostinfo;
	sockaddr_in sin;

	////////////////////////////////////////////////////////////////////////////////////////
	// Get the local machine's NetBIOS address.
	////////////////////////////////////////////////////////////////////////////////////////
	::GetComputerName(netbios_hostname.GetBuffer(MAX_NETWORK_NAME), &namelength);
	netbios_hostname.ReleaseBuffer();
	netbios_hostname = "\\\\" + netbios_hostname + "\\";

	////////////////////////////////////////////////////////////////////////////////////////
	// Assemble a list of all local IP addresses (one for each NIC).
	////////////////////////////////////////////////////////////////////////////////////////
	if (WSAStartup(MAKEWORD(2, 0), &wd)) {
		ErrorMessage("Error initializing WinSock in CGalileoApp::InitInstance().");
		return;
	}

	if (gethostname(hostname, sizeof(hostname)) != SOCKET_ERROR) {
		hostinfo = gethostbyname(hostname);
		if (hostinfo != NULL) {
			for (int counter = 0; hostinfo->h_addr_list[counter] != NULL; counter++) {
//                              ip_addresses.Add(counter);
				memcpy(&sin.sin_addr.s_addr, hostinfo->h_addr_list[counter], hostinfo->h_length);
//                              ip_addresses[counter] = inet_ntoa( sin.sin_addr );
				ip_addresses.SetAtGrow(counter, inet_ntoa(sin.sin_addr));
			}
		} else {
			// Non-fatal (might not recognize all local managers as being local, though)
			ErrorMessage("Error getting host info (HOSTENT) for \"" + (CString) hostname + "\".");
		}
	} else {
		// Non-fatal (might not recognize all local managers as being local, though)
		ErrorMessage("Error getting local host name in CGalileoApp::InitInstance().");
	}

	WSACleanup();
}

//
// Determines whether an address is local to the computer Iometer is running on.
// This address can be a NetBIOS address (the NT name of the machine) or the IP
// address of any NIC attached to the computer.
//
BOOL CGalileoApp::IsAddressLocal(const CString & addr)
{
	if (netbios_hostname.CompareNoCase(addr) == 0)
		return TRUE;

	for (int counter = 0; counter < ip_addresses.GetSize(); counter++) {
		if (ip_addresses[counter].CompareNoCase(addr) == 0)
			return TRUE;
	}

	return FALSE;
}

//
// Launch a local Dynamo with the given name.
//
void CGalileoApp::LaunchDynamo(const CString & mgr_name /* ="" */ )
{
	CString cmd;
	CString portparam;

	// Make sure the formatting string was initialized.
	ASSERT(!theApp.new_manager_command_line_format.IsEmpty());

	portparam.Format(" /p %d ", cmdline.GetLoginportnumber());

	// Create a string with the appropriate command line parameters.
	cmd.Format(new_manager_command_line_format, mgr_name, portparam, mgr_name, portparam);

	// Launch Dynamo.
	system(cmd);
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg:public CDialog {
      public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CStatic m_TVersion;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
      protected:
	virtual void DoDataExchange(CDataExchange * pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
      protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBViewEULA();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg():CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange * pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Control(pDX, TVersion, m_TVersion);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
    //{{AFX_MSG_MAP(CAboutDlg)
ON_BN_CLICKED(BViewEULA, OnBViewEULA)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
// App command to run the dialog
void CGalileoApp::OnAppAbout()
{
	CAboutDlg aboutDlg;

	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CGalileoApp commands

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Display correct version information in about box dialog.
	CString buffer = "Iometer ";

	buffer += theApp.GetVersionString(TRUE);
	m_TVersion.SetWindowText(buffer);

	return TRUE;		// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CAboutDlg::OnBViewEULA()
{
	CLegalBox legal_dlg;

	legal_dlg.running = TRUE;
	legal_dlg.DoModal();
}

//
// On Idle handler processes login requests from Dynamos.
//
BOOL CGalileoApp::OnIdle(LONG lCount)
{
	static Message *login_msg;
	static char *login_msg_ptr;
	static DWORD login_msg_size;
	static Data_Message *login_data_msg;
	static char *login_data_msg_ptr;
	static DWORD login_data_msg_size;
	static DWORD start_recv_timer = 0;
	DWORDLONG result;
	Message reply_to_dynamo;
	Manager *manager;

	if (CWinApp::OnIdle(lCount))
		return TRUE;

	switch (login_state) {
	case closed:
		{
			int login_port_number;

			// create and open the port
			login_port_number = theApp.cmdline.GetLoginportnumber();
 
			login_port = new PortTCP(FALSE);	// asynchronous port
			if (!login_port->Create(NULL, NULL, 0, login_port_number)) {
				ErrorMessage("Could not create TCP/IP port for Dynamo login!");
				login_state = failed;
				return FALSE; // go away and don't come back (for a while)
			}

			login_state = open;
			return TRUE; // go away and try again later
			break;
		}

	case open:
		if (!login_port->Accept())	// begin accepting...
		{
			ErrorMessage("Accept() failed on Dynamo login port!");
			login_port->Close();
			login_state = failed;
			return FALSE;	// go away and don't come back (for a while)
		}
		login_state = accepting;
		return TRUE;	// go away and try again later
		break;

	case accepting:
		// has Accept() completed?
		if (login_port->IsAcceptComplete()) {
			// Yes!  Was it successful?
			if (login_port->GetAcceptResult()) {
				// Accept() succeeded
				login_state = waiting;
				return TRUE;	// go away and try again later
			} else {
				// Accept() failed!
				ErrorMessage("GetAcceptResult() failed on Dynamo login port!");
				login_port->Close();
				login_state = failed;
				return FALSE;	// go away and don't come back (for a while)
			}
		} else {
			// Accept() has not yet completed
			return FALSE;	// go away and don't come back (for a while)
		}
		break;

	case waiting:
		if (login_port->Peek()) {
			// There is data waiting for us!
			login_msg = new Message;
			login_msg_ptr = (char *)login_msg;
			login_msg_size = sizeof(Message);
			login_port->Receive(login_msg_ptr, login_msg_size);	// begin receiving...
			login_state = receiving;
			start_recv_timer = GetTickCount();
			return TRUE;	// go away and try again later
		} else {
			// There is no data for us, go back to waiting
			return FALSE;	// go away and don't come back (for a while)
		}
		break;

	case receiving:
		// has Receive() completed?
		if (login_port->IsReceiveComplete()) {
			// Yes!  Have all requested bytes been received?
			result = login_port->GetReceiveResult();
			if (result == login_msg_size) {
				// All bytes received!  

				// Format our version number into an integer
				int year, month, day, iometer_version;

				sscanf(m_pVersionString, "%d.%d.%d", &year, &month, &day);
				iometer_version = (year * 10000) + (month * 100) + day;

				// Compare it with Dynamo's version number (will be 0 or uninitialized for 
				// versions before 1998.09.23)
				if (login_msg->data != iometer_version) {
					// give the user a message box explaining the problem
					char errmsg[2 * MAX_VERSION_LENGTH + 100];

					// versions failed to match...  refuse connection

					// tell Dynamo that the connection is being refused
					reply_to_dynamo.purpose = LOGIN;
					reply_to_dynamo.data = WRONG_VERSION;
					login_port->Send(&reply_to_dynamo);

					login_port->Disconnect();

					if (login_msg->data > 19970101 && login_msg->data < 21001231) {
						year = (int)(login_msg->data / 10000);
						month = (int)(login_msg->data / 100) - (year * 100);
						day = login_msg->data - (month * 100) - (year * 10000);

						snprintf(errmsg, 2 * MAX_VERSION_LENGTH + 100,
							"Iometer %s is not compatible with Dynamo %04d.%02d.%02d",
							m_pVersionStringWithDebug, year, month, day);
					} else {
						snprintf(errmsg, 2 * MAX_VERSION_LENGTH + 100,
							"Iometer %s is not compatible with Dynamo (unknown version number)",
							m_pVersionStringWithDebug);
					}

					ErrorMessage(errmsg);

					// Receive() completed and processed, go back to Accept().
					login_state = open;
					delete login_msg;

					return TRUE;	// go away and try again later
				}
				// we are through with login_msg
				delete login_msg;

				// Now wait for data message...
				login_state = waiting_for_data;
				return TRUE;	// go away and try again later
			} else if (result > 0) {
				// Some bytes (but not all) received, post another receive for the rest of the message
				login_msg_size -= (DWORD) result;
				login_msg_ptr += result;
				login_port->Receive(login_msg_ptr, login_msg_size);	// begin receiving...
				login_state = receiving;
				return TRUE;	// go away and try again later
			} else {
				// Receive() failed!
				ErrorMessage("GetReceiveResult() failed on Dynamo login port!");
				login_port->Close();
				login_state = failed;
				return FALSE;	// go away and don't come back (for a while)
			}
		} else {
			// Receive() has not yet completed
			DWORD current_time = GetTickCount();

			if (current_time < start_recv_timer) start_recv_timer = current_time - 1; // delay a bit if counter wrapped
			else if (current_time - start_recv_timer >= IOMETER_RECEIVE_TIMEOUT)
			{
				// send a warning
				ErrorMessage("Timed out waiting for dynamo to complete sending the login message. Please check your networking or your dynamo build.");
				
				delete login_msg;
				login_state = accepting;
			}

			return TRUE;	// go away and try again later
		}
		break;

	case waiting_for_data:
		if (login_port->Peek()) {
			// There is data waiting for us!
			login_data_msg = new Data_Message;
			login_data_msg_ptr = (char *)login_data_msg;
			login_data_msg_size = sizeof(Data_Message);
			login_port->Receive(login_data_msg_ptr, login_data_msg_size);	// begin receiving...
			login_state = receiving_data;
			start_recv_timer = GetTickCount();
			return TRUE;	// go away and try again later
		} else {
			// There is no data for us, go back to waiting
			return FALSE;	// go away and don't come back (for a while)
		}
		break;

	case receiving_data:
		// has Receive() completed?
		if (login_port->IsReceiveComplete()) {
			// Yes!  Have all requested bytes been received?
			result = login_port->GetReceiveResult();
			if (result == login_data_msg_size) {
				// Keep track of whether this manager is logging in during
				// a file restore operation.  Once the manager is added to
				// the manager list, this state of this variable determines
				// whether to add the default workers to the manager.
				// (Its state must be recorded here because the waiting list may
				// be dismissed when this manager is added to the manager list.)
				BOOL wasWaitingList = manager_list.loadmap.IsWaitingList();

				// Receive completely succeeded, login_data_msg received
				// Display the hourglass cursor to indicate a login is in progress.
				CWaitCursor wait;

				// Tell Dynamo that the connection is being accepted
				reply_to_dynamo.purpose = LOGIN;
				reply_to_dynamo.data = LOGIN_OK;
				login_port->Send(&reply_to_dynamo);

				// Receive succeeded, so contents of login_data_msg are valid...
				manager = manager_list.AddManager(&(login_data_msg->data.manager_info));

				// We are all through with login_data_msg
				delete login_data_msg;

				// In certain situations, ::AddManager() may return a null, we have to at least 
				// check for it.
				if (manager == NULL) {
					ErrorMessage("AddManager() failed to add manager to manager list!");
					login_port->Close();
					login_state = failed;
					return FALSE;
				}
				// Get the manager's list of available targets.
				manager->UpdateTargetLists();

				// Add new manager to the end of the manager list.
				pView->AddManager(manager);

				// Add the default workers (specified in Test Setup page)
				// ONLY if there is not a file restore operation happening.
				// In the case of a file restore, Iometer will explicitly create
				// any necessary workers.  Why this was done:  If default workers
				// were added when merging a saved manager configuration, the
				// default workers would never be deleted when the saved workers
				// were restored.
				if (!wasWaitingList)
					pView->AddDefaultWorkers(manager);

				// Notify worker of successful login, then disconnect them.
				login_port->Disconnect();

				// Receive() completed and processed, go back to Accept().
				login_state = open;
				return TRUE;	// go away and try again later
			} else if (result > 0) {
				// Some bytes (but not all) received, post another receive for the rest of the message
				login_data_msg_size -= (DWORD) result;
				login_data_msg_ptr += result;
				login_port->Receive(login_data_msg_ptr, login_data_msg_size);	// begin receiving...
				login_state = receiving_data;
				return TRUE;	// go away and try again later
			} else {
				// Receive() failed!
				ErrorMessage("GetReceiveResult() failed on Dynamo login port!");
				login_port->Close();
				login_state = failed;
				return FALSE;	// go away and don't come back (for a while)
			}
		} else {
			// Receive() has not yet completed
			DWORD current_time = GetTickCount();

			if (current_time < start_recv_timer) start_recv_timer = current_time - 1; // delay a bit if counter wrapped
			else if (current_time - start_recv_timer >= IOMETER_RECEIVE_TIMEOUT)
			{
				// send a warning
				ErrorMessage("Timed out waiting for dynamo to complete sending a data message. Please check your networking or your dynamo build.");
				
				delete login_data_msg;
				login_state = accepting;
			}

			return TRUE;	// go away and try again later
		}
		break;

	case failed:
		// we've suffered a fatal error, do nothing
		return FALSE;	// go away and don't come back (for a while)
		break;
	}

	ErrorMessage("Fell off end of CGalileoApp::OnIdle(), this should never happen");
	return FALSE;
}

//
// Handle an "open document" request (e.g. from a command line argument)
//
CDocument *CGalileoApp::OpenDocumentFile(LPCTSTR lpszFileName)
{
	CDocument *theDoc;
	BOOL flags[NumICFFlags], replace;

	flags[ICFTestSetupFlag] = TRUE;
	flags[ICFResultsDisplayFlag] = TRUE;
	flags[ICFGlobalAspecFlag] = TRUE;
	flags[ICFManagerWorkerFlag] = TRUE;
	flags[ICFAssignedAspecFlag] = TRUE;
	flags[ICFAssignedTargetFlag] = TRUE;
	replace = TRUE;

	// Create a CDocument object by calling the parent method
	// (this also initializes the pView member)
	if (theDoc = CWinApp::OpenDocumentFile(lpszFileName)) {
		// Attempt to open the file as a configuration file
		if (!pView->PrepareToOpenConfigFile(lpszFileName, flags, replace)) {
			// The file could not be opened!  Indicate failure.
			theDoc = NULL;
		}
	}

	return theDoc;
}

//
// See if Iometer is in batch mode.
//
BOOL CGalileoApp::IsBatchMode()
{
	return cmdline.IsBatchMode();
}

//
// Take the application out of batch mode.
// No harm will be done if Iometer is already out of batch mode.
//
void CGalileoApp::OverrideBatchMode()
{
	cmdline.OverrideBatchMode();
}
