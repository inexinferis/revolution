#include "nospread.h"
#include "interpreter.h"

cNoSpread gNoSpread;

pfnEventHook pEvent_usp,pEvent_mp5n,pEvent_ak47,pEvent_aug,pEvent_deagle,pEvent_g3sg1,pEvent_sg550,
             pEvent_glock18,pEvent_m249,pEvent_m3,pEvent_m4a1,pEvent_mac10,pEvent_p90,pEvent_p228,pEvent_awp,
             pEvent_scout,pEvent_sg552,pEvent_tmp,pEvent_fiveseven,pEvent_ump45,pEvent_xm1014,pEvent_elite_left,
             pEvent_elite_right,pEvent_galil,pEvent_famas,pEvent_knife;

static unsigned int glSeed=0;
static unsigned int seed_table[ 256 ] = {
  28985, 27138, 26457, 9451, 17764, 10909, 28790, 8716, 6361, 4853, 17798, 21977, 19643, 20662, 10834, 20103,
  27067, 28634, 18623, 25849, 8576, 26234, 23887, 18228, 32587, 4836, 3306, 1811, 3035, 24559, 18399, 315,
  26766, 907, 24102, 12370, 9674, 2972, 10472, 16492, 22683, 11529, 27968, 30406, 13213, 2319, 23620, 16823,
  10013, 23772, 21567, 1251, 19579, 20313, 18241, 30130, 8402, 20807, 27354, 7169, 21211, 17293, 5410, 19223,
  10255, 22480, 27388, 9946, 15628, 24389, 17308, 2370, 9530, 31683, 25927, 23567, 11694, 26397, 32602, 15031,
  18255, 17582, 1422, 28835, 23607, 12597, 20602, 10138, 5212, 1252, 10074, 23166, 19823, 31667, 5902, 24630,
  18948, 14330, 14950, 8939, 23540, 21311, 22428, 22391, 3583, 29004, 30498, 18714, 4278, 2437, 22430, 3439,
  28313, 23161, 25396, 13471, 19324, 15287, 2563, 18901, 13103, 16867, 9714, 14322, 15197, 26889, 19372, 26241,
  31925, 14640, 11497, 8941, 10056, 6451, 28656, 10737, 13874, 17356, 8281, 25937, 1661, 4850, 7448, 12744,
  21826, 5477, 10167, 16705, 26897, 8839, 30947, 27978, 27283, 24685, 32298, 3525, 12398, 28726, 9475, 10208,
  617, 13467, 22287, 2376, 6097, 26312, 2974, 9114, 21787, 28010, 4725, 15387, 3274, 10762, 31695, 17320,
  18324, 12441, 16801, 27376, 22464, 7500, 5666, 18144, 15314, 31914, 31627, 6495, 5226, 31203, 2331, 4668,
  12650, 18275, 351, 7268, 31319, 30119, 7600, 2905, 13826, 11343, 13053, 15583, 30055, 31093, 5067, 761,
  9685, 11070, 21369, 27155, 3663, 26542, 20169, 12161, 15411, 30401, 7580, 31784, 8985, 29367, 20989, 14203,
  29694, 21167, 10337, 1706, 28578, 887, 3373, 19477, 14382, 675, 7033, 15111, 26138, 12252, 30996, 21409,
  25678, 18555, 13256, 23316, 22407, 16727, 991, 9236, 5373, 29402, 6117, 15241, 27715, 19291, 19888, 19847
};

unsigned int cNoSpread::U_Random( void ) {
  glSeed*=69069;
  glSeed+=seed_table[glSeed&0xff];
  return (++glSeed&0x0fffffff);
}

void cNoSpread::U_Srand( unsigned int seed ) {
  glSeed=seed_table[seed&0xff];
}

int cNoSpread::UTIL_SharedRandomLong(unsigned int seed, int low, int high) {
  unsigned int range;
  U_Srand((int)seed+low+high);
  range=high-low+1;
  if(!(range-1))
    return low;
  else {
    int rnum=U_Random();
    int offset=rnum%range;
    return (low+offset);
  }
}

float cNoSpread::UTIL_SharedRandomFloat( unsigned int seed, float low, float high) {
  unsigned int range;
  U_Srand((int)seed+*(int*)&low+*(int*)&high);
  U_Random();
  U_Random();
  range=(unsigned int)(high-low);
  if(!range)
    return low;
  else {
    int tensixrand;
    float offset;
    tensixrand=U_Random()&65535;
    offset=(float)tensixrand/65536.0f;
    return (low+offset*range );
  }
}

