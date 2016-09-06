/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / LegalBox.cpp                                           ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: ...                                                   ## */
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
/* ##  Changes ...: 2004-07-24 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Replaced the license claimed in the EULA to the     ## */
/* ##                 Intel Open Source License text only.                ## */
/* ##               2003-08-05 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Replaced the license claimed in the EULA to the     ## */
/* ##                 Intel Open Source License as this the current       ## */
/* ##                 under which the Source Code and Binaries are        ## */
/* ##                 distributed.                                        ## */
/* ##                 EULA sections which are obsolet or stays in         ## */
/* ##                 contrast to that (namely LIMITED MEDIA WARRANTY,    ## */
/* ##                 TERMINATION OF THIS LICENSE and U.S. GOVERNMENT     ## */
/* ##                 RESTRICTED RIGHTS) has been blanked out.            ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-04-25 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Updated the global debug flag (_DEBUG) handling     ## */
/* ##                 of the source file (check for platform etc.).       ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include "stdafx.h"
#include "GalileoApp.h"
#include "LegalBox.h"

// Needed for MFC Library support for assisting in finding memory leaks
//
// NOTE: Based on the documentation[1] I found, it should be enough to have
//       a "#define new DEBUG_NEW" statement for the case, that we are
//       running Windows. There should be no need for checking the _DEBUG
//       flag and no need for redefiniting the THIS_FILE string. Maybe there
//       will be a MFC hacker who could advice here.
//       [1] = http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclib/html/_mfc_debug_new.asp
//
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
#ifdef IOMTR_SETTING_MFC_MEMALLOC_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// CLegalBox dialog

CLegalBox::CLegalBox(CWnd * pParent /*=NULL*/ )
:  CDialog(CLegalBox::IDD, pParent)
{
	running = FALSE;
	//{{AFX_DATA_INIT(CLegalBox)
	m_test = _T("");
	//}}AFX_DATA_INIT
}

void CLegalBox::DoDataExchange(CDataExchange * pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLegalBox)
	DDX_Control(pDX, ELegal, m_ELegal);
	DDX_Text(pDX, ELegal, m_test);
	DDV_MaxChars(pDX, m_test, 100000);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLegalBox, CDialog)
    //{{AFX_MSG_MAP(CLegalBox)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CLegalBox message handlers
BOOL CLegalBox::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString legal;

/*
	legal = "INTEL SOFTWARE LICENSE AGREEMENT\x0d\x0a\
IMPORTANT - READ BEFORE COPYING, INSTALLING OR USING\x0d\x0a \x0d\x0a";

	legal += "Do not use or load this software or any associated materials \
(collectively, the \"Software\") until you have carefully read the \
following terms and conditions.  By loading or using the Software, you \
agree to the terms of this Agreement.  If you do not wish to so agree, \
do not install or use the Software.\x0d\x0a \x0d\x0a";
*/

