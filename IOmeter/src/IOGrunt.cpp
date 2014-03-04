/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / IOGrunt.cpp                                               ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: This is Dynamo's worker class.                        ## */
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
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Changes ...: 2005-04-18 (raltherr@apple.com)                       ## */
/* ##               - Support for MacOS X                                 ## */
/* ##               2005-04-05 (ACurrid@nvidia.com)                       ## */
/* ##               - Fixed bug #1088486. Wait outstanding IO complete.   ## */
/* ##               2004-09-01 (henryx.w.tieman@intel.com)                ## */
/* ##               - The Interlocked functions take different parameters ## */
/* ##                 in some of the different environments. The x86_64   ## */
/* ##                 DDK compiler and x86_64 GCC are the two most at     ## */
/* ##                 odds. Doing a cast in a macro seemed to be the best ## */
/* ##                 solution.                                           ## */
/* ##               2004-05-13 (lamontcranston41@yahoo.com)               ## */
/* ##               - Use cur_trans_slots to prevent shifting transaction ## */
/* ##                 queue.                                              ## */
/* ##               2004-03-27 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Code cleanup to ensure common style.                ## */
/* ##               - Applied Thayne Harmon's patch for supporting        ## */
/* ##                 Netware support (on I386).                          ## */
/* ##               2004-03-04 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Suggested by Ming Zhang, I removed an old left      ## */
/* ##                 over _DETAIL code.                                  ## */
/* ##               2003-12-21 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Changed NO_DYNAMO_VI to IOMTR_SETTING_VI_SUPPORT    ## */
/* ##               2003-08-02 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Integrated the modification contributed by          ## */
/* ##                 Vedran Degoricija, to get the code compile with     ## */
/* ##                 the MS DDK on IA64.                                 ## */
/* ##               2003-07-17 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Implemented the IOMTR_[OSFAMILY|OS|CPU]_* global    ## */
/* ##                 define as well as their integrity checks.           ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-05-07 (yakker@aparity.com)                       ## */
/* ##               - Applied the iometerCIOB5.2003.05.02.patch file      ## */
/* ##                 (avoiding cache line collisions and performance     ## */
/* ##                 lock-ups for some chipsets).                        ## */
/* ##               2003-03-04 (joe@eiler.net)                            ## */
/* ##               - Changed NO_LINUX_VI to NO_DYNAMO_VI                 ## */
/* ##               2003-02-15 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##               - Applied Matt D. Robinson's iometer.patch file       ## */
/* ##                 (modifies the Set_Access() method to ensure that    ## */
/* ##                 there is an allocated buffer for each worker        ## */
/* ##                 thread - needed for large amounts of I/O through    ## */
/* ##                 ServerWorks chipsets).                              ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include "IOGrunt.h"
#include "IOCommon.h"
#include "IOTargetDisk.h"
#include "IOTargetTCP.h"

extern struct dynamo_param param;

#if defined(IOMTR_SETTING_VI_SUPPORT)
#include "IOTargetVI.h"
#endif

#if defined(IOMTR_OS_SOLARIS)
#include <thread.h>
#endif
#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE)
#include <assert.h>
#endif

#if defined(IOMTR_OS_NETWARE)
void nwtime(timeb * a)
{
	double time = clock();

	a->time = time / CLOCKS_PER_SEC;
	a->millitm = (time - (a->time * CLOCKS_PER_SEC)) * 10;
}
#endif

//
// Initializing Grunt variables before their first use.
//
Grunt::Grunt()
{
	type = InvalidType;

	holdrand = 1L;
	grunt_state = TestIdle;
	not_ready = 0;
	target_count = 0;
	targets_closing_count = 0;

	// Transaction request information.
	total_trans_slots = 0;
	cur_trans_slots = 0;
	trans_slots = NULL;
	available_trans_queue = NULL;
	outstanding_ios = 0;

	io_cq = NULL;

	idle = TRUE;
	data_size = 0;
	read_data = NULL;
	write_data = NULL;
	prepare_thread = NULL;
	targets = NULL;
}

//
// Grunt destructor.
//
Grunt::~Grunt()
{
	// Remove completion queue, if any.
	if ((type != InvalidType) && !IsType(type, GenericVIType) && io_cq)
		delete io_cq;

	Size_Target_Array(0);

	// Release grunt's I/O data buffers if they are in use.
	if (data_size)
#if defined(IOMTR_OSFAMILY_NETWARE)
	NXMemFree(read_data);
	NXMemFree(write_data);
#elif defined(IOMTR_OSFAMILY_UNIX)
	free(read_data);
	free(write_data);
#elif defined(IOMTR_OSFAMILY_WINDOWS)
	VirtualFree(read_data, 0, MEM_RELEASE);
	VirtualFree(write_data, 0, MEM_RELEASE);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

	Free_Transaction_Arrays();
}

//
// Initialize all results for all targets.
//
void Grunt::Initialize_Results()
{
	memset(&worker_performance, 0, sizeof(Worker_Results));
	memset(&prev_worker_performance, 0, sizeof(Worker_Results));
}

int Grunt::Get_Maximum_Sector_Size() {

	int max_sector_size = 0;
	for (int i = 0; i < target_count; i++) {
		max_sector_size = __max(targets[i]->spec.disk_info.sector_size, max_sector_size);
	}
	return max_sector_size;
}

BOOL Grunt::Need_Random_Buffer() {

	BOOL need_random_buffer = false;
	BOOL has_write = access_spec.HasWrite();

	for (int i = 0; i < target_count; i++) {
		if(targets[i]->spec.DataPattern == DATA_PATTERN_FULL_RANDOM && has_write) {
			need_random_buffer = true;
			break;
		}
	}
	return need_random_buffer;
}

DWORDLONG Grunt::Get_Target_Spec_Random_Value(int target_index){
	return targets[target_index]->spec.random;
}

//
// Setting the size of the target array to hold the number and type of 
// targets specified.  If the requested number of targets is 0, the 
// array will be freed.
//
BOOL Grunt::Size_Target_Array(int count, const Target_Spec * target_specs)
{
	int i;

	// Free all current targets.  This is needed in case the newer targets
	// are of a different type, even if we have the same number of targets.
	for (i = 0; i < target_count; i++)
		delete targets[i];

	target_count = 0;

	// Reset the grunt's target type.
	type = InvalidType;

	// Release the memory if everything is being freed.
	if (!count || !target_specs) {
#if defined(IOMTR_OSFAMILY_NETWARE)
		NXMemFree(targets);
#elif defined(IOMTR_OSFAMILY_UNIX)
		free(targets);
#elif defined(IOMTR_OSFAMILY_WINDOWS)
		free(targets);	// TODO: Check if VirtualFree() is not needed here.
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
		targets = NULL;
		return TRUE;
	}
	// Allocate enough pointers to refer to all targets.
#if defined(IOMTR_OSFAMILY_NETWARE)
	targets = (Target **) NXMemRealloc(targets, sizeof(Target *) * count, 1);
#elif defined(IOMTR_OSFAMILY_UNIX) || defined(IOMTR_OSFAMILY_WINDOWS)
	targets = (Target **) realloc(targets, sizeof(Target *) * count);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	if (!targets)
		return FALSE;

	// Create the requested number of targets.
	for (i = 0; i < count; i++) {
		type = (TargetType) (type | target_specs[i].type);

		if (IsType(target_specs[i].type, GenericDiskType))
			targets[i] = new TargetDisk;
		else if (IsType(target_specs[i].type, GenericTCPType))
			targets[i] = new TargetTCP;
#if defined(IOMTR_SETTING_VI_SUPPORT)
		else if (IsType(target_specs[i].type, GenericVIType))
			targets[i] = new TargetVI;
#endif				// IOMTR_SETTING_VI_SUPPORT

		if (!targets[i])
			return FALSE;
	}

	target_count = count;
	return TRUE;
}

//
// Checking and resizing the related transaction arrays based on the requested
// number of outstanding I/Os.
//
BOOL Grunt::Resize_Transaction_Arrays()
{
	int i;
	int max_queue_per_target = 0;
	void *new_mem;

	cur_trans_slots = 0;

	// Get the maximum number of I/O's which can be outstanding at one time 
	// to all targets and a single target.
	for (i = 0; i < target_count; i++) {
		max_queue_per_target = __max(targets[i]->spec.queue_depth, max_queue_per_target);
		cur_trans_slots += targets[i]->spec.queue_depth;
	}

#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
	if (io_cq->SetQueueSize(cur_trans_slots) == FALSE) {
		return FALSE;
	}
#elif(IOMTR_OSFAMILY_WINDOWS)
	// nop
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

	//
	// Ensure that the related I/O request arrays are large enough.
	//
	if (total_trans_slots >= cur_trans_slots) {
		return TRUE;
	}
	// Grow the transaction array.  We perform two casts to ensure that the
	// memory is aligned properly.
#if defined(IOMTR_OSFAMILY_NETWARE)
	if (!(new_mem = (void *)((Transaction *) NXMemRealloc(trans_slots, cur_trans_slots * sizeof(Transaction), 1))))
#elif defined(IOMTR_OSFAMILY_UNIX) || defined(IOMTR_OSFAMILY_WINDOWS)
	if (!(new_mem = (void *)((Transaction *) realloc(trans_slots, cur_trans_slots * sizeof(Transaction)))))
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	{
		Free_Transaction_Arrays();
		cout << "*** Grunt could not allocate transaction request list." << endl;
		return FALSE;
	}
	trans_slots = (Transaction *) new_mem;

	// Grow the available queue.  We need one more element in the available
	// queue than there are transactions.  This allows us to determine if the
	// queue is empty by checking that head == tail.  (Without it, head == tail
	// could also indicate that the queue is entirely full.)
#if defined(IOMTR_OSFAMILY_NETWARE)
	if (!(new_mem = (void *)((int *)NXMemRealloc(available_trans_queue, (cur_trans_slots + 1) * sizeof(int), 1))))
#elif defined(IOMTR_OSFAMILY_UNIX) || defined(IOMTR_OSFAMILY_WINDOWS)
	if (!(new_mem = (void *)((int *)realloc(available_trans_queue, (cur_trans_slots + 1) * sizeof(int)))))
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	{
		Free_Transaction_Arrays();
		cout << "*** Grunt could not allocate available queue." << endl;
		return FALSE;
	}
	available_trans_queue = (int *)new_mem;

	// Successfully resized all related transaction arrays.
	total_trans_slots = cur_trans_slots;
	return TRUE;
}

//
// Free all memory associated with related I/O request arrays.
//
void Grunt::Free_Transaction_Arrays()
{
	if (trans_slots)
#if defined(IOMTR_OSFAMILY_NETWARE)
		NXMemFree(trans_slots);
#elif defined(IOMTR_OSFAMILY_UNIX) || defined(IOMTR_OSFAMILY_WINDOWS)
		free(trans_slots);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
		if (available_trans_queue)
#if defined(IOMTR_OSFAMILY_NETWARE)
			NXMemFree(available_trans_queue);
#elif defined(IOMTR_OSFAMILY_UNIX) || defined(IOMTR_OSFAMILY_WINDOWS)
			free(available_trans_queue);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
			total_trans_slots = 0;
	cur_trans_slots = 0;
}

//
// Initialize transaction availability queue.
//
void Grunt::Initialize_Transaction_Arrays()
{
	int i, depth;
	int max_queue_per_target = 0;
	int trans_slot = 0;

	// Get the maximum number of I/O's which can be outstanding at one time 
	// to a single target.
	for (i = 0; i < target_count; i++) {
		max_queue_per_target = __max(targets[i]->spec.queue_depth, max_queue_per_target);
	}

	//
	// Initialize transaction information.
	//
	for (i = 0; i < cur_trans_slots; i++) {
		trans_slots[i].asynchronous_io.hEvent = NULL;
		trans_slots[i].request_number = i;

		trans_slots[i].remaining_requests = 0;
		trans_slots[i].remaining_replies = 0;
	}

	// Initialize the available queue.
	available_head = 0;
	available_tail = 0;

	// Adding transactions to available queue in a round robin fashion.
	for (depth = 0; depth < max_queue_per_target; depth++) {
		for (i = 0; i < target_count; i++) {
			if (targets[i]->spec.queue_depth > depth) {
				trans_slots[trans_slot++].target_id = i;
				// We have to init available_trans_queue[] slots in two steps
				// because the order of execution could give different results
				// with different compilers.
				available_trans_queue[available_tail] = available_tail;
				available_tail++;
			}
		}
	}
}

//
// Setting the targets that this grunt will access.
//
BOOL Grunt::Set_Targets(int count, Target_Spec * target_specs)
{
	// Remove previous completion queue if it was created by the Grunt.
	if (!IsType(type, GenericVIType) && io_cq) {
		delete io_cq;
	}
	io_cq = NULL;

	// If no targets are being set, simply clear the target list.
	if (!count) {
		cout << "   clearing target list." << endl;
		return Size_Target_Array(0);
	}
	// Allocate enough targets
	if (!Size_Target_Array(count, target_specs))
		return FALSE;

	worker_performance.target_results.count = count;

	// Create appropriate completion queue object based on targets.
	// If the Grunt will manage VI targets, the targets will provide a
	// pointer to the completion queue to use.
#if defined(IOMTR_SETTING_VI_SUPPORT)
	if (IsType(type, GenericVIType)) {
		// VI targets must know where the data buffer is and its size before
		// being initialized.
		((TargetVI *) targets[0])->data_buffer = (char *)read_data;
		((TargetVI *) targets[0])->data_buffer_size = access_spec.max_transfer;
		io_cq = &((TargetVI *) targets[0])->vi.vi_cq;
	} else
#endif				// IOMTR_SETTING_VI_SUPPORT
	{
		// Create completion queue and verify its creation.
		if (!(io_cq = new CQAIO)) {
			cout << "*** Unable to create completion queue while setting " << "targets." << endl;
			return FALSE;
		}
	}

	// Initialize the specific targets.
	for (int i = 0; i < count; i++) {
		if (!targets[i]->Initialize(&target_specs[i], io_cq))
			return FALSE;
	}


	// Seed the random number generator.  Grunts transferring data over a
	// network will use the same seed to produce the same sequence of random
	// numbers.  This will keep them in synch.
	cout << "Seeding random Number Generator(" << target_specs[0].random << ")" << endl;
	Srand(target_specs[0].random);

	return Resize_Transaction_Arrays();
}

//
// Start recording test data.
//
void Grunt::Record_On()
{
	if (!target_count || idle || critical_error)
		return;

	worker_performance.time[FIRST_SNAPSHOT] = timer_value();
	prev_worker_performance.time[LAST_SNAPSHOT] = timer_value();

	InterlockedExchange(IOMTR_MACRO_INTERLOCK_CAST(int)&grunt_state, (int)TestRecording);
}

//
// Stop recording test data.
//
void Grunt::Record_Off()
{
	if (!target_count || idle || critical_error)
		return;

	InterlockedExchange(IOMTR_MACRO_INTERLOCK_CAST(int)&grunt_state, (int)TestRampingDown);

	worker_performance.time[LAST_SNAPSHOT] = timer_value();
}

//
// Stopping a test.
//
void Grunt::Stop_Test()
{
	if (!target_count || idle || critical_error)
		return;

	InterlockedExchange(IOMTR_MACRO_INTERLOCK_CAST(long)&not_ready, 1L);
	InterlockedExchange(IOMTR_MACRO_INTERLOCK_CAST(int) & grunt_state, (int)TestIdle);
}

//
// Waiting for threads to process stop command.
//
void Grunt::Wait_For_Stop()
{
	if (!target_count || idle)
		return;

	// Waiting for all threads to finish.
	while (not_ready)
		Sleep(1);
}

void Grunt::Set_Random_Data_Buffer(unsigned char* _random_data_buffer, long long _random_data_buffer_size) 
{
	random_data_buffer = _random_data_buffer;
	random_data_buffer_size = _random_data_buffer_size;
}

//
// Setting access specifications for worker.  Also ensuring that a data buffer
// large enough to support the maximum requested transfer has been allocated.
// Note that Iometer will call Set_Access before testing starts to ensure that
// Dynamo can run the spec with the largest transfer request.
//
BOOL Grunt::Set_Access(const Test_Spec * spec)
{
	// Check for idle spec.
	if ((idle = (spec->access[0].of_size == IOERROR)))
		return TRUE;

	access_spec.Initialize(&(spec->access[0]));

	// Allocate a data buffer large enough to support the maximum requested 
	// transfer.  We do this only if the current buffer is too small and
	// we're using per worker data buffers.
	if (data_size >= access_spec.max_transfer) {
		cout << "Grunt: Grunt data buffer size " << data_size << " >= "
		    << access_spec.max_transfer << ", returning" << endl;
		return TRUE;
	} else if (!data_size) {
		// We always want to use our own buffers, not the manager's
		// buffer.  This is due to a bug in some ServerWorks chipsets
		// (confirmed on the HE-SL chipset) where performing both
		// read and write operations to the same cache line can cause
		// the PCI bridge (the CIOB5) to hang indefinitely until a
		// third PCI bus request comes in.
		//
		// Using per-grunt buffers eliminates that problem, as you
		// aren't thrashing on the same buffer for both read and
		// write operations.
		data_size = access_spec.max_transfer;
		cout << "Grunt: Growing grunt data buffer from " << data_size << " to "
		    << access_spec.max_transfer << endl;
	}
	// Allocating a larger buffer.
#ifdef _DEBUG
	cout << "Growing grunt data buffers from " << data_size << " to " << access_spec.max_transfer << endl;
#endif

#if defined(IOMTR_OSFAMILY_NETWARE)
	if (read_data) {
		NXMemFree(read_data);
	}
	errno = 0;
	if (!(read_data = NXMemAlloc(access_spec.max_transfer, 1)))
#elif defined(IOMTR_OSFAMILY_UNIX)
	if (read_data) {
		free(read_data);
	}
	errno = 0;
#if defined(IOMTR_OS_LINUX)
	if (posix_memalign(&read_data, sysconf(_SC_PAGESIZE), access_spec.max_transfer))
#elif defined(IOMTR_OS_SOLARIS) || defined(IOMTR_OS_OSX)
	if (!(read_data = valloc(access_spec.max_transfer)))
#else
#warning ===> WARNING: You have to do some coding here to get the port done! 
#endif

#elif defined(IOMTR_OSFAMILY_WINDOWS)
	if (read_data) {
		VirtualFree(read_data, 0, MEM_RELEASE);
	}
	if (!(read_data = VirtualAlloc(NULL, access_spec.max_transfer, MEM_COMMIT, PAGE_READWRITE)))
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	{
		// Could not allocate a larger buffer.  Signal failure.
		cout << "*** Grunt could not allocate read data buffer for I/O transfers." << endl;
		data_size = 0;
		return FALSE;
	}

#if defined(IOMTR_OSFAMILY_NETWARE)
	if (write_data) {
		NXMemFree(write_data);
	}
	errno = 0;
	if (!(write_data = NXMemAlloc(access_spec.max_transfer, 1)))
#elif defined(IOMTR_OSFAMILY_UNIX)
	if (write_data) {
		free(write_data);
	}
	errno = 0;
#if defined(IOMTR_OS_LINUX)
        if (posix_memalign(&write_data, sysconf(_SC_PAGESIZE), access_spec.max_transfer))
#elif defined(IOMTR_OS_SOLARIS) || defined(IOMTR_OS_OSX)
        if (!(write_data = valloc(access_spec.max_transfer)))
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

#elif defined(IOMTR_OSFAMILY_WINDOWS)
	if (write_data) {
		VirtualFree(write_data, 0, MEM_RELEASE);
	}
	if (!(write_data = VirtualAlloc(NULL, access_spec.max_transfer, MEM_COMMIT, PAGE_READWRITE)))
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	{
		// Could not allocate a larger buffer.  Signal failure.
		cout << "*** Grunt could not allocate write data buffer for I/O transfers." << endl;
		data_size = 0;
		return FALSE;
	}

	data_size = access_spec.max_transfer;

	saved_write_data_pointer = write_data;

	return TRUE;
}

// Do not do this yet
#if defined(IOMTR_SETTING_CPU_AFFINITY)
//
// The idea is to set each of the worker threads to their own CPU, within the 
// constraint of any other cpu affinity we have been passed at the cmd line.
//
void Grunt::Set_Affinity(DWORD_PTR affinity)
{
	int found_bits = -1; // seed this guy so the first ++ starts at 0 to help with 0-based index
	int effective_procs = 0;
	int effective_index = 0;
	DWORD_PTR effective_affinity = 1, temp_affinity = affinity;

	// Calculate effective_procs, whcih  may be equal to or less 
	// then actual number of procs, depending on the mask specified
	// and never greater
	while (temp_affinity)
	{
		if (temp_affinity & 0x1)
			effective_procs++;
		temp_affinity = temp_affinity >> (DWORD_PTR) 1;
	}

	// Both index values are 0-based. Round-robin the threads
	// if their number exceeds number processors.
	effective_index = worker_index % effective_procs;
	
	while (effective_affinity)
	{
		if (affinity & effective_affinity)
			found_bits++;

		if (effective_index == found_bits)
		{
			// We have a match for our index, so set the affinity for the thread
#if defined(IOMTR_OSFAMILY_WINDOWS)
			SetThreadAffinityMask(GetCurrentThread(), effective_affinity);
//#elif defined(IOMTR_OS_LINUX)
//			sched_setaffinity(gettid(), CPU_SETSIZE, &s)
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
			cout << "Worker " << worker_index << "setting thread affinity mask to 0x" 
				 << hex << effective_affinity << endl;
			break;
		}
		effective_affinity = effective_affinity << (DWORD_PTR) 0x1;
	}
}
#else
void Grunt::Set_Affinity(DWORD_PTR affinity)
{
	return;
}
#endif //IOMTR_SETTING_CPU_AFFINITY
//
// Starting threads to prepare disks for tests.  Returning TRUE if we
// successfully started the disk preparation.
//
BOOL Grunt::Prepare_Disks()
{
#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
	pthread_t newThread;
#elif defined(IOMTR_OSFAMILY_WINDOWS)
	// nop
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

	grunt_state = TestPreparing;
	InterlockedExchange(IOMTR_MACRO_INTERLOCK_CAST(long)&not_ready, (long)target_count);

	// Creating a thread to prepare each disk.
	cout << "Preparing disks..." << endl;

#if defined(IOMTR_OSFAMILY_NETWARE)
	prepare_thread = (Thread_Info *) NXMemAlloc(sizeof(Thread_Info) * target_count, 1);
#elif defined(IOMTR_OSFAMILY_UNIX) || defined(IOMTR_OSFAMILY_WINDOWS)
	prepare_thread = (Thread_Info *) malloc(sizeof(Thread_Info) * target_count);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	if (!prepare_thread) {
		cout << "*** Unable to allocate memory for preparation threads." << endl;
		return FALSE;
	};

	for (int i = 0; i < target_count; i++) {
		if (IsType(targets[i]->spec.type, LogicalDiskType)) {
			prepare_thread[i].parent = this;
			prepare_thread[i].id = i;
			cout << "   " << targets[i]->spec.name << " preparing." << endl;
#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
			// Assuming that thr_create call will not fail !!!

			pthread_create(&newThread, NULL, (void *(*)(void *))Prepare_Disk_Wrapper,
				       (void *)&(prepare_thread[i]));
			pthread_detach(newThread);
#elif defined(IOMTR_OSFAMILY_WINDOWS)
			_beginthread(Prepare_Disk_Wrapper, 0, (void *)&(prepare_thread[i]));
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
		}
	}
	return TRUE;
}

//
// Wrapper for Grunt::Prepare_Disk(), used by _beginthread().
//
void CDECL Prepare_Disk_Wrapper(void *disk_thread_info)
{
	Grunt *grunt = (Grunt *) (((Thread_Info *) disk_thread_info)->parent);
	int disk_id = ((Thread_Info *) disk_thread_info)->id;

	grunt->Prepare_Disk(disk_id);
}

//
// Preparing a disk for access by a worker thread.  The disk must have been 
// previously initialized.
//
void Grunt::Prepare_Disk(int disk_id)
{
	void *buffer = NULL;
	//DWORD buffer_size;
	DWORDLONG prepare_offset = 0;
	TargetDisk *disk = (TargetDisk *) targets[disk_id];

	critical_error = FALSE;
/*
	// Allocate a large (64k for 512 byte sector size) buffer for the preparation.
	buffer_size = disk->spec.disk_info.sector_size * 128;
#if defined(IOMTR_OSFAMILY_NETWARE)
	NXMemFree(buffer);
	errno = 0;
	if (!(buffer = NXMemAlloc(buffer_size, 1)))
#elif defined(IOMTR_OSFAMILY_UNIX)
	free(buffer);
	errno = 0;
	if (!(buffer = valloc(buffer_size)))
#elif defined(IOMTR_OSFAMILY_WINDOWS)
	VirtualFree(buffer, 0, MEM_RELEASE);
	if (!(buffer = VirtualAlloc(NULL, buffer_size, MEM_COMMIT, PAGE_READWRITE)))
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	{
		cout << "*** Could not allocate buffer to prepare disk." << endl;
		critical_error = TRUE;
		InterlockedDecrement(IOMTR_MACRO_INTERLOCK_CAST(long) & not_ready);

		return;
	}
*/
	// Open the disk for preparation.
