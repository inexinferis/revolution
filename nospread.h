#ifndef NOSPREAD_H
#define NOSPREAD_H

#include "weapons.h"
#include "drawing.h"
#include "players.h"
#include "main.h"
#include "cvar.h"

typedef void (*pfnEventHook)(struct event_args_s *args);

class cNoSpread {
  public:
    void GetRecoilOffset(float *outangles);
    void GetSpreadOffset(float *outangles);
    void HUD_PostRunCmd(struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed);
    void UpdateWeaponRecoil(struct event_args_s *args);
  private:
    unsigned int U_Random( void );
    float GetVecSpread(float speed);
    void DefaultSpreadVar(int weaponid);
    void PrimaryAttack(int id);
    void U_Srand( unsigned int seed );
    void GetSpreadXY(int seed, int future, float *velocity, float *vec);
    int UTIL_SharedRandomLong( unsigned int seed, int low, int high );
    float UTIL_SharedRandomFloat( unsigned int seed, float low, float high );
    unsigned int grandom_seed;
  public:
    int recoil;
    float gtime;
    float prevtime;
    float spreadvar;
    float recoiltime;
    float vecspread;
    float xrecoil,yrecoil;
    bool firing;
    int prcflags;
};

extern cNoSpread gNoSpread;

void Event_deagle(struct event_args_s *args);
void Event_elite_left(struct event_args_s *args);
void Event_elite_right(struct event_args_s *args);
void Event_galil(struct event_args_s *args);
void Event_famas(struct event_args_s *args);
void Event_fiveseven(struct event_args_s *args);
void Event_glock18(struct event_args_s *args);
void Event_p228(struct event_args_s *args);
void Event_g3sg1(struct event_args_s *args);
void Event_sg550(struct event_args_s *args);
void Event_usp(struct event_args_s *args);
void Event_ak47(struct event_args_s *args);
void Event_sg552(struct event_args_s *args);
void Event_aug(struct event_args_s *args);
void Event_m249(struct event_args_s *args);
void Event_m4a1(struct event_args_s *args);
void Event_mp5n(struct event_args_s *args);
void Event_mac10(struct event_args_s *args);
void Event_p90(struct event_args_s *args);
void Event_tmp(struct event_args_s *args);
void Event_ump45(struct event_args_s *args);
void Event_awp(struct event_args_s *args);
void Event_scout(struct event_args_s *args);
void Event_xm1014(struct event_args_s *args);
void Event_m3(struct event_args_s *args);
void Event_knife(struct event_args_s *args);

extern pfnEventHook pEvent_usp,pEvent_mp5n,pEvent_ak47,pEvent_aug,pEvent_deagle,pEvent_g3sg1,pEvent_sg550,
       pEvent_glock18,pEvent_m249,pEvent_m3,pEvent_m4a1,pEvent_mac10,pEvent_p90,pEvent_p228,pEvent_awp,
       pEvent_scout,pEvent_sg552,pEvent_tmp,pEvent_fiveseven,pEvent_ump45,pEvent_xm1014,pEvent_elite_left,
       pEvent_elite_right,pEvent_galil,pEvent_famas,pEvent_knife;


#endif //NOSPREAD_H
