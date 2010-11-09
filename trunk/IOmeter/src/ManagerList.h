/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Iometer / ManagerList.h                                            ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Interface for Iometer's ManagerList class, which      ## */
/* ##               contains a list of managers and provides functions    ## */
/* ##               for manager-list actions.                             ## */
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
#ifndef MANAGER_LIST_DEFINED
#define MANAGER_LIST_DEFINED

#include "IOCommon.h"
#include "Manager.h"
#include "ManagerMap.h"
#include "WaitingForManagers.h"

// Class information for a list of managers.
class ManagerList {
      public:
	ManagerList();
	~ManagerList();
	void Reset();

	///////////////////////////////////////////////////////////////////////////
	// Information about the manager list instance.
	//
	char name[MAX_WORKER_NAME];	// Displayable name of list.
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Managing list of managers.
	//
	Manager *GetManager(int index, TargetType type = GenericType);
	Manager *GetManagerByName(const char *mgr_name, const int mgr_id);
	//
	// Updating worker information.
	Manager *AddManager(Manager_Info * manager_info);
	BOOL RemoveManager(int index, int purpose = EXIT);
	void RemoveAllManagers(int purpose = EXIT);
	//
	int TargetCount(TargetType type = GenericType);
	int WorkerCount(TargetType type = GenericType);
	int ManagerCount(TargetType type = GenericType);
	//
	// Disambiguate same-named managers.  This allows managers
	// to be uniquely identified in saved files and other places.
	// This should be called any time managers are added, removed, or renamed.
	void IndexManagers();
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Managing targets of manager's workers.
	void ClearActiveTargets();
	BOOL SetTargets();
	//
	BOOL ActiveInCurrentTest();
	BOOL HasActiveCurrentSpec();
	BOOL HasIdleCurrentSpec();
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Managing communication with corresponding Dynamos.
	void SendManagers(Message * msg, int recipient = MANAGER);
	void SendManagers(int purpose, int recipient = MANAGER);
	DWORDLONG Send(int manager_index, int purpose, int recipient = MANAGER);
	DWORDLONG Send(int manager_index, Message * msg, int recipient = MANAGER);
	BOOL SendActiveManagers(int purpose);
	DWORDLONG ReceiveManager(int index, Message * msg);
	void ReceiveManagers();
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Functions to set/retrieve worker target information
	int GetDataPattern(TargetType type);
	int GetConnectionRate(TargetType type);
	DWORDLONG GetDiskStart(TargetType type);
	DWORDLONG GetDiskSize(TargetType type);
	int GetQueueDepth(TargetType type);
	int GetMaxSends(TargetType type);
	int GetTransPerConn(TargetType type);
	int GetUseFixedSeed(TargetType type);
	DWORDLONG GetFixedSeedValue(TargetType type);
	//
	void SetDataPattern(int data_pattern, TargetType type);
	void SetConnectionRate(BOOL connection_rate, TargetType type);
	void SetDiskSize(DWORDLONG disk_size);
	void SetDiskStart(DWORDLONG disk_start);
	void SetQueueDepth(int queue_size, TargetType type);
	void SetMaxSends(int max_sends);
	void SetTransPerConn(int trans_per_conn, TargetType type);
	void SetUseFixedSeed(BOOL use_fixed_seed, TargetType type);
	void SetFixedSeedValue(DWORDLONG fixed_seed_value, TargetType type);
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Functions to deal with access specs.
	//
	BOOL InsertAccessSpec(Test_Spec * spec, int before_index);
	void RemoveAccessSpec(Test_Spec * spec);
	void RemoveAccessSpecAt(int index);
	void RemoveAllAccessSpecs();
	//
	void MoveAccessSpec(int index, int before_index);
	void DownAccessSpec(int index);
	void UpAccessSpec(int index);
	//
	BOOL SetAccess(int spec_index);
	BOOL InvalidSetup(BOOL & invalidSpecOK);
	int GetMaxAccessSpecCount();
	void AssignDefaultAccessSpecs();
	//
	// Returns TRUE if all of the workers (on all managers) have
	// the same access specification list.
	BOOL AreAccessSpecsIdentical();
	//
	// Gets the name of the current access spec for all workers.
	char *GetCommonAccessSpec(int spec_index, char *const specname);
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Results from a test.
	//
	// Functions to deal with results.
	void ResetResults(int which_perf);
	void ResetAllResults();
	void SaveResults(ostream * file, int access_index, int result_type);
	void UpdateResults(int which_perf);
	//
	// Results for the last run test.
	Results results[MAX_PERF];
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Saving and loading of Iometer Configuration Files (ICF's)
	//
	ManagerMap loadmap;
	BOOL SaveConfig(ostream & outfile, BOOL save_aspecs, BOOL save_targets);
	BOOL LoadConfigPreprocess(const CString & infilename, BOOL * flags, BOOL replace);
	BOOL LoadConfig(const CString & infilename, BOOL save_aspecs, BOOL save_targets, BOOL replace);
      private:
	 BOOL GetManagerInfo(ICF_ifstream & infile, CString & manager_name, int &id, CString & network_name);
	//
	///////////////////////////////////////////////////////////////////////////

      protected:
	 BOOL CompareNames(char *net_name1, char *net_name2);

	 CTypedPtrArray < CPtrArray, Manager * >managers;
};

#endif
