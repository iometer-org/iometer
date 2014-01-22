/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / Worker.cpp                                             ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: This file implements the the worker class which keeps ## */
/* ##               track of results, access specifications, and targets  ## */
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
/* ##  Changes ...: 2004-09-01 (henryx.w.tieman@intel.com)                ## */
/* ##               - X86_64 is little-endian for VIPL code.              ## */
/* ##               2004-06-11 (lamontcranston41@yahoo.com)               ## */
/* ##               - Add code to allow potentially invalid access specs  ## */
/* ##                 but warn the user.                                  ## */
/* ##               2003-10-17 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               2003-07-19 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Removed IOTime.h inclusion (now in IOCommon.h)      ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-04-25 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Updated the global debug flag (_DEBUG) handling     ## */
/* ##                 of the source file (check for platform etc.).       ## */
/* ##               2003-03-02 (joe@eiler.net)                            ## */
/* ##               - Changed LONG_PTR to ULONG_PTR, which is what it is  ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include "IOCommon.h"
#include "stdafx.h"
#include "GalileoApp.h"
#include "Worker.h"
#include "Manager.h"
#include "GalileoView.h"
#include "ManagerList.h"
#include "AccessSpecList.h"

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
#ifdef IOMTR_SETTING_MFC_MEMALLOC_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

//
// Initializing information for a new worker.
//
Worker::Worker(Manager * mgr, TargetType wkr_type)
{
	manager = mgr;

	net_partner = NULL;

	// Initializing target information.
	targets.SetSize(INITIAL_ARRAY_SIZE, ARRAY_GROW_STEP);

	ResetAllResults();

	// Set the initial default target information.
	memset(&spec, 0, sizeof(Target_Spec));
	spec.type = wkr_type;
	spec.queue_depth = 1;
	spec.test_connection_rate = FALSE;
	spec.trans_per_conn = 1;
	spec.DataPattern = DATA_PATTERN_REPEATING_BYTES;

	if (IsType(wkr_type, GenericServerType)) {
		SetLocalNetworkInterface(0, (TargetType) (wkr_type & NETWORK_COMPATIBILITY_MASK));
	} else if (IsType(wkr_type, GenericClientType)) {
		net_target_index = 0;
		return;
	}
	// Initialize the access spec array and fill in the defaults.
	access_spec_list.SetSize(INITIAL_ARRAY_SIZE, ARRAY_GROW_STEP);
	AssignDefaultAccessSpecs();
}

Worker::~Worker()
{
	// When a client is removed, its server must remove it as a target.
	if (IsType(Type(), GenericClientType) && net_partner) {
		net_partner->net_partner = NULL;
		net_partner->RemoveTarget(net_target_index);
	}
	// Free all assigned targets.
	RemoveTargets();
	targets.FreeExtra();
}

//
// Copies all settings from the source worker.
//
void Worker::Clone(Worker * source_worker)
{
	int i;
	TargetType type;

	strcpy(name, source_worker->name);
	memcpy(&spec, &source_worker->spec, sizeof(Target_Spec));

	// Copy the worker's access specs.
	RemoveAllAccessSpecs();
	for (i = 0; i < source_worker->AccessSpecCount(); i++) {
		InsertAccessSpec(source_worker->GetAccessSpec(i));
	}

	// Copy the template's target information.
	for (i = 0; i < source_worker->TargetCount(); i++) {
		AddTarget(&source_worker->GetTarget(i)->spec);

		// Create a network client if necessary.
		type = source_worker->GetTarget(i)->spec.type;
		if (IsType(type, GenericClientType))
			CreateNetClient(source_worker->net_partner->manager, type);
	}
}

//
// Getting a worker's target.
// 
Target *Worker::GetTarget(int index, TargetType type)
{
	int i, target_count;

	// Verify valid index into target array.
	if (index < 0 || index >= TargetCount(type)) {
		ErrorMessage("Invalid index in Worker::GetTarget().");
		return NULL;
	}

	target_count = TargetCount();
	for (i = 0; i < target_count; i++) {
		if (IsType(targets[i]->spec.type, type)) {
			if (!index--)
				return targets[i];
		}
	}

	ErrorMessage("Unexpectedly reached end of Worker::GetTarget().");
	return NULL;
}

//
// Getting count of worker's targets of the specified type.
// 
int Worker::TargetCount(TargetType type)
{
	int i, count = 0, target_count;

	target_count = targets.GetSize();
	for (i = 0; i < target_count; i++) {
		if (IsType(targets[i]->spec.type, type))
			count++;
	}
	return count;
}

//
// Determines if a target matching the given target specifications has been
// assigned to the worker.
//
BOOL Worker::IsTargetAssigned(Target_Spec * target_info)
{
	int i, target_count;

	// Network clients have the target assigned if it refers to their server.
	if (IsType(Type(), GenericClientType) && IsType(target_info->type, GenericNetType)) {
		return !strcmp(target_info->name, net_partner->GetTarget(net_target_index)->spec.name);
	}
	// Loop through all of the worker's targets and see if there's a match.
	target_count = TargetCount();
	for (i = 0; i < target_count; i++) {
		// Currently only a small subset of the target information is compared.
		if (IsType(target_info->type, GenericDiskType)) {
			// Look for a disk with the same name.
			if (!strcmp(target_info->name, GetTarget(i)->spec.name))
				return TRUE;
		} else if (IsType(target_info->type, GenericNetType)) {
			// Look for a network target with the given remote address.
			if (!strcmp(target_info->name, GetTarget(i)->spec.tcp_info.remote_address)) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

//
// Adding a new target to the end of the worker's list.
//
void Worker::AddTarget(Target_Spec * target_info)
{
	Target *target;

	// Create a new target, copy the specified information, and 
	// initialize it.
	target = new Target;
	memcpy(&(target->spec), target_info, sizeof(Target_Spec));
	memset(&(target->results[WHOLE_TEST_PERF]), 0, sizeof(Results));
	memset(&(target->results[LAST_UPDATE_PERF]), 0, sizeof(Results));

	// Use the worker's default settings for the target.
	target->spec.queue_depth = spec.queue_depth;
	target->spec.random = spec.random;
	target->spec.test_connection_rate = spec.test_connection_rate;
	target->spec.trans_per_conn = spec.trans_per_conn;

	// Copy default settings specific to the target's type.
	if (IsType(target->spec.type, GenericDiskType)) {
		memcpy(&target->spec.disk_info, &spec.disk_info, sizeof(Disk_Spec));
		target->spec.DataPattern = spec.DataPattern;
	}
	else if (IsType(target->spec.type, VIClientType))
		target->spec.vi_info.outstanding_ios = spec.vi_info.outstanding_ios;
	else if (!IsType(target->spec.type, GenericClientType)) {
		ErrorMessage("Invalid target type in Worker::AddTarget().");
		delete target;

		target = NULL;
		return;
	}

	targets.Add(target);
}

//
// Removing the specified target from the worker's list.
//
void Worker::RemoveTarget(int index)
{
	if (index < 0 || index >= TargetCount()) {
		ErrorMessage("Invalid index in Worker::RemoveTarget().");
		return;
	}
	// Remove associated network clients along with target.
	if (IsType(targets[index]->spec.type, GenericClientType)) {
		if (net_partner) {
			net_partner->net_partner = NULL;
			theApp.pView->m_pWorkerView->RemoveWorker(net_partner);
			net_partner = NULL;
		}
	}
	delete GetTarget(index);

	targets.RemoveAt(index);
}

//
// Removing all targets of the specified type.
//
void Worker::RemoveTargets(TargetType target_type)
{
	// Walk through the target array backwards and remove all targets of the
	// given type.  We move backwards to avoid jumping over targets when some
	// get removed.
	for (int i = TargetCount() - 1; i >= 0; i--) {
		if (IsType(targets[i]->spec.type, target_type))
			RemoveTarget(i);
	}
}

//
// Returns if a worker is ready to start a test or if additional work needs
// to be done, such as preparing a logical drive for access.
//
BOOL Worker::ReadyToRunTests()
{
	// Ensure that all targets are ready to run.
	for (int i = 0; i < TargetCount(); i++) {
		if (!TargetReadyToRunTests(GetTarget(i)))
			return FALSE;
	}
	return TRUE;
}

//
// Returns whether or not a target is ready to run a test.
//
BOOL Worker::TargetReadyToRunTests(Target * target)
{
	int i, iface_count;

	// Only logical disk drives need to be prepared.
	if (!IsType(target->spec.type, LogicalDiskType))
		return TRUE;

	// Disk targets are ready if the disk is marked ready in the manager's
	// disk list.  Search through the manager's list of drives to find
	// the corresponding drive and check if it's ready.  We search through the
	// manager's list in case another worker has prepared the same drive.
	iface_count = manager->InterfaceCount(GenericDiskType);
	for (i = 0; i < iface_count; i++) {
		if (!strcmp(target->spec.name, manager->GetInterface(i, GenericDiskType)->name))
			return manager->GetInterface(i, GenericDiskType)->disk_info.ready;
	}

	ErrorMessage("Worker's assigned disk not found in manager's list in " "Worker::TargetReadyToRunTests().");
	return FALSE;
}

//
// Resetting worker and its targets to indicate that they are not active.
//
void Worker::ClearActiveTargets()
{
	// Resetting all targets to indicate that they are not running.
	for (int i = 0; i < TargetCount(); i++) {
		targets[i]->spec.type = (TargetType)
		    (targets[i]->spec.type & ~ActiveType);
	}
}

//
// Setting a specified number of targets to indicate that they are active.
// We return the actual number of targets set active.
//
int Worker::SetActiveTargets(int targets_to_set)
{
	int i, count = 0, target_count = TargetCount();

	for (i = 0; i < target_count; i++) {
		if (targets_to_set-- > 0) {
			targets[i]->spec.type = (TargetType)
			    (targets[i]->spec.type | ActiveType);
			count++;
		} else {
			targets[i]->spec.type = (TargetType)
			    (targets[i]->spec.type & ~ActiveType);
		}
	}

	return count;
}

//
// Initializing worker results.
//
void Worker::ResetResults(int which_perf)
{
	if ((which_perf < 0) || (which_perf >= MAX_PERF))
		return;

	memset(&(results[which_perf]), 0, sizeof(Results));

	// Reset results for all targets as well.
	for (int i = 0; i < TargetCount(); i++) {
		memset(&(GetTarget(i)->results[which_perf]), 0, sizeof(Results));
	}
}

//
// Reset all results stored for an individual worker, along with any results for any 
// associated devices.
//
void Worker::ResetAllResults()
{
	// Reset worker's results.
	ResetResults(WHOLE_TEST_PERF);
	ResetResults(LAST_UPDATE_PERF);
}

//
// Return this worker's index in the manager's worker list.
//
int Worker::GetIndex(TargetType list_type)
{
	for (int i = 0; i < manager->WorkerCount(list_type); i++) {
		if (this == manager->GetWorker(i, list_type))
			return i;
	}
	return IOERROR;
}

//
// Returning information about what type of worker we're dealing with.
//
// This should change to return only those types of targets supported or
// currently assigned to a worker.  The function name should change to some-
// thing like TargetTypes().  This will avoid having to catch and switch
// between a server have client targets and vice-versa.  (I.e. a disk worker
// has disk targets, but a server worker has client targets.  This causes
// the casting from client to server types seen below.)
// Do not do this until after the IDF release. 8/19/98. - SH
TargetType Worker::Type()
{
	int i, target_count;
	TargetType type, target_type;

	// If the worker does not have any targets, use the last stored type.
	if (!(target_count = TargetCount()))
		return spec.type;

	// Otherwise return a type based on its assigned targets.
	type = GenericType;
	for (i = 0; i < target_count; i++) {
		if (IsType(targets[i]->spec.type, GenericClientType)) {
			// Mark this worker as the server by setting the target's client
			// bit to 0 and server bit to 1.
			target_type = (TargetType) ((targets[i]->spec.type &
						     ~(GenericClientType ^ GenericNetType)) | GenericServerType);

			type = (TargetType) (type | target_type);
		} else
			type = (TargetType) (type | targets[i]->spec.type);
	}

	return type;
}

//
// Sets a worker's nth access spec, where n is specified by access_entry.
// Also sets any corresponding network clients.
//
BOOL Worker::SetAccess(int access_entry)
{
	Message msg;
	Data_Message *data_msg;

	// Network clients are set by their server.
	if (IsType(Type(), GenericClientType))
		return TRUE;

	data_msg = new Data_Message;

	// If the worker has an access spec for the specified entry, use it.
	// If there is no entry, send the idle spec.
	if (access_entry < AccessSpecCount()) {
		// Copy the spec into a message and send it.
		memcpy((void *)&(data_msg->data.spec), (void *)(GetAccessSpec(access_entry)), sizeof(Test_Spec));
	} else {
		// Otherwise, send the idle spec.
		memcpy((void *)&(data_msg->data.spec),
		       (void *)(theApp.access_spec_list.Get(IDLE_SPEC)), sizeof(Test_Spec));
	}

	// Tell Dynamo to set the access spec.
	msg.purpose = SET_ACCESS;
	msg.data = GetIndex();
	manager->Send(&msg);
	manager->SendData(data_msg);
	manager->Receive(&msg);

	// Set the access spec for the corresponing client, if any.
	if (msg.data && IsType(Type(), GenericServerType) && net_partner) {
		msg.purpose = SET_ACCESS;
		msg.data = net_partner->GetIndex();
		net_partner->manager->Send(&msg);
		net_partner->manager->SendData(data_msg);
		net_partner->manager->Receive(&msg);
	}

	delete data_msg;
	return msg.data;	// msg.data indicates success.
}

//
// Sets all targets that are marked as active.  This sends the SET_TARGET
// message to Dynamo and gets the response.
//
BOOL Worker::SetTargets()
{
	Target *target;
	Message msg;
	Data_Message *data_msg;
	VI_DISCRIMINATOR_TYPE vi_discriminator;
	int vi_discriminator_length;
	VIP_NET_ADDRESS *vi_addr;
	int i, target_count;

	// Network clients are set by their servers.
	if (IsType(Type(), GenericClientType))
		return TRUE;

	data_msg = new Data_Message;

	data_msg->count = 0;

	// Loop through all targets and add them to the message if active.
	target_count = TargetCount(ActiveType);
	for (i = 0; i < target_count; i++) {
		target = GetTarget(i, ActiveType);

		// Initialize random number generator.
		// If Using a fixed seed is specified use that value, else get a timestamp to use as a seed value
		target->spec.use_fixed_seed = spec.use_fixed_seed;
		if(spec.use_fixed_seed)
			target->spec.random = spec.fixed_seed_value;
		else
			target->spec.random = timer_value();

		// Initialize unique discriminator for VI targets.
		if (IsType(target->spec.type, VIClientType)) {
			// Find the smallest of the two NICs' discriminator length and VI_DISCRIMINATOR_SIZE
			vi_discriminator_length = min(target->spec.vi_info.local_address.DiscriminatorLen,
						      target->spec.vi_info.remote_address.DiscriminatorLen);
			vi_discriminator_length = min(vi_discriminator_length, VI_DISCRIMINATOR_SIZE);

#if defined(IOMTR_CPU_I386) || defined(IOMTR_CPU_IA64) || defined(IOMTR_CPU_X86_64)
			// Intel processor (little-endian) -- use the first VI_DISCRIMINATOR_SIZE bytes 
			// (least significant bytes) of the current counter/clock
			vi_discriminator = (VI_DISCRIMINATOR_TYPE) timer_value();

			// Copy the first vi_discriminator_length bytes of the generated discriminator
			// to the VI_Spec's local_address.
			vi_addr = &target->spec.vi_info.local_address;
			vi_addr->DiscriminatorLen = vi_discriminator_length;
			memcpy(vi_addr->HostAddress + vi_addr->HostAddressLen, &vi_discriminator,
			       vi_discriminator_length);

			// Copy the first vi_discriminator_length bytes of the generated discriminator
			// to the VI_Spec's remote_address.
			vi_addr = &target->spec.vi_info.remote_address;
			vi_addr->DiscriminatorLen = vi_discriminator_length;
			memcpy(vi_addr->HostAddress + vi_addr->HostAddressLen, &vi_discriminator,
			       vi_discriminator_length);
#else
			// Non-Intel processor -- generate a random discriminator and use the first
			// or last vi_discriminator_length bytes, depending on whether the processor
			// is big-endian or little-endian.  (Tip: if htonl(1234) == 1234, it's big-endian.)
#error "Non-IA version of this code is not yet implemented."
#endif
		}
		// Copy active targets into set target data message.
		memcpy(&data_msg->data.targets[data_msg->count++], &target->spec, sizeof(Target_Spec));
	}

	// Send the message of targets to set to Dynamo and get the reply.
	manager->Send(GetIndex(), SET_TARGETS);
	manager->SendData(data_msg);

	// Reply message indicates if targets were set successfully along with
	// additional error data or target settings that can only be determined
	// by Dynamo (such as TCP port numbers).
	manager->Receive(&msg);
	manager->ReceiveData(data_msg);

	// If a network server set its targets correctly, set its client.
	// This currently assumes one client per server.
	if (msg.data && IsType(Type(), GenericServerType) && TargetCount()) {
		// Set the client's targets if we're not just resetting targets.
		if (data_msg->count) {
			data_msg->count = 1;

			if (IsType(targets[0]->spec.type, TCPClientType)) {
				// Record port used by server
				targets[0]->spec.tcp_info.local_port = data_msg->data.targets[0].tcp_info.local_port;

				// Set the server's client to use the server as a target.
				memcpy(&data_msg->data.targets[0], &targets[0]->spec, sizeof(Target_Spec));

				// Set the client's remote port to the server's local port
				data_msg->data.targets[0].tcp_info.remote_port = targets[0]->spec.tcp_info.local_port;

				// Reverse the local and remote addresses for the client.
				// The *server* is a client's target.
				strcpy(data_msg->data.targets[0].name, targets[0]->spec.tcp_info.remote_address);
				strcpy(data_msg->data.targets[0].tcp_info.remote_address, targets[0]->spec.name);
				data_msg->data.targets[0].type = TCPServerType;
			} else if (IsType(targets[0]->spec.type, VIClientType)) {
				// Set the server's client to use the server as the target.
				memcpy(&data_msg->data.targets[0], &targets[0]->spec, sizeof(Target_Spec));

				// Reverse the local and remote addresses for the client.
				// The *server* is a client's target.
				// Set client's local VI NIC to use.
				strcpy(data_msg->data.targets[0].name, target->spec.vi_info.remote_nic_name);
				// Set address that client should connect to on remote side.
				memcpy(&data_msg->data.targets[0].vi_info.remote_address,
				       &targets[0]->spec.vi_info.local_address, VI_ADDRESS_SIZE);
				memcpy(&data_msg->data.targets[0].vi_info.local_address,
				       &targets[0]->spec.vi_info.remote_address, VI_ADDRESS_SIZE);
				data_msg->data.targets[0].type = VIServerType;
			} else {
				ErrorMessage("Unsupported client target type in Worker::" "SetTargets().");
				return FALSE;
			}
		}
		// Send the message of targets to set to Dynamo and get the reply.
		net_partner->manager->Send(net_partner->GetIndex(), SET_TARGETS);
		net_partner->manager->SendData(data_msg);

		// Reply message indicates if targets were set successfully.
		net_partner->manager->Receive(&msg);
		net_partner->manager->ReceiveData(data_msg);
	}

	delete data_msg;

	return (msg.data);
}

//
// Checks all targets to make sure that they're prepared to run, and marks those
// that need to be as active.  If any need to be prepared, it sends a message
// to Dynamo and begins preparing them.
//
void Worker::SetTargetsToPrepare()
{
	int i, target_count = TargetCount();

	// Find which targets need to be prepared and mark them.
	for (i = 0; i < target_count; i++) {
		if (TargetReadyToRunTests(targets[i])) {
			targets[i]->spec.type = (TargetType)
			    (targets[i]->spec.type & ~ActiveType);
		} else {
			// Set targets needing to be prepared as active.
			targets[i]->spec.type = (TargetType)
			    (targets[i]->spec.type | ActiveType);
		}
	}

	// Send a message to Dynamo to set the targets needing preparation.
	theApp.test_state = TestPreparing;
	SetTargets();
	manager->Send(GetIndex(), PREP_DISKS);
}

//
// Saving the results for the worker and all its targets.
//
void Worker::SaveResults(ostream * file, int access_index, int result_type)
{
	int i, stat, target_count;
	Target *target;

	// Saving combined results.
	if (!ActiveInCurrentTest())
		return;

	// Writing results for worker.
	(*file) << "WORKER" << "," << name << "," << GetAccessSpec(access_index)->name << ",,";	// space for managers running and workers running.

	if (IsType(Type(), GenericClientType))
		(*file) << "," << 1;	// currently only 1 target (server) per client
	else
		(*file) << "," << TargetCount(ActiveType);

	(*file) << "," << results[WHOLE_TEST_PERF].IOps
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
	    // Writing raw result information for completed I/Os.
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
		(*file) << ",";	// Space for CPU utilization

	(*file) << "," <<  manager->timer_resolution << ",,"; // Space for IRQ/sec, CPU_effectiveness

	for (stat = 0; stat < NI_COMBINE_RESULTS + TCP_RESULTS; stat++) {
		(*file) << ",";	// Space for network results
	}

	(*file) << endl;

	// If requested, save target results.
	if (result_type != RecordAll)
		return;

	// Saving results for all targets.
	if (IsType(Type(), GenericClientType)) {
		// Network clients have their server as their target, but do not have
		// any other targets.  Eventually, a client may be able to be a client
		// as well as a server, disk worker, etc.  This will require changes
		// in how a client's results are gathered and stored, until then, the
		// following code will work-around the issue by assuming that a client
		// has only a single server target.
		if (!(target = new Target)) {
			ErrorMessage("Unable to allocate memory saving results.");
			return;
		}
		target_count = 1;
		memcpy(&target->results[WHOLE_TEST_PERF], &results[WHOLE_TEST_PERF], sizeof(Results));
		target->spec.type = Type();

		// Store local and remote addresses.
		if (IsType(target->spec.type, TCPClientType)) {
			strcpy(target->spec.name,
			       net_partner->GetTarget(net_target_index)->spec.tcp_info.remote_address);
			strcpy(target->spec.tcp_info.remote_address,
			       net_partner->GetTarget(net_target_index)->spec.name);
		} else {
			strcpy(target->spec.name,
			       net_partner->GetTarget(net_target_index)->spec.vi_info.remote_nic_name);
			strcpy(target->spec.vi_info.remote_nic_name,
			       net_partner->GetTarget(net_target_index)->spec.name);
		}
	} else {
		target_count = TargetCount();
	}

	for (i = 0; i < target_count; i++) {
		char PrintName[MAX_NAME], *TheOffendingChar;

		if (!IsType(Type(), GenericClientType))
			target = GetTarget(i);
		if (!IsType(target->spec.type, ActiveType))
			continue;

		strcpy(PrintName, target->spec.name);
		TheOffendingChar = PrintName;
		// convert commas, newlines and cariage returns to spaces so these don't break the .csv output
		while ((TheOffendingChar = strpbrk(TheOffendingChar, "\n\r,")) != NULL)
			*TheOffendingChar = ' ';

		// Retrieving results for a single target in order to save it to a file.
		if (IsType(target->spec.type, GenericDiskType)) {
			(*file) << "DISK" << "," << PrintName;
		} else if (IsType(target->spec.type, TCPClientType)) {
			// Show name as local address >> remote address.
			(*file) << "NETWORK" << "," << manager->name << ":"
			    << PrintName << " >> "
			    << net_partner->manager->name << ":" << target->spec.tcp_info.remote_address;
		} else if (IsType(target->spec.type, VIClientType)) {
			// Show name as local address >> remote address.
			(*file) << "NETWORK" << "," << manager->name << ":"
			    << PrintName << " >> "
			    << net_partner->manager->name << ":" << target->spec.vi_info.remote_nic_name;
		} else {
			(*file) << "UNKNOWN" << "," << PrintName;
		}

		(*file) << ",,,,"	// space for access spec name, workers, managers and targets running.
		    << "," << target->results[WHOLE_TEST_PERF].IOps
		    << "," << target->results[WHOLE_TEST_PERF].read_IOps
		    << "," << target->results[WHOLE_TEST_PERF].write_IOps
		    << "," << target->results[WHOLE_TEST_PERF].MBps_Bin
		    << "," << target->results[WHOLE_TEST_PERF].read_MBps_Bin
		    << "," << target->results[WHOLE_TEST_PERF].write_MBps_Bin
		    << "," << target->results[WHOLE_TEST_PERF].MBps_Dec
		    << "," << target->results[WHOLE_TEST_PERF].read_MBps_Dec
		    << "," << target->results[WHOLE_TEST_PERF].write_MBps_Dec
		    << "," << target->results[WHOLE_TEST_PERF].transactions_per_second
		    << "," << target->results[WHOLE_TEST_PERF].connections_per_second
		    << "," << target->results[WHOLE_TEST_PERF].ave_latency
		    << "," << target->results[WHOLE_TEST_PERF].ave_read_latency
		    << "," << target->results[WHOLE_TEST_PERF].ave_write_latency
		    << "," << target->results[WHOLE_TEST_PERF].ave_transaction_latency
		    << "," << target->results[WHOLE_TEST_PERF].ave_connection_latency
		    << "," << target->results[WHOLE_TEST_PERF].max_latency
		    << "," << target->results[WHOLE_TEST_PERF].max_read_latency
		    << "," << target->results[WHOLE_TEST_PERF].max_write_latency
		    << "," << target->results[WHOLE_TEST_PERF].max_transaction_latency
		    << "," << target->results[WHOLE_TEST_PERF].max_connection_latency
		    << "," << target->results[WHOLE_TEST_PERF].total_errors
		    << "," << target->results[WHOLE_TEST_PERF].raw.read_errors
		    << "," << target->results[WHOLE_TEST_PERF].raw.write_errors
		    << "," << target->results[WHOLE_TEST_PERF].raw.bytes_read
		    << "," << target->results[WHOLE_TEST_PERF].raw.bytes_written
		    << "," << target->results[WHOLE_TEST_PERF].raw.read_count
		    << "," << target->results[WHOLE_TEST_PERF].raw.write_count
		    << "," << target->results[WHOLE_TEST_PERF].raw.connection_count << ",";

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
			(*file) << ",";	// Space for CPU utilization

		(*file) << "," << manager->timer_resolution << ",,"; // Space for IRQ/sec and CPU_effectiveness.		

		for (stat = 0; stat < NI_COMBINE_RESULTS + TCP_RESULTS; stat++)
			(*file) << ",";	// Space for network results

		(*file) << endl;
	}
	if (IsType(Type(), GenericClientType))
		delete target;
}

//
// Updating the workers results for the last test.  This functions performs
// some heavy duty calculations.  Enter at your own risk.
//
void Worker::UpdateResults(int which_perf)
{
	Data_Message *data_msg;
	double run_time;
	Worker_Results *new_wkr_results;
	Results *device_results;	// Results for a specific target.
	Raw_Result *raw;	// Raw results stored for the worker.
	Raw_Result *raw_device_results;	// Raw results returned for a single device.
	double timer_resolution;

	if ((which_perf < 0) || (which_perf >= MAX_PERF))
		return;

	data_msg = new Data_Message;

	// Initializing worker's results.
	ResetResults(which_perf);
	timer_resolution = manager->timer_resolution;

	// Receive the update from Dynamo.  The manager should have already made
	// the request.
	if (manager->ReceiveData(data_msg) == PORT_ERROR)
		return;

	new_wkr_results = &(data_msg->data.worker_results);
	raw = &(results[which_perf].raw);
	raw->counter_time = new_wkr_results->time[LAST_SNAPSHOT] - new_wkr_results->time[FIRST_SNAPSHOT];
	
	run_time = ((double)raw->counter_time) / timer_resolution;

	// Network clients do not *have* targets, but rather, *are* the targets.
	if (IsType(Type(), GenericClientType))
		device_results = new Results;

	// Updating device results for worker reporting results.
	for (int i = 0; i < new_wkr_results->target_results.count; i++) {
		if (!IsType(Type(), GenericClientType))
			device_results = &(GetTarget(i)->results[which_perf]);

		raw_device_results = &(new_wkr_results->target_results.result[i]);

		// Copy reported raw results to results stored for the device.
		memcpy((void *)&(device_results->raw), raw_device_results, sizeof(Raw_Result));

		//
		// Updating error results.
		//
		// Recording errors which have occurred to the device.
		device_results->total_errors = raw_device_results->read_errors + raw_device_results->write_errors;

		// Updating recorded results for worker.
		raw->read_errors += raw_device_results->read_errors;
		raw->write_errors += raw_device_results->write_errors;
		results[which_perf].total_errors += device_results->total_errors;

		//
		// Updating maximum latency information.
		//
		// Determining maximum latencies of device.
		device_results->max_read_latency = ((double)(_int64)
						    raw_device_results->max_raw_read_latency) * (double)1000 / timer_resolution;

		device_results->max_write_latency = ((double)(_int64)
						     raw_device_results->max_raw_write_latency) * (double)1000 / timer_resolution;

		device_results->max_transaction_latency = ((double)(_int64)
							   raw_device_results->max_raw_transaction_latency) * (double)1000 / timer_resolution;

		device_results->max_connection_latency = ((double)(_int64)
							  raw_device_results->max_raw_connection_latency) * (double)1000 / timer_resolution;

		if (device_results->max_read_latency > device_results->max_write_latency) {
			device_results->max_latency = device_results->max_read_latency;
		} else {
			device_results->max_latency = device_results->max_write_latency;
		}

		// Determining maximum latencies of worker.
		if (device_results->max_read_latency > results[which_perf].max_read_latency) {
			results[which_perf].max_read_latency = device_results->max_read_latency;
			raw->max_raw_read_latency = raw_device_results->max_raw_read_latency;
		}
		if (device_results->max_write_latency > results[which_perf].max_write_latency) {
			results[which_perf].max_write_latency = device_results->max_write_latency;
			raw->max_raw_write_latency = raw_device_results->max_raw_write_latency;
		}
		if (device_results->max_transaction_latency > results[which_perf].max_transaction_latency) {
			results[which_perf].max_transaction_latency = device_results->max_transaction_latency;
			raw->max_raw_transaction_latency = raw_device_results->max_raw_transaction_latency;
		}
		if (device_results->max_latency > results[which_perf].max_latency)
			results[which_perf].max_latency = device_results->max_latency;
		if (device_results->max_connection_latency > results[which_perf].max_connection_latency) {
			results[which_perf].max_connection_latency = device_results->max_connection_latency;
			raw->max_raw_connection_latency = raw_device_results->max_raw_connection_latency;
		}
		//
		// Updating throughput data.
		//
		// Calculating MB/s and IO/s data rates.
		if (run_time) {
			// Calculating results on a per drive basis.
			device_results->read_MBps_Bin = ((double)(_int64)
						     raw_device_results->bytes_read / (double)MEGABYTE_BIN) / run_time;
			device_results->write_MBps_Bin = ((double)(_int64)
						      raw_device_results->bytes_written / (double)MEGABYTE_BIN) / run_time;
			device_results->MBps_Bin = device_results->read_MBps_Bin + device_results->write_MBps_Bin;
			device_results->read_MBps_Dec = ((double)(_int64)
						     raw_device_results->bytes_read / (double)MEGABYTE_DEC) / run_time;
			device_results->write_MBps_Dec = ((double)(_int64)
						      raw_device_results->bytes_written / (double)MEGABYTE_DEC) / run_time;
			device_results->MBps_Dec = device_results->read_MBps_Dec + device_results->write_MBps_Dec;
			device_results->read_IOps = ((double)(_int64)
						     raw_device_results->read_count) / run_time;
			device_results->write_IOps = ((double)(_int64)
						      raw_device_results->write_count) / run_time;
			device_results->IOps = device_results->read_IOps + device_results->write_IOps;
			device_results->transactions_per_second = ((double)(_int64)
								   raw_device_results->transaction_count) / run_time;
			device_results->connections_per_second = ((double)(_int64)
								  raw_device_results->connection_count) / run_time;

			// Updating results for the worker based on the results reported for individual drives.

			// Raw results.
			raw->bytes_read += raw_device_results->bytes_read;
			raw->bytes_written += raw_device_results->bytes_written;
			raw->read_count += raw_device_results->read_count;
			raw->write_count += raw_device_results->write_count;
			raw->connection_count += raw_device_results->connection_count;
			raw->transaction_count += raw_device_results->transaction_count;

			// Calculated results.
			results[which_perf].MBps_Bin += device_results->MBps_Bin;
			results[which_perf].read_MBps_Bin += device_results->read_MBps_Bin;
			results[which_perf].write_MBps_Bin += device_results->write_MBps_Bin;
			results[which_perf].MBps_Dec += device_results->MBps_Dec;
			results[which_perf].read_MBps_Dec += device_results->read_MBps_Dec;
			results[which_perf].write_MBps_Dec += device_results->write_MBps_Dec;
			results[which_perf].IOps += device_results->IOps;
			results[which_perf].read_IOps += device_results->read_IOps;
			results[which_perf].write_IOps += device_results->write_IOps;
			results[which_perf].transactions_per_second += device_results->transactions_per_second;
			results[which_perf].connections_per_second += device_results->connections_per_second;
		} else {
			device_results->MBps_Bin = (double)0;
			device_results->read_MBps_Bin = (double)0;
			device_results->write_MBps_Bin = (double)0;
			device_results->MBps_Dec = (double)0;
			device_results->read_MBps_Dec = (double)0;
			device_results->write_MBps_Dec = (double)0;
			device_results->IOps = (double)0;
			device_results->read_IOps = (double)0;
			device_results->write_IOps = (double)0;
			device_results->transactions_per_second = (double)0;
			device_results->connections_per_second = (double)0;
		}

		// Determining average latencies of transfers to a single drive.
		if (raw_device_results->read_count || raw_device_results->write_count) {
			device_results->ave_latency = ((double)(_int64)
				(raw_device_results->read_latency_sum +
				raw_device_results->write_latency_sum)) * (double)1000 / timer_resolution 
				/ (double)(_int64)(raw_device_results->read_count + raw_device_results->write_count);

			if (raw_device_results->read_count) {
				device_results->ave_read_latency = ((double)(_int64)
					raw_device_results->read_latency_sum) * (double)1000 / timer_resolution
					/ (double)(_int64)raw_device_results->read_count;
				raw->read_latency_sum += raw_device_results->read_latency_sum;
			} else {
				device_results->ave_read_latency = (double)0;
			}

			if (raw_device_results->write_count) {
				device_results->ave_write_latency = ((double)(_int64)
					raw_device_results->write_latency_sum) * (double)1000 / timer_resolution
					/ (double)(_int64)raw_device_results->write_count;
				raw->write_latency_sum += raw_device_results->write_latency_sum;
			} else {
				device_results->ave_write_latency = (double)0;
			}

			if (raw_device_results->transaction_count) {
				device_results->ave_transaction_latency = ((double)(_int64)
					raw_device_results->transaction_latency_sum) * (double) 1000 / 
					timer_resolution / (double)(_int64)(raw_device_results->transaction_count);
				raw->transaction_latency_sum += raw_device_results->transaction_latency_sum;
			} else {
				device_results->ave_transaction_latency = (double)0;
			}
		} else {
			device_results->ave_latency = (double)0;
			device_results->ave_read_latency = (double)0;
			device_results->ave_write_latency = (double)0;
			device_results->ave_transaction_latency = (double)0;
		}

		// Determining the average connection time for each drive.
		if (raw_device_results->connection_count) {
			// Calculate the average connection time.
			device_results->ave_connection_latency = ((double)(_int64)
				(raw_device_results->connection_latency_sum)) * (double)1000 / 
				timer_resolution / (double)(_int64)(raw_device_results->connection_count);
			raw->connection_latency_sum += raw_device_results->connection_latency_sum;
		} else {
			device_results->ave_connection_latency = (double)0;
		}
	}

	// Calculating average latencies for the worker.
	if (raw->read_count || raw->write_count) {
		results[which_perf].ave_latency = (double)(_int64) (raw->read_latency_sum +
			raw->write_latency_sum) * (double)1000 / timer_resolution
			/ (double)(_int64) (raw->read_count + raw->write_count);

		if (raw->read_count) {
			results[which_perf].ave_read_latency = (double)(_int64)
			    raw->read_latency_sum * (double)1000 / timer_resolution
				/ (double)(_int64) raw->read_count;
		} else {
			results[which_perf].ave_read_latency = (double)0;
		}

		if (raw->write_count) {
			results[which_perf].ave_write_latency = (double)(_int64)
			    raw->write_latency_sum * (double)1000 / timer_resolution
				/ (double)(_int64) raw->write_count;
		} else {
			results[which_perf].ave_write_latency = (double)0;
		}

		if (raw->transaction_count) {
			results[which_perf].ave_transaction_latency = (double)(_int64)
			    raw->transaction_latency_sum * (double)1000 / timer_resolution
			    / (double)(_int64) (raw->transaction_count);
		} else {
			results[which_perf].ave_transaction_latency = (double)0;
		}
	} else {
		results[which_perf].ave_latency = (double)0;
		results[which_perf].ave_read_latency = (double)0;
		results[which_perf].ave_write_latency = (double)0;
		results[which_perf].ave_transaction_latency = (double)0;
	}

	// Calculating taverage connection time for the worker.
	if (raw->connection_count) {
		results[which_perf].ave_connection_latency = (double)(_int64)
		    raw->connection_latency_sum * (double)1000 /
			timer_resolution / (double)(_int64) raw->connection_count;
	} else {
		results[which_perf].ave_connection_latency = (double)0;
	}

	if (IsType(Type(), GenericClientType))
		delete device_results;

	delete data_msg;
}

//
// Create a network partner (client) for me on the specified manager.  
// Store a pointer to the newly-created worker in my net_partner member 
// variable. If there is already a network partner, remove it unless the 
// requested network partner is on the same manager.
//
void Worker::CreateNetClient(Manager * target_manager, TargetType client_type)
{
	if (!target_manager) {
		ErrorMessage("Invalid parameter in Worker::CreateNetPartner()");
		return;
	}
	// Create the client worker on the target manager (net_partner)
	if (!(net_partner = target_manager->AddWorker(client_type))) {
		ErrorMessage("Could not create new worker in Worker::" "CreateNetPartner().");
		return;
	}
	// Record that I am the client's net_partner and what target it is.
	net_partner->net_partner = this;
	net_partner->net_target_index = 0;
	snprintf(net_partner->name, MAX_WORKER_NAME, "[%s]", name);

	// add the new worker to the display - this should be handled by the GUI
	theApp.pView->m_pWorkerView->AddWorker(net_partner);

	// refresh the result display - this should be handled by the GUI
	theApp.manager_list.ResetAllResults();
	theApp.pView->m_pPageDisplay->Reset();
	theApp.pView->ResetDisplayforNewTest();
}

////////////////////////////////////////////////////////////////////////////////////
//
// The following functions update the values for the starting sector, the number of
// sectors to access, the queue depth, and the disk selection.
// On the ManagerList level, it propagates down the tree to all the managers and
// calls the managers function.
//
////////////////////////////////////////////////////////////////////////////////////

//
// Changes the specified value for all targets matching the given type.
//
// Not yet implemented.  Wait to add after IDF 8/19/98 - SH
/*
void Worker::SetValue( TargetType type, int offset, int value )
{
	int i, target_count;

	// Currently all workers have the same type of targets.  Make sure that the
	// worker supports the type of targets being modified.
	if ( (Type() & type) != type )
		return;

	// Record the value directly into the worker's specifications.
	*(int*)(&spec + offset) = value;

	// Record the value for all targets matching the given type.
	target_count = TargetCount( type );
	for ( i = 0; i < target_count; i++ )
	{
		if ( (GetTarget(i, type)->spec.type & type) == type )
			*(int*)(&(GetTarget(i, type)->spec) + offset) = value;
	}
}
*/

