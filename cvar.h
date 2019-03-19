#ifndef _CVARS_
#define _CVARS_

//#include "main.h"

class cVars{
public:
  cVars();
  void Init();
  void Save();

  float active;
  float showtime,bombtime,showmsgs;
  float takingss;
  float autoname,autonametime;

	float nospread,norecoil,recoilinc,eventnospread;
	float aimingmethod,aimingatme,alivemethod;
	float aimpred,predahead;
	float aimspot,aimshield,triggerbot,aimvisiblespot;
	float smoothaim,smoothbox,triggerboxb,triggerboxh;
	float randspot;
	float autoshoot;
	float rapidfire;
	float autoaim,aimteam;
	float autowall,autobuy,wallsensitivity;
	float avdraw;
	float aimfov,aimdist,drawfov,fovmethod,customfov;
	float box,distance,name,weapon,entesp,health;
	float radar,infobox,netstatus;
	float speed,aspeed,knivespeed,speedfix,frozzen,autoreload,reloadspeed,fpsspeed;
	float aimkey,speedkey;
	float nosmoke,noflash,nosky;
	float crosshair;
	float wallhack;
	float spinhack;
	float soundesp;
	float fullbright,whitewalls,nightmode,nigger,lambert;
	float extrablood;
	float chasecam,quakeguns;
	float bunnyhop,duckhop,duckjump;
	float glowshells,glowents,transhield,playerlight,barrel,trailsesp,chams;
	float spycam,camzoom,czoom;
	float antiss,antissint,antissshow,antissshowtime;
	float hadjh,hadjf,hadjr;
	float speedmode,silentaim,spawnprotect;
  float wireframe,wiremodels,hudhealth;
	float autoroute,ardraw,armaxdist,recorddist,viewroute,smallview;
	float autojump,autoduck,autostrafe,autostandup,fastrun,showspeed;
	float blocksrvcmd,fastunban;
};

extern cVars cvar;

#endif
