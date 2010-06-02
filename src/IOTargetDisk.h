/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / IOTargetDisk.h                                            ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Interface for an implementation of the Target class   ## */
/* ##               for disks.                                            ## */
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
/* ##               - Replaced the [BIG|LITTLE]_ENDIAN_ARCH defines by    ## */
/* ##                 IsBigEndian() function calls.                       ## */
/* ##               2003-07-19 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Removed IOTime.h inclusion (now in IOCommon.h)      ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef TARGET_DISK_DEFINED
#define TARGET_DISK_DEFINED

#include "IOCommon.h"
#include "IOTarget.h"
#include "IOTest.h"
#include "IOCQAIO.h"

#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
#ifndef _LP64
#define _LP64			/* to get at the 64 bit max long. */
#define _LP64_DEFINED
#endif				/* _LP64 */
#define MAX_DISK_SIZE	LONG_MAX
#ifdef _LP64_DEFINED
#undef _LP64
#endif				/* _LP64_DEFINED */
#elif defined(IOMTR_OSFAMILY_WINDOWS)
#define MAX_DISK_SIZE	_I64_MAX
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

#define MAX_PARTITIONS	26

// File names used to access a given drive.
#define LOGICAL_DISK	":\\"
#define PHYSICAL_DISK	"\\\\.\\PHYSICALDRIVE"
#define TEST_FILE		"iobw.tst"

#if defined(IOMTR_OSFAMILY_UNIX)
#define ERROR_DISK_FULL					ENOSPC
#define SECTOR_SIZE						512
#if defined(IOMTR_OS_SOLARIS)
#define RAW_DEVICE_DIR					"/dev/rdsk"
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_OSX)
#define RAW_DEVICE_DIR					"/dev"
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
#elif defined(IOMTR_OSFAMILY_NETWARE)
#define ERROR_DISK_FULL					1
#define SECTOR_SIZE						512
LONG NWalertroutine(unsigned long, unsigned long, unsigned long, unsigned long);
#endif

//
// Logical or physical disk drives.
//
class TargetDisk:public Target {
      public:

	TargetDisk();
	~TargetDisk();

	BOOL Initialize(Target_Spec * target_info, CQ * cq);

#ifndef USE_NEW_DISCOVERY_MECHANISM

#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	// Initialize logical disks (e.g. C:, D:, E:, etc.).
	BOOL Init_Logical(char drive);
	// Initialize physical (system) disks (e.g. physicaldisk0, physicaldisk1, etc.).
	BOOL Init_Physical(int drive);
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)

	BOOL Init_Logical(char *drive);
	BOOL Init_Physical(char *drive);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
#else
	// All systems should use the same interface
	BOOL Init_Logical(char *drive);
	BOOL Init_Physical(char *drive);
#endif



	void Set_Size(DWORDLONG maximum_size = 0);
	void Set_Starting_Sector(DWORDLONG starting_sector = 0);

	BOOL Prepare(void *buffer, DWORDLONG * prepare_offset, DWORD bytes, volatile TestState * test_state);

	BOOL Open(volatile TestState * test_state, int open_flag = 0);
	BOOL Close(volatile TestState * test_state);

	ReturnVal Read(LPVOID buffer, Transaction * trans);
	ReturnVal Write(LPVOID buffer, Transaction * trans);

	// Set the offset to the next position on the disk (random or sequential).
	void Seek(BOOL random, DWORD request_size, DWORD user_alignment, DWORDLONG user_align_mask);

	char file_name[MAX_NAME];

      protected:

	 BOOL Set_Sizes(BOOL open_disk = TRUE);	// Get physical drive dimensions.
	void Set_Sector_Info();

      private:

	 CQAIO * io_cq;
	HANDLE disk_file;

#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
	struct File file_handle;
#endif

	DWORDLONG size;		// Size of the disk in bytes.
	DWORD alignment;	// Alignment I/Os on this address.
	DWORDLONG sector_align_mask;	// Bit mask to align requests to sector sizes.
	// This only works for sectors a power of 2.
	// Set to NOT_POWER_OF_TWO otherwise.
	DWORDLONG starting_position;	// First bytes where transfers occur.
	DWORDLONG ending_position;	// Last byte where transfers can occur.
	DWORDLONG offset;
	DWORD bytes_transferred;	// Number of bytes successfully transferred to the disk.
	unsigned char* randomDataBuffer;

#if defined(IOMTR_OSFAMILY_NETWARE)
	LONG mmAppTag;
	HNDL reservationHandle;
	HNDL applicationHandle;
	OBID objectid;
	LONG reservationmode;
	LONG alerttoken;
	struct ApplicationRegistrationDef1 {
		LONG classobjectsignature;
		BYTE *name;
		 LONG(*controlroutine) (LONG token, LONG _function, LONG p1, LONG p2, LONG p3, LONG bufferlength,
					void *buffer);
		LONG privilegedapplicationkey;
		LONG type;
		LONG token;
		LONG identifier;
	} appDef;
      public:
	int NWOpenDevice(LONG device, LONG mode);
	int NWCloseDevice(HNDL handle);
#endif
#if defined(IOMTR_OS_SOLARIS)
	BOOL Look_For_Partitions();	// private member function to look for partitions on disk.
	DWORDLONG Get_Partition_Size(char *, int);	// private member function to get the partition size.
	DWORDLONG Get_Slice_Size(char *, int);	// private member function to get the slice size.
#endif
};

#endif
