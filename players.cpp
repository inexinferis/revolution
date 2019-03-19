#include "weapons.h"
#include "interpreter.h"
#include "players.h"
#include "sprites.h"
#include "drawing.h"
#include "aimbot.h"
#include "client.h"
#include "radar.h"
#include "utils.h"
#include "main.h"
#include "cvar.h"

cMe gMe;
cPlayers gPlayers;
cEspEnts gEspEnts;
cBomb gBomb;
cHostages gHostages;
cSoundMark gSoundMark[3*MAX_VPLAYERS];

static struct cl_entity_s dummy= {0};

/*******************************************\
 cMe
\*******************************************/

cMe::cMe() {
  alive=team=index=zoom=aimkey=trigger=inway=inspeed=0;
  iClip=iHealth=iArmor=iKills=iDeaths=iHs=iMoney=0;
  pmFallVelocity=pmGroundSpeed=0;
  lastflashtime=0;
  //Dummy Ents
  ent=&dummy;
  bShouldJump=false;
  bNoNeedDuck=false;
  bNeedDuck=false;
  bInDuck=false;
  iFOV=90;//default fov to avoid divide-by-zero exception
  fps=0;
  ping=0;
  iSpectator=0;
  strcpy(gServerName,"Unknown");
  strcpy(gRemoteIP,gServerName);
  strcpy(gLocalIP,gServerName);
}

void cMe::PlayerMove(struct playermove_s *pmove) {
  vec3_t vFromOrigin,vToOrigin,vDeltaOrigin;
  //save vars...
  pmFlags=pmove->flags;
  pmMoveType=pmove->movetype;
  pmFrameTime=pmove->frametime;
  VectorCopy(pmove->origin,pmOrigin);
  VectorCopy(pmove->velocity,pmVelocity);
  VectorCopy(pmove->angles,pmViewAngles);
  gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(pmEyePos);
  pmEyePos=pmEyePos+pmove->origin;
  pmMaxSpeed=pmove->maxspeed;
  pmAirAccelerate=pmove->movevars->airaccelerate;
  pmGroundSpeed=sqrt(pmove->velocity[0]*pmove->velocity[0]+pmove->velocity[1]*pmove->velocity[1]);
  pmFallVelocity=-pmove->velocity[2];
  bInDuck=pmove->bInDuck;
}

void cMe::Update() {
  ent=gEngfuncs.GetLocalPlayer();
  if((DWORD)ent<0x1000)ent=&dummy;//0xBB8 ???
  index=ent->index;
  iSpectator=ent->curstate.iuser1;

  // BLOCKED (serverside)
  //"curstate" alive detection method can fail if server change this values
  //alive=((ent->curstate.effects&EF_NODRAW)&&ent->player&&ent->curstate.movetype!=6&&ent->curstate.movetype!=0);
  /*if(!cvar.alivemethod)
    alive=(ent->curstate.solid)||(ent->curstate.movetype!=0&&ent->curstate.movetype!=6);*/

  alive=(ent->curstate.gaitsequence);
  // TODO: fix spectator
}

void cMe::DoBunnyHop(struct usercmd_s *cmd) {
  if((cmd->buttons&IN_JUMP)&&!(pmFlags&(FL_ONGROUND|FL_PARTIALGROUND|FL_INWATER)))
    cmd->buttons&=~IN_JUMP;
}

void cMe::DoDuckHop(struct usercmd_s *cmd) {
  static bool needduck=true;
  if(!needduck&&(cmd->buttons&IN_DUCK)&&(pmFlags&(FL_ONGROUND|FL_PARTIALGROUND|FL_INWATER)))
    cmd->buttons&=~IN_DUCK;
  needduck=!needduck;
}

void cMe::DoDuckJump(struct usercmd_s *cmd) {
  if(!(pmFlags&(FL_DUCKING|FL_ONGROUND|FL_PARTIALGROUND))&&pmVelocity[2]>0)
    cmd->buttons|=IN_DUCK;
}

