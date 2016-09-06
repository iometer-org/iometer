/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer & Dynamo) / IOPortTCP.cpp                                 ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: This class covers the communication between Iometer   ## */
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
/* ##               2005-04-10 (mingz@ele.uri.edu)                        ## */
/* ##               - Add type cast to remove compile warning for Solaris.## */
/* ##               2005-04-01 (mingz@ele.uri.edu)                        ## */
/* ##               - Added space in binding error msg print out          ## */
/* ##               2005-01-12 (henryx.w.tieman@intel.com)                ## */
/* ##               - fixed include file differences for Windows DDK.     ## */
/* ##               2004-09-01 (henryx.w.tieman@intel.com)                ## */
/* ##               - Cleanup some warnings with type casts.              ## */
/* ##               2004-03-27 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Code cleanup to ensure common style.                ## */
/* ##               - Applied Thayne Harmon's patch for supporting        ## */
/* ##                 Netware support (on I386).                          ## */
/* ##               2003-07-19 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Assimilated the patch from Robert Jones which is    ## */
/* ##                 needed to build under Solaris 9 on x86 (i386).      ## */
/* ##               2003-07-18 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-04-26 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Improved error messages for better support.         ## */
/* ##               2003-04-25 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Updated the global debug flag (_DEBUG) handling     ## */
/* ##                 of the source file (check for platform etc.).       ## */
/* ##               2003-03-04 (joe@eiler.net)                            ## */
/* ##               - Added a #ifdef for SOLARIS support                  ## */
/* ##               2003-03-01 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Cut out the Windows Pipes support for               ## */
/* ##                 communication efforts.                              ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */

#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
#include "StdAfx.h"
#endif

#include "IOPortTCP.h"

#if defined(IOMTR_OS_LINUX)
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#if defined(IOMTR_OS_NETWARE)
#include <sys/select.h>
#endif

#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
#if defined(USING_DDK)
#include "winsock.h"
#else
#include "mswsock.h"
#endif
#endif

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

//////////////////////////////////////////////////////////////////////
// Static data member initialization.
//////////////////////////////////////////////////////////////////////

unsigned int PortTCP::sockets_in_use = 0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PortTCP::PortTCP(BOOL synch)
:  Port(synch)			// call Port constructor
{
	type = PORT_TYPE_TCP;

	server_socket = (int)INVALID_SOCKET;
	client_socket = (int)INVALID_SOCKET;

	// initialize WinSock -- do this only if it has not yet been initialized in this process

#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	if (sockets_in_use++ == 0) {
#ifdef _DEBUG
		cout << "Initializing WinSock." << endl << flush;
#endif

		int retval;
		WSADATA wd;

		// request WinSock version 2.0
		retval = WSAStartup(MAKEWORD(2, 0), &wd);

		if (retval != 0) {
			*errmsg << "===> ERROR: Could not initialize WinSock." << endl
			    << "     [PortTCP::PortTCP() in " << __FILE__ << " line " << __LINE__ << "]" << endl
			    << "     retval = " << retval << ends;
			OutputErrMsg();
		}
	}
	// The accept_ex_buffer is used in PortTCP::Accept().  We need to provide enough space 
	// for two addresses plus 16 additional bytes per address (see AcceptEx() documentation).
	// This data will not be used, but we can't tell AcceptEx() not to give it to us, so
	// we need to provide the buffer anyway.
	accept_ex_buffer = new char[sizeof(struct sockaddr_in) + 16 + sizeof(struct sockaddr_in) + 16 + 1];
#endif				// IOMTR_OS_WIN32 || IOMTR_OS_WIN64
// Note: synch is not used.
}

