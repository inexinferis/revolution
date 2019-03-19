#ifndef _ANTISCREENSHOT_
#define _ANTISCREENSHOT_

#include "utils.h"
#include "jpeg/jpge.h"

//OpenGL Defs
#ifndef GL_RGB
#define GL_RGB      0x1907
#endif

#ifndef GL_RGBA
#define GL_RGBA     0x1908
#endif

#ifndef GL_BGR
#define GL_BGR      0x80E0
#endif

#ifndef GL_BGRA
#define GL_BGRA     0x80E1
#endif

#ifndef GL_PIXEL_PACK_BUFFER
#define GL_PIXEL_PACK_BUFFER              0x88EB
#endif

#ifndef GL_PIXEL_PACK_BUFFER_BINDING
#define GL_PIXEL_PACK_BUFFER_BINDING      0x88ED
#endif

#ifndef GL_READ_ONLY
#define GL_READ_ONLY 0x88B8
#endif

#ifndef GL_WRITE_ONLY
#define GL_WRITE_ONLY 0x88B9
#endif

#ifndef GL_STREAM_READ
#define GL_STREAM_READ 0x88E1
#endif

#pragma pack(push,1)

typedef struct PACKED _SSMSG{
  DWORD uBuffSize;
  BITMAPINFO bmi;
  BYTE  pBuffer[];
}SSMSG,*PSSMSG;

#pragma pack(pop)

class cScreenShots{
  public:
    cScreenShots(){
      hFile=hssth=hssee=NULL;
      pBuff32=NULL;
      pFlippedBuffer=NULL;
      bSkipDeath=FALSE;
      bScreenTook=FALSE;
      uiTakesCount=0;
    };
    ~cScreenShots();
    BOOL InitScreenShot(DWORD dwWidth,DWORD dwHeight);
    VOID TakeScreenShot();
    VOID TakeScreenShot2();
    VOID CopyScreenShot(UINT width,UINT height,UINT format,PBYTE data);
    BOOL bSkipDeath,bScreenTook;
    UINT uiTakesCount;
  private:
    VOID SaveBitmap(PSSMSG pBuff,BOOL bIncludeBmp=FALSE);
    BOOL SaveJPG(PSSMSG pBuff,PCHAR filename,INT quality);
    HANDLE hFile,hssth,hssee;
    DWORD ssthid,bufSize;
    UINT sWidth,sHeight;
    PSSMSG pBuff32;
    PBYTE pFlippedBuffer;
    HWND hWnd;
  static DWORD WINAPI ScreenShotsTH(LPVOID);
};

extern cScreenShots gScreenShots;

#endif //_ANTISCREENSHOT_