void Worker::SetDiskSize(DWORDLONG disk_size)
{
	int i, target_count;

	spec.disk_info.maximum_size = disk_size;

	// Loop through all the worker's disks.
	target_count = TargetCount(GenericDiskType);
	for (i = 0; i < target_count; i++) {
		GetTarget(i, GenericDiskType)->spec.disk_info.maximum_size = disk_size;
	}
}

void Worker::SetDiskStart(DWORDLONG disk_start)
{
	int i, target_count;

	spec.disk_info.starting_sector = disk_start;

	// Loop through all the worker's disks.
	target_count = TargetCount(GenericDiskType);
	for (i = 0; i < target_count; i++) {
		GetTarget(i, GenericDiskType)->spec.disk_info.starting_sector = disk_start;
	}
}

//
// Sets the queue depth for the worker and all of its targets.
//
void Worker::SetQueueDepth(int queue_depth)
{
	int i, target_count;

	// Do not change the queue depth for network workers.  Dynamo does not
	// support this.
	if (!IsType(Type(), GenericDiskType))
		return;

	// Set the workers queue depth.  We need to do this in case the worker
	// doesn't have any targets currently assigned.
	spec.queue_depth = queue_depth;

	if (!(target_count = TargetCount(GenericDiskType)))
		return;

	// Set all of the worker's targets.
	for (i = 0; i < target_count; i++)
		GetTarget(i, GenericDiskType)->spec.queue_depth = queue_depth;
}