void cMe::DrawSpeed() {
  if(pmGroundSpeed) {
    CHAR speedstring[32];
    sprintf(speedstring,"%.2f Units/Sec",pmGroundSpeed);
    DrawHudString(true,screeninfo.iWidth/2,screeninfo.iHeight-100,255,255,255,speedstring);
  }
}

void cMe::DoAutoBuy() {
  if(cvar.speedmode) {
    if(iMoney<2800)
      cmd.exec(".impulse 101;.deagle;.secammo;.vest;.vesthelm");
    else
      cmd.exec(".impulse 101;.scout;.primammo;.vest;.vesthelm;.deagle;.secammo");
  } else {
    switch(team) {
      case 1://t
        if(iMoney==800)
          cmd.exec(".impulse 101;.vest;.flash");
        else if(iMoney<3600)
          cmd.exec(".impulse 101;.vest;.vesthelm;.deagle;.secammo;.hegren;.flash");
        else
          cmd.exec(".impulse 101;.vest;.vesthelm;.ak47;.primammo;.hegren;.flash;.deagle;.secammo");
        break;
      case 2://ct
        if(iMoney==800)
          cmd.exec(".impulse 101;.flash;.sgren;.defuser");
        else if(iMoney<4200)
          cmd.exec(".impulse 101;.vest;.vesthelm;.deagle;.secammo;.defuser;.hegren;.flash");
        else
          cmd.exec(".impulse 101;.vest;.vesthelm;.m4a1;.primammo;.defuser;.hegren;.flash;.deagle;.secammo");
        break;
      default:
        break;
    }
  }
}

void cMe::HandleZoom(unsigned int zoom) {
  iFOV=100-zoom*10;
  BOUND_VALUE(iFOV,10,90);
  pSetFOV("SetFOV",1,&iFOV);
}

bool cMe::ShouldSpeed() {
  return ((gEngfuncs.GetClientTime() - gMe.ent->ph[gMe.ent->current_position].animtime) < cvar.frozzen);
}

float cMe::GetDistanceFrom(const float* pos) {
  return VectorDistance(pos,pmOrigin);
}

bool cMe::PathFree(float* to) {
  pmtrace_t tr;
  gEngfuncs.pEventAPI->EV_SetTraceHull(2);
  gEngfuncs.pEventAPI->EV_PlayerTrace(pmOrigin,to,PM_GLASS_IGNORE|PM_STUDIO_BOX,index,&tr);
  return (tr.fraction==1.0);
}

float cMe::CalcFovAngle(const float* origin) {
  float vec[3],view[3],dot;
  VectorSubtract(origin,pmOrigin,vec);
  VectorNormalize(vec);
  AnglesVector(pmViewAngles,view);
  dot=view[0]*vec[0]+view[1]*vec[1]+view[2]*vec[2];
  if(dot>1.0)dot=1.0;
  return (float)(acos(dot)*M_R2D);
}

void cMe::CheckIsFlashed() {
  static float fadetime=0;
  static screenfade_t sf;
  gEngfuncs.pfnGetScreenFade(&sf);
  if(sf.fadeEnd>serverTime) {
    fadetime=(sf.fadeEnd-serverTime);
    lastflashtime=sf.fadeEnd;
    sf.fadeEnd=0;
    gEngfuncs.pfnSetScreenFade(&sf);
  }
  if(lastflashtime>serverTime&&fadetime) {
    float fp=((lastflashtime-serverTime)/fadetime)*100;
    DrawHudString(true,screeninfo.iWidth/2,screeninfo.iHeight-screeninfo.iHeight/4,255,255,255,"You are Flashed %.1f%%",fp);
  }
}

float cMe::GroundDistance() {
  vec3_t vecFloor;
  VectorCopy(gMe.pmOrigin,vecFloor);
  vecFloor[2]-=8192;
  pmtrace_t *tr=gEngfuncs.PM_TraceLine(gMe.pmOrigin,vecFloor,PM_TRACELINE_ANYVISIBLE,0,-1);
  return (gMe.pmOrigin[2]-tr->endpos[2]);
}

