/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer & Dynamo) / IOTransfers.h                                 ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Include file defining data structures used to define  ## */
/* ##               a "transaction" (one or more related I/O operations)  ## */
/* ##               in Dynamo.                                            ## */
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
/* ##  Changes ...: 2004-03-26 (daniel.scheibli@edelbyte.org)             ## */
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
#ifndef IO_TRANSFERS_DEFINED
#define IO_TRANSFERS_DEFINED

#include "IOCommon.h"
#include "vipl.h"

#include "pack.h"

//
// Generic structure to hold information that may be passed to a thread.
//
struct Thread_Info {
	int id;			// Thread's identification.  Used by thread to
					// determine what work it is to do, such as which disk to access.
	void *parent;	// Pointer used to get parent object.
} STRUCT_ALIGN_IOMETER;

//
// Information about a single transaction performed by a thread.  Note that a 
// worker has at most one I/O outstanding at a time for a given transaction.
//
struct Transaction {
	// NOTE!  This must be the first member of this structure!
	OVERLAPPED asynchronous_io;

	// Target to use for current transaction.
	int target_id;
	//
	int request_number;	// each trans. holds its own index in the Transaction array
	//
	// these are set when a new transaction is started in a transaction slot
	DWORD request_size;	// size of request posting
	DWORD reply_size;	// size of reply posting
	DWORDLONG start_transaction;	// starting time of entire transaction
	//
	// these are changed when the transaction's REQUESTs have
	// all been completed and replies are about to be queued
	BOOL is_read;		// is this I/O a read (TRUE) or a write (FALSE)?
	//
	// these change per I/O within the transaction
	// ("size" must also be changed in the event of a partial I/O)
	DWORD size;		// number of bytes to be transferred in the current request
	DWORDLONG start_IO;	// time I/O request was made
	//
	// these two values are decremented when a request or reply I/O is completed,
	// not when it is queued
	int remaining_requests;	// PING (number of requests remaining in this transaction)
	int remaining_replies;	// PONG (number of replies remaining in this transaction)
} STRUCT_ALIGN_IOMETER;

#include "unpack.h"

#endif