#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
	// The disk::prepare() operation writes to a file iobw.tst till it uses up
	// all the available disk space. Now, Solaris allows a file to be created
	// with a (logical) size that is larger than the actual size of the file.
	// Later, when an attempt is made to write to the unwritten portion of the
	// file, Solaris attempts to expand the actual size on the disk to 
	// accomodate the new writes.
	//
	// The disk::prepare() throws up a problem here. Since we write in parallel
	// to the the file, Solaris allows us to create an iobw.tst that is larger
	// than the available space on the disk. A later write to the unfilled
	// portion throws up an ENOSPC error. To avoid this problem, we use the
	// O_APPEND flag which always sets the write offset to the eof.
	if (!disk->Open(&grunt_state, O_APPEND))
#elif defined(IOMTR_OSFAMILY_WINDOWS)
	if (!disk->Open(&grunt_state))
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	{
		cout << "*** Could not open disk." << endl;
		critical_error = TRUE;
	}
	else {
		// Prepare the disk, first with large block sizes, then with single sectors.
		if (!disk->Prepare(&prepare_offset, &grunt_state, disk->spec.disk_info.sector_size, random_data_buffer, random_data_buffer_size)) {
			cout << "*** An error occurred while preparing the disk." << endl;
			critical_error = TRUE;
		}

		disk->Close(NULL);
	}

