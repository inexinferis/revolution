#include "screenshots.h"
#include "interpreter.h"
#include "weapons.h"
#include "autoroute.h"
#include "textures.h"
#include "nospread.h"
#include "console.h"
#include "drawing.h"
#include "players.h"
#include "client.h"
#include "opengl.h"
#include "aimbot.h"
#include "radar.h"
#include "main.h"
#include "cvar.h"
#include "menu.h"
#include "gui.h"
#include "FakeSteamId.h"

pfnUserMsgHook pResetHUD=NULL,pTeamInfo=NULL,pCurWeapon=NULL,pScoreAttrib=NULL,pSetFOV=NULL,
               pHealth=NULL,pBattery=NULL,pScoreInfo=NULL,pDeathMsg=NULL,pSayText=NULL,pDamage=NULL,//pTextMsg=NULL,
               pAmmoX=NULL,pMoney=NULL,pWeaponList=NULL,pRadar=NULL,pFog=NULL,pBombDrop=NULL,pBombPickup=NULL,
               pHostagePos=NULL,pHostageK=NULL,pServerName=NULL;
pfnClientCmd pConnect=NULL,pMotdWrite=NULL,pUnbindAll=NULL,pUnbind=NULL,pTimeRefresh=NULL,pWriteCfg=NULL,
             pQuit=NULL,pExit=NULL,pDropClient=NULL,pKill=NULL;
tPreS_Sound pPreS_DynamicSound=NULL,pPreS_StaticSound=NULL;
tStudioRenderModel pStudioRenderModel=NULL;
//clientcmdlist_t** pClientCmdList=NULL;
eventlist_t** pEventList=NULL;
usermsglist_t** pUserMsgList=NULL;
srvcmdlist_t** pSrvCmdList=NULL;

float gSpeed=0.0;
double serverTime=0.0;
bool bFlipFlop=true,bMidMenu=true,bShouldShoot=false,bShouldWait=false,bFirtsShoot=true,bFalling=false;
bool bzoomdisabled=false,bEngRadar=-1;
UINT gzoom=1,iLastFOV=90,display=0,drawingCam=0,bClean=0,iNeedBuy=0;

int (*pStudioDrawPlayer)(int flags,entity_state_s *pplayer);
int (*pStudioDrawModel)(int flags);
int (*pStudioCheckBBox)(void);
void (*pStudioEntityLight)(struct alight_s *plight);

int HUD_Redraw(float x,int y) {
  static bool bOnce=false;
  if(!bOnce) {
    InitVisuals();
    bOnce=true;
  }

  if(cvar.takingss)
    bClean++;

  //other options
  if(gMe.alive&&cvar.active&&!cvar.takingss) {
    if(cvar.czoom&&gzoom!=1&&bzoomdisabled) {
      gMe.HandleZoom(gzoom);
      bzoomdisabled=false;
    }
    if(cvar.chasecam&&chase_active&&r_drawviewmodel) {
      chase_active->value=1.0;
      r_drawviewmodel->value=0.0;
    }
  } else {
    if(cvar.czoom&&gzoom!=1&&!bzoomdisabled) {
      gMe.HandleZoom(1);
      bzoomdisabled=true;
    }
    if(cvar.chasecam&&chase_active&&r_drawviewmodel) {
      chase_active->value=0.0;
      r_drawviewmodel->value=1.0;
    }
  }

  //Call Client!
  int ret=gExport.HUD_Redraw(x,y);
  cmd.FrameEvent();

  //save picture (take 3 frames)
  if(cvar.takingss&&bClean>2) {
    if(!bInvalidVideoMode)
      gScreenShots.TakeScreenShot();
    else
      gScreenShots.TakeScreenShot2();
    cvar.takingss=FALSE;
    bClean=0;
  }

  //do esps
  if(cvar.active) {
    if(!cvar.takingss&&(!cvar.antiss||!gMe.IsFreeSpectator())) { //cvar.antiss!=2|| <- bug with sprites!
      if(display<2000) { //
        DrawHudString(true,displayCenterX,screeninfo.iHeight/6,235,138,display%255,
          "Inexinferis Revolution 2017 - http://www.inexinferis.com");
        display++;
      }
      if(!cvar.speedmode||(cvar.speedmode&&!gMe.inspeed)) {
        if(cvar.showmsgs)
          gGui.DrawMessages();
        if(gMenu.active)
          gMenu.Draw();
        if(gConsole.active)
          gConsole.Draw();
        if(cvar.showtime) {
          gGui.DrawTime();
          gBomb.DrawTime();
        }
        if(cvar.showspeed)
          gMe.DrawSpeed();
        if(cvar.infobox&&!cvar.speedmode)
          gGui.DrawInfoBox();
        if(cvar.netstatus&&!cvar.speedmode)
          gGui.DrawNetStatusBox();
        if(cvar.antissshow&&!cvar.speedmode&&gScreenShots.bScreenTook)
          gGui.DrawScreenShot();
        else if(cvar.spycam&&!cvar.speedmode&&!cvar.chasecam)
          gGui.DrawSpyCam();
        if(cvar.crosshair)
          DrawCrosshair();
        if(cvar.radar)
          gRadar.DrawRadar();
        if(bEngRadar!=cvar.radar) {
          if(cvar.radar)
            gEngfuncs.pfnClientCmd("hideradar");
          else
            gEngfuncs.pfnClientCmd("drawradar");
          bEngRadar=cvar.radar;
        }
        if(!cvar.antiss&&cvar.speedmode&&cvar.autoroute&&(cvar.ardraw==1||(cvar.ardraw==2&&cvar.barrel)))
          gAutoRoute.DrawLines(true);
        cSoundMark::DrawSound();
        gPlayers.DrawEsp();
        gEspEnts.DrawEsp();
        gBomb.DrawPosition();
        gHostages.DrawPositions();
      }
      if(cvar.noflash)
        gMe.CheckIsFlashed();
      if(cvar.autoaim||cvar.aimkey||cvar.triggerbot) {
        if(cvar.avdraw||cvar.drawfov)
          gAimbot.DrawAimSpot();
      }
    }
    if(cvar.autoaim||cvar.aimkey||cvar.triggerbot)
      gAimbot.FindTarget();
    if(cvar.autobuy&&iNeedBuy) {
      if(iNeedBuy>100) {
        gMe.DoAutoBuy();
        iNeedBuy=0;
      } else
        iNeedBuy++;
    }
  }
  return ret;
}

void HUD_PostRunCmd(struct local_state_s *from,struct local_state_s *to,struct usercmd_s *cmd,int runfuncs,double time,unsigned int random_seed) {
  gExport.HUD_PostRunCmd(from,to,cmd,runfuncs,time,random_seed);
  if(cvar.active&&gMe.alive&&runfuncs) {
    gNoSpread.HUD_PostRunCmd(from,to,cmd,runfuncs,time,random_seed);
    gMe.m_flNextPrimaryAttack=to->weapondata[to->client.m_iId].m_flNextPrimaryAttack;
  }
  serverTime=time;
}

