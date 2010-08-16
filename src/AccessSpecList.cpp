/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / AccessSpecList.cpp                                     ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implementation of the AccessSpecList class that       ## */
/* ##               is in charge of keeping track of all the access       ## */
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
/* ##  Remarks ...: - Each access spec is defined by a Test_Spec          ## */
/* ##                 structure. There is always exactly one              ## */
/* ##                 AccessSpecList object in an instance of Iometer     ## */
/* ##                 (CGalileoApp::access_spec_list).                    ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Changes ...: 2004-04-24 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added a large set of Global Access Specifications   ## */
/* ##                 that can be used as a starting point. The values    ## */
/* ##                 was taken from the same ICF's provided by           ## */
/* ##                 Richard Riggs.                                      ## */
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
#include "AccessSpecList.h"
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
#ifdef IOMTR_SETTING_MFC_MEMALLOC_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

//
// Initializing the pointer list.
//
AccessSpecList::AccessSpecList()
{
	// Initialize the pointer array.
	spec_list.SetSize(INITIAL_ARRAY_SIZE, ARRAY_GROW_STEP);

	// Insert the idle spec as the first in the row
	InsertIdleSpec();

	// Insert a default spec that will be assigned
	// (in former days, this one was assigned to all workers that log in)
	Test_Spec *spec = New();

	strcpy(spec->name, "Default");
	spec->default_assignment = FALSE;	// [AssignAll|AssignDisk|...]

	// Insert the default specifications
	// (based on the file provided by Richard Riggs)
	InsertDefaultSpecs();
}

//
// Inserts the idle spec.
//
void AccessSpecList::InsertIdleSpec()
{
	// Set idle spec.
	Test_Spec *spec = New();

	_snprintf(spec->name, MAX_NAME, IDLE_NAME);
	spec->access[0].of_size = IOERROR;	// indicates the end of the spec.
	// Note that the end is the first
	// line of the spec, making it
	// empty.
}