#if defined(IOMTR_OSFAMILY_NETWARE)
	NXMemFree(buffer);
#elif defined(IOMTR_OSFAMILY_UNIX)
	free(buffer);
#elif defined(IOMTR_OSFAMILY_WINDOWS)
	VirtualFree(buffer, 0, MEM_RELEASE);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	cout << "   " << disk->spec.name << " done." << endl;
	InterlockedDecrement(IOMTR_MACRO_INTERLOCK_CAST(long) & not_ready);
}

 //
// Recording information about an asynchronous I/O.  The results of the
// I/O may or may not actually be added to the total results, depending on if
// there are any outstanding non-recorded I/O's still pending.
//
// If 0 is passed in for the end_IO time, an I/O error is assumed.
// If the I/O was not completed and an error occurred, record the error.
//
// Each I/O is part of a transaction.  The transaction slots hold information
// about the number of I/Os remaining in each transaction.
//
void Grunt::Record_IO(Transaction * transaction, DWORDLONG end_IO)
{
	DWORDLONG transfer_time;	// Time to complete a successful transfer.
	Raw_Result *result = &(worker_performance.target_results.result[transaction->target_id]);
	Raw_Result *prev_result = &(prev_worker_performance.target_results.result[transaction->target_id]);

	// Update the target's number of outstanding I/Os.
	targets[transaction->target_id]->outstanding_ios--;

	// Update the grunt's number of outstanding I/Os
	outstanding_ios--;

	// See if it's okay to record the completed I/O.
	if (transaction->start_IO && (ramp_up_ios_pending <= 0) && (grunt_state == TestRecording)) {
		// Updating results for I/O.
		// (If end_IO is zero, this value won't be used anyway)
		transfer_time = end_IO - transaction->start_IO;

		// Calculating is_read...the value is the opposite for client threads.
		if (transaction->is_read ^ IsType(targets[transaction->target_id]->spec.type, GenericServerType)) {
			if (end_IO) {
				// I/O was completed without an error.
				result->read_count++;
				result->bytes_read += transaction->remaining_requests
				    ? transaction->request_size : transaction->reply_size;
				result->read_latency_sum += transfer_time;
				// Finding if last transfer took the longest.
				if (transfer_time > result->max_raw_read_latency)
					result->max_raw_read_latency = transfer_time;
				if (transfer_time > prev_result->max_raw_read_latency)
					prev_result->max_raw_read_latency = transfer_time;

				for(int binNum=1; binNum < LATENCY_BIN_SIZE; binNum++) {
					if((double)transfer_time < access_spec.latency_bin_values[binNum]) {
						result->latency_bin[binNum - 1] += 1;
						break;
					}
				}
			} else {
				// Error occurred.
				result->read_errors++;
			}
		} else		// was a write I/O
		{
			if (end_IO) {
				// I/O was completed without an error.
				result->write_count++;
				result->bytes_written += transaction->remaining_requests
				    ? transaction->request_size : transaction->reply_size;
				result->write_latency_sum += transfer_time;
				// Finding if last transfer took the longest.
				if (transfer_time > result->max_raw_write_latency)
					result->max_raw_write_latency = transfer_time;
				if (transfer_time > prev_result->max_raw_write_latency)
					prev_result->max_raw_write_latency = transfer_time;

				for(int binNum=1; binNum < LATENCY_BIN_SIZE; binNum++) {
					if((double)transfer_time < access_spec.latency_bin_values[binNum]) {
						result->latency_bin[binNum - 1] += 1;
						break;
					}
				}

			} else {
				// Error occurred.
				result->write_errors++;
			}
		}

		// If this I/O was the last in the transaction, record the transaction time.
		if (transaction->start_transaction &&
		    (transaction->remaining_requests + transaction->remaining_replies) == 1) {
			// A transaction was completed.
			result->transaction_count++;

			// this is the final I/O of a recorded transaction,
			// so record the elapsed transaction time
			transfer_time = end_IO - transaction->start_transaction;

			result->transaction_latency_sum += transfer_time;
			if (transfer_time > result->max_raw_transaction_latency)
				result->max_raw_transaction_latency = transfer_time;
			if (transfer_time > prev_result->max_raw_transaction_latency)
				prev_result->max_raw_transaction_latency = transfer_time;
		}
	} else {
		// there was no starting time on this transfer

		if (ramp_up_ios_pending) {
			--ramp_up_ios_pending;

			// If ramp_up_ios_pending just went to ZERO and the grunt is in the
			// recording state, set the test's start_counter to the current time.
			if ((ramp_up_ios_pending <= 0) && (grunt_state == TestRecording)) {
				worker_performance.time[FIRST_SNAPSHOT] = timer_value();
			}
		}
	}

	// If there are any REQUEST I/Os remaining for this transfer
	// deduct the completed I/O from the REQUEST counter.
	if (transaction->remaining_requests > 0) {
		if (!--(transaction->remaining_requests)) {
			// If this was the last REQUEST, start replies
			transaction->is_read = !transaction->is_read;
		}
	} else
#ifdef _DEBUG
	if (transaction->remaining_requests == 0 && transaction->remaining_replies > 0)
#endif
	{
		--transaction->remaining_replies;
	}
#ifdef _DEBUG
	else {
		if (transaction->remaining_requests < 0)
			cout << "*** ERROR: remaining_requests is negative" << endl;

		if (transaction->remaining_replies < 0)
			cout << "*** ERROR: remaining_replies is negative" << endl;
	}
#endif

	// Put this transaction slot back on the available queue.
	available_trans_queue[available_tail] = transaction->request_number;
	if ((++available_tail) > cur_trans_slots) {
		available_tail = 0;
	}
}

