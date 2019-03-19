#include <fstream>
#include "client.h"
#include "autoroute.h"
#include "weapons.h"

cAutoRute gAutoRoute;

#define FORWARD 1
#define BACKWARD -1

cAutoRute::cAutoRute() {
  firstGroup=-1;
  firstPoint=-1;
  beforeGroup=-1;
  beforePoint=-1;
  direction=FORWARD;
  runninginroute=false;
  isRecord=0;
  CalcRecordIndex=false;
  recordIndex=0;
}

void cAutoRute::MoveTo(float* point,usercmd_t* cmd) {
  if(cvar.viewroute) {
    float vec_to_target[3],viewangles[3];
    VectorSubtract(point,gMe.ent->origin,vec_to_target);
    VectorAngles(vec_to_target,viewangles);
    if(viewangles[1]>180)viewangles[1]-=360;
    cmd->viewangles[1]=viewangles[1];
    gEngfuncs.SetViewAngles(cmd->viewangles);
  }
  float fPointAng=atan2((point[1]-gMe.pmEyePos[1]),(point[0]-gMe.pmEyePos[0]));
  float fNeedAng=gMe.pmViewAngles[1]*M_PI/180.0f-fPointAng;
  cmd->forwardmove=cos(fNeedAng)*gMe.pmMaxSpeed;
  cmd->sidemove=sin(fNeedAng)*gMe.pmMaxSpeed;
}

void cAutoRute::FindLine(usercmd_t* cmd) {
  if(!gMe.alive||(gAimbot.HasTarget()&&(!gWeapon.IsKnife()||gAimbot.tDistance<2*gMe.pmMaxSpeed))) {
    beforeGroup=beforePoint=firstGroup=firstPoint=-1;
    runninginroute=false;
    return;
  }
  if(isRecord)
    RecordWayPoints();
  else {
    //for autoknife when running in the waypoint!
    if((fabs(cmd->forwardmove)>10||fabs(cmd->sidemove)>10)||(!gMe.inway&&!gMe.inspeed)) {
      beforeGroup=beforePoint=firstGroup=firstPoint=-1;
      runninginroute=false;
      return;
    }
    if(!runninginroute) {
      bool first=true;
      float nearDist=9999.0f,nearFov=180.0f;
      for(int i=0; i<=routes.ngroups; i++) {
        if(i!=beforeGroup&&routes.group[i].npoints>0) {
          for(int j=0; j<routes.group[i].npoints; j++) {
            if(gMe.PathFree(routes.group[i].point[j].vec)) {
              float dist=gMe.GetDistanceFrom(routes.group[i].point[j].vec);
              if(dist<gMe.pmMaxSpeed*4) {
                if(cvar.viewroute) {
                  float fov=gMe.CalcFovAngle(routes.group[i].point[j].vec);
                  if(fov<gMe.iFOV) {
                    if(first||sin(fov/M_R2D)*dist<sin(nearFov/M_R2D)*nearDist) {
                      firstGroup=i;
                      firstPoint=j;
                      nearDist=dist;
                      nearFov=fov;
                      first=false;
                    }
                  }
                } else {
                  if(dist<nearDist) {
                    firstGroup=i;
                    firstPoint=j;
                    nearDist=dist;
                  }
                }
              }
            }
          }
        }
      }
      if(beforeGroup!=-1&&beforePoint!=-1) {
        if(firstGroup==-1&&firstPoint==-1) {
          firstGroup=beforeGroup;
          firstPoint=beforePoint;
        }
        beforeGroup=beforePoint=-1;
      }
      if(firstGroup!=-1&&firstPoint!=-1) {
        if(firstPoint==0)
          direction=FORWARD;
        else if(firstPoint==routes.group[firstGroup].npoints-1)
          direction=BACKWARD;
        else {
          if(gMe.CalcFovAngle(routes.group[firstGroup].point[firstPoint+1].vec)<
              gMe.CalcFovAngle(routes.group[firstGroup].point[firstPoint-1].vec))
            direction=FORWARD;
          else
            direction=BACKWARD;
        }
        runninginroute=true;
      }
    }
    if(firstGroup!=-1&&firstPoint!=-1) {
      if(cvar.armaxdist<1||cvar.armaxdist>9){
        cvar.armaxdist=1;
      }
      MoveTo(routes.group[firstGroup].point[firstPoint].vec,cmd);
      if(gMe.GetDistanceFrom(routes.group[firstGroup].point[firstPoint].vec)<(gMe.pmMaxSpeed/cvar.armaxdist)) {
        if((direction==FORWARD&&firstPoint!=routes.group[firstGroup].npoints-1)||(direction==BACKWARD&&firstPoint!=0)) {
          firstPoint+=direction;
        } else {
          beforeGroup=firstGroup;
          beforePoint=firstPoint;
          firstGroup=firstPoint=-1;
          runninginroute=false;
        }
      }
    }
  }
}

