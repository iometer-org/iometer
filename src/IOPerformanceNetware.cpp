/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / IOPerformanceNetware.cpp                                  ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: The Netware variant of the Performance class.         ## */
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
/* ##  Changes ...: 2005-04-07 (thayneharmon@users.sourceforge.net)       ## */
/* ##               - Added code for doing LAN performance stuff for      ## */
/* ##                 an intel request.                                   ## */
/* ##               2004-mm-dd (thayneharmon@users.sourceforge.net)       ## */
/* ##               - Initial code.                                       ## */
/* ##                                                                     ## */
/* ######################################################################### */
#define PERFORMANCE_DETAILS	0	// Turn on to display additional performance messages.
#if defined(IOMTR_OS_NETWARE)

#include <assert.h>
#include "IOPerformance.h"
#include <monitor.h>

//
// Initializing system performance data.
//
Performance::Performance()
{
	// Obtaining the number of CPUs in the system and their speed.
	processor_count = Get_Processor_Count();
	timer_resolution = Get_Timer_Resolution();
	// Network performance statistics are disabled by default.  Assume this
	// unless later performance calls succeed.  Then, set the correct number of
	// interfaces.
	network_interfaces = 0;

	if (!timer_resolution || !processor_count) {
		cout << "*** Unable to initialize needed performance data.\n"
		    << "This error may indicate that you are trying to run on an\n"
		    << "unsupported processor or OS.  See the Iometer User's Guide for\n"
		    << "information on supported platforms.\n";
		exit(1);
	}
	// Set the clock ticks per second
	clock_tick = GetTimerMinorTicksPerSecond();

	// Initialize all the arrays to 0.
	memset(raw_cpu_data, 0, (MAX_CPUS * CPU_RESULTS * MAX_SNAPSHOTS * sizeof(_int64)));
	memset(raw_ni_data, 0, (MAX_NUM_INTERFACES * NI_RESULTS * MAX_SNAPSHOTS * sizeof(_int64)));
	memset(raw_tcp_data, 0, (TCP_RESULTS * MAX_SNAPSHOTS * sizeof(_int64)));
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
	return NXGetCpuCount();
}

// Note: We just take the first CPU we find and return its CPU speed.
double Performance::Get_Timer_Resolution()
{
	double speed = 0.0;
	struct cpu_info info;
	int seq = 0;

	netware_cpu_info(&info, &seq);
	speed = (double)(info.Speed * 1000000.0);	// in MHz

	return speed;
}

