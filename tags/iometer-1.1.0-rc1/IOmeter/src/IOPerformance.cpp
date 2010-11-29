/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / IOPerformance.cpp                                         ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: The generic variant of the Performance class.         ## */
/* ##               This class does two jobs:                             ## */
/* ##               1.) Collects basic informations like the number       ## */
/* ##                   of CPU's.                                         ## */
/* ##               2.) Collection of system-wide performance             ## */
/* ##                   informations.                                     ## */
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
/* ##               2005-04-10 (mingz@ele.uri.edu)                        ## */
/* ##               - Add type cast to remove compile warning for Solaris.## */
/* ##               2004-09-01 (henryx.w.tiemam@intel.com)                ## */
/* ##               - Switched to more generic IOMTR_CPU_X86_64.          ## */
/* ##               2004-03-27 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Code cleanup to ensure common style.                ## */
/* ##               - Applied Thayne Harmon's patch for supporting        ## */
/* ##                 Netware support (on I386).                          ## */
/* ##               2004-03-18 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Corrected the CPU speed detection output as it      ## */
/* ##                 was proposed Kenneth Morse.                         ## */
/* ##               2003-10-05 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Integrated the modification contributed by          ## */
/* ##                 Vedran Degoricija, to get the code compile with     ## */
/* ##                 the Windows 64 Bit on AMD64.                        ## */
/* ##               2003-08-02 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Integrated the modification contributed by          ## */
/* ##                 Vedran Degoricija, to get the code compile with     ## */
/* ##                 the MS DDK on IA64.                                 ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Massive cleanup of this file (grouping the          ## */
/* ##                 different blocks together).                         ## */
/* ##               2003-07-19 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Removed IOTime.h inclusion (now in IOCommon.h)      ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-03-04 (joe@eiler.net)                            ## */
/* ##               -Cleaned up some compiler warnings on Solaris         ## */
/* ##               2003-02-26 (joe@eiler.net)                            ## */
/* ##               -Added some more processor info stuff                 ## */
/* ##               2003-02-02 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##               - Applied proc-speed-fix.txt patch file               ## */
/* ##                 (dropping two type casts in the                     ## */
/* ##                 Get_Processor_Speed() method).                      ## */
/* ##                                                                     ## */
/* ######################################################################### */
#define PERFORMANCE_DETAILS	0	// Turn on to display additional performance messages.
#if !defined(IOMTR_OS_LINUX) && !defined(IOMTR_OS_NETWARE) && !defined(IOMTR_OS_OSX)

#include "IOPerformance.h"
#if defined(IOMTR_OS_SOLARIS)
#include <sys/types.h>
#include <unistd.h>
#include <sys/processor.h>
#endif

//
// Initializing system performance data.
//
Performance::Performance()
{
	int i;

#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	// Allocating buffer to receive performance data.
	if (!(perf_data = (LPBYTE) malloc(MAX_PERF_SIZE))) {
		cout << "*** Unable to allocate space for performance data." << endl << flush;
		exit(1);
	}
	perf_size = MAX_PERF_SIZE;
	perf_object = NULL;
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE) || defined(IOMTR_OS_SOLARIS)
	// nop
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

	// Obtaining the number of CPUs in the system and their speed.
	processor_count = Get_Processor_Count();
	timer_resolution = Get_Timer_Resolution();

	// Network performance statistics are disabled by default.  Assume this unless later
	// performance calls succeed.  Then, set the correct number of interfaces.
	network_interfaces = 0;

	if (!timer_resolution || !processor_count) {
		cout << "*** Unable to initialize needed performance data." << endl
		    << "This error may indicate that you are trying to run on an unsupported" << endl
		    << "processor or OS.  See the Iometer User's Guide for information on" << endl
		    << "supported platforms." << endl;
		exit(1);
	}
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	// Setting the NT CPU performance counters to extract.
	cpu_perf_counter_info[CPU_TOTAL_UTILIZATION].index = PERF_CPU_TOTAL_UTILIZATION;
	cpu_perf_counter_info[CPU_USER_UTILIZATION].index = PERF_CPU_USER_UTILIZATION;
	cpu_perf_counter_info[CPU_PRIVILEGED_UTILIZATION].index = PERF_CPU_PRIVILEGED_UTILIZATION;
	cpu_perf_counter_info[CPU_DPC_UTILIZATION].index = PERF_CPU_DPC_UTILIZATION;
	cpu_perf_counter_info[CPU_IRQ_UTILIZATION].index = PERF_CPU_IRQ_UTILIZATION;
	cpu_perf_counter_info[CPU_IRQ].index = PERF_CPU_IRQ;

	// Setting the NT network performance counters to extract.
	tcp_perf_counter_info[TCP_SEGMENTS_RESENT].index = PERF_TCP_SEGMENTS_RESENT;
	ni_perf_counter_info[NI_PACKETS].index = PERF_NI_PACKETS;
	ni_perf_counter_info[NI_OUT_ERRORS].index = PERF_NI_OUT_ERRORS;
	ni_perf_counter_info[NI_IN_ERRORS].index = PERF_NI_IN_ERRORS;

	// Initializing the offsets to specific performance counters to invalid.
	for (i = 0; i < CPU_RESULTS; i++)
		cpu_perf_counter_info[i].offset = IOERROR;
	for (i = 0; i < TCP_RESULTS; i++)
		tcp_perf_counter_info[i].offset = IOERROR;
	for (i = 0; i < NI_RESULTS; i++)
		ni_perf_counter_info[i].offset = IOERROR;
