#include "weapons.h"
#include "screenshots.h"
#include "drawing.h"
#include "sprites.h"
#include "console.h"
#include "players.h"
#include "opengl.h"
#include "radar.h"
#include "menu.h"
#include "cvar.h"
#include "gui.h"

struct cvar_s *chase_active=NULL,*r_drawviewmodel=NULL,*cl_showfps=NULL,*hud_draw=NULL,*hud_saytext_time=NULL,*cl_timeout=NULL;

int displayCenterX,displayCenterY;
int fontWidth,fontHeight;
SCREENINFO screeninfo;

void InitVisuals(void){
	screeninfo.iSize=sizeof(SCREENINFO);
	gEngfuncs.pfnGetScreenInfo(&screeninfo);
	displayCenterX=screeninfo.iWidth/2;
	displayCenterY=screeninfo.iHeight/2;
	spritedata.iSpritesLoaded=0;int o=1;
	client_sprite_t *pList=gEngfuncs.pfnSPR_GetList("sprites/hud.txt",&o);
	if(pList)LoadSprites(pList,o);
	//string size (9 - 14) || (13 - 21)
	gEngfuncs.pfnDrawConsoleStringLen("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",&fontWidth,&fontHeight);
  fontWidth/=46;
	gGui.Init(screeninfo.iWidth,screeninfo.iHeight,fontWidth,fontHeight);
	gMenu.Init(screeninfo.iWidth,screeninfo.iHeight,fontWidth,fontHeight);
  gRadar.Init(screeninfo.iWidth,screeninfo.iHeight,fontWidth,fontHeight);
  gConsole.Init(screeninfo.iWidth,screeninfo.iHeight,fontWidth,fontHeight);
	while(!gScreenShots.InitScreenShot(screeninfo.iWidth,screeninfo.iHeight))
    Sleep(10);
  r_drawviewmodel=gEngfuncs.pfnGetCvarPointer("r_drawviewmodel");
  chase_active=gEngfuncs.pfnGetCvarPointer("chase_active");
  cl_showfps=gEngfuncs.pfnGetCvarPointer("cl_showfps");
  hud_draw=gEngfuncs.pfnGetCvarPointer("hud_draw");
  hud_saytext_time=gEngfuncs.pfnGetCvarPointer("hud_saytext_time");
  cl_timeout=gEngfuncs.pfnGetCvarPointer("cl_timeout");
}

float VectorLength(const float *v){
	return (float)sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
}

float VectorNormalize(float * v){
	float	length,ilength;
	length=VectorLength(v);
	if(length){
		ilength=1/length;
		v[0]*=ilength;
		v[1]*=ilength;
		v[2]*=ilength;
	}
	return length;
}

float VectorDistance(const float* origin,const float* dest){
  float vec[3];
  VectorSubtract(origin,dest,vec);
  return VectorLength(vec);
}

float VectorAngle(const float *a,const float *b){
  float length_a = VectorLength(a);
	float length_b = VectorLength(b);
	float length_ab = length_a*length_b;
	if( length_ab==0.0 ){
    return 0.0;
  }else{
    return (float) (acos(_DotProduct(a,b)/length_ab)*M_R2D);
  }
}

void VectorAngles(const float *vector,float *angles){
	double tmp,yaw,pitch;
	if(vector[1]==0&&vector[0]==0){
		if(vector[2]>0)pitch=90.0f;
		else pitch=270.0f;yaw=0;
	}else{
		yaw=(atan2(vector[1],vector[0])*M_R2D);
		if(yaw<0)yaw+=360.0f;
		tmp=sqrt(vector[0]*vector[0]+vector[1]*vector[1]);
		pitch=(atan2(vector[2],tmp)*M_R2D);
	}
	angles[0]=(float)pitch;angles[1]=(float)yaw;angles[2]=0;
}

//void MakeVector(const vec3_t angle,vec3_t vector)
void AnglesVector(const float *angle,float *vector){
	float pitch,yaw,tmp;
	pitch=(angle[0]*M_D2R);
	yaw=(angle[1]*M_D2R);
	tmp=cos(pitch);
	vector[0]=(-tmp*-cos(yaw));
	vector[1]=(sin(yaw)*tmp);
	vector[2]=-sin(pitch);
}