PortTCP::~PortTCP()
{
	(void)CloseSocket(&server_socket, "server");
	(void)CloseSocket(&client_socket, "client");

	// clean up WinSock -- do this only if nobody else is using it in this process

#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	if (sockets_in_use-- == 0) {
#if PORT_DETAILS || _DETAILS
		cout << "Cleaning up WinSock." << endl;
#endif

		if (WSACleanup() != 0) {
			*errmsg << "===> ERROR: Could not clean up WinSock." << endl
			    << "     [PortTCP::~PortTCP() in " << __FILE__ << " line " << __LINE__ << "]" << endl
			    << "     return code = " << WSAGetLastError() << ends;
			OutputErrMsg();
		}
	}

	delete accept_ex_buffer;
#endif				// IOMTR_OS_WIN32 || IOMTR_OS_WIN64
}

//
// Creates a TCP socket for communication.  Used by server.
// "size" argument is ignored, but accepted for compatibility with PortPipe.
// (PortPipe does no longer exists, but one day there might be an successor)
//
BOOL PortTCP::Create(char *port_name, char *remote_name, DWORD size, unsigned short port_number)
{
	struct sockaddr_in sin;
	socklen_t buflen;

	network_port = port_number;	// listen at specified port (0 = system assigns port number)

	if (port_name) {
		strcpy(name, port_name);
	} else {
		// get the unqualified local host name
		if (gethostname(name, MAX_NETWORK_NAME) == SOCKET_ERROR) {
			*errmsg << "===> ERROR: Getting local host name failed." << endl
			    << "     [PortTCP::Create() in " << __FILE__ << " line " << __LINE__ << "]" << endl
			    << "     errno = " << WSAGetLastError() << ends;
			OutputErrMsg();
			return FALSE;
		}
	}

	if (remote_name) {
		strcpy(network_name, remote_name);
	} else {
		// get the specified host's first network address
		struct hostent *hostinfo = gethostbyname(name);

		if (hostinfo == NULL) {
			*errmsg << "===> ERROR: Getting host name for \"" << name << "\" failed." << endl
			    << "     [PortTCP::Create() in " << __FILE__ << " line " << __LINE__ << "]" << endl
			    << "     errno = " << WSAGetLastError() << ends;
			OutputErrMsg();
			return FALSE;
		}
		memcpy(&sin.sin_addr.s_addr, hostinfo->h_addr_list[0], hostinfo->h_length);
		strncpy(network_name, inet_ntoa(sin.sin_addr), sizeof(network_name) - 1);

	}

	//
	// Create socket.
	//

#if PORT_DETAILS || _DETAILS
	cout << "Creating socket." << endl;
	WSASetLastError(0);
#endif

	if (synchronous) {
		server_socket = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);
	}
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	else {
		server_socket = WSASocket(AF_INET, SOCK_STREAM, PF_UNSPEC, NULL, 0, WSA_FLAG_OVERLAPPED);
	}
#endif

	if (server_socket == (int)INVALID_SOCKET) {
		*errmsg << "===> ERROR: Creating socket failed." << endl
		    << "     [PortTCP::Create() in " << __FILE__ << " line " << __LINE__ << "]" << endl
		    << "     errno = " << WSAGetLastError() << ends;
		OutputErrMsg();
		return FALSE;
	}
	//
	// Bind to socket.
	//

#if PORT_DETAILS || _DETAILS
	cout << "Binding to socket " << name << "." << endl;
	WSASetLastError(0);
#endif

	sin.sin_family = AF_INET;	// use Internet Protocol
	sin.sin_addr.s_addr = htonl(INADDR_ANY);	// listen at any available address
	sin.sin_port = htons(network_port);	// listen at specified port (0 = system assigns port number)

	if (bind(server_socket, (struct sockaddr *)&sin, sizeof(sin)) != 0) {
		*errmsg << "===> ERROR: Binding to socket " << name << " failed." << endl
		    << "     [PortTCP::Create() in " << __FILE__ << " line " << __LINE__ << "]" << endl
		    << "     errno = " << WSAGetLastError() << ends;
		OutputErrMsg();
		return FALSE;
	}
	// get actual port number in use
	buflen = sizeof(sin);
	if (getsockname(server_socket, (struct sockaddr *)&sin, &buflen) != 0) {
		*errmsg << "===> ERROR: Getting information about server socket failed." << endl
		    << "     [PortTCP::Create() in " << __FILE__ << " line " << __LINE__ << "]" << endl
		    << "     errno = " << WSAGetLastError() << ends;
		OutputErrMsg();
		return FALSE;
	}
	network_port = ntohs(sin.sin_port);

