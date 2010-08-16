/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Dynamo) / IOManagerSolaris.cpp                                    ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: OS specific Manager functions for Solaris             ## */
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
/* ##  Changes ...: 2004-09-26 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Removed the Reported_As_Logical() function as       ## */
/* ##                 it does not get used in any part of the code.       ## */
/* ##               - Removed the IOMTR_SETTING_OVERRIDE_FS dependency    ## */
/* ##                 for the Part_Reported_As_Logical() function.        ## */
/* ##               2004-02-13 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Reworked the Get_All_Swap_Devices() function to     ## */
/* ##                 fix a few minor inconsistencies.                    ## */
/* ##               2003-12-21 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Changed DYNAMO_DESTRUCTIVE to                       ## */
/* ##                 IOMTR_SETTING_OVERRIDE_FS                           ## */
/* ##               2003-10-05 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Integrated the error correction provided by         ## */
/* ##                 Rob Creecy to prevent the segmentation fault        ## */
/* ##                 when having multiple swap devices configured.       ## */
/* ##               2003-07-19 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Assimilated the patch from Robert Jones which is    ## */
/* ##                 needed to build under Solaris 9 on x86 (i386).      ## */
/* ##               2003-07-18 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-03-04 (joe@eiler.net)                            ## */
/* ##               - cleanup some warnings for Solaris                   ## */
/* ##               2003-02-26 (joe@eiler.net)                            ## */
/* ##               - replaces EXCLUDE_FILESYS define with a string       ## */
/* ##                 so filesystem types are no longer hard coded.       ## */
/* ##                                                                     ## */
/* ######################################################################### */
#if defined(IOMTR_OS_SOLARIS)

#include "IOManager.h"
#include "IOTargetDisk.h"
#include <dirent.h>
#include <ctype.h>
#include <sys/dktp/fdisk.h>
#include <sys/wait.h>
#include <sys/dkio.h>
#include <sys/vtoc.h>
#include <sys/swap.h>
#include <sys/mnttab.h>

#define MNTTAB_FILE				"/etc/mnttab"
static char *mnttab;

