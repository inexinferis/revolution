#include "screenshots.h"
#include "autoroute.h"
#include "players.h"
#include "opengl.h"
#include "aimbot.h"
#include "client.h"
#include "utils.h"
#include "cvar.h"

tglEnable pglEnable=NULL;
tglDisable pglDisable=NULL;
tglColor4f pglColor4f=NULL;
tglGetFloatv pglGetFloatv=NULL;
tglGetIntegerv pglGetIntegerv=NULL;
tglClearColor pglClearColor=NULL;
tglTexEnvi pglTexEnvi=NULL;
tglBlendFunc pglBlendFunc=NULL;
tglDepthRange pglDepthRange=NULL;
tglDepthFunc pglDepthFunc=NULL;
tglClear pglClear=NULL;
//tglVertex2f pglVertex2f=NULL;
tglVertex3f pglVertex3f=NULL;
tglVertex3fv pglVertex3fv=NULL;
tglBegin pglBegin=NULL;
tglEnd pglEnd=NULL;
tglFrustum pglFrustum=NULL;
tglReadBuffer pglReadBuffer=NULL;
tglReadPixels pglReadPixels=NULL;
tglPolygonMode pglPolygonMode=NULL;
tglLineWidth pglLineWidth=NULL;
tglColor3f pglColor3f=NULL;
tglViewport pglViewport=NULL;
tglPopMatrix pglPopMatrix=NULL;
tglGenTextures pglGenTextures=NULL;
tglBindTexture pglBindTexture=NULL;
tglTexImage2D pglTexImage2D=NULL;
tglTexParameteri pglTexParameteri=NULL;
tglTexCoord2f pglTexCoord2f=NULL;

//New OpenGL Functions
twglGetProcAddress pwglGetProcAddress=NULL;
tglBindBuffer pglBindBuffer=NULL;
tglMapBuffer pglMapBuffer=NULL;
tglUnmapBuffer pglUnmapBuffer=NULL;
tglGenBuffers pglGenBuffers=NULL;
tglBufferData pglBufferData=NULL;

HMODULE hOpengl32=NULL;

bool bSkyTex=false,bSmoke=false,bTex=false,oglSubtractive=false,oglChams=false;//,bFlash=false;
float oglChamsColor[3];

GLuint textureID=0;

PBYTE dwFSBase=0,dwFSBase2=0;tSetCltProcTable pSetCltProcTable=NULL;
VOID WINAPI DoTIBHook(PBYTE dwFSBase,PBYTE dwFSBase2);

BOOL WINAPI CheckFuncs(){
  static BOOL bCheckOK=FALSE;
  if(!bCheckOK)
    bCheckOK=(pglEnable&&pglDisable&&pglColor4f&&pglGetFloatv&&pglGetIntegerv&&pglClearColor&&pglTexEnvi&&pglBlendFunc&&pglDepthRange&&pglDepthFunc&&pglClear&&
      pglVertex3f&&pglVertex3fv&&pglBegin&&pglEnd&&pglFrustum&&pglReadBuffer&&pglReadPixels&&pglPolygonMode&&pglLineWidth&&pglColor3f);
  return bCheckOK;
}

void WINAPI hglBegin(GLenum mode){
  GLfloat curcolor[4];
  if(cvar.active&&!cvar.takingss&&(!cvar.antiss||!gMe.IsFreeSpectator())&&CheckFuncs()){
    if(cvar.wallhack==2){
      if(mode==GL_POLYGON){
        pglDisable(GL_DEPTH_TEST);
        pglEnable(GL_BLEND);
        pglBlendFunc(GL_ONE_MINUS_SRC_ALPHA,GL_SRC_ALPHA_SATURATE);
        pglGetFloatv(GL_CURRENT_COLOR,curcolor);
        pglColor4f(curcolor[0],curcolor[1],curcolor[2],0.667f);
      }else
        if(mode==GL_TRIANGLES||mode==GL_TRIANGLE_STRIP||mode==GL_TRIANGLE_FAN){
          pglEnable(GL_DEPTH_TEST);
          pglDisable(GL_BLEND);
        }
    }
    if(mode!=GL_TRIANGLES&&mode!=GL_TRIANGLE_STRIP&&mode!=GL_TRIANGLE_FAN&&mode!=GL_QUADS){
      if(cvar.nightmode&&!gMe.zoom)
        pglTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_BLEND);
      if(cvar.whitewalls&&!gMe.zoom)
        pglTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
      if(cvar.fullbright&&bTex&&!gMe.zoom)
        pglDisable(GL_TEXTURE_2D);
    }else{
      if(cvar.nigger)
        pglTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_BLEND);
      if(cvar.lambert&&!cvar.glowshells&&mode==GL_TRIANGLE_STRIP)
        pglTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
    }
    switch((int)cvar.wireframe){
      case 1:
        if(mode==GL_POLYGON){
          pglPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
          pglLineWidth(1.0f);
          pglColor3f(1.0f,1.0f,1.0f);
        }else
          pglPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      break;
      case 2:
        if(mode==GL_POLYGON){
          pglPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
          pglLineWidth(2.0f);
          pglColor3f(1.0f,1.0f,1.0f);
        }else
          pglPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
			case 3:
        if(mode==GL_POLYGON){
          pglPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
          pglLineWidth(4.0f);
          pglColor3f(1.0f,1.0f,1.0f);
        }else
          pglPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      break;
      default:break;
    }
    switch((int)cvar.wiremodels){
      case 1:
    		if(mode==GL_TRIANGLE_STRIP||mode==GL_TRIANGLE_FAN){
          pglPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
          pglLineWidth(1.0f);
        }else if(mode!=GL_POLYGON)
          pglPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      break;
      case 2:
        if(mode==GL_TRIANGLE_STRIP||mode==GL_TRIANGLE_FAN){
          pglPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
          pglLineWidth(3.0f);
        }else if(mode!=GL_POLYGON)
          pglPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      break;
			case 3:
        if(mode==GL_TRIANGLE_STRIP||mode==GL_TRIANGLE_FAN){
          pglPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
          pglLineWidth(5.0f);
        }else if(mode!=GL_POLYGON)
          pglPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      break;
      default:break;
    }
    if(mode==GL_QUADS){
      pglGetFloatv(GL_CURRENT_COLOR,curcolor);
      //fix laserbeam with no sky
      bSkyTex=!((curcolor[0]>0.0&&curcolor[1]==0.0&&curcolor[2]==0.0)||
                (curcolor[0]==0.0&&curcolor[1]>0.0&&curcolor[2]==0.0)||
                (curcolor[0]==0.0&&curcolor[1]==0.0&&curcolor[2]>0.0));
      //fix no smoke engine!
      bSmoke=(curcolor[0]==curcolor[1]&&curcolor[0]==curcolor[2]&&curcolor[0]!=0.0&&curcolor[0]!=1.0);
      //bFlash=(curcolor[0]>=1.0&&curcolor[1]>=1.0&&curcolor[2]>=1.0&&curcolor[3]>0.2);
    }else{
      bSkyTex=false;
      bSmoke=false;
    }
  }
  pglBegin(mode);
  //Update TIB Hook
  DoTIBHook(dwFSBase,dwFSBase2);
}

void WINAPI hglClear(GLbitfield mask){
  //don't clear cam view!
  if(cvar.active&&!cvar.takingss&&(!cvar.antiss||!gMe.IsFreeSpectator())&&
    (cvar.nosky||cvar.wallhack==2||cvar.wireframe)&&!drawingCam){
		if(mask&GL_DEPTH_BUFFER_BIT){
		  if(cvar.wireframe){
        if(pglClearColor)
          pglClearColor(0.1f,0.1f,0.1f,0.1f);
      }else{
        if(pglClearColor)
          pglClearColor(0.0f,0.0f,0.0f,0.0f);
      }
      mask|=GL_COLOR_BUFFER_BIT;
		}
	}
	pglClear(mask);
	//Update TIB Hook
  DoTIBHook(dwFSBase,dwFSBase2);
}