#if PORT_DETAILS || _DETAILS
	cout << "Bound server socket on port " << network_port << "." << endl;
#endif

	return TRUE;
}

//
// Connects to an existing port waiting to accept a connection.  Used by client.
//     port_name is a hostname or a string IP address ("xxx.xxx.xxx.xxx").
//     port_number is the port number to use at that address.
//
BOOL PortTCP::Connect(char *port_name, unsigned short port_number)
{
	unsigned long server_address = INADDR_NONE;
	struct sockaddr_in sin;
	struct hostent *hostinfo;
	int retval;

	// get hostname to connect to (default local host name).

	if (strlen(port_name)) {
		strcpy(name, port_name);
	} else {
		if (gethostname(name, MAX_NETWORK_NAME) == SOCKET_ERROR) {
			*errmsg << "===> ERROR: Getting local host name failed." << endl
			    << "     [PortTCP::Connect() in " << __FILE__ << " line " << __LINE__ << "]" << endl
			    << "     errno = " << WSAGetLastError() << ends;
			OutputErrMsg();
			return FALSE;
		}
	}

	if (atoi(name) > 0) {
		// looks like a numeric IP address was specified
		// if this is not an IP address (e.g., a name starting with a digit),
		//      INADDR_NONE (0xffffffff) will be returned.
		server_address = inet_addr(name);
	}

	if (server_address == INADDR_NONE) {
		// treat it as a hostname
		hostinfo = gethostbyname(name);
		if (hostinfo == NULL) {
			*errmsg << "===> ERROR: Getting host information for \"" << name << "\" failed." << endl
			    << "     [PortTCP::Connect() in " << __FILE__ << " line " << __LINE__ << "]" << endl
			    << "     errno = " << WSAGetLastError() << ends;
			OutputErrMsg();
			return FALSE;
		}

		memcpy(&server_address, hostinfo->h_addr, hostinfo->h_length);
	}
	// create socket for the connnection.

#if PORT_DETAILS || _DETAILS
	cout << "Creating socket " << name << "." << endl;
	WSASetLastError(0);
#endif

	if (synchronous) {
		client_socket = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);
	}
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	else {
		client_socket = WSASocket(AF_INET, SOCK_STREAM, PF_UNSPEC, NULL, 0, WSA_FLAG_OVERLAPPED);
	}
#endif

	if (client_socket == (int)INVALID_SOCKET) {
		*errmsg << "===> ERROR: Creating socket failed." << endl
		    << "     [PortTCP::Connect() in " << __FILE__ << " line " << __LINE__ << "]" << endl
		    << "     errno = " << WSAGetLastError() << ends;
		OutputErrMsg();
		return FALSE;
	}
	// specify protocol, port, and address to connect to.

	sin.sin_family = AF_INET;	// use Internet Protocol
	sin.sin_port = htons(port_number);	// connect to this port
	sin.sin_addr.s_addr = server_address;	// connect to this server

#if PORT_DETAILS || _DETAILS
	cout << "Host address: " << inet_ntoa(sin.sin_addr) << endl;
