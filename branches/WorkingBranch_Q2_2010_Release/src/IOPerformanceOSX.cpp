/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / IOPerformanceOSX.cpp                                      ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: The Mac OSX variant of the Performance class.         ## */
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
/* ##  Changes ...: 2005-03-09 (raltherr@apple.com)                       ## */
/* ##                - Created                                            ## */
/* ##                                                                     ## */
/* ######################################################################### */

#if defined(IOMTR_OS_OSX)

#include "IOPerformance.h"
#include <mach/mach_host.h>
#include <mach/vm_map.h>
#include <Carbon/Carbon.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/socket.h>
#include <net/route.h>
#include <net/if.h>
#include <net/if_dl.h>

//
// Initializing system performance data.
//
Performance::Performance()
{
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

	char *sysctlData = NULL, *sysctlDataEnd, *sysctlDataCurrent;
	struct if_msghdr *ifmsg;
	struct sockaddr_dl *sdl;
	size_t sysctlExpectedSize;
	int mib[] = { CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST, 0 };

	// Retrieve number of clock ticks per second (should be 100)
	clock_tick = sysconf(_SC_CLK_TCK);

	memset(raw_cpu_data, 0, (MAX_CPUS * CPU_RESULTS * MAX_SNAPSHOTS * sizeof(_int64)));
	memset(raw_ni_data, 0, (MAX_NUM_INTERFACES * NI_RESULTS * MAX_SNAPSHOTS * sizeof(_int64)));
	memset(raw_tcp_data, 0, (TCP_RESULTS * MAX_SNAPSHOTS * sizeof(_int64)));

	// Need to load nic_names[] with the names of our network interfaces

	// Retrieve the amount of space we need for the routing table
	sysctl(mib, 6, NULL, &sysctlExpectedSize, NULL, 0);
	// Allocate that space
	sysctlData = (char *)malloc(sysctlExpectedSize);
	// Retrieve the routing table information
	sysctl(mib, 6, sysctlData, &sysctlExpectedSize, NULL, 0);

	sysctlDataEnd = sysctlData + sysctlExpectedSize;
	sysctlDataCurrent = sysctlData;

	// Walk through the routing table and find the network interface names
	while (sysctlDataCurrent < sysctlDataEnd) {
		ifmsg = (struct if_msghdr *)sysctlDataCurrent;
		if (ifmsg->ifm_type == RTM_IFINFO) {
			if (!(ifmsg->ifm_flags & IFF_LOOPBACK)) {
				sdl = (struct sockaddr_dl *)(ifmsg + 1);
				if (sdl->sdl_family == AF_LINK && ifmsg->ifm_flags & IFF_UP) {
					strncpy(nic_names[network_interfaces], sdl->sdl_data, sdl->sdl_nlen);
					nic_names[network_interfaces][sdl->sdl_nlen] = '\0';
					network_interfaces++;
				}
			}
		}
		sysctlDataCurrent += ifmsg->ifm_msglen;
	}

	free(sysctlData);
}

//
// Freeing memory used by class object.
//
Performance::~Performance()
{
}

//
// Getting the number of processors in the system.
//
int Performance::Get_Processor_Count()
{
	int i;
	size_t intsize = sizeof(int);

	sysctlbyname("hw.ncpu", &i, &intsize, NULL, 0);
	return i;
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

	quad_t freq;
	size_t intsize = sizeof(quad_t);

	sysctlbyname("hw.cpufrequency", &freq, &intsize, NULL, 0);

	// hw.cpufrequency is in Hz, convert to MHz
	speed = freq / 1000000;

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

void Performance::Get_Perf_Data(DWORD perf_data_type, int snapshot)
{
	// Get the performance data stored by the system.
#ifdef _DEBUG
	cout << "   Getting system performance data." << endl << flush;
#endif

	AbsoluteTime blah;
	Nanoseconds s;

	// Uptime() returns number of timebase ticks since poweron
	blah = UpTime();
	// It is easily converted to nanoseconds since poweron
	s = AbsoluteToNanoseconds(blah);
	time_counter[snapshot] = s.hi;
	time_counter[snapshot] <<= 32;
	time_counter[snapshot] += s.lo;

	if (snapshot == LAST_SNAPSHOT)
		// Calculate time diff in clock ticks..
		timediff =
		    ((double)time_counter[LAST_SNAPSHOT] -
		     time_counter[FIRST_SNAPSHOT]) * clock_tick / (double)1000000000;

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
		double result;

		result = ((double)raw_tcp_data[stat][LAST_SNAPSHOT] - raw_tcp_data[stat][FIRST_SNAPSHOT]) / timediff;
		result *= clock_tick;	// note that timediff is in CLK_TCKs and not seconds
		net_results->tcp_stats[stat] = result;

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

#if PERFORMANCE_DETAILS || _DETAILS
			cout << "   Network interface " << net << " recorded stat " << stat << " = "
			    << net_results->ni_stats[net][stat] << endl;
#endif
		}
	}
}