void V_CalcRefdef(struct ref_params_s *pparams) {
  //Pre V_CalcRefdef
  if(cvar.active&&pparams->nextView==0) {
    if(gMe.alive) {
      //for recoil removal...
      VectorCopy(pparams->punchangle,gMe.punchangle);
      //new world to screen
      VectorCopy(pparams->viewangles,gMe.viewangles);
      VectorCopy(pparams->vieworg,gMe.vieworg);
      //Visual Recoil Removal
      if(cvar.norecoil)
        VectorClear(pparams->punchangle);
    } else {
      // avoid problems with free chase cam view
      VectorCopy(pparams->vieworg,gMe.pmEyePos);
    }
  }
  //Call Client!
  gExport.V_CalcRefdef(pparams);
  //SpyCam
  drawingCam=pparams->nextView;
  //Post V_CalcRefdef
  if(cvar.active&&gMe.alive) {
    //view state
    switch(pparams->nextView) {
      case 0:
        gMe.Update();
        //new world to screen / Radar Spin Fix
        VectorCopy(pparams->viewangles,gMe.viewangles);
        VectorCopy(pparams->vieworg,gMe.vieworg);
        //quake guns
        if(!cvar.speedmode&&!cvar.takingss&&cvar.quakeguns) {
          cl_entity_s* vm=gEngfuncs.GetViewModel();
          if(vm&&vm->model&&vm->model->name) {
            float gunoffsetr=gWeapon.GetWeaponViewOffset(vm->model->name+9);
            if(gunoffsetr) {
              gunoffsetr=-((cvar.quakeguns==1)?gunoffsetr:gunoffsetr-2.5f);
              vm->origin[0]+=pparams->forward[0]+pparams->up[0]+pparams->right[0]*gunoffsetr;
              vm->origin[1]+=pparams->forward[1]+pparams->up[1]+pparams->right[1]*gunoffsetr;
              vm->origin[2]+=pparams->forward[2]+pparams->up[2]+pparams->right[2]*gunoffsetr;
              VectorCopy(pparams->viewangles,vm->angles);
            }
          }
        }
        //SpyCam
        if(cvar.active&&!cvar.takingss&&cvar.spycam&&gMe.ent->curstate.solid&&
            !cvar.speedmode&&!cvar.chasecam) { //chasecam fucks world to screen...
          pparams->nextView=1;
          pparams->onlyClientDraw=0;
        }
        break;
      case 1:
        pparams->onlyClientDraw=0;
        pparams->viewport[0]=screeninfo.iWidth-(screeninfo.iWidth/4+10)+1;//camera x;
        pparams->viewport[1]=10+fontHeight+1;//camera y
        pparams->viewport[2]=screeninfo.iWidth/4;//camera w;
        pparams->viewport[3]=screeninfo.iHeight/4;//camera h;
        if(cvar.spycam==2)pparams->viewangles[1]-=180;//mirror
        pparams->nextView=2;
        break;
      default://pparams->nextView==2
        pparams->nextView=0;
        pparams->onlyClientDraw=1;
        break;
    }
  }
}

void HUD_PlayerMoveInit(struct playermove_s *pmove) {
  InitTextureTypes(pmove);
  //Call Client!
  gExport.HUD_PlayerMoveInit(pmove);
  //init data...
  gMe.PlayerMove(pmove);
}

void HUD_PlayerMove(struct playermove_s *pmove,int server) {
  //Call Client!
  gExport.HUD_PlayerMove(pmove,server);
  //init data...
  gMe.PlayerMove(pmove);
  //kz Hack
  if(cvar.active) {
    //Auto Jump
    if(pmove->velocity[0]||pmove->velocity[1]) {
      if(cvar.autojump&&(pmove->flags&FL_ONGROUND)&&!(pmove->flags&FL_DUCKING)) {
        vec3_t vecBegin,vecEnd,velocity;
        pmtrace_t *tr;
        float fraction=1.0f;
        //get velocity vector...
        velocity=pmove->velocity;
        VectorNormalize(velocity);
        velocity[2]=0;
        //jump boxes, etc...
        VectorCopy(gMe.pmEyePos,vecBegin);
        vecBegin[2]-=30;
        vecEnd=vecBegin+velocity*70;
        //DrawLine(vecBegin,vecEnd,255,0,0,0.01f);
        tr=gEngfuncs.PM_TraceLine(vecBegin,vecEnd,PM_TRACELINE_PHYSENTSONLY,2,-1);
        if(tr->fraction!=1.0) {
          fraction=tr->fraction;
          VectorCopy(pmove->origin,vecBegin);
          vecEnd=vecBegin+velocity*70;
          //DrawLine(vecBegin,vecEnd,0,255,0,0.01f);
          tr=gEngfuncs.PM_TraceLine(vecBegin,vecEnd,PM_TRACELINE_PHYSENTSONLY,2,-1);
          if(tr->fraction==1.0f) {
            VectorCopy(gMe.pmEyePos,vecBegin);
            vecBegin[2]-=45;
            vecEnd=vecBegin+velocity*70;
            //DrawLine(vecBegin,vecEnd,0,0,255,0.01f);
            tr=gEngfuncs.PM_TraceLine(vecBegin,vecEnd,PM_TRACELINE_PHYSENTSONLY,2,-1);
            if(fraction==tr->fraction)
              gMe.bShouldJump=true;
          }
        }
        //jump holes...
        VectorCopy(pmove->origin,vecBegin);
        VectorCopy(pmove->origin,vecEnd);
        vecEnd[2]-=65.0f;//42.0f;
        vecEnd=vecEnd-velocity*13;
        //DrawLine(vecBegin,vecEnd,0,255,0,0.01f);
        tr=gEngfuncs.PM_TraceLine(vecBegin,vecEnd,PM_TRACELINE_PHYSENTSONLY,2,-1);
        if(tr->fraction==1.0) {
          vecEnd=vecEnd-velocity*13;
          //DrawLine(vecBegin,vecEnd,255,0,0,0.01f);
          tr=gEngfuncs.PM_TraceLine(vecBegin,vecEnd,PM_TRACELINE_PHYSENTSONLY,2,-1);
          if(tr->fraction!=1.0f)
            gMe.bShouldJump=true;
          //gMe.bNeedDuck=true;
        }
      }
      //kz hack
      if(cvar.autoduck&&!pmove->bInDuck&&!(pmove->flags&(FL_ONGROUND|FL_INWATER|FL_DUCKING))) {
        vec3_t vecBegin,vecEnd,nextpoint,velocity;
        VectorCopy(pmove->origin,vecBegin);
        VectorCopy(pmove->origin,vecEnd);
        vecEnd[2]-=36.0f;
        //from sing-engine code!
        velocity=pmove->velocity;
        nextpoint=velocity+pmove->basevelocity;
        nextpoint[2]-=((pmove->gravity)?pmove->gravity:1.0f)*pmove->movevars->gravity*pmove->frametime*0.5;
        vecEnd=vecEnd+nextpoint*pmove->frametime;
        VectorNormalize(nextpoint);
        vecEnd[0]+=nextpoint[0]*32.0;
        vecEnd[1]+=nextpoint[1]*32.0;
        //DrawLine(vecBegin,vecEnd,255,0,0,0.01f);
        if(gEngfuncs.PM_PointContents(vecEnd,NULL)==CONTENTS_SOLID) {
          vecEnd[2]+=18.0f;
          //DrawLine(vecBegin,vecEnd,255,0,0,0.01f);
          if(gEngfuncs.PM_PointContents(vecEnd,NULL)!=CONTENTS_SOLID)
            gMe.bNeedDuck=true;
        }
      }
    }
    if(cvar.autostandup&&!(pmove->flags&(FL_ONGROUND|FL_INWATER))) {
      vec3_t vecEnd;
      VectorCopy(pmove->origin,vecEnd);
      if(pmove->flags&FL_DUCKING) {
        vecEnd[2]-=36.0f;
        //DrawLine(vecBegin,vecEnd,255,0,0,0.01f);
        if(gEngfuncs.PM_PointContents(vecEnd,NULL)==CONTENTS_SOLID)
          gMe.bNoNeedDuck=true;
      } else {
        vecEnd[2]-=42.0f;
        //DrawLine(vecBegin,vecEnd,255,0,0,0.01f);
        if(gEngfuncs.PM_PointContents(vecEnd,NULL)==CONTENTS_SOLID)
          gMe.bNeedDuck=true;
      }
    }
  }
}

void CL_CreateMove(float frametime,struct usercmd_s *cmd,int active) {
  //Call Client!
  gExport.CL_CreateMove(frametime,cmd,active);
  gMe.fps=1.0/frametime;
  gMe.inspeed=FALSE;
  //Post CL_CreateMove
  if(cvar.active&&gMe.alive) {
    gMe.Update();
    //copy for spinhack
    memcpy(&gAimbot.usercmd,cmd,sizeof(usercmd_t));
    bShouldShoot=FALSE;
    bShouldWait=FALSE;
    BOOL meInWay=cvar.speedmode&&cvar.autoroute&&gMe.inway;
    // aim
    if(gAimbot.HasTarget()&&!gWeapon.IsNonAttack()&&(gMe.iClip||gWeapon.IsKnife())) {
      gAimbot.CalculateAimingView();
      // auto aim
      if(cvar.autoaim||gMe.aimkey||meInWay) {
        //knife aim...
        if(gWeapon.IsKnife()) {
          if(gMe.aimkey||(gAimbot.tDistance<120)) {
            VectorCopy(gAimbot.aim_viewangles,cmd->viewangles);
            gEngfuncs.SetViewAngles(cmd->viewangles);
            if(gAimbot.tDistance<54)
              bShouldShoot=TRUE;
          }
        } else {
          //smooth aim...
          if(cvar.smoothaim) {
            if(gAimbot.SmoothAim(cmd->viewangles))
              bShouldShoot=TRUE;
            else
              bShouldWait=TRUE;
            gEngfuncs.SetViewAngles(cmd->viewangles);
          } else {
            //normal aim...
            if(!cvar.silentaim) {
              VectorCopy(gAimbot.aim_viewangles,cmd->viewangles);
              gEngfuncs.SetViewAngles(cmd->viewangles);
            } else
              gAimbot.ApplySilentAngles(gAimbot.aim_viewangles,cmd);
            bShouldShoot=TRUE;
          }
        }
      }
      //trigger bot...
      if(!bShouldShoot&&(cvar.triggerbot==1||gMe.trigger)&&!gWeapon.IsKnife())
        bShouldShoot=gAimbot.ShouldShoot(cmd);//triggerbot
    }

    //aim shoot flag
    if(bShouldShoot) {
      // auto shoot (includes triggetbot)
      if(cvar.autoshoot||cvar.speedmode) {
        if(gWeapon.IsKnife())
          gAimbot.AutoKnife(cmd);
        else if(bFirtsShoot&&((cvar.autoshoot==2&&VectorEmpty(gMe.punchangle))||
                              (cvar.autoshoot!=2&&(gMe.m_flNextPrimaryAttack-cmd->msec/1000)<=0.0f))) {
          cmd->buttons|=IN_ATTACK;
          //don't move if autoshot is enabled...
          cmd->forwardmove=0;
          cmd->sidemove=0;
        }
      } else if(gMe.aimkey||gMe.trigger)
        cmd->buttons|=IN_ATTACK;
    } else {
      if(gMe.aimkey&&!bShouldWait)
        cmd->buttons|=IN_ATTACK;
    }
    //first shoot ignored...
    bFirtsShoot=bShouldShoot;

    //LTFX/WAV Speed
    float spd=cvar.speed;
    //Handle Knive Speed
    if(meInWay||(cvar.knivespeed&&gWeapon.IsKnife())) {
      //automatic knife mode...
      if(meInWay||cmd->buttons&IN_ATTACK||cmd->buttons&IN_ATTACK2) {
        spd=KNIFESPEED;
        // auto move
        if(gAimbot.HasTarget()&&gWeapon.IsKnife()&&(cvar.autoaim||gMe.aimkey||
            (meInWay&&gAimbot.tDistance<2*gMe.pmMaxSpeed)))
          gAimbot.CreateMove(cmd);
      }
    } else { //Handle ASpeed/Weapon speed
      if(cvar.speedkey&&((cvar.speedkey==1&&(cmd->buttons&IN_ATTACK||bShouldShoot))||
                         (cvar.speedkey==2&&cmd->buttons&IN_ATTACK2))&&!gWeapon.IsNade()&&!gWeapon.IsC4()) {
        if(cvar.speedmode)
          spd=gWeapon.GetSpeed();
        else
          spd=cvar.aspeed;
      }
    }

    //no fall damage
    /*if(cvar.nofalldamage&&fpsTimeMultiplier) {
      double spd2=1.0;
      if(gMe.pmFallVelocity>500.0)
        bFalling=true;
      if(bFalling) {
        float distance=gMe.GroundDistance();
        float playertime=(gMe.fps > 100.0 ? 100.0 : gMe.fps)/2.0;
        if((gMe.pmFallVelocity/playertime)>=distance) {
          if(distance>2.0) {
            spd2=0.001*gMe.fps+0.001;
            //spd=(0.01*gMe.fps+0.01)-10;
            if(gMe.bInDuck&&!(cmd->buttons&IN_JUMP)) {
              if(gMe.pmFlags&FL_ONGROUND) {
                spd2=1.0;
              }
            } else {
              cmd->buttons|=IN_DUCK;
              cmd->buttons&=~IN_JUMP;
            }
          } else {
            cmd->buttons|=IN_JUMP;
            cmd->buttons&=~IN_DUCK;
            bFalling=false;
          }
        }
        *fpsTimeMultiplier=spd2*1000.0;
      }
    }*/

    // auto-reload
    if(cvar.autoreload&&!gMe.iClip&&!gWeapon.IsKnife()) {
      cmd->buttons&=~IN_ATTACK;
      if(cvar.reloadspeed&&gMe.pmFlags&FL_ONGROUND) {
        //full speed, don't move!
        cmd->forwardmove=0;
        cmd->sidemove=0;
        spd=KNIFESPEED;
      }
    }

    //do speed
    if(globalTime){
      if(cvar.speedfix&&!gMe.ShouldSpeed())
        spd=-spd/5;
      *globalTime+=frametime*spd/5;
    }

    //flag to disable visuals in speed
    gMe.inspeed=(spd!=cvar.speed);

    //rapid fire!
    if(cvar.rapidfire&&cmd->buttons&IN_ATTACK&&gWeapon.IsSecondary()) {
      if(bFlipFlop)
        cmd->buttons|=IN_ATTACK;
      else
        cmd->buttons&=~IN_ATTACK;
      bFlipFlop=!bFlipFlop;
    }

    if(cmd->buttons&IN_ATTACK&&!gWeapon.IsNonAttack()) {
      // no spread
      if(cvar.nospread||cvar.speedmode)
        gNoSpread.GetSpreadOffset(cmd->viewangles);
      // no recoil
      if(cvar.norecoil||cvar.speedmode)
        gNoSpread.GetRecoilOffset(cmd->viewangles);
    } else// spinhack
      if(cvar.spinhack)
        gAimbot.FixupAngleDifference(cmd);

    // bunny hop
    if(cvar.bunnyhop)
      gMe.DoBunnyHop(cmd);

    //duck hop
    if(cvar.duckhop)
      gMe.DoDuckHop(cmd);

    // duck jump
    if(cvar.duckjump)
      gMe.DoDuckJump(cmd);

    // auto jump
    if(cvar.autojump&&gMe.bShouldJump) {
      gMe.bShouldJump=false;
      cmd->buttons|=IN_JUMP;
    }

    // auto duck
    if(cvar.autoduck) {
      if(gMe.bNeedDuck) {
        gMe.bNeedDuck=false;
        cmd->buttons|=IN_DUCK;
      }

      if(gMe.bNoNeedDuck) {
        gMe.bNoNeedDuck=false;
        cmd->buttons&=~IN_DUCK;
      }
    }

    // auto strafe
    if(cvar.autostrafe&&!(cmd->buttons&IN_ATTACK)&&!(gMe.pmFlags&(FL_ONGROUND|FL_INWATER))&&gMe.pmGroundSpeed) {
      static bool bStrafing=true;//false;
      float strafe_speed=0;
      if(gMe.pmAirAccelerate==10) {
        if(frametime>0.006666666666666)
          strafe_speed=4.02f; //<150fps
        else if(frametime<=0.006666666666666&&frametime>0.004)
          strafe_speed=5.651f; //>150fps && <250fps
        else if(frametime<=0.004&&frametime>0.002857142857142)
          strafe_speed=6.1135f; //>250fps && <350fps
        else if(frametime<=0.002857142857142&&frametime>0.002222222222222)
          strafe_speed=6.2f; //>350fps && <450fps
        else if(frametime<=0.002222222222222)
          strafe_speed=6.399f; // >450fps
      } else if(gMe.pmAirAccelerate==100)
        strafe_speed=3.435f;
      else
        strafe_speed=3.921;
      if(bStrafing) {
        cmd->sidemove=-gMe.pmMaxSpeed;
        cmd->viewangles[1]-=strafe_speed*gMe.pmMaxSpeed/gMe.pmGroundSpeed;
      } else {
        cmd->sidemove=+gMe.pmMaxSpeed;
        cmd->viewangles[1]+=strafe_speed*gMe.pmMaxSpeed/gMe.pmGroundSpeed;
      }
      bStrafing=!bStrafing;
      cmd->forwardmove=0;
    }

    // Fast run
    if(cvar.fastrun&&(gMe.pmFlags&FL_ONGROUND)) {
      static bool FastRun=false;
      if((cmd->buttons&IN_FORWARD&&cmd->buttons&IN_MOVELEFT)||(cmd->buttons&IN_BACK&&cmd->buttons&IN_MOVERIGHT)) {
        if(FastRun) {
          cmd->sidemove-=89.6f;
          cmd->forwardmove-=89.6f;
        } else {
          cmd->sidemove+=89.6f;
          cmd->forwardmove+=89.6f;
        }
        FastRun=!FastRun;
      } else if((cmd->buttons&IN_FORWARD&&cmd->buttons&IN_MOVERIGHT)||(cmd->buttons&IN_BACK&&cmd->buttons&IN_MOVELEFT)) {
        if(FastRun) {
          cmd->sidemove-=89.6f;
          cmd->forwardmove+=89.6f;
        } else {
          cmd->sidemove+=89.6f;
          cmd->forwardmove-=89.6f;
        }
        FastRun=!FastRun;
      } else if(cmd->buttons&IN_FORWARD||cmd->buttons&IN_BACK) {
        if(FastRun)
          cmd->sidemove-=126.6f;
        else
          cmd->sidemove+=126.6f;
        FastRun=!FastRun;
      } else if(cmd->buttons&IN_MOVELEFT||cmd->buttons&IN_MOVERIGHT) {
        if(FastRun)
          cmd->forwardmove-=126.6f;
        else
          cmd->forwardmove+=126.6f;
        FastRun=!FastRun;
      }
    }

  }
  //AutoRoute!
  if(cvar.autoroute&&cvar.speedmode)
    gAutoRoute.FindLine(cmd);
}

