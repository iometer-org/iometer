/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer & Dynamo) / IOCommon.h                                    ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: This is THE central headerfile for Iometer as well    ## */
/* ##               as Dynamo.                                            ## */
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
/* ##  Remarks ...: - All Defines, Includes etc. should be moved to       ## */
/* ##                 this file to cleanup the code.                      ## */
/* ##               - Functions like Strip() and IsBigEndian() should     ## */
/* ##                 be moved to a new code file named IOCommon.cpp      ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Changes ...: 2005-04-18 (raltherr@apple.com)                       ## */
/* ##               - Support for MacOS X                                 ## */
/* ##               2005-04-17 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Code cleanup.                                       ## */
/* ##               2005-04-10 (mingz@ele.uri.edu)                        ## */
/* ##               - Corrected the macro definition for BLKGETSIZE64.    ## */
/* ##               2005-04-07 (thayneharmon@users.sourceforge.net)       ## */
/* ##               - Correct a typo of IOMTR_OSFAMILY_NETWARE            ## */
/* ##               - Added a declaration for SSGetLANCommonCounters()    ## */
/* ##               2004-09-28 (mingz@ele.uri.edu)                        ## */
/* ##               - Added the syslog.h header for linux.                ## */
/* ##               2004-09-01 (henryx.w.tieman@intel.com)                ## */
/* ##               - Added ifdef's to get Iometer to compile in x86_64   ## */
/* ##                 environment. See IOMTR_SETTING_GCC_M64.             ## */
/* ##               - Replaced most use of macro SOCKET with CONNECTION.  ## */
/* ##               - Created IOMTR_MACRO_INTERLOCK_CAST to get the       ## */
/* ##                 Interlocked functions and macros to work in with    ## */
/* ##                 several different compilers.                        ## */
/* ##               - Switched to more generic IOMTR_CPU_X86_64.          ## */
/* ##               2004-07-26 (mingz@ele.uri.edu)                        ## */
/* ##               - Added the BLKGETSIZE here.                          ## */
/* ##               2004-06-10 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Corrected the macro definition for BLKGETSIZE64.    ## */
/* ##               2004-03-27 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Applied Dan Bar Dov's patch for adding              ## */
/* ##                 Linux on PPC support.                               ## */
/* ##               2004-03-26 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Code cleanup to ensure common style.                ## */
/* ##               - Applied Thayne Harmon's patch for supporting        ## */
/* ##                 Netware support (on I386).                          ## */
/* ##               2004-03-05 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved the Dump_*_Results() function prototypes      ## */
/* ##                 (used for debugging purposes) from here to          ## */
/* ##                 ByteOrder.cpp                                       ## */
/* ##               - Moved the *_double_swap() function prototypes       ## */
/* ##                 (for Linux/XScale) from IOManager.h to here.        ## */
/* ##               2004-02-13 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Increased MAX_NAME from 64 to 80 - according to     ## */
/* ##                 swapctl(2) man page in Solaris 2.6 (relevant due    ## */
/* ##                 to the Get_All_Swap_Devices() function).            ## */
/* ##               2004-02-12 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved BLKSSZGET, BLKBSZGET and BLKGETSIZE64         ## */
/* ##                 from the IOTargetDisk.cpp file to here.             ## */
/* ##               2004-02-06 (mingz@ele.uri.edu)                        ## */
/* ##               - Added a IOMTR_CPU_XSCALE cpu type                   ## */
/* ##               - Added define for /proc/stat style in order to       ## */
/* ##                 support different kernel version                    ## */
/* ##               2003-10-05 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Integrated the modification contributed by          ## */
/* ##                 Vedran Degoricija, to get the code compile with     ## */
/* ##                 the Windows 64 Bit on AMD64.                        ## */
/* ##               2003-08-03 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Integrated the modification contributed by          ## */
/* ##                 Vedran Degoricija, to get the code compile with     ## */
/* ##                 the MS DDK on IA64.                                 ## */
/* ##               2003-08-02 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added the currently not supported CPU types         ## */
/* ##                 (see README under IOMTR_CPU_*) as well.             ## */
/* ##               2003-07-27 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Removed the [BIG|LITTLE]_ENDIAN_ARCH defines.       ## */
/* ##               - Added the implementation of the IsBigEndian()       ## */
/* ##                 function which detects the endian type of the CPU.  ## */
/* ##               2003-07-19 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Merged IOTime.h stuff into (parts of)               ## */
/* ##               2003-07-13 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Massive cleanup of this file (grouping the          ## */
/* ##                 different blocks together).                         ## */
/* ##               - Implemented the IOMTR_[OSFAMILY|OS|CPU]_* global    ## */
/* ##                 define as well as their integrity checks.           ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-03-28 (joe@eiler.net)                            ## */
/* ##               - changes so VC++ 7 (.NET) will compile correctly.    ## */
/* ##               2003-03-04 (joe@eiler.net)                            ## */
/* ##               - Made a change for Solaris to work, I had to extern  ## */
/* ##                 processor_speed_to_nsecs, I don't know how this     ## */
/* ##                 would have ever compiled, let alone worked!!        ## */
/* ##               2003-03-02 (joe@eiler.net)                            ## */
/* ##               - Changed LONG_PTR to ULONG_PTR, which is what it is  ## */
/* ##               2003-03-02 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added the implementation of the Strip() function    ## */
/* ##                 which removes leading and trailing blanks.          ## */
/* ##               2003-02-26 (joe@eiler.net)                            ## */
/* ##               - Added MAX_EXCLUDE_FILESYS so excluded filesystem    ## */
/* ##                 types are no longer hard coded.                     ## */
/* ##               2003-02-15 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Different changes to support compilation with       ## */
/* ##                 gcc 3.2 (known as cout << hex error).               ## */
/* ##               2003-02-08 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added ULONG_PTR and DWORD_PTR typedef to the        ## */
/* ##                 Windows part to get compiled.                       ## */
/* ##               2003-02-04 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Applied proc-speed-fix.txt patch file               ## */
/* ##                 (changes the data type of processor_speed).         ## */
/* ##               2003-02-02 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##               - Increase of MAX_TARGETS from 256 to 512. This was   ## */
/* ##                 needed for a successfull login of a Dynamo instance ## */
/* ##                 on a Iometer frontend (Version 2001.07.19).         ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef	___IOCOMMON_H_DEFINED___
#define ___IOCOMMON_H_DEFINED___



// Check and map the different global defines for Operating
// System family, Operating System and CPU (see README).
// ----------------------------------------------------------------------------
// Check Operating System family mapping
#if ( defined(IOMTR_OSFAMILY_NETWARE) && !defined(IOMTR_OSFAMILY_UNIX) && !defined(IOMTR_OSFAMILY_WINDOWS)) || \
    (!defined(IOMTR_OSFAMILY_NETWARE) &&  defined(IOMTR_OSFAMILY_UNIX) && !defined(IOMTR_OSFAMILY_WINDOWS)) || \
    (!defined(IOMTR_OSFAMILY_NETWARE) && !defined(IOMTR_OSFAMILY_UNIX) &&  defined(IOMTR_OSFAMILY_WINDOWS))
 // nop
#else    
 #error ===> ERROR: Check the Operating System to Operating System family mapping!
#endif
// ----------------------------------------------------------------------------
// Check the Operating System mapping
#if ( defined(IOMTR_OS_LINUX) && !defined(IOMTR_OS_NETWARE) && !defined(IOMTR_OS_OSX) && !defined(IOMTR_OS_SOLARIS) && !defined(IOMTR_OS_WIN32) && !defined(IOMTR_OS_WIN64)) || \
    (!defined(IOMTR_OS_LINUX) &&  defined(IOMTR_OS_NETWARE) && !defined(IOMTR_OS_OSX) && !defined(IOMTR_OS_SOLARIS) && !defined(IOMTR_OS_WIN32) && !defined(IOMTR_OS_WIN64)) || \
    (!defined(IOMTR_OS_LINUX) && !defined(IOMTR_OS_NETWARE) &&  defined(IOMTR_OS_OSX) && !defined(IOMTR_OS_SOLARIS) && !defined(IOMTR_OS_WIN32) && !defined(IOMTR_OS_WIN64)) || \
    (!defined(IOMTR_OS_LINUX) && !defined(IOMTR_OS_NETWARE) && !defined(IOMTR_OS_OSX) &&  defined(IOMTR_OS_SOLARIS) && !defined(IOMTR_OS_WIN32) && !defined(IOMTR_OS_WIN64)) || \
    (!defined(IOMTR_OS_LINUX) && !defined(IOMTR_OS_NETWARE) && !defined(IOMTR_OS_OSX) && !defined(IOMTR_OS_SOLARIS) &&  defined(IOMTR_OS_WIN32) && !defined(IOMTR_OS_WIN64)) || \
    (!defined(IOMTR_OS_LINUX) && !defined(IOMTR_OS_NETWARE) && !defined(IOMTR_OS_OSX) && !defined(IOMTR_OS_SOLARIS) && !defined(IOMTR_OS_WIN32) &&  defined(IOMTR_OS_WIN64))
 // nop
#else    
 #error ===> ERROR: You have to define exactly one IOMTR_OS_* global define!
#endif
// ----------------------------------------------------------------------------
// Check the Processor mapping
#if ( defined(IOMTR_CPU_ALPHA) && !defined(IOMTR_CPU_I386) && !defined(IOMTR_CPU_IA64) && !defined(IOMTR_CPU_MIPS) && !defined(IOMTR_CPU_PPC) && !defined(IOMTR_CPU_SPARC) && !defined(IOMTR_CPU_X86_64) && !defined(IOMTR_CPU_XSCALE)) || \
    (!defined(IOMTR_CPU_ALPHA) &&  defined(IOMTR_CPU_I386) && !defined(IOMTR_CPU_IA64) && !defined(IOMTR_CPU_MIPS) && !defined(IOMTR_CPU_PPC) && !defined(IOMTR_CPU_SPARC) && !defined(IOMTR_CPU_X86_64) && !defined(IOMTR_CPU_XSCALE)) || \
    (!defined(IOMTR_CPU_ALPHA) && !defined(IOMTR_CPU_I386) &&  defined(IOMTR_CPU_IA64) && !defined(IOMTR_CPU_MIPS) && !defined(IOMTR_CPU_PPC) && !defined(IOMTR_CPU_SPARC) && !defined(IOMTR_CPU_X86_64) && !defined(IOMTR_CPU_XSCALE)) || \
    (!defined(IOMTR_CPU_ALPHA) && !defined(IOMTR_CPU_I386) && !defined(IOMTR_CPU_IA64) &&  defined(IOMTR_CPU_MIPS) && !defined(IOMTR_CPU_PPC) && !defined(IOMTR_CPU_SPARC) && !defined(IOMTR_CPU_X86_64) && !defined(IOMTR_CPU_XSCALE)) || \
    (!defined(IOMTR_CPU_ALPHA) && !defined(IOMTR_CPU_I386) && !defined(IOMTR_CPU_IA64) && !defined(IOMTR_CPU_MIPS) &&  defined(IOMTR_CPU_PPC) && !defined(IOMTR_CPU_SPARC) && !defined(IOMTR_CPU_X86_64) && !defined(IOMTR_CPU_XSCALE)) || \
    (!defined(IOMTR_CPU_ALPHA) && !defined(IOMTR_CPU_I386) && !defined(IOMTR_CPU_IA64) && !defined(IOMTR_CPU_MIPS) && !defined(IOMTR_CPU_PPC) &&  defined(IOMTR_CPU_SPARC) && !defined(IOMTR_CPU_X86_64) && !defined(IOMTR_CPU_XSCALE)) || \
    (!defined(IOMTR_CPU_ALPHA) && !defined(IOMTR_CPU_I386) && !defined(IOMTR_CPU_IA64) && !defined(IOMTR_CPU_MIPS) && !defined(IOMTR_CPU_PPC) && !defined(IOMTR_CPU_SPARC) &&  defined(IOMTR_CPU_X86_64) && !defined(IOMTR_CPU_XSCALE)) || \
    (!defined(IOMTR_CPU_ALPHA) && !defined(IOMTR_CPU_I386) && !defined(IOMTR_CPU_IA64) && !defined(IOMTR_CPU_MIPS) && !defined(IOMTR_CPU_PPC) && !defined(IOMTR_CPU_SPARC) && !defined(IOMTR_CPU_X86_64) &&  defined(IOMTR_CPU_XSCALE))
 // nop
#else    
 #error ===> ERROR: You have to define exactly one IOMTR_CPU_* global define!
#endif
// ----------------------------------------------------------------------------



// Include the different header files
// (both, OS family based and common)
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_WINDOWS)   // Only first, because it is needed here!
 //#define VC_EXTRALEAN
 //#pragma warning (disable: 4242)
#include "StdAfx.h"
//#define WIN32_LEAN_AND_MEAN 1
//#include <Windows.h>
//#include <windef.h>  
 #include <process.h>
 #include <io.h>
 #include <direct.h>
// #include <afxwin.h>
// #include <afxext.h>
// #include <afxcmn.h>
 #include <iomanip>
 #include <winperf.h>
 #include <winreg.h>
 #include <winioctl.h>
// #include <afxmt.h>
 #include <malloc.h>
#endif
// ----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <map>
// ----------------------------------------------------------------------------
#include <iostream>
#include <fstream>
using namespace std;
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_NETWARE)
 #include <sys/timeval.h>
 #include <sys/time.h>
 #include <sys/socket.h>
 #include <unistd.h>
 #include <signal.h>
 #include <netinet/in.h>   // in_addr_t
 #include <nks/memory.h>
 #include <nks/fsio.h>
 #include <pthread.h>
 #include <mmpublic.h>
 #include <malloc.h>
#endif
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_UNIX)
 #include <sys/timeb.h>
 #include <unistd.h>
 #include <pthread.h>
 #include <signal.h>
 #include <netinet/in.h>   // in_addr_t

 #if defined(IOMTR_OS_OSX)
  #include <sys/aio.h>
  #define aiocb64 		aiocb
  #define aio_suspend64		aio_suspend
  #define aio_error64		aio_error
  #define aio_cancel64		aio_cancel
  #define aio_read64		aio_read
  #define aio_write64		aio_write
  #define aio_return64		aio_return

 #elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_SOLARIS)
  #include <malloc.h>
  #include <aio.h>
  #ifdef IOMTR_SETTING_LINUX_LIBAIO
   #include <libaio.h>
  #endif
 #else
  #error ===> ERROR: You have to define exactly one IOMTR_CPU_* global define!
 #endif

 #if defined(IOMTR_OS_LINUX)
  #include <sys/ioctl.h>
 #endif
 #if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
  #include <syslog.h>
 #endif
#endif
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_WINDOWS)
 #include <sys/timeb.h>
 #if (_MSC_VER < 1300) || defined(USING_DDK) // not needed for vista ddk using stl70
  #include "ostream64.h"
 #endif
#endif
// ----------------------------------------------------------------------------
#include "IOVersion.h"   // version info definitions
// ----------------------------------------------------------------------------



// Define the different data types
// (both, OS family based and common)
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
 #define __int64   long long
 #define __int32   long
 #define __int16   short
 #define __int8    char

 #if defined(IOMTR_SETTING_GCC_M64)
  // in the gcc on x86_64 environment long is 64 bits
  #undef __int32
  #define __int32 int
 #endif

 typedef long long	       _int64;
 //typedef long long	       LARGE_INTEGER; 
 typedef unsigned long long    DWORDLONG;

 typedef union _LARGE_INTEGER 
 {
    struct 
    {
        uint32_t LowPart;
        int32_t  HighPart;
    };
    int64_t QuadPart;
 } LARGE_INTEGER, *PLARGE_INTEGER;

 typedef union _ULARGE_INTEGER 
 {
    struct 
    {
        uint32_t LowPart;
        uint32_t HighPart;
    };
    uint64_t QuadPart;
 } ULARGE_INTEGER, *PULARGE_INTEGER;

 #if defined(IOMTR_OSFAMILY_NETWARE)
  #ifndef LONG
   #define LONG	unsigned long
  #endif
  #ifndef WORD
   #define WORD	unsigned short
  #endif
 #elif defined(IOMTR_OSFAMILY_UNIX)
  // This is OK for x86-64 processors because LONG is only used in Netware or
  // in MeterCtrl.cpp.or safely internally in IOTargetDisk.cpp.
  // It could cause problems because in the x86-64 environment long is 64 bits.
  //typedef long		       LONG; // careful, on windows long is 32bits -- see above
  #if defined(IOMTR_OS_OSX)
   #define off64_t	off_t  
  #endif 
 #else
  #warning ===> WARNING: You have to do some coding here to get the port done!
 #endif
 
 #if defined(IOMTR_SETTING_GCC_M64)
  // DWORD is supposed to be an unsigned 32 bit number.
  typedef unsigned int	       		DWORD;
  typedef DWORD				LONG;
  typedef unsigned __int64     		ULONG_PTR, *PULONG_PTR;
  typedef ULONG_PTR            		DWORD_PTR;
  typedef __int64			LONG_PTR;
  #define IOMTR_FORMAT_SPEC_64BIT	"L" 
#else
  typedef unsigned long			DWORD;
  typedef DWORD				LONG;
  typedef unsigned long			ULONG_PTR, *PULONG_PTR;
  typedef ULONG_PTR			DWORD_PTR;
  typedef int				LONG_PTR;
  #define IOMTR_FORMAT_SPEC_64BIT	"l"
 #endif
 
 typedef int		       INT;
 typedef int		       BOOL;
 typedef int		       BOOLEAN;
 typedef unsigned int	       UINT;
 typedef unsigned int	      *PUINT;

 typedef unsigned short	       WCHAR;
 typedef unsigned short	      *LPWSTR;

 typedef char		      *LPSTR;
 typedef unsigned char	       BYTE;
 typedef const char	      *LPCSTR;

 typedef void		      *HANDLE;
 typedef void		      *LPVOID;

 //  The places in the Iometer code that now use CONNECTION used to use
 //  SOCKET. But SOCKET is a defined type in winsock2.h used for network
 //  access. Iometer defined a type called SOCKET used in NetTCP.cpp but 
 //  the original winsock SOCKET was used in IOPortTCP.cpp. Defining
 //  a new CONNECTION type allows me to separate the two different concepts
 //  cleanly.
 typedef void                 *CONNECTION;
 
 typedef int		       SOCKET;
 typedef const void	      *LPCVOID;

 typedef DWORD		      *LPDWORD;
 typedef BYTE		      *LPBYTE;
 typedef LPCSTR		       LPCTSTR;

 #endif 
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_WINDOWS)

// All references to types
typedef          __int64    int64_t;
typedef unsigned __int64   uint64_t;
typedef          __int32    int32_t;
typedef unsigned __int32   uint32_t;
typedef          __int16    int16_t;
typedef unsigned __int16   uint16_t;
typedef          __int8      int8_t;
typedef unsigned __int8     uint8_t;

#ifndef LONG_PTR
  #if defined(IOMTR_OS_WIN32)
   // dps: Was __int32 in before, but conflicts while conversion
   //      from unsigned __int32 to unsigned long for instance
   //      (using Microsoft Visual C++).
   typedef long               LONG_PTR, *PLONG_PTR;
   typedef unsigned long      ULONG_PTR, *PULONG_PTR;
   typedef ULONG_PTR          DWORD_PTR;
  #endif
  #if defined(IOMTR_OS_WIN64)
   typedef __int64            LONG_PTR, *PLONG_PTR;
   typedef unsigned __int64   ULONG_PTR, *PULONG_PTR;
   typedef ULONG_PTR          DWORD_PTR;
  #endif
#endif // LONG_PTR
 //  The places in the Iometer code that now use CONNECTION used to use
 //  SOCKET. But SOCKET is a defined type in winsock2.h used for network
 //  access. Iometer defined a type called SOCKET used in NetTCP.cpp but 
 //  the original winsock SOCKET was used in IOPortTCP.cpp. Defining
 //  a new CONNECTION type allows me to separate the two different concepts
 //  cleanly.
 typedef ULONG_PTR CONNECTION;

#define IOMTR_FORMAT_SPEC_64BIT	"I64"
#endif 

#if defined(IOMTR_OS_WIN32)
 #define IOMTR_FORMAT_SPEC_POINTER
#else
 #define IOMTR_FORMAT_SPEC_POINTER	IOMTR_FORMAT_SPEC_64BIT
#endif

// ----------------------------------------------------------------------------

// Because of some of the memory over writing issues in the 64 bit environment
// and the typing required by the windows environment, variables need to be cast
// differently depending on OSFAMILY. this gets the job done... 
#if defined(IOMTR_OSFAMILY_WINDOWS)
 #define IOMTR_MACRO_INTERLOCK_CAST(a) (long *)
#elif defined(IOMTR_OSFAMILY_UNIX) || defined(IOMTR_OSFAMILY_NETWARE)
 #define IOMTR_MACRO_INTERLOCK_CAST(a) (a *)
#else
 #error ===> ERROR: You have to do some coding here to get the port done!
#endif

// ----------------------------------------------------------------------------



// Definition of the defines itself
// (both, OS family based and common)
// ----------------------------------------------------------------------------
// Different return values for the Iometer and Dynamo function calls.
#define IOERROR	-1   // keep negative

#define RETRY_DELAY   200    // msec. to wait before reattempting an action
#define LONG_DELAY    3000   // msec. to wait before reattempting an action
#define IDLE_DELAY    1000   // max msec to wait between calls to CGalileoApp::OnIdle()

#define DATA_PATTERN_REPEATING_BYTES   0
#define DATA_PATTERN_PSEUDO_RANDOM     1
#define DATA_PATTERN_FULL_RANDOM       2

#define RANDOM_BUFFER_SIZE   16*1024*1024

#define KILOBYTE_BIN   1024
#define MEGABYTE_BIN   1048576
#define GIGABYTE_BIN   1073741824
#define TERABYTE_BIN   1099511627776

#define KILOBYTE_DEC   1000
#define MEGABYTE_DEC   1000000
#define GIGABYTE_DEC   1000000000
#define TERABYTE_DEC   1000000000000

#define MAX_NAME	      80   // xca1019: Was 64 - changed according to
                                   // the Solaris 2.6 man page for swapctl(2)
#define	MAX_CPUS	      64
#define	MAX_WORKERS	      128
#define MAX_TARGETS	      2048
#define MAX_NETWORK_NAME      128
#define MAX_NUM_INTERFACES    64
#define MAX_WORKER_NAME	      128
#define MAX_EXCLUDE_FILESYS   128

#define ALL_MANAGERS	      -16
#define MANAGER		      -8
#define ALL_WORKERS	      -4
#define WORKER		      -2

// Definitions for CPU results.
#define	CPU_RESULTS		     6
#define CPU_UTILIZATION_RESULTS	     5
// All CPU utilization results should be listed together.
#define CPU_TOTAL_UTILIZATION	     0
#define CPU_USER_UTILIZATION	     1
#define CPU_PRIVILEGED_UTILIZATION   2
#define CPU_DPC_UTILIZATION	     3
#define CPU_IRQ_UTILIZATION   	     4
// Other CPU counters.
#define CPU_IRQ			     5

// Definitions for network tcp results
#define TCP_RESULTS           1   // total number of network results
// List all specific performance results that are desired.
#define TCP_SEGMENTS_RESENT   0

// Definitions for network interface results
#define NI_RESULTS	     3   // total number of network interface results reported
#define NI_COMBINE_RESULTS   2   // combine some reported results together
// List all specific performance results that are desired.
#define NI_PACKETS	     0
#define NI_OUT_ERRORS	     1
#define NI_IN_ERRORS	     2
#define NI_ERRORS	     1   // combine in and out errors together when saving

// Initialization parameters for CArray and related classes.
#define INITIAL_ARRAY_SIZE   0    // Sets the size of the array.
#define ARRAY_GROW_STEP	     10   // Sets the amount of memory to allocate when the array is grown.

// To record performance statistics, two snapshots of specific counters are needed.
#define FIRST_SNAPSHOT	 0
#define LAST_SNAPSHOT	 1
#define MAX_SNAPSHOTS	 2

// We keep a Performance object to record results for the whole test and another to record
// only the results since the last update.
#define WHOLE_TEST_PERF	   0
#define LAST_UPDATE_PERF   1
#define MAX_PERF	   2

#define SEND   1
#define RECV   2

#define SMLBUFSIZE   512 

#define MAX_GUI_IOPS   ((uint32) -1) // max 32bit value

#define LATENCY_BIN_SIZE 21

// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
 //param does not contain a valid internet address
 // (For Win32 || _WIN64, INADDR_NONE is defined as 0xffffffff)
 #ifndef INADDR_NONE
  #define INADDR_NONE   (in_addr_t)-1
 #endif

 // Needed to get the CDECL stuff thru (under non Windows platforms)
 #define CDECL

 #define FALSE			0
 #define TRUE			1
 #define INVALID_HANDLE_VALUE   (HANDLE)-1
 #define INVALID_SOCKET		(~0L)
 #define PERF_NO_INSTANCES	-1
 #define SOCKET_ERROR		-1
 #define SD_BOTH		2
 #define INFINITE		(unsigned)~0
 #define ERROR_IO_PENDING	501
 #define WAIT_TIMEOUT		ERROR_IO_PENDING
 #define ERROR_IO_INCOMPLETE	ERROR_IO_PENDING

 #define FILE_ELEMENT		1
 #define CQ_ELEMENT		0

 #if defined(IOMTR_OSFAMILY_NETWARE)
  #define SIGEV_NONE		0
  #define AIO_NOTCANCELED	1
 #endif

#endif
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_WINDOWS)
 #define IOMETER_RECEIVE_TIMEOUT   10000 // 10 seconds in miliseconds
#endif
// ----------------------------------------------------------------------------



// Define the enumerations, structures etc.
// (both, OS family based and common)
// ----------------------------------------------------------------------------
enum ReturnVal {
	ReturnError   = IOERROR,   // catch all unexpected errors
	ReturnSuccess = 0,	   // Normal completion
	ReturnTimeout,		   // Operation timed out
	ReturnPending,		   // Request has been queued
	ReturnAbort,		   // Operation aborted
	ReturnRetry	   	   // Operation was interrupted and can be retried
};
// Testing states that Iometer and worker threads go through
enum TestState {
	TestIdle,	  // before Start_Test() and after Stop_Test()
	TestPreparing,
	TestOpening,	  // after Start_Test()
	TestRampingUp,	  // after Begin_IO()
	TestRecording,	  // after Record_On()
	TestRampingDown	  // after Record_Off()
};
// Specified on the Test Setup tab: which test results to record.
// (In IOCommon.h because Dynamo uses these as parameters.)
enum {
	RecordAll = 0,
	RecordNoTargets,
	RecordNoWorkers,
	RecordNoManagers,
	RecordNone
};

// Defines various timer types supported, see iotime.c
typedef enum {
	TIMER_UNDEFINED = 0, 
	TIMER_OSHPC     = 1, 
	TIMER_RDTSC     = 2,  
	TIMER_HPET      = 3, 
	TIMER_TYPE_MAX  = 3
} timer_type;

// #ifdef USE_NEW_DETECTION_MECHANISM
// Controls the disk view options in the new disk detection code. Not ifdefed beuase it is tied
// to the dynamo_param relocation which is not ifdefed either.
typedef enum {
	RAWDISK_VIEW_COMPAT = 0, 
	RAWDISK_VIEW_NOPART = 1, 
	RAWDISK_VIEW_FULL   = 2
} diskview_type;

// ----------------------------------------------------------------------------

// FORCE_STRUCT_ALIGN forces compiler controlled structure alignment. This seems
// tough to get right between Windows and Unix. So far, the GCC_ATTRIBUTE_ALIGN 
// does not produce the proper Windows packing. The only way it seems to work 
// right is with a pragma pack of 1.
#ifdef FORCE_STRUCT_ALIGN

// The intent of GCC_STRUCT_ALIGN is only to be used on UNIX/GCC as an 
// alternative to pragma pack. With the 2010 IOmeter release, this is not being
// not be used, but its here for future...
#ifdef GCC_ATTRIBUTE_ALIGN
#if defined(IOMTR_OSFAMILY_UNIX)

// Other __attribute__ options: packed, ms_struct
#define STRUCT_ALIGN(Bytes)  __attribute__ ((aligned (Bytes)))    

// If GCC_ATTRIBUTE_ALIGN is defines, the STRUCT_ALIGN argument 
// must match the pragma pack value in pack.h
#define STRUCT_ALIGN_IOMETER STRUCT_ALIGN(1)  

#endif // IOMTR_OSFAMILY_UNIX

#else
#define STRUCT_ALIGN_IOMETER
#endif // GCC_ATTRIBUTE_ALIGN

#else
#define STRUCT_ALIGN_IOMETER
#endif // FORCE_STRUCT_ALIGN

// Allows for a 8bit.16bit.8bit version value
#define common_encode_version(x,y,z) ((x << 24) | (y << 8) | z)
#define common_major_version(x) ((x >> 24) & 0xff)
#define common_minor_version(y) ((y >> 8) & 0xffff)
#define common_submn_version(z) (z & 0xf)
#define compat_version_mask 0xffffff00 // all but the last byte

#include "pack.h"

struct Manager_Info
{
	char	       version[MAX_VERSION_LENGTH];
	char	       names[2][MAX_NETWORK_NAME];
	unsigned short port_number;   // used only with TCP/IP.
#ifndef FORCE_STRUCT_ALIGN
#if defined(IOMTR_OSFAMILY_NETWARE)	
	char pad[2];
#endif	
#endif 
	int	       processors;
	double	       timer_resolution;
} STRUCT_ALIGN_IOMETER;
// Basic result information stored by worker threads.
struct Raw_Result
{
	DWORDLONG    bytes_read;   	       // Number of bytes transferred.
	DWORDLONG    bytes_written;
	DWORDLONG    read_count;	       // Number of I/Os completed.
	DWORDLONG    write_count;
	DWORDLONG    transaction_count;	       // Number of transactions completed.
	DWORDLONG    connection_count;
	unsigned int read_errors;	       // Number of errors seen.
	unsigned int write_errors;
	DWORDLONG    max_raw_read_latency;     // Application latencies for an I/O to complete, recorded
	DWORDLONG    read_latency_sum;	       // using the processor counter clock.
	DWORDLONG    max_raw_write_latency;
	DWORDLONG    write_latency_sum;
	DWORDLONG    max_raw_transaction_latency;
	DWORDLONG    max_raw_connection_latency;
	DWORDLONG    transaction_latency_sum;
	DWORDLONG    connection_latency_sum;   // Application latencies for a Connection.
	__int64	     counter_time;	       // Difference between ending and starting counter time stamps.
	__int64		 latency_bin[LATENCY_BIN_SIZE];
} STRUCT_ALIGN_IOMETER;
// Storing results for all targets in a single structure.
struct Target_Results
{
	int	   count;    // Number of targets.
#ifndef FORCE_STRUCT_ALIGN
	char	   pad[4];   // padding
#endif
	Raw_Result result[MAX_TARGETS];
} STRUCT_ALIGN_IOMETER;
// Storing results for a worker.  This includes the worker's target results.
struct Worker_Results
{
	DWORDLONG      time[MAX_SNAPSHOTS];   // Processor based counters to provide time stamps.
	Target_Results target_results;
} STRUCT_ALIGN_IOMETER;
// All CPU related results are stored in a single structure.
struct CPU_Results
{
	int    count;	 // Number of processors.
#ifndef FORCE_STRUCT_ALIGN
	char   pad[4];   // padding
#endif
	double CPU_utilization[MAX_CPUS][CPU_RESULTS];
} STRUCT_ALIGN_IOMETER;
// All network related results are stored in a single structure.
struct Net_Results
{
	double tcp_stats[TCP_RESULTS];
	int    ni_count;   // Number of NICs.
#ifndef FORCE_STRUCT_ALIGN
	char   pad[4];     // padding
#endif
	double ni_stats[MAX_NUM_INTERFACES][NI_RESULTS];
} STRUCT_ALIGN_IOMETER;
// Results SPECIFIC to a single system.  This is NOT compiled system results.
struct Manager_Results
{
	__int64	    time_counter[MAX_SNAPSHOTS];
	CPU_Results cpu_results;
	Net_Results net_results;
} STRUCT_ALIGN_IOMETER;
// Result structure used by the manager list, managers, and workers to store
// results that will be saved.
struct Results
{
	Raw_Result   raw;
	double	     IOps;
	double	     read_IOps;
	double	     write_IOps;
	double	     MBps_Bin;
	double	     read_MBps_Bin;
	double	     write_MBps_Bin;
	double	     MBps_Dec;
	double	     read_MBps_Dec;
	double	     write_MBps_Dec;
	double	     ave_latency;
	double	     max_latency;
	double	     ave_read_latency;
	double	     max_read_latency;
	double	     ave_write_latency;
	double	     max_write_latency;
	double	     transactions_per_second;
	double	     ave_transaction_latency;
	double	     max_transaction_latency;
	unsigned int total_errors;
	double	     CPU_utilization[CPU_RESULTS];
	double	     CPU_effectiveness;
	double	     individual_CPU_utilization[MAX_CPUS][CPU_RESULTS];
	double	     tcp_statistics[TCP_RESULTS];
	double	     ni_statistics[NI_COMBINE_RESULTS];
	double	     connections_per_second;
	double	     ave_connection_latency;
	double	     max_connection_latency;
} STRUCT_ALIGN_IOMETER;

#include "unpack.h"

// Moved to a global location so that other classes can get access 
// to it w/out complicated C++ relationships
struct dynamo_param {
	char *iometer;
	char *manager_name;
	char *manager_computer_name;
	char *manager_exclude_fs;
	char (*blkdevlist)[MAX_TARGETS][MAX_NAME];
	ULONG_PTR cpu_affinity;   // Needs to be 64bits on 64bit systems!
	                          // Someone needs to fix the linux definitions
	int login_port_number;
	int timer_type;           // control timer used from cmdline
	int disk_control;         // control what disk get displayed in iometer from cmdline
};

// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_UNIX) || defined(IOMTR_OSFAMILY_NETWARE)
 // This LPOVERLAPPED typedef is from WINBASE.H
 #if defined(IOMTR_OSFAMILY_NETWARE)
 struct  aiocb64 {
 	int		aio_fildes;
 	void		*aio_buf;
 	size_t		aio_nbytes;
 	off_t		aio_offset;
 	int		aio_flag;
 	int		error;
 	int		returnval;
#if defined(IOMTR_SETTING_GCC_M64)
 	unsigned int	completion_key;
#else
 	unsigned long	completion_key;
#endif
 	struct {
 		int sigev_notify;
 	} aio_sigevent;
 };
 struct timeb {
#if defined(IOMTR_SETTING_GCC_M64)
	int time;
#else
	long time;
#endif
	unsigned short millitm;
	short timezone;
	short dstflag;
 };
 #endif
 typedef struct _OVERLAPPED {   
	DWORD  Internal;
       	DWORD  InternalHigh;
	DWORD  Offset;
	DWORD  OffsetHigh;
	HANDLE hEvent;
 } OVERLAPPED, *LPOVERLAPPED;
// typedef struct _LARGE_INTEGER {
//	long LowPart;
//	long HighPart;
// } LARGE_INTEGER;
 struct CQ_Element {
	struct  aiocb64 aiocbp;
#ifdef IOMTR_SETTING_LINUX_LIBAIO
        struct  iocb iocbp;
#endif
	void   *data;
	int	done;
	int	error;
	int	completion_key;
#if defined(IOMTR_SETTING_GCC_M64)
	long	bytes_transferred;
#else
	int	bytes_transferred;
#endif
 };
 struct IOCQ {
	CQ_Element      *element_list;
	struct aiocb64 **aiocb_list;
#ifdef IOMTR_SETTING_LINUX_LIBAIO
	struct iocb **iocb_list;
        io_context_t io_ctx_id;
        struct io_event *events;
#endif
	int	        size;
	int	         last_freed;
	int	         position;
 };
 struct File {
	int   fd;
	int   completion_key;
	IOCQ *iocq;
 #if defined(IOMTR_OSFAMILY_NETWARE)
	int type;
 #endif
 };
#endif 
// ----------------------------------------------------------------------------



// Define the global variables
// (both, OS family based and common)
// ----------------------------------------------------------------------------
const char NEW_WORKER_COMMAND[]    = "start /MIN ";
const char NEW_WORKER_EXECUTABLE[] = "dynamo";
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_UNIX) || defined(IOMTR_OSFAMILY_NETWARE)
 extern pthread_mutex_t lock_mt;   // we use one global locking mutex
#endif
// ----------------------------------------------------------------------------
#if defined(IOMTR_CPU_SPARC) || defined(IOMTR_CPU_PPC)
 extern double processor_speed_to_nsecs;
#endif
// ----------------------------------------------------------------------------



// Define the different function prototypes and macros for
// global functions - including the Strip() and IsBigEndian()
// function (both, OS family based and common)
// ----------------------------------------------------------------------------
extern void GetAppFileVersionString( char **ppStrStandard, char **ppStrWithDebug );

inline void rotate(char *ptr, int size);
template <class T> inline void reorder(T&);
inline void reorder(CPU_Results&, int);
inline void reorder(Net_Results&, int);
inline void reorder(Raw_Result&);

inline char *Strip(char *pcString)
{
	size_t uiLength;
	size_t I;
	size_t iIndex = 0;
	size_t iPos1, iPos2;

	if (!pcString)
		return pcString;
	uiLength = strlen(pcString); // 64bit version now return qword values!
	if (!uiLength)
		return pcString;
	for (iPos1 = 0; ((iPos1 < uiLength) && (pcString[iPos1] == (int)' ')); iPos1++) {
		/* NOP */
	}
	for (iPos2 = uiLength - 1; ((iPos2 != 0) && (pcString[iPos2] == (int)' ')); iPos2--) {
		/* NOP */
	}
	if ((iPos2 == 0) && (pcString[iPos2] == (int)' '))
		pcString[0] = '\0';
	
	if ((iPos1 != 0) || (iPos2 != (uiLength - 1))) {
		for (I = iPos1; I <= iPos2; I++) {
			pcString[iIndex] = pcString[I];
			iIndex++;
		}
		pcString[iIndex] = '\0';
	}
	return(pcString);
}