//
// Inserts the different default specifications.
//
void AccessSpecList::InsertDefaultSpecs()
{
	Test_Spec *spec;

	// 512 Bytes

	spec = New();
	_snprintf(spec->name, MAX_NAME, "512B; 100%% Read; 0%% random");
	spec->access[0].of_size = 100;
	spec->access[0].size = 512;
	spec->access[0].reads = 100;
	spec->access[0].random = 0;
	spec->access[0].delay = 0;
	spec->access[0].burst = 1;
	spec->access[0].align = 0;
	spec->access[0].reply = 0;

	spec = New();
	_snprintf(spec->name, MAX_NAME, "512B; 75%% Read; 0%% random");
	spec->access[0].of_size = 100;
	spec->access[0].size = 512;
	spec->access[0].reads = 75;
	spec->access[0].random = 0;
	spec->access[0].delay = 0;
	spec->access[0].burst = 1;
	spec->access[0].align = 0;
	spec->access[0].reply = 0;

	spec = New();
	_snprintf(spec->name, MAX_NAME, "512B; 50%% Read; 0%% random");
	spec->access[0].of_size = 100;
	spec->access[0].size = 512;
	spec->access[0].reads = 50;
	spec->access[0].random = 0;
	spec->access[0].delay = 0;
	spec->access[0].burst = 1;
	spec->access[0].align = 0;
	spec->access[0].reply = 0;

	spec = New();
	_snprintf(spec->name, MAX_NAME, "512B; 25%% Read; 0%% random");
	spec->access[0].of_size = 100;
	spec->access[0].size = 512;
	spec->access[0].reads = 25;
	spec->access[0].random = 0;
	spec->access[0].delay = 0;
	spec->access[0].burst = 1;
	spec->access[0].align = 0;
	spec->access[0].reply = 0;

	spec = New();
	_snprintf(spec->name, MAX_NAME, "512B; 0%% Read; 0%% random");
	spec->access[0].of_size = 100;
	spec->access[0].size = 512;
	spec->access[0].reads = 0;
	spec->access[0].random = 0;
	spec->access[0].delay = 0;
	spec->access[0].burst = 1;
	spec->access[0].align = 0;
	spec->access[0].reply = 0;

	// 4096 Bytes / 4 Kilo Bytes

	spec = New();
	_snprintf(spec->name, MAX_NAME, "4K; 100%% Read; 0%% random");
	spec->access[0].of_size = 100;
	spec->access[0].size = 4096;
	spec->access[0].reads = 100;
	spec->access[0].random = 0;
	spec->access[0].delay = 0;
	spec->access[0].burst = 1;
	spec->access[0].align = 0;
	spec->access[0].reply = 0;

	spec = New();
	_snprintf(spec->name, MAX_NAME, "4K; 75%% Read; 0%% random");
	spec->access[0].of_size = 100;
	spec->access[0].size = 4096;
	spec->access[0].reads = 75;
	spec->access[0].random = 0;
	spec->access[0].delay = 0;
	spec->access[0].burst = 1;
	spec->access[0].align = 0;
	spec->access[0].reply = 0;

	spec = New();
	_snprintf(spec->name, MAX_NAME, "4K; 50%% Read; 0%% random");
	spec->access[0].of_size = 100;
	spec->access[0].size = 4096;
	spec->access[0].reads = 50;
	spec->access[0].random = 0;
	spec->access[0].delay = 0;
	spec->access[0].burst = 1;
	spec->access[0].align = 0;
	spec->access[0].reply = 0;

	spec = New();
	_snprintf(spec->name, MAX_NAME, "4K; 25%% Read; 0%% random");
	spec->access[0].of_size = 100;
	spec->access[0].size = 4096;
	spec->access[0].reads = 25;
	spec->access[0].random = 0;
	spec->access[0].delay = 0;
	spec->access[0].burst = 1;
	spec->access[0].align = 0;
	spec->access[0].reply = 0;

	spec = New();
	_snprintf(spec->name, MAX_NAME, "4K; 0%% Read; 0%% random");
	spec->access[0].of_size = 100;
	spec->access[0].size = 4096;
	spec->access[0].reads = 0;
	spec->access[0].random = 0;
	spec->access[0].delay = 0;
	spec->access[0].burst = 1;
	spec->access[0].align = 0;
	spec->access[0].reply = 0;

	// 16384 Bytes / 16 Kilo Bytes

	spec = New();
	_snprintf(spec->name, MAX_NAME, "16K; 100%% Read; 0%% random");
	spec->access[0].of_size = 100;
	spec->access[0].size = 16384;
	spec->access[0].reads = 100;
	spec->access[0].random = 0;
	spec->access[0].delay = 0;
	spec->access[0].burst = 1;
	spec->access[0].align = 0;
	spec->access[0].reply = 0;

	spec = New();
	_snprintf(spec->name, MAX_NAME, "16K; 75%% Read; 0%% random");
	spec->access[0].of_size = 100;
	spec->access[0].size = 16384;
	spec->access[0].reads = 75;
	spec->access[0].random = 0;
	spec->access[0].delay = 0;
	spec->access[0].burst = 1;
	spec->access[0].align = 0;
	spec->access[0].reply = 0;

	spec = New();
	_snprintf(spec->name, MAX_NAME, "16K; 50%% Read; 0%% random");
	spec->access[0].of_size = 100;
	spec->access[0].size = 16384;
	spec->access[0].reads = 50;
	spec->access[0].random = 0;
	spec->access[0].delay = 0;
	spec->access[0].burst = 1;
	spec->access[0].align = 0;
	spec->access[0].reply = 0;

	spec = New();
	_snprintf(spec->name, MAX_NAME, "16K; 25%% Read; 0%% random");
	spec->access[0].of_size = 100;
	spec->access[0].size = 16384;
	spec->access[0].reads = 25;
	spec->access[0].random = 0;
	spec->access[0].delay = 0;
	spec->access[0].burst = 1;
	spec->access[0].align = 0;
	spec->access[0].reply = 0;

	spec = New();
	_snprintf(spec->name, MAX_NAME, "16K; 0%% Read; 0%% random");
	spec->access[0].of_size = 100;
	spec->access[0].size = 16384;
	spec->access[0].reads = 0;
	spec->access[0].random = 0;
	spec->access[0].delay = 0;
	spec->access[0].burst = 1;
	spec->access[0].align = 0;
	spec->access[0].reply = 0;

	// 32768 Bytes / 32 Kilo Bytes

	spec = New();
	_snprintf(spec->name, MAX_NAME, "32K; 100%% Read; 0%% random");
	spec->access[0].of_size = 100;
	spec->access[0].size = 32768;
	spec->access[0].reads = 100;
	spec->access[0].random = 0;
	spec->access[0].delay = 0;
	spec->access[0].burst = 1;
	spec->access[0].align = 0;
	spec->access[0].reply = 0;

	spec = New();
	_snprintf(spec->name, MAX_NAME, "32K; 75%% Read; 0%% random");
	spec->access[0].of_size = 100;
	spec->access[0].size = 32768;
	spec->access[0].reads = 75;
	spec->access[0].random = 0;
	spec->access[0].delay = 0;
	spec->access[0].burst = 1;
	spec->access[0].align = 0;
	spec->access[0].reply = 0;

	spec = New();
	_snprintf(spec->name, MAX_NAME, "32K; 50%% Read; 0%% random");
	spec->access[0].of_size = 100;
	spec->access[0].size = 32768;
	spec->access[0].reads = 50;
	spec->access[0].random = 0;
	spec->access[0].delay = 0;
	spec->access[0].burst = 1;
	spec->access[0].align = 0;
	spec->access[0].reply = 0;

	spec = New();
	_snprintf(spec->name, MAX_NAME, "32K; 25%% Read; 0%% random");
	spec->access[0].of_size = 100;
	spec->access[0].size = 32768;
	spec->access[0].reads = 25;
	spec->access[0].random = 0;
	spec->access[0].delay = 0;
	spec->access[0].burst = 1;
	spec->access[0].align = 0;
	spec->access[0].reply = 0;

	spec = New();
	_snprintf(spec->name, MAX_NAME, "32K; 0%% Read; 0%% random");
	spec->access[0].of_size = 100;
	spec->access[0].size = 32768;
	spec->access[0].reads = 0;
	spec->access[0].random = 0;
	spec->access[0].delay = 0;
	spec->access[0].burst = 1;
	spec->access[0].align = 0;
	spec->access[0].reply = 0;

	// Brings all specifications into one

	spec = New();
	_snprintf(spec->name, MAX_NAME, "All in one");

	spec->access[0].of_size = 5;
	spec->access[0].size = 512;
	spec->access[0].reads = 100;
	spec->access[0].random = 0;
	spec->access[0].delay = 0;
	spec->access[0].burst = 1;
	spec->access[0].align = 0;
	spec->access[0].reply = 0;

	spec->access[1].of_size = 5;
	spec->access[1].size = 512;
	spec->access[1].reads = 75;
	spec->access[1].random = 0;
	spec->access[1].delay = 0;
	spec->access[1].burst = 1;
	spec->access[1].align = 0;
	spec->access[1].reply = 0;

	spec->access[2].of_size = 5;
	spec->access[2].size = 512;
	spec->access[2].reads = 50;
	spec->access[2].random = 0;
	spec->access[2].delay = 0;
	spec->access[2].burst = 1;
	spec->access[2].align = 0;
	spec->access[2].reply = 0;

	spec->access[3].of_size = 5;
	spec->access[3].size = 512;
	spec->access[3].reads = 25;
	spec->access[3].random = 0;
	spec->access[3].delay = 0;
	spec->access[3].burst = 1;
	spec->access[3].align = 0;
	spec->access[3].reply = 0;

	spec->access[4].of_size = 5;
	spec->access[4].size = 512;
	spec->access[4].reads = 0;
	spec->access[4].random = 0;
	spec->access[4].delay = 0;
	spec->access[4].burst = 1;
	spec->access[4].align = 0;
	spec->access[4].reply = 0;

	spec->access[5].of_size = 5;
	spec->access[5].size = 4096;
	spec->access[5].reads = 100;
	spec->access[5].random = 0;
	spec->access[5].delay = 0;
	spec->access[5].burst = 1;
	spec->access[5].align = 0;
	spec->access[5].reply = 0;

	spec->access[6].of_size = 5;
	spec->access[6].size = 4096;
	spec->access[6].reads = 75;
	spec->access[6].random = 0;
	spec->access[6].delay = 0;
	spec->access[6].burst = 1;
	spec->access[6].align = 0;
	spec->access[6].reply = 0;

	spec->access[7].of_size = 5;
	spec->access[7].size = 4096;
	spec->access[7].reads = 50;
	spec->access[7].random = 0;
	spec->access[7].delay = 0;
	spec->access[7].burst = 1;
	spec->access[7].align = 0;
	spec->access[7].reply = 0;

	spec->access[8].of_size = 5;
	spec->access[8].size = 4096;
	spec->access[8].reads = 25;
	spec->access[8].random = 0;
	spec->access[8].delay = 0;
	spec->access[8].burst = 1;
	spec->access[8].align = 0;
	spec->access[8].reply = 0;

	spec->access[9].of_size = 5;
	spec->access[9].size = 4096;
	spec->access[9].reads = 0;
	spec->access[9].random = 0;
	spec->access[9].delay = 0;
	spec->access[9].burst = 1;
	spec->access[9].align = 0;
	spec->access[9].reply = 0;

	spec->access[10].of_size = 5;
	spec->access[10].size = 16384;
	spec->access[10].reads = 100;
	spec->access[10].random = 0;
	spec->access[10].delay = 0;
	spec->access[10].burst = 1;
	spec->access[10].align = 0;
	spec->access[10].reply = 0;

	spec->access[11].of_size = 5;
	spec->access[11].size = 16384;
	spec->access[11].reads = 75;
	spec->access[11].random = 0;
	spec->access[11].delay = 0;
	spec->access[11].burst = 1;
	spec->access[11].align = 0;
	spec->access[11].reply = 0;

	spec->access[12].of_size = 5;
	spec->access[12].size = 16384;
	spec->access[12].reads = 50;
	spec->access[12].random = 0;
	spec->access[12].delay = 0;
	spec->access[12].burst = 1;
	spec->access[12].align = 0;
	spec->access[12].reply = 0;

	spec->access[13].of_size = 5;
	spec->access[13].size = 16384;
	spec->access[13].reads = 25;
	spec->access[13].random = 0;
	spec->access[13].delay = 0;
	spec->access[13].burst = 1;
	spec->access[13].align = 0;
	spec->access[13].reply = 0;

	spec->access[14].of_size = 5;
	spec->access[14].size = 16384;
	spec->access[14].reads = 0;
	spec->access[14].random = 0;
	spec->access[14].delay = 0;
	spec->access[14].burst = 1;
	spec->access[14].align = 0;
	spec->access[14].reply = 0;

	spec->access[15].of_size = 5;
	spec->access[15].size = 32768;
	spec->access[15].reads = 100;
	spec->access[15].random = 0;
	spec->access[15].delay = 0;
	spec->access[15].burst = 1;
	spec->access[15].align = 0;
	spec->access[15].reply = 0;

	spec->access[16].of_size = 5;
	spec->access[16].size = 32768;
	spec->access[16].reads = 75;
	spec->access[16].random = 0;
	spec->access[16].delay = 0;
	spec->access[16].burst = 1;
	spec->access[16].align = 0;
	spec->access[16].reply = 0;

	spec->access[17].of_size = 5;
	spec->access[17].size = 32768;
	spec->access[17].reads = 50;
	spec->access[17].random = 0;
	spec->access[17].delay = 0;
	spec->access[17].burst = 1;
	spec->access[17].align = 0;
	spec->access[17].reply = 0;

	spec->access[18].of_size = 5;
	spec->access[18].size = 32768;
	spec->access[18].reads = 25;
	spec->access[18].random = 0;
	spec->access[18].delay = 0;
	spec->access[18].burst = 1;
	spec->access[18].align = 0;
	spec->access[18].reply = 0;

	spec->access[19].of_size = 5;
	spec->access[19].size = 32768;
	spec->access[19].reads = 0;
	spec->access[19].random = 0;
	spec->access[19].delay = 0;
	spec->access[19].burst = 1;
	spec->access[19].align = 0;
	spec->access[19].reply = 0;

	spec->access[20].of_size = IOERROR;
}