void cMe::AddEntity(struct cl_entity_s *ent) {
  if(cvar.chasecam==2) {
    ent->curstate.renderamt=60;
    ent->curstate.rendermode=kRenderTransColor;
  }
}

void cMe::SetServerName(char *name) {
  if(strlen(name)>14)
    sprintf(gServerName,"%.14s...",name);
  else
    strcpy(gServerName,name);
}

void cMe::UpdateNetStatus() {
  net_status_s netstat;
  gEngfuncs.pNetAPI->Status(&netstat);
  ping=(int)(netstat.latency*1000);
  sprintf(gLocalIP,"%d.%d.%d.%d:%u",netstat.local_address.ip[0],netstat.local_address.ip[1],
          netstat.local_address.ip[2],netstat.local_address.ip[3],netstat.local_address.port);
  sprintf(gRemoteIP,"%d.%d.%d.%d:%u",netstat.remote_address.ip[0],netstat.remote_address.ip[1],
          netstat.remote_address.ip[2],netstat.remote_address.ip[3],netstat.remote_address.port);
}

bool cMe::IsFreeSpectator() {
  return (!alive&&(iSpectator==OBS_ROAMING||iSpectator==OBS_MAP_FREE||iSpectator==OBS_MAP_CHASE));
}

/*******************************************
* cPlayer
*******************************************/

cPlayer::cPlayer(int i) {
  index=i;
  //team
  team=0;
  follow=0;
  //visible
  bVisible=false;
  distance=1000.0;
  //Ent Info
  entinfo.name=(char *)"[unknown-name]";
  //Player data
  strcpy(m_weapon,"unknown");
  //Dummy Ents
  ent=&dummy;
  //aim
  bGotHead=false;
  bCanAim=false;
  bSpawning=false;
  //init health
  hitpoints=MAX_HITPOINTS;
}

ColorEntry &cPlayer::Color() {
  //target
  if(index==gAimbot.iTarget)
    return gColorList.get(3);
  if(follow)
    return gColorList.get(14);
  //someone has the bomb or is vip
  if(info&4||info&2)
    return gColorList.get(12);
  //Terrorist
  if(team==1)return gColorList.get(1);
  //Counter-Terrorist
  if(team==2)return gColorList.get(2);
  return gColorList.get(0);
}

bool cPlayer::CalcScreen(float *vecScreen) {
  return ::CalcScreen(ent->origin,vecScreen);
}

void cPlayer::Update() {
  const char *wname;
  //me data
  gMe.Update();
  //player data
  ent=gEngfuncs.GetEntityByIndex(index);
  if((DWORD)ent<0x1000)
    ent=&dummy;
  else {
    wname=gWeapon.GetNameByModel(ent->curstate.weaponmodel);
    if(wname&&strlen(wname)<30)strcpy(m_weapon,wname);
    distance=gMe.GetDistanceFrom(ent->origin)/22.0f;
    if(distance<1)distance=1;
    if(!(ent->curstate.messagenum<gMe.ent->curstate.messagenum)) {
      VectorCopy(ent->origin,lastpos);
      cSoundMark::Reset(index);
    }
  }
  //player info
  gEngfuncs.pfnGetPlayerInfo(index,&entinfo);
  if(!entinfo.name)
    entinfo.name=(char*)"[unknown-name]";
}

