/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / Pulsar.cpp                                                ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: This is file contains is basically the interface      ## */
/* ##               for dynamo.  It handles parameter parsing, usage      ## */
/* ##               info and the actual main() method.                    ## */
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
/* ##               2005-04-18 (mingz@ele.uri.edu)                        ## */
/* ##               - Changed block device check to device check in       ## */
/* ##                 order to allow use of raw devices.                  ## */
/* ##               - Changed kstat related error message.                ## */
/* ##               2005-04-17 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Changed IOMTR_SET_AFFINITY to the new switch        ## */
/* ##                 named IOMTR_SETTING_CPU_AFFINITY.                   ## */
/* ##               - Restructured the Syntax() function structure.       ## */
/* ##               - Code cleanup.                                       ## */
/* ##               2005-04-06 (mingz@ele.uri.edu)                        ## */
/* ##               - Introduced a dynamo_param structure to simplify     ## */
/* ##                 the ParseParam() interface.                         ## */
/* ##               2005-04-05 (mingz@ele.uri.edu)                        ## */
/* ##               - Added set CPU affinity support. Disabled by default.## */
/* ##               2004-09-28 (mingz@ele.uri.edu)                        ## */
/* ##               - Added syslog support.                               ## */
/* ##               - Added warning for the common login fail error.      ## */
/* ##               2004-08-18 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Code cleanup to ensure common style.                ## */
/* ##               - Added some OS checks to make the block device       ## */
/* ##                 stuff work on non Linux/Solaris OS'es.              ## */
/* ##               2004-07-26 (mingz@ele.uri.edu)                        ## */
/* ##               - Added a new parameter to support user supplied      ## */
/* ##                 block device list.                                  ## */
/* ##               2004-04-19 (tharmon@novell.com)                       ## */
/* ##               - Extended the usage output for Netware to            ## */
/* ##                 list the switch for excluding volumes.              ## */
/* ##               2004-04-17 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added a check to enforce that -m has to be          ## */
/* ##                 specified if -i was given.                          ## */
/* ##               2004-03-27 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Code cleanup to ensure common style.                ## */
/* ##               - Applied Thayne Harmon's patch for supporting        ## */
/* ##                 Netware support (on I386).                          ## */
/* ##               2004-03-24 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Changed the "manager_network_name" in the usage     ## */
/* ##                 help to "manager_computer_name" to better reflect   ## */
/* ##                 the questioned field (as suggested by Tony Renna).  ## */
/* ##               2004-02-12 (mingz@ele.uri.edu)                        ## */
/* ##               - Added /dev/ccnt ioctl code for linux                ## */
/* ##                 on some Intel XScale architectures.                 ## */
/* ##               2004-02-07 (mingz@ele.uri.edu)                        ## */
/* ##               - Changed call from im_kstat to iomtr_kstat.          ## */
/* ##                 Daniel suggest to use this clearer name.            ## */
/* ##               2004-02-06 (mingz@ele.uri.edu)                        ## */
/* ##               - Added /proc/stat style detect code and invoke       ## */
/* ##                 it in main().                                       ## */
/* ##               - Added code to try to utilize the im_kstat module.   ## */
/* ##               2003-12-21 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Changed DYNAMO_DESTRUCTIVE to                       ## */
/* ##                 IOMTR_SETTING_OVERRIDE_FS                           ## */
/* ##               - Consolidated the ParseParam() method.               ## */
/* ##               - Disabled accepting parameters without switch.       ## */
/* ##               2003-08-02 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Integrated the modification contributed by          ## */
/* ##                 Vedran Degoricija, to get the code compile with     ## */
/* ##                 the MS DDK on IA64.                                 ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               2003-07-27 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Implemented a test call of the IsBigEndian()        ## */
/* ##                 function to ensure, that we are able to detect      ## */
/* ##                 the endian type of the CPU.                         ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-02-26 (joe@eiler.net)                            ## */
/* ##               - Added a command line option for passing the         ## */
/* ##                 list of filesystem types to ignore when building    ## */
/* ##                 the device list for Iometer).                       ## */
/* ##               2003-02-08 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Addition command line option for passing an         ## */
/* ##                 alternative network name to Iometer (which that     ## */
/* ##                 one is using to communicate with this Dynamo).      ## */
/* ##               2003-02-02 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Applied the iometer-initial-datatypes.patch file.   ## */
/* ##                 (changing the datatype of the "temp" variable in    ## */
/* ##                 the GetStatus(int*, int*, int) method).             ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include "IOCommon.h"
#include "IOManager.h"

extern timer_type TimerType;
#if defined(IOMTR_OSFAMILY_UNIX)
#include <sys/resource.h>
#include <ctype.h>
#if defined(IOMTR_OS_SOLARIS)
#include <synch.h>
#elif defined(IOMTR_OS_LINUX)
int kstatfd;
#elif defined(IOMTR_OS_OSX)
  // nop
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
#endif

#if defined(IOMTR_OS_NETWARE)
#include <ctype.h>
#endif

#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
int do_syslog = FALSE;
#endif


struct dynamo_param param;	// global scope so other modules have access to it
							// needed specifically for timer_type and disk_contorl

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////
static void Syntax(const char *errmsg = NULL);

static void ParseParam(int argc, char *argv[], struct dynamo_param *param);

static void Banner();

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

// Temporary global data for Syntax() to use
const char *g_pVersionStringWithDebug = NULL;

#if defined(IOMTR_OS_LINUX)
int InitIoctlInterface(void)
{
	int res;

	res = open("/dev/iomtr_kstat", O_RDONLY);
	if (res < 0) {
#if defined(IOMTR_CPU_XSCALE)
		cerr << "Failed to open kstat device file. You can ignore this warning" << endl;
		cerr << "unless you are running dynamo on XSCALE CPU." << endl;
#endif
	}
	return res;
}

void CleanupIoctlInterface(int fd)
{
	if (fd > 0)
		close(fd);
}

#if defined(IOMTR_CPU_XSCALE)

int ccntfd;

//
// Detect whether an embedded Linux kernel for XScale CPU like IOP321 and IOP331 has
// /proc/cpu/ccnt support. We use this CCNT to get rdtsc like value.
//
int InitCCNTInterface(void)
{
	int res;

	res = open("/dev/ccnt", O_RDONLY);
	if (res < 0) {
		cerr << "Fail to find CCNT interface." << endl;
		cerr << "Please compile your kernel with CCNT (cycle counter) support" << endl;
		cerr << "   and create corresponding /dev/ccnt node by ccntmknod script." << endl;
	}
	return res;
}

void CleanupCCNTInterface(int fd)
{
	if (fd > 0)
		close(fd);
}

#endif				// IOMTR_CPU_XSCALE
#endif				// IOMTR_OS_LINUX

#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
int check_dev(char *devname)
{
	struct stat buf;

	if (strlen(devname) >= MAX_NAME) {
		return -1;
	}
	if (stat(devname, &buf)) {
		return -1;
	}
	if (!S_ISBLK(buf.st_mode) && !S_ISCHR(buf.st_mode)) {
		return -1;
	}
	return 0;
}
#elif defined(IOMTR_OS_NETWARE) || defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	// nop  
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

#if defined(IOMTR_SETTING_CPU_AFFINITY)
static int iomtr_set_cpu_affinity(ULONG_PTR affinity_mask)
{
	int res = -1;

#if defined(IOMTR_OS_LINUX)
	cpu_set_t s;
	int i, j;
	unsigned long t = 1;

	if (!affinity_mask) {
		//affinity_mask = 1;
		return 0;
	}
	i = sizeof(unsigned long) * 8;
	if (i > CPU_SETSIZE) {
		i = CPU_SETSIZE;
	}
	CPU_ZERO(&s);
	for (j = 0; j < i; j++)
		if ((t << j) & affinity_mask) {
			CPU_SET(j, &s);
			break;
		}
	/* Developer Note:
	 *
	 * If meet compiling error about following line in a relatively old Linux system,
	 * please try "sched_setaffinity(getpid(), &s)" instead.
	 */
	if (sched_setaffinity(getpid(), CPU_SETSIZE, &s) < 0) {
		cout << "Set CPU affinity fail." << errno << endl;
		if (errno == ENOSYS) {
			cout << "Warning: System does not support set CPU affinity." << endl;
			cout << "Be sure to check if measured results are reasonable." << endl;
		}
	} else {
#ifdef _DEBUG
		cout << "Set CPU affinity sucessfully." << endl;
#endif
	}