//
// Inserting a delay between transfers.  We still need to process completed 
// I/O's, but no new ones should be initiated.
//
void Grunt::Asynchronous_Delay(int transfer_delay)
{
	struct _timeb start_wait_time;	// Time before thread started waiting for an I/O to complete.
	struct _timeb end_wait_time;	// Time after a thread waited for an I/O to complete./

	do {
		_ftime(&start_wait_time);

		if (Complete_IO(transfer_delay) == ReturnTimeout)
			return;

		// More waiting is needed before allowing additional requests.
		_ftime(&end_wait_time);
		transfer_delay -= (((end_wait_time._time - start_wait_time._time) * 1000)
				   + end_wait_time._millitm - start_wait_time._millitm);
	}
	while (transfer_delay > 0);
}

//
// Wrapper for Grunt::DoIOs(), used by _beginthread().
//
void CDECL Grunt_Thread_Wrapper(void *grunt)
{
	if (param.cpu_affinity) // only if we have been provided an overriding affinity
		((Grunt *) grunt)->Set_Affinity(param.cpu_affinity);

	((Grunt *) grunt)->access_spec.latency_bin_values[0] =	0;												// 0us
	((Grunt *) grunt)->access_spec.latency_bin_values[1] =	((Grunt *) grunt)->timer_resolution / 20000;	// 50us
	((Grunt *) grunt)->access_spec.latency_bin_values[2] =	((Grunt *) grunt)->timer_resolution / 10000;	// 100us
	((Grunt *) grunt)->access_spec.latency_bin_values[3] =	((Grunt *) grunt)->timer_resolution / 5000;		// 200us
	((Grunt *) grunt)->access_spec.latency_bin_values[4] =	((Grunt *) grunt)->timer_resolution / 2000;		// 500us
	((Grunt *) grunt)->access_spec.latency_bin_values[5] =	((Grunt *) grunt)->timer_resolution / 1000;		// 1ms
	((Grunt *) grunt)->access_spec.latency_bin_values[6] =	((Grunt *) grunt)->timer_resolution / 500;		// 2ms
	((Grunt *) grunt)->access_spec.latency_bin_values[7] =	((Grunt *) grunt)->timer_resolution / 200;		// 5ms
	((Grunt *) grunt)->access_spec.latency_bin_values[8] =	((Grunt *) grunt)->timer_resolution / 100;		// 10ms
	((Grunt *) grunt)->access_spec.latency_bin_values[9] =	((Grunt *) grunt)->timer_resolution / 67;		// 15ms
	((Grunt *) grunt)->access_spec.latency_bin_values[10] =	((Grunt *) grunt)->timer_resolution / 50;		// 20ms
	((Grunt *) grunt)->access_spec.latency_bin_values[11] =	((Grunt *) grunt)->timer_resolution / 33;		// 30ms
	((Grunt *) grunt)->access_spec.latency_bin_values[12] =	((Grunt *) grunt)->timer_resolution / 20;		// 50ms
	((Grunt *) grunt)->access_spec.latency_bin_values[13] =	((Grunt *) grunt)->timer_resolution / 10;		// 100ms
	((Grunt *) grunt)->access_spec.latency_bin_values[14] =	((Grunt *) grunt)->timer_resolution / 5;		// 200ms
	((Grunt *) grunt)->access_spec.latency_bin_values[15] =	((Grunt *) grunt)->timer_resolution / 2;		// 500ms
	((Grunt *) grunt)->access_spec.latency_bin_values[16] =	((Grunt *) grunt)->timer_resolution;			// 1s
	((Grunt *) grunt)->access_spec.latency_bin_values[17] =	((Grunt *) grunt)->timer_resolution * 2;		// 2s
	((Grunt *) grunt)->access_spec.latency_bin_values[18] =	((Grunt *) grunt)->timer_resolution * 4;		// 4.7s
	((Grunt *) grunt)->access_spec.latency_bin_values[19] =	((Grunt *) grunt)->timer_resolution * 5;		// 5s
	((Grunt *) grunt)->access_spec.latency_bin_values[20] =	((Grunt *) grunt)->timer_resolution * 10;		// 10s

	// open targets
	((Grunt *) grunt)->Open_Targets();

	// If open failed, abort.
	if (((Grunt *) grunt)->critical_error)
		return;

	// run the tests
	((Grunt *) grunt)->Do_IOs();

	// close targets
	((Grunt *) grunt)->Close_Targets();
}