void cPlayer::AddEntity(struct cl_entity_s *ent) {
  bSpawning=(ent->curstate.renderfx==kRenderFxGlowShell&&((ent->curstate.rendercolor.r&&ent->curstate.rendercolor.r!=11)||
             (ent->curstate.rendercolor.g&&ent->curstate.rendercolor.g!=11)||
             (ent->curstate.rendercolor.b&&ent->curstate.rendercolor.b!=11)));
  ent->curstate.renderfx=0;
  ent->curstate.rendermode=0;
  gAimbot.CalcFovAndVisibility(*this);
  bGotHead=false;
  ColorEntry &color=Color();
  // Ligth, barrel and trail (not whit antiscreenshot)
  if(!cvar.antiss&&(!cvar.speedmode||(cvar.speedmode&&!gMe.inspeed))) {
    if(cvar.playerlight) {
      dlight_t *pDl=gEngfuncs.pEfxAPI->CL_AllocDlight(0);
      if(pDl) {
        pDl->color.r=color.r;
        pDl->color.g=color.g;
        pDl->color.b=color.b;
        pDl->origin=ent->origin;
        pDl->die=gEngfuncs.GetClientTime()+0.05;
        pDl->radius=100;
      }
    }
    if(cvar.barrel) {
      vec3_t vecBegin,vecEnd,up,right,forward;
      VectorCopy(ent->origin,vecBegin);
      if(ent->curstate.usehull)
        vecBegin[2]+=12;
      else
        vecBegin[2]+=17;
      gEngfuncs.pfnAngleVectors(ent->angles,forward,right,up);
      forward[2]=-forward[2];
      vecBegin[0]+=forward[0]*10;
      vecBegin[1]+=forward[1]*10;
      vecBegin[2]+=forward[2]*10;
      vecEnd=vecBegin+forward*999;
      DrawLine(vecBegin,vecEnd,color.r,color.g,color.b,0.001f);
    }
    if(cvar.trailsesp&&((cvar.trailsesp==1&&team!=gMe.team)||cvar.trailsesp==2))
      DrawTrace(ent->index,2.5,1,color.r,color.g,color.b);
  }
}

void cPlayer::SetDead() {
  bGotHead=false;
  bCanAim=false;
  bVisible=false;
  hitpoints=MAX_HITPOINTS;
}

void cPlayer::SetWeapon(char *wname) {
  if(wname&&strlen(wname)<30)strcpy(m_weapon,wname);
}

bool cPlayer::IsValidEnt() {
  gMe.Update();
  ent=gEngfuncs.GetEntityByIndex(index);
  return (ent&&gMe.ent&&ent!=gMe.ent&&ent->player&&!(ent->curstate.effects&EF_NODRAW)&&
          !(ent->curstate.messagenum<gMe.ent->curstate.messagenum)&&
          ((ent->curstate.movetype!=0&&ent->curstate.movetype!=6)||
           (ent->curstate.solid&&!ent->curstate.spectator))&&
          !(gMe.ent->curstate.iuser1==OBS_IN_EYE&&gMe.ent->curstate.iuser2==ent->index)
         );
}

void cPlayer::DrawBlood() {
  int speed,spray_count;
  for(spray_count=0; cvar.active&&!cvar.takingss&&spray_count<5; spray_count++) {
    vec3_t blood_origin;
    vec3_t blood_direction;
    blood_origin[0]=ent->origin[0];
    blood_origin[1]=ent->origin[1];
    blood_origin[2]=ent->origin[2]+30;
    blood_direction[0]=(float)(rand()%41)-20;
    blood_direction[1]=(float)(rand()%41)-20;
    blood_direction[2]=(float)(rand()%251)+50;
    speed=(rand()%101)+100;
    gEngfuncs.pEfxAPI->R_BloodStream(blood_origin,blood_direction,70,speed);
  }
}