#elif defined(IOMTR_OSFAMILY_WINDOWS)
	DWORD_PTR processAffinity, systemAffinity;

	if (!affinity_mask)
	{
		return 0;
	}

	res = GetProcessAffinityMask(GetCurrentProcess(), &processAffinity,  &systemAffinity);

	if (!res) 
	{
		res = GetLastError();
		cout << "Could not obtain process affinity. GetProcessAffinityMask() failed with error=" 
			 << GetLastError() << endl;
		
		return 0;
	}

	// Set affinity only if it is different than current and 
	// if it is a subset of the system affinity mask
	if (affinity_mask != processAffinity && ((affinity_mask & systemAffinity) == affinity_mask))
	{
		res = SetProcessAffinityMask(GetCurrentProcess(), affinity_mask);
		if (!res) 
		{
			res = GetLastError();
			cout << "Set cpu affinity failed with error=" << GetLastError() << endl;
			return 0;
		}
		else 
		{
#ifdef _DEBUG
			cout << "Set CPU affinity sucessfully." << endl;
#endif
		}
	}
	else
	{
		cout << "Warning: ignoring affinity mask due to redundant or invalid value." << endl;
	}

#elif defined(IOMTR_OS_NETWARE) || defined(IOMTR_OS_SOLARIS)
	// nop  
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	return res;
}
#else
static int iomtr_set_cpu_affinity(unsigned long affinity_mask)
{
	return 0;
}
#endif

