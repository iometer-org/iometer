/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / IOTime.cpp                                                ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implements the timing functions.                      ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Remarks ...: <none>                                                ## */
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
/* ##  Changes ...: 2008-06-18 (vedrand@yahoo.com)                        ## */
/* ##               -redesigned the code to support multiple timers       ## */
/* ##                for Windows - needs work for others (comments below) ## */
/* ##               2005-04-18 (raltherr@apple.com)                       ## */
/* ##               - Support for MacOS X                                 ## */
/* ##               2005-01-12 (henryx.w.tieman@intel.com)                ## */
/* ##               - Added code for Linux on Intel Itanium (ia64).       ## */
/* ##               2004-09-01 (henryx.w.tieman@intel.com)                ## */
/* ##               - The x86_64 architecture can use rdtsc.              ## */
/* ##               - Switched to the more generic IOMTR_CPU_X86_64.      ## */
/* ##               2004-03-27 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Applied Dan Bar Dov's patch for adding              ## */
/* ##                 Linux on PPC support.                               ## */
/* ##               - Code cleanup to ensure common style.                ## */
/* ##               - Applied Thayne Harmon's patch for supporting        ## */
/* ##                 Netware support (on I386).                          ## */
/* ##               2004-02-12 (mingz@ele.uri.edu)                        ## */
/* ##               - Implemented rdtsc under xscale via CCNT.            ## */
/* ##               2004-02-07 (mingz@ele.uri.edu)                        ## */
/* ##               - Changed call from im_kstat to iomtr_kstat           ## */
/* ##               2004-02-06 (mingz@ele.uri.edu)                        ## */
/* ##               - Added ioctl call to get jiffies value from          ## */
/* ##                 im_kstat module;                                    ## */
/* ##               - Added code to get jiffies value from 2.6 /proc/stat ## */
/* ##                 style;                                              ## */
/* ##               2003-10-05 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Integrated the modification contributed by          ## */
/* ##                 Vedran Degoricija, to get the code compile with     ## */
/* ##                 the Windows 64 Bit on AMD64.                        ## */
/* ##               2003-08-02 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Integrated the modification contributed by          ## */
/* ##                 Vedran Degoricija, to get the code compile with     ## */
/* ##                 the MS DDK on IA64.                                 ## */
/* ##               2003-07-19 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Merged IOTime.h stuff into (parts of)               ## */
/* ##               - Implemented the IOMTR_[OSFAMILY|OS|CPU]_* global    ## */
/* ##                 define as well as their integrity checks.           ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-03-04 (joe@eiler.net)                            ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##               - Moved contents of rdtsc.c into here                 ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include "IOCommon.h"

#if defined(IOMTR_OS_LINUX)
#include "iomtr_kstat/iomtr_kstat.h"

extern int kstatfd;
#endif

#if defined(IOMTR_OS_LINUX)
DWORDLONG jiffies(void)
{
	DWORDLONG j[8], jf;
	FILE *fp;
	char tmpBuf[SMLBUFSIZE];
	int i;

	if (kstatfd > 0 && ioctl(kstatfd, IM_IOC_GETCURJIFFIES, (unsigned long long*)&jf) >= 0) {
		return jf;
	}

	for (i = 0; i < 8; i++)
		j[i] = 0;
	fp = fopen("/proc/stat", "r");
	fgets(tmpBuf, SMLBUFSIZE, fp);
	fgets(tmpBuf, SMLBUFSIZE, fp);
	sscanf(tmpBuf, "cpu0 %lld %lld %lld %lld %lld %lld %lld %lld\n", 
			j, j + 1, j + 2, j + 3, j + 4, j + 5, j + 6, j + 7);
	fclose(fp);
	jf = 0;
	for (i = 0; i < 8; i++)
		jf += j[i];
	return jf;
}

#if defined(IOMTR_CPU_I386) || defined(IOMTR_CPU_X86_64)
DWORDLONG timer_value(void)
{
	// Original code (returning the cpu cycle counter)
	unsigned int lo, hi;
	__asm__ __volatile__("rdtsc":"=a"(lo), "=d"(hi));

	return (lo | ((DWORDLONG) hi << 32));
	// Alternative code (returning the cpu cycle counter too)
	//      unsigned long long int x;
	//      __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
	//      return(x);
}
#elif defined(IOMTR_CPU_PPC)
#define CPU_FTR_601                     0x00000100

DWORD get_tbl()
{
	DWORD ccc;
	__asm__ __volatile__("98:    mftb %0\n"
			     "99:\n"
			     ".section __ftr_fixup,\"a\"\n"
			     "       .long %1\n"
			     "       .long 0\n"
			     "       .long 98b\n" "       .long 99b\n" ".previous":"=r"(ccc):"i"(CPU_FTR_601));
	return ccc;
}

DWORD get_tbu()
{
	DWORD ccc;
	__asm__ __volatile__("98:    mftbu %0\n"
			     "99:\n"
			     ".section __ftr_fixup,\"a\"\n"
			     "       .long %1\n"
			     "       .long 0\n"
			     "       .long 98b\n" "       .long 99b\n" ".previous":"=r"(ccc):"i"(CPU_FTR_601));
	return ccc;
}

DWORDLONG timer_value(void)
{
	// Original code (returning the cpu cycle counter)

	// read 64 bit tbl (time base) using motorola example 
	DWORD lo, hi1, hi2;

	do {
		hi1 = get_tbu();
		lo = get_tbl();
		hi2 = get_tbu();
	} while (hi1 != hi2);

	return ((DWORDLONG) hi1) << 32 | (DWORDLONG) lo;
}
#elif defined(IOMTR_CPU_XSCALE)

#define CCNT_IOC_MAGIC		0xAC
#define CCNT_IOC_GETCCNT	_IOR(CCNT_IOC_MAGIC, 1, unsigned long long)
extern int ccntfd;

DWORDLONG timer_value(void)
{
	unsigned long long ccnt;

	if (ioctl(ccntfd, CCNT_IOC_GETCCNT, &ccnt) < 0) {
		ccnt = 0;
	}
	return (ccnt);
}
#elif defined(IOMTR_CPU_IA64)
DWORDLONG timer_value(void)
{
	unsigned long temp;

	/* TODO: If you are going to use the Intel compiler */
	/*       you need a different line of code. */
	__asm__ __volatile__("mov %0=ar.itc":"=r"(temp)::"memory");

	return temp;
}
#else
  // Was the following 2 lines in before, but for which CPU (nevertheless it is useless!)?
  //    /* Totally cheesy rewrite of rdtsc! */
  //    return((DWORDLONG)time(NULL) * 200);
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
// ----------------------------------------------------------------------------
#elif defined(IOMTR_OSFAMILY_NETWARE)
#if defined(IOMTR_CPU_I386)
__declspec(naked)
extern DWORDLONG timer_value()
{
	__asm__ {
		_emit 0Fh	// Store low  32-bits of counter in EAX.
		_emit 31h	// Store high 32-bits of counter in EDX.
		ret
	}
}
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
// ----------------------------------------------------------------------------
#elif defined(IOMTR_OS_SOLARIS)
#if defined(IOMTR_CPU_I386)
unsigned long long timer_value()
{
	asm(".byte 0x0f, 0x31");
}
#elif defined(IOMTR_CPU_SPARC)
#include <sys/types.h>
#include <sys/time.h>
double processor_speed_to_nsecs;
unsigned long long timer_value()
{
	return (DWORDLONG) ((double)gethrtime() * (double)processor_speed_to_nsecs);
}
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
// ----------------------------------------------------------------------------
#elif defined(IOMTR_OSFAMILY_WINDOWS)