#elif defined(IOMTR_OS_SOLARIS)
	// Initialize all the arrays to 0.
	memset(raw_cpu_data, 0, (MAX_CPUS * CPU_RESULTS * MAX_SNAPSHOTS * sizeof(_int64)));
	memset(raw_ni_data, 0, (MAX_NUM_INTERFACES * NI_RESULTS * MAX_SNAPSHOTS * sizeof(_int64)));
	memset(raw_tcp_data, 0, (TCP_RESULTS * MAX_SNAPSHOTS * sizeof(_int64)));

	// Note:
	//              We get a handle to the kstat data which contains CPU and Net Interface info.
	//              The TCP info is obtained through streams.
	//
	clock_tick = sysconf(_SC_CLK_TCK);

	// Open and obtain a handle to the kstat.
	kc = kstat_open();
	if (kc == NULL) {
		cout << "kstat open failed with error " << errno << endl;
		// exit(1);
	}
	// Note that cpu count is already in place.
	// Get all the network interface names into a array.
	int sockfd;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket open failed ");
		exit(1);
	}
	// Now do an ioctl (SIOCGIFCONF) on the socket. The ioctl(SIGIOCIFCONF)
	// is used to get info about all the network interfaces in the system
	// including the interface names.
	//
	// We need the interface name to do a kstat_lookup() for that interface.
	// (for example : iprb0, fddi0 etc)
	//
	struct ifconf myifconf;
	char buf[(sizeof(struct ifreq) * MAX_NUM_INTERFACES) + BUFSIZ];

	myifconf.ifc_len = sizeof(buf);
	myifconf.ifc_buf = buf;

	if (ioctl(sockfd, SIOCGIFCONF, (char *)&myifconf) < 0) {
		cout << "ioctl (SIOCGIFCONF) failed with error " << errno << endl;
		exit(1);
	}
	// else ioctl succeeded and we have all the data !
	struct ifreq *ifreqp;

	ifreqp = myifconf.ifc_req;
	for (i = 0; i < (int)(myifconf.ifc_len / sizeof(struct ifreq)); i++, ifreqp++) {
		if (ifreqp->ifr_addr.sa_family != AF_INET)
			continue;
		// ioctl (SIOCGIFFLAGS) on the socket.
		if (ioctl(sockfd, SIOCGIFFLAGS, (char *)ifreqp) < 0) {
			cout << "ioctl (SIOCGIFFLAGS) failed with error " << errno << endl;
			exit(1);
		}
		if (!(ifreqp->ifr_flags & IFF_LOOPBACK))
			// We have a non-loopback IP interface name. get the name into the array
			// and increment network_interfaces.
			strcpy(nic_names[network_interfaces++], ifreqp->ifr_name);
	}
	close(sockfd);

	// Set up the Streams modules to get the tcpSegments retrans data.
	// This is moved to the get_tcp_counters call because after reading the initial data
	// off the stream, we still have to run getmsg() many more times (till we get a -1)
	// to read off all mib2_tcp data off the stream queues.
	if ((getuid() == 0) || (geteuid() == 0)) {
		// only the super-user can play with Streams.
		// We are allocating 64 and 1024 bytes because we know that that the data we
		// are sending to and receivng from the streams is under our control and never
		// comes anywhere near these numbers.
		ctlbuf = (char *)malloc(64);
		if (ctlbuf == NULL) {
			cout << "unable to allocate memory " << endl;
			exit(1);
		}
		databuf = (char *)malloc(1024);
		if (databuf == NULL) {
			cout << "unable to allocate memory " << endl;
			exit(1);
		}
		streamfd = 0;
	} else
		streamfd = -1;
#elif defined(IOMTR_OS_LINUX)
	// nop
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
}

//
// Freeing memory used by class object.
//
Performance::~Performance()
{
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	free(perf_data);
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_SOLARIS)
	free(ctlbuf);
	free(databuf);
	kstat_close(kc);
	streamfd = -1;
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
}

//
// Getting the number of processors in the system.
//
int Performance::Get_Processor_Count()
{
#if defined(IOMTR_OS_SOLARIS)
	return (sysconf(_SC_NPROCESSORS_CONF));
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE)
	// nop
#elif defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	SYSTEM_INFO system_info;

	SetLastError(0);
	GetSystemInfo(&system_info);
	if (GetLastError()) {
		cout << "*** Unable to determine number of processors in system.";
		return 0;
	}
	cout << "Number of processors: " << system_info.dwNumberOfProcessors << endl;
	return (int)system_info.dwNumberOfProcessors;
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
}

#define SPEED_VALUE_COUNT 117

//
// Getting the speed of the processors in Hz.
//
double Performance::Get_Timer_Resolution()
{
	// Note: When adding entries, make sure you also incerement the upper
	// bound of the index in the for loop below.
	int speed_values[SPEED_VALUE_COUNT] = {
		3060, 3000, 2930, 2900, 2800, 2700, 2660, 2600, 2530,
		2500, 2400, 2300, 2260, 2200, 2000, 1900, 1800, 1700,
		1600, 1500, 1400, 1300, 1260, 1200, 1130, 1100, 1060,
		1000, 980, 975, 966, 950, 940, 933, 920, 910,
		900, 880, 875, 866, 850, 840, 833, 820, 810,
		800, 780, 775, 766, 750, 740, 733, 720, 710,
		700, 680, 675, 666, 650, 640, 633, 620, 610,
		600, 580, 575, 566, 550, 540, 533, 520, 510,
		500, 480, 475, 466, 450, 440, 433, 420, 410,
		400, 380, 375, 366, 350, 340, 333, 320, 310,
		300, 280, 275, 266, 250, 240, 233, 220, 210,
		200, 180, 175, 166, 150, 140, 133, 120, 110,
		100, 80, 75, 66, 50, 40, 33, 25, 20
	};
	DWORD speed;

#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)

#if 1 // was WIN_USE_RDTSC
	//
	// Delete the old code once this works reliably -- ved
	return timer_frequency(); // iotime.cpp

#else

	int speed_magnitude;	/* 0=MHz,1=GHz */
	DWORD type;
	DWORD size_of_speed = sizeof(DWORD);

	BOOL rdtsc_supported = TRUE;
	HKEY processor_speed_key;

	// Try RDTSC and see if it causes an exception.  (This code is NT-specific 
	// because Solaris does not support __try/__except.)
	__try {
		timer_value();
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {
		rdtsc_supported = FALSE;
	}

	if (!rdtsc_supported) {
		cout << "*** Processor does not support RDTSC/ITC instruction!" << endl <<
		    "    Dynamo requires this for high-resolution timing." << endl;
		return (double)0.0;
	}
	// Retrieving the estimated speed from the NT system registry.
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0\0",
			 0, KEY_READ, &processor_speed_key) != ERROR_SUCCESS) {
		cout << "*** Could not locate processor information in registry." << endl;
		return (double)0.0;
	}
	if (RegQueryValueEx(processor_speed_key, "~MHz\0", NULL, &type, (LPBYTE) & speed, &size_of_speed)
	    != ERROR_SUCCESS) {
		/* if getting MHz fails try to get GHz */
		if (RegQueryValueEx(processor_speed_key, "~GHz\0", NULL, &type, (LPBYTE) & speed, &size_of_speed)
		    != ERROR_SUCCESS) {
			cout << "*** Could not retrieve estimated processor speed from registry." << endl;
			return (double)0.0;
		} else {
			speed_magnitude = 1;	/* set to GHz */
		}
	} else {
		speed_magnitude = 0;	/* set to MHz */
	}

	if (speed_magnitude == 1) {
		speed *= 1000;	/* convert the GHz value to MHz */
	}
