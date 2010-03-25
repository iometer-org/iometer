/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer & Dynamo) / IOMessage.h                                   ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: This headerfile defines the message structures which  ## */
/* ##               are passed during communication between Iometer and   ## */
/* ##               Dynamo.                                               ## */
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
/* ##               2003-07-27 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Removed the [BIG|LITTLE]_ENDIAN_ARCH defines.       ## */
/* ##               - Cleanup the formatting.                             ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef MESSAGE_DEFINED
#define MESSAGE_DEFINED

#include "IOCommon.h"
#include "IOTest.h"

///////////////////////////////////////////////////////////////////////////////
// Message purposes.
//
// The following messages require an immediate reply.
//
#define REPLY_FILTER		0x10000000	// DO NOT CHANGE!!!
#define LOGIN			REPLY_FILTER + 1	// DO NOT CHANGE!!!
#define ADD_WORKERS		REPLY_FILTER + 2
#define REPORT_TARGETS		REPLY_FILTER + 3

#define START			REPLY_FILTER + 4
#define BEGIN_IO		REPLY_FILTER + 5
#define RECORD_OFF		REPLY_FILTER + 6
#define STOP			REPLY_FILTER + 7
#define REPORT_RESULTS		REPLY_FILTER + 8
#define REPORT_UPDATE		REPLY_FILTER + 9

#define	STOP_PREPARE		REPLY_FILTER + 10

#define SET_TARGETS		REPLY_FILTER + 11

#define SET_ACCESS		REPLY_FILTER + 12

///////////////////////////////////////////////////////////////////////////////
// These messages are sent by Iometer to Dynamo at login
// For compatibility reasons, do NOT change these definitions.
//
#define LOGIN_OK		REPLY_FILTER + 15	// DO NOT CHANGE!!!
#define WRONG_VERSION		REPLY_FILTER + 16	// DO NOT CHANGE!!!
//
///////////////////////////////////////////////////////////////////////////////

//
// The following messages require a reply, but not immediately.
//
#define DELAY_REPLY_FILTER	0x00100000	// DO NOT CHANGE!!!
#define PREP_DISKS		DELAY_REPLY_FILTER + 1

//
// A reply is NOT expected (or wanted) for the following messages.
//
#define	NO_REPLY_FILTER		0x01000000	// DO NOT CHANGE!!!
#define READY			NO_REPLY_FILTER + 1
#define RECORD_ON		NO_REPLY_FILTER + 2
#define RESET			NO_REPLY_FILTER + 3
#define EXIT			NO_REPLY_FILTER + 4
//
///////////////////////////////////////////////////////////////////////////////

#ifdef FORCE_STRUCT_ALIGN
#include "pack8.h"
#endif

// Different data a message can contain.
union Message_Data {
	Manager_Info manager_info;

	//
	// Specifications used to access specific targets.
	//
	Target_Spec targets[MAX_TARGETS];

	// Access specifications for a test.
	Test_Spec spec;

	// 
	// Test results for Dynamo, including CPU utilization.
	//
	Manager_Results manager_results;
	// Results for worker threads, including target results.
	Worker_Results worker_results;
};

// Format of informative and data messages.
struct Message {
	int purpose;		// Reason message was sent.  See constants above.
	int data;		// Brief amount of data sent with the message.
};

struct Data_Message {
	int count;		// Number of data items in the message.
#ifndef FORCE_STRUCT_ALIGN
	char pad[4];		// coz of Solaris - NT differences
#endif
	Message_Data data;	// Detailed data sent with the data message.
};

#ifdef FORCE_STRUCT_ALIGN
#include "unpack8.h"
#endif

#define MESSAGE_SIZE		sizeof( Message )
#define DATA_MESSAGE_SIZE	sizeof( Data_Message )

void reorder(Message &);
void reorder(Data_Message &, int union_type, int send_recv);

#define DATA_MESSAGE_MANAGER_INFO	1
#define DATA_MESSAGE_TARGET_SPEC	2
#define DATA_MESSAGE_TEST_SPEC		3
#define DATA_MESSAGE_MANAGER_RESULTS	4
#define DATA_MESSAGE_WORKER_RESULTS	5

#endif
