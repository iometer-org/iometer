/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / Network.cpp                                               ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implementation of generic methods for the Network     ## */
/* ##               class. Network is a pure virtual class that is used   ## */
/* ##               as a base class for any network connection. It does   ## */
/* ##               not contain any code except for the utility function  ## */
/* ##               OutputErrMsg().                                       ## */
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
/* ##  Remarks ...: - Network objects are used for connections between    ## */
/* ##                 network workers in Dynamo (contrast Port, which is  ## */
/* ##                 used for communication between Dynamo and Iometer). ## */
/* ##                 The Network class was designed for use in both      ## */
/* ##                 Iometer and Dynamo, but is currently used only in   ## */
/* ##                 Dynamo.                                             ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Changes ...: 2003-10-19 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Modified error message output destinction (Window   ## */
/* ##                 vs. Console) using _GALILEO_ define.                ## */
/* ##               2003-10-17 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-02-15 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##               - Different changes to support compilation with       ## */
/* ##                 gcc 3.2 (known as cout << hex error).               ## */
/* ##                                                                     ## */
/* ######################################################################### */

#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
#include <afx.h>
#endif

#include "Network.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Network::Network()
{
	errmsg = new ostringstream;
}

Network::~Network()
{
	delete errmsg;
}

//
// Utility function: output the "errmsg" message in an appropriate manner for the
// current environment (Dynamo or Iometer).
//
void Network::OutputErrMsg()
{
	if (!errmsg) {
		errmsg = new ostringstream;
		*errmsg << "Port::OutputErrMsg() called with invalid errmsg value!" << ends;
	}
#if defined(_GALILEO_)
	// Iometer
	ErrorMessage(errmsg->str().c_str());
#else
	// Dynamo
	cout << errmsg->str() << endl;
#endif

	// str() returns pointer to buffer and freezes it, we must call freeze(FALSE) to 
	// unfreeze the buffer before we can delete the object
	// ---
	// REMARK: freeze() no longer needed because new are now
	// using ostringstream instead of ostrstream.   
	//
	// errmsg->rdbuf()->freeze( FALSE );
	delete errmsg;

	errmsg = new ostringstream;
}
