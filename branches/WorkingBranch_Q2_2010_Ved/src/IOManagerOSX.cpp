/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Dynamo) / IOManagerOSX.cpp                                        ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: OS specific Manager functions for Mac OSX             ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Intel Open Source License                                          ## */
/* ##                                                                     ## */
/* ##  Copyright (c) 2005 Apple Computer, Inc.                            ## */
/* ##  All rights reserved.                                               ## */
/* ##  Redistribution and use in source and binary forms, with or without ## */
/* ##  modification, are permitted provided that the following conditions ## */
/* ##  are met:                                                           ## */
/* ##                                                                     ## */
/* ##     Redistributions of source code must retain the above copyright  ## */
/* ##     notice, this list of conditions and the following disclaimer.   ## */
/* ##                                                                     ## */
/* ##     Redistributions in binary form must reproduce the above         ## */
/* ##     copyright notice, this list of conditions and the following     ## */
/* ##     disclaimer in the documentation and/or other materials provided ## */
/* ##     with the distribution.                                          ## */
/* ##                                                                     ## */
/* ##     Neither the name of Apple Computer nor the names of its         ## */
/* ##     contributors may be used to endorse or promote products derived ## */
/* ##     from this software without specific prior written permission.   ## */
/* ##                                                                     ## */
/* ##  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS## */
/* ##  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT  ## */
/* ##  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS  ## */
/* ##  FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE  ## */
/* ##  OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,## */
/* ##  SPECIAL, EXEPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT    ## */
/* ##  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF   ## */
/* ##  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED    ## */
/* ##  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT        ## */
/* ##  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING     ## */
/* ##  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE ## */
/* ##  POSSIBILITY OF SUCH DAMAGE.                                        ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Remarks ...: <none>                                                ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Changes ...: 2003-08-06 (raltherr@apple.com)                       ## */
/* ##               - File created                                        ## */
/* ##                                                                     ## */
/* ######################################################################### */
#if defined(IOMTR_OS_OSX)

#include "IOManager.h"
#include "IOTargetDisk.h"
#include <dirent.h>
#include <ctype.h>
#include <sys/mount.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOMedia.h>

//
// Checking for all accessible drives.  Storing them, and returning the number
// of disks found.  Drives are reported in order so that Iometer does not
// need to sort them.
//
int Manager::Report_Disks(Target_Spec * disk_spec)
{
	TargetDisk d;
	int count = 0, disksFound = 0, i, logicalDisks;
	struct statfs *buf = NULL;
	char disk_name[MNAMELEN], rawDeviceName[MNAMELEN];
	kern_return_t kernResult;
	DIR *directory;
	struct dirent *directoryEntry;
	mach_port_t masterPort;

	cout << "Reporting drive information..." << endl;

	// Reporting Logical drives first (filesystems).
	cout << "  Logical drives (mounted filesystems)..." << endl;

	count = getfsstat(NULL, 0, MNT_WAIT);
	buf = (struct statfs *)malloc(sizeof(struct statfs) * count);

	if (buf != NULL) {
		getfsstat(buf, sizeof(struct statfs) * count, MNT_WAIT);

		for (i = 0; i < count; i++) {
			if (strstr(exclude_filesys, buf[i].f_fstypename) != NULL)
				continue;

			strcpy(disk_name, buf[i].f_mntonname);
			if (!d.Init_Logical(disk_name))
				continue;

			// Drive exists and ready for use
			d.spec.type = LogicalDiskType;
			memcpy(&disk_spec[disksFound], &d.spec, sizeof(Target_Spec));

			disksFound++;

			if (disksFound >= MAX_TARGETS)
				break;

		}
		free(buf);
	}

	logicalDisks = disksFound;

	// Now reporting physical drives (raw devices)
	cout << "  Physical drives (raw devices)..." << endl << flush;
	kernResult = IOMasterPort(MACH_PORT_NULL, &masterPort);
	if (kernResult != KERN_SUCCESS) {
		printf("Failed to obtain mach master port\nSkipping physical device detection\n");
	} else {
		directory = opendir(RAW_DEVICE_DIR);
		while ((directoryEntry = readdir(directory)) != NULL) {
			if (strstr(directoryEntry->d_name, "disk") != NULL
			    && strstr(directoryEntry->d_name, "rdisk") == NULL) {
				if (!containsPartitions(masterPort, directoryEntry->d_name)) {
					strcpy(rawDeviceName, "r");
					strcat(rawDeviceName, directoryEntry->d_name);
					if (!d.Init_Physical(rawDeviceName))
						continue;

					// the physical drive is ready for use
					memcpy(&disk_spec[disksFound], &d.spec, sizeof(Target_Spec));

					disksFound++;
					if (disksFound >= MAX_TARGETS)
						break;
				}
			}
		}

		closedir(directory);
	}

	Sort_Raw_Disk_Names(disk_spec, logicalDisks, disksFound);

	cout << "    done." << endl << flush;
	return disksFound;
}

