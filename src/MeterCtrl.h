/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer) / MeterCtrl.h                                            ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Interface for the CMeterCtrl class, which provides    ## */
/* ##               the "speedometer" control in the "Presentation Meter" ## */
/* ##               dialog.                                               ## */
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
#ifndef METERCTRL_DEFINED
#define METERCTRL_DEFINED

#include "TextDisplay.h"

// Angle of half pie slice not used by display, in degrees.
#define NEEDLE_SWEEP		270
#define PIVOT_ARC_ANGLE		(360 - NEEDLE_SWEEP) / 2

// Degree of change needed in result before needed is moved.
#define NEEDLE_SENSITIVITY	3	// degrees

class CMeterCtrl:public CWnd {
	DECLARE_DYNAMIC(CMeterCtrl)

      public:
	CMeterCtrl();

	// Members which handle window class registration.
	static BOOL Register();
	static BOOL is_registered;

      protected:

	///////////////////////////////////////////////////////////////////////////
	// Meter setup information.
      public:
	void SetRange(int range1, int range2);
	void SetValue(double new_value);
	//
      protected:
	int min_range;
	int max_range;
	double value;
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Information about low to high performance marks.
      public:
	void ResetWatermark();
	BOOL show_watermark;
	//
      protected:
	double low_value;
	double high_value;
	//
	RECT watermark_box;
	void DrawWatermark();
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Information about the position and size of the bounding control box.
	//
	POINT pivot_point;
	//
	// The display is bounded by two circles.  The outer circle defines the
	// limit of the display, and the text is located between the outer and 
	// inner circles.  The inner circle defines the limit of how far the 
	// needed reaches.  Two circles are needed to prevent the needle from
	// sweeping over the text markers.
	int outer_radius;
	int inner_radius;
	int label_radius;
	int tick_mark_radius;
	int pivot_radius;
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Managing the updates to the needle display.
	//
	void DrawNeedle();
	void CalculatePoint(int angle, int radius, LONG * x, LONG * y);
	void SetNeedlePoints();
	//
	int actual_angle;
	int shown_angle;
	//
	// The needle is drawn as a triangle, given three points.
	POINT needle[3];
	CRect needle_pivot;	// inner circle needle pivots around
	//
	POINT min_point, max_point;
	//
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Managing the updates to the display.
	//
	RECT meter_box;
	CFont scale_font;
	CFont label_font;
	//
	void UpdateScaleInfo();
	CString scale_text;
	RECT scale_box;
	//
	void UpdateLabelInfo();
	int label_box_size;	// used to adjust size of the labels
	RECT label_box[11];
	CString label[11];
	int label_count;
	//
	CPen marker_pen;
	POINT tick_mark[11][2];	// endpoints of tick marks
	//
	///////////////////////////////////////////////////////////////////////////

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMeterCtrl)
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CMeterCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	//}}AFX_MSG
	 DECLARE_MESSAGE_MAP()

	static LRESULT CALLBACK EXPORT MeterCtrlWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif				// !METERCTRL_DEFINED