//
// Removes all AccessSpecObjects form memory and removes all references 
// from the pointer array.
//
AccessSpecList::~AccessSpecList()
{
	DeleteAll();
}

//
// Creates a new AccessSpecObject and adds a pointer to it to the end of
// the pointer array.
//
Test_Spec *AccessSpecList::New()
{
	Test_Spec *spec = new Test_Spec;

	if (!spec)
		return NULL;
	// Make the default new access spec
	InitAccessSpecLine(&(spec->access[0]));
	spec->access[1].of_size = IOERROR;

	// Set the access spec to not load by default.
	spec->default_assignment = FALSE;

	// Name the new access spec.
	NextUntitled(spec->name);

	// Add the new access spec to the end of the array.
	spec_list.Add(spec);

	// Return the index in the pointer array of the newly added object's pointer.
	return spec;
}

//
// Creates a copy of the specified access spec, and returns a copy.
//
Test_Spec *AccessSpecList::Copy(Test_Spec * source_spec)
{
	int copy_number;
	CString name;
	Test_Spec *spec;

	// Create a new spec and check the validity of source_spec.
	if (!(spec = New()) || IndexByRef(source_spec) == IOERROR)
		return NULL;

	// Copy the source spec into the newly created one.
	memcpy(spec, source_spec, sizeof Test_Spec);

	// Assign a unique name: Copy of 'name of source spec' ('unique copy number').
	name.Format("Copy of %s", spec->name);
	copy_number = 1;
	do {
		snprintf(spec->name, MAX_NAME, "%s (%d)", name, copy_number++);
	} while (RefByName(spec->name) != spec);

	return spec;
}

