/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Dynamo) / ByteOrder.cpp                                           ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Contains the code which is needed to transform the    ## */
/* ##               messages - passed between Iometer and Dynamo - from   ## */
/* ##               big to little endian.                                 ## */
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
/* ##  Changes ...: 2004-03-05 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved the Dump_*_Results() functions (used for      ## */
/* ##                 debugging purposes) from IOManager.cpp to here.     ## */
/* ##               - Moved the *_double_swap() functions for the         ## */
/* ##                 Linux/XScale combination from IOManager.cpp         ## */
/* ##                 to here.                                            ## */
/* ##               2004-02-16 (mingz@ele.uri.edu)                        ## */
/* ##               - Added a special double-precision number word        ## */
/* ##                 swapping code for ARM architecture here. A 2 word   ## */
/* ##                 double number is same order in word but different   ## */
/* ##                 order between two words compared with IA32.         ## */
/* ##               2003-10-15 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               2003-07-27 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Removed the [BIG|LITTLE]_ENDIAN_ARCH defines.       ## */
/* ##               - Cleanup the formatting.                             ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include "IOCommon.h"
#include "IOTest.h"
#include "IOMessage.h"

inline void rotate(char *ptr, int size);
template < class T > inline void reorder(T &);
inline void reorder(CPU_Results &, int);
inline void reorder(Net_Results &, int);
inline void reorder(Raw_Result &);
void reorder(Message &);
void reorder(Data_Message &, int union_type, int send_recv);

#if defined(_DEBUG)
void Dump_CPU_Results(struct CPU_Results *res)
{
	if (!res)
		return;
	cout << " CPU Counters:";
	cout << "   " << res->CPU_utilization[0][CPU_TOTAL_UTILIZATION];
	cout << "   " << res->CPU_utilization[0][CPU_USER_UTILIZATION];
	cout << "   " << res->CPU_utilization[0][CPU_PRIVILEGED_UTILIZATION];
	cout << "   " << res->CPU_utilization[0][CPU_IRQ];
	cout << endl;
}

void Dump_Net_Results(struct Net_Results *res)
{
	if (!res)
		return;
	cout << " Net Counters:";
	cout << "   " << res->tcp_stats[TCP_SEGMENTS_RESENT];
	cout << "   " << res->ni_stats[0][NI_PACKETS];
	cout << "   " << res->ni_stats[0][NI_OUT_ERRORS];
	cout << "   " << res->ni_stats[0][NI_IN_ERRORS];
	cout << endl;
}

void Dump_Manager_Results(struct Manager_Results *res)
{
	if (!res)
		return;
	cout << "Dump manager result: " << endl;
	cout << " time_counter = " << res->time_counter << endl;
	Dump_CPU_Results(&(res->cpu_results));
	Dump_Net_Results(&(res->net_results));
}

void Dump_Raw_Result(struct Raw_Result *res)
{
	if (!res)
		return;
	cout << "Dump raw result: " << endl;
	cout << "   " << "bytes_read =" << res->bytes_read << endl;
	cout << "   " << "bytes_written =" << res->bytes_written << endl;
	cout << "   " << "read_count =" << res->read_count << endl;
	cout << "   " << "write_count =" << res->write_count << endl;
	cout << "   " << "transaction_count =" << res->transaction_count << endl;
	cout << "   " << "connection_count =" << res->connection_count << endl;
	cout << "   " << "read_errors =" << res->read_errors << endl;
	cout << "   " << "write_errors =" << res->write_errors << endl;
	cout << "   " << "max_raw_read_latency =" << res->max_raw_read_latency << endl;
	cout << "   " << "read_latency_sum =" << res->read_latency_sum << endl;
	cout << "   " << "max_raw_write_latency =" << res->max_raw_write_latency << endl;
	cout << "   " << "write_latency_sum =" << res->write_latency_sum << endl;
	cout << "   " << "max_raw_transaction_latency =" << res->max_raw_transaction_latency << endl;
	cout << "   " << "max_raw_connection_latency =" << res->max_raw_connection_latency << endl;
	cout << "   " << "transaction_latency_sum =" << res->transaction_latency_sum << endl;
	cout << "   " << "connection_latency_sum =" << res->connection_latency_sum << endl;
	cout << "   " << "counter_time =" << res->counter_time << endl;
	cout << "Dump raw result end. " << endl;
}
#endif

#if defined(IOMTR_OS_LINUX) && defined(IOMTR_CPU_XSCALE)
//
//      This can be optimized definitely.
//
/* This is the standard way, leave here for understanding its purpose.
void double_wordswap(double *d)
{
	unsigned char *c;
	unsigned char tmp[4];
	
	c = (unsigned char *)d;
	memcpy(tmp, c, 4);
	memcpy(c, c + 4, 4);
	memcpy(c + 4, tmp, 4);
}
*/