float cNoSpread::GetVecSpread(float speed) {
  float spread;
  switch(gWeapon.GetId()) {
    case WEAPON_DEAGLE:
      if(prcflags&FL_ONGROUND) {
        if(speed)
          spread=0.25*(1.0f-spreadvar);
        else if(prcflags&FL_DUCKING)
          spread=0.115*(1.0f-spreadvar);
        else
          spread=0.13*(1.0f-spreadvar);
      } else
        spread=1.5*(1.0f-spreadvar);
      break;
    case WEAPON_ELITE:
      if(prcflags&FL_ONGROUND) {
        if(speed)
          spread=0.175*(1.0f-spreadvar);
        else if(prcflags&FL_DUCKING)
          spread=0.08*(1.0f-spreadvar);
        else
          spread=0.1*(1.0f-spreadvar);
      } else
        spread=1.3*(1.0f-spreadvar);
      break;
    case WEAPON_GALIL:
      if(prcflags&FL_ONGROUND) {
        if(speed>140.0f)
          spread=spreadvar*0.07+0.04;
        else
          spread=spreadvar*0.0375;
      } else
        spread=spreadvar*0.4+0.03;
      break;
    case WEAPON_FAMAS:
      if(prcflags&FL_ONGROUND) {
        if(speed>140.0f)
          spread=spreadvar*0.07+0.03;
        else
          spread=spreadvar*0.02;
      } else
        spread=spreadvar*0.4+.04;
      break;
    case WEAPON_GLOCK18:
      if(!gWeapon.IsInBurst()) {
        if(prcflags&FL_ONGROUND) {
          if(speed)
            spread=0.165*(1.0f-spreadvar);
          else if(prcflags&FL_DUCKING)
            spread=0.075*(1.0f-spreadvar);
          else
            spread=0.1*(1.0f-spreadvar);
        } else
          spread=1.0f-spreadvar;
      } else {
        if(prcflags&FL_ONGROUND) {
          if(speed)
            spread=0.185*(1.0f-spreadvar);
          else if(prcflags&FL_DUCKING)
            spread=0.095*(1.0f-spreadvar);
          else
            spread=0.3*(1.0f-spreadvar);
        } else
          spread=1.2*(1.0f-spreadvar);
      }
      break;
    case WEAPON_P228:
    case WEAPON_FIVESEVEN:
      if(prcflags&FL_ONGROUND) {
        if(speed)
          spread=0.255*(1.0f-spreadvar);
        else if(prcflags&FL_DUCKING)
          spread=0.075*(1.0f-spreadvar);
        else
          spread=0.15*(1.0f-spreadvar);
      } else
        spread=1.5*(1.0f-spreadvar);
      break;
    case WEAPON_G3SG1:
      if(prcflags&FL_ONGROUND) {
        if(speed)
          spread=0.15f;
        else if(prcflags&FL_DUCKING)
          spread=0.035*(1.0f-spreadvar);
        else
          spread=0.055*(1.0f-spreadvar);
      } else
        spread=0.45*(1.0f-spreadvar);
      if(!(gMe.iFOV<90.0f))
        spread+=0.025f;
      break;
    case WEAPON_SG550:
      if(prcflags&FL_ONGROUND) {
        if(speed)
          spread=0.15f;
        else if(prcflags&FL_DUCKING)
          spread=0.04*(1.0f-spreadvar);
        else
          spread=0.05*(1.0f-spreadvar);
      } else
        spread=0.45*(1.0f-spreadvar);
      if(!(gMe.iFOV<90.0f))
        spread+=0.025f;
      break;
    case WEAPON_USP:
      if(!gWeapon.IsSilenced()) {
        if(prcflags&FL_ONGROUND) {
          if(speed)
            spread=0.225*(1.0f-spreadvar);
          else if(prcflags&FL_DUCKING)
            spread=0.08*(1.0f-spreadvar);
          else
            spread=0.1*(1.0f-spreadvar);
        } else
          spread=1.2*(1.0f-spreadvar);
      } else {
        if(prcflags&FL_ONGROUND) {
          if(speed)
            spread=0.25*(1.0f-spreadvar);
          else if(prcflags&FL_DUCKING)
            spread=0.125*(1.0f-spreadvar);
          else
            spread=0.15*(1.0f-spreadvar);
        } else
          spread=1.3*(1.0f-spreadvar);
      }
      break;
    case WEAPON_AK47:
      if(prcflags&FL_ONGROUND) {
        if(speed>140.0f)
          spread=spreadvar*0.07+0.04;
        else
          spread=spreadvar*0.0275;
      } else
        spread=spreadvar*0.4+.04;
      break;
    case WEAPON_SG552:
      if(prcflags&FL_ONGROUND) {
        if(speed>140.0f)
          spread=spreadvar*0.07+0.035;
        else
          spread=spreadvar*0.02;
      } else
        spread=spreadvar*0.4+.035;
      break;
    case WEAPON_AUG:
      if(prcflags&FL_ONGROUND) {
        if(speed>140.0f)
          spread=spreadvar*0.07+0.035;
        else
          spread=spreadvar*0.02;
      } else
        spread=spreadvar*0.4+.035;
      break;
    case WEAPON_M249:
      if(prcflags&FL_ONGROUND) {
        if(speed>140.0f)
          spread=spreadvar*0.095+0.045;
        else
          spread=spreadvar*0.03;
      } else
        spread=spreadvar*0.5+.045;
      break;
    case WEAPON_M4A1:
      if(prcflags&FL_ONGROUND) {
        if(!gWeapon.IsSilenced()) {
          if(speed>140.0f)
            spread=spreadvar*0.07+0.035;
          else
            spread=spreadvar*0.02;
        } else {
          if(speed>140.0f)
            spread=spreadvar*0.07+0.035;
          else
            spread=spreadvar*0.025;
        }
      } else
        spread=spreadvar*0.4+.035;
      break;
    case WEAPON_MP5:
      if(prcflags&FL_ONGROUND)
        spread=0.04*spreadvar;
      else
        spread=0.2*spreadvar;
      break;
    case WEAPON_MAC10:
      if(prcflags&FL_ONGROUND)
        spread=0.03*spreadvar;
      else
        spread=0.375*spreadvar;
      break;
    case WEAPON_P90:
      if(prcflags&FL_ONGROUND) {
        if(speed>170.0f)
          spread=spreadvar*0.115;
        else
          spread=spreadvar*0.045;
      } else
        spread=spreadvar*0.3;
      break;
    case WEAPON_TMP:
      if(prcflags&FL_ONGROUND)
        spread=0.03*spreadvar;
      else
        spread=0.25*spreadvar;
      break;
    case WEAPON_UMP45:
      if(prcflags&FL_ONGROUND)
        spread=0.04*spreadvar;
      else
        spread=0.24*spreadvar;
      break;
    case WEAPON_AWP:
      if(prcflags&FL_ONGROUND) {
        if(speed<10.0f) {
          if(prcflags&FL_DUCKING)
            spread=0;
          else
            spread=0.001f;
        } else if(speed<140.0f) {
          spread=0.1f;
        } else
          spread=0.25f;
      } else
        spread=0.85f;
      if(!(gMe.iFOV<90.0f))
        spread+=0.08f;
      break;
    case WEAPON_SCOUT:
      if(prcflags&FL_ONGROUND) {
        if(speed<170.0f) {
          if(prcflags&FL_DUCKING)
            spread=0;
          else
            spread=0.007f;
        } else
          spread=0.075f;
      } else
        spread=0.2f;
      if(!(gMe.iFOV<90.0f))
        spread+=0.025f;
      break;
    default:
      spread=0;
      break;
  }
  return spread;
}

