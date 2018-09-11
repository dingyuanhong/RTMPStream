
// GDICaptureDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GDICapture.h"
#include "GDICaptureDlg.h"
#include "afxdialogex.h"

#include "screen_capturer.h"
#include "desktop_capture_options.h"
#include "desktop_frame.h"

#include "x264Encoder.h"
#include "libyuv.h"
#include <assert.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib,"ws2_32.lib")

#pragma comment(lib,"zlibstat.lib")
#pragma comment(lib,"libeay32.lib")
#pragma comment(lib,"ssleay32.lib")
#pragma comment(lib,"libyuv.lib")
#pragma comment(lib,"x264.dll.lib")

#pragma comment(lib,"capture.lib")
#pragma comment(lib,"Stream.lib")

#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avdevice.lib")
#pragma comment(lib,"avfilter.lib")
#pragma comment(lib,"avutil.lib")

#define WM_TIME_CAPTURE (WM_USER + 1)
// CGDICaptureDlg dialog

CGDICaptureDlg::CGDICaptureDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_GDICAPTURE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGDICaptureDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CGDICaptureDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDCANCEL, &CGDICaptureDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CGDICaptureDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CGDICaptureDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CGDICaptureDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CGDICaptureDlg message handlers

BOOL CGDICaptureDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	av_register_all();
	pkt = av_packet_alloc();
	av_init_packet(pkt);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CGDICaptureDlg::OnPaint()
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
HCURSOR CGDICaptureDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CGDICaptureDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
}

void CGDICaptureDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}


void CGDICaptureDlg::OnBnClickedOk()
{
}


void CGDICaptureDlg::OnBnClickedButton1()
{
	OnClose();

	webrtc::DesktopCaptureOptions options;
	if (screen == NULL)
	{
		screen = webrtc::ScreenCapturer::Create(options);
		webrtc::ScreenCapturer::ScreenList list;
		screen->GetScreenList(&list);
		screen->SelectScreen(list[0].id);
	}
	
	if (mouse_monitor == NULL)
	{
		mouse_monitor = webrtc::MouseCursorMonitor::CreateForScreen(options, 0);
	}

	//capturer = screen;

	composer = new webrtc::DesktopAndCursorComposer(screen,mouse_monitor);
	screen = NULL;
	mouse_monitor = NULL;
	capturer = composer;

	capturer->Start(this);

	first_encode = true;
	mp4 = new Encode();
	mp4->Open("Output.mp4");

	file = new CAviFile(_T("Output.Avi"), mmioFOURCC('M', 'S', 'V', 'C'), 5);

	SetTimer(WM_TIME_CAPTURE,1000/30,NULL);
}

void CGDICaptureDlg::OnBnClickedButton2()
{
	OnClose();
}

void CGDICaptureDlg::OnClose()
{
	KillTimer(WM_TIME_CAPTURE);

	if (mp4 != NULL)
	{
		int ret = mp4->WriteTrailer();
		assert(ret == 0);

		delete mp4;

		mp4 = NULL;
	}

	index_frame = 0;

	if (file != NULL)
	{
		delete file;
		file = NULL;
	}

	if (composer != NULL)
	{
		delete composer;
		composer = NULL;
	}
	if (screen != NULL)
	{
		delete screen;
		screen = NULL;
	}
	if (mouse_monitor != NULL)
	{
		delete mouse_monitor;
		mouse_monitor = NULL;
	}


	if (capturer != NULL)
	{
		capturer = NULL;
	}

	if (encode != NULL) 
	{
		encode->close();
		delete encode;
		encode = NULL;
	}

	if (cache != NULL)
	{
		free(cache);
		cache = NULL;
	}

	if (cache_e != NULL)
	{
		free(cache_e);
		cache_e = NULL;
	}

	first_encode = false;
}

void RGBAtRGB(uint8_t* src,int width,int height, uint8_t *des) {
	for (int y = height - 1; y >= 0; y--) {
		for (int x = 0; x < width; x += 1) {
			des[(height - 1 - y) * width * 3 + x * 3] = src[y * width * 4 + x*4];
			des[(height - 1 - y) * width * 3 + x * 3 + 1] = src[y * width * 4 + x * 4 + 1];
			des[(height - 1 - y) * width * 3 + x * 3 + 2] = src[y * width * 4 + x * 4 + 2];
		}
	}
}

