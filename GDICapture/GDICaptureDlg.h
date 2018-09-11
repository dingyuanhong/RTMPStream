
// GDICaptureDlg.h : header file
//

#pragma once

#include "desktop_and_cursor_composer.h"
#include "screen_capturer.h"
#include "mouse_cursor_monitor.h"
using namespace webrtc;

#include "avi/AviFile.h"
#include "Encode.h"
#include "x264Encoder.h"

// CGDICaptureDlg dialog
class CGDICaptureDlg : public CDialogEx
	,public webrtc::DesktopCapturer::Callback
	,public X264Encode::Callback
{
// Construction
public:
	CGDICaptureDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GDICAPTURE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
private:
	webrtc::DesktopAndCursorComposer * composer = NULL;
	webrtc::ScreenCapturer* screen = NULL;
	webrtc::MouseCursorMonitor* mouse_monitor = NULL;

	webrtc::DesktopCapturer *capturer = NULL;

	uint8_t * cache = NULL;

	void OnCaptureCompleted(webrtc::DesktopFrame* frame);
	void onPacket(uint8_t * packet, int len, int keyFrame, int64_t timestamp);

	void OnCaptureCompletedAVI(webrtc::DesktopFrame* frame);
	void OnCaptureCompletedMP4(webrtc::DesktopFrame* frame);

	void CheckCache(int size);

	CAviFile * file = NULL;
	X264Encode * encode = NULL;
	Encode *mp4 = NULL;
	bool first_encode = false;

	uint8_t * cache_e = NULL;
	int cache_e_max_len = 0;

	AVPacket * pkt = NULL;
	int index_frame = 0;
	int64_t start_time = 0;
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	void OnClose();
public:
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR);
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
};
