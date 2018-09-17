
// ShowDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Show.h"
#include "ShowDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "../Stream/x264_encoder.h"
#include "../Stream/librtmp_send264.h"

#include "../Stream/x264Encoder.h"
#include "../Stream/rtmp_push.h"
#include "../Stream/rtmp_pull.h"

extern "C" {
#include "../librtmp/log.h"
#include "../librtmp/rtmp.h"
}

#include "libyuv.h"
#include "EvoHeader.h"

#include <WinSock2.h>
#include <assert.h>

#pragma comment(lib,"ws2_32.lib")

//#pragma comment(lib,"zlibwapi.lib")
#pragma comment(lib,"zlibstat.lib")
#pragma comment(lib,"libeay32.lib")
#pragma comment(lib,"ssleay32.lib")
#pragma comment(lib,"libyuv.lib")
#pragma comment(lib,"x264.dll.lib")

#pragma comment(lib,"capture.lib")
#pragma comment(lib,"librtmp.lib")
#pragma comment(lib,"Stream.lib")

#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avdevice.lib")
#pragma comment(lib,"avfilter.lib")
#pragma comment(lib,"avutil.lib")

#include <timeapi.h>
#pragma comment(lib,"Winmm.lib")

#include <string>
std::string UnicodeToANSI(const std::wstring &wstrCmd)
{
	int bytes = ::WideCharToMultiByte(CP_ACP, 0, wstrCmd.c_str(), wstrCmd.size(), NULL, 0, NULL, NULL);
	std::string strCmd;
	strCmd.resize(bytes);
	bytes = ::WideCharToMultiByte(CP_ACP, 0, wstrCmd.c_str(), wstrCmd.size(), const_cast<char*>(strCmd.data()), strCmd.size(), NULL, NULL);
	return strCmd;
}

