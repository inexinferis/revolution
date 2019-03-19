#include "screenshots.h"
#include "opengl.h"
#include "main.h"
#include "cvar.h"

cScreenShots gScreenShots;
BOOL bTakingSS=FALSE;

BOOL cScreenShots::InitScreenShot(DWORD dwWidth,DWORD dwHeight){
  //finish thread if still waiting
  sWidth=dwWidth;sHeight=dwHeight;
  //32 bits buffer
  bufSize=sWidth*32*sHeight/8+sizeof(BITMAPINFO)+4;
  pBuff32=(PSSMSG)LocalAlloc(LPTR,bufSize);
  if(pBuff32){
    //create buffer for internal operations
    pFlippedBuffer=(PBYTE)LocalAlloc(LPTR,sWidth*32*sHeight/8);
    //Initialize Buffer!!!
    pBuff32->uBuffSize=bufSize;
    ZeroMemory(&pBuff32->bmi,sizeof(pBuff32->bmi));
    pBuff32->bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    pBuff32->bmi.bmiHeader.biWidth=sWidth;
    pBuff32->bmi.bmiHeader.biHeight=sHeight;
    pBuff32->bmi.bmiHeader.biBitCount=32;
    pBuff32->bmi.bmiHeader.biPlanes=1;
    pBuff32->bmi.bmiHeader.biCompression=BI_RGB;
    pBuff32->bmi.bmiHeader.biSizeImage=sWidth*32*sHeight/8;
    hssee=CreateEvent(NULL,FALSE,FALSE,"EXITSCREENSHOTTHREAD");
    hssth=CreateThread(NULL,0,ScreenShotsTH,(PVOID)this,0,&ssthid);
    hWnd=FindWindow(NULL,"Counter-Strike");
    return TRUE;
  }
  return FALSE;
}

DWORD WINAPI cScreenShots::ScreenShotsTH(LPVOID arg){
  cScreenShots *pScreenShots=(cScreenShots *)arg;
  DWORD dwRes=WAIT_TIMEOUT;
  do{
    switch(dwRes){
      case WAIT_OBJECT_0:
        return 0;
      case WAIT_TIMEOUT:
        if(pScreenShots->bSkipDeath)
          pScreenShots->bSkipDeath=FALSE;
        else
          if(cvar.active&&cvar.antiss)
            cvar.takingss=TRUE;
        if(pScreenShots->bScreenTook){
          if(++pScreenShots->uiTakesCount>cvar.antissshowtime){
            pScreenShots->bScreenTook=FALSE;
          }
        }else{
          pScreenShots->uiTakesCount=0;
        }
      break;
      default:break;
    }
    dwRes=WaitForSingleObject(pScreenShots->hssee,(INT)(cvar.antissint+1)*1000);
  }while(1);
}

cScreenShots::~cScreenShots(){
  if(hssee){
    SetEvent(hssee);
    CloseHandle(hssee);
  }
  if(hssth){
    TerminateThread(hssth,0);
    CloseHandle(hssth);
  }
  if(hFile)
    CloseHandle(hFile);
  if(pBuff32)
    LocalFree(pBuff32);
  if(pFlippedBuffer)
    LocalFree(pFlippedBuffer);
}

VOID BGRAtoFormat(PBYTE in,PBYTE out,UINT format,UINT width,UINT height){
  for(UINT i=0;i<(width*height);i++){
    switch(format){
      case GL_RGB:{
        PBYTE pixel_in=&in[i*4];
        PBYTE pixel_out=&out[i*3];
        pixel_out[0]=pixel_in[2];
        pixel_out[1]=pixel_in[1];
        pixel_out[2]=pixel_in[0];
      }break;
      case GL_BGR:{
        PBYTE pixel_in=&in[i*4];
        PBYTE pixel_out=&out[i*3];
        pixel_out[0]=pixel_in[0];
        pixel_out[1]=pixel_in[1];
        pixel_out[2]=pixel_in[2];
      }break;
      case GL_RGBA:{
        PBYTE pixel_in=&in[i*4];
        PBYTE pixel_out=&out[i*4];
        pixel_out[0]=pixel_in[2];
        pixel_out[1]=pixel_in[1];
        pixel_out[2]=pixel_in[0];
        pixel_out[3]=pixel_in[3];
      }break;
      default:{//BGRA -> BGRA
        PBYTE pixel_in=&in[i*4];
        PBYTE pixel_out=&out[i*4];
        pixel_out[0]=pixel_in[0];
        pixel_out[1]=pixel_in[1];
        pixel_out[2]=pixel_in[2];
        pixel_out[3]=pixel_in[3];
      }break;
    }
  }
}