void cNoSpread::GetSpreadXY(int shared_rand, int future, float *velocity, float *vec) {
  //float vecspread;
  vec[0]=UTIL_SharedRandomFloat(shared_rand+future,-0.5,0.5)+UTIL_SharedRandomFloat(shared_rand+1+future,-0.5,0.5);
  vec[1]=UTIL_SharedRandomFloat(shared_rand+2+future,-0.5,0.5)+UTIL_SharedRandomFloat(shared_rand+3+future,-0.5,0.5);
  vecspread=GetVecSpread(sqrt(velocity[0]*velocity[0]+velocity[1]*velocity[1]));
  vec[0]*=vecspread;
  vec[1]*=vecspread;
}

void cNoSpread::GetSpreadOffset(float *outangles) {
  float forward[3],right[3],up[3],vecDir[3];
  float view[3],dest[3],spread[2];
  gEngfuncs.pfnAngleVectors(outangles,forward,right,up);
  GetSpreadXY(grandom_seed,1,gMe.pmVelocity,spread);
  vecDir[0]=forward[0]+spread[0]*right[0]+spread[1]*up[0];
  view[0]=8192*vecDir[0];
  vecDir[1]=forward[1]+spread[0]*right[1]+spread[1]*up[1];
  view[1]=8192*vecDir[1];
  vecDir[2]=forward[2]+spread[0]*right[2]+spread[1]*up[2];
  view[2]=8192*vecDir[2];
  VectorAngles(view,dest);
  dest[0]*=-1;
  outangles[0]+=outangles[0]-dest[0];
  outangles[1]+=outangles[1]-dest[1];
  outangles[2]=0;
}

void cNoSpread::GetRecoilOffset(float *outangles) {
  switch((int)cvar.norecoil) {
    case 3://old...
      outangles[0]-=2*gMe.punchangle[0]*cvar.recoilinc;
      outangles[1]-=2*gMe.punchangle[1]*cvar.recoilinc;
      break;
    case 2://frametime based...
      float Punch[3], Length;
      VectorCopy(gMe.punchangle,Punch);
      Length=VectorNormalize(Punch);
      Length-=(10.0+Length*0.5)*gMe.pmFrameTime;
      Length=fmax(Length,0.0);
      VectorScale(Punch,Length,Punch);
      outangles[0]-=2*Punch[0]*cvar.recoilinc;
      outangles[1]-=2*Punch[1]*cvar.recoilinc;
      break;
    default://weapon based... (0 [speedmode] o 1)
      outangles[0]-=2*gMe.punchangle[0]*xrecoil*cvar.recoilinc;
      outangles[1]-=2*gMe.punchangle[1]*yrecoil*cvar.recoilinc;
      break;
  }
}

void cNoSpread::DefaultSpreadVar(int weaponid) {
  if(weaponid==WEAPON_DEAGLE)
    spreadvar=0.9f;
  else if(weaponid==WEAPON_MP5)
    spreadvar=0.0f;
  else if(weaponid==WEAPON_AK47)
    spreadvar=0.2f;
  else if(weaponid==WEAPON_SG552)
    spreadvar=0.2f;
  else if(weaponid==WEAPON_AUG)
    spreadvar=0.3f;
  else if(weaponid==WEAPON_M4A1)
    spreadvar=0.2f;
  else if(weaponid==WEAPON_M249)
    spreadvar=0.2f;
  else if(weaponid==WEAPON_MAC10)
    spreadvar=0.15f;
  else if(weaponid==WEAPON_UMP45)
    spreadvar=0;
  else if(weaponid==WEAPON_TMP)
    spreadvar=0.2f;
  else if(weaponid==WEAPON_P90)
    spreadvar=0.15f;
  else if(weaponid==WEAPON_P228)
    spreadvar=0.9f;
  else if(weaponid==WEAPON_FIVESEVEN)
    spreadvar=0.92f;
  else if(weaponid==WEAPON_ELITE)
    spreadvar=0.88f;
  else if(weaponid==WEAPON_GLOCK18)
    spreadvar=0.9f;
  else if(weaponid==WEAPON_USP)
    spreadvar=0.92f;
  else if(weaponid==WEAPON_G3SG1)
    spreadvar=0.2f;
  else if(weaponid==WEAPON_SG550)
    spreadvar=0.2f;
  else
    spreadvar=0;
}

