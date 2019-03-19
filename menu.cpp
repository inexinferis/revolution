#include <fstream>

#include "interpreter.h"
#include "autoroute.h"
#include "players.h"
#include "drawing.h"
#include "utils.h"
#include "main.h"
#include "menu.h"
#include "cvar.h"
#include "gui.h"

cMenu gMenu;

bool AntiSSDisabled(){
  return !cvar.antiss;
}

cMenu::cMenu(){
  playerslist=NULL;
  waypoints=NULL;
  iSelected=0;
  active=false;
}

cMenu::~cMenu(){
  Clear(menus);
}

void cMenu::UpdateUsersList(MenuEntry *sMenus,void *args){
  hud_player_info_t entinfo;
  char tmp[32];
  if(playerslist){
    Clear(*playerslist);
    for(int i=0;i<MAX_VPLAYERS;i++)
      if(i!=gMe.index){
        gEngfuncs.pfnGetPlayerInfo(i,&entinfo);
        if(entinfo.name&&entinfo.name[0]!='\\'){
          sprintf(tmp,"%.14s",entinfo.name);
          AddCVarMenu(playerslist,tmp,&gPlayers[i]->follow,1,1,0);
        }
      }
  }
}

void cMenu::UpdateWayPoints(MenuEntry *sMenus,void *args){
  if(waypoints){
    Clear(*waypoints);
    AddCVarMenu(waypoints,"AutoRoute",&cvar.autoroute,1,1,0);
    AddCVarMenu(waypoints,"DrawRoute",&cvar.ardraw,1,2,0,AntiSSDisabled);
    AddCVarMenu(waypoints,"ViewRoute",&cvar.viewroute,1,1,0);
    AddCVarMenu(waypoints,"SmallView",&cvar.smallview,1,1,0);
    if(cvar.autoroute){
      AddCVarMenu(waypoints,"Record",&gAutoRoute.isRecord,1,1,0);
      if(gAutoRoute.routes.ngroups){
        char tmp[32];
        for(int i=0;i<gAutoRoute.routes.ngroups;i++){
          if(gAutoRoute.routes.group[i].npoints){
            sprintf(tmp,"Clear %d",i);
            AddFuncMenu(waypoints,tmp,&cMenu::ClearWayPoint,(void*)i);
          }
        }
        AddFuncMenu(waypoints,"Clear All",&cMenu::ClearAllWayPoints);
      }
    }
  }
}

void cMenu::ClearWayPoint(MenuEntry *sMenus,void *args){
  if(!gAutoRoute.isRecord){
    gAutoRoute.ClearWayPoint((int)args);
    gGui.AddMessage(255,255,255,1500,"WayPoint %d Cleared",(int)args);
  }
}

void cMenu::ClearAllWayPoints(MenuEntry *sMenus,void *args){
  if(!gAutoRoute.isRecord){
    gAutoRoute.ClearAllWayPoints();
    gGui.AddMessage(255,255,255,1500,"WayPoints Cleared");
  }
}

void cMenu::ChangeName(MenuEntry *sMenus,void *args){
  char cmd[32];
  sprintf(cmd,"name \"%s\"",sMenus->cCVarName);
  gEngfuncs.pfnClientCmd(cmd);
  gGui.AddMessage(255,255,255,1500,"Name changed to %s",sMenus->cCVarName);
}