#endif // WIN_USE_RDTSC

#elif defined(IOMTR_OS_SOLARIS)
	processor_info_t infop;
	int j, status;

	for (j = 0; j < MAX_CPUS; j++) {
		status = p_online((processorid_t) j, P_STATUS);
		if ((status == -1) || (status == EINVAL))
			continue;

		// Ok we found a processor.
		// This call should succeed. Else it means that the system has no processors !!!
		// ("its haunted" !!!) or that this call is unsupported.
		if (processor_info((processorid_t) j, &infop) < 0) {
			cout << "*** Could not determine processor speed." << endl << flush;
			return (double)0.0;
		}
		// found a processor and also the obtained the infop structure. So break.
		break;
	}
	if (j == MAX_CPUS) {
		cout << "*** Could not determine processor speed." << endl << flush;
		return (double)0.0;
	}
	// If there are more than one processors, well, we get the speed of the first one.
	speed = infop.pi_clock;
#elif defined(IOMTR_OS_LINUX)
	// nop
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

	for (int i = 0; i < SPEED_VALUE_COUNT; i++) {
		// Try to match estimated speed with a known one.
		if (((int)speed > speed_values[i] - 5) && ((int)speed < speed_values[i] + 5)) {
			speed = speed_values[i];
			if (speed < 1000) {
				cout << "Processor speed: " << speed << " MHz." << endl;
			} else {
				cout << "Processor speed: " << speed / 1000.0 << " GHz." << endl;
			}
			return (double)(speed * 1000000);
		}
	}
	// Round the registry value to the nearest 10
	speed = ((speed + 5) / 10) * 10;
	if (speed < 1000) {
		cout << "Processor speed: " << speed << " MHz." << endl;
	} else {
		cout << "Processor speed: " << speed / 1000.0 << " GHz." << endl;
	}
	return (double)(speed * 1000000);
}

/*
Getting NT performance counters.  These counters provide the same performance
information found running perfmon.

This is the structure of the data returned by the RegQueryValueEx 
function.

PERF_DATA_BLOCK: A simple header for all of the performance data returned.

	PERF_OBJECT_TYPE 1:	Describes the "system object" for which performance
						data has been returned.  Examples of objects
						are processors, network interface, and memory.

		PERF_COUNTER_DEF 1..k:	Defines a specific type of performance counter.
							Example counters are % processor time and
							interrupts per second.  One definition is given
							for each counter returned.
			
		PERF_INSTANCE_DEF 1:If multiple instances of an "object" can exist,
							lists the first one.  E.g. the processor object can
							have many instances (multi-processor system).
			
			PERF_COUNTER_BLOCK:	Header above actual performance counters.

			counters 1..k:		Performance counters, one for each defined above.
		...
		PERF_INSTANCE_DEF i:Next instance of an object

			PERF_COUNTER_BLOCK:

			counters 1..k:
	...
	PERF_OBJECT_TYPE t:
		...
*/
//
// Extracting system performance data.  The data is accessed through the NT registry, but stored elsewhere.
//
void Performance::Get_Perf_Data(DWORD perf_data_type, int snapshot)
{
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	long query_result;	// Value returned trying to query performance data.
	DWORD perf_object_size;	// Size of buffer allocated to storing performance data.
	char perf_data_type_name[10];	// ASCII representation of performance data index.
	_int64 perf_update_freq;	// Frequency that performance counters are updated.
#elif defined(IOMTR_OS_SOLARIS) || defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE)
	// nop
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

	// Get the performance data stored by the system.
#ifdef _DEBUG
	cout << "   Getting system performance data." << endl << flush;
#endif

#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	perf_object_size = perf_size;
	_itoa(perf_data_type, perf_data_type_name, 10);	// convert index to a string

	// Continue trying to get the performance data until successful or utter failure.
	// Grow the buffer allocated to hold the performance data until it is big enough.
	while ((query_result = RegQueryValueEx(HKEY_PERFORMANCE_DATA, perf_data_type_name,
					       NULL, NULL, perf_data, &perf_object_size)) != ERROR_SUCCESS) {
		if (query_result == ERROR_MORE_DATA) {
			// More data than we had room for!  Try a bigger buffer.
#ifdef _DEBUG
			cout << "   RegQueryValueEx() returned ERROR_MORE_DATA!  Number of bytes used: "
			    << perf_object_size << endl << flush
			    << "   Bumping performance data buffer up from " << perf_size
			    << " to " << perf_size + MAX_PERF_SIZE << " bytes." << endl << flush;
#endif
			perf_size += MAX_PERF_SIZE;
			if (!(perf_data = (LPBYTE) realloc(perf_data, perf_size))) {
				cout << "Error allocating more space for performance data." << endl << flush;
				exit(1);
			}
			perf_object_size = perf_size;
		} else {
			// RegQueryValueEx failed -- get error message text from system
			LPSTR message;

			if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, NULL,	// get message from system
					  query_result, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	// default language
					  (LPSTR) & message, 0, NULL)) {
				cout << "Error " << query_result << " getting performance data: " << message << flush;
				LocalFree(message);
			} else {
				// FormatMessage failed, just print error number
				cout << "Error " << query_result << " getting performance data." << endl << flush;
			}

			// special case for known Win95 error
			if (query_result == ERROR_KEY_DELETED) {
				cout << "This error may indicate that Dynamo is being run under Windows 95." << endl
				    << "Windows 95 is not supported; Dynamo requires Windows NT 4.0 or greater." << endl
				    << flush;
			}
			return;
		}
	}			// end while (trying to query registry values)
#if PERFORMANCE_DETAILS || _DETAILS
	cout << "   Number of bytes used for performance data: " << perf_object_size << endl << flush;
