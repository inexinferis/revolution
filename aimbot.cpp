#include "weapons.h"
#include "textures.h"
#include "players.h"
#include "drawing.h"
#include "aimbot.h"
#include "main.h"
#include "cvar.h"
#include "nospread.h"

#include "opengl.h"
#include <time.h>

cAimbot gAimbot;

void cAimbot::DrawAimSpot() {
  vec3_t vecScreen,EntViewOrg;
  if(cvar.avdraw||(cvar.drawfov&&cvar.fovmethod==2)) {
    float dynfov=GetDynamicFov();
    for(int ax=0; ax<MAX_VPLAYERS; ax++) {
      cPlayer *player=gPlayers[ax];
      if(!player||!player->IsValidEnt())
        continue;
      if(cvar.avdraw) {
        //get aim origin
        CalculateAimOrigin(player,EntViewOrg);
        DrawVectorDot(EntViewOrg,255,255,255,255);
      }
      if(cvar.drawfov&&CheckTeam(*player)) {
        int boxradius=(int)(((dynfov*25)*90.0)/(player->distance*gMe.iFOV));
        if(boxradius<displayCenterX) {
          //from ent origin...
          if(!CalcScreen(player->ent->origin,vecScreen))
            continue;
          ColorEntry &color=gColorList.get(0);
          DrawBox((int)vecScreen[0],(int)vecScreen[1],boxradius,2,color.r,color.g,color.b,100);
        }
      }
    }
  }
  if(cvar.drawfov&&cvar.fovmethod!=2) {
    float fov=GetAimFov();
    float pixelfov=GetPixelFov();
    ColorEntry &color=gColorList.get(0);
    if(cvar.fovmethod) {
      if(pixelfov&&pixelfov<displayCenterX)
        DrawBox(displayCenterX,displayCenterY,(int)pixelfov,2,color.r,color.g,color.b,100);
    } else {
      if(fov&&fov<60)
        DrawBox(displayCenterX,displayCenterY,(int)(sin(fov*M_D2R)*displayCenterX),2,color.r,color.g,color.b,100);
    }
  }
  if(cvar.avdraw&&gMe.alive&&HasTarget()) {
    vec3_t aimangles,position;
    VectorCopy(aim_viewangles,aimangles);
    // no spread
    if(cvar.nospread||cvar.speedmode)
      gNoSpread.GetSpreadOffset(aimangles);
    // no recoil
    if(cvar.norecoil||cvar.speedmode)
      gNoSpread.GetRecoilOffset(aimangles);
    AnglesVector(aimangles,position);
    VectorScale(position,tDistance,position);
    VectorAdd(gMe.pmEyePos,position,EntViewOrg);
    DrawVectorDot(EntViewOrg,255,0,0,255);
    //DumpVector("EntViewOrg",EntViewOrg);
    //cPlayer *player=gPlayers[iTarget];
    //CalculateAimOrigin(player,EntViewOrg);
    //DumpVector("player",EntViewOrg);
  }
}

void cAimbot::PredictTarget(cPlayer *player,vec3_t &pred) {
  //player movement prediction
  vec3_t vDeltaOrigin,vVelocity;
  net_status_s net;
  float fTimeFraction=0;
  VectorSubtract(player->ent->origin,player->ent->ph[player->ent->current_position].origin,vDeltaOrigin);
  if(cvar.smoothaim!=0) {
    vDeltaOrigin[0]*=cvar.smoothaim;
    vDeltaOrigin[1]*=cvar.smoothaim;
    vDeltaOrigin[2]*=cvar.smoothaim;
  }
  if(cvar.aimpred==3) {
    gEngfuncs.pNetAPI->Status(&net);
    pred[0]+=player->velocity[0]*net.latency*cvar.predahead;
    pred[1]+=player->velocity[1]*net.latency*cvar.predahead;
  } else {
    switch((int)cvar.aimpred) {
      case 1:
        fTimeFraction=cvar.predahead;
        break;
      default: {
          gEngfuncs.pNetAPI->Status(&net);
          fTimeFraction=net.latency*10*cvar.predahead;
        }
        break;
    }
    pred[0]-=vDeltaOrigin[0]*fTimeFraction;
    pred[1]-=vDeltaOrigin[1]*fTimeFraction;
  }
  //me movement prediction... (server v43) this is fucking amazing!
  if(!VectorEmpty(gMe.pmVelocity)) {
    VectorCopy(gMe.pmVelocity,vVelocity);
    VectorScale(vVelocity,2*gMe.pmFrameTime,vVelocity);
    VectorSubtract(pred,vVelocity,pred);
  }
}

