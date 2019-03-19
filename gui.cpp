#include "drawing.h"
#include "players.h"
#include "cvar.h"
#include "gui.h"
#include "nospread.h"
#include "opengl.h"
#include <time.h>

cGui gGui;

void cGui::Init(int sWidth,int sHeight,int sFontWidth,int sFontHeight){
  iWidth=sWidth;
  iHeight=sHeight;
  iFontWidth=sFontWidth;
  iFontHeight=sFontHeight;
  //Fade...
	for(int i = 0; i < sFontHeight+2; i++){
		r[i] = 32 - (i*1);
		g[i] = 180 - (i*5);
		b[i] = 0;
	}
}

void cGui::DrawTime(){
  SYSTEMTIME sSystemTime;
  GetLocalTime(&sSystemTime);
  DrawHudString(true,iWidth/2,5,255,255,255,"Current time: %02d:%02d:%02d",sSystemTime.wHour,sSystemTime.wMinute,sSystemTime.wSecond);
}

void cGui::DrawInfoBox(){
  char status[32];
  int x=10,y=iHeight-iFontHeight*6;
  sprintf(status,"Infobox: %s",gMe.alive?"Alive":"Dead");
	DrawWindow(x,y,iFontWidth*17,iFontHeight*5+7,true,status);
	DrawString(false,x+4,y+=2,255,255,255,"Health: %i", gMe.iHealth);
	DrawString(false,x+4,y+=iFontHeight,255,255,255,"Ammo: %i", gMe.iClip < 0 ? 0 : gMe.iClip);
	DrawString(false,x+4,y+=iFontHeight,255,255,255,"Armor: %i", gMe.iArmor);
	DrawString(false,x+4,y+=iFontHeight,255,255,255,"K: %i - D: %i - HS: %i", gMe.iKills, gMe.iDeaths, gMe.iHs);
	DrawString(false,x+4,y+=iFontHeight,255,255,255,"Money: %i", gMe.iMoney);
}

void cGui::DrawNetStatusBox(){
	int x=iWidth-(iFontWidth*26+10),y=iHeight-iFontHeight*6;
	gMe.UpdateNetStatus();
	DrawWindow(x,y,iFontWidth*26,iFontHeight*5+7,true,"Netgraph");
  DrawString(false,x+4,y+=2,255,255,255,"Server's Name: %s",gMe.gServerName);
  DrawString(false,x+4,y+=iFontHeight,255,255,255,"Server's IP: %s",gMe.gRemoteIP);
  DrawString(false,x+4,y+=iFontHeight,255,255,255,"Player's IP: %s",gMe.gLocalIP);
  DrawString(false,x+4,y+=iFontHeight,255,255,255,"Ping: %i",gMe.ping);
  DrawString(false,x+4,y+=iFontHeight,255,255,255,"FPS: %.2f",gMe.fps);
}

void cGui::DrawSpyCam(){
  if(gMe.alive){
    int x=iWidth-(iWidth/4+10),y=10+iFontHeight;
    DrawWindow(x,y,iWidth/4+2,iHeight/4+2,false,"SpyCam");
    if(cvar.spycam==1.0)
      DrawCrosshair((7*iWidth/8)-9,iHeight-(7*iHeight)/8+10+iFontHeight,iWidth/4+2,iHeight/4+2);
  }
}

void cGui::DrawScreenShot(){
  int x=iWidth-(iWidth/3+10),y=10+iFontHeight;
  DrawWindow(x,y,iWidth/3,iHeight/3+1,false,"ScreenShot");
  glDrawTexture(x,y,iWidth/3,iHeight/3);
}

void cGui::DrawTitleBox(int x, int y, int w, int h,const char* title){
	for(int i=1;i<iFontHeight;i++)
		pfnTintRGBA(x+1,y+i,w-2,1,r[i],g[i],b[i],150);
  DrawRectangle(x,y,w,h,0,0,0,255);
  Draw3DEdge(x+1,y+1,w-2,h-2,20,200,0,150,3,30,0,150);
  DrawFilledBox(x+iFontHeight/2,y+iFontHeight/2,iFontHeight/2+1,1,0,0,0,254,136,131,127,150);
	DrawString(false,x+iFontHeight,y,255,255,255,title);
}

void cGui::DrawContentBox(int x, int y, int w, int h, bool bg){
  if(bg)pfnTintRGBA(x,y,w,h,50,50,50,150);//136,131,127,150);
  DrawRectangle(x,y,w,h,0,0,0,255);
}

void cGui::DrawWindow(int x, int y, int w, int h, bool bg,const char* title){
	DrawTitleBox(x,y-(iFontHeight+2),w,iFontHeight+2,title);// + 2 = 1 pixel border * 2
	DrawContentBox(x,y,w,h,bg);
}

void cGui::DrawMessages(){
  int pos=0;
  //remove old elements
  while(lMsgs.size()>4){
    lMsgs.pop_back();
  }
  //draw elements
  std::list<HUDMsg>::iterator it=lMsgs.begin();
  while(it!=lMsgs.end()){
    if(it->timeout>GetTickCount()){
      DrawString(false,10,iHeight+((pos++-14)*(iFontHeight+1)),it->mr,it->mg,it->mb,it->msg);
      ++it;
    }else
      it=lMsgs.erase(it);
  }
}

void cGui::AddMessage(int r,int g,int b,unsigned int timeout,const char *fmt,...){
  HUDMsg tmp;char buf[1024];
  va_list va_alist;
	va_start(va_alist,fmt);
	vsnprintf(buf,sizeof(buf),fmt,va_alist);
	va_end (va_alist);
  tmp.mr=r;tmp.mg=g;tmp.mb=b;
  strcpy(tmp.msg,buf);
  tmp.timeout=GetTickCount()+timeout;
  lMsgs.push_front(tmp);
}