#endif

	// attempt to connect (keep trying forever if necessary).

	do {
#if PORT_DETAILS || _DETAILS
		cout << "Attempting to connect to socket " << name << "." << endl;
		WSASetLastError(0);
#endif

		retval = connect(client_socket, (struct sockaddr *)&sin, sizeof(sin));

		if (retval != 0) {
#if PORT_DETAILS || _DETAILS
			// WSAECONNREFUSED means the server isn't up yet or is busy,
			// don't print an error message
#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
			if (errno != ECONNREFUSED)
#elif defined(IOMTR_OSFAMILY_WINDOWS)
			if (WSAGetLastError() != WSAECONNREFUSED)
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
			{
				*errmsg << "===> ERROR: Connecting to socket " << name << " failed." << endl
				    << "     [PortTCP::Connect() in " << __FILE__ << " line " << __LINE__ << "]" << endl
				    << "     errno = " << WSAGetLastError() << ends;
				OutputErrMsg();
			}
#endif
#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
			// According to connect(3XN):
			//      If connect() fails, the state of the socket
			//      is  unspecified.  Portable  applications
			//      should close the file descriptor and
			//      create a new socket before attempting to
			//      reconnect.

#if PORT_DETAILS || _DETAILS
			cout << "Re-creating socket " << name << "." << endl;
			WSASetLastError(0);
#endif

			if (close(client_socket) < 0) {
				*errmsg << "===> ERROR: Closing socket " << name << " failed." << endl
				    << "     [PortTCP::Connect() in " << __FILE__ << " line " << __LINE__ << "]" << endl
				    << "     errno = " << WSAGetLastError() << ends;
				OutputErrMsg();
				return FALSE;
			}

			if ((client_socket = socket(AF_INET, SOCK_STREAM, PF_UNSPEC))
			    == (int)INVALID_SOCKET) {
				*errmsg << "===> ERROR: Recreating socket " << name << " failed." << endl
				    << "     [PortTCP::Connect() in " << __FILE__ << " line " << __LINE__ << "]" << endl
				    << "     errno = " << WSAGetLastError() << ends;
				OutputErrMsg();
				return FALSE;
			}
#endif				// IOMTR_OSFAMILY_UNIX
			Sleep(RETRY_DELAY);
		}
	}
	while (retval != 0);

	printf("Successful PortTCP::Connect\n  - port name: %s\n", name);

	return TRUE;
}

//
// Begin accepting a connection to an existing port.  Used by server.
// For synchronous port, blocks until a connection is made (returns TRUE) or an error occurs (returns FALSE).
// For asynchronous port, returns immediately.  Returns TRUE for success or FALSE if any error occurs.
//     Call IsAcceptComplete() to determine if it has completed, and GetAcceptResult() to get result.
//
BOOL PortTCP::Accept()
{
	//
	// Listen to socket.
	//

#if PORT_DETAILS || _DETAILS
	cout << "Listening to socket " << name << "." << endl;
	WSASetLastError(0);
#endif

	if (listen(server_socket, 0) != 0)	// allow at most one connection at a time
	{
		*errmsg << "===> ERROR: Listening to socket " << name << " failed." << endl
		    << "     [PortTCP::Accept() in " << __FILE__ << " line " << __LINE__ << "]" << endl
		    << "     errno = " << WSAGetLastError() << ends;
		OutputErrMsg();
		return FALSE;
	}
	//
	// Accept connections to socket.
	//

#if PORT_DETAILS || _DETAILS
	cout << "Accepting connections to socket " << name << "." << endl;
	WSASetLastError(0);
#endif

	if (synchronous) {
		// don't need any info about who we're talking to
		client_socket = accept(server_socket, NULL, NULL);

		if (client_socket == (int)INVALID_SOCKET) {
			*errmsg << "===> ERROR: Accepting connection to socket " << name << " failed." << endl
			    << "     [PortTCP::Accept() in " << __FILE__ << " line " << __LINE__ << "]" << endl
			    << "     errno = " << WSAGetLastError() << ends;
			OutputErrMsg();
			return FALSE;
		} else {
			return TRUE;
		}
	}
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	else {
		DWORD bytes_received;

		if (!InitOverlapped(&accept_overlapped)) {
			*errmsg << "===> ERROR: Creating OVERLAPPED structure for socket " << name << " failed." << endl
			    << "     [PortTCP::Accept() in " << __FILE__ << " line " << __LINE__ << "]" << ends;
			OutputErrMsg();
			return FALSE;
		}

		//
		// Create client socket.
		//

#if PORT_DETAILS || _DETAILS
		cout << "Creating client socket for " << name << "." << endl;
		WSASetLastError(0);
#endif

		client_socket = WSASocket(AF_INET, SOCK_STREAM, PF_UNSPEC, NULL, 0, WSA_FLAG_OVERLAPPED);

		if (client_socket == INVALID_SOCKET) {
			*errmsg << "===> ERROR: Creating client socket for " << name << " failed." << endl
			    << "     [PortTCP::Accept() in " << __FILE__ << " line " << __LINE__ << "]" << endl
			    << "     errno = " << WSAGetLastError() << ends;
			OutputErrMsg();
			return FALSE;
		}
		//
		// Accept connections to socket.
		//
		if (AcceptEx(server_socket, client_socket, accept_ex_buffer, 0,	// read no data, only the two addresses, into accept_ex_buffer
			     sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
			     &bytes_received, &accept_overlapped)) {
#if PORT_DETAILS || _DETAILS
			cout << "Connection accepted." << endl;
#endif

			return TRUE;
		} else {
			if (WSAGetLastError() == WSA_IO_PENDING)	// Read started OK...
			{
				return TRUE;
			} else {
				*errmsg << "===> ERROR: AcceptEx() failed." << endl
				    << "     [PortTCP::Accept() in " << __FILE__ << " line " << __LINE__ << "]" << ends;
				OutputErrMsg();
				return FALSE;
			}
		}
	}
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
	else {
		*errmsg << "===> ERROR: Attempting asynchronous connection in Unix." << endl
		    << "     [PortTCP::Accept() in " << __FILE__ << " line " << __LINE__ << "]" << ends;
		OutputErrMsg();
		return (FALSE);
	}
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif				// (asynch code)
}