bool cAimbot::SmoothAim(float *me) {
  float diff[3];
  VectorSubtract(aim_viewangles,me,diff);
  if(diff[YAW]>180)diff[YAW]-=360;
  if(diff[YAW]<-180)diff[YAW]+=360;
  if((diff[PITCH]>=-cvar.smoothbox&&diff[PITCH]<=cvar.smoothbox)&&(diff[YAW]>=-cvar.smoothbox&&diff[YAW]<=cvar.smoothbox)) {
    VectorCopy(aim_viewangles,me);
    return true;
  }
  me[PITCH]+=diff[PITCH]/(cvar.smoothaim+1);
  me[YAW]+=diff[YAW]/(cvar.smoothaim+1);
  if(me[YAW]>180)me[YAW]-=360;
  if(me[YAW]<-180)me[YAW]+=360;
  return false;
}

void cAimbot::CalculateVector(cPlayer *player) {
  vec3_t up,right,forward,playerAngles;
  float sih,sif,sir;
  switch(player->ent->curstate.gaitsequence) {
    case 1://Standing
      sih=19;
      sif=10;
      sir=0;
      break;
    case 2://Ducking
      sih=26;
      sif=2;
      sir=0;
      break;
    case 3://Walking
      sih=19;
      sif=6;
      sir=0;
      break;
    case 4://Running
      sih=20;
      sif=6;
      sir=0;
      break;
    case 5://DuckWalk
      sih=20;
      sif=6;
      sir=0;
      break;
    case 6://Jumping
      sih=19;
      sif=6;
      sir=0;
      break;
    default:
      if(player->ent->curstate.usehull==0) {
        sih=19;
        sif=10;
        sir=0;
      } else { //ducking
        sih=26;
        sif=2;
        sir=0;
      }
      break;
  }
  //angles to vector
  playerAngles[0]=0;
  playerAngles[1]=player->ent->angles[1];
  playerAngles[2]=0;
  gEngfuncs.pfnAngleVectors(playerAngles,forward,right,up);
  forward[1]=-forward[2];
  //calc offset
  player->vOffsets=forward*sif+up*sih+right*sir;
  //for new method...
  player->vHitbox=player->ent->origin+player->vOffsets;
}

void cAimbot::CalculateHitbox(cPlayer *player) {
  float aimspot;
  if(!cvar.aimingmethod)return;
  if(!player||!player->IsValidEnt())
    return;
  //hitbox / bone aim
  if(!player->bGotHead) {
    if(cvar.randspot) {
      srand(time(NULL));
      aimspot=rand()%2+1;
    } else
      aimspot=cvar.aimspot;
    TransformMatrix *pBoneTransform=(TransformMatrix*)gEngStudio.StudioGetBoneTransform();
    model_s	*pModel=gEngStudio.SetupPlayerModel(player->index);
    studiohdr_t *pStudioHeader=(studiohdr_t*)gEngStudio.Mod_Extradata(pModel);
    if(cvar.aimingmethod==1) { //hitbox aim
      mstudiobbox_t *pStudioBox=(mstudiobbox_t*)((byte*)pStudioHeader+pStudioHeader->hitboxindex);
      vec3_t vMin,vMax;
      int i=11;
      //Head 11 bone 7 | Low Head 9 bone 5 | Chest 8 bone 4 | Stomach 7 bone 3
      if(gWeapon.IsKnife())
        i=9;
      else if(aimspot==1)i=11;
      else if(aimspot==2)i=9;
      else if(aimspot==3)i=8;
      else if(aimspot==4)i=7;
      VectorTransform(pStudioBox[i].bbmin,(*pBoneTransform)[pStudioBox[i].bone],vMin);
      VectorTransform(pStudioBox[i].bbmax,(*pBoneTransform)[pStudioBox[i].bone],vMax);
      player->vHitbox=(vMin+vMax)*0.5f;
      //save other hitboxes just in case...
      for(i=0;i<pStudioHeader->numhitboxes&&i<(int)ARRAYSIZE(player->vHitboxs);i++) {
        VectorTransform(pStudioBox[i].bbmin,(*pBoneTransform)[pStudioBox[i].bone],vMin);
        VectorTransform(pStudioBox[i].bbmax,(*pBoneTransform)[pStudioBox[i].bone],vMax);
        player->vHitboxs[i]=(vMax+vMin)*0.5f;
      }
      player->numhitboxes=min(pStudioHeader->numhitboxes,(int)ARRAYSIZE(player->vHitboxs));
    } else { //bone aim
      int i=7;
      //Head 11 bone 7 | Low Head 9 bone 5 | Chest 8 bone 4 | Stomach 7 bone 3
      if(gWeapon.IsKnife())
        i=5;
      else if(aimspot==1)i=7;
      else if(aimspot==2)i=5;
      else if(aimspot==3)i=4;
      else if(aimspot==4)i=3;
      player->vHitbox[0]=(*pBoneTransform)[i][0][3];
      player->vHitbox[1]=(*pBoneTransform)[i][1][3];
      player->vHitbox[2]=(*pBoneTransform)[i][2][3];
      //save other bones just in case...
      for(i=0;i<pStudioHeader->numbones&&i<(int)ARRAYSIZE(player->vBones);i++) {
        player->vBones[i][0]=(*pBoneTransform)[i][0][3];
        player->vBones[i][1]=(*pBoneTransform)[i][1][3];
        player->vBones[i][2]=(*pBoneTransform)[i][2][3];
      }
      player->numhitboxes=min(pStudioHeader->numbones,(int)ARRAYSIZE(player->vBones));
    }
    //calc offset
    player->vOffsets=player->vHitbox-player->ent->origin;
    player->bGotAnyVisiblePart=false;
    player->bGotHead=true;
  }
}

