/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (IOmeter & Dynamo) / IOAccess.cpp                                  ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implements the Access class.  This class represents   ## */
/* ##               the current access specification for a Grunt in       ## */
/* ##               Dynamo. It contains an array of ACCESS structures,    ## */
/* ##               each of which defines one access spec line.           ## */
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
/* ##  Changes ...: 2004-03-26 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Code cleanup to ensure common style.                ## */
/* ##               - Applied Thayne Harmon's patch for supporting        ## */
/* ##                 Netware support (on I386).                          ## */
/* ##               2003-07-17 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include "IOAccess.h"
#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
pthread_mutex_t lock_mt;
#endif

//
// Setting the access specifications based on the given input.  This will also
// record the maximum requested transfer size.
//
void Access::Initialize(const Access_Specs specs)
{
	int i, temp_value, start_percent = 0, spec_no = 0;

	max_transfer = 0;

	do {
		for (i = start_percent; (i < (start_percent + specs[spec_no].of_size) && (i < MAX_ACCESS_SPECS)); i++) {
			access_grid[i].size = specs[spec_no].size;
			access_grid[i].read = specs[spec_no].reads;
			access_grid[i].random = specs[spec_no].random;
			access_grid[i].delay = specs[spec_no].delay;
			access_grid[i].burst = specs[spec_no].burst;
			access_grid[i].align = specs[spec_no].align;
			access_grid[i].reply = specs[spec_no].reply;

			temp_value = access_grid[i].align;

			if (temp_value != 0) {
				// See if alignment value is a power of two by repeatedly
				// dividing it in half (right shifting by 1 bit) and checking
				// to see if the result is odd at any stage.
				while (temp_value > 1) {
					if (temp_value % 2)
						break;

					temp_value >>= 1;
				}

				// If alignment is a power of two, byte alignment can
				// be speed optimized by using a bit mask like so:
				//
				// (access_grid[i].align is abbreviated "align" in examples)
				//
				// "& ~((DWORDLONG)align - 1)" masks off lower bits to force
				//                      sector alignment.  sector_size is cast to a
				//                      DWORDLONG to force the result to be a 64-bit
				//                      quantity.
				//
				//     For example, if alignment = 512 (0x00000200):
				//           (DWORDLONG)align      = 0x0000000000000200
				//          ((DWORDLONG)align - 1) = 0x00000000000001FF
				//         ~((DWORDLONG)align - 1) = 0xFFFFFFFFFFFFFE00

				if (temp_value == 1)
					access_grid[i].align_mask = ~((DWORDLONG) access_grid[i].align - 1);
				else
					access_grid[i].align_mask = NOT_POWER_OF_TWO;
			} else {
				// assuming sector alignment (mask value is ignored, setting it anyway)
				access_grid[i].align_mask = 0;
			}
		}
		// Checking for new maximum transfer size (request or reply).
		if (i > start_percent) {
			if (specs[spec_no].size > (DWORD) max_transfer)
				max_transfer = (int)specs[spec_no].size;
			if (specs[spec_no].reply > (DWORD) max_transfer)
				max_transfer = (int)specs[spec_no].reply;
		}

		start_percent = i;	// Begin again where we left off.
		spec_no++;
	} while (i < MAX_ACCESS_SPECS);
	cout << "Access specifications for test defined." << endl << flush;
}

//
// Gets information from the access spec to prepare the next burst.
// Doesn't retrieve ALL data from the access spec.
//
void Access::GetNextBurst(int access_percent,
			  int *burst, DWORD * size, int *delay, DWORD * align, DWORDLONG * align_mask, DWORD * reply)
{
	*burst = access_grid[access_percent].burst;
	*size = access_grid[access_percent].size;
	*delay = access_grid[access_percent].delay;
	*align = access_grid[access_percent].align;
	*align_mask = access_grid[access_percent].align_mask;
	*reply = access_grid[access_percent].reply;
}

//
// Determines if an access is to be a read.
//
BOOL Access::Read(int access_percent, int read_percent)
{
	return (read_percent < access_grid[access_percent].read);
}

//
// Determines is an access is to be random.
//
BOOL Access::Random(int access_percent, int random_percent)
{
	return (random_percent < access_grid[access_percent].random);
}
