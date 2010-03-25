/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / TextDisplay.h                                          ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Interface for the CTextDisplay class, which provides  ## */
/* ##               the large text in the "Presentation Meter" dialog.    ## */
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
/* ##  Changes ...: 2003-10-17 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef TEXTDISPLAY_DEFINED
#define TEXTDISPLAY_DEFINED

#include "GalileoDefs.h"

enum TextAlignment {
	AlignLeft, AlignRight, AlignCenter
};

class CTextDisplay:public CWnd {
	DECLARE_DYNAMIC(CTextDisplay);

      public:

	CTextDisplay();
	void SetAll(CString * show_text, char *font_name, LONG font_size,
		    TextAlignment alignment, BOOL bold, BOOL italic, BOOL underline, COLORREF color);

	void SetText(CString * show_text, BOOL refresh = TRUE);

	void SetFontType(char *font_name, BOOL refresh = TRUE);
	void SetFontSize(LONG size, BOOL refresh = TRUE);
	void SetTextColor(COLORREF color, BOOL refresh = TRUE);

	void Bold(BOOL enable = TRUE, BOOL refresh = TRUE);
	void Italic(BOOL enable = TRUE, BOOL refresh = TRUE);
	void Underline(BOOL enable = TRUE, BOOL refresh = TRUE);

	void SetTextAlignment(TextAlignment alignment, BOOL refresh = TRUE);

      protected:
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextDisplay)
	 virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

	CString text;
	int text_alignment;	// left, right, center
	int text_position;	// location of text in text box
	COLORREF text_color;

	CFont font;
	LOGFONT font_settings;
	void UpdateFont(BOOL refresh = TRUE);

	RECT text_box;

	// Used to register the custom control with windows.
	static BOOL registered;
	static BOOL Register();

      protected:
	// Generated message map functions
	//{{AFX_MSG(CTextDisplay)
	 afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	//}}AFX_MSG
	 DECLARE_MESSAGE_MAP()

	static LRESULT CALLBACK EXPORT TextDisplayWndProc(HWND hWnd, UINT message, WPARAM xParam, LPARAM lParam);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif
