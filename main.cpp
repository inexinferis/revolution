#include "screenshots.h"
#include "opengl.h"
#include "client.h"
#include "utils.h"
#include "cvar.h"
#include "main.h"
#include "offsets.h"

HINSTANCE g_hInstance=NULL;
DWORD g_dwModSize=0;

//paths!
CHAR dllpath[MAX_PATH]={0};
CHAR chtpath[MAX_PATH]={0};
CHAR inipath[MAX_PATH]={0};
CHAR cfgpath[MAX_PATH]={0};
CHAR waypath[MAX_PATH]={0};
CHAR nkspath[MAX_PATH]={0};
CHAR scrpath[MAX_PATH]={0};

BOOL iniFound=FALSE;
BOOL cfgFound=FALSE;
BOOL wayFound=FALSE;
BOOL nksFound=FALSE;
BOOL scrFound=FALSE;

//Cheat Flags
BOOL bSteam=FALSE,bSteamSW=FALSE;
BOOL bNeedPath=TRUE;
BOOL bInvalidVideoMode=FALSE;

//engine pointers
exporttable_t *pExport=NULL;
cl_enginefunc_t *pEngfuncs=NULL;
engine_studio_api_t *pEngStudio=NULL;
r_studio_interface_t *pInterface=NULL;
studio_model_renderer_t *pRenderer=NULL;
//playermove_t *ppmove=NULL;

//engine pointers copy
exporttable_t gExport;
cl_enginefuncs_s gEngfuncs;
engine_studio_api_t gEngStudio;
r_studio_interface_t gInterface;
studio_model_renderer_t gRenderer;

PVOID PreSDynamicSoundAddress;
double* globalTime=NULL;
tGetBaseGun pGetBaseGun=NULL;

VOID HookExport(exporttable_t* pExport){
  //hook HUD_Frame
  //pExport->HUD_Frame=HUD_Frame;
  //hook V_CalcRefdef
  pExport->V_CalcRefdef=V_CalcRefdef;
  //hook HUD_Key_Event
  pExport->HUD_Key_Event=HUD_Key_Event;
  //hook HUD_PlayerMove
  pExport->HUD_PlayerMove=HUD_PlayerMove;
  //hook HUD_PlayerMoveInit
  pExport->HUD_PlayerMoveInit=HUD_PlayerMoveInit;
  //hook HUD_UpdateClientData
  pExport->HUD_UpdateClientData=HUD_UpdateClientData;
  //hook HUD_PostRunCmd
  pExport->HUD_PostRunCmd=HUD_PostRunCmd;
  //hook HUD_Redraw
  pExport->HUD_Redraw=HUD_Redraw;
  //hook CL_CreateMove
  pExport->CL_CreateMove=CL_CreateMove;
  //hook HUD_AddEntity
  pExport->HUD_AddEntity=HUD_AddEntity;
}

VOID HookEngFunc(cl_enginefuncs_s *pEngfuncs){
  //path engfuncs...
  //path engfuncs...
  PDWORD pAddress=(PDWORD)&gEngfuncs;
  for(int i=0;i<80;i++){
    pAddress[i]=(DWORD)RedirectFunc((PBYTE)pAddress[i]);
  }
}

VOID HookStudio(r_studio_interface_t* pInterface,engine_studio_api_t *pStudioApi,studio_model_renderer_t* pRenderer){
  //fix aim behind me
  pStudioCheckBBox=pStudioApi->StudioCheckBBox;
  pStudioApi->StudioCheckBBox=hStudioCheckBBox;
  //hook StudioEntityLight
  pStudioEntityLight=pStudioApi->StudioEntityLight;
  pStudioApi->StudioEntityLight=hStudioEntityLight;
  //hook StudioDrawPlayer
  pStudioDrawPlayer=pInterface->StudioDrawPlayer;
  pInterface->StudioDrawPlayer=hStudioDrawPlayer;
  //hook StudioDrawModel
  pStudioDrawModel=pInterface->StudioDrawModel;
  pInterface->StudioDrawModel=hStudioDrawModel;
  //hook StudioModelRenderer (for chams -> this is a thiscall method!)
  //Can't replace in UCP!
  /*VirtualProtect((PVOID)&pRenderer->StudioRenderModel,4,PAGE_READWRITE,&dwback);
  pStudioRenderModel=pRenderer->StudioRenderModel;
  pRenderer->StudioRenderModel=hStudioRenderModel;
  VirtualProtect((PVOID)&pRenderer->StudioRenderModel,4,dwback,&dwback);*/
  //hwbp
  pStudioRenderModel=(tStudioRenderModel)HookFunc((PBYTE)pRenderer->StudioRenderModel,(PBYTE)hStudioRenderModel);
}