//
// Opens all targets before Do_IOs() runs - returns FALSE if any opens failed
// (In case of failure, this function will behave as if it is running the IO tests.)
//
// called by the Grunt_Thread_Wrapper function
//
void Grunt::Open_Targets()
{
	int i;

	// Opening all targets for access.
	cout << "   Opening targets." << endl;
	for (i = 0; i < target_count; i++) {
		// All targets are opened to ensure that they can connect.
		if (!targets[i]->Open(&grunt_state)) {
			// An error occurred opening a target.  Do not go any further.
			cout << "*** Error opening " << targets[i]->spec.name << endl;
			break;
		}
#if _DETAILS
		cout << "Opened " << targets[i]->spec.name << endl;
#endif

		// If the target is testing connection rate, it will be opened when
		// we first begin doing I/O.  Close these targets.
		if (targets[i]->spec.test_connection_rate) {
			targets[i]->trans_left_in_conn = 0;
#ifdef _DEBUG
			cout << "Defering open for " << targets[i]->spec.name << endl;
#endif
			if (!targets[i]->Close(&grunt_state)) {
				cout << "*** Error re-closing target " << targets[i]->spec.name << endl;
				break;
			}
		}
	}

	// See if an error occurred.  If so, close any that are open and stop.
	if (i < target_count) {
		critical_error = TRUE;
		Close_Targets();
	}
	// Signalling that thread is done (successfully or not) opening its targets.
	InterlockedExchange(IOMTR_MACRO_INTERLOCK_CAST(long) & not_ready, 0);
}

