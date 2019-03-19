#include "sprites.h"

spritedata_t spritedata;

void LoadSprites(client_sprite_t *pList, int iCount){
  char buf[512];
  spritedata.iSpritesLoaded=0;
  if(!pList)return;
  client_sprite_t *p=pList;int i=iCount;
  while(i--&&spritedata.iSpritesLoaded<MAX_SPRITES){
    if(p->szName&&p->szSprite&&strlen(p->szSprite)){
      strncpy(spritedata.spriteinfo[spritedata.iSpritesLoaded].szName,p->szName,MAX_SPRITES);
      strncpy(spritedata.spriteinfo[spritedata.iSpritesLoaded].szSprite,p->szSprite,MAX_SPRITES);
      sprintf(buf,"sprites/%s.spr",p->szSprite);
      spritedata.spriteinfo[spritedata.iSpritesLoaded].hspr=gEngfuncs.pfnSPR_Load(buf);
      memcpy(&(spritedata.spriteinfo[spritedata.iSpritesLoaded].rc),&(p->rc),sizeof(p->rc));
      spritedata.spriteinfo[spritedata.iSpritesLoaded].iRes=p->iRes;
      spritedata.iSpritesLoaded++;
    }
    p++;
  }
}

int GetSpriteIndex(char *szName){
  int i=-1;
  if(!szName)
    return i;
  for(i=0;i<spritedata.iSpritesLoaded;i++)
    if(spritedata.spriteinfo[i].iRes==640&&!strcmp(spritedata.spriteinfo[i].szName,szName))
      return i;
  return i;
}