#endif

	// Record time that snapshot was taken.
	time_counter[snapshot] = (_int64) ((PERF_DATA_BLOCK *) perf_data)->PerfTime.QuadPart;

	// Do we need to record the length of time between snapshots?
	if (snapshot == LAST_SNAPSHOT) {
		// Yes, get the update frequency.
		perf_update_freq = (_int64) ((PERF_DATA_BLOCK *) perf_data)->PerfFreq.QuadPart;
#if PERFORMANCE_DETAILS || _DETAILS
		cout << "   Performance update frequency = " 
			<< perf_update_freq 
			<< endl
			<< flush;
#endif

		// Calculate the length of time that measurements were taken.
		if (perf_update_freq && ((time_counter[LAST_SNAPSHOT] - time_counter[FIRST_SNAPSHOT]) > 0))
			perf_time =
			    (double)(time_counter[LAST_SNAPSHOT] - time_counter[FIRST_SNAPSHOT]) / perf_update_freq;
		else {
			perf_time = (double)0.0;	// Error gathering performance time, mark as invalid.
		}
#ifdef _DEBUG
		cout << "   Performance time = " << perf_time << endl << flush;
#endif
	}
	Extract_Counters(perf_data_type, snapshot);
#elif defined(IOMTR_OS_SOLARIS)
	time_counter[snapshot] = gethrtime();
	if (snapshot == LAST_SNAPSHOT)
		// calculate time diff in clock ticks..
		timediff = ((double)time_counter[LAST_SNAPSHOT] - time_counter[FIRST_SNAPSHOT]) * clock_tick
		    / (double)1000000000;

	switch (perf_data_type) {
	case PERF_PROCESSOR:
		Get_CPU_Counters(snapshot);
		break;
	case PERF_NETWORK_TCP:
		Get_TCP_Counters(snapshot);
		break;
	case PERF_NETWORK_INTERFACE:
		Get_NI_Counters(snapshot);
		break;
	default:
		break;
	}
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE)
	// nop
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
}

//
// Calculating CPU statistics based on snapshots of performance counters.
//
void Performance::Calculate_CPU_Stats(CPU_Results * cpu_results)
{
	int cpu, stat;		// Loop control variables.

	// Loop though all CPUs and determine various utilization statistics.
	cpu_results->count = processor_count;
	for (cpu = 0; cpu < processor_count; cpu++) {
		// Loop through the counters and calculate performance.
		for (stat = 0; stat < CPU_RESULTS; stat++) {
#if PERFORMANCE_DETAILS
			cout << "Calculating stat " << stat << " for CPU " << cpu << endl;
#endif

#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
			// If we've never set the counter offsets, then we've never successfully retrieved
			// the performance data.  Set all of the values to 0.
			if (cpu_perf_counter_info[stat].offset == IOERROR) {
				cout << "*** Offset to CPU performance counter not defined for stat "
				    << stat << "." << endl;
				cpu_results->CPU_utilization[cpu][stat] = (double)0.0;
			} else {
				cpu_results->CPU_utilization[cpu][stat] =
				    Calculate_Stat(raw_cpu_data[cpu][stat][FIRST_SNAPSHOT],
						   raw_cpu_data[cpu][stat][LAST_SNAPSHOT],
						   cpu_perf_counter_info[stat].type);
			}
#elif defined(IOMTR_OS_SOLARIS)
			double result;

			if (stat == CPU_IRQ) {
				// we have to calculate Interrupts/sec.
				// This is similar to calculating Network packets per second
				// but we are more fortunate here.
				// See the corresponding Notes at the end of this file for a description.
				//
				result = ((double)raw_cpu_data[cpu][stat][LAST_SNAPSHOT]
					  - raw_cpu_data[cpu][stat][FIRST_SNAPSHOT]) * clock_tick / timediff;
				cpu_results->CPU_utilization[cpu][stat] = result;
			} else {
				// All other CPU statistics.
				result = ((double)raw_cpu_data[cpu][stat][LAST_SNAPSHOT]
					  - raw_cpu_data[cpu][stat][FIRST_SNAPSHOT]) / timediff;

				if (result < 0.0) {
					result = 0.0;
					//
					// CPU Utilization figures are outside valid range far too often.
					// Ok, not in every cycle but frequent still.
					// So, it is better to comment it out rather than have the message
					// pop up on the screen at regular intervals.
					//
					// cout << "***** Error : CPU utilization outside valid range 0% - 100% *****" << endl;
				}
				if (result > 1.0) {
					result = 1.0;
				}

				cpu_results->CPU_utilization[cpu][stat] = (result * 100);
			}
#elif defined(IOMTR_OS_LINUX)
			// nop
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

#if PERFORMANCE_DETAILS || _DETAILS
			cout << "CPU " << cpu << " recorded stat " << stat << " = "
			    << cpu_results->CPU_utilization[cpu][stat] << endl;
#endif
		}
	}
}

//
// Calculate network performance statistics based on snapshots of performance counters.
//
void Performance::Calculate_TCP_Stats(Net_Results * net_results)
{
	int stat;		// Loop control variable.

	// Loop through the counters and calculate performance.
	for (stat = 0; stat < TCP_RESULTS; stat++) {
		// If we've never set the counter offsets, then we've never successfully retrieved
		// the performance data.  Set all of the values to 0.
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
		if (tcp_perf_counter_info[stat].offset == IOERROR) {
			net_results->tcp_stats[stat] = (double)0.0;
		} else {
			net_results->tcp_stats[stat] = Calculate_Stat(raw_tcp_data[stat][FIRST_SNAPSHOT],
								      raw_tcp_data[stat][LAST_SNAPSHOT],
								      tcp_perf_counter_info[stat].type);
		}
#elif defined(IOMTR_OS_SOLARIS)
		double result;

		result = ((double)raw_tcp_data[stat][LAST_SNAPSHOT] - raw_tcp_data[stat][FIRST_SNAPSHOT]) / timediff;
		result *= clock_tick;	// note that timediff is in CLK_TCKs and not seconds
		net_results->tcp_stats[stat] = result;
#elif defined(IOMTR_OS_LINUX)
		// nop
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

#if PERFORMANCE_DETAILS || _DETAILS
		cout << "TCP recorded stat " << stat << " = " << net_results->tcp_stats[stat] << endl;
#endif
	}
}

