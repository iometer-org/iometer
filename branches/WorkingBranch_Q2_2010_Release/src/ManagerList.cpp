/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Iometer / ManagerList.cpp                                          ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Represents the list of managers associated to the     ## */
/* ##               Iometer GUI itself.                                   ## */
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
/* ##               2003-05-07 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Modified output of the current timestamp to         ## */
/* ##                 contain milliseconds as well.                       ## */
/* ##               2003-05-01 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added output of current timestamp after each list   ## */
/* ##                 of results (in the result file - CSV format).       ## */
/* ##               2003-04-25 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Updated the global debug flag (_DEBUG) handling     ## */
/* ##                 of the source file (check for platform etc.).       ## */
/* ##               2003-03-01 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Cut out the Windows Pipes support for               ## */
/* ##                 communication efforts.                              ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include "stdafx.h"
#include "ManagerList.h"
#include "GalileoView.h"
#include "IOPortTCP.h"

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

ManagerList::ManagerList()
{
	ResetResults(WHOLE_TEST_PERF);
	ResetResults(LAST_UPDATE_PERF);
	strcpy(name, "All Managers");
}

ManagerList::~ManagerList()
{
	RemoveAllManagers();
}

//
// Get the nth manager matching the specified type.
//
Manager *ManagerList::GetManager(int index, TargetType type)
{
	int i, manager_count;

	// Make sure that the manager exists.
	if (index < 0 || index >= ManagerCount(type))
		return NULL;

	manager_count = ManagerCount();
	for (i = 0; i < manager_count; i++) {
		if (IsType(managers[i]->Type(), type)) {
			if (!index--)
				return managers[i];
		}
	}

	ASSERT(0);		// requested manager not found - should not happen
	return NULL;
}

//
// Get the first manager having the specified name and ID, if any.
//
Manager *ManagerList::GetManagerByName(const char *mgr_name, const int mgr_id)
{
	int i, manager_count;

	if (!mgr_name) {
		ErrorMessage("Invalid string in ManagerList::GetManagerByName()");
		return NULL;
	}

	manager_count = ManagerCount();
	for (i = 0; i < manager_count; i++) {
		if (strcmp(managers[i]->name, mgr_name) == 0 && mgr_id == managers[i]->id) {
			return managers[i];
		}
	}

	// Didn't find it (not an error).
	return NULL;
}

//
// Adding a new manager to the manager list.  The new manager is always added 
// to the end of the list.  A connection to the manager is also made
//
Manager *ManagerList::AddManager(Manager_Info * manager_info)
{
	unsigned short network_port = manager_info->port_number;

	// Creating space for a new manager.
	Manager *manager;

	manager = new Manager;

	strcpy(manager->computer_name, manager_info->names[0]);

	strcpy(manager->name, manager->computer_name);
	manager->processors = manager_info->processors;
	manager->timer_resolution = (double)manager_info->timer_resolution;

	// Connect to the new manager using TCP/IP as appropriate
	if (theApp.login_port->type == PORT_TYPE_TCP) {
		strcpy(manager->network_name, manager_info->names[1]);

		manager->port = new PortTCP;

		// Do not connect to "network_name".  See Manager.h for detailed comments.
		if (!manager->port->Connect(manager_info->names[1], network_port)) {
			ErrorMessage("Could not connect to new manager using "
				     "TCP/IP port in ManagerList::AddManager");
			return NULL;
		}
	} else {
		ErrorMessage("Invalid login port in ManagerList::AddManager");
		return NULL;
	}

	// Adding manager to end of the manager list and updating the last manager accessed
	// to be the new manager.
	managers.Add(manager);

	// Assign disambiguating integer identifiers to same-named managers.
	theApp.manager_list.IndexManagers();

	// Notify the manager loadmap that a new manager has logged in.
	// This notification is only necessary if a configuration file restore
	// is taking place and some of the managers required to restore the
	// manager/worker configuration are not yet connected to Iometer.
	if (loadmap.IsWaitingList())
		(void)loadmap.ManagerLoggedIn(manager->name, manager->network_name, manager);

	// Return a pointer to the new manager.
	return manager;
}

//
// Comparing the network names of two machines.
//
BOOL ManagerList::CompareNames(char *net_name1, char *net_name2)
{
	CString name1, name2;

	name1 = net_name1;
	name2 = net_name2;
	name1 = name1.Mid(2);	// remove the leading '\\'
	name2 = name2.Mid(2);	// remove the leading '\\'
	name1 = name1.Mid(0, name1.Find('\\'));
	name2 = name2.Mid(0, name2.Find('\\'));
	return (name1 == name2);
}

//
// Removing a manager from the list.
//
BOOL ManagerList::RemoveManager(int index, int purpose)
{
	int i;

	if (index < 0 || index >= ManagerCount()) {
		ErrorMessage("Invalid index in ManagerList::RemoveManager().");
		return FALSE;
	}
	// Informing manager to end execution or reset depending on the purpose.
	Send(index, purpose);

	// Remove both the manager and it's reference in the list.
	delete GetManager(index);

	managers.RemoveAt(index);

	// Assign disambiguating integer identifiers to same-named managers.
	theApp.manager_list.IndexManagers();

	// Perform a full reset if all managers have
	// been removed to reset the display properly.
	i = ManagerCount();
	if (!i)
		theApp.pView->Reset();

	return (i);
}

//
// Clearing the list of all managers.
//
void ManagerList::RemoveAllManagers(int purpose)
{
	while (ManagerCount())
		RemoveManager(0, purpose);
}

//
// Initializing results for all systems and their workers.
//
void ManagerList::ResetResults(int which_perf)
{
	if ((which_perf < 0) || (which_perf >= MAX_PERF))
		return;

	memset(&(results[which_perf]), 0, sizeof(Results));
}

//
// Reset all results for all managers.  This includes resetting the results for all
// managers and any results stored for them.
//
void ManagerList::ResetAllResults()
{
	ResetResults(WHOLE_TEST_PERF);
	ResetResults(LAST_UPDATE_PERF);

	// Have all managers reset their results too.
	for (int i = 0; i < ManagerCount(); i++)
		GetManager(i)->ResetAllResults();
}

//
// Sending a message to a manager.
//
DWORDLONG ManagerList::Send(int manager_index, Message * msg, int recipient)
{
	msg->data = recipient;
	return GetManager(manager_index)->Send(msg);
}

DWORDLONG ManagerList::Send(int manager_index, int purpose, int recipient)
{
	Message msg;

	msg.purpose = purpose;
	msg.data = recipient;
	return GetManager(manager_index)->Send(&msg);
}

//
// Sending a message to all managers.
//
void ManagerList::SendManagers(Message * msg, int recipient)
{
	msg->data = recipient;
	for (int i = 0; i < ManagerCount(); i++)
		GetManager(i)->Send(msg);
}

void ManagerList::SendManagers(int purpose, int recipient)
{
	Message msg;

	msg.purpose = purpose;
	SendManagers(&msg, recipient);
}

//
// Sending a message to all managers which are active in the current test.
//
BOOL ManagerList::SendActiveManagers(int purpose)
{
	Manager *manager;
	Message reply_msg;

	for (int i = 0; i < ManagerCount(); i++) {
		manager = GetManager(i);

		// Skip managers not active in the current test.
		if (!manager->ActiveInCurrentTest())
			continue;

		manager->Send(ALL_WORKERS, purpose);

		// Check to see if a reply is necessary.
		if (purpose & REPLY_FILTER) {
			// Reply expected.  Check for success.
			manager->Receive(&reply_msg);
			if (!reply_msg.data)
				return FALSE;
		}
	}
	return TRUE;
}

//
// Getting a message from a specified manager.
//
DWORDLONG ManagerList::ReceiveManager(int index, Message * msg)
{
	return GetManager(index)->Receive(msg);
}

//
// Receiving a message from all managers.  The contents of the message are 
// discarded. This is usually used to indicate reply messages from the 
// workers, so that Iometer knows that all managers have received the messages.
//
void ManagerList::ReceiveManagers()
{
	Message msg;

	for (int m = 0; m < ManagerCount(); m++)
		ReceiveManager(m, &msg);
}

BOOL ManagerList::SetAccess(int spec_index)
{
	for (int m = 0; m < ManagerCount(); m++) {
		if (!GetManager(m)->SetAccess(spec_index))
			return FALSE;
	}
	return TRUE;
}

//
// Setting active targets of all workers.
//
BOOL ManagerList::SetTargets()
{
	for (int m = 0; m < ManagerCount(); m++) {
		if (!GetManager(m)->SetTargets())
			return FALSE;
	}
	return TRUE;
}

void ManagerList::Reset()
{
	RemoveAllManagers(RESET);
	// Clear the manager list's results.
	ResetResults(WHOLE_TEST_PERF);
	ResetResults(LAST_UPDATE_PERF);
}

void ManagerList::SaveResults(ostream * file, int access_index, int result_type)
{
	char specname[MAX_WORKER_NAME];
	int stat = 0;
	struct _timeb tb;
	struct tm *ptm;
	char acDummy[64];

	// Writing result header information.
	(*file) << "'Results" << endl
	    << "'Target Type,Target Name,Access Specification Name,# Managers,"
	    << "# Workers,# Disks,IOps,Read IOps,Write IOps,MBps (Binary),Read MBps (Binary),Write MBps (Binary),"
		<< "MBps (Decimal),Read MBps (Decimal),Write MBps (Decimal),Transactions per Second,Connections per Second,"
	    << "Average Response Time,Average Read Response Time,"
	    << "Average Write Response Time,Average Transaction Time,"
	    << "Average Connection Time,Maximum Response Time,"
	    << "Maximum Read Response Time,Maximum Write Response Time,"
	    << "Maximum Transaction Time,Maximum Connection Time,"
	    << "Errors,Read Errors,Write Errors,Bytes Read,Bytes Written,Read I/Os,"
	    << "Write I/Os,Connections,Transactions per Connection,"
	    << "Total Raw Read Response Time,Total Raw Write Response Time,"
	    << "Total Raw Transaction Time,Total Raw Connection Time,"
	    << "Maximum Raw Read Response Time,Maximum Raw Write Response Time,"
	    << "Maximum Raw Transaction Time,Maximum Raw Connection Time,"
	    << "Total Raw Run Time,Starting Sector,Maximum Size,Queue Depth,"
	    << "% CPU Utilization,% User Time,% Privileged Time,% DPC Time,"
	    << "% Interrupt Time,Processor Speed,Interrupts per Second,"
	    << "CPU Effectiveness,Packets/Second,Packet Errors," << "Segments Retransmitted/Second" << endl;

	// Writing manager list results

	(*file) << "ALL" << "," << "All" << "," << GetCommonAccessSpec(access_index, specname)
	    << "," << ManagerCount(ActiveType)
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
	    << "," << results[WHOLE_TEST_PERF].raw.read_errors
	    << "," << results[WHOLE_TEST_PERF].raw.write_errors
	    << "," << results[WHOLE_TEST_PERF].raw.bytes_read
	    << "," << results[WHOLE_TEST_PERF].raw.bytes_written
	    << "," << results[WHOLE_TEST_PERF].raw.read_count
	    << "," << results[WHOLE_TEST_PERF].raw.write_count
	    << "," << results[WHOLE_TEST_PERF].raw.connection_count << ",";

	if (GetConnectionRate(ActiveType) == ENABLED_VALUE)
		(*file) << GetTransPerConn(ActiveType);
	else
		(*file) << AMBIGUOUS_VALUE;

	(*file) << ",,,,,,,,,";	// unused raw results for manager list

	(*file) << "," << GetDiskStart((TargetType) (GenericDiskType | ActiveType))
	    << "," << GetDiskSize((TargetType) (GenericDiskType | ActiveType))
	    << "," << GetQueueDepth(ActiveType);

	for (stat = 0; stat < CPU_UTILIZATION_RESULTS; stat++)
		(*file) << "," << results[WHOLE_TEST_PERF].CPU_utilization[stat];

	(*file) << ","		// processor speed
	    << "," << results[WHOLE_TEST_PERF].CPU_utilization[CPU_IRQ];

	(*file) << "," << results[WHOLE_TEST_PERF].CPU_effectiveness;

	for (stat = 0; stat < NI_COMBINE_RESULTS; stat++)
		(*file) << "," << results[WHOLE_TEST_PERF].ni_statistics[stat];

	for (stat = 0; stat < TCP_RESULTS; stat++)
		(*file) << "," << results[WHOLE_TEST_PERF].tcp_statistics[stat];

	(*file) << endl;

	// If requested, save manager results.
	if (result_type == RecordAll || result_type == RecordNoTargets || result_type == RecordNoWorkers) {
		for (int i = 0; i < ManagerCount(); i++)
			GetManager(i)->SaveResults(file, access_index, result_type);
	}
	// Write current timestamp into the result file
	(*file) << "'Time Stamp" << endl;
	_ftime(&tb);
	ptm = localtime(&tb.time);
	snprintf(acDummy, 64, "%04d-%02d-%02d %02d:%02d:%02d:%003d", ptm->tm_year + 1900,
		ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, tb.millitm);
	(*file) << acDummy << endl;
}

//
// Calculation intensive code to determine the combined performance for
// all managers and their workers.  Programmers beware.
//
void ManagerList::UpdateResults(int which_perf)
{
	Manager *manager;

	// Total time each manager used transferring I/Os in ms.
	double read_latency_sum = 0;
	double write_latency_sum = 0;
	double transaction_latency_sum = 0;
	double connection_latency_sum = 0;
	int stat;		// loop control

	if ((which_perf < 0) || (which_perf >= MAX_PERF))
		return;

	ResetResults(which_perf);

	// Loop through all managers to get their results.
	for (int i = 0; i < ManagerCount(); i++) {
		manager = GetManager(i);

		// Skip managers not active in the current test.
		if (!manager->ActiveInCurrentTest()) {
			// Clear the results of idle managers to prevent them from being
			// displayed in the results window.
			manager->ResetAllResults();
			continue;
		}
		// Request an update from all managers and process the results.
		manager->UpdateResults(which_perf);

		// Recording error results.
		results[which_perf].total_errors += manager->results[which_perf].total_errors;
		results[which_perf].raw.read_errors += manager->results[which_perf].raw.read_errors;
		results[which_perf].raw.write_errors += manager->results[which_perf].raw.write_errors;

		// Recording results related to the number of I/Os completed.
		results[which_perf].IOps += manager->results[which_perf].IOps;
		results[which_perf].read_IOps += manager->results[which_perf].read_IOps;
		results[which_perf].write_IOps += manager->results[which_perf].write_IOps;
		results[which_perf].raw.read_count += manager->results[which_perf].raw.read_count;
		results[which_perf].raw.write_count += manager->results[which_perf].raw.write_count;

		// Recording throughput results.
		results[which_perf].MBps_Bin += manager->results[which_perf].MBps_Bin;
		results[which_perf].read_MBps_Bin += manager->results[which_perf].read_MBps_Bin;
		results[which_perf].write_MBps_Bin += manager->results[which_perf].write_MBps_Bin;
		results[which_perf].MBps_Dec += manager->results[which_perf].MBps_Dec;
		results[which_perf].read_MBps_Dec += manager->results[which_perf].read_MBps_Dec;
		results[which_perf].write_MBps_Dec += manager->results[which_perf].write_MBps_Dec;
		results[which_perf].raw.bytes_read += manager->results[which_perf].raw.bytes_read;
		results[which_perf].raw.bytes_written += manager->results[which_perf].raw.bytes_written;

		// Recording results related to the number of transactions completed.
		results[which_perf].transactions_per_second += manager->results[which_perf].transactions_per_second;
		results[which_perf].raw.transaction_count += manager->results[which_perf].raw.transaction_count;

		// Recording results related to the number of connections completed.
		results[which_perf].connections_per_second += manager->results[which_perf].connections_per_second;
		results[which_perf].raw.connection_count += manager->results[which_perf].raw.connection_count;

		// Recording maximum latency information.
		if (results[which_perf].max_latency < manager->results[which_perf].max_latency) {
			results[which_perf].max_latency = manager->results[which_perf].max_latency;
		}
		if (results[which_perf].max_read_latency < manager->results[which_perf].max_read_latency) {
			results[which_perf].max_read_latency = manager->results[which_perf].max_read_latency;
		}
		if (results[which_perf].max_write_latency < manager->results[which_perf].max_write_latency) {
			results[which_perf].max_write_latency = manager->results[which_perf].max_write_latency;
		}
		if (results[which_perf].max_transaction_latency < manager->results[which_perf].max_transaction_latency) {
			results[which_perf].max_transaction_latency =
			    manager->results[which_perf].max_transaction_latency;
		}
		if (results[which_perf].max_connection_latency < manager->results[which_perf].max_connection_latency) {
			results[which_perf].max_connection_latency =
			    manager->results[which_perf].max_connection_latency;
		}

		read_latency_sum += (double)(_int64)
		    manager->results[which_perf].raw.read_latency_sum / (double)manager->timer_resolution;

		write_latency_sum += (double)(_int64)
		    manager->results[which_perf].raw.write_latency_sum / (double)manager->timer_resolution;

		transaction_latency_sum += (double)(_int64)
		    manager->results[which_perf].raw.transaction_latency_sum / (double)manager->timer_resolution;

		connection_latency_sum += (double)(_int64)
		    manager->results[which_perf].raw.connection_latency_sum / (double)manager->timer_resolution;

		for (stat = 0; stat < CPU_RESULTS; stat++) {
			results[which_perf].CPU_utilization[stat] += manager->results[which_perf].CPU_utilization[stat];
		}

		for (stat = 0; stat < TCP_RESULTS; stat++) {
			results[which_perf].tcp_statistics[stat] += manager->results[which_perf].tcp_statistics[stat];
		}
		for (stat = 0; stat < NI_COMBINE_RESULTS; stat++) {
			results[which_perf].ni_statistics[stat] += manager->results[which_perf].ni_statistics[stat];
		}
	}
	if (results[which_perf].raw.read_count || results[which_perf].raw.write_count) {
		results[which_perf].ave_latency = (read_latency_sum + write_latency_sum) * (double)
		    1000 / (double)(_int64) (results[which_perf].raw.read_count + results[which_perf].raw.write_count);

		if (results[which_perf].raw.read_count)
			results[which_perf].ave_read_latency = read_latency_sum * (double)1000 /
			    (double)(_int64) results[which_perf].raw.read_count;
		else
			results[which_perf].ave_read_latency = (double)0;

		if (results[which_perf].raw.write_count)
			results[which_perf].ave_write_latency = write_latency_sum * (double)1000
			    / (double)(_int64) results[which_perf].raw.write_count;
		else
			results[which_perf].ave_write_latency = (double)0;

		if (results[which_perf].raw.transaction_count) {
			results[which_perf].ave_transaction_latency = transaction_latency_sum *
			    (double)1000 / (double)(_int64)
			    (results[which_perf].raw.transaction_count);
		} else {
			results[which_perf].ave_transaction_latency = (double)0;
		}
	} else {
		results[which_perf].ave_latency = (double)0;
		results[which_perf].ave_read_latency = (double)0;
		results[which_perf].ave_write_latency = (double)0;
		results[which_perf].ave_transaction_latency = (double)0;
	}

	if (results[which_perf].raw.connection_count) {
		results[which_perf].ave_connection_latency = connection_latency_sum * (double)1000
		    / (double)(_int64) (results[which_perf].raw.connection_count);
	} else {
		results[which_perf].ave_connection_latency = (double)0;
	}

	for (stat = 0; stat < CPU_UTILIZATION_RESULTS; stat++) {
		results[which_perf].CPU_utilization[stat] /= ManagerCount(ActiveType);
	}

	if (results[which_perf].CPU_utilization[CPU_TOTAL_UTILIZATION] != (double)0) {
		results[which_perf].CPU_effectiveness = results[which_perf].IOps /
		    results[which_perf].CPU_utilization[CPU_TOTAL_UTILIZATION];
	} else {
		results[which_perf].CPU_effectiveness = (double)0;
	}
}

////////////////////////////////////////////////////////////////////////////////////
//
// The following functions update the values for the starting sector, the number of
// sectors to access, the queue depth, and the disk selection.
// On the ManagerList level, it propagates down the tree to all the managers and
// calls the managers function.
//
////////////////////////////////////////////////////////////////////////////////////
void ManagerList::SetDiskSize(DWORDLONG disk_size)
{
	int i, mgr_count;

	// Loop through all the managers.
	mgr_count = ManagerCount(GenericDiskType);
	for (i = 0; i < mgr_count; i++)
		GetManager(i, GenericDiskType)->SetDiskSize(disk_size);
}

void ManagerList::SetDiskStart(DWORDLONG disk_start)
{
	int i, mgr_count;

	// Loop through all the managers.
	mgr_count = ManagerCount(GenericDiskType);
	for (i = 0; i < mgr_count; i++)
		GetManager(i, GenericDiskType)->SetDiskStart(disk_start);
}

void ManagerList::SetQueueDepth(int queue_size, TargetType type)
{
	int i, mgr_count;

	// Loop through all the managers.
	mgr_count = ManagerCount(type);
	for (i = 0; i < mgr_count; i++)
		GetManager(i, type)->SetQueueDepth(queue_size, type);
}

void ManagerList::SetMaxSends(int max_sends)
{
	int i, mgr_count;

	// Loop through all the managers.
	mgr_count = ManagerCount(VIServerType);
	for (i = 0; i < mgr_count; i++)
		GetManager(i, VIServerType)->SetMaxSends(max_sends);
}

void ManagerList::SetConnectionRate(BOOL connection_rate, TargetType type)
{
	int i, mgr_count;

	// Loop through all the managers.
	mgr_count = ManagerCount(type);
	for (i = 0; i < mgr_count; i++)
		GetManager(i, type)->SetConnectionRate(connection_rate, type);
}

void ManagerList::SetUseRandomData(BOOL use_random_data, TargetType type)
{
	int i, mgr_count;

	// Loop through all the managers.
	mgr_count = ManagerCount(type);
	for (i = 0; i < mgr_count; i++)
		GetManager(i, type)->SetUseRandomData(use_random_data, type);
}

void ManagerList::SetTransPerConn(int trans_per_conn, TargetType type)
{
	int i, mgr_count;

	// Loop through all the managers.
	mgr_count = ManagerCount(type);
	for (i = 0; i < mgr_count; i++)
		GetManager(i, type)->SetTransPerConn(trans_per_conn, type);
}

///////////////////////////////////////////////
//
// Functions to retrieve worker information
//
// Functions return a valid value if all 
// underlying workers have the same value.
//
///////////////////////////////////////////////

int ManagerList::GetConnectionRate(TargetType type)
{
	int m, mgr_count, mgr_result;

	if (!(mgr_count = ManagerCount(type)))
		return AMBIGUOUS_VALUE;

	// Get the value of the first manager.
	mgr_result = GetManager(0, type)->GetConnectionRate(type);

	// Compare each manager's value with the first manager.
	for (m = 1; m < mgr_count; m++) {
		if (GetManager(m, type)->WorkerCount(type) &&
		    mgr_result != GetManager(m, type)->GetConnectionRate(type)) {
			// The values are not the same.
			return AMBIGUOUS_VALUE;
		}
	}
	// All managers have the same value.
	return mgr_result;
}

int ManagerList::GetUseRandomData(TargetType type)
{
	BOOL mgr_result;
	int m, mgr_count;

	if (!(mgr_count = ManagerCount(type)))
		return AMBIGUOUS_VALUE;

	// Get the value of the first manager.
	mgr_result = GetManager(0, type)->GetUseRandomData(type);

	// Compare each manager's value with the first manager.
	for (m = 1; m < mgr_count; m++) {
		if (GetManager(m, type)->WorkerCount(type) &&
		    mgr_result != GetManager(m, type)->GetUseRandomData(type)) {
			// The values are not the same.
			return AMBIGUOUS_VALUE;
		}
	}
	// All managers have the same value.
	return mgr_result;
}

int ManagerList::GetTransPerConn(TargetType type)
{
	int m, mgr_count, mgr_result;

	if (!(mgr_count = ManagerCount(type)))
		return AMBIGUOUS_VALUE;

	// Get the value of the first manager.
	mgr_result = GetManager(0, type)->GetTransPerConn(type);

	// Compare each manager's value with the first manager, if it has any
	// workers of the specified type.
	for (m = 1; m < mgr_count; m++) {
		if (GetManager(m, type)->WorkerCount(type) && mgr_result != GetManager(m, type)->GetTransPerConn(type)) {
			// The values are not the same.
			return AMBIGUOUS_VALUE;
		}
	}
	return mgr_result;
}

DWORDLONG ManagerList::GetDiskStart(TargetType type)
{
	int m, mgr_count;
	DWORDLONG mgr_result;

	if (!(mgr_count = ManagerCount(type)))
		return AMBIGUOUS_VALUE;

	// Get the value of the first manager.
	mgr_result = GetManager(0, type)->GetDiskStart(type);

	// Compare each manager's value with the first manager.
	for (m = 1; m < mgr_count; m++) {
		if (GetManager(m, type)->WorkerCount(type) && mgr_result != GetManager(m, type)->GetDiskStart(type)) {
			// The values are not the same.
			return AMBIGUOUS_VALUE;
		}
	}
	return mgr_result;
}

DWORDLONG ManagerList::GetDiskSize(TargetType type)
{
	int m, mgr_count;
	DWORDLONG mgr_result;

	if (!(mgr_count = ManagerCount(type)))
		return AMBIGUOUS_VALUE;

	// Get the value of the first manager.
	mgr_result = GetManager(0, type)->GetDiskSize(type);

	// Compare each manager's value with the first manager.
	for (m = 1; m < mgr_count; m++) {
		if (GetManager(m, type)->WorkerCount(type) && mgr_result != GetManager(m, type)->GetDiskSize(type)) {
			// The values are not the same.
			return AMBIGUOUS_VALUE;
		}
	}
	return mgr_result;
}

int ManagerList::GetQueueDepth(TargetType type)
{
	int m, mgr_count, mgr_result;

	if (!(mgr_count = ManagerCount(type)))
		return AMBIGUOUS_VALUE;

	// Get the value of the first manager.
	mgr_result = GetManager(0, type)->GetQueueDepth(type);

	// Compare each manager's value with the first manager.
	for (m = 1; m < mgr_count; m++) {
		if (GetManager(m, type)->WorkerCount(type) && mgr_result != GetManager(m, type)->GetQueueDepth(type)) {
			// The values are not the same.
			return AMBIGUOUS_VALUE;
		}
	}
	return mgr_result;
}

int ManagerList::GetMaxSends(TargetType type)
{
	int m, mgr_count, mgr_result;

	if (!(mgr_count = ManagerCount(type)))
		return AMBIGUOUS_VALUE;

	// Get the value of the first manager.
	mgr_result = GetManager(0, type)->GetMaxSends(type);

	// Compare each manager's value with the first manager.
	for (m = 1; m < mgr_count; m++) {
		if (GetManager(m, type)->WorkerCount(type) && mgr_result != GetManager(m, type)->GetMaxSends(type)) {
			// The values are not the same.
			return AMBIGUOUS_VALUE;
		}
	}
	return mgr_result;
}

//
// Return the number of managers that have workers with targets of the
// specified type
//
int ManagerList::ManagerCount(TargetType type)
{
	int i, mgr_count, count = 0;

	mgr_count = managers.GetSize();
	for (i = 0; i < mgr_count; i++) {
		if (IsType(managers[i]->Type(), type))
			count++;
	}
	return count;
}

//
// Return the total number of workers summed for all managers.
//
int ManagerList::WorkerCount(TargetType type)
{
	int i, count = 0, mgr_count = ManagerCount(type);

	for (i = 0; i < mgr_count; i++)
		count += GetManager(i, type)->WorkerCount(type);

	return count;
}

//
// Return the total number of targets for all workers of all managers.
//
int ManagerList::TargetCount(TargetType type)
{
	int i, count = 0, mgr_count = ManagerCount(type);

	for (i = 0; i < mgr_count; i++)
		count += GetManager(i, type)->TargetCount(type);

	return count;
}

//
// Marking all assigned targets as inactive.
//
void ManagerList::ClearActiveTargets()
{
	int i, mgr_count = ManagerCount();

	for (i = 0; i < mgr_count; i++)
		GetManager(i)->ClearActiveTargets();
}

///////////////////////////////////////////////
//
// Functions that deal with access specs.
//
///////////////////////////////////////////////

//
// Find the maximum number of access specs assigned to any worker.
//
int ManagerList::GetMaxAccessSpecCount()
{
	int i, max = 0, mgr_max, mgr_count = ManagerCount();

	for (i = 0; i < mgr_count; i++) {
		if ((mgr_max = GetManager(i)->GetMaxAccessSpecCount()) > max)
			max = mgr_max;
	}
	return max;
}

//
// Verify that all assigned configuration parameters are valid.
//
BOOL ManagerList::InvalidSetup(BOOL & invalidSpecOK)
{
	int m, w;
	Manager *mgr;
	Worker *wkr;
	BOOL nonidle_worker_with_targets = FALSE;
	BOOL all_workers_idle = TRUE;

	// Verify that at least one spec was assigned, even if Idle.
	if (!GetMaxAccessSpecCount()) {
		ErrorMessage("You must assign at least one access specification " "to a worker.");
		return TRUE;
	}
	// We need to loop through all workers of all managers to verify
	// their assigned specs are valid.
	for (m = 0; m < ManagerCount(); m++) {
		mgr = GetManager(m);
		if (mgr->InvalidSetup(invalidSpecOK)) {
			return TRUE;
		}
		// Verify that at least one worker has targets assigned with a
		// non-idle spec or all specs are idle.
		for (w = 0; w < mgr->WorkerCount(); w++) {
			wkr = mgr->GetWorker(w);

			// Keep track if all workers have only the idle spec.
			if (wkr->AccessSpecCount() != wkr->IdleAccessSpecCount()) {
				all_workers_idle = FALSE;

				// Keep track if any non-idle worker has targets.
				if (wkr->TargetCount())
					nonidle_worker_with_targets = TRUE;
			}
		}
	}

	// Verify that at least one worker has targets assigned with a
	// non-idle spec or all specs are idle.
	if (!all_workers_idle && !nonidle_worker_with_targets) {
		ErrorMessage("If no worker has targets selected, then all workers "
			     "must have the idle spec assigned.");
		return TRUE;
	}

	return FALSE;
}

//
// Assigns the default access specs to the workers based on their type.
//
void ManagerList::AssignDefaultAccessSpecs()
{
	int i, mgr_count = ManagerCount();

	for (i = 0; i < mgr_count; i++) {
		GetManager(i)->AssignDefaultAccessSpecs();
	}
}

//
// Remove all instances of an access spec from all the managers'
// workers' access spec lists.
//
void ManagerList::RemoveAccessSpec(Test_Spec * spec)
{
	int manager_count = ManagerCount();

	for (int i = 0; i < manager_count; i++)
		GetManager(i)->RemoveAccessSpec(spec);
}

//
// Remove a specific instance of an access spec from all the managers'
// workers' access spec lists (only if they are all the same).
//
void ManagerList::RemoveAccessSpecAt(int index)
{
	if (!AreAccessSpecsIdentical())
		return;

	int manager_count = ManagerCount();

	for (int i = 0; i < manager_count; i++)
		GetManager(i)->RemoveAccessSpecAt(index);
}

//
// Remove all entries from all the managers'
// workers' access spec lists.
//
void ManagerList::RemoveAllAccessSpecs()
{
	int i, mgr_count = ManagerCount();

	for (i = 0; i < mgr_count; i++)
		GetManager(i)->RemoveAllAccessSpecs();
}

//
// Adds spec[index] from global access spec list
// to all the manager's workers at position 'before_index'.
//
BOOL ManagerList::InsertAccessSpec(Test_Spec * spec, int before_index)
{
	int i, mgr_count = ManagerCount();

	for (i = 0; i < mgr_count; i++) {
		if (!GetManager(i)->InsertAccessSpec(spec, before_index))
			return FALSE;
	}
	return TRUE;
}

//
// Moves the entry at position [index] to [before_index] in the 
// workers' access spec list to all the manager's workers.
//
void ManagerList::MoveAccessSpec(int index, int before_index)
{
	int i, mgr_count = ManagerCount();

	for (i = 0; i < mgr_count; i++)
		GetManager(i)->MoveAccessSpec(index, before_index);
}

//
// Returns TRUE if any manager will be active for the current test.
//
BOOL ManagerList::ActiveInCurrentTest()
{
	int i, mgr_count = ManagerCount();

	for (i = 0; i < mgr_count; i++) {
		if (GetManager(i)->ActiveInCurrentTest())
			return TRUE;
	}
	return FALSE;
}

//
// Returns TRUE if any worker has an access spec assigned for the current
// access index and the spec is not the Idle spec.
//
BOOL ManagerList::HasActiveCurrentSpec()
{
	int i, mgr_count = ManagerCount();

	for (i = 0; i < mgr_count; i++) {
		if (GetManager(i)->HasActiveCurrentSpec())
			return TRUE;
	}
	return FALSE;
}

//
// Returns TRUE if any of the manager's workers has an idle spec assigned
// at the current access spec index.
//
BOOL ManagerList::HasIdleCurrentSpec()
{
	int i, mgr_count = ManagerCount();

	for (i = 0; i < mgr_count; i++) {
		if (GetManager(i)->HasIdleCurrentSpec())
			return TRUE;
	}
	return FALSE;
}

//
// Returns TRUE if all of the workers (on all managers) have
// the same access specification list.
//
BOOL ManagerList::AreAccessSpecsIdentical()
{
	int m, w, s, wkr_count, mgr_count, spec_count;
	Worker *compare_worker, *current_worker;
	Manager *mgr;

	// Get the first non-client worker for any manager.
	mgr_count = ManagerCount();
	for (m = 0; m < mgr_count; m++) {
		mgr = GetManager(m);
		wkr_count = mgr->WorkerCount();

		// Find the first non-client worker for this manager.
		for (w = 0; w < wkr_count; w++) {
			compare_worker = mgr->GetWorker(w);

			if (!IsType(compare_worker->Type(), GenericClientType)) {
				spec_count = compare_worker->AccessSpecCount();
				break;
			}
		}

		// See if we found a valid worker for this manager
		if (w < wkr_count)
			break;
	}

	// Did we find a worker to compare against?
	if (m == mgr_count)
		return TRUE;

	// Compare the first worker's Test_Spec to each other worker's Test_Spec.
	// Include the manager with the worker being compared with in our testing.
	for (m; m < mgr_count; m++) {
		mgr = GetManager(m);
		wkr_count = mgr->WorkerCount();

		// Only compare against non-client workers.
		for (w = 0; w < wkr_count; w++) {
			current_worker = mgr->GetWorker(w);

			// Skip network clients.
			if (IsType(current_worker->Type(), GenericClientType))
				continue;

			// If this worker doesn't have the same number of access specs, return FALSE.
			if (current_worker->AccessSpecCount() != spec_count)
				return FALSE;

			// Check to make sure each returned Test_Spec pointer is identical to (and in the
			// same order as) the pointer in the first_worker.
			for (s = 0; s < spec_count; s++) {
				if (current_worker->GetAccessSpec(s) != compare_worker->GetAccessSpec(s))
					return FALSE;
			}
		}
	}

	return TRUE;
}

//
// Sees whether all managers have the same access spec assigned at index spec_index.
// Returns a pointer to the name of the spec if all managers have the same one assigned.
// Returns a pointer to a null string (not a null pointer) otherwise.
//
// Calling function supplies pointer to a char[MAX_WORKER_NAME].
// This same pointer is returned for convenience.
//
char *ManagerList::GetCommonAccessSpec(int spec_index, char *const specname)
{
	char compare_string[MAX_WORKER_NAME];
	int manager_index;

	// Return null string if there are no managers.
	if (ManagerCount() == 0) {
		specname[0] = '\0';
		return specname;
	}
	// Find the first manager with workers.
	for (manager_index = 0; manager_index < ManagerCount(); manager_index++) {
		if (GetManager(manager_index)->WorkerCount()) {
			// Store that manager's access spec name.
			GetManager(manager_index)->GetCommonAccessSpec(spec_index, specname);
			break;
		}
	}

	if (manager_index >= ManagerCount()) {
		// Return a null string if no managers have workers.
		specname[0] = '\0';
		return specname;
	}

	manager_index++;

	// Compare the first manager's spec name to each other manager's spec name.
	while (manager_index < ManagerCount()) {
		// Return a null string if this manager's spec is different from the first manager's.
		// (Ignore this manager if it doesn't have any workers.)
		if (GetManager(manager_index)->WorkerCount()
		    && strcmp(GetManager(manager_index)->GetCommonAccessSpec(spec_index, compare_string), specname)) {
			specname[0] = '\0';
			return specname;
		}

		manager_index++;
	}

	return specname;
}

//
// Save the manager/worker configuration to the specified stream.
//              save_aspecs --> should each worker's access spec assignments be saved?
//              save_targets --> should each worker's target assignments be saved?
//
BOOL ManagerList::SaveConfig(ostream & outfile, BOOL save_aspecs, BOOL save_targets)
{
	int mgr_count = ManagerCount();

	outfile << "'MANAGER LIST ==========================" "========================================" << endl;

	for (int counter = 0; counter < mgr_count; counter++)
		if (!GetManager(counter)->SaveConfig(outfile, save_aspecs, save_targets))
			return FALSE;	// error saving data - propagate back

	outfile << "'END manager list" << endl;

	return TRUE;
}

//
// This first pass through the load file maps saved managers to the managers
// currently running in Iometer.  Any managers that aren't found in Iometer
// are added to the "waiting list".
//
BOOL ManagerList::LoadConfigPreprocess(const CString & infilename, BOOL * flags, BOOL replace)
{
	ICF_ifstream infileForCount(infilename);
	CString key, value;
	CString mgr_name, mgr_network;
	int mgr_id, counter;
	Manager *mgr;
	CArray < BOOL, int >mgr_flag;
	int mgr_count = ManagerCount();

	// Set the array size, initialize to FALSE.
	for (counter = 0; counter < mgr_count; counter++)
		mgr_flag.Add(FALSE);

	// Skip into the MANAGER LIST section.
	// If it can't find it, return TRUE.  (This is okay.)
	if (!infileForCount.SkipTo("'MANAGER LIST"))
		return TRUE;

	//Count the # of managers in the .icf file being processed.
	int icfManagerCount = 0;

	while (infileForCount.SkipTo("'Manager ID, manager name")) {
		// Read the manager info from the config file.
		if (!GetManagerInfo(infileForCount, mgr_name, mgr_id, mgr_network))
			return FALSE;
		icfManagerCount++;
	}

	// "Rewind" the .icf file and re-position to start of "MANAGER LIST" section.
	ICF_ifstream infile(infilename);

	infile.SkipTo("'MANAGER LIST");

	while (infile.SkipTo("'Manager ID, manager name")) {
		// Read the manager info from the config file.
		if (!GetManagerInfo(infile, mgr_name, mgr_id, mgr_network))
			return FALSE;

		// Identify the first unused manager matching these specs
		for (counter = 0; counter < mgr_count; counter++) {
			// For a saved manager's settings to be restored, a manager by the
			// same name, connecting to Iometer via the same network address,
			// must be currently available in Iometer.
			//
			// The mgr_id is used to distinguish between managers with the
			// same names.

			mgr = GetManager(counter);
			if (mgr_name.CompareNoCase(mgr->name) == 0
			    && mgr_network.CompareNoCase(mgr->network_name) == 0 && !mgr_flag[counter]) {
				mgr_flag[counter] = TRUE;	// mark this off
				loadmap.Store(mgr_name, mgr_id, mgr_network, mgr);
				break;
			}
		}

		// Did we go past the end without a match?
		if (counter >= mgr_count) {
			if (mgr_count == 0 && icfManagerCount == 1) {
				// Special case:  No managers logged in, and exactly one in the file.
				// Create a special entry with a null address and a special local
				// manager name.  This special entry is used to identify the need to
				// create a local manager in ManagerMap::SpawnLocalManager(), and to
				// match against a local manager in ManagerMap::ManagerLoggedIn() and
				// ManagerMap::ManagerRetrieve().
				loadmap.Store(HOSTNAME_LOCAL, 1, "", NULL);
			} else {
				// Store an incomplete entry (NULL mgr ptr) in the manager map.
				// This will put the load operation in a waiting state.
				loadmap.Store(mgr_name, mgr_id, mgr_network, NULL);
			}
		}
	}

	infile.close();

	// If there is exactly one manager in Iometer and one manager
	// in the ManagerMap, they should be mapped together.
	if (mgr_count == 1 && mgr_flag[0] == FALSE) {
		mgr = GetManager(0);

		// "SetIfOneManager" means "if there is one manager in the
		//   load map and it is unassigned, assign it this pointer"
		loadmap.SetIfOneManager(mgr);
	}
	// Spawn any local managers that are missing.
	loadmap.SpawnLocalManagers();

	return TRUE;
}

//
// Restore the manager/worker configuration from the specified filename.
//              load_aspecs --> should each worker's access spec assignments be loaded?
//              load_targets --> should each worker's target assignments be loaded?
//              replace --> should these settings...
//                                              ...overwrite the current worker settings (TRUE)
//                                              ...or be merged with current worker settings (FALSE)
//
// Returns TRUE on success, FALSE if any of the requested information
// couldn't be found in the file.
//
// If replace is TRUE, it also removes managers that weren't affected by the
// file restore (those that weren't specified in the config file).
//
BOOL ManagerList::LoadConfig(const CString & infilename, BOOL load_aspecs, BOOL load_targets, BOOL replace)
{
	ICF_ifstream infile;
	long version;
	CString key, value;
	int counter;
	Manager *mgr;
	int mgr_count;

	infile.open(infilename);
	version = infile.GetVersion();
	if (version == -1)
		return FALSE;

	// Skip into the MANAGER LIST section.
	// If it can't find it, return TRUE.  (This is okay.)
	if (!infile.SkipTo("'MANAGER LIST"))
		return TRUE;

	mgr_count = ManagerCount();

	// If we're replacing everything, go through each
	// of the managers and remove all of its workers.
	if (replace) {
		for (counter = 0; counter < mgr_count; counter++) {
			mgr = GetManager(counter);

			while (mgr->WorkerCount())
				theApp.pView->m_pWorkerView->RemoveWorker(mgr->GetWorker(0));
		}
	}

	while (1) {
		key = infile.GetNextLine();

		if (key.CompareNoCase("'END manager list") == 0) {
			break;
		} else if (key.CompareNoCase("'Manager ID, manager name") == 0) {
			CString mgr_name, mgr_netaddr;
			int mgr_id;

			// Read the manager info from the config file.
			if (!GetManagerInfo(infile, mgr_name, mgr_id, mgr_netaddr))
				return FALSE;

			// Gets the pointer to the specified manager from the ManagerMap.
			mgr = loadmap.Retrieve(mgr_name, mgr_id);

			if (mgr == NULL) {
				ErrorMessage("Manager could not be found in the ManagerMap.  "
					     "Please report this as an Iometer bug.");
				return FALSE;
			}
			// Load the manager.
			if (!mgr->LoadConfig(infile, load_aspecs, load_targets))
				return FALSE;
		} else {
			ErrorMessage("File is improperly formatted.  Expected another "
				     "manager or \"End manager list\" comment.");
			return FALSE;
		}
	}

	infile.close();

	// If a manager connected to Iometer wasn't mentioned in the config file, remove it.
	if (replace) {
		for (counter = mgr_count - 1; counter >= 0; counter--) {
			if (!theApp.manager_list.loadmap.IsThisManagerNeeded(GetManager(counter)))
				theApp.pView->m_pWorkerView->RemoveManager(GetManager(counter));
		}
	}

	return TRUE;
}

//
// Disambiguate same-named managers.  This allows managers
// to be uniquely identified in saved files and other places.
// This should be called any time managers are added, removed, or renamed.
//
void ManagerList::IndexManagers()
{
	Manager *mgr, *othermgr;
	int count_up, count_down;
	const int mgr_count = ManagerCount();

	for (count_up = 0; count_up < mgr_count; count_up++) {
		mgr = GetManager(count_up);
		mgr->id = 1;	// Assume this is unique, unless proven otherwise.

		for (count_down = count_up - 1; count_down >= 0; count_down--) {
			othermgr = GetManager(count_down);

			if (CString(mgr->name).CompareNoCase(othermgr->name) == 0) {
				// If the managers' names are identical, assign
				// this manager an ID value one greater.
				mgr->id = othermgr->id + 1;
				break;
			}
		}
	}
}

//
// Retrieves the manager name, network address, and discriminator value from an
// the infile.  Expects the file pointer to be immediately after the newline
// following a "'Manager ID, manager name'" comment.
//
// Return value of FALSE indicates an error.  The calling function
// should NOT report an error.  Error reporting is handled here.
//
BOOL ManagerList::GetManagerInfo(ICF_ifstream & infile, CString & manager_name, int &id, CString & network_name)
{
	CString key = "";
	CString value = "";

	value = infile.GetNextLine();

	if (value.IsEmpty()) {
		ErrorMessage("File is improperly formatted.  " "Error retrieving manager name or empty manager name.");
		return FALSE;
	}

	if (!ICF_ifstream::ExtractFirstInt(value, id)) {
		ErrorMessage("File is improperly formatted.  "
			     "Error retrieving manager ID.  This value must be an integer.");
		return FALSE;
	}

	manager_name = value;

	if (!infile.GetPair(key, value)) {
		ErrorMessage("File is improperly formatted.  " "Error retrieving manager network address.");
		return FALSE;
	}

	if (key.CompareNoCase("'Manager network address") != 0) {
		ErrorMessage("File is improperly formatted.  Expected a \"Manager network "
			     "address\" comment after manager ID.");
		return FALSE;
	}

	network_name = value;

	return TRUE;
}