#if defined(IOMTR_OS_WIN32) && defined(IOMTR_CPU_I386)

// Only x86 can have the QueryPerfCounter as the default
timer_type TimerType = TIMER_OSHPC;

ULONGLONG rdtsc()
{

	_asm {
		_emit 0Fh	// Store low  32-bits of counter in EAX.
		_emit 31h	// Store high 32-bits of counter in EDX.
		ret
	}
}

// ----------------------------------------------------------------------------
#elif defined(IOMTR_OS_WIN64) && defined(IOMTR_CPU_IA64)

// Define the default timer
timer_type TimerType = TIMER_OSHPC;

//
// DDK definitions for ITC intrinsics and ia64 registers...
//

typedef enum IA64_REG_INDEX {
	// ... Bunch of registers deleted here...
	CV_IA64_ApITC = 3116,	// Interval Time Counter (ITC, AR-44)
	// ... Bunch of registers deleted here...
} IA64_REG_INDEX;



#ifdef __cplusplus
extern "C" {
#endif

#pragma intrinsic (__getReg)
	
	unsigned __int64 __getReg(int);

#ifdef __cplusplus
}
#endif

////////////////////////////////////////////////////////////////////////////////////
//  Name:       readITC
//  Purpose:    To read the IA64 Itanium's Interval Time Counter (ITC, AR-44).  The
//              ITC is equivalent to the IA32 Time Stamp Counter (TSC).  The IA32
//              TSC can be read using the IA32 RDTSC instruction (opcode 0F 31h) but
//              there is no equivalent IA64 instruction to read the ITC.
//  Returns:    The value of the ITC
//  Parameters: None.
/////////////////////////////////////////////////////////////////////////////////////
DWORDLONG rdtsc()
{
	//
	// Should read the Itanium's Interval Time Counter (ITC - AR44).
	// This is equivalent to the IA32 Time Stamp Counter (TSC) that is read by
	// the IA32 RDTSC instruction (opcode 0F 31h)
	//
	// __getReg is a compiler intrinsic defined in 'wdm.h' of the DDK.
	// defined.  CV_IA64_ApITC is defined in 'ia64reg.h' in the DDK.
	//
	return __getReg(CV_IA64_ApITC);

}

// ----------------------------------------------------------------------------
#elif defined(IOMTR_OS_WIN64) && defined(IOMTR_CPU_X86_64)

// Define the default timer
timer_type TimerType = TIMER_OSHPC;

#pragma intrinsic(__rdtsc)

DWORDLONG rdtsc()
{
	return __rdtsc();
}
#endif

//
// Windows arch independent code to allow for more programmatic timer usage
//

// Abstract the OS high perf counter/frequency, so other OSs can use the 
// rdtsc_freq and timer_* code below. These 2 are just wrappers around 
// QueryPerfCounter/Frequency

LONGLONG oshpc()
{
	LARGE_INTEGER perf_count;

	if (!QueryPerformanceCounter(&perf_count))
	{
#if _DETAILS
		cerr << "QueryPerformanceCounter failed with error " << GetLastError() << 
			    ", dynamo results are invalid." << endl;
#endif
		return 1;
	}

	return perf_count.QuadPart;
}

LONGLONG oshpc_freq()
{
	LARGE_INTEGER perf_freq;

	if (!QueryPerformanceFrequency(&perf_freq))
	{
#if _DETAILS
		cerr << "QueryPeformanceFrequency failed with error " << GetLastError() << 
			    ", dynamo results are invalid." << endl;
#endif 
		return 1;
	}

	return perf_freq.QuadPart;
}

// ----------------------------------------------------------------------------
#elif defined(IOMTR_OS_OSX)

timer_type TimerType = TIMER_OSHPC;

#if defined(IOMTR_CPU_PPC)

// NOTE:
// This is the orginal code converted to the new timer format, but it 
// should not really be used. Despite the architecture specfic macro, there
// is really no code differece between x86, x64 and PPC -- see more comments 
// below.

// This does not seem like a verry efficient method of getting clock cycles
// since it involves unnecessary math. I think this can be removed in favor of
// the x86 version below -- need to test.

#include <Carbon/Carbon.h>
double processor_speed_to_nsecs;	// declared as extern double in IOCommon.h
DWORDLONG oshpc() //timer_value()
{
	DWORDLONG temp;
	AbsoluteTime now;
	Nanoseconds s;

	now = UpTime();
	s = AbsoluteToNanoseconds(now);
	temp = s.hi;
	temp <<= 32;
	temp += s.lo;

	// temp contains timestamp in nanosecs
	// temp * processor_speed_to_nsecs = timestamp in cpu cycles
	return (DWORDLONG) (temp * processor_speed_to_nsecs);
}

double oshpc_freq()
{
	return 1e9;
}

uint64_t rdtsc()
{
	ULARGE_INTEGER uli;
    uint32_t better_not_change;
    do {
          __asm__ __volatile__ ("mftbu %0" : "=r"(uli.HighPart));
          __asm__ __volatile__ ("mftb %0"  : "=r"(uli.LowPart) );
          __asm__ __volatile__ ("mftbu %0" : "=r"(better_not_change));
     } while (uli.HighPart != better_not_change);

   return uli.QuadPart; 
}

#else

// NOTE for PPC builds:
// The else here is supposed to apply to the Intel architecture only based on 
// the macro alone, but it is safe to use it for PPC too. The intrinsic macros
// that the OSX environment provides are used below to implement the arch
// specific timestamp counter code.

