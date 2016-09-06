/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / NetTCP.h                                                  ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Class definition for the NetAsyncTCP class.           ## */
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
/* ##  Changes ...: 2005-04-18 (raltherr@apple.com)                       ## */
/* ##               - Support for MacOS X                                 ## */
/* ##               2005-01-19 (mingz@ele.uri.edu)                        ## */
/* ##               - Removed socketlen_t define.                         ## */
/* ##               2004-09-01 (henryx.w.tieman@intel.com)                ## */
/* ##               - Changed SOCKET to CONNECTION for improved clarity,  ## */
/* ##                 because SOCKET has a standard meaning outside       ## */
/* ##                 Iometer.                                            ## */
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
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef NET_ASYNC_TCP_DEFINED
#define NET_ASYNC_TCP_DEFINED

#include "Network.h"
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
#include "winsock2.h"
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define SOCKADDR_IN sockaddr_in

typedef struct linger LINGER;
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
 // nop
#elif defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
#define socklen_t int
#elif defined(IOMTR_OS_NETWARE)
#define socklen_t unsigned int
#define TCP_NODELAY	1
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

class NetAsyncTCP:public Network {
      public:
	// Member Functions.
	NetAsyncTCP();
	~NetAsyncTCP();
	virtual ReturnVal Create(BOOL create_server);
	virtual ReturnVal Connect(const char *ip_address, unsigned short port_number);
	ReturnVal ConnectSocket(SOCKADDR_IN * address);
	virtual ReturnVal Accept();
	virtual ReturnVal Destroy();
	virtual ReturnVal Receive(LPVOID buffer, DWORD bytes, LPDWORD return_value,
				  LPOVERLAPPED asynchronous_io, DWORD flags = 0);
	virtual ReturnVal Send(LPVOID buffer, DWORD bytes, LPDWORD return_value,
			       LPOVERLAPPED asynchronous_io, DWORD flags = 0);
	virtual DWORD Peek();
	virtual ReturnVal Close(BOOL close_server);
	ReturnVal WaitForDisconnect();
	void SetTimeout(int sec, int usec);
	void SetAddress(BOOL set_server, const char *ip_address = NULL, unsigned short port_num = 0);

	// Member Variables
	SOCKADDR_IN server_address;	// IP address, port.
	SOCKADDR_IN client_address;	// IP address, port.

	CONNECTION server_socket;	// Socket where server listens for 
	// connections.  Not used by client.
	CONNECTION client_socket;	// Socket used for client/server data 
	// transmission.
#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
	struct File server_fp;	// The actual structures that will hold
	struct File client_fp;	// the client and server sockets on UNIX.
	int maxfd;		// The max nos of fds' for select() call.

#endif				// IOMTR_OSFAMILY_UNIX

      protected:
	// Member Functions.
	ReturnVal CreateSocket(CONNECTION * s);
	ReturnVal BindSocket(CONNECTION * s, SOCKADDR_IN * address);
	ReturnVal CloseSocket(CONNECTION * s);
	void SetOptions(CONNECTION * s);
	char *Error(int error_num);

	// Member Variables.
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	WSABUF wsa_buf;
#endif
	timeval timeout;
	static LONG sockets_in_use;	// Used to control WinSock 
	// initialization/cleanup.
};

#endif