void cAimbot::CalculateAimOrigin(cPlayer *player,vec3_t &vAimOrigin) {
  vec3_t playerAngles,up,right,forward;
  if(!cvar.aimingmethod)
    CalculateVector(player);
  VectorCopy(player->ent->origin,vAimOrigin);
  vAimOrigin=vAimOrigin+player->vOffsets;
  //prediction...
  if(cvar.aimpred)
    PredictTarget(player,vAimOrigin);
  //Fix Aimbot
  playerAngles[0]=0;
  playerAngles[1]=player->ent->angles[1];
  playerAngles[2]=0;
  gEngfuncs.pfnAngleVectors(playerAngles,forward,right,up);
  forward[2]=-forward[2];
  vAimOrigin=vAimOrigin+forward*cvar.hadjf;
  vAimOrigin=vAimOrigin+up*cvar.hadjh;
  vAimOrigin=vAimOrigin+right*cvar.hadjr;
}

void cAimbot::CalculateAimingView() {
  float view[3];
  vec3_t EntViewOrg,MeEyePos;
  if(iTarget==-1)return;
  cPlayer *player=gPlayers[iTarget];
  if(!player||!player->IsValidEnt())
    return;
  // get aim origin
  CalculateAimOrigin(player,EntViewOrg);
  // calculate view
  VectorSubtract(EntViewOrg,gMe.pmEyePos,view);
  //extra data
  tDistance=VectorLength(view);
  // calculate angle vectors
  VectorAngles(view,aim_viewangles);
  aim_viewangles[0]*=-1;
  if(aim_viewangles[0]>180)aim_viewangles[0]-=360;
  if(aim_viewangles[1]>180)aim_viewangles[1]-=360;
}

bool cAimbot::AutoKnife(struct usercmd_s *cmd) {
  cl_entity_s *ent;
  float flDistance=32;
  int iEntity;
  pmtrace_t gVis;
  vec3_t vForward, vecEnd;
  gEngfuncs.pfnAngleVectors(aim_viewangles,vForward,NULL,NULL);
  for(int iCount=1; iCount<3; iCount++) {
    vecEnd=gMe.pmEyePos+vForward*flDistance;
    gEngfuncs.pEventAPI->EV_SetTraceHull(2);
    gEngfuncs.pEventAPI->EV_PlayerTrace(gMe.pmEyePos,vecEnd,PM_STUDIO_BOX,-1,&gVis);
    iEntity=gEngfuncs.pEventAPI->EV_IndexFromTrace(&gVis);
    ent=gEngfuncs.GetEntityByIndex(iEntity);
    if(ent->player) {
      if(iEntity==iTarget) {
        if(iCount==1) {
          cmd->buttons|=IN_ATTACK2;
          return true;
        } else if(iCount==2) {
          cmd->buttons|=IN_ATTACK;
          return false;
        }
      } else	if(iCount==2)
        return false;
    } else	if(iCount==2)
      return false;
    flDistance+=16.0f;
  }
  return false;
}

void cAimbot::CreateMove(struct usercmd_s *cmd) {
  if(iTarget==-1||!tDistance)return;
  float angle=aim_viewangles[1]-gMe.pmViewAngles[1];
  if(angle>180)angle-=360;
  if(cmd->forwardmove>-10&&cmd->forwardmove<10&&cmd->sidemove>-10&&cmd->sidemove<10) {
    cmd->forwardmove=cos(angle*M_D2R)*tDistance;
    cmd->sidemove=-sin(angle*M_D2R)*tDistance;
  }
  AutoKnife(cmd);
}

bool cAimbot::PathFree(float* xfrom,float* xto) {
  pmtrace_t tr;
  gEngfuncs.pEventAPI->EV_SetTraceHull(2);
  gEngfuncs.pEventAPI->EV_PlayerTrace(xfrom,xto,PM_GLASS_IGNORE,gMe.index,&tr);
  if(tr.fraction!=1.0&&cvar.autowall) {
    if(tr.fraction<0.9)
      return CanPenetrate(xfrom,xto);
    return CanPenetrateX(xfrom,xto);
  }
  return (tr.fraction==1.0);
}