/* ######################################################################### */
/* ##                                                                     ## */
/* ##   M A I N ( )                                                       ## */
/* ##                                                                     ## */
/* ######################################################################### */

int CDECL main(int argc, char *argv[])
{
	Manager *manager;
	char iometer[MAX_NETWORK_NAME];
	int error = 0;
	// struct dynamo_param param; //move up to global scope

#if defined(IOMTR_OS_LINUX)
	struct aioinit aioDefaults;

	memset(&aioDefaults, 0, sizeof(aioDefaults));
	aioDefaults.aio_threads = 2;
	aioDefaults.aio_threads = 2;
	aio_init(&aioDefaults);
	kstatfd = InitIoctlInterface();
#if defined(IOMTR_CPU_XSCALE)
	if ((ccntfd = InitCCNTInterface()) < 0) {
		exit(1);
	}
#endif
#endif

	Banner();

#if !defined(DO_NOT_PARSE_BEFORE_MANAGER)

	// In order to allow command line parameters to influence default values of
	// the Manager class members, we need to parse parameters before instantiating
	// the Manager, but to do this, we need to:

	// Setup local storage -- could just be in the global param structure???
	char blkdevlist[MAX_TARGETS][MAX_NAME];
	char manager_name[MAX_WORKER_NAME];
	char network_name[MAX_NETWORK_NAME];
	char exclude_filesys[MAX_EXCLUDE_FILESYS];

	// Init the local storage to match the original code
	iometer[0] = 0;
	manager_name[0] = 0;
	exclude_filesys[0] = 0;
	network_name[0] = 0;

	// Setup the param structure to defaults and to point to buffers above
	param.iometer = iometer;
	param.manager_name = manager_name;
	param.manager_computer_name = network_name;
	param.manager_exclude_fs = exclude_filesys;
	param.blkdevlist = &blkdevlist;
	param.login_port_number = 0;
	param.cpu_affinity = 0; // not specified or default
	param.timer_type = TIMER_UNDEFINED; // use the default
	param.disk_control = RAWDISK_VIEW_NOPART; // do not show raw disks with partitions

	// The manager's GetVersionString method is not available yet since it does not exist, 
	// so we do away with the variable and have ParseParam rely directly on the source of 
	// the strings in ioversion.h. Not too clean but functional...
	// g_pVersionStringWithDebug = NULL; // not needed

	// Parse params and then instantiate the manager next...
	ParseParam(argc, argv, &param);

#endif

	manager = new Manager;

#if !defined(DO_NOT_PARSE_BEFORE_MANAGER)
	// Restore the param globals retrieved above back to the manager
	// since the manager buffers were not available prior to the parse call.
	memcpy(manager->manager_name, manager_name, sizeof(manager_name));
	memcpy(manager->prt->network_name, network_name, sizeof(network_name));
	memcpy(manager->exclude_filesys, exclude_filesys, sizeof(exclude_filesys));
	memcpy(manager->blkdevlist, blkdevlist, sizeof(blkdevlist));

#else // defined(DO_NOT_PARSE_BEFORE_MANAGER) // the original code
	iometer[0] = 0;
	manager->manager_name[0] = 0;
	manager->exclude_filesys[0] = 0;

	//provide a temporary global ptr to the version string for Syntax() to use
	g_pVersionStringWithDebug = manager->GetVersionString(TRUE);

	param.iometer = iometer;
	param.manager_name = manager->manager_name;
	param.manager_computer_name = manager->prt->network_name;
	param.manager_exclude_fs = manager->exclude_filesys;
	param.blkdevlist = &manager->blkdevlist;
	param.login_port_number = 0;
	param.cpu_affinity = 0; // not specified or default
	param.timer_type = TIMER_UNDEFINED; // use the default
	param.disk_control = RAWDISK_VIEW_NOPART; // do not show raw disks with partitions

	ParseParam(argc, argv, &param);

	g_pVersionStringWithDebug = NULL;	//should use manager object after this...

#endif
	iomtr_set_cpu_affinity(param.cpu_affinity);

	// If there were command line parameters, indicate that they were recognized.
	if (iometer[0] || manager->manager_name[0]) {
		cout << "\nCommand line parameter(s):" << endl;

		if (iometer[0]) {
			cout << "   Looking for Iometer on \"" << iometer << "\"" << endl;
		}
		if (manager->manager_name[0]) {
			cout << "   New manager name is \"" << manager->manager_name << "\"" << endl;
		}
	}
	if (manager->exclude_filesys[0]) {
		cout << "\nExcluding the following filesystem types:" << endl;
		cout << "   \"" << manager->exclude_filesys << "\"" << endl;
	} else {
		strcpy(manager->exclude_filesys, DEFAULT_EXCLUDE_FILESYS);
	}
	cout << endl;

#if defined(IOMTR_OSFAMILY_UNIX)
#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_OSX)
	signal(SIGALRM, SIG_IGN);
#elif defined(IOMTR_OS_SOLARIS)
	sigignore(SIGALRM);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

	// Initialize the lock on UNIX platforms.
	if (pthread_mutex_init(&lock_mt, NULL)) {
		cout << "unable to init the lock" << endl;
		error = 1;
		goto CleanUp;
		//exit(1);
	}
	// Block SIGPIPE signal. Needed to ensure that Network worker
	// threads don't exit due to a broken pipe signal.
	sigset_t sigset;

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGPIPE);
	if (sigprocmask(SIG_BLOCK, &sigset, NULL) < 0) {
		cout << "sigprocmask() call failed." << endl;
		cout << "dynamo could be unstable" << endl;
	}
	//
	// the number of file descriptors a process may create can be a small value like 64.
	//
	struct rlimit rlimitp;

	if (getrlimit(RLIMIT_NOFILE, &rlimitp) < 0) {
		cout << "error " << errno << " trying to get rlimit (# file descriptors)" << endl;
	} else {
		// it succeeded. We leave out atleast 25 file descriptors for non-targets
		// and compare with the hard limit.
		unsigned int targets = MAX_TARGETS + 25;

		if (rlimitp.rlim_max < targets) {
			cout << "Only " << rlimitp.rlim_max << " file descriptors available" << endl;
			rlimitp.rlim_cur = rlimitp.rlim_max;
		} else {
			// set the soft limit to the required value.
			rlimitp.rlim_cur = targets;
		}
		if (setrlimit(RLIMIT_NOFILE, &rlimitp) < 0) {
			cout << "error " << errno << " trying to set rlimit (# file descriptors)" << endl;
		}
	}

	// Check for super-user permissions. If not super-user, we
	// cannot get many of the info from the kernel.
	if (getuid() || geteuid()) {
		cout << "Dynamo not running as super-user." << endl;
		cout << "       All available disks might not be reported " << endl;
		cout << "       Cannot get TCP statistics from the kernel " << endl;
	}