void cNoSpread::HUD_PostRunCmd(struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed) {
  unsigned int i;
  prcflags=to->client.flags;
  grandom_seed=random_seed;
  gtime=time;
  //old mode (mix)
  if(!cvar.eventnospread&&cmd->buttons&IN_ATTACK&&gWeapon.CanAttack())
    PrimaryAttack(to->client.m_iId);
  else
    //no more attaking? min recoil
    if(!(cmd->buttons&(IN_ATTACK|IN_ATTACK2))) {
      if(firing) {
        firing=false;
        if(recoil>15)
          recoil=15;
        recoiltime=time+0.4f;
      }
      if(gWeapon.IsSecondary())
        recoil=0;
      else if(recoil>0) {
        if(recoiltime<=time) {
          recoiltime+=0.0225f;
          recoil--;
        }
      }
    }

  //change weapon?
  for(i=0; i<vWeapons.size(); i++) {
    if(vWeapons[i].Id==to->client.m_iId) {
      if(!vWeapons[i].CurWeapon) {
        DefaultSpreadVar(vWeapons[i].Id);
        vWeapons[i].CurWeapon=true;
        recoiltime=time;
        firing=true;
        prevtime=0;
        recoil=0;
        //weapon alias...
        char wname[128];
        sprintf(wname,"w_%s",gWeapon.GetNameByModel(gMe.ent->curstate.weaponmodel));
        ::cmd.exec(wname);
      }
    } else
      vWeapons[i].CurWeapon=false;
  }

  //reloading?
  if(gWeapon.IsReloading()) {
    DefaultSpreadVar(to->client.m_iId);
    recoiltime=time;
    firing=false;
    prevtime=0;
    recoil=0;
  }

  //update weapon data...
  for(i=0; i<MAX_WEAPONS; i++) {
    int index=gWeapon.GetIndexById(i);
    if(index!=-1) {
      if(to->weapondata[i].m_iId)
        gWeapon.SetData(index,&to->weapondata[i]);
      else
        gWeapon.ClearData(index);
    }
  }
}

float GetEngineSpread(int wId) {
  if(pGetBaseGun) {
    PBYTE pGun=pGetBaseGun(wId);
    if(pGun)
      return *(float*)(pGun+0xF8);
  }
  return 0.0f;
}