DWORD WINAPI thread_callback_read(
	LPVOID lpThreadParameter
	)
{
	CShowDlg * thiz = (CShowDlg*)lpThreadParameter;
	thiz->Run();
	return 0;
}
DWORD WINAPI thread_callback_decode(
	LPVOID lpThreadParameter
	)
{
	CShowDlg * thiz = (CShowDlg*)lpThreadParameter;
	thiz->RunDecode();
	return 0;
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CShowDlg dialog

#define WM_MESSAGE_UPDATE (WM_USER+1)

CShowDlg::CShowDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SHOW_DIALOG, pParent)
	,packet_pool(30)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CShowDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CShowDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_MESSAGE_UPDATE, onUpdateData)
	ON_BN_CLICKED(IDOK, &CShowDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CShowDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CShowDlg message handlers

BOOL CShowDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	//av_register_all();
	avcodec_register_all();

	InitBITMAP(1920,1080);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CShowDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CShowDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CShowDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CShowDlg::OnBnClickedOk()
{
	if (pull == NULL) {
		pull = new RTMPPull();
		pull->start(this);

	}

	if (pull->Handle() != NULL)
	{
		if (RTMP_IsConnected(pull->Handle())) {
			return;
		}
	}
	

	CString url;
	GetDlgItem(IDC_EDIT1)->GetWindowText(url);
	std::string strUrl = UnicodeToANSI(url.GetBuffer());
	int ret = pull->connect((char*)strUrl.c_str());
	if (ret != 1) {
		AfxMessageBox(_T("Á¬½ÓÊ§°Ü"));
	}
	else {
		bStop = false;
		hThread = CreateThread(NULL,0,thread_callback_read,(PVOID)this,0,NULL);
		hThreadDecode = CreateThread(NULL, 0, thread_callback_decode, (PVOID)this, 0, NULL);
	}
}

void CShowDlg::OnBnClickedCancel()
{
	bStop = true;
	if (pull != NULL) {
		pull->stop(true);
	}
	if (hThread != NULL)
	{
		WaitForSingleObject(hThread, INFINITE);
	}

	if (hThreadDecode != NULL)
	{
		WaitForSingleObject(hThreadDecode, INFINITE);
	}

	if (hThread != NULL)
	{
		CloseHandle(hThread);
		hThread = NULL;
	}

	if (hThreadDecode != NULL)
	{
		CloseHandle(hThreadDecode);
		hThreadDecode = NULL;
	}

	if (pull != NULL) {
		pull->close();
		delete pull;
		pull = NULL;
	}

	if (decoder != NULL)
	{
		delete decoder;
		decoder = NULL;
	}

	if (codecContext != NULL) {
		avcodec_close(codecContext);
		avcodec_free_context(&codecContext);
	}

	FreeAVFrame(&nowFrame);

	if (argb != NULL)
	{
		free(argb);
		argb = NULL;
	}

	CDialogEx::OnCancel();
}

void CShowDlg::InitBITMAP(int width, int height)
{
	memset(&bmi_, 0, sizeof(bmi_));
	bmi_.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi_.bmiHeader.biPlanes = 1;
	bmi_.bmiHeader.biBitCount = 32;
	bmi_.bmiHeader.biCompression = BI_RGB;
	bmi_.bmiHeader.biWidth = width;
	bmi_.bmiHeader.biHeight = -height;
	bmi_.bmiHeader.biSizeImage = width * height *
		(bmi_.bmiHeader.biBitCount >> 3);
}

void CShowDlg::SetBITMAPSize(int width, int height)
{
	if (width == bmi_.bmiHeader.biWidth && height == abs(bmi_.bmiHeader.biHeight)) {
		return;
	}
	bmi_.bmiHeader.biWidth = width;
	bmi_.bmiHeader.biHeight = -height;
	bmi_.bmiHeader.biSizeImage = width * height *
		(bmi_.bmiHeader.biBitCount >> 3);
}

static void DrawPicture(CWnd * wnd, const BITMAPINFO bmi, const uint8_t* image)
{
	int height = abs(bmi.bmiHeader.biHeight);
	int width = bmi.bmiHeader.biWidth;
	if (image == NULL) return;

	CWnd * pPicture = wnd;
	if (pPicture == NULL) return;

	HDC hdc = ::GetDC(pPicture->GetSafeHwnd());
	if (hdc == NULL) return;

	RECT rc;
	pPicture->GetClientRect(&rc);

	{
		HDC dc_mem = ::CreateCompatibleDC(hdc);
		::SetStretchBltMode(dc_mem, HALFTONE);

		// Set the map mode so that the ratio will be maintained for us.
		HDC all_dc[] = { hdc, dc_mem };
		for (int i = 0; i < ARRAYSIZE(all_dc); ++i) {
			SetMapMode(all_dc[i], MM_ISOTROPIC);
			SetWindowExtEx(all_dc[i], width, height, NULL);
			SetViewportExtEx(all_dc[i], rc.right, rc.bottom, NULL);
		}

		HBITMAP bmp_mem = ::CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
		HGDIOBJ bmp_old = ::SelectObject(dc_mem, bmp_mem);

		POINT logical_area = { rc.right, rc.bottom };
		DPtoLP(hdc, &logical_area, 1);

		HBRUSH brush = ::CreateSolidBrush(RGB(0, 0, 0));
		RECT logical_rect = { 0, 0, logical_area.x, logical_area.y };
		::FillRect(dc_mem, &logical_rect, brush);
		::DeleteObject(brush);

		int x = 0;
		int y = 0;

		StretchDIBits(dc_mem, x, y, logical_area.x, logical_area.y,
			0, 0, width, height, image, &bmi, DIB_RGB_COLORS, SRCCOPY);

		BitBlt(hdc, 0, 0, logical_area.x, logical_area.y,
			dc_mem, 0, 0, SRCCOPY);

		// Cleanup.
		::SelectObject(dc_mem, bmp_old);
		::DeleteObject(bmp_mem);
		::DeleteDC(dc_mem);
	}
	::ReleaseDC(pPicture->GetSafeHwnd(), hdc);
}

void CShowDlg::UpdateImage(AVFrame *frame)
{
	if (bStop) return;
	if (frame == NULL) return;

	int len = frame->width * frame->height * 4;
	if (argb == NULL || argb_len < len) {
		argb_len = len;
		if (argb != NULL) free(argb);
		argb = (uint8_t*)malloc(argb_len);
		memset(argb,0,argb_len);
	}

	if (frame->format == AV_PIX_FMT_YUV420P)
	{
		libyuv::I420ToARGB(frame->data[0], frame->linesize[0],
			frame->data[1], frame->linesize[1],
			frame->data[2], frame->linesize[2],
			argb, frame->width * 4,
			frame->width, frame->height);
	}

	CWnd * wnd = GetDlgItem(IDC_VIEW);
	SetBITMAPSize(frame->width, frame->height);
	DrawPicture(wnd, bmi_, argb);

	CString str;
	str.Format(_T("%lld"), timeGetTime() - lastDraw);
	GetDlgItem(IDC_DELAY_FRAME)->SetWindowText(str);
	str.Format(_T("%lld"), timeGetTime() - frame->pkt_duration);
	GetDlgItem(IDC_STATIC_TOTAL)->SetWindowText(str);
	lastDraw = timeGetTime();
}

LRESULT AFX_MSG_CALL CShowDlg::onUpdateData(WPARAM, LPARAM)
{
	AVFrame *tmpFrame = NULL;
	//mylock.Lock();
	tmpFrame = nowFrame;
	nowFrame = NULL;
	//mylock.Unlock();
	if (tmpFrame != NULL)
	{
		UpdateImage(tmpFrame);
		FreeAVFrame(&tmpFrame);
	}

	CString str;
	str.Format(_T("%lld"), delayDecode);
	GetDlgItem(IDC_DELAY_DECODE)->SetWindowText(str);

	str.Format(_T("%lld"), delayNet);
	GetDlgItem(IDC_DELAY_NET)->SetWindowText(str);
	return 0;
}

void CShowDlg::onDataExtra(uint8_t * data, int size) {
	CreateDecoder(data,size);
}

void CShowDlg::onDataRaw(uint8_t * data, int size,int keyFrame,int64_t timeStamp) {
	if (decoder == NULL) {
		return;
	}
	
	EvoPacket packet = { 0 };
	packet.data = data;
	packet.size = size;
	packet.flags = keyFrame;
	packet.timestamp = timeGetTime();
	packet.pts = timeStamp;
	packet.dts = timeStamp;

	AVFrame *outFrame = NULL;
	decoder->DecodePacket(&packet, &outFrame);
	if (outFrame != NULL)
	{
		int64_t time = timeGetTime();
		delayDecode = time - (uint64_t)outFrame->pkt_pos;

		AVFrame *tmpFrame = NULL;
		//mylock.Lock();
		tmpFrame = nowFrame;
		nowFrame = outFrame;
		//mylock.Unlock();
		FreeAVFrame(&tmpFrame);
		SendMessage(WM_MESSAGE_UPDATE);
	}
}

void CShowDlg::onDataPacket(uint8_t * data, int size, int keyFrame, int64_t timeStamp) {
	AVPacket * pkt = av_packet_alloc();
	av_new_packet(pkt,size);

	memcpy(pkt->data,data, pkt->size);
	pkt->pts = timeStamp;
	pkt->dts = timeStamp;
	pkt->pos = timeGetTime();
	pkt->duration = timeStamp;
	pkt->flags = keyFrame;

	int ret = packet_pool.Enqueue(pkt);
	if (ret == false)
	{
		av_packet_free(&pkt);
	}
}

void CShowDlg::onPacketVideo(RTMPPacket *pkt) {
	int bodyLen = pkt->m_nBodySize;
	if (bodyLen < 5) return;
	uint8_t * body = (uint8_t*)pkt->m_body;

	int keyFrame = (body[0] == 0x17);

	uint8_t * data = body + 5;
	int size = bodyLen - 5;
	if (body[1] == 0x00) {
		onDataExtra(data, size);
	}
	else if (body[1] == 0x01) {
		uint64_t time = pkt->m_nTimeStamp;
		uint64_t curr = timeGetTime();
		delayNet = curr - time;

		onDataPacket(data, size, keyFrame, time);
	}
}

void CShowDlg::onPacket(RTMPPacket *pkt)
{
	if (pkt->m_body == NULL) return;
	if (pkt->m_packetType == RTMP_PACKET_TYPE_VIDEO) {
		onPacketVideo(pkt);
	}
}

int CShowDlg::CreateDecoder(uint8_t * extData, int size) {
	if (decoder != NULL) return 0;

	AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!codec) return -1;
	codecContext = avcodec_alloc_context3(codec);

	uint8_t * extData_ = (uint8_t*)av_malloc(size);
	if (extData_ != NULL)
	{
		memcpy(extData_, extData, size);
		codecContext->extradata = extData_;
		codecContext->extradata_size = size;
	}

	//codecContext->thread_count = 4;
	if (avcodec_open2(codecContext, codec, NULL) < 0)
	{
		return -1;
	}
	decoder = new VideoDecoder(codecContext);
	decoder->SetKeepIFrame(true);
	return 0;
}