bool cAimbot::ShouldShoot(struct usercmd_s *cmd) {
  if(iTarget==-1)return false;
  cPlayer *player=gPlayers[iTarget];
  if(!player||!player->IsValidEnt())
    return false;
  float vec[3],view[3],viewangles[3],dot,distance;
  vec3_t up,right,forward,playerAngles,origin;
  //body aim origin
  VectorCopy(player->ent->origin,origin);
  playerAngles[0]=0;
  playerAngles[1]=player->ent->angles[1];
  playerAngles[2]=2;
  gEngfuncs.pfnAngleVectors(playerAngles,forward,right,up);
  forward[1]=-forward[2];
  if(player->ent->curstate.usehull==0) {
    origin=origin+forward*1.5;
    origin=origin+up*7;
  } else { //agachado
    origin=origin+forward*-5.5;
    origin=origin+up*11;
  }
  //calc real view..
  VectorCopy(cmd->viewangles,viewangles);
  // no spread
  if(cvar.nospread||cvar.speedmode)
    gNoSpread.GetSpreadOffset(viewangles);
  // no recoil
  if(cvar.norecoil||cvar.speedmode)
    gNoSpread.GetRecoilOffset(viewangles);
  AnglesVector(viewangles,view);
  //calc vector
  VectorSubtract(origin,gMe.pmEyePos,vec);
  distance=VectorNormalize(vec);
  dot=view[0]*vec[0]+view[1]*vec[1]+view[2]*vec[2];
  if(dot>1.0)dot=1.0;
  if(dot>.70&&(distance*tan(acos(dot)))<cvar.triggerboxb)//10
    return true;
  //get aim origin
  CalculateAimOrigin(player,origin);
  VectorSubtract(origin,gMe.pmEyePos,vec);
  distance=VectorNormalize(vec);
  dot=view[0]*vec[0]+view[1]*vec[1]+view[2]*vec[2];
  if(dot>1.0)dot=1.0;
  if(dot>.70&&(distance*tan(acos(dot)))<cvar.triggerboxh)//10
    return true;
  return false;
}

bool cAimbot::IsAimingAtMe(cPlayer *player) {
  float view[3],invAimAngles[3];
  vec3_t EntViewOrg;//get aim origin
  CalculateAimOrigin(player,EntViewOrg);
  VectorSubtract(gMe.pmEyePos,EntViewOrg,view);
  VectorAngles(view,invAimAngles);
  invAimAngles[0]*=-1;
  if(invAimAngles[0]>180)invAimAngles[0]-=360;
  if(invAimAngles[1]>180)invAimAngles[1]-=360;
  double variationPitch=invAimAngles[0]-player->ent->angles[0];
  double variationYaw=invAimAngles[1]-player->ent->angles[1];
  return (fabs(variationPitch)<2&&fabs(variationYaw)<3);
}

int cAimbot::FindTarget() {
  if(!gMe.alive)return false;
  iTarget=-1;
  tDistance=0;
  for(int ax=0; ax<MAX_VPLAYERS; ax++) {
    cPlayer *player=gPlayers[ax];
    if(player->bCanAim&&player->IsValidEnt()) {
      if(iTarget==-1) {
        iTarget=ax;
        continue;
      }
      if(cvar.aimingatme&&IsAimingAtMe(player)) {
        iTarget=ax;
        break;
      }
      if(gPlayers[iTarget]->bGotAnyVisiblePart&&!player->bGotAnyVisiblePart){
        iTarget=ax;
        continue;
      }
      if(gPlayers[iTarget]->fovangle<gMe.iFOV&&player->fovangle<gMe.iFOV) {
        if(!gWeapon.IsKnife()&&(gMe.iFOV==90||!gWeapon.IsSnipper())) {
          // this is based in sin(angle)*distance to get the opposite cathetus * distance
          if(sin(player->fovangle/M_R2D)*pow(player->distance,2)<
            sin(gPlayers[iTarget]->fovangle/M_R2D)*pow(gPlayers[iTarget]->distance,2))
            iTarget=ax;
        } else {
          if(player->distance<gPlayers[iTarget]->distance)
            iTarget=ax;
        }
      } else {
        if(player->distance<5||gPlayers[iTarget]->distance<5) {
          if(player->distance<gPlayers[iTarget]->distance)
            iTarget=ax;
        } else if(player->fovangle<gPlayers[iTarget]->fovangle)
          iTarget=ax;
      }
    }
  }
  return (iTarget!=-1);
}

