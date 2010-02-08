/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / IOManagerLinux.cpp                                        ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: This file contains the Linux related methods of       ## */
/* ##               Dynamo's main class (IOManager.cpp).                  ## */
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
/* ##  Changes ...: 2005-04-18 (mingz@ele.uri.edu)                        ## */
/* ##               - Added a swap-device-check to avoid use swap devices ## */
/* ##                 as target devices by mistake.                       ## */
/* ##               2004-09-26 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Removed a note about the Reported_As_Logical()      ## */
/* ##                 function as this one was removed.                   ## */
/* ##               2004-07-26 (mingz@ele.uri.edu)                        ## */
/* ##               - Extended the Report_Disks() method to considere     ## */
/* ##                 the blkdevlist.                                     ## */
/* ##               2004-03-27 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added MAX_NAME change detection to ensure that      ## */
/* ##                 the fscanf()'s stays in sync with it.               ## */
/* ##               - Applied Dan Bar Dov's patch for adding              ## */
/* ##                 Linux on PPC support.                               ## */
/* ##               2004-02-19 (mingz@ele.uri.edu)                        ## */
/* ##               - Rewrote the Report_TCP for linux code,              ## */
/* ##                 The old one is buggy and if you do not set          ## */
/* ##                 hostname correctly, it always return a 127.0.0.1    ## */
/* ##               2003-07-13 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-03-05 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Removed LINUX_DEBUG, because it is redundant.       ## */
/* ##                 We can use the generic _DEBUG therefor.             ## */
/* ##               2003-02-26 (joe@eiler.net)                            ## */
/* ##               - replaces EXCLUDE_FILESYS define with a string       ## */
/* ##                 so filesystem types are no longer hard coded.       ## */
/* ##               2003-02-15 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##               - Modified EXCLUDE_FILESYS to support NFS devices     ## */
/* ##                 per default (was excluded by default).              ## */
/* ##                                                                     ## */
/* ######################################################################### */
#if defined(IOMTR_OS_LINUX)

#include "IOManager.h"
#include "IOTargetDisk.h"
#include <dirent.h>
#include <ctype.h>
#include <mntent.h>
#include <sys/wait.h>
#include <sys/swap.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <assert.h>

#define USED_DEVS_MAX_SIZE (4 * 1024)

/**********************************************************************
 * Forward Declarations
 **********************************************************************/
static int compareRawDiskNames(const void *a, const void *b);