#ifdef IOMTR_SETTING_OVERRIDE_FS
	// No command line args specifies destructive testing. Check to see if there
	// are any environment variables specifying the same. We need to warn the user.
	if (getenv("IOMTR_SETTING_OVERRIDE_FS") != NULL) {
		cout << "       ************ WARNING **************" << endl;
		cout << "       dynamo running in Destructive mode." << endl;
		cout << "         (overriding the not mounted fs)" << endl;
		cout << "       ************ WARNING **************" << endl;
	}
#endif				// IOMTR_SETTING_OVERRIDE_FS
#endif				// IOMTR_OSFAMILY_UNIX
#if defined(IOMTR_OSFAMILY_NETWARE)
	// Initialize the lock on NetWare platforms.
	if (pthread_mutex_init(&lock_mt, NULL)) {
		cout << "unable to init the lock" << endl;
		error = 1;
		goto CleanUp;
		//exit(1);
	}
#endif

#if defined(IOMTR_OSFAMILY_WINDOWS)
	// IOmeter/Dynamo now utilizes Windows UAC for privilege elevation,
	// but on version of Windows in which that is not supported we
	// match the UNIX output above.

	BOOL bReturned;
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	PSID AdminGroup; 

	bReturned = AllocateAndInitializeSid(
		&NtAuthority,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&AdminGroup); 

	if(bReturned) 
	{
		CheckTokenMembership( NULL, AdminGroup, &bReturned);

		if (!bReturned)
		{
			cout << "Dynamo not running as an administrators." << endl;
			cout << "       All available disks might not be reported " << endl;
			//cout << "       Cannot get TCP statistics from the kernel " << endl;
			cout << endl;
		} 
		FreeSid(AdminGroup); 
	}

