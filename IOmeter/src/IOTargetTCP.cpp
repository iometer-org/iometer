/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / IOTargetTCP.cpp                                           ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implementation of the Target class for TCP/IP         ## */
/* ##               sockets.                                              ## */
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
/* ##  Changes ...: 2004-03-27 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Code cleanup to ensure common style.                ## */
/* ##               - Applied Thayne Harmon's patch for supporting        ## */
/* ##                 Netware support (on I386).                          ## */
/* ##               2003-10-15 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include "IOTargetTCP.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TargetTCP::TargetTCP()
{
	io_cq = NULL;
	is_server = CLIENT;
}

//
// Initializes the local side of a TCP connection depending on whether
// the remote side is a TCP server or client.
//
BOOL TargetTCP::Initialize(Target_Spec * target_info, CQ * cq)
{
	// Record completion queue.
	io_cq = (CQAIO *) cq;

	if (IsType(target_info->type, TCPClientType)) {
		// Target is a client - local connection is a server.
		return InitServer(target_info);
	} else if (IsType(target_info->type, TCPServerType)) {
		// Target is a server - local connection is a client.
		return InitClient(target_info);
	} else {
		cout << "***Invalid target type in TargetTCP::Initialize()." << endl;
		return FALSE;
	}
}

//
// Initialize a TCP server at a specific IP address, specified as a string.  
// Returns the server's port number in *server_port_out (USHRT_MAX in 
// case of error).
//
BOOL TargetTCP::InitServer(Target_Spec * target_info)
{
	is_server = SERVER;
	memcpy(&spec, target_info, sizeof(Target_Spec));

	// Initialize lastIO with opposite values for server and client
	// to avoid hang on Close() if no I/Os are performed.
	lastIO = READ;

	// Set the address of the server socket.
	tcp_socket.SetAddress(SERVER, spec.name);
	// Create and bind the server socket.
	if (tcp_socket.Create(SERVER) != ReturnSuccess) {
		cout << "*** Failed to create a TCP server port." << endl;
		target_info->tcp_info.local_port = USHRT_MAX;
		return FALSE;
	}
	// Record the assigned port number (in host byte order).
	target_info->tcp_info.local_port = ntohs(tcp_socket.server_address.sin_port);
	return TRUE;
}

//
// Initialize a TCP client, given the server's IP address (specified as 
// a string) and port number. Returns the client's port number in 
// *client_port_out (USHRT_MAX in case of error).
//
BOOL TargetTCP::InitClient(Target_Spec * target_info)
{
	is_server = CLIENT;
	memcpy(&spec, target_info, sizeof(Target_Spec));

	// Initialize lastIO with opposite values for server and client
	// to avoid hang on Close() if no I/Os are performed.
	lastIO = WRITE;

	// Set the address of the client socket.
	tcp_socket.SetAddress(CLIENT, spec.name);

	// Create and bind the client socket.
#ifdef _DEBUG
	cout << "Creating socket " << spec.name << "." << endl << flush;
#endif

	// Make sure we can create the socket.
	if (tcp_socket.Create(CLIENT) != ReturnSuccess) {
		cout << "*** Failed to create TCP client socket." << endl;
		spec.tcp_info.local_port = USHRT_MAX;
		return FALSE;
	}
	// Record the assigned port number (in host byte order).
	spec.tcp_info.local_port = ntohs(tcp_socket.client_address.sin_port);

	// Socket will be re-created in Open() with the same
	// address and port number, so close it now to prevent
	// binding error WSAEADDRINUSE.
	tcp_socket.Close(CLIENT);

	// Store the ip_address for the remote connection.
	// we will connect to specified address
	tcp_socket.SetAddress(SERVER, spec.tcp_info.remote_address, spec.tcp_info.remote_port);
	return TRUE;
}

//
// Open a socket.  Blocking call: does not return until connection is established.
//
BOOL TargetTCP::Open(volatile TestState * test_state, int open_flag)
{
	ReturnVal result;

	if (is_server) {
		// Accept an incomming connection on the server socket
		// until either the accept completes successfully or
		// the test state changes.
		tcp_socket.SetTimeout(1, 0);

		do {
			result = tcp_socket.Accept();
		}
		while (result == ReturnRetry && *test_state != TestIdle);

		if (result == ReturnError) {
			cout << "*** Unable to accept connection opening TCP target." << endl;
			return FALSE;
		}
	} else {
		// Create and bind the client socket.
		// We cannot re-use the same port number.  Set it to 0 so that the system
		// will assign a new port.
		tcp_socket.client_address.sin_port = htons(0);
		tcp_socket.Create(CLIENT);

		// Connect to the server address specified in InitClient()
		do {
			result = tcp_socket.ConnectSocket(&tcp_socket.server_address);
#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
			if (result != ReturnSuccess) {
				// According to connect(3XN):
				//      If connect() fails, the state of the socket
				//      is unspecified.  Portable applications
				//      should close the file descriptor and
				//      create a new socket before attempting to
				//      reconnect.

				tcp_socket.Close(CLIENT);
				if (tcp_socket.Create(CLIENT) != ReturnSuccess) {
					cout << "*** Failed to re-create TCP client socket." << endl;
					return FALSE;
				}
			}
#endif				// UNIX
		}
		while (result == ReturnRetry && *test_state != TestIdle);

		if (result == ReturnError) {
			cout << "*** Unable to connect to server opening TCP target." << endl;
			return FALSE;
		}
	}

	// Indicating where completed asynchronous transfers should be queued.
	return (CreateIoCompletionPort((HANDLE) tcp_socket.client_socket, io_cq->completion_queue, 0, 1) != NULL);
}

//
// Close a socket.
//
BOOL TargetTCP::Close(volatile TestState * test_state)
{
	// Allow the receiving side to disconnect first.
	if (lastIO == WRITE) {
		while (tcp_socket.WaitForDisconnect() == ReturnRetry && *test_state != TestIdle) ;
	}

	return (tcp_socket.Close(CLIENT) == ReturnSuccess);
}

//
// Perform receive operation.  NOTE: Direction of call is reversed for client!
//
ReturnVal TargetTCP::Read(LPVOID buffer, Transaction * trans)
{
	DWORD bytes_read;
	int retval;

	if (is_server) {
		// Keep track of whether the last I/O performed by the target was a read or a write.
		// This is used to determine which side closes first.
		lastIO = READ;

		if ((retval = tcp_socket.Receive(buffer, trans->size, &bytes_read,
						 &trans->asynchronous_io)) == ReturnError) {
			return ReturnError;
		}
	} else {
		// Keep track of whether the last I/O performed by the target was a read or a write.
		// This is used to determine which side closes first.
		lastIO = WRITE;

		if ((retval = tcp_socket.Send(buffer, trans->size, &bytes_read,
					      &trans->asynchronous_io)) == ReturnError) {
			return ReturnError;
		}
	}
	// If the I/O completed immediately or is deferred, the completion
	// will still be pending in the completion queue.
#if defined(IOMTR_OSFAMILY_UNIX) && defined(IMMEDIATE_AIO_COMPLETION)
	// This prevents the code from looking in the completionQ for immediately completed IOs.
	if (retval == ReturnSuccess)
		return ReturnSuccess;
#endif				// UNIX && IMMEDIATE_AIO_COMPLETION
	return ReturnPending;
}

//
// Perform send operation.  NOTE: Direction of call is reversed for client!
//
ReturnVal TargetTCP::Write(LPVOID buffer, Transaction * trans)
{
	DWORD bytes_sent;
	int retval;

	if (is_server) {
		// Keep track of whether the last I/O performed by the target was a read or a write.
		// This is used to determine which side closes first.
		lastIO = WRITE;

		if ((retval = tcp_socket.Send(buffer, trans->size, &bytes_sent,
					      &trans->asynchronous_io)) == ReturnError) {
			return ReturnError;
		}
	} else {
		// Keep track of whether the last I/O performed by the target was a read or a write.
		// This is used to determine which side closes first.
		lastIO = READ;

		if ((retval = tcp_socket.Receive(buffer, trans->size, &bytes_sent,
						 &trans->asynchronous_io)) == ReturnError) {
			return ReturnError;
		}
	}
	// If the I/O completed immediately or is deferred, the completion
	// will still be pending in the completion queue.
#if defined(IOMTR_OSFAMILY_UNIX) && defined(IMMEDIATE_AIO_COMPLETION)
	// This prevents the code from looking in the completionQ for immediately completed IOs.
	if (retval == ReturnSuccess)
		return ReturnSuccess;
#endif				// UNIX && IMMEDIATE_AIO_COMPLETION
	return ReturnPending;
}
