/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / IOTargetVI.cpp                                            ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implementation of the Target class for Virtual        ## */
/* ##               Interface Architecture (VI) targets.                  ## */
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
/* ##  Changes ...: 2003-10-15 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include "IOTargetVI.h"

#define TARGETVI_DETAILS 0	// Set to 1 for debug messages.

//
// Initializing member variables.
//
TargetVI::TargetVI()
{
	descriptors = NULL;
	descriptor_handle = 0;
	send_descriptors = NULL;
	recv_descriptors = NULL;

	descriptor_count = 0;
	send_count = 0;
	recv_count = 0;
	next_send_index = 0;
	next_recv_index = 0;

	data_buffer = NULL;
	data_buffer_handle = 0;
}

//
// Cleaning up VI target memory.
//
TargetVI::~TargetVI()
{
	// Release resources registered with VI NIC.
	if (descriptors && descriptor_handle)
		vi_nic.DeregisterMemory(descriptors, descriptor_handle);

	if (data_buffer && data_buffer_handle)
		vi_nic.DeregisterMemory(data_buffer, data_buffer_handle);

	// Release descriptor memory.
	if (descriptors)
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
		VirtualFree(descriptors, 0, MEM_RELEASE);
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_SOLARIS)
		free(descriptors);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

		// Close the VI NIC.
		vi_nic.Close();
}

//
// Initialize a VI target.  This allocates and registers the descriptors,
// opens the VI NIC, and associates the completion queue with the NIC.
//
BOOL TargetVI::Initialize(Target_Spec * target_info, CQ * completion_queue)
{
#ifdef _DEBUG
	cout << "Initializing VI target on NIC " << target_info->name << endl;
#endif

	// Open the VI NIC.
	if (!vi_nic.Open(target_info->name)) {
		cout << "*** VI target unable to open VI NIC." << endl;
		return FALSE;
	}
	// Allocate I/O descriptors for VI requests and align them on a page
	// boundary.

	// First free the descriptors.
	if (descriptors)
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
		VirtualFree(descriptors, 0, MEM_RELEASE);
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_SOLARIS)
		free(descriptors);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

		// Allocate enough descriptors to meet requested queue depth setting.
		// We need the requested number of receives, plus one send and two for
		// control for each queue depth specified
		descriptor_count = (target_info->vi_info.outstanding_ios + 1) * target_info->queue_depth + 2;
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	if (!(descriptors = (VIP_DESCRIPTOR *) VirtualAlloc(NULL, descriptor_count *
							    sizeof(VIP_DESCRIPTOR), MEM_COMMIT, PAGE_READWRITE)))
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_SOLARIS)
	if (!(descriptors = (VIP_DESCRIPTOR *) valloc(descriptor_count * sizeof(VIP_DESCRIPTOR))))
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	{
		// Could not allocate descriptors.  Signal failure.
		cout << "*** VI target could not allocate memory for descriptors." << endl;
		descriptors = NULL;
		descriptor_count = 0;
		send_count = 0;
		recv_count = 0;
		return FALSE;
	}

	// Register descriptors with VI NIC.
	if (!vi_nic.RegisterMemory(descriptors, descriptor_count * sizeof(VIP_DESCRIPTOR), &descriptor_handle)) {
		cout << "*** VI target unable to register descriptors with VI NIC." << endl;
		return FALSE;
	}
	// Allocate descriptors for sends and receives.
	send_count = target_info->queue_depth + 1;
	recv_count = descriptor_count - send_count;
	send_descriptors = descriptors;
	recv_descriptors = &descriptors[send_count];

	// Initialize information needed to maintain connection management.  See
	// VI specification for pre-posting requirements.  Send control information
	// after receiving just over half of the total queue depth.  This way, we
	// know that the remote side has enough pre-posted receives available to
	// continue receiving data messages (up to the queue depth), plus one more
	// to handle the control information.  And we have enough send descriptors
	// available to send enough data messages, plus the control message.
	control_countdown_start = (target_info->vi_info.outstanding_ios >> 1) + 1;
	outstanding_ios = descriptor_count;

	// Register data buffer with VI NIC.
	if (!vi_nic.RegisterMemory(data_buffer, data_buffer_size, &data_buffer_handle)) {
		cout << "*** Unable to register data buffer with VI NIC." << endl;
		return FALSE;
	}
	// Return a pointer to the VI completion queue used by this VI connection.
	completion_queue = &(vi.vi_cq);
	memcpy(&spec, target_info, sizeof(Target_Spec));

	// Record whether this is the server or client side of the connection.
	// It is the opposite of what the target is!
	if ((is_server = IsType(target_info->type, VIClientType))) {
		// If this is the server side of the connection, initialize the last
		// I/O to be a read.  This way, if no I/Os are ever done, the server
		// will be the one who initializes the close and does not need to
		// wait on the client.
		lastIO = READ;
#if TARGETVI_DETAILS
		cout << "Initialized VI server." << endl;
#endif
	} else {
		lastIO = WRITE;
#if TARGETVI_DETAILS
		cout << "Initialized VI client." << endl;
#endif
	}

	return TRUE;
}

//
// Opening a VI connection using the pre-specified NIC interface.  This is a
// blocking call.  Note data buffer to use for transfers and data buffer 
// handle returned from registering the data buffer with the VI NIC must have 
// been set before making this call.
//
BOOL TargetVI::Open(volatile TestState * test_state, int open_flag)
{
	int i;
	ReturnVal result;

	// Initialize information needed to maintain connection management.  See
	// VI specification for pre-posting requirements.
	control_countdown = control_countdown_start;
	available_receives = 0;
	more_sends_available = FALSE;
	requested_receives = 0;
	available_sends = spec.vi_info.outstanding_ios;
	next_send_index = 0;
	next_recv_index = 0;

	if (vi.Create(&vi_nic, this) != ReturnSuccess) {
		cout << "*** Unable to create VI on opened nic." << endl;
		return FALSE;
	}
	// Pre-post enough receives to ensure remote side can send immediately
	// after making the connection.
	for (i = 0; i < recv_count; i++) {
		if (PostRecv() != ReturnSuccess) {
			cout << "*** Unable to pre-post receives while opening VI." << endl;
			return FALSE;
		}
	}

#if TARGETVI_DETAILS
	int ii;

	cout << "In TargetVI::Open():" << endl;
	cout << "    is_server = " << is_server << endl;

	cout << "    spec.vi_info.local_address.HostAddressLen = " << spec.vi_info.local_address.HostAddressLen << endl;
	cout << "    spec.vi_info.local_address.HostAddress[0.." << spec.vi_info.local_address.HostAddressLen -
	    1 << "] = ";
	for (ii = 0; ii < spec.vi_info.local_address.HostAddressLen; ii++) {
		cout << hex << (int)spec.vi_info.local_address.HostAddress[ii] << dec << " ";
	}
	cout << endl;
	cout << "    spec.vi_info.local_address.DiscriminatorLen = " << spec.vi_info.local_address.
	    DiscriminatorLen << endl;
	cout << "    spec.vi_info.local_address.HostAddress[" << spec.vi_info.local_address.
	    HostAddressLen << ".." << spec.vi_info.local_address.HostAddressLen +
	    spec.vi_info.local_address.DiscriminatorLen - 1 << "] = ";
	for (ii = spec.vi_info.local_address.HostAddressLen;
	     ii < spec.vi_info.local_address.HostAddressLen + spec.vi_info.local_address.DiscriminatorLen; ii++) {
		cout << hex << (int)spec.vi_info.local_address.HostAddress[ii] << dec << " ";
	}
	cout << endl;

	cout << "    spec.vi_info.remote_address.HostAddressLen = " << spec.vi_info.remote_address.
	    HostAddressLen << endl;
	cout << "    spec.vi_info.remote_address.HostAddress[0.." << spec.vi_info.remote_address.HostAddressLen -
	    1 << "] = ";
	for (ii = 0; ii < spec.vi_info.remote_address.HostAddressLen; ii++) {
		cout << hex << (int)spec.vi_info.remote_address.HostAddress[ii] << dec << " ";
	}
	cout << endl;
	cout << "    spec.vi_info.remote_address.DiscriminatorLen = " << spec.vi_info.remote_address.
	    DiscriminatorLen << endl;
	cout << "    spec.vi_info.remote_address.HostAddress[" << spec.vi_info.remote_address.
	    HostAddressLen << ".." << spec.vi_info.remote_address.HostAddressLen +
	    spec.vi_info.remote_address.DiscriminatorLen - 1 << "] = ";
	for (ii = spec.vi_info.remote_address.HostAddressLen;
	     ii < spec.vi_info.remote_address.HostAddressLen + spec.vi_info.remote_address.DiscriminatorLen; ii++) {
		cout << hex << (int)spec.vi_info.remote_address.HostAddress[ii] << dec << " ";
	}
	cout << endl;
#endif

	// Open VI connection depending on whether this is the client or server
	// side of the connection.
	if (is_server) {
		// Accept an incomming connection until either the accept completes 
		// successfully or the test state is idle.
		do {
			result = vi.Accept(&spec.vi_info.local_address);
		} while (result == ReturnRetry && *test_state != TestIdle);

		if (result == ReturnError) {
			cout << "*** Unable to accept connection opening VI target." << endl;
			return FALSE;
		}
	} else {
		// Connect to the specified VI connection on the server.
		do {
			result = vi.Connect(&spec.vi_info.local_address, &spec.vi_info.remote_address);
		} while (result == ReturnRetry && *test_state != TestIdle);

		if (result == ReturnError) {
			cout << "*** Unable to connect to server opening VI target." << endl;
			return FALSE;
		}
	}
	return TRUE;
}

//
// Closing a VI connection.
//
BOOL TargetVI::Close(volatile TestState * test_state)
{
	// Allow the receiving side to disconnect first.
	if (lastIO == WRITE) {
		while (vi.CheckForDisconnect() == ReturnRetry && *test_state != TestIdle) ;
	}

	return (vi.Close() == ReturnSuccess);
}

//
// Performing read operation.  Note that for clients, the call is actually a 
// write.  The data buffer to use for I/O transfers has already been set, so
// we ignore the buffer passed in.
//
ReturnVal TargetVI::Read(LPVOID buffer, Transaction * trans)
{
	// Clients perform a write.
	if (!is_server)
		return Send(trans);

	// Servers perform a read.
	return Receive(trans);
}

//
// Performing an actual receive (read) from the VI connection.
//
ReturnVal TargetVI::Receive(Transaction * trans)
{
	lastIO = READ;
#if TARGETVI_DETAILS
	cout << "Performing I/O read request for transaction " << trans->request_number << endl;
#endif

	// If any receives are available, pre-post one.
	if (available_receives) {
		// Posting an additional receive.
		if (PostRecv() != ReturnSuccess) {
			cout << "*** Failed to pre-post a receive over VI." << endl;
			return ReturnError;
		}
		available_receives--;

		// See if we should send notification to remote side that we have posted 
		// additional receives.
		if (--control_countdown <= 0) {
			if (SendControl() != ReturnSuccess) {
				cout << "*** Failed to send control message over VI." << endl;
				return ReturnError;
			}
			control_countdown = control_countdown_start;
		}
	}
	// We need to check on the read that we entered this function to 
	// actually perform.  Due to pre-posting requirements, we need to see 
	// if the remote side has already sent the data before we called this.  
	// If so, then the receive could have been processed by the completion 
	// queue *before* we requested it.  The completion queue will have 
	// trapped this and modified the number of requested receives.
	if (requested_receives++ < 0) {
		// Receive is already done and has already posted to the completion
		// queue.  Return immediate success to signal that the I/O should be
		// processed immediately and will not appear in the completion queue.
#if TARGETVI_DETAILS
		cout << "Receive was already done." << endl;
#endif
		return ReturnSuccess;
	}
	// Receive has not yet completed, and its completion will still go to the
	// completion queue.
	return ReturnPending;
}

//
// Performing write operation.  Note that for clients, the call is actually a
// read.  The data buffer to use for I/O transfers has already been set, so
// we ignore the buffer passed in.
//
ReturnVal TargetVI::Write(LPVOID buffer, Transaction * trans)
{
	// Servers perform a write.
	if (is_server)
		return Send(trans);

	// Clients perform a read.
	return Receive(trans);
}

//
// Post a receive.
//
ReturnVal TargetVI::PostRecv()
{
	// Format the receive descriptor for the maximum transfer size possible.
	// Not all of this formatting may be necessary.
	recv_descriptors[next_recv_index].CS.SegCount = 1;
	recv_descriptors[next_recv_index].CS.Control = VIP_CONTROL_OP_SENDRECV & VIP_CONTROL_IMMEDIATE;
	recv_descriptors[next_recv_index].CS.Reserved = 0;
	recv_descriptors[next_recv_index].CS.ImmediateData = 0;
	recv_descriptors[next_recv_index].CS.Length = data_buffer_size;
	recv_descriptors[next_recv_index].CS.Status = 0;

	recv_descriptors[next_recv_index].DS[0].Local.Data.Address = data_buffer;
	recv_descriptors[next_recv_index].DS[0].Local.Handle = data_buffer_handle;
	recv_descriptors[next_recv_index].DS[0].Local.Length = data_buffer_size;

	if (vi.Receive(&recv_descriptors[next_recv_index], descriptor_handle)
	    != ReturnSuccess) {
		cout << "*** Failed to post a receive for VI connection." << endl;
		return ReturnError;
	}
	// Receives complete in the order posted, so update index of next one
	// available using a circular queue.
	if (++next_recv_index >= recv_count)
		next_recv_index = 0;

	return ReturnSuccess;
}

//
// Performing an actual write (send) over a VI connection.
//
ReturnVal TargetVI::Send(Transaction * trans)
{
	lastIO = WRITE;
#if TARGETVI_DETAILS
	cout << "Performing I/O write request for transaction " << trans->request_number << endl;
#endif

	// Check to see if the completion queue has marked that more sends are
	// available for requesting.
	if (more_sends_available) {
#if TARGETVI_DETAILS
		cout << "Updating number of sends available." << endl;
#endif

		more_sends_available = FALSE;
		available_sends += control_countdown_start;

		// We need to repost the receive needed for control flow.
		if (PostRecv() != ReturnSuccess) {
			cout << "*** Unable to post send for control flow." << endl;
			return ReturnError;
		}
	}
	// We cannot perform the send unless the remote side has pre-posted the
	// receive.  Checking to make sure that remote side is ready.
	if (!available_sends) {
#if TARGETVI_DETAILS
		cout << "No sends are available." << endl;
#endif

		// Remote side is not ready.  Abort the operation to signal that the
		// send failed and should be retried.
		return ReturnRetry;
	}

	available_sends--;

	// Format descriptor.  Not all of this formatting may be necessary.
	send_descriptors[next_send_index].CS.SegCount = 1;
	send_descriptors[next_send_index].CS.Control = VIP_CONTROL_OP_SENDRECV & VIP_CONTROL_IMMEDIATE;
	send_descriptors[next_send_index].CS.Reserved = 0;
	send_descriptors[next_send_index].CS.ImmediateData = trans->request_number;
	send_descriptors[next_send_index].CS.Length = trans->size;
	send_descriptors[next_send_index].CS.Status = 0;

	send_descriptors[next_send_index].DS[0].Local.Data.Address = data_buffer;
	send_descriptors[next_send_index].DS[0].Local.Handle = data_buffer_handle;
	send_descriptors[next_send_index].DS[0].Local.Length = trans->size;

	if (vi.Send(&send_descriptors[next_send_index], descriptor_handle)
	    != ReturnSuccess) {
		cout << "*** VI send failed to remote VI target." << endl;
		return ReturnError;
	}
	// Sends complete in the order posted, so update index of next
	// available using a circular queue.
	if (++next_send_index >= send_count)
		next_send_index = 0;

	return ReturnPending;
}

//
// Send a control message to the remote side informing it that additional
// receives are now available.
//
ReturnVal TargetVI::SendControl()
{
	// Format the descriptor to send with the control information.
	// Not all of this formatting may be necessary.
	send_descriptors[next_send_index].CS.SegCount = 1;
	send_descriptors[next_send_index].CS.Control = VIP_CONTROL_OP_SENDRECV | VIP_CONTROL_IMMEDIATE;
	send_descriptors[next_send_index].CS.Reserved = 0;
	send_descriptors[next_send_index].CS.ImmediateData = CONTROL_MESSAGE;
	send_descriptors[next_send_index].CS.Length = 0;
	send_descriptors[next_send_index].CS.Status = 0;

	send_descriptors[next_send_index].DS[0].Local.Data.Address = data_buffer;
	send_descriptors[next_send_index].DS[0].Local.Handle = data_buffer_handle;
	send_descriptors[next_send_index].DS[0].Local.Length = 0;

#if TARGETVI_DETAILS
	cout << "Sending control message to remote VI connection." << endl;
#endif

	if (vi.Send(&send_descriptors[next_send_index], descriptor_handle)
	    != ReturnSuccess) {
		cout << "*** Failed to send VI control message." << endl;
		return ReturnError;
	}
	// Sends complete in the order posted, so update index of next
	// available using a circular queue.
	if (++next_send_index >= send_count)
		next_send_index = 0;

	return ReturnSuccess;
}
