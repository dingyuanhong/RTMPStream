//Download By Http://www.newxing.com
#include "stdafx.h"
#include "AviFile.h"

CAviFile avi("Output.Avi", mmioFOURCC('M','S','V','C'), 5); 
  
HBITMAP Screen();
void main()
{
	for (int i=0;i<10;i++) //��ʾ¼��10֡
	{
		avi.AppendNewFrame(Screen()); //��׽��ǰ��Ļ������AVI�ļ��С�
		Sleep(500);
	}

}

//ץȡ��ǰ��Ļ����
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