inline float AngleNormalizePitch(float angle){
	angle=fmod(angle,360.0f);
	if(angle<-89.0f)
		angle=89.0f;
	else if(angle>89.0f)
		angle=89.0f;
	return angle;
}

inline float AngleNormalizeYaw(float angle){
	angle=fmod(angle,360.0f);
	if(angle<-180.0f)
		angle+=360.0f;
	else if(angle>180.0f)
		angle-=360.0f;
	return angle;
}

inline float AngleNormalize(float angle){
	while(angle<-180)
	  angle+=360;
	while(angle>180)
	  angle-=360;
	return angle;
}

void VectorTransform(float *in1,float in2[3][4],float *out){
  out[0]=DotProduct(in1,in2[0])+in2[0][3];
  out[1]=DotProduct(in1,in2[1])+in2[1][3];
  out[2]=DotProduct(in1,in2[2])+in2[2][3];
}

void VectorRotateX(const float* in,float angle,float* out){
	float a,c,s;
	a=(angle*M_D2R);
	c=cos(a);
	s=sin(a);
	out[0]=in[0];
	out[1]=c*in[1]-s*in[2];
	out[2]=s*in[1]+c*in[2];
}

void VectorRotateY(const float* in,float angle,float* out){
	float a,c,s;
	a=(angle*M_D2R);
	c=cos(a);
	s=sin(a);
	out[0]=c*in[0]+s*in[2];
	out[1]=in[1];
	out[2]=-s*in[0]+c*in[2];
}

void VectorRotateZ(const float* in,float angle,float* out){
	float a,c,s;
	a=(angle*M_D2R);
	c=cos(a);
	s=sin(a);
	out[0]=c*in[0]-s*in[1];
	out[1]=s*in[0]+c*in[1];
	out[2]=in[2];
}

bool NewCalcScreen(const float* in, float* out){
  vec3_t aim,newaim,view,tmp;
  float num;
  if(!in||!out||!gMe.iFOV)
    return false;
  VectorSubtract(in,gMe.vieworg,aim);
  AnglesVector(gMe.viewangles,view);
  if(VectorAngle(view,aim)>(gMe.iFOV/1.8))
    return false;
  VectorRotateZ(aim,-gMe.viewangles[1],newaim);
  VectorRotateY(newaim,-gMe.viewangles[0],tmp);
  VectorRotateX(tmp,-gMe.viewangles[2],newaim);
  if(newaim[0]<=0)
    return false;
  num=((displayCenterX/newaim[0])*(120.0/gMe.iFOV - 1.0/3.0));
  out[0]=displayCenterX-num*newaim[1];
  out[1]=displayCenterY-num*newaim[2];
  BOUND_VALUE(out[0],0,displayCenterX*2);
  BOUND_VALUE(out[1],0,displayCenterY*2);
  return true;
}

bool CalcScreen(const float *origin,float *vecScreen){
  int result=gEngfuncs.pTriAPI->WorldToScreen(const_cast<float*>(origin),vecScreen);
  if(!result&&vecScreen[0]<1&&vecScreen[1]<1&&vecScreen[0]>-1&&vecScreen[1]>-1){
    vecScreen[0]=vecScreen[0]*displayCenterX+displayCenterX;
    vecScreen[1]=-vecScreen[1]*displayCenterY+displayCenterY;
    return true;
  }
  return false;
}

void TraceGround(float *vInput,float *vOutput){
	float vTemp[3];
	VectorCopy(vInput,vTemp);
	vTemp[2]-=999;
	vInput[2]+=30;
	pmtrace_t *trace=gEngfuncs.PM_TraceLine(vInput,vTemp,PM_TRACELINE_PHYSENTSONLY,2,-1);
	vInput[2]-=30;
	VectorCopy(trace->endpos,vOutput);
}

