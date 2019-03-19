#ifndef UTILS_H
#define UTILS_H

#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <tlhelp32.h>
#include <ntdll.h>
#include <psapi.h>

#ifndef __max
  #define __max(a,b)  (((a) > (b)) ? (a) : (b))
#endif
#ifndef __min
  #define __min(a,b)  (((a) < (b)) ? (a) : (b))
#endif

#define WUNKNOWN  (DWORD)-1
#define WINXP     0
#define WIN2K3    1
#define WINVISTA  2
#define WIN7      3
#define WIN8      4
#define WIN81     5
#define WIN10     6
#define WIN10TH2  7
#define WIN10AU   8
#define WIN10CU   9
#define WIN10FCU  10
#define WVERSIONS 11

#define WW             0x01
#define SS             0x02
#define WS             0x03
#define W3             0x08
#define CC             0x10
#define FF             0x20
#define LL             0x40
#define PR             0x80
#define WP             0x81

#define NOS            0xFF
#define NNN            0x00
#define REG            0x01
#define MRG            0x02
#define IMM            0x03
#define IM1            0x04
#define IM2            0x05
#define IMA            0x06
#define JOB            0x07
#define JOW            0x08
#define JMF            0x09
#define SEG_UNDEF      0x00
#define SEG_DEF        0x01

#define TLS_EXPANSION_SLOTS 1024

typedef struct _t_cmddata {
  ULONG          mask;
  ULONG          code;
  BYTE           len;
  BYTE           bits;
  BYTE           arg1,arg2,arg3;
} t_cmddata;

typedef struct _t_res {
  PBYTE address;
  DWORD offset;
  DWORD value;
} t_res;

DWORD WINAPI GetWinVr();
BOOL WINAPI FileExists(LPCSTR filename);
BOOL WINAPI DirectoryExist(LPCSTR szPath);

PVOID WINAPI RedirectFunc(PBYTE Address);
PVOID WINAPI HookFunc(PBYTE Address,PBYTE newAddress);
BOOL WINAPI UnHookFunc(PBYTE Address,PBYTE newAddress);
UINT WINAPI IstructionSize(PBYTE src);
PBYTE WINAPI FindIstruction(PBYTE src,BYTE instruction);
ULONG WINAPI CalcIstructionSize(PBYTE src,DWORD lenNeed,t_res *res);
DWORD WINAPI GetModuleBaseAndSize(LPCSTR lpModuleName,PDWORD pSize);

typedef struct _CLIENT_ID {
   HANDLE UniqueProcess;
   HANDLE UniqueThread;
} CLIENT_ID;

typedef struct _TEB
{
    NT_TIB Tib;                           //0x000
    PVOID EnvironmentPointer;             //0x01c
    CLIENT_ID ClientId;                   //0x020
    PVOID ActiveRpcHandle;                //0x028
    PVOID ThreadLocalStoragePointer;      //0x02C
    struct _PEB *ProcessEnvironmentBlock; //0x030
    ULONG LastErrorValue;                 //0x034
    ULONG CountOfOwnedCriticalSections;   //0x038
    PVOID CsrClientThread;                //0x03C
    PVOID Win32ThreadInfo;   //0x040
    ULONG User32Reserved[0x1A];           //0x044
    ULONG UserReserved[5];
    PVOID WOW32Reserved;
    LCID CurrentLocale;
    ULONG FpSoftwareStatusRegister;
    PVOID SystemReserved1[0x36];
    LONG ExceptionCode;
    PVOID ActivationContextStackPointer;
#ifdef _WIN64
    UCHAR SpareBytes1[24];
#else
    UCHAR SpareBytes1[0x24];
#endif
    ULONG TxFsContext;
    DWORD GdiTebBatch;
    CLIENT_ID RealClientId;
    PVOID GdiCachedProcessHandle;
    ULONG GdiClientPID;
    ULONG GdiClientTID;
    PVOID GdiThreadLocalInfo;
    SIZE_T Win32ClientInfo[62];
    PVOID glDispatchTable[0xE9];
    SIZE_T glReserved1[0x1D];
    PVOID glReserved2;
    PVOID glSectionInfo;
    PVOID glSection;
    PVOID glTable;
    PVOID glCurrentRC;
    PVOID glContext;
    DWORD LastStatusValue;
    PVOID StaticUnicodeString;
    WCHAR StaticUnicodeBuffer[0x105];
    PVOID DeallocationStack;
    PVOID TlsSlots[0x40];
    LIST_ENTRY TlsLinks;
    PVOID Vdm;
    PVOID ReservedForNtRpc;
    PVOID DbgSsReserved[0x2];
    ULONG HardErrorDisabled;
#ifdef _WIN64
    PVOID Instrumentation[11];
#else
    PVOID Instrumentation[9];
#endif
    GUID ActivityId;
    PVOID SubProcessTag;
    PVOID EtwTraceData;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    PVOID EtwLocalData;
#endif
    PVOID WinSockData;
    ULONG GdiBatchCount;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    BOOLEAN SpareBool0;
    BOOLEAN SpareBool1;
    BOOLEAN SpareBool2;
#else
    BOOLEAN InDbgPrint;
    BOOLEAN FreeStackOnTermination;
    BOOLEAN HasFiberData;
#endif
    UCHAR IdealProcessor;
    ULONG GuaranteedStackBytes;
    PVOID ReservedForPerf;
    PVOID ReservedForOle;
    ULONG WaitingOnLoaderLock;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    PVOID SavedPriorityState;
#else
    ULONG SparePointer1;
#endif
    ULONG SoftPatchPtr1;
    ULONG SoftPatchPtr2;
    PVOID *TlsExpansionSlots;
    ULONG ImpersonationLocale;
    ULONG IsImpersonating;
    PVOID NlsCache;
    PVOID pShimData;
    ULONG HeapVirualAffinity;
    PVOID CurrentTransactionHandle;
    PVOID ActiveFrame;
#if (NTDDI_VERSION >= NTDDI_WS03)
    PVOID FlsData;
#endif
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    PVOID PreferredLangauges;
    PVOID UserPrefLanguages;
    PVOID MergedPrefLanguages;
    ULONG MuiImpersonation;
    union
    {
        struct
        {
            USHORT SpareCrossTebFlags:16;
        };
        USHORT CrossTebFlags;
    };
    union
    {
        struct
        {
            USHORT DbgSafeThunkCall:1;
            USHORT DbgInDebugPrint:1;
            USHORT DbgHasFiberData:1;
            USHORT DbgSkipThreadAttach:1;
            USHORT DbgWerInShipAssertCode:1;
            USHORT DbgIssuedInitialBp:1;
            USHORT DbgClonedThread:1;
            USHORT SpareSameTebBits:9;
        };
        USHORT SameTebFlags;
    };
    PVOID TxnScopeEntercallback;
    PVOID TxnScopeExitCAllback;
    PVOID TxnScopeContext;
    ULONG LockCount;
    ULONG ProcessRundown;
    ULONGLONG LastSwitchTime;
    ULONGLONG TotalSwitchOutTime;
    LARGE_INTEGER WaitReasonBitMap;
#else
    UCHAR SafeThunkCall;
    UCHAR BooleanSpare[3];
#endif
} TEB, *PTEB;

#endif //UTILS_H