#endif

	// Ensure, that the endian type of the CPU is detectable
	if ((IsBigEndian() != 0) && (IsBigEndian() != 1)) {
		cout << "===> ERROR: Endian type of the CPU couldn't be detected." << endl;
		cout << "     [main() in " << __FILE__ << " line " << __LINE__ << "]" << endl;
		error = 1;
		goto CleanUp;
		//exit(1);
	}
	// Entering infinite loop to allow Dynamo to run multiple tests.  Outer while loop allows
	// Dynamo to be reset from Iometer.  If everything works smoothly, resets should be rare.
	while (TRUE) {
		// Initializing worker and logging into Iometer director.
		if (!manager->Login(iometer, param.login_port_number))
			break;

		// Manager will continue to run until an error, or stopped by Iometer.
		if (!manager->Run())
			break;	// Stop running when the manager is done.
	}
	cout << "Ending execution." << endl;
	Sleep(1000);

#if defined(IOMTR_OS_LINUX)
	CleanupIoctlInterface(kstatfd);
#if defined(IOMTR_CPU_XSCALE)
	CleanupCCNTInterface(ccntfd);
#endif
#endif

	//return (0);

CleanUp:
	if (manager) delete manager;
	if (error) exit(error);
	return (0);
}

void Banner()
{
	//cout << "Version " << g_pVersionStringWithDebug << endl;
	cout << "Dynamo version " << IOVER_FILEVERSION << VERSION_DEBUG ; 

#if (defined(IOMTR_OSFAMILY_WINDOWS) ||  defined(IOMTR_OS_LINUX))
  #if defined(IOMTR_CPU_I386)		//#if defined(_M_IX86)
	cout << ", Intel x86 32bit";
  #elif defined(IOMTR_CPU_IA64)		//#elif defined(_M_IA64)
	cout << ", Intel Itanium 64bit";
  #elif defined(IOMTR_CPU_X86_64)	//#elif defined(_M_X64)
	cout << ", Intel/AMD x64 64bit";
 #endif

#elif defined(IOMTR_OS_LINUX)
 #if defined(IOMTR_CPU_XSCALE) 
	cout << ", Intel XScale";
 #endif
#elif defined(IOMTR_OS_OSX)
 #if defined(__ppc__)
	cout << ", PowerPC 32bit";
 #elif defined(__ppc64__)
	cout << ", PowerPC 64bit";
 #elif defined(__i386__)
	cout << ", Intel x86 32bit";
 #elif defined(__x86_64__)
	cout << ", Intel x86_64 64bit";
 #else
	cout << ", unknown architecture and bitness";
 #endif
  
 #if defined(__BIG__ENDIAN__)
	cout << ", big-endian";
 #endif
#endif 

	// cout << endl;
	cout << ", built " << __DATE__ << " " << __TIME__ << endl;
	cout << endl;
}