//
// Deletes the specified Test_Spec object, as well as the related entry in 
// the pointer array.  Also removes any reference that spec by any worker.
//
void AccessSpecList::Delete(Test_Spec * spec)
{
	// Check for an invalid pointer.
	if (IndexByRef(spec) == IOERROR)
		return;

	// Remove any references to this access spec from the workers.
	theApp.manager_list.RemoveAccessSpec(spec);

	// Remove the reference to the access spec from the array.
	spec_list.RemoveAt(IndexByRef(spec));

	// Remove the access spec from memory.
	delete spec;
}

//
// Deletes all the access spec objects and their related entries.
//
void AccessSpecList::DeleteAll()
{
	while (spec_list.GetSize()) {
		delete spec_list[spec_list.GetUpperBound()];

		spec_list.RemoveAt(spec_list.GetUpperBound());
	}
	theApp.manager_list.RemoveAllAccessSpecs();
}

//
// Returns the pointer to the access spec object specified by the index.
//
Test_Spec *AccessSpecList::Get(int index)
{
	if (index >= 0 && index < spec_list.GetSize())
		return spec_list[index];
	else
		return NULL;
}

//
// Returns the number of entries in the array.
//
int AccessSpecList::Count()
{
	return spec_list.GetSize();
}

//
// Returns a pointer to a spec of a given name, if it exists.
//
Test_Spec *AccessSpecList::RefByName(const char *check_name)
{
	Test_Spec *spec;
	int spec_count = spec_list.GetSize();

	for (int s = 0; s < spec_count; s++) {
		spec = Get(s);
		if (_stricmp(spec->name, check_name) == 0)
			return spec;
	}
	return NULL;
}