void Performance::Get_Perf_Data(DWORD perf_data_type, int snapshot)
{
	NXTime_t NXTime;

	// Get the performance data stored by the system.
#ifdef _DEBUG
	cout << "   Getting system performance data." << endl << flush;
#endif

//      NXGetTime(NX_SINCE_BOOT, NX_MSECONDS, &NXTime); //NX_TICKS
	NXGetTime(NX_SINCE_BOOT, NX_NSECONDS, &NXTime);	//NX_NSECONDS
	time_counter[snapshot] = (__int64) NXTime;
	if (snapshot == LAST_SNAPSHOT) {
		// calculate time diff in clock ticks..
		timediff = (time_counter[LAST_SNAPSHOT] - time_counter[FIRST_SNAPSHOT]);
	}

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

void Performance::Get_CPU_Counters(int snapshot)
{
	int i, j;

	for (i = j = 0; i < processor_count; i++) {
		netware_cpu_info(&info, &j);	// j is auto-incremented by the call itself 
		raw_cpu_data[i][CPU_USER_UTILIZATION][snapshot] = info.ProcessorUtilization;
		raw_cpu_data[i][CPU_TOTAL_UTILIZATION][snapshot] = info.ProcessorUtilization;
		raw_cpu_data[i][CPU_IRQ][snapshot] = 1000;
	}
}

void Performance::Get_TCP_Counters(int snapshot)
{
	long rc = 0;
	LONG blkNumber = 0;
	typedef struct CommonLANStructure {
		LONG notSupportedMask;
		LONG TotalTxPacketCount;
		LONG TotalRxPacketCount;
		LONG NoECBAvailableCount;
		LONG PacketTxTooBigCount;
		LONG PacketTxTooSmallCount;
		LONG PacketRxOverflowCount;
		LONG PacketRxTooBigCount;
		LONG PacketRxTooSmallCount;
		LONG PacketTxMiscErrorCount;
		LONG PacketRxMiscErrorCount;
		LONG RetryTxCount;
		LONG ChecksumErrorCount;
		LONG HardwareRxMismatchCount;
		/* to here */
		LONG TotalTxOKByteCountLow;
		LONG TotalTxOKByteCountHigh;
		LONG TotalRxOKByteCountLow;
		LONG TotalRxOKByteCountHigh;
		LONG TotalGroupAddrTxCount;
		LONG TotalGroupAddrRxCount;
		LONG AdapterResetCount;
		LONG AdapterOprTimeStamp;
		LONG AdapterQueDepth;
		LONG MediaSpecificCounter1;
		LONG MediaSpecificCounter2;
		LONG MediaSpecificCounter3;
		LONG MediaSpecificCounter4;
		LONG MediaSpecificCounter5;
		LONG MediaSpecificCounter6;
		LONG MediaSpecificCounter7;
		LONG MediaSpecificCounter8;
		LONG MediaSpecificCounter9;
		LONG MediaSpecificCounter10;
		LONG ValidMask1;
		LONG MediaSpecificCounter11;
		LONG MediaSpecificCounter12;
		LONG MediaSpecificCounter13;
		LONG MediaSpecificCounter14;
	} CommonLANStructure;

	typedef struct GetLANCommonCountersStructure {
		LONG currentServerTime;
		BYTE vConsoleVersion;
		BYTE vConsoleRevision;
		BYTE statMajorVersion;
		BYTE statMinorVersion;
		LONG totalCommonCnts;
		LONG totalCntBlocks;
		LONG customCounters;
		LONG nextCntBlock;
		CommonLANStructure info;
	} GetLANCommonCountersStructure;
	GetLANCommonCountersStructure lanInfo;

	for (network_interfaces = 1; network_interfaces < MAX_NUM_INTERFACES; network_interfaces++) {
		blkNumber = 0;
//              while(1)
		if ((rc =
		     SSGetLANCommonCounters((LONG) network_interfaces, blkNumber, (BYTE *) & lanInfo,
					    sizeof(GetLANCommonCountersStructure))) == 0) {
			raw_tcp_data[TCP_SEGMENTS_RESENT][snapshot] = lanInfo.info.RetryTxCount;

//                      if (lanInfo.nextCntBlock == 0)
//                              break;
//                      else
//                              blkNumber = lanInfo.nextCntBlock;
		} else {
			//                    cout << "Performance::Get_TCP_Counters: Error (" << rc << ") in getting stats on board " << network_interfaces << endl;
			break;
		}
//              if (rc != 0) break;
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
				result =
				    (double)((raw_cpu_data[cpu][stat][LAST_SNAPSHOT] +
					      raw_cpu_data[cpu][stat][FIRST_SNAPSHOT]) / 2.0);
				cpu_results->CPU_utilization[cpu][stat] = result;
			} else {
				// All other CPU statistics.
				cpu_results->CPU_utilization[cpu][stat] =
				    (double)((raw_cpu_data[cpu][stat][LAST_SNAPSHOT] +
					      raw_cpu_data[cpu][stat][FIRST_SNAPSHOT]) / 2);
			}

#if PERFORMANCE_DETAILS || _DETAILS
			cout << "CPU " << cpu << " recorded stat " << stat << " = "
			    << (long long)cpu_results->CPU_utilization[cpu][stat] << endl;
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

		result = ((double)raw_tcp_data[stat][LAST_SNAPSHOT] -
			  raw_tcp_data[stat][FIRST_SNAPSHOT]) * 1000000000.0 /
		    ((double)time_counter[LAST_SNAPSHOT] - time_counter[FIRST_SNAPSHOT]);

//              result = ((double) raw_tcp_data[stat][LAST_SNAPSHOT] - raw_tcp_data[stat][FIRST_SNAPSHOT]) / timediff;
//              result *= clock_tick;           // note that timediff is in CLK_TCKs and not seconds
		net_results->tcp_stats[stat] = result;

#if PERFORMANCE_DETAILS || _DETAILS
		cout << "TCP recorded stat " << stat << " = " << net_results->tcp_stats[stat] << endl;
#endif
	}
}

//
// Extracting counters for NT network interface performance data. This is done
//////////////////////////////////////////////////////////////////////
#define NET_IF_TO_IGNORE "ians"

