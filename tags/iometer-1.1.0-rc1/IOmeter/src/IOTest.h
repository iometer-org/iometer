/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer & Dynamo) / IOTest.h                                      ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Common include file for Dynamo and Iometer, defining  ## */
/* ##               data structures used to describe a test.              ## */
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
/* ##  Remarks ...: - If you make any changes to this file, you need to   ## */
/* ##                 change Iometer/Dynamo's version number and make     ## */
/* ##                 corresponding changes in ByteOrder.cpp.             ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Changes ...: 2004-09-01 (henryx.w.tieman@intel.com)                ## */
/* ##               - Type change to make structure size stable across    ## */
/* ##                 platforms.                                          ## */
/* ##               2004-03-26 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Code cleanup to ensure common style.                ## */
/* ##               - Applied Thayne Harmon's patch for supporting        ## */
/* ##                 Netware support (on I386).                          ## */
/* ##               2004-02-15 (mingz@ele.uri.edu)                        ## */
/* ##               - Added padding in struct Target_Spec because of      ## */
/* ##                 the alignment issue between IA32 and ARM arch.      ## */
/* ##               2003-10-15 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef TEST_SPEC_DEFINED
#define TEST_SPEC_DEFINED

#include "IOAccess.h"
#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
#include "IOCommon.h"
#endif
#include "vipl.h"

#define PHYSICAL_DRIVE_PREFIX	"PHYSICALDRIVE:"

#include "pack.h"

//
// Specifications for a single test for one worker.
//
struct Test_Spec {
	char name[MAX_WORKER_NAME];
	int default_assignment;
	Access_Spec access[MAX_ACCESS_SPECS];
} STRUCT_ALIGN_IOMETER;

//
// Different type of I/O targets and workers.
//
enum TargetType {
	// Valid...             0xX0000000
	GenericType = 0x80000000,
	ActiveType = 0x40000000,

	// Disk...              0x-X000000
	GenericDiskType = 0x88000000,
	PhysicalDiskType = 0x8C000000,

#ifdef USE_NEW_DISCOVERY_MECHANISM
	// 
	// Optional TODO for other OSes.
	//

	// By tagging the raw disk with this type tells iometer GUI to chain
	// it under the preceding logical disk. This means, that following
	// a logical disk you would insert a physical disk with this type
	// if you wanted iometer to associate them. For a software raid, you 
	// would insert all of the disks belonging to that logical disk with this
	// type.
	//
	// If you have multiple striped partitions over the same group of 
	// physical disks, it is up to you if you duplicate the same physical
	// disks after every logical disk, or just show them once.
	//
	PhysicalDiskTypeHasPartitions = 0x8C100000, 
#endif

	LogicalDiskType = 0x8A000000,

	// Network...           0x-00X0000
	GenericNetType = 0x80080000,
	GenericServerType = 0x800C0000,
	GenericClientType = 0x800A0000,

	// TCP...               0x-00-X000
	GenericTCPType = 0x80088000,
	TCPServerType = 0x800C8000,
	TCPClientType = 0x800A8000,

	// VI...                0x-00-0X00
	GenericVIType = 0x80080800,
	VIServerType = 0x800C0800,
	VIClientType = 0x800A0800,

	// Invalid...           0xX0000000
	InvalidType = 0x00000000
};

#define IsType(src, chk) ((src & chk) == chk)

// Allows comparing two types for worker compatibility.
const int WORKER_COMPATIBILITY_MASK = GenericDiskType | GenericServerType | GenericClientType;

// Allows comparing a server/client pair for compatibility.
const int NETWORK_COMPATIBILITY_MASK = GenericTCPType | GenericVIType;

//
// Disk specific specifications for drives accessed during a test.
//
struct Disk_Spec {
#ifdef USE_NEW_DISCOVERY_MECHANISM

	// This member is used during the Windows disk discovery phase.
	// It was easiest to put it here. Can be used by other OSes for 
	// additional per disk context. e.g. scsi address, etc...
	// todo: byteordering!
	DWORD disk_reserved[5];

	BOOL has_partitions;// rawdisk has partitions (set in TargetDisk::Set_Sizes)
	BOOL not_mounted; // volume is recognized but unmounted (set in iomanager)
#endif

	BOOL ready;

	int sector_size;
	DWORDLONG maximum_size;
	DWORDLONG starting_sector;
} STRUCT_ALIGN_IOMETER;

//
// TCP specific specifications for TCP networks accessed during a test.
//
struct TCP_Spec {
	// Address of local and remote TCP connection.
	unsigned int local_port;

	char remote_address[MAX_NAME];
	unsigned int remote_port;
} STRUCT_ALIGN_IOMETER;

#define VI_ADDRESS_SIZE				16
// (Note that current VI hardware only supports a 4 byte 
// discriminator.  - 08/24/1998 - SH)
#define VI_DISCRIMINATOR_TYPE		int
#define VI_DISCRIMINATOR_SIZE		(sizeof(VI_DISCRIMINATOR_TYPE))
//
// VI specific specifications for VI networks accessed during a test.
//
struct VI_Spec {
	// Name and address of local and remote VI NICs to connect.
	// Since the VIP_NET_ADDRESS can be variable length, we use fill bytes to ensure
	// that there's enough room to store the entire address.  The total number of bytes
	// available for the address and discriminator is VI_ADDRESS_SIZE + VI_DISCRIMINATOR_SIZE.
	VIP_NET_ADDRESS local_address;
	char local_address_fill_bytes[VI_ADDRESS_SIZE + VI_DISCRIMINATOR_SIZE - 1];

	char remote_nic_name[MAX_NAME];
	VIP_NET_ADDRESS remote_address;
	char remote_address_fill_bytes[VI_ADDRESS_SIZE + VI_DISCRIMINATOR_SIZE - 1];

	// VI specific limitations.
	int max_transfer_size;
	int max_connections;
	int outstanding_ios;
#ifndef FORCE_STRUCT_ALIGN
	// Different compilers have different structure member alignment rules, w/out
	// an alignment directive, this is the behavior you see!
#if defined(IOMTR_OSFAMILY_NETWARE)
	char padnw[2];		// this has to keep changing and I do not know why
	// orginally it was none, had to make it 4 to work with NetWare now 2??
	// somebodies data size is changing on me and I don't know whose  or why
#endif
#endif

} STRUCT_ALIGN_IOMETER;

//
// Possible specifications for a generic target to have.
//
struct Target_Spec {
	// Name and type of target.
	char name[MAX_NAME]; // Device name -- for Windows, this is used to display the friendly name
						 // that has no resembelance to the string representing the device.

#ifdef USE_NEW_DISCOVERY_MECHANISM
	char actual_name[MAX_NAME]; // For Windows, this is the real device string
	char basic_name[MAX_NAME]; // For Windows, used internally for sorting.

	BOOL read_only;
	BOOL reserved;
#endif

	BOOL test_connection_rate;
	TargetType type;

	// Target type specific specifications.
	union {
		Disk_Spec disk_info;
		TCP_Spec tcp_info;
		VI_Spec vi_info;
	};

	// Target independent test specifications.
	int queue_depth;

	// Data pattern type for writes
	int DataPattern;

	int trans_per_conn;

#ifndef FORCE_STRUCT_ALIGN
	char padding[4];	// xscale and ia32 arch difference. need this padding.
#endif
	// Random value used to keep connections in synch.
	DWORDLONG random;

	// Variables for Using Fixed Seed for RNG
	BOOL use_fixed_seed;
	DWORDLONG fixed_seed_value;
} STRUCT_ALIGN_IOMETER;

#include "unpack.h"

#endif