void cAutoRute::DrawLines(bool bPoints) {
  if(cvar.speedmode&&(runninginroute||gMe.inspeed))return;
  for(int i=0; i<=routes.ngroups; i++) {
    if(routes.group[i].npoints>0) {
      for(int j=0; j<routes.group[i].npoints; j++) {
        if(gMe.GetDistanceFrom(routes.group[i].point[j].vec)<800) {
          float from[3];
          from[0]=routes.group[i].point[j].x;
          from[1]=routes.group[i].point[j].y;
          from[2]=routes.group[i].point[j].z-30;
          if(bPoints) {
            float vecScreen[2];
            if(CalcScreen(from,vecScreen)) {
              DrawHudString(false,(int)vecScreen[0],(int)vecScreen[1],170,170,170,".");
              if(j==0||j==routes.group[i].npoints-1)
                DrawHudString(false,(int)vecScreen[0],(int)vecScreen[1]+12,170,170,170,"%d",i);
            }
          } else {
            int beamindex=gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/laserbeam.spr");
            if(beamindex) {
              if(j!=routes.group[i].npoints-1) {
                float to[3];
                to[0]=routes.group[i].point[j+1].x;
                to[1]=routes.group[i].point[j+1].y;
                to[2]=routes.group[i].point[j+1].z-30;
                gEngfuncs.pEfxAPI->R_BeamPoints(from,to,beamindex,0.001f,2.4f,0,10,0,0,0,1,0,0);
              }
              gEngfuncs.pEfxAPI->R_BeamPoints(from,routes.group[i].point[j].vec,beamindex,0.001f,2.4f,0,10,0,0,0,(j==0||j==routes.group[i].npoints-1)?1:0,1,0);
            }
          }
        }
      }
    }
  }
}

void cAutoRute::Load(const char *mapname) {
  char buf[256];
  int i,j;
  float x,y,z;
  //wpt file
  routes.ngroups=0;
  strcpy(map_name,mapname);
  sprintf(file_name,"%s\\%s.wp",waypath,map_name);
  //init
  for(i=0; i<GROUP_MAX; i++)
    routes.group[i].npoints=0;
  //load
  if(FileExists(file_name)) {
    ifstream ifs(file_name,ifstream::in);
    while(ifs.good()) {
      if(ifs.getline(buf,256)) {
        if(buf[0]!='P')continue;
        if(sscanf(buf,"P(%d)(%d):%f %f %f",&i,&j,&x,&y,&z)) {
          if(i<GROUP_MAX&&j<POINT_MAX) {
            routes.group[i].point[j].x=x;
            routes.group[i].point[j].y=y;
            routes.group[i].point[j].z=z;
            routes.group[i].npoints++;
            if(routes.ngroups<i)
              routes.ngroups=i;
          }
        }
      }
    }
    ifs.close();
  }
}

void cAutoRute::Save() {
  char buf[256];
  int i,j;
  // check directory
  if(!DirectoryExist(waypath)) {
    CreateDirectory(waypath,NULL);
    return;
  }
  //save
  ofstream ofs(file_name,ofstream::out);
  if(ofs.good()) {
    ofs<<"//WayPoint File "<<map_name<<endl;
    ofs.flush();
    for(i=0; i<=GROUP_MAX; i++) {
      for(j=0; j<routes.group[i].npoints; j++) {
        sprintf(buf,"P(%d)(%d):%f %f %f\n",i,j,
                routes.group[i].point[j].x,
                routes.group[i].point[j].y,
                routes.group[i].point[j].z
               );
        ofs<<buf;
        ofs.flush();
      }
    }
    ofs.close();
  }
}

void cAutoRute::AddWayPoint(int index) {
  routes.group[index].point[routes.group[index].npoints].vec[0]=gMe.ent->origin[0];
  routes.group[index].point[routes.group[index].npoints].vec[1]=gMe.ent->origin[1];
  routes.group[index].point[routes.group[index].npoints].vec[2]=gMe.ent->origin[2];
  routes.group[index].npoints++;
  Save();
}

void cAutoRute::ClearWayPoint(int index) {
  if(!cvar.autoroute)return;
  if(index<GROUP_MAX) {
    routes.group[index].npoints=0;
    Save();
  }
}

void cAutoRute::ClearAllWayPoints() {
  if(!cvar.autoroute)return;
  routes.ngroups=0;
  for(int i=0; i<GROUP_MAX; i++)
    routes.group[i].npoints=0;
  Save();
}

void cAutoRute::RecordWayPoints() {
  if(!gMe.alive)return;
  if(!cvar.autoroute)return;
  if(CalcRecordIndex) {
    for(int i=0; i<GROUP_MAX; i++) {
      if(!routes.group[i].npoints) {
        recordIndex=i;
        break;
      }
    }
  }
  CalcRecordIndex=false;
  if(routes.group[recordIndex].npoints==0)
    AddWayPoint(recordIndex);
  else if(routes.group[recordIndex].npoints<POINT_MAX) {
    if(gMe.GetDistanceFrom(routes.group[recordIndex].point[routes.group[recordIndex].npoints-1].vec)>cvar.recorddist&&
        gMe.GetDistanceFrom(routes.group[recordIndex].point[routes.group[recordIndex].npoints-1].vec)<2*cvar.recorddist)
      AddWayPoint(recordIndex);
  } else
    CalcRecordIndex=true;
}