//
// Checking for all accessible drives.  Storing them, and returning the number
// of disks found.  Drives are reported in order so that Iometer does not
// need to sort them.
//
int Manager::Report_Disks(Target_Spec * disk_spec)
{
	TargetDisk d;
	int count = 0;
	char usedDevs[USED_DEVS_MAX_SIZE + 1];
	const char *mnttab;

	usedDevs[0] = '\0';

	FILE *sfile;
	char *swapbuf = NULL;

	// prepare swap buffer which contains content of /proc/swaps
	if ((sfile = fopen("/proc/swaps", "r")) == NULL)
		swapbuf = NULL;
	else {
		swapbuf = (char *)malloc(4096);
		if (swapbuf) {
			memset(swapbuf, 0, 4096);
			fread(swapbuf, sizeof(char), 4095, sfile);
			if (ferror(sfile)) {
				free(swapbuf);
				swapbuf = NULL;
			}
		}
		fclose(sfile);
	}

	cout << "Reporting drive information..." << endl;

	// *********************************************************************************
	// DEVELOPER NOTES
	// ---------------
	//
	// For linux, I return three sets of data:
	// 1) The root directory for all mounted normal (non-exclude_filesys) filesystems.
	//    These are our "logical" disks. They are found by scanning /etc/mtab.
	// 2) The device names supplied from command line.
	// 3) The device names for all unmounted block devices. These are our "physical"
	//    disks. They are found by reading /proc/partitions, then skipping all devices
	//    that are in mtab.
	//
	// **********************************************************************************

	//
	// First find all virtual disks by inspecting the mount table. Then search for
	// physical disks that aren't mounted.
	//

	if ((mnttab = getenv("MNTTAB")) == NULL)
		mnttab = _PATH_MOUNTED;

	FILE *file;

	if ((file = fopen(mnttab, "r")) == NULL) {
		cout << "open (mount tab) file " << mnttab << " failed with error " << errno << endl;
		cout << "Set environment variable MNTTAB to correct pathname" << endl;
		exit(1);
	}

	struct mntent *ment;

	int length;
	char disk_name[MAX_NAME];

	while ((ment = getmntent(file)) != NULL) {
#ifdef _DEBUG
		cout << "*** File system found: " << ment->mnt_fsname << "\n";
#endif
		if (!strncmp(ment->mnt_fsname, "/dev/", 5)) {
			// This is a real disk. Add it to our list of disks not to use as
			// physical devices.
			if (strlen(usedDevs) + 2 + strlen(ment->mnt_fsname + 5) >= USED_DEVS_MAX_SIZE) {
				cerr << "Too many devices for our list! Aborting.\n";
				exit(1);
			}
			strcat(usedDevs, " ");
			strcat(usedDevs, ment->mnt_fsname + 5);
			strcat(usedDevs, " ");
		}
		// see if the current file sys is an excluded file system type for dynamo.
		if (strstr(exclude_filesys, ment->mnt_type) != NULL) {
#ifdef _DEBUG
			cout << "*** File system type \"" << ment->mnt_type << "\" excluded.\n";
#endif
			continue;
		}

		length = MAX_NAME - strlen(ment->mnt_type);

		// take away 4 bytes from length for the decorations (spaces, [, ])
		length -= 4;

		strncpy(disk_name, ment->mnt_dir, length);
		disk_name[length] = 0;

		if (!d.Init_Logical(disk_name)) {
#ifdef _DEBUG
			cout << "*** " << __FUNCTION__ << ": Init_Logical failed.\n";
#endif
			continue;
		}
		// Drive exists and ready for use.
		d.spec.type = LogicalDiskType;
		memcpy(&disk_spec[count], &d.spec, sizeof(Target_Spec));

		disk_spec[count].name[length] = 0;
		// and TargetDisk::Init_Logical().
		strcat(disk_spec[count].name, " [");
		strcat(disk_spec[count].name, ment->mnt_type);
		strcat(disk_spec[count].name, "]");

#ifdef _DEBUG
		cout << "   Found " << disk_spec[count].name << "." << endl << flush;
#endif
		count++;
		if (count >= MAX_TARGETS)
			break;
	}
	fclose(file);

	if (count >= MAX_TARGETS)
		return count;

	cout << "  Physical drives (raw devices)..." << endl;

	int i, valid_devcnt;

	for (i = 0; i < MAX_TARGETS; i++) {
		if (blkdevlist[i][0] == 0) {
			break;
		}
		if (d.Init_Physical(blkdevlist[i])) {
			d.spec.type = PhysicalDiskType;
			memcpy(&disk_spec[count], &d.spec, sizeof(Target_Spec));
			if (++count >= MAX_TARGETS)
				return count;
		}
	}
	valid_devcnt = i;

	// Now reporting physical drives (raw devices). Data from /proc/partitions.
	// Example output from /proc/partitions:
	//////////////////////////////////////////////////////////////////////
	// major minor  #blocks  name
	//
	//    8     0    8887080 sda
	//    8     1    8225248 sda1
	//    8     2     658665 sda2
	//   22     0 1073741823 hdc
	//////////////////////////////////////////////////////////////////////
	// Note: In the above example, "hdc" is a CD-ROM, and "sda1" and "sda2" are
	// two partitions on the disk represented by "sda".

	file = fopen("/proc/partitions", "r");
	if (file == NULL) {
		cerr << "Open \"/proc/partitions\" failed (errno " << errno << "). " "Cannot locate physical disks.\n";
		exit(1);
	}
	//
	// Pull out the first line. It just describes the columns. The second line
	// is blank, but fscanf will skip that automatically.
	//
	int c;

	do {
		c = getc(file);
	} while ((c != '\n') && (c != EOF));

	char devName[MAX_NAME], paddedDevName[MAX_NAME + 2];

#if MAX_NAME != 80
#warning ===> WARNING: You have to keep the fscanf() and MAX_NAME in sync!
	// "fscanf(... %<nn>s)" has to be "MAX_NAME - 1"
#endif
	while ((count < MAX_TARGETS) && (fscanf(file, "%*d %*d %*d %79s", devName) == 1)) {
		snprintf(paddedDevName, MAX_NAME + 2, " %s ", devName);
#ifdef _DEBUG
		cout << __FUNCTION__ << ": Found device " << devName << "\n";
#endif
		if (strstr(usedDevs, paddedDevName) == NULL) {
			int skip;

			// Nobody has mounted this device. Try to open it for reading; if we can,
			// then add it to our list of physical devices.
#ifdef _DEBUG
			cout << __FUNCTION__ << ": Device is not mounted.\n";
#endif
			skip = 0;
			for (i = 0; i < valid_devcnt; i++) {
				if ((devName[0] == '/' && !strcmp(devName, blkdevlist[i])) ||
				    (devName[0] != '/' && !strcmp(devName, blkdevlist[i] + strlen("/dev/")))) {
#ifdef _DEBUG
					cout << "Find duplicate device " << devName << ", Ignoring." << endl;
#endif
					skip = 1;
					break;
				}
			}
			// check if is swap device
			if (!skip && swapbuf) {
				int i = strlen(devName) + 2;
				char *tmp = (char *)malloc(i);

				if (tmp) {
					memset(tmp, 0, i);
					strcpy(tmp, devName);
					tmp[i - 2] = ' ';
					if (strstr(swapbuf, tmp) != NULL) {
#ifdef _DEBUG
						cout << __FUNCTION__ << ": Device " << devName <<
						    " is a swap device, Ignoring." << endl;
#endif
						skip = 1;
					}
					free(tmp);
				}
			}
			if ((!skip) && d.Init_Physical(devName)) {
				d.spec.type = PhysicalDiskType;
				memcpy(&disk_spec[count], &d.spec, sizeof(Target_Spec));
				++count;
			}
		}
#ifdef _DEBUG
		else {
			cout << __FUNCTION__ << ": Device is mounted. Ignoring.\n";
		}
#endif

		//
		// Now we cut to the end of the line to get ready to start the next line.
		//
		do {
			c = fgetc(file);
		} while ((c != '\n') && (c != EOF));
	}
	fclose(file);
	if (swapbuf) {
		free(swapbuf);
	}
	qsort(disk_spec, count, sizeof(Target_Spec), compareRawDiskNames);
	return (count);
}