// ver producto scalar de vectores...
// cos @ = A.B/|A|.|B| => |A|.|B| = 1 => cos @ = A.B
float cAimbot::CalcFovAngle(const float* origin_viewer, const float* angle_viewer, const float* origin_target, float* dynfov) {
  float vec[3],view[3],dot,distance,angle;
  VectorSubtract(origin_target,origin_viewer,vec);
  distance=VectorNormalize(vec);
  AnglesVector(angle_viewer,view);
  dot=view[0]*vec[0]+view[1]*vec[1]+view[2]*vec[2];
  if(dot>1.0)dot=1.0;
  if(dynfov) {
    angle=(float)acos(dot);
    if(angle==M_PI_2||angle==(M_PI+M_PI_2))
      angle-=0.00001;//Just in case...
    *dynfov=tan(angle)*distance;
    return angle*M_R2D;
  }
  return (float)(acos(dot)*M_R2D);
}

float cAimbot::CalcPixelFov(const float* origin_target) {
  vec3_t vecScreen;
  if(!CalcScreen(origin_target,vecScreen))
    return 99999999.0;
  return max(fabs(displayCenterX-vecScreen[0]),fabs(displayCenterY-vecScreen[1]));
}

bool cAimbot::IsShielded(cPlayer &player) {
  int seqinfo=gWeapon.GetSequenceInfo(player.ent->curstate.sequence);
  if(seqinfo&SEQUENCE_RELOAD)
    return false;
  if(seqinfo&SEQUENCE_SHIELD)
    return true;
  return false;
}

bool cAimbot::CheckTeam(cPlayer &player) {
  switch((int)cvar.aimteam) {
    case 2:
      return true;
    case 1:
      if(gMe.team==player.team) return true;
      break;
    default:
      if (gMe.team!=player.team) return true;
      break;
  }
  return false;
}

float cAimbot::GetDynamicFov() {
  static float factor=0.0;
  if(!factor)
    factor=displayCenterX/gMe.iFOV;
  if(cvar.customfov)
    return fabs(cvar.customfov);
  switch((int)cvar.aimfov) {
    case 1:
      return 25;//fovbox
    case 2:
      return 75;
    case 3:
      return 150;
    case 4:
      return 500;
    case 5:
      return 2000;
    case 6:
      return 7500;
    default:
      return MAX_MAP_SIZE;
  }
  return 360;
}

float cAimbot::GetAimFov() {
  if(cvar.customfov)
    return fabs(cvar.customfov);
  switch((int)cvar.aimfov) {
    //case 0:return 0;
    case 1:
      return 5;
    case 2:
      return 10;
    case 3:
      return 15;
    case 4:
      return 30;
    case 5:
      return 60;
    case 6:
      return 90;
    default:
      return 180;
  }
  return 360;
}

float cAimbot::GetPixelFov() {
  if(cvar.customfov)
    return fabs(cvar.customfov);
  switch((int)cvar.aimfov) {
    //case 0:return 0;
    case 1:
      return displayCenterX/50;
    case 2:
      return displayCenterX/30;
    case 3:
      return displayCenterX/20;
    case 4:
      return displayCenterX/10;
    case 5:
      return displayCenterX/7;
    case 6:
      return displayCenterX/4;
    default:
      return displayCenterX/2;
  }
  return displayCenterX;
}

bool cAimbot::FindVisibleSpot(cPlayer &player){
  vec3_t *points,*closest=NULL;int i;
  float dist,mindist=9999999.9f;
  if(cvar.aimingmethod==1) { //hitbox aim
    points=player.vHitboxs;
  }else{
    points=player.vBones;
  }
  for(i=0;i<player.numhitboxes;i++) {
    if(PathFree(gMe.pmEyePos,points[i])){
      dist=gMe.GetDistanceFrom(points[i]);
      if(dist<mindist){
        mindist=dist;
        closest=&points[i];
      }
    }
  }
  if(closest){
    player.vOffsets=*closest-player.ent->origin;
    player.bGotAnyVisiblePart=true;
    return TRUE;
  }
  return FALSE;
}

