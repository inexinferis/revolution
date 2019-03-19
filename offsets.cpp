#include "offsets.h"


BOOL CompareMemory(LPBYTE bAddress,LPBYTE bCode,UINT uSize,BOOL bPattern){
  UINT i;
  for(i=0;i<uSize;i++,bCode++,bAddress++){
    if((*bAddress!=*bCode)&&(!bPattern||*bCode!=0xFF))
      return FALSE;
  }
  return TRUE;
}

DWORD FindCodeAddress(DWORD dwStart,DWORD dwEnd,LPBYTE bCode,UINT CodeSize,INT OpcodeNum,BOOL bPattern){
  DWORD i;
  for(i=dwStart;(i+CodeSize)<dwEnd;i++){
    if(CompareMemory((LPBYTE)i,bCode,CodeSize,bPattern))
      return (DWORD)(i+OpcodeNum);
  }
  return 0;
}

DWORD GetEngineBaseAndSize(PDWORD pSize,PBOOL pSteam,PBOOL pSteamSW,BOOL bForce){
  static DWORD hwBase=0,hwSize=0;
  CHAR buffer1[]="hw.dll";
  CHAR buffer2[]="sw.dll";
  BOOL bSteam=FALSE,bSteamSW=FALSE;
  if(!hwBase||(bForce&&hwBase==0x01D00000)){
    //Try Steam -> "hw.dll"
    hwBase=(DWORD)GetModuleBaseAndSize(buffer1,&hwSize);
    if(!hwBase){//try software mode -> "sw.dll"
      hwBase=(DWORD)GetModuleBaseAndSize(buffer2,&hwSize);
      bSteamSW=(hwBase!=0);
    }
    bSteam=(hwBase!=0);
    if(!hwBase){//try non steam
      hwBase=(DWORD)0x01D00000;
      hwSize=(DWORD)0x01300000;
    }
  }
  if(pSteamSW)
    *pSteamSW=bSteamSW;
  if(pSteam)
    *pSteam=bSteam;
  if(pSize)
    *pSize=hwSize;
  return hwBase;
}

DWORD GetClientBaseAndSize(PDWORD pSize,BOOL bSteam){
  static DWORD clBase=0,clSize=0;
  CHAR buffer[]="client.dll";
  if(!clBase){
    do{
      //Try Steam -> "client.dll"
      clBase=(DWORD)GetModuleBaseAndSize(buffer,&clSize);
    }while(!clBase&&bSteam);
    if(!clBase){//try non steam
      clBase=(DWORD)0x01900000;
      clSize=(DWORD)0x00400000;
    }
  }
  if(pSize)
    *pSize=clSize;
  return clBase;
}

DWORD FindExport(){
  DWORD hwBase,hwSize,dwStringAddress;
  BYTE PushAddrPattern[5]={0x68,0x90,0x90,0x90,0x90};
  //get engine base
  hwBase=GetEngineBaseAndSize(&hwSize,NULL,NULL,TRUE);//
  CHAR sString[]="could not link client.dll function Initialize";
  dwStringAddress=FindCodeAddress(hwBase,hwBase+hwSize,(PBYTE)sString,strlen(sString)-1,0,FALSE);
  if(dwStringAddress){
    RtlCopyMemory((PVOID)((PCHAR)&PushAddrPattern+1),&dwStringAddress,4);
    PDWORD retAddress=(PDWORD)FindCodeAddress(hwBase,hwBase+hwSize,PushAddrPattern,5,-6,FALSE);
    if(retAddress)
      return *retAddress;
  }
  return 0;
}

DWORD FindEngine(DWORD InitializeAddress){
  BYTE enginePattern[]={0x57,0xbf,0xff,0xff,0xff,0xff,0xf3,0xa5};
  PDWORD retAddress=(PDWORD)FindCodeAddress(InitializeAddress,InitializeAddress+0x50,enginePattern,8,2,TRUE);
  if(retAddress){
    return *retAddress;
  }
  return 0;
}