void WINAPI hglVertex3fv(const GLfloat *v){
  if(cvar.active&&!cvar.takingss&&(!cvar.antiss||!gMe.IsFreeSpectator())&&
     ((cvar.nosmoke&&bSmoke)||(bSkyTex&&(cvar.nosky||cvar.wallhack==2))))
    return;
  pglVertex3fv(v);
  //Update TIB Hook
  DoTIBHook(dwFSBase,dwFSBase2);
}

/*void WINAPI hglVertex2f(GLfloat x,GLfloat y){
  if(cvar.active&&!cvar.takingss&&cvar.noflash&&bFlash&&x==0.0&&y==0.0)
    pglColor4f(1.0f,1.0f,1.0f,0.1f);
  pglVertex2f(x,y);
  //Update TIB Hook
  DoTIBHook(dwFSBase,dwFSBase2);
}*/

void WINAPI hglEnable(GLenum mode){
  if(mode==GL_TEXTURE_2D)bTex=true;
  pglEnable(mode);
  //Update TIB Hook
  DoTIBHook(dwFSBase,dwFSBase2);
}

void WINAPI hglDisable(GLenum mode){
  if(mode==GL_TEXTURE_2D)bTex=false;
  pglDisable(mode);
  //Update TIB Hook
  DoTIBHook(dwFSBase,dwFSBase2);
}

//Cam zoom
void WINAPI hglFrustum(GLdouble left,GLdouble right,GLdouble bottom,GLdouble top,GLdouble zNear,GLdouble zFar){
	if(cvar.active&&!cvar.takingss&&(!cvar.antiss||!gMe.IsFreeSpectator())&&
    drawingCam==1&&cvar.spycam==1){
		top=zNear*tan((50-cvar.camzoom*5)*M_PI/360);
		bottom=-top;
		left=bottom*4/3;
		right=top*4/3;
	}
  pglFrustum(left,right,bottom,top,zNear,zFar);
  //Update TIB Hook
  DoTIBHook(dwFSBase,dwFSBase2);
}

//1.5 tint func!
void WINAPI hglBlendFunc(GLenum sfactor,GLenum dfactor){
  if(cvar.active&&!cvar.takingss&&(!cvar.antiss||!gMe.IsFreeSpectator())&&
    cvar.hudhealth&&dfactor==GL_ONE&&gMe.iHealth){
    GLfloat iRed = 1.0f;
    GLfloat iGreen = 1.0f;
    GLfloat iBlue = 1.0f;
    if(gMe.iHealth >= 60){
      iRed = 0.0f;
      iGreen = 1.0f;
      iBlue = 0.0f;
    }else if((gMe.iHealth >= 20) && (gMe.iHealth <= 59)){
      iRed = 1.0f;
      iGreen = 1.0f;
      iBlue = 0.2f;
    }else if(gMe.iHealth <= 19){
      iRed = 1.0f;
      iGreen = 0.0f;
      iBlue = 0.0f;
    }
    pglColor3f(iRed,iGreen,iBlue);
  }
	if(oglSubtractive)
    pglBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	else
    pglBlendFunc(sfactor,dfactor);
  //Update TIB Hook
  DoTIBHook(dwFSBase,dwFSBase2);
}

void WINAPI hglColor4f(GLfloat red,GLfloat green,GLfloat blue,GLfloat alpha){
  if(cvar.active&&!cvar.takingss&&(!cvar.antiss||!gMe.IsFreeSpectator())&&oglChams){
    red=oglChamsColor[0];
    green=oglChamsColor[1];
    blue=oglChamsColor[2];
    alpha=1.0;
  }
  pglColor4f(red,green,blue,alpha);
  //Update TIB Hook
  DoTIBHook(dwFSBase,dwFSBase2);
}

