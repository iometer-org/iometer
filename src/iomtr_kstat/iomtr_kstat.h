/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / iomtr_kstat.h                                             ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Interface for the kernel status module.               ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  This program is free software; you can redistribute it and/or      ## */
/* ##  modify it under the terms of the GNU General Public License        ## */
/* ##  as published by the Free Software Foundation; either version 2     ## */
/* ##  of the License, or (at your option) any later version.             ## */
/* ##                                                                     ## */
/* ##  This program is distributed in the hope that it will be useful,    ## */
/* ##  but WITHOUT ANY WARRANTY; without even the implied warranty of     ## */
/* ##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       ## */
/* ##  GNU General Public License for more details.                       ## */
/* ##                                                                     ## */
/* ##  You should have received a copy of the GNU General Public License  ## */
/* ##  along with this program; if not, write to the Free Software        ## */
/* ##  Foundation, Inc., 59 Temple Place - Suite 330, Boston,             ## */
/* ##  MA  02111-1307, USA.                                               ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Remarks ...: <none>                                                ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Changes ...: 2004-04-08 (daniel@scheibli.com)                      ## */
/* ##               - Changed the license to GPL (after consulting Ming). ## */
/* ##               2004-02-04 (mingz(at)ele.uri.edu)                     ## */
/* ##               - initital code. defines shared by kernel module and  ## */
/* ##                 dynamo                                              ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef __IM_KSTAT__
#define __IM_KSTAT__

#include <linux/ioctl.h> 

#define IMKSTAT_IOC_MAGIC	0xAE

// copied from IOCommon.h, should merge to one later
#ifndef MAX_CPUS
#define MAX_CPUS	32
#endif
#ifndef MAX_NUM_INTERFACES
#define MAX_NUM_INTERFACES	32
#endif

struct cpu_data_type {
	unsigned long long user_time[MAX_CPUS];
	unsigned long long system_time[MAX_CPUS];
	unsigned long long intr;
};

struct ni_data_type {
	unsigned long long in_err[MAX_NUM_INTERFACES];
	unsigned long long out_err[MAX_NUM_INTERFACES];
	unsigned long long nr_pkt[MAX_NUM_INTERFACES];
};

struct tcp_data_type {
	long long insegs;
	long long outsegs;
	long long retranssegs;
};

#define IM_IOC_GETCPUNUM	_IOR(IMKSTAT_IOC_MAGIC, 1, int)
#define IM_IOC_GETCPUKHZ	_IOR(IMKSTAT_IOC_MAGIC, 2, unsigned long)
#define IM_IOC_GETCURJIFFIES	_IOR(IMKSTAT_IOC_MAGIC, 3, unsigned long long)
#define IM_IOC_GETCPUDATA	_IOR(IMKSTAT_IOC_MAGIC, 4, struct cpu_data_type)
#define IM_IOC_GETNIDATA	_IOR(IMKSTAT_IOC_MAGIC, 5, struct ni_data_type)
#define IM_IOC_GETTCPDATA	_IOR(IMKSTAT_IOC_MAGIC, 6, struct tcp_data_type)

#define IM_IOC_MAXNR 7

#endif