int HUD_AddEntity(int type, struct cl_entity_s *ent, const char *modelname) {
  static float last_timer=0;
  //Call Client!
  int ret=gExport.HUD_AddEntity(type,ent,modelname);
  if(cvar.active) {
    if(!ent||!modelname||(type!=ET_NORMAL&&type!=ET_PLAYER))
      return ret;
    if(gPlayers.IsValidEnt(ent)||gMe.index==ent->index) {
      if(gMe.alive&&gMe.index!=ent->index) {
        gEngfuncs.CL_CreateVisibleEntity(ET_PLAYER,ent);
        ret=0;
      }
      gPlayers.AddEntity(ent);
    }
    if(gMe.ent->curstate.iuser1==OBS_IN_EYE&&gMe.ent->curstate.iuser2==ent->index)
      ret=0;
    if(cvar.glowents&&!cvar.takingss&&(!cvar.antiss||!gMe.IsFreeSpectator()))
      gEspEnts.AddEntity(ent,modelname);
    if(gMe.alive&&cvar.autoname&&(((GetTickCount()/1000.0)-last_timer)>(cvar.autonametime*10))) {
      gMenu.AutoName();
      last_timer=GetTickCount()/1000.0;
    }
    if(!cvar.antiss&&cvar.speedmode&&cvar.autoroute&&cvar.ardraw==2&&!cvar.barrel)
      gAutoRoute.DrawLines(false);
  }
  return ret;
}

int HUD_Key_Event(int eventcode,int keynum,const char *pszCurrentBinding) {
  //Call Client!
  int ret=gExport.HUD_Key_Event(eventcode,keynum,pszCurrentBinding);
  //DbgPrint("Key_Event: %d - eventcode: %d - bind: %s -> ret %d",keynum,eventcode,pszCurrentBinding,ret);

  if(eventcode&&cvar.active) {
    //Menu
    if(gMenu.active)
      ret=gMenu.Key(keynum);
    //Console
    if(gConsole.active)
      ret=gConsole.Key(keynum);
  }

  if(cvar.active&&gMe.alive&&!gMenu.active&&!gConsole.active) {
    //Handle HLH ZOOM
    if(cvar.czoom) {
      if(keynum==K_UPARROW||keynum==K_MWHEELUP) { //uparrow || mwheelup
        if(gzoom<9)
          gzoom++;
        else
          gzoom=9;
        gMe.HandleZoom(gzoom);
        ret=0;
      } else if(keynum==K_DOWNARROW||keynum==K_MWHEELDOWN) { //downarrow || mwheeldown
        if(gzoom>1)
          gzoom--;
        else
          gzoom=1;
        gMe.HandleZoom(gzoom);
        ret=0;
      }
    }

    //Handle HLH AIMING
    if(cvar.aimkey&&keynum==(K_MWHEELUP+cvar.aimkey)) { //MOUSE 1/2/3
      if(gAimbot.HasTarget()&&!gWeapon.IsNonAttack()&&(gWeapon.GetAmmo()||gWeapon.IsKnife())) {
        gMe.aimkey=eventcode;
        ret=!eventcode;//not catched by engine
      } else
        gMe.aimkey=false;
    }
  } else
    gMe.aimkey=false;

  //bind manager... (not catched!)
  //ret 1 handle keyup
  //ret 0 not handle keyup
  //eventcode 1 keydown
  //eventcode 0 keyup
  if(ret)
    ret=cmd.KeyEvent(keynum,(eventcode==1));
  return ret;
}

int HUD_UpdateClientData(client_data_t *cdata, float flTime) {
  //Call Client!
  int ret=gExport.HUD_UpdateClientData(cdata,flTime);
  gPlayers.Update();
  gWeapon.Update(cdata->iWeaponBits);
  //speed optimization
  if(cl_showfps) {
    if(cvar.speedmode) {
      cl_showfps->value=1.0;
      if(hud_draw&&hud_saytext_time&&cl_timeout) {
        if(gMe.inspeed) {
          hud_draw->value=0.0;
          hud_saytext_time->value=0.0;
          cl_timeout->value=999999999999999.0;
        } else {
          hud_draw->value=1.0;
          hud_saytext_time->value=5.0;
          cl_timeout->value=99999999.0;
        }
      }
    } else
      cl_showfps->value=0.0;
  }
  return ret;
}

//========================================================================================

inline void cleancpy(char* dest,const char* pos,bool ignorenums=false) {
  while(*pos) {
    if(ignorenums&&*pos>='0'&&*pos<='9') {
      ++pos;
      continue;
    }
    if(*pos=='.'||*pos=='-'||*pos=='_') {
      *dest=0;
      break;
    }
    *dest++=*pos++;
  }
}