void cAimbot::CalcFovAndVisibility(cPlayer &player) {
  vec3_t EntViewOrg;//get aim origin

  if(!player.IsValidEnt()) {
    player.bVisible=false;
    return;
  }

  CalculateAimOrigin(&player,EntViewOrg);
  player.fovangle=CalcFovAngle(gMe.ent->origin,gMe.pmViewAngles,player.ent->origin,&player.dynfov);
  player.fovrect=CalcPixelFov(player.ent->origin);
  player.bVisible=PathFree(gMe.pmEyePos,EntViewOrg);
  if(!player.bVisible&&cvar.aimvisiblespot&&player.bGotHead){
    player.bVisible=FindVisibleSpot(player);
  }

  if(!CheckTeam(player)||!player.bVisible)player.bCanAim=false;
  else if(cvar.aimshield&&IsShielded(player))player.bCanAim=false;
  else if(cvar.aimdist&&(cvar.aimdist*10)>player.distance)player.bCanAim=false;
  else if(cvar.spawnprotect&&player.bSpawning)player.bCanAim=false;
  else {
    switch((int)cvar.fovmethod) {
      case 2:
        player.bCanAim=(player.dynfov<GetDynamicFov());
        break;
      case 1:
        player.bCanAim=(player.fovrect<GetPixelFov());
        break;
      default:
        player.bCanAim=(player.fovangle<GetAimFov());
        break;
    }
  }
}

void cAimbot::FixupAngleDifference(usercmd_t *cmd) {
  Vector viewforward,viewright,viewup,aimforward,aimright,aimup,vTemp;
  float newforward,newright,newup,fTime;
  float forward=usercmd.forwardmove;
  float right=usercmd.sidemove;
  float up=usercmd.upmove;

  if(gMe.ent->curstate.movetype==MOVETYPE_WALK)
    gEngfuncs.pfnAngleVectors(Vector(0.0f,usercmd.viewangles.y,0.0f),viewforward,viewright,viewup);
  else
    gEngfuncs.pfnAngleVectors(usercmd.viewangles,viewforward,viewright,viewup);

  if(gMe.ent->curstate.movetype==MOVETYPE_WALK&&!GetAsyncKeyState(VK_LSHIFT)&&!(cmd->buttons&IN_ATTACK)&&!(cmd->buttons&IN_USE)) {
    fTime=gEngfuncs.GetClientTime();
    cmd->viewangles.y=fmod(fTime*SPIN_REVS_PER_SECOND*360.0f, 360.0f);
  }

  if(gMe.ent->curstate.movetype==MOVETYPE_WALK)
    gEngfuncs.pfnAngleVectors(Vector(0.0f,cmd->viewangles.y,0.0f),aimforward,aimright,aimup);
  else
    gEngfuncs.pfnAngleVectors(cmd->viewangles,aimforward,aimright,aimup);

  newforward=DotProduct(forward*viewforward.Normalize(),aimforward)+DotProduct(right*viewright.Normalize(),aimforward)+DotProduct(up*viewup.Normalize(), aimforward);
  newright=DotProduct(forward*viewforward.Normalize(),aimright)+DotProduct(right*viewright.Normalize(), aimright)+DotProduct(up*viewup.Normalize(), aimright);
  newup=DotProduct(forward*viewforward.Normalize(),aimup)+DotProduct(right*viewright.Normalize(),aimup)+DotProduct(up*viewup.Normalize(),aimup);

  cmd->forwardmove=newforward;
  cmd->sidemove=newright;
  cmd->upmove=newup;
}

void cAimbot::ApplySilentAngles(float *angles,usercmd_s *cmd) {
  Vector viewforward,viewright,viewup,aimforward,aimright,aimup,vTemp;
  float newforward,newright,newup;
  float forward=cmd->forwardmove;
  float right=cmd->sidemove;
  float up=cmd->upmove;

  if(gMe.ent->curstate.movetype==MOVETYPE_WALK)
    gEngfuncs.pfnAngleVectors(Vector(0.0f,cmd->viewangles.y,0.0f),viewforward,viewright,viewup);
  else
    gEngfuncs.pfnAngleVectors(cmd->viewangles,viewforward,viewright,viewup);

  if(gMe.ent->curstate.movetype==MOVETYPE_WALK&&!GetAsyncKeyState(VK_LSHIFT)&&!(gMe.pmMoveType==5)) {
    cmd->viewangles.x=angles[0];
    cmd->viewangles.y=angles[1];
  }

  if(gMe.ent->curstate.movetype==MOVETYPE_WALK)
    gEngfuncs.pfnAngleVectors(Vector(0.0f,cmd->viewangles.y,0.0f),aimforward,aimright,aimup);
  else
    gEngfuncs.pfnAngleVectors(cmd->viewangles,aimforward,aimright,aimup);

  newforward=DotProduct(forward*viewforward.Normalize(),aimforward)+DotProduct(right*viewright.Normalize(),aimforward)+DotProduct(up*viewup.Normalize(),aimforward);
  newright=DotProduct(forward*viewforward.Normalize(),aimright)+DotProduct(right*viewright.Normalize(),aimright)+DotProduct(up*viewup.Normalize(),aimright);
  newup=DotProduct(forward*viewforward.Normalize(),aimup)+DotProduct(right*viewright.Normalize(),aimup)+DotProduct(up*viewup.Normalize(),aimup);

  if(angles[0]>81)
    cmd->forwardmove=-newforward;
  else
    cmd->forwardmove=newforward;
  cmd->sidemove=newright;
  cmd->upmove=newup;
}

