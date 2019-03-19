#ifndef CSPRITES_H
#define CSPRITES_H

#include "main.h"

#define MAX_SPRITES 200

typedef struct {
  char szName[256];
  char szSprite[256];
  HSPRITE hspr;
  wrect_t rc;
  int iRes;
} spriteinfo_t;

typedef struct {
  int iSpritesLoaded;
  spriteinfo_t spriteinfo[MAX_SPRITES];
} spritedata_t;

int GetSpriteIndex(char *szName);
void LoadSprites(client_sprite_t *pList, int iCount);

extern spritedata_t spritedata;

#endif //CSPRITES_H
