/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / Manager.cpp                                            ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implementation of the Manager class for Iometer,      ## */
/* ##               which contains a list of workers and provides         ## */
/* ##               functions for manager-level actions (creating and     ## */
/* ##               removing workers, communication between Iometer and   ## */
/* ##               a single copy of Dynamo, etc.). A Manager object is   ## */
/* ##               created whenever a copy of Dynamo logs in.            ## */
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
/* ##  Changes ...: 2004-06-11 (lamontcranston41@yahoo.com)               ## */
/* ##               - Add code to allow potentially invalid access specs  ## */
/* ##                 but warn the user.                                  ## */
/* ##               2003-10-17 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-04-25 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Updated the global debug flag (_DEBUG) handling     ## */
/* ##                 of the source file (check for platform etc.).       ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include "stdafx.h"
#include "Manager.h"
#include "ManagerList.h"
#include "GalileoApp.h"
#include "GalileoView.h"

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
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

Manager::Manager()
{
	id = IOERROR;
	ResetResults(WHOLE_TEST_PERF);
	ResetResults(LAST_UPDATE_PERF);
	tcps.SetSize(INITIAL_ARRAY_SIZE, ARRAY_GROW_STEP);
	vis.SetSize(INITIAL_ARRAY_SIZE, ARRAY_GROW_STEP);
	disks.SetSize(INITIAL_ARRAY_SIZE, ARRAY_GROW_STEP);
	workers.SetSize(INITIAL_ARRAY_SIZE, ARRAY_GROW_STEP);
	port = NULL;
}

Manager::~Manager()
{
	// Removing all worker, disk info, and net info from the manager.
	while (WorkerCount()) {
		delete GetWorker(0);

		workers.RemoveAt(0);
	}
	RemoveDiskInfo();
	RemoveNetInfo();

	// Close the manager's communications pipe.
	port->Close();
	delete port;
}

//
// Returns the index of the manager.
//
int Manager::GetIndex()
{
	for (int m = 0; m < theApp.manager_list.ManagerCount(); m++) {
		if (this == theApp.manager_list.GetManager(m))
			return m;
	}
	return IOERROR;
}

//
// Returns an indication of all types of workers that a manager has.
//
TargetType Manager::Type()
{
	int i, worker_count;
	TargetType type = GenericType;

	worker_count = WorkerCount();
	for (i = 0; i < worker_count; i++)
		type = (TargetType) (type | workers[i]->Type());

	// See if the manager is active.
	// (A manager may be active even if none of its workers are.)
	if (ActiveInCurrentTest())
		type = (TargetType) (type | ActiveType);

	return type;
}

//
// Get information about a given worker.  This returns the nth worker of the
// specified type.
//
Worker *Manager::GetWorker(int index, TargetType type)
{
	int i, worker_count;

	// Make sure the desired worker exists.
	if (index < 0 || index >= WorkerCount(type))
		return NULL;

	// Find the worker.
	worker_count = WorkerCount();
	for (i = 0; i < worker_count; i++) {
		if (IsType(workers[i]->Type(), type)) {
			if (!index--)
				return workers[i];
		}
	}

	ASSERT(0);		// requested worker not found - should not happen
	return NULL;
}

//
// Get the first worker having the specified name, if any.
//
Worker *Manager::GetWorkerByName(const char *wkr_name, const int wkr_id)
{
	int i, worker_count;

	if (!wkr_name) {
		ErrorMessage("Invalid string in Manager::GetWorkerByName()");
		return NULL;
	}

	worker_count = WorkerCount();
	for (i = 0; i < worker_count; i++) {
		if (strcmp(workers[i]->name, wkr_name) == 0 && workers[i]->id == wkr_id) {
			return workers[i];
		}
	}

	// Didn't find it (not an error).
	return NULL;
}

//
// Adding a new worker to the worker list.  The new worker is always added to 
// the end of the list.  A connection to the worker is also made
//  * May use a given worker as a template.
//  * May specify the new worker's name.  (Ignored if src_worker is defined.)
//
Worker *Manager::AddWorker(TargetType type, Worker * src_worker, const CString & in_name)
{
	Message msg;
	Worker *new_worker;

	// Adding the new worker to the manager.
	if (WorkerCount() >= MAX_WORKERS)
		return (NULL);

	// Request that a new worker be spawned.
	// The data of the message is the number of workers to spawn.
	msg.purpose = ADD_WORKERS;
	msg.data = 1;
	if (Send(&msg) != MESSAGE_SIZE)
		ErrorMessage("Message may not be sent correctly in Manager::AddWorker().");
	if (Receive(&msg) != MESSAGE_SIZE)
		ErrorMessage("Message may not be received correctly in Manager::AddWorker().");

	// Verifying that the worker was created.
	if (!msg.data)
		return (NULL);

	// Allocating space for the new worker under the manager.
	new_worker = new Worker(this, type);

	// Adding worker to end of the worker list.
	workers.Add(new_worker);

	if (!src_worker) {
		// Set the name of the worker based on the number of workers for 
		// this manager
		CString worker_name;

		if (in_name.IsEmpty())
			worker_name.Format("Worker %d", WorkerCount());
		else
			worker_name = in_name;

		strcpy(new_worker->name, worker_name);
	} else {
		// Copy the template's name and other information.
		new_worker->Clone(src_worker);
	}

	// Reassign the disambiguating IDs this manager's workers.
	IndexWorkers();

	return new_worker;
}

//
// Removing a worker from the list.  Return FALSE if all workers have
// been removed.
//
BOOL Manager::RemoveWorker(int index, TargetType type)
{
	Worker *worker;
	int main_index;

	// Get the desired worker.
	if (!(worker = GetWorker(index, type))) {
		ErrorMessage("Invalid worker in Manager::RemoveWorker().");
		return FALSE;
	}
	main_index = worker->GetIndex();

	// Informing Dynamo to remove the worker.
	// The data of the message is the index of the worker to remove.
	Send(main_index, EXIT);

	// Remove the worker from memory.
	workers.RemoveAt(main_index);
	delete worker;

	// Reassign the disambiguating IDs this manager's workers.
	IndexWorkers();

	return WorkerCount(type);
}

//
// Returns the number of workers in the worker list.
//
int Manager::WorkerCount(TargetType type)
{
	int i, count = 0, worker_count;

	worker_count = workers.GetSize();
	for (i = 0; i < worker_count; i++) {
		if (IsType(workers[i]->Type(), type))
			count++;
	}
	return count;
}

//
// Number of targets for all workers under this manager.
//
int Manager::TargetCount(TargetType type)
{
	int w, count = 0, wkr_count = WorkerCount();

	for (w = 0; w < wkr_count; w++)
		count += GetWorker(w)->TargetCount(type);

	return count;
}

//
// Removing all cached disk information for manager and all its workers.
//
void Manager::RemoveDiskInfo()
{
	int i, wkr_count;

	// Deleting Disk_Info objects held in "disks" array
	for (i = 0; i < InterfaceCount(GenericDiskType); i++)
		delete disks[i];

	// Deleting memory used by "disks" array itself
	disks.RemoveAll();

	// Removing all disk targets assigned to the manager's workers.
	wkr_count = WorkerCount();
	for (i = 0; i < wkr_count; i++)
		GetWorker(i)->RemoveTargets(GenericDiskType);
}

//
// Removing all cached network information for manager and all its workers.
//
void Manager::RemoveNetInfo()
{
	int i, net_count, wkr_count;

	// Deleting network information referenced by "tcp" array
	net_count = tcps.GetSize();
	for (i = 0; i < net_count; i++)
		delete tcps[i];

	tcps.RemoveAll();

	net_count = vis.GetSize();
	for (i = 0; i < net_count; i++)
		delete vis[i];

	vis.RemoveAll();

	// Removing all network targets assigned to the manager's workers.
	wkr_count = WorkerCount();
	for (i = 0; i < wkr_count; i++)
		GetWorker(i)->RemoveTargets(GenericNetType);
}

//
// Setting the test specifications for all workers.
//
BOOL Manager::SetAccess(int access_index)
{
	int w, wkr_count = WorkerCount();

	for (w = 0; w < wkr_count; w++) {
		if (!GetWorker(w)->SetAccess(access_index))
			return FALSE;
	}
	return TRUE;
}

//
// Initializing results for the manager.
//
void Manager::ResetResults(int which_perf)
{
	if ((which_perf < 0) || (which_perf >= MAX_PERF))
		return;

	memset(&(results[which_perf]), 0, sizeof(Results));
}

//
// Resetting all results related to the manager.  This includes resetting all results
// for its workers and whatever results a worker relies on.
//
void Manager::ResetAllResults()
{
	int w, wkr_count = WorkerCount();

	ResetResults(WHOLE_TEST_PERF);
	ResetResults(LAST_UPDATE_PERF);

	// Resetting results for all worker threads as well.
	for (w = 0; w < wkr_count; w++)
		GetWorker(w)->ResetAllResults();
}

//
// Removing all targets from manager's workers.
//
void Manager::RemoveTargets(TargetType type)
{
	int w;

	// A target might be a network client, so always check against the current
	// worker count (WorkerCount()) and not a saved value.
	for (w = 0; w < WorkerCount(); w++)
		GetWorker(w)->RemoveTargets(type);
}

//
// Resets a worker's drives to indicate that they are not running.
//
void Manager::ClearActiveTargets()
{
	int w, wkr_count = WorkerCount();

	for (w = 0; w < wkr_count; w++)
		GetWorker(w)->ClearActiveTargets();
}

//
// Marks which targets accessible by a worker are to run, and sending the
// worker the list.  Returns number of targets actually set (IOERROR in case of error).
//
int Manager::SetActiveTargets(int worker_index, int targets_to_set)
{
	return GetWorker(worker_index)->SetActiveTargets(targets_to_set);
}

//
// Peeking to see if a given worker has sent back a message that's waiting 
// in its message queue.
//
DWORD Manager::Peek(int worker_index)
{
	return port->Peek();
}

//
// Sending a message to Dynamo.
//
DWORDLONG Manager::Send(Message * msg)
{
	return port->Send(msg);
}

DWORDLONG Manager::SendData(Data_Message * data_msg)
{
	return port->Send(data_msg, DATA_MESSAGE_SIZE);
}

DWORDLONG Manager::Send(int data, int purpose)
{
	Message msg;

	msg.purpose = purpose;
	msg.data = data;
	return Send(&msg);
}

//
// Getting a message from Dynamo.
//
DWORDLONG Manager::Receive(Message * msg)
{
	return port->Receive(msg);
}

DWORDLONG Manager::ReceiveData(Data_Message * data_msg)
{
	return port->Receive(data_msg, DATA_MESSAGE_SIZE);
}

DWORDLONG Manager::Receive()
{
	Message msg;

	return Receive(&msg);
}

//
// Updating the list of targets known by a manager.  This retrieves the
// target information from Dynamo.
//
void Manager::UpdateTargetLists()
{
	// Query worker for list of available targets.
	Send(MANAGER, REPORT_TARGETS);

	// Reset information about currently stored targets.
	RemoveDiskInfo();
	RemoveNetInfo();

	// Get the reply containing the target information and initialize the lists.
	InitTargetList(&disks);
	InitTargetList(&tcps);
	InitTargetList(&vis);
}

//
// Initialize the specified target list using the target specifications
// given in the data message.
//
void Manager::InitTargetList(CTypedPtrArray < CPtrArray, Target_Spec * >*targets)
{
	int i;
	Data_Message data_msg;
	Target_Spec *target_spec;

	// Receive the target specifications in a data message.
	ReceiveData(&data_msg);

	// Add the targets to the specified array.
	for (i = 0; i < data_msg.count; i++) {
		target_spec = new Target_Spec;
		memcpy(target_spec, &data_msg.data.targets[i], sizeof(Target_Spec));

		// Initialize the target specs to the default settings.
		target_spec->queue_depth = 1;
		target_spec->test_connection_rate = FALSE;
		target_spec->trans_per_conn = 1;

		targets->Add(target_spec);
	}
}

//
// Sets each workers targets to run for the next test.  This call will result
// in each worker sending their active target list to Dynamo.
//
BOOL Manager::SetTargets()
{
	int w, wkr_count = WorkerCount();

	for (w = 0; w < wkr_count; w++) {
		if (!GetWorker(w)->SetTargets())
			return FALSE;
	}
	return TRUE;
}

//
// Setting targets that need to be prepared for the specified worker.
//
void Manager::SetTargetsToPrepare(int worker_index)
{
	GetWorker(worker_index)->SetTargetsToPrepare();
}

//
// Receives the answer to a prepare command.
//
BOOL Manager::PreparedAnswer(int worker_index)
{
	Message msg;
	int t, i;
	Worker *wkr = GetWorker(worker_index);

	// Get the notification message that the prepare is done.
	Receive(&msg);

	// Verify that all targets were successfully prepared.
	if (!msg.data)
		return FALSE;

	// Update the ready status of all disk targets that the worker prepared.
	for (t = 0; t < wkr->TargetCount(); t++) {
		// Find the worker's corresponding disk in the manager's disk list.
		for (i = 0; i < InterfaceCount(GenericDiskType); i++) {
			if (!strcmp(wkr->GetTarget(t)->spec.name, GetInterface(i, GenericDiskType)->name)) {
				GetInterface(i, GenericDiskType)->disk_info.ready = TRUE;
			}
		}
	}

	// Update the Targets display if a manager or worker is selected, 
	// in case the manager or worker that just finished preparing its 
	// drives is currently displayed.  This will update the icon.
	theApp.pView->m_pPageDisk->ShowData();
	return TRUE;
}

//
// Saving the manager's results to a file along with all of its workers and
// their targets.
//
void Manager::SaveResults(ostream * file, int access_index, int result_type)
{
	int stat;
	char specname[MAX_WORKER_NAME];

	if (!ActiveInCurrentTest())
		return;

	// Save manager's results.
	(*file) << "MANAGER" << "," << name << "," << GetCommonAccessSpec(access_index, specname)
	    << ","		// Space for managers running.
	    << "," << WorkerCount(ActiveType)
	    << "," << TargetCount(ActiveType) + WorkerCount((TargetType) (GenericClientType | ActiveType))
	    << "," << results[WHOLE_TEST_PERF].IOps
	    << "," << results[WHOLE_TEST_PERF].read_IOps
	    << "," << results[WHOLE_TEST_PERF].write_IOps
	    << "," << results[WHOLE_TEST_PERF].MBps_Bin
	    << "," << results[WHOLE_TEST_PERF].read_MBps_Bin
	    << "," << results[WHOLE_TEST_PERF].write_MBps_Bin
	    << "," << results[WHOLE_TEST_PERF].MBps_Dec
	    << "," << results[WHOLE_TEST_PERF].read_MBps_Dec
	    << "," << results[WHOLE_TEST_PERF].write_MBps_Dec
	    << "," << results[WHOLE_TEST_PERF].transactions_per_second
	    << "," << results[WHOLE_TEST_PERF].connections_per_second
	    << "," << results[WHOLE_TEST_PERF].ave_latency
	    << "," << results[WHOLE_TEST_PERF].ave_read_latency
	    << "," << results[WHOLE_TEST_PERF].ave_write_latency
	    << "," << results[WHOLE_TEST_PERF].ave_transaction_latency
	    << "," << results[WHOLE_TEST_PERF].ave_connection_latency
	    << "," << results[WHOLE_TEST_PERF].max_latency
	    << "," << results[WHOLE_TEST_PERF].max_read_latency
	    << "," << results[WHOLE_TEST_PERF].max_write_latency
	    << "," << results[WHOLE_TEST_PERF].max_transaction_latency
	    << "," << results[WHOLE_TEST_PERF].max_connection_latency
	    << "," << results[WHOLE_TEST_PERF].total_errors
	    << "," << results[WHOLE_TEST_PERF].raw.read_errors << "," << results[WHOLE_TEST_PERF].raw.write_errors
	    // Save raw result information as well.
	    << "," << results[WHOLE_TEST_PERF].raw.bytes_read
	    << "," << results[WHOLE_TEST_PERF].raw.bytes_written
	    << "," << results[WHOLE_TEST_PERF].raw.read_count
	    << "," << results[WHOLE_TEST_PERF].raw.write_count
	    << "," << results[WHOLE_TEST_PERF].raw.connection_count << ",";

	if (GetConnectionRate(ActiveType) == ENABLED_VALUE)
		(*file) << GetTransPerConn(ActiveType);
	else
		(*file) << AMBIGUOUS_VALUE;

	(*file) << "," << results[WHOLE_TEST_PERF].raw.read_latency_sum
	    << "," << results[WHOLE_TEST_PERF].raw.write_latency_sum
	    << "," << results[WHOLE_TEST_PERF].raw.transaction_latency_sum
	    << "," << results[WHOLE_TEST_PERF].raw.connection_latency_sum
	    << "," << results[WHOLE_TEST_PERF].raw.max_raw_read_latency
	    << "," << results[WHOLE_TEST_PERF].raw.max_raw_write_latency
	    << "," << results[WHOLE_TEST_PERF].raw.max_raw_transaction_latency
	    << "," << results[WHOLE_TEST_PERF].raw.max_raw_connection_latency
	    << "," << results[WHOLE_TEST_PERF].raw.counter_time;

	(*file) << "," << GetDiskStart((TargetType) (GenericDiskType | ActiveType))
	    << "," << GetDiskSize((TargetType) (GenericDiskType | ActiveType))
	    << "," << GetQueueDepth(ActiveType);

	for (stat = 0; stat < CPU_UTILIZATION_RESULTS; stat++)
		(*file) << "," << results[WHOLE_TEST_PERF].CPU_utilization[stat];

		(*file) << "," << timer_resolution
		
		<< "," << results[WHOLE_TEST_PERF].CPU_utilization[CPU_IRQ]
	    << "," << results[WHOLE_TEST_PERF].CPU_effectiveness;

	for (stat = 0; stat < NI_COMBINE_RESULTS; stat++)
		(*file) << "," << results[WHOLE_TEST_PERF].ni_statistics[stat];

	for (stat = 0; stat < TCP_RESULTS; stat++)
		(*file) << "," << results[WHOLE_TEST_PERF].tcp_statistics[stat];

	(*file) << endl;

	// Save individual CPU results.
	for (int cpu = 0; cpu < processors; cpu++) {
		(*file) << "PROCESSOR" << "," << "CPU " << cpu << ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,";

		for (stat = 0; stat < CPU_UTILIZATION_RESULTS; stat++)
			(*file) << "," << results[WHOLE_TEST_PERF].individual_CPU_utilization[cpu][stat];

		(*file) << "," << timer_resolution

			<< "," << results[WHOLE_TEST_PERF].individual_CPU_utilization[cpu][CPU_IRQ]
		    << ",";	// Space for CPU_effectiveness (no way to calculate IOs per processor)

		for (stat = 0; stat < NI_COMBINE_RESULTS + TCP_RESULTS; stat++)
			(*file) << ",";	// space for network results

		(*file) << endl;
	}

	// If requested, save workers' results.
	if (result_type == RecordAll || result_type == RecordNoTargets) {
		for (int i = 0; i < WorkerCount(); i++)
			GetWorker(i)->SaveResults(file, access_index, result_type);
	}
}

//
// Updating the results stored with the manager.
//
void Manager::UpdateResults(int which_perf)
{
	Worker *worker;
	Data_Message data_msg;
	CPU_Results *cpu_results;
	Net_Results *net_results;
	_int64 start_perf_time, end_perf_time;
	int i, stat;		// loop control variables

	if ((which_perf < 0) || (which_perf >= MAX_PERF))
		return;

	ResetResults(which_perf);

	// Dynamo will send one reply for itself and one for every worker running.

	// Send appropriate request for results to manager.
	if (which_perf == WHOLE_TEST_PERF) {
		if (Send(MANAGER, REPORT_RESULTS) == PORT_ERROR)
			return;
	} else			// which_perf == LAST_UPDATE_PERF
	{
		if (Send(MANAGER, REPORT_UPDATE) == PORT_ERROR)
			return;
	}

	// Get results from manager.
	if (ReceiveData(&data_msg) == PORT_ERROR)
		return;

	cpu_results = &(data_msg.data.manager_results.cpu_results);
	net_results = &(data_msg.data.manager_results.net_results);
	start_perf_time = data_msg.data.manager_results.time_counter[FIRST_SNAPSHOT];
	end_perf_time = data_msg.data.manager_results.time_counter[LAST_SNAPSHOT];

	// Reset aggregate related utilizations.
	for (stat = 0; stat < CPU_RESULTS; stat++)
		results[which_perf].CPU_utilization[stat] = (double)0;
	for (stat = 0; stat < NI_COMBINE_RESULTS; stat++)
		results[which_perf].ni_statistics[stat] = (double)0;

	// Loop though all CPUs.
	for (i = 0; i < processors; i++) {
		// Loop through the utilization counters.
		for (stat = 0; stat < CPU_RESULTS; stat++) {
			// Storing returned CPU utilization statistics.
			results[which_perf].individual_CPU_utilization[i][stat] = cpu_results->CPU_utilization[i][stat];
			results[which_perf].CPU_utilization[stat] += cpu_results->CPU_utilization[i][stat];	// calc. ave. below
		}
	}
	// Determine average aggregate CPU related utilizations.
	// Interrupts per second is a total, so is not averaged across CPUs.
	if (processors) {
		for (stat = 0; stat < CPU_UTILIZATION_RESULTS; stat++)
			results[which_perf].CPU_utilization[stat] /= processors;
	}
	// Record all network related statistics.
	for (stat = 0; stat < TCP_RESULTS; stat++)
		results[which_perf].tcp_statistics[stat] = net_results->tcp_stats[stat];
	for (i = 0; i < net_results->ni_count; i++) {
		results[which_perf].ni_statistics[NI_PACKETS] += net_results->ni_stats[i][NI_PACKETS];
		results[which_perf].ni_statistics[NI_ERRORS] += net_results->ni_stats[i][NI_OUT_ERRORS];
		results[which_perf].ni_statistics[NI_ERRORS] += net_results->ni_stats[i][NI_IN_ERRORS];
	}

	//
	// Update Worker Results
	//
	for (i = 0; i < WorkerCount(); i++) {
		worker = GetWorker(i);

		// Only update the results of workers active in the current test.
		if (!worker->ActiveInCurrentTest()) {
			// Reset the results of idle workers to prevent any older results
			// from being visible.
			worker->ResetAllResults();
			continue;
		}
		// Receive an update from a worker and process the results.
		worker->UpdateResults(which_perf);

		// Recording maximum time any of the workers ran.
		if (worker->results[which_perf].raw.counter_time > results[which_perf].raw.counter_time)
			results[which_perf].raw.counter_time = worker->results[which_perf].raw.counter_time;

		// Recording error results.
		results[which_perf].total_errors += worker->results[which_perf].total_errors;
		results[which_perf].raw.read_errors += worker->results[which_perf].raw.read_errors;
		results[which_perf].raw.write_errors += worker->results[which_perf].raw.write_errors;

		// Recording results related to the number of I/Os completed.
		results[which_perf].IOps += worker->results[which_perf].IOps;
		results[which_perf].read_IOps += worker->results[which_perf].read_IOps;
		results[which_perf].write_IOps += worker->results[which_perf].write_IOps;
		results[which_perf].raw.read_count += worker->results[which_perf].raw.read_count;
		results[which_perf].raw.write_count += worker->results[which_perf].raw.write_count;

		// Recording throughput results.
		results[which_perf].MBps_Bin += worker->results[which_perf].MBps_Bin;
		results[which_perf].read_MBps_Bin += worker->results[which_perf].read_MBps_Bin;
		results[which_perf].write_MBps_Bin += worker->results[which_perf].write_MBps_Bin;
		results[which_perf].MBps_Dec += worker->results[which_perf].MBps_Dec;
		results[which_perf].read_MBps_Dec += worker->results[which_perf].read_MBps_Dec;
		results[which_perf].write_MBps_Dec += worker->results[which_perf].write_MBps_Dec;
		results[which_perf].raw.bytes_read += worker->results[which_perf].raw.bytes_read;
		results[which_perf].raw.bytes_written += worker->results[which_perf].raw.bytes_written;

		// Recording results related to the number of transactions completed.
		results[which_perf].transactions_per_second += worker->results[which_perf].transactions_per_second;
		results[which_perf].raw.transaction_count += worker->results[which_perf].raw.transaction_count;

		// Recording results related to the number of connections completed.
		results[which_perf].connections_per_second += worker->results[which_perf].connections_per_second;
		results[which_perf].raw.connection_count += worker->results[which_perf].raw.connection_count;

		// Recording maximum latency results.
		if (results[which_perf].max_latency < worker->results[which_perf].max_latency)
			results[which_perf].max_latency = worker->results[which_perf].max_latency;
		if (results[which_perf].max_read_latency < worker->results[which_perf].max_read_latency) {
			results[which_perf].max_read_latency = worker->results[which_perf].max_read_latency;
			results[which_perf].raw.max_raw_read_latency =
			    worker->results[which_perf].raw.max_raw_read_latency;
		}
		if (results[which_perf].max_write_latency < worker->results[which_perf].max_write_latency) {
			results[which_perf].max_write_latency = worker->results[which_perf].max_write_latency;
			results[which_perf].raw.max_raw_write_latency =
			    worker->results[which_perf].raw.max_raw_write_latency;
		}
		if (results[which_perf].max_transaction_latency < worker->results[which_perf].max_transaction_latency) {
			results[which_perf].max_transaction_latency =
			    worker->results[which_perf].max_transaction_latency;
			results[which_perf].raw.max_raw_transaction_latency =
			    worker->results[which_perf].raw.max_raw_transaction_latency;
		}
		if (results[which_perf].max_connection_latency < worker->results[which_perf].max_connection_latency) {
			results[which_perf].max_connection_latency = worker->results[which_perf].max_connection_latency;
			results[which_perf].raw.max_raw_connection_latency =
			    worker->results[which_perf].raw.max_raw_connection_latency;
		}
		results[which_perf].raw.read_latency_sum += worker->results[which_perf].raw.read_latency_sum;
		results[which_perf].raw.write_latency_sum += worker->results[which_perf].raw.write_latency_sum;
		results[which_perf].raw.transaction_latency_sum +=
		    worker->results[which_perf].raw.transaction_latency_sum;
		results[which_perf].raw.connection_latency_sum +=
		    worker->results[which_perf].raw.connection_latency_sum;

		// Copying results only reported for the manager to the workers.
		// This allows the results to be displayed in the results page in the GUI.
		for (stat = 0; stat < CPU_RESULTS; stat++)
			worker->results[which_perf].CPU_utilization[stat] = results[which_perf].CPU_utilization[stat];
		for (stat = 0; stat < TCP_RESULTS; stat++)
			worker->results[which_perf].tcp_statistics[stat] = results[which_perf].tcp_statistics[stat];
		for (stat = 0; stat < NI_COMBINE_RESULTS; stat++)
			worker->results[which_perf].ni_statistics[stat] = results[which_perf].ni_statistics[stat];
	}

	// Calculate CPU_effectiveness (number of IOs per second divided by CPU efficiency)
	if (results[which_perf].CPU_utilization[CPU_TOTAL_UTILIZATION] != (double)0)	// avoid a divide by zero
		results[which_perf].CPU_effectiveness =
		    results[which_perf].IOps / results[which_perf].CPU_utilization[CPU_TOTAL_UTILIZATION];
	else
		results[which_perf].CPU_effectiveness = (double)0;

	// Calculating average latencies.
	if (results[which_perf].raw.read_count || results[which_perf].raw.write_count) {
		results[which_perf].ave_latency =
		    (double)(_int64) (results[which_perf].raw.read_latency_sum +
			results[which_perf].raw.write_latency_sum)
		    * (double)1000 / timer_resolution /
			(double)(_int64) (results[which_perf].raw.read_count +
			results[which_perf].raw.write_count);

		if (results[which_perf].raw.read_count)
			results[which_perf].ave_read_latency =
			    (double)(_int64) results[which_perf].raw.read_latency_sum * (double)1000 / 
				timer_resolution / (double)(_int64) results[which_perf].raw.read_count;
		else
			results[which_perf].ave_read_latency = (double)0;

		if (results[which_perf].raw.write_count)
			results[which_perf].ave_write_latency =
			    (double)(_int64) results[which_perf].raw.write_latency_sum * (double)1000 /
				timer_resolution / (double)(_int64) results[which_perf].raw.write_count;
		else
			results[which_perf].ave_write_latency = (double)0;

		if (results[which_perf].raw.transaction_count) {
			results[which_perf].ave_transaction_latency =
			    (double)(_int64) results[which_perf].raw.transaction_latency_sum * (double)1000 /
				timer_resolution / (double)(_int64) (results[which_perf].raw.transaction_count);
		} else {
			results[which_perf].ave_transaction_latency = (double)0;
		}
	} else {
		results[which_perf].ave_latency = (double)0;
		results[which_perf].ave_read_latency = (double)0;
		results[which_perf].ave_write_latency = (double)0;
		results[which_perf].ave_transaction_latency = (double)0;
	}

	// Calculating average connection time.
	if (results[which_perf].raw.connection_count) {
		results[which_perf].ave_connection_latency =
		    (double)(_int64) (results[which_perf].raw.connection_latency_sum) * (double)1000 / 
			timer_resolution / (double)(_int64) results[which_perf].raw.connection_count;
	} else {
		results[which_perf].ave_connection_latency = (double)0;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
// The following functions update the values for the starting sector, the 
// number of sectors to access, the queue depth, and the disk selection.
// On the Manager level, it propagates down the tree to all the workers and
// calls the workers function.
//
///////////////////////////////////////////////////////////////////////////////
void Manager::SetDiskSize(DWORDLONG disk_size)
{
	int w, wkr_count;

	// Loop through all the workers.
	wkr_count = WorkerCount(GenericDiskType);
	for (w = 0; w < wkr_count; w++)
		GetWorker(w, GenericDiskType)->SetDiskSize(disk_size);
}

void Manager::SetDiskStart(DWORDLONG disk_start)
{
	int w, wkr_count;

	// Loop through all the workers.
	wkr_count = WorkerCount(GenericDiskType);
	for (w = 0; w < wkr_count; w++)
		GetWorker(w, GenericDiskType)->SetDiskStart(disk_start);
}

void Manager::SetQueueDepth(int queue_depth, TargetType type)
{
	int w, wkr_count;

	// Loop through all the workers.
	wkr_count = WorkerCount(type);
	for (w = 0; w < wkr_count; w++)
		GetWorker(w, GenericDiskType)->SetQueueDepth(queue_depth);
}

void Manager::SetMaxSends(int max_sends)
{
	int w, wkr_count;

	// Loop through all the workers.
	wkr_count = WorkerCount(VIServerType);
	for (w = 0; w < wkr_count; w++)
		GetWorker(w, VIServerType)->SetMaxSends(max_sends);
}

void Manager::SetLocalNetworkInterface(int iface_index)
{
	int w, wkr_count;

	// Loop through all the workers.
	wkr_count = WorkerCount(GenericServerType);
	for (w = 0; w < wkr_count; w++)
		GetWorker(w, GenericServerType)->SetLocalNetworkInterface(iface_index);
}

void Manager::SetConnectionRate(BOOL test_connection_rate, TargetType type)
{
	int w, wkr_count;

	// Loop through all the workers.
	wkr_count = WorkerCount(type);
	for (w = 0; w < wkr_count; w++)
		GetWorker(w, type)->SetConnectionRate(test_connection_rate);
}

void Manager::SetDataPattern(int data_pattern, TargetType type)
{
	int w, wkr_count;

	// Loop through all the workers.
	wkr_count = WorkerCount(type);
	for (w = 0; w < wkr_count; w++)
		GetWorker(w, type)->SetDataPattern(data_pattern);
}

void Manager::SetTransPerConn(int trans_per_conn, TargetType type)
{
	int w, wkr_count;

	// Loop through all the workers.
	wkr_count = WorkerCount(type);
	for (w = 0; w < wkr_count; w++)
		GetWorker(w, type)->SetTransPerConn(trans_per_conn);
}

///////////////////////////////////////////////
//
// Functions to retrieve worker information
//
// Functions return a valid value if all 
// underlying workers have the same value.
//
///////////////////////////////////////////////

//
// Returns a valid connection rate setting if all the manager's workers of the
// same type have the same value.
//
int Manager::GetConnectionRate(TargetType type)
{
	int w, wkr_count, wkr_result;

	// If there are no workers, return immediately.
	if (!(wkr_count = WorkerCount(type)))
		return AMBIGUOUS_VALUE;

	// Find the first worker of the specified type's transaction per
	// connection value.
	wkr_result = GetWorker(0, type)->GetConnectionRate(type);

	// Compare the value with all the other workers of the same type.
	for (w = 1; w < wkr_count; w++) {
		if (wkr_result != GetWorker(w, type)->GetConnectionRate(type)) {
			// The value isn't the same.
			return AMBIGUOUS_VALUE;
		}
	}
	// All workers have the same value.
	return wkr_result;
}

//
// Returns if the user has chosen to use random data
//
int Manager::GetDataPattern(TargetType type)
{
	BOOL wkr_result;
	int w, wkr_count;

	// If there are no workers, return immediately.
	if (!(wkr_count = WorkerCount(type)))
		return 1; //Full random data pattern

	// Find the first worker of the specified type's transaction per
	// connection value.
	wkr_result = GetWorker(0, type)->GetDataPattern(type);

	// Compare the value with all the other workers of the same type.
	for (w = 1; w < wkr_count; w++) {
		if (wkr_result != GetWorker(w, type)->GetDataPattern(type)) {
			// The value isn't the same.
			AfxMessageBox("The data pattern values selected for each worker is not the same. Defaulting to 'Full random'.");
			return DATA_PATTERN_FULL_RANDOM; //Full random data pattern
		}
	}
	// All workers have the same value.
	return wkr_result;
}

//
// Returns a valid transaction per connection value if all the manager's
// workers of the specified type have the same value.
//
int Manager::GetTransPerConn(TargetType type)
{
	int w, wkr_count, wkr_result;

	// If there are no workers, return immediately.
	if (!(wkr_count = WorkerCount(type)))
		return AMBIGUOUS_VALUE;

	// Find the first worker of the specified type's transaction per
	// connection value.
	wkr_result = GetWorker(0, type)->GetTransPerConn(type);

	// Compare the value with all the other workers of the same type.
	for (w = 1; w < wkr_count; w++) {
		if (wkr_result != GetWorker(w, type)->GetTransPerConn(type)) {
			// The value isn't the same.
			return AMBIGUOUS_VALUE;
		}
	}
	// Return the first worker's value if all other workers have the same.
	return wkr_result;
}

//
// Returns a valid disk start value if all the manager's disk workers
// have the same value.
//
DWORDLONG Manager::GetDiskStart(TargetType type)
{
	int w, wkr_count;
	DWORDLONG wkr_result;

	// If there are no workers, return immediately.
	if (!(wkr_count = WorkerCount(type)))
		return AMBIGUOUS_VALUE;

	// Find the first disk worker.
	wkr_result = GetWorker(0, type)->GetDiskStart(type);

	// Compare the value with all the other workers of the same type.
	for (w = 1; w < wkr_count; w++) {
		if (wkr_result != GetWorker(w, type)->GetDiskStart(type)) {
			// The value isn't the same.
			return AMBIGUOUS_VALUE;
		}
	}
	// Return the first worker's value if all other workers have the same.
	return wkr_result;
}

//
// Returns a valid disk start value if all the manager's disk workers
// have the same value.
//
DWORDLONG Manager::GetDiskSize(TargetType type)
{
	int w, wkr_count;
	DWORDLONG wkr_result;

	// If there are no workers, return immediately.
	if (!(wkr_count = WorkerCount(type)))
		return AMBIGUOUS_VALUE;

	// Find the first disk worker.
	wkr_result = GetWorker(0, type)->GetDiskSize(type);

	// Compare the value with all the other workers of the same type.
	for (w = 1; w < wkr_count; w++) {
		if (wkr_result != GetWorker(w, type)->GetDiskSize(type)) {
			// The value isn't the same.
			return AMBIGUOUS_VALUE;
		}
	}
	// Return the first worker's value if all other workers have the same.
	return wkr_result;
}

//
// Returns a valid queue depth value if all the manager's workers
// have the same value.
//
int Manager::GetQueueDepth(TargetType type)
{
	int w, wkr_count, wkr_result;

	// If there are no workers, return immediately.
	if (!(wkr_count = WorkerCount(type)))
		return AMBIGUOUS_VALUE;

	// Find the first worker of the specified type's queue depth value.
	wkr_result = GetWorker(0, type)->GetQueueDepth(type);

	// Compare the value with all the other workers of the same type.
	for (w = 1; w < wkr_count; w++) {
		if (wkr_result != GetWorker(w, type)->GetQueueDepth(type)) {
			// The value isn't the same.
			return AMBIGUOUS_VALUE;
		}
	}
	// Return the first worker's value if all other workers have the same.
	return wkr_result;
}

//
// Returns a valid maximum number of outstanding sends value if all the 
// manager's disk workers have the same value.
//
int Manager::GetMaxSends(TargetType type)
{
	int w, wkr_count, wkr_result;

	// If there are no workers, return immediately.
	if (!(wkr_count = WorkerCount(type)))
		return AMBIGUOUS_VALUE;

	// Find the first worker of the specified type's transaction per
	// connection value.
	wkr_result = GetWorker(0, type)->GetMaxSends(type);

	// Compare the value with all the other workers of the same type.
	for (w = 1; w < wkr_count; w++) {
		if (wkr_result != GetWorker(w, type)->GetMaxSends(type)) {
			// The value isn't the same.
			return AMBIGUOUS_VALUE;
		}
	}
	// Return the first worker's value if all other workers have the same.
	return wkr_result;
}

//
// Returns a valid pointer to a net address string if all the manager's 
// workers have the same net address selected, otherwise NULL.
// 
char *Manager::GetLocalNetworkInterface()
{
	char *net_address;
	int w, wkr_count;

	// If the manager has no GenericServerType workers, there is no valid 
	// net address.
	if (!(wkr_count = WorkerCount(GenericServerType)))
		return NULL;

	// Find the first GenericServerType's local address.
	net_address = GetWorker(0, GenericServerType)->GetLocalNetworkInterface();

	// Compare the first GenericServerType's net address with all other 
	// GenericServerType's.
	for (w = 1; w < wkr_count; w++) {
		if (strcmp(net_address, GetWorker(w, GenericServerType)->GetLocalNetworkInterface())) {
			return NULL;
		}
	}

	// All the manager's workers are using the same local address.
	return net_address;
}

//
// Return a pointer to the manager's information about a given interface.
// This applies to Disk targets as well as Network targets.
//
Target_Spec *Manager::GetInterface(int index, TargetType type)
{
	if (IsType(type, GenericDiskType))
		return disks[index];
	else if (IsType(type, GenericTCPType))
		return tcps[index];
	else if (IsType(type, GenericVIType))
		return vis[index];
	else if (IsType(type, GenericNetType)) {
		// Networks are viewed as TCP interfaces then VI interfaces.
		if (index < InterfaceCount(GenericTCPType))
			return tcps[index];
		return vis[index - InterfaceCount(GenericTCPType)];
	} else {
		ErrorMessage("Unknown target type in Manager::GetInterface()");
		return NULL;
	}
}

//
// Returning the number of interfaces accessible by a manager.  If desired,
// a count of a specific type may be returned.
//
int Manager::InterfaceCount(TargetType type)
{
	switch (type) {
	case GenericType:
		return disks.GetSize() + tcps.GetSize() + vis.GetSize();
	case GenericDiskType:
		return disks.GetSize();
	case GenericNetType:
	case GenericServerType:
	case GenericClientType:
		return tcps.GetSize() + vis.GetSize();
	case GenericTCPType:
	case TCPServerType:
	case TCPClientType:
		return tcps.GetSize();
	case GenericVIType:
	case VIServerType:
	case VIClientType:
		return vis.GetSize();
	default:
		ErrorMessage("Unknown target type in Manager::InterfaceCount()");
		return 0;
	}
}

///////////////////////////////////////////////
//
// Functions that deal with access specs.
//
///////////////////////////////////////////////

//
// Return the maximum number of access specs assigned to a worker.
//
int Manager::GetMaxAccessSpecCount()
{
	int max_spec_count = 0, wkr_count = WorkerCount();
	int w, spec_count;

	for (w = 0; w < wkr_count; w++) {
		if ((spec_count = GetWorker(w)->AccessSpecCount()) > max_spec_count) {
			max_spec_count = spec_count;
		}
	}
	return max_spec_count;
}

//
// Verify that all assigned access specs are valid.
//
BOOL Manager::InvalidSetup(BOOL & invalidSpecOK)
{
	int i, w, wkr_count, iface_count, conn_count, queue_depth;
	DWORD max_size;
	Worker *wkr;
	CString msg_box;

	// Verify that all specs for each worker is valid.
	wkr_count = WorkerCount();
	for (w = 0; w < wkr_count; w++) {
		if (GetWorker(w)->InvalidSetup(invalidSpecOK))
			return TRUE;
	}

	// If there are no targets using VI interfaces, everything is valid.
	if (!WorkerCount(GenericVIType))
		return FALSE;

	// Verify that the manager's VIA interfaces can handle the assigned
	// test setup.
	iface_count = InterfaceCount(VIClientType);
	wkr_count = WorkerCount(GenericVIType);
	for (i = 0; i < iface_count; i++) {
		// Update how much of the VI hardware is in use.
		conn_count = 0;
		queue_depth = 0;
		for (w = 0; w < wkr_count; w++) {
			wkr = GetWorker(w, GenericVIType);

			// Skip workers without VI targets.
			if (!wkr->TargetCount(GenericVIType))
				continue;

			// If the worker will use the VI interface, count it.
			if (!strcmp(wkr->GetLocalNetworkInterface(), vis[i]->name)) {
				conn_count++;
				queue_depth += wkr->MaxOutstandingIOs(0);
			}
			// Ensure that the worker will not make an I/O request larger
			// than that supported by the VI NIC.
			max_size = wkr->MaxTransferSize();
			if (max_size > (DWORD) vis[i]->vi_info.max_transfer_size) {
				msg_box.Format((CString) "Worker \"" + wkr->name + "\" on " +
					       "manager \"" + name + "\" cannot perform the largest requested " +
					       "transfer size over VI NIC \"" + vis[i]->name +
					       "\".  VI hardware supports transfers up to " +
					       "%i bytes.  %i bytes were requested.",
					       vis[i]->vi_info.max_transfer_size, max_size);
				ErrorMessage(msg_box);
				return TRUE;
			}
		}

		// See that the VI NIC can handle the requested number of connections.
		if (conn_count > vis[i]->vi_info.max_connections) {
			msg_box.Format((CString) "Manager \"" + name + "\" cannot support the " +
				       "requested number of VI connections over VI NIC \""
				       + vis[i]->name + "\".  Only %i connections are allowed.  " +
				       "%i were requested.", vis[i]->vi_info.max_connections, conn_count);
			ErrorMessage(msg_box);
			return TRUE;
		}
		// See that the VI NIC can handle the requested number of posted 
		// descriptors.
		if (queue_depth > vis[i]->vi_info.outstanding_ios) {
			msg_box.Format((CString) "Manager \"" + name + "\" cannot support the " +
				       "requested number of outstanding I/Os over VI NIC \""
				       + vis[i]->name + "\".  Only %i connections are allowed.  " +
				       "%i were requested.", vis[i]->vi_info.outstanding_ios, queue_depth);
			ErrorMessage(msg_box);
			return TRUE;
		}
	}
	return FALSE;
}

//
// Assigns the default access specs to workers based on their type.
//
void Manager::AssignDefaultAccessSpecs()
{
	int w, wkr_count = WorkerCount();

	for (w = 0; w < wkr_count; w++) {
		GetWorker(w)->AssignDefaultAccessSpecs();
	}
}

//
// Remove all instances of an access spec from all the manager's
// worker's access spec lists.
//
void Manager::RemoveAccessSpec(Test_Spec * spec)
{
	int worker_count = WorkerCount();

	for (int w = 0; w < worker_count; w++)
		GetWorker(w)->RemoveAccessSpec(spec);
}

//
// Remove a specific instance of an access spec from all the manager's
// worker's access spec lists (only if they are all the same).
//
void Manager::RemoveAccessSpecAt(int index)
{
	if (!AreAccessSpecsIdentical())
		return;

	int worker_count = WorkerCount();

	for (int w = 0; w < worker_count; w++)
		GetWorker(w)->RemoveAccessSpecAt(index);
}

//
// Removes all entries from all the manager's
// worker's access spec lists.
//
void Manager::RemoveAllAccessSpecs()
{
	int w, wkr_count = WorkerCount();
	Worker *wkr;

	for (w = 0; w < wkr_count; w++) {
		wkr = GetWorker(w);

		if (IsType(wkr->Type(), GenericClientType))
			continue;

		wkr->RemoveAllAccessSpecs();
	}
}

//
// Adds spec[index] from global access spec list
// to all the manager's workers at position 'before_index'.
//
BOOL Manager::InsertAccessSpec(Test_Spec * spec, int before_index)
{
	int w, wkr_count = WorkerCount();
	Worker *wkr;

	for (w = 0; w < wkr_count; w++) {
		wkr = GetWorker(w);

		// Do not add specs to client workers.
		if (IsType(wkr->Type(), GenericClientType))
			continue;

		if (!wkr->InsertAccessSpec(spec, before_index))
			return FALSE;
	}
	return TRUE;
}

//
// Moves the entry at position [index] to [before_index] in the 
// workers' access spec list to all the manager's workers.
//
void Manager::MoveAccessSpec(int index, int before_index)
{
	int w, wkr_count = WorkerCount();
	Worker *wkr;

	for (w = 0; w < wkr_count; w++) {
		wkr = GetWorker(w);

		// Do not move client worker specs.
		if (IsType(wkr->Type(), GenericClientType))
			continue;

		wkr->MoveAccessSpec(index, before_index);
	}
}

//
// Determines if the manager will be active in the current test.  This is
// TRUE if any of its workers are active or have been explicitly assigned the
// Idle spec.
//
BOOL Manager::ActiveInCurrentTest()
{
	int w, wkr_count = WorkerCount();
	int access_index = theApp.pView->GetCurrentAccessIndex();
	Worker *wkr;

	// Loop through all workers and see if they're either active or have been
	// explicitly assigned the Idle spec.
	for (w = 0; w < wkr_count; w++) {
		wkr = GetWorker(w);

		// See if the worker's active or has the Idle spec.
		if (wkr->ActiveInCurrentTest() || wkr->HasIdleCurrentSpec())
			return TRUE;
	}
	return FALSE;
}

//
// Returns TRUE if any worker has an access spec assigned for the current
// access index and the spec is not the Idle spec.
//
BOOL Manager::HasActiveCurrentSpec()
{
	int w, wkr_count = WorkerCount();

	for (w = 0; w < wkr_count; w++) {
		if (GetWorker(w)->HasActiveCurrentSpec())
			return TRUE;
	}
	return FALSE;
}

//
// Returns TRUE if any of the manager's workers has an idle spec assigned
// at the current access spec index.
//
BOOL Manager::HasIdleCurrentSpec()
{
	int w, wkr_count = WorkerCount();

	for (w = 0; w < wkr_count; w++) {
		if (GetWorker(w)->HasIdleCurrentSpec())
			return TRUE;
	}
	return FALSE;
}

//
// Returns a TRUE if all of this manager's workers have the same access 
// specification list.
//
BOOL Manager::AreAccessSpecsIdentical()
{
	int w, s, wkr_count, spec_count;
	Worker *compare_worker, *current_worker;

	// Get the first non-client worker
	wkr_count = WorkerCount();
	for (w = 0; w < wkr_count; w++) {
		compare_worker = GetWorker(w);

		if (!IsType(compare_worker->Type(), GenericClientType)) {
			spec_count = compare_worker->AccessSpecCount();
			break;
		}
	}

	// If there were no non-client workers, return TRUE;
	if (w == wkr_count)
		return TRUE;

	// Compare the first worker's Test_Spec to each other worker's Test_Spec.
	for (++w; w < wkr_count; w++) {
		current_worker = GetWorker(w);

		// Skip network clients.
		if (IsType(current_worker->Type(), GenericClientType))
			continue;

		// If this worker doesn't have the same number of access specs, return FALSE.
		if (current_worker->AccessSpecCount() != spec_count)
			return FALSE;

		// Check to make sure each returned Test_Spec pointer is identical to 
		// and in the same order as the pointer in the first_worker.
		for (s = 0; s < spec_count; s++) {
			if (current_worker->GetAccessSpec(s) != compare_worker->GetAccessSpec(s))
				return FALSE;
		}
	}

	return TRUE;
}

//
// Sees whether all workers have the same access spec assigned at index spec_index.
// Returns a pointer to the name of the spec if all workers have the same one assigned.
// Returns a pointer to a null string (not a null pointer) otherwise.
//
// Calling function supplies pointer to a char[MAX_WORKER_NAME].
// This same pointer is returned for convenience.
//
char *Manager::GetCommonAccessSpec(int spec_index, char *const specname)
{
	int w, wkr_count = WorkerCount();
	Worker *wkr;

	// Return null string if there are no workers.
	if (wkr_count == 0) {
		specname[0] = '\0';
		return specname;
	}
	// Find the first worker with this spec assigned and get the name of the spec.
	for (w = 0; w < wkr_count; w++) {
		wkr = GetWorker(w);
		if (spec_index < wkr->AccessSpecCount()) {
			strcpy(specname, wkr->GetAccessSpec(spec_index)->name);
			break;
		}
	}

	if (w >= wkr_count) {
		ASSERT(0);	// (THIS SHOULD NOT HAPPEN.)

		// Return an error if no worker has this access spec.
		return strcpy(specname, "***ERROR***");
	}

	w++;

	// Compare that spec name to each other worker's spec name.
	while (w < wkr_count) {
		// Return null string if this worker's spec is different.
		// (Ignore this worker if it is not participating in this test.)
		wkr = GetWorker(w);
		if (wkr->ActiveInCurrentTest() && strcmp(wkr->GetAccessSpec(spec_index)->name, specname)) {
			specname[0] = '\0';
			return specname;
		}

		w++;
	}

	// They all matched, so return the name of the spec.
	return specname;
}

//
// Save the manager/worker configuration to the specified stream.
//              save_aspecs --> should each worker's access spec assignments be saved?
//              save_targets --> should each worker's target assignments be saved?
//
BOOL Manager::SaveConfig(ostream & outfile, BOOL save_aspecs, BOOL save_targets)
{
	int wkr_count = WorkerCount();

	outfile << "'Manager ID, manager name" << endl << "\t" << id << "," << name << endl;
	outfile << "'Manager network address" << endl << "\t" << network_name << endl;

	for (int counter = 0; counter < wkr_count; counter++)
		if (!GetWorker(counter)->SaveConfig(outfile, save_aspecs, save_targets))
			return FALSE;	// error saving data - propagate back

	outfile << "'End manager" << endl;
	return TRUE;
}

//
// Restore the manager/worker configuration from the specified stream.
//              load_aspecs --> should each worker's access spec assignments be loaded?
//              load_targets --> should each worker's target assignments be loaded?
//
// Returns TRUE on success, FALSE if any of the requested information
// couldn't be found in the file.
//
BOOL Manager::LoadConfig(ICF_ifstream & infile, BOOL load_aspecs, BOOL load_targets)
{
	CString key, value;
	int counter;
	CString wkr_name, token;
	TargetType wkr_type_hex;
	Worker *wkr;
	int wkr_count, wkr_svr_count;

	// Using a dynamic because size isn't known at compile time.
	CArray < BOOL, int >wkr_svr_loaded;	// identifies workers that have been loaded
	CArray < Worker *, int >wkr_svr_list;	// Pointers to each of the workers (they may move)

	wkr_count = WorkerCount();

	// The number of array elements needed is really (wkr_count - n),
	// where n is the number of network clients, but n is unknown and
	// this doesn't waste much space, as n should always be small.
	wkr_svr_loaded.SetSize(wkr_count);
	wkr_svr_list.SetSize(wkr_count);

	// Identify all workers that are not clients.
	// (We don't want to deal with the clients here - that's the job of target selection.)
	wkr_svr_count = 0;
	for (counter = 0; counter < wkr_count; counter++) {
		wkr = GetWorker(counter);
		if (!IsType(wkr->Type(), GenericClientType)) {
			wkr_svr_list[wkr_svr_count] = wkr;
			wkr_svr_loaded[wkr_svr_count] = FALSE;
			wkr_svr_count++;
		}
	}

	while (1) {
		if (!infile.GetPair(key, value)) {
			ErrorMessage("File is improperly formatted.  Expected a " "worker or \"End manager\".");
			return FALSE;
		}

		if (key.CompareNoCase("'End manager") == 0) {
			break;
		} else if (key.CompareNoCase("'Worker") == 0) {
			wkr_name = value;

			if (!infile.GetPair(key, value)) {
				ErrorMessage("File is improperly formatted.  Expected " "\"Worker type\".");
				return FALSE;
			}

			if (key.CompareNoCase("'Worker type") != 0) {
				ErrorMessage("File is improperly formatted.  Worker name "
					     "should be followed by \"Worker type\" comment.");
				return FALSE;
			}

			token = ICF_ifstream::ExtractFirstToken(value);
			if (token.CompareNoCase("DISK") == 0) {
				wkr_type_hex = GenericDiskType;
			} else if (token.CompareNoCase("NETWORK") == 0) {
				token = ICF_ifstream::ExtractFirstToken(value);
				if (token.CompareNoCase("TCP") == 0) {
					wkr_type_hex = TCPServerType;
				} else if (token.CompareNoCase("VI") == 0) {
					wkr_type_hex = VIServerType;
				} else {
					ErrorMessage("Unknown network worker subtype encountered "
						     "for worker \"" + wkr_name + "\": \"" + token
						     + "\".  Should be either TCP or VI.");
				}
			} else {
				ErrorMessage("Unknown worker type encountered for worker \""
					     + wkr_name + "\": \"" + token + "\".  Should be either DISK or NETWORK.");
				return FALSE;
			}

			// See if this worker already exists on this manager.
			for (counter = 0; counter < wkr_svr_count; counter++) {
				if (wkr_name.CompareNoCase(wkr_svr_list[counter]->name) == 0
				    && IsType(wkr_svr_list[counter]->Type(),
					      (TargetType) (wkr_type_hex & WORKER_COMPATIBILITY_MASK))
				    && !wkr_svr_loaded[counter]) {
					wkr_svr_loaded[counter] = TRUE;
					wkr = wkr_svr_list[counter];

					break;	// Worker was found, stop looking.
				}
			}

			// See if we weren't able to find a match.
			if (counter >= wkr_svr_count) {
				// This worker doesn't already exist, or two workers of the
				// same name have been defined for a single manager, and only
				// one worker by that name exists in the running copy of
				// Iometer.

				// So this worker needs to be created.

				// wkr_count is intentionally not being incremented after the
				// addition of this new worker.  It is only used above, to
				// search for previously existing workers.

				wkr = theApp.pView->AddWorker(wkr_type_hex, this, wkr_name);

				if (load_aspecs) {
					// If the user has specified that he wishes to restore
					// access spec assignments, no default assignments should
					// take place.
					wkr->RemoveAllAccessSpecs();
				}
			}
			// Load the worker.
			if (!wkr->LoadConfig(infile, load_aspecs, load_targets))
				return FALSE;
		} else {
			ErrorMessage("File is improperly formatted.  MANAGER section "
				     "contained an unrecognized \"" + key + "\" comment.");
			return FALSE;
		}
	}

	return TRUE;
}

//
// Disambiguate same-named managers.  This allows managers
// to be uniquely identified in saved files and other places.
// This should be called any time managers are added, removed, or renamed.
//
void Manager::IndexWorkers()
{
	Worker *wkr, *otherwkr;
	int count_up, count_down;
	const int wkr_count = WorkerCount();

	for (count_up = 0; count_up < wkr_count; count_up++) {
		wkr = GetWorker(count_up);

		wkr->id = 1;	// Assume this is unique, unless proven otherwise.

		for (count_down = count_up - 1; count_down >= 0; count_down--) {
			otherwkr = GetWorker(count_down);

			if (CString(wkr->name).CompareNoCase(otherwkr->name) == 0) {
				// If the workers' names are identical, assign
				// this worker an ID value one greater.
				wkr->id = otherwkr->id + 1;
				break;
			}
		}
	}
}