void SoundFilter(int index,const char *sample,float *origin) {
  static bool bignorestep=false;
  if(cvar.soundesp) {
    bool bWeapon=(sample&&sample[0]=='w'&&sample[1]=='e'&&sample[7]=='/');//"weapons/"
    bool bPlayer=(sample&&sample[0]=='p'&&sample[3]=='y'&&sample[6]=='/');//"player/"
    bool bPlats=(sample&&sample[0]=='p'&&sample[3]=='t'&&sample[5]=='/');//"plats/"
    bool bHostage=(sample&&sample[0]=='h'&&sample[1]=='o'&&sample[2]=='s');//"hostage/"
    //radar not has sample!
    if(!sample||bWeapon||bPlayer) {
      //ignore bullets hits...
      if(cvar.soundesp==2) {
        if(bWeapon) {
          if((sample[8]=='r'&&sample[9]=='i')||(sample[8]=='s'&&sample[9]=='s')||(sample[8]=='b'))// ricxxx // sshellx  // bullet_hitx // boltxxx
            return;
          bignorestep=true;
        }
        if(bPlayer) {
          if(sample[7]=='p'&&sample[9]=='_') { //pl_
            if(!strncmp(sample+10,"shell",5)) {
              bignorestep=false;
              return;
            }
            if(bignorestep&&!strncmp(sample+10,"step",4)) {
              bignorestep=(cSoundMark::FindClose(origin)==-1);
              if(bignorestep)
                return;
            }
          }
        }
      }
      gMe.Update();
      int smcindex=cSoundMark::GetIndex(index,origin,cvar.soundesp==2);
      //DbgPrint("sound %d sample %s is proccesed! index %d",index,sample,smcindex);
      if(smcindex==gMe.index)return;
      cSoundMark &mark=gSoundMark[smcindex];
      mark.player=false;

      if(smcindex>0&&smcindex<MAX_VPLAYERS) {
        //look health...
        if(cvar.health&&bPlayer) {
          int damage=0;
          if(strstr(sample,"headshot"))damage=4;
          if(strstr(sample,"helmet"))damage=3;
          if(strstr(sample,"flesh"))damage=2;
          if(strstr(sample,"kevlar"))damage=1;
          if(damage) {
            cPlayer* splayer=gPlayers[smcindex];
            splayer->hitpoints-=damage;
            if(splayer->hitpoints<=0)
              splayer->hitpoints=1;
          }
        }
        //try get ent... ( only if not was catched by DrawEsp! )
        cl_entity_s *ent=gEngfuncs.GetEntityByIndex(smcindex);
        if(ent&&ent->curstate.messagenum==gMe.ent->curstate.messagenum)
          return;

        //get player
        cPlayer* splayer=gPlayers[smcindex];
        //have change his weapon?
        if(bWeapon) {
          char* pos=(char*)sample+8;
          int len=strlen(pos);
          if(len<30) {
            char tmp[32];
            strcpy(tmp,pos);
            tmp[len-4]=0;
            if(!strcmp(tmp,"zoom"))
              strcpy(tmp,"awp");
            else {
              pos=strchr(tmp,'_');
              if(pos)*pos=0;
              else {
                pos=strchr(tmp,'-');
                if(pos)*pos=0;
              }
            }
            splayer->SetWeapon(tmp);
          }
        }
        //update player data
        splayer->bGotHead=false;
        gAimbot.CalcFovAndVisibility(*splayer);
        //update mark data...
        mark.color=splayer->team;
        mark.name=splayer->entinfo.name;
        mark.weapon=splayer->m_weapon;
        mark.player=true;
        if(!mark.radar)
          mark.radar=(sample==NULL);
      } else {
        if(!sample)
          return;
        if(bPlayer) {
          if(cvar.soundesp!=2)
            return;
          const char* pos=sample+7;
          if(pos[0]=='p'&&pos[2]=='_')
            pos+=3;
          else if(pos[0]=='b'&&pos[1]=='h'&& pos[2]=='i')//"bhit_..."
            pos+=5;
          gSoundMark[smcindex].color=0;
          cleancpy(gSoundMark[smcindex].desc,pos,true);
        }
        if(bWeapon) {
          char* pos=(char*)sample+8;
          if(pos[0]=='c'&&pos[1]=='4') {
            gSoundMark[smcindex].color=5;
            strcpy(gSoundMark[smcindex].desc,"C4");
          } else {
            if(cvar.soundesp!=2)
              return;
            gSoundMark[smcindex].color=0;
            cleancpy(gSoundMark[smcindex].desc,pos);
          }
        }
      }
      VectorCopy(origin,mark.origin);
      mark.time=GetTickCount()+300;
    } else if(cvar.soundesp==2&&(bPlats||bHostage)) {
      int smcindex=cSoundMark::GetIndex(index,origin,false);
      cSoundMark &mark=gSoundMark[smcindex];
      mark.player=(smcindex>0&&smcindex<MAX_VPLAYERS);
      mark.weapon="Unknown";
      if(bPlats) {
        strcpy(mark.desc,"Vehicle");
        mark.color=4;
      }
      if(bHostage) {
        strcpy(mark.desc,"Hostage");
        mark.color=5;
      }
      VectorCopy(origin,mark.origin);
      mark.time=GetTickCount()+300;
    }
  } else if(cvar.health) {
    if(index>0&&index<MAX_VPLAYERS) {
      cPlayer* splayer=gPlayers[index];
      if(sample[0]=='p'&&sample[3]=='y'&&sample[6]=='/') { //"player/"
        int damage=0;
        if(strstr(sample,"headshot"))damage=4;
        if(strstr(sample,"helmet"))damage=3;
        if(strstr(sample,"flesh"))damage=2;
        if(strstr(sample,"kevlar"))damage=1;
        if(damage) {
          splayer->hitpoints-=damage;
          if(splayer->hitpoints<=0)
            splayer->hitpoints=1;
        }
      }
    }
  }
}

void hPreS_StaticSound(int index,int channel,char *sample,float *origin,float volume,float attenuation,int fFlags,int pitch) {
  //filter sounds...
  if(cvar.active)
    SoundFilter(index,sample,origin);
  //Call Client!
  pPreS_StaticSound(index,channel,sample,origin,volume,attenuation,fFlags,pitch);
}

void hPreS_DynamicSound(int index,int channel,char *sample,float *origin,float volume,float attenuation,int fFlags,int pitch) {
  //filter sounds...
  if(cvar.active)
    SoundFilter(index,sample,origin);
  //Call Client!
  pPreS_DynamicSound(index,channel,sample,origin,volume,attenuation,fFlags,pitch);
}

//==================================================================================

int hStudioCheckBBox(void) {
  int ret=pStudioCheckBBox();
  if(cvar.active&&!ret&&gAimbot.iTarget!=-1) {
    // need to check hitbox \ bone, even when we do not look at the enemy
    cl_entity_t *ent=gEngStudio.GetCurrentEntity();
    if(ent&&ent->index==gAimbot.iTarget)
      return 1;
  }
  return ret;
}

void hStudioEntityLight(struct alight_s *plight){
  if(cvar.active){
    cl_entity_t *ent=gEngStudio.GetCurrentEntity();
    if(ent){
      //get hitboxes for autoaim! (No FPS drops!)
      cPlayer *player=gPlayers[ent->index];
      gAimbot.CalculateHitbox(player);
    }
  }
  pStudioEntityLight(plight);
}

int hStudioDrawPlayer(int flags,entity_state_s *pplayer) {
  bool saved=false;int studioret;
  entity_state_t prevstate;
  cPlayer *player=gPlayers[pplayer->number];
  if(cvar.active&&flags&STUDIO_RENDER) {
    //no draw players in speed
    if(cvar.speedmode&&gMe.inspeed)
      return 0;
    if(!cvar.takingss&&(!cvar.antiss||!gMe.IsFreeSpectator())) {
      //perfect wallhack
      if(cvar.wallhack==1&&!cvar.chams) {
        if(pglDepthRange)
          pglDepthRange(0,0.5);
      }
      //player glow
      if(cvar.glowshells) {
        ColorEntry &color=player->Color();
        memcpy(&prevstate,&player->ent->curstate,sizeof(prevstate));
        player->ent->curstate.renderamt=1;
        player->ent->curstate.renderfx|=kRenderFxGlowShell;
        player->ent->curstate.rendercolor.r=color.r;
        player->ent->curstate.rendercolor.g=color.g;
        player->ent->curstate.rendercolor.b=color.b;
      }
      saved=true;
    }
  }
  //call client!
  studioret=pStudioDrawPlayer(flags,pplayer);
  //restore status
  if(saved) {
    //perfect wallhack
    if(cvar.wallhack==1&&!cvar.chams) {
      if(pglDepthRange)
        pglDepthRange(0,1);
    }
    //player glow
    if(cvar.glowshells)
      memcpy(&player->ent->curstate,&prevstate,sizeof(prevstate));
  }
  return studioret;
}

