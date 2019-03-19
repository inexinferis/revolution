#include "textures.h"

static char *gcTexturesFile = "sound/materials.txt";
static char grgszTextureName[CTEXTURESMAX][CBTEXTURENAMEMAX];
static char grgchTextureType[CTEXTURESMAX];
static int gcTextures = 0;

char FindTextureType(char *name){
	int right,pivot,val,left=0;
	right=gcTextures-1;
	while(left<=right){
		pivot=(left+right)/2;
		val=strnicmp(name,grgszTextureName[pivot],CBTEXTURENAMEMAX-1);
		if(val==0)
      return grgchTextureType[pivot];
		else if(val>0)
      left=pivot+1;
		else if(val<0)
      right=pivot-1;
	}
	return CHAR_TEX_CONCRETE;
}

char GetTextureType(pmtrace_t *tr, float *vecSrc, float *vecEnd){
	char chTextureType=CHAR_TEX_CONCRETE;
	int entity;char *pTextureName;
	char texname[64];char szbuffer[64];
	entity=gEngfuncs.pEventAPI->EV_IndexFromTrace(tr);
	chTextureType=0;
	if(entity>=1&&entity<=gEngfuncs.GetMaxClients())
		chTextureType=CHAR_TEX_FLESH;
	else if(entity==0){
		pTextureName=(char *)gEngfuncs.pEventAPI->EV_TraceTexture(tr->ent,vecSrc,vecEnd);
		if(pTextureName){
			strcpy(texname,pTextureName);
			pTextureName=texname;
			if(*pTextureName=='-'||*pTextureName=='+')
				pTextureName += 2;
			if(*pTextureName=='{'||*pTextureName=='!'||*pTextureName=='~'||*pTextureName==' ')
				pTextureName++;
			strcpy(szbuffer,pTextureName);
			szbuffer[CBTEXTURENAMEMAX-1] = 0;
			chTextureType=FindTextureType(szbuffer);
		}
	}
	return chTextureType;
}

void SwapTextures(int i,int j){
	char chTemp,szTemp[CBTEXTURENAMEMAX];
	strcpy(szTemp,grgszTextureName[i]);
	chTemp=grgchTextureType[i];
	strcpy(grgszTextureName[i],grgszTextureName[j]);
	grgchTextureType[i]=grgchTextureType[j];
	strcpy(grgszTextureName[j],szTemp);
	grgchTextureType[j]=chTemp;
}

void SortTextures(void){
	int i,j;
	for(i=0;i<gcTextures;i++)
  for(j=i+1;j<gcTextures;j++)
    if(stricmp(grgszTextureName[i],grgszTextureName[j])>0)
      SwapTextures(i,j);
}

void InitTextureTypes(struct playermove_s *ppmove){
	static qboolean bTextureTypeInit=false;
	if(bTextureTypeInit||!ppmove||!ppmove->COM_FileSize||!ppmove->memfgets||!ppmove->COM_FreeFile)
    return;
  char buffer[512];int i,j;
  memset(buffer,0,512);
	memset(&(grgszTextureName[0][0]),0,CTEXTURESMAX*CBTEXTURENAMEMAX);
	memset(grgchTextureType,0,CTEXTURESMAX);
	gcTextures=0;
	int fileSize=ppmove->COM_FileSize((char*)gcTexturesFile);
	byte *pMemFile=ppmove->COM_LoadFile((char*)gcTexturesFile,5,NULL);
	if(!pMemFile)
    return;
	int filePos=0;
	while(ppmove->memfgets(pMemFile,fileSize,&filePos,buffer,511)!=NULL&&gcTextures<CTEXTURESMAX){
		i=0;
		while(buffer[i]&&isspace(buffer[i]))
      i++;
		if(!buffer[i])
      continue;
		if(buffer[i]=='/'||!isalpha(buffer[i]))
      continue;
		grgchTextureType[gcTextures]=toupper(buffer[i++]);
		while(buffer[i]&&isspace(buffer[i]))
      i++;
		if(!buffer[i])
      continue;
		j=i;
		while(buffer[j]&&!isspace(buffer[j]))j++;
    if(!buffer[j])
      continue;
		j=min(j,CBTEXTURENAMEMAX-1+i);
		buffer[j]=0;
		strcpy(&(grgszTextureName[gcTextures++][0]),&(buffer[i]));
	}
	ppmove->COM_FreeFile(pMemFile);
	SortTextures();
	bTextureTypeInit=true;
}