void pfnTintRGBA(int x,int y,int width,int height,int r,int g,int b,int a){
  //For 1.5 gEngfuncs.pfnTintRGBA NOT WORK
  oglSubtractive=true;
  gEngfuncs.pfnFillRGBA(x,y,width,height,r,g,b,a);
  oglSubtractive=false;
}

void DrawCircle(int x,int y,int s,int r,int g,int b,int a){
  for(int i=0;i<(2*s);i++){
    int ty=i-s;int tx=(int)sqrt(s*s-ty*ty);
    pfnTintRGBA(x-tx,y+ty,2*tx,1,r,g,b,a);
  }
}

void DrawVectorDot(float *origin,int r,int g,int b,int a){
  float fVecScreen[3];
  if(CalcScreen(origin,fVecScreen))
  pfnTintRGBA((int)fVecScreen[0],(int)fVecScreen[1],2,2,r,g,b,a);
}

void DrawDot(int x,int y,int r,int g,int b,int a){
  pfnTintRGBA(x,y,2,2,r,g,b,a);
}

void DrawSemiCircle(int x,int y,int s,int ag,int am,int r,int g,int b,int a){
  for(int i=3;i<s;i++)
  for(int j=0;j<(2*am);j+=5){
    int al=ag-am+j;
    int tx=(int)(i*cos(al*M_D2R));
    int ty=(int)(-i*sin(al*M_D2R));
    gEngfuncs.pfnFillRGBA(x+tx,y+ty,1,1,r,g,b,a);
  }
}

void DrawLine(float *from,float *to,int r,int g,int b,float d){
  int beamindex=gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/laserbeam.spr");
  if(beamindex)
    gEngfuncs.pEfxAPI->R_BeamPoints(from,to,beamindex,d,1.2f,0,32,2,0,0,r/255,g/255,b/255);
}

void DrawTrace(int ent,float life,float width,int r,int g,int b){
  int beamindex=gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/laserbeam.spr");
  if(beamindex)
    gEngfuncs.pEfxAPI->R_BeamFollow(ent,beamindex,life,width,r,g,b,0.0001);
}

void DrawCrosshair(int xcross,int ycross,int iWidth,int iHeight){

  static int lastCrosshair=0;

  if(gMe.iFOV<50&&gWeapon.IsSnipper())return;

	ColorEntry color = gColorList.get(10); // "cross"
	ColorEntry colo2 = gColorList.get(11); // "cross2"

	int r=color.r,g=color.g,b=color.b,a=color.a;
	int R=colo2.r,G=colo2.g,B=colo2.b,A=colo2.a;

	if(!xcross||!ycross){
    xcross=displayCenterX;
    ycross=displayCenterY;
  }

  if(!iWidth&&!iHeight){
    iWidth=screeninfo.iWidth;
    iHeight=screeninfo.iHeight;
  }

	if(gMe.zoom){
    if(cvar.crosshair!=6){
      lastCrosshair=cvar.crosshair;
      cvar.crosshair=6;
    }
	}else{
	  if(lastCrosshair){
      cvar.crosshair=lastCrosshair;
      lastCrosshair=0;
	  }
	}

	if(cvar.crosshair==1){
		pfnTintRGBA(xcross-14,ycross,9,1,r,g,b,a);
		pfnTintRGBA(xcross+5,ycross,9,1,r,g,b,a);
		pfnTintRGBA(xcross,ycross-14,1,9,r,g,b,a);
		pfnTintRGBA(xcross,ycross+5,1,9,r,g,b,a);
		pfnTintRGBA(xcross,ycross,1,1,R,G,B,A);
	}else if(cvar.crosshair==2){
		pfnTintRGBA(xcross-25,ycross,50,1,r,g,b,a);
		pfnTintRGBA(xcross,ycross-25,1,50,r,g,b,a);
		pfnTintRGBA(xcross-5,ycross,10,1,R,G,B,A);
		pfnTintRGBA(xcross,ycross-5,1,10,R,G,B,A);
	}else if(cvar.crosshair==3){
		pfnTintRGBA(xcross-25,ycross,50,2,r,g,b,a);
		pfnTintRGBA(xcross,ycross-25,2,50,r,g,b,a);
		pfnTintRGBA(xcross-5,ycross,10,2,R,G,B,A);
		pfnTintRGBA(xcross,ycross-5,2,10,R,G,B,A);
	}else if(cvar.crosshair==4){
		pfnTintRGBA(xcross-25,ycross-25,50,1,255,255,255,180);
		pfnTintRGBA(xcross-25,ycross+25,50,1,255,255,255,180);
		pfnTintRGBA(xcross-25,ycross-25+1,1,50-1,255,255,255,180);
		pfnTintRGBA(xcross+25,ycross-25,1,50+1,255,255,255,180);
		pfnTintRGBA(xcross,ycross-25+1,1,50-1,0,160,0,180);
		pfnTintRGBA(xcross-25+1,ycross,50-1,1,0,160,0,180);
	}else if(cvar.crosshair==5){
    pfnTintRGBA(xcross-iWidth/2,ycross,iWidth,1,r,g,b,a);
    pfnTintRGBA(xcross,ycross-iHeight/2,1,iHeight,r,g,b,a);
    pfnTintRGBA(xcross-5,ycross,10,1,R,G,B,A);
    pfnTintRGBA(xcross,ycross-5,1,10,R,G,B,A);
  }else{
		pfnTintRGBA(xcross-1,ycross,3,1,R,G,B,A);
		pfnTintRGBA(xcross,ycross-1,1,3,R,G,B,A);
  }
}

