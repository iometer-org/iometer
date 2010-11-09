/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / IOCompletionQ.cpp                                         ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: UNIX implementations of several key Windows NT        ## */
/* ##               functions for asynchronous I/O (such as ReadFile(),   ## */
/* ##               WriteFile(), and GetQueuedCompletionStatus()),        ## */
/* ##               making it easier to change the Windows code without   ## */
/* ##               breaking the UNIX versions.                           ## */
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
/* ##               - ensure that all aio requests are canceled when the  ## */
/* ##                 Grunt finishes the test                             ## */
/* ##               - Support for MacOS X                                 ## */
/* ##               2004-09-01 (henryx.w.tieman@intel.com)                ## */
/* ##               - Added a little more initialization.                 ## */
/* ##               - Converted casts to fixed size types.                ## */
/* ##               2004-05-14 (lamontcranston41@yahoo.com)               ## */
/* ##               - Handle disk full condition in GetOverlappedResult.  ## */
/* ##               2004-03-26 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Code cleanup to ensure common style.                ## */
/* ##               - Applied Thayne Harmon's patch for supporting        ## */
/* ##                 Netware support (on I386).                          ## */
/* ##               2004-02-12 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Modified the GetQueuedCompletionStatus() function   ## */
/* ##                 according to the proposed code by Doug Haigh. This  ## */
/* ##                 ensures, that even in the case of an error, the     ## */
/* ##                 completion_key is set.                              ## */
/* ##               2003-10-15 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
 // nop
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_NETWARE) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)

#include "IOCommon.h"
#include <assert.h>

#if defined(IOMTR_OS_NETWARE)
#include "IOTest.h"
#endif

BOOL SetQueueSize(HANDLE cqid, int size)
{
	// Allocate memory for size elements in the completion queue.
	// and also for the parallel array of struct aiocb pointers.
	struct IOCQ *this_cqid;

	this_cqid = (struct IOCQ *)cqid;
#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
	this_cqid->element_list = (struct CQ_Element *)malloc(sizeof(CQ_Element) * size);
#elif defined(IOMTR_OS_NETWARE)
	this_cqid->element_list = (struct CQ_Element *)NXMemAlloc(sizeof(CQ_Element) * size, 1);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	if (this_cqid->element_list == NULL) {
		cout << "memory allocation failed" << endl;
		return (FALSE);
	}
#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
	this_cqid->aiocb_list = (struct aiocb64 **)malloc(sizeof(struct aiocb64 *) * size);
#elif defined(IOMTR_OS_NETWARE)
	this_cqid->aiocb_list = (struct aiocb64 **)NXMemAlloc(sizeof(struct aiocb64 *) * size, 1);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	if (this_cqid->aiocb_list == NULL) {
		cout << "memory allocation failed" << endl;
#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
		free(this_cqid->element_list);
#elif defined(IOMTR_OS_NETWARE)
		NXMemFree(this_cqid->element_list);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
		return (FALSE);
	}

	memset(this_cqid->element_list, 0, sizeof(struct CQ_Element) * size);
	memset(this_cqid->aiocb_list, 0, sizeof(struct aiocb64 *) * size);
	this_cqid->size = size;
	this_cqid->last_freed = -1;
	this_cqid->position = 0;

#ifdef _DEBUG
	cout << "allocated a completion queue of size " << size << " for handle : " << this_cqid << endl;
#endif
	return (TRUE);
}

//
// This function is the UNIX equivalent of the NT call to create a IO Completion Port.
// The behavior is similar to the NT call wherein a Completion Queue is created and
// associated with a file handle (if required).
//
// A Completion Queue is a structure that holds the status of the various asynch IO
// requests queued by the program.
// 
// The function returns a handle to the Completion Queue.
//
HANDLE CreateIoCompletionPort(HANDLE file_handle, HANDLE cq, DWORD completion_key, DWORD num_threads)
{
	struct File *filep;
	struct IOCQ *cqid;

	cqid = (struct IOCQ *)cq;
	if (cqid == NULL) {
		// cqid is NULL. We assign a new completion queue.
#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
		cqid = (struct IOCQ *)malloc(sizeof(struct IOCQ));
#elif defined(IOMTR_OS_NETWARE)
		cqid = (struct IOCQ *)NXMemAlloc(sizeof(struct IOCQ), 1);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
		if (cqid == NULL) {
			cout << "memory allocation failed. Exiting...." << endl;
			exit(1);
		}
		cqid->element_list = NULL;
		cqid->aiocb_list = NULL;
		cqid->size = 0;
		cqid->last_freed = -1;
		cqid->position = 0;
	}
	// cqid is not-NULL. Trying to assign an existing completion queue.
	// to the file handle.
	// If file_handle is INVALID then do nothing. 
	// We are required to create the comp queue only.
	filep = (struct File *)file_handle;
	if (filep != INVALID_HANDLE_VALUE) {
		filep->iocq = cqid;
		filep->completion_key = completion_key;
	}

	return ((HANDLE) cqid);
}

//
// This is the UNIX equivalent of the NT call CreateEvent() which creates an Event Queue.
// It is similar to a Completion Queue and works in the same way. The only difference is
// that an Event Queue is not associated with any particular file handle.
//
// This function returns a handle to an Event Queue.
//
// Note that this implementation currently supports only one event queue. So multiple
// calls to this function returns the same event queue handle.
//
HANDLE CreateEvent(void *, BOOL, BOOL, LPCTSTR)
{
	// We need to create an event queue.
	IOCQ *eventqid;

#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
	eventqid = (struct IOCQ *)malloc(sizeof(struct IOCQ));
#elif defined(IOMTR_OS_NETWARE)
	eventqid = (struct IOCQ *)NXMemAlloc(sizeof(struct IOCQ), 1);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	if (eventqid == NULL) {
		cout << "memory allocation failed. Exiting...." << endl;
		return (NULL);
	}
	eventqid->element_list = NULL;
	eventqid->aiocb_list = NULL;
	eventqid->size = 0;
	eventqid->last_freed = -1;
	eventqid->position = 0;

	return ((HANDLE) eventqid);
}

//
// Again, another NT-like call.
//
// The GetQueuedCompletionStatus() returns the status of exactly one completed
// (with or without errors) I/O. It searches the specified completion queue to
// see if any asynch I/O operation has completed. If not, then depending on 
// the timeout value it blocks the caller until one or more I/Os complete 
// using the aio_suspend() call.
//
// Note that it is the callers responsibility to call this function with the
// correct completion queue handle and timeout value.
//
// The function returns the #bytes transferred and the completion key (tied to
// the file handle and a pointer to void data.
//
BOOL GetQueuedCompletionStatus(HANDLE cq, LPDWORD bytes_transferred, LPDWORD completion_key,
			       LPOVERLAPPED * lpOverlapped, DWORD tmout)
{
	struct timespec *timeoutp;
	struct timespec timeout;
	struct IOCQ *cqid;
	int i, j;
	int aio_error_return;

	cqid = (struct IOCQ *)cq;

	//
	// We have two arrays of completion queue information.
	// The first one is the element_list[] which holds all info about each of the async
	// I/O request including the all important aiocb structure (for aio control block).
	//
	// The second array is the aiocb_list[], which is an array of pointers
	// to aiocb structures (see any of the aio_...() man pages for details on aiocb)
	// in the element_list[] and they are required since aio_suspend() requires the start 
	// address of a list of all aiocb pointers to be monitored and the size of the list.
	//
	// The way we search for AIO completion is very simple.
	// Start with current position (initially 0) in and then scan the list for completion.
	// If atleast one AIO is done return the status and set position to the next element
	// in the list. (that is where the next search begin - effectively simulating a round
	// -robin search.
	//
	// If none of the AIO is done, block with aio_suspend() until one or more AIOs
	// complete. When aio_suspend() returns, tag *all* of the completed AIOs and return.
	// This helps in avoiding two problems. One, aio_suspend() is not called as often as
	// it otherwise would have been. So, it improves performance. Two, the number of
	// outstanding I/Os queued to the kernel is not very large. That keeps the kernel happy.
	//
	// The next call to GetQueuedCompletionStatus() will pick up the completed AIOs.
	//
	// Network AIOs can be speeded up a bit by having an initial tagging loop that
	// will tag and break when an AIO completion is detected.
	//

	// IO Return Loop - return one AIO completion.

	i = cqid->position;

	for (j = 0; j < cqid->size; j++) {

		if (i == cqid->size) {
			i = 0;
		}

		if (cqid->element_list[i].done == TRUE) {

			// IO operation completed with either success or failure.

			*completion_key = cqid->element_list[i].completion_key;

			// Always set completion key

			*lpOverlapped = (LPOVERLAPPED) cqid->element_list[i].data;

			// Always set overlap data

			cqid->element_list[i].done = FALSE;
			cqid->last_freed = i;
			cqid->position = i + 1;
			*bytes_transferred = cqid->element_list[i].bytes_transferred;

			// We are returning the status of this aio. Set it to NULL to free the slot.

			cqid->aiocb_list[i] = 0;

			if ((DWORD) * bytes_transferred < (DWORD) 0) {
				*bytes_transferred = 0;
				// TODO: Here - and in the other locations where SetLastError()
				// is called in this method - we have the problem, that it is
				// set to
				// a.) defines defined by us - like WAIT_TIMEOUT
				// b.) whatever is in the errno variable
				// We can not realy be shure what each one is and if there is
				// maybe an overlaps, so we have to consolidate that in some
				// way.
				// As this method is called by CQAIO::GetStatus() (only?), we
				// have to considere changes there as well.
				SetLastError(cqid->element_list[i].error);
				return (FALSE);
			} else {
				return (TRUE);
			}
		}

		i++;
	}			// end of IO Return loop.

	//
	// Beyond this point return FALSE. No I/O has completed yet.
	// aio_suspend() till atleast one completes. But do not return
	// a completion. Only mark them.
	// 
	if (tmout == INFINITE)
		timeoutp = NULL;
	else {
		timeout.tv_sec = tmout / 1000;
		timeout.tv_nsec = (tmout % 1000) * 1000000;
		timeoutp = &timeout;
	}

	if (aio_suspend64(cqid->aiocb_list, cqid->size, timeoutp) < 0) {
		*lpOverlapped = NULL;
		*bytes_transferred = 0;
		*completion_key = 0;
		if ((errno == EAGAIN) || (errno == EINVAL)) {
#if defined(IOMTR_OS_LINUX)
			assert(errno == EAGAIN);
#endif
			SetLastError(WAIT_TIMEOUT);
		} else {
			SetLastError(errno);
		}

		return (FALSE);
	}
	// Tagging loop - to tag completed AIOs.
	for (j = 0; j < cqid->size; j++) {
		errno = 0;
		if (cqid->aiocb_list[j]) {
			if ((aio_error_return = aio_error64(cqid->aiocb_list[j])) != EINPROGRESS) {
				cqid->element_list[j].bytes_transferred = aio_return64(cqid->aiocb_list[j]);
				//
				// We have done an aio_return() on this element. Anull it.
				// The slot will be picked up by the next request.
				// But do not do it here. It should be done when the status
				// of this transaction is actually returned.
				// Else, ReadFile() and WriteFile() will pick up the slot
				// thinking it is empty.
				//
				// cqid->aiocb_list[j] = 0;
				cqid->element_list[j].done = TRUE;
				if (aio_error_return)
					cqid->element_list[j].error = aio_error_return;
				else if (errno)
					cqid->element_list[j].error = errno;
			}
		}
	}
	SetLastError(WAIT_TIMEOUT);
	return (FALSE);
}

//
// This call is very similar to GetQueuedCompletionStatus().
//
// The difference is that it searches for results on the event queue associated with
// a file handle. The call does one quick scan of the event queue and returns an I/O
// completion.
// Depending on a wait value being TRUE or FALSE, the call either blocks indefinitely
// scans the event queue just once waiting for an I/O completion.
//
BOOL GetOverlappedResult(HANDLE file_handle, LPOVERLAPPED lpOverlapped, LPDWORD bytes_transferred, BOOL wait)
{
	struct timespec *timeoutp;
	struct timespec timeout;
	struct File *filep;
	int this_fd;
	int i, j;
	int aio_error_return;
	IOCQ *eventqid;

	//
	// This function either blocks for ever or scans the AIO list just once for completions.
	//
	filep = (struct File *)file_handle;
	if (wait == TRUE)
		timeoutp = NULL;
	else {
		timeout.tv_sec = 0;
		timeout.tv_nsec = 0;
		timeoutp = &timeout;
	}

	//
	// GetOverlappedResult() function blocks until one or more AIOs complete and then
	// returns a completion status.
	// The search for an AIO completion is very similar to the GetQueuedCompletionStatus()
	// function.
	//
	// get a handle to the current event queue.
	eventqid = (IOCQ *) ((ULONG_PTR) lpOverlapped->hEvent ^ 0x1);

	// Call aio_suspend() now.
	if (aio_suspend64(eventqid->aiocb_list, eventqid->size, timeoutp) < 0) {
		if ((errno == EAGAIN) || (errno == EINVAL)) {
			// No operations completed in the given timeout.
			// Note that changing lpOverlapped has no effect. Its a local copy.
			lpOverlapped = NULL;
			*bytes_transferred = 0;
			SetLastError(WAIT_TIMEOUT);
			return (FALSE);
		}
#ifdef _DEBUG
		cout << "aio_suspend returned error " << errno << endl;
#endif
		SetLastError(errno);
		return (FALSE);
	}
	// aio_suspend() returned successfully. Check if atleast one of the
	// I/Os completed. must have!.
	i = eventqid->position;
	for (j = 0; j < eventqid->size; j++) {
		if (i == eventqid->size)
			i = 0;

		errno = 0;
		if (eventqid->aiocb_list[i]) {
			if ((aio_error_return = aio_error64(eventqid->aiocb_list[i])) != EINPROGRESS) {
				this_fd = eventqid->element_list[i].aiocbp.aio_fildes;
				if (this_fd == filep->fd) {
					DWORD bytes_expected;

					*bytes_transferred = (DWORD) aio_return64(eventqid->aiocb_list[i]);

					bytes_expected = eventqid->aiocb_list[i]->aio_nbytes;

					// We have done an aio_return() on this element. So anull it.

					eventqid->aiocb_list[i] = 0;
					eventqid->last_freed = i;
					eventqid->position = i + 1;

					// Note that changing lpOverlapped has no effect. Its a local copy.

					lpOverlapped = (LPOVERLAPPED) eventqid->element_list[i].data;

					if (*bytes_transferred != bytes_expected) {
						if (aio_error_return == ENOSPC) {
							SetLastError(aio_error_return);
							return (FALSE);
						}
					}

					if ((DWORD) * bytes_transferred < 0) {
						*bytes_transferred = 0;
						if (aio_error_return)
							SetLastError(aio_error_return);
						else if (errno)
							SetLastError(errno);

						return (FALSE);
					} else
						return (TRUE);
				}
			}
		}
		i++;
	}			// end of for() loop
	// At this point NO I/O has completed. Return WAIT_TIMEOUT and FALSE.
	SetLastError(WAIT_TIMEOUT);
	return (FALSE);
}

//
// ReadFile() reads "bytes_to_read" bytes from the file_handle into the buffer.
// The call uses asynch I/O routine aio_read().
//
// ReadFile() checks the Overlapped structure to determine the read offset in the file handle.
// It also determines from the Overlapped structure if the I/O completion status should be 
// posted on the event queue or the completion queue associated with the file handle.
//
BOOL ReadFile(HANDLE file_handle, void *buffer, DWORD bytes_to_read, LPDWORD bytes_read, LPOVERLAPPED lpOverlapped)
{
	struct File *filep;
	struct IOCQ *this_cq;
	struct aiocb64 *aiocbp;
	int i, free_index = -1;

#ifdef IMMEDIATE_AIO_COMPLETION
	int aio_error_return;
#endif

	filep = (struct File *)file_handle;
	//
	// At this point we have to decide whether to place this in the Completion queue
	// or the event queue.
	if ((ULONG_PTR) lpOverlapped->hEvent & 0x00000001) {
		// forcibly place this on the event queue even though a completion queue is associated
		// with the file. Well, thats what you asked for.
		this_cq = (IOCQ *) ((ULONG_PTR) lpOverlapped->hEvent ^ 0x1);
	} else
		this_cq = filep->iocq;

	if (this_cq == NULL) {
		cout << "event or completion queue not allocated " << endl;
		return (FALSE);
	}
	// First locate an empty slot in the queue.
	if (this_cq->last_freed != -1) {
		free_index = this_cq->last_freed;
		this_cq->last_freed = -1;	// the slot is taken. Thanks
	} else {
		// search for a free index. 
		for (i = 0; i < this_cq->size; i++) {
			if (this_cq->aiocb_list[i] == NULL) {
				free_index = i;
				break;
			}
		}
	}

	// either free_index holds the free index or there is no free space in the Q.
	if (free_index == -1)
		return (FALSE);

	aiocbp = &this_cq->element_list[free_index].aiocbp;

	aiocbp->aio_buf = buffer;
	aiocbp->aio_fildes = filep->fd;
	aiocbp->aio_nbytes = bytes_to_read;
	aiocbp->aio_offset = (off64_t) lpOverlapped->OffsetHigh;
	aiocbp->aio_offset = aiocbp->aio_offset << 32;
	aiocbp->aio_offset += lpOverlapped->Offset;
	aiocbp->aio_sigevent.sigev_notify = SIGEV_NONE;

	this_cq->element_list[free_index].data = lpOverlapped;
	this_cq->element_list[free_index].bytes_transferred = 0;
	this_cq->element_list[free_index].completion_key = filep->completion_key;

	*bytes_read = 0;

#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
	if (aio_read64(&this_cq->element_list[free_index].aiocbp) < 0)
#elif defined(IOMTR_OS_NETWARE)
	if (aio_read64(&this_cq->element_list[free_index].aiocbp, filep->type) < 0)
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	{
		if (errno == EAGAIN)
		{
			cout << "aio_read64 failed with EAGAIN -- system limitation???" << endl;
		}
		else
		{
			cout << "queuing for read failed with error " << errno << endl;
		}
		// Note that we have not set aiocb_list[] with the correct pointers.
		// So, this slot will get grabbed in the next loop.
		SetLastError(errno);
		return (FALSE);
	}
#ifdef IMMEDIATE_AIO_COMPLETION
	// Check if the aio_read completed successfully.
	if ((aio_error_return = aio_error64(&this_cq->element_list[free_index].aiocbp)) != EINPROGRESS) {
		*bytes_read = (DWORD) aio_return64(&this_cq->element_list[free_index].aiocbp);
		if ((long)*bytes_read < 0) {
			*bytes_read = 0;
			if (aio_error_return)
				SetLastError(aio_error_return);
			else if (errno)
				SetLastError(errno);
			return (FALSE);
		} else {
			SetLastError(0);
			return (TRUE);
		}
	}
#endif
	else
		// aio_read is in progress. We have to set the aiocb_list[] to point correctly.
		this_cq->aiocb_list[free_index] = aiocbp;

	SetLastError(ERROR_IO_PENDING);
	return (FALSE);
}

//
// WriteFile() writes "bytes_to_write" bytes from the buffer into the file pointed to by
// the file handle.
// The call uses asynch I/O routine aio_write().
//
// WriteFile() checks the Overlapped structure to determine the write offset in the file handle.
// It also determines from the Overlapped structure if the I/O completion status should be 
// posted on the event queue or the completion queue associated with the file handle.
//
BOOL WriteFile(HANDLE file_handle, void *buffer, DWORD bytes_to_write, LPDWORD bytes_written, LPOVERLAPPED lpOverlapped)
{
	struct File *filep;
	struct IOCQ *this_cq;
	struct aiocb64 *aiocbp;
	int i, free_index = -1;

#ifdef IMMEDIATE_AIO_COMPLETION
	int aio_error_return;
#endif

	filep = (struct File *)file_handle;
	//
	// At this point we have to decide whether to place this in the Completion queue
	// or the event queue.
	if ((ULONG_PTR) lpOverlapped->hEvent & 0x00000001) {
		// forcibly place this on the event queue even though a completion queue is associated
		// with the file. Well, thats what you asked for.
		this_cq = (IOCQ *) ((ULONG_PTR) lpOverlapped->hEvent ^ 0x1);
	} else
		this_cq = filep->iocq;

	if (this_cq == NULL) {
		cout << "event or completion queue not allocated " << endl;
		return (FALSE);
	}
	// First locate an empty slot in the queue.
	if (this_cq->last_freed != -1) {
		free_index = this_cq->last_freed;
		this_cq->last_freed = -1;	// the slot is taken. Thanks
	} else
		// search for a free index. 
		for (i = 0; i < this_cq->size; i++) {
			if (this_cq->aiocb_list[i] == NULL) {
				free_index = i;
				break;
			}
		}

	// either free_index holds the free index or there is no free space in the Q.
	if (free_index == -1)
		return (FALSE);

	aiocbp = &this_cq->element_list[free_index].aiocbp;

	aiocbp->aio_buf = buffer;
	aiocbp->aio_fildes = filep->fd;
	aiocbp->aio_nbytes = bytes_to_write;
	aiocbp->aio_offset = (off64_t) lpOverlapped->OffsetHigh;
	aiocbp->aio_offset = aiocbp->aio_offset << 32;
	aiocbp->aio_offset += lpOverlapped->Offset;
	aiocbp->aio_sigevent.sigev_notify = SIGEV_NONE;

	this_cq->element_list[free_index].data = lpOverlapped;
	this_cq->element_list[free_index].bytes_transferred = 0;
	this_cq->element_list[free_index].completion_key = filep->completion_key;

	*bytes_written = 0;

#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
	if (aio_write64(&this_cq->element_list[free_index].aiocbp) < 0)
#elif defined(IOMTR_OS_NETWARE)
	if (aio_write64(&this_cq->element_list[free_index].aiocbp, filep->type) < 0)
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
	{
		cout << "queuing for write failed with error " << errno << endl;
		// Note that we have not set aiocb_list[] with the correct pointers.
		// So, this slot will get grabbed in the next loop.
		SetLastError(errno);
		return (FALSE);
	}
#ifdef IMMEDIATE_AIO_COMPLETION
	// Check if the aio_write completed successfully.
	if ((aio_error_return = aio_error64(&this_cq->element_list[free_index].aiocbp)) != EINPROGRESS) {
		*bytes_written = (DWORD) aio_return64(&this_cq->element_list[free_index].aiocbp);
		if ((long)*bytes_written < 0) {
			*bytes_written = 0;
			if (aio_error_return)
				SetLastError(aio_error_return);
			else if (errno)
				SetLastError(errno);
			return (FALSE);
		} else {
			SetLastError(0);
			return (TRUE);
		}
	}
#endif
	else
		// aio_write is in progress. We have to set the aiocb_list[] to point correctly.
		this_cq->aiocb_list[free_index] = aiocbp;

	SetLastError(ERROR_IO_PENDING);
	return (FALSE);
}

//
// CloseHandle() has a slightly different interface from the NT call. It takes an
// additional input parameter to determine the object type. The object can be either
// a FILE_ELEMENT or a CQ_ELEMENT.
//
// CloseHandle() closes the file handle or the completion queue handle and frees all
// the allocated memory. It does an additional check to ensure that all the queued
// Asynch I/Os that have not yet completed are cancelled before actually closing the
// handle. This helps clean up the kernel queues of any pending requests.
//
// Although it takes longer, this is acceptable since the code is not in the performance 
// critical region.
// 
BOOL CloseHandle(HANDLE object, int object_type)
{
	struct File *filep;
	struct IOCQ *cqid;
	int retval, i;

#ifdef _DEBUG
	cout << "CloseHandle() freeing : handle = " << object << " objecttype = " << object_type << endl;
#endif

	switch (object_type) {
	case FILE_ELEMENT:
		filep = (struct File *)object;
		cqid = filep->iocq;
		// cancel any pending aio requests.
		retval = aio_cancel64(filep->fd, NULL);
		while (retval == AIO_NOTCANCELED) {
			retval = aio_cancel64(filep->fd, NULL);
		}

		if (cqid != NULL && cqid->element_list != NULL && cqid->aiocb_list != NULL) {
			for (i = 0; i < cqid->size; i++) {
				if (cqid->element_list[i].aiocbp.aio_fildes != filep->fd)
					continue;

				// We are not interested in the return values of aio_error() and aio_return().
				// only have to dequeue all the requests.
				if (!cqid->aiocb_list[i])
					continue;

				retval = aio_error64(cqid->aiocb_list[i]);
				retval = aio_return64(cqid->aiocb_list[i]);
			}
		}
#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
		close(filep->fd);
#elif defined(IOMTR_OS_NETWARE)
		if (IsType(filep->type, LogicalDiskType))
			NXClose(filep->fd);
		else if (IsType(filep->type, PhysicalDiskType))
			MM_ReleaseIOObject(filep->fd);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
		break;
	case CQ_ELEMENT:
		cqid = (struct IOCQ *)object;

		// cancel any pending aio requests.
		for (i = 0; i < cqid->size; i++) {
			if (!cqid->aiocb_list[i])
				continue;

#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_NETWARE)
			/*
			 * In Linux, you crash (!) if the aiocpb isn't in your queue. :-(
			 * This code seems to occasionally do this...so I just cancel all
			 * AIOs for the queue, thus avoiding the problem of cancelling a
			 * message not in the queue.
			 */
			retval = aio_cancel64(cqid->element_list[i].aiocbp.aio_fildes, NULL);
