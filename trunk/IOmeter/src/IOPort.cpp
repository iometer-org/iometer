/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer & Dynamo) / IOPort.cpp                                    ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implementation of generic methods for the Port class. ## */
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
/* ##  Remarks ...: - As the code never supported asynchronous type of    ## */
/* ##                 connections for Unix, we are focusing and aiming    ## */
/* ##                 for synchronous connections only!                   ## */
/* ##               - There is still code for asynchronous conncetions,   ## */
/* ##                 but it will be skipped in a later versions.         ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Changes ...: 2005-04-18 (raltherr@apple.com)                       ## */
/* ##               - Support for MacOS X                                 ## */
/* ##               2004-03-27 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Code cleanup to ensure common style.                ## */
/* ##               - Applied Thayne Harmon's patch for supporting        ## */
/* ##                 Netware support (on I386).                          ## */
/* ##               2003-10-19 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Modified error message output destinction (Window   ## */
/* ##                 vs. Console) using _GALILEO_ define.                ## */
/* ##               2003-10-15 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-04-25 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Updated the global debug flag (_DEBUG) handling     ## */
/* ##                 of the source file (check for platform etc.).       ## */
/* ##               2003-03-04 (joe@eiler.net)                            ## */
/* ##               - Added a #ifdef for SOLARIS support                  ## */
/* ##               2003-03-01 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Cut out the Windows Pipes support for               ## */
/* ##                 communication efforts.                              ## */
/* ##               2003-02-15 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##               - Different changes to support compilation with       ## */
/* ##                 gcc 3.2 (known as cout << hex error).               ## */
/* ##                                                                     ## */
/* ######################################################################### */

#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
#include "GalileoApp.h"
#endif
#include "IOPort.h"

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

//
// Constructor and destructor.
//

Port::Port(BOOL synch)
{
	synchronous = synch;
	name[0] = '\0';
	type = PORT_TYPE_INVALID;
	errmsg = new ostringstream;
	accept_overlapped.hEvent = NULL;
	receive_overlapped.hEvent = NULL;
	send_overlapped.hEvent = NULL;
}

Port::~Port()
{
	delete errmsg;

	if (accept_overlapped.hEvent)
		CloseHandle(accept_overlapped.hEvent);
	if (receive_overlapped.hEvent)
		CloseHandle(receive_overlapped.hEvent);
	if (send_overlapped.hEvent)
		CloseHandle(send_overlapped.hEvent);
}

// 
// Determine if an asynchronous operation has completed yet (TRUE = yes, FALSE = no).  Does not block.
// Utility function called by IsAcceptComplete(), IsReceiveComplete(), and IsSendComplete().
//
BOOL Port::IsOperationComplete(OVERLAPPED * olap)
{
	if (synchronous) {
		return TRUE;
	}

	if (olap->hEvent) {
#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
		cout << "Async Port objects not supported on UNIX or NetWare" << endl;
		return FALSE;
#elif defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
		DWORD d = WaitForSingleObject(olap->hEvent, 0);

		return (d == WAIT_OBJECT_0);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	} else {
		return FALSE;
	}
}

// 
// Determine if Accept() has completed yet.
//
BOOL Port::IsAcceptComplete()
{
	return (IsOperationComplete(&accept_overlapped));
}

// 
// Determine if Receive() has completed yet.
//
BOOL Port::IsReceiveComplete()
{
	return (IsOperationComplete(&receive_overlapped));
}

// 
// Determine if Send() has completed yet.
//
BOOL Port::IsSendComplete()
{
	return (IsOperationComplete(&send_overlapped));
}

//
// Utility function: Initialize an OVERLAPPED structure (works even if it has been previously used).  
// Return value indicates success or failure.
// 
BOOL Port::InitOverlapped(OVERLAPPED * olap)
{
	if (synchronous) {
		return FALSE;
	}

	if (olap->hEvent)
		CloseHandle(olap->hEvent);

	// specify the file pointer (meaningless for a pipe or socket, but must be set!)
	olap->Offset = 0;
	olap->OffsetHigh = 0;

	// create a manual-reset event object for the OVERLAPPED structure
	olap->hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (olap->hEvent == NULL)
		return FALSE;
	else
		return TRUE;
}

//
// Utility function: output the "errmsg" message in an appropriate manner for the
// current environment (Dynamo or Iometer).
//
void Port::OutputErrMsg()
{
	if (!errmsg) {
		errmsg = new ostringstream;
		*errmsg << "Port::OutputErrMsg() called with invalid errmsg value!" << ends;
	}
#if defined(_GALILEO_)
	// Iometer
	ErrorMessage(errmsg->str().c_str());
#else
	// Dynamo
	cout << errmsg->str() << endl;
#endif

	// str() returns pointer to buffer and freezes it, we must call freeze(FALSE) to 
	// unfreeze the buffer before we can delete the object
	// ---
	// REMARK: freeze() no longer needed because new are now
	// using ostringstream instead of ostrstream.
	//
	// errmsg->rdbuf()->freeze( FALSE );

	delete errmsg;

	errmsg = new ostringstream;
}