/* ######################################################################### */
/* ##                                                                     ## */
/* ##   S Y N T A X ( )                                                   ## */
/* ##                                                                     ## */
/* ######################################################################### */

void Syntax(const char *errmsg /*=NULL*/ )
{
	if (errmsg) {
		cout << endl << "*** Error processing the command line." << endl;
		cout << "*** " << errmsg << endl;
	}

	cout << endl;

	//Banner();

	cout << "SYNTAX" << endl;
	cout << endl;

#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_OSX)
	cout << "dynamo -?" << endl;
#elif defined(IOMTR_OS_NETWARE) || defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	cout << "dynamo /?" << endl;
#elif defined(IOMTR_OS_SOLARIS)
	// Solaris 2.7 must have the switch (? is used for its own purpose).
	cout << "dynamo \\?" << endl;
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

#if defined(IOMTR_OS_LINUX)
	cout << "dynamo [-i iometer_computer_name -m manager_computer_name] [-n manager_name]" << endl;
	cout << "       [-x excluded_fs_type] [-d extra_device] [-f extra_device_file] [-l]" << endl;
	cout << "       [-c cpu_affinity] [-p login_port_number]" << endl;
#elif defined (IOMTR_OS_SOLARIS)
	cout << "dynamo [-i iometer_computer_name -m manager_computer_name] [-n manager_name]" << endl;
	cout << "       [-x excluded_fs_type] [-d extra_device] [-f extra_device_file] [-l]" << endl;
	cout << "       [-p login_port_number]" << endl;
#elif defined(IOMTR_OS_OSX)
	cout << "dynamo [-i iometer_computer_name -m manager_computer_name] [-n manager_name]" << endl;
	cout << "       [-x excluded_fs_type] [-d extra_device] [-f extra_device_file] [-l]" << endl;
	cout << "       [-p login_port_number]  [use_rdtsc]" << endl;
#elif defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	cout << "dynamo [/i iometer_computer_name /m manager_computer_name] [/n manager_name]" << endl;
	cout << "       [/c cpu_affinity] [/p login_port_number]" << endl;
	cout << "       [use_rdtsc] [force_raw] " << endl;
#elif defined(IOMTR_OS_NETWARE)
	cout << "dynamo [/i iometer_computer_name /m manager_computer_name] [/n manager_name]" << endl;
	cout << "       [/x excluded_volumes] [/c cpu_affinity] [/p login_port_number]" << endl;
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

	cout << endl;
	cout << "   ? - show Dynamo version number and command line syntax" << endl;
	cout << endl;
	cout << "   iometer_computer_name - the name of the computer running Iometer" << endl;
	cout << "      This is only needed if Dynamo and Iometer are running on different" << endl;
	cout << "      computers.  Without this parameter, Dynamo will search for Iometer" << endl;
	cout << "      on the local host." << endl;
	cout << endl;
	cout << "   manager_name - the name of this Dynamo" << endl;
	cout << "      This name is the one Iometer will use as the manager name, important" << endl;
	cout << "      when restoring config files.  This defaults to the host's name." << endl;
	cout << endl;
	cout << "   manager_computer_name - the name of the computer running this Dynamo" << endl;
	cout << "      This name or IP address is the one Iometer will use to communicate" << endl;
	cout << "      with this manager. The default is the IP adress of the host's first" << endl;
	cout << "      NIC." << endl;
	cout << "      Make sure iometer box can ping this name successfully, otherwise dynamo" << endl;
	cout << "      and iometer will hang a long time during login." << endl;
	cout << endl;
	cout << "   login_port_number - the port number Iometer is listening on. If this" << endl;
	cout << "      parameter is not given, the default port(" << WELL_KNOWN_TCP_PORT << ") will be used." << endl;
	cout << endl;

#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
	cout << "   excluded_fs_type - type of filesystems to exclude from device search" << endl;
	cout << "      This string should contain the filesystem types that are not reported" << endl;
	cout << "      to Iometer. The default is \"" << DEFAULT_EXCLUDE_FILESYS << "\"." << endl;
	cout << endl;
	cout << "   extra_device - block device name to be included in the test if this device" << endl;
	cout << "      CAN NOT be detected automatically by dynamo. You can use it multiple times," << endl;
	cout << "      for example, -b dev1 -b dev2 -b dev3..." << endl;
	cout << endl;
	cout << "   extra_device_file - a file store extra device names with each name in one line" << endl;
	cout << "      use this in case you have lots of extra device need to be claimed." << endl;
	cout << "      You MUST use absolute path for device name here." << endl;
	cout << endl;
	cout << "   -l - to record events in syslog, default is /var/log/messages." << endl;
	cout << endl;
#elif defined(IOMTR_OSFAMILY_NETWARE)
	cout << "   excluded_volumes - volumes to exclude from volume or device search" << endl;
	cout << "      The default is \"" << "none" << "\"." << endl;
	cout << endl;
#elif defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	// nop  
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	cout << "   cpu_affinity - cpu affinity is used to bind dynamo to certain processor(s)." << endl;
	cout << "      This is used for getting a more consistent timing value. For example, if" << endl;
	cout << "      you want to bind to the third cpu, you should have value 4(00000100) here." << endl;
	cout << "      If this parameter is not specified, dynamo will bind to the first CPU" << endl;
	cout << "      in system by default." << endl;
	cout << endl;
#endif

#if defined(IOMTR_OSFAMILY_WINDOWS) || defined(IOMTR_OS_OSX)
	cout << "   use_rdtsc - enables explicit rdtsc use in favor of other OS API" << endl;
	cout << "      for measuring I/O performance." << endl;
	cout << endl;
#endif

#if defined(IOMTR_OSFAMILY_WINDOWS)
	cout << "   force_raw - forces dynamo to report all raw disks regardless of partitions" << endl;
	cout << "      contained within them. " << endl;
	cout << endl;
#endif

	exit(0);
}

