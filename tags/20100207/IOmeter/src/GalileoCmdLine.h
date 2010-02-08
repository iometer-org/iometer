/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / GalileoCmdLine.h                                       ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Interface for the CGalileoCmdLine class, which        ## */
/* ##               parses Iometer's command line.                        ## */
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
#ifndef GALILEO_CMD_LINE_INCLUDED
#define GALILEO_CMD_LINE_INCLUDED

class CGalileoCmdLine:public CCommandLineInfo {
      public:
	CGalileoCmdLine();
	virtual ~ CGalileoCmdLine();

	// see definitions in GalileoCmdLine.cpp
	static const int DefaultTimeout;
	static const char DefaultConfigFile[];

	void ParseParam(const char *pszParam, BOOL bFlag, BOOL bLast);

	CString GetConfigFile();
	CString GetResultFile();
	int GetTimeout();
	int GetLoginportnumber();
	BOOL GetShowBigmeter();

	// Interactive mode is the normal mode of operation for Iometer.
	// If the user specifies both the config file and the results file
	// on the command line, Iometer goes into batch mode, in which
	// Iometer starts up, runs a test for its specified run time, writes
	// the results file, and closes without user interaction.
	//              TRUE = batch mode
	//              FALSE = interactive mode
	BOOL IsBatchMode();

	void OverrideBatchMode();	// set m_bOverrideBatch (manually override batch mode)

	BOOL m_bFail;		// did the command line parser fail?

      protected:
	void Fail(const CString & errmsg);
	BOOL IsValidInteger(const CString & instring);
	BOOL IsValidFilename(const CString & instring);

	// See if the file is writable, otherwise clear the filename string.
	BOOL VerifyWritable(const CString & filename);
	BOOL VerifyReadable(const CString & filename);

	BOOL m_bSwitches;	// are switches being used

	CString m_sConfigFile;	// the specified config file
	CString m_sResultFile;	// the specified result file
	int m_iTimeout;		// the specified timeout value
	int m_iLoginportnumber; // the specified port number
	BOOL m_bShowBigmeter;// automatically show bigmeter

	BOOL m_bOverrideBatch;	// if set, forces IsBatchMode to return FALSE
};

#endif				// GALILEO_CMD_LINE_INCLUDED