static void change_name(char *str){
	if(strstr(str,"0")!=NULL){
		*strstr(str,"0")='O';
		return;
	}
	if(strstr(str,"O")!=NULL){
		*strstr(str,"O")='0';
		return;
	}
	if(strstr(str,"o")!=NULL){
		*strstr(str,"o")='0';
		return;
	}
	if(strstr(str,".")!=NULL){
		*strstr(str,".")=',';
		return;
	}
	if(strstr(str,",")!=NULL){
		*strstr(str,",")='.';
		return;
	}
	if(strstr(str,":")!=NULL){
		*strstr(str,":")=';';
		return;
	}
	if(strstr(str,";")!=NULL){
		*strstr(str,";")=':';
		return;
	}
	if(strstr(str,"l")!=NULL){
		*strstr(str,"l")='1';
		return;
	}
	if(strstr(str,"1")!=NULL){
		*strstr(str,"1")='l';
		return;
	}
	if(strstr(str,"|")!=NULL){
		*strstr(str,"|")='l';
		return;
	}
	if(strstr(str,"-")!=NULL){
		*strstr(str,"-")='=';
		return;
	}
	if(strstr(str,"=")!=NULL){
		*strstr(str,"=")='-';
		return;
	}
	if(str[strlen(str)-1]==str[strlen(str)-2]){
		str[strlen(str)-1]=0;
		return;
	}
	strcat(str,".");
}

void cMenu::AutoName(){
  static unsigned int last=0;
  static char names[32][64];
  char cmd[256];
  if(cvar.autoname==1){
    if(nicklist.size()){
      if((++last)>=nicklist.size())
        last=0;
      sprintf(cmd,"name \"%s\"",nicklist[last].c_str());
      gEngfuncs.pfnClientCmd(cmd);
    }
  }else{
    int names_count=0;
    for(int i=0;i<MAX_VPLAYERS;i++){
      if(i!=gMe.index){
        hud_player_info_t entinfo;
        gEngfuncs.pfnGetPlayerInfo(i,&entinfo);
        if(entinfo.name&&entinfo.name[0]!='\\')
          sprintf(names[names_count++],"%.63s",entinfo.name);
      }
    }
    if(names_count>0){
		  int i=rand()%names_count;
		  change_name(names[i]);
		  sprintf(cmd,"name \"%s\"",names[i]);
		  gEngfuncs.pfnClientCmd(cmd);
		}
  }
}

void cMenu::Load(MenuEntry *sMenus,void *args){
  cmd.load_cvars((int)args);
  sMenus->bExpanded=false;
  gGui.AddMessage(255,255,255,1500,"Config %d Loaded",(int)args);
}

void cMenu::Save(MenuEntry *sMenus,void *args){
  cmd.save_cvars((int)args);
  sMenus->bExpanded=false;
  gGui.AddMessage(255,255,255,1500,"Config %d Saved",(int)args);
}