//
// Returns the index of a spec given a reference to that spec.
// This function is used to validate spec pointers.
// Also used in Delete() to index into the array, and in 
// Save() to determine which specs where running. 
//
int AccessSpecList::IndexByRef(const Test_Spec * spec)
{
	int spec_count = spec_list.GetSize();

	for (int index = 0; index < spec_count; index++) {
		if (Get(index) == spec)
			return index;
	}
	return IOERROR;
}

//
// Saving the access specification list to a result file.
// Saves only the currently active access specs.
//
BOOL AccessSpecList::SaveResults(ostream & outfile)
{
	int i, j;
	int access_count, manager_count, worker_count, current_access_index;
	Manager *mgr;
	Worker *wkr;
	BOOL *spec_active;
	Test_Spec *spec;

	outfile << "'Access specifications" << endl;

	access_count = Count();
	spec_active = new BOOL[access_count];

	// Determine which access specs are active for the current test
	// Start by marking all access specs inactive.
	for (i = 1; i < access_count; i++) {
		spec_active[i] = FALSE;
	}

	current_access_index = theApp.pView->GetCurrentAccessIndex();
	// Now go through all the workers and mark each worker's current access spec 
	// as active, IF the worker is actually doing anything in this test.
	manager_count = theApp.manager_list.ManagerCount();
	for (i = 0; i < manager_count; i++) {
		mgr = theApp.manager_list.GetManager(i);
		worker_count = mgr->WorkerCount();
		for (j = 0; j < worker_count; j++) {
			wkr = mgr->GetWorker(j);
			if (wkr->ActiveInCurrentTest()) {
				spec_active[IndexByRef(wkr->GetAccessSpec(current_access_index))] = TRUE;
			}
		}
	}

	// Save all the active access specs except the idle spec.
	for (i = 1; i < access_count; i++) {
		if (!spec_active[i])
			continue;

		spec = Get(i);

		outfile << "'Access specification name,default assignment" << endl;
		outfile << spec->name << "," << spec->default_assignment << endl;

		// Write access specifications to a file, data comma separated.
		outfile << "'size,% of size,% reads,% random,delay,burst,align,reply" << endl;

		for (int line_index = 0; line_index < MAX_ACCESS_SPECS; line_index++) {
			if (spec->access[line_index].of_size == IOERROR)
				break;
			outfile
			    << spec->access[line_index].size << ","
			    << spec->access[line_index].of_size << ","
			    << spec->access[line_index].reads << ","
			    << spec->access[line_index].random << ","
			    << spec->access[line_index].delay << ","
			    << spec->access[line_index].burst << ","
			    << spec->access[line_index].align << "," << spec->access[line_index].reply << endl;
		}
	}

	outfile << "'End access specifications" << endl;

	delete spec_active;

	return TRUE;
}