void cNoSpread::PrimaryAttack(int id) {
  switch (id) {
    case WEAPON_DEAGLE:
      if(prevtime) {
        if(cvar.nospread==2&&pGetBaseGun)
          spreadvar=GetEngineSpread(gWeapon.GetId());
        else {
          spreadvar=spreadvar-(0.35*(0.4-(gtime-prevtime)));
          if(spreadvar>0.9)
            spreadvar=0.9f;
          else if(spreadvar<0.55)
            spreadvar=0.55f;
        }
      }
      recoil++;
      prevtime=gtime;
      break;
    case WEAPON_ELITE:
      if(prevtime) {
        if(cvar.nospread==2&&pGetBaseGun)
          spreadvar=GetEngineSpread(gWeapon.GetId());
        else {
          spreadvar=spreadvar-(0.275*(0.325-(gtime-prevtime)));
          if(spreadvar>0.88)
            spreadvar=0.88f;
          else if(spreadvar<0.55)
            spreadvar=0.55f;
        }
      }
      recoil++;
      prevtime=gtime;
      break;
    case WEAPON_GALIL:
      recoil++;
      firing=true;
      if(cvar.nospread==2&&pGetBaseGun)
        spreadvar=GetEngineSpread(gWeapon.GetId());
      else {
        spreadvar=((float)recoil*recoil*recoil)/200.0f+0.35f;
        if(spreadvar>1.25)
          spreadvar=1.25f;
      }
      prevtime=gtime;
      break;
    case WEAPON_FAMAS:
      recoil++;
      firing=true;
      if(cvar.nospread==2&&pGetBaseGun)
        spreadvar=GetEngineSpread(gWeapon.GetId());
      else {
        spreadvar=((float)recoil*recoil*recoil)/215.0f+0.3f;
        if(spreadvar>1.0)
          spreadvar=1.0f;
      }
      prevtime=gtime;
      break;
    case WEAPON_FIVESEVEN:
      if(prevtime) {
        if(cvar.nospread==2&&pGetBaseGun)
          spreadvar=GetEngineSpread(gWeapon.GetId());
        else {
          spreadvar=spreadvar-(0.25*(0.275-(gtime-prevtime)));
          if(spreadvar>0.92)
            spreadvar=0.92f;
          else if(spreadvar<0.725)
            spreadvar=0.725f;
        }
      }
      recoil++;
      prevtime=gtime;
      break;
    case WEAPON_GLOCK18:
      if(!gWeapon.IsInBurst())
        recoil++;
      if(prevtime) {
        if(cvar.nospread==2&&pGetBaseGun)
          spreadvar=GetEngineSpread(gWeapon.GetId());
        else {
          spreadvar=spreadvar-(0.275*(0.325-(gtime-prevtime)));
          if(spreadvar>0.9)
            spreadvar=0.9f;
          else if(spreadvar<0.6)
            spreadvar=0.6f;
        }
      }
      prevtime=gtime;
      break;
    case WEAPON_P228:
      if(prevtime) {
        if(cvar.nospread==2&&pGetBaseGun)
          spreadvar=GetEngineSpread(gWeapon.GetId());
        else {
          spreadvar=spreadvar-(0.3*(0.325-(gtime-prevtime)));
          if(spreadvar>0.9)
            spreadvar=0.9f;
          else if(spreadvar<0.6)
            spreadvar=0.6f;
        }
      }
      recoil++;
      prevtime=gtime;
      break;
    case WEAPON_G3SG1:
      if(prevtime) {
        if(cvar.nospread==2&&pGetBaseGun)
          spreadvar=GetEngineSpread(gWeapon.GetId());
        else {
          spreadvar=0.55+(0.3*(gtime-prevtime));
          if(spreadvar>0.98)
            spreadvar=0.98f;
        }
      }
      recoil++;
      prevtime=gtime;
      firing=true;
      break;
    case WEAPON_SG550:
      if(prevtime) {
        if(cvar.nospread==2&&pGetBaseGun)
          spreadvar=GetEngineSpread(gWeapon.GetId());
        else {
          spreadvar=0.65+(0.35*(gtime-prevtime));
          if(spreadvar>0.98)
            spreadvar=0.98f;
        }
      }
      recoil++;
      prevtime=gtime;
      firing=true;
      break;
    case WEAPON_USP:
      if(prevtime) {
        if(cvar.nospread==2&&pGetBaseGun)
          spreadvar=GetEngineSpread(gWeapon.GetId());
        else {
          spreadvar=spreadvar-(0.275*(0.3-(gtime-prevtime)));
          if(spreadvar>0.92)
            spreadvar=0.92f;
          else if(spreadvar<0.6)
            spreadvar=0.6f;
        }
      }
      recoil++;
      prevtime=gtime;
      break;
    case WEAPON_AK47:
      recoil++;
      firing=true;
      if(cvar.nospread==2&&pGetBaseGun)
        spreadvar=GetEngineSpread(gWeapon.GetId());
      else {
        spreadvar=recoil*recoil*recoil/200.0f+0.35;
        if(spreadvar>1.25)
          spreadvar=1.25f;
      }
      prevtime=gtime;
      break;
    case WEAPON_SG552:
      recoil++;
      firing=true;
      if(cvar.nospread==2&&pGetBaseGun)
        spreadvar=GetEngineSpread(gWeapon.GetId());
      else {
        spreadvar=recoil*recoil*recoil/220.0f+0.3;
        if(spreadvar>1)
          spreadvar=1.0f;
      }
      prevtime=gtime;
      break;
    case WEAPON_AUG:
      recoil++;
      firing=true;
      if(cvar.nospread==2&&pGetBaseGun)
        spreadvar=GetEngineSpread(gWeapon.GetId());
      else {
        spreadvar=recoil*recoil*recoil/215.0f+0.3;
        if(spreadvar>1.0)
          spreadvar=1.0f;
      }
      prevtime=gtime;
      break;
    case WEAPON_M249:
      recoil++;
      firing=true;
      if(cvar.nospread==2&&pGetBaseGun)
        spreadvar=GetEngineSpread(gWeapon.GetId());
      else {
        spreadvar=recoil*recoil*recoil/175.0f+0.4;
        if(spreadvar>0.9)
          spreadvar=0.9f;
      }
      prevtime=gtime;
      break;
    case WEAPON_M4A1:
      recoil++;
      firing=true;
      if(cvar.nospread==2&&pGetBaseGun)
        spreadvar=GetEngineSpread(gWeapon.GetId());
      else {
        spreadvar=recoil*recoil*recoil/220.0f+0.3;
        if(spreadvar>1)
          spreadvar=1.0f;
      }
      prevtime=gtime;
      break;
    case WEAPON_MP5:
      recoil++;
      if(cvar.nospread==2&&pGetBaseGun)
        spreadvar=GetEngineSpread(gWeapon.GetId());
      else {
        spreadvar=recoil*recoil*recoil/220.1f+0.45;
        if(spreadvar>0.75)
          spreadvar=0.75f;
      }
      prevtime=gtime;
      firing=true;
      break;
    case WEAPON_MAC10:
      recoil++;
      firing=true;
      if(cvar.nospread==2&&pGetBaseGun)
        spreadvar=GetEngineSpread(gWeapon.GetId());
      else {
        spreadvar=recoil*recoil*recoil/200.0f+0.6;
        if(spreadvar>1.65)
          spreadvar=1.65f;
      }
      prevtime=gtime;
      break;
    case WEAPON_P90:
      recoil++;
      firing=true;
      if(cvar.nospread==2&&pGetBaseGun)
        spreadvar=GetEngineSpread(gWeapon.GetId());
      else {
        spreadvar=recoil*recoil/175.0f+0.45;
        if(spreadvar>1)
          spreadvar=1.0f;
      }
      prevtime=gtime;
      break;
    case WEAPON_TMP:
      recoil++;
      firing=true;
      if(cvar.nospread==2&&pGetBaseGun)
        spreadvar=GetEngineSpread(gWeapon.GetId());
      else {
        spreadvar=recoil*recoil*recoil/200.0f+0.55;
        if(spreadvar>1.4)
          spreadvar=1.4f;
      }
      prevtime=gtime;
      break;
    case WEAPON_UMP45:
      recoil++;
      firing=true;
      if(cvar.nospread==2&&pGetBaseGun)
        spreadvar=GetEngineSpread(gWeapon.GetId());
      else {
        spreadvar=recoil*recoil/210.0f+0.5;
        if(spreadvar>1.0)
          spreadvar=1.0f;
      }
      prevtime=gtime;
      break;
    case WEAPON_AWP:
      recoil++;
      prevtime=gtime;
      firing=true;
      break;
    case WEAPON_SCOUT:
      recoil++;
      prevtime=gtime;
      firing=true;
      break;
    case WEAPON_XM1014:
      recoil++;
      prevtime=gtime;
      firing=true;
      break;
    case WEAPON_M3:
      recoil++;
      prevtime=gtime;
      firing=true;
      break;
    default:
      break;
  }
  return;
}

//new method (event based)

