/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#ifndef _UTILS_H
#define _UTILS_H

#include <windows.h>
#include "../engine/wrect.h"
#include "../engine/cl_dll.h"
#include "../engine/cdll_int.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*pfnCommand)(void);
typedef struct command_s{
  struct command_s* next;
  char* name;
  pfnCommand pfn;
}command_t;

typedef void (*pfnEventHook)(struct event_args_s *args);
typedef struct event_s{
  struct event_s* next;
  char* name;
  pfnEventHook pfn;
}event_t;

typedef struct usermsg_s{
  int number;
  int size;
  char name[16];
  struct usermsg_s* next;
  pfnUserMsgHook pfn;
}usermsg_t;

class cGetFunc {
  public:
    cGetFunc():umt(NULL),et(NULL),ct(NULL){}
    void Init();

    pfnCommand GetAddCommand(PCHAR name);
    pfnEventHook GetHookEvent(PCHAR name);
    pfnUserMsgHook GetHookUserMsg(PCHAR name);
  private:
    usermsg_t *umt;
    event_t *et;
    command_t *ct;
};

#ifdef __cplusplus
}
#endif

#endif // _UTILS_H
