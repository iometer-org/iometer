/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / AccessSpecList.h                                       ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Interface for the AccessSpecList class, which is      ## */
/* ##               in charge of keeping track of all the access          ## */
/* ##               specifications available to the user.                 ## */
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
/* ##  Changes ...: 2004-04-24 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added the InsertDefaultSpecs() method.              ## */
/* ##               2003-10-17 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef ACCESS_LIST_DEFINED
#define ACCESS_LIST_DEFINED

#include "IOAccess.h"
#include "IOTest.h"
#include "ICF_ifstream.h"
#include <afxtempl.h>

#define IDLE_SPEC		0	// The index of the idle spec.
#define	IDLE_NAME		"Idle"	// The name of the idle spec.

// The AccessSpecList controls the addition and removal of the global access specs.
class AccessSpecList {
      public:
// Member Functions     
	AccessSpecList();
	~AccessSpecList();

	Test_Spec *New();	// Creates a new Test_Spec in memory,
	// and adds a pointer to it in the SpecList array.
	// Returns the index to the entry in the pointer array
	// if successful.

	Test_Spec *Copy(Test_Spec * source_spec);	// Creates a copy of the 
	// specified access spec.

	void Delete(Test_Spec * spec);	// Removes the given pointer to a 
	// Test_Spec from the SpecList array, 
	// and removes the associated spec.
	// Returns True if successful.

	Test_Spec *Get(int index);	// Returns the pointer to the Test_Spec at the provided index.

	int Count();		// Return the number of entries in the array.

	int IndexByRef(const Test_Spec * spec);	// Returns the index of the given access spec.
	Test_Spec *RefByName(const char *check_name);	// Checks the access spec list for a spec of the given name.

	BOOL SaveResults(ostream & outfile);	// Saves only the currently active access specs to a result file.

	BOOL SaveConfig(ostream & outfile);	// Saves all access specs to a config file.
	BOOL LoadConfig(const CString & infilename, BOOL replace);	// Gets the file version and calls the appropriate LoadConfigXXX function.
	BOOL LoadConfigNew(ICF_ifstream & infile);	// Loads the global access spec list from a current version's setup file.
	BOOL LoadConfigOld(ICF_ifstream & infile);	// Loads the global access spec list from a previous version's setup file.

      protected:
// Private Functions
	void InsertIdleSpec();	// Creates the idle spec and inserts it's pointer into the poiter array.
	void InsertDefaultSpecs();	// Inserts the different default specifications.

	void DeleteAll();	// Deletes all entries in the access spec list.

	void InitAccessSpecLine(Access_Spec * spec_line);	//Initializes a line of an access spec to the default values.

	void SmartName(Test_Spec * spec);	//Takes an access spec and assigns it a descriptive name
	void NextUntitled(char *name);	// Name an untitled access specs ("Untitled n").

// Private data structures
	 CTypedPtrArray < CPtrArray, Test_Spec * >spec_list;	// Pointer array to AccessSpecObjects
};

#endif
