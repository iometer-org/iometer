/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / IOGrunt.h                                                 ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Contains the definition of Dynamo's worker class.     ## */
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
/* ##  Changes ...: 2005-04-05 (ACurrid@nvidia.com)                       ## */
/* ##               - Added a class member outstanding_io.                ## */
/* ##               2004-05-13 (lamontcranston41@yahoo.com)               ## */
/* ##               - conditionalize vinic.h include; Add cur_trans_slots ## */
/* ##               2003-08-02 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Integrated the modification contributed by          ## */
/* ##                 Vedran Degoricija, to get the code compile with     ## */
/* ##                 the MS DDK on IA64.                                 ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-05-07 (yakker@aparity.com)                       ## */
/* ##               - Applied the iometerCIOB5.2003.05.02.patch file      ## */
/* ##                 (avoiding cache line collisions and performance     ## */
/* ##                 lock-ups for some chipsets).                        ## */
/* ##               2003-05-07 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef	GRUNT_DEFINED
#define	GRUNT_DEFINED

#include "IOCommon.h"
#include "IOTarget.h"
#include "IOAccess.h"
#include "IOTest.h"
#include "IOTransfers.h"

#if defined(IOMTR_SETTING_VI_SUPPORT)
#include "VINic.h"
#endif

// Wrappers for Grunt member functions, used by _beginthread()
void CDECL Prepare_Disk_Wrapper(void *disk_thread_info);
void CDECL Grunt_Thread_Wrapper(void *grunt);

#define TIMEOUT_TIME	1000	// max time to wait for an asynch
				// I/O to complete, in milliseconds

//
// An I/O worker who does all of the dirty work.
//
class Grunt {
      public:
	Grunt();
	~Grunt();

	///////////////////////////////////////////////////////////////////////////
	// Indicates type of targets assigned to grunt.
	//
	TargetType type;
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Functions to set test configuration information.
	//
	BOOL Set_Targets(int count, Target_Spec * target_specs = NULL);
	BOOL Set_Access(const Test_Spec * spec);
	void Start_Test(int index);
	void Begin_IO();
	void Record_On();
	void Record_Off();
	void Stop_Test();
	void Wait_For_Stop();
	void Set_Affinity(DWORD_PTR affinity);
	int Get_Maximum_Sector_Size();
	BOOL Need_Random_Buffer();
	void Set_Random_Data_Buffer(unsigned char* _random_data_buffer, long long _random_data_buffer_size);
	DWORDLONG Get_Target_Spec_Random_Value(int target_index);
	//
	///////////////////////////////////////////////////////////////////////////

	// Function to prepare logical drives for access.
	BOOL Prepare_Disks();

	///////////////////////////////////////////////////////////////////////////
	// Functions to access the targets (called via _Wrapper() interfaces)
	// These functions are run by threads which perform the actual I/O transfers.
	// They are intentially monolithic in nature for performance reasons.
	//
	void Prepare_Disk(int disk_id);
	//
	// the three functions called by Grunt_Thread_Wrapper()
	void Open_Targets();
	void Do_IOs();
	void Close_Targets();
	//
	// Number of threads still not ready.
	volatile long not_ready;
	//
	///////////////////////////////////////////////////////////////////////////

	ReturnVal Complete_IO(int delay);
	void Do_Partial_IO(Transaction * transaction, int bytes_done);

	volatile TestState grunt_state;	// Grunt's status within the test

	int target_count;	// Number of disks/networks.
	Access access_spec;	// Access specs for a test.
	void *read_data;	// Pointer to general data memory area for reading and writing.
	void *write_data;
	void *saved_write_data_pointer;
	int data_size;		// Size of currently allocated data buffers.
	// This is 0 when the grunt is using the manager's buffer.

	BOOL critical_error;

	// Worker results are to be stored.
	Worker_Results worker_performance;
	Worker_Results prev_worker_performance;

	// Indicates the grunt was assigned an idle access spec.
	BOOL idle;

	long long random_data_buffer_size;
	unsigned char* random_data_buffer;

      private:
	void Initialize_Results();

	///////////////////////////////////////////////////////////////////////////
	// Manages list of targets currently assigned to a worker for testing.
	//
	BOOL Size_Target_Array(int count, const Target_Spec * target_specs = NULL);
	Target **targets;
	//
	///////////////////////////////////////////////////////////////////////////

	// To process an I/O error with Record_IO(),
	// pass in a zero for the end_IO time.
	void Record_IO(Transaction * transaction, DWORDLONG end_IO);
	void Asynchronous_Delay(int transfer_delay);

	// Handle of completion queue for asynchronous I/O operations.
	CQ *io_cq;

	// Information needed by an disk preparation IO thread.
	Thread_Info *prepare_thread;

	///////////////////////////////////////////////////////////////////////////
	// Related I/O request arrays.
	//
	Transaction *trans_slots;
	int total_trans_slots;
	int cur_trans_slots;
	//
	//
	// Queue of I/O requests which are available for transmitting.
	// Each entry in this queue refers to a Transaction that may
	// be used to generate I/O.  That is, the transaction has no
	// pending I/O requests.
	int *available_trans_queue;
	int available_head;
	int available_tail;
	int outstanding_ios;
	//
	// Operations on related I/O transaction arrays.
	void Initialize_Transaction_Arrays();
	BOOL Resize_Transaction_Arrays();
	void Free_Transaction_Arrays();
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Variables used to control operation while testing performance.
	//
	// Number of requested nonrecorded asynch I/O's pending.
	// Recording of performance data does not begin until this is 0.
	int ramp_up_ios_pending;
	//
	// Number of disks/networks that are closing for connection rate testing.
	int targets_closing_count;
	//
	///////////////////////////////////////////////////////////////////////////

	int worker_index; // use for per-thread affinity

	// Random number generation functions
	void Srand(DWORDLONG seed);
	DWORDLONG Rand(DWORDLONG limit);

	// Used by random number generator functions.
	DWORDLONG holdrand;
};

#endif