//
// Sets the maximum number of sends for the worker and all of its VI targets.
//
void Worker::SetMaxSends(int max_sends)
{
	int i, target_count;

	// Loop through all the worker's targets.
	target_count = TargetCount(GenericVIType);
	for (i = 0; i < target_count; i++) {
		GetTarget(i, GenericVIType)->spec.vi_info.outstanding_ios = max_sends;
	}

	spec.vi_info.outstanding_ios = max_sends;
}

//
// Sets the local address used by network target connections.  It takes as
// input an index into the worker's manager's interface list.
//
void Worker::SetLocalNetworkInterface(int iface_index, TargetType type)
{
	Target_Spec *iface;
	Target *target;
	int i;

	// Only set the address for server workers.  This is not an error.
	if (!IsType(Type(), GenericServerType))
		return;

	// Get the new local interface.
	iface = manager->GetInterface(iface_index, type);

	// Record what type of worker this is now in case it doesn't have any
	// targets or they get deleted.
	switch (iface->type) {
	case TCPClientType:
		spec.type = TCPServerType;
		break;
	case VIClientType:
		// If we're switching types, reset the number of oustanding IOs
		if (!IsType(spec.type, GenericVIType))
			spec.vi_info.outstanding_ios = 1;
		spec.type = VIServerType;
		break;
	}

	// Update all targets to be accessed through the specified local interface.
	// We move backwards to avoid jumping over targets if some get removed.
	for (i = TargetCount() - 1; i >= 0; i--) {
		target = GetTarget(i);

		// If the target cannot be accessed through the selected interface,
		// remove it as an active target.
		if (!IsType(target->spec.type, iface->type)) {
			RemoveTarget(i);
			continue;
		}
		// Update the target information.
		if (IsType(target->spec.type, TCPClientType)) {
			// TCP servers only need to copy the address of the interface that
			// will be used.
			strcpy(target->spec.name, iface->name);
		} else if (IsType(target->spec.type, VIClientType)) {
			// VI servers need the NIC name and network address needed to
			// make the connection.
			strcpy(target->spec.name, iface->name);
			memcpy(&target->spec.vi_info.local_address, &iface->vi_info.local_address, VI_ADDRESS_SIZE);
			target->spec.vi_info.outstanding_ios = spec.vi_info.outstanding_ios;
		}
	}

	strcpy(spec.name, iface->name);
}

