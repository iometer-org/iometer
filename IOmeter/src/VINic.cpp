/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / VINic.cpp                                                 ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implements the VINic class, which abstracts the VI    ## */
/* ##               NIC hardware interface.                               ## */
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
/* ##  Remarks ...: - Note that applications should verify that vipl.dll  ## */
/* ##                 is loaded before using this class.                  ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Changes ...: 2003-10-15 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */

#define VINIC_DETAILS 0		// Set to 1 for additional debugging messages.

#include "VINic.h"

#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
#include <iostream>
using namespace std;
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_SOLARIS)
#include <iostream.h>
#endif

//
// Opening a VI NIC, recording its attributes, creating its memory protection
// tag, and setting its memory attributes for subsequent VI connections.
//
BOOL VINic::Open(const char *nic_name)
{
	VIP_RETURN result;

#if VINIC_DETAILS
	cout << "Opening VI NIC " << nic_name << "." << endl;
#endif

	// Open the NIC.
	if ((result = vipl.VipOpenNic((const VIP_CHAR *)nic_name, &nic)) != VIP_SUCCESS) {
#ifdef _DEBUG
		cout << "Unable to open VI nic " << nic_name << ": " << vipl.Error(result) << endl;
#endif
		return FALSE;
	}
	// Record the NIC attributes.
	if ((result = vipl.VipQueryNic(nic, &nic_attributes)) != VIP_SUCCESS) {
		cout << "*** Unable to record attributes for VI nic " << nic_name << ": " << vipl.Error(result) << endl;
		vipl.VipCloseNic(nic);
		return FALSE;
	}
	// Create the memory protection tag.
	if ((result = vipl.VipCreatePtag(nic, &nic_mem_attributes.Ptag)) != VIP_SUCCESS) {
		cout << "*** Unable to create protection tag for VI nic "
		    << nic_name << ": " << vipl.Error(result) << endl;
		vipl.VipCloseNic(nic);
		return FALSE;
	}
	// Record the address for this NIC.
	if (!GetLocalAddress()) {
		vipl.VipCloseNic(nic);
		return FALSE;
	}
	// Set the VI NIC's memory attributes.  Disallow RDMA.
	nic_mem_attributes.EnableRdmaRead = FALSE;
	nic_mem_attributes.EnableRdmaWrite = FALSE;

	return TRUE;
}

//
// Closing the connection to the VI NIC, removing all associated memory, and
// destroying its protection tag.
//
BOOL VINic::Close()
{
	BOOL result = TRUE;

#if VINIC_DETAILS
	cout << "Closing VI NIC " << nic_attributes.Name << "." << endl;
#endif

	// Destroy the Ptag.
	if (vipl.VipDestroyPtag(nic, nic_mem_attributes.Ptag) != VIP_SUCCESS)
		result = FALSE;

	// Close the NIC.
	if (vipl.VipCloseNic(nic) != VIP_SUCCESS)
		result = FALSE;

	return result;
}

//
// Retrieving the network address for this VI NIC.
//
BOOL VINic::GetLocalAddress()
{
#if VINIC_DETAILS
	cout << "Retrieving the address for " << nic_attributes.Name << endl;
#endif

	// Allocate memory for the address.  A VI network address consists of:
	// | address length | discriminator length | address | discriminator |
	nic_address_length = sizeof(VIP_NET_ADDRESS) + nic_attributes.NicAddressLen +
	    nic_attributes.MaxDiscriminatorLen;
	if (!(nic_address = (VIP_NET_ADDRESS *) malloc(nic_address_length))) {
		cout << "*** Unable to allocate memory for VI NIC addresss." << endl;
		return FALSE;
	}
	// Fill in the address.  Initialize the descriptor to 0.
	nic_address->HostAddressLen = nic_attributes.NicAddressLen;
	nic_address->DiscriminatorLen = nic_attributes.MaxDiscriminatorLen;
	memcpy((char *)nic_address->HostAddress, nic_attributes.LocalNicAddress, nic_attributes.NicAddressLen);
	memset((void *)(nic_address->HostAddress +
			nic_attributes.NicAddressLen), 0, nic_attributes.MaxDiscriminatorLen);

	return TRUE;
}

//
// Registering memory with the opened VI NIC.
//
BOOL VINic::RegisterMemory(void *buffer, DWORD size, VIP_MEM_HANDLE * mem)
{
#if VINIC_DETAILS
	cout << "Registering memory for nic " << nic_attributes.Name << endl;
#endif

	// Register the memory with the VI NIC.
	return (vipl.VipRegisterMem(nic, buffer, size, &nic_mem_attributes, mem)
		== VIP_SUCCESS);
}

//
// Deregistering all memory associated with the opened VI NIC.  The memory
// must have been registered first.
//
BOOL VINic::DeregisterMemory(void *buffer, VIP_MEM_HANDLE mem)
{
#if VINIC_DETAILS
	cout << "Deregistering memory for nic " << nic_attributes.Name << endl;
#endif

	return (vipl.VipDeregisterMem(nic, buffer, mem) == VIP_SUCCESS);
}
