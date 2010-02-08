/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer & Dynamo) / IOPortTCP.h                                   ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Contains the class definition of the Port class,      ## */
/* ##               which covers the communication between Iometer        ## */
/* ##               and Dynamo (implementing the TCP/IP transport).       ## */
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
/* ##               2005-01-19 (mingz@ele.uri.edu)                        ## */
/* ##               - Removed socketlen_t define.                         ## */
/* ##               2005-01-12 (henryx.w.tieman@intel.com)                ## */
/* ##               - prototype of AcceptEx() defined.                    ## */
/* ##               2004-03-26 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Code cleanup to ensure common style.                ## */
/* ##               - Applied Thayne Harmon's patch for supporting        ## */
/* ##                 Netware support (on I386).                          ## */
/* ##               2003-07-27 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Changed the socklen_t defintion (based on the OS).  ## */
/* ##               2003-07-19 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Assimilated the patch from Robert Jones which is    ## */
/* ##                 needed to build under Solaris 9 on x86 (i386).      ## */
/* ##               2003-07-18 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Massive cleanup of this file (grouping the          ## */
/* ##                 different blocks together).                         ## */
/* ##               - Implemented the IOMTR_[OSFAMILY|OS|CPU]_* global    ## */
/* ##                 define as well as their integrity checks.           ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-03-01 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Cut out the Windows Pipes support for               ## */
/* ##                 communication efforts.                              ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef PORTTCP_DEFINED
#define PORTTCP_DEFINED

#if _MSC_VER >= 1000
#pragma once
#endif				// _MSC_VER >= 1000

#include "IOPort.h"
#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#elif defined(IOMTR_OSFAMILY_WINDOWS)
#include "winsock2.h"
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
#define SOCKET	int
#endif

#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
 // nop
#elif defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
#define socklen_t int
#elif defined(IOMTR_OS_NETWARE)
#define socklen_t unsigned int
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

class PortTCP:public Port {
      public:
	// constructor and destructor   
	PortTCP(BOOL synchronous = TRUE);
	virtual ~ PortTCP();

	// public functions common to all Ports (implemented here, pure virtual in Port)
	virtual BOOL Create(char *port_name = NULL, char *remote_name = NULL,
			    DWORD size = MESSAGE_PORT_SIZE, unsigned short port_number = 0);
	virtual BOOL Connect(char *port_name = NULL, unsigned short port_number = WELL_KNOWN_TCP_PORT);
	virtual BOOL Accept();
	virtual BOOL Disconnect();
	virtual BOOL Close();
	virtual DWORDLONG Receive(LPVOID data, DWORD size = MESSAGE_SIZE);
	virtual DWORDLONG Send(LPVOID data, DWORD size = MESSAGE_SIZE);
	virtual DWORD Peek();

	// public functions common to all asynchronous Ports (implemented here, pure virtual in Port)
	virtual BOOL GetAcceptResult();
	virtual DWORDLONG GetReceiveResult();
	virtual DWORDLONG GetSendResult();

      protected:
	// private data members used only by PortTCP
#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
	int server_socket;
	int client_socket;
#elif defined(IOMTR_OSFAMILY_WINDOWS)
	 SOCKET server_socket;
	SOCKET client_socket;
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

	// private data members used only by asynchronous PortTCP's
	char *accept_ex_buffer;

	// static private data members shared by all PortTCP's
	static unsigned int sockets_in_use;

	// private functions used only by PortTCP
	BOOL CloseSocket(SOCKET * s, const char *socket_name);	// utility function
	DWORDLONG SynchReceive(LPVOID data, DWORD size = MESSAGE_SIZE);
	DWORDLONG SynchSend(LPVOID data, DWORD size = MESSAGE_SIZE);

	// private functions used only by asynchronous PortTCP's
	DWORDLONG AsynchReceive(LPVOID data, DWORD size = MESSAGE_SIZE);
	DWORDLONG AsynchSend(LPVOID data, DWORD size = MESSAGE_SIZE);
};

#if defined(USING_DDK)

// TODO: This section is for recent beta Windows DDK releases. Recent
//       DDK versions have had changes made in their network include
//       files. Because of the interaction between winsock.h and
//       winsock2.h the only reasonable place to make a prototype for
//       AcceptEx() is here. At some future date this issue may be
//       resolved and this prototype may be removed.
//
//       This change was made for DDK 3790.1247 and DDK 3790.1260.

#ifdef __cplusplus
extern "C" {
#endif

	BOOL PASCAL FAR
	    AcceptEx(IN SOCKET sListenSocket,
		     IN SOCKET sAcceptSocket,
		     IN PVOID lpOutputBuffer,
		     IN DWORD dwReceiveDataLength,
		     IN DWORD dwLocalAddressLength,
		     IN DWORD dwRemoteAddressLength, OUT LPDWORD lpdwBytesReceived, IN LPOVERLAPPED lpOverlapped);

#ifdef __cplusplus
}
#endif
#endif
#endif				// PORTTCP_DEFINED
