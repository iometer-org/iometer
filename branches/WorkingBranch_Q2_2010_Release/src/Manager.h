/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Iometer / Manager.h                                                ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Interface for Iometer's Manager class, which contains ## */
/* ##               a list of workers and provides functions for manager- ## */
/* ##               level actions.                                        ## */
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
/* ##               2003-03-01 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Cut out the Windows Pipes support for               ## */
/* ##                 communication efforts.                              ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef MANAGER_DEFINED
#define MANAGER_DEFINED

#include "IOCommon.h"
#include "Worker.h"
#include "IOPort.h"
#include <afxtempl.h>

// Class information for a single Manager
class Manager {
      public:
	Manager();
	~Manager();

	///////////////////////////////////////////////////////////////////////////
	// Information about an instance of a manager.
	//
	char name[MAX_WORKER_NAME];	// Displayable name of Manager.
	char computer_name[MAX_NETWORK_NAME];	// Computer this manager is running on.
	char network_name[MAX_NETWORK_NAME];	// Network address of the computer.
	//
	// Note that network_name is derived from the address Dynamo is connecting from,
	// but it should NOT be used to refer to the Dynamo link.
	//
	// Uniquely distinguishes managers with the same name.
	// To fill in this value, call ManagerList::IndexManagers().
	// If any managers are renamed, added, or removed, call
	// ManagerList::IndexManagers().
	int id;
	//
	// Return a manager's index into the manager list.
	int GetIndex();
	TargetType Type();
	//
	double	timer_resolution;		// Speed of manager's system high performance times in tic\sec or processor frequency.
	int		processors;		// Number of processors in manager's system.
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Interfaces available to the manager.  A worker is assigned a target
	// that is accessed through one of these interfaces.
	//
	Target_Spec *GetInterface(int index, TargetType type);
	int InterfaceCount(TargetType type);
	//
	// Removes cached interface information and all associated worker targets.
	void RemoveDiskInfo();
	void RemoveNetInfo();
	//
      protected:
	 CTypedPtrArray < CPtrArray, Target_Spec * >disks;
	 CTypedPtrArray < CPtrArray, Target_Spec * >tcps;
	 CTypedPtrArray < CPtrArray, Target_Spec * >vis;
	//
	void InitTargetList(CTypedPtrArray < CPtrArray, Target_Spec * >*targets);
	//
	///////////////////////////////////////////////////////////////////////////

      public:
	///////////////////////////////////////////////////////////////////////////
	// Retrieving/setting information about a worker's targets.
	//
	BOOL GetUseRandomData(TargetType type);
	int GetConnectionRate(TargetType type);
	DWORDLONG GetDiskStart(TargetType type);
	DWORDLONG GetDiskSize(TargetType type);
	char *GetLocalNetworkInterface();
	int GetQueueDepth(TargetType type);
	int GetMaxSends(TargetType type);
	int GetTransPerConn(TargetType type);
	//
	void SetUseRandomData(BOOL use_random_data, TargetType type);
	void SetConnectionRate(BOOL test_connection_rate, TargetType type);
	void SetDiskSize(DWORDLONG disk_size);
	void SetDiskStart(DWORDLONG disk_start);
	void SetLocalNetworkInterface(int iface_index);
	void SetQueueDepth(int queue_depth, TargetType type);
	void SetMaxSends(int max_sends);
	void SetTransPerConn(int trans_per_conn, TargetType type);
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Manages a manager's workers.
	//
	Worker *GetWorker(int index, TargetType type = GenericType);
	Worker *GetWorkerByName(const char *wkr_name, const int wkr_id);
	int WorkerCount(TargetType type = GenericType);
	//
	Worker *AddWorker(TargetType type, Worker * src_worker = NULL, const CString & in_name = "");
	//
	BOOL RemoveWorker(int index, TargetType type = GenericType);
	//
	// Disambiguate same-named managers.  This allows managers
	// to be uniquely identified in saved files and other places.
	// This should be called any time managers are added, removed, or renamed.
	void IndexWorkers();
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Manages a manager's workers' targets.
	//
	void UpdateTargetLists();
	//
	int TargetCount(TargetType type = GenericType);
	//
	int SetActiveTargets(int worker_index, int active_target_count);
	void ClearActiveTargets();
	void RemoveTargets(TargetType type = GenericType);
	//
	BOOL ActiveInCurrentTest();
	BOOL HasActiveCurrentSpec();
	BOOL HasIdleCurrentSpec();
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Manage communications with corresponding Dynamo.
	//
	DWORD Peek(int worker_index);
	DWORDLONG Send(int data, int purpose);
	DWORDLONG Send(Message * msg);
	DWORDLONG SendData(Data_Message * msg);
	DWORDLONG Receive(Message * msg);
	DWORDLONG ReceiveData(Data_Message * msg);
	DWORDLONG Receive();
	//
	Port *port;
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Functions to deal with specific messages to Dynamo
	//
	BOOL SetAccess(int access_index);
	BOOL SetTargets();
	void SetTargetsToPrepare(int worker_index);
	BOOL PreparedAnswer(int worker_index);
	//
	// Functions to deal with results.
	void ResetResults(int which_perf);
	void ResetAllResults();
	void SaveResults(ostream * file, int access_index, int result_type);
	void UpdateResults(int which_perf);
	//
	// Results for the last run test
	Results results[MAX_PERF];
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Functions to deal with access specs.
	//
	void AssignDefaultAccessSpecs();
	void RemoveAccessSpec(Test_Spec * spec);
	void RemoveAccessSpecAt(int index);
	void RemoveAllAccessSpecs();
	int GetMaxAccessSpecCount();
	//
	void MoveAccessSpec(int index, int before_index);
	void UpAccessSpec(int index);
	void DownAccessSpec(int index);
	BOOL InsertAccessSpec(Test_Spec * spec, int before_index);
	//
	// Returns a TRUE if all of this manager's workers
	// have the same access specification list.
	BOOL AreAccessSpecsIdentical();
	// Gets the name of the current access spec for all managers.
	char *GetCommonAccessSpec(int spec_index, char *const specname);
	//
	BOOL InvalidSetup(BOOL & invalidSpecOK);
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Functions to deal with the saving and loading of config files.
	//
	BOOL SaveConfig(ostream & outfile, BOOL save_aspecs, BOOL save_targets);
	BOOL LoadConfig(ICF_ifstream & infile, BOOL load_aspecs, BOOL load_targets);
	//
	///////////////////////////////////////////////////////////////////////////

      protected:
	 CTypedPtrArray < CPtrArray, Worker * >workers;
};

#endif