VOID OffsetsThread(){
  DWORD hwSize,pPreSDynamicSound=0,pPreSStaticSound=0;
  BOOL bExportFound=FALSE,bEngFuncsFound=FALSE,bEngStudioFound=FALSE;
  //get client data
  GetEngineBaseAndSize(&hwSize,&bSteam,&bSteamSW);
  //client is loaded after...
  GetClientBaseAndSize(NULL,bSteam);//need to be done here for UCP
  while(!bExportFound||!bEngFuncsFound||!bEngStudioFound){
    if(!bExportFound){
      if(!pExport){
        pExport=(exporttable_t*)FindExport();
      }
      if(pExport){
        if(pExport->HUD_GetStudioModelInterface){
          memcpy(&gExport,pExport,sizeof(exporttable_t));
          HookExport(pExport);
          bExportFound=TRUE;
        }
      }
    }
    if(!bEngFuncsFound&&bExportFound){
      if(!pEngfuncs){
        pEngfuncs=(cl_enginefunc_t*)FindEngine((DWORD)*pExport->Initialize);
      }
      if(pEngfuncs){
        if(pEngfuncs->pfnHookEvent){
          memcpy(&gEngfuncs,pEngfuncs,sizeof(cl_enginefunc_t));
          HookEngFunc(pEngfuncs);
          bEngFuncsFound=TRUE;
        }
      }
    }
    if(!bEngStudioFound&&bExportFound){
      if(!pEngStudio){
        pEngStudio=(engine_studio_api_t*)FindStudio((DWORD)*pExport->HUD_GetStudioModelInterface);
      }
      if(!pInterface){
        pInterface=(r_studio_interface_t*)FindInterface((DWORD)*pExport->HUD_GetStudioModelInterface);
      }
      if(pEngStudio&&pInterface){
        if(pEngStudio->Mem_Calloc&&pInterface->StudioDrawModel){
          if(!pRenderer){
            pRenderer=(studio_model_renderer_t*)FindRenderer((DWORD)pInterface->StudioDrawModel);
          }
          if(pRenderer&&pRenderer->Init){
            memcpy(&gEngStudio,pEngStudio,sizeof(engine_studio_api_s));
            memcpy(&gInterface,pInterface,sizeof(r_studio_interface_s));
            memcpy(&gRenderer,pRenderer,sizeof(studio_model_renderer_t));
            HookStudio(pInterface,pEngStudio,pRenderer);
            bEngStudioFound=TRUE;
          }
        }
      }
    }
    Sleep(bEngFuncsFound ? 100 : 10);
  }
  bInvalidVideoMode=!pEngStudio->IsHardware();
  //pEventList
  pEventList=(eventlist_t**)OffsetHookEventList();
  if(pEventList){
    HookEvents();
  }
  //pUserMsgList
  pUserMsgList=(usermsglist_t**)OffsetUserMsgList();
  if(pUserMsgList){
    HookUserMsgs();
  }
  //pSrvCmdList
  pSrvCmdList=(srvcmdlist_t**)OffsetSrvCmdList();
  if(pSrvCmdList){
    HookSrvCmds();
  }
  //SOUND ESP
  pPreSDynamicSound=OffsetPreSDynSound();
  if(pPreSDynamicSound){
    pPreS_DynamicSound=(tPreS_Sound)HookFunc((PBYTE)pPreSDynamicSound,(PBYTE)hPreS_DynamicSound);
  }
  pPreSStaticSound=OffsetPreSStatSound();
  if(pPreSStaticSound){
    pPreS_StaticSound=(tPreS_Sound)HookFunc((PBYTE)pPreSStaticSound,(PBYTE)hPreS_StaticSound);
  }
  //GlobalTime
  globalTime=OffsetGlobalTime((DWORD)pEngfuncs->pNetAPI->SendRequest);
  //Base Gun Spread...
  pGetBaseGun=(tGetBaseGun)OffsetGetBaseGun();
  cvar.Init();
}

VOID ShowInfo(){
  Sleep(2000);//Fix 1.5 bug...
  gEngfuncs.pfnConsolePrint("============================================\n\
\t\t\t\t\t\t\t\t\t\t\tInexinferis Revolution 2017\n\
\t\t\t\t\t\t\t\t\t\t\t\t\t\thttp://www.inexinferis.com\n\
============================================\n\n\
Options:\n\
----------\n\
 * F4 - Disable\n\
 * F5/Middle Button - Menu\n\
 * F6 - Console\n\n\
Extra:\n\
-------\n\
 * Alt - (In Triggerbot 2) Activate Triggerbot\n\
 * Bloq Mayus - AutoRoute\n\n\
 [See Forum To Customize Keys]\n\n\
============================================\n");
  if(bInvalidVideoMode)
    gEngfuncs.pfnConsolePrint(" **********      WARNING! Invalid Video Mode!      ***********\n\
============================================\n");
  gEngfuncs.pfnClientCmd("fs_lazy_precache 1");
  gEngfuncs.pfnClientCmd("toggleconsole");
  //unbind cheat keys
  gEngfuncs.pfnClientCmd("unbind F4");
  gEngfuncs.pfnClientCmd("unbind F5");
  gEngfuncs.pfnClientCmd("unbind F6");
}

DWORD WINAPI StartThread(HINSTANCE hInstance){
  OffsetsThread();
  if(!bInvalidVideoMode){
    HookOpenGl();
  }
  ShowInfo();
  return 0;
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance,DWORD fdwReason,LPVOID lpvReserved){
  BOOL bRet=TRUE;
  switch(fdwReason){
    case DLL_PROCESS_ATTACH: {
      //init
      g_hInstance=hInstance;
      GetModuleFileName(hInstance,dllpath,MAX_PATH);

      //Make Paths...
      strcpy(chtpath,dllpath);
      CHAR *tmp=chtpath;
      while(*(++tmp));
      while(*(--tmp)!='\\');
      *(tmp)=0;

      strcat(inipath,"\\inexinferis.ini");
      iniFound=FileExists(inipath);

      strcat(cfgpath,"\\inexinferis.cfg");
      cfgFound=FileExists(cfgpath);

      strcat(nkspath,"\\inexinferis.txt");
      nksFound=FileExists(nkspath);

      strcat(waypath,"\\WPoints");
      wayFound=DirectoryExist(waypath);

      strcat(scrpath,"\\ScreenShots");
      scrFound=DirectoryExist(scrpath);

      //cheat thread...
      CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)StartThread,hInstance,0,NULL);
    }
    break;
    default:// DLL_THREAD_ATTACH, DLL_THREAD_DETACH
      return FALSE;
  }
  return bRet;
}
