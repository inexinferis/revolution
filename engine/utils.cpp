#include "utils.h"
#include <windows.h>

extern cl_enginefuncs_s gEngfuncs;

void cGetFunc::Init(){}

pfnCommand cGetFunc::GetAddCommand(PCHAR name){
  PBYTE address=(PBYTE)gEngfuncs.pfnAddCommand+0x1B;
  command_t *uml=*(command_t**)(*(PDWORD)((address+*((PDWORD)address)+4)+0x0D));
  while(uml){
    if(!strncmp(uml->name,name,16))
      return uml->pfn;
    uml=uml->next;
  }
  return 0;
}

pfnEventHook cGetFunc::GetHookEvent(PCHAR name){
  PBYTE address=(PBYTE)gEngfuncs.pfnHookEvent+0x1B;
  event_t *uml=*(event_t**)(*(PDWORD)((address+*(PDWORD)(address)+4)+0x0D));
  while(uml){
    if(!strncmp(uml->name,name,16))
      return uml->pfn;
    uml=uml->next;
  }
  return 0;
}

pfnUserMsgHook cGetFunc::GetHookUserMsg(PCHAR name){
  PBYTE address=(PBYTE)gEngfuncs.pfnHookUserMsg+0x1B;;
  usermsg_t *uml=*(usermsg_t**)(*(PDWORD)((address+*(PDWORD)(address)+4)+0x0D));
  while(uml){
    if(!strncmp(uml->name,name,16))
      return uml->pfn;
    uml=uml->next;
  }
  return 0;
}