//
// Checking for all accessible drives.  Storing them, and returning the number
// of disks found.  Drives are reported in order so that Iometer does not
// need to sort them.
//
int Manager::Report_Disks(Target_Spec * disk_spec)
{
//      DWORD   dummy_word;
	TargetDisk d;

//      char    drive_letter;
//      int             drive_number = 0;
	int count = 0;
	DIR *dirp;
	struct dirent *dp;

//      int             i;

	cout << "Reporting drive information..." << endl;

	// *********************************************************************************
	// DEVELOPER NOTES
	// ---------------
	//              Currently the only disk types supported are Logical Disks (through the magic 
	// file iobw.tst on a mounted file system) and Physical disks (through the /dev/rdsk
	// interface).
	//
	// We can easily support a few other types with small changes to this function.They 
	// are 
	// 1) Direct Logical Disks (is Generic-UNIX)
	// 2) Meta Logical Disks   (might be Solaris-specific)
	// 3) Meta Physical disks. (      -do-               )
	//
	// 1)
	//              The Direct Logical Disks (*DO NOT* go through the magic file iobw.tst) can be
	// accessed through the /dev/dsk interfaces. Only ensure that the disks are not 
	// already mounted as file systems and also sort the list of disks.
	//
	// 2)
	//              The Meta Logical Disks (probably Solaris specific) are similar to Direct Logical
	// Disks. They are actually one or more logical disks grouped together and handled by a 
	// special driver. RAID Logical Disks come under this category.
	//
	// These disks can be accessed through the /dev/md/dsk interfaces. Ensure that the disks
	// are not already mounted as file systems and again file name sorting is required.
	//
	// 3)
	//              The Meta Physical Disks (probably Solaris specific) are similar to Direct 
	// Physical Disks. Again, Meta is for a grouping of one or more physical disks and RAID 
	// Physical Disks come under this category.
	//
	// These disks can be accessed through the /dev/md/rdsk interfaces. Again, they should
	// not already be mounted as file systems and file names should be sorted.
	//
	// The existing functions Sort_Raw_Disk_Names() (for physical disks) and
	// Look_For_Partitions() should easily work for the others too.
	// We should probably give more generic names to these functions.
	//
	// Also, if the new cases are implemented, then we should re-introduce prefixing raw
	// disk names with full pathnames. (eg: instead of c0t0d0p0 => /dev/rdsk/c0t0d0p0)
	// or evolve a scheme on the lines of the logical disks where we indicate the file sys
	// type along with the path names.
	//
	// **********************************************************************************

	// Reporting Logical drives first (filesystems).
	cout << "  Logical drives (mounted filesystems)..." << endl;

	if ((mnttab = getenv("MNTTAB")) == NULL)
		mnttab = MNTTAB_FILE;

	FILE *fp;

	if ((fp = fopen(mnttab, "r")) == NULL) {
		cout << "open (mount tab) file " << mnttab << " failed with error " << errno << endl;
		cout << "Set environment variable MNTTAB to correct pathname" << endl;
		exit(1);
	}

	struct mnttab mtab;
	int retval, length, buffered;
	char disk_name[MAX_NAME];

	while ((retval = getmntent(fp, &mtab)) == 0) {
		buffered = FALSE;
		// see if the current file sys is an excluded file system type for dynamo.
		if (strstr(exclude_filesys, mtab.mnt_fstype) != NULL)
			continue;

		if (hasmntopt(&mtab, "forcedirectio") == NULL) {
			cout << "NOTICE: filesystem " << mtab.mnt_mountp
			    << " not mounted with option - forcedirectio" << endl;
			// continue;            // enable only if we do not want to report this disk.
			buffered = TRUE;
			this->is_buffered = TRUE;
		}

		length = MAX_NAME - strlen(mtab.mnt_fstype);

		// take away 4 bytes from length for the decorations (spaces, [, ])
		// and 5 bytes for the string, ", BUF"
		if (buffered)
			length -= (4 + 4);
		else
			length -= 4;

		strncpy(disk_name, mtab.mnt_mountp, length);
		disk_name[length] = 0;

		if (!d.Init_Logical(disk_name))
			continue;

		// Drive exists and ready for use.
		d.spec.type = LogicalDiskType;
		memcpy(&disk_spec[count], &d.spec, sizeof(Target_Spec));

		disk_spec[count].name[length] = 0;
		// check for this pattern is also in TargetDisk::Init_Logical().
		strcat(disk_spec[count].name, " [");
		strcat(disk_spec[count].name, mtab.mnt_fstype);
		if (buffered)
			strcat(disk_spec[count].name, ", BUF");
		strcat(disk_spec[count].name, "]");

#ifdef _DEBUG
		cout << "   Found " << disk_spec[count].name << "." << endl << flush;
#endif
		count++;
		if (count >= MAX_TARGETS)
			break;
	}
	fclose(fp);

	if (this->is_buffered) {
		cout << endl
		    << "NOTICE: One or more filesystems not mounted with 'forcedirecio'"
		    << endl
		    << "        option set. All I/O will be via kernel buffers."
		    << endl << "  --->  See mount(1), mount_ufs(1) for details." << endl << endl;
		this->is_buffered = FALSE;
	}

	if (count >= MAX_TARGETS)
		return count;

	// Now reporting physical drives (raw devices)
	cout << "  Physical drives (raw devices)..." << endl;
	int logical_count = count;

	// Get a list of all the swap devices into a static public variable.
	// Its easier to lookup. We need to check if the partition/slice under
	// consideration is a swap device or not.
	Get_All_Swap_Devices();

	dirp = opendir(RAW_DEVICE_DIR);
	while ((dp = readdir(dirp)) != NULL) {
		//
		// Either we take all of the disk or none. No individual partitions. 
		// Do we really want to test partition by partition ? I guess not.
		//
		// Now we do - dive deep into the disk and look at the FDISK and
		// VTOC tables.
#if defined(IOMTR_CPU_I386) || defined(IOMTR_CPU_IA64)
		if (strstr(dp->d_name, "p0") != NULL)
#elif defined(IOMTR_CPU_SPARC)
		if (strstr(dp->d_name, "s2") != NULL)
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
		{
#if defined(IOMTR_CPU_I386) || defined(IOMTR_CPU_IA64)
			// Read partition table on fdisk partition p0.
			if (Report_FDISK_Partitions(dp->d_name, disk_spec, &count, logical_count) == TRUE) {
				// Some partitions were processed. The disk is not empty.
				// Skip this disk and jump back to start of while loop.
				if (count >= MAX_TARGETS)
					break;	// no room for more.

				continue;
			}
			// Here we continue with reporting fdisk partition p0. The fdisk
			// table is empty.
#elif defined(IOMTR_CPU_SPARC)
			if (Report_VTOC_Partitions(dp->d_name, disk_spec, &count, logical_count) == TRUE) {
				// some vtoc partitions were processed. The disk is not empty
				// skip this disk and jump back to start of while loop.
				if (count == MAX_TARGETS)
					break;	// no room for more.

				continue;
			}
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

			// Check if this is a swap device.
			if (strstr(swap_devices, dp->d_name)) {
				// Yes, it is. skip.
				continue;
			}

			if (!d.Init_Physical(dp->d_name))
				continue;

			// the physical drive is ready for use.
			memcpy(&disk_spec[count], &d.spec, sizeof(Target_Spec));
			d.spec.type = PhysicalDiskType;
#ifdef _DEBUG
			cout << "   Found " << disk_spec[count].name << "." << endl << flush;
#endif
			count++;
			if (count >= MAX_TARGETS)
				break;
		}
	}
	closedir(dirp);

	// sort all the raw disk names in disk_spec array.
	Sort_Raw_Disk_Names(disk_spec, logical_count, count);
	if (this->is_destructive) {
		cout << endl
		    << "NOTICE: One or more disk partitions/slices ignored because Iometer"
		    << endl
		    << "        detected the presence of file systems."
		    << endl
		    << "  --->  To enable access to the partition/slice mount it or destroy"
		    << endl
		    << "        it or over-ride protection by setting the IOMTR_SETTING_OVERRIDE_FS"
		    << endl << "        environment var." << endl << endl;
		this->is_destructive = FALSE;
	}

	cout << "   done." << endl << flush;
	return count;
}

