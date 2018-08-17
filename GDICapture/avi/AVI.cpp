//Download By Http://www.newxing.com
#include "stdafx.h"
#include "AviFile.h"

CAviFile avi("Output.Avi", mmioFOURCC('M','S','V','C'), 5); 
  
HBITMAP Screen();
void main()
{
	for (int i=0;i<10;i++) //演示录制10帧
	{
		avi.AppendNewFrame(Screen()); //捕捉当前屏幕并插入AVI文件中。
		Sleep(500);
	}

}

//抓取当前屏幕函数
HBITMAP Screen(){
	HDC	hScreen	= CreateDC("DISPLAY", NULL, NULL, NULL);
	HDC		hCompDC	= CreateCompatibleDC(hScreen);
	int		nWidth	= GetSystemMetrics(SM_CXSCREEN);	
	int		nHeight	= GetSystemMetrics(SM_CYSCREEN);
	HBITMAP	hBmp	= CreateCompatibleBitmap(hScreen, nWidth, nHeight);
	HBITMAP	hOld	= (HBITMAP)SelectObject(hCompDC, hBmp);
	BitBlt(hCompDC, 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY);	
	SelectObject(hCompDC, hOld);
	DeleteDC(hScreen);	
	DeleteDC(hCompDC);
	return  hBmp;
}