//
// Sets the local address used by network target connections.  It takes as
// input a CString which identifies which network interface is to be used.
// (FUNCTION OVERLOAD)
//
void Worker::SetLocalNetworkInterface(const CString & iface_name)
{
	int nic_total, nic_counter;

	// Only set the address for server workers.  This is not an error.
	if (!IsType(Type(), GenericServerType))
		return;

	nic_total = manager->InterfaceCount(GenericNetType);

	// Find the index of the NIC on the manager with the given name.
	for (nic_counter = 0; nic_counter < nic_total; nic_counter++) {
		if (iface_name.CompareNoCase(manager->GetInterface(nic_counter, GenericNetType)->name) == 0) {
			break;
		}
	}

	// If we went past the end and found no match.  This is an error.
	if (nic_counter == nic_total) {
		ErrorMessage("Could not find local network interface \"" + iface_name
			     + "\" on worker " + (CString) name + ".  Worker::SetLocalNetworkInterface() failed.");
		return;
	}
	// Set the worker to use the specified local
	// NIC to communicate with the remote target.
	SetLocalNetworkInterface(nic_counter);
}

// Sets whether the targets will stay open for the duration of the test or 
// open and close after trans_per_conn transactions.  If test_connection_rate
// is TRUE, it means that the target will NOT stay open.
void Worker::SetConnectionRate(BOOL test_connection_rate)
{
	int i, target_count;

	spec.test_connection_rate = test_connection_rate;

	// Loop through all the worker's targets.
	target_count = TargetCount();
	for (i = 0; i < target_count; i++)
		GetTarget(i)->spec.test_connection_rate = test_connection_rate;
}