int cAimbot::CanPenetrate(float *start, float *end) {
  static pmtrace_t pmtrace;
  pmtrace_t*tr=(pmtrace_t*)&pmtrace;
  int power=gWeapon.GetPenetration();
  if(!power)return 0;
  vec3_t dir,position;
  VectorSubtract(end,start,dir);
  VectorNormalize(dir);
  position=start;
  tr->startsolid=true;
  while(power) {
    if(!tr->startsolid)
      power--;
    tr=gEngfuncs.PM_TraceLine(position,end,PM_TRACELINE_PHYSENTSONLY,2,-1);
    if(tr->fraction==1.0f||tr->ent)
      return 1;
    if(tr->allsolid)
      return 0;
    position=tr->endpos+dir*8.0f;
  }
  return 0;
}

int cAimbot::CanPenetrateX(float* start,float* end) {
  float distfrac,damagemult,distancemax,vecSrc[3],vecEnd[3],vecDir[3],length,tmpVec[3];
  float tmplen,length1,length2;
  pmtrace_t tr;
  float bulletvelocity,tmppen=gWeapon.GetPenetration()+1;
  float wallpierce=gWeapon.GetWallPierce();
  float flDistance=gWeapon.GetDistance();
  float damage=gWeapon.GetDamage();
  vecSrc[0]=start[0];
  vecSrc[1]=start[1];
  vecSrc[2]=start[2];
  vecEnd[0]=end[0];
  vecEnd[1]=end[1];
  vecEnd[2]=end[2];
  switch(gWeapon.GetBulletType()) {
    case BULLETTYPE_1:
      bulletvelocity=21;
      distancemax=800.0f;
      break;
    case BULLETTYPE_9:
      if (cvar.wallsensitivity==0)
        damage-=2;
      else if (cvar.wallsensitivity==2)
        damage+=2;
      bulletvelocity=15;
      distancemax=500.0f;
      break;
    case BULLETTYPE_13:
      bulletvelocity=30;
      distancemax=1000.0f;
      break;
    case BULLETTYPE_11:
      if (cvar.wallsensitivity==0)
        damage-=2;
      else if (cvar.wallsensitivity==2)
        damage+=2;
      bulletvelocity=39;
      distancemax=5000.0f;
      break;
    case BULLETTYPE_12:
      if (cvar.wallsensitivity==0)
        damage-=3;
      else if (cvar.wallsensitivity==2)
        damage+=3;
      bulletvelocity=35;
      distancemax=4000.0f;
      break;
    case BULLETTYPE_10:
      if (cvar.wallsensitivity==0)
        damage-=4;
      else if (cvar.wallsensitivity==2)
        damage+=4;
      bulletvelocity=45;
      distancemax=8000.0f;
      break;
    case BULLETTYPE_14:
      if (cvar.wallsensitivity==0)
        damage-=4;
      else if (cvar.wallsensitivity==2)
        damage+=6;
      else
        damage+=1;
      bulletvelocity=30;
      distancemax=2000.0f;
      break;
    case BULLETTYPE_15:
      if (cvar.wallsensitivity==0)
        damage-=4;
      else if (cvar.wallsensitivity==2)
        damage+=6;
      else
        damage+=1;
      bulletvelocity=25;
      distancemax=800.0f;
      break;
    default:
      bulletvelocity=0;
      distancemax=0.0f;
      break;
  }
  vecDir[0]=vecEnd[0]-vecSrc[0];
  vecDir[1]=vecEnd[1]-vecSrc[1];
  vecDir[2]=vecEnd[2]-vecSrc[2];
  length=VectorLength(vecDir);
  vecDir[0]/=length;
  vecDir[1]/=length;
  vecDir[2]/=length;
  vecEnd[0]=vecDir[0]*flDistance+vecSrc[0];
  vecEnd[1]=vecDir[1]*flDistance+vecSrc[1];
  vecEnd[2]=vecDir[2]*flDistance+vecSrc[2];
  damagemult=0.5f;
  while(tmppen) {
    //MP_TraceLine
    if(vecSrc[0]==vecEnd[0]&&vecSrc[1]==vecEnd[1]&&vecSrc[2]==vecEnd[2]) {
      memset(&tr,0,sizeof(pmtrace_t));
      tr.endpos[0]=vecEnd[0];
      tr.endpos[1]=vecEnd[1];
      tr.endpos[2]=vecEnd[2];
      tr.fraction=1.0f;
    } else {
      float tmpSrc[3];
      gEngfuncs.pEventAPI->EV_SetTraceHull(2);
      gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc,vecEnd,PM_GLASS_IGNORE,-1,&tr);
      if(tr.fraction==0&&tr.startsolid&&!tr.allsolid) {
        tmpSrc[0]=vecSrc[0];
        tmpSrc[1]=vecSrc[1];
        tmpSrc[2]=vecSrc[2];
        while(!tr.allsolid&&tr.fraction==0) {
          tmpSrc[0]+=vecDir[0];
          tmpSrc[1]+=vecDir[1];
          tmpSrc[2]+=vecDir[2];
          gEngfuncs.pEventAPI->EV_SetTraceHull(2);
          gEngfuncs.pEventAPI->EV_PlayerTrace(tmpSrc,vecEnd,PM_GLASS_IGNORE,-1,&tr);
        }
        if(!tr.allsolid&&tr.fraction!=1.0) {
          tmpSrc[0]=vecEnd[0]-vecSrc[0];
          tmpSrc[1]=vecEnd[1]-vecSrc[1];
          tmpSrc[2]=vecEnd[2]-vecSrc[2];
          length1=VectorLength(tmpSrc);
          tmpSrc[0]=tr.endpos[0]-vecSrc[0];
          tmpSrc[1]=tr.endpos[1]-vecSrc[1];
          tmpSrc[2]=tr.endpos[2]-vecSrc[2];
          length2=VectorLength(tmpSrc);
          tr.fraction=length2/length1;
          tr.startsolid=true;
        }
      }
      if(tr.allsolid)tr.fraction=1.0f;
    }
    switch(GetTextureType(&tr,vecSrc,vecEnd)) {
      case CHAR_TEX_CONCRETE:
        bulletvelocity *= 0.25;
        break;
      case CHAR_TEX_GRATE:
        bulletvelocity *= 0.5;
        damagemult=0.4f;
        break;
      case CHAR_TEX_METAL:
        bulletvelocity *= 0.15;
        damagemult=0.2f;
        break;
      case CHAR_TEX_COMPUTER:
        bulletvelocity *= 0.4;
        break;
      case CHAR_TEX_TILE:
        bulletvelocity *= 0.65;
        damagemult=0.2f;
        break;
      case CHAR_TEX_VENT:
        bulletvelocity *= 0.5;
        damagemult=0.45f;
        break;
      case CHAR_TEX_WOOD:
        damagemult=0.6f;
      default:
        break;
    }
    if(tr.fraction!=1.0) {
      tmppen--;
      tmpVec[0]=tr.endpos[0]-start[0];
      tmpVec[1]=tr.endpos[1]-start[1];
      tmpVec[2]=tr.endpos[2]-start[2];
      tmplen=VectorLength(tmpVec);
      if(tmplen>=length) {
        tmpVec[0]=end[0]-vecSrc[0];
        tmpVec[1]=end[1]-vecSrc[1];
        tmpVec[2]=end[2]-vecSrc[2];
        length1=VectorLength(tmpVec);
        tmpVec[0]=vecEnd[0]-vecSrc[0];
        tmpVec[1]=vecEnd[1]-vecSrc[1];
        tmpVec[2]=vecEnd[2]-vecSrc[2];
        length2=VectorLength(tmpVec);
        tr.fraction=length1 / length2;
        distfrac=flDistance*tr.fraction;
        damage*=pow(wallpierce,distfrac*0.002);
        return (int)(damage > 0);
      }
      distfrac=flDistance*tr.fraction;
      damage*=pow(wallpierce,distfrac*0.002);
      if(distfrac>distancemax)tmppen=0;
      if(tmppen) {
        vecSrc[0]=bulletvelocity*vecDir[0]+tr.endpos[0];
        vecSrc[1]=bulletvelocity*vecDir[1]+tr.endpos[1];
        vecSrc[2]=bulletvelocity*vecDir[2]+tr.endpos[2];
        flDistance=(flDistance-distfrac)*0.5;
        vecEnd[0]=vecDir[0]*flDistance+vecSrc[0];
        vecEnd[1]=vecDir[1]*flDistance+vecSrc[1];
        vecEnd[2]=vecDir[2]*flDistance+vecSrc[2];
        distfrac=damage;
        damage=distfrac*damagemult;
      } else {
        vecSrc[0]=42.0*vecDir[0]+tr.endpos[0];
        vecSrc[1]=42.0*vecDir[1]+tr.endpos[1];
        vecSrc[2]=42.0*vecDir[2]+tr.endpos[2];
        flDistance=(flDistance-distfrac)*0.75;
        vecEnd[0]=vecDir[0]*flDistance+vecSrc[0];
        vecEnd[1]=vecDir[1]*flDistance+vecSrc[1];
        vecEnd[2]=vecDir[2]*flDistance+vecSrc[2];
        damage*=0.75;
      }
    } else tmppen=0;
  }
  return 0;
}
