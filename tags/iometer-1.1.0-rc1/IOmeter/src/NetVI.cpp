/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / NetVI.cpp                                                 ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implements the NetVI class for communication between  ## */
/* ##               network workers using the VI technology.              ## */
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
/* ##               - Replaced the [BIG|LITTLE]_ENDIAN_ARCH defines by    ## */
/* ##                 IsBigEndian() function calls.                       ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */

#define VI_DETAILS 0		// Set to 1 to enable additional debug messages.

#include "NetVI.h"

#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
#include <iostream>
using namespace std;
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_SOLARIS)
#include <iostream.h>
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

//
// Initialization.
//
NetVI::NetVI()
{
	vi = NULL;
}

//
// Creating a new virtual interface on the specified VI NIC.
//
ReturnVal NetVI::Create(VINic * nic, TargetVI * vi_target)
{
	VIP_VI_STATE vi_state;
	VIP_BOOLEAN send_queue_empty, receive_queue_empty;
	VIP_VI_ATTRIBUTES vi_attributes;
	VIP_RETURN result;

#if VI_DETAILS
	cout << "Creating VI on nic " << nic->nic_attributes.Name << endl;
#endif

	// Create the VI completion queue for the specified VI NIC.
	if (!vi_cq.Create(nic, vi_target))
		return ReturnError;

	// Set the VI attributes to match the NIC it's created on.
	vi_attributes.ReliabilityLevel = VIP_SERVICE_RELIABLE_DELIVERY;
	vi_attributes.MaxTransferSize = nic->nic_attributes.MaxTransferSize;
	vi_attributes.QoS = 0;	// not defined by spec as of 7/24/98
	vi_attributes.Ptag = nic->nic_mem_attributes.Ptag;
	vi_attributes.EnableRdmaWrite = nic->nic_mem_attributes.EnableRdmaWrite;
	vi_attributes.EnableRdmaRead = nic->nic_mem_attributes.EnableRdmaRead;

	// Create the virtual interface.
	result = vipl.VipCreateVi(nic->nic, &vi_attributes, vi_cq.completion_queue, vi_cq.completion_queue, &vi);
	if (result != VIP_SUCCESS) {
		cout << "*** Unable to create VI: " << vipl.Error(result) << endl;
		vi_cq.Destroy();
		return ReturnError;
	}
	// Get the VI attributes that were actually assigned.
	if ((result = vipl.VipQueryVi(vi, &vi_state, &vi_attributes,
				      &send_queue_empty, &receive_queue_empty)) != VIP_SUCCESS) {
		cout << "*** Unable to retrieve VI attributes: " << vipl.Error(result) << endl;
		vipl.VipDestroyVi(&vi);
		vi_cq.Destroy();
		return ReturnError;
	}
	vi_nic = nic;

	return ReturnSuccess;
}

//
// Closing the VI connection.
//
ReturnVal NetVI::Close()
{
	VIP_DESCRIPTOR *descriptor;
	VIP_RETURN result;

#if VI_DETAILS
	cout << "Closing VI on " << vi_nic->nic_attributes.Name << endl;
#endif

	// See if the VI exists.  If it doesn't, we're done.
	if (!vi)
		return ReturnSuccess;

	// Disconnect the VI.  This will mark all outstanding requests as complete.
	if ((result = vipl.VipDisconnect(vi)) != VIP_SUCCESS) {
		cout << "*** Unable to disconnect VI while closing: " << vipl.Error(result) << endl;
		return ReturnError;
	}
	// Remove outstanding requests from the completion queue.
	do {
		result = vipl.VipRecvDone(vi, &descriptor);
	}
	while (result == VIP_SUCCESS || result == VIP_DESCRIPTOR_ERROR && descriptor);
	do {
		result = vipl.VipSendDone(vi, &descriptor);
	}
	while (result == VIP_SUCCESS || result == VIP_DESCRIPTOR_ERROR && descriptor);

	// Destroy the VI.
	if ((result = vipl.VipDestroyVi(vi)) != VIP_SUCCESS) {
		cout << "*** Unable to destroy VI while closing: " << vipl.Error(result) << endl;
		return ReturnError;
	}
	// Destroy the associated completion queue.
	if (!vi_cq.Destroy()) {
		cout << "*** Unable to destroy VI completion queue." << endl;
		return ReturnError;
	}

	vi = NULL;
	return ReturnSuccess;
}

//
// Waiting for the remote side to disconnect from the VI connection.
//
ReturnVal NetVI::CheckForDisconnect()
{
	VIP_VI_STATE vi_state;
	VIP_VI_ATTRIBUTES vi_attributes;
	VIP_BOOLEAN no_sends;
	VIP_BOOLEAN no_receives;

	// See if the VI exists, if not, we're done.
	if (!vi)
		return ReturnSuccess;

	// Get the current state of the VI connection.
	if (vipl.VipQueryVi(vi, &vi_state, &vi_attributes, &no_sends, &no_receives) != VIP_SUCCESS) {
		return ReturnError;
	}
	// See if it's still connected.
	if (vi_state == VIP_STATE_CONNECTED)
		return ReturnRetry;

	return ReturnSuccess;
}