void Worker::SetDataPattern(int data_pattern)
{
	int i, target_count;

	spec.DataPattern = data_pattern;

	// Loop through all the worker's targets.
	target_count = TargetCount(GenericDiskType);
	for (i = 0; i < target_count; i++)
		GetTarget(i)->spec.DataPattern = data_pattern;
}

void Worker::SetTransPerConn(int trans_per_conn)
{
	int i, target_count;

	spec.trans_per_conn = trans_per_conn;

	// Loop through all the worker's targets.
	target_count = TargetCount();
	for (i = 0; i < target_count; i++)
		GetTarget(i)->spec.trans_per_conn = trans_per_conn;
}

// Sets whether to use Fixed RNG Seed Values for the worker. 
// If use_fixed_seed is TRUE, it means that a fixed seed will be
// used.  If FALSE, the default RNG seed will be used.
void Worker::SetUseFixedSeed(BOOL use_fixed_seed)
{
	int i, target_count;

	spec.use_fixed_seed = use_fixed_seed;

	// Loop through all the worker's targets.
	target_count = TargetCount();
	for (i = 0; i < target_count; i++)
		GetTarget(i)->spec.use_fixed_seed = use_fixed_seed;
}

void Worker::SetFixedSeedValue(DWORDLONG fixed_seed_value)
{
	int i, target_count;

	spec.fixed_seed_value = fixed_seed_value;

	// Loop through all the worker's targets.
	target_count = TargetCount();
	for (i = 0; i < target_count; i++)
		GetTarget(i)->spec.fixed_seed_value = fixed_seed_value;
}

///////////////////////////////////////////////
//
// Functions to retrieve worker information
//
// Currently these functions assume that all targets are of the same type
// and have the same values.
//
///////////////////////////////////////////////

//
// Get a stored value from the specified offset into the Target_Spec.  Return
// the value if all similar targets have the same value, otherwise return an
// ambiguous value.
//
// Not yet implemented.  Wait to add after IDF 8/19/98 - SH
/*
int Worker::GetValue( TargetType type, int offset )
{
	int i, test_value, target_count;

	// See if there's any targets of the specified type.
	if ( (target_count = TargetCount( type )) )
		return AMBIGUOUS_VALUE;

	// Find the first target matching the given type.
	for ( i = 0; i < target_count; i++ )
	{
		if ( (GetTarget(i, type)->spec.type & type) == type )
		{
			test_value = *(int*)(&(GetTarget(i, type)->spec) + offset);
			break;
		}
	}

	// Ensure that all other workers have the same value.
	for ( ++i; i < target_count; i++ )
	{
		if ( test_value != *(int*)(&(GetTarget(i, type)->spec) + offset) )
			return AMBIGUOUS_VALUE;
	}
	return test_value;
}
*/

int Worker::GetConnectionRate(TargetType type)
{
	if (IsType(Type(), GenericClientType))
		return net_partner->GetConnectionRate(type);

	if (!IsType(Type(), type))
		return AMBIGUOUS_VALUE;

	// Assume that all targets have the same value.
	if (spec.test_connection_rate)
		return ENABLED_VALUE;
	else
		return DISABLED_VALUE;
}

BOOL Worker::GetDataPattern(TargetType type)
{
	if (IsType(Type(), GenericClientType))
		return net_partner->GetDataPattern(type);

	return spec.DataPattern;
}

int Worker::GetUseFixedSeed(TargetType type)
{
	if (IsType(Type(), GenericClientType))
		return net_partner->GetUseFixedSeed(type);

	if (!IsType(Type(), type))
		return AMBIGUOUS_VALUE;

	// Assume that all targets have the same value.
	if (spec.use_fixed_seed)
		return ENABLED_VALUE;
	else
		return DISABLED_VALUE;
}

DWORDLONG Worker::GetFixedSeedValue(TargetType type)
{
	if (IsType(Type(), GenericClientType))
		return net_partner->GetFixedSeedValue(type);

	if (!IsType(Type(), type))
		return AMBIGUOUS_VALUE;

	// Assume all targets have the same value.
	return spec.fixed_seed_value;
}

DWORDLONG Worker::GetDiskStart(TargetType type)
{
	if (!IsType(Type(), type))
		return AMBIGUOUS_VALUE;

	// Assume that all targets have the same value.
	return spec.disk_info.starting_sector;
}

DWORDLONG Worker::GetDiskSize(TargetType type)
{
	if (!IsType(Type(), type))
		return AMBIGUOUS_VALUE;

	// Assume that all targets have the same value.
	return spec.disk_info.maximum_size;
}

char *Worker::GetLocalNetworkInterface()
{
	if (IsType(Type(), GenericServerType)) {
		return spec.name;
	} else if (IsType(Type(), GenericClientType)) {
		if (IsType(Type(), GenericVIType)) {
			return net_partner->GetTarget(net_target_index)->spec.vi_info.remote_nic_name;
		} else {
			return net_partner->GetTarget(net_target_index)->spec.tcp_info.remote_address;
		}
	} else {
		return NULL;
	}
}

//
// Return a valid queue depth if all targets of the specified type
// have the same one.
//
int Worker::GetQueueDepth(TargetType type)
{
	if (IsType(Type(), GenericClientType))
		return net_partner->GetQueueDepth(type);

	if (!IsType(Type(), type))
		return AMBIGUOUS_VALUE;

	// Assume all targets have the same value.
	return spec.queue_depth;
}

//
// Return a valid maximum number of outstanding sends if all targets of the 
// specified type have the same one.
//
int Worker::GetMaxSends(TargetType type)
{
	if (IsType(Type(), GenericClientType))
		return net_partner->GetMaxSends(type);

	if (!IsType(Type(), type))
		return AMBIGUOUS_VALUE;

	// Assume all targets have the same value.
	return spec.vi_info.outstanding_ios;
}

int Worker::GetTransPerConn(TargetType type)
{
	if (IsType(Type(), GenericClientType))
		return net_partner->GetTransPerConn(type);

	if (!IsType(Type(), type))
		return AMBIGUOUS_VALUE;

	// Assume all targets have the same value.
	return spec.trans_per_conn;
}

//
// Return the maximum number of I/Os that can be outstanding to a single
// target.
//
int Worker::MaxOutstandingIOs(int target_index)
{
	// The number of outstanding I/Os by a client is controlled by its server.
	if (IsType(Type(), GenericClientType) && !target_index)
		return net_partner->MaxOutstandingIOs(net_target_index);

	if (target_index < 0 || target_index >= TargetCount()) {
		ErrorMessage("Invalid target index in Worker::MaxOutstandingIOs().");
		return 0;
	}

	if (IsType(targets[target_index]->spec.type, GenericVIType)) {
		// We need to support the requested number of outstanding sends
		// (these will result in pre-posted receives) + 1 send descriptor 
		// per requested queue depth + 2 control descriptors.
		// See Dynamo - TargetVI for additional details.
		return (targets[target_index]->spec.queue_depth *
			(targets[target_index]->spec.vi_info.outstanding_ios + 1) + 2);
	} else {
		return targets[target_index]->spec.queue_depth;
	}
}

//
// Return the maximum transfer size that this worker will request from any of
// its assigned access specs.
//
DWORD Worker::MaxTransferSize()
{
	int a, s, spec_count;
	DWORD max_size = 0;
	Access_Spec *spec;

	if (IsType(Type(), GenericClientType))
		return net_partner->MaxTransferSize();

	// Loop through all access specs to find the one with the largest transfer
	// size.
	spec_count = AccessSpecCount();
	for (a = 0; a < spec_count; a++) {
		spec = access_spec_list[a]->access;

		// Loop through all possible transfer sizes for the given spec.
		for (s = 0; spec[s].of_size != IOERROR; s++) {
			// Mark the spec with the largest request or reply size.
			if (spec[s].size > max_size)
				max_size = spec[s].size;

			if (spec[s].reply > max_size)
				max_size = spec[s].reply;
		}
	}
	return max_size;
}