void cNoSpread::UpdateWeaponRecoil(struct event_args_s *args) {
  xrecoil=0;
  yrecoil=0;
  if(recoil&&(gMe.punchangle[0]!=0.0||gMe.punchangle[1]!=0.0)) {
    float pitch,yaw;
    int WeaponID=gWeapon.GetId();
    if(WeaponID==WEAPON_FAMAS||WeaponID==WEAPON_GALIL) {
      pitch=((float)args->iparam1/10000000.0f);
      yaw=((float)args->iparam2/10000000.0f);
    } else {
      pitch=((float)args->iparam1/100.0f);
      yaw=((float)args->iparam2/100.0f);
    }
    if(pitch||yaw) {
      float multiplier=(float)recoil/(float)(recoil+1);
      if(multiplier>0.96f)
        multiplier=0.96f;
      if(gMe.punchangle[0]&&pitch) {
        xrecoil=pitch*multiplier/gMe.punchangle[0];
        xrecoil*=xrecoil;
      }
      if(gMe.punchangle[1]&&yaw) {
        yrecoil=yaw*multiplier/gMe.punchangle[1];
        yrecoil*=yrecoil;
      }
    }
  }
}

void Event_deagle(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      if(gNoSpread.prevtime){
        if(cvar.nospread==2&&pGetBaseGun)
          gNoSpread.spreadvar=GetEngineSpread(gWeapon.GetId());
        else{
          gNoSpread.spreadvar=gNoSpread.spreadvar-(0.35*(0.4-(gNoSpread.gtime-gNoSpread.prevtime)));
          if(gNoSpread.spreadvar>0.9f)
            gNoSpread.spreadvar=0.9f;
          else if(gNoSpread.spreadvar<0.55f)
            gNoSpread.spreadvar=0.55f;
        }
      }
      gNoSpread.recoil++;
      gNoSpread.prevtime=gNoSpread.gtime;
    }
    gNoSpread.UpdateWeaponRecoil(args);
  }
  pEvent_deagle(args);
}

void Event_elite_left(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      if(gNoSpread.prevtime){
        if(cvar.nospread==2&&pGetBaseGun)
          gNoSpread.spreadvar=GetEngineSpread(gWeapon.GetId());
        else{
          gNoSpread.spreadvar=gNoSpread.spreadvar-(0.275*(0.325-(gNoSpread.gtime-gNoSpread.prevtime)));
          if(gNoSpread.spreadvar>0.88)
            gNoSpread.spreadvar=0.88f;
          else if(gNoSpread.spreadvar<0.55)
            gNoSpread.spreadvar=0.55f;
        }
      }
      gNoSpread.recoil++;
      gNoSpread.prevtime=gNoSpread.gtime;
    }
    gNoSpread.UpdateWeaponRecoil(args);
  }
  pEvent_elite_left(args);
}

void Event_elite_right(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      if(gNoSpread.prevtime){
        if(cvar.nospread==2&&pGetBaseGun)
          gNoSpread.spreadvar=GetEngineSpread(gWeapon.GetId());
        else{
          gNoSpread.spreadvar=gNoSpread.spreadvar-(0.275*(0.325-(gNoSpread.gtime-gNoSpread.prevtime)));
          if(gNoSpread.spreadvar>0.88)
            gNoSpread.spreadvar=0.88f;
          else if(gNoSpread.spreadvar<0.55)
            gNoSpread.spreadvar=0.55f;
        }
      }
      gNoSpread.recoil++;
      gNoSpread.prevtime=gNoSpread.gtime;
    }
    gNoSpread.UpdateWeaponRecoil(args);
  }
  pEvent_elite_right(args);
}

void Event_galil(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      gNoSpread.recoil++;
      gNoSpread.firing=true;
      if(cvar.nospread==2&&pGetBaseGun)
        gNoSpread.spreadvar=GetEngineSpread(gWeapon.GetId());
      else{
        gNoSpread.spreadvar=gNoSpread.recoil*gNoSpread.recoil*gNoSpread.recoil/200.0f+0.35f;
        if(gNoSpread.spreadvar>1.25)
          gNoSpread.spreadvar=1.25f;
      }
      gNoSpread.prevtime=gNoSpread.gtime;
    }
    gNoSpread.UpdateWeaponRecoil(args);
  }
  pEvent_galil(args);
}

void Event_famas(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      gNoSpread.recoil++;
      gNoSpread.firing=true;
      if(cvar.nospread==2&&pGetBaseGun)
        gNoSpread.spreadvar=GetEngineSpread(gWeapon.GetId());
      else{
        gNoSpread.spreadvar=gNoSpread.recoil*gNoSpread.recoil*gNoSpread.recoil/215.0f+0.3f;
        if(gNoSpread.spreadvar>1.0)
          gNoSpread.spreadvar=1.0f;
      }
      gNoSpread.prevtime=gNoSpread.gtime;
    }
    gNoSpread.UpdateWeaponRecoil(args);
  }
  pEvent_famas(args);
}

void Event_fiveseven(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      if(gNoSpread.prevtime){
        if(cvar.nospread==2&&pGetBaseGun)
          gNoSpread.spreadvar=GetEngineSpread(gWeapon.GetId());
        else{
          gNoSpread.spreadvar=gNoSpread.spreadvar-(0.25*(0.275-(gNoSpread.gtime-gNoSpread.prevtime)));
          if(gNoSpread.spreadvar>0.92)
            gNoSpread.spreadvar=0.92f;
          else if(gNoSpread.spreadvar<0.725)
            gNoSpread.spreadvar=0.725f;
        }
      }
      gNoSpread.recoil++;
      gNoSpread.prevtime=gNoSpread.gtime;
    }
    gNoSpread.UpdateWeaponRecoil(args);
  }
  pEvent_fiveseven(args);
}

void Event_glock18(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      if(gNoSpread.prevtime){
        if(cvar.nospread==2&&pGetBaseGun)
          gNoSpread.spreadvar=GetEngineSpread(gWeapon.GetId());
        else{
          gNoSpread.spreadvar=gNoSpread.spreadvar-(0.275*(0.325-(gNoSpread.gtime-gNoSpread.prevtime)));
          if(gNoSpread.spreadvar>0.9)
            gNoSpread.spreadvar=0.9f;
          else if(gNoSpread.spreadvar<0.6)
            gNoSpread.spreadvar=0.6f;
        }
      }
      gNoSpread.recoil++;
      gNoSpread.prevtime=gNoSpread.gtime;
    }
    gNoSpread.UpdateWeaponRecoil(args);
  }
  pEvent_glock18(args);
}