//
// Closes all targets after Do_IOs() runs
//
// called by the Grunt_Thread_Wrapper() function
//
void Grunt::Close_Targets()
{
	// Closing disks.
	for (int i = 0; i < target_count; i++) {
		if (!(targets[i]->Close(&grunt_state)))
			cout << "*** Error closing " << targets[i]->spec.name << endl;
	}
	InterlockedExchange(IOMTR_MACRO_INTERLOCK_CAST(long) & not_ready, 0);
}

//
// Performing accesses based on specifications to targets.
// The following function initiates I/O, recording performance information only when the
// grunt is in the TestRecording state.  The function needs to be as efficient as possible
// while still being readable and maintainable.
//
// "data" is the location of data to use for transfers.
//
void Grunt::Do_IOs()
{
	////////////////////////////////////////////////////////////////////////////
	// these are working variables for the IO loop - they aren't referenced
	// outside of it and their values don't span more than one burst of data.
	//
	int remaining_transactions_in_burst = 0;	// how's that for a name?
	int access_percent = 0;	// Determines the access spec.
	int target_id;		// Index into target array of target to access.
	DWORD size;		// Size of transfer request to target.
	int transfer_delay;	// Milliseconds to wait before accessing.
	ReturnVal transfer_result;	// Success/failure result of read or write operation.
	Transaction *transaction = NULL;	// Pointer to the transaction being processed.
	DWORD user_alignment;
	DWORDLONG user_align_mask;
	DWORD reply;		// Size of reply, or 0 for no reply
	DWORDLONG conn_time;	// Used to calculate average and max connection times.
	Target *target;
	Raw_Result *target_results;	// Pointer to results for selected target.
	Raw_Result *prev_target_results;

	while (grunt_state != TestIdle) {
#if defined(IOMTR_OSFAMILY_NETWARE)
		pthread_yield();	// NetWare is non-preemptive
#elif defined(IOMTR_OSFAMILY_UNIX) || defined(IOMTR_OSFAMILY_WINDOWS)
		// nop
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
		// If we can't queue another request, wait for a completion.
		// If we CAN queue another request, get only one completion and do 
		// so immediately (with a time out of 0).
		Complete_IO((available_head == available_tail ||
			     target_count == targets_closing_count) ? TIMEOUT_TIME : 0);

		// Now check to see if there really are any completed requests.
		// Complete_IO may not have freed a slot in the available queue.
		if (available_head == available_tail)
			continue;	// repeat the outermost while loop

		// Getting an index into the target array of the next target to access.
		target_id = trans_slots[available_trans_queue[available_head]].target_id;
		target = targets[target_id];

		// Checking the next target to access to see if it is closing.
		// This if statement is separated from the one below for performance.
		if (target->spec.test_connection_rate && target->is_closing) {
			// Target is closing.  Move it to the tail of the available queue.
			available_trans_queue[available_tail++] = available_trans_queue[available_head++];
			if (available_head > cur_trans_slots) {
				available_head = 0;
			}
			if (available_tail > cur_trans_slots) {
				available_tail = 0;
			}
			// Check to see if we can close the target.  Targets are not closed
			// until all outstanding I/Os have completed.
			if (target->outstanding_ios == 0) {
#ifdef _DEBUG
				cout << "Testing connection rate: Closing " << targets[target_id]->spec.name << endl;
#endif

				// Close target and record connection time.
				target->Close(&grunt_state);

				conn_time = timer_value() - target->conn_start_time;

				// Since target is closed, it is no longer closing.
				target->is_closing = FALSE;

				// Record connection latencies.
				if (ramp_up_ios_pending <= 0 && grunt_state == TestRecording) {
					target_results = &(worker_performance.target_results.result[target_id]);
					target_results->connection_count++;
					target_results->connection_latency_sum += conn_time;
					if (conn_time > target_results->max_raw_connection_latency)
						target_results->max_raw_connection_latency = conn_time;

					prev_target_results =
					    &(prev_worker_performance.target_results.result[target_id]);
					if (conn_time > prev_target_results->max_raw_connection_latency)
						prev_target_results->max_raw_connection_latency = conn_time;
				}
			}
			continue;
		}
		// Check to see if we need to open the target.
		if (target->spec.test_connection_rate && target->trans_left_in_conn <= 0) {
#ifdef _DEBUG
			cout << "Testing connection rate: Opening " << target->spec.name << endl;
#endif
			// Set the number of transactions to do before closing.
			target->trans_left_in_conn = target->spec.trans_per_conn;
			
			// Record the start time for the transaction.
			target->conn_start_time = timer_value();

			// Open the target.
			target->Open(&grunt_state);

			// If not doing any transactions, set the target to close.
			if ((target->is_closing = !target->spec.trans_per_conn))
				continue;
		}
		// "transaction" will now point to the next available
		// transaction slot
		transaction = &(trans_slots[available_trans_queue[available_head]]);

		// See if this is the start of a new transaction
		if (!(transaction->remaining_requests || transaction->remaining_replies)) {
			// No requests or replies remain, start a new transaction.

			// Check to see if this transaction will be the last one
			// before closing the target.
			target->is_closing = (target->spec.test_connection_rate && --target->trans_left_in_conn <= 0);

			// See if we need to get a new access spec line.  All transactions
			// for the current burst have been sent.
			if (--remaining_transactions_in_burst <= 0) {
				access_spec.GetNextBurst(access_percent = (unsigned int)Rand(100),
							 &remaining_transactions_in_burst, &size, &transfer_delay,
							 &user_alignment, &user_align_mask, &reply);
				// Possibly waiting some delay before sending next burst of transfers.
				if (transfer_delay)
					Asynchronous_Delay(transfer_delay);
			}
			// Fill in the information for a new transaction

			// Set number and size of requests.  (Currently always one request; we can support 
			// any number, but this is not yet implemented in Iometer.)
			transaction->request_size = size;
			transaction->remaining_requests = 1;

			// Set number and size of replies.  (Currently reply size = 0 means no reply, 
			// reply size != 0 means one reply; we can support any number, but this is not
			// yet implemented in Iometer.)
			transaction->reply_size = reply;
			transaction->remaining_replies = (reply ? 1 : 0);

			// Determine if the first I/O will be a read or write.
			// (is_read will change when replies are sent)
			transaction->is_read = access_spec.Read(access_percent, (unsigned int)target->Rand(100));

			// Set flag to record transaction start time when the transaction actually begins.
			transaction->start_transaction = 0;
		}
		// Prepare the next I/O of the transaction...

		// Set the transaction's size.
		if (transaction->remaining_requests)
			transaction->size = transaction->request_size;
		else
			transaction->size = transaction->reply_size;

		if (IsType(target->spec.type, GenericDiskType)) {
			((TargetDisk *) targets[target_id])->Seek(access_spec.Random(access_percent,
										     (unsigned int)targets[target_id]->
										     Rand(100)), size,
								  user_alignment, user_align_mask);
		}

		transaction->start_IO = (grunt_state == TestRecording) ? timer_value() : 0;

		// If the transaction start time hasn't been set,
		// this is the first I/O of this transaction.  Set the start time.
		if (!transaction->start_transaction)
			transaction->start_transaction = transaction->start_IO;

		//
		// Finally doing the actual I/O request - whew!  Continue to try to do
		// the I/O while it should be retried.  A retry indicates that there
		// currently aren't enough reources to fulfill the request.
		//
		do {
#if defined(IOMTR_OSFAMILY_NETWARE)
			pthread_yield();	// NetWare is Non-preemptive
#elif defined(IOMTR_OSFAMILY_UNIX) || defined(IOMTR_OSFAMILY_WINDOWS)
			// nop
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
			if (transaction->is_read) {
				transfer_result = target->Read(read_data, transaction);
			} else {

				// Depending on the data pattern selected, set the write_data appropriately
				switch (((TargetDisk *) targets[target_id])->spec.DataPattern){
					case DATA_PATTERN_REPEATING_BYTES:
						write_data = saved_write_data_pointer;
						memset(write_data, rand(), transaction->size);
						break;
					case DATA_PATTERN_PSEUDO_RANDOM:
						// Do nothing...pattern set by the "Set_Access" routine
						break;
					case DATA_PATTERN_FULL_RANDOM:
						//Buffer offset must be DWORD-aligned in memory, otherwise the transfer fails
						//Choose a pointer into the buffer
						long long offset = (long long)Rand(RANDOM_BUFFER_SIZE - transaction->size);

						//See how far it is from being DWORD-aligned
						long long remainder = offset & (sizeof(DWORD) - 1);

						//Align the pointer using the remainder
						offset = offset - remainder;

						write_data = &random_data_buffer[offset];
						break;
				}

				transfer_result = target->Write(write_data, transaction);
			}

			// Continue to process completions.  This may free up the resource
			// that is needed.
			if (transfer_result == ReturnRetry)
				Complete_IO(TIMEOUT_TIME);
		}
		while (transfer_result == ReturnRetry && grunt_state != TestIdle);

		// The request finished.  See what the result was and process it
		// accordingly.
		switch (transfer_result) {
		case ReturnPending:
			// An I/O was successfully started, and its completion will be
			// posted to the completion queue.

			// Increment the number of outstanding I/Os to the target.
			target->outstanding_ios++;

			// Increment the number of outstanding I/Os on this grunt
			outstanding_ios++;

			// See if the I/O occurred during the ramp up period.
			if (!transaction->start_IO)
				++ramp_up_ios_pending;
			break;
		case ReturnSuccess:
			// The request completed successfully, and its completion will not
			// go to the completion queue.
			// We need to treat its full completion as "pending" to ensure that 
			// Record_IO can properly handle it.

			// Increment the number of outstanding I/Os to the target.
			target->outstanding_ios++;

			// Increment the number of outstanding I/Os on this grunt
			outstanding_ios++;

			// See if the I/O occurred during the ramp up period.
			if (!transaction->start_IO)
				++ramp_up_ios_pending;

			// An I/O completed successfully and will not go to the completion 
			// queue.  Record the request as done.
			Record_IO(transaction, timer_value());
			break;
		default:
			// see whether it should record the error
			if (grunt_state == TestRecording) {
				// ERROR: The I/O was not queued successfully.
				cout << "*** Error performing I/O to " << target->spec.name << endl;

				if (transaction->is_read)
					worker_performance.target_results.result[target_id].read_errors++;
				else
					worker_performance.target_results.result[target_id].write_errors++;
			}
			// Move the failed transaction to the end of the available queue to
			// allow other requests, if any, to have a chance of going.
			available_trans_queue[available_tail++] = available_trans_queue[available_head];
			if (available_tail > cur_trans_slots) {
				available_tail = 0;
			}
		}

		// Move the head of the available queue to reflect the last request.
		if ((++available_head) > cur_trans_slots) {
			available_head = 0;
		}
	}			// while grunt_state is not TestIdle

	// Drain any outstanding I/Os from the completion queue
	while (outstanding_ios > 0) {
#if defined(IOMTR_OSFAMILY_NETWARE)
		pthread_yield();	// NetWare is non-preemptive
#elif defined(IOMTR_OSFAMILY_UNIX) || defined(IOMTR_OSFAMILY_WINDOWS)
		// nop
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

		switch (Complete_IO(TIMEOUT_TIME)) {
		case ReturnTimeout:
			cout << "*** Grunt thread exiting with " << outstanding_ios << " still active" << endl;
			break;
		default:
			break;
		}
	}
}