//SmallView
void WINAPI hglViewport(GLint x,GLint y,GLsizei width,GLsizei height){
	bool bSmall=(cvar.active&&!cvar.takingss&&cvar.smallview&&gMe.alive&&
    cvar.speedmode&&gMe.inspeed&&gAutoRoute.runninginroute&&!drawingCam);
	pglViewport(
		(bSmall?(width-200)/2:x),
		(bSmall?(height-150)/2:y),
		(bSmall?200:width),
		(bSmall?150:height)
	);
	//Update TIB Hook
  DoTIBHook(dwFSBase,dwFSBase2);
}

//UCP/SXE Screenshot
void WINAPI hglReadPixels(GLint x,GLint	y,GLsizei	width,GLsizei	height,GLenum format,GLenum type,GLvoid *data){
  if(cvar.active&&cvar.antiss)
    gScreenShots.CopyScreenShot(width,height,format,(PBYTE)data);
  else
    pglReadPixels(x,y,width,height,format,type,data);
  //Update TIB Hook
  DoTIBHook(dwFSBase,dwFSBase2);
}

void WINAPI glCreateTextureBuffer(GLint width,GLint height,GLvoid* data){
  static BOOL bInit=FALSE;
  if(!bInit){
    pglGenTextures(1,&textureID);
    bInit=TRUE;
  }
  if(pglBindTexture)
    pglBindTexture(GL_TEXTURE_2D,textureID);
  if(pglTexImage2D)
    pglTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_BGRA,GL_UNSIGNED_BYTE,data);
  if(pglTexParameteri){
    pglTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    pglTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  }
}

void WINAPI glDrawTexture(GLint x,GLint y,GLint width,GLint height){
  if(pglEnable)
    pglEnable(GL_TEXTURE_2D);
  if(pglBindTexture)
    pglBindTexture(GL_TEXTURE_2D,textureID);
  if(pglColor4f)
    pglColor4f(1.0f,1.0f,1.0f,0.1f);
  if(pglBegin)
    pglBegin(GL_QUADS);
  if(pglTexCoord2f&&pglVertex3f){
    pglTexCoord2f(1.0f,1.0f);
    pglVertex3f(x+width,y+height,1.0f);
    pglTexCoord2f(0.0f,1.0f);
    pglVertex3f(x,y+height,1.0f);
    pglTexCoord2f(0.0f,0.0f);
    pglVertex3f(x,y,1.0f);
    pglTexCoord2f(1.0f,0.0f);
    pglVertex3f(x+width,y,1.0f);
  }
  if(pglEnd)
    pglEnd();
  if(pglDisable)
    pglDisable(GL_TEXTURE_2D);
}

/*******************************************************************************\
 TIB HOOKING V3.0
\*******************************************************************************/

//do TIB hooking
VOID WINAPI DoTIBHook(PBYTE dwFSBase,PBYTE dwFSBase2){
  //Update redirect call
  if(pglEnd!=(tglEnd)*(PDWORD)(dwFSBase+0x858)){
    pglEnd=(tglEnd)*(PDWORD)(dwFSBase+0x858);
  }
  if(pglColor3f!=(tglColor3f)*(PDWORD)(dwFSBase+0x7E0)){
    pglColor3f=(tglColor3f)*(PDWORD)(dwFSBase+0x7E0);
  }
  if(pglVertex3f!=(tglVertex3f)*(PDWORD)(dwFSBase+0x94C)){
    pglVertex3f=(tglVertex3f)*(PDWORD)(dwFSBase+0x94C);
  }
  if(pglTexCoord2f!=(tglTexCoord2f)*(PDWORD)(dwFSBase+0x8CC)){
    pglTexCoord2f=(tglTexCoord2f)*(PDWORD)(dwFSBase+0x8CC);
  }
  if(pglBindTexture!=(tglBindTexture)*(PDWORD)(dwFSBase+0xA08)){
    pglBindTexture=(tglBindTexture)*(PDWORD)(dwFSBase+0xA08);
  }

  if(dwFSBase2){
    if(pglGetFloatv!=(tglGetFloatv)*(PDWORD)(dwFSBase2+0x418)){
      pglGetFloatv=(tglGetFloatv)*(PDWORD)(dwFSBase2+0x418);
    }
    if(pglGetIntegerv!=(tglGetIntegerv)*(PDWORD)(dwFSBase2+0x41c)){
      pglGetIntegerv=(tglGetIntegerv)*(PDWORD)(dwFSBase2+0x41c);
    }
    if(pglClearColor!=(tglClearColor)*(PDWORD)(dwFSBase2+0x338)){
      pglClearColor=(tglClearColor)*(PDWORD)(dwFSBase2+0x338);
    }
    if(pglTexEnvi!=(tglTexEnvi)*(PDWORD)(dwFSBase2+0x2E8)){
      pglTexEnvi=(tglTexEnvi)*(PDWORD)(dwFSBase2+0x2E8);
    }
    if(pglDepthRange!=(tglDepthRange)*(PDWORD)(dwFSBase2+0x480)){
      pglDepthRange=(tglDepthRange)*(PDWORD)(dwFSBase2+0x480);
    }
    if(pglDepthFunc!=(tglDepthFunc)*(PDWORD)(dwFSBase2+0x3D4)){
      pglDepthFunc=(tglDepthFunc)*(PDWORD)(dwFSBase2+0x3D4);
    }
    if(pglReadBuffer!=(tglReadBuffer)*(PDWORD)(dwFSBase2+0x3F8)){
      pglReadBuffer=(tglReadBuffer)*(PDWORD)(dwFSBase2+0x3F8);
    }
    if(pglLineWidth!=(tglLineWidth)*(PDWORD)(dwFSBase2+0x2A0)){
      pglLineWidth=(tglLineWidth)*(PDWORD)(dwFSBase2+0x2A0);
    }
    if(pglPolygonMode!=(tglPolygonMode)*(PDWORD)(dwFSBase2+0x2B8)){
      pglPolygonMode=(tglPolygonMode)*(PDWORD)(dwFSBase2+0x2B8);
    }
    if(pglGenTextures!=(tglGenTextures)*(PDWORD)(dwFSBase2+0x520)){
      pglGenTextures=(tglGenTextures)*(PDWORD)(dwFSBase2+0x520);
    }
    if(pglTexImage2D!=(tglTexImage2D)*(PDWORD)(dwFSBase2+0x2DC)){
      pglTexImage2D=(tglTexImage2D)*(PDWORD)(dwFSBase2+0x2DC);
    }
    if(pglTexParameteri!=(tglTexParameteri)*(PDWORD)(dwFSBase2+0x2D0)){
      pglTexParameteri=(tglTexParameteri)*(PDWORD)(dwFSBase2+0x2D0);
    }
  }

  //Update TIB Hook
  if(*(PDWORD)(dwFSBase+0x7CC)!=(DWORD)hglBegin){
    pglBegin=(tglBegin)*(PDWORD)(dwFSBase+0x7CC);
    *(PDWORD)(dwFSBase+0x7CC)=(DWORD)hglBegin;
  }
  if(*(PDWORD)(dwFSBase+0x998)!=(DWORD)hglEnable){
    pglEnable=(tglEnable)*(PDWORD)(dwFSBase+0x998);
    *(PDWORD)(dwFSBase+0x998)=(DWORD)hglEnable;
  }
  if(*(PDWORD)(dwFSBase+0x994)!=(DWORD)hglDisable){
    pglDisable=(tglDisable)*(PDWORD)(dwFSBase+0x994);
    *(PDWORD)(dwFSBase+0x994)=(DWORD)hglDisable;
  }
  /*if(*(PDWORD)(dwFSBase+0x92C)!=(DWORD)hglVertex2f){
    pglVertex2f=(tglVertex2f)*(PDWORD)(dwFSBase+0x92C);
    *(PDWORD)(dwFSBase+0x92C)=(DWORD)hglVertex2f;
  }*/
  if(*(PDWORD)(dwFSBase+0x950)!=(DWORD)hglVertex3fv){
    pglVertex3fv=(tglVertex3fv)*(PDWORD)(dwFSBase+0x950);
    *(PDWORD)(dwFSBase+0x950)=(DWORD)hglVertex3fv;
  }
  if(*(PDWORD)(dwFSBase+0x820)!=(DWORD)hglColor4f){
    pglColor4f=(tglColor4f)*(PDWORD)(dwFSBase+0x820);
    *(PDWORD)(dwFSBase+0x820)=(DWORD)hglColor4f;
  }

  if(dwFSBase2){
    if(*(PDWORD)(dwFSBase2+0x484)!=(DWORD)hglFrustum){
      pglFrustum=(tglFrustum)*(PDWORD)(dwFSBase2+0x484);
      *(PDWORD)(dwFSBase2+0x484)=(DWORD)hglFrustum;
    }
    if(*(PDWORD)(dwFSBase2+0x32C)!=(DWORD)hglClear){
      pglClear=(tglClear)*(PDWORD)(dwFSBase2+0x32C);
      *(PDWORD)(dwFSBase2+0x32C)=(DWORD)hglClear;
    }
    if(*(PDWORD)(dwFSBase2+0x3C4)!=(DWORD)hglBlendFunc){
      pglBlendFunc=(tglBlendFunc)*(PDWORD)(dwFSBase2+0x3C4);
      *(PDWORD)(dwFSBase2+0x3C4)=(DWORD)hglBlendFunc;
    }
    if(*(PDWORD)(dwFSBase2+0x400)!=(DWORD)hglReadPixels){
      pglReadPixels=(tglReadPixels)*(PDWORD)(dwFSBase2+0x400);
      *(PDWORD)(dwFSBase2+0x400)=(DWORD)hglReadPixels;
    }
    if(*(PDWORD)(dwFSBase2+0x4C4)!=(DWORD)hglViewport){
      pglViewport=(tglViewport)*(PDWORD)(dwFSBase2+0x4C4);
      *(PDWORD)(dwFSBase2+0x4C4)=(DWORD)hglViewport;
    }
  }

  //New OpenGL functions...
  if(!pwglGetProcAddress){
    pwglGetProcAddress=(twglGetProcAddress)GetProcAddress(hOpengl32,"wglGetProcAddress");
  }

  if(pwglGetProcAddress){
    if(!pglGenBuffers){
      pglGenBuffers=(tglGenBuffers)pwglGetProcAddress("glGenBuffers");
    }
    if(!pglBufferData){
      pglBufferData=(tglBufferData)pwglGetProcAddress("glBufferData");
    }
    if(!pglBindBuffer){
      pglBindBuffer=(tglBindBuffer)pwglGetProcAddress("glBindBuffer");
    }
    if(!pglMapBuffer){
      pglMapBuffer=(tglMapBuffer)pwglGetProcAddress("glMapBuffer");
    }
    if(!pglUnmapBuffer){
      pglUnmapBuffer=(tglUnmapBuffer)pwglGetProcAddress("glUnmapBuffer");
    }
  }
}

