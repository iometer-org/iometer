/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / vipl.h                                                    ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Contains the complete user interface to the VIPL.     ## */
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
/* ##  Changes ...:                                                       ## */
/* ##               2003-02-15 (mingz@ele.uri.edu)                        ## */
/* ##               - Added padding in VIP_NET_ADDRESS because of         ## */
/* ##                 the alignment issue between IA32 and ARM arch       ## */
/* ##               2003-10-19 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Changed the CS/_CS destinction so that Solaris on   ## */
/* ##                 x86 (i386) is the only exception.                   ## */
/* ##               2003-07-19 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Assimilated the patch from Robert Jones which is    ## */
/* ##                 needed to build under Solaris 9 on x86 (i386).      ## */
/* ##               2003-07-18 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef _VIPL_
#define _VIPL_  1

/**********************************************************************
 * Constants
**********************************************************************/

#define IN
#define OUT

/*
 * This constant is used only by the user and the VIPL library.
 */

#ifndef VIP_KERNEL

#define VIP_INFINITE INFINITE

#endif				/* !VIP_KERNEL */

/**********************************************************************
 * Data Types
 **********************************************************************/

/*
 * Generic types for portability
 */
typedef void *VIP_PVOID;
typedef int VIP_BOOLEAN;
typedef char VIP_CHAR;
typedef unsigned char VIP_UCHAR;
typedef unsigned short VIP_USHORT;
typedef unsigned long VIP_ULONG;
typedef unsigned __int64 VIP_UINT64;
typedef unsigned __int32 VIP_UINT32;
typedef unsigned __int16 VIP_UINT16;
typedef unsigned __int8 VIP_UINT8;

typedef VIP_PVOID VIP_QOS;	/* details are not defined */

/*
 * Constants used with VIP_BOOLEAN
 */
#define VIP_TRUE    (1)
#define VIP_FALSE   (0)

/*
 * Handles are used for opaque objects.
 */
typedef VIP_PVOID VIP_NIC_HANDLE;
typedef VIP_PVOID VIP_VI_HANDLE;
typedef VIP_PVOID VIP_PROTECTION_HANDLE;
typedef VIP_PVOID VIP_CONN_HANDLE;
typedef VIP_PVOID VIP_CQ_HANDLE;

/* 
 * Handles used by the NIC Hardware
 */
typedef VIP_UINT32 VIP_MEM_HANDLE;

/*
 * VI Network Address
 *
 * A VI Network Address holds the network specific address for a VI 
 * endpoint. Each VI Provider may have a unique network address format.  
 * It is composed of two elements, a host address and an endpoint
 * discriminator. These elements are qualified with a byte length in
 * order to maintain network independence. 
 */
typedef struct {
	VIP_UINT16 HostAddressLen;
	VIP_UINT16 DiscriminatorLen;
	VIP_UINT8 HostAddress[1];

	// architecture difference between IA32 and ARM
	char padding[8 - sizeof(VIP_UINT16) * 2 - sizeof(VIP_UINT8)];
}
VIP_NET_ADDRESS;

typedef VIP_USHORT VIP_RELIABILITY_LEVEL;

/*
 *	Bit values for VIP_RELIABILITY_LEVEL
 */

#define VIP_SERVICE_UNRELIABLE          0x01
#define VIP_SERVICE_RELIABLE_DELIVERY   0x02
#define VIP_SERVICE_RELIABLE_RECEPTION  0x04

/*
 * A VIP_NIC_ATTRIBUTES describes a nic.
 *
 * The NIC attributes structure is returned from the VipQueryNic
 * function. It contains information related to an instance of a NIC 
 * within a VI Provider. All values that are returned in the NIC 
 * Attributes structure are static values that are set by the VI 
 * Provider at the time that it is initialized.
 */
typedef struct {
	VIP_CHAR Name[64];
	VIP_ULONG HardwareVersion;
	VIP_ULONG ProviderVersion;
	VIP_UINT16 NicAddressLen;
	const VIP_UINT8 *LocalNicAddress;
	VIP_BOOLEAN ThreadSafe;
	VIP_UINT16 MaxDiscriminatorLen;
	VIP_ULONG MaxRegisterBytes;
	VIP_ULONG MaxRegisterRegions;
	VIP_ULONG MaxRegisterBlockBytes;
	VIP_ULONG MaxVI;
	VIP_ULONG MaxDescriptorsPerQueue;
	VIP_ULONG MaxSegmentsPerDesc;
	VIP_ULONG MaxCQ;
	VIP_ULONG MaxCQEntries;
	VIP_ULONG MaxTransferSize;
	VIP_ULONG NativeMTU;
	VIP_ULONG MaxPtags;
	VIP_RELIABILITY_LEVEL ReliabilityLevelSupport;
	VIP_RELIABILITY_LEVEL RDMAReadSupport;
} VIP_NIC_ATTRIBUTES;

