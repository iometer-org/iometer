/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / IOTargetVI.h                                              ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Interface for an implementation of the Target class   ## */
/* ##               for Virtual Interface Architecture (VI) targets.      ## */
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
#ifndef	TARGETVI_DEFINED
#define	TARGETVI_DEFINED

#include "IOTarget.h"
#include "NetVI.h"
#include "IOCQVI.h"
#include "Network.h"

///////////////////////////////////////////////////////////////////////////////
// Used to indicate that immediate data contains information used
// to maintain control flow.
#define CONTROL_MESSAGE 0xFFFFFFFF
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Abstracts VI connections used as targets of I/O requests.
//
// Note that applications should verify that vipl.dll is loaded before using 
// this class.
//
///////////////////////////////////////////////////////////////////////////////
class TargetVI:public Target {
      public:

	TargetVI();
	~TargetVI();

	///////////////////////////////////////////////////////////////////////////
	// Public functions used to manage a VI target.
	//
	BOOL Initialize(Target_Spec * target_info, CQ * completion_queue);
	//
	BOOL Open(volatile TestState * test_state, int open_flag = 0);
	BOOL Close(volatile TestState * test_state);
	//
	ReturnVal Read(LPVOID buffer, Transaction * trans);
	ReturnVal Write(LPVOID buffer, Transaction * trans);
      private:
	 ReturnVal Send(Transaction * trans);
	ReturnVal Receive(Transaction * trans);
	ReturnVal SendControl();
	//
	ReturnVal PostRecv();
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Pointers to descriptors to use for I/O requests and related information.
	//
	// One block of memory is allocated for all requests, then partitioned
	// for sends and receives.  Both VI sends and receives complete in the
	// order that they are posted with respect to other sends/receives.
	//
	VIP_DESCRIPTOR *descriptors;
	VIP_MEM_HANDLE descriptor_handle;
	int descriptor_count;
	int send_count;
	int recv_count;
	//
	// First half of descriptors are allocated for sends.
	VIP_DESCRIPTOR *send_descriptors;
	int next_send_index;
	//
	// Second half of descriptors are allocated for receives.
	VIP_DESCRIPTOR *recv_descriptors;
	int next_recv_index;
	//
	///////////////////////////////////////////////////////////////////////////

	// Is this the client or server side of the connection.  Note that this is
	// independent of VI client/server or peer-to-peer connections.
	BOOL is_server;
	ReadWriteType lastIO;	// Was the last I/O a read or write?

	///////////////////////////////////////////////////////////////////////////
	// Resouces used to perform I/O to a VI target.
	//
      public:
	 NetVI vi;
	// Information needed to maintain connection information over a VI.  Due to VI
	// pre-posting requirements, the VI completion queue needs to intercept and
	// process control flow messages.  When this occurs, it updates the following
	// information needed by a VI target to request I/O.
	//
	// Number of receives that may be posted.  Due to pre-posting requirements
	// all receives may be outstanding when we go to post another.
	int available_receives;
	//
	// Number of receives that have been posted, but have not yet completed.
	// This number will be negative if receives complete *before* being 
	// requested.  (This can happen due to pre-posting requirements.)
	int requested_receives;
	//
	// Indicates that a control message was received signalling that additional
	// receives have been posted on the remote side.  When set, this (local) 
	// side of the connection updates the number of available sends.
	BOOL more_sends_available;
	//
	// Total number of I/Os that can be outstanding at any one time, including
	// sends, receives, and any control messages.
	int outstanding_ios;
	//
      private:
	// The total number of sends that are available to be sent.  If more than
	// this number are sent, we cannot guarantee that the remote side has pre-
	// posted enough receives to get them all.
	int available_sends;
	//
	// Manage the number of receives that can be posted before we need to send
	// notification to the remote side that additional receives have been pre-
	// posted.
	int control_countdown;
	int control_countdown_start;
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// VI buffer and handle to buffer memory must be set before the target is
	// initialized.
      public:
	 VINic vi_nic;
	//
	char *data_buffer;
	int data_buffer_size;
      private:
	 VIP_MEM_HANDLE data_buffer_handle;
	//
	///////////////////////////////////////////////////////////////////////////
};

#endif				// TARGETVI_DEFINED