void DrawCross(int x,int y,int w,int h,int r,int g,int b,int a){
	pfnTintRGBA(x-w,y,w*2,2,r,g,b,a);
	pfnTintRGBA(x,y-h,2,h,r,g,b,a);
	pfnTintRGBA(x-w,y-1,w,1,0,0,0,a);
	pfnTintRGBA(x+2,y-1,w-2,1,0,0,0,a);
	pfnTintRGBA(x-w,y+2,w*2,1,0,0,0,a);
	pfnTintRGBA(x-w-1,y-1,1,4,0,0,0,a);
	pfnTintRGBA(x+w,y-1,1,4,0,0,0,a);
	pfnTintRGBA(x-1,y-h,1,h-1,0,0,0,a);
	pfnTintRGBA(x+2,y-h,1,h-1,0,0,0,a);
	pfnTintRGBA(x-1,y-h-1,4,1,0,0,0,a);
}

void DrawFilledBox(int x,int y,int r,int w,int br,int bg,int bb,int ba,int cr,int cg,int cb,int ca){
  int rr=r>>1;
  pfnTintRGBA(x-rr+1,y-rr+1,r-1,r-1,cr,cg,cb,ca);//bg
	pfnTintRGBA(x-rr,y-rr,r,w,br,bg,bb,ba);//sup horiz
	pfnTintRGBA(x+rr,y-rr,w,r,br,bg,bb,ba);//rigth vert
	pfnTintRGBA(x-rr,y+rr,r,w,br,bg,bb,ba);//inf horiz
	pfnTintRGBA(x-rr,y-rr,w,r,br,bg,bb,ba);//left vert
}

void DrawBox(int x,int y,int radius,int width,int r,int g,int b,int a){
	int radius2=radius<<1;
	gEngfuncs.pfnFillRGBA(x-radius,y-radius,radius2,width,r,g,b,a);
	gEngfuncs.pfnFillRGBA(x+radius,y-radius,width,radius2+width,r,g,b,a);
  gEngfuncs.pfnFillRGBA(x-radius,y+radius,radius2,width,r,g,b,a);
	gEngfuncs.pfnFillRGBA(x-radius,y-radius+width,width,radius2-width,r,g,b,a);
}

void DrawRectangle(int x,int y,int w,int h,int r,int g,int b,int a){
	Draw3DEdge(x,y,w,h,r,g,b,a,r,g,b,a);
}

void Draw3DEdge(int x,int y,int w,int h,int br,int bg,int bb,int ba,int dr,int dg,int db,int da){
  pfnTintRGBA(x-1,y-1,w+2,1,br,bg,bb,ba);//top
	pfnTintRGBA(x-1,y,1,h,br,bg,bb,ba);//left
	pfnTintRGBA(x,y+h-1,w,1,dr,dg,db,da);//bottom
	pfnTintRGBA(x+w,y,1,h,dr,dg,db,da);//right
}