/*
 * Memory alignment required by the NIC for descriptors, in bytes.  
 */
#define VIP_DESCRIPTOR_ALIGNMENT    64

/*
 *  The address structure for registered memory accesses.
 */
typedef union {
	VIP_UINT64 AddressBits;
	VIP_PVOID Address;
} VIP_PVOID64;

/*
 * The control portion of the descriptor.
 */
typedef struct {
	VIP_PVOID64 Next;
	VIP_MEM_HANDLE NextHandle;
	VIP_UINT16 SegCount;
	VIP_UINT16 Control;
	VIP_UINT32 Reserved;
	VIP_UINT32 ImmediateData;
	VIP_UINT32 Length;
	VIP_UINT32 Status;
} VIP_CONTROL_SEGMENT;

/*
 * Descriptor RDMA Segment
 */
typedef struct {
	VIP_PVOID64 Data;
	VIP_MEM_HANDLE Handle;
	VIP_UINT32 Reserved;
} VIP_ADDRESS_SEGMENT;

/*
 * Descriptor Data Segment
 */
typedef struct {
	VIP_PVOID64 Data;
	VIP_MEM_HANDLE Handle;
	VIP_UINT32 Length;
} VIP_DATA_SEGMENT;

typedef union {
	VIP_ADDRESS_SEGMENT Remote;
	VIP_DATA_SEGMENT Local;
} VIP_DESCRIPTOR_SEGMENT;

/*
 * Complete VI Descriptor
 */
typedef struct _VIP_DESCRIPTOR {
// NOTE: This might be to generic, maybe we need an seperate global define for
//       this stuff. Current knowledge is Sparc build without underscore and i386
//       build with it (otherwise it conflicts with /usr/include/ia32/sys/reg.h).
#if defined(IOMTR_OS_SOLARIS) && defined(IOMTR_CPU_I386)
	VIP_CONTROL_SEGMENT _CS;
	VIP_DESCRIPTOR_SEGMENT _DS[2];
#else
	VIP_CONTROL_SEGMENT CS;
	VIP_DESCRIPTOR_SEGMENT DS[2];
#endif
} VIP_DESCRIPTOR;

/*
 * Bit field macros for descriptor control segment: Control field
 */
#define     VIP_CONTROL_OP_SENDRECV                   0x0000
#define     VIP_CONTROL_OP_RDMAWRITE                  0x0001
#define     VIP_CONTROL_OP_RDMAREAD                   0x0002
#define     VIP_CONTROL_OP_RESERVED                   0x0003
#define     VIP_CONTROL_OP_MASK                       0x0003
#define     VIP_CONTROL_IMMEDIATE                     0x0004
#define     VIP_CONTROL_QFENCE                        0x0008
#define     VIP_CONTROL_RESERVED                      0xFFF0

/*
 * Bit field macros for descriptor control segment: Status field
 */
#define     VIP_STATUS_DONE                           0x00000001
#define     VIP_STATUS_FORMAT_ERROR                   0x00000002
#define     VIP_STATUS_PROTECTION_ERROR               0x00000004
#define     VIP_STATUS_LENGTH_ERROR                   0x00000008
#define     VIP_STATUS_PARTIAL_ERROR                  0x00000010
#define     VIP_STATUS_DESC_FLUSHED_ERROR             0x00000020
#define     VIP_STATUS_TRANSPORT_ERROR                0x00000040
#define     VIP_STATUS_RDMA_PROT_ERROR                0x00000080
#define     VIP_STATUS_REMOTE_DESC_ERROR              0x00000100
#define     VIP_STATUS_ERROR_MASK                     0x000001FE

#define     VIP_STATUS_OP_SEND                        0x00000000
#define     VIP_STATUS_OP_RECEIVE                     0x00010000
#define     VIP_STATUS_OP_RDMA_WRITE                  0x00020000
#define     VIP_STATUS_OP_REMOTE_RDMA_WRITE           0x00030000
#define     VIP_STATUS_OP_RDMA_READ                   0x00040000
#define     VIP_STATUS_OP_MASK                        0x00070000

#define     VIP_STATUS_IMMEDIATE                      0x00080000

#define     VIP_STATUS_RESERVED                       0xFFF0FE00

/*
 * VIP_VI_STATE is used in the return of VipQueryVi.
 */
typedef enum {
	VIP_STATE_IDLE,
	VIP_STATE_CONNECTED,
	VIP_STATE_CONNECT_PENDING,
	VIP_STATE_ERROR
} VIP_VI_STATE;

/*
 * VIP_VI_ATTRIBUTES is used to create and query VIs.
 */
typedef struct {
	VIP_RELIABILITY_LEVEL ReliabilityLevel;
	VIP_ULONG MaxTransferSize;
	VIP_QOS QoS;
	VIP_PROTECTION_HANDLE Ptag;
	VIP_BOOLEAN EnableRdmaWrite;
	VIP_BOOLEAN EnableRdmaRead;
} VIP_VI_ATTRIBUTES;

/*
 * VIP_MEM_ATTRIBUTES is used to create and query Memory regions.
 */
typedef struct _VIP_MEM_ATTRIBUTES {
	VIP_PROTECTION_HANDLE Ptag;
	VIP_BOOLEAN EnableRdmaWrite;
	VIP_BOOLEAN EnableRdmaRead;
} VIP_MEM_ATTRIBUTES;

/*
 * Error Descriptor Error Codes
 */
typedef enum _VIP_ERROR_CODE {
	VIP_ERROR_POST_DESC,
	VIP_ERROR_CONN_LOST,
	VIP_ERROR_RECVQ_EMPTY,
	VIP_ERROR_VI_OVERRUN,
	VIP_ERROR_RDMAW_PROT,
	VIP_ERROR_RDMAW_DATA,
	VIP_ERROR_RDMAW_ABORT,
	VIP_ERROR_RDMAR_PROT,
	VIP_ERROR_COMP_PROT,
	VIP_ERROR_RDMA_TRANSPORT,
	VIP_ERROR_CATASTROPHIC
} VIP_ERROR_CODE;

/*
 * Error Descriptor Resource Codes
 */
typedef enum _VIP_RESOURCE_CODE {
	VIP_RESOURCE_NIC,
	VIP_RESOURCE_VI,
	VIP_RESOURCE_CQ,
	VIP_RESOURCE_DESCRIPTOR
} VIP_RESOURCE_CODE;

/*
 * Error Descriptor for Asynch. errors
 */
typedef struct _VIP_ERROR_DESCRIPTOR {
	VIP_NIC_HANDLE NicHandle;
	VIP_VI_HANDLE ViHandle;
	VIP_CQ_HANDLE CQHandle;
	VIP_DESCRIPTOR *DescriptorPtr;
	VIP_ULONG OpCode;
	VIP_RESOURCE_CODE ResourceCode;
	VIP_ERROR_CODE ErrorCode;
} VIP_ERROR_DESCRIPTOR;

/*
 * Return codes from the functions.
 */
typedef enum {
	VIP_SUCCESS,
	VIP_NOT_DONE,
	VIP_INVALID_PARAMETER,
	VIP_ERROR_RESOURCE,
	VIP_TIMEOUT,
	VIP_REJECT,
	VIP_INVALID_RELIABILITY_LEVEL,
	VIP_INVALID_MTU,
	VIP_INVALID_QOS,
	VIP_INVALID_PTAG,
	VIP_INVALID_RDMAREAD,
	VIP_DESCRIPTOR_ERROR,
	VIP_INVALID_STATE,
	VIP_ERROR_NAMESERVICE,
	VIP_NO_MATCH
} VIP_RETURN;

/*
 * InfoType values
 */
#define VIP_SMI_AUTODISCOVERY   ((VIP_ULONG) 1)

/*
 * AutoDiscovery List fields
 */
typedef struct {
	VIP_ULONG NumberOfHops;
	VIP_NET_ADDRESS **ADAddrArray;
	VIP_ULONG NumAdAddrs;
} VIP_AUTODISCOVERY_LIST;

/**********************************************************************
 * Functions
**********************************************************************/

extern VIP_RETURN VipOpenNic(IN const VIP_CHAR * DeviceName, OUT VIP_NIC_HANDLE * NicHandle);

extern VIP_RETURN VipCloseNic(IN VIP_NIC_HANDLE NicHandle);