//
// Checking for a completed I/O and processing the completion.
//
ReturnVal Grunt::Complete_IO(int timeout)
{
	int bytes, trans_id;

	// See if any I/Os completed.
	switch (io_cq->GetStatus(&bytes, &trans_id, timeout)) {
	case ReturnSuccess:
		// I/O completed.  Make sure we received everything we requested.
		if (bytes < (int)trans_slots[trans_id].size)
			Do_Partial_IO(&trans_slots[trans_id], bytes);
		else
			Record_IO(&trans_slots[trans_id], timer_value());
		return ReturnSuccess;

	case ReturnAbort:
		Record_IO(&trans_slots[trans_id], 0);
		return ReturnSuccess;

	case ReturnTimeout:
		return ReturnTimeout;

	case ReturnRetry:
		return ReturnRetry;

	default:
		return ReturnError;
	}
}

//
// Issues an additional I/O request for one that partially completed.  Note
// that currently only TCP target types will complete partial I/Os.
//
void Grunt::Do_Partial_IO(Transaction * transaction, int bytes_done)
{
	ReturnVal result;

	// Queue another request for the rest of the bytes.
	transaction->size -= bytes_done;

#if _DETAILS
	cout << "Doing another " << (transaction->is_read ? "read" : "write")
	    << " for the remaining " << transaction->size << " bytes." << endl;
#endif

	if (transaction->is_read) {
		result = targets[transaction->target_id]->Read(read_data, transaction);
	} else {
		result = targets[transaction->target_id]->Write((unsigned char* )write_data, transaction);
	}

	if ((result != ReturnSuccess) && (result != ReturnPending) && (grunt_state == TestRecording)) {
		// The I/O was not started successfully.
		cout << "*** ERROR performing read or write to " << targets[transaction->target_id]->spec.name << endl;

		if (transaction->is_read)
			worker_performance.target_results.result[transaction->target_id].read_errors++;
		else
			worker_performance.target_results.result[transaction->target_id].write_errors++;
	}
}

