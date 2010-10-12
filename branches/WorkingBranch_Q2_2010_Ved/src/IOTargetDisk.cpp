/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / IOTargetDisk.cpp                                          ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implements a Target subclass named TargetDisk.        ## */
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
/* ##               2005-04-02 (moozart@gmx.de)                           ## */
/* ##               - Corrected a overflow bug in getSizeOfPhysDisk().    ## */
/* ##               2004-09-26 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Removed a note about the Reported_As_Logical()      ## */
/* ##                 function as this one was removed.                   ## */
/* ##               2004-09-01 (henryx.w.tieman@intel.com)                ## */
/* ##               - Initialize iocq structure member.                   ## */
/* ##               - Cleanup some function pointer math.                 ## */
/* ##               - Clean a few comments with non-printing characters.  ## */
/* ##               - Correct the size of data stored through pointers.   ## */
/* ##               2004-08-21 (mingz@ele.uri.edu)                        ## */
/* ##               - Changed "/dev" to RAW_DEVICE_DIR in                 ## */
/* ##                 getSectorSizeOfPhysDisk() and getSizeOfPhysDisk()   ## */
/* ##               2004-05-27 (lamontcranston41@yahoo.com)               ## */
/* ##               - Close files in getSizeOfPhysDisk and                ## */
/* ##                 getSectorSizeOfPhysDisk for linux                   ## */
/* ##               - Complete partial I/O properly to prevent disk       ## */
/* ##                 preparation from hanging                            ## */
/* ##               2004-03-27 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Applied Dan Bar Dov's patch for adding              ## */
/* ##                 Linux on PPC support.                               ## */
/* ##               - Code cleanup to ensure common style.                ## */
/* ##               - Applied Thayne Harmon's patch for supporting        ## */
/* ##                 Netware support (on I386).                          ## */
/* ##               2004-02-12 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved BLKSSZGET, BLKBSZGET and BLKGETSIZE64         ## */
/* ##                 from here to the IOCommon.h file.                   ## */
/* ##               2004-02-06 (mingz@ele.uri.edu)                        ## */
/* ##               - Added Set_Starting_Sector() code for XScale         ## */
/* ##               - Changed getSectorSizeOfPhysDisk() to use BLKBSZGET  ## */
/* ##                 ioctl call to get sector size. Dmitry Yusupov       ## */
/* ##                 suggest this;                                       ## */
/* ##               - Removed getSectorTableLoc() and getDevNums()        ## */
/* ##               - Changed getSizeOfPhysDisk() to use BLKGETSIZE64     ## */
/* ##                 ioctl call to get disk size                         ## */
/* ##               2003-10-17 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Applied the large_work_file patch from              ## */
/* ##                 Russell Reed (for files >2GB on 32 bit).            ## */
/* ##               2003-08-05 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-03-28 (joe@eiler.net)                            ## */
/* ##               - changes so VC++ 7 (.NET) will compile correctly.    ## */
/* ##               2003-03-05 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Removed LINUX_DEBUG, because it is redundant.       ## */
/* ##                 We can use the generic _DEBUG therefor.             ## */
/* ##               2003-03-04 (joe@eiler.net)                            ## */
/* ##               - Cleaned up some compiler warnings for Solaris       ## */
/* ##               2003-02-27 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added output of major & minor number within         ## */
/* ##                 the getSectorSizeOfPhysDisk() method.               ## */
/* ##               2003-02-15 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Fixed the getSectorSizeOfPhysDisk() method to       ## */
/* ##                 detect the sector size was on major and minor       ## */
/* ##                 number (instead on major number only).              ## */
/* ##               2003-02-02 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##               - Applied the iometer-initial-largefile.patch file    ## */
/* ##                 (modifies the Open() method and adds O_LARGEFILE    ## */
/* ##                 to the flags field of the open() function calls).   ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include "IOCommon.h"
#include "IOTargetDisk.h"
#include "IOAccess.h"

#define _DISK_MSGS 0

#if defined(IOMTR_OSFAMILY_NETWARE)
#include <assert.h>
#include <math.h>
#include <monitor.h>
#include <library.h>
#include <netware.h>
#elif defined(IOMTR_OSFAMILY_UNIX)
#ifdef WORKAROUND_MOD_BUG
#include <math.h>
#endif				// WORKAROUND_MOD_BUG

#if defined(IOMTR_OS_SOLARIS)
#if defined(IOMTR_CPU_I386) || defined(IOMTR_CPU_IA64)
   // highly specific to Solaris on Intel
#include <sys/dktp/fdisk.h>
#endif
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <sys/dkio.h>
#include <sys/vtoc.h>
#endif

#if defined(IOMTR_OS_LINUX)
#include <assert.h>
#include <sys/vfs.h>
#endif

#if defined(IOMTR_OS_OSX)
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/ioctl.h>
#include <sys/disk.h>
#endif
#endif

/**********************************************************************
 * Forward Declarations
 **********************************************************************/
#if defined(IOMTR_OS_LINUX)
static int getSectorSizeOfPhysDisk(const char *devName);
static unsigned long long getSizeOfPhysDisk(const char *devName);
#endif

//
// Constructor.
//
TargetDisk::TargetDisk()
{
#if defined(IOMTR_OSFAMILY_NETWARE)
	LONG rc;
#endif
	sector_align_mask = NOT_POWER_OF_TWO;

#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
	disk_file = (HANDLE) & file_handle;
	file_handle.iocq = NULL;
#endif

#if defined(IOMTR_OSFAMILY_NETWARE)
	mmAppTag = (LONG) AllocateResourceTag(getnlmhandle(), "dynamo", (LONG) MM_APPLICATION_SIGNATURE);
	appDef.classobjectsignature = MM_APPLICATION_SIGNATURE;
	appDef.controlroutine = NULL;
	appDef.name = (unsigned char *)"mmDynamo";
	appDef.type = MM_GENERAL_STORAGE_APPLICATION;
	appDef.identifier = 0xEEDDCCBB;

	if ((rc = MM_RegisterObject(&applicationHandle, MM_APPLICATION_CLASS, &appDef, NULL, mmAppTag)) != MM_OK) {
		printf("Failed MM_RegisterObject (0x%X)\n", rc);
		exit(1);
	}
#endif

	memset(&spec, 0, sizeof(spec));
}

TargetDisk::~TargetDisk()
{
#if defined(IOMTR_OS_NETWARE)
	MM_UnregisterObject(applicationHandle, 0);
#endif
};

//
// Initializing the disk based on the specified target information.
//
BOOL TargetDisk::Initialize(Target_Spec * target_info, CQ * cq)
{
	BOOL retval;

	io_cq = (CQAIO *) cq;
#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
	file_handle.iocq = (IOCQ *) io_cq->completion_queue;
#endif
	memcpy(&spec, target_info, sizeof(Target_Spec));

	// Initializing logical disks.
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
#ifdef USE_NEW_DISCOVERY_MECHANISM
	if (IsType(target_info->type, LogicalDiskType))
		retval = Init_Logical(spec.actual_name);
	else if (IsType(target_info->type, PhysicalDiskType))
		retval = Init_Physical(spec.actual_name);
#else
	if (IsType(target_info->type, LogicalDiskType))
		retval = Init_Logical(spec.name[0]);
	else if (IsType(target_info->type, PhysicalDiskType))
		retval = Init_Physical(atoi(spec.name + 14));
#endif
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
	if (IsType(target_info->type, LogicalDiskType))
		retval = Init_Logical(spec.name);
	else if (IsType(target_info->type, PhysicalDiskType))
		retval = Init_Physical(spec.name);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

	else {
		cout << "*** Invalid disk type in TargetDisk::Initialize()." << endl;
		retval = FALSE;
	}

	if (retval) {
		// Setting size of disk, if it was successfully initialized
		Set_Size(spec.disk_info.maximum_size);

#if defined(IOMTR_CPU_SPARC)
		// To tell the truth, I have no idear, if this is
		// a.) Only specific to the Sparc systems (so Linux on Sparc has to do the same)
		// b.) Only specific to the Solaris operating system (so Solaris on i386 has to do the same)
		// c.) Specific to the Sparc system / Solaris operating system combination (current implementation)
#if defined(IOMTR_OS_SOLARIS)
		if (spec.disk_info.starting_sector)
			Set_Starting_Sector(spec.disk_info.starting_sector);
		else if (IsType(target_info->type, PhysicalDiskType)) {
			// always skip sector 0 for non-i386 || IA64 platform (only for raw disks).
			// We need it to avoid destroying the vtoc table which is
			// not protected on these (sparc) platforms.
			//
			// But this is serious performance implications.
			// Can slow down access quite a bit.
			//
			Set_Starting_Sector(1);
		}
#else
#error ===> ERROR: Broken port, advice needed!
#endif
#elif defined(IOMTR_CPU_I386) || defined(IOMTR_CPU_IA64) || defined(IOMTR_CPU_PPC) || defined(IOMTR_CPU_X86_64)
		Set_Starting_Sector(spec.disk_info.starting_sector);
#elif defined(IOMTR_CPU_XSCALE)
		// TODO: Need to double check if this is correct for xscale
		cout << spec.disk_info.starting_sector << endl;
		Set_Starting_Sector(spec.disk_info.starting_sector);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	}

	return retval;
}

