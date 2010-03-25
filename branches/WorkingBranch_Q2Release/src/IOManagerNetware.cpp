/* ######################################################################### */
/* ##                                                                     ## *
/* ##  Dynamo / IOManagerNetware.cpp                                      ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: This file contains the NetWare related methods of     ## */
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
/* ##  Changes ...: 2005-04-07 (thayneharmon@users.sourceforge.net)       ## */
/* ##               - Adjusted some print output.                         ## */
/* ##               2004-09-26 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Removed a note about the Reported_As_Logical()      ## */
/* ##                 function as this one was removed.                   ## */
/* ##               2004-07-02 (thayneharmon@users.sourceforge.net)       ## */
/* ##               - fixed error in collecting volumes where the         ## */
/* ##                 netware_vol_info() automacticlly increments the     ## */
/* ##                 sequence						  ## */
/* ##               2004-01-01 (thayneharmon@users.sourceforge.net)       ## */
/* ##               - Initial code.                                       ## */
/* ##                                                                     ## */
/* ######################################################################### */
#if defined(IOMTR_OS_NETWARE)

#include "IOManager.h"
#include "IOTargetDisk.h"
#include <dirent.h>
#include <ctype.h>
#include <monitor.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>

/**********************************************************************
 * Forward Declarations
 **********************************************************************/
static int compareRawDiskNames(const void *a, const void *b);
void MM_CallBackRoutine(LONG requestHandle, LONG applicationRequestToken, LONG returnParameter, LONG completionCode);