//
// Saves the access specification list to a config file.
// Saves ALL specs, whether they are active or not.
// It is tempting to combine this with SaveResults, but changes to one should
// not generally affect the other.  At one point, the output of these two
// functions was identical, but there are differences now, and there will
// continue to be more in the future.
//
BOOL AccessSpecList::SaveConfig(ostream & outfile)
{
	int i, line_index;
	int access_count;
	Test_Spec *spec;

	outfile << "'ACCESS SPECIFICATIONS =================" "========================================" << endl;

	access_count = Count();

	// Save all the active access specs except the idle spec (0).
	for (i = 1; i < access_count; i++) {
		spec = Get(i);

		outfile << "'Access specification name,default assignment" << endl;
		outfile << "\t" << spec->name << ",";

		switch (spec->default_assignment) {
		case AssignNone:
			outfile << "NONE" << endl;
			break;
		case AssignAll:
			outfile << "ALL" << endl;
			break;
		case AssignDisk:
			outfile << "DISK" << endl;
			break;
		case AssignNet:
			outfile << "NET" << endl;
			break;
		default:
			ErrorMessage("Error saving access specification list.  Access specification "
				     + (CString) spec->name + " has an illegal default assignment.");
			return FALSE;
		}

		// Write access specifications to a file, data comma separated.
		outfile << "'size,% of size,% reads,% random,delay,burst,align,reply" << endl;

		for (line_index = 0; line_index < MAX_ACCESS_SPECS; line_index++) {
			if (spec->access[line_index].of_size == IOERROR)
				break;
			outfile << "\t"
			    << spec->access[line_index].size << ","
			    << spec->access[line_index].of_size << ","
			    << spec->access[line_index].reads << ","
			    << spec->access[line_index].random << ","
			    << spec->access[line_index].delay << ","
			    << spec->access[line_index].burst << ","
			    << spec->access[line_index].align << "," << spec->access[line_index].reply << endl;
		}
	}

	outfile << "'END access specifications" << endl;

	return TRUE;
}

//
// Checks the config file version number and calls the appropriate load function.
// Also handles the replace flag.
//
// If replace is TRUE, all access specs are removed before the restore,
// otherwise, the loaded specs are merged with the preexisting specs.
//
BOOL AccessSpecList::LoadConfig(const CString & infilename, BOOL replace)
{
	ICF_ifstream infile(infilename);
	long version;
	BOOL retval;

	if (replace) {
		// Clear memory and display before loading accesses.
		DeleteAll();
		InsertIdleSpec();
	}

	version = infile.GetVersion();
	if (version == -1)
		return FALSE;

	if (!infile.SkipTo("'ACCESS SPECIFICATIONS"))
		return TRUE;	// no access spec list to restore (this is OK)

	// Check to see whether to load a per-worker access spec, or an old style access spec.
	if (version >= 19980521) {
		retval = LoadConfigNew(infile);

		// There's no need to assign the default access specs here.
		// They will be assigned automatically when workers are created
		// (by the Worker constructor itself), and they will be removed
		// by the Manager::LoadConfig code if they shouldn't be assigned.
	} else {
		retval = LoadConfigOld(infile);

		// If replacing, assign default specs to workers.
		if (replace)
			theApp.manager_list.AssignDefaultAccessSpecs();
	}

	infile.close();

	return retval;
}