BOOL Manager::containsPartitions(mach_port_t masterPort, char *bsdName)
{
	// This is a rather simple and probably incorrect way to determine if there
	// are partitions on a disk.  IOKit is queried for the node that contains
	// the bsdName passed in.  The node is then checked to see if it is a
	// Whole item by checking for the existance and value of the Whole property.
	// Only Whole items are considered as they reference an entire disc.
	//
	// The node's Content key is then checked.  If the key is empty, we assume
	// that no partition structure (at least those identified by IOKit) is present
	// and that the device is blank.

	CFMutableDictionaryRef matchingDict;
	kern_return_t kernResult;
	io_iterator_t iter;
	io_service_t service;
	CFTypeRef wholeMedia;
	CFTypeRef contents;
	char contentsCSTR[255];
	bool result = FALSE;	// Start assuming it's a clean disk

	matchingDict = IOBSDNameMatching(masterPort, 0, bsdName);
	if (NULL == matchingDict) {
		return TRUE;
	} else {
		// Return an iterator across all objects with the matching BSD node name.  Note that there
		// should only be one match!
		kernResult = IOServiceGetMatchingServices(masterPort, matchingDict, &iter);

		if (KERN_SUCCESS != kernResult) {
			// If anything fails, we play it safe and just claim it has partitions
			result = TRUE;
		} else if (!iter) {
			result = true;
		} else {
			service = IOIteratorNext(iter);

			// Release this now because we only expect the iterator to contain
			// a single io_service_t
			IOObjectRelease(iter);

			if (!service) {
				result = TRUE;
			} else {
				// retrieve the Whole property
				wholeMedia = IORegistryEntryCreateCFProperty(service,
									     CFSTR(kIOMediaWholeKey),
									     kCFAllocatorDefault, 0);
				if (NULL == wholeMedia) {
					return TRUE;
				} else if (CFBooleanGetValue((CFBooleanRef) wholeMedia)) {
					// retrieve the Contents property
					contents = IORegistryEntryCreateCFProperty(service,
										   CFSTR(kIOMediaContentKey),
										   kCFAllocatorDefault, 0);
					if (NULL == contents) {
						result = TRUE;
					} else {
						CFStringGetCString((CFStringRef) contents,
								   contentsCSTR, 255, kCFStringEncodingASCII);
						// Empty string is assumed to mean no partition structure
						// WARNING: this is probably wrong, but it seems to work
						if (strcmp(contentsCSTR, "") != 0) {
							result = TRUE;
						}
					}
					CFRelease(contents);
				} else {	// Not whole media

					result = TRUE;
				}

				CFRelease(wholeMedia);
				IOObjectRelease(service);
			}
		}
	}

	return result;
}

BOOL Manager::Sort_Raw_Disk_Names(Target_Spec * disk_spec, int start, int end)
{
	int i, j;
	Target_Spec temp_spec;

	//
	// We have the whole Target_Spec array as input and the array contains the list
	// of all the disks that are to be reported. The start and end indexes identify
	// the starting and ending point for the sort.
	//
	// Plain, old, simplified bubble sort is being used here. Most of the disk path names
	// are already sorted. So it is ok to use this method.
	//
	for (i = start; i < end - 1; i++) {
		for (j = i + 1; j < end; j++) {
			//
			// The function Compare...() compares two disk names and returns values 
			// similar to strcmp().
			//
			switch (Compare_Raw_Disk_Names(disk_spec[i].name, disk_spec[j].name)) {
			case 0:	// both are equal
				break;
			case -1:	// 1st < 2nd
				break;
			case 1:	// 1st > 2nd
				// Swap the two.
				memcpy(&temp_spec, &disk_spec[i], sizeof(Target_Spec));
				memcpy(&disk_spec[i], &disk_spec[j], sizeof(Target_Spec));
				memcpy(&disk_spec[j], &temp_spec, sizeof(Target_Spec));
				break;
			}
		}
	}
	return TRUE;
}

//
// This function compares two disk names and returns values similar to strcmp().
// The comparison is not pure lexical but pure Numeric.
// For example,
//              - When comparing disk names c0t10d0p0 and c0t2d0p0, the function determines
//                that c0t2d0p0 is less than c0t10d0p0.
//              - When comparing disk names c0t0d0p0 and c0d0p0, the function determines
//                that c0d0p0 is less than c0t0d0p0 (going by the shorter string length).
//              - Disk names c0t0d0p0 and a0b0c0d0 are returned as equal. This is acceptable
//                because we do not come across such cases in the UNIX environment.
//
int Manager::Compare_Raw_Disk_Names(char *str1, char *str2)
{
	char *endptr[1];
	int num1 = -1;
	int num2 = -1;
	BOOL alpha1 = FALSE;
	BOOL alpha2 = FALSE;

	while (1) {
		if ((!*str1) || (!*str2)) {
			// either one or both strings have hit eos.
			if (*str1)	// str2 hit eos first.
				return (1);
			else if (*str2)	// str1 hit eos first.
				return (-1);
			else
				return 0;	// both hit eos.
		}

		if (alpha1 && alpha2) {
			// compare the characters at this position.
			if (*(str1 - 1) < *(str2 - 1))
				return (-1);
			if (*(str1 - 1) > *(str2 - 1))
				return (1);

			alpha1 = FALSE;
			alpha2 = FALSE;
		}
		// Scan the strings for the next available number.
		if (isalpha(*str1)) {
			str1++;
			alpha1 = TRUE;
			continue;
		}

		if (isalpha(*str2)) {
			str2++;
			alpha2 = TRUE;
			continue;
		}

		/*  Hit a digit or something else in both str1 and str2 */
		num1 = strtol(str1, endptr, 10);
		// printf("str1 : %s num : %d  endptr: %s\n", str1, num1, *endptr); 
		if (strcmp(str1, *endptr) == 0)
			str1++;
		else
			str1 = *endptr;

		num2 = strtol(str2, endptr, 10);
		// printf("str1 : %s num : %d  endptr: %s\n", str2, num2, *endptr); 
		if (strcmp(str2, *endptr) == 0)
			str2++;
		else
			str2 = *endptr;

		if (num1 == num2)
			// Still undecided. Loop back and continue the scan.
			continue;

		if (num1 < num2)
			// String 1 is less than string 2.
			return (-1);

		if (num1 > num2)
			// String 1 is greater than string 2.
			return (1);

	}			// end-while().
}

#endif				// IOMTR_OS_OSX