//
// Calculate network performance statistics based on snapshots of performance counters.
//
void Performance::Calculate_NI_Stats(Net_Results * net_results)
{
	int net, stat;		// Loop control variables.

	// Loop through the counters and calculate performance.
	net_results->ni_count = network_interfaces;
	for (net = 0; net < network_interfaces; net++) {
		for (stat = 0; stat < NI_RESULTS; stat++) {
			// If we've never set the counter offsets, then we've never successfully retrieved
			// the performance data.  Set all of the values to 0.
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
			if (ni_perf_counter_info[stat].offset == IOERROR) {
				net_results->ni_stats[net][stat] = (double)0.0;
			} else {
				net_results->ni_stats[net][stat] =
				    Calculate_Stat(raw_ni_data[net][stat][FIRST_SNAPSHOT],
						   raw_ni_data[net][stat][LAST_SNAPSHOT],
						   ni_perf_counter_info[stat].type);
			}
#elif defined(IOMTR_OS_SOLARIS)
			double result;

			//
			// Note:
			//              The array time_counter[] stores time in nanoseconds.
			// Earlier, we used to divide by the calculated value of timediff and then
			// multiply the result by clock_ticks per second to get the NI_data per
			// second which was theoretically correct (and mathematically same as what 
			// we are doing now) but reported wrong values while working with such 
			// large numbers.
			//
			result = ((double)raw_ni_data[net][stat][LAST_SNAPSHOT] -
				  raw_ni_data[net][stat][FIRST_SNAPSHOT]) * 1000000000.0 /
			    ((double)time_counter[LAST_SNAPSHOT] - time_counter[FIRST_SNAPSHOT]);

			net_results->ni_stats[net][stat] = result;
#elif defined(IOMTR_OS_LINUX)
			// nop
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

#if PERFORMANCE_DETAILS || _DETAILS
			cout << "   Network interface " << net << " recorded stat " << stat << " = "
			    << net_results->ni_stats[net][stat] << endl;
#endif
		}
	}
}

#if defined(IOMTR_OS_SOLARIS)
void Performance::Get_CPU_Counters(int snapshot)
{
	kstat_t *ksp;
	cpu_stat_t *cpu_stat;
	int current_cpu = 0;

	for (ksp = kc->kc_chain; ksp != NULL; ksp = ksp->ks_next) {
		if (ksp->ks_type != KSTAT_TYPE_RAW)
			continue;

		// cpu_stat is raw data.
		if (strcmp(ksp->ks_module, "cpu_stat") == 0) {
			// found a cpu_stat.
			if (-1 == kstat_read(kc, ksp, NULL)) {
				cout << "kstat_read() failed with error " << errno << endl;
				exit(1);
			}
			// else kstat_read thru..
			cpu_stat = (cpu_stat_t *) ksp->ks_data;
			// store the cpu_data
			raw_cpu_data[current_cpu][CPU_TOTAL_UTILIZATION][snapshot] =
			    cpu_stat->cpu_sysinfo.cpu[CPU_USER] + cpu_stat->cpu_sysinfo.cpu[CPU_KERNEL];
			raw_cpu_data[current_cpu][CPU_USER_UTILIZATION][snapshot] = cpu_stat->cpu_sysinfo.cpu[CPU_USER];
			raw_cpu_data[current_cpu][CPU_PRIVILEGED_UTILIZATION][snapshot] =
			    cpu_stat->cpu_sysinfo.cpu[CPU_KERNEL];
			raw_cpu_data[current_cpu][CPU_DPC_UTILIZATION][snapshot] = 0;
			raw_cpu_data[current_cpu][CPU_IRQ_UTILIZATION][snapshot] = 0;
			raw_cpu_data[current_cpu][CPU_IRQ][snapshot] = cpu_stat->cpu_sysinfo.intr;
			if (current_cpu++ > processor_count)
				break;
		}
	}
	return;
}

void Performance::Get_TCP_Counters(int snapshot)
{
	struct opthdr *opthdr;
	struct T_optmgmt_req *optreq;
	int flags = 0, retval;

	// We have already taken care of this in the constructor.
	// streamfd will be -1 iff !superuser.
	if (streamfd == -1) {
		raw_tcp_data[TCP_SEGMENTS_RESENT][snapshot] = 0;
		return;
	}
	// only root can open "/dev/ip" and push "tcp" module onto it.
	if ((streamfd = open("/dev/ip", O_RDWR, 0600)) < 0) {
		cout << "unable to open /dev/ip" << endl;
		streamfd = -1;
		return;
	}
	if (ioctl(streamfd, I_PUSH, "tcp") < 0) {
		cout << "unable to push tcp module onto the stream" << endl;
		close(streamfd);
		streamfd = -1;
	} else {
		// We are all okay here - get addrs of ctl and data bufs.
		strbuf_ctl.buf = ctlbuf;
		strbuf_data.buf = databuf;
	}

	// fill up the control info.
	optreq = (struct T_optmgmt_req *)strbuf_ctl.buf;
	optreq->PRIM_type = T_OPTMGMT_REQ;
	optreq->OPT_offset = sizeof(struct T_optmgmt_req);
	optreq->OPT_length = sizeof(struct opthdr);
	optreq->MGMT_flags = T_CURRENT;

	opthdr = (struct opthdr *)&strbuf_ctl.buf[sizeof(struct T_optmgmt_req)];
	opthdr->level = MIB2_TCP;
	opthdr->name = opthdr->len = 0;

	strbuf_ctl.len = optreq->OPT_offset + optreq->OPT_length;

	// Note:
	//              To get the data from the TCP streams module, we first setup the streams
	//              by opening "/dev/ip" and then pushing "tcp" module on top of it.
	//
	//              We first construct a control message (optreq + opthdr) indicating that
	//              the request is of type (management data request) T_OPTMGMT_REQ and is 
	//              requesting for data of type MIB2_TCP.
	//
	//              we putmsg() the control info onto the stream and then getmsg() for a
	//              reply. If successful, getmsg() returns a control message indicating
	//              the number of bytes of data that will be returned in another call.
	//
	//              Call getmsg() again to get the actual data bytes.
	//

	// Now putmsg() the ctl info onto the stream.
	if (putmsg(streamfd, &strbuf_ctl, NULL, 0) == -1) {
		cout << "putmsg() call failed with error " << errno << endl;
		raw_tcp_data[TCP_SEGMENTS_RESENT][snapshot] = 0;
		close(streamfd);
		return;
	}
	// Message queued successfully. Process replies.
	strbuf_ctl.maxlen = 64;
	opthdr = (struct opthdr *)&strbuf_ctl.buf[sizeof(struct T_optmgmt_ack)];

	// First get the reponse to the control message.
	if ((retval = getmsg(streamfd, &strbuf_ctl, NULL, &flags)) < 0) {
		cout << "getmsg() call failed with error " << errno << endl;
		raw_tcp_data[TCP_SEGMENTS_RESENT][snapshot] = 0;
		close(streamfd);
		return;
	}
	// The call itself succeeded. Was it able to get the info correctly ??
	if (retval == 0) {
		// Some or no error has occured. But the call is not returning us any
		// data. So bail out.
		raw_tcp_data[TCP_SEGMENTS_RESENT][snapshot] = 0;
		close(streamfd);
		return;
	}
	if (retval == MOREDATA) {
		// At last!. Just what we want...... We are not checkin for the data size
		// since we have allocated 1024 bytes which should be more than sufficient.
		flags = 0;
		strbuf_data.len = 0;
		strbuf_data.maxlen = opthdr->len;
		if ((retval == getmsg(streamfd, NULL, &strbuf_data, &flags)) != 0) {
			cout << "getmsg() call (data) failed with error " << errno << endl;
			raw_tcp_data[TCP_SEGMENTS_RESENT][snapshot] = 0;
			close(streamfd);
			return;
		}
		// retval == 0  is the only case where we have good data
		raw_tcp_data[TCP_SEGMENTS_RESENT][snapshot] = ((struct mib2_tcp *)(strbuf_data.buf))->tcpRetransSegs;
#if _DETAILS
		cout << "tcp segments retrans : " << ((struct mib2_tcp *)(strbuf_data.buf))->tcpRetransSegs << endl;
#endif
	}
	close(streamfd);
	return;
}