DWORD FindStudio(DWORD GetStudioModelInterfaceAddress){
  BYTE studioPattern[]={0xbf,0xff,0xff,0xff,0xff,0xc7};
  PDWORD retAddress=(PDWORD)FindCodeAddress(GetStudioModelInterfaceAddress,GetStudioModelInterfaceAddress+0x50,
    studioPattern,6,1,TRUE);
  if(retAddress){
    return *retAddress;
  }
  return 0;
}

DWORD FindInterface(DWORD GetStudioModelInterfaceAddress){
  BYTE studioPattern[]={0xbf,0xff,0xff,0xff,0xff,0xc7};
  PDWORD retAddress=(PDWORD)FindCodeAddress(GetStudioModelInterfaceAddress,GetStudioModelInterfaceAddress+0x50,
    studioPattern,6,7,TRUE);
  if(retAddress){
    return *retAddress;
  }
  return 0;
}

DWORD FindRenderer(DWORD StudioDrawModel){
  PDWORD retAddress=(*(PDWORD*)((DWORD)StudioDrawModel+0x05));
  if(retAddress){
    return *retAddress;
  }
  return 0;
}

DWORD FindPlayerMovePtr(){
	DWORD hwBase,hwSize,dwStringAddress;
  BYTE PushAddrPattern[5]={0x68,0x90,0x90,0x90,0x90};
  //get engine base
  hwBase=GetEngineBaseAndSize(&hwSize);
  CHAR sString[]="ScreenFade";
  dwStringAddress=FindCodeAddress(hwBase,hwBase+hwSize,(PBYTE)sString,strlen(sString)-1,0,FALSE);
  if(dwStringAddress){
    RtlCopyMemory((PVOID)((PCHAR)&PushAddrPattern+1),&dwStringAddress,4);
    PBYTE Ptr=(PBYTE)FindCodeAddress(hwBase,hwBase+hwSize,PushAddrPattern,5,0x18,FALSE);
    return *(DWORD*)Ptr;
  }
  return 0;
}

DWORD OffsetPreSDynSound(){
  DWORD hwBase,hwSize,dwStringAddress;
  BYTE PushAddrPattern[5]={0x68,0x90,0x90,0x90,0x90};
  //get engine base
  hwBase=GetEngineBaseAndSize(&hwSize);
  CHAR sString[]="S_StartDynamicSound: %s volume > 255";
  dwStringAddress=FindCodeAddress(hwBase,hwBase+hwSize,(PBYTE)sString,strlen(sString)-1,0,FALSE);
  if(dwStringAddress){
    RtlCopyMemory((PVOID)((PCHAR)&PushAddrPattern+1),&dwStringAddress,4);
    //1.5 FIX!
    PBYTE pS=(PBYTE)FindCodeAddress(hwBase,hwBase+hwSize,PushAddrPattern,5,0,FALSE);
    while(*(pS-1)!=0x90||*(pS-2)!=0x90||*(pS-3)!=0x90||*(pS-4)!=0x90)--pS;
    return (DWORD)pS;
  }
  return 0;
}

DWORD OffsetPreSStatSound(){
  DWORD hwBase,hwSize,dwStringAddress;
  BYTE PushAddrPattern[5]={0x68,0x90,0x90,0x90,0x90};
  //get engine base
  hwBase=GetEngineBaseAndSize(&hwSize);
  CHAR sString[]="S_StartStaticSound: %s volume > 255";
  dwStringAddress=FindCodeAddress(hwBase,hwBase+hwSize,(PBYTE)sString,strlen(sString)-1,0,FALSE);
  if(dwStringAddress){
    RtlCopyMemory((PVOID)((PCHAR)&PushAddrPattern+1),&dwStringAddress,4);
    //1.5 FIX!
    PBYTE pS=(PBYTE)FindCodeAddress(hwBase,hwBase+hwSize,PushAddrPattern,5,0,FALSE);
    while(*(pS-1)!=0x90||*(pS-2)!=0x90||*(pS-3)!=0x90||*(pS-4)!=0x90)--pS;
    return (DWORD)pS;
  }
  return 0;
}