void cPlayer::DrawEsp() {
  vec3_t vecScreen;
  ColorEntry &color=Color();
  if(cvar.radar)
    gRadar.DrawPoint(ent->origin,ent->angles,color.r,color.g,color.b,255,3);

  if(!CalcScreen(vecScreen))
    return;

  int boxradius=(int)((300.0*90.0)/(distance*gMe.iFOV));
  BOUND_VALUE(boxradius,1,200);

  if(cvar.box==1)
    DrawBox((int)vecScreen[0],(int)vecScreen[1],boxradius,2,color.r,color.g,color.b,color.a);
  else if(cvar.box==2)
    DrawBox((int)vecScreen[0],(int)vecScreen[1],10,2,color.r,color.g,color.b,color.a);

  const char* format_string;
  const char* format_int;

  if(bVisible) {
    format_string="%s";
    format_int="%i";
  } else {
    format_string="-%s-";
    format_int="-%i-";
  }

  int ystep=-14;
  int x=(int)vecScreen[0];
  int y=(int)vecScreen[1]+14;
  if(gMe.alive)y=y-(int)boxradius-1;

  if(cvar.weapon) {
    bool bGotSeq=true;
    const char* displayname=NULL;
    int seqinfo=gWeapon.GetSequenceInfo(ent->curstate.sequence);
    if(seqinfo&SEQUENCE_ARM_C4)
      displayname=" ARMING C4 ";
    else if(seqinfo&SEQUENCE_RELOAD)
      displayname=" RELOADING ";
    else if(seqinfo&SEQUENCE_THROW)
      displayname=" THROWING ";
    else if(seqinfo&SEQUENCE_SHIELD)
      displayname=" SHIELDED ";
    else if(seqinfo&SEQUENCE_SHIELD_SIDE)
      displayname=" SHIELD TO SIDE ";
    else {
      displayname=m_weapon;
      bGotSeq=false;
    }

    if(cvar.weapon==1) {
      DrawString(true,x,y,color.r,color.g,color.b,format_string,displayname);
      y+=ystep;
    } else if(cvar.weapon==2) {
      int index=-1;
      if(!bGotSeq) {
        char buf[255];
        if(strstr(displayname,"nade"))
          sprintf(buf,"d_grenade");
        else if(strstr(displayname,"c4"))
          sprintf(buf,"c4");
        else if(strstr(displayname,"mp5"))
          sprintf(buf,"d_mp5navy");
        else
          sprintf(buf,"d_%s",displayname);
        if(!strstr(buf,"unknown")&&!strstr(buf,"dryfire"))
          index=GetSpriteIndex(buf);
      }
      if(index!=-1) {
        gEngfuncs.pfnSPR_Set(spritedata.spriteinfo[index].hspr,color.r,color.g,color.b);
        gEngfuncs.pfnSPR_DrawAdditive(0,(int)vecScreen[0]-10,(int)vecScreen[1]-50,&(spritedata.spriteinfo[index].rc));
      } else
        DrawString(true, x,y,color.r,color.g,color.b,format_string,displayname);
      y+=ystep;
    }
  }

  if(cvar.name) {
    DrawString(true,x,y,color.r,color.g,color.b,format_string,entinfo.name);
    y+=ystep;
  }

  if(cvar.distance) {
    DrawString(true,x,y,color.r,color.g,color.b,format_int,(short int)distance);
    y+=ystep;
  }

  if(cvar.health==1) {
    int numBars=(int)ceil(hitpoints/2.0);
    ColorEntry &chealth=gColorList.get(15+numBars);
    int offset=10;
    DrawRectangle(x-offset-1,y-1,21,6,255,255,255,155);
    for(int i=0; i<5; i++) {
      if(i<numBars)
        gEngfuncs.pfnFillRGBA(x+(i*4)-offset,y,3,3,chealth.r,chealth.g,chealth.b,chealth.a);
      else
        gEngfuncs.pfnFillRGBA(x+(i*4)-offset,y,3,3,0,0,0,chealth.a);
    }
  } else if(cvar.health==2) {
    int numBars=(int)ceil(hitpoints/2.0);
    ColorEntry &bar=gColorList.get(15+numBars);
    DrawString(true,x,y,bar.r,bar.g,bar.b,"%i hp",(short int)hitpoints*10);
  }
}

float cPlayer::GetDistanceFrom(const float* pos) {
  return VectorDistance(pos,lastpos);
}

/*******************************************\
 cPlayers
\*******************************************/

