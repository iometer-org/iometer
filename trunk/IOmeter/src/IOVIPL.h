/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / IOVIPL.h                                                  ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Interface for the VIPL class, which provides an       ## */
/* ##               interface to the Virtual Interface Programmer's       ## */
/* ##               Library.                                              ## */
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
/* ##  Changes ...: 2005-04-18 (raltherr@apple.com)                       ## */
/* ##               - Support for MacOS X                                 ## */
/* ##               2004-03-26 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Code cleanup to ensure common style.                ## */
/* ##               - Applied Thayne Harmon's patch for supporting        ## */
/* ##                 Netware support (on I386).                          ## */
/* ##               2003-10-15 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef	VIPL_DEFINED
#define	VIPL_DEFINED

#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
#define HINSTANCE HANDLE
#endif

#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
#include "IOCommon.h"
#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
#include <dlfcn.h>
#endif
#elif defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
#include <afxwin.h>
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
#include "vipl.h"

///////////////////////////////////////////////////////////////////////////////
// The VIPL, Virtual Interface Programmer's Library, class dynamically links
// to vipl.dll.  This library may or may not be loaded on a system.
// Dynamically linking to the library allows Iometer/Dynamo to run properly on
// a system that it is not installed on.  It is intended to simply abstract the
// functions found in the actual library.  As such, all member variables are
// declared statically to provide only one instance of the class.
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Type definitions for dynamically linked functions.  Actual definitions are
// located in vipl.h, and these should match accordingly.
//
// NIC
typedef VIP_RETURN(*fVipOpenNic)
 (const VIP_CHAR * DeviceName, VIP_NIC_HANDLE * NicHandle);
typedef VIP_RETURN(*fVipCloseNic)
 (VIP_NIC_HANDLE NicHandle);

//
// VI
typedef VIP_RETURN(*fVipCreateVi)
 (VIP_NIC_HANDLE NicHandle, VIP_VI_ATTRIBUTES * ViAttribs,
  VIP_CQ_HANDLE SendCQHandle, VIP_CQ_HANDLE RecvCQHandle, VIP_VI_HANDLE * ViHandle);
typedef VIP_RETURN(*fVipDestroyVi)
 (VIP_VI_HANDLE ViHandle);

//
// Client/server connection
typedef VIP_RETURN(*fVipConnectWait)
 (VIP_NIC_HANDLE NicHandle, VIP_NET_ADDRESS * LocalAddr,
  VIP_ULONG Timeout, VIP_NET_ADDRESS * RemoteAddr, VIP_VI_ATTRIBUTES * RemoteViAttribs, VIP_CONN_HANDLE * ConnHandle);
typedef VIP_RETURN(*fVipConnectAccept)
 (VIP_CONN_HANDLE ConnHandle, VIP_VI_HANDLE ViHandle);
typedef VIP_RETURN(*fVipConnectReject)
 (VIP_CONN_HANDLE ConnHandle);
typedef VIP_RETURN(*fVipConnectRequest)
 (VIP_VI_HANDLE ViHandle, VIP_NET_ADDRESS * LocalAddr,
  VIP_NET_ADDRESS * RemoteAddr, VIP_ULONG Timeout, VIP_VI_ATTRIBUTES * RemoteViAttribs);
typedef VIP_RETURN(*fVipDisconnect)
 (VIP_VI_HANDLE ViHandle);

//
// Ptag
typedef VIP_RETURN(*fVipCreatePtag)
 (VIP_NIC_HANDLE NicHandle, VIP_PROTECTION_HANDLE * Ptag);
typedef VIP_RETURN(*fVipDestroyPtag)
 (VIP_NIC_HANDLE NicHandle, VIP_PROTECTION_HANDLE Ptag);

//
// Memory
typedef VIP_RETURN(*fVipRegisterMem)
 (VIP_NIC_HANDLE NicHandle, VIP_PVOID VirtualAddress,
  VIP_ULONG Length, VIP_MEM_ATTRIBUTES * MemAttribs, VIP_MEM_HANDLE * MemoryHandle);
typedef VIP_RETURN(*fVipDeregisterMem)
 (VIP_NIC_HANDLE NicHandle, VIP_PVOID VirtualAddress, VIP_MEM_HANDLE MemoryHandle);

//
// Send/receive
typedef VIP_RETURN(*fVipPostSend)
 (VIP_VI_HANDLE ViHandle, VIP_DESCRIPTOR * DescriptorPtr, VIP_MEM_HANDLE MemoryHandle);
typedef VIP_RETURN(*fVipSendDone)
 (VIP_VI_HANDLE ViHandle, VIP_DESCRIPTOR ** DescriptorPtr);
typedef VIP_RETURN(*fVipSendWait)
 (VIP_VI_HANDLE ViHandle, VIP_ULONG Timeout, VIP_DESCRIPTOR ** DescriptorPtr);
typedef VIP_RETURN(*fVipPostRecv)
 (VIP_VI_HANDLE ViHandle, VIP_DESCRIPTOR * DescriptorPtr, VIP_MEM_HANDLE MemoryHandle);
typedef VIP_RETURN(*fVipRecvDone)
 (VIP_VI_HANDLE ViHandle, VIP_DESCRIPTOR ** DescriptorPtr);
typedef VIP_RETURN(*fVipRecvWait)
 (VIP_VI_HANDLE ViHandle, VIP_ULONG Timeout, VIP_DESCRIPTOR ** DescriptorPtr);

//
// Completion queues
typedef VIP_RETURN(*fVipCQDone)
 (VIP_CQ_HANDLE CQHandle, VIP_VI_HANDLE * ViHandle, VIP_BOOLEAN * RecvQueue);
typedef VIP_RETURN(*fVipCQWait)
 (VIP_CQ_HANDLE CQHandle, VIP_ULONG Timeout, VIP_VI_HANDLE * ViHandle, VIP_BOOLEAN * RecvQueue);
typedef VIP_RETURN(*fVipCreateCQ)
 (VIP_NIC_HANDLE NicHandle, VIP_ULONG EntryCount, VIP_CQ_HANDLE * CQHandle);
typedef VIP_RETURN(*fVipDestroyCQ)
 (VIP_CQ_HANDLE CQHandle);
typedef VIP_RETURN(*fVipResizeCQ)
 (VIP_CQ_HANDLE CQHandle, VIP_ULONG EntryCount);

//
// Information
typedef VIP_RETURN(*fVipQueryNic)
 (VIP_NIC_HANDLE NicHandle, VIP_NIC_ATTRIBUTES * NicAttribs);
typedef VIP_RETURN(*fVipSetViAttributes)
 (VIP_VI_HANDLE ViHandle, VIP_VI_ATTRIBUTES * ViAttribs);
typedef VIP_RETURN(*fVipQueryVi)
 (VIP_VI_HANDLE ViHandle, VIP_VI_STATE * State,
  VIP_VI_ATTRIBUTES * ViAttribs, VIP_BOOLEAN * ViSendQEmpty, VIP_BOOLEAN * ViRecvQEmpty);
typedef VIP_RETURN(*fVipSetMemAttributes)
 (VIP_NIC_HANDLE NicHandle, VIP_PVOID Address, VIP_MEM_HANDLE MemHandle, VIP_MEM_ATTRIBUTES * MemAttribs);
typedef VIP_RETURN(*fVipQueryMem)
 (VIP_NIC_HANDLE NicHandle, VIP_PVOID Address, VIP_MEM_HANDLE MemHandle, VIP_MEM_ATTRIBUTES * MemAttribs);
typedef VIP_RETURN(*fVipQuerySystemManagementInfo)
 (VIP_NIC_HANDLE NicHandle, VIP_ULONG InfoType, VIP_PVOID SysManInfo);

//
// Peer-to-peer connection
typedef VIP_RETURN(*fVipConnectPeerRequest)
 (VIP_VI_HANDLE ViHandle, VIP_NET_ADDRESS * LocalAddr, VIP_NET_ADDRESS * RemoteAddr, VIP_ULONG Timeout);
typedef VIP_RETURN(*fVipConnectPeerDone)
 (VIP_VI_HANDLE ViHandle, VIP_VI_ATTRIBUTES * RemoteViAttribs);
typedef VIP_RETURN(*fVipConnectPeerWait)
 (VIP_VI_HANDLE ViHandle, VIP_VI_ATTRIBUTES * RemoteViAttribs);
typedef VIP_RETURN(*fVipNSInit)
 (VIP_NIC_HANDLE NicHandle, VIP_PVOID NSInitInfo);

//
// Name service
typedef VIP_RETURN(*fVipNSGetHostByName)
 (VIP_NIC_HANDLE NicHandle, VIP_CHAR * Name, VIP_NET_ADDRESS * Address, VIP_ULONG NameIndex);
typedef VIP_RETURN(*fVipNSGetHostByAddr)
 (VIP_NIC_HANDLE NicHandle, VIP_NET_ADDRESS * Address, VIP_CHAR * Name, VIP_ULONG * NameLen);
typedef VIP_RETURN(*fVipNSShutdown)
 (VIP_NIC_HANDLE NicHandle);

//
///////////////////////////////////////////////////////////////////////////////

class VIPL {
      public:
	VIPL();
	~VIPL();
	char *Error(VIP_RETURN result);

	// Handle to vipl.dll library.  Set to NULL if the library is not loaded.
	static HINSTANCE vipl_dll;

	///////////////////////////////////////////////////////////////////////////
	// Dynamic linked functions.

	// NIC
	static fVipOpenNic VipOpenNic;
	static fVipCloseNic VipCloseNic;

	// VI
	static fVipCreateVi VipCreateVi;
	static fVipDestroyVi VipDestroyVi;

	// Client/server connection
	static fVipConnectWait VipConnectWait;
	static fVipConnectAccept VipConnectAccept;
	static fVipConnectReject VipConnectReject;
	static fVipConnectRequest VipConnectRequest;
	static fVipDisconnect VipDisconnect;

	// Ptag
	static fVipCreatePtag VipCreatePtag;
	static fVipDestroyPtag VipDestroyPtag;

	// Memory
	static fVipRegisterMem VipRegisterMem;
	static fVipDeregisterMem VipDeregisterMem;

	// Send/receive
	static fVipPostSend VipPostSend;
	static fVipSendDone VipSendDone;
	static fVipSendWait VipSendWait;
	static fVipPostRecv VipPostRecv;
	static fVipRecvDone VipRecvDone;
	static fVipRecvWait VipRecvWait;

	// Completion queues
	static fVipCQDone VipCQDone;
	static fVipCQWait VipCQWait;
	static fVipCreateCQ VipCreateCQ;
	static fVipDestroyCQ VipDestroyCQ;
	static fVipResizeCQ VipResizeCQ;

	// Information
	static fVipQueryNic VipQueryNic;
	static fVipSetViAttributes VipSetViAttributes;
	static fVipQueryVi VipQueryVi;
	static fVipSetMemAttributes VipSetMemAttributes;
	static fVipQueryMem VipQueryMem;
	static fVipQuerySystemManagementInfo VipQuerySystemManagementInfo;

	// Peer-to-peer connection
	static fVipConnectPeerRequest VipConnectPeerRequest;
	static fVipConnectPeerDone VipConnectPeerDone;
	static fVipConnectPeerWait VipConnectPeerWait;

	// Name service
	static fVipNSInit VipNSInit;
	static fVipNSGetHostByName VipNSGetHostByName;
	static fVipNSGetHostByAddr VipNSGetHostByAddr;
	static fVipNSShutdown VipNSShutdown;
	//
	///////////////////////////////////////////////////////////////////////////
};

#endif				// VIPL_DEFINED
