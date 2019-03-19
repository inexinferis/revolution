#include "drawing.h"
#include "players.h"
#include "radar.h"
#include "cvar.h"
#include "gui.h"
#include "opengl.h"

cRadar gRadar;
overviewInfo_t cRadar::m_OverviewData;

void cRadar::Init(int sWidth,int sHeight,int sFontWidth,int sFontHeight){
  size=sHeight/9;
  radarx=size+10,radary=size+10+sFontHeight;
  wWidth=sWidth;
  wHeight=sHeight;
}

bool cRadar::ParseOverview(char* overview_txt){
	m_OverviewData.origin[0] = 0.0f;
	m_OverviewData.origin[1] = 0.0f;
	m_OverviewData.origin[2] = 0.0f;
	m_OverviewData.zoom	= 1.0f;
	m_OverviewData.layers = 0;
	m_OverviewData.layersHeights[0]= 0.0f;
	m_OverviewData.layersImages[0][0] = 0;
	char token[1024];
	char* pfile=(char*)gEngfuncs.COM_LoadFile(overview_txt,5,NULL);
	if(!pfile){
		mapLoaded=false;
		return false;
	}
	for(;;){
		pfile=gEngfuncs.COM_ParseFile(pfile,token);
		if(!pfile)
			break;
		if(!stricmp(token,"global")){
			pfile=gEngfuncs.COM_ParseFile(pfile,token);
			if(strcmp(token,"{")){
				mapLoaded=false;
				return false;
			}
			pfile=gEngfuncs.COM_ParseFile(pfile,token);
			if(!pfile)
				break;
			while(strcmp(token,"}")){
				if(!stricmp(token,"zoom")){
					pfile=gEngfuncs.COM_ParseFile(pfile,token);
					m_OverviewData.zoom=(float)atof(token);
				}else if(!stricmp(token,"origin")){
					pfile=gEngfuncs.COM_ParseFile(pfile,token);
					m_OverviewData.origin[0]=(float)atof(token);
					pfile=gEngfuncs.COM_ParseFile(pfile,token);
					m_OverviewData.origin[1]=(float)atof(token);
					pfile=gEngfuncs.COM_ParseFile(pfile,token);
					m_OverviewData.origin[2]=(float)atof(token);
				}else if(!stricmp(token,"rotated")){
					pfile=gEngfuncs.COM_ParseFile(pfile,token);
					m_OverviewData.rotated=atoi(token);
				}
				pfile=gEngfuncs.COM_ParseFile(pfile,token);
			}
		}else if(!stricmp(token,"layer")){
			pfile=gEngfuncs.COM_ParseFile(pfile,token);
			if(strcmp(token,"{")){
				mapLoaded=false;
				return false;
			}
			pfile=gEngfuncs.COM_ParseFile(pfile,token);
			while(strcmp(token,"}")){
				if(!stricmp(token,"image")){
					pfile=gEngfuncs.COM_ParseFile(pfile,token);
					strcpy(m_OverviewData.layersImages[m_OverviewData.layers],token);
				}else if(!stricmp(token,"height")){
					pfile=gEngfuncs.COM_ParseFile(pfile,token);
					float height=(float)atof(token);
					m_OverviewData.layersHeights[m_OverviewData.layers]=height;
				}
				pfile=gEngfuncs.COM_ParseFile(pfile,token);
			}
			m_OverviewData.layers++;
		}
	}
	return true;
}

void cRadar::LoadOverview(char* levelname){
	static char last_levelname[256]="";
	char overview_txt[256];
	if(!strcmp(last_levelname,levelname))
    return;
	if(levelname[0]==0)
    strcpy(levelname,"cs_miltia");
	sprintf(overview_txt,"overviews/%s.txt",levelname);
	if(!ParseOverview(overview_txt)){
		strcpy(last_levelname,levelname);
		mapLoaded=false;
		return;
	}
	m_MapSprite=gEngfuncs.LoadMapSprite(m_OverviewData.layersImages[0]);
	if(!m_MapSprite){
		strcpy(last_levelname,levelname);
		mapLoaded=false;
		return;
	}
	mapLoaded=true;
	int i=(int)sqrt(m_MapSprite->numframes/(4*3));
	xTiles=i*4;
	yTiles=i*3;
}

