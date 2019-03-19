#include "FakeSteamId.h"

#define SENDTOLEN 5
typedef int (PASCAL *tSendTo)(SOCKET s,const char *buf,int len,int flags,const struct sockaddr *to,int tolen);
tSendTo pSendTo=NULL,pOrigSendTo=NULL;
int PASCAL hSendTo(SOCKET s,const char *buf,int len,int flags,const struct sockaddr *to,int tolen);

VOID InitHook(){
  if(!pSendTo||!pOrigSendTo){
    HMODULE hWS2=GetModuleHandle("ws2_32.dll");
    if(hWS2){
      pOrigSendTo=(tSendTo)GetProcAddress(hWS2,"sendto");
      if(pOrigSendTo){
        pSendTo=(tSendTo)VirtualAlloc(NULL,SENDTOLEN+5,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
        if(pSendTo){
          memmove((PBYTE)pSendTo,(PBYTE)pOrigSendTo,SENDTOLEN);
          *((PBYTE)pSendTo+SENDTOLEN)=0xE9;
          *(DWORD*)((PBYTE)pSendTo+SENDTOLEN+1)=(DWORD)((PBYTE)pOrigSendTo-(PBYTE)pSendTo)-5;
        }
      }
    }
  }
}

VOID HookSendTo(){
  DWORD dwback=0;
  InitHook();
  if(pSendTo&&pOrigSendTo){
    if(VirtualProtect((PVOID)pOrigSendTo,SENDTOLEN,PAGE_READWRITE,&dwback)){
      *((PBYTE)pOrigSendTo)=0xE9;
      *(DWORD*)((PBYTE)pOrigSendTo+1)=(DWORD)((PBYTE)hSendTo-(PBYTE)pOrigSendTo)-5;
      VirtualProtect((PVOID)pOrigSendTo,SENDTOLEN,dwback,&dwback);
    }
  }
}

VOID UnHookSendTo(){
  DWORD dwback=0;
  if(pSendTo&&pOrigSendTo&&*((PBYTE)pOrigSendTo)==0xE9){
    VirtualProtect((PVOID)pOrigSendTo,SENDTOLEN,PAGE_READWRITE,&dwback);
    memmove((PBYTE)pOrigSendTo,(PBYTE)pSendTo,SENDTOLEN);
    VirtualProtect((PVOID)pOrigSendTo,SENDTOLEN,dwback,&dwback);
  }
}

char *strnchr(char *str, char ch, size_t siz){
  register char *p = str;
	register char c = ch;
	register size_t n = siz;
	if(p){
    while(n--!=0){
      if(*p==c)
        return p;
      p++;
    }
	}
	return NULL;
}

DWORD seed=0;

void steamsrand(){
  LARGE_INTEGER li;
  if(QueryPerformanceCounter(&li))
    seed=li.LowPart;
  else
    seed=GetTickCount();
}

int steamrand(unsigned int rmax){
  seed=134775813*seed+1;
  return seed*(INT64)rmax >> 32;
}

CHAR SteamProtocol::protocol[2048];
CHAR SteamProtocol::challenge[2048];

SteamProtocol::SteamProtocol(){
  header=0xFFFFFFFF;
  strcpy(connect,"connect");
  strcpy(getchallenge,"getchallenge");
  strcpy(steam,"steam");
  strcpy(svr,"48");
  memset(vr,0,sizeof(vr));
  memset(unknown,0,sizeof(unknown));
  arg1len=arg2len=0;
  memset(args,0,sizeof(args));
  memset(&pack,0,sizeof(pack));
  memset(protocol,0,sizeof(protocol));
}

void SteamProtocol::FillSteamId(int len,signed int aon,bool adspace,char *buf){
  char *p=buf;
  if(adspace)
    buf[len--]=' ';
  do{
    *p++=(aon||steamrand(2))?(steamrand(9)+'0'):(steamrand(25)+'A');
  }while(--len);
}

int SteamProtocol::MakeSteamId(char *buff){
  unsigned int i=0,id=0x4E67C6A7;
  steamsrand();
  switch(steamrand(9)){
    case 0:
      FillSteamId(8,false,false,buff);
      break;
    case 1:
      FillSteamId(9,false,true,buff);
      break;
    case 2:
      FillSteamId(9,true,false,buff);
      break;
    case 3:
      FillSteamId(9,false,false,buff);
      break;
    case 4:
      FillSteamId(10,true,false,buff);
      break;
    case 5:
      FillSteamId(11,true,true,buff);
      break;
    case 6:
      FillSteamId(12,false,false,buff);
      break;
    case 7:
      FillSteamId(14,false,false,buff);
      break;
    case 8:
      FillSteamId(15,false,true,buff);
      break;
    default:break;
  }
  for(i=0;buff[i];i++)
    id^=32*id+buff[i]+(id>>2);
  return (int)id;
  /*unsigned int len=0,id=0x4E67C6A7;
  steamsrand();
  do{
    buff[len]=steamrand(25)+'A';
    id^=32*id+buff[len++]+(id>>2);
  }while(len!=8);
  return (int)id;*/
}

bool SteamProtocol::ParseConnect(PCHAR buffer,UINT len){
  PCHAR p,q,vs,ve,end=buffer+len;
  INT i;
  if(len<14)return false;
  buffer+=12;

  vr[0]=buffer[0];
  vr[1]=buffer[1];
  if(strcmp(vr,svr))return false;

  p=strnchr(buffer+2,' ',len-14);
  if(!p)return false;p++;
  q=strnchr(p,' ',end-p);
  if(!q)return false;
  strncpy(unknown,p,q-p);

  p=strnchr(q,'\"',end-q);
  if(!p)return false;p++;
  q=strnchr(p,'\"',end-p);
  if(!q)return false;

  ve=p;i=0;
  while(1){
    vs=strnchr(ve,'\\',q-ve);
    if(!vs)break;vs++;
    ve=strnchr(vs,'\\',q-vs);
    if(!ve)break;
    strncpy(args[i].key,vs,ve-vs);
    args[i].key[ve-vs]=0;
    vs=strnchr(ve,'\\',q-ve);
    if(!vs)break;vs++;
    ve=strnchr(vs,'\\',q-vs);
    if(!ve)
      ve=q;
    strncpy(args[i].value,vs,ve-vs);
    args[i].value[ve-vs]=0;
    i++;
  }
  arg1len=i;

  p=strnchr(q+1,'\"',end-q);
  if(!p)return false;p++;
  q=strnchr(p,'\"',end-p);
  if(!q)return false;

  ve=p;
  while(1){
    vs=strnchr(ve,'\\',q-ve);
    if(!vs)break;vs++;
    ve=strnchr(vs,'\\',q-vs);
    if(!ve)break;
    strncpy(args[i].key,vs,ve-vs);
    args[i].key[ve-vs]=0;
    vs=strnchr(ve,'\\',q-ve);
    if(!vs)break;vs++;
    ve=strnchr(vs,'\\',q-vs);
    if(!ve)
      ve=q;
    strncpy(args[i].value,vs,ve-vs);
    args[i].value[ve-vs]=0;
    i++;
  }
  arg2len=i-arg1len;

  p=q+2;
  if(*p!='J')return false;

  return true;
}

char *SteamProtocol::DumpConnect(int* len){
  if(!len)return NULL;
  memset(protocol,0,sizeof(protocol));
  PCHAR p=protocol;
  *(PDWORD) p=header;
  strcat(p,connect);
  strcat(p," ");
  strcat(p,vr);
  strcat(p," ");
  strcat(p,unknown);
  if(arg1len){
    strcat(p," \"");
    for(int i=0;i<arg1len;i++){
      strcat(p,"\\");
      strcat(p,args[i].key);
      strcat(p,"\\");
      strcat(p,args[i].value);
    }
    strcat(p,"\"");
  }
  if(arg2len){
    strcat(p," \"");
    for(int i=arg1len;i<(arg1len+arg2len);i++){
      strcat(p,"\\");
      strcat(p,args[i].key);
      strcat(p,"\\");
      strcat(p,args[i].value);
    }
    strcat(p,"\"");
  }
  strcat(p,"\x0a");
  p+=strlen(p);

  pack.data=0x4A;//J
  pack.key1=MakeSteamId(pack.keys);
  pack.ver=0x00726576;//ver
  pack.unk1=0;
  pack.key2=2*((int)pack.key1<0 ? -pack.key1 : pack.key1);
  pack.key3=0x01100001;
  pack.unk[118]=0x20;
  pack.unk[122]=0x01;

  memcpy(p,&pack,sizeof(pack));
  p+=sizeof(pack);
  *len=p-protocol;
  return protocol;
}

bool SteamProtocol::ParseChallenge(PCHAR buffer,UINT len){
  if(len<22)return false;
  return !strncmp(&buffer[17],steam,5);
}

char *SteamProtocol::DumpChallenge(int* len){
  if(!len)return NULL;
  memset(challenge,0,sizeof(challenge));
  PCHAR p=challenge;
  *(PDWORD) p=header;
  strcat(p,getchallenge);
  strcat(p," ");
  strcat(p,steam);
  strcat(p,"\x0a");
  p+=strlen(p);
  *len=p-challenge;
  return challenge;
}

bool SteamProtocol::Parse(const char * &buf,int &len){
  if(len>3&&!strncmp(buf,(CHAR*)&header,4)){
    if(len>=11&&!strncmp(buf+4,connect,7)){

      if(ParseConnect((char*)buf,len))
        buf=DumpConnect(&len);

      return false;
    }else if(len>=16&&!strncmp(buf+4,getchallenge,12)){

      if(!ParseChallenge((char*)buf,len))
        return false;
      buf=DumpChallenge(&len);
    }
  }else
    return false;
  return true;
}

int PASCAL hSendTo(SOCKET s,const char *buf,int len,int flags,const struct sockaddr *to,int tolen){
  SteamProtocol steamconnect;
  if(!steamconnect.Parse(buf,len))
    UnHookSendTo();
  return pSendTo(s,buf,len,flags,to,tolen);
}