int readAble(int fd, uint32_t usec)
{
	struct timeval timeout;
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	timeout.tv_sec = usec / 1000000;
	timeout.tv_usec = usec % 1000000;
	int ret = select(0, &fds, NULL, &fds, &timeout);
	return ret;
}

void CShowDlg::Run()
{
	while (!bStop) {
		int n = pull->ReadPacket();
		readAble(RTMP_Socket(pull->Handle()), 1000);
	}
}

void CShowDlg::RunDecode()
{
	while (!bStop) {
		if (decoder == NULL) {
			sleep(1);
			continue;
		}

		AVPacket * pkt = packet_pool.Dequeue();
		if (pkt == NULL) {
			sleep(1);
		}

		AVFrame *outFrame = NULL;
		int ret = decoder->Decode(pkt, &outFrame);
		if (outFrame != NULL)
		{
			delayDecode = timeGetTime() - (uint64_t)outFrame->pkt_pos;

			AVFrame *tmpFrame = NULL;
			//mylock.Lock();
			tmpFrame = nowFrame;
			nowFrame = outFrame;
			//mylock.Unlock();
			FreeAVFrame(&tmpFrame);
			if (!bStop) {
				SendMessage(WM_MESSAGE_UPDATE);
			}
		}

		if (pkt != NULL)
		{
			av_packet_unref(pkt);
			av_packet_free(&pkt);
		}
	}
}