//
// Loads the global access spec list from a file.
// Does not support old style saved configurations.
//
BOOL AccessSpecList::LoadConfigNew(ICF_ifstream & infile)
{
	CString key, value;
	CString token;
	int line_index;
	int total_access;
	Test_Spec *spec;
	string str;

	while (1) {
		if (!infile.GetPair(key, value)) {
			ErrorMessage("File is improperly formatted.  Expected an "
				     "access specification or \"END access specifications\".");
			return FALSE;
		}

		if (key.CompareNoCase("'END access specifications") == 0) {
			break;
		}
		if (key.CompareNoCase("'Access specification name,default assignment") == 0) {
			if (spec = RefByName((LPCTSTR) value.Left(value.Find(',')))) {
#ifdef VERBOSE
				ErrorMessage("An access specification named \"" + value.Left(value.Find(','))
					     + "\" already exists.  It will be replaced by "
					     "the new access specification.");
#endif
			} else {
				// No previous access spec by this name.  Create a new one.
				spec = New();

				// Sanity check.
				if (!spec) {
					ErrorMessage("Error while loading file.  Out of memory.  "
						     "Error occured in AccessSpecList::LoadConfig()");
					return FALSE;
				}
			}

			// Fill in the name.
			token = ICF_ifstream::ExtractFirstToken(value, TRUE);
			strcpy(spec->name, (LPCTSTR) token);

			// Read the default assignment.
			token = ICF_ifstream::ExtractFirstToken(value);
			if (token.CompareNoCase("NONE") == 0)
				spec->default_assignment = AssignNone;
			else if (token.CompareNoCase("ALL") == 0)
				spec->default_assignment = AssignAll;
			else if (token.CompareNoCase("DISK") == 0)
				spec->default_assignment = AssignDisk;
			else if (token.CompareNoCase("NET") == 0)
				spec->default_assignment = AssignNet;
			else
				spec->default_assignment = atoi((LPCTSTR) token);

			key = infile.GetNextLine();
			if (key.GetAt(0) != '\'') {
				ErrorMessage("File is improperly formatted.  Expected a "
					     "comment line to follow the access specification name.");
				return FALSE;
			}
			// Get the details of the access spec.

			line_index = 0;
			total_access = 0;

			// Read lines until total percentage reaches 100.
			while (infile.peek() != '\'') {
				getline(infile,str,',');
				spec->access[line_index].size = atoi(str.c_str());

				getline(infile,str,',');
				spec->access[line_index].of_size = atoi(str.c_str());

				getline(infile,str,',');
				spec->access[line_index].reads = atoi(str.c_str());

				getline(infile,str,',');
				spec->access[line_index].random = atoi(str.c_str());

				getline(infile,str,',');
				spec->access[line_index].delay = atoi(str.c_str());

				getline(infile,str,',');
				spec->access[line_index].burst = atoi(str.c_str());

				getline(infile,str,',');
				spec->access[line_index].align = atoi(str.c_str());

				getline(infile,str);
				spec->access[line_index].reply = atoi(str.c_str());
				
				// Sanity check.
				if (spec->access[line_index].of_size > 100 ||
				    spec->access[line_index].reads > 100 ||
				    spec->access[line_index].random > 100 ||
				    spec->access[line_index].of_size < 0 ||
				    spec->access[line_index].reads < 0 || spec->access[line_index].random < 0) {
					Delete(spec);
					ErrorMessage("Error loading global access specifications.  "
						     "Invalid value encountered.");
					return FALSE;
				}

				total_access += spec->access[line_index].of_size;
				line_index++;
			}

			// Mark the end of the access spec.
			if (line_index <= MAX_ACCESS_SPECS)
				spec->access[line_index].of_size = IOERROR;

			// Sanity check.
			if (total_access != 100) {
				ErrorMessage("Error loading global access specifications.  "
					     "Percentages don't add to 100.");
				Delete(spec);
				return FALSE;
			}

		} else {
			ErrorMessage("File is improperly formatted.  Global ACCESS "
				     "SPECIFICATION section contained an unrecognized \"" + key + "\" comment.");
			return FALSE;
		}
	}

	return TRUE;
}

