/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / ManagerMap.h                                           ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Interface for Iometer's ManagerMap class, which       ## */
/* ##               maintains the mapping between managers in an .ICF     ## */
/* ##               file and the current Manager objects.                 ## */
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
/* ##  Remarks ...: - This class is used when Iometer Configuration Files ## */
/* ##                 (ICF files) are being restored. Managers from the   ## */
/* ##                 file (identified by name and a disambiguating ID    ## */
/* ##                 number) are mapped to the actual running Managers   ## */
/* ##                 by the ManagerList at file load time.               ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Changes ...: 2003-10-17 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-04-25 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Updated the global debug flag (_DEBUG) handling     ## */
/* ##                 of the source file (check for platform etc.).       ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef MANAGERMAP_DEFINED
#define MANAGERMAP_DEFINED

#include "Manager.h"
#include "WaitingForManagers.h"

struct ManagerMapEntry {
	CString name;
	int id;
	CString address;
	Manager *mgr;
};

class ManagerMap {
      private:
	CArray < ManagerMapEntry, int >map;
	CWaitingForManagers waiting_dialog;

      public:
	 ManagerMap();

	// Prepares the ManagerMap for reuse.
	void Reset();

	// Add a manager to the list.  (mgr pointer, if unknown, can be NULL)
	void Store(const CString & name, const int id, const CString & address, Manager * mgr);

	// Retrieve a manager pointer matching the specified criteria
	Manager *Retrieve(const CString & name, const int id);

	// When a manager logs in, call this function to find the first matching
	// entry in the map and store the new manager's pointer there.
	BOOL ManagerLoggedIn(const CString & name, const CString & address, Manager * mgr);

	// If there is only one manager in the ManagerMap and it is unassigned
	// (mgr pointer is NULL), assign it the given manager pointer, return TRUE.
	// Otherwise, return FALSE.  (Yes, awkward name...)
	BOOL SetIfOneManager(Manager * mgr);

	// Find all unassigned managers (mgr ptrs are NULL) with the local
	// network address and spawn Dynamos with the appropriate names.
	void SpawnLocalManagers();

	// Are we waiting for any managers to log in before restoring
	// the manager configuration from a saved file?
	BOOL IsWaitingList();

	// Show the waiting list dialog box, return immediately.
	void ShowWaitingList(const CString & infilename, BOOL * flags, BOOL replace);

	// See the manager at this address is found in the manager map.  Used to
	// determine whether this manager should be kept (TRUE) or disconnected
	// (FALSE) after the config file manager settings are restored.
	BOOL IsThisManagerNeeded(const Manager * const mgr);
};

#endif