extern VIP_RETURN
VipCreateVi(IN VIP_NIC_HANDLE NicHandle,
	    IN VIP_VI_ATTRIBUTES * ViAttribs,
	    IN VIP_CQ_HANDLE SendCQHandle, IN VIP_CQ_HANDLE RecvCQHandle, OUT VIP_VI_HANDLE * ViHandle);

extern VIP_RETURN VipDestroyVi(IN VIP_VI_HANDLE ViHandle);

extern VIP_RETURN
VipConnectWait(IN VIP_NIC_HANDLE NicHandle,
	       IN VIP_NET_ADDRESS * LocalAddr,
	       IN VIP_ULONG Timeout,
	       OUT VIP_NET_ADDRESS * RemoteAddr,
	       OUT VIP_VI_ATTRIBUTES * RemoteViAttribs, OUT VIP_CONN_HANDLE * ConnHandle);

extern VIP_RETURN VipConnectAccept(IN VIP_CONN_HANDLE ConnHandle, IN VIP_VI_HANDLE ViHandle);

extern VIP_RETURN VipConnectReject(IN VIP_CONN_HANDLE ConnHandle);

extern VIP_RETURN
VipConnectRequest(IN VIP_VI_HANDLE ViHandle,
		  IN VIP_NET_ADDRESS * LocalAddr,
		  IN VIP_NET_ADDRESS * RemoteAddr, IN VIP_ULONG Timeout, OUT VIP_VI_ATTRIBUTES * RemoteViAttribs);

extern VIP_RETURN VipDisconnect(IN VIP_VI_HANDLE ViHandle);

extern VIP_RETURN VipCreatePtag(IN VIP_NIC_HANDLE NicHandle, OUT VIP_PROTECTION_HANDLE * Ptag);

extern VIP_RETURN VipDestroyPtag(IN VIP_NIC_HANDLE NicHandle, IN VIP_PROTECTION_HANDLE Ptag);

extern VIP_RETURN
VipRegisterMem(IN VIP_NIC_HANDLE NicHandle,
	       IN VIP_PVOID VirtualAddress,
	       IN VIP_ULONG Length, IN VIP_MEM_ATTRIBUTES * MemAttribs, OUT VIP_MEM_HANDLE * MemoryHandle);

extern VIP_RETURN
VipDeregisterMem(IN VIP_NIC_HANDLE NicHandle, IN VIP_PVOID VirtualAddress, IN VIP_MEM_HANDLE MemoryHandle);

extern VIP_RETURN
VipPostSend(IN VIP_VI_HANDLE ViHandle, IN VIP_DESCRIPTOR * DescriptorPtr, IN VIP_MEM_HANDLE MemoryHandle);

extern VIP_RETURN VipSendDone(IN VIP_VI_HANDLE ViHandle, OUT VIP_DESCRIPTOR ** DescriptorPtr);

extern VIP_RETURN VipSendWait(IN VIP_VI_HANDLE ViHandle, IN VIP_ULONG Timeout, OUT VIP_DESCRIPTOR ** DescriptorPtr);

extern VIP_RETURN
VipPostRecv(IN VIP_VI_HANDLE ViHandle, IN VIP_DESCRIPTOR * DescriptorPtr, IN VIP_MEM_HANDLE MemoryHandle);

extern VIP_RETURN VipRecvDone(IN VIP_VI_HANDLE ViHandle, OUT VIP_DESCRIPTOR ** DescriptorPtr);

extern VIP_RETURN VipRecvWait(IN VIP_VI_HANDLE ViHandle, IN VIP_ULONG Timeout, OUT VIP_DESCRIPTOR ** DescriptorPtr);

extern VIP_RETURN VipCQDone(IN VIP_CQ_HANDLE CQHandle, OUT VIP_VI_HANDLE * ViHandle, OUT VIP_BOOLEAN * RecvQueue);

extern VIP_RETURN
VipCQWait(IN VIP_CQ_HANDLE CQHandle, IN VIP_ULONG Timeout, OUT VIP_VI_HANDLE * ViHandle, OUT VIP_BOOLEAN * RecvQueue);

extern VIP_RETURN
VipSendNotify(IN VIP_VI_HANDLE ViHandle,
	      IN VIP_PVOID Context,
	      IN void (*Handler) (IN VIP_PVOID Context,
				  IN VIP_NIC_HANDLE NicHandle,
				  IN VIP_VI_HANDLE ViHandle, IN VIP_DESCRIPTOR * DescriptorPtr)
    );