//
// Using the VI "client/server" connection model to wait and accept a VI 
// connection.
// This is called by the worker designated as the server.
//
ReturnVal NetVI::Accept(VIP_NET_ADDRESS * local_nic_address)
{
	VIP_RETURN result;
	VIP_NET_ADDRESS remote_nic_address;
	VIP_VI_ATTRIBUTES remote_vi_attributes;
	VIP_CONN_HANDLE connection;

#if VI_DETAILS
	cout << "Waiting to accept VI connection on " << vi_nic->nic_attributes.Name << endl;
#endif

	// Waiting until the client can connect.
	switch (result = vipl.VipConnectWait(vi_nic->nic, local_nic_address,
					     RETRY_DELAY, &remote_nic_address, &remote_vi_attributes, &connection)) {
		// Client ready to connect, accept the connection.
	case VIP_SUCCESS:
#if VI_DETAILS
		cout << "VipConnectWait succeeded!  Accepting connection." << endl;
#endif
		switch (result = vipl.VipConnectAccept(connection, vi)) {
		case VIP_SUCCESS:
#if VI_DETAILS
			cout << "VipConnectAccept succeeded!" << endl;
#endif
			return ReturnSuccess;
		case VIP_TIMEOUT:
		case VIP_ERROR_RESOURCE:
#if VI_DETAILS
			cout << "VipConnectAccept() failed (trying again): " << vipl.Error(result) << endl;
#endif
			return ReturnRetry;
		default:
			cout << "*** VipConnectAccept() failed: " << vipl.Error(result) << endl;
			return ReturnError;
		}

		// Client did not connect within the time limit.
	case VIP_TIMEOUT:
	case VIP_ERROR_RESOURCE:
#if VI_DETAILS
		cout << "VipConnectWait() failed (trying again): " << vipl.Error(result) << endl;
#endif
		return ReturnRetry;
	default:
		cout << "*** VipConnectWait() failed: " << vipl.Error(result) << endl;
		return ReturnError;
	}
}

//
// Using the VI "client/server" connection model to connect to a waiting VI 
// connection.
// This is called by the worker designated as the client.
//
ReturnVal NetVI::Connect(VIP_NET_ADDRESS * local_nic_address, VIP_NET_ADDRESS * remote_nic_address)
{
	VIP_RETURN result;
	VIP_VI_ATTRIBUTES remote_vi_attributes;

#if VI_DETAILS
	cout << "Requesting VI connection from " << vi_nic->nic_attributes.Name << endl;
#endif

	// Request to connect to a remote VI.
	switch (result = vipl.VipConnectRequest(vi, local_nic_address,
						remote_nic_address, RETRY_DELAY, &remote_vi_attributes)) {
	case VIP_SUCCESS:
#if VI_DETAILS
		cout << "VipConnectRequest succeeded!" << endl;
#endif
		return ReturnSuccess;
	case VIP_TIMEOUT:
	case VIP_NO_MATCH:
	case VIP_ERROR_RESOURCE:
	case VIP_REJECT:
#if VI_DETAILS
		cout << "VipConnectRequest() failed (trying again): " << vipl.Error(result) << endl;
#endif
		return ReturnRetry;
	default:
		cout << "*** VipConnectRequest() failed: " << vipl.Error(result) << endl;
		return ReturnError;
	}
}

//
// Sending data to the remote VI connection.
//
ReturnVal NetVI::Send(VIP_DESCRIPTOR * descriptor, VIP_MEM_HANDLE descriptor_handle)
{
	VIP_RETURN result;

#if VI_DETAILS
	cout << "Sending " << descriptor->CS.Length << " bytes over VI NIC " << vi_nic->nic_attributes.Name << endl;
#endif

	// Send the I/O request.
	if ((result = vipl.VipPostSend(vi, descriptor, descriptor_handle)) != VIP_SUCCESS) {
		cout << "*** Unable to post VI send: " << vipl.Error(result) << endl;
		return ReturnError;
	}
	return ReturnSuccess;
}

//
// Pre-posting a receiving for data from the remote VI connection.
//
ReturnVal NetVI::Receive(VIP_DESCRIPTOR * descriptor, VIP_MEM_HANDLE descriptor_handle)
{
	VIP_RETURN result;

#if VI_DETAILS
	cout << "Receiving " << descriptor->CS.Length << " bytes over VI NIC " << vi_nic->nic_attributes.Name << endl;
#endif

	// Send the I/O request.
	if ((result = vipl.VipPostRecv(vi, descriptor, descriptor_handle)) != VIP_SUCCESS) {
		cout << "*** Unable to post VI receive: " << vipl.Error(result) << endl;
		return ReturnError;
	}
	return ReturnSuccess;
}
