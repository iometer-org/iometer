/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / IOVIPL.cpp                                                ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implementation of the VIPL class, which provides an   ## */
/* ##               interface to the Virtual Interface Programmer's       ## */
/* ##               Library (hiding the dynamic linking to vipl.dll). All ## */
/* ##               members of this class are static, so in effect there  ## */
/* ##               is at most one VIPL object in a copy of Dynamo, even  ## */
/* ##               though there may be multiple instantiations of it.    ## */
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
/* ##  Changes ...: 2003-10-15 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include "IOVIPL.h"

#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
#include <iostream>
using namespace std;
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_SOLARIS)
#include <iostream.h>
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

///////////////////////////////////////////////////////////////////////////////
// Initial declarations for static variables.
//
HINSTANCE VIPL::vipl_dll = NULL;
fVipOpenNic VIPL::VipOpenNic = NULL;
fVipCloseNic VIPL::VipCloseNic = NULL;

// VI
fVipCreateVi VIPL::VipCreateVi = NULL;
fVipDestroyVi VIPL::VipDestroyVi = NULL;

 // Client/server connection
fVipConnectWait VIPL::VipConnectWait = NULL;
fVipConnectAccept VIPL::VipConnectAccept = NULL;
fVipConnectReject VIPL::VipConnectReject = NULL;
fVipConnectRequest VIPL::VipConnectRequest = NULL;
fVipDisconnect VIPL::VipDisconnect = NULL;

// Ptag
fVipCreatePtag VIPL::VipCreatePtag = NULL;
fVipDestroyPtag VIPL::VipDestroyPtag = NULL;

// Memory
fVipRegisterMem VIPL::VipRegisterMem = NULL;
fVipDeregisterMem VIPL::VipDeregisterMem = NULL;

// Send/receive
fVipPostSend VIPL::VipPostSend = NULL;
fVipSendDone VIPL::VipSendDone = NULL;
fVipSendWait VIPL::VipSendWait = NULL;
fVipPostRecv VIPL::VipPostRecv = NULL;
fVipRecvDone VIPL::VipRecvDone = NULL;
fVipRecvWait VIPL::VipRecvWait = NULL;

// Completion queues
fVipCQDone VIPL::VipCQDone = NULL;
fVipCQWait VIPL::VipCQWait = NULL;
fVipCreateCQ VIPL::VipCreateCQ = NULL;
fVipDestroyCQ VIPL::VipDestroyCQ = NULL;
fVipResizeCQ VIPL::VipResizeCQ = NULL;

// Information
fVipQueryNic VIPL::VipQueryNic = NULL;
fVipSetViAttributes VIPL::VipSetViAttributes = NULL;
fVipQueryVi VIPL::VipQueryVi = NULL;
fVipSetMemAttributes VIPL::VipSetMemAttributes = NULL;
fVipQueryMem VIPL::VipQueryMem = NULL;
fVipQuerySystemManagementInfo VIPL::VipQuerySystemManagementInfo = NULL;

// Peer-to-peer connection
fVipConnectPeerRequest VIPL::VipConnectPeerRequest = NULL;
fVipConnectPeerDone VIPL::VipConnectPeerDone = NULL;
fVipConnectPeerWait VIPL::VipConnectPeerWait = NULL;

// Name service
fVipNSInit VIPL::VipNSInit = NULL;
fVipNSGetHostByName VIPL::VipNSGetHostByName = NULL;
fVipNSGetHostByAddr VIPL::VipNSGetHostByAddr = NULL;
fVipNSShutdown VIPL::VipNSShutdown = NULL;

//
///////////////////////////////////////////////////////////////////////////////

//
// Loading vipl.dll and dynamically linking all VI functions.  Setting 
// vipl_loaded if the library loaded successfully.

//
// UNIX/SOLARIS NOTE
// -----------------
//              The UNIX code should work straightaway as soon as a VI shared object is
// available. The dlopen(3), dlsym(3) and dlclose(3) calls setup the VI shared
// libraries and make the symbols available for access by dynamo.
//
// The dlopen(3), dlsym(3) and dlclose(3) calls are available on almost all UNIX
// systems but, I think, they might be called dl_open(3) and so on in other flavors.
// Not sure, though. So for now, I have included these calls under a SOLARIS directive.
// If at a later stage, the calls are the same on all flavors of UNIX, then the
// SOLARIS directive can be removed.
//
// Another ***IMPORTANT*** note. It is assumed as of now that as yet unavailable
// UNIX VI library will have the same symbols as the NT library. (If you did'nt know,
// this is called "day-dreaming"). Do not be shocked if the program crashes while 
// trying to load the symbols. You see, we are not checking for errors from dlsym(3).
// The exact symbols will have to be determined when the library becomes available.
//
// Yet another ***IMPORTANT*** note. Since we are linking with the VI shared library
// at runtime, there should be no attempt to link with the library at compiler time.
// But for dynamo to get at the dl...(3) calls we have to link dynamo with -ldl
// (libdl.so) at compile time.
//
VIPL::VIPL()
{
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	// Load vipl.dll.
	if (!(vipl_dll = LoadLibrary("vipl.dll"))) {
#ifdef _DEBUG
		cout << "Unable to load vipl.dll" << endl;
#endif
		return;
	}
	// Dynamically linking VI functions.

	// NIC
	VipOpenNic = (fVipOpenNic) GetProcAddress(vipl_dll, "VipOpenNic");
	VipCloseNic = (fVipCloseNic) GetProcAddress(vipl_dll, "VipCloseNic");

	// VI
	VipCreateVi = (fVipCreateVi) GetProcAddress(vipl_dll, "VipCreateVi");
	VipDestroyVi = (fVipDestroyVi) GetProcAddress(vipl_dll, "VipDestroyVi");

	// Client/server connection
	VipConnectWait = (fVipConnectWait)
	    GetProcAddress(vipl_dll, "VipConnectWait");
	VipConnectAccept = (fVipConnectAccept)
	    GetProcAddress(vipl_dll, "VipConnectAccept");
	VipConnectReject = (fVipConnectReject)
	    GetProcAddress(vipl_dll, "VipConnectReject");
	VipConnectRequest = (fVipConnectRequest)
	    GetProcAddress(vipl_dll, "VipConnectRequest");
	VipDisconnect = (fVipDisconnect)
	    GetProcAddress(vipl_dll, "VipDisconnect");

	// Ptag
	VipCreatePtag = (fVipCreatePtag)
	    GetProcAddress(vipl_dll, "VipCreatePtag");
	VipDestroyPtag = (fVipDestroyPtag)
	    GetProcAddress(vipl_dll, "VipDestroyPtag");

	// Memory
	VipRegisterMem = (fVipRegisterMem)
	    GetProcAddress(vipl_dll, "VipRegisterMem");
	VipDeregisterMem = (fVipDeregisterMem)
	    GetProcAddress(vipl_dll, "VipDeregisterMem");

	// Send/receive
	VipPostSend = (fVipPostSend) GetProcAddress(vipl_dll, "VipPostSend");
	VipSendDone = (fVipSendDone) GetProcAddress(vipl_dll, "VipSendDone");
	VipSendWait = (fVipSendWait) GetProcAddress(vipl_dll, "VipSendWait");
	VipPostRecv = (fVipPostRecv) GetProcAddress(vipl_dll, "VipPostRecv");
	VipRecvDone = (fVipRecvDone) GetProcAddress(vipl_dll, "VipRecvDone");
	VipRecvWait = (fVipRecvWait) GetProcAddress(vipl_dll, "VipRecvWait");

	// Completion queues
	VipCQDone = (fVipCQDone) GetProcAddress(vipl_dll, "VipCQDone");
	VipCQWait = (fVipCQWait) GetProcAddress(vipl_dll, "VipCQWait");
	VipCreateCQ = (fVipCreateCQ) GetProcAddress(vipl_dll, "VipCreateCQ");
	VipDestroyCQ = (fVipDestroyCQ) GetProcAddress(vipl_dll, "VipDestroyCQ");
	VipResizeCQ = (fVipResizeCQ) GetProcAddress(vipl_dll, "VipResizeCQ");

	// Information
	VipQueryNic = (fVipQueryNic) GetProcAddress(vipl_dll, "VipQueryNic");
	VipSetViAttributes = (fVipSetViAttributes)
	    GetProcAddress(vipl_dll, "VipSetViAttributes");
	VipQueryVi = (fVipQueryVi) GetProcAddress(vipl_dll, "VipQueryVi");
	VipSetMemAttributes = (fVipSetMemAttributes)
	    GetProcAddress(vipl_dll, "VipSetMemAttributes");
	VipQueryMem = (fVipQueryMem) GetProcAddress(vipl_dll, "VipQueryMem");
	VipQuerySystemManagementInfo = (fVipQuerySystemManagementInfo)
	    GetProcAddress(vipl_dll, "VipQuerySystemManagementInfo");

	// Peer-to-peer connection
	VipConnectPeerRequest = (fVipConnectPeerRequest)
	    GetProcAddress(vipl_dll, "VipConnectPeerRequest");
	VipConnectPeerDone = (fVipConnectPeerDone)
	    GetProcAddress(vipl_dll, "VipConnectPeerDone");
	VipConnectPeerWait = (fVipConnectPeerWait)
	    GetProcAddress(vipl_dll, "VipConnectPeerWait");

	// Name service
	VipNSInit = (fVipNSInit) GetProcAddress(vipl_dll, "VipNSInit");
	VipNSGetHostByName = (fVipNSGetHostByName)
	    GetProcAddress(vipl_dll, "VipNSGetHostByName");
	VipNSGetHostByAddr = (fVipNSGetHostByAddr)
	    GetProcAddress(vipl_dll, "VipNSGetHostByAddr");
	VipNSShutdown = (fVipNSShutdown)
	    GetProcAddress(vipl_dll, "VipNSShutdown");
#elif defined(IOMTR_OS_SOLARIS)
	// When VI library becomes available on Solaris, this should work
	if ((vipl_dll = dlopen("vipl.so", RTLD_NOW | RTLD_GLOBAL | RTLD_PARENT)) == NULL) {
#ifdef _DEBUG
		cout << "unable to load VI shared library" << endl;
#endif
		return;
	}
	// Link all the VI functions.
	// NIC
	VipOpenNic = (fVipOpenNic) dlsym(vipl_dll, "VipOpenNic");
	VipCloseNic = (fVipCloseNic) dlsym(vipl_dll, "VipCloseNic");

	// VI
	VipCreateVi = (fVipCreateVi) dlsym(vipl_dll, "VipCreateVi");
	VipDestroyVi = (fVipDestroyVi) dlsym(vipl_dll, "VipDestroyVi");

	// Client/server connection
	VipConnectWait = (fVipConnectWait)
	    dlsym(vipl_dll, "VipConnectWait");
	VipConnectAccept = (fVipConnectAccept)
	    dlsym(vipl_dll, "VipConnectAccept");
	VipConnectReject = (fVipConnectReject)
	    dlsym(vipl_dll, "VipConnectReject");
	VipConnectRequest = (fVipConnectRequest)
	    dlsym(vipl_dll, "VipConnectRequest");
	VipDisconnect = (fVipDisconnect)
	    dlsym(vipl_dll, "VipDisconnect");

	// Ptag
	VipCreatePtag = (fVipCreatePtag)
	    dlsym(vipl_dll, "VipCreatePtag");
	VipDestroyPtag = (fVipDestroyPtag)
	    dlsym(vipl_dll, "VipDestroyPtag");

	// Memory
	VipRegisterMem = (fVipRegisterMem)
	    dlsym(vipl_dll, "VipRegisterMem");
	VipDeregisterMem = (fVipDeregisterMem)
	    dlsym(vipl_dll, "VipDeregisterMem");

	// Send/receive
	VipPostSend = (fVipPostSend) dlsym(vipl_dll, "VipPostSend");
	VipSendDone = (fVipSendDone) dlsym(vipl_dll, "VipSendDone");
	VipSendWait = (fVipSendWait) dlsym(vipl_dll, "VipSendWait");
	VipPostRecv = (fVipPostRecv) dlsym(vipl_dll, "VipPostRecv");
	VipRecvDone = (fVipRecvDone) dlsym(vipl_dll, "VipRecvDone");
	VipRecvWait = (fVipRecvWait) dlsym(vipl_dll, "VipRecvWait");

	// Completion queues
	VipCQDone = (fVipCQDone) dlsym(vipl_dll, "VipCQDone");
	VipCQWait = (fVipCQWait) dlsym(vipl_dll, "VipCQWait");
	VipCreateCQ = (fVipCreateCQ) dlsym(vipl_dll, "VipCreateCQ");
	VipDestroyCQ = (fVipDestroyCQ) dlsym(vipl_dll, "VipDestroyCQ");
	VipResizeCQ = (fVipResizeCQ) dlsym(vipl_dll, "VipResizeCQ");

	// Information
	VipQueryNic = (fVipQueryNic) dlsym(vipl_dll, "VipQueryNic");
	VipSetViAttributes = (fVipSetViAttributes)
	    dlsym(vipl_dll, "VipSetViAttributes");
	VipQueryVi = (fVipQueryVi) dlsym(vipl_dll, "VipQueryVi");
	VipSetMemAttributes = (fVipSetMemAttributes)
	    dlsym(vipl_dll, "VipSetMemAttributes");
	VipQueryMem = (fVipQueryMem) dlsym(vipl_dll, "VipQueryMem");
	VipQuerySystemManagementInfo = (fVipQuerySystemManagementInfo)
	    dlsym(vipl_dll, "VipQuerySystemManagementInfo");

	// Peer-to-peer connection
	VipConnectPeerRequest = (fVipConnectPeerRequest)
	    dlsym(vipl_dll, "VipConnectPeerRequest");
	VipConnectPeerDone = (fVipConnectPeerDone)
	    dlsym(vipl_dll, "VipConnectPeerDone");
	VipConnectPeerWait = (fVipConnectPeerWait)
	    dlsym(vipl_dll, "VipConnectPeerWait");

	// Name service
	VipNSInit = (fVipNSInit) dlsym(vipl_dll, "VipNSInit");
	VipNSGetHostByName = (fVipNSGetHostByName)
	    dlsym(vipl_dll, "VipNSGetHostByName");
	VipNSGetHostByAddr = (fVipNSGetHostByAddr)
	    dlsym(vipl_dll, "VipNSGetHostByAddr");
	VipNSShutdown = (fVipNSShutdown)
	    dlsym(vipl_dll, "VipNSShutdown");
#elif defined(IOMTR_OS_LINUX)
	// nop
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
}

//
// Freeing the dynamically linked vipl.dll
//
VIPL::~VIPL()
{
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	// If library had previously been loaded, free it.
	if (vipl_dll)
		FreeLibrary(vipl_dll);
#elif defined(IOMTR_OS_SOLARIS)
	if (vipl_dll)
		dlclose(vipl_dll);
	return;
#elif defined(IOMTR_OS_LINUX)
	// nop
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
}

//
// Handy debugger function.  Call like so:
//
// cout << "*** Error: " << vipl.Error( result ) << endl;
//
char *VIPL::Error(VIP_RETURN result)
{
	static char errmsg[2048];

	switch (result) {
	case VIP_NOT_DONE:
		strcpy(errmsg, "VIP_NOT_DONE - The operation is still in" " progress.");
		break;
	case VIP_INVALID_PARAMETER:
		strcpy(errmsg, "VIP_INVALID_PARAMETER - One of the input" " parameters was invalid.");
		break;
	case VIP_ERROR_RESOURCE:
		strcpy(errmsg, "VIP_ERROR_RESOURCE - The operation failed due" " to insufficient resources.");
		break;
	case VIP_TIMEOUT:
		strcpy(errmsg, "VIP_TIMEOUT - The operation timed out.");
		break;
	case VIP_REJECT:
		strcpy(errmsg, "VIP_REJECT - The connection was rejected.");
		break;
	case VIP_INVALID_RELIABILITY_LEVEL:
		strcpy(errmsg, "VIP_INVALID_RELIABILITY_LEVEL - The requested"
		       " reliability level attribute was invalid or not supported.");
		break;
	case VIP_INVALID_MTU:
		strcpy(errmsg, "VIP_INVALID_MTU - The maximum transfer size"
		       " attribute was invalid or not supported.");
		break;
	case VIP_INVALID_QOS:
		strcpy(errmsg, "VIP_INVALID_QOS - The quality of service" " attribute was invalid or not supported.");
		break;
	case VIP_INVALID_PTAG:
		strcpy(errmsg, "VIP_INVALID_PTAG - The protection tag" " attribute was invalid.");
		break;
	case VIP_INVALID_RDMAREAD:
		strcpy(errmsg, "VIP_INVALID_RDMAREAD - The attributes"
		       " requested support for RDMA Read, but the VI Provider" " does not support it.");
		break;
	case VIP_DESCRIPTOR_ERROR:
		strcpy(errmsg, "VIP_DESCRIPTOR_ERROR - The operation returned" " a null Descriptor pointer.");
		break;
	case VIP_INVALID_STATE:
		strcpy(errmsg, "VIP_INVALID_STATE - The VI is not in the" " appropriate state for the operation.");
		break;
	case VIP_ERROR_NAMESERVICE:
		strcpy(errmsg, "VIP_ERROR_NAMESERVICE - There was an error" " contacting the Name Service.");
		break;
	case VIP_NO_MATCH:
		strcpy(errmsg, "VIP_NO_MATCH - The specified discriminator" " was not matched.");
		break;
	default:
		sprintf(errmsg, "Unknown VI error: %d.", result);
		break;
	}

	return errmsg;
}