DWORD OffsetUserMsgList(){
  DWORD hwBase,hwSize;
  //get engine base
  hwBase=GetEngineBaseAndSize(&hwSize);
  BYTE pattern[]={0x8B,0x35,0xff,0xff,0xff,0xff,0x83,0xC4,0x30};
  PDWORD retAddress=(PDWORD)FindCodeAddress(hwBase,hwBase+hwSize,pattern,9,2,TRUE);
  if(retAddress)
    return *retAddress;
  return 0;
}

DWORD OffsetHookEventList(){
  DWORD hwBase,hwSize;
  //get engine base
  hwBase=GetEngineBaseAndSize(&hwSize);
  BYTE pattern[]={0x8B,0x15,0xff,0xff,0xff,0xff,0x89,0x46,0x04,0x8B};
  PDWORD retAddress=(PDWORD)FindCodeAddress(hwBase,hwBase+hwSize,pattern,10,2,TRUE);
  if(retAddress)
    return *retAddress;
  return 0;
}

DWORD OffsetClientCmdList(){
  DWORD hwBase,hwSize;
  //get engine base
  hwBase=GetEngineBaseAndSize(&hwSize);
  BYTE pattern[]={0x8b,0x35,0xff,0xff,0xff,0xff,0x57,0x8b,0x46,0x10};
  PDWORD retAddress=(PDWORD)FindCodeAddress(hwBase,hwBase+hwSize,pattern,10,2,TRUE);
  if(retAddress)
    return *retAddress;
  return 0;
}

DWORD OffsetSrvCmdList(){
  DWORD hwBase,hwSize;
  //get engine base
  hwBase=GetEngineBaseAndSize(&hwSize);
  BYTE pattern[]={0xa1,0xff,0xff,0xff,0xff,0xc7,0x47};
  PDWORD retAddress=(PDWORD)FindCodeAddress(hwBase,hwBase+hwSize,pattern,7,1,TRUE);
  if(retAddress)
    return *retAddress;
  return 0;
}

double *OffsetGlobalTime(DWORD SendRequestAddress){
  BYTE globaltimePattern[]={0x89,0x48,0x0c,0x8b,0x0d,0xff,0xff,0xff,0xff,0x89,0x48,0x10};
  PDWORD retAddress=(PDWORD)FindCodeAddress(SendRequestAddress,SendRequestAddress+0x60,
    globaltimePattern,12,5,TRUE);
  if(retAddress){
    return (double*)*retAddress;
  }
  return NULL;
}

double *OffsetFPSSpeed(){
  //r_renderview: null worldmodel
  DWORD hwBase,hwSize,dwStringAddress;
  BYTE PushAddrPattern[5]={0x68,0x90,0x90,0x90,0x90};
  //get engine base
  hwBase=GetEngineBaseAndSize(&hwSize);
  CHAR sString[]="Texture load: %6.1fms";
  dwStringAddress=FindCodeAddress(hwBase,hwBase+hwSize,(PBYTE)sString,strlen(sString)-1,0,FALSE);
  if(dwStringAddress){
    RtlCopyMemory((PVOID)((PCHAR)&PushAddrPattern+1),&dwStringAddress,4);
    PDWORD retAddress=(PDWORD)FindCodeAddress(hwBase,hwBase+hwSize,PushAddrPattern,5,-7,FALSE);
    if(retAddress)
      return (double*)*retAddress;
  }
  return NULL;
}

PVOID OffsetGetBaseGun(){
  DWORD hwBase,hwSize;PBYTE BaseGunString;
  //get engine base
  hwBase=GetClientBaseAndSize(&hwSize,FALSE);
  // 8B  ??  ??  ??  48  83  ??  ??  0F  87  ??  ??  ??  ??  FF  24  85  ??  ??  ??  ??  B8  ??  ??  ??  ??  C3 (0x0190DAD5)
  BaseGunString=(PBYTE)"\x8B\xFF\xFF\xFF\x48\x83\xFF\xFF\x0F\x87\xFF\xFF\xFF\xFF\xFF\x24\x85\xFF\xFF\xFF\xFF\xB8\xFF\xFF\xFF\xFF\xC3";
  return (PVOID)FindCodeAddress(hwBase,hwBase+hwSize,BaseGunString,27,0,TRUE);
}
