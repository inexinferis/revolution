#ifndef WALLHACK_H
#define WALLHACK_H

#define M_PI 3.14159265358979323846
#define BOUND_VALUE(var,min,max) if((var)>(max)){(var)=(max);};if((var)<(min)){(var)=(min);}

#include "engine/wrect.h"
#include "engine/cl_dll.h"
#include "engine/pmtrace.h"
#include "engine/pm_defs.h"
#include "engine/cdll_int.h"
#include "engine/const.h"
#include "engine/progdefs.h"
#include "engine/eiface.h"
#include "engine/edict.h"
#include "engine/studio_event.h"
#include "engine/entity_types.h"
#include "engine/entity_state.h"
#include "engine/r_efx.h"
#include "engine/r_studioint.h"
#include "engine/pmtrace.h"
#include "engine/usercmd.h"
#include "engine/studio.h"
#include "engine/triangleapi.h"
#include "engine/com_model.h"
#include "engine/studiomodelrenderer.h"
#include "engine/com_model.h"
#include "engine/ref_params.h"
#include "engine/event_api.h"
#include "engine/screenfade.h"
#include "engine/parsemsg.h"
#include "engine/net_api.h"
#include "engine/interface.h"
#include "engine/keydefs.h"

#include "utils.h"

typedef PBYTE (*tGetBaseGun)(int index);

extern HINSTANCE g_hInstance;
extern DWORD g_dwModSize;

extern CHAR dllpath[MAX_PATH];
extern CHAR chtpath[MAX_PATH];
extern CHAR drvpath[MAX_PATH];
extern CHAR inipath[MAX_PATH];
extern CHAR cfgpath[MAX_PATH];
extern CHAR waypath[MAX_PATH];
extern CHAR nkspath[MAX_PATH];
extern CHAR scrpath[MAX_PATH];

extern BOOL iniFound;
extern BOOL cfgFound;
extern BOOL wayFound;
extern BOOL nksFound;
extern BOOL scrFound;

extern BOOL bInvalidVideoMode;
extern BOOL bSteam;

extern exporttable_t *pExport;
extern cl_enginefunc_t *pEngfuncs;
extern engine_studio_api_t *pEngStudio;
extern r_studio_interface_t *pInterface;
extern studio_model_renderer_t *pRenderer;
//extern playermove_t *ppmove;

extern exporttable_t gExport;
extern cl_enginefuncs_s gEngfuncs;
extern engine_studio_api_t gEngStudio;
extern r_studio_interface_t gInterface;
extern studio_model_renderer_t gRenderer;

extern PVOID PreSDynamicSoundAddress;
extern double* globalTime;

extern tGetBaseGun pGetBaseGun;

#endif //WALLHACK_H