cPlayer* cPlayers::GetCurrentEntity() {
  cl_entity_s *pEnt=gEngStudio.GetCurrentEntity();
  if(pEnt&&pEnt->index>0&&pEnt->index<MAX_VPLAYERS)
    return player[pEnt->index];
  return player[0];
}

cPlayer* cPlayers::GetViewModel() {
  cl_entity_s *pEnt=gEngfuncs.GetViewModel();
  if(pEnt&&pEnt->index>0&&pEnt->index<MAX_VPLAYERS)
    return player[pEnt->index];
  return player[0];
}

void cPlayers::DrawEsp() {
  for(int i=0; i<MAX_VPLAYERS; i++) {
    if(player[i]->IsValidEnt())
      player[i]->DrawEsp();
  }
}

bool cPlayers::IsValidEnt(cl_entity_s *ent) {
  cl_entity_s* lent=gEngfuncs.GetLocalPlayer();
  return (ent&&lent&&ent!=lent&&ent->player&&!(ent->curstate.effects&EF_NODRAW)&&
          !(ent->curstate.messagenum<gMe.ent->curstate.messagenum)&&
          ((ent->curstate.movetype!=0&&ent->curstate.movetype!=6)||
           (ent->curstate.solid&&!ent->curstate.spectator))
         );
}

void cPlayers::SetDead() {
  for(int i=0; i<MAX_VPLAYERS; i++) {
    player[i]->SetDead();
    cSoundMark::Reset(i);
  }
}

void cPlayers::Update() {
  for(int i=0; i<MAX_VPLAYERS; i++)
    player[i]->Update();
}

void cPlayers::AddEntity(struct cl_entity_s *ent) {
  int px=ent->index;//never will be me!
  player[px]->AddEntity(ent);
}

/*******************************************\
 cEspEnts
\*******************************************/

void cEspEnts::AddEntity(struct cl_entity_s *ent, const char *modelname) {
  if(ent&&IsValidEnt(ent)&&modelname) {
    char *pos=(char*)modelname;
    ColorEntry color;
    if(strstr(pos,"hostage"))
      color=gColorList.get(5);
    else if(strstr(pos,"scientist"))
      color=gColorList.get(5);
    else if((pos=(char*)strstr(pos,"w_"))!=0) {
      if(strstr(pos,"flashbang"))
        color=gColorList.get(8);
      else if(strstr(pos,"hegrenade"))
        color=gColorList.get(1);
      else if(strstr(pos,"smokegrenade"))
        color=gColorList.get(0);
      else if(strstr(pos,"backpack")||strstr(pos,"c4"))
        color=gColorList.get(5);
      else if(strstr(pos,"thighpack"))
        color=gColorList.get(13);
      else
        color=gColorList.get(4);
    } else
      return;
    ent->curstate.rendercolor.r=color.r;
    ent->curstate.rendercolor.g=color.g;
    ent->curstate.rendercolor.b=color.b;
    ent->curstate.renderamt=1;
    ent->curstate.renderfx|=kRenderFxGlowShell;
  }
}

bool cEspEnts::IsValidEnt(cl_entity_s * ent) {
  gMe.Update();
  return (ent&&gMe.ent&&(ent!=gMe.ent)&&ent->curstate.solid
          &&ent->model&&ent->model->name&&!(ent->curstate.effects&EF_NODRAW)
          &&(ent->curstate.messagenum+10>gMe.ent->curstate.messagenum));
}

