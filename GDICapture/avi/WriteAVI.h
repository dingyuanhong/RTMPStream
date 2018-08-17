#include <vfw.h>
 
#define TEXT_HEIGHT	20
#define AVIIF_KEYFRAME	0x00000010L // this frame is a key frame.
#define BUFSIZE 260

class CAVIFile : public CObject
{
public:
	CAVIFile(LPCTSTR lpszFileName, int xdim = -1, int ydim = -1);
	virtual ~CAVIFile();

	virtual bool AddFrame(CBitmap& bmp);
	CString GetFName() const             {return FName;};
	virtual bool IsOK() const            {return bOK;};

 
private:
	CString FName;
	int xDim;
	int yDim;

	AVISTREAMINFO strhdr;
	PAVIFILE pfile;
	PAVISTREAM ps;
	PAVISTREAM psCompressed;
	PAVISTREAM psText;
	AVICOMPRESSOPTIONS opts;
	AVICOMPRESSOPTIONS FAR * aopts[1];
	DWORD dwTextFormat;
	char szText[BUFSIZE];
	int nFrames;
	bool bOK;

	//add by hw
	void* pBmpFileData;
	


	//video info
	PAVISTREAM pVideoStream;
	AVISTREAMINFO VideoStreamInfo;
	PAVIFILE pAviFile;
	int nVideoFrameCount;

	//audio info
	PAVISTREAM* pAudioStream;
	AVISTREAMINFO* AudioStreamInfo;
	int nAudioFrameCount;
	void* pWaveFileData;
	BYTE* pAudioData;
	DWORD nAudioDataLeft;

	//
	int nAFrames;
};