//
// Called from the "server" side of a port to disconnect a "client" from
// its port.
//
BOOL PortTCP::Disconnect()
{
	return CloseSocket(&client_socket, "client");
}

//
// Get result of completed asynchronous Accept().  Be sure that IsAcceptComplete() has returned TRUE
// before calling this function.  Does not block.
//
BOOL PortTCP::GetAcceptResult()
{
	if (synchronous) {
		return (client_socket != (int)INVALID_SOCKET);
	}
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	DWORD bytes_read;
	BOOL result;

	result = GetOverlappedResult((HANDLE) client_socket, &accept_overlapped, &bytes_read, FALSE);

	return result;
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
	*errmsg << "===> ERROR: Asynchronous socket accept attempted under Unix." << endl
	    << "     [PortTCP::GetAcceptResult() in " << __FILE__ << " line " << __LINE__ << "]" << ends;
	OutputErrMsg();
	return (FALSE);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
}

//
// Closes connections to a port.  Called from either "client" or "server" side.
//
BOOL PortTCP::Close()
{
	BOOL client_close_state, server_close_state;

	// make two separate calls to make sure both are closed
	client_close_state = CloseSocket(&client_socket, "client");
	server_close_state = CloseSocket(&server_socket, "server");

	return (client_close_state && server_close_state);
}

//
// Call which receives data from a connected port.  Blocking call for synchronous port; non-blocking
// call for asynchronous port.  Note that a receive call will not receive data from itself.
//
DWORDLONG PortTCP::Receive(LPVOID msg, DWORD size)
{
	if (synchronous) {
		return SynchReceive(msg, size);
	}
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	else {
		return AsynchReceive(msg, size);
	}
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
	else {
		*errmsg << "===> ERROR: Asynchronous TCP attempted under Unix." << endl
		    << "     [PortTCP::Receive() in " << __FILE__ << " line " << __LINE__ << "]" << ends;
		OutputErrMsg();
		return (0);
	}
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
}