//OGL TIB HOOK METHOD...
int WINAPI hSetCltProcTable(PDWORD pDst1,PDWORD pDst2,DWORD iSize){
  DWORD defRet=pSetCltProcTable(pDst1,pDst2,iSize);
  //Update TIB Hook
  DoTIBHook(dwFSBase,dwFSBase2);
  return defRet;
}

extern "C" {
  VOID WINAPI DoThisTIBHook(){
    //Update TIB Hook
    DoTIBHook(dwFSBase,dwFSBase2);
  }

  PVOID pCaller=NULL;
  VOID WINAPI hThisSetCltProcTable();
  asm(
    ".text;\r\n"
    ".globl _hThisSetCltProcTable\r\n"
    "_hThisSetCltProcTable:\r\n"
    " pop _pCaller;\r\t"
    " call *_pSetCltProcTable;\r\t"
    " push %eax;\r\t"
    " call _DoThisTIBHook;\r\t"
    " pop %eax;\r\t"
    " push _pCaller;\r\t"
    " ret"
  );
}

BOOL WINAPI GetOffsets(HMODULE hOpengl32,PDWORD pdwTlsOffset,PDWORD pdwSetCltProcTable,PDWORD pdwTlsIndex,PBOOL bThisCall){
  switch(GetWinVr()){
    case WINXP:
      *pdwSetCltProcTable=0x18FA1;//SetCltProcTable
      *pdwTlsOffset=0xAC80C;//_dwTlsOffset
      *pdwTlsIndex=0xA53CC;//_dwTlsIndex
      *bThisCall=FALSE;
    break;
    case WIN2K3:
      *pdwSetCltProcTable=0x18F9B;//SetCltProcTable
      *pdwTlsOffset=0xADB2C;//_dwTlsOffset
      *pdwTlsIndex=0xA63BC;//_dwTlsIndex
      *bThisCall=FALSE;
    break;
    case WINVISTA:
      *pdwSetCltProcTable=0x19C7F;//SetCltProcTable
      *pdwTlsOffset=0xAB958;//_dwTlsOffset
      *pdwTlsIndex=0xA43FC;//_dwTlsIndex
      *bThisCall=FALSE;
    break;
    case WIN7:
      *pdwSetCltProcTable=0x389EE;//SetCltProcTable
      *pdwTlsOffset=0xA100C;//_dwTlsOffset
      *pdwTlsIndex=0xA102C;//_dwTlsIndex
      *bThisCall=FALSE;
    break;
    case WIN8:
      *pdwSetCltProcTable=0x3921E;//SetCltProcTable
      *pdwTlsOffset=0xA500C;//_dwTlsOffset
      *pdwTlsIndex=0xA502C;//_dwTlsIndex
      *bThisCall=TRUE;
    break;
    case WIN81:
      *pdwSetCltProcTable=0x18DEF;//SetCltProcTable
      *pdwTlsOffset=0xC6850;//_dwTlsOffset
      *pdwTlsIndex=0xAC004;//_dwTlsIndex
      *bThisCall=TRUE;
    break;
    case WIN10:
      *pdwSetCltProcTable=0x1BE89;//SetCltProcTable
      *pdwTlsOffset=0xD2870;//_dwTlsOffset
      *pdwTlsIndex=0xB8004;//_dwTlsIndex
      *bThisCall=TRUE;
    break;
    case WIN10TH2:
      *pdwSetCltProcTable=0x1BE89;//SetCltProcTable
      *pdwTlsOffset=0xD2890;//_dwTlsOffset
      *pdwTlsIndex=0xB8004;//_dwTlsIndex
      *bThisCall=TRUE;
    break;
    case WIN10AU:
      *pdwSetCltProcTable=0x1BEA9;//SetCltProcTable
      *pdwTlsOffset=0xD2890;//_dwTlsOffset
      *pdwTlsIndex=0xB9314;//_dwTlsIndex
      *bThisCall=TRUE;
    break;
    case WIN10CU:
      *pdwSetCltProcTable=0x1BCFF;//SetCltProcTable
      *pdwTlsOffset=0xD4854;//_dwTlsOffset
      *pdwTlsIndex=0xB8370;//_dwTlsIndex
      *bThisCall=TRUE;
    break;
    case WIN10FCU:
      *pdwSetCltProcTable=0x1BDBF;//SetCltProcTable
      *pdwTlsOffset=0xD4854;//_dwTlsOffset
      *pdwTlsIndex=0xB8370;//_dwTlsIndex
      *bThisCall=TRUE;
    break;
    default://Incompatible Windows Version...
    return FALSE;
  }
  return TRUE;
}