/////////////////////////////////////////////////////////////////
//
// The following functions handle the management of access specs
//
/////////////////////////////////////////////////////////////////

//
// Verify that all assigned test paramters are valid.  This checks that
// the worker can allocate enough memory to fulfill the largest request
// size for all access specs and all requests are sector sized aligned.
//
BOOL Worker::InvalidSetup(BOOL & invalidSpecOK)
{
	int a, s, d;		// loop variables
	Access_Spec *spec;
	DWORD max_size = 0;
	int max_size_index = 0;
	CString message;
	int mgr_index;
	int mgr_target_count = manager->InterfaceCount(GenericDiskType);

	// Workers without any targets are automatically valid.
	if (!TargetCount())
		return FALSE;

	// Verify that all workers with targets have a spec assigned.
	if (TargetCount() && !access_spec_list.GetSize()) {
		ErrorMessage((CString) "\"" + name + "\" on \"" + manager->name + "\" " +
			     "has targets selected, but no assigned access specifications.");
		return TRUE;
	}
	// Loop through all access specs to verify that the worker can handle them.
	for (a = 0; a < AccessSpecCount(); a++) {
		spec = access_spec_list[a]->access;

		// Loop through all possible transfer sizes for the given spec.
		for (s = 0; spec[s].of_size != IOERROR; s++) {
			// If the worker is a disk worker, verify that the request size
			// aligns with all selected disk sector sizes.
			for (d = 0; d < TargetCount(); d++) {
				if (!IsType(GetTarget(d)->spec.type, GenericDiskType))
					continue;

				// For each of assigned worker targets, locate the corresponding target data
				// in the manager's list.
				for (mgr_index = 0; mgr_index < mgr_target_count; mgr_index++) {
					if (strcmp(GetTarget(d)->spec.name,
						   manager->GetInterface(mgr_index, GenericDiskType)->name) == 0)
						break;
				}

				if (invalidSpecOK == FALSE) {

					message.Empty();

					// Verify a valid request size.

					if (spec[s].size % manager->GetInterface(mgr_index,
										 GenericDiskType)->disk_info.
					    sector_size) {

						message.
						    Format
						    ("Access specification %s for \"%s\" on \"%s\" may be invalid.\n\n"
						     "The request size does not align with disk sector size for disk %s.  "
						     "Sector size = %i.\n\n" "Would you like to continue anyway?\n\n"
						     "Selecting \"Yes\" will also ignore other potentially invalid "
						     "specifications for this test.", GetAccessSpec(a)->name, name,
						     manager->name, manager->GetInterface(mgr_index,
											  GenericDiskType)->name,
						     manager->GetInterface(mgr_index,
									   GenericDiskType)->disk_info.sector_size);
					}
					// Verify I/Os are aligned on sector boundaries.

					else if (spec[s].align % manager->GetInterface(mgr_index,
										       GenericDiskType)->disk_info.
						 sector_size) {

						message.
						    Format
						    ("Access specification %s for \"%s\" on \"%s\" may be invalid.\n\n"
						     "The alignment value is not a multiple of the "
						     "sector size for disk %s.  Sector size = %i.\n\n"
						     "Would you like to continue anyway?\n\n"
						     "Selecting \"Yes\" will also ignore other potentially invalid "
						     "specifications for this test.", GetAccessSpec(a)->name, name,
						     manager->name, manager->GetInterface(mgr_index,
											  GenericDiskType)->name,
						     manager->GetInterface(mgr_index,
									   GenericDiskType)->disk_info.sector_size);
					}
					// Verify a valid reply size.

					else if (spec[s].reply % manager->GetInterface(mgr_index,
										       GenericDiskType)->disk_info.
						 sector_size) {

						message.
						    Format
						    ("Access specification %s for \"%s\" on \"%s\" may be invalid.\n\n"
						     "The reply size is not a multiple of the sector size "
						     "for disk %s.  Sector size = %i.\n\n"
						     "Would you like to continue anyway?\n\n"
						     "Selecting \"Yes\" will also ignore other potentially invalid "
						     "specifications for this test.", GetAccessSpec(a)->name, name,
						     manager->name, manager->GetInterface(mgr_index,
											  GenericDiskType)->name,
						     manager->GetInterface(mgr_index,
									   GenericDiskType)->disk_info.sector_size);
					}

					if (!message.IsEmpty()) {
						if (AfxMessageBox(message, MB_ICONQUESTION | MB_YESNO, 0) == IDYES) {
							invalidSpecOK = TRUE;
						} else {
							return TRUE;
						}
					}
				}
			}

			// Mark the spec with the largest request or reply size.
			if (spec[s].size > max_size) {
				max_size = spec[s].size;
				max_size_index = s;
			}
			if (spec[s].reply > max_size) {
				max_size = spec[s].reply;
				max_size_index = s;
			}
		}
	}

	// Verify that the worker can handle the spec with the largets transfer size.
	// This checks that Dynamo can create a buffer large enough to handle the spec.
	if (!SetAccess(max_size_index)) {
		message.Format("\"%s\" on \"%s\" cannot set specification %s.  "
			       "The largest request or reply size (%i bytes) may be too large.",
			       name, manager->name, GetAccessSpec(max_size_index)->name, max_size);
		ErrorMessage(message);
		return TRUE;
	}
	return FALSE;		// all specs valid
}

//
// Sets all the default access specs for the worker based on the worker's type.
//
void Worker::AssignDefaultAccessSpecs()
{
	int spec_count;
	Test_Spec *spec;

	spec_count = theApp.access_spec_list.Count();
	for (int i = 0; i < spec_count; i++) {
		spec = theApp.access_spec_list.Get(i);
		switch (spec->default_assignment) {
		case AssignAll:
			InsertAccessSpec(spec);
			break;
		case AssignDisk:
			if (IsType(Type(), GenericDiskType))
				InsertAccessSpec(spec);
			break;
		case AssignNet:
			if (IsType(Type(), GenericServerType))
				InsertAccessSpec(spec);
			break;
		case AssignNone:
			break;
		default:
			ErrorMessage("Invalid default assignment in Worker::AssignDefaultAccessSpecs()");
			return;
		}
	}
}

//
// Inserts a new entry pointing to access spec 'index' in the global list
// before the item at 'insert_at_index' in the worker's access spec list.
//
BOOL Worker::InsertAccessSpec(Test_Spec * spec, int insert_at_index)
{
	// Network clients use their server's access spec.
	if (IsType(Type(), GenericClientType))
		return FALSE;

	// Inserts to the end of the array by default.
	if (insert_at_index == IOERROR || insert_at_index > access_spec_list.GetSize())
		insert_at_index = access_spec_list.GetSize();

	// Validating global_index.
	if (theApp.access_spec_list.IndexByRef(spec) == IOERROR)
		return FALSE;

	// Add the new index into the local array.
	access_spec_list.InsertAt(insert_at_index, spec);
	return TRUE;
}

//
// Remove all instances of an access spec from the worker's access spec list.
//
void Worker::RemoveAccessSpec(Test_Spec * spec)
{
	if (IsType(Type(), GenericClientType))
		return;

	// Loop through the worker's access specs and remove any that match.
	// Loop backwards, so that we don't miss anyone.
	for (int i = AccessSpecCount() - 1; i >= 0; i--) {
		// Compare the worker's spec to the one to be removed.
		if (GetAccessSpec(i) == spec) {
			// Remove the matching spec.
			access_spec_list.RemoveAt(i);
		}
	}
}

//
// Remove a specific instance of an access spec from the worker's access spec list.
//
void Worker::RemoveAccessSpecAt(int index)
{
	if (IsType(Type(), GenericClientType))
		return;

	if (index < 0 || index >= access_spec_list.GetSize())
		return;

	access_spec_list.RemoveAt(index);
}

//
// Remove all entries from the worker's access spec list.
//
void Worker::RemoveAllAccessSpecs()
{
	access_spec_list.RemoveAll();
}

//
// Returns the number of entries in the access spec list.
//
int Worker::AccessSpecCount()
{
	if (IsType(Type(), GenericClientType))
		return net_partner->AccessSpecCount();

	return access_spec_list.GetSize();
}

//
// Returns the number of idle specs in the access spec list.
//
int Worker::IdleAccessSpecCount()
{
	int spec_count;
	int idle_specs = 0;
	Test_Spec *idle_spec = theApp.access_spec_list.Get(IDLE_SPEC);

	if (IsType(Type(), GenericClientType))
		return net_partner->IdleAccessSpecCount();

	// Count the number of idle specs.
	spec_count = AccessSpecCount();
	for (int spec = 0; spec < spec_count; spec++) {
		if (GetAccessSpec(spec) == idle_spec)
			idle_specs++;
	}
	return idle_specs;
}

//
// Return a pointer to the desired access spec object.
// 
Test_Spec *Worker::GetAccessSpec(int index)
{
	if (IsType(Type(), GenericClientType))
		return net_partner->GetAccessSpec(index);

	if (index < 0 || index >= access_spec_list.GetSize())
		return NULL;

	return access_spec_list.GetAt(index);
}

//
// Moves the entry at 'index' to before 'before_index'
//
void Worker::MoveAccessSpec(int index, int before_index)
{
	Test_Spec *spec;

	// Sanity check.
	if (index < 0 || index >= access_spec_list.GetSize()) {
		ErrorMessage("Invalid index in Worker::MoveAccessSpec().");
		return;
	}
	// Store for later use.
	spec = GetAccessSpec(index);

	// Remove the given entry.  This shifts all the following entries up by one.
	RemoveAccessSpecAt(index);

	// Insert the stored entry before the before_index.
	InsertAccessSpec(spec, before_index);
	// NOTE: if the access spec is being moved down (index < before_index),
	// this will actually insert the acccess spec AFTER the indicated entry
	// (because all the entries below the original location have been moved
	// up one).  This is less "correct" but it "feels" better.
}

//
// Returns TRUE if the worker will or is performing I/O in the current test.
//
BOOL Worker::ActiveInCurrentTest()
{
	// Network clients are active if their servers are.
	if (IsType(Type(), GenericClientType))
		return net_partner->ActiveInCurrentTest();

	// The worker is active if it has an active spec and targets to run with.
	return (HasActiveCurrentSpec() && TargetCount(ActiveType));
}