//
// Synchronous implementation of Receive().
//
DWORDLONG PortTCP::SynchReceive(LPVOID msg, DWORD size)
{
	if (!synchronous) {
		return PORT_ERROR;
	}

	int bytes_read;
	DWORD total_bytes_read = 0;
	DWORD total_size = size;

	while (total_bytes_read < total_size) {
		bytes_read = recv(client_socket, (char *)msg, size, 0);
		if (bytes_read == 0 || bytes_read == SOCKET_ERROR) {
			// socket has been closed, return error
			cerr << "Get port error in SynchReceive." << endl;
			return PORT_ERROR;
		}
#if PORT_DETAILS || _DETAILS
		cout << "Received " << bytes_read << " of " << size << " bytes from socket " << name << "." << endl;
#endif

		total_bytes_read += bytes_read;	// add bytes read so far to total
		msg = ((char *)msg) + bytes_read;	// move pointer to end of bytes read so far
		size -= bytes_read;	// decrease number of bytes to read in next recv()
	}

	return (DWORDLONG) total_bytes_read;
}

//
// Asynchronous implementation of Receive().
//
DWORDLONG PortTCP::AsynchReceive(LPVOID msg, DWORD size)
{
	if (synchronous) {
		return PORT_ERROR;
	}
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)

	DWORD bytes_read;

	if (!InitOverlapped(&receive_overlapped)) {
		*errmsg << "===> ERROR: Creating OVERLAPPED structure for socket " << name << " failed." << endl
		    << "     [PortTCP::AsynchReceive() in " << __FILE__ << " line " << __LINE__ << "]" << ends;
		OutputErrMsg();
		return PORT_ERROR;
	}

	if (ReadFile((HANDLE) client_socket, msg, size, &bytes_read, &receive_overlapped)) {
#if PORT_DETAILS || _DETAILS
		cout << "Received " << bytes_read << " of " << size << " bytes from socket " << name << "." << endl;
#endif

		return (DWORDLONG) bytes_read;
	} else {
		if (GetLastError() == ERROR_IO_PENDING)	// Read started OK...
		{
			return 0;	// not an error code, but no bytes read yet
		}
		return PORT_ERROR;
	}
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
	else {
		*errmsg << "===> ERROR: Asynchronous TCP attempted under Unix." << endl
		    << "     [PortTCP::AsynchReceive() in " << __FILE__ << " line " << __LINE__ << "]" << ends;
		OutputErrMsg();
		return (PORT_ERROR);
	}
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
}

//
// Get result of completed asynchronous Receive() (number of bytes read, -1 for any error).  Be sure that 
// IsReceiveComplete() has returned TRUE before calling this function.  Does not block.
//
// NOTE: For asynchronous PortTCP's, IsReceiveComplete() will return TRUE when the receive operation 
// completes, which does NOT necessarily indicate that all requested bytes have been read.  It is the
// caller's responsibility to check the return value of GetReceiveResult() and, if not all requested
// bytes have been read, to post additional Receive() calls until all the bytes are read.
//
DWORDLONG PortTCP::GetReceiveResult()
{
	if (synchronous) {
		return PORT_ERROR;
	}
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	DWORD bytes_read;

	if (GetOverlappedResult((HANDLE) client_socket, &receive_overlapped, &bytes_read, FALSE)) {
		return (DWORDLONG) bytes_read;
	} else {
		return PORT_ERROR;
	}
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
	else {
		*errmsg << "===> ERROR: Asynchronous TCP attempted under Unix." << endl
		    << "     [PortTCP::GetReceiveResult() in " << __FILE__ << " line " << __LINE__ << "]" << ends;
		OutputErrMsg();
		return (PORT_ERROR);
	}
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
}

//
// Call which sends data to a connected port.  Blocking call for synchronous port; non-blocking
// call for asynchronous port.  Note that a process will not send data to itself.
//
DWORDLONG PortTCP::Send(LPVOID msg, DWORD size)
{
	if (synchronous) {
		return SynchSend(msg, size);
	} else {
		return AsynchSend(msg, size);
	}
}