// Detecting endianness of the architecture is needed for encoding network
// protocol data correctly. Unlike many applications, Iometer does not use
// the usual htonl() etc. functions to convert between host byte order and 
// network byte order. Instead it uses it's own functions.
//
// Detecting endianness at runtime (as opposed to compile time) is now a
// feature (as opposed to optimization potential), as it allows to ship a
// single Dynamo binary for PPC and x86 based OS X machines.
//
// Once there is a rewrite of the network protocol, this should be cleaned
// up by using network byte order, htonl() etc. That will eliminate the
// need for explicit endianness detection.

inline int IsBigEndian(void)
{
 union EndianUnion
 {
  struct EndianStruct
  {
   unsigned ubBit1 :1;
   unsigned ubBit2 :1;
   unsigned ubBit3 :1;
   unsigned ubBit4 :1;
   unsigned ubBit5 :1;
   unsigned ubBit6 :1;
   unsigned ubBit7 :1;
   unsigned ubBit8 :1;
   unsigned ubBitN :8;
  } sStruct;
  unsigned char ucNumber;
 } sUnion;
 sUnion.sStruct.ubBit1 = 0;
 sUnion.sStruct.ubBit2 = 0;
 sUnion.sStruct.ubBit3 = 0;
 sUnion.sStruct.ubBit4 = 0;
 sUnion.sStruct.ubBit5 = 0;
 sUnion.sStruct.ubBit6 = 0;
 sUnion.sStruct.ubBit7 = 0;
 sUnion.sStruct.ubBit8 = 0;
 sUnion.sStruct.ubBitN = 0;
 sUnion.ucNumber       = 5;

 if( (sUnion.sStruct.ubBit1 == 1) && (sUnion.sStruct.ubBit3 == 1) )
 { return(0); }      /* ##### Little Endian */
 else
 {
  if( (sUnion.sStruct.ubBit6 == 1) && (sUnion.sStruct.ubBit8 == 1) )
  { return(1); }     /* ##### Big Endian    */
  else
  { return(-42); }   /* ##### Unknown       */
 }
}

