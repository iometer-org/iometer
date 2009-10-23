/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / Worker.h                                               ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Interface for Iometer's Worker class, which is in     ## */
/* ##               charge of keeping results for each worker, as well    ## */
/* ##               as maintaining the list of currently-assigned targets ## */
/* ##               for each worker.                                      ## */
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
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef WORKER_DEFINED
#define WORKER_DEFINED

#include "IOCommon.h"
#include "IOMessage.h"
#include "IOTransfers.h"
#include "ICF_ifstream.h"
#include <afxtempl.h>

class Manager;			// forward declaration

#ifdef FORCE_STRUCT_ALIGN
#include "pack8.h"
#endif

//
// Structure to store information about a worker's targets.
//
struct Target {
	Target_Spec spec;

	// Results of last run test.
	Results results[MAX_PERF];
};

#ifdef FORCE_STRUCT_ALIGN
#include "unpack8.h"
#endif

#define AMBIGUOUS_VALUE	-1
#define DISABLED_VALUE	0
#define ENABLED_VALUE	1

// Class information for a single worker.
class Worker {
      public:
	Worker(Manager * mgr = NULL, TargetType wkr_type = GenericType);
	~Worker();
	// Copies all settings from specified worker.
	void Clone(Worker * source_worker);

	// Uniquely distinguishes workers with the same name on the
	// same manager.  To fill in this value, call Manager::IndexWorkers().
	// If any workers are renamed, added, or removed, call its
	// manager's Manager::IndexWorkers().
	int id;

	///////////////////////////////////////////////////////////////////////////
	// Information about an instance of a worker.
	//
	char name[MAX_WORKER_NAME];	// Displayable name of worker.
	Manager *manager;	// Worker's manager.
	//
	TargetType Type();	// What kind of worker is this?
	//
	// Return a worker's index into its manager's worker list.
	int GetIndex(TargetType list_type = GenericType);
	//
	// Settings worker uses for all its targets by default.
	// Currently all targets for a worker have the same type and settings.
	// This allows the worker to remember its settings, even if it all of 
	// its targets are removed.
	Target_Spec spec;
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Manages a worker's targets.
	//
	Target *GetTarget(int index, TargetType type = GenericType);
	int TargetCount(TargetType type = GenericType);
	BOOL IsTargetAssigned(Target_Spec * target_info);
	//
	void AddTarget(Target_Spec * target_info);
	//
	void RemoveTarget(int index);
	void RemoveTargets(TargetType target_type = GenericType);
	//
	// List of targets belonging to a worker.
	 CTypedPtrArray < CPtrArray, Target * >targets;
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Returns information about a worker's current test setup.
	//
	int MaxOutstandingIOs(int target_index);
	DWORD MaxTransferSize();
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Retrieving/setting information about a worker's targets.
	//
	int GetConnectionRate(TargetType type);
	DWORDLONG GetDiskSize(TargetType type);
	DWORDLONG GetDiskStart(TargetType type);
	char *GetLocalNetworkInterface();
	int GetQueueDepth(TargetType type);
	int GetMaxSends(TargetType type);
	int GetTransPerConn(TargetType type);
	// GetValue will replace most Get*() functions.
	//      int                     GetValue( TargetType type, int offset );
	//
	void SetConnectionRate(BOOL test_connection_rate);
	void SetDiskSize(DWORDLONG disk_size);
	void SetDiskStart(DWORDLONG disk_start);
	void SetLocalNetworkInterface(int iface_index, TargetType type = GenericNetType);
	void SetLocalNetworkInterface(const CString & iface_name);
	void SetQueueDepth(int queue_depth);
	void SetMaxSends(int queue_depth);
	void SetTransPerConn(int trans_per_conn);
	//      SetValue will replace most Set*() functions.
	//      void            SetValue( TargetType type, int offset, int value );
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Functions dealing with managing which targets are running
	//
	int SetActiveTargets(int targets_to_set);
	void ClearActiveTargets();
	//
	BOOL ReadyToRunTests();
	BOOL TargetReadyToRunTests(Target * target);
	//
	BOOL ActiveInCurrentTest();
	BOOL HasActiveCurrentSpec();
	BOOL HasIdleCurrentSpec();
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Functions to deal with specific messages to Dynamo
	//
	BOOL SetTargets();
	void SetTargetsToPrepare();
	BOOL SetAccess(int access_entry);
	//
	// Functions to deal with results.
	void ResetResults(int which_perf);
	void ResetAllResults();
	void SaveResults(ostream * file, int access_index, int result_type);
	void UpdateResults(int which_perf);
	//
	// Results for last run test.
	Results results[MAX_PERF];
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Functions to deal with the saving and loading of config files.
	//
	BOOL SaveConfig(ostream & outfile, BOOL save_aspecs, BOOL save_targets);
	BOOL LoadConfig(ICF_ifstream & infile, BOOL load_aspecs, BOOL load_targets);
	BOOL LoadConfigDefault(ICF_ifstream & infile);
	BOOL LoadConfigAccess(ICF_ifstream & infile);
	BOOL LoadConfigTargets(ICF_ifstream & infile);
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Functions to manage access specs
	//
	// Removes all instances of an access spec from the worker's access spec list.
	void RemoveAccessSpec(Test_Spec * spec);
	//
	// Removes a specific instance of an access spec from the worker's access spec list.
	void RemoveAccessSpecAt(int index);
	//
	// Removes all entries from the worker's access spec list.
	void RemoveAllAccessSpecs();
	//
	// Returns the number of entries in the worker's access spec list.
	int AccessSpecCount();
	int IdleAccessSpecCount();
	//
	// Returns the pointer to the access spec object specified by the index into 
	// the global access spec list.  The index into the global access spec list 
	// is found in the specified entry in the local access spec list.
	Test_Spec *GetAccessSpec(int index);
	//
	// Moves the access spec at 'index' to the position before 'before_index'.
	void MoveAccessSpec(int index, int before_index);
	//
	// Inserts the access spec 'index' to the position before 'local_index'.
	BOOL InsertAccessSpec(Test_Spec * spec, int insert_at_index = IOERROR);
	//
	// Determines if all assigned test parameters are valid for this worker.
	BOOL InvalidSetup(BOOL & invalidSpecOK);
	//
	// Assigns all the specs from the global list that match the  worker's type.
	void AssignDefaultAccessSpecs();
	//      
      protected:
	 CTypedPtrArray < CPtrArray, Test_Spec * >access_spec_list;	// Pointer array to AccessSpecObjects
	//
	///////////////////////////////////////////////////////////////////////////

      public:
	///////////////////////////////////////////////////////////////////////////
	// Network specific information.
	//
	// Create a client worker to be my partner.
	void CreateNetClient(Manager * selected_manager, TargetType client_type);
	//
	// Worker on the other end of network connection.
	Worker *net_partner;
	//
	// Client's index into server's target array.
	int net_target_index;
	//
	///////////////////////////////////////////////////////////////////////////
};

#endif