#if defined(IOMTR_OS_NETWARE)
// NetWare logical drives are accessed through path names.
BOOL TargetDisk::Init_Logical(char *drive)
{
	// Setting spec.name of the drive.
	snprintf(spec.name, MAX_NAME, "%s", drive);

	if (spec.name[strlen(spec.name) - 1] == ':') {
		snprintf(file_name, MAX_NAME, "%s%s", spec.name, TEST_FILE);
	} else {
		snprintf(file_name, MAX_NAME, "%s:%s", spec.name, TEST_FILE);
	}

	spec.type = LogicalDiskType;
	size = 0;
	starting_position = 0;
	offset = 0;
	bytes_transferred = 0;

	// Getting size information about the drive.
	return (Set_Sizes());
}
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
// UNIX logical drives are accessed through path names.
BOOL TargetDisk::Init_Logical(char *drive)
{
	// Strip off file system Information about the logical drives.
	char *p;

	// check for this pattern is also in Manager::Report_Disks()
	p = strstr(drive, " [");
	if (p) {
		strncpy(spec.name, drive, p - drive);
		spec.name[p - drive] = 0;
	} else
		// Setting spec.name of the drive.
		snprintf(spec.name, MAX_NAME, "%s", drive);

	if (spec.name[strlen(spec.name) - 1] == '/') {
		snprintf(file_name, MAX_NAME, "%s%s", spec.name, TEST_FILE);
	} else {
		snprintf(file_name, MAX_NAME, "%s/%s", spec.name, TEST_FILE);
	}

	spec.type = LogicalDiskType;
	size = 0;
	starting_position = 0;
	offset = 0;
	bytes_transferred = 0;

	// Getting size information about the drive.
	return (Set_Sizes());
}
#elif defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
#ifdef USE_NEW_DISCOVERY_MECHANISM
BOOL TargetDisk::Init_Logical( char *drive )
{
	// Setting spec.name of the drive.
	//sprintf( spec.name, "%s", drive);
	strcpy(spec.name, drive);

	// Save the name to the actual partition device: \\.\c:, not the file path
	if (drive[0] != '\\')
	{
		sprintf(spec.actual_name, "\\\\.\\%s", drive);
	}
	else
	{
		//sprintf(spec.actual_name, "%s", drive);
		strcpy(spec.actual_name, drive);
	}

	// Whack the trailing slash.
	if (spec.actual_name[strlen(spec.actual_name) - 1] == '\\')
	{
		spec.actual_name[strlen(spec.actual_name) - 1] = '\0';
	}

	sprintf( file_name, "%s\\%s", spec.actual_name, TEST_FILE );
	
#else
//
// Initialize a logical disk drive.  Logical drives are accessed through
// a drive letter and may be local or remote.
//
BOOL TargetDisk::Init_Logical(char drive)
{
	// Setting spec.name of the drive.
	snprintf(spec.name, MAX_NAME, "%c%s", drive, LOGICAL_DISK);
	snprintf(file_name, MAX_NAME, "%s%s", spec.name, TEST_FILE);

#endif //USE_NEW_DISCOVERY_MECHANISM

	spec.type = LogicalDiskType;
	size = 0;
	starting_position = 0;
	offset = 0;
	bytes_transferred = 0;

	// Getting size information about the drive.
	return (Set_Sizes());
}

 
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

#if defined(IOMTR_OS_NETWARE)
BOOL TargetDisk::Init_Physical(char *drive)
{
	struct IOObjectGenericInfoDef info;
	int drive1 = atoi(drive + 1);

	// Setting the spec.name of the drive.
	printf("TargetDisk::Init_Physical: name=%s, mmID=%X\n", drive, drive1);
	MM_ReturnObjectGenericInfo(drive1, sizeof(struct IOObjectGenericInfoDef), &info);
	snprintf(spec.name, MAX_NAME, "[%d] %s", drive1, info.name);

	snprintf(file_name, MAX_NAME, "%d", drive1);

	spec.type = PhysicalDiskType;
	size = 0;
	starting_position = 0;
	offset = 0;
	bytes_transferred = 0;

	// Getting information about the size of the drive.
	return (Set_Sizes());
}
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
BOOL TargetDisk::Init_Physical(char *drive)
{
	// Setting the spec.name of the drive.
	// Strip off any extra information in the drive name.
	char *p;

	// check for this pattern is also in Manager::Report_Disks()
	// and Manager::Reported_As_Logical().
	p = strstr(drive, " [");
	if (p) {
		strncpy(spec.name, drive, p - drive);
		spec.name[p - drive] = 0;
	} else {
		// Setting spec.name of the drive.
		if (spec.name != drive)
			snprintf(spec.name, MAX_NAME, "%s", drive);
	}

	if (!strstr(spec.name, RAW_DEVICE_DIR))
		snprintf(file_name, MAX_NAME, "%s/%s", RAW_DEVICE_DIR, spec.name);
	else
		snprintf(file_name, MAX_NAME, "%s", spec.name);

	spec.type = PhysicalDiskType;
	size = 0;
	starting_position = 0;
	offset = 0;
	bytes_transferred = 0;

	// Getting information about the size of the drive.
	return (Set_Sizes());
}
#elif defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
//
// Initialize a physical disk drive.  Physical drives are accessed below
// the file system layer for RAW access.  As a result, data corruption could
// occur.  To prevent this, only drives which contain nothing but free space
// are accessible.
//
#ifdef USE_NEW_DISCOVERY_MECHANISM
BOOL TargetDisk::Init_Physical(char *drive)
{
	strcpy(spec.name, drive);
	strcpy(spec.actual_name, drive); // may be redundant?
	strcpy(file_name, spec.actual_name);

#else
BOOL TargetDisk::Init_Physical(int drive)
{
	// Setting the spec.name of the drive.
	snprintf(spec.name, MAX_NAME, "%s%i", PHYSICAL_DISK, drive);
	strcpy(file_name, spec.name);
#endif // USE_NEW_DISCOVERY_MECHANISM
	spec.type = PhysicalDiskType;
	size = 0;
	starting_position = 0;
	offset = 0;
	bytes_transferred = 0;

	// Getting information about the size of the drive.
	return (Set_Sizes());
}

#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

//
// Setting the maximum amount of disk space to access during testing.
//
void TargetDisk::Set_Size(DWORDLONG maximum_size)
{
	DWORDLONG new_size;

	// Getting the actual size of the disk.
	Set_Sizes();
	new_size = (DWORDLONG) maximum_size *(DWORDLONG) spec.disk_info.sector_size;

	// If the specified maximum size is not 0 and is smaller than the actual disk,
	// reset the disk size.
	if (new_size && (new_size < size)) {
		size = new_size;
#ifdef _DEBUG
		cout << "Resetting accessible size of disk " << spec.name << " to " <<
		    maximum_size << " sectors." << endl << "   " << spec.name << " size = " << size << endl;
#endif
	}
}

//
// Setting the starting sector of the disk where accesses should begin.
// This needs to be called after setting the size of the disk (call to Set_Size).
//
void TargetDisk::Set_Starting_Sector(DWORDLONG starting_sector)
{
	DWORDLONG current_size;

	starting_position = (DWORDLONG) starting_sector *(DWORDLONG) spec.disk_info.sector_size;

#ifdef _DEBUG
	cout << "Moving starting sector of disk " << spec.name << " to " << starting_position << "." << endl;
#endif

	// Moving the starting disk sector may have shrunk the accessible size of the disk.
	current_size = size;	// record the currently assigned size to use
	Set_Size();		// getting the actual disk size

	if (starting_position + current_size > size) {
		// Re-adjust the accessible size of the disk to reflect the new starting sector.
		size -= starting_position;
#ifdef _DEBUG
		cout << "Starting sector size has shrunk the effective size of the disk." << endl;
#endif
	} else {
		// The new starting sector did not affect the accessible disk size.
		size = current_size;
	}
	ending_position = (DWORDLONG) starting_position + size;
}

//
// Setting information about the physical size of the drive.  This is
// used in order to guarantee that accesses to the drive are permitted,
// that is align on the sector sizes, and to allow random accesses over
// the entire drive.
//
#if defined(IOMTR_OS_LINUX)
//
// Performs same function as the win32 || _WIN64 version.
//
BOOL TargetDisk::Set_Sizes(BOOL open_disk)
{
	struct stat fileInfo;
	struct statfs fsInfo;
	int statResult;
	int fd = -1;
	char filesysName[MAX_NAME];

	if (open_disk) {
		if (!Open(NULL)) {
			cout << __FUNCTION__ << ": Open on \"" << file_name <<
			    "\" failed (error " << strerror(errno) << ")." << endl;
			return (FALSE);
		}
		fd = ((struct File *)disk_file)->fd;
	}
	if (IsType(spec.type, LogicalDiskType)) {
		/*
		 * For logical disks, we use statfs and stat to find the size of the
		 * file system, the size of the test file, the sector size for the
		 * file system, etc. Pretty straightforward, standard Unix stuff.
		 */
		strcpy(filesysName, file_name);
		filesysName[strlen(filesysName) - strlen(TEST_FILE)] = '\0';
		if (open_disk) {
			statResult = fstatfs(fd, &fsInfo);
		} else {
			statResult = statfs(filesysName, &fsInfo);
		}
		if (statResult < 0) {
			cerr << __FUNCTION__ << ": Couldn't statfs logical disk file!" << endl;
			if (open_disk) {
				Close(NULL);
			}
			return (FALSE);
		}
		spec.disk_info.sector_size = fsInfo.f_bsize;
		sector_align_mask = ~((DWORDLONG) fsInfo.f_bsize - 1);
		/* Free blocks is "f_bfree". */
		if (open_disk) {
			statResult = fstat(fd, &fileInfo);
		} else {
			statResult = stat(file_name, &fileInfo);
		}
		if (statResult < 0) {
			cerr << __FUNCTION__ << ": Error " << strerror(errno) << "statting file " << file_name << endl;
			if (open_disk) {
				Close(NULL);
			}
			return (FALSE);
		}
		size = fileInfo.st_size;
		if (size == 0) {
			spec.disk_info.ready = FALSE;
			if (open_disk) {
				Close(NULL);
			}
			unlink(file_name);
			return (TRUE);
		}
		ending_position = size;
		spec.disk_info.ready = TRUE;
		if (open_disk) {
			Close(NULL);
		}
		return (TRUE);
	} else {
		spec.disk_info.sector_size = getSectorSizeOfPhysDisk(file_name);
		if (spec.disk_info.sector_size == 0) {
			cerr << __FUNCTION__ << ": Failed to get sector size for " << file_name << ". Skip it." << endl;
			if (open_disk) {
				Close(NULL);
			}
			return (FALSE);
		}
		size = getSizeOfPhysDisk(file_name);
		if (size == 0) {
			cerr << __FUNCTION__ << ": Failed to get device size for " << file_name << ". Skip it." << endl;
			if (open_disk)
				Close(NULL);
			return (FALSE);
		}
		alignment = 0;
		sector_align_mask = ~((DWORDLONG) spec.disk_info.sector_size - 1);
		ending_position = size;
		offset = 0;
		bytes_transferred = 0;
		spec.disk_info.ready = TRUE;
		if (open_disk) {
			Close(NULL);
		}
		return (TRUE);
	}
}
#elif defined(IOMTR_OS_NETWARE)
//
// Performs same function as the win32 || _WIN64 version.
//
BOOL TargetDisk::Set_Sizes(BOOL open_disk)
{
	struct stat fileInfo;
	struct stat fsInfo;
	int statResult;
	int fd = -1;
	char filesysName[MAX_NAME];
	struct volume_info spaceInfo;
	struct IOObjectGenericInfoDef info;

	if (open_disk) {
		if (!Open(NULL)) {
#ifdef _DEBUG
			cout << __FUNCTION__ << ": Open on \"" << file_name <<
			    "\" failed (error " << strerror(errno) << ")." << endl;
#endif
			return (FALSE);
		}
		fd = ((struct File *)disk_file)->fd;
	}
	if (IsType(spec.type, LogicalDiskType)) {
		/*
		 * For logical disks, we use statfs and stat to find the size of the
		 * file system, the size of the test file, the sector size for the
		 * file system, etc. Pretty straightforward, standard Unix stuff.
		 */
		strcpy(filesysName, file_name);
		filesysName[strlen(filesysName) - (strlen(TEST_FILE) + 1)] = '\0';	//and the colon
		if (open_disk) {
			statResult = fstat(fd, &fsInfo);
		} else {
			statResult = stat(filesysName, &fsInfo);
		}

		netware_vol_info(&spaceInfo, (int *)&fsInfo.st_dev);

		if (statResult < 0) {
			cerr << __FUNCTION__ << ": Couldn't stat logical disk file!" << endl;
			if (open_disk)
				Close(NULL);
			return (FALSE);
		}
		spec.disk_info.sector_size = spaceInfo.SectorsPerCluster;
		sector_align_mask = ~((DWORDLONG) spaceInfo.SectorsPerCluster - 1);
		/* Free blocks is "f_bfree". */
		if (open_disk) {
			statResult = fstat(fd, &fileInfo);
		} else {
			statResult = stat(file_name, &fileInfo);
		}
		if (statResult < 0) {
			cerr << __FUNCTION__ << ": Error " << strerror(errno) << " statting file " << file_name << endl;
			if (open_disk) {
				Close(NULL);
			}
			return (FALSE);
		}
//              size = fileInfo.st_size;
		size = spaceInfo.FreedClusters;
		if (size == 0) {
			spec.disk_info.ready = FALSE;
			if (open_disk) {
				Close(NULL);
			}
			unlink(file_name);
			return (TRUE);
		}
		ending_position = size;
		spec.disk_info.ready = TRUE;
		if (open_disk) {
			Close(NULL);
		}
		return (TRUE);
	} else if (IsType(spec.type, PhysicalDiskType))	// physical
	{
		MM_ReturnObjectGenericInfo(atoi(file_name), sizeof(struct IOObjectGenericInfoDef), &info);
		spec.disk_info.sector_size = info.unitsize;
		if (spec.disk_info.sector_size == 0) {
			cout << __FUNCTION__ << ": Failed to get sector size. Aborting target." << endl;
			return (FALSE);
		}
		size = info.capacity;
		alignment = 0;
		sector_align_mask = ~((DWORDLONG) spec.disk_info.sector_size - 1);
		ending_position = size;
		offset = 0;
		bytes_transferred = 0;
		spec.disk_info.ready = TRUE;
		if (open_disk)
			Close(NULL);
		return (TRUE);
	} else {
		cout << "*** Unexpected drive type in Disk::SetSizes()" << endl << flush;
		if (open_disk)
			Close(NULL);
		return (FALSE);
	}
}
#elif defined(IOMTR_OS_SOLARIS)
int TargetDisk::Set_Sizes(BOOL open_disk)
{
	DWORD fd;

//      DWORD                   i;
//      DWORD                   low_size, high_size;
//      DWORD                   sectors_per_cluster, free_clusters, total_clusters;
//      BOOL                    foundPartitions = FALSE;
//      DWORD                   disk_info_size;
	struct statvfs st;
	struct dk_geom disk_geo_info;
	struct vtoc disk_vtoc;

	// Logical and physical drives are treated differently.
	if (open_disk) {
		if (!Open(NULL))
			return (FALSE);
	}

	if (IsType(spec.type, LogicalDiskType)) {
		// Getting physical information about the drive. All we need is the sector size. !!!!
		if (statvfs(file_name, &st) < 0) {
			cout << "statvfs error " << errno << ". Cannot get sector size" << endl;
			if (open_disk)
				Close(NULL);
			return (FALSE);
		}
		// Which one of these two do we take as the sector size ?
		spec.disk_info.sector_size = (int)st.f_bsize;
		//           or
		// spec.disk_info.sector_size = SECTOR_SIZE;

		// Ok move on to the file size
		struct stat64 stbuf64;

		if (stat64(file_name, &stbuf64) < 0) {
			cout << "unable to get file size. stat64() failed with error " << errno << endl;
			if (open_disk)
				Close(NULL);
			return (FALSE);
		}
		size = stbuf64.st_size;

		Set_Sector_Info();
#ifdef _DEBUG
		cout << "   " << spec.name << " size = " << size << endl << flush;
#endif
		if (open_disk)
			Close(NULL);

		// If test file exists, the drive is ready for access.
		if (size) {
			ending_position = size;
			spec.disk_info.ready = TRUE;
			return (TRUE);
		}
		// Assuming that unlink always succeeds and never fails. how clever! #@$#%
		unlink(file_name);
		spec.disk_info.ready = FALSE;
		return (TRUE);
	} else if (IsType(spec.type, PhysicalDiskType)) {
		// Dealing with a physical drive.
		// Checking for a partition to exist on the physical drive.
		// If one is there, then disallowing access to the drive.
		SetLastError(0);

		if (!strstr(file_name, "p0")) {
			// We are probably working with a slice.
			int length;
			int part;
			char part_name[MAX_NAME];

			strcpy(part_name, spec.name);
			length = strlen(spec.name);
			part = atoi((char *)(part_name + length - 1));
			spec.disk_info.sector_size = SECTOR_SIZE;

			if ((char)part_name[length - 2] == 's') {
				// We are dealing with a slice
				size = Get_Slice_Size(part_name, part);
#ifdef _DEBUG
				cout << " slice size : " << size << endl;
#endif
				if (size <= 0) {
					if (open_disk)
						Close(NULL);
					return (FALSE);
				}
			}
#if defined(IOMTR_CPU_I386) || defined(IOMTR_CPU_IA64) || defined(IOMTR_CPU_PPC)
			else {
				// We are dealing with an fdisk partition.
				size = Get_Partition_Size(part_name, part);
#ifdef _DEBUG
				cout << " partn size : " << size << endl;
#endif
				if (size <= 0) {
					if (open_disk)
						Close(NULL);
					return (FALSE);
				}
			}
#endif				// __i386 || _IA64 || _PPC
#ifdef _DEBUG
			cout << "spec name : " << spec.name
			    << " part_name : " << part_name << " part : " << part << endl;
#endif
		} else {
			// Getting information on the size of the drive.
			fd = ((struct File *)disk_file)->fd;
			if (ioctl(fd, DKIOCG_PHYGEOM, &disk_geo_info) < 0) {
				// unable to get disk geometry. Report disk as NO_DRIVE
				if (open_disk)
					Close(NULL);
				return (FALSE);
			}
			// Getting sector size of the disk.
			// First try to get the sector size from the dkio(vtoc). 
			// If that fails, then return the default.

			// NOTE: NOTE: NOTE:
			// We do this because of a problem with ioctl(DKIOCGVTOC). It fails for a
			// freshly formatted disk. But once u create something on it and then 
			// even if you clean it all up, the call still succeeds.
			//
			if (ioctl(fd, DKIOCGVTOC, &disk_vtoc) < 0)
				spec.disk_info.sector_size = SECTOR_SIZE;
			else
				spec.disk_info.sector_size = disk_vtoc.v_sectorsz;

			// Calculating the size of the physical drive..
			size = ((DWORDLONG) disk_geo_info.dkg_ncyl * disk_geo_info.dkg_nhead *
				disk_geo_info.dkg_nsect * spec.disk_info.sector_size);
		}

		Set_Sector_Info();

#ifdef _DEBUG
		cout << "   " << spec.name << " size = " << size << endl << flush;
#endif

		if (open_disk)
			Close(NULL);

		ending_position = size;
		spec.disk_info.ready = TRUE;
		return (TRUE);
	} else {
		cout << "*** Unexpected drive type in Disk::SetSizes()" << endl << flush;
		if (open_disk)
			Close(NULL);
		return (FALSE);
	}
}
#elif defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
BOOL TargetDisk::Set_Sizes(BOOL open_disk)
{
	DWORD i;
	DWORD low_size, high_size;
	DWORD sectors_per_cluster, free_clusters, total_clusters;
	DWORD sector_size;
	DWORD disk_info_size;

	// Logical and physical drives are treated differently.
	if (open_disk) {
		if (!Open(NULL))
			return FALSE;
	}

	if (IsType(spec.type, LogicalDiskType)) {
		char temp_name[MAX_NAME];

		sprintf(temp_name, "%s\\", spec.name);

		// Getting physical information about the drive.
		if (!GetDiskFreeSpace(temp_name, &sectors_per_cluster, &sector_size, &free_clusters, &total_clusters)) {
			cout << "Error getting sector size of drive " << spec.name << "." << endl;
			if (open_disk)
				Close(NULL);
			return FALSE;
		}
		spec.disk_info.sector_size = (int)sector_size;

		// Determining if the test file exists or needs to be made.
		low_size = GetFileSize(disk_file, &high_size);
		size = (((DWORDLONG) high_size) << 32) | (DWORDLONG) low_size;
		Set_Sector_Info();
#ifdef _DEBUG
		cout << "   " << spec.name << " size = " << size << endl;
#endif
		if (open_disk)
			Close(NULL);

		// If test file exists, the drive is ready for access.
		if (size) {
			ending_position = size;
			spec.disk_info.ready = TRUE;
			return TRUE;
		}

		DeleteFile(file_name);
		spec.disk_info.ready = FALSE;
		return TRUE;
	} else if (IsType(spec.type, PhysicalDiskType)) {
		// Dealing with a physical drive.
		// Checking for a partition to exist on the physical drive.
		// If one is there, then disallowing access to the drive.

		// This is more data than we need, but its ok
		DRIVE_LAYOUT_INFORMATION disk_layout_info[MAX_PARTITIONS];
		DRIVE_LAYOUT_INFORMATION_EX disk_layout_info_ex[MAX_PARTITIONS];
		DISK_GEOMETRY disk_geo_info;
		DISK_GEOMETRY_EX disk_geo_info_ex;

		SetLastError(0);

#ifdef USE_NEW_DISCOVERY_MECHANISM
		spec.disk_info.has_partitions = FALSE;
#endif

		// Try the EX version first
		if (DeviceIoControl(disk_file, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0,
				disk_layout_info_ex, sizeof(disk_layout_info_ex), &disk_info_size, NULL)){

			// Checking that drive contains nothing but free space.
			for (i = 0; i < disk_layout_info_ex[0].PartitionCount; i++) {
				if (disk_layout_info_ex[0].PartitionEntry[i].PartitionLength.HighPart ||
					disk_layout_info_ex[0].PartitionEntry[i].PartitionLength.LowPart) 
				{
#ifdef USE_NEW_DISCOVERY_MECHANISM
					// Make the decision about whether to display this disk elsewhere...
					spec.disk_info.has_partitions = TRUE;
					break;
#else					
					if ( open_disk )
						Close( NULL );				
					cout << "Physical disk \'" << spec.name << "\' contains partition information." << endl;
					return FALSE;
#endif
				}
			}
		}
		else if (DeviceIoControl(disk_file, IOCTL_DISK_GET_DRIVE_LAYOUT, NULL, 0,
				disk_layout_info, sizeof(disk_layout_info), &disk_info_size, NULL))	{

			// Checking that drive contains nothing but free space.
			for (i = 0; i < disk_layout_info[0].PartitionCount; i++) {
				if (disk_layout_info[0].PartitionEntry[i].PartitionLength.HighPart ||
					disk_layout_info[0].PartitionEntry[i].PartitionLength.LowPart) 
				{
#ifdef USE_NEW_DISCOVERY_MECHANISM
					// Make the decision about whether to display this disk elsewhere...
					spec.disk_info.has_partitions = TRUE;
					break;
#else					
					if ( open_disk )
						Close( NULL );				
					cout << "Physical disk \'" << spec.name << "\' contains partition information." << endl;
					return FALSE;
#endif
				}
			}
		}
		else {
			cout << "TargetDisk::Set_Sizes() Failed getting drive layout info, error " 
				 << GetLastError() << "." << endl;
			return FALSE;
		}

		// Getting information on the size of the drive.
		size = 0;
		spec.disk_info.sector_size = 0;
		
		// Try the EX version first
		if (DeviceIoControl(disk_file, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0,
			&disk_geo_info_ex, sizeof(disk_geo_info_ex), &disk_info_size, NULL)) {

			size = disk_geo_info_ex.DiskSize.QuadPart;
			spec.disk_info.sector_size = disk_geo_info_ex.Geometry.BytesPerSector;
		}
		else if(DeviceIoControl(disk_file, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0,
			&disk_geo_info, sizeof(disk_geo_info), &disk_info_size, NULL)) {

			// Calculating the size of the physical drive..
			size = (((DWORDLONG) disk_geo_info.Cylinders.HighPart) << 32) |
				((DWORDLONG) disk_geo_info.Cylinders.LowPart);
			size *= (_int64) disk_geo_info.TracksPerCylinder *
				(_int64) disk_geo_info.SectorsPerTrack * (_int64) disk_geo_info.BytesPerSector;
			spec.disk_info.sector_size = disk_geo_info.BytesPerSector;
		}
		else {
			// print some warning and return error
			cout << "TargetDisk::Set_Sizes() Failed getting drive geometry, error " 
				 << GetLastError() << "." << endl;
			return FALSE;
		}

		Set_Sector_Info();

#ifdef _DEBUG
		cout << "   " << spec.name << " size = " << size << endl;
#endif

		if (open_disk)
			Close(NULL);
		ending_position = size;
		spec.disk_info.ready = TRUE;
		return TRUE;
	} else {
		cout << "*** Unexpected drive type in TargetDisk::SetSizes()" << endl;
		return FALSE;
	}
}
#elif defined(IOMTR_OS_OSX)
int TargetDisk::Set_Sizes(BOOL open_disk)
{
	struct statfs st;
	DWORD fd;

	if (open_disk) {
		if (!Open(NULL))
			return (FALSE);
	}

	if (IsType(spec.type, LogicalDiskType)) {
		if (statfs(file_name, &st) < 0) {
			cout << "statfs error " << errno << ". Cannot get sector size" << endl;
			if (open_disk)
				Close(NULL);
			return (FALSE);
		}
		spec.disk_info.sector_size = (int)st.f_bsize;

		// Ok move on the file size
		struct stat stbuf64;

		if (stat(file_name, &stbuf64) < 0) {
			cout << "unable to get file size. stat failed with error " << errno << endl;
			if (open_disk)
				Close(NULL);
			return (FALSE);
		}
		size = stbuf64.st_size;

		Set_Sector_Info();
#ifdef _DEBUG
		cout << "    " << spec.name << " size = " << size << endl << flush;
#endif
		if (open_disk)
			Close(NULL);

		// If test file exists, the drive is ready for access.
		if (size) {
			ending_position = size;
			spec.disk_info.ready = TRUE;
			return (TRUE);
		}
		// Assuming that unlink always succeeds and never fails
		unlink(file_name);
		spec.disk_info.ready = FALSE;
		return (TRUE);
	} else if (IsType(spec.type, PhysicalDiskType)) {
		SetLastError(0);

		fd = ((struct File *)disk_file)->fd;

		if (ioctl(fd, DKIOCGETBLOCKSIZE, &spec.disk_info.sector_size) < 0) {
			spec.disk_info.sector_size = 0;
		}

		if (ioctl(fd, DKIOCGETBLOCKCOUNT, &size) < 0) {
			size = 0;
		}

		size *= spec.disk_info.sector_size;

		Set_Sector_Info();

#ifdef _DEBUG
		cout << "    " << spec.name << " size = " << size << endl << flush;
#endif

		if (open_disk)
			Close(NULL);

		ending_position = size;
		spec.disk_info.ready = TRUE;
		return (TRUE);
	} else {
		cout << "*** Unexpected drive type in Disk::SetSizes()" << endl << flush;
		if (open_disk)
			Close(NULL);
		return (FALSE);
	}
}
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

//
// Setting additional information about the drive's sector size.
// The sector size of the drive needs to be known before calling this function.
//
void TargetDisk::Set_Sector_Info()
{
	DWORD temp_sector;

	// Check if sector size is a power of 2, by repeatedly dividing the sector size by 2 
	// (until we get to 1) and checking to see if the result is odd at any point.
	temp_sector = spec.disk_info.sector_size;
	while (temp_sector > 1) {
		if (temp_sector % 2) {
			sector_align_mask = NOT_POWER_OF_TWO;
			return;
		}
		temp_sector = temp_sector >> 1;
	}

	// The sector size is a power of 2, we can use a bit mask to align requests on
	// sector boundaries.  This improves performance.
	//
	// "& ~((DWORDLONG)spec.disk_info.sector_size - 1)" masks off lower bits to force sector alignment.  
	//     spec.disk_info.sector_size is cast to a DWORDLONG to force the result to be a 64-bit quantity.  
	//     For example, if spec.disk_info.sector_size = 512 (0x00000200):
	//           (DWORDLONG)spec.disk_info.sector_size      = 0x0000000000000200
	//          ((DWORDLONG)spec.disk_info.sector_size - 1) = 0x00000000000001FF
	//         ~((DWORDLONG)spec.disk_info.sector_size - 1) = 0xFFFFFFFFFFFFFE00
	sector_align_mask = ~((DWORDLONG) spec.disk_info.sector_size - 1);
}

#define PREPARE_QDEPTH	16

//
// Logical drives are accessed through a file on the drive.  This will create
// the file by writing in units of "bytes" beginning at "*prepare_offset".  It
// maintains a simple I/O queue of depth PREPARE_QDEPTH, which it keeps as 
// full as possible.  It keeps queueing up new I/Os until the disk fills up, 
// or the user-specified file size (starting_sector + maximum_size) is reached,
// or a STOP_PREPARE message is received (reflected in "*test_state").  It 
// returns only when all queued I/Os have completed.  
// Return value is TRUE for success, FALSE if any error occurred.
//
BOOL TargetDisk::Prepare(DWORDLONG * prepare_offset, volatile TestState * test_state, int sector_size, unsigned char* _random_data_buffer, long long _random_datat_buffer_size)
{
	BOOL write_ok;
	int num_outstanding;
	DWORD bytes_written;
	OVERLAPPED olap[PREPARE_QDEPTH];
	BOOL busy[PREPARE_QDEPTH];
	BOOL retval;
	int i;
	void *buffer = NULL;
	DWORD bytes;
	long long random_offset_multiplier;

	// Allocate a large (64k for 512 byte sector size) buffer for the preparation.
	bytes = sector_size * 128;
#if defined(IOMTR_OSFAMILY_NETWARE)
	NXMemFree(buffer);
	errno = 0;
	if (!(buffer = NXMemAlloc(bytes, 1)))
#elif defined(IOMTR_OSFAMILY_UNIX)
	free(buffer);
	errno = 0;
	if (!(buffer = valloc(bytes)))
#elif defined(IOMTR_OSFAMILY_WINDOWS)
	VirtualFree(buffer, 0, MEM_RELEASE);
	if (!(buffer = VirtualAlloc(NULL, bytes, MEM_COMMIT, PAGE_READWRITE)))
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	{
		cout << "*** Could not allocate buffer to prepare disk." << endl;
		return FALSE;
	}

	switch (spec.DataPattern) {
		case DATA_PATTERN_REPEATING_BYTES:
			// Do nothing here...a new random byte will be chosen below for each IO
			break;
		case DATA_PATTERN_PSEUDO_RANDOM:
			for( DWORD x = 0; x < bytes; x++)
				((unsigned char*)buffer)[x] = (unsigned char)Rand(0xff);
			break;
		case DATA_PATTERN_FULL_RANDOM:
			long long rand_max = RAND_MAX;
			//calculate maximum sector-aligned offset using two random integers
			long long max_random_sector_aligned_number = (rand_max * rand_max * (long long)sector_size);

			//come up with a divisor to keep offset in the 16MB region
			random_offset_multiplier = max_random_sector_aligned_number / _random_datat_buffer_size;

			//increment if remainder
			if(max_random_sector_aligned_number % _random_datat_buffer_size)
				random_offset_multiplier += 1;
			//random data used for writes
/*
#if defined(IOMTR_OSFAMILY_NETWARE)
		randomDataBuffer = NXMemAlloc(RANDOM_BUFFER_SIZE, 1);

#elif defined(IOMTR_OSFAMILY_UNIX)
#if defined(IOMTR_OS_LINUX)
		posix_memalign((void **)&randomDataBuffer, sysconf(_SC_PAGESIZE), RANDOM_BUFFER_SIZE);

#elif defined(IOMTR_OS_SOLARIS) || defined(IOMTR_OS_OSX)
		randomDataBuffer = valloc(RANDOM_BUFFER_SIZE);

#else
#warning ===> WARNING: You have to do some coding here to get the port done! 
#endif

#elif defined(IOMTR_OSFAMILY_WINDOWS)
		randomDataBuffer = (unsigned char*)VirtualAlloc(NULL, RANDOM_BUFFER_SIZE, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);

#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

			srand(time(NULL));

			if (randomDataBuffer)
			{
				for( int x = 0; x < RANDOM_BUFFER_SIZE; x++)
					randomDataBuffer[x] = (unsigned char)rand();

				cout << "   Done generating random data." << endl;
			}
			else
			{
				// Could not allocate a larger buffer.  Signal failure.
				cout << "   Error allocating random data buffer..." << endl;
			}
*/
			break;
	}

#ifdef _DEBUG
	cout << "into function TargetDisk::Prepare()" << endl;
#endif

	write_ok = TRUE;
	num_outstanding = 0;
	retval = TRUE;

	// Initialize the OVERLAPPED structures.
	for (i = 0; i < PREPARE_QDEPTH; i++) {
		// Create an event.
		olap[i].hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
		SetQueueSize(olap[i].hEvent, 1);
#elif defined(IOMTR_OSFAMILY_WINDOWS)
		// nop
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

		// Was the event created successfully?
		if (olap[i].hEvent == NULL) {
			cerr << "*** CreateEvent() failed in TargetDisk::Prepare()" << endl;
			write_ok = FALSE;	// don't perform any I/O
			retval = FALSE;	// return failure status
			break;	// break out of initialization loop
		}
		// Set the low-order bit of the event handle to prevent it from being
		// picked up by a later call to GetQueuedCompletionStatus() (e.g. in
		// Grunt::Asynchronous_IOs).
		// The documentation for GetQueuedCompletionStatus() says:
		//
		//     "Even if you have passed the function a file handle associated with 
		//     a completion port and a valid OVERLAPPED structure, an application  
		//     can prevent completion port notification. This is done by 
		//     specifying a valid event handle for the hEvent member of the
		//     OVERLAPPED structure, and setting its low-order bit. A valid event
		//     handle whose low-order bit is set keeps I/O completion from being 
		//     queued to the completion port. 
		//
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
		olap[i].hEvent = (HANDLE) ((UINT_PTR) olap[i].hEvent | 0x0000000000000001);
#else
		olap[i].hEvent = (HANDLE) ((unsigned long)olap[i].hEvent | 0x00000001);
#endif

		// Mark the slot's initial state as idle.
		busy[i] = FALSE;
	}

	do {
		// If we're still writing and there are any available slots, queue up some
		// writes.
		if ((*test_state == TestPreparing) && write_ok && (num_outstanding < PREPARE_QDEPTH)) {
			// Loop through the I/O queue looking for idle slots.
			for (i = 0; i < PREPARE_QDEPTH; i++) {
				// Check to see if we've reached the end of the disk
				if (spec.disk_info.maximum_size &&
				    ((*prepare_offset + bytes) >
				     ((spec.disk_info.starting_sector +
				       (DWORDLONG) spec.disk_info.maximum_size) * spec.disk_info.sector_size))) {
					// A maximum disk size was specified by the user, and the next write 
					// would go past the specified maximum size.  
#ifdef _DEBUG
					cout << "User-specified maximum size reached!" << endl;
#endif
					// Stop writing and break out of the write loop.
					write_ok = FALSE;
					break;
				}
				// If we are still writing and the slot is not busy, start an I/O for
				// this slot.
				if ((*test_state == TestPreparing) && write_ok && !busy[i]) {
					// Set its address.
					olap[i].Offset = (DWORD) * prepare_offset;
					olap[i].OffsetHigh = (DWORD) (*prepare_offset >> 32);

					// Fill the buffer with some new random data so we aren't writing all zeros each time
					switch (spec.DataPattern) {
						case DATA_PATTERN_REPEATING_BYTES:
							memset(buffer, rand(), bytes);
							break;
						case DATA_PATTERN_PSEUDO_RANDOM:
							break; // Nothing to do here, buffer was set above
						case DATA_PATTERN_FULL_RANDOM:
							long long offset = ((long long)rand() * (long long)rand() / random_offset_multiplier) * (long long)sector_size ;
							buffer = &_random_data_buffer[offset];
							break; // Nothing to do here, buffer was set above
					}

					// Do the asynchronous write.
					if (WriteFile(disk_file, (char *)buffer, bytes, &bytes_written, &(olap[i]))) {
						// It succeeded immediately!
#ifdef _DEBUG
						cout << "Wrote (immediately) " << bytes_written << " of "
						    << bytes << " bytes to disk " << spec.name << "." << endl;
#endif
						// Advance the file pointer, but do not mark the slot as busy.
						*prepare_offset += bytes;
					} else {
						// It did not succeed immediately... did it start OK?
						if (GetLastError() == ERROR_IO_PENDING) {
							// It started OK.
#if _DETAILS
							cout << "I/O started successfully for slot #" << i
							    << " for " << bytes << " bytes at address "
							    << *prepare_offset << endl;
#endif
							// Advance the file pointer and mark the slot as busy.
							*prepare_offset += bytes;
							busy[i] = TRUE;
							num_outstanding++;
						} else if (GetLastError() == ERROR_DISK_FULL) {
							// The disk filled up -- this is an expected error.
#ifdef _DEBUG
							cout << "Disk full (immediately) while writing "
							    << bytes_written << " of " << bytes << " bytes to disk "
							    << spec.name << "." << endl;
#endif
							// Stop writing and break out of the write loop.
							write_ok = FALSE;
							break;
						} else {
							// It didn't start OK!  Bail out.
							cout << "***Error (immediately) writing " << bytes_written <<
							    " of " << bytes << " bytes to disk " << spec.name <<
							    ", error=" << GetLastError() << endl;
							write_ok = FALSE;	// don't perform any more I/O
							retval = FALSE;	// return failure status
							break;	// break out of write loop
						}
					}
				}
			}
		}		// Done queueing up new writes.

		// If there are any outstanding I/Os, check to see if any have completed.
		if (num_outstanding > 0) {
			// Check all the busy I/O's to see if any have completed
			for (i = 0; i < PREPARE_QDEPTH; i++) {
				if (busy[i]) {
					// Check to see if it has completed
					if (GetOverlappedResult(disk_file, &(olap[i]), &bytes_written, FALSE)) {
						// It completed successfully!
#ifdef _DEBUG
						cout << "Wrote (eventually) " << bytes_written << " of " << bytes
						    << " bytes to disk " << spec.name << "." << endl;
#endif
						// Mark the slot as idle.
						busy[i] = FALSE;
						num_outstanding--;

						if (bytes_written != bytes) {
							cout << "***Error (eventually); wrote only " << bytes_written
							    << " of " << bytes << " bytes!" << endl;
							retval = FALSE;
							write_ok = FALSE;
							break;
						}
					} else if (GetLastError() == ERROR_IO_INCOMPLETE) {
						// The I/O has not yet completed -- this is an expected error.
#if _DETAILS
						cout << "I/O not yet complete for slot #" << i << endl;
#endif
						;	// Do nothing.
					} else if (GetLastError() == ERROR_DISK_FULL) {
						// The disk filled up -- this is an expected error.
#ifdef _DEBUG
						cout << "Disk full (eventually) while writing " << bytes_written <<
						    " of " << bytes << " bytes to disk " << spec.name << "." << endl;
#endif
						// Since the write did not actually succeed, decrement the file pointer.
						*prepare_offset -= bytes;
						// Stop writing and mark the slot as idle.
						write_ok = FALSE;
						busy[i] = FALSE;
						num_outstanding--;
					} else {
						// The I/O failed!  Bail out.
						cout << "***Error (eventually) writing " << bytes_written << " of " <<
						    bytes << " bytes to disk " << spec.
						    name << ", error=" << GetLastError() << endl;
						// Stop writing and mark the slot as idle.
						write_ok = FALSE;
						busy[i] = FALSE;
						num_outstanding--;
						// Return failure status once all I/Os have completed.
						retval = FALSE;
					}
				}
			}
		}		// Done checking for I/O completions.
	}
	while ((*test_state == TestPreparing) && (write_ok || (num_outstanding > 0)));
	// Keep looping until (the user tells us to stop) OR 
	// (we're done writing AND we've accumulated all the results).

	Set_Sizes(FALSE);

	// Destroy the events.
	for (i = 0; i < PREPARE_QDEPTH; i++) {
#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
		// Reset the handles.
		olap[i].hEvent = (HANDLE) ((ULONG_PTR) olap[i].hEvent ^ 0x1);
#endif
		CloseHandle(olap[i].hEvent);
	}

#ifdef _DEBUG
	cout << "out of member function TargetDisk::Prepare()" << endl;
#endif

	return retval;
}

//
// Opening a disk for low-level access.
//
BOOL TargetDisk::Open(volatile TestState * test_state, int open_flag)
{
	// open_flag is a 
	if (IsType(spec.type, LogicalDiskType)) {
#if defined(IOMTR_OS_SOLARIS) || defined(IOMTR_OS_LINUX)
		((struct File *)disk_file)->fd =
		    open(file_name, O_RDWR | O_CREAT | O_LARGEFILE | open_flag, S_IRUSR | S_IWUSR);
#elif defined(IOMTR_OS_NETWARE)
		NXFileOpen(0, (void *)file_name, (NXMode_t) (NX_O_RDWR | NX_O_CREAT | open_flag),
			   &((struct File *)disk_file)->fd);
		((struct File *)disk_file)->type = LogicalDiskType;
#elif defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
		// Ignore errors that occur if trying to open a floppy or CD-ROM with
		// nothing in the drive.
		SetErrorMode(SEM_FAILCRITICALERRORS);
		disk_file = CreateFile(file_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ |
				       FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
				       FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL);
		SetErrorMode(0);
#elif defined(IOMTR_OS_OSX)
		((struct File *)disk_file)->fd = open(file_name, O_RDWR | O_CREAT | open_flag, S_IRUSR | S_IWUSR);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	} else if (IsType(spec.type, PhysicalDiskType)) {
#if defined(IOMTR_OS_SOLARIS) || defined(IOMTR_OS_LINUX)
		((struct File *)disk_file)->fd = open(file_name, O_RDWR | O_LARGEFILE, S_IRUSR | S_IWUSR);
#elif defined(IOMTR_OS_NETWARE)
		((struct File *)disk_file)->fd = NWOpenDevice(atoi(file_name), 0);
#elif defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
		SetErrorMode(SEM_FAILCRITICALERRORS);
		disk_file = CreateFile(file_name, GENERIC_READ | GENERIC_WRITE,
				       FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
				       OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL);
		SetErrorMode(0);
#elif defined(IOMTR_OS_OSX)
		((struct File *)disk_file)->fd = open(file_name, O_RDWR, S_IRUSR | S_IWUSR);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	} else {
		cout << "*** Unexpected drive type in TargetDisk::Open()" << endl;
	}

	offset = starting_position;
#if _DISK_MSGS
	cout << "Opening disk " << spec.name << endl;
#endif
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	if (disk_file == INVALID_HANDLE_VALUE)
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
	if (((struct File *)disk_file)->fd == (int)INVALID_SOCKET)
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	return FALSE;

	// Cludgy hack used to control reporting drives.  This will go away when
	// the TargetDisk class is divided into a disk Target subclass and a
	// Disk interface class.
	if (!test_state)
		return TRUE;

	// Indicating where completed asynchronous transfers should be queued.
	return (CreateIoCompletionPort(disk_file, io_cq->completion_queue, 0, 1)
		!= NULL);
}

//
// Closing the disk handle.
//
BOOL TargetDisk::Close(volatile TestState * test_state)
{
	// Note that test_state is not used.  It IS used by network targets.

	// If testing connection rate, the disk may already be closed.
#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
	if (((struct File *)disk_file)->fd == (int)INVALID_SOCKET)
#elif defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	if (disk_file == INVALID_HANDLE_VALUE)
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	{
#if _DISK_MSGS
		cout << "Disk is already closed." << endl;
#endif
		return TRUE;
	}

#if _DISK_MSGS
	cout << "Closing disk " << spec.name << endl;
#endif

#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
	if (!CloseHandle(disk_file, FILE_ELEMENT))
#elif defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	if (!CloseHandle(disk_file))
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	{
		cout << "*** Error " << GetLastError()
		    << " closing disk in TargetDisk::Close()." << endl;
		SetLastError(0);
		return FALSE;
	}

#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
	((struct File *)disk_file)->fd = (int)INVALID_SOCKET;
#elif defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	disk_file = INVALID_HANDLE_VALUE;
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	return TRUE;
}

//
// Seek to another position on the disk.  Whether this is a random seek or a sequential
// seek will be determined by the value of the first parameter.  This does not actually
// move the disk head, it just updates the offset of the next I/O request.
//
// Calling instructions:
//
// 1) If user enters an alignment which is a power of two, pass in the user_alignment
//              the user specified and the precalculated user_align_mask.
//              This uses fast byte alignment code.
// 2) If user enters an alignment which is NOT a power of two, pass in the user_alignment
//              the user specified and NOT_POWER_OF_TWO for user_align_mask.
//              This uses slower byte alignment code.
// 3) If user enters an alignment of 0 (zero), sector alignment is assumed.  Pass in
//              zeroes for both byte_alignment and user_align_mask.
//              This uses fast byte alignment code as long as the sector size is a power
//              of two, otherwise it reverts to slower code.
//
void TargetDisk::Seek(BOOL random, DWORD request_size, DWORD user_alignment, DWORDLONG user_align_mask)
{
	static DWORDLONG remainder;	// static for performance reasons

	// Find out if this is a random seek.
	if (random) {
		// Set the offset to a random location on the disk.
		offset = starting_position + Rand(size);
	} else {
		// Adjusting the offset pointer by the last number of bytes successfully transferred.
		offset += (DWORDLONG) bytes_transferred;
	}

	switch (user_align_mask) {
	case NOT_POWER_OF_TWO:
		// If not at a user_alignment boundary, move offset ahead to next one.
		if ((remainder = offset % user_alignment))
			offset += user_alignment - remainder;

		// See if we're near the end of the file and need to return to the start.
		if ((offset + (DWORDLONG) request_size) > ending_position) {
			offset = starting_position;

			// starting_position won't necessarily be byte-aligned.
			if ((remainder = offset % user_alignment)) {
				offset += user_alignment - remainder;

				// If we went beyond the end of the disk again, report an error.
#ifdef _DEBUG
				if ((offset + (DWORDLONG) request_size) > ending_position) {
					cout << "*** Can't align on " << user_alignment << " byte boundaries" << endl;

					offset = starting_position;
				}
#endif
			}
		}
		break;

	case 0:
		// Assume sector alignment.
		if (sector_align_mask == NOT_POWER_OF_TWO) {
			if ((remainder = offset % spec.disk_info.sector_size))
				offset += spec.disk_info.sector_size - remainder;
		} else {
			if (offset != (offset & sector_align_mask))
				offset = (offset + spec.disk_info.sector_size) & sector_align_mask;
		}

		// See if we're near the end of the file and need to return to the start.
		if ((offset + (DWORDLONG) request_size) > ending_position) {
			offset = starting_position;

			// starting_position won't necessarily be sector-aligned.
			if (sector_align_mask == NOT_POWER_OF_TWO) {
				if ((remainder = offset % spec.disk_info.sector_size)) {
					offset += spec.disk_info.sector_size - remainder;

					// If we went beyond the end of the disk again, report an error.
#ifdef _DEBUG
					if ((offset + (DWORDLONG) request_size) > ending_position) {
						cout << "*** Can't align on " << spec.disk_info.sector_size
						    << " byte boundaries" << endl;

						offset = starting_position;
					}
#endif
				}
			} else {
				if (offset != (offset & sector_align_mask)) {
					offset = (offset + spec.disk_info.sector_size) & sector_align_mask;

					// If we went beyond the end of the disk again, report an error.
#ifdef _DEBUG
					if ((offset + (DWORDLONG) request_size) > ending_position) {
						cout << "*** ERROR: Can't align on " << spec.disk_info.sector_size
						    << " byte boundaries" << endl;

						offset = starting_position;
					}
#endif
				}
			}
		}
		break;

	default:
		// If not at a user_alignment boundary, move offset ahead to next one.
		if (offset != (offset & user_align_mask))
			offset = (offset + user_alignment) & user_align_mask;

		// See if we're near the end of the file and need to return to the start.
		if ((offset + (DWORDLONG) request_size) > ending_position) {
			offset = starting_position;

			// starting_position won't necessarily be byte-aligned.
			if (offset != (offset & user_align_mask)) {
				offset = (offset + user_alignment) & sector_align_mask;

				// If we went beyond the end of the disk again, report an error.
#ifdef _DEBUG
				if ((offset + (DWORDLONG) request_size) > ending_position) {
					cout << "*** Can't align on " << alignment << " byte boundaries" << endl;

					offset = starting_position;
				}
#endif
			}
		}
	}
}

//
// Reading a user specified amount of data from the drive and returning it.
// This does not check the request size to be a multiple of the sector size 
// (except in debug).  For performance reasons, Iometer ensures that the 
// values are correct.
//
ReturnVal TargetDisk::Read(LPVOID buffer, Transaction * trans)
{
	DWORD error_no;

#ifdef _DEBUG
	// Checking for the access to be a multiple of the sector size.
	// Avoiding this check during actual testing for performance reasons.
	if (offset % spec.disk_info.sector_size) {
		cout << "Invalid offset.  Not aligned with disk sector size for : " << spec.name << endl;
		return (ReturnError);
	}
	if (trans->size % spec.disk_info.sector_size) {
		cout << "Invalid transfer size.  Not aligned with disk sector size for : " << spec.name << endl;
		return (ReturnError);
	}
#endif

#if _DETAILS
	cout << "Reading " << trans->size << " bytes from disk : "
	    << spec.name << endl << "   Accessing : " << offset << endl;
#endif

	// Determining location of read to disk.
	trans->asynchronous_io.Offset = (DWORD) offset;
	trans->asynchronous_io.OffsetHigh = (DWORD) (offset >> 32);

	// Reading information from the disk.
	if (ReadFile(disk_file, buffer, trans->size, &bytes_transferred, &trans->asynchronous_io)) {
		// Read succeeded immediately, but completion is pending.  It will
		// still go to the completion queue.
#if (defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)) && defined(IMMEDIATE_AIO_COMPLETION)
		// This code blocks reading the completion Q for the immediately completed I/Os.
		error_no = 0;
		// this is for the next sequential I/O.
		bytes_transferred = trans->size;
		return ReturnSuccess;
#else				// All other cases UNIX, NT, etc.
		error_no = ERROR_IO_PENDING;
#endif
	} else {
		error_no = GetLastError();
	}

	// See if read failed.
	if (error_no != ERROR_IO_PENDING) {
		// Record that no bytes were read.
		bytes_transferred = 0;

		// Return error.
		cout << "*** Error " << error_no << " reading " << trans->size
		    << " bytes from disk " << spec.name << "." << endl;
		return ReturnError;
	}
	// An asynchronous read was successfully initiated!

	// Record number of bytes *to be* transferred (ReadFile() set this to 0).
	// This value will be used to determine location of next sequential I/O.
	bytes_transferred = trans->size;

#if _DETAILS
	cout << "Queued read for " << trans->size << " bytes from disk " << spec.name << "." << endl;
#endif
	return ReturnPending;
}

//
// Writing a user defined buffer to the drive.  This does not check the request size 
// to be a multiple of the sector size (except in debug).  For performance reasons,
// Iometer ensures that the values are correct.
//
ReturnVal TargetDisk::Write(LPVOID buffer, Transaction * trans)
{
	DWORD error_no;

#ifdef _DEBUG
	// Verifying that the amount to be written is a multiple of the sector size.
	// Avoiding this check during actual testing for performance reasons.
	if (offset % spec.disk_info.sector_size) {
		cout << "Invalid offset.  Not aligned with disk sector size for : " << spec.name << endl;
		return (ReturnError);
	}
	if (trans->size % spec.disk_info.sector_size) {
		cout << "Invalid transfer size.  Not aligned with disk sector size for : " << spec.name << endl;
		return (ReturnError);
	}
#endif

#if _DETAILS
	cout << "Writing " << trans->size << " bytes to disk : " << spec.name
	    << endl << "   Accessing : " << offset << endl;
#endif

	// Determining location of write to disk.
	trans->asynchronous_io.Offset = (DWORD) offset;
	trans->asynchronous_io.OffsetHigh = (DWORD) (offset >> 32);

	// Writing information from the disk.
	if (WriteFile(disk_file, buffer, trans->size, &bytes_transferred, &trans->asynchronous_io)) {
		// Write succeeded immediately, but completion is pending.  It will
		// still go to the completion queue.
#if (defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)) && defined(IMMEDIATE_AIO_COMPLETION)
		// This code blocks reading the completion Q for the immediately completed I/Os.
		error_no = 0;
		// this is for the next sequential I/O.
		bytes_transferred = trans->size;
		return ReturnSuccess;
#else				// All other cases UNIX, NT, etc.
		error_no = ERROR_IO_PENDING;
#endif
	} else {
		error_no = GetLastError();
	}

	// See if write failed.
	if (error_no != ERROR_IO_PENDING) {
		// Record that no bytes were written.
		bytes_transferred = 0;

		// Return error code.
		cout << "*** Error " << error_no << " writing " << trans->size
		    << " bytes to disk " << spec.name << "." << endl;
		return ReturnError;
	}
	// An asynchronous write was successfully initiated!  

	// Record number of bytes *to be* transferred (WriteFile() set this to 0).
	// This value will be used to determine location of next sequential I/O.
	bytes_transferred = trans->size;

#if _DETAILS
	cout << "Queued write for " << trans->size << " bytes to disk " << spec.name << "." << endl;
#endif
	return ReturnPending;
}

#if defined(IOMTR_OSFAMILY_UNIX)
#if defined(IOMTR_OS_SOLARIS)
#if defined(IOMTR_CPU_I386) || defined(IOMTR_CPU_IA64)
// highly specific to Solaris on Intel
BOOL TargetDisk::Look_For_Partitions()
{
	char buffer[512];
	int bytes_read, i;
	struct mboot *mb;
	struct ipart *ip;
	struct File *fp;

	fp = (struct File *)disk_file;
	if ((bytes_read = read(fp->fd, buffer, SECTOR_SIZE)) < SECTOR_SIZE) {
		// cannot read from the disk. So we should try to use it.
		// simply return (TRUE) indicating that it has partitions.
		return (TRUE);
	}
	//
	// We have read 512 bytes of the first cylinder, first sector.
	// It contains the master boot record, the partition table and mboot signature (viz 0xAA55)
	//
	mb = (struct mboot *)buffer;
	if (mb->signature != MBB_MAGIC)
		// Hmmm... This drive appears to be freshly formatted. guess we can report it.!
		return (FALSE);
	else {
		// well, we did have a valid signature in the mboot. Look for valid partitions.
		for (i = 0; i < FD_NUMPART; i++) {
			ip = (struct ipart *)(buffer + BOOTSZ + (i * sizeof(struct ipart)));
			if (ip->numsect || ip->relsect)
				return (TRUE);
		}
	}
	return (FALSE);
}

DWORDLONG TargetDisk::Get_Partition_Size(char *part_name, int part)
{
	char disk_name[MAX_NAME];
	char buffer[512];
	int bytes_read, fd;
	struct ipart *ip;
	int length;

	length = strlen(part_name);
	part_name[length - 1] = '0';	// Converting cXtXdXpX to cXtXdXp0
	snprintf(disk_name, MAX_NAME, "%s/%s", RAW_DEVICE_DIR, part_name);
	fd = open(disk_name, O_RDONLY);
	if (fd < 0) {
		return (0);
	}

	if ((bytes_read = read(fd, buffer, SECTOR_SIZE)) < SECTOR_SIZE) {
		close(fd);
		return (0);
	}
	//
	// We have read 512 bytes of the first cylinder, first sector.
	// It contains the master boot record, the partition table and mboot signature (viz 0xAA55)
	//
	close(fd);
	ip = (struct ipart *)(buffer + BOOTSZ + ((part - 1) * sizeof(struct ipart)));
	return ((DWORDLONG) ip->numsect * SECTOR_SIZE);
}
#endif				// __i386 || _IA64

DWORDLONG TargetDisk::Get_Slice_Size(char *part_name, int part)
{
	char disk_name[MAX_NAME];

//      int bytes_read;
	int fd;
	struct vtoc this_vtoc;
	int length;

	length = strlen(part_name);
	part_name[length - 1] = '2';	// Converting cXtXdXsX to cXtXdXs2
	snprintf(disk_name, MAX_NAME, "%s/%s", RAW_DEVICE_DIR, part_name);
	fd = open(disk_name, O_RDONLY);
	if (fd < 0) {
		return (0);
	}

	if (ioctl(fd, DKIOCGVTOC, &this_vtoc) < 0) {
		close(fd);
		return (0);
	}
	//
	// We have the vtoc.
	// It contains the slice info (including size).
	//
	close(fd);
	return ((DWORDLONG) this_vtoc.v_part[part].p_size * this_vtoc.v_sectorsz);
}
#endif				// SOLARIS

#if defined(IOMTR_OS_LINUX)

static int getSectorSizeOfPhysDisk(const char *devName)
{
	char devNameBuf[MAX_NAME];
	const char *fullDevName;
	int fd, ssz;

	if (devName[0] == '/') {
		fullDevName = devName;
	} else {
		snprintf(devNameBuf, MAX_NAME, "%s/%s", RAW_DEVICE_DIR, devName);
		fullDevName = devNameBuf;
	}
	if ((fd = open(fullDevName, O_RDWR)) < 0) {
		cerr << "Fail to open device" << endl;
		return 0;
	}

	// still leave old comments here but we should use BLKSSZGET. Sector 
	// size request should be allowed no matter what.
	//
	// Use BLKBSZGET here as opposed to BLKSSZGET.  BLKSSZGET returns the
	// true sector size of the physical disk which should be what we want.
	// However, Linux will always use the potential cluster size of the 
	// file system even for raw devices with no file system.  So we use 
	// BLKBSZGET which returns the (potential) file system cluster size.

	if (ioctl(fd, BLKSSZGET, &ssz) < 0) {
		cerr << "Fail to get sector size for " << fullDevName << endl;
		close(fd);
		return 0;
	}
	close(fd);
	return ssz;
}

//////////////////////////////////////////////////////////////////////
//
// Parameters:
//   devName - The name of this device's node. Must be in the "/dev/<name>"
//             format.
// Return value:
//   The size (in bytes) of the named device.
//
static unsigned long long getSizeOfPhysDisk(const char *devName)
{
	char devNameBuf[MAX_NAME];
	const char *fullDevName;
	int fd;
	unsigned long long sz64;
	unsigned long sz32;

	if (devName[0] == '/') {
		fullDevName = devName;
	} else {
		snprintf(devNameBuf, MAX_NAME, "%s/%s", RAW_DEVICE_DIR, devName);
		fullDevName = devNameBuf;
	}
	if ((fd = open(fullDevName, O_RDWR)) < 0) {
		cerr << "Fail to open device" << endl;
		return 0;
	}
	if (ioctl(fd, BLKGETSIZE64, &sz64) < 0) {
		//cerr << "Fail to get size for " << fullDevName << " by BLKGETSIZE64" << endl;
		cerr << "Warning: Reading the device geometry for " << fullDevName << " failed using the ";
		cerr << "primary detection method - going to try backup method (this puts a 2TB limit ";
		cerr << "on Iometer for accessing the device)" << endl;
		if (ioctl(fd, BLKGETSIZE, &sz32) < 0) {
			//cerr << "Fail to get size for " << fullDevName << "by BLKGETSIZE" << endl;
			cerr << "ERROR: Reading the device geometry for " << fullDevName << " failed using ";
			cerr << "the backup detection method - give up (device will not be usable by Iometer)" << endl;
			sz64 = 0;
		} else {
			sz64 = sz32;
			sz64 <<= 9;
		}
	}
#ifdef _DEBUG
	cout << "Device " << fullDevName << " size:" << sz64 << "Bytes." << endl;
#endif

	close(fd);
	return sz64;
}

#endif				// Linux
#endif				// UNIX