//
// Checks to see if there exists an access spec matching the current access 
// spec index and that it's not the Idle spec.
//
BOOL Worker::HasActiveCurrentSpec()
{
	int access_index;

	// Network clients use their corresponding servers access specs.
	if (IsType(Type(), GenericClientType))
		return net_partner->HasActiveCurrentSpec();

	// Check to see if the current access spec index is a valid index for 
	// this worker.
	access_index = theApp.pView->GetCurrentAccessIndex();
	if (access_index < 0 || access_index >= AccessSpecCount())
		return FALSE;

	// There exists a spec.  Make sure that it's not the Idle spec.
	return (GetAccessSpec(access_index) != theApp.access_spec_list.Get(IDLE_SPEC));
}

//
// Checks to see if there exists an access spec matching the current access
// spec index and that it's the Idle spec.
//
BOOL Worker::HasIdleCurrentSpec()
{
	int access_index;

	// Network clients use their corresponding servers access specs.
	if (IsType(Type(), GenericClientType))
		return net_partner->HasIdleCurrentSpec();

	// Check to see if the current access spec index is a valid index for 
	// this worker.
	access_index = theApp.pView->GetCurrentAccessIndex();
	if (access_index < 0 || access_index >= AccessSpecCount())
		return FALSE;

	// There exists a spec.  Make sure that it's the Idle spec.
	return (GetAccessSpec(access_index) == theApp.access_spec_list.Get(IDLE_SPEC));
}

//
// Save the worker configuration to the specified stream.
//              save_aspecs --> should each worker's access spec assignments be saved?
//              save_targets --> should each worker's target assignments be saved?
//
BOOL Worker::SaveConfig(ostream & outfile, BOOL save_aspecs, BOOL save_targets)
{
	// Don't record network clients.  They are restored when the network
	// server's targets are examined.
	if (IsType(Type(), GenericClientType))
		return TRUE;

	outfile << "'Worker" << endl << "\t" << name << endl << "'Worker type" << endl;

	// Determine the worker type.
	if (IsType(Type(), GenericDiskType)) {
		outfile << "\tDISK" << endl;
	} else if (IsType(Type(), GenericNetType)) {
		outfile << "\tNETWORK";

		if (IsType(Type(), GenericTCPType))
			outfile << ",TCP" << endl;
		else if (IsType(Type(), GenericVIType))
			outfile << ",VI" << endl;
		else {
			ErrorMessage("Error saving worker \"" + (CString) name + "\": "
				     "Network worker is neither TCP nor VI.");
			return FALSE;
		}
	} else {
		ErrorMessage("Error saving worker \"" + (CString) name + "\": "
			     "Worker is neither a DISK nor a NETWORK worker.");
		return FALSE;
	}

	outfile << "'Default target settings for worker" << endl;

	outfile << "'Number of outstanding IOs,test connection rate,transactions per connection,use fixed seed,fixed seed value" << endl;
	outfile << "\t" << GetQueueDepth(Type())
		<< "," << (GetConnectionRate(Type())? "ENABLED" : "DISABLED")
		<< "," << GetTransPerConn(Type()) 
		<< "," << (GetUseFixedSeed(Type())? "ENABLED" : "DISABLED")
		<< "," << GetFixedSeedValue(Type()) << endl;

	if (IsType(spec.type, GenericDiskType)) {
		outfile << "'Disk maximum size,starting sector,Data pattern" << endl;

		outfile << "\t" << GetDiskSize(Type())
		    << "," << GetDiskStart(Type()) << "," << GetDataPattern(Type()) << endl;
	}

	if (IsType(spec.type, GenericNetType)) {
		outfile << "'Local network interface" << endl << "\t" << GetLocalNetworkInterface() << endl;
	}

	if (IsType(spec.type, GenericVIType)) {
		outfile << "'VI outstanding IOs" << endl << "\t" << GetMaxSends(Type()) << endl;
	}

	outfile << "'End default target settings for worker" << endl;

	if (save_aspecs) {
		int spec_count = AccessSpecCount();

		outfile << "'Assigned access specs" << endl;

		for (int counter = 0; counter < spec_count; counter++)
			outfile << "\t" << access_spec_list[counter]->name << endl;

		outfile << "'End assigned access specs" << endl;
	}

	if (save_targets) {
		Target_Spec tspec;
		int target_count = TargetCount();

		outfile << "'Target assignments" << endl;

		for (int counter = 0; counter < target_count; counter++) {
			tspec = GetTarget(counter)->spec;

			outfile << "'Target" << endl;

			// Determine the worker type.
			if (IsType(tspec.type, GenericDiskType)) {
				outfile << "\t" << tspec.name << endl << "'Target type" << endl << "\tDISK" << endl;
			} else if (IsType(tspec.type, GenericNetType)) {
				if (IsType(tspec.type, GenericTCPType)) {
					outfile << "\t" << tspec.tcp_info.remote_address << endl
					    << "'Target type" << endl << "\tNETWORK,TCP" << endl;
				} else if (IsType(tspec.type, GenericVIType)) {
					outfile << "\t" << tspec.vi_info.remote_nic_name << endl
					    << "'Target type" << endl << "\tNETWORK,VI" << endl;
				} else {
					ErrorMessage("Error saving target \"" + (CString) tspec.name + "\": "
						     "Network target is neither TCP nor VI.");
					return FALSE;
				}

				if (net_partner == NULL) {
					ErrorMessage("Error saving target \"" + (CString) tspec.name + "\": "
						     "Network worker with a selected network target has no net_partner.");
					return FALSE;
				}

				outfile << "'Target manager ID, manager name" << endl
				    << "\t" << net_partner->manager->id << "," << net_partner->manager->name << endl;
			} else {
				ErrorMessage("Error saving target \"" + (CString) tspec.name + "\": "
					     "Target is neither a DISK nor a NETWORK target.");
				return FALSE;
			}

			outfile << "'End target" << endl;
		}

		outfile << "'End target assignments" << endl;
	}

	outfile << "'End worker" << endl;

	return TRUE;
}

//
// Restore the worker configuration from the specified stream.
//              load_aspecs --> should each worker's access spec assignments be loaded?
//              load_targets --> should each worker's target assignments be loaded?
//
// Returns TRUE on success, FALSE if any of the requested information
// couldn't be found in the file.
//
BOOL Worker::LoadConfig(ICF_ifstream & infile, BOOL load_aspecs, BOOL load_targets)
{
	CString comment;

	while (1) {
		// The value returned may or may not actually be a comment.
		// It SHOULD be a comment.
		comment = infile.GetNextLine();

		if (comment.CompareNoCase("'End worker") == 0) {
			break;
		} else if (comment.CompareNoCase("'Default target settings for worker") == 0) {
			if (!LoadConfigDefault(infile))
				return FALSE;
		} else if (comment.CompareNoCase("'Assigned access specs") == 0) {
			if (load_aspecs) {
				if (!LoadConfigAccess(infile))
					return FALSE;
			} else {
				if (!infile.SkipTo("'End assigned access specs")) {
					ErrorMessage("File is improperly formatted.  Couldn't "
						     "find an \"End assigned access specs\" comment.");
					return FALSE;
				}
			}
		} else if (comment.CompareNoCase("'Target assignments") == 0) {
			if (load_targets) {
				if (!LoadConfigTargets(infile))
					return FALSE;
			} else {
				if (!infile.SkipTo("'End target assignments")) {
					ErrorMessage("File is improperly formatted.  Couldn't "
						     "find an \"End target assignments\" comment.");
					return FALSE;
				}
			}

		} else {
			ErrorMessage("File is improperly formatted.  WORKER section "
				     "contained an unrecognized line: \"" + comment + "\".");
			return FALSE;
		}
	}

	// Make sure TRUE is returned even if the header comment for the
	// requested setup info can't be found.  This applies only to access
	// spec and target assignments.

	return TRUE;
}

BOOL Worker::LoadConfigDefault(ICF_ifstream & infile)
{
	CString key, value;
	CString token;
	int temp_number;
	DWORDLONG temp_num64;

	while (1) {
		if (!infile.GetPair(key, value)) {
			ErrorMessage("File is improperly formatted.  Expected more default target "
				     "settings for worker or an \"End default target settings for worker\" comment.");
			return FALSE;
		}

		if (key.CompareNoCase("'End default target settings for worker") == 0) {
			return TRUE;	// This is the only normal exit.

		//Check for two keys here, this will allow backwards compatabilty to icf files before the use fixed seed changes
		//For backwards compatability with builds of Iomter that do not support the fixed seed value
		//SaveConfig() Only print out the new ICF values if the worker is using fixed seeds.
		//NOTE: If different workers have different UseFixedSeed values, the output line here could be different
		} else if (key.CompareNoCase("'Number of outstanding IOs,test connection rate,transactions per connection") == 0
				|| key.CompareNoCase("'Number of outstanding IOs,test connection rate,transactions per connection,use fixed seed,fixed seed value") == 0) 
			{
			if (!ICF_ifstream::ExtractFirstInt(value, temp_number)) {
				ErrorMessage("Error while reading file.  "
					     "\"Number of outstanding IOs\" should be specified as an integer value.");
				return FALSE;
			}

			SetQueueDepth(temp_number);

			token = ICF_ifstream::ExtractFirstToken(value);

			if (token.CompareNoCase("ENABLED") == 0)
				SetConnectionRate(TRUE);
			else if (token.CompareNoCase("DISABLED") == 0)
				SetConnectionRate(FALSE);
			else {
				ErrorMessage("Error restoring worker " + (CString) name + ".  "
					     "\"Test connection rate\" should be set to ENABLED or DISABLED.");
				return FALSE;
			}

			if (!ICF_ifstream::ExtractFirstInt(value, temp_number)) {
				ErrorMessage("Error while reading file.  "
					     "\"Transactions per connection\" should "
					     "be specified as an integer value.");
				return FALSE;
			}

			SetTransPerConn(temp_number);

			//Load the Use Fixed Seed flag and value if this icf has those values
			if(key.CompareNoCase("'Number of outstanding IOs,test connection rate,transactions per connection,use fixed seed,fixed seed value") == 0) {
				token = ICF_ifstream::ExtractFirstToken(value);
				if (token.CompareNoCase("ENABLED") == 0)
					SetUseFixedSeed(TRUE);
				else if (token.CompareNoCase("DISABLED") == 0)
					SetUseFixedSeed(FALSE);
				else {
					ErrorMessage("Error restoring worker " + (CString) name + ".  "
							 "\"Use fixed seed\" should be set to ENABLED or DISABLED.");
					return FALSE;
				}

				if (!ICF_ifstream::ExtractFirstUInt64(value, temp_num64)) {
					ErrorMessage("Error while reading file.  "
							 "\"Fixed seed value\" should "
							 "be specified as an integer value.");
					return FALSE;
				}
				SetFixedSeedValue(temp_num64);
			}


		}  else if (key.CompareNoCase("'Disk maximum size,starting sector") == 0) {
			if (!IsType(Type(), GenericDiskType)) {
				ErrorMessage("Error restoring worker " + (CString) name + ".  "
					     "Cannot specify \"Disk maximum size,starting sector\" for a non-disk worker.");
				return FALSE;
			}

			if (!ICF_ifstream::ExtractFirstUInt64(value, temp_num64)) {
				ErrorMessage("Error while reading file.  "
					     "\"Disk maximum size\" should be specified as an integer value.");
				return FALSE;
			}

			SetDiskSize(temp_num64);

			if (!ICF_ifstream::ExtractFirstUInt64(value, temp_num64)) {
				ErrorMessage("Error while reading file.  "
					     "\"Starting sector\" should be specified as an integer value.");
				return FALSE;
			}

			SetDiskStart(temp_num64);
		} else if (key.CompareNoCase("'Disk maximum size,starting sector,Data pattern") == 0) {
			if (!IsType(Type(), GenericDiskType)) {
				ErrorMessage("Error restoring worker " + (CString) name + ".  "
					     "Cannot specify \"Disk maximum size,starting sector\" for a non-disk worker.");
				return FALSE;
			}

			if (!ICF_ifstream::ExtractFirstUInt64(value, temp_num64)) {
				ErrorMessage("Error while reading file.  "
					     "\"Disk maximum size\" should be specified as an integer value.");
				return FALSE;
			}

			SetDiskSize(temp_num64);

			if (!ICF_ifstream::ExtractFirstUInt64(value, temp_num64)) {
				ErrorMessage("Error while reading file.  "
					     "\"Starting sector\" should be specified as an integer value.");
				return FALSE;
			}

			SetDiskStart(temp_num64);

			if (!ICF_ifstream::ExtractFirstInt(value, temp_number)) {
				ErrorMessage("Error while reading file.  "
					     "\"Data pattern\" should be specified as an integer value.");
				return FALSE;
			}

			SetDataPattern(temp_number);
		} else if (key.CompareNoCase("'Local network interface") == 0) {
			if (!IsType(Type(), GenericNetType)) {
				ErrorMessage("Error restoring worker " + (CString) name + ".  "
					     "Cannot specify \"Local network interface\" for a non-TCP worker.");
				return FALSE;
			}

			SetLocalNetworkInterface(value);
		} else if (key.CompareNoCase("'VI outstanding IOs") == 0) {
			if (!IsType(Type(), GenericVIType)) {
				ErrorMessage("Error restoring worker " + (CString) name + ".  "
					     "Cannot specify \"VI outstanding IOs\" for a non-VI worker.");
				return FALSE;
			}

			if (!ICF_ifstream::ExtractFirstInt(value, temp_number)) {
				ErrorMessage("Error while reading file.  "
					     "\"VI outstanding IOs\" should be specified as an integer value.");
				return FALSE;
			}

			SetMaxSends(temp_number);
		} else {
			ErrorMessage("File is improperly formatted.  "
				     "DEFAULT TARGET SETTINGS FOR WORKER "
				     "section contained an unrecognized \"" + key + "\"" "comment.");
			return FALSE;
		}
	}
}