int	hStudioDrawModel(int flags) {
  bool saved=false;
  entity_state_t prevstate;
  cl_entity_s *ent=gEngfuncs.GetViewModel();
  if(cvar.active) {
    //no draw models in speed
    if(cvar.speedmode&&gMe.inspeed)
      return 0;
    //local glow and transhield
    if(!cvar.takingss&&ent&&ent->model&&(!cvar.antiss||!gMe.IsFreeSpectator())) {
      ColorEntry &color=gPlayers[ent->index]->Color();
      if(cvar.glowshells) {
        memcpy(&prevstate,&ent->curstate,sizeof(prevstate));
        ent->curstate.renderamt=1;
        ent->curstate.renderfx|=kRenderFxGlowShell;
        ent->curstate.rendercolor.r=color.r;
        ent->curstate.rendercolor.g=color.g;
        ent->curstate.rendercolor.b=color.b;
      }
      if(cvar.transhield&&strstr(ent->model->name,"shield")) {
        ent->curstate.rendermode=kRenderTransAdd;
        ent->curstate.renderamt=100;
      }
      saved=true;
    }
  }
  //Call Client!
  int studioret=pStudioDrawModel(flags);
  //restore status
  if(saved) {
    if(cvar.glowshells)
      memcpy(&ent->curstate,&prevstate,sizeof(prevstate));
    if(cvar.transhield&&strstr(ent->model->name,"shield")) {
      ent->curstate.rendermode=kRenderNormal;
      ent->curstate.renderamt=0;
    }
  }
  return studioret;
}

void hStudioRenderModel() {
  PVOID _this;//save this pointer!
  asm("mov %%ecx,%0;\r\t":"=m"(_this));
  if(!bInvalidVideoMode) {
    cPlayer *player=gPlayers.GetCurrentEntity();
    if(cvar.active&&!cvar.takingss&&(!cvar.antiss||!gMe.IsFreeSpectator())&&
      cvar.chams&&player->IsValidEnt()) {
      ColorEntry &color=player->Color();
      if(cvar.chams==1) {
        oglChams=true;

        if(pglDisable)
          pglDisable(GL_TEXTURE_2D);

        // render non-visible part
        if(pglDisable)
          pglDisable(GL_DEPTH_TEST);

        oglChamsColor[0]=fmin(color.fr+color.fb,1.0);
        oglChamsColor[1]=fmin(color.fg+color.fr,1.0);
        oglChamsColor[2]=fmin(color.fb+color.fg,1.0);

        asm("mov %0,%%ecx;\r\t"::"m"(_this));
        gRenderer.StudioRenderFinal();

        // render visible part
        if(pglEnable)
          pglEnable(GL_DEPTH_TEST);

        oglChamsColor[0]=color.fr;
        oglChamsColor[1]=color.fg;
        oglChamsColor[2]=color.fb;

        asm("mov %0,%%ecx;\r\t"::"m"(_this));
        gRenderer.StudioRenderFinal();

        if(pglEnable)
          pglEnable(GL_TEXTURE_2D);

        oglChams=false;
      } else {
        gEngStudio.SetChromeOrigin();
        gEngStudio.SetForceFaceFlags(STUDIO_NF_CHROME);
        player->ent->curstate.renderfx=kRenderFxGlowShell;
        player->ent->curstate.renderamt=1;
        player->ent->curstate.rendercolor.r=min(color.r+color.b,255);
        player->ent->curstate.rendercolor.g=min(color.g+color.r,255);
        player->ent->curstate.rendercolor.b=min(color.b+color.g,255);

        // render non-visible part
        if(pglDepthFunc)
          pglDepthFunc(GL_GREATER);

        asm("mov %0,%%ecx;\r\t"::"m"(_this));
        gRenderer.StudioRenderFinal();

        gEngStudio.SetChromeOrigin();
        gEngStudio.SetForceFaceFlags(STUDIO_NF_CHROME);
        player->ent->curstate.renderfx=kRenderFxGlowShell;
        player->ent->curstate.renderamt=1;
        player->ent->curstate.rendercolor.r=color.r;
        player->ent->curstate.rendercolor.g=color.g;
        player->ent->curstate.rendercolor.b=color.b;

        // render visible part
        if(pglDepthFunc)
          pglDepthFunc(GL_LESS);

        asm("mov %0,%%ecx;\r\t"::"m"(_this));
        gRenderer.StudioRenderFinal();
      }
      return;
    }
  }
  asm("mov %0,%%ecx;\r\t"::"m"(_this));
  pStudioRenderModel();
}

//==================================================================================

void func_active() {
  cvar.active=!cvar.active;
}

void func_showmenu() {
  if(cvar.active&&!gConsole.active) {
    gMenu.active=!gMenu.active;
    if(!gMenu.active)
      cvar.Save();
  }
}

void func_showconsole() {
  if(cvar.active&&!gMenu.active) {
    gConsole.active=!gConsole.active;
    if(!gConsole.active)
      cvar.Save();
  }
}

void func_customaim() {
  if(cvar.active&&gMe.alive&&!gMenu.active&&!gConsole.active&&!cvar.aimkey&&gAimbot.HasTarget()&&!gWeapon.IsNonAttack())
    gMe.aimkey=bKeyDown;
  else
    gMe.aimkey=false;
}

void func_customautorute() {
  if(cvar.active&&gMe.alive&&!gMenu.active&&!gConsole.active&&cvar.autoroute&&(gWeapon.GetAmmo()||gWeapon.IsKnife()))
    gMe.inway=bKeyDown;
  else
    gMe.inway=false;
}

void func_customtriggerbot() {
  if(cvar.active&&gMe.alive&&!gMenu.active&&!gConsole.active&&cvar.triggerbot==2&&gAimbot.HasTarget()&&!gWeapon.IsNonAttack())
    gMe.trigger=bKeyDown;
  else
    gMe.trigger=false;
}

void func_saveconfig() {
  int num=atoi(cmd.argC(1));
  BOUND_VALUE(num,0,5);
  cmd.save_cvars(num);
  gGui.AddMessage(255,255,255,1500,"Config %d Saved",num);
}

void func_loadconfig() {
  int num=atoi(cmd.argC(1));
  BOUND_VALUE(num,0,5);
  cmd.load_cvars(num);
  gGui.AddMessage(255,255,255,1500,"Config %d Loaded",num);
}

void func_hidescreenshot(){
  if(gScreenShots.bScreenTook){
    gScreenShots.bScreenTook=FALSE;
  }
}

//==================================================================================

static int TeamInfo(const char *pszName, int iSize, void *pbuf) {
  BEGIN_READ(pbuf,iSize);
  int team=-1,px=READ_BYTE();
  char * teamtext=READ_STRING();
  static const char * STR_TERROR="TERRORIST";
  static const char * STR_CT="CT";
  static const char * STR_UNASSIGNED="UNASSIGNED";
  static const char * STR_SPECTATOR="SPECTATOR";
  if(!stricmp(teamtext,STR_TERROR))
    team=1;
  else if(!stricmp(teamtext,STR_CT))
    team=2;
  else if(!stricmp(teamtext,STR_UNASSIGNED))
    team=0;
  else if(!stricmp(teamtext,STR_SPECTATOR))
    team=0;
  gPlayers[px]->team=team;
  if(px==gMe.index)gMe.team=team;
  return pTeamInfo(pszName,iSize,pbuf);
}