//
// Start threads to access targets.
//
void Grunt::Start_Test(int index)
{
#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
	pthread_t newThread;
#elif defined(IOMTR_OSFAMILY_WINDOWS)
	// nop
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	char *write_ptr = NULL;

	// Clear the results.
	Initialize_Results();
	critical_error = FALSE;

	// If no drives are selected, or the access spec is the idle spec, 
	// do not create the thread.
	if (!target_count || idle)
		return;

	ramp_up_ios_pending = 0;

	worker_index = index; // save off our index

	Initialize_Transaction_Arrays();

	/* If we are doing pseudo random, fill the write buffer with a pseudo random pattern now as
	   we don't want to be doing this during IO submission */
	int target_id = trans_slots[available_trans_queue[available_head]].target_id;
	if (((TargetDisk *) targets[target_id])->spec.DataPattern==DATA_PATTERN_PSEUDO_RANDOM)
	{
		write_ptr = (char *)write_data;
		while ((ULONG_PTR)write_ptr < ((ULONG_PTR)write_data + data_size)) {
			*write_ptr++ = (char)Rand(0xff);
		}
	}

	// The grunt thread will become ready after opening its targets.
	InterlockedExchange(IOMTR_MACRO_INTERLOCK_CAST(long)&not_ready, 1);
	// Tell the grunt to begin opening its devices, but not to perform I/O yet.
	InterlockedExchange(IOMTR_MACRO_INTERLOCK_CAST(int) & grunt_state, (int)TestOpening);

#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
	pthread_create(&newThread, NULL, (void *(*)(void *))Grunt_Thread_Wrapper, (void *)this);
	pthread_detach(newThread);
#elif defined(IOMTR_OSFAMILY_WINDOWS)
	_beginthread(Grunt_Thread_Wrapper, 0, (void *)this);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
}

//
// Start performing I/O.
//
void Grunt::Begin_IO()
{
	if (!target_count || idle)
		return;

	// Wait for all threads to finish opening their devices.
	while (not_ready) {
#if defined(IOMTR_OSFAMILY_NETWARE)
		pthread_yield();
#elif defined(IOMTR_OSFAMILY_UNIX)
#if defined(IOMTR_OS_SOLARIS)
		thr_yield();
#elif defined(IOMTR_OS_LINUX)
		sleep(0);
#elif defined(IOMTR_OS_OSX)
		pthread_yield_np();
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
#elif defined(IOMTR_OSFAMILY_WINDOWS)
		Sleep(0);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	}

	InterlockedExchange(IOMTR_MACRO_INTERLOCK_CAST(int) & grunt_state, (int)TestRampingUp);
}

//
// Return a 64-bit random number, using the following random function and conditions.
//
// X(n+1)   = ( A * Xn  + B ) mod m  
//
// m = 2^65 (so the mod operation is not needed in this case)
// a = 4c + 1 (c user defined)
// b is odd
//

//
// When we use gcc, we add "LL" to take out a warning about integer overflow.
//
#if defined(IOMTR_OS_NETWARE) || defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
#define A 136204069LL		// 3x7x11x13x17x23x   29x4 + 1
#define B 28500701229LL		// 3x7x11x13x17x23x27x29x31
#elif defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
#define A 136204069		// 3x7x11x13x17x23x   29x4 + 1
#define B 28500701229		// 3x7x11x13x17x23x27x29x31
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

void Grunt::Srand(DWORDLONG seed)
{
	holdrand = seed;
}

DWORDLONG Grunt::Rand(DWORDLONG limit)
{
#if defined(IOMTR_OSFAMILY_UNIX) && defined(WORKAROUND_MOD_BUG)
	return ((DWORDLONG)fmodl(holdrand = A * holdrand + B, limit));
#else
	return (holdrand = A * holdrand + B) % limit;
#endif
}