void cMenu::Init(int sWidth,int sHeight,int sFontWidth,int sFontHeight){
  char tmp[32],nick[16];unsigned int i;
  vector<MenuEntry>* smenu;vector<MenuEntry>* smenu2;
  iLeft=7*sWidth/12;
  iHeight=sHeight;
  iFontWidth=sFontWidth;
  iFontHeight=sFontHeight;
  //Menus
  smenu=AddFuncMenu(&menus,":: Visual Options",NULL);
  AddCVarMenu(smenu,"Crosshair",&cvar.crosshair,1,6,0);
	AddCVarMenu(smenu,"Wallhack",&cvar.wallhack,1,2,0);
	AddCVarMenu(smenu,"NoSmoke",&cvar.nosmoke,1,1,0);
	AddCVarMenu(smenu,"NoFlash",&cvar.noflash,1,1,0);
	AddCVarMenu(smenu,"NoSky",&cvar.nosky,1,1,0);
	AddCVarMenu(smenu,"Fullbright",&cvar.fullbright,1,1,0);
	AddCVarMenu(smenu,"Whitewalls",&cvar.whitewalls,1,1,0);
	AddCVarMenu(smenu,"NightMode",&cvar.nightmode,1,1,0);
	AddCVarMenu(smenu,"NiggerMode",&cvar.nigger,1,1,0);
	AddCVarMenu(smenu,"WireFrame",&cvar.wireframe,1,3,0);
	AddCVarMenu(smenu,"WireModels",&cvar.wiremodels,1,3,0);
	AddCVarMenu(smenu,"Lambert",&cvar.lambert,1,1,0);
	AddCVarMenu(smenu,"TransShield",&cvar.transhield,1,1,0);
  AddCVarMenu(smenu,"Radar",&cvar.radar,1,2,0);
  AddCVarMenu(smenu,"InfoBox",&cvar.infobox,1,1,0);
  AddCVarMenu(smenu,"NetStatus",&cvar.netstatus,1,1,0);
	AddCVarMenu(smenu,"SpyCam",&cvar.spycam,1,2,0);
	AddCVarMenu(smenu,"CamZoom",&cvar.camzoom,1,9,0);
	AddCVarMenu(smenu,"Zoom",&cvar.czoom,1,1,0);
	AddCVarMenu(smenu,"Extra-Blood",&cvar.extrablood,1,1,0);
	AddCVarMenu(smenu,"ChaseCam",&cvar.chasecam,1,2,0);
	AddCVarMenu(smenu,"QuakeGuns",&cvar.quakeguns,1,2,0);
	AddCVarMenu(smenu,"Spinhack",&cvar.spinhack,1,1,0);
	AddCVarMenu(smenu,"ShowTime",&cvar.showtime,1,1,0);
	AddCVarMenu(smenu,"BombTime",&cvar.bombtime,1,6,1);
	AddCVarMenu(smenu,"HudHealth",&cvar.hudhealth,1,1,0);

  smenu=AddFuncMenu(&menus,":: Aimbot",NULL);
  AddCVarMenu(smenu,"NoSpread",&cvar.nospread,1,2,0);
	AddCVarMenu(smenu,"NoRecoil",&cvar.norecoil,1,3,0);
	//AddCVarMenu(smenu,"RecoilInc",&cvar.recoilinc,1,2,0);
	AddCVarMenu(smenu,"AimingMethod",&cvar.aimingmethod,1,2,0);
  AddCVarMenu(smenu,"AimPred",&cvar.aimpred,1,3,0);
  AddCVarMenu(smenu,"AimPredHead",&cvar.predahead,0.01,2,0);
	AddCVarMenu(smenu,"AutoAim",&cvar.autoaim,1,1,0);
	AddCVarMenu(smenu,"AimKey",&cvar.aimkey,1,2,0);
	AddCVarMenu(smenu,"AimTeam",&cvar.aimteam,1,2,0);
	AddCVarMenu(smenu,"AimDistance",&cvar.aimdist,1,9,0);
	AddCVarMenu(smenu,"AimSpot",&cvar.aimspot,1,4,1);
	AddCVarMenu(smenu,"AimVisibleSpot",&cvar.aimvisiblespot,1,1,0);
	AddCVarMenu(smenu,"RandSpot",&cvar.randspot,1,1,0);
	AddCVarMenu(smenu,"FovMethod",&cvar.fovmethod,1,2,0);
	AddCVarMenu(smenu,"AimFOV",&cvar.aimfov,1,8,0);
	AddCVarMenu(smenu,"DrawFov",&cvar.drawfov,1,1,0);
	AddCVarMenu(smenu,"SmoothAim",&cvar.smoothaim,0.5,9,0);
	AddCVarMenu(smenu,"AutoWall",&cvar.autowall,1,2,0);
	AddCVarMenu(smenu,"AutoShoot",&cvar.autoshoot,1,2,0);
	AddCVarMenu(smenu,"RapidFire",&cvar.rapidfire,1,1,0);
	AddCVarMenu(smenu,"TriggerBot",&cvar.triggerbot,1,2,0);
	AddCVarMenu(smenu,"AvDraw",&cvar.avdraw,1,1,0);
	AddCVarMenu(smenu,"AutoReload",&cvar.autoreload,1,1,0);
	AddCVarMenu(smenu,"AutoBuy",&cvar.autobuy,1,1,0);
	AddCVarMenu(smenu,"AutoName",&cvar.autoname,1,2,0);
	AddCVarMenu(smenu,"SilentAim",&cvar.silentaim,1,1,0);
	AddCVarMenu(smenu,"SpawnProtect",&cvar.spawnprotect,1,1,0);

  smenu=AddFuncMenu(&menus,":: Speed Hack",NULL);
  AddCVarMenu(smenu,"Speed",&cvar.speed,0.5,9,-9);
	AddCVarMenu(smenu,"Aspeed",&cvar.aspeed,0.5,9,-9);
	AddCVarMenu(smenu,"Speedkey",&cvar.speedkey,1,2,0);
	AddCVarMenu(smenu,"Knivespeed",&cvar.knivespeed,1,1,0);
	AddCVarMenu(smenu,"SpeedFix",&cvar.speedfix,1,1,0);
	AddCVarMenu(smenu,"SpeedMode",&cvar.speedmode,1,1,0);
	AddCVarMenu(smenu,"ReloadSpeed",&cvar.reloadspeed,1,1,0);

	smenu=AddFuncMenu(&menus,":: ESP",NULL);
	AddCVarMenu(smenu,"Box",&cvar.box,1,2,0);
	AddCVarMenu(smenu,"Name",&cvar.name,1,1,0);
	AddCVarMenu(smenu,"Weapon",&cvar.weapon,1,2,0);
	AddCVarMenu(smenu,"Distance",&cvar.distance,1,1,0);
	AddCVarMenu(smenu,"EntEsp",&cvar.entesp,1,1,0);
	AddCVarMenu(smenu,"SoundEsp",&cvar.soundesp,1,2,0);
  AddCVarMenu(smenu,"Playerlight",&cvar.playerlight,1,1,0,AntiSSDisabled);
	AddCVarMenu(smenu,"GlowShells",&cvar.glowshells,1,1,0);
	AddCVarMenu(smenu,"GlowEnts",&cvar.glowents,1,1,0);
  AddCVarMenu(smenu,"Chams",&cvar.chams,1,2,0);
  AddCVarMenu(smenu,"Barrel",&cvar.barrel,1,1,0,AntiSSDisabled);
  AddCVarMenu(smenu,"Trails Esp",&cvar.trailsesp,1,2,0,AntiSSDisabled);
	AddCVarMenu(smenu,"Health",&cvar.health,1,2,0);
	AddCVarMenu(smenu,"BlockSrvCmd",&cvar.blocksrvcmd,1,1,0);

	smenu=AddFuncMenu(&menus,":: KZ Hacks",NULL);
	AddCVarMenu(smenu,"ShowSpeed",&cvar.showspeed,1,1,0);
	AddCVarMenu(smenu,"AutoJump",&cvar.autojump,1,1,0);
	AddCVarMenu(smenu,"AutoDuck",&cvar.autoduck,1,1,0);
	AddCVarMenu(smenu,"AutoStrafe",&cvar.autostrafe,1,1,0);
	AddCVarMenu(smenu,"AutoStandUp",&cvar.autostandup,1,1,0);
	AddCVarMenu(smenu,"FastRun",&cvar.fastrun,1,1,0);
	AddCVarMenu(smenu,"Bunnyhop",&cvar.bunnyhop,1,1,0);
	AddCVarMenu(smenu,"DuckJump",&cvar.duckjump,1,1,0);
	AddCVarMenu(smenu,"DuckHop",&cvar.duckhop,1,1,0);
	smenu=AddFuncMenu(&menus,":: ScreenShot",NULL);
	AddCVarMenu(smenu,"AntiScreenShot",&cvar.antiss,1,1,0);
	AddCVarMenu(smenu,"AntiSSInterval",&cvar.antissint,1,9,0);
	AddCVarMenu(smenu,"ShowScreenShot",&cvar.antissshow,1,1,0);

	smenu=AddFuncMenu(&menus,":: Save & Load",NULL);

	smenu2=AddFuncMenu(smenu,":: Load",NULL);
	for(int i=1;i<=MAX_CUSTOM_CFG;i++){
    sprintf(tmp,"Config %d",i);
    AddFuncMenu(smenu2,tmp,&cMenu::Load,(void*)i);
  }
  smenu2=AddFuncMenu(smenu,":: Save",NULL);
  for(int i=1;i<=MAX_CUSTOM_CFG;i++){
    sprintf(tmp,"Config %d",i);
    AddFuncMenu(smenu2,tmp,&cMenu::Save,(void*)i);
  }
	//Nick List
	if(nksFound){
    ifstream ifs(nkspath,ifstream::in);
    while(ifs.good()){
      ifs.getline(tmp,32);
      sprintf(nick,"%.14s",tmp);
      nicklist.push_back(nick);
    }
    ifs.close();
    if(nicklist.size()){
      smenu=AddFuncMenu(&menus,":: Names",NULL);
      for(i=0;i<nicklist.size();i++)
        AddFuncMenu(smenu,nicklist[i].c_str(),&cMenu::ChangeName);
    }
	}
	//Follow player
	waypoints=AddFuncMenu(&menus,":: WayPoints",&cMenu::UpdateWayPoints);
	//Follow player
	playerslist=AddFuncMenu(&menus,":: Follow",&cMenu::UpdateUsersList);
	//Update!!!
	Update();
}