BOOL Worker::LoadConfigAccess(ICF_ifstream & infile)
{
	CString value;
	Test_Spec *aspec;

	while (1) {
		value = infile.GetNextLine();

		if (value.CompareNoCase("'End assigned access specs") == 0) {
			return TRUE;
		} else {
			aspec = theApp.access_spec_list.RefByName(value);

			if (aspec != NULL) {
				InsertAccessSpec(aspec);
			} else {
				ErrorMessage("Error restoring access specification "
					     "assignments for worker \"" + (CString) name + "\".  "
					     "Access spec \"" + value + "\" doesn't exist.  " "Ignoring.");
				continue;
			}
		}
	}
}

BOOL Worker::LoadConfigTargets(ICF_ifstream & infile)
{
	CString tgt_comment, tgt_name;
	CString tgt_typestring;
	CString token;
	CString mgr_info, mgr_name;
	int mgr_id;
	TargetType tgt_type;

	while (1) {
		// Expecting "Target" or "End target assignments"
		if (!infile.GetPair(tgt_comment, tgt_name)) {
			ErrorMessage("File is improperly formatted.  Expected a "
				     "target or \"End target assignments\".");
			return FALSE;
		}

		if (tgt_comment.CompareNoCase("'End target assignments") == 0) {
			return TRUE;	// This is the only normal exit.
		}

		if (tgt_comment.CompareNoCase("'Target") != 0) {
			ErrorMessage("File is improperly formatted.  Expected "
				     "a \"Target\" comment inside TARGET ASSIGNMENTS " "section.");
			return FALSE;
		}
		// Expecting "Target type"
		if (!infile.GetPair(tgt_comment, tgt_typestring)) {
			ErrorMessage("File is improperly formatted.  Expected " "\"Target type\".");
			return FALSE;
		}

		if (tgt_comment.CompareNoCase("'Target type") != 0) {
			ErrorMessage("File is improperly formatted.  Expected "
				     "a \"Target type\" comment after target name.");
			return FALSE;
		}

		token = ICF_ifstream::ExtractFirstToken(tgt_typestring);
		if (token.CompareNoCase("DISK") == 0) {
			tgt_type = GenericDiskType;
		} else if (token.CompareNoCase("NETWORK") == 0) {
			token = ICF_ifstream::ExtractFirstToken(tgt_typestring);

			if (token.CompareNoCase("TCP") == 0) {
				tgt_type = TCPClientType;
			} else if (token.CompareNoCase("VI") == 0) {
				tgt_type = VIClientType;
			} else {
				ErrorMessage("Error restoring target " + tgt_name + ".  "
					     "Network target type is neither TCP nor VI.");
				return FALSE;
			}
		} else {
			ErrorMessage("Error restoring target " + tgt_name + ".  "
				     "Target type is neither DISK nor NETWORK.");
			return FALSE;
		}

		if (IsType(Type(), GenericDiskType)) {
			CString fileDiskName(tgt_name);	//isolate "drive:" portion of name (bug #369)

			if (fileDiskName[1] == ':')
				fileDiskName = fileDiskName.Left(2);

			//Find matching disk spec (drive portion only, if formatted).
			int interface_count = manager->InterfaceCount(tgt_type);
			Target_Spec *tspec;
			int counter;

			for (counter = 0; counter < interface_count; counter++) {
				// Yes, GetInterface is used to get Disk tspecs, too.
				tspec = manager->GetInterface(counter, tgt_type);

				CString thisDiskName(tspec->name);

				if (thisDiskName[1] == ':')
					thisDiskName = thisDiskName.Left(2);

				if (fileDiskName.CompareNoCase(thisDiskName) == 0)
					break;
			}

			if (counter >= interface_count) {
				ErrorMessage("Error loading the ICF file.  A worker "
					     "is assigned a target, \"" + tgt_name + "\", that isn't available.");
				return FALSE;
			}
			// Don't assign the target if it has already been assigned.
			if (!IsTargetAssigned(tspec))
				AddTarget(tspec);
		} else if (IsType(Type(), GenericNetType)) {
			Manager *remote_manager;
			int remote_nic_counter, remote_nic_total;
			Target_Spec *remote_tspec, local_tspec;

			// Load config information identifying the network worker's target.

			// Expecting "Target manager ID, manager name"
			if (!infile.GetPair(tgt_comment, mgr_info)) {
				ErrorMessage("File is improperly formatted.  Expected "
					     "network target manager ID and name.");
				return FALSE;
			}

			if (tgt_comment.CompareNoCase("'Target manager ID, manager name") != 0) {
				ErrorMessage("File is improperly formatted.  Expected "
					     "a \"Target manager ID, manager name\" comment after \"Target type\".");
				return FALSE;
			}

			if (!ICF_ifstream::ExtractFirstInt(mgr_info, mgr_id)) {
				ErrorMessage("Error while reading file.  "
					     "\"Target manager ID\" should be specified as an integer value.");
				return FALSE;
			}

			if (mgr_info.GetLength() == 0) {
				ErrorMessage("File is improperly formatted.  Expected " "a target \"manager name\".");
				return FALSE;
			}

			mgr_name = mgr_info;

			// Make sure the worker and target are of compatible types.
			if ((Type() & NETWORK_COMPATIBILITY_MASK)
			    != (tgt_type & NETWORK_COMPATIBILITY_MASK)) {
				ErrorMessage("Error loading the ICF file.  A worker "
					     "was assigned a target of an incompatible type.  A "
					     "TCP worker can only have TCP targets, etc.");
				return FALSE;
			}
			// Remove any targets previously assigned to this worker.
			RemoveTargets(GenericNetType);

			remote_manager = theApp.manager_list.loadmap.Retrieve(mgr_name, mgr_id);

			if (remote_manager == NULL) {
				ErrorMessage("While restoring network worker target assignments, "
					     "remote manager could not be found in the ManagerMap.  "
					     "Please report this as an Iometer bug.");
				return FALSE;
			}

			remote_nic_total = remote_manager->InterfaceCount(tgt_type);

			for (remote_nic_counter = 0; remote_nic_counter < remote_nic_total; remote_nic_counter++) {
				remote_tspec = remote_manager->GetInterface(remote_nic_counter, tgt_type);
				if (tgt_name.CompareNoCase(remote_tspec->name) == 0)
					break;
			}

			if (remote_nic_counter >= remote_nic_total) {
				ErrorMessage("Error loading the ICF file.  A worker "
					     "is assigned a target, \"" + tgt_name + "\", that isn't available.");
				return FALSE;
			}
			// Copy the remote NIC's Target_Spec info.
			memcpy(&local_tspec, remote_tspec, sizeof(Target_Spec));

			// Handle TCP-specific stuff.
			if (IsType(local_tspec.type, TCPClientType)) {
				strcpy(local_tspec.tcp_info.remote_address, local_tspec.name);
			}
			// Handle VI-specific stuff.
			if (IsType(local_tspec.type, VIClientType)) {
				strcpy(local_tspec.vi_info.remote_nic_name, local_tspec.name);
				memcpy(&local_tspec.vi_info.remote_address,
				       &local_tspec.vi_info.local_address, VI_ADDRESS_SIZE);
			}
			// Add the target to the worker's assigned list.
			AddTarget(&local_tspec);

			// Writes the necessary local NIC information to
			// the network target's Target_Spec structure.
			SetLocalNetworkInterface(GetLocalNetworkInterface());

			// Start up the network client.
			CreateNetClient(remote_manager, remote_tspec->type);
		} else {
			ErrorMessage("Error loading the ICF file.  Unknown target " "type encountered.");
			return FALSE;
		}

		tgt_comment = infile.GetNextLine();
		if (tgt_comment.CompareNoCase("'End target") != 0) {
			ErrorMessage("File is improperly formatted.  Expected " "an \"End target\" comment.");
			return FALSE;
		}
	}
}