void Performance::Get_NI_Counters(int snapshot)
{
	kstat_t *ksp;
	kstat_named_t *knamed;
	int current_nic = 0, i;

	for (ksp = kc->kc_chain; ksp != NULL; ksp = ksp->ks_next) {
		if (ksp->ks_type != KSTAT_TYPE_NAMED)
			continue;

		// NET data is NAMED data.
		if (strcmp(ksp->ks_name, nic_names[current_nic]) == 0) {
			// found the interface we are looking for.
			if (0 > kstat_read(kc, ksp, NULL)) {
				cout << "kstat_read() failed with error " << errno << endl;
				exit(1);
			}
			// store data into raw_ni_data. First initialize for this snapshot to 
			// prevent a buildup of values.
			raw_ni_data[current_nic][NI_PACKETS][snapshot] = 0;
			raw_ni_data[current_nic][NI_ERRORS][snapshot] = 0;

			for (i = 0; i < (int)ksp->ks_ndata; i++) {
				knamed = KSTAT_NAMED_PTR(ksp) + i;
				if (strcmp(knamed->name, "opackets") == 0)
					raw_ni_data[current_nic][NI_PACKETS][snapshot] += knamed->value.ui32;
				if (strcmp(knamed->name, "ipackets") == 0)
					raw_ni_data[current_nic][NI_PACKETS][snapshot] += knamed->value.ui32;
				if (strcmp(knamed->name, "oerrors") == 0)
					raw_ni_data[current_nic][NI_ERRORS][snapshot] += knamed->value.ui32;
				if (strcmp(knamed->name, "ierrors") == 0) {
					raw_ni_data[current_nic][NI_IN_ERRORS][snapshot] = knamed->value.ui32;
					raw_ni_data[current_nic][NI_ERRORS][snapshot] += knamed->value.ui32;
				}
			}
			if (current_nic++ > network_interfaces)
				break;
		}
	}
	return;
}
#elif defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
//
// Obtaining the desired performance counters from the returned performance data.
//
void Performance::Extract_Counters(DWORD perf_data_type, int snapshot)
{
	// Locate and verify that we found where a valid performance object is at.
	if (!Locate_Perf_Object(perf_data_type))
		return;

	// After locating a valid performance object, set the offsets of specific performance counters.
	if (!Set_Counter_Info(perf_data_type))
		return;

	// Save specific performance counters.
	switch (perf_data_type) {
	case PERF_PROCESSOR:
#ifdef _DEBUG
		cout << "Extracting processor data." << endl << flush;
#endif
		Extract_CPU_Counters(snapshot);
		break;
	case PERF_NETWORK_TCP:
#ifdef _DEBUG
		cout << "   Extracting network TCP data." << endl << flush;
#endif
		Extract_TCP_Counters(snapshot);
		break;
	case PERF_NETWORK_INTERFACE:
#ifdef _DEBUG
		cout << "   Extracting network interface data." << endl << flush;
#endif
		Extract_NI_Counters(snapshot);
		break;
	default:
		cout << "*** Unknown performance data requested: " << perf_data_type << endl << flush;
		return;
	}
}

//
// Extracting counters for NT CPU performance data.
//
void Performance::Extract_CPU_Counters(int snapshot)
{
	char cpu_name[3], cpu_reg_name[3];
	int cpu, stat, i;

	// Loop through all processors and record performance information.
	for (cpu = 0; cpu < processor_count; cpu++) {
		// Find the desired processor instance.
		if (!Locate_Perf_Instance(cpu))
			return;

		// Verify that the instance found is for the current processor, 
		// otherwise perform an enhaustive search.
		_itoa(cpu, cpu_name, 10);

		strcpy((char *)cpu_reg_name, (char *)((LPBYTE) perf_instance + perf_instance->NameOffset));

		if (perf_instance->NameLength == 6) {
			strcat((char *)cpu_reg_name, (char *)((LPBYTE) perf_instance + perf_instance->NameOffset) + 2);
		}

		if (strncmp(cpu_name, cpu_reg_name, 2)) {
#ifdef _DEBUG
			cout << "Performing exhaustive search for processor instance " << cpu << endl;
#endif

			// Check all processor instances and 
			// try to match one with the desired processor.
			for (i = 0; i < perf_object->NumInstances; i++) {
				if (!Locate_Perf_Instance(i))
					return;
#if PERFORMANCE_DETAILS || _DETAILS
				cout << "Looking at processor name: " << (char *)cpu_reg_name << endl;
#endif
				// Match the name of the current instance with 
				//      the name of the desired cpu.
				if (!strncmp(cpu_name, (char *)((LPBYTE) perf_instance + perf_instance->NameOffset), 2))
					break;	// Found the correct instance.
			}
			if (i == perf_object->NumInstances) {
				cout << "*** Unable to locate performance instance of processor " << cpu_name << endl;
				return;
			}
		}
		// Saving CPU specific counters.
		for (stat = 0; stat < CPU_RESULTS; stat++) {
#if PERFORMANCE_DETAILS || _DETAILS
			cout << "Extracting CPU stat " << stat << " for CPU " << cpu_name << endl;
#endif
			raw_cpu_data[cpu][stat][snapshot] = Extract_Counter(&(cpu_perf_counter_info[stat]));
		}
	}
}