LPVOID WINAPI GetValueFromTeb(PTEB Teb,DWORD Index){
  if(Index<TLS_MINIMUM_AVAILABLE)
    return Teb->TlsSlots[Index];
  if(Index>=TLS_EXPANSION_SLOTS+TLS_MINIMUM_AVAILABLE||!Teb->TlsExpansionSlots)
    return NULL;
  return Teb->TlsExpansionSlots[Index-TLS_MINIMUM_AVAILABLE];
}

BOOL WINAPI HookOpenGl(){
  CONTEXT Context;LDT_ENTRY SelEntry;
  Context.ContextFlags=CONTEXT_FULL|CONTEXT_DEBUG_REGISTERS;
  DWORD pdwTlsOffset=0,pdwSetCltProcTable=0,pdwTlsIndex=0,dwTlsOffset=0,dwTlsIndex=0;BOOL bThisCall=FALSE;

  if((hOpengl32=GetModuleHandle("opengl32.dll"))==NULL){
    return FALSE;
  }

  if(!GetOffsets(hOpengl32,&pdwTlsOffset,&pdwSetCltProcTable,&pdwTlsIndex,&bThisCall))
    return FALSE;

  DWORD dwThreadId,dwProcessId;
  HWND hdHalfLife=FindWindow(NULL,"Counter-Strike");
  if(hdHalfLife){
    dwThreadId=GetWindowThreadProcessId(hdHalfLife,&dwProcessId);
  }

  HANDLE hThread=OpenThread(THREAD_GET_CONTEXT|THREAD_SUSPEND_RESUME|THREAD_QUERY_INFORMATION,FALSE,dwThreadId);
  if(!hThread){
    return FALSE;
  }

  if(!GetThreadContext(hThread,&Context)){
    CloseHandle(hThread);
    return FALSE;
  }

  GetThreadSelectorEntry(hThread,Context.SegFs,&SelEntry);
  dwFSBase=(PBYTE)((SelEntry.HighWord.Bits.BaseHi<<24)|
    (SelEntry.HighWord.Bits.BaseMid<<16)|SelEntry.BaseLow);
  if(!dwFSBase||IsBadReadPtr((PVOID)dwFSBase,4)){
    return FALSE;
  }

  dwTlsOffset=((DWORD)hOpengl32)+pdwTlsOffset;
  if(IsBadReadPtr((PVOID)dwTlsOffset,4)){
    return FALSE;
  }

  if(pdwTlsIndex){
    dwTlsIndex=((DWORD)hOpengl32)+pdwTlsIndex;
    if(IsBadReadPtr((PVOID)dwTlsIndex,4)){
      return FALSE;
    }
  }

  do{//multiple redirect...
    Sleep(100);
    if(dwTlsIndex&&(*(PDWORD)dwTlsIndex)>TLS_MINIMUM_AVAILABLE)
      dwFSBase2=(PBYTE)GetValueFromTeb((PTEB)dwFSBase,*((PDWORD)dwTlsIndex));
    else
      dwFSBase2=(PBYTE)(*(PDWORD)((DWORD)dwFSBase+(*(PDWORD)dwTlsOffset)));
  }while(dwFSBase2==NULL);

  //do Hook
  SuspendThread(hThread);
  DoTIBHook(dwFSBase,dwFSBase2);

  ResumeThread(hThread);
  CloseHandle(hThread);

  return TRUE;
}