void Event_p228(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      if(gNoSpread.prevtime){
        if(cvar.nospread==2&&pGetBaseGun)
          gNoSpread.spreadvar=GetEngineSpread(gWeapon.GetId());
        else{
          gNoSpread.spreadvar=gNoSpread.spreadvar-(0.3*(0.325-(gNoSpread.gtime-gNoSpread.prevtime)));
          if(gNoSpread.spreadvar>0.9)
            gNoSpread.spreadvar=0.9f;
          else if(gNoSpread.spreadvar<0.6)
            gNoSpread.spreadvar=0.6f;
        }
      }
      gNoSpread.recoil++;
      gNoSpread.prevtime=gNoSpread.gtime;
    }
    gNoSpread.UpdateWeaponRecoil(args);
  }
  pEvent_p228(args);
}

void Event_g3sg1(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      if(gNoSpread.prevtime){
        if(cvar.nospread==2&&pGetBaseGun)
          gNoSpread.spreadvar=GetEngineSpread(gWeapon.GetId());
        else{
          gNoSpread.spreadvar=0.55+(0.3*(gNoSpread.gtime-gNoSpread.prevtime));
          if(gNoSpread.spreadvar>0.98)
            gNoSpread.spreadvar=0.98f;
        }
      }
      gNoSpread.recoil++;
      gNoSpread.prevtime=gNoSpread.gtime;
      gNoSpread.firing=true;
    }
    gNoSpread.UpdateWeaponRecoil(args);
  }
  pEvent_g3sg1(args);
}

void Event_sg550(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      if(gNoSpread.prevtime){
        if(cvar.nospread==2&&pGetBaseGun)
          gNoSpread.spreadvar=GetEngineSpread(gWeapon.GetId());
        else{
          gNoSpread.spreadvar=0.65+(0.35*(gNoSpread.gtime-gNoSpread.prevtime));
          if(gNoSpread.spreadvar>0.98)
            gNoSpread.spreadvar=0.98f;
        }
      }
      gNoSpread.recoil++;
      gNoSpread.prevtime=gNoSpread.gtime;
      gNoSpread.firing=true;
    }
    gNoSpread.UpdateWeaponRecoil(args);
  }
  pEvent_sg550(args);
}

void Event_usp(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      if(gNoSpread.prevtime){
        if(cvar.nospread==2&&pGetBaseGun)
          gNoSpread.spreadvar=GetEngineSpread(gWeapon.GetId());
        else{
          gNoSpread.spreadvar=gNoSpread.spreadvar-(0.275*(0.3-(gNoSpread.gtime-gNoSpread.prevtime)));
          if(gNoSpread.spreadvar>0.92)
            gNoSpread.spreadvar=0.92f;
          else if(gNoSpread.spreadvar<0.6)
            gNoSpread.spreadvar=0.6f;
        }
      }
      gNoSpread.recoil++;
      gNoSpread.prevtime=gNoSpread.gtime;
    }
    gNoSpread.UpdateWeaponRecoil(args);
  }
  pEvent_usp(args);
}

void Event_ak47(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      gNoSpread.recoil++;
      gNoSpread.firing=true;
      if(cvar.nospread==2&&pGetBaseGun)
        gNoSpread.spreadvar=GetEngineSpread(gWeapon.GetId());
      else{
        gNoSpread.spreadvar=gNoSpread.recoil*gNoSpread.recoil*gNoSpread.recoil/200.0f+0.35;
        if(gNoSpread.spreadvar>1.25)
          gNoSpread.spreadvar=1.25f;
      }
      gNoSpread.prevtime=gNoSpread.gtime;
    }
    gNoSpread.UpdateWeaponRecoil(args);
  }
  pEvent_ak47(args);
}

void Event_sg552(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      gNoSpread.recoil++;
      gNoSpread.firing=true;
      if(cvar.nospread==2&&pGetBaseGun)
        gNoSpread.spreadvar=GetEngineSpread(gWeapon.GetId());
      else{
        gNoSpread.spreadvar=gNoSpread.recoil*gNoSpread.recoil*gNoSpread.recoil/220+0.3;
        if(gNoSpread.spreadvar>1)
          gNoSpread.spreadvar=1.0f;
      }
      gNoSpread.prevtime=gNoSpread.gtime;
    }
    gNoSpread.UpdateWeaponRecoil(args);
  }
  pEvent_sg552(args);
}

void Event_aug(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      gNoSpread.recoil++;
      gNoSpread.firing=true;
      if(cvar.nospread==2&&pGetBaseGun)
        gNoSpread.spreadvar=GetEngineSpread(gWeapon.GetId());
      else{
        gNoSpread.spreadvar=gNoSpread.recoil*gNoSpread.recoil*gNoSpread.recoil/215.0f+0.3;
        if(gNoSpread.spreadvar>1.0)
          gNoSpread.spreadvar=1.0f;
      }
      gNoSpread.prevtime=gNoSpread.gtime;
    }
    gNoSpread.UpdateWeaponRecoil(args);
  }
  pEvent_aug(args);
}