void CGDICaptureDlg::OnCaptureCompletedAVI(webrtc::DesktopFrame* frame)
{
	if (file != NULL) {
		int width = frame->size().width();
		int height = frame->size().height();
		if (cache == NULL) {
			cache = (uint8_t*)malloc(width*height * 3);
		}
		RGBAtRGB(frame->data(), width, height, cache);

		HRESULT res = file->AppendNewFrame(width, height, cache, 3 * 8);
		if (res == E_FAIL) {
			delete file;
			file = NULL;
		}
	}
}

void CGDICaptureDlg::OnCaptureCompletedMP4(webrtc::DesktopFrame* frame)
{
	int width = frame->size().width();
	int height = frame->size().height();

	if (encode == NULL)
	{
		encode = new X264Encode();
		encode->init(width, height, 30, 3*1024*1024, X264_CSP_I420, false);
		encode->start(this);

		AVStream stream = {0};
		stream.time_base = {1,10000};
		stream.start_time = timeGetTime();
		stream.r_frame_rate = { 30,1 };
		stream.avg_frame_rate = { 30, 1 };

		mp4->NewVideoStream(&stream,width,height, AV_PIX_FMT_YUV420P);

		int l = encode->spsLen();
		l += encode->ppsLen();
		l += 8;
		uint8_t * extraData = (uint8_t*)av_malloc(l);
		int index = 0;
		memcpy(extraData + index, encode->getSPS(),encode->spsLen());
		index += encode->spsLen();
		memcpy(extraData + index, encode->getPPS(), encode->ppsLen());
		
		AVCodecContext * codecContent = mp4->GetCodecContext(AVMEDIA_TYPE_VIDEO);
		codecContent->extradata = extraData;
		codecContent->extradata_size = l;

		int ret = mp4->WriteHeader();
		assert(ret == 0);
		if (ret != 0) {
			char buffer[255];
			av_strerror(ret, buffer, 255);
			return;
		}

		start_time = timeGetTime();
	}

	if (encode == NULL) {
		return;
	}

	x264_picture_t * input = encode->LockInput();
	while (input == NULL) {
		input = encode->LockInput();
		Sleep(1);
	}
	if (input->img.i_csp == X264_CSP_I420)
	{
		libyuv::ARGBToI420(frame->data(), frame->stride(),
			input->img.plane[0], input->img.i_stride[0],
			input->img.plane[1], input->img.i_stride[1],
			input->img.plane[2], input->img.i_stride[2],
			width, height);
	}

	encode->encode(input, timeGetTime());

	encode->UnlockInput(input);
}

void CGDICaptureDlg::OnCaptureCompleted(webrtc::DesktopFrame* frame)
{
	//OnCaptureCompletedAVI(frame);
	OnCaptureCompletedMP4(frame);
}

void CGDICaptureDlg::onPacket(uint8_t * packet, int len, int keyFrame, int64_t timestamp)
{
	if (keyFrame == PICTURE_TYPE_N) return;
	
	int i = 0;
	int size = len;

	if (first_encode) {
		size += encode->seiLen();
	}

	av_new_packet(pkt, size);

	uint8_t * data = pkt->data;

	if (first_encode && encode->seiLen() > 0)
	{
		memcpy(&data[i], encode->getSEI(), encode->seiLen());
		i += encode->seiLen();
		first_encode = false;
	}
	
	memcpy(&data[i], packet, len);

	pkt->pts = pkt->dts = (timestamp - start_time)*10;
	printf("%lld %lld %d %d\n", timeGetTime() - timestamp,pkt->pts,pkt->size, keyFrame);

	index_frame++;
	if (keyFrame == PICTURE_TYPE_I)
		pkt->flags = AV_PKT_FLAG_KEY;
	else
		pkt->flags = 0;

	int ret = mp4->WriteVideo(pkt);
	assert(ret == 0);
	av_packet_unref(pkt);
}

void CGDICaptureDlg::OnTimer(UINT_PTR event)
{
	if (event == WM_TIME_CAPTURE) {
		webrtc::DesktopRegion region;
		webrtc::DesktopRect rt = DesktopRect::MakeWH(1920, 1080);
		region.AddRect(rt);
		capturer->Capture(region);
	}
}

void CGDICaptureDlg::CheckCache(int size) {
	if (cache_e == NULL || size > cache_e_max_len) {
		if (cache_e != NULL) free(cache_e);
		cache_e_max_len = size;
		cache_e = (uint8_t*)malloc(cache_e_max_len);
	}
}