#if defined(IOMTR_CPU_I386) || defined(IOMTR_CPU_IA64)
BOOL Manager::Report_FDISK_Partitions(char *name, Target_Spec * disk_spec, int *count, int logical_count)
{
	static BOOL has_solaris_parts = FALSE;
	BOOL FDISK_table_valid = FALSE;
	BOOL VTOC_valid = FALSE;
	TargetDisk d;
	int i, fd, bytes_read, length;
	struct mboot *mb;
	struct ipart *ip;
	char fstype[32];	// to hold the file system type.
	char base_name[MAX_NAME];
	char file_name[MAX_NAME];
	char buffer[512];

	memset(base_name, 0, MAX_NAME);
	memset(file_name, 0, MAX_NAME);
	// We don't deal directly with slices cXtXdXsX. Never gets that value.
	snprintf(file_name, MAX_NAME, "%s/%s", RAW_DEVICE_DIR, name);
#ifdef _DEBUG
	cout << "   Reporting disk partitions: " << file_name << endl << flush;
#endif

	fd = open(file_name, O_RDWR | O_LARGEFILE, S_IRUSR | S_IWUSR);
	if (fd < 0) {
#ifdef _DEBUG
		cout << "  open failed.." << endl << flush;
#endif
		return (TRUE);
	}

	if ((bytes_read = read(fd, buffer, 512)) < 512) {
		// cannot read start sector. Disk might be bad.
#ifdef _DEBUG
		cout << "  read failed.. returning" << endl << flush;
#endif
		close(fd);
		return (TRUE);
	}
	close(fd);

	mb = (struct mboot *)buffer;
	if (mb->signature != MBB_MAGIC) {
		// Don't worry about including disk spec in disk_spec. The caller
		// will take care of it when you return FALSE.
		// But yes, there is an additional loop going on here - a waste
		// of time but that does'nt matter for now.
		close(fd);
		return (FALSE);
	} else {
		// We do have partitions.
		strncpy(base_name, name, (strstr(name, "p0") - name));
#ifdef _DEBUG
		cout << "   basename: " << base_name << endl << flush;
#endif
		for (i = 1; i <= FD_NUMPART; i++) {
			ip = (struct ipart *)(buffer + BOOTSZ + ((i - 1) * sizeof(struct ipart)));

			if ((ip->systid == SUNIXOS) && (has_solaris_parts == FALSE)) {
				// Do this only once even if you have multiple solaris partitions.
				// Other Solaris parts are invisible through the disk label/vtoc.
				has_solaris_parts = TRUE;
				snprintf(file_name, MAX_NAME, "%ss2", base_name);
				if (Report_VTOC_Partitions(file_name, disk_spec, count, logical_count) == TRUE) {
					VTOC_valid = TRUE;
					continue;
				}
			}

			snprintf(file_name, MAX_NAME, "%sp%d", base_name, i);
#ifdef _DEBUG
			cout << "   file_name: " << file_name << endl << flush;
#endif
			// check if this is a swap device. Search the global string.
			if (strstr(swap_devices, file_name))
				// Yes, this is a swap device, skip
				continue;

			if (d.Init_Physical(file_name)) {
				// Disk Initialization succeeded, 
				// note that this is not needed here. We get here only if SUNIXOS 
				// partition does not have any VTOC (which is not possible on IA32) and
				// for other partitions.
				if (Part_Reported_As_Logical(disk_spec, file_name, logical_count))
					continue;

				// Not mounted! Next check if there is a file system on it.
				if (Has_File_System(file_name, fstype) == TRUE) {
#if defined(IOMTR_SETTING_OVERRIDE_FS)
					if (getenv("IOMTR_SETTING_OVERRIDE_FS") != NULL) {
						cout << "WARNING: allowing raw access to unmounted fs: "
						    << file_name << endl;
					} else	// environ variable not set
					{
#endif
						cout << "NOTICE: ignoring " << file_name
						    << ". File system found on disk" << endl;
						this->is_destructive = TRUE;
						continue;	// jump to start of for-loop
#if defined(IOMTR_SETTING_OVERRIDE_FS)
					}
#endif
				}
				// take away 16 bytes the largest of fdisk part names and
				// and 5 bytes for the string elements. "[  ]"
				length = MAX_NAME - 16 - 5;

				// file opened successfully. But size might be all messed up
				// since we read the geometry of the disk.
				// Reset it to correct value.
				d.spec.disk_info.maximum_size = ip->numsect;
				d.Set_Size(ip->numsect);
				memcpy(&disk_spec[*count], &d.spec, sizeof(Target_Spec));
				// Get the fdisk partition type info.
				disk_spec[*count].name[length] = 0;
				strcat(disk_spec[*count].name, " [ ");
				switch (ip->systid) {
				case DOSOS12:
					strcat(disk_spec[*count].name, "dosos12");
					break;
				case PCIXOS:
					strcat(disk_spec[*count].name, "pcixos");
					break;
				case DOSDATA:
					strcat(disk_spec[*count].name, "dosdata");
					break;
				case DOSOS16:
					strcat(disk_spec[*count].name, "dosos16");
					break;
				case EXTDOS:
					strcat(disk_spec[*count].name, "extdos");
					break;
				case DOSHUGE:
					strcat(disk_spec[*count].name, "doshuge");
					break;
				case OTHEROS:
					strcat(disk_spec[*count].name, "otheros");
					break;
				case UNIXOS:
					strcat(disk_spec[*count].name, "unixos");
					break;
				case UNUSED:
					strcat(disk_spec[*count].name, "unused");
					break;
// NOTE: This might be to generic, maybe we need an seperate global define
//       for this stuff. Current knowledge is include PPCBOOT for Sparc
//       build and exclude it for i386 build (because it is not defined
//       within /usr/include/sys/dktp/fdisk.h).
#if defined(IOMTR_CPU_I386)
					// nop
#elif defined(IOMTR_CPU_SPARC)
				case PPCBOOT:
					strcat(disk_spec[*count].name, "ppcboot");
					break;
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
				case SUNIXOS:
					strcat(disk_spec[*count].name, "sunixos");
					break;
				case X86BOOT:
					strcat(disk_spec[*count].name, "x86boot");
					break;
				case MAXDOS:
					strcat(disk_spec[*count].name, "maxdos");
					break;
				default:
					strcat(disk_spec[*count].name, "unknown");
					break;
				}
				if (fstype[0] != 0) {
					strcat(disk_spec[*count].name, ", ");
					strcat(disk_spec[*count].name, fstype);
				}
				strcat(disk_spec[*count].name, " ]");

				d.spec.type = PhysicalDiskType;
#ifdef _DEBUG
				cout << "   Found " << disk_spec[*count].name << "." << endl << flush;
#endif
				(*count)++;
				FDISK_table_valid = TRUE;
				if (*count >= MAX_TARGETS)
					break;
			} else {
				// partition access failed.                             
#ifdef _DEBUG
				cout << "   Not found " << file_name << endl << flush;
#endif
			}
		}
		// Done with this disk. Reset the bool flag has_solaris_parts
		has_solaris_parts = FALSE;
		if (VTOC_valid)
			return (TRUE);

		if (FDISK_table_valid)
			return (TRUE);
		else
			return (FALSE);
	}
	return (FALSE);		// a dummy
}
#endif				// IOMTR_CPU_I386 || IOMTR_CPU_IA64