#if defined(_DEBUG)
 void Dump_Raw_Result(struct Raw_Result *res);
 void Dump_Manager_Results(struct Manager_Results *res);
 void Dump_CPU_Results(struct CPU_Results *res);
 void Dump_Net_Results(struct Net_Results *res);
#endif
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
 BOOL    SetQueueSize(HANDLE, int);
 HANDLE  CreateIoCompletionPort(HANDLE, HANDLE, DWORD, DWORD);
 BOOL    GetQueuedCompletionStatus(HANDLE, LPDWORD, LPDWORD, LPOVERLAPPED *, DWORD);

 BOOL    ReadFile(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
 BOOL    WriteFile(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
 HANDLE  CreateEvent(void *, BOOL, BOOL, LPCSTR);
 BOOL    GetOverlappedResult( HANDLE, LPOVERLAPPED, LPDWORD, BOOL );

 void    SetLastError(DWORD);
 DWORD   GetLastError(void);
 int     WSAGetLastError(void);
 void    WSASetLastError(DWORD);

 BOOL    CloseHandle(HANDLE, int object_type = 0);
 char   *_itoa(int, char *, int);
 DWORD   WaitForSingleObject(HANDLE h, DWORD milliSecs);

 #ifndef NOMINMAX
  #ifndef max
   #define max(a,b)             (((a) > (b)) ? (a) : (b))
  #endif
  #ifndef min
   #define min(a,b)             (((a) < (b)) ? (a) : (b))
  #endif
 #endif 
 #ifndef __max
  #define __max(a,b)            (((a) < (b)) ? (b) : (a))
 #endif

 #if defined(IOMTR_OSFAMILY_NETWARE)
  #define _timeb		timeb
  #define _ftime		nwtime
  #define _time			time
  #define _millitm		millitm
  #define Sleep(x)		delay((x))
 #elif defined(IOMTR_OSFAMILY_UNIX)
  #if defined(IOMTR_OS_OSX)
   #define _timeb		timeval
   #define _ftime(x)		gettimeofday(x,NULL)
   #define _time		tv_sec
   #define _millitm		tv_usec
   #define Sleep(x) 		usleep((x) * 1000)
  #elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_SOLARIS)
   #define _timeb		timeb
   #define _ftime		ftime
   #define _time			time
   #define _millitm		millitm
   #define Sleep(x) 		usleep((x) * 1000)
  #else
   #error ===> ERROR: You have to do some coding here to get the port done!
  #endif
 #else
  #warning ===> WARNING: You have to do some coding here to get the port done!
 #endif

 #define LOCK					    \
	 if (pthread_mutex_lock(&lock_mt))          \
	 {					    \
		cout << "unable to lock" << endl;   \
		exit(1);			    \
	 }
 #define UNLOCK					      \
	 if (pthread_mutex_unlock(&lock_mt))	      \
	 {					      \
		cout << "unable to unlock" << endl;   \
		exit(1);			      \
	 }

 #define InterlockedExchange(a,b)   \
	 LOCK;			    \
	 *(a) = (b);		    \
	 UNLOCK;
 #define InterlockedDecrement(a)   \
	 LOCK;			   \
	 --*(a);		   \
	 UNLOCK;
 #define InterlockedIncrement(a)   \
	 LOCK;			   \
	 ++*(a);		   \
	 UNLOCK;
#endif 
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_WINDOWS)
 #define _millitm     millitm
 #define snprintf     _snprintf
 #define strcasecmp   _stricmp   // Follow POSIX naming of functions
 #define _time        time
#endif
// ----------------------------------------------------------------------------
#if defined(IOMTR_OS_LINUX)
 extern DWORDLONG jiffies(void);
 extern DWORDLONG timer_value(void);

 #if defined(IOMTR_CPU_PPC)
  extern DWORD get_tbl();
  extern DWORD get_tbu();
 #endif

 #if defined(IOMTR_CPU_XSCALE)
  extern void double_wordswap(double *d);
  extern void Manager_Info_double_swap(struct Manager_Info *p);
  extern void Manager_Results_double_swap(struct Manager_Results *p);
  extern void CPU_Results_double_swap(struct CPU_Results *p);
  extern void Net_Results_double_swap(struct Net_Results *p);
 #endif
 
 #if defined(_IO)  && !defined(BLKSSZGET)
  #define BLKSSZGET    _IO(0x12,104)
 #endif
 #if defined(_IOR) && !defined(BLKBSZGET)
  #define BLKBSZGET    _IOR(0x12,112,size_t)
 #endif
 #if defined(_IO)  && !defined(BLKGETSIZE)
  #define BLKGETSIZE   _IO(0x12,96)
 #endif
 #if defined(_IO)  && !defined(BLKGETSIZE64)
  #define BLKGETSIZE64 _IOR(0x12,114, size_t)
 #endif
#endif 
// ----------------------------------------------------------------------------
#if defined(IOMTR_OS_NETWARE)
 int aio_suspend64(struct aiocb64 **cb, int a, struct timespec *);
 int aio_error64(struct aiocb64 *cb);
 int aio_return64(struct aiocb64 *cb);
 int aio_read64(struct aiocb64 *cb, int type);
 int aio_write64(struct aiocb64 *cb, int type);
 int aio_cancel64(int a, struct aiocb64 *cb);

 #ifdef __cplusplus
 extern "C"
 {
 #endif
 extern LONG  GetTimerMinorTicksPerSecond(void);
 extern unsigned long kGetProcessorInterruptCount(unsigned int, unsigned int *);
 extern void  EnterDebugger();
 extern LONG SSGetLANCommonCounters(unsigned long, unsigned long, unsigned char *, unsigned int);
 #ifdef __cplusplus
 }
 #endif

 extern DWORDLONG timer_value(void); 
#endif
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_WINDOWS) 
 #define sleep_milisec(_time)   Sleep(_time);
#elif defined(IOMTR_OSFAMILY_UNIX)
 #define sleep_milisec(_time)   usleep(_time * 1000);
#else
 #error You have some work to do here to implemenent sleep_milisec()
#endif
// ----------------------------------------------------------------------------
//#if defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
#if defined(IOMTR_OS_SOLARIS)
 extern "C" DWORDLONG timer_value();
#endif
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_WINDOWS) || defined(IOMTR_OS_OSX)
 extern "C" uint64_t timer_value();
 extern "C" double timer_frequency();
#endif
// ----------------------------------------------------------------------------

#endif	// ___IOCOMMON_H_DEFINED___




