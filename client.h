#ifndef ENGINE_H
#define ENGINE_H

#include "main.h"

#define KNIFESPEED 20
#define SPEEDMODE  8

//void HUD_Frame(double time);
void V_CalcRefdef(struct ref_params_s *pparams);
void CL_CreateMove(float frametime, struct usercmd_s *cmd, int active);
void HUD_PostRunCmd ( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed );
void HUD_PlayerMoveInit(struct playermove_s *ppmove);
void HUD_PlayerMove(struct playermove_s *ppmove, int server );
int HUD_Redraw(float x,int y);
int HUD_AddEntity(int type, struct cl_entity_s *ent, const char *modelname);
int HUD_Key_Event(int eventcode,int keynum,const char *pszCurrentBinding);
int HUD_UpdateClientData(client_data_t *cdata, float flTime);
int hStudioDrawPlayer(int flags,entity_state_s *pplayer);
int	hStudioDrawModel(int flags);
int hStudioCheckBBox(void);
void hStudioEntityLight(struct alight_s *plight);

void func_active();
void func_showmenu();
void func_showconsole();
void func_customautorute();
void func_customtriggerbot();
void func_customaim();
void func_saveconfig();
void func_loadconfig();
void func_hidescreenshot();

void hPreS_StaticSound(int index,int channel,char *sample,float *origin,float volume,float attenuation,int fFlags,int pitch);
void hPreS_DynamicSound(int index,int channel,char *sample,float *origin,float volume,float attenuation,int fFlags,int pitch);
typedef void (*tPreS_Sound)(int index,int channel,char *sample,float *origin,float volume,float attenuation,int fFlags,int pitch);
extern tPreS_Sound pPreS_DynamicSound,pPreS_StaticSound;

void hStudioRenderModel();
typedef void (*tStudioRenderModel)();
extern tStudioRenderModel pStudioRenderModel;

extern int (*pStudioDrawPlayer)(int flags,entity_state_s *pplayer);
extern int (*pStudioDrawModel)(int flags);
extern int (*pStudioCheckBBox)(void);
extern void (*pStudioEntityLight)(struct alight_s *plight);

extern double serverTime;
extern UINT drawingCam;
extern pfnUserMsgHook pResetHUD,pTeamInfo,pCurWeapon,pScoreAttrib,pSetFOV,
pHealth,pBattery,pScoreInfo,pDeathMsg,pSayText,pDamage,//pTextMsg,
pAmmoX,pMoney,pWeaponList,pRadar,pFog;

typedef void (*pfnEvent)( struct event_args_s *args );
typedef void (*pfnClientCmd)(void);
typedef void (*pfnSrvCmd)(void);

typedef struct eventlist_s{
	struct eventlist_s* next;
	char* name;
	pfnEvent pfn;
}eventlist_t;

typedef struct usermsglist_s{
	int number;
	int size;
	char name[16];
	struct usermsglist_s* next;
	pfnUserMsgHook pfn;
}usermsglist_t;

typedef struct clientcmdlist_s{
	char *name;
	pfnClientCmd pfn;
	int ivalue;
	float fvalue;
	struct clientcmdlist_s *next;
}clientcmdlist_t;

typedef struct srvcmdlist_s{
	struct srvcmdlist_s *next;
	char *name;
	pfnSrvCmd pfn;
}srvcmdlist_t;

//pfnClientCmd HookClientCmd(const char *name,pfnClientCmd pfn);
pfnEvent HookEvent(const char *name,pfnEvent pfn);
pfnUserMsgHook HookUserMsg(const char *name,pfnUserMsgHook pfn);
pfnSrvCmd HookSrvCmd(const char *name,pfnSrvCmd pfn);

//extern clientcmdlist_t** pClientCmdList;
extern eventlist_t** pEventList;
extern usermsglist_t** pUserMsgList;
extern srvcmdlist_t** pSrvCmdList;

//VOID HookClientCmds();
VOID HookEvents();
VOID HookUserMsgs();
VOID HookSrvCmds();

#endif //ENGINE_H