#include <mach/mach_time.h>
#include <sys/sysctl.h>

uint64_t oshpc()
{
	return mach_absolute_time();
}

double oshpc_freq()
{
	mach_timebase_info_data_t info;  
	

	// Note:
	// Mach_timebase_info seems to report 1/1, thus, the mach_absolute_time is 
	// already in nanosecs??
	mach_timebase_info(&info); // retrieve the time base
		
	// Invert the timebase values to get frequency, which will be used later 
	// 1e9 is hard coded based on the units being in nanoseconds -- but this
	// could change???

#if _DEBUG
	if (info.denom != 1 || info.numer != 1)
	{
		cout << "Warning: mach_timebase_info() has reported non-1 values --"
			 << "contact the IOmeter team, numerator: " << info.numer 
			 << " denominator: " << info.denom << "." << endl;
	}
#endif
	return ((double) info.denom * 1e9 / info.numer);
}

// 
// There are no other functional differences in the code between PPC and x86, 
// so use the IOMTR_CPU_I386 definition is there just to satisfy iocommon.h 
// requirements and can be safely used to build all architectures....
//
#if defined(__ppc__) || defined(__ppc64__)
uint64_t rdtsc()
{
	ULARGE_INTEGER uli;
    uint32_t better_not_change;
    do {
          __asm__ __volatile__ ("mftbu %0" : "=r"(uli.HighPart));
          __asm__ __volatile__ ("mftb %0"  : "=r"(uli.LowPart) );
          __asm__ __volatile__ ("mftbu %0" : "=r"(better_not_change));
     } while (uli.HighPart != better_not_change);

   return uli.QuadPart; 
}
#else // all other x86 architectures
uint64_t rdtsc()
{
	ULARGE_INTEGER uli;
	
	__asm volatile("rdtsc" : "=d" (uli.HighPart), "=a" (uli.LowPart));
	
	return uli.QuadPart;
}
#endif // if ppc or ppc64

#endif
// ----------------------------------------------------------------------------
#else
#error ===> ERROR: You have to do some coding here to get the port done!
#endif
// ----------------------------------------------------------------------------


//
// COMMON TIMER CODE
//

// !!!!Delete/modify this ifdef once other OSes have implemented similar fuctionality!!!
#if defined(IOMTR_OSFAMILY_WINDOWS) || defined(IOMTR_OS_OSX)
// !!!!Delete/modify this ifdef once other OSes have implemented similar fuctionality!!!

//
// Please follow the timer conventions as defined by the above windows section.
//
// Summary:
// 1. TimerType controls the runtime type of the timer. Each OS needs to instantiate 
//    a TimerType of type timer_type and set its default. 
// 2. timer_value and timer_frequency() (below) then become the top level routines 
//    that mind TimerType global.
// 3. timer_type enum in iocommon.h and timer_value()/timer_frequeny() need to be 
//    in sync for any new timer(s) defined
// 4. Define a {timer_name}() and {timer_name}_freq() functions for a desired timer 
//    e.g. rtc() rtc_freq(), rdtsc_freq is the only exception -- see below
// 5. oshpc() and oshpc_freq() are the minimum ones that need be defined, however they should
//    not rely on rdtsc. i.e. use another type of timer. If rdtsc is not really using the real
//    rdtsc, do not even define rdtsc and set the default timer_type accordingly
// 6. rdtsc() becomes an optional timer, and rdtsc_freq (below) would calculate its frequency 
//
// Note, the function naming convention move to timer_value was done because some
// implementations did not really use rdtsc, which was the original function name. So if your OS
// defines timer_value, it really needs to be renamed to reflects its behavior. And for all 
// timers besides rdtsc, you also need to define a corresponding frequency function.



// LONGLONG rdtsc_freq()
//
// The main problem is that TSC/ITC frequency may not match the internal 
// processor clock, so we derive it here based on another known counter--
// oshpc() in this case. The default counter required for all this to work.
// While doing that, we can also detect speed stepping but not 100% reliably.
//
// Note: -assumes we are running on one processor; caller should affinitize. 
//       -oshpc() is used to measure the TSC/ITC since we know its period. 
//		 -falls back to oshpc_freq() in case we find speed stepping.
//		 -abstracted for non-windows OS (at least those that support rdtsc)