/*
	legal += "LICENSE:  You may copy the Software onto your organization's computers \
or file servers and may make one back-up copy of the Software, subject \
to these conditions:\x0d\x0a \x0d\x0a";

	legal += "1.  You may not copy, modify, rent, sell, distribute or transfer any \
part of the Software except as provided in this Agreement, and you \
agree to prevent unauthorized copying of the Software.\x0d\x0a \x0d\x0a";

	legal += "2.  You may not reverse engineer, decompile, or disassemble the \
Software.\x0d\x0a \x0d\x0a";

	legal += "3.  You may not sublicense or permit simultaneous use of the Software by \
more than one user.\x0d\x0a \x0d\x0a";

	legal += "4.  The Software may include portions offered on terms in addition to \
those set out here, as set out in a license accompanying those portions.\x0d\x0a \x0d\x0a";
*/
/*
	legal += "LICENSE: \x0d\x0a \x0d\x0a";
*/
	legal = "";
	legal += "Intel Open Source License \x0d\x0a \x0d\x0a";

	legal += "Copyright (c) 2001 Intel Corporation \
All rights reserved. \
Redistribution and use in source and binary forms, with or without \
modification, are permitted provided that the following conditions are met:\x0d\x0a \x0d\x0a";

	legal += "   Redistributions of source code must retain the above copyright notice, \
   this list of conditions and the following disclaimer.\x0d\x0a \x0d\x0a";

	legal += "   Redistributions in binary form must reproduce the above copyright notice, \
   this list of conditions and the following disclaimer in the documentation \
   and/or other materials provided with the distribution.\x0d\x0a \x0d\x0a";

	legal += "   Neither the name of the Intel Corporation nor the names of its contributors \
   may be used to endorse or promote products derived from this software \
   without specific prior written permission.\x0d\x0a \x0d\x0a";

	legal += "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS'' \
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE \
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE \
ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR ITS  CONTRIBUTORS BE LIABLE \
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL \
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR \
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER \
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, \
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE \
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\x0d\x0a \x0d\x0a";

	legal += "OWNERSHIP AND COPYRIGHT OF SOFTWARE:  Title to the Software and all \
copies thereof remains with Intel or its suppliers.  The Software is \
copyrighted and is protected by the laws of the United States and other \
countries, and international treaty provisions.  You may not  remove any \
copyright notice from the Software.  Intel may make changes to the \
Software, or to items referenced therein, at any time without notice, \
but is not obligated to support or update the Software.  Except as \
otherwise expressly provided, Intel grants no express or implied rights \
under Intel patents, copyrights, trademarks, or other intellectual \
property rights.\x0d\x0a \x0d\x0a";

	legal += "EXCLUSION OF OTHER WARRANTIES:  EXCEPT AS PROVIDED ABOVE, THE SOFTWARE \
IS PROVIDED \"AS IS\" WITHOUT ANY EXPRESS OR IMPLIED WARRANTY OF ANY KIND, \
INCLUDING WARRANTIES OF MERCHANTABILITY, NONINFRINGEMENT, OR FITNESS FOR \
A PARTICULAR PURPOSES.  Intel does not warrant or assume responsibility \
for the accuracy or completeness of any information, text, graphics, \
links or other items contained within the Software.\x0d\x0a \x0d\x0a";

	legal += "LIMITATION OF LIABILITY:  IN NO EVENT SHALL INTEL OR ITS SUPPLIERS BE \
LIABLE FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION, LOST \
PROFITS, BUSINESS INTERRUPTION, OR LOST INFORMATION) ARISING OUT OF THE \
USE OF OR INABILITY TO USE THE SOFTWARE, EVEN IF INTEL HAS BEEN ADVISED \
OF THE POSSIBILITY OF SUCH DAMAGES.  SOME JURISDICTIONS PROHIBIT \
EXCLUSION OR LIMITATION OF LIABILITY FOR IMPLIED WARRANTIES OR \
CONSEQUENTIAL OR INCIDENTAL DAMAGES, SO THE ABOVE LIMITATION MAY NOT \
APPLY TO YOU.  YOU MAY ALSO HAVE OTHER LEGAL RIGHTS THAT VARY FROM \
JURISDICTION TO JURISDICTION.\x0d\x0a \x0d\x0a";

	legal += "INDEMNIFICATION:  Intel does not endorse, support or guarantee \
performance results obtained using the Software.  Any difference in \
system hardware or software design or configuration may affect actual \
performance.  Buyers should consult other sources of information to \
evaluate the performance of systems or components they are considering \
purchasing.  You agree to defend, indemnify and hold Intel harmless from \
and against any and all actions, claims, damages, expenses (including \
attorney’s fees) and liabilities arising from your use of the Software.\x0d\x0a \x0d\x0a";

	legal += "APPLICABLE LAW: Claims arising under this Agreement shall be governed by \
the laws of California, excluding its principles of conflict of laws and \
the United Nations Convention on Contracts for the Sale of Goods.  You \
may not export the Materials in violation of applicable export laws and \
regulations.  Intel is not be obligated under any other agreements \
unless they are in writing and signed by an authorized representative of \
Intel.";

	m_ELegal.SetWindowText(legal);

	if (running) {
		GetDlgItem(IDCANCEL)->ModifyStyle(WS_VISIBLE, WS_DISABLED);
		GetDlgItem(IDOK)->SetWindowText("OK");
	}

	return TRUE;		// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