void Performance::Get_CPU_Counters(int snapshot)
{
	unsigned int current_cpu = 0;
	host_name_port_t host;
	processor_cpu_load_info_t load;
	kern_return_t kr;
	unsigned int procs;
	mach_msg_type_number_t load_count;

	host = mach_host_self();

	kr = host_processor_info(host, PROCESSOR_CPU_LOAD_INFO, &procs, (processor_info_array_t *) & load, &load_count);
	if (kr != KERN_SUCCESS) {
		raw_cpu_data[0][CPU_TOTAL_UTILIZATION][snapshot] = 1;
		raw_cpu_data[0][CPU_USER_UTILIZATION][snapshot] = 1;
		raw_cpu_data[0][CPU_PRIVILEGED_UTILIZATION][snapshot] = 1;
		raw_cpu_data[0][CPU_IRQ_UTILIZATION][snapshot] = 1;
		raw_cpu_data[0][CPU_IRQ][snapshot] = 1;
	} else {
		for (current_cpu = 0; current_cpu < procs; current_cpu++) {
			raw_cpu_data[current_cpu][CPU_TOTAL_UTILIZATION][snapshot] =
			    load[current_cpu].cpu_ticks[CPU_STATE_USER] +
			    load[current_cpu].cpu_ticks[CPU_STATE_SYSTEM] + load[current_cpu].cpu_ticks[CPU_STATE_NICE];
			raw_cpu_data[current_cpu][CPU_USER_UTILIZATION][snapshot] =
			    load[current_cpu].cpu_ticks[CPU_STATE_USER] + load[current_cpu].cpu_ticks[CPU_STATE_NICE];
			raw_cpu_data[current_cpu][CPU_PRIVILEGED_UTILIZATION][snapshot] =
			    load[current_cpu].cpu_ticks[CPU_STATE_NICE];
			raw_cpu_data[current_cpu][CPU_IRQ_UTILIZATION][snapshot] = 1;
			raw_cpu_data[current_cpu][CPU_IRQ][snapshot] = 1;
		}
		vm_deallocate(mach_task_self(), (vm_address_t) load, (vm_size_t) (load_count * sizeof(*load)));
	}
	return;
}

void Performance::Get_NI_Counters(int snapshot)
{
	char *sysctlData = NULL, *sysctlDataEnd, *sysctlDataCurrent, ifname[IF_NAMESIZE];
	struct if_msghdr *ifmsg;
	struct sockaddr_dl *sdl;
	size_t sysctlExpectedSize;
	int mib[] = { CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST, 0 };
	int current_nic = 0;

	if (sysctl(mib, 6, NULL, &sysctlExpectedSize, NULL, 0) < 0) {
		for (current_nic = 0; current_nic < network_interfaces; current_nic++) {
			raw_ni_data[current_nic][NI_PACKETS][snapshot] = 1;
			raw_ni_data[current_nic][NI_ERRORS][snapshot] = 1;
			raw_ni_data[current_nic][NI_IN_ERRORS][snapshot] = 1;
		}
		return;
	}

	if ((sysctlData = (char *)malloc(sysctlExpectedSize)) == NULL) {
		for (current_nic = 0; current_nic < network_interfaces; current_nic++) {
			raw_ni_data[current_nic][NI_PACKETS][snapshot] = 1;
			raw_ni_data[current_nic][NI_ERRORS][snapshot] = 1;
			raw_ni_data[current_nic][NI_IN_ERRORS][snapshot] = 1;
		}
		return;
	}

	if (sysctl(mib, 6, sysctlData, &sysctlExpectedSize, NULL, 0) < 0) {
		for (current_nic = 0; current_nic < network_interfaces; current_nic++) {
			raw_ni_data[current_nic][NI_PACKETS][snapshot] = 1;
			raw_ni_data[current_nic][NI_ERRORS][snapshot] = 1;
			raw_ni_data[current_nic][NI_IN_ERRORS][snapshot] = 1;
		}
		free(sysctlData);
		return;
	}

	sysctlDataEnd = sysctlData + sysctlExpectedSize;
	sysctlDataCurrent = sysctlData;

	while (sysctlDataCurrent < sysctlDataEnd) {
		ifmsg = (struct if_msghdr *)sysctlDataCurrent;
		if (ifmsg->ifm_type == RTM_IFINFO && !(ifmsg->ifm_flags & IFF_LOOPBACK)) {
			sdl = (struct sockaddr_dl *)(ifmsg + 1);
			if (sdl->sdl_family == AF_LINK && ifmsg->ifm_flags & IFF_UP) {
				strncpy(ifname, sdl->sdl_data, sdl->sdl_nlen);
				ifname[sdl->sdl_nlen] = '\0';
				for (current_nic = 0; current_nic < network_interfaces; current_nic++) {
					if (strcmp(ifname, nic_names[current_nic]))
						continue;
					raw_ni_data[current_nic][NI_PACKETS][snapshot] =
					    ifmsg->ifm_data.ifi_ipackets + ifmsg->ifm_data.ifi_opackets;
					raw_ni_data[current_nic][NI_ERRORS][snapshot] =
					    ifmsg->ifm_data.ifi_ierrors + ifmsg->ifm_data.ifi_oerrors;
					raw_ni_data[current_nic][NI_IN_ERRORS][snapshot] = ifmsg->ifm_data.ifi_ierrors;
					break;
				}
			}
		}
		sysctlDataCurrent += ifmsg->ifm_msglen;
	}

	free(sysctlData);
	return;
}

void Performance::Get_TCP_Counters(int snapshot)
{
	raw_tcp_data[TCP_SEGMENTS_RESENT][snapshot] = 1;
	return;
}

#endif