BOOL Manager::Report_VTOC_Partitions(char *name, Target_Spec * disk_spec, int *count, int logical_count)
{
	TargetDisk d;
	int i, j, fd, length;

//  int bytes_read;
	struct vtoc this_vtoc;
	char fstype[32];	// to hold the file system type.
	char base_name[MAX_NAME];
	char file_name[MAX_NAME];
	BOOL vtoc_has_only_backup_slice = TRUE;

	memset(base_name, 0, MAX_NAME);
	memset(file_name, 0, MAX_NAME);

	snprintf(file_name, MAX_NAME, "%s/%s", RAW_DEVICE_DIR, name);
#ifdef _DEBUG
	cout << "   Reporting vtoc partitions: " << file_name << endl << flush;
#endif
	fd = open(file_name, O_RDWR | O_LARGEFILE, S_IRUSR | S_IWUSR);
	if (fd < 0) {
#ifdef _DEBUG
		cout << "  open failed.." << endl << flush;
#endif
		return (TRUE);
	}

	if (ioctl(fd, DKIOCGVTOC, &this_vtoc) < 0) {
		// Get VTOC failed. Something wrong here.
		// Ignore all slices in this partition.
		close(fd);
		return (TRUE);
	}
	close(fd);

	if (this_vtoc.v_sanity != VTOC_SANE) {
		// this vtoc is insane. report TRUE to ignore this partition 
		// and all slices within.
		return (TRUE);
	}

	strncpy(base_name, name, (strstr(name, "s2") - name));
#ifdef _DEBUG
	cout << "  vtoc basename: " << base_name << endl << flush;
#endif

	for (i = 0; i < this_vtoc.v_nparts; i++) {
		if (this_vtoc.v_part[i].p_size <= 3)
			continue;

		//
		// Note that we have commented out the check for V_BACKUP.
		// The V_BACKUP denotes slice 2 which by convention/default/whatever
		// denotes the whole raw disk.
		// On the sparc platform, when we write a default label to
		// the disk, three default slices are created BOOT, BACKUP and ALTSCTR.
		// While we dont test on BOOT and ALTSCTR, we should be able to work
		// on the BACKUP (slice 2) slice on the disk which is otherwise fully
		// empty.
		// So, we report BACKUP slices for testing. This move does not put
		// disk data in harm's way simply because if the disk has some data
		// say somefilesystem in it, it is not reported to Galileo.
		//
		if ((this_vtoc.v_part[i].p_tag == V_BOOT) ||
		    // (this_vtoc.v_part[i].p_tag == V_BACKUP) ||
		    (this_vtoc.v_part[i].p_tag == V_ALTSCTR))
			continue;

		// Having reached here, we now know that the vtoc has other valid
		// slices too.
		vtoc_has_only_backup_slice = FALSE;

		if (this_vtoc.v_part[i].p_flag == V_RONLY)
			continue;

		// Next check if this vtoc partition overlaps with any of the
		// subsequent parts.
		BOOL overlap = FALSE;

		for (j = 0; j < this_vtoc.v_nparts; j++) {
			if (i == j)
				continue;
			switch (this_vtoc.v_part[j].p_tag) {
				//
				// We ignore V_BOOT AND V_ALTSCTR slices. The V_BACKUP
				// slice is going to overlap with every other slice, so...
			case V_BOOT:
				continue;
			case V_BACKUP:
				break;
			case V_ALTSCTR:
				continue;
			default:
				break;
			}

			//
			// algorithm to determine overlapping partitions.
			//
			// if (start[i] < start[j])
			//              if (end[i] > start[j])
			//                      overlap = TRUE;
			// if (start[i] > start[j])
			//              if (start[i] < end[j])
			//                      overlap = TRUE;
			//
			if ((this_vtoc.v_part[i].p_start < this_vtoc.v_part[j].p_start) &&
			    ((this_vtoc.v_part[i].p_start + this_vtoc.v_part[i].p_size) > this_vtoc.v_part[j].p_start))
			{
				overlap = TRUE;
				break;
			}

			if ((this_vtoc.v_part[i].p_start > this_vtoc.v_part[j].p_start) &&
			    (this_vtoc.v_part[i].p_start < (this_vtoc.v_part[j].p_start + this_vtoc.v_part[j].p_size)))
			{
				overlap = TRUE;
				break;
			}
		}

		if (overlap == TRUE) {
			char temp[MAX_NAME];

			// Note that index i is the current slice and index j is the overlapping
			// slice.
			// check  if overlap is with a slice containing an unmounted file system.
			// We do not care about overlapping empty slices.
			// Also check if overlap is with a swap slice.
#ifdef _DEBUG
			cerr << "NOTE: Overlapping slices : " << i << ", " << j << endl;
#endif
			snprintf(temp, MAX_NAME, "%ss%d", base_name, j);
			if (Has_File_System(temp, fstype) == TRUE) {
				// this slice 'i' overlaps with an unmounted file system.
				continue;
			}
			if (strstr(swap_devices, temp)) {
				// this slice 'i' overlaps with a swap device.
				continue;
			}
			// else we are ok. overlapping empty slices is just fine.
		}
		// Here we do have VTOC parts to report.
		snprintf(file_name, MAX_NAME, "%ss%d", base_name, i);
#ifdef _DEBUG
		cout << "  vtoc  file_name: " << file_name << endl << flush;
#endif
		// check if this is a swap device. Search the global string.
		if (strstr(swap_devices, file_name))
			// Yes, this is a swap device, skip
			continue;

		if (d.Init_Physical(file_name)) {
			// Disk Initialization succeeded,
			if (Part_Reported_As_Logical(disk_spec, file_name, logical_count))
				continue;

			// Not mounted! Next check if there is a file system on it.
			if (Has_File_System(file_name, fstype) == TRUE) {
#if defined(IOMTR_SETTING_OVERRIDE_FS)
				if (getenv("IOMTR_SETTING_OVERRIDE_FS") != NULL) {
					cout << "WARNING: allowing raw access to unmounted fs: " << file_name << endl;
				} else	// environ variable not set
				{
#endif
					cout << "NOTICE: ignoring " << file_name << ". file system found on disk" <<
					    endl;
					this->is_destructive = TRUE;
					continue;
#if defined(IOMTR_SETTING_OVERRIDE_FS)
				}
#endif
			}
			// take away 10 bytes for the largest of vtoc part names and
			// and 5 bytes for the string elements. "[  ]"
			length = MAX_NAME - 10 - 5;

			// file opened successfully. But size might be all messed up
			// since we read the geometry of the disk.
			// Reset it to correct value.
			d.spec.disk_info.maximum_size = this_vtoc.v_part[i].p_size;
			d.Set_Size(this_vtoc.v_part[i].p_size);
			memcpy(&disk_spec[*count], &d.spec, sizeof(Target_Spec));

			// Get the vtoc partition type info.
			disk_spec[*count].name[length] = 0;
			strcat(disk_spec[*count].name, " [ ");
			switch (this_vtoc.v_part[i].p_tag) {
			case V_UNASSIGNED:
				strcat(disk_spec[*count].name, "unassigned");
				break;
			case V_BOOT:
				strcat(disk_spec[*count].name, "boot");
				break;
			case V_ROOT:
				strcat(disk_spec[*count].name, "root");
				break;
			case V_SWAP:
				strcat(disk_spec[*count].name, "swap");
				break;
			case V_USR:
				strcat(disk_spec[*count].name, "usr");
				break;
			case V_BACKUP:
				strcat(disk_spec[*count].name, "backup");
				break;
			case V_STAND:
				strcat(disk_spec[*count].name, "stand");
				break;
			case V_VAR:
				strcat(disk_spec[*count].name, "var");
				break;
			case V_HOME:
				strcat(disk_spec[*count].name, "home");
				break;
			case V_ALTSCTR:
				strcat(disk_spec[*count].name, "altsctr");
				break;
			case V_CACHE:
				strcat(disk_spec[*count].name, "cache");
				break;
			default:
				strcat(disk_spec[*count].name, "unknown");
				break;
			}

			if (fstype[0] != 0) {
				strcat(disk_spec[*count].name, ", ");
				strcat(disk_spec[*count].name, fstype);
			}

			strcat(disk_spec[*count].name, " ]");

			d.spec.type = PhysicalDiskType;
#ifdef _DEBUG
			cout << "   Found " << disk_spec[*count].name << "." << endl << flush;
#endif
			(*count)++;
			if (*count >= MAX_TARGETS)
				break;
		}
	}

	if (vtoc_has_only_backup_slice == TRUE) {
		// No VTOC slices qualified. We only have slices less than 3 sectors and backup
		// slices.
		// Report backup slice i.e whole disk.
		// It is beyond our control if backup slice does not span the whole disk
		// however, the format utility does warn the admin if that is the case.
		// NOTE: EXTRA LOOP TO GRAB WHOLE SLICE if nothing else left.
		for (i = 0; i < this_vtoc.v_nparts; i++) {
			if (this_vtoc.v_part[i].p_tag == V_BACKUP) {
				snprintf(file_name, MAX_NAME, "%ss%d", base_name, i);
				if (d.Init_Physical(file_name)) {
					if (d.Init_Physical(file_name))
						// Disk Initialization succeeded,
						if (Part_Reported_As_Logical(disk_spec, file_name, logical_count))
							continue;

					// Not mounted! Next check if there is a file system on it.
					if (Has_File_System(file_name, fstype) == TRUE) {
#if defined(IOMTR_SETTING_OVERRIDE_FS)
						if (getenv("IOMTR_SETTING_OVERRIDE_FS") != NULL) {
							cout << "WARNING: allowing raw access to unmounted fs: " <<
							    file_name << endl;
						} else	// environ variable not set
						{
#endif
							cout << "NOTICE: ignoring " << file_name <<
							    ". file system found on disk" << endl;
							this->is_destructive = TRUE;
							continue;
#if defined(IOMTR_SETTING_OVERRIDE_FS)
						}
#endif
					}
					// take away 10 bytes for the largest of vtoc part names and
					// and 5 bytes for the string elements. "[  ]"
					length = MAX_NAME - 10 - 5;

					// file opened successfully. But size might be all messed up
					// since we read the geometry of the disk.
					// Reset it to correct value.
					d.spec.disk_info.maximum_size = this_vtoc.v_part[i].p_size;
					d.Set_Size(this_vtoc.v_part[i].p_size);
					memcpy(&disk_spec[*count], &d.spec, sizeof(Target_Spec));

					// Get the vtoc partition type info.
					disk_spec[*count].name[length] = 0;
					strcat(disk_spec[*count].name, " [ ");
					strcat(disk_spec[*count].name, "backup");
					if (fstype[0] != 0) {
						strcat(disk_spec[*count].name, ", ");
						strcat(disk_spec[*count].name, fstype);
					}
					strcat(disk_spec[*count].name, " ]");

					d.spec.type = PhysicalDiskType;
#ifdef _DEBUG
					cout << "   Found " << disk_spec[*count].name << "." << endl << flush;
#endif
					(*count)++;
					if (*count >= MAX_TARGETS)
						break;
				}
			}
		}
	}
	return (TRUE);
}