//
// Loads an old style access spec.
//
BOOL AccessSpecList::LoadConfigOld(ICF_ifstream & infile)
{
	int total_access;
	int line_index;
	Test_Spec *spec;
	char comment[100];

	// Read and discard initial comment line.
	infile.getline(comment, 100);

	// Create new test spec.
	spec = New();
	line_index = 0;
	total_access = 0;

	// Get access specs from the file until an error occurs or all specs have been read in.
	while (!infile.rdstate()) {
		// Read in specifications until another comment is read in from the file.
		// This signifies the end of the access spec portion.
		if (infile.peek() == '\'' || infile.peek() == EOF) {
			// Delete the last spec created.
			if (spec)
				Delete(spec);

			return TRUE;
		}
		// Sanity check.
		if (!spec) {
			ErrorMessage("Error allocating memory for access specification.");
			return FALSE;
		}

		InitAccessSpecLine(&(spec->access[line_index]));
		infile >> spec->access[line_index].size;
		infile >> spec->access[line_index].of_size;
		infile >> spec->access[line_index].reads;
		infile >> spec->access[line_index].random;
		infile >> spec->access[line_index].delay;
		infile >> spec->access[line_index].burst;
		infile.ignore(1, '\n');

		// Sanity check.
		if (spec->access[line_index].of_size > 100 ||
		    spec->access[line_index].reads > 100 ||
		    spec->access[line_index].random > 100 ||
		    spec->access[line_index].of_size < 0 ||
		    spec->access[line_index].reads < 0 || spec->access[line_index].random < 0) {
			break;
		}
		// Skip lines that make up "0%" of the access.
		if (spec->access[line_index].of_size) {
			total_access += spec->access[line_index].of_size;
			line_index++;
		} else {
			ErrorMessage("Found an access specification line that makes up 0% of the test.  "
				     "This line will be discarded.");
		}

		// Sanity check.
		if (total_access > 100)
			break;

		// Mark the end of the access spec if needed.
		if (total_access == 100) {
			spec->default_assignment = AssignAll;
			if (line_index <= MAX_ACCESS_SPECS)
				spec->access[line_index].of_size = IOERROR;

			// Try smart naming if possible.
			SmartName(spec);

			// Create new test spec.
			spec = New();

			// Prepare to load next spec from file.
			line_index = 0;
			total_access = 0;
		}
	}

	// Delete the last spec created.
	if (spec)
		Delete(spec);

	ErrorMessage("Error loading access specification.  Invalid access specification.");
	return FALSE;
}

//
// This function initializes a line of an access spec to the default values.
//
void AccessSpecList::InitAccessSpecLine(Access_Spec * spec_line)
{
	spec_line->size = 2048;
	spec_line->of_size = 100;
	spec_line->random = 100;
	spec_line->reads = 67;
	spec_line->delay = 0;
	spec_line->burst = 1;
	spec_line->align = 0;
	spec_line->reply = 0;
}

//
// This function will return an integer that will be the next sequential 
// n for naming "Untitled n" access specs.
//
void AccessSpecList::NextUntitled(char *name)
{
	int next_number = 0;
	CString current_name;

	// Counting the number of "Untitled *" specs.
	for (int index = 0; index < spec_list.GetSize(); index++) {
		current_name = Get(index)->name;
		if (current_name.Find("Untitled") == 0) {
			++next_number;
		}
	}

	// Ensure that names are not duplicated.
	do {
		// We have the current number, so we add 1 for the next number.
		snprintf(name, MAX_NAME, "Untitled %d", ++next_number);
	}
	while (RefByName(name));
}

//
// Assigning a reasonable name to an older access spec that doesn't have one.
//
void AccessSpecList::SmartName(Test_Spec * spec)
{
	CString name;
	int spec_number = 1;
	int name_size;

	// Verify that smart naming is possible.
	if (spec->access[1].of_size != IOERROR)
		return;		// use existing name

	// Switch statement for size part of name
	switch (spec->access[0].size) {
	case 512:
		name = "512byte";
		break;
	case MEGABYTE_BIN:
		name = "1MB";
		break;
	default:
		name.Format("%dKB", (spec->access[0].size / KILOBYTE_BIN));
	}

	// Switch statement for random/sequential part of name
	switch (spec->access[0].random) {
	case 0:
		name += " sequential";
		break;
	case 100:
		name += " random";
		break;
	default:
		name.Format("%s %d%% random", name.Left(name.GetLength()), spec->access[0].random);
	}

	// Switch statement for read/write part of name
	switch (spec->access[0].reads) {
	case 0:
		name += " writes";
		break;
	case 100:
		name += " reads";
		break;
	default:
		name.Format("%s %d%% reads", name.Left(name.GetLength()), spec->access[0].reads);
	}

	// Test to ensure no duplicate names
	name_size = name.GetLength();
	while (RefByName((LPCTSTR) name)) {
		name.Format("%s %d", name.Left(name_size), ++spec_number);
	}
	snprintf(spec->name, MAX_NAME, "%s", name);
}
