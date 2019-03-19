#include "interpreter.h"
#include "cvar.h"
#include "main.h"
#include "client.h"
#include "screenshots.h"

cVars cvar;

cVars::cVars(){
  active=1;
  showmsgs=1;
  showtime=1;
  bombtime=3;
  takingss=0;
  nospread=1.0;
  eventnospread=0;
  norecoil=1.0;
  recoilinc=1.0;
  rapidfire=1;
  aimfov=1;
  fovmethod=0;
  drawfov=0;
  customfov=0;
  autowall=0;
  alivemethod=0;
  wallsensitivity=1;
  aimingmethod=1;
  aimingatme=0;
  autoaim=0;
  autoshoot=1;
  autobuy=1;
  weapon=1;
  name=1;
  distance=0;
  box=1;
  health=1;
	radar=1;
	aimpred=1;
	predahead=.1;
  aimspot=1;
  aimvisiblespot=1;
  aimteam=0;
  aimdist=0;
  aimshield=1;
  fpsspeed=0;
  aspeed=0;
  speed=0;
  aimkey=1;
  randspot=0;
  smoothaim=0;
  smoothbox=1.3;
  triggerbot=1;
  triggerboxh=5;
  triggerboxb=10;
  speedkey=0;
  speedfix=0;
  autoreload=1;
  reloadspeed=0;
  frozzen=.6;
  knivespeed=0;
  nosmoke=1;
  noflash=1;
  nosky=1;
  crosshair=6;
  wallhack=1;
  wireframe=0;
  wiremodels=0;
  hudhealth=0;
  infobox=1;
  netstatus=0;
  entesp=0;
  spinhack=0;
  whitewalls=0;
  nightmode=0;
  nigger=0;
  lambert=0;
  playerlight=1;
  fullbright=0;
  extrablood=1;
  chasecam=0;
  quakeguns=0;
  bunnyhop=0;
  duckhop=0;
  duckjump=0;
  avdraw=1;
  glowshells=1;
  glowents=0;
  barrel=0;
  trailsesp=0;
  soundesp=2;
  transhield=0;
  chams=0;
  spycam=0;
  camzoom=1;
  czoom=0;
  antiss=1;
  antissint=5;
  antissshow=1;
  antissshowtime=3;
  hadjh=1.00;//2.5
	hadjf=0.50;//0.5
	hadjr=0.00;//0.0
	speedmode=0;
	autoname=0;
	autonametime=10;
	silentaim=0;
	spawnprotect=0;

  autojump=0;
  autoduck=0;
  autostrafe=0;
	autostandup=0;
	showspeed=0;
	fastrun=0;

	ardraw=1;
	armaxdist=2;
	autoroute=0;
	recorddist=60;
	viewroute=1;
	smallview=0;
	blocksrvcmd=0;
	fastunban=0;
}

void func_alias(){
	cmd.AddAlias(cmd.argC(1),cmd.argC(2));
}

void func_wait(){
  cmd.needWait=TRUE;
}

void func_bind(){
	cmd.AddBind(cmd.argC(1),cmd.argC(2));
}

void func_unbind(){
	cmd.RemoveBind(cmd.argC(1));
}

void func_unbindall(){
	cmd.RemoveBind(NULL);
}

void func_loadcfg(){
  int num=cmd.argI(1);
  if(num>0&&num<=MAX_CUSTOM_CFG)
    cmd.load_cvars(num);
}

void func_exec(){
  CHAR filepath[MAX_PATH];
  const char *filename=cmd.argC(1);
  if(!*filename)return;
  sprintf(filepath,"%s\\%s",chtpath,filename);
  if(!strchr(filename,'.'))
    strcat(filepath,".cfg");
  cmd.execFile(filepath);
}

void func_quit(){
  ExitProcess(0);
}

