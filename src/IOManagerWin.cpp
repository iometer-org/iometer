/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / IOManagerWin.cpp                                          ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: OS specific Manager functions for Windows             ## */
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
/* ##  Changes ...: 2008-05-01 (vedrand@yahoo.com)                        ## */
/* ##               - Rewrote adding support for volume mount point       ## */
/* ##                 better raw disk and network share detection         ## */
/* ##               - Added new TargetType for chaining disks in Iometer  ## */
/* ##                 see comments in iotest.h                            ## */
/* ##               2004-04-15 (lamontcranston41@yahoo.com)               ## */
/* ##               - Moved Report_Disks() from IOManager.cpp to here.    ## */
/* ##               - Initial drop.                                       ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include "IOCommon.h"
#include "IOManager.h"
#include "IOTargetDisk.h"


#ifdef USE_NEW_DISCOVERY_MECHANISM
//
// DDK/WLK headers for ANY windows build, necessary for the fancy
// storage structure referenced.
//

#if 0	
// All this is now covered in the winioctl.h -- assuming you have a recent one
// Uncomment as necessary.
#include <devioctl.h>
#include <ntdddisk.h>
#include <ntddstor.h>
#include <ntddvol.h>
#endif

int Report_Disks_WinNT(Target_Spec * disk_spec)
#else
//
// Checking for all accessible drives.  Storing them, and returning the number
// of disks found.  Drives are reported in order so that Iometer does not
// need to sort them.
//
int Manager::Report_Disks(Target_Spec * disk_spec)
#endif
{
	DWORD dummy_word;
	TargetDisk d;
	char drive_letter;
	int drive_number = 0;
	int count = 0;

#ifdef USE_NEW_DISCOVERY_MECHANISM
	char buffer[_MAX_FNAME];

	cout << "Reporting drive information (WinNT compatibility mode)..." << endl;
#else
	cout << "Reporting drive information..." << endl;
#endif

	// Reporting logical drives first.
	for (drive_letter = 'A'; drive_letter <= 'Z'; drive_letter++) {
		// Checking to see if drive exists.
#ifdef USE_NEW_DISCOVERY_MECHANISM
		sprintf(buffer, "%c%s", drive_letter, LOGICAL_DISK);
		if ( ! d.Init_Logical( buffer ) )
#else
		if (!d.Init_Logical(drive_letter))
#endif
			continue;

		// Drive exists.  Getting its sector size, label, and volume name.
		memcpy(&disk_spec[count], &d.spec, sizeof(Target_Spec));
		strcat(disk_spec[count].name, "\\");
		if (!GetVolumeInformation(disk_spec[count].name, disk_spec[count].name + 2,
					  MAX_NAME - 1, NULL, &dummy_word, &dummy_word, NULL, 0)) {
			cout << "   Unable to retrieve volume information for " << d.spec.name << "." << endl;
			disk_spec[count].name[2] = '\0';
		}
#ifdef _DEBUG
		cout << "   Found " << disk_spec[count].name << "." << endl;
#endif
		count++;
	}

	// Reporting physical drives.
	while (count < MAX_TARGETS) {
		// See if the physical drive exists.
		snprintf(d.spec.name, MAX_NAME, "%s%i", PHYSICAL_DISK, drive_number);
		strcpy(d.file_name, d.spec.name);
		d.spec.type = PhysicalDiskType;

		// Try to open the drive, if it exists close it and initialize it.  If
		// it doesn't exist, no more physical drives will be found.  This is
		// temporary code until the TargetDisk class is split up.
		if (!d.Open(NULL))
			break;
		d.Close(NULL);

		// Drive exists, see if it is available for accessing.
#ifdef USE_NEW_DISCOVERY_MECHANISM
		if (!d.Init_Physical(d.spec.name)) {
#else
		if (!d.Init_Physical(drive_number)) {
#endif
			drive_number++;
			continue;
		}

		memcpy(&disk_spec[count], &d.spec, sizeof(Target_Spec));
		strcpy(disk_spec[count].name, PHYSICAL_DRIVE_PREFIX);
		_itoa(drive_number, disk_spec[count].name + strlen(disk_spec[count].name), 10);
		drive_number++;

#ifdef _DEBUG
		cout << "   Found " << disk_spec[count].name << "." << endl;
#endif
		count++;
	}
	cout << "   done." << endl << flush;
	return count;
}


#ifdef USE_NEW_DISCOVERY_MECHANISM

//
// Everything here deals with the new Windows disk discovery -- it should have no impact on other OSs.
// Some changes here as well as targetdisk and iotest allow other OSs to follow suit.
//

/////////////////////////////////////////////INCLUDES//////////////////////////////////////////////

#include <lm.h>	// for network share enum code	
// All other ntdd includes are provides via iotargetdisk.h/iotest.h

extern struct dynamo_param param;

/////////////////////////////////////////////DEFINES///////////////////////////////////////////////

// Enables the fanciest disk detection method and falls
// back to older methods on older OSes (like win2k/nt).
// FALSE will force the plain method always.
//
// Functionality moved to dynamo_param global member disk_control
// #define USE_NEW_DISK_VIEW	TRUE


// Inject dummy volume in disk wiew hierarchy 
// as parent objects to unmounted/unknown disks that 
// have valid partitions on them.
// FALSE will not, but iometer will control the display of 
// these additional physical disks.
#define NEST_ORPHAN_DISKS	FALSE

// Number of non-existant disks to scan before stopping for good.
#define PHYS_DISK_HOLE_SIZE 512


#define MINUS_ONE 0xffffffff
#define LDSTRINGS_BUF_SIZE 256

#define GET_LAST_CHAR(_x) _x[strlen(_x) - 1]
#define HAS_LAST_SLASH(_x) (GET_LAST_CHAR(_x) == '\\')
#define STRIP_LAST_SLASH(_x) if (HAS_LAST_SLASH(_x)) { GET_LAST_CHAR(_x) = '\0'; }

#define GET_STORPROP_VENDOR_STRINGS(_x) ((char *)_x + ((PSTORAGE_DEVICE_DESCRIPTOR)_x)->VendorIdOffset)
#define GET_STORPROP_PRODUCT_STRINGS(_x) ((char *)_x + ((PSTORAGE_DEVICE_DESCRIPTOR)_x)->ProductIdOffset)
#define GET_STORPROP_REVISION_STRINGS(_x) ((char *)_x + ((PSTORAGE_DEVICE_DESCRIPTOR)_x)->ProductRevisionOffset)

//
// Disk_Spec.disk_reserved is a 5-DWORD long structure that contains a STORAGE_DEVICE_NUMBER structure 
// and a pointer tothe VOLUME_DISK_EXTENTS_STRUCTURE; This could be bad if the SDN stucture changes size;
#define GET_SDN_PTR(_x) ((PSTORAGE_DEVICE_NUMBER) &_x.disk_reserved[0])
#define GET_VDE_PTR(_x) ((PVOLUME_DISK_EXTENTS *) &_x.disk_reserved[3])

#if 0
// The above GET_VDE or GET_SDN should be been phrades like this
#pragma pack(push, 1)
struct _SDN_VDEPTR
{
	STORAGE_DEVICE_NUMBER sdn;
	PVOLUME_DISK_EXTENTS pvde;
};
#pragma pack(pop)

#define GET_SDN_PTR(_x) (&((struct _SDN_VDEPTR)_x.disk_reserved).sdn)
#define GET_VDE_PTRPTR(_x) (&((struct _SDN_VDEPTR)_x.disk_reserved).pvde)
#endif

#define EQUAL_DEVICE_NUMBERS(_x, _y) ((((PSTORAGE_DEVICE_NUMBER)_x)->DeviceType == ((PSTORAGE_DEVICE_NUMBER)_y)->DeviceType) \
							       && (((PSTORAGE_DEVICE_NUMBER)_x)->DeviceNumber == ((PSTORAGE_DEVICE_NUMBER)_y)->DeviceNumber))



/////////////////////////////////////// GLOBAL FUNCTION PROTOTYPES//////////////////////////////

//
// Define function pointer prototypes. The following exist on Win2k and above,
// so this is basically here for nt4 support and lower. If these don't exist,
// we run with the orignal algorithm.
//

#ifdef UNICODE
#define FIND_FIRST_VOLUME_MOUNTPOINT_ENTRY			L"FindFirstVolumeMountPointW"
#define FIND_NEXT_VOLUME_MOUNTPOINT_ENTRY			L"FindNextVolumeMountPointW"
#define FIND_FIRST_VOLUME_ENTRY						L"FindFirstVolumeW"
#define FIND_NEXT_VOLUME_ENTRY						L"FindNextVolumeW"
#define GET_VOLUME_PATHNAMES_FOR_VOLUMENAME_ENTRY	L"GetVolumePathNamesForVolumeNameW"

#else
#define FIND_FIRST_VOLUME_MOUNTPOINT_ENTRY			"FindFirstVolumeMountPointA"
#define FIND_NEXT_VOLUME_MOUNTPOINT_ENTRY			"FindNextVolumeMountPointA"
#define FIND_FIRST_VOLUME_ENTRY						"FindFirstVolumeA"
#define FIND_NEXT_VOLUME_ENTRY						"FindNextVolumeA"
#define GET_VOLUME_PATHNAMES_FOR_VOLUMENAME_ENTRY	"GetVolumePathNamesForVolumeNameA"

#endif // UNICODE

typedef HANDLE	(WINAPI *PFIND_FIRST_VOLUME)(LPTSTR lpszVolumeName, DWORD cchBufferLength);
typedef BOOL	(WINAPI *PFIND_NEXT_VOLUME)	(HANDLE hFindVolume, LPTSTR lpszVolumeName, 
											 DWORD cchBufferLength);
typedef BOOL	(WINAPI *PFIND_VOLUME_CLOSE)(HANDLE hFindVolume);
typedef HANDLE	(WINAPI *PFIND_FIRST_VOLUME_MOUNTPOINT)	(LPTSTR lpszRootPathName, 
														 LPTSTR lpszVolumeMountPoint, 
														 DWORD cchBufferLength);
typedef BOOL	(WINAPI *PFIND_NEXT_VOLUME_MOUNTPOINT)	(HANDLE hFindVolumeMountPoint, 
														 LPTSTR lpszVolumeMountPoint, 
														 DWORD cchBufferLength);
typedef BOOL	(WINAPI *PFIND_VOLUME_MOUNTPOINT_CLOSE)	(HANDLE hFindVolumeMountPoint);

PFIND_FIRST_VOLUME	pFindFirstVolume = NULL;
PFIND_NEXT_VOLUME	pFindNextVolume = NULL;
PFIND_VOLUME_CLOSE	pFindVolumeClose = NULL;

PFIND_FIRST_VOLUME_MOUNTPOINT	pFindFirstVolumeMountPoint = NULL;
PFIND_NEXT_VOLUME_MOUNTPOINT	pFindNextVolumeMountPoint = NULL;
PFIND_VOLUME_MOUNTPOINT_CLOSE	pFindVolumeMountPointClose = NULL;

//
// The following function pointer prototypes are for winxp/win2k3 
// and newer os routines needed for win2k support. If these don't
// exist we resort to a less fancy method. These methods are controlled
// by the USE_NEW_DISK_VIEW macro -- see definition above.
//
typedef BOOL (WINAPI *PGET_VOLUME_PATHNAMES_FOR_VOLUMENAME)(LPCTSTR lpszVolumeName,
															LPTSTR lpszVolumePathNames,
															DWORD cchBufferLength,
															PDWORD lpcchReturnLength);

PGET_VOLUME_PATHNAMES_FOR_VOLUMENAME pGetVolumePathNamesForVolumeName = NULL;

//////////////////////////////////////////LOCAL VARIABLES//////////////////////////////////////

// Used for scanning the DosDevices name spacs
struct DosDeviceEntry
{
	char link[_MAX_FNAME];
	char target[_MAX_FNAME];
};

struct DosDeviceList
{
	int num_entries;
	DosDeviceEntry *list; // caller must deallocates, see notes below
};

/////////////////////////////////////// LOCAL FUNCTION PROTOTYPES//////////////////////////////
int Report_Volumes1(Target_Spec *disk_spec, int start_count);
int Report_Volumes2(Target_Spec *disk_spec, int start_count, bool force_all);
int Report_NetDisks(Target_Spec *disk_spec, int start_count, bool show_unc_path);
int Report_RawDisks(Target_Spec *disk_spec, int start_count, int view_type);
void Volume_GetInfo(Target_Spec *disk_spec, bool bCompatible);

bool GetDeviceNumber(char *name, PSTORAGE_DEVICE_NUMBER sdn);
bool GetDiskExtents(char *name, PVOID vde, PULONG size, bool allocate);
bool GetStorageProperty(char *name, PVOID in, PULONG size, bool allocate);
bool SendIoControl( char *device, DWORD control, LPVOID in,  DWORD size_in, LPVOID out, DWORD size_out);
int  MergeVolumesAndRawDisks(Target_Spec *dest, Target_Spec *source, ULONG mid_point, ULONG total, BOOL InsertUnknown);
int __cdecl TS_Compare_BasicName(const void *elem1, const void *elem2);
int __cdecl TS_Compare_DeviceNumberLogiFirst(const void *elem1, const void *elem2);

//////////////////////////////////////////////FUNCTION DEFINITIONS///////////////////////////////////////

//
// Manager::Report_Disks
//
// The main routine of the module. It calls everthing else in here.
// The goal of the code is to gather volume info, raw disk info and 
// network share info and then associated this meaningully for 
// display in iometer. Allow iometer gui to decide what to view and 
// what not to.
//
int Manager::Report_Disks( Target_Spec* disk_spec )
{
	BOOL bUseNewDiskView; 
	int device_count, vol_count, raw_count, net_count;
	HMODULE hLibModule;

	//
	// Handle version dependent code and function imports
	//

	cout << "Windows NT version: " << m_OsVersionInfo.dwMajorVersion << "." 
		 << m_OsVersionInfo.dwMinorVersion << endl;

	//win2k or higher
	if (m_OsVersionInfo.dwMajorVersion >= 5)
	{

		//
		// Try to get a handle to kernel32 and our volume entry points
		//
		hLibModule = GetModuleHandle("kernel32.dll");

		if (hLibModule == NULL)
		{
			cerr << "GetModuleHandle to kernel32.dll failed with error " 
				 << GetLastError() << "." << endl;

			return Report_Disks_WinNT(disk_spec);
		}

		pFindFirstVolumeMountPoint = (PFIND_FIRST_VOLUME_MOUNTPOINT) 
			GetProcAddress(hLibModule, FIND_FIRST_VOLUME_MOUNTPOINT_ENTRY);
		pFindNextVolumeMountPoint = (PFIND_NEXT_VOLUME_MOUNTPOINT) 
			GetProcAddress(hLibModule, FIND_NEXT_VOLUME_MOUNTPOINT_ENTRY);
		pFindVolumeMountPointClose = (PFIND_VOLUME_MOUNTPOINT_CLOSE) 
			GetProcAddress(hLibModule, "FindVolumeMountPointClose");

		if (!pFindFirstVolumeMountPoint || !pFindNextVolumeMountPoint || !pFindVolumeMountPointClose)
		{
			cerr << "Could not retrieve volume mount point entry points (>= v5.0), error="
				 << GetLastError() << "." <<  endl;

			// fall-back to original routine
			return Report_Disks_WinNT(disk_spec);
		}

		bUseNewDiskView = FALSE; // for win2k, always use the less fancy view
		
		// winxp, win2k3 or higher
		if ( (m_OsVersionInfo.dwMajorVersion == 5 && m_OsVersionInfo.dwMinorVersion >= 1) ||
			 (m_OsVersionInfo.dwMajorVersion > 5))
		{
			//
			// The next 3 are also available in win2k, but since the functionality
			// relies on the 4th which is xp/2k3 only, all of these are really
			// one bundle.
			//
			pFindFirstVolume = (PFIND_FIRST_VOLUME) GetProcAddress(hLibModule, FIND_FIRST_VOLUME_ENTRY);
			pFindNextVolume = (PFIND_NEXT_VOLUME) GetProcAddress(hLibModule, FIND_NEXT_VOLUME_ENTRY);
			pFindVolumeClose = (PFIND_VOLUME_CLOSE) GetProcAddress(hLibModule, "FindVolumeClose");
			
			// Available in xp/win2k3 or above only!!!
			pGetVolumePathNamesForVolumeName = 	(PGET_VOLUME_PATHNAMES_FOR_VOLUMENAME) 
					GetProcAddress(hLibModule, GET_VOLUME_PATHNAMES_FOR_VOLUMENAME_ENTRY);

			if (!pFindFirstVolume || !pGetVolumePathNamesForVolumeName 
								  || !pFindNextVolume || !pFindVolumeClose)
			{
				cerr << "Could not retrieve volume mount point entry points (>= v5.1), error="
					 << GetLastError() << "." <<  endl;

				bUseNewDiskView = FALSE;
			}
			else
			{
				// for xp/win2k3 we can use either view as defined by the macro
				bUseNewDiskView = param.disk_control;
			}
		}
	}
	// OS version is nt4 or lower or unknown
	else
	{
		return Report_Disks_WinNT(disk_spec);
	}

	//
	// Now, do the new disk reporting
	// 

	if (bUseNewDiskView)
	{
		//
		// This mode can report physical disk to volume associations by first 
		// listing the volume and then followed the corresponding physical disk. The
		// physical disk will be tagged by a special identifier so that iometer can 
		// place it accordingly into the (sub)tree view.
		//
		// The dynamo command line parameter /force_raw will show all raw disks.
		//
		// Raw/physical disks without recognized or any volumes will follow. The net
		// disks will bring up the rear. 
		//
		// Idea is to present all volumes and have iometer choose which to display based
		// on disk_info property.
		// 

		// On input to a routines, serves as an array start index; on return from
		// the same routine represents the number of devices discovered
		device_count = 0;
		vol_count = raw_count = net_count = 0;

		// gather volume info first
		device_count = vol_count = Report_Volumes2(disk_spec, device_count, TRUE);

		// sort the volumes
		if (vol_count > 1)
			qsort( disk_spec, vol_count, sizeof(Target_Spec), TS_Compare_BasicName );

		// append all raw disks which will be in order due to sequential search
		device_count =  Report_RawDisks(disk_spec, device_count, param.disk_control); 

		raw_count = device_count - vol_count;
#if 0
		// This would sort all disk_specs based on target name
		qsort( disk_spec, device_count, sizeof(Target_Spec), TS_Compare_DeviceNumberLogiFirst);
#else

		// Instead, do something unreasonably more complicated... :)
		//
		// Allocate temporary storage in which we will store our newly sorted list of target_specs
		// including additional raw disks for hte optional allocating dummy volumes (see below).
		Target_Spec *new_disks = new Target_Spec[device_count + raw_count];

		//
		// Sort the whole thing based on storage number, tagging each item for tree view 
		// presentation for display in iometer. If the last parameter is TRUE, this routine
		// will allocate new volumes to be the logical parent of a physical disk that contains 
		// unknown partitions. 
		//
		// Plan as of now is to not use dummy volumes. In the future, iometer will present a checkbox 
		// for filtering physical disks (and other? disks). Thus, dynamo should return as many disks 
		// as it can find and tag them. Currently, the extra disks are controlled via the cmdline /force_raw.
		//						
		device_count = MergeVolumesAndRawDisks(new_disks, disk_spec, vol_count, device_count, NEST_ORPHAN_DISKS);

		// Copy the ordered temporary list back to disk_spec 
		memcpy(disk_spec, new_disks, device_count * sizeof(Target_Spec));

		// Delete the temp list
		delete [] new_disks;
#endif

		// Now just append the networked disks
		device_count = Report_NetDisks(disk_spec, device_count, TRUE);

		net_count = device_count - (raw_count + vol_count);

		// Sort only the net disks
		if (net_count > 1)
			qsort( &disk_spec[device_count - net_count], net_count, sizeof(Target_Spec), TS_Compare_BasicName);

#if _DETAILS
		cout << "Target_Spec size=" << sizeof(Target_Spec) << endl; 
		cout << "Data_Message size=" << sizeof(Data_Message) << endl;
		cout << "Number of devices detected: " << device_count << endl;
#endif
		// Done
		return device_count;
	}

	//
	// Compatibility view -- this is the less fancy version, intended for win2k or above
	//

#if _DETAILS
	cout << "Recollecting disk information in compatibility mode..." << endl;
#endif

	device_count = 0;

	// Display the view of disks by utilizing the newer apis (if available)
	// but do it in the original order...
	
	// First fixed disk volumes...
	device_count = Report_Volumes1(disk_spec, device_count);
	
	//
	// Then enum the network shares...
	//
	// TODO: Like above, it might be nice to optionally enum network shares and/or
	// fix certain parameters that don't makes sense to do to disks across 
	// lanman networks. That and also have a NetworkDiskType type indicating 
	// a smb or nfs type of filesystem.
	//
	device_count = Report_NetDisks(disk_spec, device_count, FALSE);

	// Because volumes are returned in non-alpha order, sort the logical drives 
	// obtained above...

	if (device_count > 1)
	{
		qsort( disk_spec, device_count, sizeof(Target_Spec), TS_Compare_BasicName );
	}

	// and finally append the rest of the unused physical/raw disks to match the orignal vies
	device_count = Report_RawDisks(disk_spec, device_count, RAWDISK_VIEW_COMPAT);
	
#if _DETAILS
	cout << "Number of devices reported: " << device_count << endl;
#endif

	return device_count;
}

//
// Report_Volumes1()
// 
// INPUTS:
//		disk_spec	same disk_spec passed to Report_Disks
//		start_count	the index into the disk_spec array where to deposit the new entry
//
// OUTPUT:
//		int			The next free entry/index into the disk_spec array 
//					(start_count + number of discovered devices)
//
// Smarter logical drive enumeration and volume mount point support (win2k and above)
//	
// Note: will produce duplicate instances IFF a volume is mounted multiple times.
// These volumes can be identified by a matching volume label (the string after
// the ":" e.g. "c:\mnt\disk1:backup" "g:backup") if one exists. 
// Either way, no harm done, I think.
//
// Note: can identify networked volumes in addition to removable disks.
//
int 
Report_Volumes1( Target_Spec* disk_spec, int start_count )
{
	DWORD buf_size = LDSTRINGS_BUF_SIZE, actual_size=0;
	DWORD drive_type;
	DWORD length;
	int	  count = start_count;
	char *buffer, *bufptr, *ptr;
	TargetDisk	d;

	cout << "Reporting volume information (win2k style)..." << endl;
	//
	// Get a string of all logical drives in the system
	//
	while(actual_size == 0 || actual_size > buf_size)
	{
		//buffer = (char *) malloc(buf_size);
		buffer = new char[buf_size];

		if (buffer == NULL)
		{
			cerr << "  Error allocating buffer, error=" << GetLastError() << "." << endl;
			return start_count;
		}

		actual_size = GetLogicalDriveStrings(
			buf_size,  // size of buffer
			buffer       // drive strings buffer
		);

		if (actual_size == 0)
		{
			cerr << "  Error: GetLogicalDriveStrings() failed, error=" 
				 << GetLastError() << "." << endl;
			return start_count;
		}
		else if (actual_size > buf_size)
		{
			//free(buffer);
			delete [] buffer;
			buf_size *=4;
		}

	}

	//
	// Parse buffer, the logical drives string and find all mounted volumes
	// on each given logical drive.
	//

	for (bufptr = buffer;
		 (bufptr < &buffer[actual_size]) && (count < MAX_TARGETS); 
		 bufptr = &bufptr[strlen(bufptr) + 1]
		)
	{
		HANDLE hMntpt;
		char   mntpt_buf[_MAX_FNAME];


		#if _DETAILS
			cout << "   Found volume: " << bufptr;
		#endif

		//
		// bufptr contains a single drive string
		//

		drive_type = GetDriveType( bufptr );

		// Always include the below, and nothing else
		if ( drive_type != DRIVE_FIXED && 
			 drive_type != DRIVE_REMOVABLE &&
			 //drive_type != DRIVE_CDROM
			 drive_type != DRIVE_RAMDISK
		) 
		{

			#if _DETAILS
				cout << ", cdrom or non-fixed disk, skipping..." << endl;
			#endif
			continue;
		}

		d.spec.type = LogicalDiskType;
		// Initialize the drive; it had better exist!
		if ( !d.Init_Logical( bufptr ) )
		{

			#if _DETAILS
				cerr << ", failed initializing, error=" << GetLastError() << "." <<endl;
			#endif

			continue;
		}

		// Drive exists.  Getting its sector size, label, and volume name.
		memcpy( &disk_spec[count], &d.spec, sizeof( Target_Spec ) );
		
		Volume_GetInfo(&disk_spec[count], TRUE);

		#if _DETAILS
			cout << "." << endl;
		#endif
	
		count++;


		//
		// Setup volume mount point pointers/iterators abd repeat above
		//

		strcpy(mntpt_buf, bufptr);
		length = strlen(bufptr);

		// ptr points into the mntptr_buf where the new mount point name will start
		ptr = &mntpt_buf[length];

		// adjust the buffer size -- should still be enough
		length = sizeof(mntpt_buf) - length;

		//
		// For current drive, enum all of its mount points
		//
		// Note: this means we could list duplicates if a volume is mounted more than
		//		 once AND/OR has a drive letter. Volume info will report the volume 
		//		 label, so that wil help. But, Report_Volumes2() addresses this 
		//		 drawback.
		//

		hMntpt = pFindFirstVolumeMountPoint(bufptr, ptr, length);

		if (hMntpt == INVALID_HANDLE_VALUE)
		{
			// No mount points found on this volume just continue
		
		#if _DETAILS
			DWORD error = GetLastError();

			if (error == ERROR_NO_MORE_FILES)
			{
				cout << "   No volume mount points exist." << endl;
			}
			else
			{
				cout << "   No volume mount points found, error=" << error << "." << endl;
			}

		#endif
		
			continue;
		}

		// Loop for the rest of the mounted volumes for this drive
		do 
		{
			#if _DETAILS 
				cout << "   Found mountpoint: " << mntpt_buf;
			#endif	

			drive_type = GetDriveType( mntpt_buf );


			// Always include the below, and nothing else
			if ( drive_type != DRIVE_FIXED && 
				 drive_type != DRIVE_REMOVABLE &&
				 //drive_type != DRIVE_CDROM
				 drive_type != DRIVE_RAMDISK
			) 
			{

				#if _DETAILS
					cout << ", cdrom or non-fixed disk, skipping..." << endl;
				#endif
				continue;
			}

			d.spec.type = LogicalDiskType;
			// Initialize the drive; it had better exist!
			if ( !d.Init_Logical( mntpt_buf ) )
			{

				#if _DETAILS
					cerr << ", failed initializing, error=" << GetLastError() << "." <<endl;
				#endif

				continue;
			}

			// Drive exists.  Getting its sector size, label, and volume name.
			memcpy( &disk_spec[count], &d.spec, sizeof( Target_Spec ) );
			
			Volume_GetInfo(&disk_spec[count], TRUE);
				
			#if _DETAILS
				cout << "." << endl;
			#endif

			count ++;

		} while( pFindNextVolumeMountPoint( hMntpt, ptr, length) );

	} // for

	//free(buffer);
	delete [] buffer;

	cout << "   done." << endl << flush;

	return count;
}

//
// Report_Volumes2()
// 
// INPUTS:
//		disk_spec	same disk_spec passed to Report_Disks
//		start_count	the index into the disk_spec array where to deposit the new entry
//		force_all	force display of all volumes cdroms and others
//
// OUTPUT:
//		int			The next free entry/index into the disk_spec array 
//					(start_count + number of discovered devices)
//
// Note: This is a Winxp/Win2k3 and later ONLY implementation. It eliminates any
// volume duplication. 
//
// Note: This routine is not inherently able to identify network volumes, but still can
// enumerate removables and unmounted volumes.
//
int
Report_Volumes2( Target_Spec* disk_spec, int start_count, bool force_all )
{
	HANDLE  h_volumes;
	TargetDisk	d;
	int		drive_number = 0;
	int		count = start_count;
	char  volume_name[MAX_NAME];
	char  path_names[MAX_NAME];

	DWORD length;
	DWORD drive_type;

	cout << "Reporting volume information..." << endl;
	
	//
	// Find{First,Next}Volume routines will report all local volumes, fixed and removable
	// in GUID types.
	//

	// Prime the iterator
	h_volumes = pFindFirstVolume(
				  volume_name,   // output buffer
				  MAX_NAME    // size of output buffer
				);
	
	if ( h_volumes == INVALID_HANDLE_VALUE )
	{
		cerr << "ERROR: FindFirstVolume() failed with error " << GetLastError() << "." << endl;
		return count;	// should come up with a better way of indicating and error to the caller
	}

	do
	{

		#if _DETAILS
			cout << "\nFound volume: " << volume_name << endl;
		#endif

		//
		// Retrieve friendly volume name for above guid.
		//
		if ( !pGetVolumePathNamesForVolumeName(
				  volume_name,
				  path_names,
				  MAX_NAME,
				  &length
				))
		{
			#if _DETAILS
				cerr << "Failed getting path names, error=" << GetLastError() << endl;
			#endif
			continue;
		}

		//
		// path_names will actually contain a multi_sz and so more than one pathname 
		// if present (for multiply mounted volumes). The first string should always 
		// be the drive letter, but we don't really care which one we display, 
		// so just use the first one in the list
		//	

		if (path_names[0] == '\0')
		{
			// No name found, so volume is not mounted
			if (force_all)
			{
				// If forcing show all volumes, make sure to strip the 
				// preceding "\\.\" or "\\?\" out of it and show the volume guid.
				strcpy(path_names, volume_name);
				d.spec.disk_info.not_mounted = TRUE;
			}
			else
			{
				#if _DETAILS
					cout << "Volume is not mounted, skipping..." << endl;
				#endif

				continue;
			}
		}
		else
		{
			#if _DETAILS
			char *name = path_names;

			cout << "Mount point(s): ";
			while(name <= &path_names[length])
			{
					cout << name << "  " ;
					name = &name[strlen(name) + 1];
			}

			cout << endl;
		
			#endif
		}
		//
		// TODO: We should report removable drives as a separate disk type. Since we know 
		// what they are, it could be a device property that iometer chooses to filter 
		// on demand. (Similar for network drives.)

		drive_type = GetDriveType( volume_name );

		// Always include fixed, removable and ramdisks
		if ( drive_type != DRIVE_FIXED && 
			 drive_type != DRIVE_REMOVABLE &&
			 //drive_type != DRIVE_CDROM
			 drive_type != DRIVE_RAMDISK
		)
		{
			// If force all, include everything else -- cdroms, etc...
			if (!force_all)
			{
				#if _DETAILS
					cout << "Volume is cdrom or non-fixed disk, skipping..." << endl;
				#endif

				continue;
			}
		}
	#if _DETAILS
		//else
			//cout << endl;
	#endif

		d.spec.type = LogicalDiskType;
		//
		// Initialize the drive; it had better exist!
		// but initialize the volume GUID, which is the "safer" thing to use for all
		// subsequent requests.
		//
		if ( !d.Init_Logical( volume_name ) )
		{
			#if _DETAILS
				cerr << "Init_Logical() failed with error=" << GetLastError() << endl;
			#endif
			continue;
		}

		PVOLUME_DISK_EXTENTS *pvde = (PVOLUME_DISK_EXTENTS*) GET_VDE_PTR(d.spec.disk_info);
		PSTORAGE_DEVICE_NUMBER psdn = GET_SDN_PTR(d.spec.disk_info);

		// This allows use to get a unique device number to match against physical disks.
		if (!GetDeviceNumber(volume_name, psdn))
		{
			// if we failed getting a device number, we might have a dynamic sw volume set,
			// so try to get disk extent info as well
			if (!GetDiskExtents(volume_name, pvde, 0, TRUE)) 
			{
				cerr << "ERROR retrieving device number or extents from " 
					 << path_names << endl;
				
				// Set the device_number to all_ones and ignore extents
				memset(GET_SDN_PTR(d.spec.disk_info), DWORD(-1), sizeof(STORAGE_DEVICE_NUMBER));
			}
			else
			{
				//PVOLUME_DISK_EXTENTS pvde = (PVOLUME_DISK_EXTENTS) GET_VDE_PTR(d.spec.disk_info);
				//PSTORAGE_DEVICE_NUMBER psdn = GET_SDN_PTR(d.spec.disk_info);

				// If we have extents, populate the device_number structure
				// with something useful for associating phys to logi disks
				psdn->DeviceType = FILE_DEVICE_DISK;

				// the first disk in the volume set will match this value
				psdn->DeviceNumber = (*pvde)->Extents[0].DiskNumber;

				// stash the number of physical disks in this sw volume as a negative partition number
				psdn->PartitionNumber = (ULONG) (-1 * (*pvde)->NumberOfDiskExtents);
			}
		}

#if _DETAILS2
		cout << "d.spec.disk_info.disk_reserved array: ";
		for (int i=0; i<5; i++)
			cout << i << "=" << d.spec.disk_info.disk_reserved[i];
		cout << endl;
#endif

		// once init_logical is called, we can overwrite the .name with our "friendly name" version
		strcpy(d.spec.name, path_names);

		// Drive exists.  Getting its sector size, label, and volume name.
		memcpy( &disk_spec[count], &d.spec, sizeof( Target_Spec ) );
		
		// Volume_GetInfo will append the volume label
		Volume_GetInfo(&disk_spec[count], FALSE);

		// Filter out the ugly prefix (only for the unmounted volume case)
		// only after we have gotten all pertinent info.
		if (!strncmp(disk_spec[count].name, "\\\\.\\", strlen("\\\\.\\")) ||
			!strncmp(disk_spec[count].name, "\\\\?\\", strlen("\\\\?\\")) )
		{	
			strcpy(path_names, disk_spec[count].name);
			strcpy(disk_spec[count].name, &path_names[strlen("\\\\.\\")]);
		}

		//if (force_all && (drive_type != DRIVE_FIXED))
		if (drive_type != DRIVE_FIXED)
		{
			// display non-fixed disk type
			if (drive_type == DRIVE_REMOVABLE)
				strcat (disk_spec[count].name, " (RMV");
			else if (drive_type == DRIVE_CDROM)
				strcat (disk_spec[count].name, " (ROM");
			else if (drive_type == DRIVE_RAMDISK)
				strcat (disk_spec[count].name, " (RAM");
			else 
				strcat (disk_spec[count].name, " (UNK");

			// display read_only flag
			if (disk_spec[count].read_only)
			{
				strcat (disk_spec[count].name, ", RO)");
				
				//cout << "   Volume " << disk_spec[count].name << " (" 
				//	 << disk_spec[count].actual_name << ") is read-only." << endl;
			}
			else
				strcat (disk_spec[count].name, ")");
		}

	#if _DETAILS
		cout << "GUI volume descriptor: " << disk_spec[count].name << endl;
	#endif

		count++;

	} while (pFindNextVolume(h_volumes, volume_name, _MAX_FNAME) 
			 && (count < MAX_TARGETS) );

	pFindVolumeClose(h_volumes); // close the handle to the volume iterator
	cout << "Found: " << count << endl;
	return count; // this is the next id to write to!
}


//
// Report_NetDisks()
// 
// INPUTS:
//		disk_spec	same disk_spec passed to Report_Disks
//		start_count	the index into the disk_spec array where to deposit the new entry
//		show_unc_path
//					toggles the label to be remote or the UNC network path 
//
// OUTPUT:
//		int			The next free entry/index into the disk_spec array 
//					(start_count + number of discovered devices)
//
// This routines relies on the lanman interface to enumerate networked volumes. The name
// provided as the volume label is the UNC path of the network share.
//
//
int Report_NetDisks( Target_Spec *disk_spec, int start_count, bool show_unc_path )
{
	DWORD EntriesRead = 0, TotalEntries = 0;
	int count = start_count;
	DWORD str_length;	
	NET_API_STATUS NetStatus;
	USE_INFO_0 *NetUseInfo;
	TargetDisk d;


	cout << "Reporting network share information..." << endl;

	NetStatus = NetUseEnum (
				  NULL,  
				  0,         // USE_INFO_0
				  (LPBYTE *) &NetUseInfo,
				  MAX_PREFERRED_LENGTH,
				  &EntriesRead,
				  &TotalEntries,
				  NULL
				);	
	
	if ( NetStatus == NERR_Success )
	{
		for (DWORD i=0; (i < EntriesRead) && (count < MAX_TARGETS) ; i++ )
		{
			CString NameInfo(NetUseInfo[i].ui0_local);
			NameInfo.OemToAnsi();

			NameInfo = NameInfo + "\\";

			#if _DETAILS
				cout << "\nFound volume: " << NameInfo.GetBuffer(0) << endl;
			#endif

			d.spec.type = LogicalDiskType;
			// we don't have a volume guid for network paths
			if ( !d.Init_Logical( NameInfo.GetBuffer(0)) )
			{
			#if _DETAILS
				DWORD error = GetLastError();

				//
				// If we failed here, it is most likely because the volume is read-only
				//
				if (error == ERROR_ACCESS_DENIED)
				{
					cout << "Volume is probably read-only, skipping..." << endl;
				}
				else
				{
					cerr << "Init_Logical() failed, error=" << GetLastError() << endl;
				}
			#endif	

				continue;
			}
			
			memcpy( &disk_spec[count], &d.spec, sizeof( Target_Spec ) );

			str_length = strlen( disk_spec[count].name );

			if (!show_unc_path)
			{
				// Display the remote disk label (just like original routine)
				Volume_GetInfo(&disk_spec[count], TRUE);
			}
			else
			{
				strcpy(disk_spec[count].basic_name, disk_spec[count].name);

				// Show the unc network share name instead, as seen by the host
				// running dynamo e.g. \\servername\pathname
				NameInfo = NetUseInfo[i].ui0_remote;
				NameInfo.OemToAnsi();
				strcpy( &disk_spec[count].name[str_length-1], NameInfo.GetBuffer(0) ); 
			}

	#if _DETAILS
			cout << "GUI volume descriptor: " << disk_spec[count].name << endl;
			//cout << endl;
	#endif

			count++;
		}
		
		// We should call this in the case of ERROR_MORE_DATA also, but with the 
		// MAX_PREFERRED_LENGTH option, this should not be necessary.
		NetApiBufferFree( NetUseInfo );
	}

	else
	{
		cerr << " Failed enumerating network shares, error=" << GetLastError() << endl;
	}

	cout << "Found: " << (count - start_count) << endl;

	return count;

}

char * DeleteSpaces(char *input)
{
	int i = strlen(input);
	char *prev = &input[i];

	while (i--)
	{
		if (isalnum(input[i]))
		{
			*prev = '\0';
			break;
		}
		prev = &input[i];
	}

	return input;
}

//
// Report_RawDisks()
// 
// INPUTS:
//		disk_spec	same disk_spec passed to Report_Disks
//		start_count	the index into the disk_spec array where to deposit the new entry
//		force_all	indicates that all raw disks should be presented, regardless
//					of existing partitions, etc...
//
// OUTPUT:
//		int			The next free entry/index into the disk_spec array 
//					(start_count + number of discovered devices)
//
int Report_RawDisks( Target_Spec *disk_spec, int start_count, int view_type )
{
	int	drive_number = 0, number_skipped = 0;
	int count = start_count;
	TargetDisk d;
	DosDeviceList dev_list = {0,0};

	cout << "Reporting raw disk information..." << endl;

	for (drive_number=0; count < MAX_TARGETS; drive_number++)
	{
		// We already have the string (but w/out \\.\, so regenerate is here.
		sprintf( d.spec.name, "%s%i", PHYSICAL_DISK, drive_number );
		strcpy( d.file_name, d.spec.name );
		d.spec.type = PhysicalDiskType;

		if ( !d.Open( NULL ) )
		{
			// we should not be here, since we know that all of the disks we are opening exist
			if (number_skipped == 0)
				cerr << "Failed opening device " << d.spec.name << "." << endl;

			// if open fails, we will fail everything else, so just to a goto here, all else are ok to continue
			goto DoHoleCheck;
		}
		d.Close( NULL );

		#if _DETAILS
			cout << "\nFound disk: " << d.spec.name << endl;
		#endif


		// This is our basic method of associating physical and logical disks, we really
		// need this to work on all disk devices!
		PSTORAGE_DEVICE_NUMBER psdn = (PSTORAGE_DEVICE_NUMBER) &d.spec.disk_info.disk_reserved[0];
		if (!GetDeviceNumber(d.spec.name, psdn)) //GET_SDN_PTR(d.spec.disk_info)))
		{
			cerr << "Failed retrieving device number for " << d.spec.name << "." << endl;
			continue;
		}

		// Drive exists, see if it is available for accessing.
		// Init will check for existence of partitions known or otherwise 
		// and put them in has_partitions field
		if( ! d.Init_Physical(d.spec.name) )
		{
			cerr << "Init_Physical(): failed on " << d.spec.name << "." << endl;
			continue;
		}

		// Do here what would have otherwise been done via Init_Physical
		if ((view_type != RAWDISK_VIEW_FULL) 
			&& (d.spec.disk_info.has_partitions == TRUE))
		{
			cout << "Physical disk \'" << d.spec.name << "\' contains partition information." << endl;
			cout << "Use /force_raw option to display." << endl;
			continue;
		}

		memcpy( &disk_spec[count], &d.spec, sizeof( Target_Spec ) );

		//
		// Instead of displaying "PHYSICALDISK1:" as the display name, lets use 
		// an inquiry string windows knows about
		//
		PSTORAGE_DEVICE_DESCRIPTOR sdd;
		if ((view_type != RAWDISK_VIEW_COMPAT) 
			&& GetStorageProperty(d.spec.name, &sdd, 0, TRUE))
		{
			char * temp = (char *) sdd;
			sprintf(disk_spec[count].name, "%d: \"%s %s %s\"", 
				drive_number, 
				DeleteSpaces(GET_STORPROP_VENDOR_STRINGS(temp)),
				DeleteSpaces(GET_STORPROP_PRODUCT_STRINGS(temp)),
				DeleteSpaces(GET_STORPROP_REVISION_STRINGS(temp))
				);

			delete [] sdd; // sdd was allocated as a char[] in GetStorageProperty; hopefully this works.
		}
		else
		{
			// If we cannot get the storage property, resort to the old method
			strcpy( disk_spec[count].name, PHYSICAL_DRIVE_PREFIX );
			_itoa( drive_number, disk_spec[count].name + strlen( disk_spec[count].name ), 10 );
		}

	#if _DETAILS
		cout << "GUI volume descriptor: " << disk_spec[count].name << endl;
		//cout << endl;
	#endif

		count++;
		continue;

DoHoleCheck:
		if (++number_skipped == PHYS_DISK_HOLE_SIZE) 
			break;
	}

	delete [] dev_list.list;

	cout << "Found: " << (count - start_count) << endl;
	return count;

}

//
// Support routines
//

//
// Volume_GetInfo()
//
// Appends the volume's label into the disk_spec[n].name member,
// separating the existing volume name and the label by a colon ":".
//
void Volume_GetInfo( Target_Spec *disk_spec, bool bCompatible)
{
	DWORD vol_serialnumber, dummy_word, fs_options;
	int str_length;
	char buffer[MAX_NAME], fs_name[MAX_NAME];

	strcpy(disk_spec->basic_name, disk_spec->name); // save off a copy before we tweak with it below
	str_length = strlen( disk_spec->name );

	// GetVolumeInformation likes the trailing slash
	if ( !GetVolumeInformation( 
			disk_spec->name, 
			buffer,
			MAX_NAME, 
			&vol_serialnumber, 
			&dummy_word, 
			&fs_options, 
			fs_name, 
			MAX_NAME ) )
	{
		cerr << "Unable to retrieve volume information for " << disk_spec->name 
			 << ", error=" << GetLastError() << "." << endl;

		// No volume label, just terminate the string
		STRIP_LAST_SLASH(disk_spec->name);
		return;
	}
	else
	{
		// Remove the trailing slash
		STRIP_LAST_SLASH(disk_spec->name);

		if (bCompatible)
		{
			// Append the volume label, separated by a colon
			if (disk_spec->name[str_length - 1] != ':')
				strcat(disk_spec->name, ":");

			strcat(disk_spec->name, buffer);
		}
		else
		{
			// resuse fs_name
			sprintf(fs_name, " \"%s\"", buffer);
			strcat(disk_spec->name, fs_name);
		}

		if (fs_options & FILE_READ_ONLY_VOLUME)
			disk_spec->read_only = TRUE;
	}
}

bool GetDeviceNumber(char *name, PSTORAGE_DEVICE_NUMBER sdn)
{
	HANDLE hDisk = INVALID_HANDLE_VALUE;
	CHAR  Buffer[_MAX_PATH];
	STORAGE_DEVICE_NUMBER storDevNum;

	strcpy(Buffer, name);

	STRIP_LAST_SLASH(Buffer);

	if (!SendIoControl(Buffer, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, (LPVOID) &storDevNum, sizeof(STORAGE_DEVICE_NUMBER)))
		return FALSE;

	#if _DETAILS
	cout << " DeviceType=" << (int) storDevNum.DeviceType
		 << " DeviceNumber=" << (int) storDevNum.DeviceNumber 
		 << " PartitionNumber=" << (int) storDevNum.PartitionNumber << endl;
	#endif

	//
	// We have obtained a valid scsi address to compare against, so enable our flag
	//
	if (sdn)
		*sdn = storDevNum;

	return TRUE;
}

bool GetDiskExtents(char *name, PVOID vde, PULONG size, bool allocate)
{
	CHAR  Buffer[_MAX_PATH];
	PCHAR volBuf;
	PVOLUME_DISK_EXTENTS volDiskExt, Vde, *pVde;
	DWORD ErrorCode, Size, bufLen, tries=2;
	HANDLE hDevice = INVALID_HANDLE_VALUE;

	Vde  = (PVOLUME_DISK_EXTENTS)   vde;
	pVde = (PVOLUME_DISK_EXTENTS *) vde;

	strcpy(Buffer, name);

	STRIP_LAST_SLASH(Buffer);

	// 
	// Obtain a handle to our device
	//		
	hDevice = CreateFile(
		Buffer,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		(LPSECURITY_ATTRIBUTES) NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE) NULL);

	if (hDevice == INVALID_HANDLE_VALUE)
	{
		//#ifdef _DETAILS
		cerr << "GetDiskExtents(): Failed opening device " << Buffer 
			 << " , error=" << GetLastError() << "." << endl;
		//#endif
		return FALSE;
	}

	// some weird packing problem, so use 4 extents for starters
	bufLen = sizeof(VOLUME_DISK_EXTENTS) +  4 * sizeof(DISK_EXTENT);

	while (tries--) {
		volBuf =  new char[bufLen];
		volDiskExt = (PVOLUME_DISK_EXTENTS) volBuf;

		ErrorCode = DeviceIoControl(
			hDevice,
			IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
			NULL,
			0,
			volDiskExt,
			bufLen,
			&Size,
			NULL);

		if (!ErrorCode) {
			ErrorCode = GetLastError();
			delete [] volBuf;
			if (ErrorCode == ERROR_MORE_DATA || ErrorCode == ERROR_INSUFFICIENT_BUFFER) {
				//bufLen = sizeof(VOLUME_DISK_EXTENTS) + ((volDiskExt->NumberOfDiskExtents-1) * sizeof(DISK_EXTENT));
				bufLen *=4;
			#ifdef _DETAILS
				cout << "GetDiskExtents(): Retrying IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS to device "
					 << Buffer <<  " with size=" << bufLen << ", numext=" << volDiskExt->NumberOfDiskExtents << endl;
			#endif
			} else { 
			//#ifdef _DETAILS
				cerr << "GetDiskExtents(): Failed sending IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS with error="
					 << GetLastError() << endl;
			//#endif
				break;
			}
		}
		else break;
	}

	CloseHandle(hDevice);

	if (!ErrorCode) {
		return FALSE;
	}


	#ifdef _DETAILS
	cout << "Volume Disk Extents=" << (int) volDiskExt->NumberOfDiskExtents << " Disk(s)=";
	for (ULONG i=0; i<volDiskExt->NumberOfDiskExtents; i++)
		cout << volDiskExt->Extents[i].DiskNumber << ", ";
	cout << endl;
	#endif

	//
	// We have obtained a valid disk extent
	//
	if (allocate && vde)
	{
		// caller will de-allocate
		*pVde = volDiskExt;
		return TRUE;
	}
	else if (size && *size < bufLen) 
	{
		*size = bufLen;
		delete [] volBuf;
		return FALSE;
	}
	else if (!vde)
	{
		delete [] volBuf;
		return FALSE;
	}
	
	memcpy(Vde, volDiskExt, Size);
	delete [] volBuf; // always delete our temporary storage
	return TRUE;
}

#ifdef _DETAILS
typedef struct _STORAGE_BUS_TYPE_NAME 
{
	STORAGE_BUS_TYPE Value;
	CHAR Name[_MAX_FNAME];
} STORAGE_BUS_TYPE_NAME, *PSTORAGE_BUS_TYPE_NAME;
#endif

bool GetStorageProperty(char *name, PVOID in, PULONG size, bool allocate)
{
	STORAGE_PROPERTY_QUERY spq;
	PSTORAGE_DEVICE_DESCRIPTOR sdd, psdd, *ppsdd;
	CHAR  Buffer[_MAX_PATH];
	char *propBuf;
	DWORD ErrorCode, Size, bufLen, tries=3;
	HANDLE hDevice = INVALID_HANDLE_VALUE;

	psdd  = (PSTORAGE_DEVICE_DESCRIPTOR)   in;
	ppsdd = (PSTORAGE_DEVICE_DESCRIPTOR *) in;

	strcpy(Buffer, name);

	STRIP_LAST_SLASH(Buffer);

	// 
	// Obtain a handle to our device
	//		
	hDevice = CreateFile(
		Buffer,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		(LPSECURITY_ATTRIBUTES) NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE) NULL);

	if (hDevice == INVALID_HANDLE_VALUE)
	{
		//#ifdef _DETAILS
		cerr << "GetStorageProperty(): Failed opening device " << Buffer 
			 << " , error=" << GetLastError() << "." << endl;
		//#endif
		return FALSE;
	}

	// some weird packing problem, so use *2 for initial, just to be sure
	bufLen = sizeof(STORAGE_DEVICE_DESCRIPTOR);

	while (tries--) 
	{
		propBuf =  new char[bufLen];
		sdd = (PSTORAGE_DEVICE_DESCRIPTOR) propBuf;

		spq.PropertyId = StorageDeviceProperty;
		spq.QueryType  = PropertyStandardQuery;

		ErrorCode = DeviceIoControl(
			hDevice,
			IOCTL_STORAGE_QUERY_PROPERTY,
			&spq,
			sizeof(STORAGE_PROPERTY_QUERY),
			sdd,
			bufLen,
			&Size,
			NULL);

		if (!ErrorCode) 
		{
			delete [] propBuf;
			#ifdef _DETAILS
				cerr << "GetStorageProperty(): Failed sending IOCTL_STORAGE_QUERY_PROPERTY with error="
					 << GetLastError() << endl;
			#endif
				break;
		}
		else if (Size < sdd->Size)
		{
			if (bufLen >= sdd->Size)
				bufLen *=2;
			else
				bufLen = sdd->Size;

			delete [] propBuf;

			#ifdef _DETAILS
				cout << "GetStorageProperty(): Retrying IOCTL_STORAGE_QUERY_PROPERTY to device "
					 << Buffer <<  " with size=" << bufLen << endl;
			#endif
		}
		else break;
	}

	CloseHandle(hDevice);

	if (!ErrorCode || (Size < sdd->Size)) {
	#ifdef _DETAILS
		cout << "GetStorageProperty(): IOCTL_STORAGE_QUERY_PROPERTY failed with error=" << GetLastError() << endl;
	#endif
		return FALSE;
	}


	#ifdef _DETAILS

STORAGE_BUS_TYPE_NAME BusNames[BusTypeSata+1] = 
{
	{BusTypeUnknown, "Unknown"},
	{BusTypeScsi, "Scsi"},
	{BusTypeAtapi, "Atapi"},
	{BusTypeAta, "Ata"},
	{BusType1394, "1394"},
	{BusTypeSsa, "Ssa"},
	{BusTypeFibre, "FibreChannel"},
	{BusTypeUsb, "Usb"},
	{BusTypeRAID, "Raid"},
	{BusTypeiScsi, "iScsi"},
	{BusTypeSas, "Sas"},
	{BusTypeSata, "Sata"}
};

	char *vendor = (char*) (propBuf + sdd->VendorIdOffset);
	char *product = (char*) (propBuf + sdd->ProductIdOffset);
	char *revision = (char*) (propBuf + sdd->ProductRevisionOffset);
	char *serial = (char*) (propBuf + sdd->SerialNumberOffset);

	cout << "Storage property:" << endl;
	cout << "  DeviceType=" << (UINT) sdd->DeviceType << endl;
	cout << "  DeviceTypeModifier=" << (UINT) sdd->DeviceTypeModifier << endl;
	cout << "  RemovableMedia=" << (UINT) sdd->RemovableMedia << " CommandQueueing=" << (UINT) sdd->CommandQueueing << endl;
	if (sdd->VendorIdOffset)
		cout << "  VendorId=" <<  vendor << endl;
	if (sdd->ProductIdOffset)
		cout << "  ProductId=" <<  product << endl;
	if (sdd->ProductRevisionOffset)
		cout << "  ProductRevision=" << revision << endl;
	if (sdd->SerialNumberOffset)
		cout << "  SerialNumber=" << serial  << endl;
	cout << "  BusType=" << BusNames[sdd->BusType].Name << endl;
	#endif

	//
	// We have obtained a valid disk extent
	//
	if (allocate && in)
	{
		// caller will de-allocate
		*ppsdd = sdd;
		return TRUE;
	}
	else if (size && *size < bufLen) 
	{
		*size = bufLen;
		delete [] propBuf;
		return FALSE;
	}
	else if (!in)
	{
		delete [] propBuf;
		return FALSE;
	}
	
	memcpy(psdd, sdd, Size);
	delete [] propBuf; // always delete our temporary storage
	return TRUE;
}


bool SendIoControl( char *device, DWORD control, LPVOID in,  DWORD size_in, 
											     LPVOID out, DWORD size_out)
{
	DWORD ErrorCode, Size;
	HANDLE hDevice = INVALID_HANDLE_VALUE;

	// 
	// Obtain a handle to our device
	//		
	hDevice = CreateFile(
		device,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		(LPSECURITY_ATTRIBUTES) NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE) NULL);

	if (hDevice == INVALID_HANDLE_VALUE)
	{
		#ifdef _DETAILS
			cerr << "SendIoControl(): Failed opening device " << device 
				<< " , error=" << GetLastError() << "." << endl;
		#endif
		return FALSE;
	}

	ErrorCode = DeviceIoControl(
		hDevice,
		control,
		in,
		size_in,
		out,
		size_out,
		&Size,
		NULL);

	CloseHandle(hDevice);

	if (!ErrorCode)
	{
		#ifdef _DETAILS
			cerr << "SendIoControl(): Failed sending ioctl " << control << " to device "
				 << device <<  " , error=" << GetLastError() << "." << endl;
		#endif

		return FALSE;
	}
	
	return TRUE;
}

// Compares Target_Spec entries for use by qsort routine
int
__cdecl 
TS_Compare_BasicName(const void *elem1, const void *elem2)
{
	return strcmp(((Target_Spec*)elem1)->basic_name, ((Target_Spec*)elem2)->basic_name);
}

int MergeVolumesAndRawDisks(Target_Spec *dest, Target_Spec *source, ULONG mid_point, ULONG total, BOOL InsertUnknown)
{
	ULONG d = (ULONG) -1;

	for (ULONG v = 0; v < mid_point; v++)
	{

#ifdef _DETAILS2 
		cout << "Parsing " << source[v].name << endl;
#endif
		// skip over used volumes
		if (source[v].type == 0)
		{
#ifdef _DETAILS2 
		cout << " Skipping."  << endl;
#endif
			continue;
		}

		// copy over our volume
		dest[++d] = source[v]; 

		// find all other volumes that may reside on the same physical disk and listed them together
		for (ULONG r = v+1; r < mid_point; r++)
		{
			if (source[r].type == 0) // a cleared entry
				continue;
#ifdef _DETAILS2 
			cout << " Looking for sibling volumes..." << endl;
			cout << " Comparing " << GET_SDN_PTR(source[v].disk_info)->DeviceNumber << " " << GET_SDN_PTR(source[v].disk_info);
			cout << "  to  " << GET_SDN_PTR(source[r].disk_info)->DeviceNumber << " " << GET_SDN_PTR(source[r].disk_info) << endl;
#endif
			if (EQUAL_DEVICE_NUMBERS(GET_SDN_PTR(source[v].disk_info), GET_SDN_PTR(source[r].disk_info)))
			{
#ifdef _DETAILS2 
				cout << " Sibling volume " << source[v].name << endl;
#endif
				// just copy over another volume
				dest[++d] = source[r];
				source[r].type = (TargetType) 0; // null out this volume so we skip it above
			}
		}

		// Optimization: we know the physical disks are sorted in increasing order; the only 
		// problem is if there are holes. Could check for easy case first (plus extents) and 
		// then fall back to slow search.

		int quick_index = mid_point + GET_SDN_PTR(source[v].disk_info)->DeviceNumber + 1;
		if (EQUAL_DEVICE_NUMBERS(GET_SDN_PTR(source[v].disk_info), GET_SDN_PTR(source[quick_index].disk_info)))
		{
			PVOLUME_DISK_EXTENTS *pvde = GET_VDE_PTR(source[v].disk_info);				
			PSTORAGE_DEVICE_NUMBER psdn = GET_SDN_PTR(source[v].disk_info);;

			// update our hierarchy indicator
			source[quick_index].type = PhysicalDiskTypeHasPartitions;

#ifdef _DETAILS2 
		cout << " Quick matched raw disk " << source[quick_index].name << endl;
#endif

			dest[++d] = source[quick_index];
			source[quick_index].type = (TargetType) 0;

			if ( *pvde && ((*pvde)->NumberOfDiskExtents == (psdn->PartitionNumber * -1))) // we have multiple physical mappings
			{
				//ASSERT(dest[d].disk_info.device_number.DeviceNumber == pvde->Extents[0].DiskNumber);

				// re-scan the raw list for the rest of the extents
				ULONG e=1;
				for (ULONG rr = quick_index + 1; 
					 (rr < total) && (e < (*pvde)->NumberOfDiskExtents);
					 rr++)
				{
					if (source[rr].type == 0)
						continue;

					psdn = GET_SDN_PTR(source[rr].disk_info);
					// this only works, becuase both raw and extents disk numbers ordered
					if (psdn->DeviceType == FILE_DEVICE_DISK &&
						(psdn->DeviceNumber == (*pvde)->Extents[e].DiskNumber))
					{
#ifdef _DETAILS2 
						cout << "  Quick matched raw disk extent " << source[rr].name << endl;
#endif

						source[rr].type = PhysicalDiskTypeHasPartitions;
						dest[++d] = source[rr];
						source[rr].type = (TargetType) 0;
						e++; // increment e only if we found something, we had better hit all values of e!
					}
				}

				if (e < (*pvde)->NumberOfDiskExtents)
				{
					cout << "WARNING: only found " << e << " disk extents out of " <<  (*pvde)->NumberOfDiskExtents << " for volume." << endl;
					cout << " Extent disk numbers: ";
					for (DWORD i=0; i< (*pvde)->NumberOfDiskExtents; i++)
						cout << (*pvde)->Extents[i].DiskNumber << ", " ;
					cout << endl;
				}

				// this delete matches the callee allocated buffer in GetDiskExtents()
				// iometer does not need it
				delete [] *pvde;
			}
		}
		else
		{

			// now search for physical disks matching the device_number structure
			for (ULONG r = mid_point; r < total; r++)
			{
				// skip over used disks
				if (source[r].type == 0)
					continue;

	#ifdef _DETAILS2 
				cout << " Looking for children raw disks..." << endl;
				cout << " Comparing " << GET_SDN_PTR(source[v].disk_info)->DeviceNumber << " " << GET_SDN_PTR(source[v].disk_info);
				cout << "  to  " << GET_SDN_PTR(source[r].disk_info)->DeviceNumber << " " << GET_SDN_PTR(source[r].disk_info) << endl;
	#endif
				if (EQUAL_DEVICE_NUMBERS(GET_SDN_PTR(source[v].disk_info), GET_SDN_PTR(source[r].disk_info)))
				{
					PVOLUME_DISK_EXTENTS *pvde = GET_VDE_PTR(source[v].disk_info);				
					PSTORAGE_DEVICE_NUMBER psdn = GET_SDN_PTR(source[v].disk_info);;

					// update our hierarchy indicator
					source[r].type = PhysicalDiskTypeHasPartitions;

	#ifdef _DETAILS2 
			cout << " Matched raw disk " << source[r].name << endl;
	#endif

					dest[++d] = source[r];
					source[r].type = (TargetType) 0;

					if ( *pvde && ((*pvde)->NumberOfDiskExtents == (psdn->PartitionNumber * -1))) // we have multiple physical mappings
					{
						//ASSERT(dest[d].disk_info.device_number.DeviceNumber == pvde->Extents[0].DiskNumber);

						// re-scan the raw list for the rest of the extents
						ULONG e=1;
						for (ULONG rr = r+1; 
							 (rr < total) && (e < (*pvde)->NumberOfDiskExtents);
							 rr++)
						{
							if (source[rr].type == 0)
								continue;

							psdn = GET_SDN_PTR(source[rr].disk_info);

							// this only works, becuase both raw and extents disk numbers ordered
							if (psdn->DeviceType == FILE_DEVICE_DISK &&
								(psdn->DeviceNumber == (*pvde)->Extents[e].DiskNumber))
							{
	#ifdef _DETAILS2 
								cout << "  Matched raw disk extent " << source[rr].name << endl;
	#endif

								source[rr].type = PhysicalDiskTypeHasPartitions;
								dest[++d] = source[rr];
								source[rr].type = (TargetType) 0;
								e++; // increment e only if we found something, we had better hit all values of e!
							}
						}

						if (e < (*pvde)->NumberOfDiskExtents)
						{
							cout << "WARNING: only found " << e << " disk extents out of " <<  (*pvde)->NumberOfDiskExtents << " for volume." << endl;
							cout << " Extent disk numbers: ";
							for (DWORD i=0; i< (*pvde)->NumberOfDiskExtents; i++)
								cout << (*pvde)->Extents[i].DiskNumber << ", " ;
							cout << endl;
						}

						// this delete matches the callee allocated buffer in GetDiskExtents()
						// iometer does not need it
						delete [] *pvde;
					}

					// if we found a raw disk, we'll have scanned extens, so its ok to break
					break;
				}
			}
		}
	}

	// Continue if there are any unpaired raw devices
	if (d + 1 < total)
	{
		for (ULONG a = mid_point; (a < total) && (d < (2*total - mid_point)); a++)
		{
			// do linear search for the physical disk matching non-zero for devicetype 
			// skipping the ones we have zeroed above
			if (source[a].type != 0)
			{
				// If a physical disk has partitions, but no mounted or recognized
				// volumes, we create a dummy logical disk to represent that data
				// controlled by InsertUnknown input option 
				if (InsertUnknown && source[a].disk_info.has_partitions)
				{
					ZeroMemory(&dest[++d], sizeof(Target_Spec));
					strcpy(dest[d].name, "Unmounted or unknown");
					*(GET_SDN_PTR(dest[d].disk_info))= *(GET_SDN_PTR(source[a].disk_info));
					dest[d].type = LogicalDiskType; // ??
					
					// Alter the disk type for iometer's tree view sturctures to work
					source[a].type = PhysicalDiskTypeHasPartitions;
				}
#ifdef _DETAILS2 
				cout << "Found unassociated disk " << source[a].name << endl;
#endif
				dest[++d] = source[a];
			}
		}
	}

	return d + 1; // d is effectively a 0-based index, and we want to return 1-based
}

int
__cdecl 
TS_Compare_DeviceNumberLogiFirst(const void *elem1, const void *elem2)
{
	Target_Spec *ts1, *ts2;
	STORAGE_DEVICE_NUMBER *dn1, *dn2;

	ts1 = (Target_Spec*) elem1;
	ts2 = (Target_Spec*) elem2;
	
	dn1 = (PSTORAGE_DEVICE_NUMBER) &ts1->disk_info.disk_reserved[0];
	dn2 = (PSTORAGE_DEVICE_NUMBER) &ts2->disk_info.disk_reserved[0];

	if      (dn1->DeviceType < dn2->DeviceType) return -1;
	else if (dn1->DeviceType > dn2->DeviceType) return  1;
	else {
		if      (dn1->DeviceNumber < dn2->DeviceNumber) return -1;
		else if (dn1->DeviceNumber > dn2->DeviceNumber) return  1;
		else {
			// if this device does not have a scsi address, do not associate it with anything...
			if (dn1->DeviceType==MINUS_ONE 
				&& dn1->DeviceNumber==MINUS_ONE 
				&& dn1->PartitionNumber==MINUS_ONE)
				return 0;

			// Find adjacent physicadisktype items and update them to *haspartitions.
			// IsType() is a bitwise check, so the conditionals will still be true afterward.
			if (IsType(ts1->type, LogicalDiskType) && IsType(ts2->type, PhysicalDiskType)) {
				ts2->type = PhysicalDiskTypeHasPartitions;
				return -1;
			}
			else if (IsType(ts2->type, LogicalDiskType) && IsType(ts1->type, PhysicalDiskType)) {
				ts1->type = PhysicalDiskTypeHasPartitions;
				return 1;
			}
			else 
				return 0;
		}
	}
}


#endif