void cEspEnts::Draw(float *origin,float *angles,const char *entname,bool scolor,bool bradar) {
  vec3_t vecScreen;
  float distance;
  ColorEntry &color=gColorList.get((scolor)?13:5);
  if(cvar.radar&&bradar)
    gRadar.DrawPoint(origin,NULL,color.r,color.g,color.b,255,3);

  if(angles) {
    vec3_t forward,right,up,entOrg;
    VectorCopy(origin,entOrg);
    gEngfuncs.pfnAngleVectors(angles,forward,right,up);
    entOrg=entOrg+up*40;
    if(!CalcScreen(entOrg,vecScreen))
      return;
  } else {
    if(!CalcScreen(origin,vecScreen))
      return;
  }

  distance=gMe.GetDistanceFrom(origin)/22.0f;
  if(distance<1)distance=1;
  int boxradius=(int)((300.0*90.0)/(distance*gMe.iFOV));
  BOUND_VALUE(boxradius,1,200);

  if(cvar.box)
    DrawBox((int)vecScreen[0],(int)vecScreen[1],boxradius,2,color.r,color.g,color.b,color.a);
  if(cvar.name)
    DrawString(true,(int)vecScreen[0],(int)vecScreen[1]-7,color.r,color.g,color.b,"-%s-",entname);
  if(cvar.distance)
    DrawString(true,(int)vecScreen[0],(int)vecScreen[1]+7,color.r,color.g,color.b,"-%i-",(short int)distance);
}

void cEspEnts::DrawEsp() {
  cl_entity_s * ent;
  vec3_t vecScreen;
  char mdl[1024],*pos=mdl;
  for(int i=1; i<1024; i++) {
    ent=gEngfuncs.GetEntityByIndex(i);
    if(!ent||!IsValidEnt(ent)||!ent->model||!ent->model->name)
      continue;
    strncpy(mdl,ent->model->name,1024);
    if(strstr(mdl,"hostage"))
      Draw(ent->origin,ent->angles,"Hostage");
    else if(strstr(mdl,"scientist"))
      Draw(ent->origin,ent->angles,"Scientist");
    else if((pos=(char*)strstr(mdl,"w_"))!=0) {
      if(strstr(pos,"backpack")||strstr(pos,"c4"))
        Draw(ent->origin,NULL,"C4",false,!gBomb.IsDropped());
      else if(strstr(pos,"thighpack"))
        Draw(ent->origin,NULL,"Defusal Kit",true);
      else if(cvar.entesp&&CalcScreen(ent->origin,vecScreen)) {
        pos+=2;
        pos[strlen(pos)-4]=0;
        ColorEntry &clr = gColorList.get(4);
        DrawString(false,(int)vecScreen[0],(int)vecScreen[1],clr.r,clr.g,clr.b,"-%s-",pos);
      }
    }
  }
}

/*******************************************\
 cBomb
\*******************************************/

void cBomb::Dropped(vec3_t &vec,int flag) {
  VectorCopy(vec,origin);
  isDropped=true;
  if(flag) {
    isPlanted=true;
    plantedTime=gEngfuncs.GetClientTime();
  }
}

void cBomb::Pickup() {
  isDropped=false;
  isPlanted=false;
  plantedTime=0;
}

char* cBomb::GetString() {
  if(isPlanted) {
    int seconds=(int)(gEngfuncs.GetClientTime()-plantedTime);
    if(seconds<0||seconds>45) {
      Pickup();
      return NULL;
    }
    seconds=int(cvar.bombtime*10+5)-seconds;//35
    if(seconds<=5)
      sprintf(timerstring,"[ Bomb in %02d Sec - NO DEFUSE ]",seconds);
    else if(seconds<=10)
      sprintf(timerstring,"[ Bomb in %02d Sec - CAN DEFUSE W/KIT ]",seconds);
    else if(seconds>10)
      sprintf(timerstring,"[ Bomb in %02d Sec - CAN DEFUSE ]",seconds);
    return timerstring;
  }
  return NULL;
}

void cBomb::DrawTime() {
  char *tmp=GetString();
  if(tmp)DrawHudString(true,screeninfo.iWidth/2,screeninfo.iHeight-30,255,255,255,tmp);
}

void cBomb::DrawPosition() {
  if(isDropped&&cvar.radar) {
    //draw
    ColorEntry &color=gColorList.get(5);
    gRadar.DrawPoint(origin,NULL,color.r,color.g,color.b,255,3);
  }
}

/*******************************************\
 cHostage
\*******************************************/

void cHostages::DrawPositions() {
  if(cvar.radar)
    for(int i=0; i<MAX_HOSTAGES; i++)
      hostage[i].DrawPosition();
}