//
// Extracting counters for NT network performance data.
//
void Performance::Extract_TCP_Counters(int snapshot)
{
	int stat;

	// Saving network TCP specific counters.
	if (!Locate_Perf_Instance())
		return;
	for (stat = 0; stat < TCP_RESULTS; stat++) {
#if PERFORMANCE_DETAILS || _DETAILS
		cout << "Extracting TCP stat " << stat << endl;
#endif
		raw_tcp_data[stat][snapshot] = Extract_Counter(&(tcp_perf_counter_info[stat]));
	}
}

//
// Extracting counters for NT network interface performance data.
//
void Performance::Extract_NI_Counters(int snapshot)
{
	int stat;

	// Automatically setting the number of network interfaces that data is available for.
	network_interfaces = 0;
	do {
		// Find the desired network NI interface instance.
		if (!Locate_Perf_Instance(network_interfaces))
			return;

		// Saving network NI specific counters.
		for (stat = 0; stat < NI_RESULTS; stat++) {
#if PERFORMANCE_DETAILS || _DETAILS
			cout << "Extracting NI stat " << stat << " for NI " << network_interfaces << endl;
#endif
			raw_ni_data[network_interfaces][stat][snapshot] =
			    Extract_Counter(&(ni_perf_counter_info[stat]));
		}
		network_interfaces++;
	} while (network_interfaces < perf_object->NumInstances);
}

//
// Extracting and returning a performance counter.  Perf_counter must already be set to a valid
// performance counter block by calling Locate_Perf_Instance.
//
_int64 Performance::Extract_Counter(const Perf_Counter_Info * counter_info)
{
	// Verify that we know where to locate the counter.
	if (counter_info->offset == IOERROR) {
		cout << "*** Unable to extract performance counter, offset not set." << endl << flush;
		return (_int64) 0;
	}

	switch (counter_info->type & PERF_SIZE_MASK) {
	case PERF_SIZE_LARGE:
		return (_int64) ((LARGE_INTEGER *) ((LPBYTE) perf_counters + counter_info->offset))->QuadPart;
	case PERF_SIZE_DWORD:
		return (_int64) * ((DWORD *) ((LPBYTE) perf_counters + counter_info->offset));
	default:		// other counter types exist, but are not currently used - signal error
		cout << "*** Unknown size of performance data." << endl << flush;
	}
	return (_int64) 0;
}

//
// Locating the desired performance object from the returned performance data.
//
BOOL Performance::Locate_Perf_Object(DWORD perf_object_index)
{
	PERF_DATA_BLOCK *pperf_data;	// short-cut pointer to returned performance data
	DWORD object_offset;	// offset to current object
	int i;

	pperf_data = (PERF_DATA_BLOCK *) perf_data;
	object_offset = pperf_data->HeaderLength;

	// Search through all returned objects until we find the desired one.
	for (i = 0; i < (int)pperf_data->NumObjectTypes; i++) {
		perf_object = (PERF_OBJECT_TYPE *) ((LPBYTE) pperf_data + object_offset);
		if (perf_object->ObjectNameTitleIndex == perf_object_index) {
			break;	// This is the object we want.
		} else {
			object_offset += perf_object->TotalByteLength;	// Not it, check next object.
		}
	}

	// If we didn't find the desired object, set the pointer to NULL and signify error.
	// This is normal for performance data disabled by default (e.g. network interface).
	if (i >= (int)pperf_data->NumObjectTypes) {
		perf_object = NULL;
#if PERFORMANCE_DETAILS || defined(_DEBUG)
		cout << "No current performance object to retrieve counters from." << endl
		    << "   Searching for " << perf_object_index << endl << flush;
#endif
		return FALSE;
	}
	return TRUE;
}

//
// Locating the specific instance, base 0, of a performance object and finding its counters.
// You must call Locate_Perf_Object to locate a valid performance object before calling this.
//
BOOL Performance::Locate_Perf_Instance(int instance)
{
	// See if the current performance object supports multiple instances.
	// The performance object should have previously been set by Locate_Perf_Object.
	if (perf_object->NumInstances == PERF_NO_INSTANCES) {
		// Verify that we were not expecting multiple instances to be supported.
		if (instance != PERF_NO_INSTANCES) {
			cout << "*** One performance instance was found, but more were expected." << endl << flush;
			return FALSE;
		}
		// Only one instance supported, just set a pointer to the performance counters.
		perf_instance = NULL;
		perf_counters = (PERF_COUNTER_BLOCK *) ((LPBYTE) perf_object + perf_object->DefinitionLength);
	} else {
		// Verify that we were expecting to find multiple instances.
		if (instance == PERF_NO_INSTANCES) {
			cout << "*** Multiple performance instances were found, but not expected." << endl << flush;
			return FALSE;
		}
		// Multiple instances supported.  Locate the counters to the correct one.
		// Get a pointer to the first instance.
		perf_instance = (PERF_INSTANCE_DEFINITION *) ((LPBYTE) perf_object + perf_object->DefinitionLength);

		// Walk through instances until we find the one we want.
		for (int i = 0; i < instance; i++) {
			perf_counters = (PERF_COUNTER_BLOCK *) ((LPBYTE) perf_instance + perf_instance->ByteLength);
			perf_instance =
			    (PERF_INSTANCE_DEFINITION *) ((LPBYTE) perf_counters + perf_counters->ByteLength);
		}
		perf_counters = (PERF_COUNTER_BLOCK *) ((LPBYTE) perf_instance + perf_instance->ByteLength);
	}
	return TRUE;
}

