#ifndef _OFFSETS_
#define _OFFSETS_

#include "utils.h"

BOOL CompareMemory(LPBYTE bAddress,LPBYTE bCode,UINT uSize,BOOL bPattern);
DWORD FindCodeAddress(DWORD dwStart,DWORD dwEnd,LPBYTE bCode,UINT CodeSize,INT OpcodeNum,BOOL bPattern);
DWORD GetEngineBaseAndSize(PDWORD pSize,PBOOL pSteam=NULL,PBOOL pSteamSW=NULL,BOOL bForce=FALSE);
DWORD GetClientBaseAndSize(PDWORD pSize,BOOL bSteam);
DWORD FindExport();
DWORD FindEngine(DWORD InitializeAddress);
DWORD FindStudio(DWORD GetStudioModelInterfaceAddress);
DWORD FindInterface(DWORD GetStudioModelInterfaceAddress);
DWORD FindRenderer(DWORD StudioDrawModel);
DWORD FindPlayerMovePtr();
DWORD OffsetPreSDynSound();
DWORD OffsetPreSStatSound();
DWORD OffsetUserMsgList();
DWORD OffsetHookEventList();
DWORD OffsetClientCmdList();
DWORD OffsetSrvCmdList();
double *OffsetGlobalTime(DWORD SendRequestAddress);
double *OffsetFPSSpeed();
PVOID OffsetGetBaseGun();
PBYTE FindOffsetEngCheck(PBYTE dir);

#endif //_OFFSETS_