VOID BGRAtoFlippedFormat(PBYTE in,PBYTE out,UINT format,UINT width,UINT height){
  for(UINT y=0;y<height;y++){
    for(UINT x=0;x<width;x++){
      switch(format){
        case GL_RGB:{
          PBYTE pixel_in=&in[(y*width+x)*4];
          PBYTE pixel_out=&out[((height-y-1)*width+x)*3];
          pixel_out[0]=pixel_in[2];
          pixel_out[1]=pixel_in[1];
          pixel_out[2]=pixel_in[0];
        }break;
        case GL_BGR:{
          PBYTE pixel_in=&in[(y*width+x)*4];
          PBYTE pixel_out=&out[((height-y-1)*width+x)*3];
          pixel_out[0]=pixel_in[0];
          pixel_out[1]=pixel_in[1];
          pixel_out[2]=pixel_in[2];
        }break;
        case GL_RGBA:{
          PBYTE pixel_in=&in[(y*width+x)*4];
          PBYTE pixel_out=&out[((height-y-1)*width+x)*4];
          pixel_out[0]=pixel_in[2];
          pixel_out[1]=pixel_in[1];
          pixel_out[2]=pixel_in[0];
          pixel_out[3]=pixel_in[3];
        }break;
        default:{//GL_BGRA -> Flipped BGRA
          PBYTE pixel_in=&in[(y*width+x)*4];
          PBYTE pixel_out=&out[((height-y-1)*width+x)*4];
          pixel_out[0]=pixel_in[0];
          pixel_out[1]=pixel_in[1];
          pixel_out[2]=pixel_in[2];
          pixel_out[3]=pixel_in[3];
        }break;
      }
    }
  }
}

VOID cScreenShots::CopyScreenShot(UINT width,UINT height,UINT format,PBYTE data){
  if(pBuff32){
    if(data){
      //Convert to RGB to Replace!
      BGRAtoFormat(pBuff32->pBuffer,data,format,width,height);
    }else if(pglBindBuffer&&pglMapBuffer&&pglUnmapBuffer){
      //OpenGL buffer copy... http://lektiondestages.blogspot.com.ar/2013/01/reading-opengl-backbuffer-to-system.html
      GLvoid *buffer=pglMapBuffer(GL_PIXEL_PACK_BUFFER,GL_WRITE_ONLY);
      if(buffer){
        BGRAtoFlippedFormat(pBuff32->pBuffer,(PBYTE)buffer,format,width,height);
        pglUnmapBuffer(GL_PIXEL_PACK_BUFFER);
      }
    }
    //save screenshot to show...
    if(cvar.antissshow){
      BGRAtoFlippedFormat(pBuff32->pBuffer,pFlippedBuffer,GL_BGRA,width,height);
      glCreateTextureBuffer(width,height,pFlippedBuffer);
      uiTakesCount=0;bScreenTook=TRUE;
    }
    SaveBitmap(pBuff32);
  }
}

VOID cScreenShots::TakeScreenShot(){
  static GLuint pbo=0;
  if(hFile&&pglReadBuffer&&pglReadPixels){
    if(bTakingSS){
      bTakingSS=FALSE;
      SaveBitmap(pBuff32);
    }

    if(pglGenBuffers&&pglBindBuffer&&pglBufferData&&!pbo){
      pglGenBuffers(1,&pbo);
      pglBindBuffer(GL_PIXEL_PACK_BUFFER,pbo);
      pglBufferData(GL_PIXEL_PACK_BUFFER,sWidth*sHeight*4,NULL,GL_STREAM_READ);
      pglBindBuffer(GL_PIXEL_PACK_BUFFER,0);
    }

    if(pglBindBuffer&&pglMapBuffer&&pglUnmapBuffer&&pbo){
      pglBindBuffer(GL_PIXEL_PACK_BUFFER,pbo);
      pglReadBuffer(GL_FRONT);//GL_BACK);//<- problem with sprites!
      pglReadPixels(0,0,sWidth,sHeight,GL_BGRA,GL_UNSIGNED_BYTE,NULL);
      GLvoid *pBuffer=pglMapBuffer(GL_PIXEL_PACK_BUFFER,GL_READ_ONLY);
      if(pBuffer) {
        memcpy(pBuff32->pBuffer,pBuffer,sWidth*sHeight*4);//for GDI
        pglUnmapBuffer(GL_PIXEL_PACK_BUFFER);
      }
      pglBindBuffer(GL_PIXEL_PACK_BUFFER,0);
    }else{
      pglReadBuffer(GL_FRONT);//GL_BACK);//<- problem with sprites!
      pglReadPixels(0,0,sWidth,sHeight,GL_BGRA,GL_UNSIGNED_BYTE,pBuff32->pBuffer);//for GDI
    }
  }
}