//
// Synchronous implementation of Send().
//
DWORDLONG PortTCP::SynchSend(LPVOID msg, DWORD size)
{
	if (!synchronous) {
		return PORT_ERROR;
	}

	int bytes_written;
	DWORD total_bytes_written = 0;
	DWORD total_size = size;
	static int error_count=0;

#ifdef _DEBUG
	cout << "in SynchSend " << endl;
#endif
	while (total_bytes_written < total_size) {
		bytes_written = send(client_socket, (char *)msg, size, 0);

		if (bytes_written == 0 || bytes_written == SOCKET_ERROR) {
			error_count++;
			if (error_count == 1 || error_count%10 == 0) {
				// socket has been closed, return error
				*errmsg << "===> ERROR: Send failed." << endl
					<< "     [PortTCP::SynchSend() in " << __FILE__ << " line " << __LINE__ << "]" << endl
					<< "     errno = " << WSAGetLastError() << endl
					<< "     error repeats " << error_count << " time(s)." << ends;

				OutputErrMsg();
			}
			return PORT_ERROR;
		}
		else
		{
			if (error_count)
			{
				*errmsg << "===> ERROR: Send failed." << endl
					<< "     [PortTCP::SynchSend() in " << __FILE__ << " line " << __LINE__ << "]" << endl
					<< "     errno = " << WSAGetLastError() << endl
					<< "     error repeated " << error_count << " time(s)." << ends;
				OutputErrMsg();
			}

			error_count=0;
		}

#if PORT_DETAILS || _DETAILS
		cout << "Sent " << bytes_written << " of " << size << " bytes to socket " << name << "." << endl;
#endif

		total_bytes_written += bytes_written;	// add bytes written so far to total
		msg = ((char *)msg) + bytes_written;	// move pointer to end of bytes written so far
		size -= bytes_written;	// decrease number of bytes to write in next send()
	}

	return (DWORDLONG) total_bytes_written;
}

//
// Asynchronous implementation of Send().
//
DWORDLONG PortTCP::AsynchSend(LPVOID msg, DWORD size)
{
	if (synchronous) {
		return PORT_ERROR;
	}
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	DWORD bytes_written;

	if (!InitOverlapped(&send_overlapped)) {
		return PORT_ERROR;
	}

	if (WriteFile((HANDLE) client_socket, msg, size, &bytes_written, &send_overlapped)) {
#if PORT_DETAILS || _DETAILS
		cout << "Sent " << bytes_written << " of " << size << " bytes to socket " << name << "." << endl;
#endif
		return (DWORDLONG) bytes_written;
	} else {
		if (GetLastError() == ERROR_IO_PENDING)	// Write started OK...
		{
			return 0;	// not an error code, but no bytes read yet
		}
		return PORT_ERROR;
	}
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
	else {
		*errmsg << "===> ERROR: Asynchronous TCP attempted under Unix." << endl
		    << "     [PortTCP::AsynchSend() in " << __FILE__ << " line " << __LINE__ << "]" << ends;
		OutputErrMsg();
		return (PORT_ERROR);
	}
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
}

//
// Get result of completed asynchronous Send() (number of bytes read, -1 for any error).  Be sure that 
// IsSendComplete() has returned TRUE before calling this function.  Does not block.
//
DWORDLONG PortTCP::GetSendResult()
{
	if (synchronous) {
		return PORT_ERROR;
	}
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	DWORD bytes_written;

	if (GetOverlappedResult((HANDLE) client_socket, &send_overlapped, &bytes_written, FALSE)) {
		return (DWORDLONG) bytes_written;
	} else {
		return PORT_ERROR;
	}
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
	else {
		*errmsg << "===> ERROR: Asynchronous TCP attempted under Unix." << endl
		    << "     [PortTCP::GetSendResult() in " << __FILE__ << " line " << __LINE__ << "]" << ends;
		OutputErrMsg();
		return (PORT_ERROR);
	}
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
}