//
// Setting the offsets and size of the specific performance counters.
// You must call Locate_Perf_Object before calling this function to locate a
// valid performance object.
//
BOOL Performance::Set_Counter_Info(DWORD perf_data_type)
{
	PERF_COUNTER_DEFINITION *counter_def;	// pointer to a specific counter definition
	int number_of_counters;	// number of counters to find offsets for
	int counter;		// number current counter from returned list
	Perf_Counter_Info *counter_info;	// pointer to array of counter information

	// Setting pointers to specific counters based on type of performance data returned.
	// This is used to keep the code for finding the offsets (see below) more general.
	switch (perf_data_type) {
	case PERF_PROCESSOR:
		// Find offsets for processor specific data.
		number_of_counters = CPU_RESULTS;
		counter_info = (Perf_Counter_Info *) (&(cpu_perf_counter_info[0]));
		break;
	case PERF_NETWORK_TCP:
		number_of_counters = TCP_RESULTS;
		counter_info = (Perf_Counter_Info *) (&(tcp_perf_counter_info[0]));
		break;
	case PERF_NETWORK_INTERFACE:
		number_of_counters = NI_RESULTS;
		counter_info = (Perf_Counter_Info *) (&(ni_perf_counter_info[0]));
		break;
	default:
		cout << "*** Unknown performance data requested: " << perf_data_type << endl << flush;
		return FALSE;
	}

	// Set a pointer to the first counter defition.
	// This uses the performance object previously found by Locate_Perf_Object.
	counter_def = (PERF_COUNTER_DEFINITION *) ((LPBYTE) perf_object + perf_object->HeaderLength);

	// Check all returned counters and match to desired ones.
	for (counter = 0; counter < (int)perf_object->NumCounters; counter++) {
		// Walk through list of desired counters to find offsets for them.
		for (int i = 0; i < number_of_counters; i++) {
			if (counter_def->CounterNameTitleIndex == counter_info[i].index) {
				// We found the counter that we're looking for.  Record needed information.
				counter_info[i].offset = counter_def->CounterOffset;
				counter_info[i].type = counter_def->CounterType;
				break;
			}
		}
		// Move to the next counter  definition.
		counter_def = (PERF_COUNTER_DEFINITION *) ((LPBYTE) counter_def + counter_def->ByteLength);
	}
	return TRUE;
}

double Performance::Calculate_Stat(_int64 start_value, _int64 end_value, DWORD counter_type)
{
	double count_difference;	// Difference between two snapshots of a counter.
	double perf_stat;	// Calculated performance statistic, such as % utilization.

	// Verify valid performance time.
	if (perf_time <= 0) {
//
// *** Removed:  Need to add back in if using getTickCount.
//
//#ifndef WIN64_COUNTER_WORKAROUND
//              cout << "*** Performance time not positive." << endl;
//#endif
//
// *** End Removed.
//
		return (double)0.0;
	}
	// Determine what type of counter we're dealing with.
	switch (counter_type & PERF_TYPE_MASK) {

		// Dealing with a number (e.g. error count)
	case PERF_TYPE_NUMBER:
#if PERFORMANCE_DETAILS
		cout << "Performing calculation on a performance number." << endl;
#endif
		switch (counter_type & PERF_SUBTYPE_MASK) {
		case PERF_NUMBER_DECIMAL:
#if PERFORMANCE_DETAILS
			cout << "Performance number decimal." << endl;
#endif
			perf_stat = (double)(end_value - start_value);
			break;
		case PERF_NUMBER_DEC_1000:	// divide result by 1000 before displaying
#if PERFORMANCE_DETAILS
			cout << "Performance number decimal x 1000." << endl;
#endif
			perf_stat = (double)(end_value - start_value) / (double)1000.0;
			break;
			// other subtypes exist, but are not handled
		default:
			cout << "*** Performance counter number subtype not handled" << endl;
			return (double)0.0;
		}
		break;

		// Dealing with an actual counter value.
	case PERF_TYPE_COUNTER:
#if PERFORMANCE_DETAILS
		cout << "Performing calculation on a performance counter." << endl;
#endif

		// Computing the difference between the two counters.
		if (counter_type & PERF_DELTA_COUNTER) {
#if PERFORMANCE_DETAILS
			cout << "Performance delta counter." << endl;
#endif
			count_difference = (double)(end_value - start_value);
		} else {
			cout << "*** Unhandled performance counter delta type." << endl;
			return (double)0.0;
		}

		// Determine the calculated value based on the counter's subtype.
		switch (counter_type & PERF_SUBTYPE_MASK) {

			// Divide the counter by the time difference
		case PERF_COUNTER_RATE:
#if PERFORMANCE_DETAILS
			cout << "Performance counter is a rate." << endl;
#endif

			// Adjust the time based on the counter's update frequency.
			switch (counter_type & PERF_TIMER_MASK) {
				// Timer is based on the system timer, just use it.
			case PERF_TIMER_TICK:
#if PERFORMANCE_DETAILS
				cout << "Using system timer for update frequency." << endl;
#endif
				perf_stat = count_difference / perf_time;
				break;

				// These timers are based on a 100-ns timer, so multiply timer by 10,000,000 to get seconds.
			case PERF_TIMER_100NS:
#if PERFORMANCE_DETAILS
				cout << "Using 100-ns timer for update frequency." << endl;
#endif
				perf_stat = count_difference / (perf_time * (double)10000000.0);
				break;

				// other timer rates exist, but are not handled.
			default:
				cout << "*** Performance update frequency type not handled." << endl;
				return (double)0.0;
			}

			break;
		default:
			cout << "*** Unhandled performance counter, counter subtype." << endl;
			return (double)0.0;
		}

		break;

		// other types exist, but are not handled
	default:
		cout << "*** Unhandled performance counter type." << endl;
		return (double)0.0;
	}

	// Handle any inverted performance counters.
	if (counter_type & PERF_INVERSE_COUNTER) {
#if PERFORMANCE_DETAILS
		cout << "Inverting performance counter (1 - value)." << endl;
#endif
		perf_stat = (double)1.0 - perf_stat;
	}
	// Perform any additional operations needed based on the counter's display suffix.
	switch (counter_type & PERF_SUFFIX_MASK) {
	case PERF_DISPLAY_NO_SUFFIX:	// just return value
	case PERF_DISPLAY_PER_SEC:	// no additional calculations needed
		return perf_stat;

	case PERF_DISPLAY_PERCENT:	// scale by 100 to reflect a percentage
#if PERFORMANCE_DETAILS
		cout << "Performance counter is a percent." << endl;
#endif
		// Verify valid values.  
		// Do not print an error message unless it's more than trivially invalid.
		if (perf_stat < (double)0.0) {
			if (perf_stat < (double)-0.001) {
				cout << "*** Performance counter percentage is less than zero: " << perf_stat << endl;
			}
			return (double)0.0;
		} else if (perf_stat > (double)1.0) {
			if (perf_stat > (double)1.001) {
				cout << "*** Performance counter percentage is greater than 1: " << perf_stat << endl;
			}
			return (double)100.0;
		}

		return (perf_stat * (double)100.0);

	default:
		cout << "*** Unhandled performance display suffix." << endl;
		return (double)0.0;
	}
}
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE)
 // nop
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif

#endif				// !IOMTR_OS_LINUX || !IOMTR_OS_NETWARE || !MTR_OS_OSX