static int CurWeapon(const char *pszName, int iSize, void *pbuf) {
  BEGIN_READ(pbuf,iSize);
  int iState=READ_BYTE();
  int iID=READ_CHAR();
  int iClip=READ_CHAR();
  if(iState)gMe.iClip=iClip;
  gWeapon.Set(iState,iID,iClip);
  return pCurWeapon(pszName,iSize,pbuf);
}

static int ScoreAttrib(const char *pszName, int iSize, void *pbuf) {
  BEGIN_READ(pbuf,iSize);
  int px=READ_BYTE();
  int info=READ_BYTE();
  gPlayers[px]->info=info;
  if(px==gMe.index)gMe.alive=((info&1)==0);
  return pScoreAttrib(pszName,iSize,pbuf);
}

static int SetFOV(const char *pszName, int iSize, void *pbuf) {
  BEGIN_READ(pbuf,iSize);
  gMe.iFOV=READ_BYTE();
  if(!gMe.iFOV)gMe.iFOV=90;
  gMe.zoom=(gMe.iFOV!=90);
  return pSetFOV(pszName,iSize,pbuf);
}

static int Health(const char *pszName, int iSize, void *pbuf) {
  BEGIN_READ(pbuf,iSize);
  gMe.iHealth=READ_BYTE();
  return pHealth(pszName,iSize,pbuf);
}

static int Battery(const char *pszName, int iSize, void *pbuf) {
  BEGIN_READ(pbuf,iSize);
  gMe.iArmor=READ_BYTE();
  return pBattery(pszName,iSize,pbuf);
}

static int ScoreInfo(const char *pszName, int iSize, void *pbuf) {
  BEGIN_READ(pbuf,iSize);
  int px=READ_BYTE();
  if(px==gMe.index) {
    gMe.iKills=READ_SHORT();
    gMe.iDeaths=READ_SHORT();
    if(gMe.iHs>gMe.iKills)
      gMe.iHs=gMe.iKills;
  }
  return pScoreInfo(pszName,iSize,pbuf);
}

static int DeathMsg(const char *pszName, int iSize, void *pbuf) {
  BEGIN_READ(pbuf,iSize);
  int killer=READ_BYTE();
  int victim=READ_BYTE();
  int headshot=READ_BYTE();
  //suicide or team change
  if(!victim||victim==-1) {
    gPlayers[killer]->SetDead();
    cSoundMark::Reset(killer);
  } else {
    gPlayers[victim]->SetDead();
    cSoundMark::Reset(victim);
  }
  if(cvar.extrablood&&headshot)
    gPlayers[victim]->DrawBlood();
  if(killer==gMe.index&&headshot)
    gMe.iHs++;
  if(victim==gMe.index) {
    gMe.alive=false;
    gScreenShots.bSkipDeath=TRUE;
    gAimbot.iTarget=-1;
  }
  cSoundMark &mark=gSoundMark[victim];
  if(mark.radar)
    mark.radar=FALSE;
  return pDeathMsg(pszName,iSize,pbuf);
}

static int SayText(const char *pszName, int iSize, void *pbuf) {
  return pSayText(pszName,iSize,pbuf);
}

static int ResetHUD(const char *pszName, int iSize, void *pbuf) {
  static char currentMap[100];
  if(strcmp(currentMap,gEngfuncs.pfnGetLevelName())) {
    strcpy(currentMap,gEngfuncs.pfnGetLevelName());
    gMe.iKills=0;
    gMe.iHs=0;
    gMe.iDeaths=0;
    //map alias...
    char mname[128],map_name[100];
    strcpy(map_name,strrchr(currentMap,'/')+1);
    *strchr(map_name,'.')=0;
    sprintf(mname,"m_%s",map_name);
    ::cmd.exec(mname);
    //radar & autoroute...
    gRadar.LoadOverview(map_name);
    gAutoRoute.Load(map_name);
  }
  //reset all
  iNeedBuy=1;
  gMe.alive=false;
  gMe.lastflashtime=0;
  gBomb.Pickup();
  gAimbot.iTarget=-1;
  gPlayers.SetDead();
  gScreenShots.bSkipDeath=FALSE;
  return pResetHUD(pszName,iSize,pbuf);
}

static int Damage(const char *pszName, int iSize, void *pbuf) {
  return pDamage(pszName,iSize,pbuf);
}

static int AmmoX(const char *pszName, int iSize, void *pbuf) {
  int ID, Count;
  BEGIN_READ(pbuf, iSize);
  ID=READ_BYTE();
  Count=READ_BYTE();
  gWeapon.SetAmmoX(ID,Count);
  return pAmmoX(pszName,iSize,pbuf);
}

static int Money(const char *pszName, int iSize, void *pbuf) {
  BEGIN_READ(pbuf,iSize);
  gMe.iMoney=READ_SHORT();
  return pMoney(pszName,iSize,pbuf);
}

static int Radar(const char *pszName, int iSize, void *pbuf) {
  BEGIN_READ(pbuf,iSize);
  vec3_t origin;
  int index;
  index=READ_BYTE();
  origin[0]=READ_COORD();
  origin[1]=READ_COORD();
  origin[2]=READ_COORD();
  if(cvar.soundesp)
    SoundFilter(index,NULL,origin);
  VectorCopy(origin,gPlayers[index]->lastpos);
  return pRadar(pszName,iSize,pbuf);
}

static int WeaponList(const char *pszName, int iSize, void *pbuf) {
  char *weaponname;
  int ammo1type,max1,ammo2type,max2,slot,slotpos,id,flags;
  BEGIN_READ(pbuf,iSize);
  weaponname=READ_STRING();
  ammo1type=READ_CHAR();
  max1=READ_BYTE();
  ammo2type=READ_CHAR();
  max2=READ_BYTE();
  slot=READ_CHAR();
  slotpos=READ_CHAR();
  id=READ_CHAR();
  flags=READ_BYTE();
  gWeapon.Add(weaponname, ammo1type, max1, ammo2type, max2, slot, slotpos, id, flags);
  return pWeaponList(pszName,iSize,pbuf);
}

static int Fog(const char *pszName, int iSize, void *pbuf) {
  if(cvar.active&&!cvar.takingss&&cvar.nosmoke)
    return 1;
  return pFog(pszName,iSize,pbuf);
}

static int BombDrop(const char *pszName, int iSize, void *pbuf) {
  vec3_t origin;
  int flag;
  BEGIN_READ(pbuf,iSize);
  origin[0]=READ_COORD();
  origin[1]=READ_COORD();
  origin[2]=READ_COORD();
  flag=READ_BYTE();
  gBomb.Dropped(origin,flag);
  return pBombDrop(pszName,iSize,pbuf);
}

static int BombPickup(const char *pszName, int iSize, void *pbuf) {
  gBomb.Pickup();
  return pBombPickup(pszName,iSize,pbuf);
}

static int HostagePos(const char *pszName, int iSize, void *pbuf) {
  vec3_t origin;
  int index;//flag,
  BEGIN_READ(pbuf,iSize);
  READ_BYTE();//flag
  index=READ_BYTE();
  origin[0]=READ_COORD();
  origin[1]=READ_COORD();
  origin[2]=READ_COORD();
  gHostages[index].SetAlive(origin);
  return pHostagePos(pszName,iSize,pbuf);
}

static int HostageK(const char *pszName, int iSize, void *pbuf) {
  BEGIN_READ(pbuf,iSize);
  int index=READ_BYTE();
  gHostages[index].SetDead();
  return pHostageK(pszName,iSize,pbuf);
}

static int ServerName(const char *pszName, int iSize, void *pbuf) {
  BEGIN_READ(pbuf,iSize);
  char *name=READ_STRING();
  gMe.SetServerName(name);
  return pServerName(pszName,iSize,pbuf);
}

//==================================================================================

void connectcmd(void) {
  HookSendTo();
  pConnect();
}