vector<MenuEntry>* cMenu::AddCVarMenu(vector<MenuEntry> *menu,const char* name,float *value,float step,float max,float min,tShouldShow pShouldShow){
  if(menu){
    MenuEntry tmp;
    tmp.mType=M_CVAR;
    strcpy(tmp.cCVarName,name);
    tmp.fStep=step;
    tmp.fMax=max;
    tmp.fValue=value;
    tmp.fMin=min;
    tmp.bSelected=false;
    tmp.bExpanded=false;
    tmp.pShouldShow=pShouldShow;
    tmp.iLevel=0;
    menu->push_back(tmp);
    return &menu->back().sMenus;
	}
	return NULL;
}

vector<MenuEntry>* cMenu::AddFuncMenu(vector<MenuEntry> *menu,const char* name,funcmenu func,void *args,tShouldShow pShouldShow){
  if(menu){
    MenuEntry tmp;
    tmp.mType=M_FUNC;
    strcpy(tmp.cCVarName,name);
    tmp.pFunc=func;
    tmp.args=args;
    tmp.bSelected=false;
    tmp.bExpanded=false;
    tmp.pShouldShow=pShouldShow;
    tmp.iLevel=0;
    menu->push_back(tmp);
    return &menu->back().sMenus;
	}
	return NULL;
}