void cRadar::DrawOverview(){
  static bool LoadOvRadar=false;
  float ov_dx=3,ov_dy=4;
	if(!LoadOvRadar){
    char map_name[100];
    strcpy(map_name,strrchr(gEngfuncs.pfnGetLevelName(),'/')+1);
    *strchr(map_name,'.')=0;
		LoadOverview(map_name);
		LoadOvRadar=true;
	}
	pglViewport(radarx-size,(wHeight-radary-size),(size*2),(size*2));
	if(m_MapSprite){
    float vStepRight[2],vStepUp[2],inner[2],outer[2];
		float z=((90.0f-gMe.viewangles[0])/90.0f)*m_OverviewData.layersHeights[0];
		float xStep= (2*4096.0f/ov_zoom)/xTiles;
		float yStep=-(2*4096.0f/(ov_zoom*(ov_dy/ov_dx)))/yTiles;
		float angle=(float)((gMe.viewangles[1]+90.0)*M_D2R);
		if(m_OverviewData.rotated)
      angle-=float(M_PI/2);
		vStepRight[0]=(float)cos(angle)*xStep;
		vStepRight[1]=(float)sin(angle)*xStep;
		vStepUp[0]=(float)cos(angle+(M_PI/2))*yStep;
		vStepUp[1]=(float)sin(angle+(M_PI/2))*yStep;
		float tile_x,tile_y;
		if(m_OverviewData.rotated){
      float origin_tilex=(float)(-ov_dy+m_OverviewData.zoom*(1.0/1024.0)*m_OverviewData.origin[0]);
      float origin_tiley=(float)( ov_dx+m_OverviewData.zoom*(1.0/1024.0)*m_OverviewData.origin[1]);
      tile_y=-(float)(origin_tilex-(1.0/1024)*m_OverviewData.zoom*gMe.pmEyePos[0]);
      tile_x= (float)(origin_tiley-(1.0/1024)*m_OverviewData.zoom*gMe.pmEyePos[1]);
    }else{
      float origin_tilex=(float)(ov_dx+m_OverviewData.zoom*(1.0/1024.0)*m_OverviewData.origin[0]);
      float origin_tiley=(float)(ov_dy+m_OverviewData.zoom*(1.0/1024.0)*m_OverviewData.origin[1]);
      tile_x=(float)(origin_tilex-(1.0/1024)*m_OverviewData.zoom*gMe.pmEyePos[0]);
      tile_y=(float)(origin_tiley-(1.0/1024)*m_OverviewData.zoom*gMe.pmEyePos[1]);
    }
		outer[0]=(wWidth/2)-tile_x*vStepRight[0]-tile_y*vStepUp[0];
		outer[1]=(wHeight/2)-tile_x*vStepRight[1]-tile_y*vStepUp[1];
		gEngfuncs.pTriAPI->RenderMode(kRenderTransTexture);
		gEngfuncs.pTriAPI->CullFace(TRI_NONE);
		pglEnable(GL_BLEND);
		pglColor4f(1.0f,1.0f,1.0f,0.8f);
		for(int ix=0,frame=0;ix<yTiles;ix++){
			inner[0]=outer[0];
			inner[1]=outer[1];
			for(int iy=0;iy<xTiles;iy++){
				gEngfuncs.pTriAPI->SpriteTexture(m_MapSprite,frame);
				gEngfuncs.pTriAPI->Begin(TRI_QUADS);
				gEngfuncs.pTriAPI->TexCoord2f(0,0);
				gEngfuncs.pTriAPI->Vertex3f(inner[0],inner[1],z);
				gEngfuncs.pTriAPI->TexCoord2f(0,1);
				gEngfuncs.pTriAPI->Vertex3f(inner[0]+vStepRight[0],inner[1]+vStepRight[1],z);
				gEngfuncs.pTriAPI->TexCoord2f(1,1);
				gEngfuncs.pTriAPI->Vertex3f(inner[0]+vStepRight[0]+vStepUp[0],inner[1]+vStepRight[1]+vStepUp[1],z);
				gEngfuncs.pTriAPI->TexCoord2f(1,0);
				gEngfuncs.pTriAPI->Vertex3f(inner[0]+vStepUp[0],inner[1]+vStepUp[1],z);
				gEngfuncs.pTriAPI->End();
				frame++;
				inner[0]+=vStepUp[0];
				inner[1]+=vStepUp[1];
			}
			outer[0]+=vStepRight[0];
			outer[1]+=vStepRight[1];
		}
		pglDisable(GL_BLEND);
	}
	pglViewport(0,0,wWidth,wHeight);
	DrawCross(radarx,radary,size/8,size/8,255,255,255,254);
}

