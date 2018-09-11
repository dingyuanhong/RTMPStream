
// ShowDlg.h : header file
//

#pragma once
#include <stdint.h>
#include "../Stream/rtmp_pull.h"
#include "VideoDecoder.h"
#include "EvoQueue.hpp"

class RTMPPull;

// CShowDlg dialog
class CShowDlg : public CDialogEx
	, public RTMPPull::Callback
{
// Construction
public:
	CShowDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SHOW_DIALOG };
#endif

	void Run();
	void RunDecode();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
private:
	BITMAPINFO bmi_;

	void InitBITMAP(int width, int height);
	void SetBITMAPSize(int width, int height);
	void UpdateImage(AVFrame *frame);

	uint8_t * argb = NULL;
	int argb_len = 0;
	AVFrame *nowFrame = NULL;

	RTMPPull *pull = NULL;
	VideoDecoder * decoder = NULL;
	AVCodecContext	*codecContext = NULL;
	HANDLE hThread = NULL;
	HANDLE hThreadDecode = NULL;

	EvoQueue<AVPacket, av_packet_free> packet_pool;

	void onPacket(RTMPPacket *packet);
	void onPacketVideo(RTMPPacket *pkt);

	void onDataExtra(uint8_t * data, int size);
	void onDataRaw(uint8_t * data, int size, int keyFrame, int64_t timeStamp);
	void onDataPacket(uint8_t * data, int size, int keyFrame, int64_t timeStamp);

	int CreateDecoder(uint8_t * extData, int size);

	bool bStop = true;
	CCriticalSection mylock;
	uint64_t lastDraw;
	uint64_t delayDecode;
	int64_t delayNet;
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	LRESULT AFX_MSG_CALL onUpdateData(WPARAM, LPARAM);
};