void cMenu::Clear(vector<MenuEntry> &menu){
  for(unsigned int i=0;i<menu.size();i++){
    if(menu[i].sMenus.size())
      Clear(menu[i].sMenus);
    menu.clear();
  }
}

bool cMenu::ChildSelected(vector<MenuEntry> &menu){
  unsigned int i,nmenus=menu.size();
  for(i=0;i<nmenus;i++){
    if(menu[i].bSelected)
      return true;
    if(menu[i].bExpanded&&menu[i].sMenus.size()){
      if(ChildSelected(menu[i].sMenus))
        return true;
      menu[i].bExpanded=false;
    }
  }
  return false;
}

void cMenu::UpdateRecursive(vector<MenuEntry> &menu,int level){
  unsigned int i,nmenus=menu.size();
  for(i=0;i<nmenus;i++){
    if(!menu[i].pShouldShow||menu[i].pShouldShow()){
      entrys.push_back(&menu[i]);
      menu[i].iLevel=level;
      if(menu[i].bSelected){
        if(iSelected==UNSELECTED)
          iSelected=entrys.size()-1;
        else
          menu[i].bSelected=false;
      }
      if(menu[i].bExpanded&&menu[i].sMenus.size()){
        if(menu[i].bSelected||ChildSelected(menu[i].sMenus))
          UpdateRecursive(menu[i].sMenus,level+1);
        else
          menu[i].bExpanded=false;
      }
    }
	}
}

void cMenu::Update(){
  //reset selected element
  iSelected=UNSELECTED;
  entrys.clear();
  UpdateRecursive(menus);
  //set first element selected if no one is selected
  if(iSelected==UNSELECTED){
    iSelected=0;//mark selected!
    entrys[iSelected]->bSelected=true;
  }
}