//
// I guess this should be much faster than upper code, am I right?
//
void double_wordswap(double *d)
{
	unsigned long *c, t;

	c = (unsigned long *)d;
	t = *c;
	*c = *(c + 1);
	*(c + 1) = t;
}

//
// ARM can run as little endian or big endian. But even it runs
// at little endian, it has different word order for double-precision
// number.
//
void Manager_Info_double_swap(struct Manager_Info *p)
{
	double_wordswap(&(p->timer_resolution));
}

void Manager_Results_double_swap(struct Manager_Results *p)
{
	CPU_Results_double_swap(&(p->cpu_results));
	Net_Results_double_swap(&(p->net_results));
}

void CPU_Results_double_swap(struct CPU_Results *p)
{
	int i, j;

	for (i = 0; i < MAX_CPUS; i++)
		for (j = 0; j < CPU_RESULTS; j++)
			double_wordswap(&(p->CPU_utilization[i][j]));
}

void Net_Results_double_swap(struct Net_Results *p)
{
	int i, j;

	for (i = 0; i < TCP_RESULTS; i++)
		double_wordswap(&(p->tcp_stats[i]));
	for (i = 0; i < MAX_NUM_INTERFACES; i++)
		for (j = 0; j < NI_RESULTS; j++)
			double_wordswap(&(p->ni_stats[i][j]));
}
#endif

inline void rotate(char *ptr, int size)
{
	int i;
	int sz = size - 1;

	for (i = 0; i < (size / 2); i++) {
		ptr[i] = ptr[i] ^ ptr[sz - i];
		ptr[sz - i] = ptr[i] ^ ptr[sz - i];
		ptr[i] = ptr[i] ^ ptr[sz - i];
	}
	return;
}

template < class T > inline void reorder(T & tref)
{
	(void)rotate((char *)&tref, sizeof(T));
	return;
}

inline void reorder(CPU_Results & var, int send_recv)
{
	int i, j;

	if (send_recv == RECV)
		reorder(var.count);

	for (i = 0; i < var.count; i++)
		for (j = 0; j < CPU_RESULTS; j++)
			reorder(var.CPU_utilization[i][j]);

	if (send_recv == SEND)
		reorder(var.count);

	return;
}

inline void reorder(Net_Results & var, int send_recv)
{
	int i, j;

	if (send_recv == RECV)
		reorder(var.ni_count);

	for (i = 0; i < TCP_RESULTS; i++)
		reorder(var.tcp_stats[i]);

	for (i = 0; i < var.ni_count; i++)
		for (j = 0; j < NI_RESULTS; j++)
			reorder(var.ni_stats[i][j]);

	if (send_recv == SEND)
		reorder(var.ni_count);

	return;
}

void reorder(Raw_Result & var)
{
	reorder(var.bytes_read);
	reorder(var.bytes_written);
	reorder(var.read_count);
	reorder(var.write_count);

	reorder(var.transaction_count);
	reorder(var.connection_count);
	reorder(var.read_errors);
	reorder(var.write_errors);

	reorder(var.max_raw_read_latency);
	reorder(var.read_latency_sum);
	reorder(var.max_raw_write_latency);
	reorder(var.write_latency_sum);

	reorder(var.max_raw_transaction_latency);
	reorder(var.max_raw_connection_latency);
	reorder(var.transaction_latency_sum);
	reorder(var.connection_latency_sum);

	reorder(var.counter_time);

	return;
}

void reorder(Message & var)
{
	(void)reorder(var.purpose);
	(void)reorder(var.data);
	return;
}

void reorder(Data_Message & var, int uniontype, int send_recv)
{
	int i;

	// must be first after a recv.
	if (send_recv == RECV)
		reorder(var.count);

	switch (uniontype) {
	case 0:
		return;
		break;

	case DATA_MESSAGE_MANAGER_INFO:
		// struct Manager_Info
#ifdef _DEBUG
		cout << "Manager_Info count: " << var.count << endl;
#endif
		reorder(var.data.manager_info.port_number);
		reorder(var.data.manager_info.processors);
		reorder(var.data.manager_info.timer_resolution);
		break;

	case DATA_MESSAGE_TARGET_SPEC:
		// struct Target_Spec
#ifdef _DEBUG
		cout << "Target_Spec count: " << var.count << endl;
#endif
		for (i = 0; i < var.count; i++) {
			// must be first after recv.
			if (send_recv == RECV)
				reorder(var.data.targets[i].type);

			// Is target Disk/Network/Vi type ?
			if (IsType(var.data.targets[i].type, GenericDiskType)) {
				reorder(var.data.targets[i].disk_info.sector_size);
				reorder(var.data.targets[i].disk_info.maximum_size);
				reorder(var.data.targets[i].disk_info.starting_sector);
				reorder(var.data.targets[i].DataPattern);
				reorder(var.data.targets[i].use_fixed_seed);
				reorder(var.data.targets[i].fixed_seed_value);
			}

			if (IsType(var.data.targets[i].type, GenericNetType)) {
				reorder(var.data.targets[i].tcp_info.local_port);
				reorder(var.data.targets[i].tcp_info.remote_port);
			}

			if (IsType(var.data.targets[i].type, GenericVIType)) {
#ifdef _DEBUG
				cerr << "WARNING: VI data marshalling not supported yet." << endl;
#endif
			}
			// Now re-order the other stuff.
			reorder(var.data.targets[i].queue_depth);
#ifndef USE_NEW_DISCOVERY_MECHANISM // part of the bitwise char
			reorder(var.data.targets[i].test_connection_rate);
#endif
			reorder(var.data.targets[i].trans_per_conn);
			reorder(var.data.targets[i].random);

			// Must be last before send. Else Sparc Solaris will be confused.
			if (send_recv == SEND)
				reorder(var.data.targets[i].type);
		}
		break;

	case DATA_MESSAGE_TEST_SPEC:
		// struct Test_Spec
#ifdef _DEBUG
		cout << "Test_Spec count: " << var.count << endl;
#endif
		reorder(var.data.spec.default_assignment);
		for (i = 0; i < MAX_ACCESS_SPECS; i++) {
			reorder(var.data.spec.access[i].of_size);
			reorder(var.data.spec.access[i].reads);
			reorder(var.data.spec.access[i].random);
			reorder(var.data.spec.access[i].delay);
			reorder(var.data.spec.access[i].burst);
			reorder(var.data.spec.access[i].align);
			reorder(var.data.spec.access[i].reply);
			reorder(var.data.spec.access[i].size);
		}
		break;

	case DATA_MESSAGE_MANAGER_RESULTS:
		// Manager Results processed here.
#ifdef _DEBUG
		cout << "Test_Spec count: " << var.count << endl;
#endif
		for (i = 0; i < MAX_SNAPSHOTS; i++)
			reorder(var.data.manager_results.time_counter[i]);

		reorder(var.data.manager_results.cpu_results, send_recv);
		reorder(var.data.manager_results.net_results, send_recv);
		break;

	case DATA_MESSAGE_WORKER_RESULTS:
		// Worker Results processed here.
#ifdef _DEBUG
		cout << "Test_Spec count: " << var.count << endl;
#endif
		for (i = 0; i < MAX_SNAPSHOTS; i++)
			reorder(var.data.worker_results.time[i]);

		if (send_recv == RECV)	// never used as of now.
			reorder(var.data.worker_results.target_results.count);

		for (i = 0; i < var.data.worker_results.target_results.count; i++)
			reorder(var.data.worker_results.target_results.result[i]);

		if (send_recv == SEND)
			reorder(var.data.worker_results.target_results.count);
		break;

	default:
		return;
		break;
	}			// end of switch(uniontype)....

	// Must be last before send. Else Sparc Solaris will be confused.
	if (send_recv == SEND)
		reorder(var.count);

	return;
}