BOOL Manager::Has_File_System(char *file_name, char *fstype)
{
// int pid, status;
	int retval;
	FILE *pptr;
	char path[MAX_NAME];
	char cmd[MAX_NAME];
	char buf[BUFSIZ];

	// Initialize
	fstype[0] = 0;
	snprintf(path, MAX_NAME, "/dev/rdsk/%s", file_name);
	snprintf(cmd, MAX_NAME, "/usr/sbin/fstyp %s 2>/dev/null", path);
	if ((pptr = popen(cmd, "r")) != NULL) {
		// popen succeeded.
		fgets(buf, BUFSIZ, pptr);

		// buf probably has the file system type.
		// We have a problem here. fstyp returns 0 or 2 for matches and
		// 1 for no matches. However, pclose() returns 0 for success and
		// -1 for failure.
		// fork1() + exec(), on the other hand, return values correctly
		// but then how do we get the file system type, if it exists ?
		retval = pclose(pptr);
		if (retval == 0) {
			strcpy(fstype, buf);
			return (TRUE);
		} else
			return (FALSE);
	}
	return (FALSE);
}

BOOL Manager::Part_Reported_As_Logical(Target_Spec * spec, char *rdisk, int count)
{
	int i, retval;
	char *p;
	char rstr[MAX_NAME], lstr[MAX_NAME];	// for physical/logical disks.
	FILE *fp;
	struct mnttab mtab, mpref;

	//
	//              checks if the named partition/slice has already been mounted.
	// The file systems listed in the mnttab file are already mounted. So, we have two cases :
	// The file system is listed for testing (eg: ufs) or not listed (eg. procfs)
	// But we need only check if the file sys is mounted and thats sufficient. That will
	// ensure that we don't crash anything running.
	//
	strcpy(rstr, rdisk);

	if ((fp = fopen(mnttab, "r")) == NULL) {
		cout << "open (mount tab) file " << mnttab << " failed with error " << errno << endl;
		cout << "Set environment variable MNTTAB to correct pathname" << endl;
		// We wont try to report any disk. Could expose the OS partition to destructive tests.
		return TRUE;
	}

	for (i = 0; i < count; i++) {
		// Initialize the mpref structure to NULL. Solaris does'nt do it.
		memset(&mpref, 0, sizeof(struct mnttab));
#ifdef _DEBUG
		cout << "checking if physical disk already reported as logical." << endl
		    << "	logical disk : " << spec[i].name << " physical disk : " << rdisk << endl;
#endif
		// check for this pattern is also in Manager::Report_Disks()
		// and TargetDisk::Init_Logical().
		p = strstr(spec[i].name, " [");
		strncpy(lstr, spec[i].name, p - spec[i].name);
		lstr[p - spec[i].name] = 0;

		mpref.mnt_mountp = lstr;
		if ((retval = getmntany(fp, &mtab, &mpref)) == 0) {
			// found the entry in mnttab.
			if (strstr(mtab.mnt_special, rstr)) {
				// equal! the entry contains this physical disk name (in the form c0t0d0[p|s]?).
				// further the entry is in the list of reported logical disks.
				// So, this physical disk has already been reported as a logical disk.
#ifdef _DEBUG
				cout << "physical disk " << rdisk << " reported logical disk "
				    << mtab.mnt_mountp << endl;
#endif
				fclose(fp);
				return TRUE;
			}
		}
	}
	fclose(fp);
	return FALSE;
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

// Function to get all swap device names into a single string of the form
// <string1>:<string2>:<string3> and so on.
// It is easier to lookup if in this form.
//
void Manager::Get_All_Swap_Devices()
{
	// Presettings

	int num, i, n;
	swaptbl_t *pSwapTable;
	char *pcSwapNames;

	// Loop till list of swap devices is created

	for (;;) {

		// Get the number of swap devices
		// (if non is found or error occurred - simply return)

		if ((num = swapctl(SC_GETNSWP, 0)) <= 0) {
			return;
		}
		// Allocate the needed temporary memory structures

		pSwapTable = (swaptbl_t *) malloc(((num + 1) * sizeof(swapent_t)) + sizeof(struct swaptable));
		if (pSwapTable == NULL) {
			return;
		}
		pcSwapNames = (char *)malloc((num + 1) * MAX_NAME);
		if (pcSwapNames == NULL) {
			free(pSwapTable);
			return;
		}
		// Initialize string pointers in the swaptable

		for (i = 0; i < (num + 1); i++) {
			pSwapTable->swt_ent[i].ste_path = pcSwapNames + (i * MAX_NAME);
		}
		pSwapTable->swt_n = num + 1;

		// Request filling the swaptable

		if ((n = swapctl(SC_LIST, pSwapTable)) < 0) {
			cerr << "WARN: swapctl failed with error " << errno << endl;
			free(pSwapTable);
			free(pcSwapNames);
			return;
		}
		// Ensure that the swaptable was big enough for the data

		if (n > num) {
			free(pSwapTable);
			free(pcSwapNames);
			continue;
		}
		// Allocate the memory for the final destination of the swap device list
		// (n * swap_device_name) + (n * ":") + 1

		swap_devices = (char *)malloc((n * MAX_NAME) + n + 1);
		if (swap_devices == NULL) {
			free(pSwapTable);
			free(pcSwapNames);
			return;
		}

		for (i = 0; i < n; i++) {
			strcat(swap_devices, pSwapTable->swt_ent[i].ste_path);
			strcat(swap_devices, ":");
		}
#ifdef _DEBUG
		cout << "swap devices: " << swap_devices << endl;
#endif
		free(pSwapTable);
		free(pcSwapNames);
		return;
	}
}

#endif				// IOMTR_OS_SOLARIS