void motd_writecmd() {
  const char *p=gEngfuncs.pfnGetLevelName();
  if(cvar.blocksrvcmd&&*p!=0)
    EchoCmdBlocked("motd_write");
  else
    pMotdWrite();
}

void unbindallcmd() {
  const char *p=gEngfuncs.pfnGetLevelName();
  if(cvar.blocksrvcmd&&*p!=0)
    EchoCmdBlocked("unbindall");
  else
    pUnbindAll();
}

void unbindcmd() {
  const char *p=gEngfuncs.pfnGetLevelName();
  if(cvar.blocksrvcmd&&*p!=0)
    EchoCmdBlocked("unbind");
  else
    pUnbind();
}

void timerefreshcmd() {
  const char *p=gEngfuncs.pfnGetLevelName();
  if(cvar.blocksrvcmd&&*p!=0)
    EchoCmdBlocked("timerefresh");
  else
    pTimeRefresh();
}

void writecfgcmd() {
  const char *p=gEngfuncs.pfnGetLevelName();
  if(cvar.blocksrvcmd&&*p!=0)
    EchoCmdBlocked("writecfg");
  else
    pWriteCfg();
}

void dropclientcmd() {
  const char *p=gEngfuncs.pfnGetLevelName();
  if(cvar.blocksrvcmd&&*p!=0)
    EchoCmdBlocked("dropclient");
  else
    pDropClient();
}

void quitcmd() {
  const char *p=gEngfuncs.pfnGetLevelName();
  if(cvar.blocksrvcmd&&*p!=0)
    EchoCmdBlocked("quit");
  else
    pQuit();
}

void exitcmd() {
  const char *p=gEngfuncs.pfnGetLevelName();
  if(cvar.blocksrvcmd&&*p!=0)
    EchoCmdBlocked("exit");
  else
    pExit();
}

void killcmd() {
  const char *p=gEngfuncs.pfnGetLevelName();
  if(cvar.blocksrvcmd&&*p!=0)
    EchoCmdBlocked("kill");
  else
    pKill();
}

//==================================================================================

pfnEvent HookEvent(const char *name,pfnEvent pfn) {
  if(pEventList) {
    for(eventlist_t *pev=*pEventList; pev!=NULL; pev=pev->next) {
      if(pev->name&&pev->pfn&&!stricmp(name,pev->name)) {
        pfnEvent ret=pev->pfn;
        pev->pfn=pfn;
        return ret;
      }
    }
  }
  return NULL;
}

pfnUserMsgHook HookUserMsg(const char *name,pfnUserMsgHook pfn) {
  if(pUserMsgList) {
    for(usermsglist_t *umsg=*pUserMsgList; umsg!=NULL; umsg=umsg->next) {
      if(umsg->name&&umsg->pfn&&!stricmp(name,umsg->name)) {
        pfnUserMsgHook ret=umsg->pfn;
        umsg->pfn=pfn;
        return ret;
      }
    }
  }
  return NULL;
}

pfnSrvCmd HookSrvCmd(const char *name,pfnSrvCmd pfn) {
  if(pSrvCmdList) {
    for(srvcmdlist_t *cmd=*pSrvCmdList; cmd!=NULL; cmd=cmd->next) {
      if(cmd->name&&cmd->pfn&&!stricmp(name,cmd->name)) {
        pfnSrvCmd ret=cmd->pfn;
        cmd->pfn=pfn;
        return ret;
      }
    }
  }
  return NULL;
}

//==================================================================================

VOID HookEvents() {
  if(pEventList) {
    pEvent_usp=HookEvent("events/usp.sc",Event_usp);
    pEvent_mp5n=HookEvent("events/mp5n.sc",Event_mp5n);
    pEvent_ak47=HookEvent("events/ak47.sc",Event_ak47);
    pEvent_aug=HookEvent("events/aug.sc",Event_aug);
    pEvent_deagle=HookEvent("events/deagle.sc",Event_deagle);
    pEvent_g3sg1=HookEvent("events/g3sg1.sc",Event_g3sg1);
    pEvent_sg550=HookEvent("events/sg550.sc",Event_sg550);
    pEvent_glock18=HookEvent("events/glock18.sc",Event_glock18);
    pEvent_m249=HookEvent("events/m249.sc",Event_m249);
    pEvent_m3=HookEvent("events/m3.sc",Event_m3);
    pEvent_m4a1=HookEvent("events/m4a1.sc",Event_m4a1);
    pEvent_mac10=HookEvent("events/mac10.sc",Event_mac10);
    pEvent_p90=HookEvent("events/p90.sc",Event_p90);
    pEvent_p228=HookEvent("events/p228.sc",Event_p228);
    pEvent_awp=HookEvent("events/awp.sc",Event_awp);
    pEvent_scout=HookEvent("events/scout.sc",Event_scout);
    pEvent_sg552=HookEvent("events/sg552.sc",Event_sg552);
    pEvent_tmp=HookEvent("events/tmp.sc",Event_tmp);
    pEvent_fiveseven=HookEvent("events/fiveseven.sc",Event_fiveseven);
    pEvent_ump45=HookEvent("events/ump45.sc",Event_ump45);
    pEvent_xm1014=HookEvent("events/xm1014.sc",Event_xm1014);
    pEvent_elite_left=HookEvent("events/elite_left.sc",Event_elite_left);
    pEvent_elite_right=HookEvent("events/elite_right.sc",Event_elite_right);
    pEvent_galil=HookEvent("events/galil.sc",Event_galil);
    pEvent_famas=HookEvent("events/famas.sc",Event_famas);
    pEvent_knife=HookEvent("events/knife.sc",Event_knife);
  }
}

VOID HookUserMsgs() {
  if(pUserMsgList) {
    pResetHUD=HookUserMsg("ResetHUD",ResetHUD);
    pTeamInfo=HookUserMsg("TeamInfo",TeamInfo);
    pCurWeapon=HookUserMsg("CurWeapon",CurWeapon);
    pScoreAttrib=HookUserMsg("ScoreAttrib",ScoreAttrib);
    pSetFOV=HookUserMsg("SetFOV",SetFOV);
    pHealth=HookUserMsg("Health",Health);
    pBattery=HookUserMsg("Battery",Battery);
    pScoreInfo=HookUserMsg("ScoreInfo",ScoreInfo);
    pDeathMsg=HookUserMsg("DeathMsg",DeathMsg);
    pSayText=HookUserMsg("SayText",SayText);
    pDamage=HookUserMsg("Damage",Damage);
    pAmmoX=HookUserMsg("AmmoX",AmmoX);
    pMoney=HookUserMsg("Money",Money);
    pRadar=HookUserMsg("Radar",Radar);
    pWeaponList=HookUserMsg("WeaponList",WeaponList);
    pFog=HookUserMsg("Fog",Fog);
    pBombDrop=HookUserMsg("BombDrop",BombDrop);
    pBombPickup=HookUserMsg("BombPickup",BombPickup);
    pHostagePos=HookUserMsg("HostagePos",HostagePos);
    pHostageK=HookUserMsg("HostageK",HostageK);
    pServerName=HookUserMsg("ServerName",ServerName);
  }
}

VOID HookSrvCmds() {
  if(pSrvCmdList) {
    pConnect=HookSrvCmd("connect",connectcmd);
    pMotdWrite=HookSrvCmd("motd_write",motd_writecmd);
    pUnbindAll=HookSrvCmd("unbindall",unbindallcmd);
    pUnbind=HookSrvCmd("unbind",unbindcmd);
    pTimeRefresh=HookSrvCmd("timerefresh",timerefreshcmd);
    pWriteCfg=HookSrvCmd("writecfg",writecfgcmd);
    pDropClient=HookSrvCmd("dropclient",dropclientcmd);
    pKill=HookSrvCmd("kill",killcmd);
    pExit=HookSrvCmd("exit",exitcmd);
    pQuit=HookSrvCmd("quit",quitcmd);
  }
}

//==================================================================================