int64_t rdtsc_freq()
{
	enum {SLEEP_CASE=0, SPIN_CASE=1, MAX_CASE=2};
	uint32_t types = MAX_CASE;
	int64_t start_count, end_count;
	int64_t stop_count, spin_count;
	int64_t start_stamp, end_stamp;
	int64_t new_freq[MAX_CASE];
	int64_t perf_freq;
	int64_t frequency = 0;

	// This is the frequency that QueryPerformanceCounter() is based on
	perf_freq = oshpc_freq();

	if (perf_freq != 1)
	{
#if _DETAILS
		cout << "Performance counter frequency: " << (perf_freq / 1000000.00) << "MHz." << endl;
#endif
	}
	else 
	{
		// Should never really get here.
		cerr << "Could not query performance frequency, exiting." << endl;
		exit(1); 
	}

	//
	// Derive a sleeping and a spinning frequencies in case they differ
	//
	while (types--)
	{
		start_stamp = rdtsc();
		start_count = oshpc();

		if (types == SLEEP_CASE)
		{
			sleep_milisec(1000);
		}
		else if (types == SPIN_CASE)
		{
			stop_count = start_count + perf_freq;  // freq is cycles in 1 second
			do {
				spin_count = oshpc(); 
			} while (stop_count > spin_count);				
		}

		end_stamp = rdtsc();
		end_count = oshpc();

		// calculate interval
		end_stamp = end_stamp - start_stamp;
		end_count = end_count - start_count;

		// store the derived frequency for a given type
		new_freq[types] = perf_freq * end_stamp / end_count; // overflow??
	}

	// Catch speed-stepping CPUs
	if ( (max(new_freq[SLEEP_CASE], new_freq[SPIN_CASE]) 
		- min(new_freq[SLEEP_CASE], new_freq[SPIN_CASE])) * 100.0 / 
		         max(new_freq[SLEEP_CASE], new_freq[SPIN_CASE]) > 5.0)
	{

		cout << "##########################################################################" << endl;
		cout << " Detected speed-stepping CPU. Disable power saving mode when using IOmeter." << endl;
		cout << "##########################################################################" << endl;

#if _DEBUG
		cout << " Measured spin freq: " << new_freq[SPIN_CASE]/1000000 << 
			    "MHz, sleep freq: " <<  new_freq[SLEEP_CASE]/1000000 << "MHz. " << endl;
#endif

		// Fallback to OSHPC?
		cout << " Reverting to performance counter with frequency: " << (perf_freq / 1000000.00) << "MHz." << endl;

		TimerType = TIMER_OSHPC;
		frequency = perf_freq; 
	}
	else if ( (max(new_freq[SPIN_CASE], perf_freq) 
			 - min(new_freq[SPIN_CASE], perf_freq)) * 100.0 / 
				   max(new_freq[SPIN_CASE], perf_freq) > 5.0)
	{
		// if it's not close, we use the new spin frequency
		cout << " Using RDTSC frequency: " << (new_freq[0] / 1000000.00) << "MHz." << endl;
		frequency = new_freq[SPIN_CASE];
	}
	else
	{
		// if it's close, lets trust the original perf frequency
		cout << " Measured frequency matches performance counter frequency, using latter: " << (perf_freq / 1000000.00) << "MHz." << endl;
		frequency = perf_freq;
	}

	return frequency;
}
#endif

#if defined(IOMTR_OSFAMILY_WINDOWS) || defined(IOMTR_OS_OSX)
uint64_t timer_value()
{
	switch(TimerType) // global
	{
		case TIMER_RDTSC:
			return rdtsc();
			break;

		case TIMER_OSHPC:
		case TIMER_HPET: // fall through the default for now
		default:
			return oshpc();
			break;
	}
}

double timer_frequency()
{
	double timerFreq = 1.0;		//init to very rare flag value

	switch(TimerType)
	{
		case TIMER_RDTSC:
			timerFreq = (double) rdtsc_freq();
			break;

		case TIMER_OSHPC:
		case TIMER_HPET: // fall through the default for now
		default:
			timerFreq = (double) oshpc_freq();

			if (timerFreq == 1.0)
			{
				// This call really has to work!
				cerr << "Could not obtain peformance frequency, exiting." << endl;
				exit(1);
			}
			cout << " Using performance counter frequency: " << (timerFreq / 1000000.00) << "MHz." << endl;
			break;
    }

	return timerFreq;
}

#endif