//
// Checking for all accessible drives.  Storing them, and returning the number
// of disks found.  Drives are reported in order so that Iometer does not
// need to sort them.
//
int Manager::Report_Disks(Target_Spec * disk_spec)
{
	TargetDisk d;
	int count = 0;
	struct volume_info vInfo;
	struct IOObjectGenericInfoDef info;
	int length, volNum;
	DWORD ret = 0;
	DWORD next;
	char disk_name[MAX_NAME];

#define MM_DIRECT_ACCESS_DEVICE 0
	cout << "Reporting drive information..." << endl;
	// *********************************************************************************
	// DEVELOPER NOTES
	// ---------------
	//
	// For NetWare
	//
	// **********************************************************************************

	//
	// First find all virtual disks by returning volumes. Then search for
	// physical disks using Media Manager.
	//
	cout << "  Logical drives (volumes)..." << endl;
	for (volNum = 0, ret = 0; ret == 0;)	// volNum++
	{
		ret = netware_vol_info(&vInfo, &volNum);	// volNum is sequenced by the call - do not touch

		if (strstr("_ADMIN", (char *)vInfo.name) != NULL)
			continue;
		if (vInfo.mounted != TRUE)
			continue;

		cout << "    Volume [" << vInfo.which << "] \"" << vInfo.name << "\" found.\n";
		// see if the current volName is an excluded for dynamo.
		if (strstr(exclude_filesys, (char *)vInfo.name) != NULL) {
			cout << "    Volume [" << vInfo.which << "] \"" << vInfo.name << "\" excluded.\n";
			continue;
		}
		length = strlen((char *)vInfo.name);
		strncpy(disk_name, (char *)vInfo.name, length);
		disk_name[length] = 0;
		if (!d.Init_Logical(disk_name)) {
			cout << "    * " << __FUNCTION__ << ": Init_Logical failed (RO?).\n";
			continue;
		}
		// Drive exists and ready for use.
		d.spec.type = LogicalDiskType;
		memcpy(&disk_spec[count], &d.spec, sizeof(Target_Spec));
		disk_spec[count].name[length] = 0;
		// check for this pattern is also in TargetDisk::Init_Logical().
//              strcat(disk_spec[count].name, " [");
//              strcat(disk_spec[count].name, "NSS");
//              strcat(disk_spec[count].name, "]");
		count++;
		if (count >= MAX_TARGETS)
			break;
	}
	if (count >= MAX_TARGETS)
		return count;
	// Now reporting physical drives (raw devices).
	cout << "  Physical drives (raw devices)..." << endl;
	for (next = -1; (MM_FindObjectType(MM_IO_CLASS, MM_DEVICE_OBJECT, &next) != MM_OBJECT_NOT_FOUND);) {
		ret = MM_ReturnObjectGenericInfo(next, sizeof(struct IOObjectGenericInfoDef), &info);
		if (!(info.status & MM_IOOBJECT_FROM_NWPA))
			continue;
		if (info.removableinfo.mediatype != MM_DIRECT_ACCESS_DEVICE)
			continue;
		if (strstr((char *)info.name, "Unbound"))
			continue;
		if (strstr((char *)info.name, "Floppy"))
			continue;
		if ((ret = d.NWOpenDevice(next, 1)) != -1)
			d.NWCloseDevice(ret);
		else
			continue;
#ifdef _DEBUG
		cout << __FUNCTION__ << ": Found device " << info.name << "\n";
#endif
		if ((info.status & MM_IOOBJECT_RESERVED) == 0) {	// If it can be reserved then add it to our list of physical devices.
#ifdef _DEBUG
			cout << __FUNCTION__ << ": Device is not reserved.\n";
#endif
			snprintf(disk_name, MAX_NAME, "[%d]", next);
			if (d.Init_Physical(disk_name)) {
				d.spec.type = PhysicalDiskType;
				memcpy(&disk_spec[count], &d.spec, sizeof(Target_Spec));
#ifdef _DEBUG
				cout << "name=" << disk_spec[count].name << ",type=" << disk_spec[count].
				    type << endl << flush;
#endif
				++count;
			}
		}
#ifdef _DEBUG
		else {
			cout << __FUNCTION__ << ": Device is locked. Ignoring.\n";
		}
#endif
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

//
//
int Manager::Report_TCP(Target_Spec * tcp_spec)
{
	struct hostent *hostinfo;
	struct sockaddr_in sin;
	char hostname[128];
	int count = 0;
	int i;

	cout << "Reporting TCP network information..." << endl;
	// get the unqualified local host name
	if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) {
		cout << "*** Error " << WSAGetLastError() << "getting local host name.\n";
		return 0;
	}
	// now get the host info for that host name
	hostinfo = gethostbyname(hostname);
	if (hostinfo == NULL) {
		cout << "*** Error " << WSAGetLastError() << "getting host info for \"" << hostname << "\".\n";
		return 0;
	}
	cout << "   My hostname: \"" << hostinfo->h_name << "\"" << endl;
#ifdef _DEBUG
	i = 0;
	// this blows up - don't know why
	//   while ( hostinfo->h_aliases[i] != NULL )
	//   {
	//           printf("   Alias: \"%s\"\n", hostinfo->h_aliases[i]);
	//           i++;
	//   }
#endif
	// report the network addresses.
	for (i = 0; hostinfo->h_addr_list[i] != NULL; i++) {
		memcpy(&sin.sin_addr.s_addr, hostinfo->h_addr_list[i], hostinfo->h_length);
		strncpy(tcp_spec[count].name, inet_ntoa(sin.sin_addr), sizeof(tcp_spec[count].name) - 1);
		tcp_spec[count].type = TCPClientType;	// interface to access a client
		cout << "   Found " << tcp_spec[count].name << "." << endl;
		if (++count >= MAX_NUM_INTERFACES) {
			cout << "*** Found the maximum number of supported network interfaces: "
			    << endl << "Only returning the first " << MAX_NUM_INTERFACES << "." << endl;
			count = MAX_NUM_INTERFACES;
			break;
		}
	}
#if 0				// for debugging multiple-network-interface GUI; change "#if 0" to "#if 1" to enable
	strncpy(nets[count], "foo", sizeof(nets[count]) - 1);
	cout << "   Added fake entry " << nets[count] << "." << endl << flush;
	count++;
#endif
	// All done.
	cout << "   done." << endl;
	return count;
}
int aio_suspend64(struct aiocb64 **cb, int a, struct timespec *)
{
	pthread_t tid = pthread_self();

	// for (cd list; if any not in progress; return else block till one exist)
	//use semaphores to start and stop
	// suspend until an IO completes and restarts this thread - loop?
	return 0;
}

//
//
int aio_error64(struct aiocb64 *cb)
{
	return cb->error;
}

//
//
int aio_return64(struct aiocb64 *cb)
{
	return cb->returnval;
}

//
//
int aio_read64(struct aiocb64 *cb, int type)
{
	if (IsType(type, LogicalDiskType)) {
		cb->error =
		    NXRead(cb->aio_fildes, (NXOffset_t) cb->aio_offset, cb->aio_nbytes, cb->aio_buf, (unsigned long)0,
			   (size_t *) & cb->returnval);
		cb->completion_key = EBUSY;
	} else if (IsType(type, PhysicalDiskType)) {
		cb->error = MM_ObjectIO(&cb->completion_key, cb->aio_fildes, MM_RANDOM_READ, (cb->aio_nbytes / 512),
					(cb->aio_offset / 512), 0, cb->aio_nbytes, cb->aio_buf, (LONG) cb,
					(void (*)())MM_CallBackRoutine);
		cb->returnval = cb->aio_nbytes;
		if (cb->error == 0) {
			cb->error = EINPROGRESS;
			cb->completion_key = EBUSY;
		}
	}
	return (cb->error);
}

//
//
int aio_write64(struct aiocb64 *cb, int type)
{
	if (IsType(type, LogicalDiskType)) {
		cb->error =
		    NXWrite(cb->aio_fildes, (NXOffset_t) cb->aio_offset, cb->aio_nbytes, cb->aio_buf, (unsigned long)0,
			    (size_t *) & cb->returnval);
		cb->completion_key = EBUSY;
	} else if (IsType(type, PhysicalDiskType)) {
		cb->error = MM_ObjectIO(&cb->completion_key, cb->aio_fildes, MM_RANDOM_WRITE, (cb->aio_nbytes / 512),
					(cb->aio_offset / 512), 0, cb->aio_nbytes, cb->aio_buf, (LONG) cb,
					(void (*)())MM_CallBackRoutine);
		cb->returnval = cb->aio_nbytes;
		if (cb->error == 0) {
			cb->error = EINPROGRESS;
			cb->completion_key = EBUSY;
		}
	}
	return cb->error;
}

//
//
int aio_cancel64(int a, struct aiocb64 *cb)
{
	if (cb == NULL) {
		//printf("cancel all in queue\n");
	} else {
		printf("cancel one\n");
	}
	return FALSE;
}

//
//
void MM_CallBackRoutine(LONG requestHandle, LONG applicationRequestToken, LONG returnParameter, LONG completionCode)
{
	struct aiocb64 *cb = (struct aiocb64 *)applicationRequestToken;

	cb->error = completionCode;
}

int TargetDisk::NWOpenDevice(LONG device, LONG mode)
{
	LONG rc = 0;

	rc = MM_ReserveIOObject(&reservationHandle, device,
				(mode ? MM_IO_MODE : (MM_IO_MODE | MM_SHARED_PRIMARY_RESERVE)), device, NWalertroutine,
				applicationHandle);
	if (rc)
		((struct File *)disk_file)->fd = -1L;
	else
		((struct File *)disk_file)->fd = (int)reservationHandle;
	((struct File *)disk_file)->type = PhysicalDiskType;
	if (rc)
		cout << "TargetDisk::Open: MM_ReserveIOObject rc=" << rc << " for MMID " << atoi(file_name) << endl;
	return ((struct File *)disk_file)->fd;
}

int TargetDisk::NWCloseDevice(HNDL handle)
{
	return MM_ReleaseIOObject(handle);
}

LONG NWalertroutine(unsigned long reservationHandle, unsigned long alertToken, unsigned long alertType,
		    unsigned long alertReasion)
{
	// drive alerts - i.e deactivation and so on
	return MM_SHARED_RESERVE_GRANTED;
}

#endif				// IOMTR_OS_NETWARE