/* ######################################################################### */
/* ##                                                                     ## */
/* ##   P A R S E P A R A M ( )                                           ## */
/* ##                                                                     ## */
/* ######################################################################### */
static void ParseParam(int argc, char *argv[], struct dynamo_param *param)
{
	// Local variables

	char cSwitchKey;

	BOOL bParamIometer = FALSE;
	BOOL bParamDynamo = FALSE;
	int count = 0;
	ifstream devfile;

	// Walk through the parameter list

	for (int I = 1; I < argc; I++) {

		// See if the user is requesting syntax help.

		if ((argv[I][0] == '?') || (argv[I][1] == '?')) {
			Syntax();
			return;
		}
		// Ensure that the each parameter has a leading switch
		if ((argv[I][0] != '-') && (argv[I][0] != '/')) {
			Syntax("Exactly one letter must follow a switch character.\n"
			       "Switch characters are \"-\" and \"/\".");
			return;
		}

		if (strlen(argv[I]) != 2) {

#if defined(IOMTR_OSFAMILY_WINDOWS)
			// TODO for non-Windows
			//
			// This should be turned on for all OSes, but I am not sure about the underscode in _stricmp
			//
			if (!stricmp(&argv[I][1], "force_raw"))
			{
				param->disk_control = RAWDISK_VIEW_FULL;
				continue;
			} 
			else
#endif
#if defined(IOMTR_OSFAMILY_WINDOWS) || defined(IOMTR_OS_OSX)
			if (!stricmp(&argv[I][1], "use_rdtsc"))
			{
				param->timer_type = TIMER_RDTSC;

				//
				// Kludge, we don't really need the above param->timer_type since we can 
				// directly access the TimerType global defines in IoTime.h. This should
				// be cleaned up better.
				//
				if (param->timer_type != TIMER_UNDEFINED && param->timer_type < TIMER_TYPE_MAX)
				{
					TimerType = (timer_type) param->timer_type;
					cout << "Dynamo will attempt to use the TSC/ITC CPU timer." << endl;
				}

				continue;
			}
			else
#endif
			{
				Syntax("Exactly one letter must follow a switch character.\n"
					   "Switch characters are \"-\" and \"/\".");
				return;
			}
		}
		// Ensure that each parameter has a value following the switch

		if ((I + 1) >= argc) {
			Syntax("An additional parameter was expected after the last switch.");
			return;
		}
		// Process the parameters based on the switch

		cSwitchKey = toupper(argv[I][1]);
		I++;

		switch (cSwitchKey) {
		case 'I':
			if (bParamIometer == TRUE) {
				Syntax("Iometer address was specified more than once.");
				return;
			}
			if (strlen(argv[I]) >= MAX_NETWORK_NAME) {
				Syntax("Iometer address parameter was too long.");
				return;
			}
			strcpy(param->iometer, argv[I]);
			bParamIometer = TRUE;
			break;
		case 'M':
			// No check for more then once specification (as we have a default)
			if (strlen(argv[I]) >= MAX_NETWORK_NAME) {
				Syntax("Manager network name parameter was too long.");
				return;
			}
			strcpy(param->manager_computer_name, argv[I]);
			bParamDynamo = TRUE;
			break;
		case 'N':
			if (param->manager_name[0] != '\0') {
				Syntax("Manager name was specified more than once.");
				return;
			}
			if (strlen(argv[I]) >= MAX_WORKER_NAME) {
				Syntax("Manager name parameter was too long.");
				return;
			}
			strcpy(param->manager_name, argv[I]);
			break;
		case 'P':
			if (argv[I])
				param->login_port_number = atoi(argv[I]);
			if (param->login_port_number < 1 || param->login_port_number > 65535) {
				Syntax("Port number was out of range.");
				param->login_port_number = 0;
				return;
			}
			break;
#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
		case 'X':
			if ((strlen(argv[I]) + strlen(param->manager_exclude_fs)) >= MAX_EXCLUDE_FILESYS) {
				Syntax("Excluded filesystem list too long.");
				return;
			}
			strcat(param->manager_exclude_fs, argv[I]);
			strcat(param->manager_exclude_fs, " ");
			break;
#endif
#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
		case 'D':
			if (check_dev(argv[I])) {
				Syntax("Not a valid device.");
				return;
			}
			if (count < MAX_TARGETS) {
				strcpy((*param->blkdevlist)[count++], argv[I]);
			} else {
				cout << "Too many targets you want to test, skip " << argv[I] << endl;
			}
			break;
		case 'F':
			char devname[MAX_NAME];

			devfile.open(argv[I]);
			if (!devfile.is_open()) {
				Syntax("Can not open device file list.");
				return;
			}
			while (!devfile.eof()) {
				memset(devname, 0, MAX_NAME);
				devfile.getline(devname, MAX_NAME - 1);
				if (check_dev(devname))
					continue;
				if (count < MAX_TARGETS) {
					strcpy((*param->blkdevlist)[count++], devname);
				} else {
					cout << "Too many targets you want to test, skip " << devname << endl;
				}
			}
			break;
		case 'L':
			do_syslog = TRUE;
			openlog(NEW_WORKER_EXECUTABLE, 0, LOG_USER);
			break;
#endif
#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
		case 'C':
			if (argv[I])
			{
				// Lets use sscanf below to support hex input
				ULONG_PTR tempMask = 0;
				
				// Handle both hex and decimal values. Abstract format spec syntax (in iocommon.h)
				if (argv[I][0] == '0' && argv[I][1] == 'x')
					sscanf(argv[I],"0x%" IOMTR_FORMAT_SPEC_64BIT "x", &tempMask);
				else
					sscanf(argv[I],"%" IOMTR_FORMAT_SPEC_64BIT "d", &tempMask);
			}
			break;
#endif

		default:
			{
				char tmpary[2] = { cSwitchKey, 0 };
#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
				char temp_array[128];

				strcpy(temp_array, "Unrecognized switch: ");
				strcat(temp_array, tmpary);
				strcat(temp_array, ".");
				Syntax(temp_array);
#elif defined(IOMTR_OSFAMILY_WINDOWS)
				Syntax("Unrecognized switch: " + (CString) tmpary + ".");
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
			}
			break;
		}
	}

	// Enforce switch combinations
	if (bParamIometer && !bParamDynamo) {
		Syntax("When specifying the Iometer address, the Manager network name parameter is mandatory.");
	}

	return;
}