void cMenu::Draw(){
  unsigned int i=0;
  int mHeight=entrys.size()*iFontHeight+7;
  int iTop=(iHeight-mHeight)/2+iFontHeight;
	gGui.DrawWindow(iLeft,iTop,iFontWidth*20,mHeight,true,"Inexinferis Revolution");
	for(;i<entrys.size();i++){
    ColorEntry &clr=gColorList.get((iSelected==i)?9:8);
    switch(entrys[i]->mType){
      case M_CVAR:
        if(entrys[i]->fValue){
          DrawSpecialString(false,2+iLeft+entrys[i]->iLevel*iFontHeight,iTop+i*iFontHeight,clr.r,clr.g,clr.b,entrys[i]->cCVarName);
          DrawString(false,2+iLeft+iFontWidth*15,iTop+i*iFontHeight,clr.r,clr.g,clr.b,"%2.2f",*entrys[i]->fValue);//130
        }
      break;
      case M_FUNC:
        DrawSpecialString(false,2+iLeft+entrys[i]->iLevel*iFontHeight,iTop+i*iFontHeight,clr.r,clr.g,clr.b,entrys[i]->cCVarName);
      break;
      default:break;
    }
	}
}

int cMenu::Key(int keynum){
	if(keynum==K_UPARROW||keynum==K_MWHEELUP){
    entrys[iSelected]->bSelected=false;
		if(iSelected>0)
      iSelected--;
		else
      iSelected=entrys.size()-1;
    entrys[iSelected]->bSelected=true;
		return 0;
	}else if(keynum==K_DOWNARROW||keynum==K_MWHEELDOWN){
	  entrys[iSelected]->bSelected=false;
		if(iSelected<entrys.size()-1)
      iSelected++;
		else
      iSelected=0;
    entrys[iSelected]->bSelected=true;
		return 0;
	}else if(keynum==K_RIGHTARROW||keynum==K_MOUSE1){
	  switch(entrys[iSelected]->mType){
	    case M_CVAR:
        if(entrys[iSelected]->fValue){
          *entrys[iSelected]->fValue+=entrys[iSelected]->fStep;
          if(*entrys[iSelected]->fValue>entrys[iSelected]->fMax)
            *entrys[iSelected]->fValue=entrys[iSelected]->fMin;
          gGui.AddMessage(255,255,255,1500,"%s changed to %.2f",entrys[iSelected]->cCVarName,*entrys[iSelected]->fValue);
        }
      break;
      case M_FUNC:
        entrys[iSelected]->bExpanded=!entrys[iSelected]->bExpanded;
        if(entrys[iSelected]->bExpanded&&entrys[iSelected]->pFunc)
          (this->*(entrys[iSelected]->pFunc))(entrys[iSelected],entrys[iSelected]->args);
      break;
      default:break;
    }
    Update();
		return 0;
	}else if(keynum==K_LEFTARROW||keynum==K_MOUSE2){
		switch(entrys[iSelected]->mType){
	    case M_CVAR:
        if(entrys[iSelected]->fValue){
          *entrys[iSelected]->fValue-=entrys[iSelected]->fStep;
          if(*entrys[iSelected]->fValue<entrys[iSelected]->fMin)
            *entrys[iSelected]->fValue=entrys[iSelected]->fMax;
          gGui.AddMessage(255,255,255,1500,"%s changed to %.2f",entrys[iSelected]->cCVarName,*entrys[iSelected]->fValue);
        }
      break;
      case M_FUNC:
        entrys[iSelected]->bExpanded=!entrys[iSelected]->bExpanded;
        if(entrys[iSelected]->bExpanded&&entrys[iSelected]->pFunc)
          (this->*(entrys[iSelected]->pFunc))(entrys[iSelected],entrys[iSelected]->args);
      break;
      default:break;
		}
		Update();
		return 0;
	}
	return 1;
}
