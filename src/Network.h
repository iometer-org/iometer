/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / Network.h                                                 ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Interface for the Network class. Network is a pure    ## */
/* ##               virtual class that is used as a base class for any    ## */
/* ##               network connection.                                   ## */
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
/* ##  Remarks ...: - Network objects are used for connections between    ## */
/* ##                 network workers in Dynamo (contrast Port, which is  ## */
/* ##                 used for communication between Dynamo and Iometer). ## */
/* ##                 The Network class was designed for use in both      ## */
/* ##                 Iometer and Dynamo, but is currently used only in   ## */
/* ##                 Dynamo.                                             ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Changes ...: 2005-02-26 (mingz@ele.uri.edu)                        ## */
/* ##               - Added null definition for namespace std for old     ## */
/* ##                 version MS VC++.                                    ## */
/* ##               - Corrected pure virtual function definition.         ## */
/* ##               2003-10-17 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-02-15 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##               - Different changes to support compilation with       ## */
/* ##                 gcc 3.2 (known as cout << hex error).               ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef NETWORK_DEFINED
#define NETWORK_DEFINED

#if defined(IOMTR_OSFAMILY_WINDOWS) || defined(IOMTR_OSFAMILY_NETWARE)
namespace std {
}
#endif
using namespace std;

#include <sstream>

#include "IOCommon.h"

// Set NETWORK_DETAILS to 1 to enable network related debug messages, 0 to disable them.
// If _DETAILS  is defined and NETWORK_DETAILS is set to 0, some messages will
// still appear.
#define NETWORK_DETAILS	0

#define NETWORK_ERROR	0xffffffffffffffff
#define SERVER	TRUE
#define CLIENT	FALSE

enum ReadWriteType {
	READ,
	WRITE
};

class Network {
      public:
	// constructor and destructor   
	Network();
	virtual ~ Network();

	// public functions common to all Networks (pure virtual, not implemented by Network)
	virtual ReturnVal Create(BOOL create_server) = 0;
	virtual ReturnVal Connect(const char *ip_address, unsigned short port_number) = 0;
	virtual ReturnVal Accept() = 0;
	virtual ReturnVal Destroy() = 0;
	virtual ReturnVal Receive(LPVOID buffer, DWORD bytes, LPDWORD return_value,
				  LPOVERLAPPED asynchronous_io, DWORD flags = 0) = 0;
	virtual ReturnVal Send(LPVOID buffer, DWORD bytes, LPDWORD return_value,
			       LPOVERLAPPED asynchronous_io, DWORD flags = 0) = 0;
	virtual DWORD Peek() = 0;
	virtual ReturnVal Close(BOOL close_server) = 0;

      protected:
	 virtual void OutputErrMsg();

	ostringstream *errmsg;
};

#endif