extern VIP_RETURN
VipRecvNotify(IN VIP_VI_HANDLE ViHandle,
	      IN VIP_PVOID Context,
	      IN void (*Handler) (IN VIP_PVOID Context,
				  IN VIP_NIC_HANDLE NicHandle,
				  IN VIP_VI_HANDLE ViHandle, IN VIP_DESCRIPTOR * DescriptorPtr)
    );

extern VIP_RETURN
VipCQNotify(IN VIP_CQ_HANDLE CQHandle,
	    IN VIP_PVOID Context,
	    IN void (*Handler) (IN VIP_PVOID Context,
				IN VIP_NIC_HANDLE NicHandle, IN VIP_VI_HANDLE ViHandle, IN VIP_BOOLEAN RecvQueue)
    );

extern VIP_RETURN VipCreateCQ(IN VIP_NIC_HANDLE NicHandle, IN VIP_ULONG EntryCount, OUT VIP_CQ_HANDLE * CQHandle);

extern VIP_RETURN VipDestroyCQ(IN VIP_CQ_HANDLE CQHandle);

extern VIP_RETURN VipResizeCQ(IN VIP_CQ_HANDLE CQHandle, IN VIP_ULONG EntryCount);

extern VIP_RETURN VipQueryNic(IN VIP_NIC_HANDLE NicHandle, OUT VIP_NIC_ATTRIBUTES * NicAttribs);

extern VIP_RETURN VipSetViAttributes(IN VIP_VI_HANDLE ViHandle, IN VIP_VI_ATTRIBUTES * ViAttribs);

extern VIP_RETURN
VipQueryVi(IN VIP_VI_HANDLE ViHandle,
	   OUT VIP_VI_STATE * State,
	   OUT VIP_VI_ATTRIBUTES * ViAttribs, OUT VIP_BOOLEAN * ViSendQEmpty, OUT VIP_BOOLEAN * ViRecvQEmpty);

extern VIP_RETURN
VipSetMemAttributes(IN VIP_NIC_HANDLE NicHandle,
		    IN VIP_PVOID Address, IN VIP_MEM_HANDLE MemHandle, IN VIP_MEM_ATTRIBUTES * MemAttribs);

extern VIP_RETURN
VipQueryMem(IN VIP_NIC_HANDLE NicHandle,
	    IN VIP_PVOID Address, IN VIP_MEM_HANDLE MemHandle, OUT VIP_MEM_ATTRIBUTES * MemAttribs);

extern VIP_RETURN
VipQuerySystemManagementInfo(IN VIP_NIC_HANDLE NicHandle, IN VIP_ULONG InfoType, OUT VIP_PVOID SysManInfo);

extern VIP_RETURN
VipErrorCallback(IN VIP_NIC_HANDLE NicHandle,
		 IN VIP_PVOID Context, IN void (*Handler) (IN VIP_PVOID Context, IN VIP_ERROR_DESCRIPTOR * ErrorDesc)
    );

/**********************************************************************
 * Peer-to-Peer Connection Model APIs
 *********************************************************************/

extern VIP_RETURN
VipConnectPeerRequest(IN VIP_VI_HANDLE ViHandle,
		      IN VIP_NET_ADDRESS * LocalAddr, IN VIP_NET_ADDRESS * RemoteAddr, IN VIP_ULONG Timeout);

extern VIP_RETURN VipConnectPeerDone(IN VIP_VI_HANDLE ViHandle, OUT VIP_VI_ATTRIBUTES * RemoteViAttribs);

extern VIP_RETURN VipConnectPeerWait(IN VIP_VI_HANDLE ViHandle, OUT VIP_VI_ATTRIBUTES * RemoteViAttribs);

/**********************************************************************
 * Name service APIs
 *********************************************************************/

extern VIP_RETURN VipNSInit(IN VIP_NIC_HANDLE NicHandle, IN VIP_PVOID NSInitInfo);

extern VIP_RETURN
VipNSGetHostByName(IN VIP_NIC_HANDLE NicHandle,
		   IN VIP_CHAR * Name, OUT VIP_NET_ADDRESS * Address, IN VIP_ULONG NameIndex);

extern VIP_RETURN
VipNSGetHostByAddr(IN VIP_NIC_HANDLE NicHandle,
		   IN VIP_NET_ADDRESS * Address, OUT VIP_CHAR * Name, IN OUT VIP_ULONG * NameLen);

extern VIP_RETURN VipNSShutdown(IN VIP_NIC_HANDLE NicHandle);

#endif				/* _VIPL_ */