//
// This function compares two disk names. It just uses strcmp. I'm lazy.
//
static int compareRawDiskNames(const void *a, const void *b)
{
	const Target_Spec *at = (const Target_Spec *)a;
	const Target_Spec *bt = (const Target_Spec *)b;

	return (strcmp(at->name, bt->name));
}

int Manager::Report_TCP(Target_Spec * tcp_spec)
{
	int c, scanCount, i, skfd, count = 0;
	char ifname[32];
	FILE *netInfo;
	struct ifreq ifr;

	cout << "Reporting TCP network information..." << endl;

	netInfo = fopen("/proc/net/dev", "r");
	assert(netInfo != NULL);
	skfd = socket(PF_INET, SOCK_DGRAM, 0);
	if (skfd < 0) {
		cerr << "Can not create socket in Manager::Report_TCP" << endl;
		return -1;
	}
	// Pull out the first two lines of the file. These two lines contain
	// labels for the columns.
	for (i = 0; i < 2; ++i) {
		do {
			c = getc(netInfo);
		} while ((c != '\n') && (c != EOF));
	}

	for (i = 0; i < MAX_NUM_INTERFACES; ++i) {
		// grab the interface names (if there are leading blanks,
		// then they are removed using the Strip() function)
		scanCount = fscanf(netInfo, "%[^:]: %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d", ifname);
		if (scanCount == EOF) {
			break;
		}
		assert(scanCount == 1);
		Strip(ifname);

		// get ip address for the interface
		strcpy(ifr.ifr_name, ifname);
		ifr.ifr_addr.sa_family = AF_INET;
		if (ioctl(skfd, SIOCGIFADDR, &ifr) == 0) {
			strncpy(tcp_spec[count].name, inet_ntoa(((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr),
				sizeof(tcp_spec[count].name) - 1);
			tcp_spec[count].type = TCPClientType;	// interface to access a client

#ifdef _DEBUG
			cout << "   Found " << tcp_spec[count].name << "." << endl;
#endif
			count++;
		} else {
#ifdef _DEBUG
			cerr << "ioctl fail in Manager::Report_TCP()" << endl;
#endif
		}
		// Skip to the next line.
		do {
			c = getc(netInfo);
		} while ((c != '\n') && (c != EOF));
	}
	fclose(netInfo);
	close(skfd);
	// All done.
	cout << "   done." << endl;
	return count;
}

#endif				// IOMTR_OS_LINUX