#elif defined(IOMTR_OS_SOLARIS)
			retval = aio_cancel64(cqid->element_list[i].aiocbp.aio_fildes, cqid->aiocb_list[i]);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
			if (retval == AIO_NOTCANCELED) {
				retval = aio_error64(cqid->aiocb_list[i]);
				retval = aio_return64(cqid->aiocb_list[i]);
			}
		}

#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
		free(cqid->aiocb_list);
#elif defined(IOMTR_OS_NETWARE)
		NXMemFree(cqid->aiocb_list);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
		// Something strange here. If I free the element_list, the next round
		// of aio_write() and aio_read() calls fail. If I dont free this, then they
		// succeed. But then, there is a memory leak equal to the max number of outstanding
		// I/Os * sizeof(CQ_Element).    Does that mean that the above aio_cancel() calls
		// are broken ??? It should be mentioned here that the element_list holds the 
		// actual aiocb structures.
		//
		// It suddenly seems to be working now.
		// Remember to turn this "free" off when you hit the problem again.
		// NEED TO LOOK INTO THIS.
#if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_OSX) || defined(IOMTR_OS_SOLARIS)
		free(cqid->element_list);
		free(cqid);
#elif defined(IOMTR_OS_NETWARE)
		NXMemFree(cqid->element_list);
		NXMemFree(cqid);
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
		break;
	default:
		break;
	}
	return (TRUE);
}

//
// Here are some simple self-explanatory routines.
//
// A note on "errno". 
//              According to the threads man pages and documents, when the compilation symbol 
// _REENTRANT is defined, the all important global error variable "errno" is made 
// thread-specific. This is done in the errno.h
//
// Note: This is a demand of the pthreads package. I am taking out the
// "_REENTRANT" flags and taking out the solaris references. -wms
//
void SetLastError(DWORD num)
{
	errno = num;
}

DWORD GetLastError(void)
{
	int error_to_return;

	error_to_return = errno;
	return (error_to_return);
}

void WSASetLastError(DWORD num)
{
	errno = num;
}

int WSAGetLastError(void)
{
	int error_to_return;

	error_to_return = errno;
	errno = 0;
	return (error_to_return);
}

//
// Simple utility routine.
char *_itoa(int value, char *string, int radix)
{
	if (radix == 10)
		sprintf(string, "%d", value);
	else {
		cerr << "_itoa() not implemented for radix != 10" << endl;
		exit(1);
	}
	return (string);
}

unsigned int SetErrorMode(unsigned int umode)
{
	// This function does nothing. Just a dummy equivalent of NT calls.
	// But it can be made more functional, if required.
	return (0);
}

#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