void DrawString(bool bCenter,int x,int y,int r,int g,int b,const char *fmt,...){
  va_list va_alist;char buf[1024];
  va_start(va_alist,fmt);
  vsnprintf(buf,sizeof(buf),fmt,va_alist);
  va_end(va_alist);

  if(bCenter){
    int iLength,iHeight;
    gEngfuncs.pfnDrawConsoleStringLen(buf,&iLength,&iHeight);
    x-=iLength/2;y-=iHeight/2;
  }

  gEngfuncs.pfnDrawSetTextColor(r/255.0f,g/255.0f,b/255.0f);
  gEngfuncs.pfnDrawConsoleString(x,y,buf);
}

void DrawSpecialString(bool bCenter,int x,int y,int r,int g,int b,const char *fmt,...){
  va_list va_alist;char buf[1024],esp[32],*p,*q;
  va_start(va_alist,fmt);
  vsnprintf(buf,sizeof(buf),fmt,va_alist);
  va_end(va_alist);

  if(bCenter){
    int iLength,iHeight;
    gEngfuncs.pfnDrawConsoleStringLen(buf,&iLength,&iHeight);
    x-=iLength/2;y-=iHeight/2;
  }

  //draw ::
  p=buf;q=esp;
  while(p&&*p==':')
    *(q++)=*(p++);
  *q=0;
  if(esp[0]){
    int iLength,iHeight;
    gEngfuncs.pfnDrawSetTextColor(r/(3*255.0f),g/(1.5*255.0f),b/(3*255.0f));
    gEngfuncs.pfnDrawConsoleString(x,y,esp);
    gEngfuncs.pfnDrawConsoleStringLen(esp,&iLength,&iHeight);
    x+=iLength;
  }

  //draw first letter (omit blanks)
  while(p&&*p==' ')
    p++;
  esp[0]=*(p++);
  esp[1]=0;
  if(esp[0]){
    int iLength,iHeight;
    gEngfuncs.pfnDrawSetTextColor(r/(1.5*255.0f),g/255.0f,b/(1.5*255.0f));
    gEngfuncs.pfnDrawConsoleString(x,y,esp);
    gEngfuncs.pfnDrawConsoleStringLen(esp,&iLength,&iHeight);
    x+=iLength;
  }

  gEngfuncs.pfnDrawSetTextColor(r/255.0f,g/255.0f,b/255.0f);
  gEngfuncs.pfnDrawConsoleString(x,y,p);
}

void DrawHudString(bool bCenter,int x,int y,int r,int g,int b,const char *fmt,...){
	int drawLen=0;
	va_list va_alist;char buf[1024];
	va_start(va_alist,fmt);
	vsnprintf(buf,sizeof(buf),fmt,va_alist);
	va_end (va_alist);

	int borderY = displayCenterY*2-18;
	int borderX = displayCenterX*2-11;

	if(y<0||y>borderY)return;
  for(const char *p=fmt;*p;p++)
    drawLen+=screeninfo.charWidths[(int)*p];

  if(bCenter)
    x-=drawLen/2;

	if(x<1||x+drawLen>borderX){
		for(char *p=buf;*p;p++){
			int next=x+screeninfo.charWidths[(int)*p];
			if(x>0&&x<borderX)
				gEngfuncs.pfnDrawCharacter(x,y,*p,r,g,b);
			x=next;
		}
	}else {
		for(char *p=buf;*p;p++){
			int next=x+screeninfo.charWidths[(int)*p];
			gEngfuncs.pfnDrawCharacter(x,y,*p,r,g,b);
			x=next;
		}
	}
}

void EchoCmdBlocked(char *cmd){
  CHAR tmp[256];
  sprintf(tmp,"[%s] command has been blocked while you play in a server\n",cmd);
  gEngfuncs.pfnConsolePrint(tmp);
  gGui.AddMessage(255,0,0,3000,tmp);
}