void cHostage::DrawPosition() {
  if(isAlive) {
    //draw
    ColorEntry &color=gColorList.get(5);
    gRadar.DrawPoint(origin,NULL,color.r,color.g,color.b,255,3);
  }
}

/*******************************************\
 cSoundMark
\*******************************************/

void cSoundMark::Reset(int i) {
  if(i>0&&i<MAX_VPLAYERS) {
    cSoundMark &sound=gSoundMark[i];
    sound.player=sound.radar=false;
    VectorClear(sound.origin);
  }
}

int cSoundMark::FindClose(vec3_t origin) {
  int minindex=-1;
  float mindist=999999.0;
  for(int i=0; i<MAX_VPLAYERS; i++)
    if(i!=gMe.index&&gPlayers[i]->IsValidEnt()) {
      float dist=gPlayers[i]->GetDistanceFrom(origin);
      if(dist<mindist&&dist<=gMe.pmMaxSpeed) {
        mindist=dist;
        minindex=i;
      }
    }
  return minindex;
}

int cSoundMark::GetIndex(int index,vec3_t origin,bool bfindindex) {
  static int smcindex=MAX_VPLAYERS;
  if(index>0&&index<MAX_VPLAYERS) {
    if(!gPlayers[index]->IsValidEnt())
      VectorCopy(origin,gPlayers[index]->lastpos);
    return index;
  }

  if(bfindindex) {
    int minindex=cSoundMark::FindClose(origin);
    if(minindex!=-1) {
      if(!gPlayers[minindex]->IsValidEnt())
        VectorCopy(origin,gPlayers[minindex]->lastpos);
      return minindex;
    }
  }

  if(++smcindex==3*MAX_VPLAYERS)
    smcindex=MAX_VPLAYERS;
  return smcindex;
}

void cSoundMark::Draw(cSoundMark &mark) {
  vec3_t vecScreen;
  ColorEntry &color=gColorList.get(mark.color);
  if(cvar.radar)
    gRadar.DrawPoint(mark.origin,NULL,color.r,color.g,color.b,255,3);

  if(!CalcScreen(mark.origin,vecScreen))
    return;

  float distance=gMe.GetDistanceFrom(mark.origin)/22.0f;
  if(distance<1)distance=1;
  int boxradius=(int)((300.0*90.0)/(distance*gMe.iFOV));
  BOUND_VALUE(boxradius,1,200);
  if(cvar.box==1)
    DrawBox((int)vecScreen[0],(int)vecScreen[1],boxradius,2,color.r,color.g,color.b,color.a);
  else if(cvar.box==2)
    DrawBox((int)vecScreen[0],(int)vecScreen[1],10,2,color.r,color.g,color.b,color.a);

  int ystep=-14;
  int x=(int)vecScreen[0];
  int y=(int)vecScreen[1]+14;
  if(gMe.alive)y=y-(int)boxradius-1;

  if(mark.player) {
    if(cvar.weapon) {
      DrawString(true,x,y,color.r,color.g,color.b,"-%s-",mark.weapon);
      y+=ystep;
    }
    if(cvar.name) {
      DrawString(true,x,y,color.r,color.g,color.b,"-%s-",mark.name);
      y+=ystep;
    }
  } else if(cvar.name) {
    DrawString(true,x,y,color.r,color.g,color.b,"-%s-",mark.desc);
    y+=ystep;
  }

  if(cvar.distance) {
    DrawString(true,x,y,color.r,color.g,color.b,"-%i-",(short int)distance);
  }
}

void cSoundMark::DrawSound() {
  if(cvar.soundesp&&!cvar.takingss) {
    for(int i=0; i<3*MAX_VPLAYERS; i++) {
      cSoundMark &sound=gSoundMark[i];
      if(sound.time>GetTickCount()||sound.radar)
        Draw(sound);
      else if(sound.player)
        Reset(i);
    }
  }
}