void Event_m249(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      gNoSpread.recoil++;
      gNoSpread.firing=true;
      if(cvar.nospread==2&&pGetBaseGun)
        gNoSpread.spreadvar=GetEngineSpread(gWeapon.GetId());
      else{
        gNoSpread.spreadvar=gNoSpread.recoil*gNoSpread.recoil*gNoSpread.recoil/175+0.4;
        if(gNoSpread.spreadvar>0.9)
          gNoSpread.spreadvar=0.9f;
      }
      gNoSpread.prevtime=gNoSpread.gtime;
    }
    gNoSpread.UpdateWeaponRecoil(args);
  }
  pEvent_m249(args);
}

void Event_m4a1(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      gNoSpread.recoil++;
      gNoSpread.firing=true;
      if(cvar.nospread==2&&pGetBaseGun)
        gNoSpread.spreadvar=GetEngineSpread(gWeapon.GetId());
      else{
        gNoSpread.spreadvar=gNoSpread.recoil*gNoSpread.recoil*gNoSpread.recoil/220.0f+0.3;
        if(gNoSpread.spreadvar>1)
          gNoSpread.spreadvar=1.0f;
      }
      gNoSpread.prevtime=gNoSpread.gtime;
    }
    gNoSpread.UpdateWeaponRecoil(args);
  }
  pEvent_m4a1(args);
}

void Event_mp5n(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      gNoSpread.recoil++;
      gNoSpread.firing=true;
      if(cvar.nospread==2&&pGetBaseGun)
        gNoSpread.spreadvar=GetEngineSpread(gWeapon.GetId());
      else{
        gNoSpread.spreadvar=gNoSpread.recoil*gNoSpread.recoil/220.1f+0.45;
        if(gNoSpread.spreadvar>0.75)
          gNoSpread.spreadvar=0.75f;
      }
      gNoSpread.prevtime=gNoSpread.gtime;
    }
    gNoSpread.UpdateWeaponRecoil(args);
  }
  pEvent_mp5n(args);
}

void Event_mac10(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      gNoSpread.recoil++;
      gNoSpread.firing=true;
      if(cvar.nospread==2&&pGetBaseGun)
        gNoSpread.spreadvar=GetEngineSpread(gWeapon.GetId());
      else{
        gNoSpread.spreadvar=gNoSpread.recoil*gNoSpread.recoil*gNoSpread.recoil/200.0f+0.6;
        if(gNoSpread.spreadvar>1.65)
          gNoSpread.spreadvar=1.65f;
      }
      gNoSpread.prevtime=gNoSpread.gtime;
    }
    gNoSpread.UpdateWeaponRecoil(args);
  }
  pEvent_mac10(args);
}

void Event_p90(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      gNoSpread.recoil++;
      gNoSpread.firing=true;
      if(cvar.nospread==2&&pGetBaseGun)
        gNoSpread.spreadvar=GetEngineSpread(gWeapon.GetId());
      else{
        gNoSpread.spreadvar=gNoSpread.recoil*gNoSpread.recoil/175.0f+0.45;
        if(gNoSpread.spreadvar>1)
          gNoSpread.spreadvar=1.0f;
      }
      gNoSpread.prevtime=gNoSpread.gtime;
    }
    gNoSpread.UpdateWeaponRecoil(args);
  }
  pEvent_p90(args);
}

void Event_tmp(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      gNoSpread.recoil++;
      gNoSpread.firing=true;
      if(cvar.nospread==2&&pGetBaseGun)
        gNoSpread.spreadvar=GetEngineSpread(gWeapon.GetId());
      else{
        gNoSpread.spreadvar=gNoSpread.recoil*gNoSpread.recoil*gNoSpread.recoil/200+0.55;
        if(gNoSpread.spreadvar>1.4)
          gNoSpread.spreadvar=1.4f;
      }
      gNoSpread.prevtime=gNoSpread.gtime;
    }
    gNoSpread.UpdateWeaponRecoil(args);
  }
  pEvent_tmp(args);
}

void Event_ump45(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      gNoSpread.recoil++;
      gNoSpread.firing=true;
      if(cvar.nospread==2&&pGetBaseGun)
        gNoSpread.spreadvar=GetEngineSpread(gWeapon.GetId());
      else{
        gNoSpread.spreadvar=gNoSpread.recoil*gNoSpread.recoil/210+0.5;
        if(gNoSpread.spreadvar>1.0)
          gNoSpread.spreadvar=1.0f;
      }
      gNoSpread.prevtime=gNoSpread.gtime;
    }
    gNoSpread.UpdateWeaponRecoil(args);
  }
  pEvent_ump45(args);
}

void Event_awp(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      gNoSpread.recoil++;
      gNoSpread.prevtime=gNoSpread.gtime;
      gNoSpread.firing=true;
    }
    gNoSpread.UpdateWeaponRecoil(args);
  }
  pEvent_awp(args);
}

void Event_scout(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      gNoSpread.recoil++;
      gNoSpread.prevtime=gNoSpread.gtime;
      gNoSpread.firing=true;
    }
    gNoSpread.UpdateWeaponRecoil(args);
  }
  pEvent_scout(args);
}

void Event_xm1014(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      gNoSpread.recoil++;
      gNoSpread.prevtime=gNoSpread.gtime;
      gNoSpread.firing=true;
    }
    gNoSpread.UpdateWeaponRecoil(args);
  }
  pEvent_xm1014(args);
}

void Event_m3(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      gNoSpread.recoil++;
      gNoSpread.prevtime=gNoSpread.gtime;
      gNoSpread.firing=true;
    }
    gNoSpread.UpdateWeaponRecoil(args);
  }
  pEvent_m3(args);
}

void Event_knife(struct event_args_s *args) {
  if(args->entindex==gMe.index) {
    if(cvar.eventnospread){
      gNoSpread.recoil++;
      gNoSpread.prevtime=gNoSpread.gtime;
    }
  }
  pEvent_knife(args);
}
