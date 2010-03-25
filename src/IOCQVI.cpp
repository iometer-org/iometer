/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / IOCQVI.cpp                                                ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implementation of the class CQVI, which provides a    ## */
/* ##               Completion Queue for asynchronous access to TargetVI  ## */
/* ##               objects. CQVI is an implementation of the abstract    ## */
/* ##               class CQ.                                             ## */
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

#include "IOCQVI.h"
#include "NetVI.h"
#include "IOTargetVI.h"

#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
#include <iostream>
using namespace std;
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_SOLARIS)
#include <iostream.h>
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

#define CQVI_DETAILS 0		// Set to 1 for additional debug messages.

//
// Initializing VI completion queue.
//
CQVI::CQVI()
{
	completion_queue = NULL;
	vi_target = NULL;
}

//
// Creating the CQ on the given VI NIC.
//
BOOL CQVI::Create(VINic * vi_nic, TargetVI * target)
{
	VIP_RETURN result;

#if CQVI_DETAILS
	cout << "Creating VI CQ for VI NIC " << vi_nic->nic_attributes.Name << endl;
#endif

	// Create a completion queue for the NIC.  The completion queue must be
	// able to handle the specified number of oustanding I/Os.
	if ((result = vipl.VipCreateCQ(vi_nic->nic, target->outstanding_ios, &completion_queue)) != VIP_SUCCESS) {
		cout << "*** Unable to create VI completion queue: " << vipl.Error(result) << endl;
		return FALSE;
	}
	// Record which target this completion is associated with.
	vi_target = target;

	return TRUE;
}

//
// Destroying the CQ.
// 
BOOL CQVI::Destroy()
{
	VIP_RETURN result;

#if CQVI_DETAILS
	cout << "Destroying VI CQ." << endl;
#endif

	// Destroy the completion queue.
	result = vipl.VipDestroyCQ(completion_queue);

	if (result != VIP_SUCCESS) {
		cout << "*** Unable to VI completion queue: " << vipl.Error(result) << endl;
		return FALSE;
	}

	completion_queue = NULL;
	vi_target = NULL;
	return TRUE;
}

//
// Getting and returning that status of any completed I/O calls.
//
ReturnVal CQVI::GetStatus(int *bytes, int *data, int delay)
{
	VIP_VI_HANDLE vi;
	VIP_BOOLEAN receive_done;
	VIP_DESCRIPTOR *descriptor;
	VIP_RETURN result;

#if CQVI_DETAILS
	cout << "Checking VI completion queue." << endl;
#endif

	// Verify that completion queue exists.  If not, it needs to be created
	// and then it can be retried.  This happens when testing connection rate.
	if (!completion_queue)
		return ReturnRetry;

	// Check completion queue for completed I/Os.
	if (delay)
		result = vipl.VipCQWait(completion_queue, delay, &vi, &receive_done);
	else
		result = vipl.VipCQDone(completion_queue, &vi, &receive_done);

	switch (result) {
		// An I/O has completed, process it.
	case VIP_SUCCESS:
#if CQVI_DETAILS
		cout << "   I/O has been posted to VI completion queue." << endl;
#endif

		if (receive_done == VIP_TRUE) {
			// Getting the completed receive.
			if (vipl.VipRecvDone(vi, &descriptor) != VIP_SUCCESS) {
				cout << "*** Unable to successfully complete receive." << endl;
				return ReturnError;
			}
		} else {
			// Getting the completed send.
			if (vipl.VipSendDone(vi, &descriptor) != VIP_SUCCESS) {
				cout << "*** Unable to successfully complete send." << endl;
				return ReturnError;
			}
		}
		break;

		// No I/O is done yet.
	case VIP_NOT_DONE:
	case VIP_TIMEOUT:
#if CQVI_DETAILS
		cout << "   no completions posted within the timeout period." << endl;
#endif
		return ReturnTimeout;

	default:
#if CQVI_DETAILS
		cout << "   error checking completion queue: " << vipl.Error(result) << endl;
#endif
		return ReturnError;
	}

	// Record information about the completed request.
	*bytes = descriptor->CS.Length;
	*data = descriptor->CS.ImmediateData;

	// See if the completion was for an I/O request or control data.
	if (*data != CONTROL_MESSAGE) {
		// It was for an I/O request.
#if CQVI_DETAILS
		cout << "   completed " << (receive_done ? "receive" : "send")
		    << " for transaction " << *data << endl;
#endif

		// Perform additional processing on completed receives.
		if (receive_done == VIP_TRUE) {
			// Increment the number of available receives.
			vi_target->available_receives++;

			// Decrement the number of outstanding receives to the VI.  If
			// there were no receives outstanding, then this completion is for
			// an I/O request that has not yet been made.  (This can happen
			// because the receives are pre-posted.)  In this case, signal a
			// retry to indicate that the I/O should not be processed as
			// complete until after it's been requested.
			if (vi_target->requested_receives-- <= 0) {
#if CQVI_DETAILS
				cout << "   receive has not been requested yet." << endl;
#endif
				return ReturnRetry;
			}
		}

		return ReturnSuccess;
	}
	//
	// Message contains information needed to maintain pre-posting receive
	// requirements.  Intercept these and update the VI connection management
	// information.
	//
#if CQVI_DETAILS
	cout << "   completion is for maintaining flow control." << endl;
#endif

	// If we received a control message, update that more sends are available.
	if (receive_done != VIP_FALSE)
		vi_target->more_sends_available = TRUE;

	// Return that no data messages completed, but we may not have waited
	// the full timeout period.
	return ReturnRetry;
}