void cRadar::DrawRadar(){
  if(gEngfuncs.Con_IsVisible())
    return;
  if(cvar.speedmode){
    pfnTintRGBA(wWidth/2,wHeight/2-size,1,2*size,220,128,60,60);
		pfnTintRGBA(wWidth/2-size,wHeight/2,2*size,1,220,128,60,60);
  }else{
    gGui.DrawWindow(radarx-size,radary-size,2*size,2*size+1,true,"Radar");
    if(cvar.radar==2&&mapLoaded)
      DrawOverview();
    else{
      pfnTintRGBA(radarx,radary-size,1,2*size,255,255,255,180);
      pfnTintRGBA(radarx-size,radary,2*size,1,255,255,255,180);
    }
	}
}

void cRadar::DrawPoint(float* origin,float* angles,int r,int g,int b,int a,int boxsize){
  int screenx,screeny;
  if(gEngfuncs.Con_IsVisible())
    return;
  if(cvar.radar==2&&!cvar.speedmode&&mapLoaded){
    float aim[3],newaim[3];
    aim[0]=origin[0]-gMe.pmEyePos[0];
    aim[1]=origin[1]-gMe.pmEyePos[1];
    aim[2]=origin[2]-gMe.pmEyePos[2];
    VectorRotateZ(aim,-gMe.viewangles[1],newaim);
    screenx=(radarx)-int(newaim[1]/ov_zoom*m_OverviewData.zoom*0.3f*size/160);
    screeny=(radary)-int(newaim[0]/ov_zoom*m_OverviewData.zoom*0.4f*size/160);
		if(screenx>(radarx+size-boxsize-1))
      screenx=radarx+size-boxsize-1;
		else if(screenx<(radarx-size-1))
      screenx=radarx-size-1;
		if(screeny>(radary+size-boxsize-1))
      screeny=radary+size-boxsize-1;
		else if(screeny<(radary-size-1))
      screeny=radary-size-1;
	}else{
    float sin_yaw, minus_cos_yaw;
    float dx=origin[0]-gMe.pmEyePos[0];
    float dy=origin[1]-gMe.pmEyePos[1];
    float yaw=gMe.viewangles[1]*(M_PI/180.0);
    sin_yaw=sin(yaw);minus_cos_yaw=-cos(yaw);
    float x=dy*minus_cos_yaw+dx*sin_yaw;
    float y=dx*minus_cos_yaw-dy*sin_yaw;
    float range=2500.0f;
    if(fabs(x)>range||fabs(y)>range){
      if(y>x){
        if(y>-x){
          x=range*x/y;
          y=range;
        }else{
          y=-range*y/x;
          x=-range;
        }
      }else{
        if(y>-x){
          y=range*y/x;
          x=range;
        }else{
          x=-range*x/y;
          y=-range;
        }
      }
    }
    if(cvar.speedmode){
      screenx=wWidth/2+int(x/range*float(size));
      screeny=wHeight/2+int(y/range*float(size));
    }else{
      screenx=radarx+int(x/range*float(size));
      screeny=radary+int(y/range*float(size));
    }
  }
  if(angles&&!cvar.speedmode){//cvar.radar==2&&
    float view[3],rangles[3];
    view[0]=origin[0]-gMe.pmEyePos[0];
    view[1]=origin[1]-gMe.pmEyePos[1];
    view[2]=origin[2]-gMe.pmEyePos[2];
    VectorAngles(view,rangles);
    DrawSemiCircle(screenx-1,screeny-1,15,(int)(angles[1]-rangles[1]+90),30,r,g,b,50);
  }
  pfnTintRGBA(screenx-1,screeny-1,boxsize,boxsize,r,g,b,a);
  DrawRectangle(screenx-1,screeny-1,boxsize,boxsize+1,0,0,0,254);
}
