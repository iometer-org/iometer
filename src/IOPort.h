/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer & Dynamo) / IOPort.h                                      ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Contains the class definition of the Port class,      ## */
/* ##               which covers the communication between Iometer        ## */
/* ##               and Dynamo.                                           ## */
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
/* ##  Changes ...: 2005-02-26 (mingz@ele.uri.edu)                        ## */
/* ##               - Added null definition for namespace std for old     ## */
/* ##                 version MS VC++.                                    ## */
/* ##               - Corrected pure virtual function definition.         ## */
/* ##               2003-10-15 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-03-01 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Cut out the Windows Pipes support for               ## */
/* ##                 communication efforts.                              ## */
/* ##               2003-02-15 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##               - Different changes to support compilation with       ## */
/* ##                 gcc 3.2 (known as cout << hex error).               ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef PORT_DEFINED
#define PORT_DEFINED

#if defined(IOMTR_OSFAMILY_WINDOWS) || defined(IOMTR_OSFAMILY_NETWARE)
namespace std {
}
#endif
using namespace std;

#include <sstream>

#include "IOCommon.h"
#include "IOMessage.h"

// Set PORT_DETAILS to 1 to enable port related debug messages, 0 to disable them.
// If _DETAILS  is defined and PORT_DETAILS is set to 0, some messages will
// still appear.
#define PORT_DETAILS	0

#define PORT_TYPE_INVALID	0
#define PORT_TYPE_TCP		1

#define WELL_KNOWN_TCP_PORT	1066

#define MESSAGE_PORT_SIZE	MESSAGE_SIZE * 3	// make pipe buffer big enough for 3 messages (arbitrary number?)

#define PORT_ERROR			(~(DWORDLONG)0)

class Port {
      public:
	// constructor and destructor   
	Port(BOOL synchronous = TRUE);
	virtual ~ Port();

	// public functions common to all Ports (pure virtual, not implemented by Port)
	virtual BOOL Create(char *port_name = NULL, char *remote_name = NULL,
			    DWORD size = MESSAGE_PORT_SIZE, unsigned short port_number = 0) = 0;
	virtual BOOL Connect(char *port_name = NULL, unsigned short port_number = WELL_KNOWN_TCP_PORT) = 0;
	virtual BOOL Accept() = 0;
	virtual BOOL Disconnect() = 0;
	virtual BOOL Close() = 0;
	virtual DWORDLONG Receive(LPVOID data, DWORD size = MESSAGE_SIZE) = 0;
	virtual DWORDLONG Send(LPVOID data, DWORD size = MESSAGE_SIZE) = 0;
	virtual DWORD Peek() = 0;

	// public functions common to all asynchronous Ports
	//     (implemented by Port)
	virtual BOOL IsAcceptComplete();
	virtual BOOL IsReceiveComplete();
	virtual BOOL IsSendComplete();
	//     (pure virtual, not implemented by Port)
	virtual BOOL GetAcceptResult() = 0;
	virtual DWORDLONG GetReceiveResult() = 0;
	virtual DWORDLONG GetSendResult() = 0;

	// public data members common to all Ports
	char network_name[MAX_NETWORK_NAME];
	unsigned short network_port;	// used only by PortTCP (was ignored by PortPipe)
	int type;		// PORT_TYPE_INVALID or PORT_TYPE_TCP

      protected:
	// private data members common to all Ports
	 BOOL synchronous;
	char name[MAX_NETWORK_NAME];
	ostringstream *errmsg;

	// private functions common to all Ports (implemented by Port)
	virtual void OutputErrMsg();

	// private functions common to all asynchronous Ports (implemented by Port)
	virtual BOOL InitOverlapped(OVERLAPPED * olap);
	virtual BOOL IsOperationComplete(OVERLAPPED * olap);

	// private data members common to all asynchronous Ports
	OVERLAPPED accept_overlapped;
	OVERLAPPED receive_overlapped;
	OVERLAPPED send_overlapped;
};

#endif