VOID cScreenShots::TakeScreenShot2(){
  WINDOWPLACEMENT lpwndpl;
  RECT clientRect;INT iBorder=0;
  lpwndpl.length=sizeof(WINDOWPLACEMENT);
  //bitblt screenshot for driver...
  if(hFile&&hWnd){
    if(bTakingSS){
      bTakingSS=FALSE;
      SaveBitmap(pBuff32);
    }

    //get client rect
    GetWindowPlacement(hWnd,&lpwndpl);
    memcpy(&clientRect,&lpwndpl.rcNormalPosition,sizeof(RECT));
    iBorder=((clientRect.right-clientRect.left)-sWidth)/2;
    if(iBorder){//is full screen?
      clientRect.left+=iBorder;
      clientRect.right-=iBorder;
      clientRect.bottom-=iBorder;
      clientRect.top=clientRect.bottom-sHeight;
    }

    HDC hdc=GetDC(NULL);
    HDC memDC=CreateCompatibleDC(hdc);
    HBITMAP hbm=CreateCompatibleBitmap(hdc,sWidth,sHeight);
    HBITMAP hbmOld=(HBITMAP)SelectObject(memDC,hbm);
    BitBlt(memDC,0,0,sWidth,sHeight,hdc,clientRect.left,clientRect.top,SRCCOPY);
    GetDIBits(memDC,hbm,0,sHeight,pBuff32->pBuffer,&pBuff32->bmi,DIB_RGB_COLORS);
    SelectObject(memDC,hbmOld);
    DeleteObject(hbm);
    DeleteDC(memDC);
    ReleaseDC(hWnd,hdc);
  }
}

VOID cScreenShots::SaveBitmap(PSSMSG pBuff,BOOL bIncludeBmp){
  SYSTEMTIME sSystemTime;CHAR filename[MAX_PATH];
  if(!DirectoryExist(scrpath))
    CreateDirectory(scrpath,NULL);
  GetLocalTime(&sSystemTime);
  sprintf(filename,"%s\\%s_%02i_%02i_%04i__%02i_%02i_%02i.jpg",scrpath,"screen_",sSystemTime.wDay,sSystemTime.wMonth,sSystemTime.wYear,sSystemTime.wHour,sSystemTime.wMinute,sSystemTime.wSecond);
  if(!SaveJPG(pBuff,filename,75)||bIncludeBmp){
    sprintf(filename,"%s\\%s_%02i_%02i_%04i__%02i_%02i_%02i.bmp",scrpath,"screen_",sSystemTime.wDay,sSystemTime.wMonth,sSystemTime.wYear,sSystemTime.wHour,sSystemTime.wMinute,sSystemTime.wSecond);
    HANDLE hFile=CreateFile(filename,GENERIC_WRITE,0,NULL,
    CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if(hFile){
      BITMAPFILEHEADER bfh={0};DWORD dwWritten=0;
      bfh.bfType=0x4D42;
      bfh.bfOffBits=sizeof(BITMAPINFOHEADER)+sizeof(BITMAPFILEHEADER);
      bfh.bfSize=bfh.bfOffBits+pBuff->bmi.bmiHeader.biSizeImage;
      WriteFile(hFile,&bfh,sizeof(bfh),&dwWritten,NULL);
      WriteFile(hFile,&pBuff->bmi.bmiHeader, sizeof(pBuff->bmi.bmiHeader),&dwWritten,NULL);
      WriteFile(hFile,pBuff->pBuffer,pBuff->bmi.bmiHeader.biSizeImage,&dwWritten,NULL);
      CloseHandle(hFile);
    }
  }
}

BOOL cScreenShots::SaveJPG(PSSMSG pBuff,PCHAR filename,INT quality){
  BOOL bRet=FALSE;
  int width=pBuff->bmi.bmiHeader.biWidth;
  int height=pBuff->bmi.bmiHeader.biHeight;
  BGRAtoFlippedFormat(pBuff->pBuffer,pFlippedBuffer,GL_RGB,width,height);
  bRet=jpge::compress_image_to_jpeg_file(filename,width,height,3,pFlippedBuffer);
  return bRet;
}