void cVars::Init(){
  //init cmd...
  cmd.Init();
  //
  cmd.AddCvarFloat("showmsgs",&showmsgs);
	cmd.AddCvarFloat("showtime",&showtime);
	cmd.AddCvarFloat("bombtime",&bombtime);
  cmd.AddCvarFloat("nospread",&nospread);
  cmd.AddCvarFloat("eventnospread",&eventnospread);
  cmd.AddCvarFloat("norecoil",&norecoil);
  cmd.AddCvarFloat("recoilinc",&recoilinc);
  cmd.AddCvarFloat("aimfov",&aimfov);
  cmd.AddCvarFloat("fovmethod",&fovmethod);
  cmd.AddCvarFloat("drawfov",&drawfov);
  cmd.AddCvarFloat("customfov",&customfov);
  cmd.AddCvarFloat("autowall",&autowall);
  cmd.AddCvarFloat("aimingmethod",&aimingmethod);
  cmd.AddCvarFloat("aimingatme",&aimingatme);
  cmd.AddCvarFloat("alivemethod",&alivemethod);
  cmd.AddCvarFloat("autoshoot",&autoshoot);
  cmd.AddCvarFloat("autobuy", &autobuy);
  cmd.AddCvarFloat("weapon",&weapon);
  cmd.AddCvarFloat("name",&name);
  cmd.AddCvarFloat("distance",&distance);
  cmd.AddCvarFloat("health",&health);
  cmd.AddCvarFloat("box",&box);
 	cmd.AddCvarFloat("autoaim",&autoaim);
	cmd.AddCvarFloat("radar",&radar);
  cmd.AddCvarFloat("aimspot",&aimspot);
  cmd.AddCvarFloat("aimvisiblespot",&aimvisiblespot);
  cmd.AddCvarFloat("aimshield",&aimshield);
  cmd.AddCvarFloat("aspeed",&aspeed);
  cmd.AddCvarFloat("speed",&speed);
  cmd.AddCvarFloat("aimkey",&aimkey);
  cmd.AddCvarFloat("aimdist",&aimdist);
  cmd.AddCvarFloat("fpsspeed",&fpsspeed);
  cmd.AddCvarFloat("speedkey",&speedkey);
  cmd.AddCvarFloat("knivespeed",&knivespeed);
  cmd.AddCvarFloat("speedfix",&speedfix);
  cmd.AddCvarFloat("autoreload",&autoreload);
  cmd.AddCvarFloat("reloadspeed",&reloadspeed);
  cmd.AddCvarFloat("wallsensitivity",&wallsensitivity);
  cmd.AddCvarFloat("nosmoke",&nosmoke);
  cmd.AddCvarFloat("noflash",&noflash);
  cmd.AddCvarFloat("crosshair",&crosshair);
  cmd.AddCvarFloat("wallhack",&wallhack);
  cmd.AddCvarFloat("wireframe",&wireframe);
  cmd.AddCvarFloat("wiremodels",&wiremodels);
  cmd.AddCvarFloat("hudhealth",&hudhealth);
  cmd.AddCvarFloat("infobox",&infobox);
  cmd.AddCvarFloat("netstatus",&netstatus);
  cmd.AddCvarFloat("entesp",&entesp);
  cmd.AddCvarFloat("spinhack",&spinhack);
  cmd.AddCvarFloat("whitewalls",&whitewalls);
  cmd.AddCvarFloat("nightmode",&nightmode);
  cmd.AddCvarFloat("nigger",&nigger);
  cmd.AddCvarFloat("lambert",&lambert);
  cmd.AddCvarFloat("playerlight",&playerlight);
  cmd.AddCvarFloat("fullbright",&fullbright);
  cmd.AddCvarFloat("extrablood",&extrablood);
  cmd.AddCvarFloat("chasecam",&chasecam);
  cmd.AddCvarFloat("quakeguns",&quakeguns);
  cmd.AddCvarFloat("bunnyhop",&bunnyhop);
  cmd.AddCvarFloat("duckhop",&duckhop);
  cmd.AddCvarFloat("avdraw",&avdraw);
  cmd.AddCvarFloat("glowshells",&glowshells);
  cmd.AddCvarFloat("glowents",&glowents);
  cmd.AddCvarFloat("aimpred", &aimpred);
  cmd.AddCvarFloat("predahead", &predahead);
  cmd.AddCvarFloat("aimteam", &aimteam);
	cmd.AddCvarFloat("smoothaim", &smoothaim);
	cmd.AddCvarFloat("smoothbox", &smoothbox);
	cmd.AddCvarFloat("triggerbot",&triggerbot);
	cmd.AddCvarFloat("triggerboxh", &triggerboxh);
	cmd.AddCvarFloat("triggerboxb", &triggerboxb);
	cmd.AddCvarFloat("randspot", &randspot);
	cmd.AddCvarFloat("rapidfire", &rapidfire);
	cmd.AddCvarFloat("soundesp", &soundesp);
	cmd.AddCvarFloat("barrel", &barrel);
	cmd.AddCvarFloat("trailsesp", &trailsesp);
	cmd.AddCvarFloat("nosky", &nosky);
	cmd.AddCvarFloat("duckjump", &duckjump);
	cmd.AddCvarFloat("transshield", &transhield);
	cmd.AddCvarFloat("chams", &chams);
	cmd.AddCvarFloat("spycam", &spycam);
	cmd.AddCvarFloat("camzoom", &camzoom);
	cmd.AddCvarFloat("czoom", &czoom);
	cmd.AddCvarFloat("antiss", &antiss);
	cmd.AddCvarFloat("antissint", &antissint);
	cmd.AddCvarFloat("antissshow", &antissshow);
	cmd.AddCvarFloat("antissshowtime", &antissshowtime);
	cmd.AddCvarFloat("hadjh", &hadjh);
	cmd.AddCvarFloat("hadjr", &hadjr);
	cmd.AddCvarFloat("hadjf", &hadjf);

	cmd.AddCvarFloat("autojump", &autojump);
	cmd.AddCvarFloat("autoduck", &autoduck);
	cmd.AddCvarFloat("autostrafe", &autostrafe);
	cmd.AddCvarFloat("autostandup", &autostandup);
	cmd.AddCvarFloat("fastrun", &fastrun);
	cmd.AddCvarFloat("showspeed", &showspeed);

	cmd.AddCvarFloat("ardraw", &ardraw);
	cmd.AddCvarFloat("autoroute", &autoroute);
	cmd.AddCvarFloat("armaxdist", &armaxdist);
	cmd.AddCvarFloat("recorddist", &recorddist);
	cmd.AddCvarFloat("viewroute", &viewroute);
	cmd.AddCvarFloat("smallview", &smallview);

	cmd.AddCvarFloat("autoname", &autoname);
	cmd.AddCvarFloat("autonametime", &autonametime);

	cmd.AddCvarFloat("silentaim",&silentaim);
	cmd.AddCvarFloat("spawnprotect",&spawnprotect);

	//low cuality for speed
	cmd.AddCvarFloat("speedmode", &speedmode);
	cmd.AddCvarFloat("blocksrvcmd", &blocksrvcmd);
	cmd.AddCvarFloat("fastunban", &fastunban);

	cmd.load_cvars();

	if(speedmode){//&&!antiss
    cmd.exec("#gl_affinemodels 0");
    cmd.exec("#gl_alphamin 0.25");
    cmd.exec("#gl_clear 1");
    cmd.exec("#gl_dither 0");
    cmd.exec("#gl_flipmatrix 0");
    cmd.exec("#gl_fog 0");
    cmd.exec("#gl_cull 1");//Activa el render solo para los objetos visibles; en 0 tendreis menos FPS.
    cmd.exec("#gl_keeptjunctions 0");
    cmd.exec("#gl_max_size 128");
    cmd.exec("#gl_lightholes 0");
    cmd.exec("#gl_palette_tex 0");
    cmd.exec("#gl_picmip 2");
    cmd.exec("#gl_playermip 2");
    cmd.exec("#gl_round_down 100");
    cmd.exec("#gl_smoothmodels 0");
    cmd.exec("#gl_spriteblend 1");
    cmd.exec("#gl_texturemode \"GL_LINEAR_MIPMAP_NEAREST\"");
    cmd.exec("#gl_wateramp 0");
    cmd.exec("#gl_wireframe 0");
    cmd.exec("#cl_timeout 9999999");
	}

  cmd.AddCommand("active",func_active);
  cmd.AddCommand("showmenu",func_showmenu);
  cmd.AddCommand("showconsole",func_showconsole);
  cmd.AddCommand("loadconfig",func_saveconfig);
  cmd.AddCommand("saveconfig",func_loadconfig);
  cmd.AddCommand("hidescreenshot",func_hidescreenshot);
  cmd.AddCommand("+customaim",func_customaim);
  cmd.AddCommand("+customautorute",func_customautorute);
  cmd.AddCommand("+customtriggerbot",func_customtriggerbot);
  cmd.AddCommand("-customaim",func_customaim);
  cmd.AddCommand("-customautorute",func_customautorute);
  cmd.AddCommand("-customtriggerbot",func_customtriggerbot);

	cmd.AddCommand("alias",func_alias);
	cmd.AddCommand("bind",func_bind);
	cmd.AddCommand("wait",func_wait);
	cmd.AddCommand("unbind",func_unbind);
	cmd.AddCommand("unbindall",func_unbindall);
	cmd.AddCommand("loadcfg",func_loadcfg);
	cmd.AddCommand("exec",func_exec);
	cmd.AddCommand("quit",func_quit);

  cmd.exec("bind F4 active");
	cmd.exec("bind F5 showmenu");
	cmd.exec("bind MOUSE3 showmenu");
	cmd.exec("bind F6 showconsole");
	cmd.exec("bind ENTER hidescreenshot");
	cmd.exec("bind CAPSLOCK +customautorute");
	cmd.exec("bind ALT +customtriggerbot");

	if(cfgFound)//load config...
    cmd.execFile(cfgpath);
}

void cVars::Save(){
  cmd.save_cvars();
}