//
// Non blocking call which reads data from a connected port without
// removing it.  Note that a process does not receive data from itself.
//
DWORD PortTCP::Peek()
{
#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE)
	int bytes_available = 0;
#elif defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS) || defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	DWORD bytes_available = 0;
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	BOOL success = FALSE;
	fd_set sock_set;	//For use with select
	timeval timeout = { 0, 0 };	//For use with select, initialized to zero for instaneous return
	char buf[1];		// we have to provide a buffer, so we provide just one character of buffer

	if (synchronous) {
		FD_ZERO(&sock_set);	// clear the fd_set structure.
		FD_SET(client_socket, &sock_set);	// Add the one and only socket to it.
		if (select(client_socket + 1, &sock_set, NULL, NULL, &timeout)) {
			//if there is data available or select returns an error, 
			//we want bytes_available to set by the result of recv().
			bytes_available = recv(client_socket, buf, sizeof(buf), MSG_PEEK);
		} else {
			//otherwise, there are no bytes available, this prevents recv() from blocking
			bytes_available = 0;
		}
		success = (bytes_available != SOCKET_ERROR); // was (long)bytes_available ...
	}
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	else {
		DWORD flags;	// flags must be provided as a (DWORD *), so we need to have a DWORD variable
		WSABUF wsabuf[1];

		flags = MSG_PEEK;
		wsabuf[0].buf = buf;
		wsabuf[0].len = sizeof(buf);

		success = (WSARecv(client_socket, wsabuf, 1, &bytes_available, &flags, NULL, NULL) == 0);
	}
#endif

	if (success) {
#if PORT_DETAILS || _DETAILS
		cout << "Peeked " << bytes_available << " bytes from socket " << name << "." << endl;
#endif
		return (DWORD) bytes_available;
	} else {
#if PORT_DETAILS || _DETAILS
		*errmsg << "===> ERROR: Peeking from socket " << name << " failed." << endl
		    << "     [PortTCP::Peek() in " << __FILE__ << " line " << __LINE__ << "]" << endl
		    << "     errno = " << WSAGetLastError() << ends;
		OutputErrMsg();
#endif
		return 0;	// no data available at this time, maybe later
	}
}

//
// Utility function to close a socket.  socket_name parameter is used only for debugging outputs.
//
BOOL PortTCP::CloseSocket(SOCKET * s, const char *socket_name)
{
	if (*s == (int)INVALID_SOCKET) {
#if PORT_DETAILS || _DETAILS
		cout << socket_name << " socket is already closed." << endl;
#endif
		return TRUE;
	}
#if PORT_DETAILS || _DETAILS
	cout << "Shutting down " << socket_name << " socket." << endl;
#endif

	if (shutdown(*s, SD_BOTH) != 0) {
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
		// WSAENOTCONN = "socket is not connected", which occurs on the server side if the socket is
		// closed with no Accept() active -- this is no problem since we're just closing it anyway
		if (WSAGetLastError() != WSAENOTCONN) {
			*errmsg << "===> ERROR: Shutting down socket " << socket_name << " failed." << endl
			    << "     [PortTCP::CloseSocket() in " << __FILE__ << " line " << __LINE__ << "]" << endl
			    << "     errno = " << WSAGetLastError() << ends;
			OutputErrMsg();
			return FALSE;
		}
#endif
#if PORT_DETAILS || _DETAILS
		cout << socket_name << " socket is already shut down." << endl;
#endif
	}
#if PORT_DETAILS || _DETAILS
	cout << "Closing " << socket_name << " socket." << endl;
#endif

#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	if (closesocket(*s) != 0)
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
	if (close(*s) != 0)
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	{
		*errmsg << "===> ERROR: Closing socket " << socket_name << " failed." << endl
		    << "     [PortTCP::CloseSocket() in " << __FILE__ << " line " << __LINE__ << "]" << endl
		    << "     errno = " << WSAGetLastError() << ends;
		OutputErrMsg();
		return FALSE;
	}

	*s = (int)INVALID_SOCKET;
	return TRUE;
}