void Performance::Get_NI_Counters(int snapshot)
{
	typedef struct CommonLANStructure {
		LONG notSupportedMask;
		LONG TotalTxPacketCount;
		LONG TotalRxPacketCount;
		LONG NoECBAvailableCount;
		LONG PacketTxTooBigCount;
		LONG PacketTxTooSmallCount;
		LONG PacketRxOverflowCount;
		LONG PacketRxTooBigCount;
		LONG PacketRxTooSmallCount;
		LONG PacketTxMiscErrorCount;
		LONG PacketRxMiscErrorCount;
		LONG RetryTxCount;
		LONG ChecksumErrorCount;
		LONG HardwareRxMismatchCount;
		/* to here */
		LONG TotalTxOKByteCountLow;
		LONG TotalTxOKByteCountHigh;
		LONG TotalRxOKByteCountLow;
		LONG TotalRxOKByteCountHigh;
		LONG TotalGroupAddrTxCount;
		LONG TotalGroupAddrRxCount;
		LONG AdapterResetCount;
		LONG AdapterOprTimeStamp;
		LONG AdapterQueDepth;
		LONG MediaSpecificCounter1;
		LONG MediaSpecificCounter2;
		LONG MediaSpecificCounter3;
		LONG MediaSpecificCounter4;
		LONG MediaSpecificCounter5;
		LONG MediaSpecificCounter6;
		LONG MediaSpecificCounter7;
		LONG MediaSpecificCounter8;
		LONG MediaSpecificCounter9;
		LONG MediaSpecificCounter10;
		LONG ValidMask1;
		LONG MediaSpecificCounter11;
		LONG MediaSpecificCounter12;
		LONG MediaSpecificCounter13;
		LONG MediaSpecificCounter14;
	} CommonLANStructure;

	typedef struct GetLANCommonCountersStructure {
		LONG currentServerTime;
		BYTE vConsoleVersion;
		BYTE vConsoleRevision;
		BYTE statMajorVersion;
		BYTE statMinorVersion;
		LONG totalCommonCnts;
		LONG totalCntBlocks;
		LONG customCounters;
		LONG nextCntBlock;
		CommonLANStructure info;
	} GetLANCommonCountersStructure;

	LONG blkNumber = 0;
	long rc;
	GetLANCommonCountersStructure lanInfo;

	for (network_interfaces = 1; network_interfaces < MAX_NUM_INTERFACES; network_interfaces++) {
		blkNumber = 0;
//              while(1)
		if ((rc =
		     SSGetLANCommonCounters((LONG) network_interfaces, blkNumber, (BYTE *) & lanInfo,
					    sizeof(GetLANCommonCountersStructure))) == 0) {
			raw_ni_data[network_interfaces][NI_IN_ERRORS][snapshot] = lanInfo.info.PacketRxMiscErrorCount;
			raw_ni_data[network_interfaces][NI_OUT_ERRORS][snapshot] = lanInfo.info.PacketTxMiscErrorCount;
			raw_ni_data[network_interfaces][NI_PACKETS][snapshot] =
			    (lanInfo.info.TotalRxPacketCount + lanInfo.info.TotalTxPacketCount);

//                      printf("\rGet_NI_Counters: blk:%x PacketRxMiscErrorCount:%x, PacketTxMiscErrorCount:%x, TotalRxPacketCount:%x, TotalTxPacketCount:%x\n", 
//                              lanInfo.nextCntBlock,
//                              lanInfo.info.PacketRxMiscErrorCount, lanInfo.info.PacketTxMiscErrorCount,
//                              lanInfo.info.TotalRxPacketCount, lanInfo.info.TotalTxPacketCount);

//                      if (lanInfo.nextCntBlock == 0)
//                              break;
//                      else
//                              blkNumber = lanInfo.nextCntBlock;
		} else {
//                      cout << "Performance::Get_NI_Counters: Error (" << rc << ") in getting stats on board " << network_interfaces << endl;
			break;
		}
//              if (rc != 0) break;
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
	for (net = 1; net < network_interfaces; net++) {
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

//                      #if PERFORMANCE_DETAILS || _DETAILS
			cout << "   Network interface " << net << " recorded stat " << stat << " = "
			    << (long long)net_results->ni_stats[net][stat] << endl;
//                      #endif
		}
	}
}

#endif				/* NETWARE */
